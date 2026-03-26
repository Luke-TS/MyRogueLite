#include <cstdlib>
#include <ctime>
#include <optional>
#include <raylib.h>
#include <string>
#include <vector>

// header for obaining sprites 
// from dungeon_tileset.png
#include "dungeon_tileset.hpp"

// operator overloads for raylib's Vector2
// helper functions length() and normalize()
#include "vec2_ops.hpp"

// checking intersections and such
#include "collision_logic.hpp"

const float speed = 300.0f;
const int currentFPS = 120;
const int screenWidth = 1200;
const int screenHeight = 800;

using Entity = int;

const Entity playerID = 0;
const Entity enemyID  = 1;
const Entity weaponID = 2;
const int entityCount = 3;

// reads WASD input and returns
// normalized direction vector
Vector2 GetWASDMovement();
Vector2 getSize(Entity e, Rectangle* sprites, float* scales);

typedef struct {
    Entity target;
    float radius;
    float angle;
    float speed;
} Orbit;

int main() {
    srand(time(NULL));

    // screen setup
    InitWindow(screenWidth, screenHeight, "MyRogueLite");

    const Texture2D tilesetTexture = LoadTexture(DungeonTileSet::texturePath.c_str());

    // tile position
    const int tileStartX = 200;
    const int tileStartY = 100;
    const int tileWidth  = 800;
    const int tileHeight = 600;
    Rectangle tile = {tileStartX, tileStartY, tileWidth, tileHeight};
    Vector2 tileCenter = {
        tileStartX + tileWidth  / 2.f,
        tileStartY + tileHeight / 2.f
    };

    // random tile sprite
    Sprite tileSprite = DungeonTileSet::floorTileStart;
    tileSprite.x += (rand() % 20) * DungeonTileSet::gridSquareSize;
    tileSprite.y += (rand() % 2)  * DungeonTileSet::gridSquareSize;

    // render component
    Rectangle sprites[entityCount];
    float     scales[entityCount] = {1.f};

    sprites[playerID]    = DungeonTileSet::characterStart;
    sprites[playerID].x += 17 * DungeonTileSet::gridSquareSize; // 17th character
    scales[playerID] = 4.f;
    sprites[enemyID] = DungeonTileSet::monsterStart;
    scales[enemyID] = 4.f;
    sprites[weaponID] = DungeonTileSet::weaponStart;
    sprites[weaponID].y += 10;
    sprites[weaponID].height -= 10;
    scales[weaponID] = 4.f;

    // transform component
    Vector2   positions[entityCount];

    positions[playerID] = tileCenter;
    positions[enemyID] =  tileCenter / 3.f;

    float     rotations[entityCount] = {0.f};  // in degrees
    float     spinSpeeds[entityCount] = {0.f}; // degrees/second
    spinSpeeds[weaponID] = currentFPS * 3.f; // one spin per second

    // WASD movement component
    bool isWASD[entityCount] = {false};
    isWASD[playerID] = true;

    // AI movement component
    bool isAI[entityCount] = {false};
    isAI[enemyID] = true;

    // health component
    bool isLiving[entityCount] = {false};
    float health[entityCount] = {0.f};

    isLiving[playerID] = true;
    health[playerID] = 100.f;
    isLiving[enemyID] = true;
    health[enemyID] = 100.f;

    // orbit component
    std::vector<Orbit> orbits;
    std::vector<Entity> orbitEntities;

    orbits.push_back({
        .target = playerID,
        .radius = 128.f,
        .angle  = 0.f,
        .speed  = 1.5f
    });
    orbitEntities.push_back(weaponID);


    Camera2D camera = { 0 };
    camera.target = positions[playerID];
    camera.offset = Vector2{screenWidth, screenHeight} / 2.f;
    camera.rotation = 0.f;
    camera.zoom = 1.f;

    SetTargetFPS(currentFPS);
    while (!WindowShouldClose()) { // close button or ESC key

        // time since last frame render
        float deltaTime = GetFrameTime();

        // WASD movement system
        Vector2 moveDir = GetWASDMovement();
        for(Entity e = 0; e < entityCount; e++) {
            if(isWASD[e]) {
                auto deltaPos = moveDir * deltaTime * speed;

                positions[e] += deltaPos;
            }
        }

        // enemy AI movement system
        for(Entity e = 0; e < entityCount; e++) {
            if(isAI[e]) {
                Vector2 enemyToPlayer = positions[playerID] - positions[e];

                positions[e] += enemyToPlayer / 500.f;
            }
        }

        // rotation system
        for(Entity e = 0; e < entityCount; e++) {
            rotations[e] += spinSpeeds[e] * deltaTime;
        }

        // orbit system
        for (int i = 0; i < orbits.size(); i++) {
            Entity e = orbitEntities[i];
            Orbit& o = orbits[i];

            // update angle
            o.angle += deltaTime * o.speed;

            // get target position (player)
            Vector2 center = positions[o.target];

            // compute offset
            Vector2 offset = {
                std::cos(o.angle) * o.radius,
                std::sin(o.angle) * o.radius
            };

            // set weapon position
            positions[e] = center + offset;
        }

        // clamp player to tile
        Vector2 half = getSize(playerID, sprites, scales) / 2.f;
        positions[playerID].x = std::clamp(
            positions[playerID].x,
            tileStartX + half.x,
            tileStartX + tileWidth - half.x
        );
        positions[playerID].y = std::clamp(
            positions[playerID].y,
            tileStartY + half.y,
            tileStartY + tileHeight - half.y
        );

        auto sizeEnemy = getSize(enemyID, sprites, scales);
        auto sizeWeapon = getSize(weaponID, sprites, scales);

        // check axe collision with enemy
        if (rectIntersectionCentered(
                positions[enemyID], sizeEnemy.x, sizeEnemy.y, 
                positions[weaponID], sizeWeapon.x, sizeWeapon.y))
        {
            health[enemyID] = 0.f;
        }

        // update camera target
        camera.target = positions[playerID];

        BeginDrawing();
        {
            ClearBackground(BLACK);

            BeginMode2D(camera);
            {
                // draw tile first
                DrawTexturePro(tilesetTexture, tileSprite, tile, {0.f, 0.f}, 0.f, WHITE);

                // render system
                for (Entity e = 0; e < entityCount; e++) {
                    if (isLiving[e] && (health[e] == 0)) continue;

                    Vector2 destSize = getSize(e, sprites, scales);
                    Rectangle destRec = {
                        positions[e].x,
                        positions[e].y,
                        destSize.x,
                        destSize.y,
                    };
                    Vector2 origin = {
                        destRec.width / 2.f,
                        destRec.height / 2.f,
                    };
                    DrawTexturePro(
                        tilesetTexture,
                        sprites[e],
                        destRec,
                        origin,
                        rotations[e],
                        WHITE
                    );
                }
            }
            EndMode2D();
        }
        EndDrawing();
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

Vector2 getSize(Entity e, Rectangle* sprites, float* scales) {
    return {
        sprites[e].width  * scales[e],
        sprites[e].height * scales[e]
    };
}
