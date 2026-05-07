#include "../include/MapRenderer.hpp"

void MapRenderer::render(sf::RenderWindow& window, const EnvironmentManager* envManager) const {
    if (!envManager) return;

    const auto& chunks = envManager->getActiveChunks();
    
    // Предсоздаем прямоугольник для отрисовки для экономии ресурсов
    sf::RectangleShape tileShape(sf::Vector2f(static_cast<float>(Chunk::TILE_SIZE), static_cast<float>(Chunk::TILE_SIZE)));

    for (int i = 0; i < chunks.get_length(); ++i) {
        Chunk* chunk = chunks[i];
        if (!chunk) continue;

        double baseWorldX = chunk->getX() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
        double baseWorldY = chunk->getY() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;

        for (int localY = 0; localY < Chunk::CHUNK_SIZE; ++localY) {
            for (int localX = 0; localX < Chunk::CHUNK_SIZE; ++localX) {
                Tile tile = chunk->getTile(localX, localY);

                // Назначаем цвета тайлам
                switch (tile.type) {
                    case TileType::EMPTY:  tileShape.setFillColor(sf::Color(40, 40, 40));      break; // Темно-серый фон поля
                    case TileType::FOREST: tileShape.setFillColor(sf::Color(34, 139, 34, 180));  break; // Зеленый лес
                    case TileType::WATER:  tileShape.setFillColor(sf::Color(65, 105, 225, 180)); break; // Синяя вода
                    case TileType::WALL:   tileShape.setFillColor(sf::Color(169, 169, 169, 255));break; // Бетон
                }

                float screenX = static_cast<float>(baseWorldX + localX * Chunk::TILE_SIZE);
                float screenY = static_cast<float>(baseWorldY + localY * Chunk::TILE_SIZE);
                
                tileShape.setPosition(screenX, screenY);
                window.draw(tileShape);
            }
        }
    }
// === 2. ОТРИСОВКА СТАЦИОНАРНЫХ ВЫШЕК ===
    // Получаем массив вышек ПО ЗНАЧЕНИЮ (без звездочки)
    auto towers = envManager->getStaticTowers();
    
    if (towers.get_length() > 0) {
        sf::CircleShape towerBase(8.0f);
        towerBase.setOrigin(8.0f, 8.0f);
        towerBase.setFillColor(sf::Color(50, 205, 50)); 
        towerBase.setOutlineThickness(2.0f);
        towerBase.setOutlineColor(sf::Color::Black);

        sf::CircleShape towerLight(3.0f);
        towerLight.setOrigin(3.0f, 3.0f);
        towerLight.setFillColor(sf::Color::Red);

        for (int i = 0; i < towers.get_length(); ++i) {
            Point2D pos = towers[i]; // Обращаемся напрямую к элементу

            towerBase.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
            window.draw(towerBase);

            towerLight.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
            window.draw(towerLight);
        }
    }
}