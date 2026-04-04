#pragma once

#include "defs.hpp"
#include "game/states.hpp"

#include <cmath>
#include <raylib.h>

// HELPERS

// returns a random point on a circle around a center
// used to spawn enemies just off screen
static Vector2 randomPointOnCircle(Vector2 center, float radius) {
    float angle = ((float)rand() / RAND_MAX) * 2.f * 3.14159f;
    return {
        center.x + cosf(angle) * radius,
        center.y + sinf(angle) * radius,
    };
}

static void initCollider(ECS& ecs, Entity e) {
    ecs.colliders[e].halfSize = {
        ecs.sprites[e].src.width  * ecs.sprites[e].scale / 2.f,
        ecs.sprites[e].src.height * ecs.sprites[e].scale / 2.f,
    };
}

// -----------------------------------------------
// SPAWN ENEMY
// -----------------------------------------------

inline Entity spawnEnemy(GameContext& ctx, const EnemyDef& def, Vector2 pos) {
    ECS& ecs = ctx.ecs;

    Entity e = ecs.create();
    ecs.tags[e].hasEnemy       = true;
    ecs.tags[e].hasContainment = true;
    ecs.transforms[e].position = pos;
    ecs.healths[e].value       = def.health;
    ecs.healths[e].maxValue    = def.health;
    ecs.sprites[e]             = {def.sprite, def.scale};
    ecs.speeds[e].value        = def.speed; // used by systemEnemyAI

    initCollider(ecs, e);
    return e;
}

// -----------------------------------------------
// SPAWN WEAPON
// -----------------------------------------------

inline Entity spawnWeaponForPlayer(GameContext& ctx, const WeaponDef& def, Entity player, float startAngleD) {
    ECS& ecs = ctx.ecs;

    Entity w = ecs.create();
    ecs.tags[w].hasWeapon = true;
    ecs.sprites[w]        = {def.sprite, def.scale};

    switch (def.kind) {

        case WeaponKind::Orbit: {
            ecs.hasOrbit[w]  = true;
            ecs.orbits[w]    = {
                .target     = player,
                .radius     = def.orbitParams.radius,
                .angleD     = startAngleD,
                .rotateRate = def.orbitParams.rotateRate,
            };
            ecs.transforms[w].rotationSpeedD = def.rotationSpeedD;
            break;
        }

        case WeaponKind::Projectile: {
            // projectile weapons aren't entities themselves —
            // they live in PlayerProgress and fire via systemFireArrows.
            // here we just record that the player has this weapon.
            ctx.progress.unlockedWeapons.push_back(WEAPON_ARROW);
            ecs.destroy(w); // don't need the entity
            return -1;
        }
    }

    initCollider(ecs, w);
    return w;
}

// -----------------------------------------------
// SPAWNER SYSTEM
// -----------------------------------------------

// how far off screen enemies spawn
constexpr float spawnRadius = 900.f;

// enemy pool per wave — index maps to ENEMY_DEFS
// early waves only spawn zombies, later waves mix in tougher enemies
static int enemyPoolForWave(int waveNumber) {
    // clamp to available def count
    int maxDef = (int)std::size(ENEMY_DEFS) - 1;
    // every 3 waves, unlock the next enemy type
    return std::min(waveNumber / 3, maxDef);
}

inline void systemSpawner(GameContext& ctx) {
    SpawnerState& s  = ctx.spawner;
    float         dt = GetFrameTime();

    s.timeSinceLastWave += dt;
    if (s.timeSinceLastWave < s.waveInterval) return;

    // reset timer and advance wave
    s.timeSinceLastWave = 0.f;
    s.waveNumber++;

    // ramp difficulty — shorter intervals, more enemies
    s.waveInterval = std::max(1.f, s.waveInterval - 0.15f);
    int count      = 4 + s.waveNumber * 2;

    Vector2 playerPos = ctx.ecs.transforms[ctx.player].position;
    int     poolSize  = enemyPoolForWave(s.waveNumber);

    for (int i = 0; i < count; i++) {
        int     defIndex = rand() % (poolSize + 1);
        Vector2 pos      = randomPointOnCircle(playerPos, spawnRadius);
        spawnEnemy(ctx, ENEMY_DEFS[defIndex], pos);
    }
}
