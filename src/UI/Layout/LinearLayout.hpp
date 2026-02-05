#pragma once

#include "UI/Layout/Layout.hpp"
#include "UI/Enums.hpp"

namespace Izo {

class LinearLayout : public Layout {
public:
    LinearLayout(Orientation orientation = Orientation::Vertical);

    void layout_children() override;
    void measure(int parent_w, int parent_h) override;

protected:
    int content_height() const override { return m_content_height; }

private:
    Orientation m_orientation;
};

}