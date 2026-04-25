#pragma once

#include "game/dungeon_sprites.hpp"
#include "core/ecs.hpp"
#include "core/vec2_ops.hpp" // Vector2 overloads
#include "core/physics.hpp"

#include "game/defs.hpp"
#include "game/skills.hpp"
#include "states.hpp"

#include "types.hpp"

#include <cstdio>
#include <raylib.h>

// angle calculation helpers
// PI is defined by raylib as 3.14159265358979323846f
constexpr float degreesToRadians   = PI / 180.f;
constexpr float radiansToDegrees   = 180.f / PI;
constexpr int   degreesPerRotation = 360;

// moves all player entities using WASD controls
// speed is determined by speed component
inline void systemPlayerMovement(ECS& ecs) {
    Vector2 moveVec = {0, 0};
    if(IsKeyDown(KEY_D))
        moveVec.x += 1; // value of 1 is arbitrary
    if(IsKeyDown(KEY_A))
        moveVec.x -= 1;
    if(IsKeyDown(KEY_W))
        moveVec.y -= 1;
    if(IsKeyDown(KEY_S))
        moveVec.y += 1;
    Vector2 moveDir = normalize(moveVec);

    for (const auto& p : ecs.players) 
    {
        if (moveDir.x < -0.1f) ecs.directions[p].value = -1;
        if (moveDir.x > +0.1f) ecs.directions[p].value = 1;

        ecs.velocities[p].value = moveDir * ecs.speeds[p].value;
    }
}

inline void systemEnemyAI(ECS& ecs) {
    for(const auto& e : ecs.enemies) 
    {
        if(ecs.healths[e].value == 0) { // dead enemy
            ecs.velocities[e].value = {0.f, 0.f};
            continue;
        }

        // TODO: remove hard-coded playerID
        Vector2 dir = normalize(ecs.transforms[ecs.players[0]].position - ecs.transforms[e].position);

        ecs.velocities[e].value = dir * ecs.speeds[e].value;

        if( dir.x < 0) ecs.directions[e].value = -1;
        else           ecs.directions[e].value = 1;
    }
}

inline void systemIntegration(ECS& ecs, float dt) {
    for (Entity e = 0; e < ecs.capacity(); e++) 
    {
        if (!ecs.isAlive(e)) continue;
        if (ecs.hasOrbit[e]) continue; // orbit system owns these
        auto& eTran = ecs.transforms[e];
        eTran.position += ecs.velocities[e].value * dt;
        eTran.angleD += eTran.rotationSpeedD * dt;
    }
}

inline void systemOrbit(ECS& ecs, float dt) {
    for (Entity e = 0; e < ecs.capacity(); e++)
    {
        if (!ecs.isAlive(e))  continue; // skip
        if (!ecs.hasOrbit[e]) continue; // skip

        OrbitComp& o = ecs.orbits[e];

        // update angle
        o.angleD += dt * o.rotateRate * degreesPerRotation;

        // get target position
        if(!ecs.isAlive(o.target)) continue; // target entity invalid
        Vector2 center = ecs.transforms[o.target].position;

        // compute offset
        Vector2 offset = {
            static_cast<float>(std::cos(o.angleD * degreesToRadians) * o.radius),
            static_cast<float>(std::sin(o.angleD * degreesToRadians) * o.radius),
        };

        // set entity position
        ecs.transforms[e].position = center + offset;
        ecs.transforms[e].angleD += ecs.transforms[e].rotationSpeedD * dt;
    }
}

inline void systemRenderEntities(GameContext& ctx, const Color c = WHITE) {
    auto& ecs = ctx.ecs;

    for (Entity e = 0; e < ecs.capacity(); e++) 
    {
        if(!ecs.isAlive(e)) continue;
        Rectangle src = ecs.sprites[e].src; // copy

        // apply horizontal mirror based off direction
        src.width *= ecs.directions[e].value;

        Vector2 renderSize = {
            .x = ecs.sprites[e].src.width * ecs.sprites[e].scale,
            .y = ecs.sprites[e].src.height * ecs.sprites[e].scale,
        };
        Rectangle destRec = {
            ecs.transforms[e].position.x,
            ecs.transforms[e].position.y,
            renderSize.x,
            renderSize.y,
        };
        Vector2 origin = {
            destRec.width / 2.f,
            destRec.height / 2.f,
        };
        DrawTexturePro(
            ctx.tileTexture,
            src,
            destRec,
            origin,
            ecs.transforms[e].angleD,
            c 
        );

        // DEBUG: draw bounding box

        if(!ctx.debugMode) continue;

        Rectangle debugRect = {
            ecs.transforms[e].position.x - renderSize.x / 2.f,
            ecs.transforms[e].position.y - renderSize.y / 2.f,
            renderSize.x,
            renderSize.y
        };

        DrawRectangleLinesEx(debugRect, 2.0f, GREEN);
        DrawCircleV(ecs.transforms[e].position, 3.0f, RED);
    }
}

