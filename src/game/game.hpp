#pragma once

#include "core/dungeon_sprites.hpp"
#include "core/tilemap.hpp"

#include "defs.hpp"
#include "systems.hpp"
#include "spawner.hpp"

#include <raylib.h>

constexpr float playerSpeed = 350.f;
constexpr float enemySpeed  = 150.f;
constexpr float arrowSpeed  = 500.f;
constexpr int   screenWidth  = 1500;
constexpr int   screenHeight = 1000;
constexpr float tileSize     = 500.f;

inline void initGame(GameContext& ctx) {
    InitWindow(screenWidth, screenHeight, "Delve");
    SetTargetFPS(120);

    // fresh contex
    ctx = {};

    ctx.tileTexture = LoadTexture(DungeonSprites::texturePath.c_str());
    ctx.map         = loadTileMap(std::string(LEVELS_DIR)+"/map.txt", tileSize);

    Vector2 tileCenter = {
        ctx.map.width  * ctx.map.tileSize / 2.f,
        ctx.map.height * ctx.map.tileSize / 2.f,
    };

    // camera
    ctx.camera.offset   = {screenWidth/2.f, screenHeight/2.f}; // center screen
    ctx.camera.rotation = 0.f;
    ctx.camera.zoom     = 1.f;
}

inline void updateMainMenu(GameContext& ctx) {
    if (IsKeyPressed(KEY_ENTER))
        ctx.state = GameState::CharacterSelect;

    const char* title    = "Delve";
    const char* subtitle = "created with raylib";
    const char* prompt   = "Press ENTER to Play";
    const char* quit     = "Press ESC to Quit";

    BeginDrawing();
    ClearBackground(BLACK);

    DrawText(title,
        screenWidth/2 - MeasureText(title, 80)/2,
        screenHeight/4,
        80, RED);

    DrawText(subtitle,
        screenWidth/2 - MeasureText(subtitle, 24)/2,
        screenHeight/4 + 90,
        24, RAYWHITE);

    float alpha = (sinf(GetTime() * 3.f) + 1.f) / 2.f;
    DrawText(prompt,
        screenWidth/2 - MeasureText(prompt, 32)/2,
        screenHeight/2 + 40,
        32, Fade(RAYWHITE, alpha));

    DrawText(quit,
        screenWidth/2 - MeasureText(quit, 20)/2,
        screenHeight - 60,
        20, DARKGRAY);

    EndDrawing();
}

inline void updateCharSelect(GameContext& ctx) {
    auto& ecs = ctx.ecs;

    if (IsKeyPressed(KEY_BACKSPACE))
        ctx.state = GameState::MainMenu;
    if (IsKeyPressed(KEY_ONE)) {
        ctx.playerID = spawnPlayer(ctx, Defs::characters[Defs::WARRIOR], getCenterPos(ctx.map));
        ctx.state = GameState::Playing;
    }
    if (IsKeyPressed(KEY_TWO)) {
        ctx.playerID = spawnPlayer(ctx, Defs::characters[Defs::ARCHER], getCenterPos(ctx.map));
        ctx.state = GameState::Playing;
    }

    for(int i = 0; i < Defs::CHARACTER_COUNT; i++) {
        if (IsKeyPressed(KEY_ONE + i)) { // keys one through CHARACTER_COUNT
            ctx.playerID = spawnPlayer(ctx, Defs::characters[i], getCenterPos(ctx.map));
            ctx.state = GameState::Playing;
        }
    }

    const char* title    = "Character Select";
    const char* subtitle = "Press 1 or 2";
    //const char* prompt   = "Select with ";
    const char* quit     = "Press BACK to Return";

    BeginDrawing();
    ClearBackground(BLACK);

    DrawText(title,
        screenWidth/2 - MeasureText(title, 80)/2,
        screenHeight/4,
        80, RED);

    DrawText(subtitle,
        screenWidth/2 - MeasureText(subtitle, 24)/2,
        screenHeight/4 + 90,
        24, RAYWHITE);

    DrawText(quit,
        screenWidth/2 - MeasureText(quit, 20)/2,
        screenHeight - 60,
        20, DARKGRAY);

    // render starting characters
    for(int i = 0; i < Defs::CHARACTER_COUNT; i++) {
        Defs::CharacterDef& def = Defs::characters[i];

        const Rectangle& src = DungeonSprites::sprites[def.sprite];

        Vector2 renderSize = {
            .x = src.width * 6.f,
            .y = src.height * 6.f,
        };
        Rectangle destRec = {
            (screenWidth * (i+1) / (float)(Defs::CHARACTER_COUNT + 1)),
            screenHeight/1.7f,
            renderSize.x,
            renderSize.y,
        };
        Vector2 origin = {
            destRec.width / 2.f,
            destRec.height / 2.f,
        };
        DrawTexturePro(
            ctx.tileTexture,
            src,
            destRec,
            origin,
            0.f,
            WHITE
        );

    }

    EndDrawing();
}

