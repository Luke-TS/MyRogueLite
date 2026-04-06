#pragma once

#include <string>
#include <vector>
#include "core/dungeon_sprites.hpp"

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
// WEAPON DEFINITIONS
// -----------------------------------------------

enum WeaponKind {
    Orbit,       // axe, shield — circles the player
    Projectile,  // arrow — fires toward cursor
};

struct OrbitParams {
    float radius;
    float rotateRate;
    float startAngleD; // offset for multi-weapon spread
};

struct ProjectileParams {
    float speed;
    float fireRate; // shots per second
};

struct WeaponDef {
    std::string               name;
    float                     damage;
    WeaponKind                kind;
    DungeonSprites::SpriteIdx sprite;
    float                     scale;
    float                     rotationSpeedD; // spin of the sprite itself

    // only one of these is used depending on kind
    OrbitParams      orbitParams;
    ProjectileParams projParams;
};

enum WeaponIdx {
    WEAPON_AXE,
    WEAPON_BOW,
    WEAPON_FIRE,

    WEAPON_COUNT // total number of base enemies
};

inline std::array<WeaponDef, WEAPON_COUNT> weapons = {{
    {
        .name           = "Axe",
        .damage         = 50.f,
        .kind           = WeaponKind::Orbit,
        .sprite         = DungeonSprites::SpriteIdx::AXE,
        .scale          = 4.f,
        .rotationSpeedD = 720.f,
        .orbitParams    = {
            .radius      = 128.f,
            .rotateRate  = 0.25f,
            .startAngleD = 0.f,
        },
    },
    {
        .name      = "Bow",
        .damage    = 25.f,
        .kind      = WeaponKind::Projectile,
        .sprite    = DungeonSprites::SpriteIdx::BOW,
        .scale     = 4.f,
        .projParams = {
            .speed    = 500.f,
            .fireRate = 4.f, // per second
        },
    },
}};

// -----------------------------------------------
// PLAYER CHARACTER DEFINITIONS
// -----------------------------------------------

struct CharacterDef {
    std::string                  name;
    float                        speed;
    float                        health;
    DungeonSprites::SpriteIdx    sprite;
    float                        scale;
    Defs::WeaponIdx              startingWeapon;
};

enum CharacterIdx {
    WARRIOR,
    ARCHER,
    MAGE,

    CHARACTER_COUNT // total number of base enemies
};
inline std::array<CharacterDef, CharacterIdx::CHARACTER_COUNT> characters = {{
    {
        .name   = "Warrior",
        .speed  = 150.f,
        .health = 100.f,
        .sprite = DungeonSprites::SpriteIdx::CHARACTER_1,
        .scale  = 4.f,
        .startingWeapon = Defs::WeaponIdx::WEAPON_AXE,
    },
    {
        .name   = "Archer",
        .speed  = 175.f,
        .health = 100.f,
        .sprite = DungeonSprites::SpriteIdx::CHARACTER_2,
        .scale  = 4.f,
        .startingWeapon = Defs::WeaponIdx::WEAPON_BOW,
    },
    {
        .name   = "Mage",
        .speed  = 125.f,
        .health = 80.f,
        .sprite = DungeonSprites::SpriteIdx::CHARACTER_3,
        .scale  = 4.f,
        .startingWeapon = Defs::WeaponIdx::WEAPON_BOW,
    },
}};

}
