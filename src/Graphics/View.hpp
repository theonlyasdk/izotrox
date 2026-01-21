// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Widget.hpp"
#include <memory>
#include "Input/KeyCode.hpp"

namespace Izo {

class View {
public:
    View(std::shared_ptr<Widget> root);
    
    void resize(int w, int h);
    void update();
    void draw(Painter& painter);
    
    // Event injection
    void on_touch(int x, int y, bool down);
    void on_key(KeyCode key);

private:
    std::shared_ptr<Widget> m_root;
    int m_width = 0, m_height = 0;
};

} // namespace Izo
