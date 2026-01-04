#pragma once
#include <functional>
#include <sfdm/decode_result.hpp>
#include <sfdm/image_view.hpp>
#include <vector>

namespace sfdm {
    class ICodeReader {
    public:
        virtual ~ICodeReader() = default;

        // virtual std::vector<DetectionResult> detect(const cv::Mat& image) = 0;
        [[nodiscard]] virtual std::vector<DecodeResult> decode(const ImageView &image) const = 0;

        virtual void setTimeout(uint32_t msec) = 0;
        [[nodiscard]] virtual uint32_t getTimeout() const = 0;
        virtual bool isTimeoutSupported() = 0;

        virtual void setMaximumNumberOfCodesToDetect(uint32_t count) = 0;
        [[nodiscard]] virtual uint32_t getMaximumNumberOfCodesToDetect() const = 0;

        virtual void setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) = 0;
        virtual bool isDecodingFinishedCallbackSupported() = 0;
    };
} // namespace sfdm
