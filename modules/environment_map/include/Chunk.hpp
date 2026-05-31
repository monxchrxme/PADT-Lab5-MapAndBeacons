#pragma once
#include "../../../core/include/Structures.hpp" 

enum class TileType {
    EMPTY,    
    FOREST,   
    WATER,    
    WALL,      
    PATH,       
    TOWER_BASE   
};

struct Tile {
    TileType type = TileType::EMPTY;
    bool isPassable = true;
    double transmittance = 1.0; 
    int height = 0; // Для дома это высота, для леса - густота
};

class Chunk {
public:
    static constexpr int CHUNK_SIZE = 32;
    static constexpr double TILE_SIZE = 25.0;

private:
    int chunkX;
    int chunkY;
    Tile* tiles;

public:
    Chunk(int x, int y);
    ~Chunk();

    Chunk(const Chunk& other);
    Chunk& operator=(const Chunk& other);
    Chunk(Chunk&& other) noexcept;
    Chunk& operator=(Chunk&& other) noexcept;

    [[nodiscard]] int getX() const noexcept { return chunkX; }
    [[nodiscard]] int getY() const noexcept { return chunkY; }
    
    [[nodiscard]] Tile getTile(int localX, int localY) const;
    void setTile(int localX, int localY, TileType type, int height = 0);
};