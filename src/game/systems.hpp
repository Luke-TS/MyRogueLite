#pragma once

#include "core/dungeon_sprites.hpp"
#include "core/ecs.hpp"
#include "core/vec2_ops.hpp" // Vector2 overloads
#include "core/physics.hpp"

#include "game/defs.hpp"
#include "game/game.hpp"
#include "states.hpp"
#include <cstdio>
#include <iostream>
#include <raylib.h>

// angle calculation helpers
constexpr auto degreesToRadians = 3.14 / 180.f;
constexpr auto radiansToDegrees = 1 / degreesToRadians;
constexpr auto degreesPerRotation = 360;

inline void systemPlayerMovement(ECS& ecs, float speed) {
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

        ecs.velocities[p].value = moveDir * speed;
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

struct CollisionEvent {
    Entity a;
    Entity b;            // -1 is tile
    Vector2 penetration; // a into b
};

// detection — calls core/physics, produces raw events
inline void systemCollisionDetect(GameContext& ctx, std::vector<CollisionEvent>& out) {
    auto& ecs = ctx.ecs; 

    // player vs enemy detection
    for (Entity p : ecs.players)
    {
        for (Entity e : ecs.enemies)
        {
            auto pen = physics::intersectCentered(
                ecs.transforms[p].position, ecs.colliders[p].halfSize * 2.f,
                ecs.transforms[e].position, ecs.colliders[e].halfSize * 2.f
            );
            if (pen) out.push_back({p, e, *pen});
        }
    }

    // weapon vs enemy detection
    for (Entity w : ecs.weapons)
    {
        for (Entity e : ecs.enemies)
        {
            auto pen = physics::intersectCentered(
                ecs.transforms[w].position, ecs.colliders[w].halfSize * 2.f,
                ecs.transforms[e].position, ecs.colliders[e].halfSize * 2.f
            );
            if (pen) out.push_back({w, e, *pen});
        }
    }

    // enemy vs enemy detection
    for (Entity e1 : ecs.enemies)
    {
        for (Entity e2 : ecs.enemies)
        {
            if (e1 == e2) continue; // skip

            auto pen = physics::intersectCentered(
                ecs.transforms[e1].position, ecs.colliders[e1].halfSize,
                ecs.transforms[e2].position, ecs.colliders[e2].halfSize
            );

            if (pen) out.push_back({e1, e2, *pen});
        }
    }

    // entity vs tile detection
    for (Entity e = 0; e < ecs.capacity(); e++) 
    {
        if (!ecs.isAlive(e))             continue; // skip
        if (!ecs.tags[e].hasContainment) continue; // skip

        Rectangle entityRect = physics::centerToRectangle(
            ecs.transforms[e].position,
            ecs.colliders[e].halfSize * 2.f
        );

        auto tiles = getNearbySolidTiles(ctx.map, ecs.transforms[e].position);

        for (auto& tileRect : tiles) 
        {
            if (auto pen = physics::intersect(entityRect, tileRect)) {
                out.push_back({e, -1, *pen}); // -1 = tile
            }
        }
    }
}

// resolution — pure game rules, consumes events
inline void systemCollisionResolve(GameContext& ctx, std::vector<CollisionEvent>& events) {
    auto& ecs = ctx.ecs;

    for (auto& c : events) 
    {
        // entity vs tile
        if (c.b == -1) {
            if (ecs.tags[c.a].hasWallBounce) {
                auto norm = normalize(c.penetration);
                auto vin  = ecs.velocities[c.a].value;

                // Reflect velocity
                auto vout = vin - (2.f * dot(vin, norm) * norm);
                ecs.velocities[c.a].value = vout;

                // Compute angle from velocity
                float angleRad = atan2(vout.y, vout.x);
                float angleDeg = angleRad * radiansToDegrees;

                // Adjust because sprite points LEFT by default
                ecs.transforms[c.a].angleD = angleDeg + 180.f;

                continue;
            }

            if (ecs.tags[c.a].hasProjectile) { // destroy projectile
                ecs.markForDestroy(c.a);
                continue;
            }

            if (ecs.tags[c.a].hasContainment) { // contain player
                auto& pos = ecs.transforms[c.a].position;
                pos += c.penetration;
                continue;
            }
            assert(false);
        }

        // weapon vs enemy
        if (ecs.tags[c.a].hasWeapon && ecs.tags[c.b].hasEnemy) {
            // TODO: add weapon damage component
            ecs.healths[c.b].value -= 1.f; // hit = 1 hp for now

            // log hit
            // TODO: logging system?
            // std::cout << "hit:\n  entity: " << c.b << "\n  health: " << ecs.healths[c.b].value << "\n";

            // projectiles are destroyed
            // TODO: modify to support different project on-hit rules
            if(ecs.tags[c.a].hasProjectile)
                ecs.markForDestroy(c.a);

            ecs.transforms[c.b].position -=  normalize(c.penetration) * 20.f;
            
            continue;
        }

        // enemy vs enemy
        // pushes back only 25% of colider penetration
        // to avoid completely overlapping enemies
        if (ecs.tags[c.a].hasEnemy && ecs.tags[c.b].hasEnemy) {
            c.penetration /= 4.f;
            ecs.transforms[c.a].position += c.penetration;
            ecs.transforms[c.b].position -= c.penetration;
            continue;
        }

        // enemy vs player
        if (ecs.tags[c.a].hasPlayer && ecs.tags[c.b].hasEnemy) {
            auto penNomr = normalize(c.penetration);
            ecs.transforms[c.b].position -= penNomr * 25.f;
            ecs.healths[c.a].value -= 20.f; 
            continue;
        }
    }
}

struct DeathEvent {
    Entity e;
};

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

inline void systemBowFire(GameContext& ctx) {
    ECS&   ecs = ctx.ecs;
    float  dt  = GetFrameTime();

    const Defs::WeaponDef* bowDef = &Defs::weapons[Defs::WEAPON_BOW];

    // check if player has bow unlocked
    bool hasBow = false;
    for (int i : ctx.progress.unlockedWeapons)
        if (i == Defs::WeaponIdx::WEAPON_BOW) { hasBow = true; break; }
    if (!hasBow) return;

    // accumulate time and fire on interval
    ctx.progress.bowCooldown += dt;
    float fireInterval = 1.f / bowDef->projParams.fireRate;
    if (ctx.progress.bowCooldown < fireInterval) return;
    ctx.progress.bowCooldown = 0.f;

    // fire
    const Vector2 playerPos  = ecs.transforms[ctx.playerID].position;
    const Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), ctx.camera);
    const Vector2 dir        = normalize(mouseWorld - playerPos);

    // compute sprite angle — arrow sprite points right at 180 degrees
    float angleD = 180.f + atan2f(dir.y, dir.x) * radiansToDegrees;

    Entity arrow = ecs.create();
    ecs.tags[arrow] = {
        .hasPlayer      = false,
        .hasEnemy       = false,
        .hasWeapon      = true,
        .hasContainment = true,
        .hasProjectile  = false,
        .hasWallBounce  = true,
    };
    ecs.sprites[arrow]             = {DungeonSprites::sprites[DungeonSprites::ARROW], bowDef->scale};
    ecs.transforms[arrow].position = playerPos;
    ecs.transforms[arrow].angleD   = angleD;
    ecs.velocities[arrow].value    = dir * bowDef->projParams.speed;
    ecs.colliders[arrow].halfSize  = {
        ecs.sprites[arrow].src.width  * ecs.sprites[arrow].scale / 2.f,
        ecs.sprites[arrow].src.height * ecs.sprites[arrow].scale / 2.f,
    };
    ecs.markDestroyDelayed(arrow, 2.f); // 3 second active time
}

inline void systemEventTimer(GameContext& ctx, int fps) {
    for(Entity e = 0; e < ctx.ecs.capacity(); e++) {
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
