#pragma once

#include "UI/Widgets/Widget.hpp"
#include "Graphics/Font.hpp"
#include "Motion/Animator.hpp"
#include "Geometry/Primitives.hpp"

#include <vector>
#include <string>
#include <functional>

namespace Izo {

class OptionBox : public Widget {
public:
    OptionBox(Font* font);

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(IntPoint point, bool down) override;
    void measure(int parent_w, int parent_h) override;

    void add_option(const std::string& option);
    void set_options(const std::vector<std::string>& options);
    
    void set_selected_index(int index);
    int selected_index() const { return m_selected_index; }
    std::string selected_value() const;
    
    void set_on_change(std::function<void(int, const std::string&)> callback) { m_on_change = callback; }

private:
    int item_height() const;

    Font* m_font;
    std::vector<std::string> m_options;
    int m_selected_index = 0;
    
    bool m_pressed = false;
    
    Animator<Color> m_bg_anim;
    
    std::function<void(int, const std::string&)> m_on_change;
};

} 
