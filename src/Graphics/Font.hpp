#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Painter.hpp"

// Forward declaration for stb_truetype structs
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

    bool valid() const { return valid_; }
    float size() const { return size_; }
    int height() const;

    void draw_text(Painter& painter, int x, int y, const std::string& text, Color color);
    int width(const std::string& text);

private:
    std::string path_;
    float size_;
    bool valid_;
    std::vector<unsigned char> data_;
    
    // Pimpl idiom or void* to avoid including stb header in our header
    // But since we are allowed to use stb in implementation, we can just use void* or forward decl.
    // stbtt_fontinfo is a struct.
    std::unique_ptr<stbtt_fontinfo> info_;
    
    float scale_;
    int ascent_, descent_, line_gap_, baseline_;
    
    // Simple atlas for ASCII
    struct Atlas {
        int width, height;
        std::vector<unsigned char> pixels;
    } atlas_;

    Glyph glyphs_[128]; // Cache for ASCII 32-127

    void load();
};

} // namespace Izo
