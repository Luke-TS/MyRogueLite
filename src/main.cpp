#include "raylib.h"

const float speed = 25.0f;

Vector2 GetWASDMovement(float deltaTime) {
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
    const int scr_w = 800;
    const int scr_h = 600;
    InitWindow(scr_w, scr_h, "MyRogueLite");

    int currentFPS = 60;

    // circle position
    Vector2 deltaCircle = { 0, (float)scr_h/3.f};

    SetTargetFPS(currentFPS);

    while (!WindowShouldClose()) { // close button or ESC key

        float mouseWheel = GetMouseWheelMove();
        if(mouseWheel != 0) {
            currentFPS += (int)mouseWheel;
            if (currentFPS < 0) currentFPS = 0;
            SetTargetFPS(currentFPS);
        }

        // reset key
        if(IsKeyPressed(KEY_R)) {
            deltaCircle.x = scr_w/2.0;
            deltaCircle.y = scr_h/2.0;
        }

        auto moveDelta = GetWASDMovement(GetFrameTime());
        deltaCircle.x += moveDelta.x;
        deltaCircle.y += moveDelta.y;

        BeginDrawing();
        {
            ClearBackground(BLACK);

            DrawCircleV(deltaCircle, 30, BLUE);

            //DrawText("It works!", 350, 280, 20, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
