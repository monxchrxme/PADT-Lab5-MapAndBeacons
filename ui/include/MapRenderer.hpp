#pragma once
#include <SFML/Graphics.hpp>
#include "../../modules/environment_map/include/EnvironmentManager.hpp"

class MapRenderer {
public:
    MapRenderer() = default;

    // Отрисовываем тайлы, принимая конкретный менеджер
    void render(sf::RenderWindow& window, const EnvironmentManager* envManager) const;
};