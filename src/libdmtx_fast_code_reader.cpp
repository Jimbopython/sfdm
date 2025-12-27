#include <sfdm/libdmtx_fast_code_reader.hpp>
#include <dmtx.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_vector.h>

namespace {
    std::vector<cv::Rect> findDataMatrixROIs_MSER(const cv::Mat &input) {
        std::vector<cv::Rect> rois;

        cv::Ptr<cv::MSER> mser = cv::MSER::create(
            5, // delta
            100, // min area
            20000 // max area
        );

        std::vector<std::vector<cv::Point> > regions;
        std::vector<cv::Rect> boxes;

        mser->detectRegions(input, regions, boxes);

        for (const cv::Rect &r: boxes) {
            if (r.width < 16 || r.height < 16)
                continue;

            float aspect = static_cast<float>(r.width) / r.height;
            if (aspect < 0.5f || aspect > 2.0f)
                continue;

            rois.push_back(r);
        }

        return rois;
    }

    void mergeROIs(std::vector<cv::Rect> &rois) {
        for (size_t i = 0; i < rois.size(); ++i) {
            for (size_t j = i + 1; j < rois.size();) {
                cv::Rect intersection = rois[i] & rois[j];

                if (intersection.area() > 0) {
                    rois[i] = rois[i] | rois[j];
                    rois.erase(rois.begin() + j);
                } else {
                    ++j;
                }
            }
        }
    }

    std::vector<cv::Rect> findDataMatrixROIs_Canny(const cv::Mat &input) {
        std::vector<cv::Rect> rois;
        cv::Mat blurImg, edges, morph;

        cv::GaussianBlur(input, blurImg, cv::Size(3, 3), 0);
        cv::Canny(blurImg, edges, 60, 160);

        cv::Mat kernel = cv::getStructuringElement(
            cv::MORPH_RECT, cv::Size(5, 5)
        );
        cv::morphologyEx(edges, morph, cv::MORPH_CLOSE, kernel);

        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(
            morph, contours,
            cv::RETR_EXTERNAL,
            cv::CHAIN_APPROX_SIMPLE
        );

        for (const std::vector<cv::Point> &c: contours) {
            double area = cv::contourArea(c);
            if (area < 200)
                continue;

            cv::Rect r = cv::boundingRect(c);

            float aspect = static_cast<float>(r.width) / r.height;
            if (aspect < 0.5f || aspect > 2.0f)
                continue;

            rois.push_back(r);
        }

        return rois;
    }
}

namespace sfdm {
    std::vector<DecodeResult> LibdmtxFastCodeReader::decode(const cv::Mat &image) const {
        std::vector<cv::Rect> rois;

        auto mserROIs = findDataMatrixROIs_MSER(image);
        auto cannyROIs = findDataMatrixROIs_Canny(image);

        rois.insert(rois.end(), mserROIs.begin(), mserROIs.end());
        rois.insert(rois.end(), cannyROIs.begin(), cannyROIs.end());

        mergeROIs(rois);

        tbb::concurrent_vector<std::string> tmpData;
        tmpData.reserve(rois.size());

        tbb::parallel_for(
            tbb::blocked_range<size_t>(0, rois.size()),
            [&](const tbb::blocked_range<size_t> &range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const cv::Rect &r = rois[i];

                    cv::Mat roi = image(r).clone();
                    if (roi.cols < 120) {
                        cv::resize(roi, roi, cv::Size(), 2.0, 2.0, cv::INTER_CUBIC);
                    }
                    DmtxImage *dmtxImage = dmtxImageCreate(
                        roi.data, roi.cols, roi.rows, DmtxPack8bppK);
                    if (!dmtxImage) break;

                    DmtxDecode *decoder = dmtxDecodeCreate(dmtxImage, 1);
                    if (!decoder) {
                        dmtxImageDestroy(&dmtxImage);
                        break;
                    }

                    DmtxTime timeout = dmtxTimeNow();
                    timeout = dmtxTimeAdd(timeout, m_timeoutMSec);
                    DmtxRegion *region = dmtxRegionFindNext(decoder, m_timeoutMSec ? &timeout : nullptr);
                    if (!region) {
                        continue;
                    }

                    DmtxMessage *message =
                            dmtxDecodeMatrixRegion(decoder, region, DmtxTrue);

                    dmtxRegionDestroy(&region);
                    if (!message) {
                        continue;
                    }
                    tmpData.push_back({reinterpret_cast<const char *>(message->output)});
                    dmtxMessageDestroy(&message);


                    dmtxDecodeDestroy(&decoder);
                    dmtxImageDestroy(&dmtxImage);
                }
            });
        return {tmpData.begin(), tmpData.end()};
    }

    void LibdmtxFastCodeReader::setTimeout(uint32_t msec) {
        m_timeoutMSec = msec;
    }

    bool LibdmtxFastCodeReader::isTimeoutSupported() {
        return true;
    }
}
