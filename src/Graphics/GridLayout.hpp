#pragma once

#include "Widgets/Layout.hpp"

namespace Izo {

class GridLayout : public Layout {
public:
    GridLayout(int cols) : m_cols(cols) {}
    void layout_children() override;
private:
    int m_cols;
};

} 
