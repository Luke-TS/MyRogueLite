#pragma once

// Pure-data engine components — no game-layer dependencies.
// Game-specific components (SkillComponent, ProjectileComponent) live in game/game_components.hpp.

#include "types.hpp"

#include <cstdint>
#include <raylib.h>

struct TransformComp {
    Vector2 position       = {0, 0};
    float   angleD         = 0.f;
    float   rotationSpeedD = 0.f;
};

struct VelocityComp {
    Vector2 value = {0, 0};
};

struct SpeedComp {
    float value = 0.f;
};

struct ColliderComp {
    Vector2 halfSize = {0, 0};
};

struct SpriteComp {
    Rectangle src   = {0, 0, 0, 0};
    float     scale = 1.f;
};

struct HealthComp {
    float value    = 0.f;
    float maxValue = 0.f;
};

// Used to mark entities for delayed destruction.
struct TimedComp {
    float    thresholdSec = 0.f;
    uint16_t frameCount   = 0;
};

struct TagComp {
    bool hasPlayer        = false;
    bool hasEnemy         = false;
    bool hasWeapon        = false;
    bool hasContainment   = false;
    bool hasProjectile    = false;
};

struct OrbitComp {
    Entity target     = NULL_ENTITY;
    float  radius     = 0.f;
    float  angleD     = 0.f;
    float  rotateRate = 0.f; // rotations per second
};

struct DirectionComp {
    int value = 1; // 1 = right, -1 = left
};
