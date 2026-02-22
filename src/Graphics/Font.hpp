#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Geometry/Primitives.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Color.hpp"
#include "Core/ResourceManager.hpp"
#include "UI/Enums.hpp"

typedef struct stbtt_fontinfo stbtt_fontinfo;

namespace Izo {

struct Glyph {
    int x0, y0, x1, y1;
    int advance, lsb;
};

class Font {
public:
    Font(const std::string& path, float size);
    Font(const Font&) = delete;
    Font(const Font&&) = delete;
    Font& operator=(const Font&) = delete;

    ~Font();

    bool valid() const { return font_loaded; }
    float size() const { return font_size; }
    int height() const { return (int)((ascent - descent + lineGap) * scale); }
    int width(const std::string& text) const;

    void draw_text(Painter& painter, IntPoint pos, const std::string& text, Color color);
    void draw_text_multiline(Painter& painter, IntPoint pos, const std::string& text, Color color, int wrap_width = -1, int align_width = -1, TextAlign align = TextAlign::Left);
    void measure_multiline(const std::string& text, int& out_w, int& out_h, int max_width = -1);

private:
    void load();

    std::string path;
    int ascent, descent, lineGap, baseline;
    float font_size;
    float scale;
    bool font_loaded;
    std::vector<unsigned char> data;
    std::unique_ptr<stbtt_fontinfo> info;

    struct Atlas {
        int width, height;
        std::vector<unsigned char> pixels;
    } atlas;

    Glyph glyphs[128]; 
};

using FontManager = ResourceManager<Font>;

} 