

#include <algorithm>

#include "Rect.hpp"

namespace Izo {

template <typename T> T Rect<T>::left() const { return x; }

template <typename T> T Rect<T>::top() const { return y; }

template <typename T> T Rect<T>::right() const { return x + w; }

template <typename T> T Rect<T>::bottom() const { return y + h; }

template <typename T> bool Rect<T>::contains(T px, T py) const {
  return px >= x && px < right() && py >= y && py < bottom();
}

template <typename T> bool Rect<T>::contains(const Point<T>& point) const {
    return contains(point.x, point.y);
}

template <typename T> bool Rect<T>::intersects(const Rect &other) const {
  return x < other.right() && right() > other.x && y < other.bottom() &&
         bottom() > other.y;
}

template <typename T> Rect<T> Rect<T>::intersection(const Rect &other) const {
  T x1 = std::max(x, other.x);
  T y1 = std::max(y, other.y);
  T x2 = std::min(right(), other.right());
  T y2 = std::min(bottom(), other.bottom());

  if (x2 < x1 || y2 < y1)
    return {0, 0, 0, 0};
  return {x1, y1, x2 - x1, y2 - y1};
}

template <typename T> Rect<T> Rect<T>::expanded(T amount) const {
  return {x - amount, y - amount, w + amount * 2, h + amount * 2};
}

template <typename T> Rect<T> Rect<T>::contracted(T amount) const {
  return {x + amount, y + amount, w - amount * 2, h - amount * 2};
}

template <typename T> void Rect<T>::expand(T amount) {
  x -= amount;
  y -= amount;
  w += amount * 2;
  h += amount * 2;
}

template <typename T> void Rect<T>::contract(T amount) {
  x += amount;
  y += amount;
  w -= amount * 2;
  h -= amount * 2;
}

} 

template class Izo::Rect<int>;
template class Izo::Rect<float>;