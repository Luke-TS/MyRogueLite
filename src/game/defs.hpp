#pragma once

#include <string>
#include "core/ecs.hpp"
#include "core/dungeon_tileset.hpp"

// -----------------------------------------------
// ENEMY DEFINITIONS
// -----------------------------------------------

struct EnemyDef {
    std::string name;
    float       speed;
    float       health;
    Rectangle   sprite;
    float       scale;
};

constexpr auto ZOMBIE = 0;
inline const EnemyDef ENEMY_DEFS[] = {
    {
        .name   = "Zombie",
        .speed  = 150.f,
        .health = 100.f,
        .sprite = DungeonTileSet::monsterStart,
        .scale  = 4.f,
    },
    // add more enemy types here as you make new sprites
};

// -----------------------------------------------
// WEAPON DEFINITIONS
// -----------------------------------------------

enum class WeaponKind {
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
    std::string    name;
    float          damage;
    WeaponKind     kind;
    Rectangle      sprite;
    float          scale;
    float          rotationSpeedD; // spin of the sprite itself

    // only one of these is used depending on kind
    OrbitParams      orbitParams;
    ProjectileParams projParams;
};

constexpr auto WEAPON_AXE   = 0;
constexpr auto WEAPON_ARROW = 1;
inline const WeaponDef WEAPON_DEFS[] = {
    {
        .name           = "Axe",
        .damage         = 50.f,
        .kind           = WeaponKind::Orbit,
        .sprite         = DungeonTileSet::axeSprite,
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
        .sprite    = DungeonTileSet::arrowSprite,
        .scale     = 4.f,
        .projParams = {
            .speed    = 500.f,
            .fireRate = 4.f, // per second
        },
    },
};
