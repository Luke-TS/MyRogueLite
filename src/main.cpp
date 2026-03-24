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

// Returns nullopt if the circle is fully enclosed by rec.
// Otherwise returns penetration depth on each axis.
std::optional<Vector2> boundsPenetration(const Vector2& origin, const int radius, const Rectangle& rec) {
    float px = 0.0f, py = 0.0f;

    if      (origin.x - radius < rec.x)              px = (origin.x - radius) - rec.x;
    else if (origin.x + radius > rec.x + rec.width)  px = (origin.x + radius) - (rec.x + rec.width);

    if      (origin.y - radius < rec.y)              py = (origin.y - radius) - rec.y;
    else if (origin.y + radius > rec.y + rec.height) py = (origin.y + radius) - (rec.y + rec.height);

    if (px == 0.0f && py == 0.0f)
        return std::nullopt;

    return Vector2{ px, py };
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
    const int maxPlayerPositionHistory = currentFPS / 8.f; // 0.125 second of history
    std::vector<Vector2> playerPositionHistory;

    // 17th character in tileset
    Sprite playerSprite = DungeonTileSet::characterStart; 
    playerSprite.x += 17 * DungeonTileSet::gridSquareSize;
    const float playerScale = 4.f;

    // rotating axe around character
    Sprite axeSprite = DungeonTileSet::weaponsStart;
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

    SetTargetFPS(currentFPS);

    /*
     * Note: camera is attatched to center of player
     *       by shifting all sprites accordinly.
     *       playerPos is tracked as if it were 
     *       moving in a fixed environment
     */
    while (!WindowShouldClose()) { // close button or ESC key

        float deltaTime = GetFrameTime();
        auto moveDelta = GetWASDMovement(deltaTime);
        playerPos.x += moveDelta.x;
        playerPos.y += moveDelta.y;

        axeTheta += deltaTime * 8.f;
        axeToCharTheta += deltaTime * 1.5f;

        // snap player inside tile
        if (auto p = boundsPenetration(playerPos, playerRadius, {tileStartX, tileStartY, tileWidth, tileHeight})) {
            playerPos.x -= p->x;
            playerPos.y -= p->y;
        }

        // manage character history
        if (playerPositionHistory.size() > maxPlayerPositionHistory) {
            playerPositionHistory.erase(playerPositionHistory.begin());
        }
        playerPositionHistory.push_back(playerPos);

        // difference in player position
        // and the center of the screen
        Vector2 deltaToCenter = {(scr_w / 2.f) - playerPos.x, (scr_h / 2.f) - playerPos.y};

        // adjust tile position by this delta
        tile.x = tileStartX + deltaToCenter.x;
        tile.y = tileStartY + deltaToCenter.y;

        BeginDrawing();
        {
            ClearBackground(BLACK);

            // draw tile first
            DrawTexturePro(tilesetTexture, tileSprite, tile, {0.f, 0.f}, 0.f, WHITE);

            // faded player history trail including
            // the current, fully opaque player position
            int count = playerPositionHistory.size();
            for(int i = 0; i < count; i++) {
                float t = (float)i / count;

                Color c = WHITE;
                c.a = (unsigned char)(255 * t * t); // fade in

                // position is corrected by deltaToCenter
                Rectangle destRec = {
                    playerPositionHistory[i].x + deltaToCenter.x,
                    playerPositionHistory[i].y + deltaToCenter.y,
                    (float)playerSprite.width * playerScale,
                    (float)playerSprite.height * playerScale,
                };
                Vector2 origin = {
                    (playerSprite.width * playerScale) / 2.f,
                    (playerSprite.height * playerScale) / 2.f,
                };
                DrawTexturePro(tilesetTexture, playerSprite, destRec, origin, 0.f, c);
            }

            // render axe
            // position is corrected by deltaToCenter
            Rectangle destRec = {
                playerPos.x + deltaToCenter.x + std::cos(axeToCharTheta) * axeRadius,
                playerPos.y + deltaToCenter.y + std::sin(axeToCharTheta) * axeRadius,
                (float)axeSprite.width * axeScale,
                (float)axeSprite.height * axeScale,
            };

            Vector2 origin = {
                (axeSprite.width * axeScale) / 2.f,
                (axeSprite.height * axeScale) / 2.f + 4.f * axeScale,
            };

            DrawTexturePro(
                tilesetTexture,
                axeSprite,
                destRec,
                origin,
                axeTheta * 180.f / 3.14f,
                WHITE
            );
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
