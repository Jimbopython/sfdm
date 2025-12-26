#pragma once
#include <sfdm/icode_reader.hpp>

namespace sfdm {
    struct ZXingCodeReaderImpl;


    class ZXingCodeReader : public ICodeReader {
    public:
        ZXingCodeReader();

        //DetectionResult detect(const cv::Mat &image) override;

        std::vector<DecodeResult> decode(const cv::Mat &image) override;

    private:
        std::unique_ptr<ZXingCodeReaderImpl> m_impl;
    };
}
