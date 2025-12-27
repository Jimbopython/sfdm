#pragma once
#include <sfdm/icode_reader.hpp>

namespace sfdm {
    class LibdmtxFastCodeReader : public ICodeReader {
    public:
        [[nodiscard]] std::vector<DecodeResult> decode(const cv::Mat &image) const override;

        void setTimeout(uint32_t msec) override;

        bool isTimeoutSupported() override;

    private:
        uint32_t m_timeoutMSec{};
    };
}
