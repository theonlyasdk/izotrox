#include "Button.hpp"

namespace Izo {

Button::Button(const std::string& text, Font* font) 
    : text_(text), font_(font), bg_color_(Color::Blue), text_color_(Color::White) {}

void Button::draw(Painter& painter) {
    Color c = is_pressed_ ? Color(50, 50, 200) : bg_color_;
    painter.fill_rect(x, y, w, h, c);
    painter.draw_rect(x, y, w, h, Color::White);
    
    if (font_) {
        int tw = font_->width(text_);
        int th = font_->height();
        int tx = x + (w - tw) / 2;
        int ty = y + (h - th) / 2;
        font_->draw_text(painter, tx, ty, text_, text_color_);
    }
}

bool Button::on_touch(int tx, int ty, bool down) {
    bool inside = (tx >= x && tx < x + w && ty >= y && ty < y + h);
    bool old_pressed = is_pressed_;
    
    if (inside) {
        is_pressed_ = down;
    } else {
        is_pressed_ = false;
    }
    
    if (old_pressed != is_pressed_) {
        Widget::invalidate();
    }
    
    if (inside && down) return true;
    return false;
}

void Button::measure(int& mw, int& mh) {
     if (font_) {
        mw = font_->width(text_) + 20;
        mh = font_->height() + 10;
    } else {
        mw = 50; mh = 20;
    }
}

} // namespace Izo
