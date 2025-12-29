#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <ranges>
#include <sfdm/libdmtx_code_reader.hpp>
#include <sfdm/zxing_code_reader.hpp>
#include <string>
#include "test_utils.hpp"

template<typename Callable>
void testDecoding(const Callable &callable) {
    const auto data = readDataMatrixFile("../_deps/images-src/annotations.txt");

    SECTION("Single") {
        for (const auto &entry: std::filesystem::directory_iterator("../_deps/images-src")) {
            if (entry.is_regular_file() && entry.path().extension() == ".jpg") {
                cv::Mat image = get_image(entry);

                const auto fileName = entry.path().stem().string();
                const auto it = data.find(fileName);
                if (it == data.end()) {
                    continue;
                }
                const auto currentData = it->second;
                SECTION(fileName) {
                    const auto foundTexts = callable(image);
                    const auto foundCount = foundTexts.size() - extraElementsCount(foundTexts, currentData);
                    CAPTURE(foundTexts, currentData);
                    REQUIRE(foundCount == currentData.size());
                }
            }
        }
    }
    SECTION("Overall") {
        int foundTotal = 0;
        int totalCodes = 0;
        for (const auto &entry: std::filesystem::directory_iterator("../_deps/images-src")) {
            if (entry.is_regular_file() && entry.path().extension() == ".jpg") {
                cv::Mat image = get_image(entry);

                const auto fileName = entry.path().stem().string();
                const auto it = data.find(fileName);
                if (it == data.end()) {
                    continue;
                }
                const auto currentData = it->second;
                const auto foundTexts = callable(image);
                const auto foundCount = foundTexts.size() - extraElementsCount(foundTexts, currentData);
                foundTotal += foundCount;
                totalCodes += currentData.size();
            }
        }
        REQUIRE(foundTotal == totalCodes);
    }
}

auto testReader(const auto &reader, const cv::Mat &image) {
    const auto foundData = reader.decode(image);
    std::vector<std::string> foundTexts;
    foundTexts.reserve(foundData.size());
    std::ranges::transform(foundData.begin(), foundData.end(), std::back_inserter(foundTexts), [](const auto &result) {
        auto text = result.text;
        // some codes actually contain newlines, but annotations dont
        std::erase(text, '\r');
        std::replace(text.begin(), text.end(), '\n', ' ');
        return text;
    });
    return foundTexts;
}

TEST_CASE("LibDMTX Decoding") {
    const auto timeout = GENERATE_REF(from_range(std::vector{100, 200, 0}));
    SECTION(std::to_string(timeout) + "ms timeout") {
        testDecoding([&](const cv::Mat &image) {
            sfdm::LibdmtxCodeReader reader;
            reader.setTimeout(timeout);
            return testReader(reader, image);
        });
    }
}

TEST_CASE("ZXing Decoding") {
    testDecoding([](const cv::Mat &image) {
        const sfdm::ZXingCodeReader reader;
        return testReader(reader, image);
    });
}
