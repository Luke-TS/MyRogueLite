#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
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

    std::vector<int> tiles; // indices into tileset
};
std::optional<int> getTile(const TileMap& map, int x, int y) {
    if(x < 0 || x > map.width-1) return std::nullopt;
    if(y < 0 || y > map.height-1) return std::nullopt;
    return map.tiles[y * map.width + x];
};
std::vector<std::optional<int>> getNeighbors(const TileMap& map, int x, int y) {
    std::vector<std::optional<int>> tiles(4);
    tiles[0] = getTile(map, x, y-1); // top
    tiles[1] = getTile(map, x+1, y); // right
    tiles[2] = getTile(map, x, y+1); // bottom
    tiles[3] = getTile(map, x-1, y); // right
    return tiles;
}

typedef struct _tfc {
    Vector2 position;
    float   rotationThetaD; // degrees
    float   rotationSpeedD; // degrees per sec
                            // fps * 5 = 5 rotations per sec
} TransformComp;

typedef struct _cc {
    Vector2 halfSize;
} ColliderComp;

typedef struct _hc {
    float value;
} HealthComp;

typedef struct _tc {
    bool hasPlayer = false;
    bool hasEnemy  = false;
    bool hasWeapon = false;
} TagComp;

typedef struct _o {
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

    /*
    // tile position
    const int tileStartX = 200;
    const int tileStartY = 100;
    const int tileWidth  = 800;
    const int tileHeight = 600;
    Rectangle tile = {tileStartX, tileStartY, tileWidth, tileHeight};
    */

    TileMap map;
    map.tileSize = 600.f;

    std::ifstream file("../src/map.txt");
    file >> map.width >> map.height;
    for(int i = 0; i < map.height; i++) {
        for(int j = 0; j < map.width; j++) {
            bool isTile;
            file >> isTile;
            if(isTile)
                map.tiles.push_back(DungeonTileSet::randomFloorTileIdx());
            else
                map.tiles.push_back(-1);
        }
    }

    Vector2 tileCenter = {
        map.width * map.tileSize / 2.f,
        map.height * map.tileSize / 2.f
    };

    std::vector<TransformComp> transforms(entityCount);

    transforms[playerID] = {
        .position = tileCenter,
        .rotationThetaD = 0.f,
    };
    transforms[enemyID] = {
        .position = tileCenter,
        .rotationThetaD = 0.f,
    };
    transforms[weaponID] = {
        .rotationThetaD = 0.f,
        .rotationSpeedD = currentFPS * 5.f,
    };
    transforms[weapon2ID] = {
        .rotationThetaD = 0.f,
        .rotationSpeedD = currentFPS * 5.f,
    };
    transforms[weapon3ID] = {
        .rotationThetaD = 0.f,
        .rotationSpeedD = currentFPS * 5.f,
    };

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

    std::vector<int> direction(entityCount, 1);

    std::vector<bool> isContained(entityCount, false);
    isContained[playerID] = true;
    isContained[enemyID] = true;

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
    camera.target = tileCenter;
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
            transforms[enemyID].rotationThetaD = 0.f;
        }

        if(IsKeyPressed(KEY_LEFT))
            sprites[playerID].x -= DungeonTileSet::gridSquareSize;
        if(IsKeyPressed(KEY_RIGHT))
            sprites[playerID].y += DungeonTileSet::gridSquareSize;

        if(IsKeyPressed(KEY_T)) {
            map.tiles.clear();
            for(int i = 0; i < map.width*map.height; i++)
                map.tiles.push_back(DungeonTileSet::randomFloorTileIdx());
        }

        // Camera zoom controls
        // Uses log scaling to provide consistent zoom speed
        camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove()*0.1f));
        if(IsKeyPressed(KEY_I))
            camera.zoom += 0.2f;
        if(IsKeyPressed(KEY_O))
            camera.zoom -= 0.2f;

        if (camera.zoom > 4.0f) camera.zoom = 4.0f;
        else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        // WASD movement system
        Vector2 moveDir = GetWASDMovement();
        for(Entity e = 0; e < entityCount; e++) {
            if(isPlayer[e]) {
                if( moveDir.x < -0.1f)
                    direction[e] = -1;
                if( moveDir.x > +0.1f)
                    direction[e] = 1;

                auto deltaPos = moveDir * deltaTime * speed;
                auto newPos = transforms[e].position + deltaPos;

                auto eSize = getSize(e, sprites, scales);
                auto half = eSize / 2.f;

                auto neighbors = getNeighbors(map, int(newPos.x / map.tileSize), int(newPos.y / map.tileSize)); 
                auto currTile = getTile(map, int(newPos.x / map.tileSize), int(newPos.y / map.tileSize));
                assert(currTile != -1); 

                Rectangle tileDest = {
                    .x = int(newPos.x / map.tileSize) * map.tileSize,
                    .y = int(newPos.y / map.tileSize) * map.tileSize,
                    .width = map.tileSize,
                    .height = map.tileSize,
                };

                if(!neighbors[0] || neighbors[0] == -1) { // top neighbor
                    if(newPos.y - half.y < tileDest.y)
                        newPos.y = tileDest.y + half.y;
                }
                if(!neighbors[1] || neighbors[1] == -1) { // right neighbor
                    if(newPos.x + half.x > tileDest.x + tileDest.width)
                        newPos.x = tileDest.x + tileDest.width - half.x;
                }
                if(!neighbors[2] || neighbors[2] == -1) { // bottom neighbor
                    if(newPos.y + half.y > tileDest.y + tileDest.height)
                        newPos.y = tileDest.y + tileDest.height - half.y;
                }
                if(!neighbors[3] || neighbors[3] == -1) { // left neighbor
                    if(newPos.x - half.x < tileDest.x)
                        newPos.x = tileDest.x + half.x;
                }

                transforms[e].position = newPos;
            }
        }

        // enemy AI movement system
        for(Entity e = 0; e < entityCount; e++) {
            if(isEnemy[e]) {
                if(health[e] == 0) continue;

                // TODO: remove hard-coded playerID
                Vector2 enemyToPlayer = transforms[playerID].position - transforms[e].position;

                transforms[e].position += enemyToPlayer / 500.f;

                if( enemyToPlayer.x < 0)
                    direction[e] = -1;
                else
                    direction[e] = 1;
            }
        }

        // rotation system
        for(Entity e = 0; e < entityCount; e++) {
            auto& eTran = transforms[e];
            eTran.rotationThetaD += eTran.rotationSpeedD * deltaTime;
        }

        // orbit system
        for (int i = 0; i < orbits.size(); i++) {
            Entity e = orbitEntities[i];
            Orbit& o = orbits[i];

            // update angle
            o.angle += deltaTime * o.speed;

            // get target position
            Vector2 center = transforms[o.target].position;

            // compute offset
            Vector2 offset = {
                std::cos(o.angle) * o.radius,
                std::sin(o.angle) * o.radius
            };

            // set entity position
            transforms[e].position = center + offset;
        }

        // check weapon collisions with enemies
        for(Entity e_i = 0; e_i < entityCount; e_i++) {
            if(isWeapon[e_i]) {
                for(Entity e_j = 0; e_j < entityCount; e_j++) {
                    if(e_i == e_j) continue;

                    if(isEnemy[e_j]) {
                        auto sizeWeapon = getSize(e_i, sprites, scales);
                        auto sizeEnemy = getSize(e_j, sprites, scales);

                        auto& wTran = transforms[e_i];
                        auto& eTran = transforms[e_j];

                        if (collision::intersectCentered(
                            eTran.position, sizeEnemy, 
                            wTran.position, sizeWeapon))
                        {
                            health[e_j] = 0.f;
                            eTran.rotationThetaD = 90.f;
                        }
                    }
                }
            }
        }

        // update camera target
        camera.target = transforms[playerID].position;

        BeginDrawing();
        {
            ClearBackground(BLACK);

            BeginMode2D(camera);
            {
                // draw tile first
                //DrawTexturePro(tilesetTexture, tileSprite, tile, {0.f, 0.f}, 0.f, WHITE);
                for (int y = 0; y < map.height; y++) {
                    for (int x = 0; x < map.width; x++) {
                        auto src = getTile(map, x, y);

                        if(src == -1) continue; // empty map tile

                        Rectangle dest = {
                            x * map.tileSize,
                            y * map.tileSize,
                            map.tileSize,
                            map.tileSize
                        };

                        DrawTexturePro(dungeonTextureSet, DungeonTileSet::getFloorTile(*src), dest, {0,0}, 0, WHITE);
                    }
                }

                // render system
                for (Entity e = 0; e < entityCount; e++) {

                    Rectangle src = {
                        sprites[e].x,
                        sprites[e].y,
                        sprites[e].width,
                        sprites[e].height,
                    };

                    // apply horizontal mirror based off direction
                    if ((isPlayer[e] || isEnemy[e]))
                        src.width *= direction[e];

                    Vector2 destSize = getSize(e, sprites, scales);
                    Rectangle destRec = {
                        transforms[e].position.x,
                        transforms[e].position.y,
                        destSize.x,
                        destSize.y,
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
                        transforms[e].rotationThetaD,
                        WHITE
                    );

                    // DEBUG: draw bounding box
                    
                    if(!debugMode) continue;

                    Rectangle debugRect = {
                        transforms[e].position.x - destSize.x / 2.f,
                        transforms[e].position.y - destSize.y / 2.f,
                        destSize.x,
                        destSize.y
                    };

                    DrawRectangleLinesEx(debugRect, 2.0f, GREEN);
                    DrawCircleV(transforms[e].position, 3.0f, RED);
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
