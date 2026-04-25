#pragma once

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <raylib.h>

struct Tile {
    int spriteIndex;
    bool solid;
};

struct TileMap {
    int width, height;
    float tileSize;

    std::vector<Tile> tiles;
};

inline const Tile* getTile(const TileMap& map, int x, int y) {
    if (x < 0 || x >= map.width)  return nullptr;
    if (y < 0 || y >= map.height) return nullptr;
    return &map.tiles[y * map.width + x];
}

inline const Vector2 getCenterPos(const TileMap& map) {
    return {
        .x = map.width  * map.tileSize / 2.f,
        .y = map.height * map.tileSize / 2.f,
    };
}

inline TileMap createMap(float tileSize, const Vector2 dims) {
    TileMap map = TileMap{
        .width = static_cast<int>(dims.x),
        .height = static_cast<int>(dims.y),
        .tileSize = tileSize,
    };
    for(int _ = 0; _ < map.width * map.height; _++) {
        map.tiles.push_back({rand() % 70, false});
    }
    return map;
}

inline TileMap loadTileMap(const std::string& path, float tileSize) {
    TileMap map;
    map.tileSize = tileSize;

    std::ifstream file(path);
    file >> map.width >> map.height;

    for (int i = 0; i < map.height; i++) {
        for (int j = 0; j < map.width; j++) {
            bool isTile;
            file >> isTile;
            if (isTile) {
                map.tiles.push_back({
                    .spriteIndex = rand() % 70,
                    .solid = false
                });
            } else {
                map.tiles.push_back({
                    .spriteIndex = 0,
                    .solid = true
                });
            }
        }
    }

    return map;
}

inline std::vector<Rectangle> getNearbyMapTiles(const TileMap& map, Vector2 pos) {
    std::vector<Rectangle> result;

    int tx = int(pos.x / map.tileSize);
    int ty = int(pos.y / map.tileSize);

    for (int y = ty - 1; y <= ty + 1; y++) {
        for (int x = tx - 1; x <= tx + 1; x++) {
            const Tile* t = getTile(map, x, y);
            bool outOfBounds = (t == nullptr);
            if (!(outOfBounds || t->solid)) {
                result.push_back(Rectangle{
                    x * map.tileSize,
                    y * map.tileSize,
                    map.tileSize,
                    map.tileSize
                });
            }
        }
    }

    return result;
}

inline std::vector<Rectangle> getNearbySolidTiles(const TileMap& map, Vector2 pos) {
    std::vector<Rectangle> result;

    int tx = int(pos.x / map.tileSize);
    int ty = int(pos.y / map.tileSize);

    for (int y = ty - 1; y <= ty + 1; y++) {
        for (int x = tx - 1; x <= tx + 1; x++) {
            const Tile* t = getTile(map, x, y);
            bool outOfBounds = (t == nullptr);
            if (outOfBounds || t->solid) {
                result.push_back(Rectangle{
                    x * map.tileSize,
                    y * map.tileSize,
                    map.tileSize,
                    map.tileSize
                });
            }
        }
    }

    return result;
}