inline void detectPlayerVsEnemy(const GameContext& ctx, std::vector<CollisionEvent>& out) {
    auto& ecs = ctx.ecs;
    for (Entity p : ecs.players)
    {
        for (Entity e : ecs.enemies)
        {
            auto pen = physics::intersectCentered(
                ecs.transforms[p].position, ecs.colliders[p].halfSize * 2.f,
                ecs.transforms[e].position, ecs.colliders[e].halfSize * 2.f
            );
            if (pen) 
                out.push_back({p, e, *pen});
        }
    }
}

inline void detectWeaponVsEnemy(const GameContext& ctx, std::vector<CollisionEvent>& out) {
    auto& ecs = ctx.ecs;
    for (Entity w : ecs.weapons)
    {
        for (Entity e : ecs.enemies)
        {
            auto pen = physics::intersectCentered(
                ecs.transforms[w].position, ecs.colliders[w].halfSize * 2.f,
                ecs.transforms[e].position, ecs.colliders[e].halfSize * 2.f
            );
            if (pen) 
                out.push_back({w, e, *pen});
        }
    }
}

inline void detectEnemyVsEnemy(const GameContext& ctx, std::vector<CollisionEvent>& out) {
    auto& ecs = ctx.ecs;

    for (int i = 0; i < (int)ecs.enemies.size(); i++)
    {
        for (int j = i + 1; j < (int)ecs.enemies.size(); j++)
        {
            Entity ei = ecs.enemies[i];
            Entity ej = ecs.enemies[j];

            auto pen = physics::intersectCentered(
                ecs.transforms[ei].position, ecs.colliders[ei].halfSize * 2.f,
                ecs.transforms[ej].position, ecs.colliders[ej].halfSize * 2.f
            );

            if (pen)
                out.push_back({ei, ej, *pen});
        }
    }
}

inline void detectEntityVsTile(const GameContext& ctx, std::vector<CollisionEvent>& out) {
    auto& ecs = ctx.ecs;
    for (Entity e = 0; e < ecs.capacity(); e++) 
    {
        if (!ecs.isAlive(e))            continue;
        if (!ecs.tags[e].hasContainment) continue;

        Rectangle entityRect = physics::centerToRectangle(
            ecs.transforms[e].position,
            ecs.colliders[e].halfSize * 2.f
        );

        auto tiles = getNearbySolidTiles(ctx.map, ecs.transforms[e].position);

        for (auto& tileRect : tiles) 
        {
            if (auto pen = physics::intersect(entityRect, tileRect)) {
                out.push_back({e, -1, *pen});
            }
        }
    }
}

// detection — calls core/physics, produces collision events
inline void systemCollisionDetect(const GameContext& ctx, CollisionSets& out) {
    detectPlayerVsEnemy(ctx, out.playerEnemy);
    detectWeaponVsEnemy(ctx, out.weaponEnemy);
    detectEnemyVsEnemy(ctx, out.enemyEnemy);
    detectEntityVsTile(ctx, out.entityTile);
}

inline void resolveSeparation(GameContext& ctx, CollisionEvent& event) {
    auto& ecs = ctx.ecs;
    event.penetration /= 4.f;
    ecs.transforms[event.a].position += event.penetration;
    ecs.transforms[event.b].position -= event.penetration;
}

inline void resolveContained(GameContext& ctx, CollisionEvent& event) {
    auto& ecs = ctx.ecs;

    if(!ecs.tags[event.a].hasContainment) return;

    ecs.transforms[event.a].position += event.penetration;
}

