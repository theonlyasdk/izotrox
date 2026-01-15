#pragma once
#include "Canvas.hpp"
#include "Painter.hpp"
#include <string>

namespace Izo {

class Image {
public:
    Image(const std::string& path);
    ~Image();

    bool valid() const { return data != nullptr; }
    int width() const { return w; }
    int height() const { return h; }
    
    enum class Anchor {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center,
        CenterStartHoriz, // Top Center
        CenterStartVert,  // Left Center
        CenterEndHoriz,   // Bottom Center
        CenterEndVert     // Right Center
    };
    
    // Draw this image onto a canvas at (x,y)
    void draw(Painter& painter, int x, int y);
    void draw_scaled(Painter& painter, int x, int y, int w, int h, Anchor anchor = Anchor::TopLeft);

private:
    int w = 0, h = 0, channels = 0;
    unsigned char* data = nullptr;
};

} // namespace Izo
