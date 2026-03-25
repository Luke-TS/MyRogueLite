#pragma once

#include <raylib.h>
#include <string> // for texture file path

// Sprite as an alias of Rectangle containing:
// (x, y) as coordinates in texture
// (width, height) as dimensions of the texture
using Sprite = Rectangle;

// defines sprite structs for each asset 
// in dungeon_tileset.png
//
// contains helper functions for random
// floor tiles and such
namespace DungeonTileSet {

inline std::string texturePath = std::string(ASSETS_DIR)
                                + "/dungeon_tileset.png";

// texture grid contains 16x16 squares
inline const int gridSquareSize = 16;

inline const Sprite floorTileStart = {
    .x = 0 * gridSquareSize,
    .y = 7 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 1 * gridSquareSize
};

inline const Sprite characterStart = {
    .x = 0 * gridSquareSize,
    .y = 26 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 2 * gridSquareSize,
};

inline const Sprite weaponStart = {
    .x = 8 * gridSquareSize,
    .y = 24 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 1 * gridSquareSize,
};

inline const Sprite monsterStart = {
    .x = 13 * gridSquareSize,
    .y = 29 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 2 * gridSquareSize,
};

};
