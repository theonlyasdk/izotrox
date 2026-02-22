#pragma once

#include "UI/Widgets/Widget.hpp"

#include <vector>
#include <memory>

namespace Izo {

class Container : public Widget {
public:
    void add_child(std::unique_ptr<Widget> child);
    virtual void draw_content(Painter& painter) override;
    virtual void draw_focus(Painter& painter) override;
    virtual void update() override;

    virtual bool on_touch(IntPoint point, bool down, bool captured = false) override;
    virtual bool on_scroll(int y) override;
    virtual bool on_key(KeyCode key) override;
    virtual void on_theme_update() override;

    const std::vector<std::unique_ptr<Widget>>& children() const { return m_children; }
    void collect_focusable_widgets(std::vector<Widget*>& out_list);

    virtual void layout() override;
    bool subtree_layout_dirty() const override;
    void clear_layout_dirty_subtree() override;
    bool has_running_animations() const override;

protected:
    std::vector<std::unique_ptr<Widget>> m_children;
    Widget* m_captured_child = nullptr;
};

} 
