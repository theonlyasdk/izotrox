#include "Container.hpp"
#include "../Input/Input.hpp"

namespace Izo {

void Container::add_child(std::shared_ptr<Widget> child) {
    children.push_back(child);
    Widget::invalidate();
}

void Container::draw(Painter& painter) {
    for (auto& child : children) {
        child->draw(painter);
    }
}

void Container::update() {
    int tx = Input::instance().touch_x();
    int ty = Input::instance().touch_y();
    bool down = Input::instance().touch_down();
    
    // bool consumed = false; // Unused variable
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->on_touch(tx, ty, down)) {
            // consumed = true;
            break;
        }
    }
    
    int k = Input::instance().key();
    if (k > 0) {
        for (auto& child : children) {
            if (child->is_focused) {
                if (child->on_key(k)) {
                     // Key handled
                }
                break;
            }
        }
    }

    for (auto& child : children) {
        child->update();
    }
}

void Container::layout() {
    int cur_x = x;
    int cur_y = y;
    for (auto& child : children) {
        int mw, mh;
        child->measure(mw, mh);
        child->set_pos(cur_x, cur_y);
        child->set_size(mw, mh);
        cur_y += mh + 10;
    }
    h = cur_y - y;
    Widget::invalidate();
}

} // namespace Izo
