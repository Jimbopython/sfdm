#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <set>

#include <sfdm/sfdm.hpp>

#include "test_utils.hpp"

// note: start benchmarks with --benchmark-samples 57

namespace {
    auto getImagesAndCodeCounts(auto &imagesAndFileNames) {
        std::vector<std::string> fileNames;
        fileNames.reserve(imagesAndFileNames.size());
        std::ranges::transform(imagesAndFileNames, std::back_inserter(fileNames),
                               [](const auto &p) { return p.second; });

        std::vector<size_t> codeCounts;
        codeCounts.reserve(imagesAndFileNames.size());
        const auto annotations = readDataMatrixFile("../_deps/images-src/annotations.txt");
        for (auto fileName: fileNames) {
            std::filesystem::path p(fileName);
            p.replace_extension();
            if (annotations.contains(p.string())) {
                codeCounts.emplace_back(annotations.at(p.string()).size());
            } else {
                std::erase_if(imagesAndFileNames, [&](const auto &entry) { return entry.second == p.string(); });
            }
        }

        std::vector<sfdm::ImageView> images;
        images.reserve(imagesAndFileNames.size());
        std::ranges::transform(imagesAndFileNames, std::back_inserter(images), [](const auto &p) {
            auto &cvImage = p.first;
            return sfdm::ImageView{static_cast<size_t>(cvImage.cols), static_cast<size_t>(cvImage.rows), cvImage.data};
        });
        return std::make_pair(images, codeCounts);
    }
} // namespace

TEST_CASE("Decoder benchmark") {
    auto imagesAndFileNames = getImagesFromFiles();
    auto [images, codeCounts] = getImagesAndCodeCounts(imagesAndFileNames);

    int counter = 0;
    BENCHMARK_ADVANCED("ZXing")(Catch::Benchmark::Chronometer meter) {
        sfdm::ZXingCodeReader zxingCodeReader;
        meter.measure([&] {
            zxingCodeReader.setMaximumNumberOfCodesToDetect(codeCounts[counter % codeCounts.size()]);
            return zxingCodeReader.decode(images[counter++ % images.size()]);
        });
    };

    counter = 0;
    BENCHMARK_ADVANCED("Combined 0ms")(Catch::Benchmark::Chronometer meter) {
        sfdm::LibdmtxZXingCombinedCodeReader combinedReader;
        meter.measure([&] {
            combinedReader.setMaximumNumberOfCodesToDetect(codeCounts[counter % codeCounts.size()]);
            return combinedReader.decode(images[counter++ % images.size()]);
        });
    };

    counter = 0;
    BENCHMARK_ADVANCED("Combined 100ms")(Catch::Benchmark::Chronometer meter) {
        sfdm::LibdmtxZXingCombinedCodeReader combinedReader;
        combinedReader.setTimeout(100);
        meter.measure([&] {
            combinedReader.setMaximumNumberOfCodesToDetect(codeCounts[counter % codeCounts.size()]);
            return combinedReader.decode(images[counter++ % images.size()]);
        });
    };

    counter = 0;
    BENCHMARK_ADVANCED("Combined 200ms")(Catch::Benchmark::Chronometer meter) {
        sfdm::LibdmtxZXingCombinedCodeReader combinedReader;
        combinedReader.setTimeout(200);
        meter.measure([&] {
            combinedReader.setMaximumNumberOfCodesToDetect(codeCounts[counter % codeCounts.size()]);
            return combinedReader.decode(images[counter++ % images.size()]);
        });
    };

    counter = 0;
    BENCHMARK_ADVANCED(std::format("Libdmtx 0ms"))(Catch::Benchmark::Chronometer meter) {
        sfdm::LibdmtxCodeReader dmtxCodeReader;
        meter.measure([&] {
            dmtxCodeReader.setMaximumNumberOfCodesToDetect(codeCounts[counter % codeCounts.size()]);
            return dmtxCodeReader.decode(images[counter++ % images.size()]);
        });
    };

    counter = 0;
    BENCHMARK_ADVANCED(std::format("Libdmtx 100ms"))(Catch::Benchmark::Chronometer meter) {
        sfdm::LibdmtxCodeReader dmtxCodeReader100ms;
        dmtxCodeReader100ms.setTimeout(100);
        meter.measure([&] {
            dmtxCodeReader100ms.setMaximumNumberOfCodesToDetect(codeCounts[counter % codeCounts.size()]);
            return dmtxCodeReader100ms.decode(images[counter++ % images.size()]);
        });
    };

    counter = 0;
    BENCHMARK_ADVANCED(std::format("Libdmtx 200ms"))(Catch::Benchmark::Chronometer meter) {
        sfdm::LibdmtxCodeReader dmtxCodeReader200ms;
        dmtxCodeReader200ms.setTimeout(200);
        meter.measure([&] {
            dmtxCodeReader200ms.setMaximumNumberOfCodesToDetect(codeCounts[counter % codeCounts.size()]);
            return dmtxCodeReader200ms.decode(images[counter++ % images.size()]);
        });
    };
}
