#pragma once

#include "core/components.hpp"

#include <cassert>
#include <vector>

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
            speeds     .emplace_back();
            colliders  .emplace_back();
            sprites    .emplace_back();
            healths    .emplace_back();
            tags       .emplace_back();
            orbits     .emplace_back();
            directions .emplace_back();
            eventTimers.emplace_back();
            projectiles.emplace_back();
            skills     .emplace_back();
            hasOrbit   .push_back(false);
            alive      .push_back(false);
        }

        alive[e] = true;

        // zero out components so recycled slots are clean
        transforms [e] = {};
        velocities [e] = {};
        speeds     [e] = {};
        colliders  [e] = {};
        sprites    [e] = {};
        healths    [e] = {};
        tags       [e] = {};
        orbits     [e] = {};
        directions [e] = {};
        eventTimers[e] = {};
        projectiles[e] = {};
        skills     [e] = {};
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
        speeds     [id] = speeds[e];
        colliders  [id] = colliders[e];
        sprites    [id] = sprites[e];
        healths    [id] = healths[e];
        tags       [id] = tags[e];
        orbits     [id] = orbits[e];
        directions [id] = directions[e];
        eventTimers[id] = eventTimers[e];
        projectiles[id] = projectiles[e];
        skills     [id] = skills[e];
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

    void markDestroyDelayed(Entity e, float delayS) {
        assert(isAlive(e));
        eventTimers[e].thresholdSec = delayS;
    }

    void destroyPending() {
        for (Entity e : pendingDestroy)
            if (isAlive(e)) destroy(e);
        pendingDestroy.clear();
    }

    // component arrays
    std::vector<TransformComp> transforms;
    std::vector<VelocityComp>  velocities;
    std::vector<SpeedComp>     speeds;
    std::vector<ColliderComp>  colliders;
    std::vector<SpriteComp>    sprites;
    std::vector<HealthComp>    healths;
    std::vector<TagComp>       tags;
    std::vector<OrbitComp>     orbits;
    std::vector<DirectionComp> directions;
    std::vector<TimedComp>     eventTimers;
    std::vector<SkillComponent>skills;
    std::vector<ProjectileComponent>projectiles;
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
