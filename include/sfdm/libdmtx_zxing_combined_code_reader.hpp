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

        bool isTimeoutSupported() override;

        void setMaximumNumberOfCodesToDetect(uint32_t count) override;
        uint32_t getMaximumNumberOfCodesToDetect() const override;

        void setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) override;
        bool isDecodingFinishedCallbackSupported() override;

    private:
        LibdmtxCodeReader m_libdmtxCodeReader;
        ZXingCodeReader m_zxingCodeReader;
    };
} // namespace sfdm
