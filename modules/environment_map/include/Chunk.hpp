#pragma once
#include "../../../core/include/Structures.hpp"

enum class TileType {
    Grass,      // Идеальный сигнал, проходимо
    Forest,     // Сигнал глушится слабо, проходимо
    Water,      // Идеальный сигнал, НЕпроходимо
    Concrete    // Сигнал глушится сильно, НЕпроходимо
};

class Chunk {
private:
    int chunkSize;
    TileType** grid; 
    int offsetX;     // Смещение чанка по X (в тайлах)
    int offsetY;     // Смещение чанка по Y (в тайлах)

public:
    Chunk(int size, int offX, int offY);
    ~Chunk();

    // Запрещаем копирование (ручное управление памятью)
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    TileType getTileLocal(int localX, int localY) const;
    void setTileLocal(int localX, int localY, TileType type);
    
    int getOffsetX() const { return offsetX; }
    int getOffsetY() const { return offsetY; }
    int getSize() const { return chunkSize; }
    
    bool containsGlobal(int globalX, int globalY) const;
};