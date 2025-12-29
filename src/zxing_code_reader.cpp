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

    std::vector<DecodeResult> ZXingCodeReader::decode(const cv::Mat &image) const {
        ZXing::ImageView source{image.data, image.cols, image.rows, ZXing::ImageFormat::Lum};

        const auto results = ZXing::ReadBarcodes(source, m_impl->options);

        std::vector<DecodeResult> decodeResults;
        std::ranges::transform(results, std::back_inserter(decodeResults),
                               [&](const auto &result) { return DecodeResult{result.text()}; });
        return decodeResults;
    }

    void ZXingCodeReader::setTimeout(uint32_t msec) { throw std::runtime_error{"setTimeout is not supported!"}; }

    bool ZXingCodeReader::isTimeoutSupported() { return false; }

    void ZXingCodeReader::setMaximumNumberOfCodesToDetect(uint32_t count) {
        m_impl->options.setMaxNumberOfSymbols(count);
    }
    void ZXingCodeReader::setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) {
        throw std::runtime_error{"setDecodingFinishedCallback is not supported!"};
    }
    bool ZXingCodeReader::isDecodingFinishedCallbackSupported() { return false; }
} // namespace sfdm
