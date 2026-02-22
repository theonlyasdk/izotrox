#pragma once

#include "Geometry/Primitives.hpp"
#include "Input/KeyCode.hpp"

#include <memory>

namespace Izo {

class Widget;
class Painter;

class View {
public:
    View(std::unique_ptr<Widget> root);
    virtual ~View();

    void resize(int w, int h);
    void update();
    void draw(Painter& painter);
    bool has_running_animations() const;

    void on_touch(IntPoint point, bool down);
    void on_scroll(int y);
    void on_key(KeyCode key);

private:
    std::unique_ptr<Widget> m_root;
    int m_width = 0, m_height = 0;
};

} 
