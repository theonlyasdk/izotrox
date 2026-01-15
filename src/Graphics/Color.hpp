#pragma once
#include <cstdint>

namespace Izo {

struct Color {
    uint8_t r, g, b, a;

    constexpr Color() : r(0), g(0), b(0), a(255) {}
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    constexpr Color(uint32_t argb) {
        a = (argb >> 24) & 0xFF;
#ifdef __ANDROID__
        b = (argb >> 16) & 0xFF;
        g = (argb >> 8)  & 0xFF;
        r =  argb        & 0xFF;
#else
        r = (argb >> 16) & 0xFF;
        g = (argb >> 8)  & 0xFF;
        b =  argb        & 0xFF;
#endif
    }

    constexpr uint32_t to_argb() const {
#ifdef __ANDROID__
        return (a << 24) | (b << 16) | (g << 8) | r;
#else
        return (a << 24) | (r << 16) | (g << 8) | b;
#endif
    }

    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Transparent;
};

} // namespace Izo