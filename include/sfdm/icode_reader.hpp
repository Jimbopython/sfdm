#pragma once
#include <sfdm/decode_result.hpp>
#include <opencv2/opencv.hpp>

namespace sfdm {
    class ICodeReader {
    public:
        virtual ~ICodeReader() = default;

        //virtual std::vector<DetectionResult> detect(const cv::Mat& image) = 0;
        virtual std::vector<DecodeResult> decode(const cv::Mat &image) = 0;
    };
}
