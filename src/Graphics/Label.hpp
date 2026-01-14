#pragma once
#include "Widget.hpp"
#include "Font.hpp"
#include "Color.hpp"
#include <string>

namespace Izo {

class Label : public Widget {
public:
    Label(const std::string& text, Font* font, Color color);

    void draw(Painter& painter) override;
    void measure(int& mw, int& mh) override;

private:
    std::string text_;
    Font* font_;
    Color color_;
};

} // namespace Izo
