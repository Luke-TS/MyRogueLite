#include <cmath>
#include <cstdlib>
#include <ctime>
#include <raylib.h>
#include <string>
#include <vector>

// header for obaining sprites 
// from dungeon_tileset.png
#include "dungeon_tileset.hpp"

const float speed = 50.0f;

Vector2 GetWASDMovement(const float deltaTime) {
    Vector2 result = {0, 0};
    if(IsKeyDown(KEY_D)) {
        result.x += deltaTime*6.0f*speed;
    }
    if(IsKeyDown(KEY_A)) {
        result.x -= deltaTime*6.0f*speed;
    }
    if(IsKeyDown(KEY_W)) {
        result.y -= deltaTime*6.0f*speed;
    }
    if(IsKeyDown(KEY_S)) {
        result.y += deltaTime*6.0f*speed;
    }
    return result;
}

int main() {
    srand(time(NULL));

    // screen setup
    const int scr_w = 1200;
    const int scr_h = 800;
    InitWindow(scr_w, scr_h, "MyRogueLite");

    const int currentFPS = 120;

    const Texture2D tilesetTexture = LoadTexture(DungeonTileSet::texturePath.c_str());

    // character position (modeled by a circle)
    const int playerRadius = 64;
    Vector2 playerPos = { scr_w/2.f, scr_h/2.f};
    const int maxPlayerPositionHistory = currentFPS / 5.f; // 0.2 second of history
    std::vector<Vector2> playerPositionHistory;

    // 17th character in tileset
    Sprite playerSprite = DungeonTileSet::characterStart; 
    playerSprite.x += 17 * DungeonTileSet::gridSquareSize;

    // rotating axe around character
    Sprite axeSprite = DungeonTileSet::weaponsStart;
    axeSprite.x += 0 * DungeonTileSet::gridSquareSize;
    const float axeRadius = 64;
    float axeTheta = 0.f;
    float axeToCharTheta = 0.f;

    // tile position
    const int tileStartX = 200;
    const int tileStartY = 100;
    Rectangle tile = {tileStartX, tileStartY, 800, 600};

    // random tile sprite
    Sprite tileSprite = DungeonTileSet::floorTileStart;
    tileSprite.x += (rand() % 20) * DungeonTileSet::gridSquareSize;
    tileSprite.y += (rand() % 2)  * DungeonTileSet::gridSquareSize;

    SetTargetFPS(currentFPS);

    /*
     * Note: camera is attatched to center of player
     *       by shifting all visuals accordinly.
     *       playerPos is tracked as if it were 
     *       moving in a static environment
     */
    while (!WindowShouldClose()) { // close button or ESC key

        float deltaTime = GetFrameTime();
        auto moveDelta = GetWASDMovement(deltaTime);
        playerPos.x += moveDelta.x;
        playerPos.y += moveDelta.y;

        axeTheta += deltaTime * 3.f;
        axeToCharTheta += deltaTime;

        // clamp player to tile
        if((playerPos.x - playerRadius) < tileStartX) {
            playerPos.x = (tileStartX + playerRadius);
        } else if((playerPos.x + playerRadius) > (tileStartX + tile.width)) {
            playerPos.x = (tileStartX + tile.width - playerRadius);
        }

        if((playerPos.y - playerRadius) < tileStartY) {
            playerPos.y = (tileStartY + playerRadius);
        } else if((playerPos.y + playerRadius) > (tileStartY + tile.height)) {
            playerPos.y = (tileStartY + tile.height- playerRadius);
        }

        // manage character history
        while (playerPositionHistory.size() > maxPlayerPositionHistory) {
            playerPositionHistory.erase(playerPositionHistory.begin());
        }
        playerPositionHistory.push_back(playerPos);

        Vector2 snapToCenter = {(scr_w / 2.f) - playerPos.x, (scr_h / 2.f) - playerPos.y};

        tile.x = tileStartX + snapToCenter.x;
        tile.y = tileStartY + snapToCenter.y;

        BeginDrawing();
        {
            ClearBackground(BLACK);

            //DrawRectangleRec(tile, SKYBLUE);
            DrawTexturePro(tilesetTexture, tileSprite, tile, {0.f, 0.f}, 0.f, WHITE);

            // faded history trail
            int count = playerPositionHistory.size();
            for(int i = 0; i < count; i++) {
                float t = (float)i / count;

                Color c = WHITE;
                c.a = (unsigned char)(255 * t * t); // fade in

                Rectangle destRec = {
                    playerPositionHistory[i].x + snapToCenter.x - playerRadius,
                    playerPositionHistory[i].y + snapToCenter.y - playerRadius,
                    (float)playerSprite.width*4.f,
                    (float)playerSprite.height*4.f,
                };
                DrawTexturePro(tilesetTexture, playerSprite, destRec, {0.f, 0.f}, 0.f, c);
            }

            // render axe
            Rectangle destRec = {
                playerPos.x + snapToCenter.x + std::cos(axeToCharTheta) * axeRadius,
                playerPos.y + snapToCenter.y + std::sin(axeToCharTheta) * axeRadius,
                (float)axeSprite.width*4.f,
                (float)axeSprite.height*4.f,
            };
            DrawTexturePro(tilesetTexture, axeSprite, destRec, {32.f, 64.f}, axeTheta * 180.f/3.14, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
