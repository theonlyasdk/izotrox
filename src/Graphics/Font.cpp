#include "Debug/Logger.hpp"
#include "Graphics/Font.hpp"
#include <cstdio>
#include <sstream>

#define STB_TRUETYPE_IMPLEMENTATION
#include "Lib/stb_truetype.h"

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
        LogError("Failed to open font file: {}", path);
        return;
    }

    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    data.resize(size);
    std::fread(data.data(), 1, size, f);
    std::fclose(f);

    if (!stbtt_InitFont(info.get(), data.data(), 0)) {
        LogError("Failed to init font");
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

void Font::draw_text(Painter& painter, IntPoint pos, const std::string& text, Color color) {
    if (!validState) return;

    int curX = pos.x;

    for (char c : text) {
        if (c < 32 || c >= 127) continue;

        const Glyph& g = glyphs[(int)c];

        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(info.get(), c, scale, scale, &x1, &y1, &x2, &y2);
        int gw = x2 - x1;
        int gh = y2 - y1;

        int drawX = curX + x1;
        int drawY = pos.y + baseline + y1;

        for (int row = 0; row < gh; row++) {
            for (int col = 0; col < gw; col++) {
                uint8_t alpha = atlas.pixels[(g.y0 + row) * atlas.width + (g.x0 + col)];
                if (alpha > 0) {
                     uint8_t finalAlpha = (uint8_t)((color.a * alpha) / 255);
                     Color finalColor = color;
                     finalColor.a = finalAlpha;
                     painter.draw_pixel({drawX + col, drawY + row}, finalColor);
                }
            }
        }
        curX += g.advance;
    }
}

void Font::measure_multiline(const std::string& text, int& out_w, int& out_h, int max_width) {
    out_w = 0;
    out_h = 0;
    if (!validState || text.empty()) {
        if (text.empty()) out_h = height();
        return;
    }

    int lineHeight = height();
    int maxW = 0;
    int totalH = 0;

    auto process_line = [&](const std::string& l) {
        if (max_width <= 0) {
            int w = width(l);
            if (w > maxW) maxW = w;
            totalH += lineHeight;
        } else {
            std::string currentLine;
            int currentW = 0;
            std::string word;
            
            auto flush_word = [&]() {
                if (word.empty()) return;
                int wordW = width(word);
                if (!currentLine.empty() && currentW + wordW > max_width) {
                    if (currentW > maxW) maxW = currentW;
                    totalH += lineHeight;
                    currentLine = word;
                    currentW = wordW;
                } else {
                    currentLine += word;
                    currentW += wordW;
                }
                word = "";
            };

            for (char c : l) {
                word += c;
                if (c == ' ') {
                    flush_word();
                }
            }
            flush_word();

            if (!currentLine.empty() || l.empty()) {
                if (currentW > maxW) maxW = currentW;
                totalH += lineHeight;
            }
        }
    };

    size_t start = 0;
    size_t end = text.find('\n');
    while (end != std::string::npos) {
        process_line(text.substr(start, end - start));
        start = end + 1;
        end = text.find('\n', start);
    }
    process_line(text.substr(start));

    out_w = maxW;
    out_h = totalH;
}

void Font::draw_text_multiline(Painter& painter, IntPoint pos, const std::string& text, Color color, int wrap_width, int align_width, TextAlign align) {
    if (!validState) return;

    int lineHeight = height();
    int curY = pos.y;

    auto draw_line_str = [&](const std::string& l) {
        int tx = pos.x;
        if (align_width > 0) {
            int tw = width(l);
            if (align == TextAlign::Center) {
                tx += (align_width - tw) / 2;
            } else if (align == TextAlign::Right) {
                tx += align_width - tw;
            }
        }
        draw_text(painter, {tx, curY}, l, color);
        curY += lineHeight;
    };

    auto process_line = [&](const std::string& l) {
        if (wrap_width <= 0) {
            draw_line_str(l);
        } else {
            std::string currentLine;
            int currentW = 0;
            std::string word;

            auto flush_word = [&]() {
                if (word.empty()) return;
                int wordW = width(word);
                if (!currentLine.empty() && currentW + wordW > wrap_width) {
                    draw_line_str(currentLine);
                    currentLine = word;
                    currentW = wordW;
                } else {
                    currentLine += word;
                    currentW += wordW;
                }
                word = "";
            };

            for (char c : l) {
                word += c;
                if (c == ' ') {
                    flush_word();
                }
            }
            flush_word();

            if (!currentLine.empty() || l.empty()) {
                draw_line_str(currentLine);
            }
        }
    };

    size_t start = 0;
    size_t end = text.find('\n');
    while (end != std::string::npos) {
        process_line(text.substr(start, end - start));
        start = end + 1;
        end = text.find('\n', start);
    }
    process_line(text.substr(start));
}

} 
