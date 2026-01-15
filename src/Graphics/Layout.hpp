// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include "Container.hpp"

namespace Izo {

class Layout : public Container {
public:
    void layout() override;
    virtual void layout_children() = 0;
    
    void measure(int parent_w, int parent_h) override;
};

} // namespace Izo
