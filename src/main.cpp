#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

#include <raylib.h> // goat

// custom freelist ECS for the game 
#include "ecs.hpp"

// header for obaining sprites 
// from dungeon_tileset.png
#include "dungeon_tileset.hpp"

// operator overloads for raylib's Vector2
// helper functions length() and normalize()
#include "vec2_ops.hpp"

// checking intersections and such
#include "physics.hpp"

// tile system
#include "tilemap.hpp"

constexpr int fps = 120;
constexpr float playerSpeed = 350.f;
constexpr float enemySpeed = 150.f;
constexpr float arrowSpeed = 500.f;
constexpr int screenWidth = 1200;
constexpr int screenHeight = 800;
constexpr auto tileSize = 500.f;

// angle constants
constexpr auto degreesToRadians = 3.14 / 180.f;
constexpr auto radiansToDegrees = 1 / degreesToRadians;
constexpr auto degreesPerRotation = 360;

// reads WASD input and returns
// normalized direction vector
Vector2 GetWASDMovement();

int main() {
    srand(time(NULL));

    // screen setup
    InitWindow(screenWidth, screenHeight, "MyRogueLite");

    const Texture2D dungeonTextureSet = LoadTexture(DungeonTileSet::texturePath.c_str());

    //TileMap map = loadTileMap(std::string(LEVELS_DIR)+"/asdf.txt", tileSize);
    TileMap map = createMap(tileSize, {5, 5});

    Vector2 tileCenter = {
        map.width * map.tileSize / 2.f,
        map.height * map.tileSize / 2.f
    };

    ECS ecs;

    // spawn player
    Entity player = ecs.create();
    ecs.tags[player].hasPlayer      = true;
    ecs.tags[player].hasContainment = true;
    ecs.transforms[player].position = tileCenter;
    ecs.healths[player].value       = 100.f;
    ecs.healths[player].maxValue    = 100.f;
    ecs.sprites[player]             = {DungeonTileSet::characterStart, 4.f};

    // spawn enemy
    Entity enemy = ecs.create();
    ecs.tags[enemy].hasEnemy       = true;
    ecs.tags[enemy].hasContainment = true;
    ecs.transforms[enemy].position = tileCenter;
    ecs.healths[enemy].value       = 100.f;
    ecs.sprites[enemy]             = {DungeonTileSet::monsterStart, 4.f};

    // spawn orbiting weapon
    Entity weapon1 = ecs.create();
    ecs.tags[weapon1].hasWeapon = true;
    ecs.hasOrbit[weapon1]       = true;
    ecs.transforms[weapon1].rotationSpeedD = 720.f;
    ecs.orbits[weapon1] = {
        .target     = player,
        .radius     = 128.f,
        .angleD     = 0.f,
        .rotateRate = 0.25f,
    };
    ecs.sprites[weapon1] = {DungeonTileSet::axeSprite, 4.f};

    Entity weapon2 = ecs.createCopy(weapon1);
    ecs.orbits[weapon2].angleD = 120.f;
    Entity weapon3 = ecs.createCopy(weapon1);
    ecs.orbits[weapon3].angleD = 240.f;

    /*
    Entity bow = ecs.createCopy(player);
    ecs.sprites[bow].src = DungeonTileSet::bowSprite;
    ecs.transforms[bow].angleD = 180.f;
    */

    // default collider components
    for(Entity e = 0; e < ecs.capacity(); e++) {
        ecs.colliders[e].halfSize = {
            .x = ecs.sprites[e].src.width*ecs.sprites[e].scale/2.f,
            .y = ecs.sprites[e].src.height*ecs.sprites[e].scale/2.f,
        };
    }

    // camera setup

    Camera2D camera;
    camera.offset = Vector2{screenWidth, screenHeight} / 2.f; // defines {0, 0} as mid-screen
    camera.rotation = 0.f;
    camera.zoom = 1.f;

    // game state

    bool debugMode = false;

    // derived data - cleared each frame

    struct CollisionEvent {
        Entity a;
        Entity b;
        Vector2 penetration; // a into b
    };
    std::vector<CollisionEvent> collisions;

    // main loop

    uint64_t frame_count = 0;
    SetTargetFPS(fps);
    while (!WindowShouldClose()) { // close button or ESC key

        // game logic (inputs etc)

        float deltaTime = GetFrameTime();

        if(IsKeyPressed(KEY_B)) {
            debugMode = 1 - debugMode;
        }

        // spawn enemy on cursor using key_r
        if(IsKeyPressed(KEY_R)) {
            const auto mouseScreen = GetMousePosition();
            const auto mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
            Entity enemy = ecs.create();
            ecs.tags[enemy].hasEnemy       = true;
            ecs.tags[enemy].hasContainment = true;
            ecs.transforms[enemy].position = mouseWorld;
            ecs.healths[enemy].value       = 100.f;
            ecs.sprites[enemy]             = {DungeonTileSet::monsterStart, 4.f};
            ecs.colliders[enemy].halfSize = {
                .x = ecs.sprites[enemy].src.width*ecs.sprites[enemy].scale/2.f,
                .y = ecs.sprites[enemy].src.height*ecs.sprites[enemy].scale/2.f,
            };
        }

        // fire arrow in direction of mouse
        if(frame_count % (fps / 4) == 0) { // fire 4 arrows per second
            const auto mouseScreen = GetMousePosition();
            const auto mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
            const auto projDir = normalize(mouseWorld - ecs.transforms[player].position);

            auto projAngle = 180.f; // points sprite to the right
            projAngle += atan((projDir.y/projDir.x)) * radiansToDegrees;
            if( projDir.x < 0) projAngle -= 180;

            Entity arrow = ecs.create();
            ecs.tags[arrow] = {
                .hasPlayer      = false,
                .hasEnemy       = false,
                .hasWeapon      = true,
                .hasContainment = true,
                .hasProjectile  = false,
                .hasWallBounce  = true,
            };
            ecs.sprites[arrow] = {DungeonTileSet::arrowSprite, 4.f};
            ecs.transforms[arrow] = {
                .position = ecs.transforms[player].position,
                .angleD   = static_cast<float>(projAngle),
            };
            ecs.velocities[arrow].value = projDir * arrowSpeed;
            ecs.colliders[arrow].halfSize = {
                .x = ecs.sprites[arrow].src.width*ecs.sprites[arrow].scale/2.f,
                .y = ecs.sprites[arrow].src.height*ecs.sprites[arrow].scale/2.f,
            };
        }

        // teleport player to cursor using key_t
        if(IsKeyPressed(KEY_T)) {
            const auto mouseScreen = GetMousePosition();
            const auto mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
            assert(ecs.isAlive(player));
            ecs.transforms[player].position = mouseWorld;
        }

        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
        if(IsKeyPressed(KEY_I))
            camera.zoom += 0.2f;
        if(IsKeyPressed(KEY_O))
            camera.zoom -= 0.2f;

        camera.zoom = std::clamp(camera.zoom, 0.1f, 4.f);

        // IMPORTANT: rebuils cached views
        ecs.rebuildViews();

        // INTENT SYSTEMS

        // WASD
        Vector2 moveDir = GetWASDMovement();
        for (const auto& p : ecs.players) 
        {
            if (moveDir.x < -0.1f) ecs.directions[p].value = -1;
            if (moveDir.x > +0.1f) ecs.directions[p].value = 1;

            ecs.velocities[p].value = moveDir * playerSpeed;
        }

        // enemy AI movement system
        for(const auto& e : ecs.enemies) 
        {
            if(ecs.healths[e].value == 0) { // dead enemy
                ecs.velocities[e].value = {0.f, 0.f};
                continue;
            }

            // TODO: remove hard-coded playerID
            Vector2 dir = normalize(ecs.transforms[player].position - ecs.transforms[e].position);

            ecs.velocities[e].value = dir * enemySpeed;

            if( dir.x < 0) ecs.directions[e].value = -1;
            else           ecs.directions[e].value = 1;
        }

        // INTEGRATION

        // updates transformation
        for (Entity e = 0; e < ecs.capacity(); e++) 
        {
            if (!ecs.isAlive(e)) continue;
            if (ecs.hasOrbit[e]) continue; // orbit system owns these
            auto& eTran = ecs.transforms[e];
            eTran.position += ecs.velocities[e].value * deltaTime;
            eTran.angleD += eTran.rotationSpeedD * deltaTime;
        }

        // orbit system
        for (Entity e = 0; e < ecs.capacity(); e++)
        {
            if (!ecs.isAlive(e))  continue; // skip
            if (!ecs.hasOrbit[e]) continue; // skip

            OrbitComp& o = ecs.orbits[e];

            // update angle
            o.angleD += deltaTime * o.rotateRate * degreesPerRotation;

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
            ecs.transforms[e].angleD += ecs.transforms[e].rotationSpeedD * deltaTime;
        }

        // collision detection

        collisions.clear();

        // weapon vs enemy detection
        for (Entity w : ecs.weapons)
        {
            for (Entity e : ecs.enemies)
            {
                auto pen = physics::intersectCentered(
                    ecs.transforms[w].position, ecs.colliders[w].halfSize * 2.f,
                    ecs.transforms[e].position, ecs.colliders[e].halfSize * 2.f
                );
                if (pen) collisions.push_back({w, e, *pen});
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

                if (pen) collisions.push_back({e1, e2, *pen});
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

            auto tiles = getNearbySolidTiles(map, ecs.transforms[e].position);

            for (auto& tileRect : tiles) 
            {
                if (auto pen = physics::intersect(entityRect, tileRect)) {
                    collisions.push_back({e, -1, *pen}); // -1 = tile
                }
            }
        }

        // collision resolution
        for (auto& c : collisions) 
        {
            // entity vs tile
            if (c.b == -1) {
                if (ecs.tags[c.a].hasWallBounce) {
                    auto norm   = normalize(c.penetration); // surface normal
                    auto vin = ecs.velocities[c.a].value;   // vector in
                    auto rout = vin - (2.f * dot(vin, norm) * norm);
                    ecs.velocities[c.a].value = rout;
                    auto theta = asin((dot(vin, norm) / length(vin))) * radiansToDegrees * 2.f;
                    ecs.transforms[c.a].angleD -= (vin.y < 0) ? -theta : theta;
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
                //ecs.healths[c.b].value = 0.f;
                ecs.markForDestroy(c.b);
                if(ecs.tags[c.a].hasProjectile)
                    ecs.markForDestroy(c.a);
                continue;
            }

            // enemy vs enemy
            if (ecs.tags[c.a].hasEnemy && ecs.tags[c.b].hasEnemy) {
                c.penetration /= 2.f;
                ecs.transforms[c.a].position += c.penetration / 2.f;
                ecs.transforms[c.b].position -= c.penetration / 2.f;
                continue;
            }
        }

        // update camera target
        camera.target = ecs.transforms[player].position;
        BeginDrawing();
        {
            ClearBackground(BLACK);

            BeginMode2D(camera);
            {
                // draw map
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
                            dungeonTextureSet,
                            DungeonTileSet::getFloorTile(tile->spriteIndex),
                            dest, 
                            {0,0}, 
                            0,
                            WHITE);
                    }
                }

                // render system
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
                        dungeonTextureSet,
                        src,
                        destRec,
                        origin,
                        ecs.transforms[e].angleD,
                        WHITE
                    );

                    // DEBUG: draw bounding box
                    
                    if(!debugMode) continue;

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
            EndMode2D();
        }
        EndDrawing();

        ecs.destroyPending();
        frame_count++;
    }

    CloseWindow();
    return 0;
}

Vector2 GetWASDMovement() {
    Vector2 result = {0, 0};
    if(IsKeyDown(KEY_D))
        result.x += 1; // value of 1 is arbitrary
    if(IsKeyDown(KEY_A))
        result.x -= 1;
    if(IsKeyDown(KEY_W))
        result.y -= 1;
    if(IsKeyDown(KEY_S))
        result.y += 1;

    return normalize(result);
}
