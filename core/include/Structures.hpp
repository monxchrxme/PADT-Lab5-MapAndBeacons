#pragma once

struct Point2D {
    double x;
    double y;
};

enum class TileType {
    EMPTY,    
    FOREST,   
    WATER,    
    WALL,      
    PATH,       
    TOWER_BASE   
};

// Типы путей (для отрисовки отражений)
enum class PathType {
    LOS,        // Прямой луч
    GROUND,     // Отражение от земли (для 2D не рисуем, он сливается с прямым)
    WALL        // Отражение от стены (рисуем V-образный излом)
};

// Радио-путь (Отдельный луч, дошедший от Вышки к Приемнику)
struct RadioPath {
    Point2D txPos;           // Откуда вылетел луч (Вышка)
    Point2D bouncePoint;     // Точка удара о стену (если это WALL)
    double distance;         // Точная геометрическая длина пути (для фазы)
    Point2D arrivalVector;   // Нормализованный вектор прихода (для Азимута/AoA)
    double attenuation;      // Общий коэффициент пропускания пути (от 0.0 до 1.0)
    int bounceCount;         // 0 - прямой (LOS), 1 - отраженный луч
    PathType type;           // Тип луча
};

// Результат работы алгоритма локализации
struct LocationResult {
    Point2D estimatedPos;   // Вычисленные координаты
    double errorRadius;     // Радиус погрешности 
};

// Обработанный радиосигнал (после интерференции и пеленгации)
struct ProcessedSignal {
    Point2D sourcePos; // Реальные координаты вышки (нужны для уравнений)
    double rssi;       // Итоговая мощность с учетом наложения фаз
    double azimuth;    // Вычисленный угол прихода (Angle of Arrival)
};

// Типы сетевых связей для отрисовки разными цветами
enum class LinkType {
    TargetToBeacon,  // Желтый
    BeaconToBeacon,  // Голубой
    BeaconToTower    // Зеленый (Успешная доставка)
};

// Пакет Mesh-сети (Лавинная маршрутизация)
struct MeshPacket {
    int packetId;         // Уникальный ID пакета (защита от зацикливания)
    int ttl;              // Time-To-Live (оставшееся число прыжков)
    Point2D payloadPos;   // Передаваемые данные: координаты Цели
    double payloadError;  // Передаваемые данные: погрешность Цели
};

// Линия связи (для отрисовки передачи данных в UI)
struct NetworkLink {
    Point2D from;
    Point2D to;
    float lifeTime; // Время жизни линии на экране (для эффекта затухания)
    LinkType type;  // Тип связи 
};

// Глобальные настройки (CVars) 
struct SimulationSettings {
    float baseTxPower = 100000.0f; 
    float signalThreshold = 0.001f; // Порог чувствительности приемника (Receiver Sensitivity)
    float smoothing = 0.1f;
    float frequencyGHz = 2.4f;      // Несущая частота (по умолчанию 2.4 ГГц)
    float hardwareJitter = 10.0f;   // Аппаратная погрешность (в пикселях)

    float maxErrorRadius = 600.0f;  // Ограничитель (размер экрана)
    float meshCommRadius = 300.0f;  // длина связи mesh-сети
    
    bool showAoARays = false;       // Включена ли отрисовка векторов пеленгов (AoA Rays)
    bool showMeshNetwork = true;    // Включена ли отрисовка лазеров сети
    bool showMultipathRays = false; // Включена ли отрисовка лучей с отражениями 
};

// глобальная переменная, которая безопасно объявляется прямо в заголовочном файле
inline SimulationSettings g_Settings;