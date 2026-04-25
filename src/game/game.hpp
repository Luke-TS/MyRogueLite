#pragma once

#include "game/dungeon_sprites.hpp"
#include "core/tilemap.hpp"

#include "defs.hpp"
#include "systems.hpp"
#include "spawner.hpp"

#include <raylib.h>

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

// check if Vector2 lies inside a centered rectangle
inline bool VectorInRectangleCentered(const Vector2& pos, const Rectangle& rec) {
    const Vector2 halfExtents = {rec.width/2.f, rec.height/2.f};
    return
        (pos.x >= (rec.x - halfExtents.x) && pos.x <= (rec.x + halfExtents.x)) &&
        (pos.y >= (rec.y - halfExtents.y) && pos.y <= (rec.y + halfExtents.y));
}

inline void updateCharacterSelect(GameContext& ctx) {
    auto& ecs = ctx.ecs;

    if (IsKeyPressed(KEY_BACKSPACE))
        ctx.state = GameState::MainMenu;

    const char* title    = "Character Select";
    const char* subtitle = "Click to select";
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

    for(int i = 0; i < Defs::CHARACTER_COUNT; i++) {
        const Defs::CharacterDef& def = Defs::characters[i];
        const Rectangle& src = DungeonSprites::sprites[def.sprite];

        constexpr auto renderScale = 6.f;
        Vector2 renderSize = {
            .x = src.width * renderScale, 
            .y = src.height * renderScale,
        };
        Rectangle destRec = {
            (screenWidth * (i+1) / (float)(Defs::CHARACTER_COUNT + 1)),
            screenHeight/1.7f,
            renderSize.x,
            renderSize.y,
        };

        // per character mouse events
        const Vector2 mouse = GetMousePosition();
        if(VectorInRectangleCentered(mouse, destRec)) {
            // select with left click
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                ctx.playerID = spawnPlayer(ctx, def, getCenterPos(ctx.map));
                ctx.state = GameState::Playing;
            }
            // increase sprite scale
            destRec.width  *= 1.2;
            destRec.height *= 1.2;
        }

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
    // TODO: pass only what needed instead of full context?
    systemPlayerMovement(ctx.ecs);
    systemEnemyAI(ctx.ecs);
    //systemProjectile(ctx, dt);
    systemSpawner(ctx);
    systemIntegration(ctx.ecs, dt);
    //systemOrbit(ctx.ecs, dt);
    systemEventTimer(ctx, GetFPS());

    systemEffectExecution(ctx, dt);

    // collision events
    static CollisionSets collisions;
    collisions.weaponEnemy.clear();
    collisions.playerEnemy.clear();
    collisions.entityTile.clear();
    collisions.enemyEnemy.clear();
    systemCollisionDetect(ctx, collisions);
    systemPhysicsResolve(ctx, collisions);

    static std::vector<HitEvent>     hits;
    hits.clear();
    static std::vector<WallHitEvent> wallHits;
    wallHits.clear();

    systemCollisionToHitEvents(ctx, collisions, hits, wallHits);

    systemOnHitEffects(ctx, hits);
    systemOnWallHitEffects(ctx, wallHits);

    // death events
    static std::vector<DeathEvent> deaths;
    deaths.clear();
    systemDeathDetection(ctx, deaths);

    // note: player death 'GameOver' state transition
    //       occurs here
    systemDeathResolution(ctx, deaths);

    // player entity should not be destroyed
    assert(ecs.isAlive(ctx.playerID));

    // camera follows player
    ctx.camera.target = ecs.transforms[ctx.playerID].position;

    // rendering
    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(ctx.camera);
        systemRenderMap(ctx);
        systemRenderEntities(ctx);
    EndMode2D();
    systemRenderHUD(ctx);   // health bar, XP bar — drawn in screen space
    EndDrawing();

    // check player progress
    auto& prog = ctx.progress;
    if( prog.xp >= prog.xpToNext ) {
        prog.level++;  // level up
        prog.xp = 0.f; // reset xp

        // scales linearly with player level
        prog.xpToNext += 100.f * prog.level;
        ctx.state = GameState::LevelUp;
    }

    ecs.destroyPending();
    ctx.frameCount++;
}

