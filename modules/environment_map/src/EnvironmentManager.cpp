#include "../include/EnvironmentManager.hpp"
#include "../../../core/include/exceptions/EnvironmentExceptions.hpp"
#include "../../../core/include/physics/RadioPhysics.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio> 

class PerlinNoise 
{
private:
    static float fade(float t) 
    { 
        return t * t * t * (t * (t * 6 - 15) + 10); 
    }
    static float lerp(float t, float a, float b) 
    { 
        return a + t * (b - a); 
    }
    static float grad(int hash, float x, float y) 
    {
        int h = hash & 3; 
        float u = h < 2 ? x : y; 
        float v = h < 2 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }
public:
    static int hash(int x, int y) 
    {
        uint32_t a = static_cast<uint32_t>(x * 3284157443 ^ y * 19349663);
        a ^= a << 13; a ^= a >> 17; a ^= a << 5;
        return a & 255;
    }
    static double noise(double x, double y) 
    {
        int X = static_cast<int>(std::floor(x)) & 255; 
        int Y = static_cast<int>(std::floor(y)) & 255;
        x -= std::floor(x); y -= std::floor(y);
        float u = fade(static_cast<float>(x)); 
        float v = fade(static_cast<float>(y));
        int A = hash(X, Y);
        int B = hash((X + 1) & 255, Y);
        int C = hash(X, (Y + 1) & 255);
        int D = hash((X + 1) & 255, (Y + 1) & 255);
        
        return lerp(v, lerp(u, grad(A, (float)x, (float)y), grad(B, (float)x - 1, (float)y)), 
                       lerp(u, grad(C, (float)x, (float)y - 1), grad(D, (float)x - 1, (float)y - 1)));
    }
    static double get(double x, double y) 
    { 
        return (noise(x, y) + 1.0) / 2.0; 
    }
    static double randomPos(double x, double y) 
    {
        return static_cast<double>(hash(static_cast<int>(x), static_cast<int>(y))) / 255.0;
    }
};

//Конструктор
EnvironmentManager::EnvironmentManager(Mode mode) : currentMode(mode) {}

EnvironmentManager::~EnvironmentManager() 
{
    saveWorldToDisk();
    for (int i = 0; i < activeChunks.get_length(); ++i) 
    {
        delete activeChunks[i];
    }
}

bool EnvironmentManager::isChunkInHistory(int cx, int cy) const 
{
    for (int i = 0; i < exploredChunksHistory.get_length(); ++i) 
    {
        if (exploredChunksHistory[i].x == cx && exploredChunksHistory[i].y == cy) 
        {
            return true;
        }
    }
    return false;
}

//Работа с диском (world_save.bin)
void EnvironmentManager::saveWorldToDisk() const 
{
    FILE* file = std::fopen("world_save.bin", "wb");
        if (!file) 
        {
            return;
        }
        //Сохраняем Вышки
        int towerCount = staticTowers.get_length();
        std::fwrite(&towerCount, sizeof(int), 1, file); 
        
        for (int i = 0; i < towerCount; ++i) 
        {
            Point2D pos = staticTowers[i];
            std::fwrite(&pos, sizeof(Point2D), 1, file); 
        }

        //Сохраняем координаты посещенных чанков
        int historyCount = exploredChunksHistory.get_length();
        std::fwrite(&historyCount, sizeof(int), 1, file);
        
        for (int i = 0; i < historyCount; ++i) 
        {
            std::fwrite(&exploredChunksHistory[i], sizeof(ChunkCoord), 1, file);
        }

        std::fclose(file);
}

void EnvironmentManager::loadWorldFromDisk() 
{
    FILE* file = std::fopen("world_save.bin", "rb");
    if (!file) 
    {
        throw EnvironmentException("No saved world found (world_save.bin is missing)");
    }

    //Читаем вышки
    int towerCount = 0;
    if (std::fread(&towerCount, sizeof(int), 1, file) == 1) 
    {
        for (int i = 0; i < towerCount; ++i) 
        {
            Point2D pos;
            std::fread(&pos, sizeof(Point2D), 1, file);
            staticTowers.append(pos); 
        }
    }

    int historyCount = 0;
    if (std::fread(&historyCount, sizeof(int), 1, file) == 1) 
    {
        for (int i = 0; i < historyCount; ++i) 
        {
            ChunkCoord coord;
            std::fread(&coord, sizeof(ChunkCoord), 1, file);
            exploredChunksHistory.append(coord);

            //Генерация сохраненного чанка 
            Chunk* chunk = new Chunk(coord.x, coord.y);
            generateChunkData(chunk);
            activeChunks.append(chunk);
        }
    }
    std::fclose(file);
}

