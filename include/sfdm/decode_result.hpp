#pragma once
#include <string>

namespace sfdm {
    struct Point {
        uint32_t x{};
        uint32_t y{};
        Point(uint32_t x, uint32_t y) : x{x}, y{y} {}
        Point() = default;
        auto operator<=>(const Point &) const = default;
        Point(const Point &other) = default;
        Point(Point &&other) noexcept = default;
        Point &operator=(const Point &other) = default;
        Point &operator=(Point &&other) noexcept = default;
    };
    struct CodePosition {
        Point topLeft{};
        Point topRight{};
        Point bottomLeft{};
        Point bottomRight{};
        CodePosition() = default;
        CodePosition(Point topLeft, Point topRight, Point bottomLeft, Point bottomRight) :
            topLeft{topLeft}, topRight{topRight}, bottomLeft{bottomLeft}, bottomRight{bottomRight} {}
        auto operator<=>(const CodePosition &) const = default;
        CodePosition(const CodePosition &other) = default;
        CodePosition(CodePosition &&other) noexcept = default;
        CodePosition &operator=(const CodePosition &other) = default;
        CodePosition &operator=(CodePosition &&other) noexcept = default;
    };
    struct DecodeResult {
        std::string text;
        CodePosition position{};
        DecodeResult() = default;
        DecodeResult(std::string text, const CodePosition &position) : text{std::move(text)}, position{position} {}
        auto operator<=>(const DecodeResult &) const = default;
        DecodeResult(const DecodeResult &other) = default;
        DecodeResult(DecodeResult &&other) noexcept = default;
        DecodeResult &operator=(const DecodeResult &other) = default;
        DecodeResult &operator=(DecodeResult &&other) noexcept = default;
    };
} // namespace sfdm
