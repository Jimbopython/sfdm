#pragma once
#include <sfdm/decode_result.hpp>
#include <opencv2/opencv.hpp>

namespace sfdm {
    class ICodeReader {
    public:
        virtual ~ICodeReader() = default;

        //virtual std::vector<DetectionResult> detect(const cv::Mat& image) = 0;
        [[nodiscard]] virtual std::vector<DecodeResult> decode(const cv::Mat &image) const = 0;

        virtual void setTimeout(uint32_t msec) = 0;
 virtual void setMaximumNumberOfCodesToDetect(uint32_t count) = 0;

        virtual bool isTimeoutSupported() = 0;
    };
}
