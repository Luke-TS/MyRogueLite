#pragma once

#include <raylib.h>
#include <optional>
#include <algorithm>

namespace physics {

// ----------------------
// Representation helpers
// ----------------------

inline Rectangle centerToRectangle(Vector2 center, Vector2 dims) {
    return Rectangle{
        center.x - dims.x * 0.5f,
        center.y - dims.y * 0.5f,
        dims.x, dims.y 
    };
}

inline Vector2 getCenter(const Rectangle& r) {
    return {
        r.x + r.width * 0.5f,
        r.y + r.height * 0.5f
    };
}

// ----------------------
// Core collision
// ----------------------

// Returns penetration of vector to resolve A out of B
// nullopt = no intersection
inline std::optional<Vector2> intersect(const Rectangle& a, const Rectangle& b) {
    float overlapX = std::min(a.x + a.width,  b.x + b.width)  - std::max(a.x, b.x);
    float overlapY = std::min(a.y + a.height, b.y + b.height) - std::max(a.y, b.y);

    if (overlapX <= 0.0f || overlapY <= 0.0f)
        return std::nullopt;

    Vector2 ac = getCenter(a);
    Vector2 bc = getCenter(b);

    // resolve along least penetration axis
    if (overlapX < overlapY) {
        float sign = (ac.x < bc.x) ? -1.f : 1.f;
        return Vector2{ overlapX * sign, 0.f };
    } else {
        float sign = (ac.y < bc.y) ? -1.f : 1.f;
        return Vector2{ 0.f, overlapY * sign };
    }
}

// ----------------------
// Containment (clamping)
// ----------------------

// pushes `inner` fully inside `outer`
inline std::optional<Vector2> contain(const Rectangle& inner, const Rectangle& outer) {
    float px = 0.f;
    float py = 0.f;

    if (inner.x < outer.x)
        px = inner.x - outer.x;
    else if (inner.x + inner.width > outer.x + outer.width)
        px = (inner.x + inner.width) - (outer.x + outer.width);

    if (inner.y < outer.y)
        py = inner.y - outer.y;
    else if (inner.y + inner.height > outer.y + outer.height)
        py = (inner.y + inner.height) - (outer.y + outer.height);

    if (px == 0.f && py == 0.f)
        return std::nullopt;

    return Vector2{ px, py };
}

// ----------------------
// Centered wrappers
// ----------------------

inline std::optional<Vector2> intersectCentered(
    Vector2 c1, Vector2 dims1,
    Vector2 c2, Vector2 dims2)
{
    return intersect(
        centerToRectangle(c1, dims1),
        centerToRectangle(c2, dims1)
    );
}

inline std::optional<Vector2> containCentered(
    Vector2 cInner, Vector2 dInner,
    Vector2 cOuter, Vector2 dOuter)
{
    return contain(
        centerToRectangle(cInner, dInner),
        centerToRectangle(cOuter, dOuter)
    );
}

inline std::optional<Vector2> intersectCentered(
    Rectangle a,
    Rectangle b)
{
    return intersect(
        centerToRectangle({a.x, a.y}, {a.width, a.height}),
        centerToRectangle({b.x, b.y}, {b.width, b.height})
    );
}

inline std::optional<Vector2> containCentered(
    Rectangle a,
    Rectangle b)
{
    return contain(
        centerToRectangle({a.x, a.y}, {a.width, a.height}),
        centerToRectangle({b.x, b.y}, {b.width, b.height})
    );
}

} // namespace collision
