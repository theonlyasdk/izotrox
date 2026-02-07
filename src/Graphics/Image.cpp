#include "Core/Application.hpp"
#include "Debug/Logger.hpp"
#include "Graphics/Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "Lib/stb_image.h"

namespace Izo {

Image::Image(const std::string& path) {
    data = stbi_load(path.c_str(), &w, &h, &channels, 4); 

    if (!data) {
        LogError("Failed to load image: {}", path);
    }
}

Image::~Image() {
    if (data) {
        stbi_image_free(data);
    }
}

void Image::draw(Painter& painter, IntPoint pos) {
    if (!data) return;
    if (!Application::the().screen_rect().contains(pos)) return;

    for (int iy = 0; iy < h; ++iy) {
        for (int ix = 0; ix < w; ++ix) {
            int offset = (iy * w + ix) * 4;
            uint8_t r = data[offset];
            uint8_t g = data[offset+1];
            uint8_t b = data[offset+2];
            uint8_t a = data[offset+3];

            if (a > 0) {
                painter.draw_pixel({pos.x + ix, pos.y + iy}, Color(r, g, b, a));
            }
        }
    }
}

void Image::draw_scaled(Painter& painter, const IntRect& rect, Anchor anchor) {
    if (!data || rect.w <= 0 || rect.h <= 0) return;
    if (!Application::the().screen_rect().contains(rect.x, rect.y)) return;

    int dx = rect.x;
    int dy = rect.y;
    int dw = rect.w;
    int dh = rect.h;

    switch (anchor) {
        case Anchor::TopLeft: break;
        case Anchor::TopRight: dx -= dw; break;
        case Anchor::BottomLeft: dy -= dh; break;
        case Anchor::BottomRight: dx -= dw; dy -= dh; break;
        case Anchor::Center: dx -= dw / 2; dy -= dh / 2; break;
        case Anchor::CenterStartHoriz: dx -= dw / 2; break; 
        case Anchor::CenterStartVert: dy -= dh / 2; break; 
        case Anchor::CenterEndHoriz: dx -= dw / 2; dy -= dh; break; 
        case Anchor::CenterEndVert: dx -= dw; dy -= dh / 2; break; 
    }

    for (int iy = 0; iy < dh; ++iy) {
        int sy = (iy * h) / dh;
        if (sy >= h) sy = h - 1;

        for (int ix = 0; ix < dw; ++ix) {
            int sx = (ix * w) / dw;
            if (sx >= w) sx = w - 1;

            int offset = (sy * w + sx) * 4;
            uint8_t r = data[offset];
            uint8_t g = data[offset+1];
            uint8_t b = data[offset+2];
            uint8_t a = data[offset+3];

            if (a > 0) {
                painter.draw_pixel({dx + ix, dy + iy}, Color(r, g, b, a));
            }
        }
    }
}

} 
