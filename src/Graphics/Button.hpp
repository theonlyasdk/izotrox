#pragma once
#include "Widget.hpp"
#include "Font.hpp"
#include "Color.hpp"
#include <string>

namespace Izo {

class Button : public Widget {
public:
    Button(const std::string& text, Font* font);

    void draw(Painter& painter) override;
    bool on_touch(int tx, int ty, bool down) override;
    void measure(int& mw, int& mh) override;

private:
    std::string text_;
    Font* font_;
    Color bg_color_;
    Color text_color_;
    bool is_pressed_ = false;
};

} // namespace Izo
