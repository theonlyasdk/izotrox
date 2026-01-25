#pragma once

namespace Izo {

template <typename T>
struct Point {
    T x;
    T y;

    constexpr Point() : x(0), y(0) {}
    constexpr Point(T x, T y) : x(x), y(y) {}

    constexpr Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
    constexpr Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
    constexpr Point operator*(T scalar) const { return {x * scalar, y * scalar}; }
    constexpr Point operator/(T scalar) const { return {x / scalar, y / scalar}; }

    Point& operator+=(const Point& other) { x += other.x; y += other.y; return *this; }
    Point& operator-=(const Point& other) { x -= other.x; y -= other.y; return *this; }
    Point& operator*=(T scalar) { x *= scalar; y *= scalar; return *this; }
    Point& operator/=(T scalar) { x /= scalar; y /= scalar; return *this; }

    constexpr bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    constexpr bool operator!=(const Point& other) const { return !(*this == other); }
};

using IntPoint = Point<int>;
using FloatPoint = Point<float>;
using DoublePoint = Point<double>;

} 
