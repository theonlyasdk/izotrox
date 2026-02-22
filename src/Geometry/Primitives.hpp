#pragma once

#include <algorithm>

namespace Izo {

template <typename T>
struct Rect;

template <typename T>
struct Point {
    T x{0};
    T y{0};

    constexpr Point() = default;
    constexpr Point(T x_value, T y_value) : x(x_value), y(y_value) {}

    constexpr Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
    constexpr Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
    constexpr Point operator*(T scalar) const { return {x * scalar, y * scalar}; }
    constexpr Point operator/(T scalar) const { return {x / scalar, y / scalar}; }

    constexpr Point& operator+=(const Point& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Point& operator-=(const Point& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Point& operator+=(const Rect<T>& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Point& operator-=(const Rect<T>& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Point& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Point& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    constexpr bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    constexpr bool operator!=(const Point& other) const { return !(*this == other); }

    constexpr bool negative() const { return x < 0 && y < 0; }

    static constexpr Point from_rect(const Rect<T>& rect) { return {rect.x, rect.y}; }
};

using IntPoint = Point<int>;
using FloatPoint = Point<float>;
using DoublePoint = Point<double>;

template <typename T>
struct Rect {
    T x{0};
    T y{0};
    T w{0};
    T h{0};

    constexpr Rect() = default;
    constexpr Rect(T x_value, T y_value, T w_value, T h_value)
        : x(x_value), y(y_value), w(w_value), h(h_value) {}

    constexpr T left() const { return x; }
    constexpr T top() const { return y; }
    constexpr T right() const { return x + w; }
    constexpr T bottom() const { return y + h; }

    constexpr T width() const { return w; }
    constexpr T height() const { return h; }

    constexpr bool contains(T px, T py) const {
        return px >= x && px < right() && py >= y && py < bottom();
    }

    constexpr bool contains(const Point<T>& point) const {
        return contains(point.x, point.y);
    }

    constexpr bool intersects(const Rect& other) const {
        return x < other.right() && right() > other.x && y < other.bottom() && bottom() > other.y;
    }

    constexpr Rect intersection(const Rect& other) const noexcept {
        const T x1 = std::max(x, other.x);
        const T y1 = std::max(y, other.y);
        const T x2 = std::min(right(), other.right());
        const T y2 = std::min(bottom(), other.bottom());
        if (x2 <= x1 || y2 <= y1) {
            return {0, 0, 0, 0};
        }
        return {x1, y1, x2 - x1, y2 - y1};
    }

    constexpr Rect expanded(T amount) const {
        return {x - amount, y - amount, w + amount * 2, h + amount * 2};
    }

    constexpr Rect contracted(T amount) const {
        return {x + amount, y + amount, w - amount * 2, h - amount * 2};
    }

    constexpr void expand(T amount) {
        x -= amount;
        y -= amount;
        w += amount * 2;
        h += amount * 2;
    }

    constexpr void contract(T amount) {
        x += amount;
        y += amount;
        w -= amount * 2;
        h -= amount * 2;
    }

    constexpr void contract_vert(T amount) {
        y += amount;
        h -= amount * 2;
    }

    constexpr void contract_horiz(T amount) {
        x += amount;
        w -= amount * 2;
    }
};

using IntRect = Rect<int>;
using FloatRect = Rect<float>;

struct Padding {
    int left{0};
    int right{0};
    int top{0};
    int bottom{0};

    constexpr Padding() = default;
    constexpr explicit Padding(int all) : left(all), right(all), top(all), bottom(all) {}
    constexpr Padding(int left_value, int right_value, int top_value, int bottom_value)
        : left(left_value), right(right_value), top(top_value), bottom(bottom_value) {}
};

}  // namespace Izo
