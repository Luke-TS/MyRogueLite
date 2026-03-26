#pragma once

#include <raylib.h>  // raylib types
#include <optional>  // for std::optional
#include <algorithm> // for std::min

inline Rectangle rectFromCenter(const Vector2& center, float width, float height) {
    return Rectangle{
        center.x - width / 2.f,
        center.y - height / 2.f,
        width,
        height
    };
}

// Returns nullopt if the circle is fully enclosed by rec.
// Otherwise returns penetration depth on each axis.
// Note: rec is top-left rectangle
inline std::optional<Vector2> boundsPenetration(const Vector2& origin, const int radius, const Rectangle& rec) {
    float px = 0.0f, py = 0.0f;

    if      (origin.x - radius < rec.x)              px = (origin.x - radius) - rec.x;
    else if (origin.x + radius > rec.x + rec.width)  px = (origin.x + radius) - (rec.x + rec.width);

    if      (origin.y - radius < rec.y)              py = (origin.y - radius) - rec.y;
    else if (origin.y + radius > rec.y + rec.height) py = (origin.y + radius) - (rec.y + rec.height);

    if (px == 0.0f && py == 0.0f)
        return std::nullopt;

    return Vector2{ px, py };
}

// Returns nullopt if rec1 is fully inside rec2.
// Otherwise returns penetration depth of rec1 into rec2 on each axis.
inline std::optional<Vector2> boundsPenetration(const Rectangle& rec1, const Rectangle& rec2) {
    float px = 0.0f, py = 0.0f;

    if(rec1.x < rec2.x)
        px = rec1.x - rec2.x;
    else if (rec1.x + rec1.width > rec2.x + rec2.width)
        px = (rec1.x + rec1.width) - (rec2.x + rec2.width);

    if (rec1.y < rec2.y)
        py = rec1.y - rec2.y;
    else if (rec1.y + rec1.height > rec2.y + rec2.height)
        py = (rec1.y + rec1.height) - (rec2.y + rec2.height);

    if (px == 0.0f && py == 0.0f)
        return std::nullopt;

    return Vector2{ px, py };
}

// same but centered rectangles
inline std::optional<Vector2> boundsPenetrationCentered(
    const Vector2& center1, float w1, float h1,
    const Vector2& center2, float w2, float h2)
{
    Rectangle rec1 = rectFromCenter(center1, w1, h1);
    Rectangle rec2 = rectFromCenter(center2, w2, h2);

    return boundsPenetration(rec1, rec2);
}

// Returns nullopt if rec1 and rec2 do not intersect.
// Otherwise returns how far rec1 has penetrated into rec2 on each axis.
inline std::optional<Vector2> rectIntersection(const Rectangle& rec1, const Rectangle& rec2) {
    const float overlapX = std::min(rec1.x + rec1.width,  rec2.x + rec2.width)  - std::max(rec1.x, rec2.x);
    const float overlapY = std::min(rec1.y + rec1.height, rec2.y + rec2.height) - std::max(rec1.y, rec2.y);

    if (overlapX <= 0.0f || overlapY <= 0.0f)
        return std::nullopt;

    // Push out along the axis of least penetration
    const float px = (rec1.x + rec1.width  * 0.5f) < (rec2.x + rec2.width  * 0.5f) ? -overlapX : overlapX;
    const float py = (rec1.y + rec1.height * 0.5f) < (rec2.y + rec2.height * 0.5f) ? -overlapY : overlapY;

    return Vector2{ px, py };
}

// same but with centered rectangles
inline std::optional<Vector2> rectIntersectionCentered(
    const Vector2& center1, float w1, float h1,
    const Vector2& center2, float w2, float h2)
{
    Rectangle rec1 = rectFromCenter(center1, w1, h1);
    Rectangle rec2 = rectFromCenter(center2, w2, h2);

    return rectIntersection(rec1, rec2);
}
