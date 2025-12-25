cmake_minimum_required(VERSION 3.20)
project(barcoder)

set(CMAKE_CXX_STANDARD 20)

find_package(Catch2 REQUIRED)
find_package(libdmtx REQUIRED)
find_package(OpenCV REQUIRED)
find_package(ZXing REQUIRED)

add_executable(barcoder main.cpp)
target_link_libraries(barcoder PRIVATE Catch2::Catch2WithMain libdmtx::libdmtx opencv::opencv ZXing::ZXing)

include(FetchContent)
FetchContent_Declare(
        images
        URL https://mdpi-res.com/d_attachment/jimaging/jimaging-07-00163/article_deploy/jimaging-07-00163-s001.zip?version=1630062264
)

FetchContent_MakeAvailable(images)
