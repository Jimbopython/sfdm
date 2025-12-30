#include <dmtx.h>
#include <sfdm/libdmtx_code_reader.hpp>
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
        DecodeGuard decodeGuard(image);

        std::vector<DecodeResult> results;
        results.reserve(m_maximumNumberOfCodesToDetect);

        while (results.size() < m_maximumNumberOfCodesToDetect) {
            const auto [region, stopCause] = detectNext(decodeGuard.getDecoder());
            // stopCause can be NotFound, but a valid region is returned, which may actually contain a valid code.
            if (!region && stopCause != StopCause::ScanSuccess) {
                break;
            }

            const auto message = decode(decodeGuard.getDecoder(), region);
            if (!message) {
                break;
            }

            DecodeResult decodeResult{reinterpret_cast<const char *>(message->output)};
            if (m_decodingFinishedCallback) {
                std::thread(m_decodingFinishedCallback, decodeResult).detach();
            }
            results.emplace_back(decodeResult);
        }

        results.shrink_to_fit();

        return results;
    }

    void LibdmtxCodeReader::setTimeout(uint32_t msec) { m_timeoutMSec = msec; }

    bool LibdmtxCodeReader::isTimeoutSupported() { return true; }

    void LibdmtxCodeReader::setMaximumNumberOfCodesToDetect(uint32_t count) { m_maximumNumberOfCodesToDetect = count; }
    void LibdmtxCodeReader::setDecodingFinishedCallback(std::function<void(DecodeResult)> callback) {
        m_decodingFinishedCallback = callback;
    }
    bool LibdmtxCodeReader::isDecodingFinishedCallbackSupported() { return true; }
} // namespace sfdm
