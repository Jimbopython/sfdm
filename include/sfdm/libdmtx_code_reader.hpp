#pragma once
#include <sfdm/icode_reader.hpp>

extern "C" {
struct DmtxRegion_struct;
struct DmtxDecode_struct;
struct DmtxMessage_struct;
}

namespace sfdm {
    class LibdmtxCodeReader : public ICodeReader {
    public:
        [[nodiscard]] std::vector<DecodeResult> decode(const cv::Mat &image) const override;

        void setTimeout(uint32_t msec) override;

        bool isTimeoutSupported() override;

    private:
        [[nodiscard]] std::shared_ptr<DmtxRegion_struct>
        detectNext(const std::shared_ptr<DmtxDecode_struct> &decoder) const;

        [[nodiscard]] std::shared_ptr<DmtxMessage_struct>
        decode(const std::shared_ptr<DmtxDecode_struct> &decoder,
               const std::shared_ptr<DmtxRegion_struct> &region) const;

        uint32_t m_timeoutMSec{};
    };
}
