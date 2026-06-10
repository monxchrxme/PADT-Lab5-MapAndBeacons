#include "../include/MapRenderer.hpp"
#include <algorithm>

void MapRenderer::render(sf::RenderWindow& window, const EnvironmentManager* envManager) const 
{
    if (!envManager) 
    {
        return;
    }

    const auto& chunks = envManager->getActiveChunks();

    for (int i = 0; i < chunks.get_length(); ++i) 
    {
        Chunk* chunk = chunks[i];
        if (!chunk) 
        {
            continue;
        }
        const int tilesCount = Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE;
        sf::Vertex vertices[tilesCount * 4];

        double baseWorldX = chunk->getX() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
        double baseWorldY = chunk->getY() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;

        int vertexIndex = 0;

        for (int localY = 0; localY < Chunk::CHUNK_SIZE; ++localY) 
        {
            for (int localX = 0; localX < Chunk::CHUNK_SIZE; ++localX) 
            {
                Tile tile = chunk->getTile(localX, localY);

                float screenX = static_cast<float>(baseWorldX + localX * Chunk::TILE_SIZE);
                float screenY = static_cast<float>(baseWorldY + localY * Chunk::TILE_SIZE);
                float size = static_cast<float>(Chunk::TILE_SIZE);

                //Фактура
                int worldIntX = static_cast<int>(baseWorldX/Chunk::TILE_SIZE) + localX;
                int worldIntY = static_cast<int>(baseWorldY/Chunk::TILE_SIZE) + localY;
                int colorVariation = ((worldIntX * 374761393 + worldIntY * 668265263) % 21) - 10;

                auto applyTexture = [&](int r, int g, int b) 
                {
                    r = std::clamp(r + colorVariation, 0, 255);
                    g = std::clamp(g + colorVariation, 0, 255);
                    b = std::clamp(b + colorVariation, 0, 255);
                    return sf::Color(r, g, b, 255);
                };

                sf::Color tileColor;

                //Цвета 
                switch (tile.type) 
                {
                    case TileType::EMPTY:  tileColor = applyTexture(85, 180, 85); 
                    {
                        break; 
                    }
                    case TileType::PATH:   tileColor = applyTexture(170, 140, 90); 
                    {
                        break; 
                    }
                    case TileType::WATER:  tileColor = sf::Color(60, 140, 220); 
                    {
                        break; 
                    }
                    case TileType::TOWER_BASE: tileColor = applyTexture(85, 180, 85); 
                    {
                        break; 
                    }
                    case TileType::FOREST: 
                    {
                        int greenAmount = 140 - (tile.height * 30); 
                        tileColor = applyTexture(20, greenAmount, 20);
                        break;
                    }
                    case TileType::WALL: 
                    {
                        int grayAmount = 60 + (tile.height * 25); 
                        tileColor = sf::Color(grayAmount, grayAmount, grayAmount);
                        break;
                    }
                }
                //Верхний левый
                vertices[vertexIndex].position = sf::Vector2f(screenX, screenY);
                vertices[vertexIndex].color = tileColor;
                //Верхний правый
                vertices[vertexIndex + 1].position = sf::Vector2f(screenX + size, screenY);
                vertices[vertexIndex + 1].color = tileColor;
                //Нижний правый
                vertices[vertexIndex + 2].position = sf::Vector2f(screenX + size, screenY + size);
                vertices[vertexIndex + 2].color = tileColor;
                //Нижний левый
                vertices[vertexIndex + 3].position = sf::Vector2f(screenX, screenY + size);
                vertices[vertexIndex + 3].color = tileColor;

                vertexIndex += 4;
            }
        }
        
        window.draw(vertices, tilesCount * 4, sf::Quads);
    }

    //Отрисовка вышек
    auto towers = envManager->getStaticTowers();
    
    sf::RectangleShape towerBase(sf::Vector2f(24.0f, 24.0f)); 
    towerBase.setOrigin(12.0f, 12.0f); //Центр квадрата
    towerBase.setFillColor(sf::Color(0, 255, 0)); 
    towerBase.setOutlineThickness(2.0f);
    towerBase.setOutlineColor(sf::Color::Black);

    sf::RectangleShape towerLight(sf::Vector2f(8.0f, 8.0f));
    towerLight.setOrigin(4.0f, 4.0f);
    towerLight.setFillColor(sf::Color::Red);

    for (int i = 0; i < towers.get_length(); ++i) 
    {
        Point2D pos = towers[i]; 
        towerBase.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
        window.draw(towerBase);
        towerLight.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
        window.draw(towerLight);
    }
}