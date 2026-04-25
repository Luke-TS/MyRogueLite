#pragma once

#include <raylib.h>
#include <string>
#include <array>

namespace DungeonSprites {

inline std::string texturePath = std::string(ASSETS_DIR)
                                + "/dungeon_tileset.png";

inline constexpr int grid = 16;

// Sprite Index Definitions
enum SpriteIdx {
    FLOOR_BASE,     // starting tile for floor atlas
    ENEMY_BASE,     // starting for enemies

    CHARACTER_1,
    CHARACTER_2,
    CHARACTER_3,
    WEAPON,
    AXE,
    ARROW,
    BOW,
    SKELETON,
    ZOMBIE,
    BIGBOI,
    FIRE,
    BONES,

    COUNT // total number of base sprites
};

// Sprite Array
inline std::array<Rectangle, COUNT> sprites = {{
    // FLOOR_BASE (acts as origin for floor atlas)
    {0 * grid, 7 * grid, 1 * grid, 1 * grid},

    // ENEMY_BASE (acts as origin for floor atlas)
    {12 * grid, 29 * grid, 1 * grid, 2 * grid},

    // CHARACTER_1
    {7 * grid, 26 * grid, 1 * grid, 2 * grid},

    // CHARACTER_2
    {9 * grid, 26 * grid, 1 * grid, 2 * grid},

    // CHARACTER_2
    {11 * grid, 26 * grid, 1 * grid, 2 * grid},

    // WEAPON
    {8 * grid, 24 * grid, 1 * grid, 1 * grid},

    // AXE
    {15 * grid, 24 * grid + 8, 1 * grid, 1 * grid - 8},

    // ARROW
    {23 * grid, 24 * grid + 11, 1 * grid, 1 * grid - 11},

    // BOW
    {18 * grid, 24 * grid, 1 * grid, 1 * grid},

    // SKELETON
    {12 * grid, 29 * grid, 1 * grid, 2 * grid},

    // ZOMBIE 
    {13 * grid, 29 * grid, 1 * grid, 2 * grid},

    // BIGBOI
    {18 * grid, 29 * grid, 2 * grid, 2 * grid},

    // FIRE
    {15 * grid, 32 * grid, 1 * grid, 1 * grid},

    // BONES
    {7 * grid, 15 * grid, 1 * grid, 1 * grid},
}};

inline Rectangle getFloorTile(int idx) {
    const int row = idx / 30;
    const int col = idx % 30;

    Rectangle sprite = sprites[FLOOR_BASE];
    sprite.x += col * grid;
    sprite.y += row * grid;

    return sprite;
}

inline int randomFloorTileIdx() {
    return rand() % 70;
}

}
