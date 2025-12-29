cmake_minimum_required(VERSION 3.20)
project(sfdm C CXX)

option(sfdm_BUILD_TESTS "build Tests" ON)
option(sfdm_WITH_ZXING_DECODER "build with ZXING decoder" ON)
option(sfdm_WITH_LIBDMTX_DECODER "build with LIBDMTX decoder" ON)

if (NOT sfdm_WITH_ZXING_DECODER AND NOT sfdm_WITH_LIBDMTX_DECODER)
    message(FATAL_ERROR "Library has to be built with eiter ZXING or LIBDMTX")
endif ()

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
if (sfdm_WITH_ZXING_DECODER)
    find_package(ZXing REQUIRED)
endif ()
if (sfdm_WITH_LIBDMTX_DECODER)
    find_package(libdmtx REQUIRED)
endif ()

add_library(sfdm)

target_sources(sfdm
    PRIVATE
        $<$<BOOL:${sfdm_WITH_ZXING_DECODER}>:src/zxing_code_reader.cpp>
        $<$<BOOL:${sfdm_WITH_LIBDMTX_DECODER}>:src/libdmtx_code_reader.cpp>
    PUBLIC
        FILE_SET headers
        TYPE HEADERS
        BASE_DIRS include
        FILES
        include/sfdm/decode_result.hpp
        include/sfdm/icode_reader.hpp
        $<$<BOOL:${sfdm_WITH_LIBDMTX_DECODER}>:include/sfdm/libdmtx_code_reader.hpp>
        $<$<BOOL:${sfdm_WITH_ZXING_DECODER}>:include/sfdm/zxing_code_reader.hpp>
)

target_include_directories(sfdm
        PUBLIC
        include
)

target_compile_features(sfdm PUBLIC cxx_std_20)
target_link_libraries(sfdm
        PRIVATE
        $<$<BOOL:${sfdm_WITH_ZXING_DECODER}>:ZXing::ZXing>
        $<$<BOOL:${sfdm_WITH_LIBDMTX_DECODER}>:libdmtx::libdmtx>
        PUBLIC
            opencv::opencv)

if (sfdm_BUILD_TESTS)
    add_subdirectory(test)
endif ()
