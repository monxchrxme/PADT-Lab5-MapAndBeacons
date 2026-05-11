#include "../include/MapRenderer.hpp"
#include <algorithm> // Обязательно для std::clamp

void MapRenderer::render(sf::RenderWindow& window, const EnvironmentManager* envManager) const {
    if (!envManager) return;

    // ОТРИСОВКА ТАЙЛОВ КАРТЫ
    const auto& chunks = envManager->getActiveChunks();
    sf::RectangleShape tileShape(sf::Vector2f(static_cast<float>(Chunk::TILE_SIZE), static_cast<float>(Chunk::TILE_SIZE)));

    for (int i = 0; i < chunks.get_length(); ++i) {
        Chunk* chunk = chunks[i];
        if (!chunk) continue;

        double baseWorldX = chunk->getX() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
        double baseWorldY = chunk->getY() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;

        for (int localY = 0; localY < Chunk::CHUNK_SIZE; ++localY) {
            for (int localX = 0; localX < Chunk::CHUNK_SIZE; ++localX) {
                Tile tile = chunk->getTile(localX, localY);

                float screenX = static_cast<float>(baseWorldX + localX * Chunk::TILE_SIZE);
                float screenY = static_cast<float>(baseWorldY + localY * Chunk::TILE_SIZE);

                // ФАКТУРА (Шум для цвета)
                int worldIntX = static_cast<int>(baseWorldX/Chunk::TILE_SIZE) + localX;
                int worldIntY = static_cast<int>(baseWorldY/Chunk::TILE_SIZE) + localY;
                int colorVariation = ((worldIntX * 374761393 + worldIntY * 668265263) % 31) - 15;
                //int colorVariation = 0;

                auto applyTexture = [&](int r, int g, int b, int a = 255) {
                    r = std::clamp(r + colorVariation, 0, 255);
                    g = std::clamp(g + colorVariation, 0, 255);
                    b = std::clamp(b + colorVariation, 0, 255);
                    return sf::Color(r, g, b, a);
                };

                // Назначаем базовые цвета + накладываем фактуру
                switch (tile.type) {
                    case TileType::EMPTY:  
                        tileShape.setFillColor(applyTexture(50, 160, 60)); break; 
                    case TileType::FOREST: 
                        tileShape.setFillColor(applyTexture(20, 100, 30)); break; 
                    case TileType::PATH:   
                        tileShape.setFillColor(applyTexture(139, 100, 60)); break;
                    case TileType::WATER:  
                        tileShape.setFillColor(sf::Color(60, 120, 220, 220)); break; 
                    case TileType::WALL:   
                        tileShape.setFillColor(sf::Color(140, 140, 140, 255)); break; 
                    case TileType::TOWER_BASE: 
                        tileShape.setFillColor(applyTexture(50, 160, 60)); break; 
                }

                tileShape.setPosition(screenX, screenY);
                window.draw(tileShape);
            }
        }
    }

    // ОТРИСОВКА СТАЦИОНАРНЫХ ВЫШЕК 
    auto towers = envManager->getStaticTowers();
    
    sf::CircleShape towerBase(15.0f); 
    towerBase.setOrigin(15.0f, 15.0f);
    towerBase.setFillColor(sf::Color(0, 255, 0)); 
    towerBase.setOutlineThickness(3.0f);
    towerBase.setOutlineColor(sf::Color::Black);

    sf::CircleShape towerLight(5.0f);
    towerLight.setOrigin(5.0f, 5.0f);
    towerLight.setFillColor(sf::Color::Red);

    for (int i = 0; i < towers.get_length(); ++i) {
        Point2D pos = towers[i]; 
        towerBase.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
        window.draw(towerBase);
        towerLight.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
        window.draw(towerLight);
    }
}