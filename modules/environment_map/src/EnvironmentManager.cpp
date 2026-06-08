#include "../include/EnvironmentManager.hpp"
#include "../../../core/include/exceptions/EnvironmentExceptions.hpp"
#include "../../../core/include/physics/RadioPhysics.hpp"
#include <cmath>
#include <cstdint>

class PerlinNoise {
private:
    static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    static float lerp(float t, float a, float b) { return a + t * (b - a); }
    static float grad(int hash, float x, float y) {
        int h = hash & 3;
        float u = h < 2 ? x : y;
        float v = h < 2 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }
public:
    static int hash(int x, int y) {
        uint32_t a = static_cast<uint32_t>(x * 3284157443 ^ y * 19349663);
        a ^= a << 13; a ^= a >> 17; a ^= a << 5;
        return a & 255;
    }
    static double noise(double x, double y) {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        x -= std::floor(x);
        y -= std::floor(y);
        float u = fade(x);
        float v = fade(y);
        int A = hash(X, Y), B = hash(X + 1, Y);
        int C = hash(X, Y + 1), D = hash(X + 1, Y + 1);
        return lerp(v, lerp(u, grad(A, x, y), grad(B, x - 1, y)),
                       lerp(u, grad(C, x, y - 1), grad(D, x - 1, y - 1)));
    }
    static double get(double x, double y) {
        return (noise(x, y) + 1.0) / 2.0;
    }
    static double randomPos(double x, double y) {
        return static_cast<double>(hash(static_cast<int>(x), static_cast<int>(y))) / 255.0;
    }
};

