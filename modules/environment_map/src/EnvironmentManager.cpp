#include "../include/EnvironmentManager.hpp"
#include "../../../core/include/exceptions/EnvironmentExceptions.hpp"
#include <cmath>
#include <cstdint>


// Простой хэш для координат
static double hash2D(int x, int y) {
    uint32_t a = static_cast<uint32_t>(x * 3284157443 ^ y * 19349663 + 719323);
    a ^= a << 13; a ^= a >> 17; a ^= a << 5;
    return static_cast<double>(a) / 4294967295.0;
}

// Плавный шум (Value Noise + Smoothstep) для генерации лесов и озер
static double smoothNoise(double x, double y) {
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    double tx = x - xi;
    double ty = y - yi;
    
    double u = tx * tx * (3.0 - 2.0 * tx);
    double v = ty * ty * (3.0 - 2.0 * ty);

    double a = hash2D(xi, yi);
    double b = hash2D(xi + 1, yi);
    double c = hash2D(xi, yi + 1);
    double d = hash2D(xi + 1, yi + 1);

    return a*(1.0-u)*(1.0-v) + b*u*(1.0-v) + c*(1.0-u)*v + d*u*v;
}

// РЕАЛИЗАЦИЯ КЛАССА ENVIRONMENT MANAGER

EnvironmentManager::~EnvironmentManager() {
    // Безопасно очищаем все чанки, чтобы избежать утечек
    for (int i = 0; i < activeChunks.get_length(); ++i) {
        delete activeChunks[i];
    }
}

Chunk* EnvironmentManager::getChunkAt(int chunkX, int chunkY) const {
    for (int i = 0; i < activeChunks.get_length(); ++i) {
        Chunk* c = activeChunks[i];
        if (c->getX() == chunkX && c->getY() == chunkY) return c;
    }
    return nullptr;
}

Tile EnvironmentManager::getTileAtWorldPos(Point2D p) const {
    int chunkX = static_cast<int>(std::floor(p.x / (Chunk::CHUNK_SIZE * Chunk::TILE_SIZE)));
    int chunkY = static_cast<int>(std::floor(p.y / (Chunk::CHUNK_SIZE * Chunk::TILE_SIZE)));

    Chunk* chunk = getChunkAt(chunkX, chunkY);
    if (!chunk) {
        Tile emptyTile; // Если чанк не подгружен, возвращаем пустоту
        return emptyTile;
    }

    double modX = std::fmod(p.x, Chunk::CHUNK_SIZE * Chunk::TILE_SIZE);
    double modY = std::fmod(p.y, Chunk::CHUNK_SIZE * Chunk::TILE_SIZE);
    if (modX < 0) modX += Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
    if (modY < 0) modY += Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;

    int localX = static_cast<int>(modX / Chunk::TILE_SIZE);
    int localY = static_cast<int>(modY / Chunk::TILE_SIZE);

    return chunk->getTile(localX, localY);
}

bool EnvironmentManager::isPassable(Point2D p) const {
    return getTileAtWorldPos(p).isPassable;
}

void EnvironmentManager::triggerLazyGeneration(Point2D p) {
    int chunkX = static_cast<int>(std::floor(p.x / (Chunk::CHUNK_SIZE * Chunk::TILE_SIZE)));
    int chunkY = static_cast<int>(std::floor(p.y / (Chunk::CHUNK_SIZE * Chunk::TILE_SIZE)));

    // Генерируем чанк, в котором стоит объект, и 8 соседей вокруг (радиус 1)
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int cx = chunkX + dx;
            int cy = chunkY + dy;
            
            if (!getChunkAt(cx, cy)) {
                Chunk* newChunk = new Chunk(cx, cy);
                generateChunkData(newChunk);
                activeChunks.append(newChunk);
            }
        }
    }
}

