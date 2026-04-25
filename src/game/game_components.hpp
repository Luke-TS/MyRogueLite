#pragma once

// Game-specific ECS components.
// These depend on game/defs.hpp and live at the game layer,
// unlike the pure-data components in core/components.hpp.

#include "types.hpp"
#include "game/defs.hpp"

#include <vector>

// -----------------------------------------------
// Skill instance (runtime skill state per entity)
// -----------------------------------------------

struct SkillInstance {
    Defs::SkillDef* def;

    std::vector<Defs::Effect> builtEffects;
    float cooldownTimer = 0.f;
};

// -----------------------------------------------
// Game-specific components
// -----------------------------------------------

struct SkillComponent {
    std::vector<SkillInstance> skills;
};

struct ProjectileComponent {
    Entity owner;
    float  damage;

    std::vector<Defs::Effect> onHitEffects;
};
