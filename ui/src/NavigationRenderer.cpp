#include "NavigationRenderer.hpp"
#include "entities/TargetObject.hpp"
#include "entities/MobileBeacon.hpp"
#include "sequences/mutable_array_sequence.hpp"
#include <algorithm>

void NavigationRenderer::render(sf::RenderWindow& window, const INavigator* navigator, const IEnvironment* env) const {
    if (!navigator || !env) return;

    // Лямбда функция для отрисовки динамических сущностей
    auto drawEntity = [&](Point2D realPos, LocationResult est, sf::Color color) {
        // Отрисовка зоны погрешности (Вычисленная позиция)
        // минимальный визуал 2 пикселя даже при идеальной точности
        float radius = std::max(2.0f, static_cast<float>(est.errorRadius));
        sf::CircleShape errorCircle(radius);
        errorCircle.setOrigin(radius, radius);
        errorCircle.setPosition(static_cast<float>(est.estimatedPos.x), static_cast<float>(est.estimatedPos.y));
        
        sf::Color fillColor = color;
        fillColor.a = 50; // Делаем заливку полупрозрачной (Альфа-канал)
        errorCircle.setFillColor(fillColor);
        errorCircle.setOutlineColor(color);
        errorCircle.setOutlineThickness(1.0f);
        window.draw(errorCircle);

        // Отрисовка реальной позиции (Сплошная точка)
        sf::CircleShape realDot(4.0f);
        realDot.setOrigin(4.0f, 4.0f);
        realDot.setPosition(static_cast<float>(realPos.x), static_cast<float>(realPos.y));
        realDot.setFillColor(color);
        window.draw(realDot);
    };

    // Отрисовка Мобильных Маяков 
    const Sequence<MobileBeacon*>& beacons = navigator->getBeacons();
    for (int i = 0; i < beacons.get_length(); ++i) {
        const MobileBeacon* beacon = beacons[i];
        if (beacon) {
            drawEntity(beacon->getRealPosition(), beacon->getEstimation(), sf::Color(100, 150, 255));
        }
    }

    // Отрисовка Главной Цели 
    const TargetObject* target = navigator->getTarget();
    if (target) {
        drawEntity(target->getRealPosition(), target->getEstimation(), sf::Color::Red);
    }
}