inline void updateConfirmQuit(GameContext& ctx) {
    // Button dimensions
    const int btnW = 200, btnH = 60;
    const int resumeX = screenWidth/2 - btnW - 20;
    const int resumeY = screenHeight/2;
    const int quitX   = screenWidth/2 + 20;
    const int quitY   = screenHeight/2;

    Vector2 mouse = GetMousePosition();

    bool hoverResume = mouse.x >= resumeX && mouse.x <= resumeX + btnW &&
                       mouse.y >= resumeY && mouse.y <= resumeY + btnH;
    bool hoverQuit   = mouse.x >= quitX   && mouse.x <= quitX   + btnW &&
                       mouse.y >= quitY   && mouse.y <= quitY   + btnH;

    // Resume: Enter key or green button click
    if (hoverResume && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ctx.state = GameState::Playing;
    }
    // Quit: ESC key or red button click
    if (hoverQuit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ctx.state = GameState::MainMenu;
        initGame(ctx);
    }

    const char* title     = "Quit game?";
    const char* resumeTxt = "RESUME";
    const char* quitTxt   = "QUIT";

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(ctx.camera);
        systemRenderMap(ctx, GRAY);
        systemRenderEntities(ctx, GRAY);
    EndMode2D();

    // Title
    DrawText(title,
        screenWidth/2 - MeasureText(title, 80)/2,
        screenHeight/4,
        80, RED);

    // ── Resume button (green) ──
    Color resumeCol = hoverResume ? (Color){0, 200, 80, 255} : (Color){0, 160, 60, 255};
    DrawRectangle(resumeX, resumeY, btnW, btnH, resumeCol);
    DrawRectangleLines(resumeX, resumeY, btnW, btnH, WHITE);
    DrawText(resumeTxt,
        resumeX + btnW/2 - MeasureText(resumeTxt, 28)/2,
        resumeY + btnH/2 - 14,
        28, WHITE);

    // ── Quit button (red) ──
    Color quitCol = hoverQuit ? (Color){220, 40, 40, 255} : (Color){180, 20, 20, 255};
    DrawRectangle(quitX, quitY, btnW, btnH, quitCol);
    DrawRectangleLines(quitX, quitY, btnW, btnH, WHITE);
    DrawText(quitTxt,
        quitX + btnW/2 - MeasureText(quitTxt, 28)/2,
        quitY + btnH/2 - 14,
        28, WHITE);

    EndDrawing();
}

inline void updateLevelUp(GameContext& ctx) {
    // present 3 weapon upgrade choices
    // selecting one resumes play

    const char* header = "LEVEL UP";
    const char* sub    = "Choose an upgrade:";

    // TODO: hook upgrade options into defs
    // TODO: apply upgrades
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
    // Button dimensions
    const int btnW = 200, btnH = 60;
    const int menuX = screenWidth/2 - btnW/2;
    const int menuY = screenHeight/2;

    Vector2 mouse = GetMousePosition();

    bool hoverMenu = mouse.x >= menuX && mouse.x <= menuX + btnW &&
                       mouse.y >= menuY && mouse.y <= menuY + btnH;

    if (hoverMenu && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ctx.state = GameState::MainMenu;
        ctx = {};
        initGame(ctx);
    }

    const char* title     = "YOU DIED";
    const char* resumeTxt = "MENU";

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(ctx.camera);
        systemRenderMap(ctx, GRAY);
        systemRenderEntities(ctx, GRAY);
    EndMode2D();

    // Title
    DrawText(title,
        screenWidth/2 - MeasureText(title, 80)/2,
        screenHeight/4,
        80, RED);

    // ── Menu button (green) ──
    Color menuCol = RED;
    DrawRectangle(menuX, menuY, btnW, btnH, menuCol);
    DrawRectangleLines(menuX, menuY, btnW, btnH, WHITE);
    DrawText(resumeTxt,
        menuX + btnW/2 - MeasureText(resumeTxt, 28)/2,
        menuY + btnH/2 - 14,
        28, WHITE);

    EndDrawing();
}
