#pragma once

#include "UI/Layout/Layout.hpp"
#include <functional>

namespace Izo {

class Widget;

class ListBox : public Layout {
public:
    ListBox();

    void add_item(std::shared_ptr<Widget> item);

    void set_item_height(int h) { m_item_height = h; } 

    void draw_content(Painter& painter) override;
    void layout_children() override;
    void measure(int parent_w, int parent_h) override;
    bool on_key(KeyCode key) override;
    void smooth_scroll_to_index(int index) override;
    bool on_scroll(int y) override;

    void select(int index);
    int selected_index() const { return m_selected_index; }

    void on_item_selected(std::function<void(int)> cb) { m_on_item_selected = cb; }

protected:
    int content_height() const override { return m_total_content_height; }

private:
    int m_item_height = 50; 
    int m_total_content_height = 0;
    int m_selected_index = -1;
    std::function<void(int)> m_on_item_selected;
};

} 