// physical resolution only - adjusts entity position
// containment, separation, knockback, etc.
inline void systemPhysicsResolve(GameContext& ctx, CollisionSets& sets) {
    auto& ecs = ctx.ecs;

    for(auto& c : sets.entityTile)
        resolveContained(ctx, c);
    for(auto& c : sets.enemyEnemy)
        resolveSeparation(ctx, c);

    // playerEnemy and weaponEnemy has no positional resolution
}


inline void systemCollisionToHitEvents(
    GameContext& ctx,
    const CollisionSets& sets,
    std::vector<HitEvent>& hits,
    std::vector<WallHitEvent>& wallHits
) {
    auto& ecs = ctx.ecs;

    for(auto& c : sets.entityTile) {
        wallHits.push_back({
            .e = c.a,
            .normal = normalize(c.penetration)
        });
    }
    for(auto& c : sets.playerEnemy) {
        hits.push_back({
            .attacker = c.b,  // enemy deals damage to player
            .target   = c.a,
            .normal   = normalize(c.penetration)
        });
    }
    for(auto& c : sets.weaponEnemy) {
        hits.push_back({
            .attacker = c.a,
            .target   = c.b,
            .normal   = normalize(c.penetration)
        });
    }
}

inline Vector2 reflect(Vector2 velocity, Vector2 normal) {
    // v - 2(v·n)n  — normal must be a unit vector
    return velocity - 2.f * dot(velocity, normal) * normal;
}

inline void systemOnHitEffects(
    GameContext& ctx,
    const std::vector<HitEvent>& hits
) {
    auto& ecs = ctx.ecs;

    for (auto& h : hits) {

        if (ecs.tags[h.attacker].hasProjectile) {
            // PoE-style: walk the compiled effect chain
            auto& proj = ecs.projectiles[h.attacker];
            for (auto& effect : proj.onHitEffects)
            if (auto fn = EffectTable[(size_t)effect.type])
                fn(ctx, h, effect);
        }

        else if (ecs.tags[h.attacker].hasEnemy) {
            // Flat stats: no gem chain, just read from enemy component directly
            //TODO: applyEnemyContactHit(ctx, h);
            ecs.healths[h.target].value -= 5.f;
            ecs.transforms[h.attacker].position -=  normalize(h.normal) * 20.f;
        }
    }
}

inline void systemOnWallHitEffects(
    GameContext& ctx,
    const std::vector<WallHitEvent>& hits
) {
    auto& ecs = ctx.ecs;

    for(auto& w : hits) {
        // players/enemies hitting walls — already position-resolved, nothing more to do
        if (!ecs.tags[w.e].hasProjectile) continue;

        auto& proj = ecs.projectiles[w.e];

        // check for WallBounce first so DealDamage knows not to destroy a bouncing projectile
        bool hasWallBounce = false;
        for (auto& effect : proj.onHitEffects)
            if (effect.type == Defs::EffectType::WallBounce) { hasWallBounce = true; break; }

        for (auto& effect : proj.onHitEffects) {
            switch (effect.type) {

                case Defs::EffectType::WallBounce:
                    ecs.velocities[w.e].value = reflect(ecs.velocities[w.e].value, w.normal);
                    break;

                case Defs::EffectType::SpawnProjectile:
                    // e.g. splitting arrow fans out on wall contact
                    // spawnProjectileEffect(ctx, w.e, w.normal, effect);
                    break;

                case Defs::EffectType::DealDamage:
                    if (!hasWallBounce)
                        ecs.destroy(w.e);
                    break;

                default:
                    break;
            }
        }
    }
}

inline void systemDeathDetection(GameContext& ctx, std::vector<DeathEvent>& deaths) {
    auto& ecs = ctx.ecs;
    for(const auto& p : ecs.players) {
        if(ecs.healths[p].value < 0.1f) {// dead
            deaths.push_back({.e = p});
        }
    }
    for(const auto& e : ecs.enemies) {
        if(ecs.healths[e].value < 0.1f) {// dead
            deaths.push_back({.e = e});
        }
    }
}

