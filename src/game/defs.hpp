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
enum class EffectType : int8_t {
    SpawnProjectile,
    //SpawnOrbit,
    WallBounce,
    DealDamage,
    Count,
};

struct Effect {
    EffectType type;

    float value0; // generic parameters
    float value1;
    int   count;
};

struct SkillDef {
    std::string               name;
    float                     baseDamage;
    DungeonSprites::SpriteIdx sprite;
    float                     scale;

    std::vector<Effect> effects;
};

enum SkillIdx {
    //SKILL_AXE,
    SKILL_BOW,
    SKILL_FIREBALL,

    SKILL_COUNT // total number of base skill
};
inline std::array<SkillDef, SKILL_COUNT> skills = {{
    /*
    {
        .name = "Axe",
        .baseDamage = 50.f,
        .sprite = DungeonSprites::SpriteIdx::AXE,
        .scale = 4.f,
        .effects = {
            {
                .type = EffectType::SpawnOrbit,
                .value0 = 128.f,   // radius
                .value1 = 0.25f,   // rotateRate
                .count  = 1
            }
        }
    },
    */
    {
        .name = "Bow",
        .baseDamage = 2.f,
        .sprite = DungeonSprites::SpriteIdx::ARROW,
        .scale = 4.f,
        .effects = {
            {
                .type = EffectType::SpawnProjectile,
                .value0 = 500.f, // speed
                .value1 = 3.f,   // fireRate
                .count  = 1
            },
            {
                .type = EffectType::WallBounce,
            },
            {
                .type = EffectType::DealDamage,
                // .value0 = baseDamage
                // computed at build/activation time
            }
        }
    },
    {
        .name = "Fireball",
        .baseDamage = 4.f,
        .sprite = DungeonSprites::SpriteIdx::FIRE,
        .scale = 4.f,
        .effects = {
            {
                .type = EffectType::SpawnProjectile,
                .value0 = 300.f, // speed
                .value1 = 2.f,   // fireRate
                .count  = 1
            },
            {
                .type = EffectType::DealDamage,
                // .value0 = baseDamage
                // computed at build/activation time
            }
        }
    }
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
    //WARRIOR,
    ARCHER,
    MAGE,

    CHARACTER_COUNT // total number of base characters
};
inline std::array<CharacterDef, CharacterIdx::CHARACTER_COUNT> characters = {{
    /*
    {
        .name   = "Warrior",
        .speed  = 150.f,
        .health = 100.f,
        .sprite = DungeonSprites::SpriteIdx::CHARACTER_1,
        .scale  = 4.f,
        .startingSkill = SKILL_AXE,
    },
    */
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
