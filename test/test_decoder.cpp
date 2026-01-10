#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <ranges>
#include <sfdm/libdmtx_code_reader.hpp>
#include <sfdm/zxing_code_reader.hpp>
#include <string>
#include "sfdm/libdmtx_zxing_combined_code_reader.hpp"
#include "test_utils.hpp"

// #define BUILD_FOR_PLOTS
// #define PAINT_FOUND_CODES

namespace {
    auto getTexts(const auto &foundData) {
        std::vector<std::string> foundTexts;
        foundTexts.reserve(foundData.size());
        std::ranges::transform(foundData.begin(), foundData.end(), std::back_inserter(foundTexts),
                               [](const auto &result) {
                                   auto text = result.text;
                                   // some codes actually contain newlines, but annotations dont
                                   std::erase(text, '\r');
                                   std::replace(text.begin(), text.end(), '\n', ' ');
                                   return text;
                               });
        return foundTexts;
    }
    void checkTexts(std::vector<std::string> foundText, const auto &currentData) {
        const auto foundCount = foundText.size() - extraElementsCount(foundText, currentData);
        std::vector<sfdm::Point> points;
        points.reserve(foundText.size() * 4);
#ifndef BUILD_FOR_PLOTS
        CHECK(foundCount == currentData.size());
#endif
        for (const auto &text: currentData) {
            const auto itText = std::find(foundText.begin(), foundText.end(), text);
            bool requirementTextFound = itText != foundText.end();
            CAPTURE(text);
            CHECK(requirementTextFound);
            if (itText != foundText.end()) {
                foundText.erase(itText);
            }
        }
    }

    auto getPositions(const auto &foundData) {
        std::vector<sfdm::Point> points;
        points.reserve(foundData.size() * 4);
        for (const auto &code: foundData) {
            const auto position = code.position;
            points.emplace_back(code.position.topLeft);
            points.emplace_back(code.position.topRight);
            points.emplace_back(code.position.bottomRight);
            points.emplace_back(code.position.bottomLeft);
        }
        std::ranges::sort(points);
        return points;
    }

    void checkPositions(const std::vector<sfdm::Point> &points) {
        constexpr int r2 = 5 * 5;

        for (size_t i = 0; i < points.size(); ++i) {
            for (size_t j = i + 1; j < points.size(); ++j) {
                int dx = points[i].x - points[j].x;
                int dy = points[i].y - points[j].y;
                CHECK(dx * dx + dy * dy > r2);
            }
        }
    }

    template<typename Callable>
    void testDecoding(const Callable &callable) {
        const auto data = readDataMatrixFile("../_deps/images-src/annotations.txt");

        SECTION("Single") {
            for (const auto &[image, fileName]: getImagesFromFiles()) {
                const auto it = data.find(fileName);
                if (it == data.end()) {
                    continue;
                }
                const auto currentData = it->second;
                SECTION(fileName) {
                    auto foundCodes = callable(image, fileName, currentData.size());
                    auto foundText = getTexts(foundCodes);
                    checkTexts(foundText, currentData);
#ifndef BUILD_FOR_PLOTS
                    checkPositions(getPositions(foundCodes));
#endif
                }
            }
        }
        SECTION("Overall") {
            size_t foundTotal = 0;
            size_t totalCodes = 0;
            for (const auto &[image, fileName]: getImagesFromFiles()) {

                const auto it = data.find(fileName);
                if (it == data.end()) {
                    continue;
                }
                const auto currentData = it->second;
                const auto foundCodes = callable(image, fileName, currentData.size());
                const auto foundTexts = getTexts(foundCodes);
                const auto foundCount = foundTexts.size() - extraElementsCount(foundTexts, currentData);
                foundTotal += foundCount;
                totalCodes += currentData.size();
            }
            REQUIRE(foundTotal == totalCodes);
        }
    }

