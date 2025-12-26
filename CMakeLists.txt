cmake_minimum_required(VERSION 3.20)
project(barcoder C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
find_package(ZXing REQUIRED)

add_library(sfdm)

# Add sources
target_sources(sfdm
    PRIVATE
        src/zxing_code_reader.cpp
    PUBLIC
        FILE_SET headers
        TYPE HEADERS
        BASE_DIRS include
        FILES
        include/sfdm/decode_result.hpp
        include/sfdm/icode_reader.hpp
        include/sfdm/zxing_code_reader.hpp
)

target_include_directories(sfdm
        PUBLIC
        include
)

target_compile_features(sfdm PUBLIC cxx_std_20)
target_link_libraries(sfdm
        PRIVATE
            ZXing::ZXing
        PUBLIC
            opencv::opencv)

add_subdirectory(test)
