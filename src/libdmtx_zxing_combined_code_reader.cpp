#include <sfdm/libdmtx_zxing_combined_code_reader.hpp>

#include <future>
#include <stdexcept>
#include <thread>

namespace {
    struct Rect {
        size_t x;
        size_t y;
        size_t width;
        size_t height;
    };

    template<size_t distance = 5>
    bool within5Pixels(const sfdm::Point &p1, const sfdm::Point &p2) {
        const auto dx = p1.x - p2.x;
        const auto dy = p1.y - p2.y;
        return dx * dx + dy * dy <= distance * distance;
    }

    bool diagonallyOppositeMatch(const sfdm::CodePosition &q1, const sfdm::CodePosition &q2) {
        if (within5Pixels(q1.bottomLeft, q2.bottomLeft) && within5Pixels(q1.topRight, q2.topRight)) {
            return true;
        }

        return within5Pixels(q1.topLeft, q2.topLeft) && within5Pixels(q1.bottomRight, q2.bottomRight);
    }
    std::vector<sfdm::DecodeResult> filterDuplicates(const std::vector<sfdm::DecodeResult> &input) {
        std::vector<sfdm::DecodeResult> result;

        for (const auto &q: input) {
            bool isDuplicate = false;

            for (const auto &existing: result) {
                if (diagonallyOppositeMatch(q.position, existing.position)) {
                    isDuplicate = true;
                    break;
                }
            }

            if (!isDuplicate) {
                result.push_back(q);
            }
        }

        return result;
    }
} // namespace

namespace sfdm {
    std::vector<DecodeResult> LibdmtxZXingCombinedCodeReader::decode(const ImageView &image) const {
        std::promise<std::vector<DecodeResult>> promiseLibdmtx;
        auto futureLibdmtx = promiseLibdmtx.get_future();
        std::promise<std::vector<DecodeResult>> promiseZXing;
        auto futureZXing = promiseZXing.get_future();

        std::jthread libdmtxThread([&image, this, p = std::move(promiseLibdmtx)]() mutable {
            p.set_value(m_libdmtxCodeReader.decode(image));
        });
        std::jthread zxingThread([&image, this, p = std::move(promiseZXing)]() mutable {
            p.set_value(m_zxingCodeReader.decode(image));
        });

        auto results = futureLibdmtx.get();
        auto libdmtxResults = futureZXing.get();

        results.insert(results.end(), libdmtxResults.begin(), libdmtxResults.end());

        return filterDuplicates(results);
    }

    void LibdmtxZXingCombinedCodeReader::setTimeout(uint32_t msec) {
        throw std::runtime_error("LibdmtxZXingCombinedCodeReader::setTimeout is not supported.");
    }

    bool LibdmtxZXingCombinedCodeReader::isTimeoutSupported() { return false; }

    void LibdmtxZXingCombinedCodeReader::setMaximumNumberOfCodesToDetect(uint32_t count) {
        m_libdmtxCodeReader.setMaximumNumberOfCodesToDetect(count);
        m_zxingCodeReader.setMaximumNumberOfCodesToDetect(count);
    }
    uint32_t LibdmtxZXingCombinedCodeReader::getMaximumNumberOfCodesToDetect() const {
        return m_libdmtxCodeReader.getMaximumNumberOfCodesToDetect();
    }
    void LibdmtxZXingCombinedCodeReader::setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) {
        throw std::runtime_error("LibdmtxZXingCombinedCodeReader::setDecodingFinishedCallback is not supported.");
    }
    bool LibdmtxZXingCombinedCodeReader::isDecodingFinishedCallbackSupported() { return false; }
} // namespace sfdm
