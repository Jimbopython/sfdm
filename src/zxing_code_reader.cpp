#include <ZXing/ReadBarcode.h>
#include <sfdm/zxing_code_reader.hpp>

namespace sfdm {
    struct ZXingCodeReaderImpl {
        ZXing::ReaderOptions options;
    };

    ZXingCodeReader::ZXingCodeReader() : m_impl{std::make_unique<ZXingCodeReaderImpl>()} {
        m_impl->options.setBinarizer(ZXing::Binarizer::LocalAverage);
        m_impl->options.setTryHarder(true);
        m_impl->options.setFormats(ZXing::BarcodeFormat::DataMatrix);
    }

    ZXingCodeReader::~ZXingCodeReader() = default;

    std::vector<DecodeResult> ZXingCodeReader::decode(const ImageView &image) const {
        ZXing::ImageView source{image.data, static_cast<int>(image.width), static_cast<int>(image.height),
                                ZXing::ImageFormat::Lum};

        const auto results = ZXing::ReadBarcodes(source, m_impl->options);

        std::vector<DecodeResult> decodeResults;
        std::ranges::transform(results, std::back_inserter(decodeResults), [&](const auto &result) {
            const auto zXingPosition = result.position();
            const auto topLeft = zXingPosition.topLeft();
            const auto topRight = zXingPosition.topRight();
            const auto bottomLeft = zXingPosition.bottomLeft();
            const auto bottomRight = zXingPosition.bottomRight();
            const CodePosition codePosition{{
                                                    static_cast<uint32_t>(topLeft.x),
                                                    static_cast<uint32_t>(topLeft.y),
                                            },
                                            {
                                                    static_cast<uint32_t>(topRight.x),
                                                    static_cast<uint32_t>(topRight.y),
                                            },
                                            {
                                                    static_cast<uint32_t>(bottomLeft.x),
                                                    static_cast<uint32_t>(bottomLeft.y),
                                            },
                                            {
                                                    static_cast<uint32_t>(bottomRight.x),
                                                    static_cast<uint32_t>(bottomRight.y),
                                            }};
            return DecodeResult{result.text(), codePosition};
        });
        return decodeResults;
    }

    void ZXingCodeReader::setTimeout(uint32_t msec) { throw std::runtime_error{"setTimeout is not supported!"}; }

    bool ZXingCodeReader::isTimeoutSupported() { return false; }

    void ZXingCodeReader::setMaximumNumberOfCodesToDetect(uint32_t count) {
        m_impl->options.setMaxNumberOfSymbols(count);
    }
    uint32_t ZXingCodeReader::getMaximumNumberOfCodesToDetect() const { return m_impl->options.maxNumberOfSymbols(); }
    void ZXingCodeReader::setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) {
        throw std::runtime_error{"setDecodingFinishedCallback is not supported!"};
    }
    bool ZXingCodeReader::isDecodingFinishedCallbackSupported() { return false; }
} // namespace sfdm
