#pragma once

#include "types.hpp"
#include "game/defs.hpp"

#include <cstdint>
#include <vector>
#include <raylib.h>

// -----------------------------------------------
// Skill instance (runtime skill state)
// -----------------------------------------------

struct SkillInstance {
    Defs::SkillDef* def;

    std::vector<Defs::Effect> builtEffects;
    float cooldownTimer = 0.f;
};

// -----------------------------------------------
// Components
// -----------------------------------------------

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

// used for timing events
// currently used by ecs to mark entities for delayed destruction
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

struct SkillComponent {
    std::vector<SkillInstance> skills;
};

struct ProjectileComponent {
    Entity owner;

    float damage;

    std::vector<Defs::Effect> onHitEffects;
};