    auto testReader(auto &reader, const cv::Mat &image, [[maybe_unused]] std::string libName,
                    [[maybe_unused]] std::string codeName) {
        std::vector<sfdm::DecodeResult> foundData;

#ifndef BUILD_FOR_PLOTS
        if (reader.isDecodeWithCallbackSupported()) {
            std::vector<sfdm::DecodeResult> callbackData;
            std::mutex dataMutex;

            foundData = reader.decode({static_cast<size_t>(image.cols), static_cast<size_t>(image.rows), image.data},
                                      [&](auto result) {
                                          std::lock_guard lock(dataMutex);
                                          callbackData.emplace_back(result);
                                      });

            REQUIRE(foundData == callbackData);
        } else {
            foundData = reader.decode({static_cast<size_t>(image.cols), static_cast<size_t>(image.rows), image.data});
        }
#endif
#ifdef PAINT_FOUND_CODES
        cv::Mat colorImage;
        cv::cvtColor(image, colorImage, cv::COLOR_GRAY2BGR);
        for (const auto &data: foundData) {
            std::array cvData{
                    cv::Point{static_cast<int>(data.position.topLeft.x), static_cast<int>(data.position.topLeft.y)},
                    cv::Point{static_cast<int>(data.position.topRight.x), static_cast<int>(data.position.topRight.y)},
                    cv::Point{static_cast<int>(data.position.bottomRight.x),
                              static_cast<int>(data.position.bottomRight.y)},
                    cv::Point{static_cast<int>(data.position.bottomLeft.x),
                              static_cast<int>(data.position.bottomLeft.y)}};
            cv::polylines(colorImage, cvData, true, cv::Scalar(0, 255, 0), 2);
            cv::imwrite(std::format("{}_{}.png", codeName, libName), colorImage);
        }
#endif

        return foundData;
    }
} // namespace

TEST_CASE("LibDMTX Decoding") {
    const auto timeout = GENERATE_REF(from_range(std::vector{100, 200, 0}));
    SECTION(std::to_string(timeout) + "ms timeout") {
        testDecoding([&](const cv::Mat &image, const std::string &codeName, size_t expectedNumberOfCodes) {
            sfdm::LibdmtxCodeReader reader;
            reader.setTimeout(timeout);
            reader.setMaximumNumberOfCodesToDetect(expectedNumberOfCodes);
            return testReader(reader, image, "libdmtx", codeName);
        });
    }
}

TEST_CASE("ZXing Decoding") {
    testDecoding([](const cv::Mat &image, const std::string &codeName, size_t expectedNumberOfCodes) {
        sfdm::ZXingCodeReader reader;
        reader.setMaximumNumberOfCodesToDetect(expectedNumberOfCodes);
        return testReader(reader, image, "zxing", codeName);
    });
}

TEST_CASE("Combined Decoding") {
    const auto timeout = GENERATE_REF(from_range(std::vector{100, 200, 0}));
    SECTION(std::to_string(timeout) + "ms timeout") {
        testDecoding([&](const cv::Mat &image, const std::string &codeName, size_t expectedNumberOfCodes) {
            sfdm::LibdmtxZXingCombinedCodeReader reader;
            reader.setTimeout(timeout);
            reader.setMaximumNumberOfCodesToDetect(expectedNumberOfCodes);
            return testReader(reader, image, "combined", codeName);
        });
    }
}

TEST_CASE("Foo") {
    auto rotate = [](const sfdm::ImageView &input, const sfdm::ImageView &output) {
        for (int y = 0; y < input.height; ++y) {
            const uint8_t *srcRow = input.data + (input.height - 1 - y) * input.width;
            uint8_t *dstRow = output.data + y * output.width;

            for (int x = 0; x < input.width; ++x) {
                dstRow[x] = srcRow[input.width - 1 - x];
            }
        }
    };
    auto [image, _] = getImagesFromFiles()[0];

    auto imageCopy = image.clone();
    rotate(sfdm::ImageView{static_cast<size_t>(image.cols), static_cast<size_t>(image.rows), image.data},
           sfdm::ImageView{static_cast<size_t>(imageCopy.cols), static_cast<size_t>(imageCopy.rows), imageCopy.data});
    cv::imwrite("foo.png", imageCopy);
}
