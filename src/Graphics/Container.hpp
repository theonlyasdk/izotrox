#pragma once
#include "Widget.hpp"
#include <vector>
#include <memory>

namespace Izo {

class Container : public Widget {
public:
    void add_child(std::shared_ptr<Widget> child);
    void draw(Painter& painter) override;
    void update() override;
    void layout();

protected:
    std::vector<std::shared_ptr<Widget>> children;
};

} // namespace Izo
