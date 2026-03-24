#include <algorithm>
#include <cmath>
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

// reads WASD input and returns
// normalized direction vector
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

int main() {
    srand(time(NULL));

    // screen setup
    const int scr_w = 1200;
    const int scr_h = 800;
    InitWindow(scr_w, scr_h, "MyRogueLite");

    const int currentFPS = 120;

    const Texture2D tilesetTexture = LoadTexture(DungeonTileSet::texturePath.c_str());

    // character position
    auto playerPos = Vector2{ scr_w, scr_h} / 2.f;
    const int maxPlayerPositionHistory = currentFPS / 8.f; // 0.125 second of history
    std::vector<Vector2> playerPositionHistory;
    // 17th character in tileset
    Sprite playerSprite = DungeonTileSet::characterStart; 
    playerSprite.x += 17 * DungeonTileSet::gridSquareSize;
    const float playerScale = 4.f;

    // enemy position
    auto enemyPos = Vector2{ scr_w, scr_h} / 4.f;
    Sprite enemySprite = DungeonTileSet::monsterStart; 
    const float enemyScale = 4.f;
    bool isAlive = true;

    // rotating axe around character
    Sprite axeSprite = DungeonTileSet::weaponStart;
    axeSprite.x += 0 * DungeonTileSet::gridSquareSize;
    const float axeRadius = 128;
    const float axeScale = 4.f;
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
    camera.target = playerPos;
    camera.offset = Vector2{scr_w, scr_h} / 2.f;
    camera.rotation = 0.f;
    camera.zoom = 1.f;

    SetTargetFPS(currentFPS);

    while (!WindowShouldClose()) { // close button or ESC key

        // time since last frame render
        float deltaTime = GetFrameTime();

        // direction vector of WASD input
        auto moveDir = GetWASDMovement();

        auto deltaPos = moveDir * deltaTime * speed;
        if(IsKeyPressed(KEY_SPACE)) {
            deltaPos *= currentFPS / 3.f;
        }

        playerPos += deltaPos;

        // enemy moves towards player
        Vector2 enemyToPlayer = playerPos - enemyPos;
        enemyPos += enemyToPlayer / 500.f;

        // character changing
        if(IsKeyPressed(KEY_RIGHT))
            playerSprite.x += DungeonTileSet::gridSquareSize;
        if(IsKeyPressed(KEY_LEFT))
            playerSprite.x -= DungeonTileSet::gridSquareSize;

        // revive enemy to starting position
        if(IsKeyPressed(KEY_R)) {
            enemyPos = Vector2{scr_w, scr_h} / 5.f;
            isAlive = true;
        }

        // axe movement
        axeTheta += deltaTime * 8.f;
        axeToCharTheta += deltaTime * 1.5f;

        // entity bounding rectangles
        // used for intersection and rendering 
        Rectangle playerRec = {
            .x = playerPos.x - (playerSprite.width*playerScale)/2.f,
            .y = playerPos.y - (playerSprite.height*playerScale)/2.f,
            .width = (float)playerSprite.width * playerScale,
            .height = (float)playerSprite.height * playerScale,
        };
        Rectangle enemyRec = {
            .x = enemyPos.x - (enemySprite.width*playerScale)/2.f,
            .y = enemyPos.y - (enemySprite.height*playerScale)/2.f,
            .width = (float)enemySprite.width * playerScale,
            .height = (float)enemySprite.height * playerScale,
        };
        Rectangle axeRec = {
            playerPos.x + std::cos(axeToCharTheta) * axeRadius,
            playerPos.y + std::sin(axeToCharTheta) * axeRadius,
            (float)axeSprite.width * axeScale,
            (float)axeSprite.height * axeScale,
        };

        // snap player inside tile
        if (auto p = boundsPenetration( playerRec, {tileStartX, tileStartY, tileWidth, tileHeight})) {
            playerPos -= *p;
        }

        // check axe collision with enemy
        if (rectIntersection(enemyRec, axeRec)) {
            isAlive = false;
        }

        // manage character history
        if (playerPositionHistory.size() > maxPlayerPositionHistory) {
            playerPositionHistory.erase(playerPositionHistory.begin());
        }
        playerPositionHistory.push_back(playerPos);

        // update camera target
        camera.target = playerPos;

        BeginDrawing();
        {
            ClearBackground(BLACK);

            BeginMode2D(camera);
            {
                // draw tile first
                DrawTexturePro(tilesetTexture, tileSprite, tile, {0.f, 0.f}, 0.f, WHITE);

                // faded player history trail including
                // the current, fully opaque player position
                int count = playerPositionHistory.size();
                for(int i = 0; i < count; i++) {
                    float t = (float)i / count;

                    Color c = WHITE;
                    c.a = (unsigned char)(255 * t * t); // fade in

                    Rectangle destRec = {
                        playerPositionHistory[i].x,
                        playerPositionHistory[i].y,
                        (float)playerSprite.width * playerScale,
                        (float)playerSprite.height * playerScale,
                    };
                    Vector2 origin = {
                        (playerSprite.width * playerScale) / 2.f,
                        (playerSprite.height * playerScale) / 2.f,
                    };
                    DrawTexturePro(tilesetTexture, playerSprite, destRec, origin, 0.f, c);
                }

                // draw enemy
                if(isAlive) {
                    DrawTexturePro(tilesetTexture, enemySprite, enemyRec, {0, 0}, 0.f, WHITE);
                }

                Vector2 axeOrigin = {
                    (axeSprite.width * axeScale) / 2.f,
                    (axeSprite.height * axeScale) / 2.f + 4.f * axeScale,
                };
                DrawTexturePro(
                    tilesetTexture,
                    axeSprite,
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
