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
    std::vector<sfdm::DecodeResult> filterDuplicates(const std::vector<sfdm::DecodeResult> &input1,
                                                     const std::vector<sfdm::DecodeResult> &input2) {
        std::vector<sfdm::DecodeResult> results;

        for (const sfdm::DecodeResult &result: input1) {
            bool isDuplicate = std::ranges::find_if(input2, [&](const sfdm::DecodeResult &res) {
                                   return diagonallyOppositeMatch(res.position, result.position);
                               }) != input2.end();
            if (!isDuplicate) {
                results.emplace_back(result);
            }
        }

        return results;
    }
} // namespace

namespace sfdm {
    std::vector<DecodeResult> LibdmtxZXingCombinedCodeReader::decode(const ImageView &image) const {
        const auto maximumNumberOfCodesToDetect = getMaximumNumberOfCodesToDetect();
        std::vector<DecodeResult> results;
        results.reserve(maximumNumberOfCodesToDetect);

        std::mutex resultsMutex;
        std::atomic zXingCount = 0;

        std::thread libdmtxThread([&] {
            auto stream = m_libdmtxCodeReader.decodeStream(image);
            int checkedCount = 0;
            bool doubleCheckZXing = m_doubleCheckZXing;
            while (stream.next()) {
                const auto result = stream.value();
                std::lock_guard lock(resultsMutex);
                if (!doubleCheckZXing && results.size() == maximumNumberOfCodesToDetect) {
                    return;
                }
                const auto it = std::ranges::find_if(results, [&](const DecodeResult &res) {
                    return diagonallyOppositeMatch(res.position, result.position);
                });
                if (results.end() == it) {
                    results.emplace_back(result);
                } else {
                    if (doubleCheckZXing) {
                        // we cannot trust zxing decoding. some results are wrong. libdmtx works better.
                        if (it->text != result.text) {
                            *it = result;
                        }
                        if (results.size() == maximumNumberOfCodesToDetect && ++checkedCount == zXingCount) {
                            return;
                        }
                    }
                }
                if (!doubleCheckZXing && results.size() == maximumNumberOfCodesToDetect) {
                    return;
                }
            }
        });
        std::thread zxingThread([&] {
            const auto result = m_zxingCodeReader.decode(image);
            std::lock_guard lock(resultsMutex);
            if (results.size() == maximumNumberOfCodesToDetect) {
                return;
            }
            const auto filteredResults = filterDuplicates(result, results);
            zXingCount = results.size();
            results.insert(results.end(), filteredResults.begin(), filteredResults.end());
        });

        zxingThread.join();
        libdmtxThread.join();
        results.shrink_to_fit();

        return results;
    }

    void LibdmtxZXingCombinedCodeReader::setTimeout(uint32_t msec) { m_libdmtxCodeReader.setTimeout(msec); }
    uint32_t LibdmtxZXingCombinedCodeReader::getTimeout() const { return m_libdmtxCodeReader.getTimeout(); }

    bool LibdmtxZXingCombinedCodeReader::isTimeoutSupported() { return true; }

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
    void LibdmtxZXingCombinedCodeReader::setDoubleCheckZXing(bool value) { m_doubleCheckZXing = value; }
    bool LibdmtxZXingCombinedCodeReader::getDoubleCheckZXing() const { return m_doubleCheckZXing; }
} // namespace sfdm
