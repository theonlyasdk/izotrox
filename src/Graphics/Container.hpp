// Mozilla Public License version 2.0. (c) theonlyasdk 2026

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
    
    // Override on_touch to dispatch to children
    bool on_touch(int tx, int ty, bool down, bool captured = false) override;
    bool on_key(KeyCode key) override;
    
    void invalidate() override;
    void layout() override; 

    void collect_focusable_widgets(std::vector<std::shared_ptr<Widget>>& out_list);

protected:
    std::vector<std::shared_ptr<Widget>> m_children;
    std::shared_ptr<Widget> m_captured_child;
};

} // namespace Izo