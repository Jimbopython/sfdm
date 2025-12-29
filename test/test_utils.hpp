#pragma once

#include <filesystem>
#include <map>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

int extraElementsCount(const std::vector<std::string> &a, const std::vector<std::string> &requirement);

std::map<std::string, std::vector<std::string>> readDataMatrixFile(const std::string &filename);

cv::Mat get_image(const std::filesystem::directory_entry &entry);
