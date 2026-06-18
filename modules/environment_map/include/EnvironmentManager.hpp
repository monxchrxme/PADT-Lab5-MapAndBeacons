#pragma once
#include "../../../core/include/Structures.hpp"
#include "../../../external/sequence/src/sequences/mutable_array_sequence.hpp"
#include "../../../core/include/IEnvironment.hpp"
#include "Chunk.hpp"

struct MapBounds 
{
    double minX, minY, maxX, maxY;
};

struct ChunkCoord 
{
    int x, y;
};

class EnvironmentManager final : public IEnvironment 
{
public:
    //Режимы работы среды
    enum class Mode 
    {
        NEW_GAME,
        LOAD_GAME
    };

private:
    Mode currentMode;
    MutableArraySequence<Chunk*> activeChunks;
    MutableArraySequence<Point2D> staticTowers;
    MutableArraySequence<ChunkCoord> exploredChunksHistory;
    
    [[nodiscard]] bool isChunkInHistory(int cx, int cy) const;
    [[nodiscard]] Chunk* getChunkAt(int chunkX, int chunkY) const;
    [[nodiscard]] Tile getTileAtWorldPos(Point2D p) const;
    void generateChunkData(Chunk* chunk);

    //Работа с диском (бинарная сериализация)
    void saveWorldToDisk() const;
    void unloadDistantChunks(Point2D center, double maxDistance);

public:
    explicit EnvironmentManager(Mode mode = Mode::NEW_GAME);
    ~EnvironmentManager() override;

    void loadWorldFromDisk();
    EnvironmentManager(const EnvironmentManager&) = delete;
    EnvironmentManager& operator=(const EnvironmentManager&) = delete;

    [[nodiscard]] bool isPassable(Point2D p) const override;
    [[nodiscard]] MutableArraySequence<RadioPath> computePaths(Point2D tx, Point2D rx, double frequencyGHz) const override;
    
    void triggerLazyGeneration(Point2D p) override;
    [[nodiscard]] MutableArraySequence<Point2D> getStaticTowers() const override; 

    [[nodiscard]] const MutableArraySequence<Chunk*>& getActiveChunks() const noexcept 
    {
        return activeChunks;
    }
    [[nodiscard]] MapBounds getWorldBounds() const;
};