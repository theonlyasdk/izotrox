#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Painter.hpp"

typedef struct stbtt_fontinfo stbtt_fontinfo;

namespace Izo {

struct Glyph {
    int x0, y0, x1, y1;
    int advance, lsb;
};

class Font {
public:
    Font(const std::string& path, float size);
    ~Font();

    bool valid() const { return validState; }
    float size() const { return sizeVal; }
    int height() const;

    void draw_text(Painter& painter, int x, int y, const std::string& text, Color color);
    int width(const std::string& text);

private:
    std::string path;
    float sizeVal;
    bool validState;
    std::vector<unsigned char> data;
    
    std::unique_ptr<stbtt_fontinfo> info;
    
    float scale;
    int ascent, descent, lineGap, baseline;
    
    struct Atlas {
        int width, height;
        std::vector<unsigned char> pixels;
    } atlas;

    Glyph glyphs[128]; 

    void load();
};

} // namespace Izo