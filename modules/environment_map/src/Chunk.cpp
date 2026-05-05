#include "Chunk.hpp"

Chunk::Chunk(int size, int offX, int offY) : chunkSize(size), offsetX(offX), offsetY(offY) {
    // Ручное выделение памяти под 2D массив
    grid = new TileType*[chunkSize];
    for (int i = 0; i < chunkSize; ++i) {
        grid[i] = new TileType[chunkSize];
        for (int j = 0; j < chunkSize; ++j) {
            grid[i][j] = TileType::Grass; // По умолчанию всё заливаем травой
        }
    }
}

Chunk::~Chunk() {
    // Очистка памяти
    for (int i = 0; i < chunkSize; ++i) {
        delete[] grid[i];
    }
    delete[] grid;
}

TileType Chunk::getTileLocal(int localX, int localY) const {
    if (localX < 0 || localX >= chunkSize || localY < 0 || localY >= chunkSize) {
        return TileType::Grass;
    }
    return grid[localX][localY];
}

void Chunk::setTileLocal(int localX, int localY, TileType type) {
    if (localX >= 0 && localX < chunkSize && localY >= 0 && localY < chunkSize) {
        grid[localX][localY] = type;
    }
}

bool Chunk::containsGlobal(int globalX, int globalY) const {
    return globalX >= offsetX && globalX < offsetX + chunkSize &&
           globalY >= offsetY && globalY < offsetY + chunkSize;
}