//РЕАЛИЗАЦИЯ КЛАССА ENVIRONMENT MANAGER
EnvironmentManager::~EnvironmentManager() {
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
        Tile emptyTile; 
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
    double scaleNature = 0.03; 
    double scalePath = 0.015;   

    //Природа (озера, леса разной густоты)
    for (int y = 0; y < Chunk::CHUNK_SIZE; ++y) {
        for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) {
            double globalX = chunk->getX() * Chunk::CHUNK_SIZE + x;
            double globalY = chunk->getY() * Chunk::CHUNK_SIZE + y;

            double noiseNature = PerlinNoise::get(globalX * scaleNature, globalY * scaleNature);
            double noisePath = PerlinNoise::get(globalX * scalePath + 100.0, globalY * scalePath + 100.0);

            TileType type = TileType::EMPTY;
            int param = 0;

            if (noiseNature < 0.35) { 
                type = TileType::WATER;
            } else if (noiseNature > 0.55) { 
                type = TileType::FOREST;
                if (noiseNature > 0.75) param = 3;      
                else if (noiseNature > 0.65) param = 2; 
                else param = 1;                         
            }

            if (type != TileType::WATER) {
                if (std::abs(noisePath - 0.5) < 0.015) type = TileType::PATH; //Тонкие тропинки
            }
            
            chunk->setTile(x, y, type, param);
        }
    }

    //Здания 
    for (int y = 3; y < Chunk::CHUNK_SIZE - 5; ++y) {
        for (int x = 3; x < Chunk::CHUNK_SIZE - 5; ++x) {
            double globalX = chunk->getX() * Chunk::CHUNK_SIZE + x;
            double globalY = chunk->getY() * Chunk::CHUNK_SIZE + y;

            if (PerlinNoise::randomPos(globalX, globalY) > 0.99) {
                
                //Проверяем огромный радиус вокруг (чтобы дома не слипались)
                bool safeToBuild = true;
                for(int dy = -3; dy <= 4; dy++) {
                    for(int dx = -3; dx <= 4; dx++) {
                        TileType t = chunk->getTile(x + dx, y + dy).type;
                        if(t == TileType::WATER || t == TileType::PATH || t == TileType::WALL) {
                            safeToBuild = false; // Рядом вода, дорога или другой дом!
                        }
                    }
                }
                if (safeToBuild) {
                    int h = (PerlinNoise::hash(static_cast<int>(globalX), static_cast<int>(globalY)) % 6) + 1;
                    int shapeType = PerlinNoise::hash(static_cast<int>(globalX*2), static_cast<int>(globalY*2)) % 4;

                    // Базовый блок (2x1 тайла)
                    chunk->setTile(x, y, TileType::WALL, h);
                    chunk->setTile(x+1, y, TileType::WALL, h);

                    if (shapeType == 0) { // Квадрат 2x2
                        chunk->setTile(x, y+1, TileType::WALL, h);
                        chunk->setTile(x+1, y+1, TileType::WALL, h);
                    } else if (shapeType == 1) { // Длинный прямоугольник 3x1
                        chunk->setTile(x+2, y, TileType::WALL, h);
                    } else if (shapeType == 2) { // Г-образный (L-shape)
                        chunk->setTile(x, y+1, TileType::WALL, h);
                        chunk->setTile(x, y+2, TileType::WALL, h);
                    } else if (shapeType == 3) { // Т-образный (T-shape)
                        chunk->setTile(x+2, y, TileType::WALL, h);
                        chunk->setTile(x+1, y+1, TileType::WALL, h);
                        chunk->setTile(x+1, y+2, TileType::WALL, h);
                    }
                }
            }
        }
    }

    //Гарантированная вышка
    bool spawned = false;
    for (int y = 2; y < Chunk::CHUNK_SIZE - 3 && !spawned; ++y) {
        for (int x = 2; x < Chunk::CHUNK_SIZE - 3 && !spawned; ++x) {
            Tile t = chunk->getTile(x, y);
            
            if (t.type == TileType::EMPTY || t.type == TileType::PATH) {
                bool noWallsAround = true;
                for (int dy = -2; dy <= 3; ++dy) {
                    for (int dx = -2; dx <= 3; ++dx) {
                        if (chunk->getTile(x + dx, y + dy).type == TileType::WALL) noWallsAround = false;
                    }
                }

                if (noWallsAround) {
                    if (staticTowers.get_length() < 3 || PerlinNoise::randomPos(chunk->getX()+x, chunk->getY()+y) > 0.98) {
                        Point2D towerPos;
                        //Центрируем вышку ровно по тайлам
                        towerPos.x = (chunk->getX() * Chunk::CHUNK_SIZE + x) * Chunk::TILE_SIZE + Chunk::TILE_SIZE;
                        towerPos.y = (chunk->getY() * Chunk::CHUNK_SIZE + y) * Chunk::TILE_SIZE + Chunk::TILE_SIZE;
                        
                        staticTowers.append(towerPos);
                        
                        //Фундамент под вышкой 2x2 мелких тайла
                        chunk->setTile(x, y, TileType::TOWER_BASE, 0); 
                        chunk->setTile(x+1, y, TileType::TOWER_BASE, 0); 
                        chunk->setTile(x, y+1, TileType::TOWER_BASE, 0); 
                        chunk->setTile(x+1, y+1, TileType::TOWER_BASE, 0); 
                        
                        spawned = true; 
                    }
                }
            }
        }
    }
}

MutableArraySequence<Point2D> EnvironmentManager::getStaticTowers() const {
    return staticTowers; 
}

// double EnvironmentManager::calculateSignal(Point2D a, Point2D b) const {
//     double distanceSq = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
//     if (distanceSq < 1.0) distanceSq = 1.0; 
    
//     double baseSignal = 100000.0 / distanceSq; 
//     double totalTransmittance = 1.0; 
    
//     int x0 = static_cast<int>(std::floor(a.x / Chunk::TILE_SIZE));
//     int y0 = static_cast<int>(std::floor(a.y / Chunk::TILE_SIZE));
//     int x1 = static_cast<int>(std::floor(b.x / Chunk::TILE_SIZE));
//     int y1 = static_cast<int>(std::floor(b.y / Chunk::TILE_SIZE));

//     int dx = std::abs(x1 - x0);
//     int dy = -std::abs(y1 - y0);
//     int sx = x0 < x1 ? 1 : -1;
//     int sy = y0 < y1 ? 1 : -1;
//     int err = dx + dy;

//     while (true) {
//         Point2D worldPos{ static_cast<double>(x0 * Chunk::TILE_SIZE), static_cast<double>(y0 * Chunk::TILE_SIZE) };
//         Tile t = getTileAtWorldPos(worldPos);
        
