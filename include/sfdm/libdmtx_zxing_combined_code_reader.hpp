#pragma once
#include <sfdm/icode_reader.hpp>
#include <sfdm/libdmtx_code_reader.hpp>
#include <sfdm/zxing_code_reader.hpp>
#include <vector>

namespace sfdm {
    class LibdmtxZXingCombinedCodeReader : public ICodeReader {
    public:
        [[nodiscard]] std::vector<DecodeResult> decode(const ImageView &image) const override;

        void setTimeout(uint32_t msec) override;
        [[nodiscard]] uint32_t getTimeout() const override;
        bool isTimeoutSupported() override;

        void setMaximumNumberOfCodesToDetect(uint32_t count) override;
        [[nodiscard]] uint32_t getMaximumNumberOfCodesToDetect() const override;

        void setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) override;
        bool isDecodingFinishedCallbackSupported() override;

        void setDoubleCheckZXing(bool value);
        [[nodiscard]] bool getDoubleCheckZXing() const;

    private:
        LibdmtxCodeReader m_libdmtxCodeReader;
        ZXingCodeReader m_zxingCodeReader;
        std::atomic<bool> m_doubleCheckZXing{true};
    };
} // namespace sfdm
