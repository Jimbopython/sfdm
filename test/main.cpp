#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <ranges>
#include <sfdm/zxing_code_reader.hpp>
#include <sfdm/libdmtx_code_reader.hpp>
#include <sfdm/libdmtx_fast_code_reader.hpp>

int extraElementsCount(const std::vector<std::string> &a,
                       const std::vector<std::string> &requirement) {
    std::unordered_map<std::string, int> freq;

    for (const auto &x: a)
        ++freq[x];

    for (const auto &x: requirement)
        --freq[x];

    int extras = 0;
    for (const auto &count: freq | std::views::values)
        if (count > 0)
            extras += count;

    return extras;
}

std::map<std::string, std::vector<std::string> > readDataMatrixFile(const std::string &filename) {
    std::map<std::string, std::vector<std::string> > result;
    std::ifstream file(filename);

    if (!file.is_open())
        return result;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        const auto pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key = line.substr(0, pos);
        std::string values = line.substr(pos + 1);

        std::stringstream ss(values);
        std::string token;

        while (std::getline(ss, token, '|')) {
            if (!token.empty()) {
                std::erase(token, '"');
                result[key].emplace_back(token);
            }
        }
    }

    return result;
}

cv::Mat get_image(const std::filesystem::directory_entry &entry) {
    cv::Mat image = cv::imread(entry.path().string(), cv::IMREAD_COLOR);

    // Convert to 8-bit grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Ensure it's 8-bit (usually already is)
    gray.convertTo(gray, CV_8U);
    return gray;
}

template<typename Callable>
void test(const Callable &callable) {
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

TEST_CASE("LibDMTX") {
    const auto timeout = GENERATE_REF(from_range(std::vector{100, 200, 0}));
    SECTION(std::to_string(timeout) + "ms timeout") {
        test(
            [&](const cv::Mat &image) {
                sfdm::LibdmtxCodeReader reader;
                reader.setTimeout(timeout);
                return testReader(reader, image);
            }
        );
    }
}

TEST_CASE("ZXing") {
    test(
        [](const cv::Mat &image) {
            const sfdm::ZXingCodeReader reader;
            return testReader(reader, image);
        }
    );
}

TEST_CASE("Fast LibDMTX") {
    const auto timeout = GENERATE_REF(from_range(std::vector{100, 200, 0}));
    SECTION(std::to_string(timeout) + "ms timeout") {
        test(
            [&](const cv::Mat &image) {
                sfdm::LibdmtxFastCodeReader reader;
                reader.setTimeout(timeout);
                return testReader(reader, image);
            }
        );
    }
}
