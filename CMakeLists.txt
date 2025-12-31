cmake_minimum_required(VERSION 3.20)
project(sfdm C CXX)

option(sfdm_BUILD_TESTS "build Tests" OFF)
option(sfdm_WITH_ZXING_DECODER "build with ZXING decoder" ON)
option(sfdm_WITH_LIBDMTX_DECODER "build with LIBDMTX decoder" ON)

if (NOT sfdm_WITH_ZXING_DECODER AND NOT sfdm_WITH_LIBDMTX_DECODER)
    message(FATAL_ERROR "Library has to be built with eiter ZXING or LIBDMTX")
endif ()

set(CMAKE_CXX_STANDARD 20)

if (sfdm_WITH_ZXING_DECODER)
    find_package(ZXing REQUIRED)
endif ()
if (sfdm_WITH_LIBDMTX_DECODER)
    find_package(libdmtx REQUIRED)
endif ()

add_library(sfdm)

include(cmake/create_config_header.cmake)
create_config_header(sfdm)

target_sources(sfdm
    PRIVATE
        $<$<BOOL:${sfdm_WITH_ZXING_DECODER}>:src/zxing_code_reader.cpp>
        $<$<BOOL:${sfdm_WITH_LIBDMTX_DECODER}>:src/libdmtx_code_reader.cpp>
    PUBLIC
        FILE_SET headers
        TYPE HEADERS
        BASE_DIRS
        include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        FILES
        include/sfdm/decode_result.hpp
        include/sfdm/icode_reader.hpp
        include/sfdm/image_view.hpp
        $<$<BOOL:${sfdm_WITH_LIBDMTX_DECODER}>:include/sfdm/libdmtx_code_reader.hpp>
        include/sfdm/sfdm.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/include/sfdm/sfdm_config.hpp
        $<$<BOOL:${sfdm_WITH_ZXING_DECODER}>:include/sfdm/zxing_code_reader.hpp>
)
set_target_properties(sfdm PROPERTIES
        VERIFY_INTERFACE_HEADER_SETS ON
)

target_include_directories(sfdm
        PUBLIC
        include
)

target_compile_features(sfdm PUBLIC cxx_std_20)
target_link_libraries(sfdm
        PRIVATE
        $<$<BOOL:${sfdm_WITH_ZXING_DECODER}>:ZXing::ZXing>
        $<$<BOOL:${sfdm_WITH_LIBDMTX_DECODER}>:libdmtx::libdmtx>)

if (sfdm_BUILD_TESTS)
    add_subdirectory(test)
endif ()

install(TARGETS sfdm FILE_SET headers)
