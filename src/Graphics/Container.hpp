#pragma once

#include "Widget.hpp"

#include <vector>
#include <memory>

namespace Izo {

class Container : public Widget {
public:
    virtual void add_child(std::shared_ptr<Widget> child);
    virtual void draw_content(Painter& painter) override;
    virtual void draw_focus(Painter& painter) override;
    virtual void update() override;

    virtual bool on_touch(IntPoint point, bool down, bool captured = false) override;
    virtual bool on_scroll(int y) override;
    virtual bool on_key(KeyCode key) override;

    const std::vector<std::shared_ptr<Widget>>& children() const { return m_children; }
    void collect_focusable_widgets(std::vector<std::shared_ptr<Widget>>& out_list);

    virtual void layout() override;

protected:
    std::vector<std::shared_ptr<Widget>> m_children;
    std::shared_ptr<Widget> m_captured_child = nullptr;
};

} 