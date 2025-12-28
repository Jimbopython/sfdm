#include <sfdm/libdmtx_code_reader.hpp>
#include <dmtx.h>

namespace {
    class DecodeGuard {
    public:
        explicit DecodeGuard(const cv::Mat &image) : m_image(dmtxImageCreate(
                                                                 image.data, image.cols, image.rows, DmtxPack8bppK),
                                                             [](DmtxImage *dmtxImage) {
                                                                 if (dmtxImage) {
                                                                     dmtxImageDestroy(&dmtxImage);
                                                                 }
                                                             }), m_decoder(dmtxDecodeCreate(m_image.get(), 1),
                                                                           [](DmtxDecode *decoder) {
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

        std::shared_ptr<DmtxDecode> getDecoder() {
            return m_decoder;
        }

    private:
        std::shared_ptr<DmtxImage> m_image;
        std::shared_ptr<DmtxDecode> m_decoder;
    };
}

namespace sfdm {
    std::shared_ptr<DmtxRegion> LibdmtxCodeReader::detectNext(
        const std::shared_ptr<DmtxDecode> &decoder) const {
        DmtxTime timeout = dmtxTimeNow();
        timeout = dmtxTimeAdd(timeout, m_timeoutMSec);
        return {
            dmtxRegionFindNext(decoder.get(), m_timeoutMSec ? &timeout : nullptr),
            [](DmtxRegion *region) {
                if (region) {
                    dmtxRegionDestroy(&region);
                }
            }
        };
    }

    [[nodiscard]] std::shared_ptr<DmtxMessage_struct>
    LibdmtxCodeReader::decode(const std::shared_ptr<DmtxDecode_struct> &decoder,
                              const std::shared_ptr<DmtxRegion_struct> &region) const {
        return {
            dmtxDecodeMatrixRegion(decoder.get(), region.get(), DmtxTrue),
            [](auto *message) {
                if (message) {
                    dmtxMessageDestroy(&message);
                }
            }
        };
    }

    std::vector<DecodeResult> LibdmtxCodeReader::decode(const cv::Mat &image) const {
        DecodeGuard decodeGuard(image);

        std::vector<DecodeResult> results;

        for (int i = 0; i < 255; ++i) {
            const auto region = detectNext(decodeGuard.getDecoder());
            if (!region) {
                break;
            }

            const auto message = decode(decodeGuard.getDecoder(), region);
            if (!message) {
                break;
            }

            results.emplace_back(DecodeResult{reinterpret_cast<const char *>(message->output)});
        }

        return results;
    }

    void LibdmtxCodeReader::setTimeout(uint32_t msec) {
        m_timeoutMSec = msec;
    }

    bool LibdmtxCodeReader::isTimeoutSupported() {
        return true;
    }
}
