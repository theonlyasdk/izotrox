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
    enum class AnimationVariant {
        // Dialog expands to screen center
        ExpandCenter,
        // Dialog expands it's height to screen height
        // But keeps it's width the same as it's owning OptionBox width
        ExpandVertical
    };

    OptionBox();

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch_event(IntPoint point, bool down) override;
    void measure(int parent_w, int parent_h) override;

    void add_option(const std::string& option);
    void set_options(const std::vector<std::string>& options);

    std::vector<std::string>* options() { return &m_options; };

    const AnimationVariant anim_variant() const { return m_anim_variant; }
    void set_anim_variant(AnimationVariant variant) { m_anim_variant = variant; }

    void select(int index);
    const int selected_index() const { return m_selected_index; }

    std::string selected_value() const {
        if (m_selected_index >= 0 && m_selected_index < (int)m_options.size()) {
            return m_options[m_selected_index];
        }
        return "";
    }
    
    void set_on_change(std::function<void(int, const std::string&)> callback) { m_on_change = callback; }

private:
    AnimationVariant m_anim_variant = AnimationVariant::ExpandVertical;
    std::vector<std::string> m_options;
    int m_selected_index = 0;
    
    bool m_pressed = false;
    
    Animator<Color> m_bg_anim;
    
    std::function<void(int, const std::string&)> m_on_change;
};

} 
