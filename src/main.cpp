#include <cstdlib>
#include <ctime>
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
const Entity weapon2ID = 3;
const Entity weapon3ID = 4;
const int entityCount = 5;

// reads WASD input and returns
// normalized direction vector
Vector2 GetWASDMovement();

Vector2 getSize(Entity e, Rectangle* sprites, float* scales);

struct TileMap {
    int width, height;
    float tileSize;

    std::vector<Sprite> tiles; // indices into tileset
};
Sprite getTile(const TileMap& map, int x, int y) {
    return map.tiles[y * map.width + x];
};

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

    const Texture2D dungeonTextureSet = LoadTexture(DungeonTileSet::texturePath.c_str());

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

    TileMap map = {
        .width  = 4,
        .height = 4,
        .tileSize = 600.f,
    };
    for(int i = 0; i < map.width*map.height; i++)
        map.tiles.push_back(DungeonTileSet::randomFloorTile());

    // random tile sprite
    Sprite tileSprite = DungeonTileSet::randomFloorTile();

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
    sprites[weapon2ID] = DungeonTileSet::weaponStart;
    sprites[weapon2ID].y += 10;
    sprites[weapon2ID].height -= 10;
    scales[weapon2ID] = 4.f;
    sprites[weapon3ID] = DungeonTileSet::weaponStart;
    sprites[weapon3ID].y += 10;
    sprites[weapon3ID].height -= 10;
    scales[weapon3ID] = 4.f;

    // transform component
    Vector2   positions[entityCount];

    positions[playerID] = tileCenter;
    positions[enemyID] =  tileCenter / 2.f;

    float     rotations[entityCount] = {0.f};  // in degrees
    float     spinSpeeds[entityCount] = {0.f}; // degrees/second
    spinSpeeds[weaponID] = currentFPS * 5.f; // 5 spin per second
    spinSpeeds[weapon2ID] = currentFPS * 5.f;
    spinSpeeds[weapon3ID] = currentFPS * 5.f;

    bool isPlayer[entityCount] = {false};
    isPlayer[playerID] = true;

    bool isEnemy[entityCount] = {false};
    isEnemy[enemyID] = true;

    // health component (players and enemies)
    float health[entityCount] = {0.f};

    health[playerID] = 100.f;
    health[enemyID] = 100.f;

    // orbit component
    std::vector<Orbit> orbits;
    std::vector<Entity> orbitEntities;

    bool isWeapon[entityCount] = {false};
    isWeapon[weaponID] = true;
    isWeapon[weapon2ID] = true;
    isWeapon[weapon3ID] = true;

    orbits.push_back({
        .target = playerID,
        .radius = 128.f,
        .angle  = 0.f,
        .speed  = 2.5f,
    });
    orbitEntities.push_back(weaponID);
    orbits.push_back({
        .target = playerID,
        .radius = 128.f,
        .angle  = (2.f/3.f) * 3.14f,
        .speed  = 2.5f,
    });
    orbitEntities.push_back(weapon2ID);
    orbits.push_back({
        .target = playerID,
        .radius = 128.f,
        .angle  = (4.f/3.f) * 3.14f,
        .speed  = 2.5f,
    });
    orbitEntities.push_back(weapon3ID);

    Camera2D camera = { 0 };
    camera.target = positions[playerID];
    camera.offset = Vector2{screenWidth, screenHeight} / 2.f;
    camera.rotation = 0.f;
    camera.zoom = 1.f;

    bool debugMode = false;

    SetTargetFPS(currentFPS);
    while (!WindowShouldClose()) { // close button or ESC key

        // time since last frame render
        float deltaTime = GetFrameTime();

        if(IsKeyPressed(KEY_B)) {
            debugMode = 1 - debugMode;
        }

        if(IsKeyPressed(KEY_R)) {
            health[enemyID] = 100.f;
            rotations[enemyID] = 0.f;
        }

        if(IsKeyPressed(KEY_LEFT))
            sprites[playerID].x -= DungeonTileSet::gridSquareSize;
        if(IsKeyPressed(KEY_RIGHT))
            sprites[playerID].y += DungeonTileSet::gridSquareSize;

        if(IsKeyPressed(KEY_T)) {
            map.tiles.clear();
            for(int i = 0; i < map.width*map.height; i++)
                map.tiles.push_back(DungeonTileSet::randomFloorTile());
        }

        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        // WASD movement system
        Vector2 moveDir = GetWASDMovement();
        for(Entity e = 0; e < entityCount; e++) {
            if(isPlayer[e]) {
                auto deltaPos = moveDir * deltaTime * speed;

                positions[e] += deltaPos;
            }
        }

        // enemy AI movement system
        for(Entity e = 0; e < entityCount; e++) {
            if(isEnemy[e]) {
                if(health[e] == 0) continue;

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

            // get target position
            Vector2 center = positions[o.target];

            // compute offset
            Vector2 offset = {
                std::cos(o.angle) * o.radius,
                std::sin(o.angle) * o.radius
            };

            // set entity position
            positions[e] = center + offset;
        }

        // clamp player to tile
        Vector2 half = getSize(playerID, sprites, scales) / 2.f;
        positions[playerID].x = std::clamp(
            positions[playerID].x,
            0 + half.x,
            0 + map.width*map.tileSize - half.x
        );
        positions[playerID].y = std::clamp(
            positions[playerID].y,
            0 + half.y,
            0 + map.height*map.tileSize - half.y
        );

        /* TODO
        // convert world → tile coordinates
        int tx = positions[playerID].x / map.tileSize;
        int ty = positions[playerID].y / map.tileSize;

        // check surrounding tiles
        for (each nearby tile) {
            if (tile is solid) {
                resolve collision
            }
        }
        */

        // check weapon collisions with enemies
        for(Entity e_i = 0; e_i < entityCount; e_i++) {
            if(isWeapon[e_i]) {
                for(Entity e_j = 0; e_j < entityCount; e_j++) {
                    if(e_i == e_j) continue;

                    if(isEnemy[e_j]) {
                        auto sizeWeapon = getSize(e_i, sprites, scales);
                        auto sizeEnemy = getSize(e_j, sprites, scales);

                        if (rectIntersectionCentered(
                            positions[e_j], sizeEnemy.x, sizeEnemy.y, 
                            positions[e_i], sizeWeapon.x, sizeWeapon.y))
                        {
                            health[e_j] = 0.f;
                            rotations[e_j] = 90.f;
                        }
                    }
                }
            }
        }

        // update camera target
        camera.target = positions[playerID];

        BeginDrawing();
        {
            ClearBackground(BLACK);

            BeginMode2D(camera);
            {
                // draw tile first
                //DrawTexturePro(tilesetTexture, tileSprite, tile, {0.f, 0.f}, 0.f, WHITE);
                for (int y = 0; y < map.height; y++) {
                    for (int x = 0; x < map.width; x++) {
                        Rectangle src = getTile(map, x, y);

                        Rectangle dest = {
                            x * map.tileSize,
                            y * map.tileSize,
                            map.tileSize,
                            map.tileSize
                        };

                        DrawTexturePro(dungeonTextureSet, src, dest, {0,0}, 0, WHITE);
                    }
                }

                // render system
                for (Entity e = 0; e < entityCount; e++) {
                    //if ((isPlayer[e] || isEnemy[e]) && (health[e] == 0)) continue;

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
                        dungeonTextureSet,
                        sprites[e],
                        destRec,
                        origin,
                        rotations[e],
                        WHITE
                    );

                    // DEBUG: draw bounding box
                    
                    if(!debugMode) continue;

                    Rectangle debugRect = {
                        positions[e].x - destSize.x / 2.f,
                        positions[e].y - destSize.y / 2.f,
                        destSize.x,
                        destSize.y
                    };

                    DrawRectangleLinesEx(debugRect, 2.0f, GREEN);
                    DrawCircleV(positions[e], 3.0f, RED);
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
