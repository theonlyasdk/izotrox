#include "TextBox.hpp"

namespace Izo {

TextBox::TextBox(const std::string& placeholder, Font* font) 
    : text_(""), placeholder_(placeholder), font_(font), 
      bg_color_(Color(30, 30, 30)), text_color_(Color::White), border_color_(Color(100, 100, 100)) {}

void TextBox::set_text(const std::string& t) { 
    if (text_ != t) {
        text_ = t;
        Widget::invalidate();
    }
}

const std::string& TextBox::get_text() const { return text_; }

void TextBox::draw(Painter& painter) {
    Color border = is_focused ? Color::Red : border_color_;
    
    painter.fill_rect(x, y, w, h, bg_color_);
    painter.draw_rect(x, y, w, h, border);

    if (font_) {
        int padding = 5;
        // Draw placeholder if empty
        if (text_.empty()) {
            font_->draw_text(painter, x + padding, y + padding, placeholder_, Color(150, 150, 150));
        } else {
            font_->draw_text(painter, x + padding, y + padding, text_, text_color_);
        }
        
        if (is_focused) {
            int tw = text_.empty() ? 0 : font_->width(text_);
            painter.fill_rect(x + padding + tw + 1, y + padding, 2, font_->height(), Color::White);
        }
    }
}

bool TextBox::on_touch(int tx, int ty, bool down) {
    bool inside = (tx >= x && tx < x + w && ty >= y && ty < y + h);
    if (down) {
        if (is_focused != inside) {
            is_focused = inside;
            Widget::invalidate();
        }
        return inside;
    }
    return false;
}

bool TextBox::on_key(int key) {
    if (!is_focused) return false;
    
    bool changed = false;
    if (key == 8) { // Backspace
        if (!text_.empty()) {
            text_.pop_back();
            changed = true;
        }
    } else if (key >= 32 && key < 127) {
        text_ += (char)key;
        changed = true;
    }
    
    if (changed) Widget::invalidate();
    return true;
}

void TextBox::measure(int& mw, int& mh) {
    mw = 200;
    mh = font_ ? font_->height() + 10 : 30;
}

} // namespace Izo
