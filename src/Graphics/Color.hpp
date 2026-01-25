#pragma once

#include "Platform/PlatformMacros.hpp"
#include <cstdint>

namespace Izo {

struct Color {
    uint8_t r, g, b, a;

    constexpr Color() : r(0), g(0), b(0), a(255) {}
    constexpr Color(const uint8_t gray, const uint8_t alpha) : r(gray), g(gray), b(gray), a(alpha) {}
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    constexpr Color(uint32_t argb) {
        a = (argb >> 24) & 0xFF;

        IF_ANDROID(
            b = (argb >> 16) & 0xFF;
            g = (argb >> 8)  & 0xFF;
            r =  argb        & 0xFF;
        )
        IF_DESKTOP(
            r = (argb >> 16) & 0xFF;
            g = (argb >> 8)  & 0xFF;
            b =  argb        & 0xFF;
        )
    }

    constexpr uint32_t as_argb() const {
        IF_ANDROID(
            return (a << 24) | (b << 16) | (g << 8) | r;
        )
        IF_DESKTOP(
            return (a << 24) | (r << 16) | (g << 8) | b;
        )
    }

    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Transparent;
};

} 