void EnvironmentManager::unloadDistantChunks(Point2D center, double maxDistance) 
{
    for (int i = 0; i < activeChunks.get_length(); ) 
    {
        Chunk* c = activeChunks[i];
        double cx = c->getX() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
        double cy = c->getY() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
        double distSq = (cx - center.x)*(cx - center.x) + (cy - center.y)*(cy - center.y);

        if (distSq > maxDistance * maxDistance) 
        {
            delete c;
            activeChunks.remove_at(i); 
        } else 
        {
            ++i;
        }
    }
}
//Основная логика
Chunk* EnvironmentManager::getChunkAt(int chunkX, int chunkY) const 
{
    for (int i = 0; i < activeChunks.get_length(); ++i) 
    {
        if (activeChunks[i]->getX() == chunkX && activeChunks[i]->getY() == chunkY) 
        {
            return activeChunks[i];
        }
    }
    return nullptr;
}

Tile EnvironmentManager::getTileAtWorldPos(Point2D p) const 
{
    int cx = static_cast<int>(std::floor(p.x / (Chunk::CHUNK_SIZE * Chunk::TILE_SIZE)));
    int cy = static_cast<int>(std::floor(p.y / (Chunk::CHUNK_SIZE * Chunk::TILE_SIZE)));
    Chunk* chunk = getChunkAt(cx, cy);
    if (!chunk) 
    {
        Tile voidTile;
        voidTile.type = TileType::EMPTY; 
        voidTile.isPassable = false;
        voidTile.transmittance = 1.0; 
        return voidTile;
    }
    double modX = std::fmod(p.x, Chunk::CHUNK_SIZE * Chunk::TILE_SIZE);
    double modY = std::fmod(p.y, Chunk::CHUNK_SIZE * Chunk::TILE_SIZE);
    if (modX < 0) 
    {
        modX += Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
    }
    if (modY < 0) 
    {
        modY += Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
    }
    return chunk->getTile(static_cast<int>(modX / Chunk::TILE_SIZE), static_cast<int>(modY / Chunk::TILE_SIZE));
}

bool EnvironmentManager::isPassable(Point2D p) const 
{ 
    return getTileAtWorldPos(p).isPassable; 
}

void EnvironmentManager::triggerLazyGeneration(Point2D p) 
{
    int chunkX = static_cast<int>(std::floor(p.x / (Chunk::CHUNK_SIZE * Chunk::TILE_SIZE)));
    int chunkY = static_cast<int>(std::floor(p.y / (Chunk::CHUNK_SIZE * Chunk::TILE_SIZE)));

    for (int dx = -2; dx <= 2; ++dx) 
    {
        for (int dy = -2; dy <= 2; ++dy) 
        {
            int cx = chunkX + dx;
            int cy = chunkY + dy;
            
            if (!getChunkAt(cx, cy)) 
            {
                if (currentMode == Mode::LOAD_GAME) 
                {
                    continue;
                }
                Chunk* newChunk = new Chunk(cx, cy);
                generateChunkData(newChunk);
                activeChunks.append(newChunk);

                if (!isChunkInHistory(cx, cy)) 
                {
                    ChunkCoord newCoord;
                    newCoord.x = cx;
                    newCoord.y = cy;
                    exploredChunksHistory.append(newCoord);
                }
            }
        }
    }
    if (currentMode != Mode::LOAD_GAME) 
    {
        unloadDistantChunks(p, 4000.0);
    }
}

