#pragma once
#include "Structures.hpp"
#include "IEnvironment.hpp"
#include "interfaces/sequence.hpp"
#include "MathUtils.hpp"

class MobileBeacon;

class Entity {
protected:
    Point2D realPosition_;      // Реальные координаты на карте
    Point2D velocity_;          // Вектор скорости (пикселей в секунду)
    LocationResult estimation_; // Вычисленные координаты и радиус ошибки
    float speed_;               // Скалярная скорость

public:
    Entity(Point2D startPos, Point2D startDir, float speed)
        : realPosition_(startPos), velocity_(startDir), speed_(speed) {
        math::normalize(velocity_); 
        if (std::abs(velocity_.x) < 1e-9 && std::abs(velocity_.y) < 1e-9) {
            velocity_.x = 1.0; 
            velocity_.y = 0.0;
        }
        estimation_ = {startPos, 0.0};
    }

    virtual ~Entity() = default;

    // Rule of 5: Запрещаем копирование и перемещение, сущность уникальна
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    // Физика 
    virtual void updatePhysics(float dt, const IEnvironment* env);

    // Сбор данных и Математика 
    virtual void updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& activeBeacons) = 0;

    [[nodiscard]] Point2D getRealPosition() const { return realPosition_; }
    [[nodiscard]] Point2D getVelocity() const { return velocity_; }
    [[nodiscard]] LocationResult getEstimation() const { return estimation_; }
};