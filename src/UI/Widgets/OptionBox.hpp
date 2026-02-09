#pragma once

#include "Graphics/Color.hpp"
#include "UI/Widgets/Widget.hpp"

#include <vector>
#include <string>
#include <functional>

namespace Izo {

class Font;

class OptionBox : public Widget {
public:
    OptionBox();

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(IntPoint point, bool down) override;
    void measure(int parent_w, int parent_h) override;

    void add_option(const std::string& option);
    void set_options(const std::vector<std::string>& options);
    
    void select(int index);
    int selected_index() const { return m_selected_index; }
    std::string selected_value() const {
        if (m_selected_index >= 0 && m_selected_index < (int)m_options.size()) {
            return m_options[m_selected_index];
        }
        return "";
    }
    
    void set_on_change(std::function<void(int, const std::string&)> callback) { m_on_change = callback; }

private:

    std::vector<std::string> m_options;
    int m_selected_index = 0;
    
    bool m_pressed = false;
    
    Animator<Color> m_bg_anim;
    
    std::function<void(int, const std::string&)> m_on_change;
};

} 
