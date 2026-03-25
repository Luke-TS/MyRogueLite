#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
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

// reads WASD input and returns
// normalized direction vector
Vector2 GetWASDMovement();

struct Player {
    const int   maxHistory;

    std::vector<Vector2> positionHistory;
};

struct Enemy {
    bool isAlive;
};

using Entity = int;

const Entity playerID = 0;
const Entity enemyID  = 1;
const Entity weaponID = 2;
const int entityCount = 3;

int main() {
    srand(time(NULL));

    // screen setup
    InitWindow(screenWidth, screenHeight, "MyRogueLite");

    const Texture2D tilesetTexture = LoadTexture(DungeonTileSet::texturePath.c_str());

    // render component
    Rectangle sprites[entityCount];
    float     scales[entityCount] = {1.f};

    // transform component
    Vector2   positions[entityCount];

    // WASD movement component
    bool isWASD[entityCount] = {false};

    // AI movement component
    bool isAI[entityCount] = {false};

    // player
    sprites[playerID]    = DungeonTileSet::characterStart;
    sprites[playerID].x += 17 * DungeonTileSet::gridSquareSize; // 17th character
    scales[playerID] = 4.f;
    positions[playerID] = Vector2{screenWidth, screenHeight} / 2.f;
    isWASD[playerID] = true;
    
    // sprites[enemy]
    sprites[enemyID] = DungeonTileSet::monsterStart;
    scales[enemyID] = 4.f;
    positions[enemyID] = Vector2{screenWidth, screenHeight} / 4.f;
    isAI[enemyID] = true;

    // weapon sprite
    sprites[weaponID] = DungeonTileSet::weaponStart;
    scales[weaponID] = 4.f;
    isWASD[weaponID] = true;

    // weapon starts 128 pixels right of player
    positions[weaponID] = positions[playerID];
    positions[weaponID].x += 128;

    // Player struct
    struct Player player = {
        .maxHistory = static_cast<int>(currentFPS / 8.f), // 0.125 second of history
    };

    // Enemy struct
    struct Enemy enemy = {
        .isAlive = true,
    };

    // rotating axe around character
    const float axeRadius = 128;
    float axeTheta = 0.f;
    float axeToCharTheta = 0.f;

    // tile position
    const int tileStartX = 200;
    const int tileStartY = 100;
    const int tileWidth  = 800;
    const int tileHeight = 600;
    Rectangle tile = {tileStartX, tileStartY, tileWidth, tileHeight};

    // random tile sprite
    Sprite tileSprite = DungeonTileSet::floorTileStart;
    tileSprite.x += (rand() % 20) * DungeonTileSet::gridSquareSize;
    tileSprite.y += (rand() % 2)  * DungeonTileSet::gridSquareSize;

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

        // axe movement
        axeTheta += deltaTime * 8.f;
        axeToCharTheta += deltaTime * 1.5f;

        // entity bounding rectangles
        // used for intersection and rendering 
        Rectangle playerRec = {
            .x = positions[playerID].x - (sprites[playerID].width * scales[playerID])/2.f,
            .y = positions[playerID].y - (sprites[playerID].height * scales[playerID])/2.f,
            .width = (float)sprites[playerID].width * scales[playerID],
            .height = (float)sprites[playerID].height * scales[playerID],
        };
        Rectangle enemyRec = {
            .x = positions[enemyID].x - (sprites[enemyID].width*scales[playerID])/2.f,
            .y = positions[enemyID].y - (sprites[enemyID].height*scales[playerID])/2.f,
            .width = (float)sprites[enemyID].width * scales[playerID],
            .height = (float)sprites[enemyID].height * scales[playerID],
        };
        Rectangle axeRec = {
            positions[playerID].x + std::cos(axeToCharTheta) * axeRadius,
            positions[playerID].y + std::sin(axeToCharTheta) * axeRadius,
            (float)sprites[weaponID].width * scales[weaponID],
            (float)sprites[weaponID].height * scales[weaponID],
        };

        // snap player inside tile
        if (auto p = boundsPenetration( playerRec, {tileStartX, tileStartY, tileWidth, tileHeight})) {
            positions[playerID] -= *p;
        }

        // check axe collision with enemy
        if (rectIntersection(enemyRec, axeRec)) {
            enemy.isAlive = false;
        }

        // manage character history
        if (player.positionHistory.size() > player.maxHistory) {
            player.positionHistory.erase(player.positionHistory.begin());
        }
        player.positionHistory.push_back(positions[playerID]);

        // update camera target
        camera.target = positions[playerID];

        BeginDrawing();
        {
            ClearBackground(BLACK);

            BeginMode2D(camera);
            {
                // draw tile first
                DrawTexturePro(tilesetTexture, tileSprite, tile, {0.f, 0.f}, 0.f, WHITE);

                // faded player history trail including
                // the current, fully opaque positions[playerID]
                int count = player.positionHistory.size();
                for(int i = 0; i < count; i++) {
                    float t = (float)i / count;

                    Color c = WHITE;
                    c.a = (unsigned char)(255 * t * t); // fade in

                    Rectangle destRec = {
                        player.positionHistory[i].x,
                        player.positionHistory[i].y,
                        (float)sprites[playerID].width * scales[playerID],
                        (float)sprites[playerID].height * scales[playerID],
                    };
                    Vector2 origin = {
                        (sprites[playerID].width * scales[playerID]) / 2.f,
                        (sprites[playerID].height * scales[playerID]) / 2.f,
                    };
                    DrawTexturePro(tilesetTexture, sprites[playerID], destRec, origin, 0.f, c);
                }

                // draw enemy
                if(enemy.isAlive) {
                    DrawTexturePro(tilesetTexture, sprites[enemyID], enemyRec, {0, 0}, 0.f, WHITE);
                }

                Vector2 axeOrigin = {
                    (sprites[weaponID].width * scales[weaponID]) / 2.f,
                    (sprites[weaponID].height * scales[weaponID]) / 2.f + 4.f * scales[weaponID],
                };
                DrawTexturePro(
                    tilesetTexture,
                    sprites[weaponID],
                    axeRec,
                    axeOrigin,
                    axeTheta * 180.f / 3.14f,
                    WHITE
                );
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
