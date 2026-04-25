#pragma once

#include <raylib.h>
#include <vector>

using Entity = int;

struct CollisionEvent {
    Entity         a, b;
    Vector2        penetration; // a into b
};

struct CollisionSets {
    std::vector<CollisionEvent> playerEnemy;
    std::vector<CollisionEvent> weaponEnemy;
    std::vector<CollisionEvent> enemyEnemy;
    std::vector<CollisionEvent> entityTile;
};

struct HitEvent {
    Entity attacker;
    Entity target;
    Vector2 normal;
};

struct WallHitEvent {
    Entity e;
    Vector2 normal;
};

struct DeathEvent {
    Entity e;
};

