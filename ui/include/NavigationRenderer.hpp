#pragma once
#include <SFML/Graphics.hpp>
#include "INavigator.hpp"
#include "IEnvironment.hpp"

class NavigationRenderer {
public:
    NavigationRenderer() = default;

    void render(sf::RenderWindow& window, const INavigator* navigator, const IEnvironment* env) const;
};