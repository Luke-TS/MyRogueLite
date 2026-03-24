#pragma once

#include <cmath>
#include <raylib.h>

inline Vector2 operator+(const Vector2& a, const Vector2& b) {
    return { a.x + b.x, a.y + b.y };
}

inline Vector2 operator-(const Vector2& a, const Vector2& b) {
    return { a.x - b.x, a.y - b.y };
}

inline Vector2 operator*(const Vector2& v, float s) {
    return { v.x * s, v.y * s };
}

inline Vector2 operator*(float s, const Vector2& v) {
    return v * s;
}

inline Vector2& operator*=(Vector2& v, float s) {
    v.x *= s; 
    v.y *= s; 
    return v;
}

inline Vector2 operator/(const Vector2& v, float s) {
    return v * (1.f/s);
}

inline Vector2& operator/=(Vector2& v, float s) {
    v.x /= s; 
    v.y /= s; 
    return v;
}

inline Vector2& operator+=(Vector2& a, const Vector2& b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Vector2& operator-=(Vector2& a, const Vector2& b) {
    a += {-b.x, -b.y};
    return a;
}

inline float length(const Vector2& v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

inline Vector2 normalize(const Vector2& v) {
    float len = length(v);
    return len > 0 ? Vector2{v.x / len, v.y / len} : Vector2{0,0};
}
