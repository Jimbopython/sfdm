#include <sfdm/libdmtx_code_reader.hpp>
#include <dmtx.h>

namespace sfdm {
    std::vector<DecodeResult> LibdmtxCodeReader::decode(const cv::Mat &image) const {
        DmtxImage *dmtxImage = dmtxImageCreate(
            image.data, image.cols, image.rows, DmtxPack8bppK);
        if (!dmtxImage) return {};

        DmtxDecode *decoder = dmtxDecodeCreate(dmtxImage, 1);
        if (!decoder) {
            dmtxImageDestroy(&dmtxImage);
            return {};
        }

        std::vector<DecodeResult> results;

        for (int i = 0; i < 255; ++i) {
            DmtxTime timeout = dmtxTimeNow();
            timeout = dmtxTimeAdd(timeout, m_timeoutMSec);
            DmtxRegion *region = dmtxRegionFindNext(decoder, m_timeoutMSec ? &timeout : nullptr);
            if (!region) {
                break;
            }

            DmtxMessage *message =
                    dmtxDecodeMatrixRegion(decoder, region, DmtxTrue);

            dmtxRegionDestroy(&region);
            if (!message) {
                break;
            }
            results.emplace_back(DecodeResult{reinterpret_cast<const char *>(message->output)});
            dmtxMessageDestroy(&message);
        }
        dmtxDecodeDestroy(&decoder);
        dmtxImageDestroy(&dmtxImage);

        return results;
    }

    void LibdmtxCodeReader::setTimeout(uint32_t msec) {
        m_timeoutMSec = msec;
    }

    bool LibdmtxCodeReader::isTimeoutSupported() {
        return true;
    }
}
