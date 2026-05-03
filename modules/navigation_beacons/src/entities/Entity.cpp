#include "entities/Entity.hpp"
#include <cmath>
#include "MathUtils.hpp"

void Entity::updatePhysics(float dt, const IEnvironment* env) {
    if (!env) return;

    // Считаем потенциальную новую позицию
    Point2D nextPos = {
        realPosition_.x + velocity_.x * speed_ * dt,
        realPosition_.y + velocity_.y * speed_ * dt
    };

    if (env->isPassable(nextPos)) {
        realPosition_ = nextPos; 
    } else {
        // 1. Инвертируем вектор 
        velocity_.x = -velocity_.x;
        velocity_.y = -velocity_.y;
        
        // 2. JITTER: случайный поворот вектора от -15 до +15 градусов
        const double jitterAngle = math::randomDouble(-0.26, 0.26);
        math::rotate(velocity_, jitterAngle);
        math::normalize(velocity_);

        // 3. Выталкиваем объект из текстуры препятствия по новому направлению
        realPosition_.x += velocity_.x * speed_ * dt * 1.1f;
        realPosition_.y += velocity_.y * speed_ * dt * 1.1f;
    }
}