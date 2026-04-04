#pragma once

#include <raylib.h>
#include <string> // for texture file path

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

inline Rectangle floorTileStart = {
    .x = 0 * gridSquareSize,
    .y = 7 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 1 * gridSquareSize
};

inline Rectangle characterStart = {
    .x = 7 * gridSquareSize,
    .y = 26 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 2 * gridSquareSize,
};

inline Rectangle weaponStart = {
    .x = 8 * gridSquareSize,
    .y = 24 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 1 * gridSquareSize,
};

inline Rectangle axeSprite = {
    .x = 15 * gridSquareSize,
    .y = 24 * gridSquareSize + 8,
    .width = 1 * gridSquareSize,
    .height = 1 * gridSquareSize - 8,
};

inline Rectangle arrowSprite = {
    .x = 23 * gridSquareSize,
    .y = 24 * gridSquareSize + 11,
    .width = 1 * gridSquareSize,
    .height = 1 * gridSquareSize - 11,
};

inline Rectangle bowSprite = {
    .x = 18 * gridSquareSize,
    .y = 24 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 1 * gridSquareSize,
};

inline Rectangle monsterStart = {
    .x = 13 * gridSquareSize,
    .y = 29 * gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 2 * gridSquareSize,
};

inline Rectangle bonesDecor = {
    .x = 7 * gridSquareSize,
    .y = 15* gridSquareSize,
    .width = 1 * gridSquareSize,
    .height = 1 * gridSquareSize,
};

inline Rectangle getFloorTile(int idx) {
    // get floor row/col
    const int row = idx / 30;
    const int col = idx % 30;

    // calculate random sprite (x, y)
    Rectangle sprite = floorTileStart; 
    sprite.x += col * gridSquareSize;
    sprite.y += row * gridSquareSize;
    return sprite;
}

inline int randomFloorTileIdx() {
    // select floor number
    const int numFloors = 70;
    const int randFloorNum = rand() % numFloors;

    return randFloorNum;
};

};
