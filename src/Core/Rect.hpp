// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once
#include <algorithm>

namespace Izo {

template <typename T>
struct Rect {
    T x, y, w, h;

    T right() const { return x + w; }
    T bottom() const { return y + h; }

    bool contains(T px, T py) const {
        return px >= x && px < right() && py >= y && py < bottom();
    }

    bool intersects(const Rect& other) const {
        return x < other.right() && right() > other.x &&
               y < other.bottom() && bottom() > other.y;
    }

    Rect intersection(const Rect& other) const {
        T x1 = std::max(x, other.x);
        T y1 = std::max(y, other.y);
        T x2 = std::min(right(), other.right());
        T y2 = std::min(bottom(), other.bottom());

        if (x2 < x1 || y2 < y1) return {0, 0, 0, 0};
        return {x1, y1, x2 - x1, y2 - y1};
    }
};

using IntRect = Rect<int>;
using FloatRect = Rect<float>;

} // namespace Izo