void EnvironmentManager::generateChunkData(Chunk* chunk) 
{
    double scaleNature = 0.03; 
    double scalePath = 0.015;   
    for (int y = 0; y < Chunk::CHUNK_SIZE; ++y) 
    {
        for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) 
        {
            double globalX = (chunk->getX() * Chunk::CHUNK_SIZE + x) + 0.5;
            double globalY = (chunk->getY() * Chunk::CHUNK_SIZE + y) + 0.5;
            double noiseNature = PerlinNoise::get(globalX * scaleNature, globalY * scaleNature);
            double noisePath = PerlinNoise::get(globalX * scalePath + 100.0, globalY * scalePath + 100.0);

            TileType type = TileType::EMPTY;
            int param = 0;

            if (noiseNature < 0.35) 
            { 
                type = TileType::WATER;
            } else if (noiseNature > 0.55) 
            { 
                type = TileType::FOREST;
                if (noiseNature > 0.75) 
                {
                    param = 3;
                }else if (noiseNature > 0.65) 
                {
                    param = 2; 
                }else 
                {
                    param = 1;  
                }                       
            }

            if (type != TileType::WATER) 
            {
                if (std::abs(noisePath - 0.5) < 0.015) type = TileType::PATH; 
            }
            
            chunk->setTile(x, y, type, param);
        }
    }
    for (int y = 3; y < Chunk::CHUNK_SIZE - 5; ++y) 
    {
        for (int x = 3; x < Chunk::CHUNK_SIZE - 5; ++x) 
        {
            double globalX = (chunk->getX() * Chunk::CHUNK_SIZE + x) + 0.5;
            double globalY = (chunk->getY() * Chunk::CHUNK_SIZE + y) + 0.5;
            if (PerlinNoise::randomPos(globalX, globalY) > 0.99) 
            {
                bool safe = true;
                for(int dy = -3; dy <= 4; dy++) 
                    for(int dx = -3; dx <= 4; dx++) 
                        if(chunk->getTile(x + dx, y + dy).type != TileType::EMPTY) 
                        {
                            safe = false;
                        }
                if (safe) 
                {
                    int h = (PerlinNoise::hash(static_cast<int>(globalX), static_cast<int>(globalY)) % 6) + 1;
                    int s = PerlinNoise::hash(static_cast<int>(globalX*2), static_cast<int>(globalY*2)) % 4;
                    
                    chunk->setTile(x, y, TileType::WALL, h); 
                    chunk->setTile(x+1, y, TileType::WALL, h);
                    if (s == 0) 
                    { 
                        chunk->setTile(x, y+1, TileType::WALL, h); 
                        chunk->setTile(x+1, y+1, TileType::WALL, h); 
                    }else if (s == 1) 
                    {
                        chunk->setTile(x+2, y, TileType::WALL, h);
                    }else if (s == 2) 
                    { 
                        chunk->setTile(x, y+1, TileType::WALL, h); 
                        chunk->setTile(x, y+2, TileType::WALL, h); 
                    }
                    else if (s == 3) 
                    { 
                        chunk->setTile(x+2, y, TileType::WALL, h); 
                        chunk->setTile(x+1, y+1, TileType::WALL, h); 
                        chunk->setTile(x+1, y+2, TileType::WALL, h); 
                    }
                }
            }
        }
    }
    bool spawned = false;
    for (int y = 2; y < Chunk::CHUNK_SIZE - 3 && !spawned; ++y) 
    {
        for (int x = 2; x < Chunk::CHUNK_SIZE - 3 && !spawned; ++x) 
        {
            Tile t = chunk->getTile(x, y);
            if (t.type == TileType::EMPTY || t.type == TileType::PATH) 
            {
                bool noWalls = true;
                for (int dy = -2; dy <= 3; ++dy) 
                    for (int dx = -2; dx <= 3; ++dx) 
                        if (chunk->getTile(x + dx, y + dy).type == TileType::WALL) 
                        {
                            noWalls = false;
                        }
                if (noWalls) 
                {
                    if (PerlinNoise::randomPos(chunk->getX()+x, chunk->getY()+y) > 0.98) 
                        {
                            Point2D tp;
                            tp.x = (chunk->getX() * Chunk::CHUNK_SIZE + x) * Chunk::TILE_SIZE + Chunk::TILE_SIZE;
                            tp.y = (chunk->getY() * Chunk::CHUNK_SIZE + y) * Chunk::TILE_SIZE + Chunk::TILE_SIZE;
                            
                            bool towerExists = false;
                            for(int i = 0; i < staticTowers.get_length(); ++i) 
                            {
                                double dx = staticTowers[i].x - tp.x;
                                double dy = staticTowers[i].y - tp.y;
                                if((dx * dx + dy * dy) < 1.0) 
                                {
                                    towerExists = true;
                                    break;
                                }
                            }

                            if (!towerExists && currentMode != Mode::LOAD_GAME) 
                            {
                                staticTowers.append(tp);
                            }

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

MutableArraySequence<Point2D> EnvironmentManager::getStaticTowers() const 
{ 
    return staticTowers; 
}

MapBounds EnvironmentManager::getWorldBounds() const 
{
    if (activeChunks.get_length() == 0) 
    {
        return {0.0, 0.0, 800.0, 600.0}; //Если мир пустой, возвращаем стандартное окно
    }

    double minX = 9999999.0, minY = 9999999.0;
    double maxX = -9999999.0, maxY = -9999999.0;

    for (int i = 0; i < activeChunks.get_length(); ++i) 
    {
        Chunk* c = activeChunks[i];
        
        //Считаем координаты углов каждого чанка
        double cx1 = c->getX() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
        double cy1 = c->getY() * Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
        double cx2 = cx1 + Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;
        double cy2 = cy1 + Chunk::CHUNK_SIZE * Chunk::TILE_SIZE;

        if (cx1 < minX) 
        {
            minX = cx1;
        }
        if (cy1 < minY) 
        {
            minY = cy1; 
        }
        if (cx2 > maxX) 
        {
            maxX = cx2;
        }
        if (cy2 > maxY) 
        {
            maxY = cy2;
        }
    }
    return {minX, minY, maxX, maxY};
}

MutableArraySequence<RadioPath> EnvironmentManager::computePaths(Point2D tx, Point2D rx, double frequencyGHz) const 
{
    MutableArraySequence<RadioPath> paths;

    // 1. Прямой луч (Line of Sight - LOS) 
    double distLOS = std::hypot(rx.x - tx.x, rx.y - tx.y);
    if (distLOS < 1.0) 
    {
        distLOS = 1.0;
    }
    Point2D arrivalVec = { (rx.x - tx.x) / distLOS, (rx.y - tx.y) / distLOS };
    double totalTransmittance = 1.0;

    int x0 = static_cast<int>(std::floor(tx.x / Chunk::TILE_SIZE));
    int y0 = static_cast<int>(std::floor(tx.y / Chunk::TILE_SIZE));
    int x1 = static_cast<int>(std::floor(rx.x / Chunk::TILE_SIZE));
    int y1 = static_cast<int>(std::floor(rx.y / Chunk::TILE_SIZE));
    int dx = std::abs(x1 - x0), dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) 
    {
        Tile t = getTileAtWorldPos({ x0 * Chunk::TILE_SIZE, y0 * Chunk::TILE_SIZE });
        totalTransmittance *= RadioPhysics::getTileTransmittance(t.type, frequencyGHz);
        
        if (totalTransmittance < 0.001 || (x0 == x1 && y0 == y1)) 
        {
            break;
        }
        int e2 = 2 * err;
        if (e2 >= dy) 
        { 
            err += dy; x0 += sx; 
        }
        if (e2 <= dx) 
        { 
            err += dx; y0 += sy; 
        }
    }

    if (totalTransmittance >= 0.001) 
    {
        // Прямой луч (bouncePoint совпадает с rx, так как удара о стену нет)
        paths.append(RadioPath{ tx, rx, distLOS, arrivalVec, totalTransmittance, 0, PathType::LOS });
    }

    // 2. Отражение от земли (Ground Bounce)
    double distGround = std::sqrt(distLOS * distLOS + (10.0 + 2.0) * (10.0 + 2.0));
    paths.append(RadioPath{ tx, rx, distGround, arrivalVec, totalTransmittance * 0.7, 1, PathType::GROUND}); 

    // 3. Отражение от стены (Wall Bounce - ISM) 
    const int searchRadius = 10;
    // Ищем стены в радиусе searchRadius вокруг приемника 
    int rx_tx = static_cast<int>(std::floor(rx.x / Chunk::TILE_SIZE));
    int rx_ty = static_cast<int>(std::floor(rx.y / Chunk::TILE_SIZE));

    for (int dy_grid = -searchRadius; dy_grid <= searchRadius; ++dy_grid) {
        for (int dx_grid = -searchRadius; dx_grid <= searchRadius; ++dx_grid) {
            if (dx_grid == 0 && dy_grid == 0) {
                continue; // Пропускаем тайл, на котором стоим
            }

            double tileWorldX = (rx_tx + dx_grid) * Chunk::TILE_SIZE;
            double tileWorldY = (rx_ty + dy_grid) * Chunk::TILE_SIZE; 
            
            // Проверяем центр тайла на наличие стены
            Tile t = getTileAtWorldPos({ tileWorldX + Chunk::TILE_SIZE / 2.0, tileWorldY + Chunk::TILE_SIZE / 2.0 });

            if (t.type == TileType::WALL) {
                
                // А) Проверка вертикальной грани тайла (Ось X)
                double wallPlaneX = (tx.x < tileWorldX + Chunk::TILE_SIZE / 2.0) ? tileWorldX : tileWorldX + Chunk::TILE_SIZE;
                Point2D virtualTx_X = { wallPlaneX + (wallPlaneX - tx.x), tx.y };
                double dBounceX = std::hypot(rx.x - virtualTx_X.x, rx.y - virtualTx_X.y);

                if (dBounceX > 1.0) {
                    double t_param = (wallPlaneX - virtualTx_X.x) / (rx.x - virtualTx_X.x);
                    Point2D bounceP = { wallPlaneX, virtualTx_X.y + t_param * (rx.y - virtualTx_X.y) };
                    
                    if (bounceP.y >= tileWorldY && bounceP.y <= tileWorldY + Chunk::TILE_SIZE) {
                        Point2D bAV = { (rx.x - virtualTx_X.x) / dBounceX, (rx.y - virtualTx_X.y) / dBounceX };
                        double rCoeff = RadioPhysics::getReflectionCoefficient(TileType::WALL, std::abs(bAV.x), frequencyGHz);
                        
                        if (rCoeff > 0.05) {
                            double finalBounceAtten = totalTransmittance * rCoeff;
                            if (finalBounceAtten > 0.001) {
                                paths.append(RadioPath{ tx, bounceP, dBounceX, bAV, finalBounceAtten, 1, PathType::WALL });
                            }
                        }
                    }
                }

                // Б) Проверка горизонтальной грани тайла (Ось Y)
                double wallPlaneY = (tx.y < tileWorldY + Chunk::TILE_SIZE / 2.0) ? tileWorldY : tileWorldY + Chunk::TILE_SIZE;
                Point2D virtualTx_Y = { tx.x, wallPlaneY + (wallPlaneY - tx.y) };
                double dBounceY = std::hypot(rx.x - virtualTx_Y.x, rx.y - virtualTx_Y.y);

                if (dBounceY > 1.0) {
                    double t_param = (wallPlaneY - virtualTx_Y.y) / (rx.y - virtualTx_Y.y);
                    Point2D bounceP = { virtualTx_Y.x + t_param * (rx.x - virtualTx_Y.x), wallPlaneY };
                    
                    if (bounceP.x >= tileWorldX && bounceP.x <= tileWorldX + Chunk::TILE_SIZE) {
                        Point2D bAV = { (rx.x - virtualTx_Y.x) / dBounceY, (rx.y - virtualTx_Y.y) / dBounceY };
                        double rCoeff = RadioPhysics::getReflectionCoefficient(TileType::WALL, std::abs(bAV.y), frequencyGHz);
                        
                        if (rCoeff > 0.05) {
                            double finalBounceAtten = totalTransmittance * rCoeff;
                            if (finalBounceAtten > 0.001) {
                                paths.append(RadioPath{ tx, bounceP, dBounceY, bAV, finalBounceAtten, 1, PathType::WALL });
                            }
                        }
                    }
                }
            }
        }
    }
    return paths;
}