#define STB_TRUETYPE_IMPLEMENTATION
#include "Font.hpp"
#include "stb_truetype.h"
#include <cstdio>
#include <iostream>
#include <cmath>

namespace Izo {

Font::Font(const std::string& path, float size) 
    : path_(path), size_(size), valid_(false) {
    info_ = std::make_unique<stbtt_fontinfo>();
    load();
}

Font::~Font() {
}

void Font::load() {
    FILE* f = std::fopen(path_.c_str(), "rb");
    if (!f) {
        std::cerr << "Failed to open font file: " << path_ << std::endl;
        return;
    }

    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    data_.resize(size);
    std::fread(data_.data(), 1, size, f);
    std::fclose(f);

    if (!stbtt_InitFont(info_.get(), data_.data(), 0)) {
        std::cerr << "Failed to init font" << std::endl;
        return;
    }

    scale_ = stbtt_ScaleForPixelHeight(info_.get(), size_);

    stbtt_GetFontVMetrics(info_.get(), &ascent_, &descent_, &line_gap_);
    baseline_ = (int)(ascent_ * scale_);

    // Generate Atlas
    atlas_.width = 512;
    atlas_.height = 512;
    atlas_.pixels.resize(atlas_.width * atlas_.height, 0);

    int cur_x = 1;
    int cur_y = 1;
    int max_row_h = 0;

    for (int i = 32; i < 127; i++) {
        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(info_.get(), i, scale_, scale_, &x1, &y1, &x2, &y2);
        
        int w = x2 - x1;
        int h = y2 - y1;

        if (cur_x + w + 1 >= atlas_.width) {
            cur_x = 1;
            cur_y += max_row_h + 1;
            max_row_h = 0;
        }

        if (cur_y + h >= atlas_.height) {
             // Atlas full (should handle this better in production)
             break; 
        }

        stbtt_MakeCodepointBitmap(info_.get(), atlas_.pixels.data() + (cur_y * atlas_.width + cur_x), 
                                  w, h, atlas_.width, scale_, scale_, i);

        glyphs_[i].x0 = cur_x;
        glyphs_[i].y0 = cur_y;
        glyphs_[i].x1 = x1;
        glyphs_[i].y1 = y1;
        
        int adv, lsb;
        stbtt_GetCodepointHMetrics(info_.get(), i, &adv, &lsb);
        glyphs_[i].advance = (int)(adv * scale_);
        glyphs_[i].lsb = (int)(lsb * scale_);

        cur_x += w + 1;
        if (h > max_row_h) max_row_h = h;
    }

    valid_ = true;
}

int Font::height() const {
    return (int)((ascent_ - descent_ + line_gap_) * scale_);
}

int Font::width(const std::string& text) {
    int w = 0;
    for (char c : text) {
        if (c >= 32 && c < 127) {
            w += glyphs_[(int)c].advance;
        }
    }
    return w;
}

void Font::draw_text(Painter& painter, int x, int y, const std::string& text, Color color) {
    if (!valid_) return;

    int cur_x = x;
    Canvas& canvas = painter.canvas();

    for (char c : text) {
        if (c < 32 || c >= 127) continue;

        const Glyph& g = glyphs_[(int)c];
        
        int x1, y1, x2, y2;
        // We re-query box dimensions from stb or store w/h in Glyph? 
        // We didn't store w/h in Glyph, but we stored x1/y1 relative to baseline?
        // Wait, stbtt_GetCodepointBitmapBox returns offset from current point.
        // And we stored those x1, y1 in glyphs_[i].x1/y1.
        
        int w = g.x1 * -1 + g.x1 + (g.x1 < 0 ? -g.x1 + g.x1 : 0); // Logic error in my head.
        // Actually: w = x2 - x1 (from load time). 
        // I didn't store w/h in Glyph struct. I should have. 
        // But I can derive it if I stored the box.
        // In load(): w = x2 - x1. 
        // But I only stored x1, y1. I lost x2, y2.
        // Let's re-query? No, that's slow.
        // Let's assume I can read from atlas?
        // Wait, I need to know how big the rect to copy is.
        // Let's modify Glyph struct or re-calculate.
        // Better: store w/h in Glyph.
        
        // However, I can't change header now without another tool call.
        // I will use stbtt_GetCodepointBitmapBox again or just rely on what I have?
        // I have x1, y1.
        // Actually, let's look at `ankrypton` implementation.
        // It calls `stbtt_GetCodepointBitmapBox` in `draw_text_ttf_alpha` loop!
        // "stbtt_GetCodepointBitmapBox(&font->info, c, font->scale, font->scale, &x1, &y1, &x2, &y2);"
        // So it's acceptable.
        
        stbtt_GetCodepointBitmapBox(info_.get(), c, scale_, scale_, &x1, &y1, &x2, &y2);
        int gw = x2 - x1;
        int gh = y2 - y1;

        int draw_x = cur_x + g.lsb;
        int draw_y = y + baseline_ + g.y1;

        for (int row = 0; row < gh; row++) {
            for (int col = 0; col < gw; col++) {
                uint8_t alpha = atlas_.pixels[(g.y0 + row) * atlas_.width + (g.x0 + col)];
                if (alpha > 0) {
                    // Manual blending or just set alpha if we had a blending set_pixel
                    // Painter doesn't have blending yet.
                    // But `Canvas` had `set_pixel_unsafe`.
                    // We need blending.
                    // Let's add a simple blend in place here or add it to Painter.
                    // Since I can't edit Painter.hpp easily, I'll do it here.
                    
                    uint32_t current_color = canvas.pixel(draw_x + col, draw_y + row);
                    
                    // Simple alpha blending
                    // src = color, dst = current_color
                    // out = src * alpha + dst * (1 - alpha)
                    
                    uint8_t inv_a = 255 - alpha;
                    uint8_t r = (color.r * alpha + ((current_color >> 16) & 0xFF) * inv_a) / 255;
                    uint8_t g = (color.g * alpha + ((current_color >> 8) & 0xFF) * inv_a) / 255;
                    uint8_t b = (color.b * alpha + (current_color & 0xFF) * inv_a) / 255;
                    
                    painter.draw_pixel(draw_x + col, draw_y + row, Color(r, g, b, 255));
                }
            }
        }
        cur_x += g.advance;
    }
}

} // namespace Izo
