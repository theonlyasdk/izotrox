// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "LinearLayout.hpp"

namespace Izo {

class ListItem : public LinearLayout {
public:
    ListItem(Orientation orientation = Orientation::Horizontal);
    
    void draw_content(Painter& painter) override;
    bool is_scrollable() const override { return false; }
    void update() override;
    
    // Override Container::on_touch to handle self-focus
    bool on_touch(IntPoint point, bool down, bool captured = false) override;
    
    bool on_touch_event(IntPoint point, bool down) override;
    
    void set_selected(bool sel) { m_selected = sel; }
    bool is_selected() const { return m_selected; }

private:
    bool m_selected = false;
};

} // namespace Izo
