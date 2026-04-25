#pragma once

#include <string>
#include <vector>
#include "game/dungeon_sprites.hpp"

namespace Defs {

// -----------------------------------------------
// ENEMY DEFINITIONS
// -----------------------------------------------

struct EnemyDef {
    std::string               name;
    float                     speed;
    float                     health;
    DungeonSprites::SpriteIdx sprite;
    float                     scale;
};

enum EnemyIdx {
    SKELETON,
    ZOMBIE,
    BIGBOI,

    ENEMY_COUNT // total number of base enemies
};
inline std::array<EnemyDef, EnemyIdx::ENEMY_COUNT> enemies = {{
    {
        .name   = "Skeleton",
        .speed  = 150.f,
        .health = 3.f,
        .sprite = DungeonSprites::SpriteIdx::SKELETON,
        .scale  = 4.f,
    },
    {
        .name   = "Zombie",
        .speed  = 150.f,
        .health = 5.f,
        .sprite = DungeonSprites::SpriteIdx::ZOMBIE,
        .scale  = 4.f,
    },
    {
        .name   = "BigBoi",
        .speed  = 120.f,
        .health = 10.f,
        .sprite = DungeonSprites::SpriteIdx::BIGBOI,
        .scale  = 4.f,
    },
}};

// -----------------------------------------------
// SKILL DEFINITIONS
// -----------------------------------------------

// How a skill activates. Each type has its own activation params below.
enum class SkillType {
    Projectile,
    Melee,
    // Curse,
    // Aura,
};

// Activation parameters for SkillType::Projectile.
struct ProjectileParams {
    float speed    = 400.f;
    float fireRate = 1.f;   // activations per second
    int   count    = 1;     // projectiles spawned per activation
    float spread   = 0.f;   // arc between projectiles in degrees (count > 1)
};

// Activation parameters for SkillType::Melee.
struct MeleeParams {
    float range      = 150.f;
    float arcDegrees = 90.f;
};

// Effects that run when a skill entity (projectile, hit-zone, etc.) touches something.
enum class EffectType : int8_t {
    WallBounce,  // reflect projectile velocity on wall contact
    DealDamage,  // subtract damage from target health
    // Chain,    // bounce to nearest enemy
    // Pierce,   // pass through enemies
    MultiShot,
    Count,
};

struct Effect {
    EffectType type;
    float      value0 = 0.f; // generic parameters (e.g. damage multiplier)
    float      value1 = 0.f;
    int        count  = 0;
};

struct SkillDef {
    std::string               name;
    SkillType                 type;
    float                     baseDamage;
    DungeonSprites::SpriteIdx sprite;
    float                     scale;

    // Activation config — read the field matching `type`.
    ProjectileParams projectile;
    MeleeParams      melee;

    // On-hit effect chain applied to entities this skill spawns/hits.
    std::vector<Effect> onHitEffects;
};

enum SkillIdx {
    SKILL_BOW,
    SKILL_FIREBALL,

    SKILL_COUNT
};
inline std::array<SkillDef, SKILL_COUNT> skills = {{
    {
        .name       = "Bow",
        .type       = SkillType::Projectile,
        .baseDamage = 2.f,
        .sprite     = DungeonSprites::SpriteIdx::ARROW,
        .scale      = 4.f,
        .projectile = { .speed = 500.f, .fireRate = 3.f, .count = 1 },
        .onHitEffects = {
            //{ .type = EffectType::WallBounce },
            { .type = EffectType::DealDamage },
            { .type = EffectType::MultiShot  },
        },
    },
    {
        .name       = "Fireball",
        .type       = SkillType::Projectile,
        .baseDamage = 4.f,
        .sprite     = DungeonSprites::SpriteIdx::FIRE,
        .scale      = 4.f,
        .projectile = { .speed = 300.f, .fireRate = 2.f, .count = 1 },
        .onHitEffects = {
            { .type = EffectType::DealDamage },
        },
    },
}};

// -----------------------------------------------
// PLAYER CHARACTER DEFINITIONS
// -----------------------------------------------

struct CharacterDef {
    std::string                  name;
    float                        speed;
    float                        maxHealth;
    DungeonSprites::SpriteIdx    sprite;
    float                        scale;
    Defs::SkillIdx               startingSkill;
};

enum CharacterIdx {
    ARCHER,
    MAGE,

    CHARACTER_COUNT // total number of base characters
};
inline std::array<CharacterDef, CharacterIdx::CHARACTER_COUNT> characters = {{
    {
        .name   = "Archer",
        .speed  = 350.f,
        .maxHealth = 100.f,
        .sprite = DungeonSprites::SpriteIdx::CHARACTER_2,
        .scale  = 4.f,
        .startingSkill = SKILL_BOW,
    },
    {
        .name   = "Mage",
        .speed  = 300.f,
        .maxHealth = 80.f,
        .sprite = DungeonSprites::SpriteIdx::CHARACTER_3,
        .scale  = 4.f,
        .startingSkill = SKILL_FIREBALL,
    },
}};

}
