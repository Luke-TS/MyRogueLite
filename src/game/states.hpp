#pragma once

#include <vector>
#include <cstdint>

#include <raylib.h>

#include "core/ecs.hpp"
#include "core/tilemap.hpp"
#include "game/defs.hpp"

// game state enum
enum class GameState {
    MainMenu,
    CharacterSelect,
    Playing,
    ConfirmQuit,
    LevelUp,
    GameOver,
};

// subsystem states
struct SpawnerState {
    float timeSinceLastWave = 0.f;
    float waveInterval      = 5.f;
    int   waveNumber        = 0;
};

// maintains player information
// NOTE: game-logic only;
//       player entity is accessed through ctx.playerID
struct PlayerInfo {
    int   level     = 1;
    float xp        = 0.f;
    float xpToNext  = 100.f;

    float bowCooldown = 0.f; // time since last shot

    // which weapons the player has unlocked
    // indexed into Defs::skills
    std::vector<Defs::SkillIdx> unlockedSkills = {};

    struct EquippedSkill {
        Defs:: SkillIdx  skillIdx;
        std::vector<int> supports; // currently unused
    };

    std::vector<EquippedSkill> loadout;
};

// central game context
struct GameContext {
    // core systems
    ECS       ecs;
    TileMap   map;
    Camera2D  camera      = {};
    Texture2D tileTexture = {};

    // game state
    GameState      state      = GameState::MainMenu;
    bool           debugMode  = false;
    uint64_t       frameCount = 0;

    // subsystems
    SpawnerState   spawner;
    PlayerInfo progress;

    // cached entity handles
    Entity playerID = -1; // set once in initGame()
};
