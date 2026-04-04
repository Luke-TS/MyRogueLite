#pragma once

#include <vector>
#include <cstdint>

#include <raylib.h>

#include "core/ecs.hpp"
#include "core/tilemap.hpp"

// game state enum
enum class GameState {
    MainMenu,
    Playing,
    LevelUp,
    GameOver,
};

// subsystem states
struct SpawnerState {
    float timeSinceLastWave = 0.f;
    float waveInterval      = 3.f;
    int   waveNumber        = 0;
};

struct PlayerProgress {
    int   level     = 1;
    float xp        = 0.f;
    float xpToNext  = 100.f;

    float bowCooldown = 0.f; // time since last shot

    // which weapon slots the player has unlocked
    // indexed into WEAPON_DEFS
    std::vector<int> unlockedWeapons = {0, 2}; // starts with axe + arrow
};

// central game context
struct GameContext {
    // core systems
    ECS       ecs;
    TileMap   map;
    Camera2D  camera    = {};
    Texture2D tileTexture = {};

    // game state
    GameState      state      = GameState::MainMenu;
    bool           debugMode  = false;
    uint64_t       frameCount = 0;

    // subsystems
    SpawnerState   spawner;
    PlayerProgress progress;

    // cached entity handles
    Entity player = -1; // set once in initGame()
};
