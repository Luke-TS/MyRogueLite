#pragma once

#include "core/tilemap.hpp"
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

// returns a random point on a circle around a center
// used to spawn enemies just off screen
static Vector2 randomPointOnRectangle(Vector2 center, Rectangle rec) {
    return {
        rec.x + (rand() % (int)rec.width),
        rec.y + (rand() & (int)rec.height),
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

inline Entity spawnEnemy(GameContext& ctx, const Defs::EnemyDef& def, Vector2 pos) {
    ECS& ecs = ctx.ecs;

    Entity e = ecs.create();
    ecs.tags[e].hasEnemy       = true;
    ecs.tags[e].hasContainment = true;
    ecs.transforms[e].position = pos;
    ecs.healths[e].value       = def.health;
    ecs.healths[e].maxValue    = def.health;
    ecs.sprites[e]             = {DungeonSprites::sprites[def.sprite], def.scale};
    ecs.speeds[e].value        = def.speed; // used by systemEnemyAI

    initCollider(ecs, e);
    return e;
}

// -----------------------------------------------
// SPAWN WEAPON
// -----------------------------------------------

inline Entity spawnOribtWeapon(GameContext& ctx, const Defs::WeaponDef& def, Entity player, float startAngleD) {
    ECS& ecs = ctx.ecs;

    Entity w = ecs.create();
    ecs.tags[w].hasWeapon = true;
    ecs.sprites[w]        = {DungeonSprites::sprites[def.sprite], def.scale};

    switch (def.kind) {

        case Defs::WeaponKind::Orbit: {
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

        case Defs::WeaponKind::Projectile: {
            // projectile weapons aren't entities themselves —
            // they live in PlayerProgress and fire via systemFireArrows.
            // here we just record that the player has this weapon.
            ctx.progress.unlockedWeapons.push_back(Defs::WEAPON_BOW);
            ecs.destroy(w); // don't need the entity
            return -1;
        }
    }

    initCollider(ecs, w);
    return w;
}

// -----------------------------------------------
// SPAWN PLAYER CHARACTER 
// -----------------------------------------------

inline Entity spawnPlayer(GameContext& ctx, const Defs::CharacterDef& def, Vector2 pos) {
    auto& ecs = ctx.ecs;

    Entity e = ecs.create();
    ecs.tags[e].hasPlayer      = true;
    ecs.tags[e].hasContainment = true;
    ecs.transforms[e].position = getCenterPos(ctx.map);
    ecs.healths[e].value       = def.health;
    ecs.healths[e].maxValue    = def.health;
    ecs.sprites[e]             = {DungeonSprites::sprites[def.sprite], def.scale};
    ctx.progress.unlockedWeapons = {def.startingWeapon};

    if(def.startingWeapon == Defs::WeaponIdx::WEAPON_AXE) {
        spawnOribtWeapon(ctx, Defs::weapons[Defs::WEAPON_AXE], e, 0.f);
    }

    initCollider(ecs, e); // default collider
    return e;
}

// -----------------------------------------------
// SPAWNER SYSTEM
// -----------------------------------------------

// how far off screen enemies spawn
constexpr float spawnRadius = 900.f;

// enemy pool per wave — index maps to Defs::enemies
// early waves only spawn skeletons, later waves mix in tougher enemies
static int enemyPoolForWave(int waveNumber) {
    // clamp to available def count
    int maxDef = (int)(Defs::ENEMY_COUNT) - 1;
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

    Vector2 playerPos = ctx.ecs.transforms[ctx.playerID].position;
    int     poolSize  = enemyPoolForWave(s.waveNumber);

    const auto nearTiles = getNearbyMapTiles(ctx.map, ctx.ecs.transforms[ctx.playerID].position);
    const auto numTiles  = nearTiles.size();
    for (int i = 0; i < count; i++) {
        int     defIndex  = rand() % (poolSize + 1);
        int     tileIndex = rand() % (numTiles);
        Vector2 pos       = randomPointOnRectangle(playerPos, nearTiles[tileIndex]);
        spawnEnemy(ctx, Defs::enemies[defIndex], pos);
    }
}
