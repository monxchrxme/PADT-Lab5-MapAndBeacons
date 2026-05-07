#pragma once
#include "../../../core/include/IEnvironment.hpp"
#include "../../../external/sequence/src/sequences/mutable_array_sequence.hpp"
#include "Chunk.hpp"

class EnvironmentManager final : public IEnvironment {
private:
    MutableArraySequence<Chunk*> activeChunks;
    MutableArraySequence<Point2D> staticTowers;

    [[nodiscard]] Chunk* getChunkAt(int chunkX, int chunkY) const;
    [[nodiscard]] Tile getTileAtWorldPos(Point2D p) const;
    void generateChunkData(Chunk* chunk);

public:
    EnvironmentManager() = default;
    ~EnvironmentManager() override;

    // Запрет копирования - мы владеем сырыми указателями на чанки
    EnvironmentManager(const EnvironmentManager&) = delete;
    EnvironmentManager& operator=(const EnvironmentManager&) = delete;

    // Реализация интерфейса IEnvironment
    [[nodiscard]] bool isPassable(Point2D p) const override;
    [[nodiscard]] double calculateSignal(Point2D a, Point2D b) const override;
    void triggerLazyGeneration(Point2D p) override;
    
    [[nodiscard]] MutableArraySequence<Point2D> getStaticTowers() const override; 

    // Экспортируем чанки ТОЛЬКО для рендерера
    [[nodiscard]] const MutableArraySequence<Chunk*>& getActiveChunks() const noexcept {
        return activeChunks;
    }
};