//         totalTransmittance *= t.transmittance;

//         if (totalTransmittance < 0.001) {
//             return 0.0;
//         }

//         if (x0 == x1 && y0 == y1) break;
//         int e2 = 2 * err;
//         if (e2 >= dy) { err += dy; x0 += sx; }
//         if (e2 <= dx) { err += dx; y0 += sy; }
//     }

//     return baseSignal * totalTransmittance; 
// }

MutableArraySequence<RadioPath> EnvironmentManager::computePaths(Point2D tx, Point2D rx, double frequencyGHz) const {
    MutableArraySequence<RadioPath> paths;

    // 1. ПРЯМОЙ ЛУЧ (Line of Sight - LOS) 
    double distLOS = std::hypot(rx.x - tx.x, rx.y - tx.y);
    if (distLOS < 1.0) distLOS = 1.0;
    
    Point2D arrivalVecLOS = { (rx.x - tx.x) / distLOS, (rx.y - tx.y) / distLOS };
    double totalTransmittanceLOS = 1.0; 

    // Алгоритм DDA для прямого луча
    int x0 = static_cast<int>(std::floor(tx.x / Chunk::TILE_SIZE));
    int y0 = static_cast<int>(std::floor(tx.y / Chunk::TILE_SIZE));
    int x1 = static_cast<int>(std::floor(rx.x / Chunk::TILE_SIZE));
    int y1 = static_cast<int>(std::floor(rx.y / Chunk::TILE_SIZE));

    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    // Флаг: врезались ли мы в стену по пути (используем для отражения)
    bool hitWall = false;
    Point2D wallHitPos = {0, 0};

    while (true) {
        Point2D worldPos{ static_cast<double>(x0 * Chunk::TILE_SIZE), static_cast<double>(y0 * Chunk::TILE_SIZE) };
        Tile t = getTileAtWorldPos(worldPos);
        
        // Физическое затухание
        totalTransmittanceLOS *= RadioPhysics::getTileTransmittance(t.type, frequencyGHz);

        // Запоминаем первую встреченную стену для расчета отражений
        if (t.type == TileType::WALL && !hitWall) {
            hitWall = true;
            wallHitPos = worldPos;
        }

        if (totalTransmittanceLOS < 0.001) break;
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }

    if (totalTransmittanceLOS >= 0.001) {
        paths.append(RadioPath{ distLOS, arrivalVecLOS, totalTransmittanceLOS, 0 });
    }

    // 2. ОТРАЖЕННЫЙ ОТ ЗЕМЛИ ЛУЧ (Ground Bounce) 
    // Постоянно присутствует в радиоэфире, создает замирания
    double h_tx = 10.0; 
    double h_rx = 2.0;  
    double distGround = std::sqrt(distLOS * distLOS + (h_tx + h_rx) * (h_tx + h_rx));
    paths.append(RadioPath{ distGround, arrivalVecLOS, totalTransmittanceLOS * 0.7, 1 });

    // 3. МЕТОД МНИМЫХ ИСТОЧНИКОВ (Wall Bounce - ISM) 
    // Если по пути мы нашли стену, то посчитаем отражение от неё
    if (hitWall) {
        double wallX = wallHitPos.x + Chunk::TILE_SIZE / 2.0; 
        Point2D virtualTx = { wallX + (wallX - tx.x), tx.y };
        double distBounce = std::hypot(rx.x - virtualTx.x, rx.y - virtualTx.y);
        if (distBounce > 1.0) {
            Point2D bounceArrivalVec = { (rx.x - virtualTx.x) / distBounce, (rx.y - virtualTx.y) / distBounce };
            double cosTheta = std::abs(bounceArrivalVec.x); 
            double reflectionCoeff = RadioPhysics::getReflectionCoefficient(TileType::WALL, cosTheta, frequencyGHz);
            if (reflectionCoeff > 0.05) {
                double finalBounceAtten = totalTransmittanceLOS * reflectionCoeff;
                if (finalBounceAtten > 0.001) {
                    paths.append(RadioPath{ distBounce, bounceArrivalVec, finalBounceAtten, 1 });
                }
            }
        }
    }
    return paths;
}