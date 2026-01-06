#pragma once
#include <atomic>
#include <sfdm/icode_reader.hpp>
#include <sfdm/libdmtx_code_reader.hpp>
#include <sfdm/zxing_code_reader.hpp>
#include <vector>

namespace sfdm {
    /*!
     * Code Reader using libdmtx and zxing in the backend.
     * Best for quantity, but can be slow, when timeout is not set or too high
     */
    class LibdmtxZXingCombinedCodeReader : public ICodeReader {
    public:
        /*!
         * Decode datamatrix codes in the provided image.
         * This is a blocking call until the decoding of all datamatrix codes in the image are finished.
         * It is recommended to set the number of datamatrix codes that can be detected, because then this function will
         * return faster. How fast the function "gives up" searching for codes in the image, can be tuned with the
         * setTimeout function.
         * @param image image used for datamatrix code detection and decoding
         * @return Decoded results that were found in the image
         */
        [[nodiscard]] std::vector<DecodeResult> decode(const ImageView &image) const override;

        /*!
         * Decode datamatrix codes in the provided image.
         * This is a blocking call until the decoding of all datamatrix codes in the image are finished. However,
         * results can be queried faster by using the callback.
         * It is recommended to set the number of datamatrix codes that can be detected, because then this function will
         * return faster. How fast the function "gives up" searching for codes in the image, can be tuned with the
         * setTimeout function.
         * @param image image used for datamatrix code detection and decoding
         * @param callback callback function that will be called when a DecodeResult is ready
         * @return Decoded results that were found in the image
         */
        [[nodiscard]] std::vector<DecodeResult> decode(const ImageView &image,
                                                       std::function<void(DecodeResult)> callback) const override;

        /*!
         * This is a timeout that will be reset after each detection of one code in an image.
         * This timeout is for detection only, not decoding. It is only applied to the libdmtx backend.
         * Note: 0 is until really nothing can be found
         * @param msec
         */
        void setTimeout(uint32_t msec) override;
        [[nodiscard]] uint32_t getTimeout() const override;
        bool isTimeoutSupported() override;

        void setMaximumNumberOfCodesToDetect(size_t count) override;
        [[nodiscard]] size_t getMaximumNumberOfCodesToDetect() const override;

        bool isDecodeWithCallbackSupported() override;

        /*!
         * Sets whether zxing results shall be double checked by libdmtx. This was proven to be necessary, as the tests
         * from this library showed, that zxing decodes some codes wrongly (wrong text). libdmtx decodes them better.
         * This was a rare case (<5%). Setting this to false makes the decode functions return earlier.
         * @param value Value to set
         */
        void setDoubleCheckZXing(bool value);
        [[nodiscard]] bool getDoubleCheckZXing() const;

    private:
        LibdmtxCodeReader m_libdmtxCodeReader;
        ZXingCodeReader m_zxingCodeReader;
        std::atomic<bool> m_doubleCheckZXing{true};
    };
} // namespace sfdm
