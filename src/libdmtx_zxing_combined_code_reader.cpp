#include <sfdm/libdmtx_zxing_combined_code_reader.hpp>
#include <stdexcept>
#include <thread>

namespace {
    struct Rect {
        size_t x;
        size_t y;
        size_t width;
        size_t height;
    };

    void fillRectFromPoints(const sfdm::Point &p1, const sfdm::Point &p2, const sfdm::Point &p3, const sfdm::Point &p4,
                            sfdm::ImageView &view) {
        // 1. Compute bounds
        auto minX = std::min({p1.x, p2.x, p3.x, p4.x});
        auto maxX = std::max({p1.x, p2.x, p3.x, p4.x});
        auto minY = std::min({p1.y, p2.y, p3.y, p4.y});
        auto maxY = std::max({p1.y, p2.y, p3.y, p4.y});

        // 2. Rect (x, y, w, h)
        Rect r{minX, minY, maxX - minX, maxY - minY};

        if (r.width <= 0 || r.height <= 0)
            return;

        // 3. Clip to framebuffer
        auto startX = r.x;
        auto startY = r.y;
        auto endX = std::min(view.width, r.x + r.width);
        auto endY = std::min(view.height, r.y + r.height);

        auto rowPixels = endX - startX;
        if (rowPixels <= 0)
            return;

        // 4. Fill (memset per row)
        for (int y = startY; y < endY; ++y) {
            auto *row = view.data + y * view.width + startX;
            std::memset(row, 0, rowPixels);
        }
    }
} // namespace

namespace sfdm {
    std::vector<DecodeResult> LibdmtxZXingCombinedCodeReader::decode(const ImageView &image) const {
        auto results = m_zxingCodeReader.decode(image);
        if (results.size() == getMaximumNumberOfCodesToDetect()) {
            return results;
        }

        auto tmpBuffer = std::make_unique<uint8_t[]>(image.width * image.height);
        std::memcpy(tmpBuffer.get(), image.data, image.width * image.height);

        ImageView imageCopy{image.width, image.height, tmpBuffer.get()};
        for (const auto &result: results) {
            fillRectFromPoints(result.position.topLeft, result.position.topRight, result.position.bottomLeft,
                               result.position.bottomRight, imageCopy);
        }
        LibdmtxCodeReader libdmtxReader;
        libdmtxReader.setMaximumNumberOfCodesToDetect(getMaximumNumberOfCodesToDetect() - results.size());
        const auto libdmtxResults = libdmtxReader.decode(imageCopy);
        results.insert(results.end(), libdmtxResults.begin(), libdmtxResults.end());
        return results;
    }

    void LibdmtxZXingCombinedCodeReader::setTimeout(uint32_t msec) {
        throw std::runtime_error("LibdmtxZXingCombinedCodeReader::setTimeout is not supported.");
    }

    bool LibdmtxZXingCombinedCodeReader::isTimeoutSupported() { return false; }

    void LibdmtxZXingCombinedCodeReader::setMaximumNumberOfCodesToDetect(uint32_t count) {
        m_zxingCodeReader.setMaximumNumberOfCodesToDetect(count);
    }
    uint32_t LibdmtxZXingCombinedCodeReader::getMaximumNumberOfCodesToDetect() const {
        return m_zxingCodeReader.getMaximumNumberOfCodesToDetect();
    }
    void LibdmtxZXingCombinedCodeReader::setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) {
        throw std::runtime_error("LibdmtxZXingCombinedCodeReader::setDecodingFinishedCallback is not supported.");
    }
    bool LibdmtxZXingCombinedCodeReader::isDecodingFinishedCallbackSupported() { return false; }
} // namespace sfdm
