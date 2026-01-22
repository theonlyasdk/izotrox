// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Layout.hpp"
#include <vector>
#include <string>
#include <functional>

namespace Izo {

class ListView : public Layout {
public:
    ListView();

    void add_item(std::shared_ptr<Widget> item);
    
    void set_item_height(int h) { m_item_height = h; } // Optional if items are fixed
    
    void draw_content(Painter& painter) override;
    void layout_children() override;
    void measure(int parent_w, int parent_h) override;
    bool on_key(KeyCode key) override;
    
    void select(int index);
    int selected_index() const { return m_selected_index; }
    
    void on_item_selected(std::function<void(int)> cb) { m_on_item_selected = cb; }

protected:
    int content_height() const override;

private:
    int m_item_height = 50; 
    int m_total_content_height = 0;
    int m_selected_index = -1;
    std::function<void(int)> m_on_item_selected;
};

} // namespace Izo