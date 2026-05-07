#pragma once
#include "../../../core/include/Structures.hpp" // Тут должен быть struct Point2D { double x; double y; };

enum class TileType {
    EMPTY,    // Трава
    FOREST,   // Лес
    WATER,    // Вода
    WALL      // Бетонная стена
};

struct Tile {
    TileType type = TileType::EMPTY;
    bool isPassable = true;
    double transmittance = 1.0; // Коэффициент пропускания радиосигнала (0.0 - 1.0)
};

class Chunk {
public:
    static constexpr int CHUNK_SIZE = 16;
    static constexpr double TILE_SIZE = 50.0; // Размер одного тайла в метрах/пикселях

private:
    int chunkX;
    int chunkY;
    Tile* tiles; // Владеющий сырой указатель на массив 16x16

public:
    Chunk(int x, int y);
    ~Chunk();

    // Правило пяти: защищаем ручную память от утечек
    Chunk(const Chunk& other);
    Chunk& operator=(const Chunk& other);
    Chunk(Chunk&& other) noexcept;
    Chunk& operator=(Chunk&& other) noexcept;

    [[nodiscard]] int getX() const noexcept { return chunkX; }
    [[nodiscard]] int getY() const noexcept { return chunkY; }
    
    [[nodiscard]] Tile getTile(int localX, int localY) const;
    void setTile(int localX, int localY, TileType type);
};