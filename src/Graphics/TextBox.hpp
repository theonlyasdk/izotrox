#pragma once
#include "Widget.hpp"
#include "Font.hpp"
#include "Color.hpp"
#include <string>

namespace Izo {

class TextBox : public Widget {
public:
    TextBox(const std::string& placeholder, Font* font);

    void set_text(const std::string& t);
    const std::string& get_text() const;

    void draw(Painter& painter) override;
    bool on_touch(int tx, int ty, bool down) override;
    bool on_key(int key) override;
    void measure(int& mw, int& mh) override;

private:
    std::string text_;
    std::string placeholder_;
    Font* font_;
    Color bg_color_;
    Color text_color_;
    Color border_color_;
};

} // namespace Izo
