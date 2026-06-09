#include "NavigationRenderer.hpp"
#include "entities/TargetObject.hpp"
#include "entities/MobileBeacon.hpp"
#include "sequences/mutable_array_sequence.hpp"
#include <algorithm>
#include "Structures.hpp"

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

    // 4. Отрисовка Mesh-сети (Лазерные линии передачи) 
    if (g_Settings.showMeshNetwork) {
        MutableArraySequence<NetworkLink> links = navigator->getActiveLinks();
        for (int i = 0; i < links.get_length(); ++i) {
            NetworkLink link = links[i];
            
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(static_cast<float>(link.from.x), static_cast<float>(link.from.y))),
                sf::Vertex(sf::Vector2f(static_cast<float>(link.to.x), static_cast<float>(link.to.y)))
            };

            sf::Uint8 alpha = static_cast<sf::Uint8>((link.lifeTime / 0.3f) * 255.0f);
            sf::Color linkColor;

            // Выбираем цвет в зависимости от типа связи
            switch (link.type) {
                case LinkType::TargetToBeacon: linkColor = sf::Color(255, 255, 0, alpha); break;   // Желтый
                case LinkType::BeaconToBeacon: linkColor = sf::Color(0, 255, 255, alpha); break;   // Голубой (Cyan)
                case LinkType::BeaconToTower:  linkColor = sf::Color(0, 255, 0, alpha); break;     // Зеленый
            }

            line[0].color = linkColor;
            line[1].color = linkColor;

            window.draw(line, 2, sf::Lines);
        }
    }
}