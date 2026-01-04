#pragma once
#include <coroutine>
#include <string>
#include <utility>

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
        Point bottomLeft{};
        Point topLeft{};
        Point topRight{};
        Point bottomRight{};
        CodePosition() = default;
        CodePosition(Point bottomLeft, Point topLeft, Point topRight, Point bottomRight) :
            bottomLeft{bottomLeft}, topLeft{topLeft}, topRight{topRight}, bottomRight{bottomRight} {}
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
    class ResultStream {
    public:
        struct promise_type {
            DecodeResult current;

            ResultStream get_return_object() {
                return ResultStream{std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_always initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }

            std::suspend_always yield_value(DecodeResult value) {
                current = std::move(value);
                return {};
            }

            void return_void() {}
            void unhandled_exception() {}
        };

        using handle_t = std::coroutine_handle<promise_type>;

        explicit ResultStream(handle_t h) : m_handle(h) {}
        ResultStream(ResultStream &&other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }

        ~ResultStream() {
            if (m_handle) {
                m_handle.destroy();
            }
        }

        bool next() {
            if (!m_handle || m_handle.done())
                return false;
            m_handle.resume();
            return !m_handle.done();
        }

        const DecodeResult &value() const { return m_handle.promise().current; }

    private:
        handle_t m_handle;
    };

} // namespace sfdm
