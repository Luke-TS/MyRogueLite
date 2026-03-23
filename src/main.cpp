#include "raylib.h"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <queue>
#include <utility>
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

int main() {
    const int scr_w = 1200;
    const int scr_h = 800;
    InitWindow(scr_w, scr_h, "MyRogueLite");

    int currentFPS = 120;

    // circle position
    const int circleRadius = 32;
    Vector2 circPos = { scr_w/2.f, scr_h/2.f};
    const int maxHistory = currentFPS / 4.f; // quarter second of history
    std::vector<Vector2> positionHistory;

    // tile
    const int tileStartX = 200;
    const int tileStartY = 100;
    Rectangle tile = {tileStartX, tileStartY, 800, 600};

    SetTargetFPS(currentFPS);

    while (!WindowShouldClose()) { // close button or ESC key

        auto moveDelta = GetWASDMovement(GetFrameTime());
        circPos.x += moveDelta.x;
        circPos.y += moveDelta.y;

        // clamp circle to tile
        if((circPos.x - circleRadius) < tileStartX) {
            circPos.x = (tileStartX + circleRadius);
        } else if((circPos.x + circleRadius) > (tileStartX + tile.width)) {
            circPos.x = (tileStartX + tile.width - circleRadius);
        }

        if((circPos.y - circleRadius) < tileStartY) {
            circPos.y = (tileStartY + circleRadius);
        } else if((circPos.y + circleRadius) > (tileStartY + tile.height)) {
            circPos.y = (tileStartY + tile.height- circleRadius);
        }

        // manage circle history
        while (positionHistory.size() > maxHistory) {
            positionHistory.erase(positionHistory.begin());
        }
        positionHistory.push_back(circPos);

        Vector2 snapToCenter = {(scr_w / 2.f) - circPos.x, (scr_h / 2.f) - circPos.y};

        tile.x = tileStartX + snapToCenter.x;
        tile.y = tileStartY + snapToCenter.y;

        BeginDrawing();
        {
            ClearBackground(BLACK);

            DrawRectangleRec(tile, SKYBLUE);

            // trail
            int count = positionHistory.size();
            for(int i = 0; i < count; i++) {
                float t = (float)i / count;

                Color c = RED;
                c.a = (unsigned char)(255 * t); // fade in

                Vector2 snapped = {
                    positionHistory[i].x + snapToCenter.x,
                    positionHistory[i].y + snapToCenter.y,
                };

                DrawCircleV(snapped, circleRadius, c);
            }

            //DrawText("It works!", 350, 280, 20, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
