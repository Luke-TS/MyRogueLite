#pragma once
#include <cassert>
#include <vector>
#include <raylib.h>

using Entity = int;
constexpr Entity NULL_ENTITY = -1;

// components

struct TransformComp {
    Vector2 position       = {0, 0};
    float   angleD         = 0.f;
    float   rotationSpeedD = 0.f;
};

struct VelocityComp {
    Vector2 value = {0, 0};
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

struct TagComp {
    bool hasPlayer      = false;
    bool hasEnemy       = false;
    bool hasWeapon      = false;
    bool hasContainment = false;
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

// ECS

struct ECS {

    Entity create() {
        Entity e;

        if (!freeList.empty()) {
            e = freeList.back();
            freeList.pop_back();
        } else {
            e = (Entity)alive.size();

            // grow all component arrays by 1
            transforms .emplace_back();
            velocities .emplace_back();
            colliders  .emplace_back();
            sprites    .emplace_back();
            healths    .emplace_back();
            tags       .emplace_back();
            orbits     .emplace_back();
            directions .emplace_back();
            hasOrbit   .push_back(false);
            alive      .push_back(false);
        }

        alive[e] = true;

        // zero out components so recycled slots are clean
        transforms [e] = {};
        velocities [e] = {};
        colliders  [e] = {};
        sprites    [e] = {};
        healths    [e] = {};
        tags       [e] = {};
        orbits     [e] = {};
        directions [e] = {};
        hasOrbit   [e] = false;

        return e;
    }

    Entity createCopy(Entity e) {
        assert(isAlive(e));
        Entity id = create();

        alive[id] = true;

        // copy over components
        transforms [id] = transforms[e];
        velocities [id] = velocities[e];
        colliders  [id] = colliders[e];
        sprites    [id] = sprites[e];
        healths    [id] = healths[e];
        tags       [id] = tags[e];
        orbits     [id] = orbits[e];
        directions [id] = directions[e];
        hasOrbit   [id] = hasOrbit[e];
        
        return id;
    }

    // call this at end-of-frame only, never mid-frame
    void destroy(Entity e) {
        assert(isAlive(e));
        alive[e] = false;
        tags [e] = {};      // clear tags so queries skip it immediately
        freeList.push_back(e);
    }

    bool isAlive(Entity e) const {
        if (e < 0 || e >= (int)alive.size()) return false;
        return alive[e];
    }

    int capacity() const { return (int)alive.size(); }

    // pending destroy (call destroyPending() end of frame)

    void markForDestroy(Entity e) {
        pendingDestroy.push_back(e);
    }

    void destroyPending() {
        for (Entity e : pendingDestroy)
            if (isAlive(e)) destroy(e);
        pendingDestroy.clear();
    }

    // component arrays
    std::vector<TransformComp> transforms;
    std::vector<VelocityComp>  velocities;
    std::vector<ColliderComp>  colliders;
    std::vector<SpriteComp>    sprites;
    std::vector<HealthComp>    healths;
    std::vector<TagComp>       tags;
    std::vector<OrbitComp>     orbits;
    std::vector<DirectionComp> directions;
    std::vector<bool>          hasOrbit;   // presence flag for sparse orbit

    // cached views - updated once per frame
    std::vector<Entity> players;
    std::vector<Entity> enemies;
    std::vector<Entity> weapons;

    void rebuildViews() {
        players.clear();
        enemies.clear();
        weapons.clear();

        for (int e = 0; e < capacity(); e++) {
            if (!isAlive(e)) continue;
            if (tags[e].hasPlayer) players.push_back(e);
            if (tags[e].hasEnemy)  enemies.push_back(e);
            if (tags[e].hasWeapon) weapons.push_back(e);
        }
    }

    // internal

    std::vector<bool>   alive;
    std::vector<Entity> freeList;
    std::vector<Entity> pendingDestroy;
};
