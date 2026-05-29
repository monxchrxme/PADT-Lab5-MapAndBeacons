#include "../include/Chunk.hpp"
#include "../../../core/include/exceptions/EnvironmentExceptions.hpp"
#include <utility>

Chunk::Chunk(int x, int y) : chunkX(x), chunkY(y) {
    tiles = new Tile[CHUNK_SIZE * CHUNK_SIZE];
}

Chunk::~Chunk() {
    delete[] tiles;
}

Chunk::Chunk(const Chunk& other) : chunkX(other.chunkX), chunkY(other.chunkY) {
    tiles = new Tile[CHUNK_SIZE * CHUNK_SIZE];
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
        tiles[i] = other.tiles[i];
    }
}

Chunk& Chunk::operator=(const Chunk& other) {
    if (this == &other) return *this;
    Tile* newTiles = new Tile[CHUNK_SIZE * CHUNK_SIZE];
    for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
        newTiles[i] = other.tiles[i];
    }
    delete[] tiles;
    tiles = newTiles;
    chunkX = other.chunkX;
    chunkY = other.chunkY;
    return *this;
}

Chunk::Chunk(Chunk&& other) noexcept : chunkX(other.chunkX), chunkY(other.chunkY), tiles(other.tiles) {
    other.tiles = nullptr;
}

Chunk& Chunk::operator=(Chunk&& other) noexcept {
    if (this == &other) return *this;
    delete[] tiles;
    tiles = other.tiles;
    chunkX = other.chunkX;
    chunkY = other.chunkY;
    other.tiles = nullptr;
    return *this;
}

Tile Chunk::getTile(int localX, int localY) const {
    if (localX < 0 || localX >= CHUNK_SIZE || localY < 0 || localY >= CHUNK_SIZE) {
        throw OutOfBoundsException("Chunk local coordinates out of bounds");
    }
    return tiles[localY * CHUNK_SIZE + localX];
}

void Chunk::setTile(int localX, int localY, TileType type, int height) {
    if (localX < 0 || localX >= CHUNK_SIZE || localY < 0 || localY >= CHUNK_SIZE) {
        throw OutOfBoundsException("Chunk local coordinates out of bounds");
    }
    
    Tile& tile = tiles[localY * CHUNK_SIZE + localX];
    tile.type = type;
    tile.height = height; 
    
    switch (type) {
        case TileType::EMPTY:  tile.isPassable = true;  tile.transmittance = 1.0; break;
        case TileType::WATER:  tile.isPassable = false; tile.transmittance = 1.0; break;
        case TileType::PATH:   tile.isPassable = true;  tile.transmittance = 1.0; break;
        case TileType::WALL:   tile.isPassable = false; tile.transmittance = 0.1; break; 
        case TileType::TOWER_BASE: tile.isPassable = false; tile.transmittance = 0.5; break;
        
        case TileType::FOREST: 
            tile.isPassable = true;
            // ФИЗИКА ГУСТОТЫ ЛЕСА:
            if (height == 1) tile.transmittance = 0.8;      // Редкий лес
            else if (height == 2) tile.transmittance = 0.5; // Обычный лес 
            else if (height == 3) tile.transmittance = 0.2; // Густая чаща 
            else tile.transmittance = 0.6;
            break;
    }
}