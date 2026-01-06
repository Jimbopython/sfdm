#pragma once
#include <memory>
#include <sfdm/icode_reader.hpp>

namespace sfdm {
    struct ZXingCodeReaderImpl;

    /*!
     * Code Reader using zxing in the backend.
     * Best for speed, but not that accurate.
     */
    class ZXingCodeReader : public ICodeReader {
    public:
        ZXingCodeReader();

        ~ZXingCodeReader() override;

        // DetectionResult detect(const cv::Mat &image) override;

        /*!
         * Decode datamatrix codes in the provided image.
         * This is a blocking call until the decoding of all datamatrix codes in the image are finished.
         * It is recommended to set the number of datamatrix codes that can be detected, because then this function will
         * return faster.
         * @param image image used for datamatrix code detection and decoding
         * @return Decoded results that were found in the image
         */
        [[nodiscard]] std::vector<DecodeResult> decode(const ImageView &image) const override;

        /*!
         * Decode datamatrix codes in the provided image.
         * This is a blocking call until the decoding of all datamatrix codes in the image are finished. However,
         * results can be queried faster by using the callback.
         * It is recommended to set the number of datamatrix codes that can be detected, because then this function will
         * return faster.
         * @param image image used for datamatrix code detection and decoding
         * @param callback callback function that will be called when a DecodeResult is ready
         * @return Decoded results that were found in the image
         */
        [[nodiscard]] std::vector<DecodeResult> decode(const ImageView &image,
                                                       std::function<void(DecodeResult)> callback) const override;
        void setTimeout(uint32_t msec) override;
        [[nodiscard]] uint32_t getTimeout() const override;
        bool isTimeoutSupported() override;

        void setMaximumNumberOfCodesToDetect(size_t count) override;
        [[nodiscard]] size_t getMaximumNumberOfCodesToDetect() const override;

        bool isDecodeWithCallbackSupported() override;

    private:
        std::unique_ptr<ZXingCodeReaderImpl> m_impl;
    };
} // namespace sfdm
