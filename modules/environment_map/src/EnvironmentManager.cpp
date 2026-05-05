#include "../include/EnvironmentManager.hpp" // <-- Жесткий относительный путь
#include "../../../core/include/exceptions/EnvironmentExceptions.hpp" 
#include <cmath>

EnvironmentManager::EnvironmentManager() {
    // Инициализируем коллекции
    chunks = MutableArraySequence<Chunk*>();
    staticTowers = MutableArraySequence<Point2D>();
    
    // Генерируем стартовый чанк в центре
    triggerLazyGeneration(Point2D{0.0, 0.0});
}

EnvironmentManager::~EnvironmentManager() {
    // Удаляем все чанки
    for (int i = 0; i < chunks.get_length(); ++i) {
        delete chunks.get(i);
    }
}

Chunk* EnvironmentManager::getChunkAt(int globalX, int globalY) const {
    for (int i = 0; i < chunks.get_length(); ++i) {
        Chunk* chunk = chunks.get(i);
        if (chunk->containsGlobal(globalX, globalY)) {
            return chunk;
        }
    }
    return nullptr;
}

bool EnvironmentManager::isPassable(Point2D point) const {
    int gX = static_cast<int>(std::floor(point.x));
    int gY = static_cast<int>(std::floor(point.y));
    
    Chunk* chunk = getChunkAt(gX, gY);
    if (!chunk) return false; // Если чанк не сгенерирован, туда идти нельзя
    
    int localX = gX - chunk->getOffsetX();
    int localY = gY - chunk->getOffsetY();
    
    TileType type = chunk->getTileLocal(localX, localY);
    return (type == TileType::Grass || type == TileType::Forest);
}

void EnvironmentManager::triggerLazyGeneration(Point2D point) {
    int gX = static_cast<int>(std::floor(point.x));
    int gY = static_cast<int>(std::floor(point.y));
    
    // Вычисляем координаты левого верхнего угла чанка
    int chunkX = (gX >= 0) ? (gX / CHUNK_SIZE) * CHUNK_SIZE : ((gX + 1) / CHUNK_SIZE - 1) * CHUNK_SIZE;
    int chunkY = (gY >= 0) ? (gY / CHUNK_SIZE) * CHUNK_SIZE : ((gY + 1) / CHUNK_SIZE - 1) * CHUNK_SIZE;
    
    if (getChunkAt(chunkX, chunkY) == nullptr) {
        generateChunk(chunkX, chunkY);
    }
}

void EnvironmentManager::generateChunk(int chunkX, int chunkY) {
    Chunk* newChunk = new Chunk(CHUNK_SIZE, chunkX, chunkY);
    
    // TODO: Позже добавим сюда Шум Перлина. Пока просто трава.
    
    // Ставим одну вышку в центр каждого чанка
    Point2D towerPos{ static_cast<double>(chunkX + CHUNK_SIZE / 2), 
                      static_cast<double>(chunkY + CHUNK_SIZE / 2) };
    staticTowers.append(towerPos);
    
    chunks.append(newChunk);
}

MutableArraySequence<Point2D> EnvironmentManager::getStaticTowers() const {
    return staticTowers; 
}

double EnvironmentManager::calculateSignal(Point2D a, Point2D b) const {
    // TODO: Позже добавим Raycasting (Брезенхем) для учета стен.
    // Пока возвращаем идеальное затухание в вакууме (1 / d^2)
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double distSq = dx * dx + dy * dy;
    
    if (distSq <= 0.0001) return 1.0; // Защита от деления на ноль
    
    double signal = 1.0 / distSq;
    return (signal > 1.0) ? 1.0 : signal; // Ограничиваем максимум единицей
}