#pragma once
#include "entities/Entity.hpp"

class TargetObject final : public Entity {
public:
    TargetObject(Point2D startPos, float speed)
        : Entity(startPos, {0.0, 0.0}, speed) {} // Стоит на месте до команды

    // Метод для управления через WASD
    void setDirection(double dx, double dy);

    void updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& activeBeacons) override;
};