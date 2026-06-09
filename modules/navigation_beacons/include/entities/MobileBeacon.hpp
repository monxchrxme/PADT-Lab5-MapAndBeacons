#pragma once
#include "entities/Entity.hpp"
#include "sequences/mutable_array_sequence.hpp"

class MobileBeacon final : public Entity {
private:
    // Сетевые буферы для Mesh-маршрутизации
    MutableArraySequence<int> seenPacketIds_;    // Память (ID пакетов, которые мы уже видели)
    MutableArraySequence<MeshPacket> txQueue_;   // Очередь на отправку соседям

    Point2D anchorPos_;       // Точка, к которой привязан маяк (центр патрулирования)
    double patrolRadius_;     // Максимальный радиус отхода от базы

public:
    MobileBeacon(Point2D startPos, Point2D startDir, float speed, double patrolRadius = 600.0)
        : Entity(startPos, startDir, speed), anchorPos_(startPos), patrolRadius_(patrolRadius) {}

    void updatePhysics(float dt, const IEnvironment* env) override;

    void updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& activeBeacons) override;

    // Прием пакета из радиоэфира
    void receivePacket(const MeshPacket& packet);
    
    // Получить очередь на отправку и очистить ее
    MutableArraySequence<MeshPacket> flushTxQueue();
};