inline void systemDeathResolution(GameContext& ctx, const std::vector<DeathEvent>& deaths) {
    for(const auto& d : deaths) {
        if(d.e == ctx.playerID) // player death
            ctx.state = GameState::GameOver;
        else {
            // TODO: add custom death rules and animations here

            // xp equal to maximum heath of enemy
            ctx.progress.xp += ctx.ecs.healths[d.e].maxValue;
            ctx.ecs.markForDestroy(d.e);
        }
    }
}

inline void systemRenderMap(GameContext& ctx, const Color c = WHITE) {
    auto& map = ctx.map;  

    for (int y = 0; y < map.height; y++) {
        for (int x = 0; x < map.width; x++) {
            auto tile = getTile(map, x, y);
            if (!tile || tile->solid) continue;

            Rectangle dest = {
                x * map.tileSize,
                y * map.tileSize,
                map.tileSize,
                map.tileSize
            };

            DrawTexturePro(
                ctx.tileTexture,
                DungeonSprites::getFloorTile(tile->spriteIndex),
                dest, 
                {0,0}, 
                0,
                c);
        }
    }
}

inline void systemEffectExecution(GameContext& ctx, float dt) {
    ECS& ecs = ctx.ecs;

    for(const auto& p : ecs.players) {
        for(auto& skill : ecs.skills[p].skills) {
            for(auto& effect : skill.builtEffects) {
                switch(effect.type) {
                    case(Defs::EffectType::SpawnProjectile): {
                        const Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), ctx.camera);
                        skill.cooldownTimer += dt;
                        if(!(skill.cooldownTimer >= (1.f / effect.value1))) continue;

                        // reset cooldown
                        skill.cooldownTimer = 0.f;

                        // fire parameters
                        const Vector2 playerPos  = ecs.transforms[p].position;
                        const Vector2 dir        = normalize(mouseWorld - playerPos);

                        auto& def = skill.def;

                        Entity proj = ecs.create();
                        ecs.tags[proj] = {
                            .hasPlayer      = false,
                            .hasEnemy       = false,
                            .hasWeapon      = true,
                            .hasContainment = true,
                            .hasProjectile  = true,
                        };
                        ecs.sprites[proj]             = {DungeonSprites::sprites[def->sprite], def->scale};
                        ecs.transforms[proj].position = playerPos;
                        ecs.transforms[proj].angleD   = 0;
                        ecs.velocities[proj].value    = dir * effect.value0;
                        ecs.colliders[proj].halfSize  = {
                            ecs.sprites[proj].src.width  * ecs.sprites[proj].scale / 2.f,
                            ecs.sprites[proj].src.height * ecs.sprites[proj].scale / 2.f,
                        };
                        ecs.projectiles[proj] = {
                            .owner = p,
                            .damage = skill.def->baseDamage,
                            .onHitEffects = skill.builtEffects,
                        };
                        ecs.markDestroyDelayed(proj, 2.f); // 2 second active time
                        break;
                    }
                    // on hit effects do not apply here
                    case(Defs::EffectType::WallBounce):
                    case(Defs::EffectType::DealDamage):
                    default:
                        break;
                }
            }
        }
    }
}

inline void systemEventTimer(GameContext& ctx, int fps) {
    for(Entity e = 0; e < ctx.ecs.capacity(); e++) {
        if(!ctx.ecs.isAlive(e)) continue;
        auto& timer = ctx.ecs.eventTimers[e];
        if(timer.thresholdSec < 0.01f) continue;

        if(timer.frameCount++ > timer.thresholdSec * fps)
            ctx.ecs.markForDestroy(e);

    }
}

inline void systemRenderHUD(GameContext& ctx) {
    const auto& playerHealth = ctx.ecs.healths[ctx.playerID];
    const auto  healthRatio = playerHealth.value / playerHealth.maxValue;

    char healthTxt[30];
    snprintf(healthTxt, 30, "Heath: %d/%d", (int)playerHealth.value, (int)playerHealth.maxValue);

    // draw health bar 
    DrawRectangle(0, 0, healthRatio * GetScreenWidth(), 30, GREEN);
    DrawRectangleLines(0, 0, GetScreenWidth(), 30, WHITE);
    DrawText(healthTxt,
        0 + GetScreenWidth()/2 - MeasureText(healthTxt, 28)/2,
        0 + 30/2 - 14,
        28, WHITE);
}
