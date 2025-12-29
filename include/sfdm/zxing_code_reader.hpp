#pragma once
#include <sfdm/icode_reader.hpp>

namespace sfdm {
    struct ZXingCodeReaderImpl;


    class ZXingCodeReader : public ICodeReader {
    public:
        ZXingCodeReader();

        ~ZXingCodeReader() override;

        // DetectionResult detect(const cv::Mat &image) override;

        [[nodiscard]] std::vector<DecodeResult> decode(const cv::Mat &image) const override;

        void setTimeout(uint32_t msec) override;

        bool isTimeoutSupported() override;

        void setMaximumNumberOfCodesToDetect(uint32_t count) override;

    private:
        std::unique_ptr<ZXingCodeReaderImpl> m_impl;
    };
} // namespace sfdm
