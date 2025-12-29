#pragma once
#include <string>

namespace sfdm {
    struct DecodeResult {
        const std::string text;
        auto operator<=>(const DecodeResult &) const = default;
    };
} // namespace sfdm