void EnvironmentManager::generateChunkData(Chunk* chunk) {
    double scaleNature = 0.10; // Изменили масштаб для более округлых форм
    double scalePath = 0.05;   

    // Природа (Круглые Озера, Леса и Тропинки)
    for (int y = 0; y < Chunk::CHUNK_SIZE; ++y) {
        for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) {
            double globalX = chunk->getX() * Chunk::CHUNK_SIZE + x;
            double globalY = chunk->getY() * Chunk::CHUNK_SIZE + y;

            double noiseNature = smoothNoise(globalX * scaleNature, globalY * scaleNature);
            double noisePath = smoothNoise(globalX * scalePath + 100.0, globalY * scalePath + 100.0);

            TileType type = TileType::EMPTY;

            // Воды стало меньше (< 0.15), и она будет более круглой
            if (noiseNature < 0.15) {
                type = TileType::WATER;
            } else if (noiseNature > 0.65) {
                type = TileType::FOREST;
            }

            // Тропинки
            if (type != TileType::WATER) {
                if (std::abs(noisePath - 0.5) < 0.04) {
                    type = TileType::PATH;
                }
            }
            
            chunk->setTile(x, y, type);
        }
    }

    // Здания разной формы
    for (int y = 2; y < Chunk::CHUNK_SIZE - 4; ++y) {
        for (int x = 2; x < Chunk::CHUNK_SIZE - 4; ++x) {
            double globalX = chunk->getX() * Chunk::CHUNK_SIZE + x;
            double globalY = chunk->getY() * Chunk::CHUNK_SIZE + y;

            if (hash2D(static_cast<int>(globalX), static_cast<int>(globalY)) > 0.99) {
                bool safeToBuild = true;
                for(int dy = -1; dy <= 3; dy++) {
                    for(int dx = -1; dx <= 3; dx++) {
                        TileType t = chunk->getTile(x + dx, y + dy).type;
                        if(t == TileType::WATER || t == TileType::PATH) safeToBuild = false;
                    }
                }

                if (safeToBuild) {
                    int shapeType = static_cast<int>(hash2D(globalX*2, globalY*2) * 100) % 4;
                    chunk->setTile(x, y, TileType::WALL);
                    chunk->setTile(x+1, y, TileType::WALL);

                    if (shapeType == 0) {
                        chunk->setTile(x, y+1, TileType::WALL);
                        chunk->setTile(x+1, y+1, TileType::WALL);
                    } else if (shapeType == 1) {
                        chunk->setTile(x+2, y, TileType::WALL);
                    } else if (shapeType == 2) {
                        chunk->setTile(x, y+1, TileType::WALL);
                        chunk->setTile(x, y+2, TileType::WALL);
                    } else if (shapeType == 3) {
                        chunk->setTile(x+2, y, TileType::WALL);
                        chunk->setTile(x+1, y+1, TileType::WALL);
                        chunk->setTile(x+1, y+2, TileType::WALL);
                    }
                }
            }
        }
    }

    // Гарантированная вышка 
    bool spawned = false;
    for (int y = 2; y < Chunk::CHUNK_SIZE - 2 && !spawned; ++y) {
        for (int x = 2; x < Chunk::CHUNK_SIZE - 2 && !spawned; ++x) {
            Tile t = chunk->getTile(x, y);
            
            if (t.type == TileType::EMPTY || t.type == TileType::PATH) {
                if (staticTowers.get_length() < 3 || hash2D(chunk->getX()+x, chunk->getY()+y) > 0.95) {
                    Point2D towerPos;
                    towerPos.x = (chunk->getX() * Chunk::CHUNK_SIZE + x) * Chunk::TILE_SIZE + (Chunk::TILE_SIZE / 2.0);
                    towerPos.y = (chunk->getY() * Chunk::CHUNK_SIZE + y) * Chunk::TILE_SIZE + (Chunk::TILE_SIZE / 2.0);
                    
                    staticTowers.append(towerPos);
                    chunk->setTile(x, y, TileType::TOWER_BASE); 
                    
                    spawned = true; 
                }
            }
        }
    }
}

MutableArraySequence<Point2D> EnvironmentManager::getStaticTowers() const {
    return staticTowers; 
}

double EnvironmentManager::calculateSignal(Point2D a, Point2D b) const {
    // Считаем идеальный физический сигнал в вакууме
    double distanceSq = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
    if (distanceSq < 1.0) distanceSq = 1.0; 
    
    double baseSignal = 100000.0 / distanceSq; // Базовая мощность вышки

    // Считаем затухание от тайлов (Raycasting Брезенхема)
    double totalTransmittance = 1.0; 
    
    int x0 = static_cast<int>(std::floor(a.x / Chunk::TILE_SIZE));
    int y0 = static_cast<int>(std::floor(a.y / Chunk::TILE_SIZE));
    int x1 = static_cast<int>(std::floor(b.x / Chunk::TILE_SIZE));
    int y1 = static_cast<int>(std::floor(b.y / Chunk::TILE_SIZE));

    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        Point2D worldPos{ static_cast<double>(x0 * Chunk::TILE_SIZE), static_cast<double>(y0 * Chunk::TILE_SIZE) };
        Tile t = getTileAtWorldPos(worldPos);
        
        // Умножаем пропускную способность
        totalTransmittance *= t.transmittance;

        // Если сигнал почти умер - обрываем вычисления
        if (totalTransmittance < 0.001) {
            return 0.0;
        }

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }

    // Возвращаем искаженный физический сигнал
    return baseSignal * totalTransmittance; 
}