#pragma once
#include "entities/Entity.hpp"

class MobileBeacon final : public Entity {
public:
    MobileBeacon(Point2D startPos, Point2D startDir, float speed)
        : Entity(startPos, startDir, speed) {}

    // Маяк использует только стационарные вышки, поэтому список beacon-ов игнорируется
    void updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& /*activeBeacons*/) override;
};