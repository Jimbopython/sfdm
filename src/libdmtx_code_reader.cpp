#include <dmtx.h>
#include <sfdm/libdmtx_code_reader.hpp>

#include <algorithm>
#include <array>
#include <span>
#include <stdexcept>
#include <thread>

namespace {
    class DecodeGuard {
    public:
        explicit DecodeGuard(const sfdm::ImageView &image) :
            m_image(dmtxImageCreate(image.data, image.width, image.height, DmtxPack8bppK),
                    [](DmtxImage *dmtxImage) {
                        if (dmtxImage) {
                            dmtxImageDestroy(&dmtxImage);
                        }
                    }),
            m_decoder(dmtxDecodeCreate(m_image.get(), 1), [](DmtxDecode *decoder) {
                if (decoder) {
                    dmtxDecodeDestroy(&decoder);
                }
            }) {
            if (!m_image) {
                throw std::runtime_error("Could not create image!");
            }

            if (!m_decoder) {
                throw std::runtime_error("Could not create decoder!");
            }
        }

        std::shared_ptr<DmtxDecode> getDecoder() { return m_decoder; }

    private:
        std::shared_ptr<DmtxImage> m_image;
        std::shared_ptr<DmtxDecode> m_decoder;
    };

    uint32_t invertYAxis(size_t imageHeight, uint32_t value) { return static_cast<uint32_t>(imageHeight - 1 - value); }

    uint32_t roundToNearest(double value) { return static_cast<uint32_t>(value + 0.5); }

    sfdm::CodePosition getPosition(const sfdm::ImageView &image, const std::shared_ptr<DmtxRegion> &region) {
        DmtxVector2 bottomLeft{0, 0};
        DmtxVector2 topLeft{0, 1};
        DmtxVector2 bottomRight{1, 0};
        DmtxVector2 topRight{1, 1};

        dmtxMatrix3VMultiplyBy(&bottomLeft, region->fit2raw);
        dmtxMatrix3VMultiplyBy(&bottomRight, region->fit2raw);
        dmtxMatrix3VMultiplyBy(&topRight, region->fit2raw);
        dmtxMatrix3VMultiplyBy(&topLeft, region->fit2raw);

        return {sfdm::Point{roundToNearest(bottomLeft.X), invertYAxis(image.height, roundToNearest(bottomLeft.Y))},
                sfdm::Point{roundToNearest(topLeft.X), invertYAxis(image.height, roundToNearest(topLeft.Y))},
                sfdm::Point{roundToNearest(topRight.X), invertYAxis(image.height, roundToNearest(topRight.Y))},
                sfdm::Point{roundToNearest(bottomRight.X), invertYAxis(image.height, roundToNearest(bottomRight.Y))}};
    }
} // namespace

namespace sfdm {
    std::pair<std::shared_ptr<DmtxRegion>, LibdmtxCodeReader::StopCause>
    LibdmtxCodeReader::detectNext(const std::shared_ptr<DmtxDecode> &decoder) const {
        DmtxScanConstraint constraint{};

        DmtxTime timeout = dmtxTimeNow();
        timeout = dmtxTimeAdd(timeout, m_timeoutMSec);
        constraint.maxTimeout = &timeout;

        std::shared_ptr<DmtxRegion> region{
                dmtxRegionFindNextDeterministic(decoder.get(), m_timeoutMSec ? &constraint : nullptr),
                [](DmtxRegion *region) {
                    if (region) {
                        dmtxRegionDestroy(&region);
                    }
                }};
        return {region, static_cast<LibdmtxCodeReader::StopCause>(constraint.stopCause)};
    }

    [[nodiscard]] std::shared_ptr<DmtxMessage_struct>
    LibdmtxCodeReader::decode(const std::shared_ptr<DmtxDecode_struct> &decoder,
                              const std::shared_ptr<DmtxRegion_struct> &region) const {
        return {dmtxDecodeMatrixRegion(decoder.get(), region.get(), DmtxTrue), [](auto *message) {
                    if (message) {
                        dmtxMessageDestroy(&message);
                    }
                }};
    }

    std::vector<DecodeResult> LibdmtxCodeReader::decode(const ImageView &image) const {
        std::vector<DecodeResult> results;
        results.reserve(m_maximumNumberOfCodesToDetect);
        std::vector<std::jthread> threads;
        if (m_decodingFinishedCallback) {
            threads.reserve(m_maximumNumberOfCodesToDetect);
        }

        auto stream = decodeStream(image);

        while (stream.next()) {
            const auto decodeResult = stream.value();
            if (m_decodingFinishedCallback) {
                threads.emplace_back(m_decodingFinishedCallback, decodeResult);
            }
            results.emplace_back(decodeResult);
        }

        results.shrink_to_fit();

        return results;
    }

    ResultStream LibdmtxCodeReader::decodeStream(const ImageView &image) const {
        DecodeGuard decodeGuard(image);

        size_t detectedCodes = 0;
        while (detectedCodes < m_maximumNumberOfCodesToDetect) {
            const auto [region, stopCause] = detectNext(decodeGuard.getDecoder());
            // stopCause can be NotFound, but a valid region is returned, which may actually contain a valid code.
            if (!region && stopCause != StopCause::ScanSuccess) {
                co_return;
            }

            const auto message = decode(decodeGuard.getDecoder(), region);
            if (!message) {
                continue;
            }
            const CodePosition position = getPosition(image, region);
            DecodeResult decodeResult{reinterpret_cast<const char *>(message->output), position};

            ++detectedCodes;
            co_yield decodeResult;
        }
    }

    void LibdmtxCodeReader::setTimeout(uint32_t msec) { m_timeoutMSec = msec; }
    uint32_t LibdmtxCodeReader::getTimeout() const { return m_timeoutMSec; }

    bool LibdmtxCodeReader::isTimeoutSupported() { return true; }

    void LibdmtxCodeReader::setMaximumNumberOfCodesToDetect(uint32_t count) { m_maximumNumberOfCodesToDetect = count; }

    uint32_t LibdmtxCodeReader::getMaximumNumberOfCodesToDetect() const { return m_maximumNumberOfCodesToDetect; }

    void LibdmtxCodeReader::setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) {
        m_decodingFinishedCallback = callback;
    }
    bool LibdmtxCodeReader::isDecodingFinishedCallbackSupported() { return true; }
} // namespace sfdm
