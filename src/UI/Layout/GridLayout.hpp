#pragma once

#include "UI/Layout/Layout.hpp"

namespace Izo {

class GridLayout : public Layout {
public:
    GridLayout(int cols) : m_cols(cols) {}
    void layout_children() override;
protected:
    int content_height() const override { return m_total_height; }
private:
    int m_cols;
    int m_total_height = 0;
};

} 