inline void updatePlaying(GameContext& ctx) {
    ECS&   ecs = ctx.ecs;
    float  dt  = GetFrameTime();

    // debug toggle
    if (IsKeyPressed(KEY_B))
        ctx.debugMode = !ctx.debugMode;
    if (IsKeyPressed(KEY_BACKSPACE))
        ctx.state = GameState::ConfirmQuit;

    // camera zoom
    ctx.camera.zoom = expf(logf(ctx.camera.zoom) + GetMouseWheelMove() * 0.1f);
    if (IsKeyPressed(KEY_I))
        ctx.camera.zoom += 0.2f;
    if (IsKeyPressed(KEY_O))
        ctx.camera.zoom -= 0.2f;

    ctx.camera.zoom = std::clamp(ctx.camera.zoom, 0.1f, 4.f);

    // teleport (debug)
    if (IsKeyPressed(KEY_T)) {
        auto mouseWorld = GetScreenToWorld2D(GetMousePosition(), ctx.camera);
        ecs.transforms[ctx.playerID].position = mouseWorld;
    }

    // spawn enemy on cursor (debug)
    if (IsKeyPressed(KEY_R)) {
        auto mouseWorld = GetScreenToWorld2D(GetMousePosition(), ctx.camera);
        spawnEnemy(ctx, Defs::enemies[Defs::BIGBOI], mouseWorld);
    }

    // rebuild cached views
    ecs.rebuildViews();

    // systems
    systemPlayerMovement(ctx.ecs, playerSpeed);
    systemEnemyAI(ctx.ecs);
    systemBowFire(ctx);
    systemSpawner(ctx);
    systemIntegration(ctx.ecs, dt);
    systemOrbit(ctx.ecs, dt);

    // collisions
    static std::vector<CollisionEvent> collisions;
    collisions.clear();
    systemCollisionDetect(ctx, collisions);
    systemCollisionResolve(ctx, collisions);

    // camera follows player
    ctx.camera.target = ecs.transforms[ctx.playerID].position;

    // check game over
    if (!ecs.isAlive(ctx.playerID) || ecs.healths[ctx.playerID].value <= 0.f)
        ctx.state = GameState::GameOver;

    // rendering
    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(ctx.camera);
        systemRenderMap(ctx);
        systemRenderEntities(ctx);
    EndMode2D();
    //systemRenderHUD(ctx);   // health bar, XP bar — drawn in screen space
    EndDrawing();

    // check level up
    if( ctx.progress.xp >= ctx.progress.xpToNext ) {
        ctx.progress.xp = 0.f;
        ctx.progress.xpToNext += 100.f;
        ctx.state = GameState::LevelUp;
    }

    ecs.destroyPending();
    ctx.frameCount++;
}

inline void updateConfirmQuit(GameContext& ctx) {
    if (IsKeyPressed(KEY_ENTER)) {
        ctx.state = GameState::MainMenu;
        initGame(ctx);
    }

    const char* title    = "Quit game?";
    const char* subtitle = "Enter to confirm";
    const char* quit     = "Press ESC to Quit";

    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode2D(ctx.camera);
        systemRenderMap(ctx, GRAY);
        systemRenderEntities(ctx, GRAY);
    EndMode2D();

    DrawText(title,
        screenWidth/2 - MeasureText(title, 80)/2,
        screenHeight/4,
        80, RED);

    DrawText(subtitle,
        screenWidth/2 - MeasureText(subtitle, 24)/2,
        screenHeight/4 + 90,
        24, RAYWHITE);

    DrawText(quit,
        screenWidth/2 - MeasureText(quit, 20)/2,
        screenHeight - 60,
        20, DARKGRAY);

    EndDrawing();
}

inline void updateLevelUp(GameContext& ctx) {
    // present 3 weapon upgrade choices
    // selecting one resumes play

    const char* header = "LEVEL UP";
    const char* sub    = "Choose an upgrade:";

    // hardcoded for now — hook into defs later
    const char* options[] = {
        "[1] Extra Axe",
        "[2] Faster Arrows",
        "[3] More Health",
    };

    /*
    if (IsKeyPressed(KEY_ONE))   { applyUpgrade(ctx, 0); ctx.state = GameState::Playing; }
    if (IsKeyPressed(KEY_TWO))   { applyUpgrade(ctx, 1); ctx.state = GameState::Playing; }
    if (IsKeyPressed(KEY_THREE)) { applyUpgrade(ctx, 2); ctx.state = GameState::Playing; }
    */

    if (IsKeyPressed(KEY_ONE)) { ctx.state = GameState::Playing; }

    BeginDrawing();
    ClearBackground(Color{10, 10, 30, 255}); // dark blue tint to signal pause

    DrawText(header,
        screenWidth/2 - MeasureText(header, 60)/2,
        screenHeight/5,
        60, RED);

    DrawText(sub,
        screenWidth/2 - MeasureText(sub, 24)/2,
        screenHeight/5 + 80,
        24, LIGHTGRAY);

    for (int i = 0; i < 3; i++) {
        DrawText(options[i],
            screenWidth/2 - MeasureText(options[i], 28)/2,
            screenHeight/2 + i * 50,
            28, WHITE);
    }

    EndDrawing();
}

inline void updateGameOver(GameContext& ctx) {
    // restart by re-running initGame
    if (IsKeyPressed(KEY_ENTER)) {
        ctx = GameContext{};   // reset all state
        initGame(ctx);
        ctx.state = GameState::Playing;
    }
    if (IsKeyPressed(KEY_ESCAPE))
        ctx.state = GameState::MainMenu;

    const char* header  = "YOU DIED";
    const char* restart = "Press ENTER to Restart";
    const char* menu    = "Press ESC for Main Menu";

    float alpha = (sinf(GetTime() * 2.f) + 1.f) / 2.f;

    BeginDrawing();
    ClearBackground(BLACK);

    DrawText(header,
        screenWidth/2 - MeasureText(header, 80)/2,
        screenHeight/3,
        80, RED);

    DrawText(restart,
        screenWidth/2 - MeasureText(restart, 28)/2,
        screenHeight/2 + 20,
        28, Fade(RAYWHITE, alpha));

    DrawText(menu,
        screenWidth/2 - MeasureText(menu, 20)/2,
        screenHeight/2 + 70,
        20, DARKGRAY);

    EndDrawing();
}
