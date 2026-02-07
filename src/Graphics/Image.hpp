#pragma once

#include "Graphics/Canvas.hpp"
#include "Graphics/Painter.hpp"
#include "Geometry/Primitives.hpp"
#include "Core/ResourceManager.hpp"
#include "UI/Enums.hpp"

#include <string>

namespace Izo {

class Image {
public:
    Image(const std::string& path);
    ~Image();

    bool valid() const { return data != nullptr; }
    int width() const { return w; }
    int height() const { return h; }

    void draw(Painter& painter, IntPoint pos);
    void draw_scaled(Painter& painter, const IntRect& rect, Anchor anchor = Anchor::TopLeft);

private:
    int w = 0, h = 0, channels = 0;
    unsigned char* data = nullptr;
};

using ImageManager = ResourceManager<Image>;

} 
