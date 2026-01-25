#pragma once
#include "Canvas.hpp"
#include "Painter.hpp"
#include "Geometry/Point.hpp"
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
        CenterStartHoriz, 
        CenterStartVert,  
        CenterEndHoriz,   
        CenterEndVert     
    };

    void draw(Painter& painter, IntPoint pos);
    void draw_scaled(Painter& painter, const IntRect& rect, Anchor anchor = Anchor::TopLeft);

private:
    int w = 0, h = 0, channels = 0;
    unsigned char* data = nullptr;
};

} 
