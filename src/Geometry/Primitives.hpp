#pragma once

#include "Geometry/Primitives.hpp"

namespace Izo {

/* Forward declaration, do not delete */
template <typename T> struct Rect; 

template <typename T>
struct Point {
    T x;
    T y;

    constexpr Point() : x(0), y(0) {}
    constexpr Point(T x, T y) : x(x), y(y) {}

    /* Addition and subtraction of a point with another Point<T> */
    constexpr Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
    constexpr Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
    Point& operator+=(const Point& other) { x += other.x; y += other.y; return *this; }
    Point& operator-=(const Point& other) { x -= other.x; y -= other.y; return *this; }

    /* Addition and subtraction of position of a Rect<T> to a Point<T> */
    Point& operator+=(const Rect<T>& other) { x += other.x; y += other.y; return *this; }
    Point& operator-=(const Rect<T>& other) { x -= other.x; y -= other.y; return *this; }

    /* Multiplication and division operations with a scalar of type T */
    constexpr Point operator*(T scalar) const { return {x * scalar, y * scalar}; }
    constexpr Point operator/(T scalar) const { return {x / scalar, y / scalar}; }
    Point& operator*=(T scalar) { x *= scalar; y *= scalar; return *this; }
    Point& operator/=(T scalar) { x /= scalar; y /= scalar; return *this; }

    constexpr bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    constexpr bool operator!=(const Point& other) const { return !(*this == other); }
    
    /* If both the coordinates of the point are negative */
    constexpr bool negative() { return x < 0 && y < 0; }
};

using IntPoint = Point<int>;
using FloatPoint = Point<float>;
using DoublePoint = Point<double>;

/* Implementation in Primitives.cpp */
template <typename T>
struct Rect {
    T x, y, w, h;

    T left() const;
    T right() const;
    T top() const;
    T bottom() const;

    bool contains(T px, T py) const;
    bool contains(const Point<T>& point) const;
    bool intersects(const Rect& other) const;

    /* Does the operation on this Rect and returns resultant Rect */
    Rect intersection(const Rect& other) const;
    Rect expanded(T amount) const;
    Rect contracted(T amount) const;

    /* In-place operations */
    void expand(T amount);
    void contract(T amount);
};

using IntRect = Rect<int>;
using FloatRect = Rect<float>;

} 
