#pragma once
#include "../../../core/include/IEnvironment.hpp" 
#include "Chunk.hpp"
#include "../../../external/sequence/src/sequences/mutable_array_sequence.hpp"

class EnvironmentManager : public IEnvironment {
private:
    MutableArraySequence<Chunk*> chunks; 
    MutableArraySequence<Point2D> staticTowers;
    const int CHUNK_SIZE = 16;

    Chunk* getChunkAt(int globalX, int globalY) const;
    void generateChunk(int chunkX, int chunkY);

public:
    EnvironmentManager();
    ~EnvironmentManager() override;

    // Реализация интерфейса IEnvironment
    bool isPassable(Point2D point) const override;
    double calculateSignal(Point2D a, Point2D b) const override;
    MutableArraySequence<Point2D> getStaticTowers() const override;
    void triggerLazyGeneration(Point2D p) override;
};