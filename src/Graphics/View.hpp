#pragma once

#include "Widget.hpp"
#include "Geometry/Primitives.hpp"
#include "Input/KeyCode.hpp"

#include <memory>

namespace Izo {

class View {
public:
    View(std::shared_ptr<Widget> root);

    void resize(int w, int h);
    void update();
    void draw(Painter& painter);

    void on_touch(IntPoint point, bool down);
    void on_scroll(int y);
    void on_key(KeyCode key);

private:
    std::shared_ptr<Widget> m_root;
    int m_width = 0, m_height = 0;
};

} 
