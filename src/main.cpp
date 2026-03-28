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

struct Sprite {
    Rectangle src;
    float scale;
};

typedef struct _hc {
    float value;
} HealthComp;

typedef struct _tc {
    bool hasPlayer      = false;
    bool hasEnemy       = false;
    bool hasWeapon      = false;
    bool hasContainment = true; // contained by map boundaries
                                // true by default
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

    // transform component

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

    // sprite component
    std::vector<Sprite> sprites(entityCount);

    sprites[playerID].src    = DungeonTileSet::characterStart;
    sprites[playerID].src.x += 17 * DungeonTileSet::gridSquareSize; // 17th character
    sprites[playerID].scale = 4.f;

    sprites[enemyID].src = DungeonTileSet::monsterStart;
    sprites[enemyID].scale= 4.f;

    sprites[weaponID].src = DungeonTileSet::weaponStart;
    sprites[weaponID].src.y += 10;
    sprites[weaponID].src.height -= 10;
    sprites[weaponID].scale = 4.f;
    sprites[weapon2ID].src = DungeonTileSet::weaponStart;
    sprites[weapon2ID].src.y += 10;
    sprites[weapon2ID].src.height -= 10;
    sprites[weapon2ID].scale = 4.f;
    sprites[weapon3ID].src = DungeonTileSet::weaponStart;
    sprites[weapon3ID].src.y += 10;
    sprites[weapon3ID].src.height -= 10;
    sprites[weapon3ID].scale = 4.f;

    // collider component; equal to sprite size*scale by default
    std::vector<ColliderComp> colliders(entityCount);
    for(Entity e = 0; e < entityCount; e++) {
        colliders[e].halfSize = {
            sprites[e].src.width * sprites[e].scale / 2.f,
            sprites[e].src.height * sprites[e].scale / 2.f,
        };
    }

    // direction component

    std::vector<int> direction(entityCount, 1);

    // health component

    std::vector<HealthComp> healths(entityCount);
    healths[playerID].value = 100.f;
    healths[enemyID].value = 100.f;

    // tags component

    std::vector<TagComp> tags(entityCount);
    tags[playerID] = {
        .hasPlayer = true,
    };
    tags[enemyID] = {
        .hasEnemy = true,
    };
    tags[weaponID] = {
        .hasWeapon = true,
    };
    tags[weapon2ID] = {
        .hasWeapon = true,
    };
    tags[weapon3ID] = {
        .hasWeapon = true,
    };

    // orbit component

    std::vector<Orbit> orbits;
    std::vector<Entity> orbitEntities;

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

    // camera setup

    Camera2D camera;
    camera.offset = Vector2{screenWidth, screenHeight} / 2.f; // defines {0, 0} as mid-screen
    camera.rotation = 0.f;
    camera.zoom = 1.f;

    // game state

    bool debugMode = false;

    // derived data - cleared each frame

    struct Collision {
        Entity a;
        Entity b;
        Vector2 penetration; // a into b
    };
    std::vector<Collision> collisions;

    // populated from tag components each frame
    // inefficient, but good enough for now
    std::vector<Entity> playerEntities;
    std::vector<Entity> weaponEntities;
    std::vector<Entity> enemyEntities;

    // main loop

    SetTargetFPS(currentFPS);
    while (!WindowShouldClose()) { // close button or ESC key

        // game logic (inputs etc)

        float deltaTime = GetFrameTime();

        if(IsKeyPressed(KEY_B)) {
            debugMode = 1 - debugMode;
        }

        if(IsKeyPressed(KEY_R)) {
            healths[enemyID].value = 100.f;
            transforms[enemyID].rotationThetaD = 0.f;
        }

        if(IsKeyPressed(KEY_LEFT))
            sprites[playerID].src.x -= DungeonTileSet::gridSquareSize;
        if(IsKeyPressed(KEY_RIGHT))
            sprites[playerID].src.y += DungeonTileSet::gridSquareSize;

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

        // segregate entities
        playerEntities.clear();
        weaponEntities.clear();
        enemyEntities.clear();
        for(Entity e = 0; e < entityCount; e++) {
            const auto& eTags = tags[e];
            if(eTags.hasPlayer)
                playerEntities.push_back(e);
            if(eTags.hasEnemy)
                enemyEntities.push_back(e);
            if(eTags.hasWeapon)
                weaponEntities.push_back(e);
        }

        // movement systems

        // WASD movement system
        Vector2 moveDir = GetWASDMovement();
        for(const auto& p : playerEntities) {
            if( moveDir.x < -0.1f)
                direction[p] = -1;
            if( moveDir.x > +0.1f)
                direction[p] = 1;

            auto deltaPos = moveDir * deltaTime * speed;
            auto newPos = transforms[p].position + deltaPos;

            auto& half = colliders[p].halfSize;

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

            transforms[p].position = newPos;
        }

        // enemy AI movement system
        for(const auto& e : enemyEntities) {
            if(healths[e].value == 0) continue; // dead enemies don't move :)

            // TODO: remove hard-coded playerID
            Vector2 enemyToPlayer = transforms[playerID].position - transforms[e].position;

            transforms[e].position += enemyToPlayer / 500.f;

            if( enemyToPlayer.x < 0)
                direction[e] = -1;
            else
                direction[e] = 1;
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

        // collision detection

        collisions.clear();

        // weapon + enemy collisions
        for (Entity w : weaponEntities) {
            for (Entity e : enemyEntities) {
                Rectangle rw = collision::fromCenter(transforms[w].position, colliders[w].halfSize * 2.f);
                Rectangle re = collision::fromCenter(transforms[e].position, colliders[e].halfSize * 2.f);

                auto pen = collision::intersect(rw, re);
                if (pen) {
                    collisions.push_back({w, e, *pen});
                }
            }
        }
        for (auto& c : collisions) {
            if (tags[c.a].hasWeapon && tags[c.b].hasEnemy) {
                healths[c.b].value = 0.f;
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

                    Rectangle src = sprites[e].src; // copy

                    // apply horizontal mirror based off direction
                    if ((tags[e].hasPlayer || tags[e].hasEnemy))
                        src.width *= direction[e];

                    Vector2 renderSize = {
                        .x = sprites[e].src.width * sprites[e].scale,
                        .y = sprites[e].src.height * sprites[e].scale,
                    };
                    Rectangle destRec = {
                        transforms[e].position.x,
                        transforms[e].position.y,
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
                        transforms[e].rotationThetaD,
                        WHITE
                    );

                    // DEBUG: draw bounding box
                    
                    if(!debugMode) continue;

                    Rectangle debugRect = {
                        transforms[e].position.x - renderSize.x / 2.f,
                        transforms[e].position.y - renderSize.y / 2.f,
                        renderSize.x,
                        renderSize.y
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
