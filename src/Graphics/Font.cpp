#define STB_TRUETYPE_IMPLEMENTATION
#include "Font.hpp"
#include "stb_truetype.h"
#include <cstdio>
#include <iostream>
#include <cmath>

namespace Izo {

Font::Font(const std::string& path, float size) 
    : path(path), sizeVal(size), validState(false) {
    info = std::make_unique<stbtt_fontinfo>();
    load();
}

Font::~Font() {
}

void Font::load() {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        std::cerr << "Failed to open font file: " << path << std::endl;
        return;
    }

    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    data.resize(size);
    std::fread(data.data(), 1, size, f);
    std::fclose(f);

    if (!stbtt_InitFont(info.get(), data.data(), 0)) {
        std::cerr << "Failed to init font" << std::endl;
        return;
    }

    scale = stbtt_ScaleForPixelHeight(info.get(), sizeVal);

    stbtt_GetFontVMetrics(info.get(), &ascent, &descent, &lineGap);
    baseline = (int)(ascent * scale);

    atlas.width = 512;
    atlas.height = 512;
    atlas.pixels.resize(atlas.width * atlas.height, 0);

    int curX = 1;
    int curY = 1;
    int maxRowH = 0;

    for (int i = 32; i < 127; i++) {
        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(info.get(), i, scale, scale, &x1, &y1, &x2, &y2);
        
        int w = x2 - x1;
        int h = y2 - y1;

        if (curX + w + 1 >= atlas.width) {
            curX = 1;
            curY += maxRowH + 1;
            maxRowH = 0;
        }

        if (curY + h >= atlas.height) break; 

        stbtt_MakeCodepointBitmap(info.get(), atlas.pixels.data() + (curY * atlas.width + curX), 
                                  w, h, atlas.width, scale, scale, i);

        glyphs[i].x0 = curX;
        glyphs[i].y0 = curY;
        glyphs[i].x1 = x1;
        glyphs[i].y1 = y1;
        
        int adv, lsb;
        stbtt_GetCodepointHMetrics(info.get(), i, &adv, &lsb);
        glyphs[i].advance = (int)(adv * scale);
        glyphs[i].lsb = (int)(lsb * scale);

        curX += w + 1;
        if (h > maxRowH) maxRowH = h;
    }

    validState = true;
}

int Font::height() const {
    return (int)((ascent - descent + lineGap) * scale);
}

int Font::width(const std::string& text) {
    int w = 0;
    for (char c : text) {
        if (c >= 32 && c < 127) {
            w += glyphs[(int)c].advance;
        }
    }
    return w;
}

void Font::draw_text(Painter& painter, int x, int y, const std::string& text, Color color) {
    if (!validState) return;

    int curX = x;

    for (char c : text) {
        if (c < 32 || c >= 127) continue;

        const Glyph& g = glyphs[(int)c];
        
        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(info.get(), c, scale, scale, &x1, &y1, &x2, &y2);
        int gw = x2 - x1;
        int gh = y2 - y1;

        int drawX = curX + x1;
        int drawY = y + baseline + y1;

        for (int row = 0; row < gh; row++) {
            for (int col = 0; col < gw; col++) {
                uint8_t alpha = atlas.pixels[(g.y0 + row) * atlas.width + (g.x0 + col)];
                if (alpha > 0) {
                     uint8_t finalAlpha = (uint8_t)((color.a * alpha) / 255);
                     Color finalColor = color;
                     finalColor.a = finalAlpha;
                     painter.draw_pixel(drawX + col, drawY + row, finalColor);
                }
            }
        }
        curX += g.advance;
    }
}

} // namespace Izo