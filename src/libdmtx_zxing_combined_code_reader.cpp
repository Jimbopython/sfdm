#include <sfdm/libdmtx_zxing_combined_code_reader.hpp>

#include <future>
#include <stdexcept>
#include <thread>

namespace {
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
    std::vector<std::tuple<sfdm::DecodeResult, sfdm::ResultOrigin, bool>>
    filterDuplicates(const std::vector<sfdm::DecodeResult> &input1,
                     const std::vector<std::tuple<sfdm::DecodeResult, sfdm::ResultOrigin, bool>> &input2) {
        std::vector<std::tuple<sfdm::DecodeResult, sfdm::ResultOrigin, bool>> results;

        for (const sfdm::DecodeResult &result: input1) {
            bool isDuplicate = std::ranges::find_if(input2, [&](const auto &res) {
                                   const auto decodeResult = std::get<0>(res);
                                   return diagonallyOppositeMatch(decodeResult.position, result.position);
                               }) != input2.end();
            if (!isDuplicate) {
                results.emplace_back(result, sfdm::ResultOrigin::ZXing, false);
            }
        }

        return results;
    }

    auto rotate = [](const sfdm::ImageView &input, const sfdm::ImageView &output) {
        for (int y = 0; y < input.height; ++y) {
            const uint8_t *srcRow = input.data + (input.height - 1 - y) * input.width;
            uint8_t *dstRow = output.data + y * output.width;

            for (int x = 0; x < input.width; ++x) {
                dstRow[x] = srcRow[input.width - 1 - x];
            }
        }
    };

    sfdm::CodePosition rotated(const sfdm::ImageView &image, const sfdm::CodePosition &position) {
        sfdm::Point bottomLeft{static_cast<uint32_t>(image.width - 1 - position.bottomLeft.x),
                               static_cast<uint32_t>(image.height - 1 - position.bottomLeft.y)};
        sfdm::Point topLeft{static_cast<uint32_t>(image.width - 1 - position.topLeft.x),
                            static_cast<uint32_t>(image.height - 1 - position.topLeft.y)};
        sfdm::Point topRight{static_cast<uint32_t>(image.width - 1 - position.topRight.x),
                             static_cast<uint32_t>(image.height - 1 - position.topRight.y)};
        sfdm::Point bottomRight{static_cast<uint32_t>(image.width - 1 - position.bottomRight.x),
                                static_cast<uint32_t>(image.height - 1 - position.bottomRight.y)};

        return {bottomLeft, topLeft, topRight, bottomRight};
    }
} // namespace

namespace sfdm {
    void
    LibdmtxZXingCombinedCodeReader::libdmtxWorker(const ImageView &image, std::mutex &resultsMutex,
                                                  std::vector<std::tuple<DecodeResult, ResultOrigin, bool>> &results,
                                                  size_t maximumNumberOfCodesToDetect, bool rotatedImage) const {
        (void) maximumNumberOfCodesToDetect;
        auto stream = m_libdmtxCodeReader.decodeStream(image);

        while (stream.next()) {
            auto result = stream.value();
            if (rotatedImage) {
                result.position = rotated(image, result.position);
            }
            std::lock_guard guard(resultsMutex);
            const auto it = std::ranges::find_if(results, [&](const std::tuple<DecodeResult, ResultOrigin, bool> &res) {
                const auto decodeResult = std::get<0>(res);
                return diagonallyOppositeMatch(decodeResult.position, result.position);
            });
            const std::tuple res{result, ResultOrigin::Libdmtx, true};
            if (results.end() == it) {
                results.emplace_back(res);
            } else {
                const auto isChecked = std::get<2>(*it);
                if (!isChecked) {
                    *it = res;
                }
            }
        }
    }

    std::vector<DecodeResult> LibdmtxZXingCombinedCodeReader::decode(const ImageView &image) const {
        const auto maximumNumberOfCodesToDetect = getMaximumNumberOfCodesToDetect();
        std::vector<std::tuple<DecodeResult, ResultOrigin, bool>> results;
        results.reserve(maximumNumberOfCodesToDetect);

        std::mutex resultsMutex;

        std::thread libdmtxThread([&] {
            std::jthread rightDirection(&LibdmtxZXingCombinedCodeReader::libdmtxWorker, this, std::cref(image),
                                        std::ref(resultsMutex), std::ref(results), maximumNumberOfCodesToDetect, false);

            const auto bufferRotated = std::make_unique<uint8_t[]>(image.width * image.height);
            const ImageView rotatedImage{image.width, image.height, bufferRotated.get()};
            rotate(image, rotatedImage);

            libdmtxWorker(rotatedImage, resultsMutex, results, maximumNumberOfCodesToDetect, true);
        });


        std::thread zxingThread([&] {
            const auto result = m_zxingCodeReader.decode(image);
            std::lock_guard lock(resultsMutex);
            if (results.size() == maximumNumberOfCodesToDetect) {
                return;
            }
            const auto filteredResults = filterDuplicates(result, results);
            results.insert(results.end(), filteredResults.begin(), filteredResults.end());
        });

        zxingThread.join();
        libdmtxThread.join();
        std::vector<DecodeResult> decodeResults;
        decodeResults.reserve(results.size());
        std::ranges::transform(results, std::back_inserter(decodeResults),
                               [](const auto &result) { return std::get<0>(result); });

        return decodeResults;
    }

    std::vector<DecodeResult> LibdmtxZXingCombinedCodeReader::decode(const ImageView &image,
                                                                     std::function<void(DecodeResult)> callback) const {
        (void) image;
        (void) callback;
        throw std::runtime_error("Decode with callback is not supported!");
    }

    void LibdmtxZXingCombinedCodeReader::setTimeout(uint32_t msec) { m_libdmtxCodeReader.setTimeout(msec); }
    uint32_t LibdmtxZXingCombinedCodeReader::getTimeout() const { return m_libdmtxCodeReader.getTimeout(); }

    bool LibdmtxZXingCombinedCodeReader::isTimeoutSupported() { return true; }

    void LibdmtxZXingCombinedCodeReader::setMaximumNumberOfCodesToDetect(size_t count) {
        m_libdmtxCodeReader.setMaximumNumberOfCodesToDetect(count);
        m_zxingCodeReader.setMaximumNumberOfCodesToDetect(count);
    }
    size_t LibdmtxZXingCombinedCodeReader::getMaximumNumberOfCodesToDetect() const {
        return m_libdmtxCodeReader.getMaximumNumberOfCodesToDetect();
    }
    bool LibdmtxZXingCombinedCodeReader::isDecodeWithCallbackSupported() { return false; }
    void LibdmtxZXingCombinedCodeReader::setDoubleCheckZXing(bool value) { m_doubleCheckZXing = value; }
    bool LibdmtxZXingCombinedCodeReader::getDoubleCheckZXing() const { return m_doubleCheckZXing; }
} // namespace sfdm
