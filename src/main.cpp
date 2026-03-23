#include <cstdlib>
#include <ctime>
#include <raylib.h>
#include <string>
#include <vector>

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

// dungeon_tileset.png asset starting coordinates
// note: tileset is a grid of 16x16 squares
const int gridSquareSize = 16;
const Vector2 floorTileTexCoord = {0, 7 * gridSquareSize};
const Vector2 floorTileTexDim = {16, 16};

int main() {
    srand(time(NULL));
    const std::string assetsPath = std::string(ASSETS_DIR);

    const int scr_w = 1200;
    const int scr_h = 800;
    InitWindow(scr_w, scr_h, "MyRogueLite");

    const int currentFPS = 120;

    // character position (modeled by a circle)
    const int playerRadius = 32;
    Vector2 playerPos = { scr_w/2.f, scr_h/2.f};
    const int maxPlayerPositionHistory = currentFPS / 5.f; // 0.2 second of history
    std::vector<Vector2> playerPositionHistory;

    // character sprite
    const std::string playerTexPath = assetsPath + "/rogue_spritesheet.png";
    Texture2D playerTex = LoadTexture(playerTexPath.c_str());
    int frameWidth = 32;
    int frameHeight = 32;
    Rectangle sourceRec = { 0, 0, (float)frameWidth, (float)frameHeight };

    // tile
    const int tileStartX = 200;
    const int tileStartY = 100;
    Rectangle tile = {tileStartX, tileStartY, 800, 600};

    // tile sprite
    const std::string tileSpritePath = assetsPath + "/dungeon_tileset.png";
    Texture2D tileTex = LoadTexture(tileSpritePath.c_str());
    Vector2 floorTileOffset = {
        float(rand() % 20),
        float(rand() % 2)
    };
    Rectangle tileSourceRec = {
        floorTileTexCoord.x + gridSquareSize * floorTileOffset.x,
        floorTileTexCoord.y + gridSquareSize * floorTileOffset.y,
        (float)floorTileTexDim.x,
        (float)floorTileTexDim.y,
    };

    SetTargetFPS(currentFPS);

    /*
     * Note: camera is attatched to center of player
     *       by shifting all visuals accordinly.
     *       playerPos is tracked as if it were 
     *       moving in a static environment
     */
    while (!WindowShouldClose()) { // close button or ESC key

        auto moveDelta = GetWASDMovement(GetFrameTime());
        playerPos.x += moveDelta.x;
        playerPos.y += moveDelta.y;

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
            DrawTexturePro(tileTex, tileSourceRec, tile, {0.f, 0.f}, 0.f, WHITE);

            // faded history trail
            int count = playerPositionHistory.size();
            for(int i = 0; i < count; i++) {
                float t = (float)i / count;

                Color c = WHITE;
                c.a = (unsigned char)(255 * t * t); // fade in

                Rectangle destRec = {
                    playerPositionHistory[i].x + snapToCenter.x - playerRadius,
                    playerPositionHistory[i].y + snapToCenter.y - playerRadius,
                    (float)frameWidth*2.f,
                    (float)frameHeight*2.f,
                };
                DrawTexturePro(playerTex, sourceRec, destRec, {0.f, 0.f}, 0.f, c);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
