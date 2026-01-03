# sfdm - Simple, fast datamatrix

sfdm is a library which targets to provide a fast, accurate and easy to use datamatrix code reader.

# Usage

There are three implementations to choose from:

A code reader based on [ZXing](https://github.com/zxing-cpp/zxing-cpp), a code reader based on
[libdmtx](https://github.com/dmtx/libdmtx) and a combination of both.

### ZXing code reader

```c++
#include <sfdm/sfdm.hpp>
#include <iostream>
sfdm::ImageView view{image.width, image.height, image.data};
sfdm::ZXingCodeReader reader;
sfdm::DecodeResult result = reader.decode(view);
std::cout << result.text << '\n';
```

### libdmtx code reader

```c++
#include <sfdm/sfdm.hpp>
#include <iostream>
sfdm::ImageView view{image.width, image.height, image.data};
sfdm::LibdmtxCodeReader reader;
sfdm::DecodeResult result = reader.decode(view);
std::cout << result.text << '\n';
```

### Combined code reader

```c++
#include <sfdm/sfdm.hpp>
#include <iostream>
sfdm::ImageView view{image.width, image.height, image.data};
sfdm::LibdmtxZXingCombinedCodeReader reader;
sfdm::DecodeResult result = reader.decode(view);
std::cout << result.text << '\n';
```

## How to build

The sfdm library needs [ZXing](https://github.com/zxing-cpp/zxing-cpp) or
[libdmtx](https://github.com/dmtx/libdmtx) or both as dependency. The recommended way is to build
dependencies with [conan](https://docs.conan.io/2/index.html). The conan file is provided
[here](conan/all/conanfile.py).

### Build instructions

Change directory into root of this repository.

In order to build the sfdm library without conan use the following (dependencies must be present):

```bash
cmake -B build -S .
cmake --build build
```

To build the sfdm library with conan use the following:

```bash
cmake --preset=conan-<build_type>
cmake --build --preset=conan-<build_type>
```

# Detection results

[See here](doc/detection_results.md)