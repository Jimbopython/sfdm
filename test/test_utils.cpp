#pragma once

#include "test_utils.hpp"

#include <fstream>
#include <map>
#include <ranges>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

int extraElementsCount(const std::vector<std::string> &a, const std::vector<std::string> &requirement) {
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

std::map<std::string, std::vector<std::string>> readDataMatrixFile(const std::string &filename) {
    std::map<std::string, std::vector<std::string>> result;
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
