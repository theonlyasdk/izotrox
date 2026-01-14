#include "Label.hpp"

namespace Izo {

Label::Label(const std::string& text, Font* font, Color color) 
    : text_(text), font_(font), color_(color) {}

void Label::draw(Painter& painter) {
    if (font_) {
        font_->draw_text(painter, x, y, text_, color_);
    }
}

void Label::measure(int& mw, int& mh) {
    if (font_) {
        mw = font_->width(text_);
        mh = font_->height();
    } else {
        mw = 0; mh = 0;
    }
}

} // namespace Izo
