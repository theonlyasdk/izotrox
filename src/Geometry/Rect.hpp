// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once

namespace Izo {

template <typename T>
class Rect {
public:
    T x, y, w, h;

    T left() const;
    T right() const;
    T top() const;
    T bottom() const;

    bool contains(T px, T py) const;
    bool intersects(const Rect& other) const;
    Rect intersection(const Rect& other) const;
};

using IntRect = Rect<int>;
using FloatRect = Rect<float>;

} // namespace Izo
