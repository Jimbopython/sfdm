#pragma once
#include <memory>
#include <sfdm/icode_reader.hpp>
#include <vector>

extern "C" {
struct DmtxRegion_struct;
struct DmtxDecode_struct;
struct DmtxMessage_struct;
}

namespace sfdm {
    class LibdmtxCodeReader : public ICodeReader {
    public:
        [[nodiscard]] std::vector<DecodeResult> decode(const ImageView &image) const override;

        void setTimeout(uint32_t msec) override;

        bool isTimeoutSupported() override;

        void setMaximumNumberOfCodesToDetect(uint32_t count) override;
        uint32_t getMaximumNumberOfCodesToDetect() const override;

        void setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) override;
        bool isDecodingFinishedCallbackSupported() override;

    private:
        enum class StopCause {
            ScanNotFound,
            ScanSuccess,
            ScanTimeLimit,
            ScanIterLimit,
        };

        [[nodiscard]] std::pair<std::shared_ptr<DmtxRegion_struct>, StopCause>
        detectNext(const std::shared_ptr<DmtxDecode_struct> &decoder) const;

        [[nodiscard]] std::shared_ptr<DmtxMessage_struct>
        decode(const std::shared_ptr<DmtxDecode_struct> &decoder,
               const std::shared_ptr<DmtxRegion_struct> &region) const;

        uint32_t m_timeoutMSec{};
        uint32_t m_maximumNumberOfCodesToDetect{255};
        std::function<void(DecodeResult)> m_decodingFinishedCallback{};
    };
} // namespace sfdm
