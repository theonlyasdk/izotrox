

#pragma once
#include "Widget.hpp"
#include <vector>
#include <memory>

namespace Izo {

class Container : public Widget {
public:
    void add_child(std::shared_ptr<Widget> child);
    void draw_content(Painter& painter) override;
    void update() override;

    bool on_touch(IntPoint point, bool down, bool captured = false) override;
    bool on_scroll(int y) override;
    bool on_key(KeyCode key) override;
    void draw_focus(Painter& painter) override;

    void layout() override; 

    void collect_focusable_widgets(std::vector<std::shared_ptr<Widget>>& out_list);

    const std::vector<std::shared_ptr<Widget>>& children() const { return m_children; }

protected:
    std::vector<std::shared_ptr<Widget>> m_children;
    std::shared_ptr<Widget> m_captured_child;
};

} 