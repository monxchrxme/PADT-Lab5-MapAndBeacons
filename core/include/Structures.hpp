#pragma once

struct Point2D {
    double x;
    double y;
};

// Данные, которые искомый объект получает от вышек
struct SignalData {
    Point2D sourcePosition; // Координаты вышки/маяка
    double rssi;            // Уровень сигнала 
    double sourceError;     // Погрешность координат источника 
};

// Результат работы алгоритма локализации
struct LocationResult {
    Point2D estimatedPos;   // Вычисленные координаты
    double errorRadius;     // Радиус погрешности 
};

// Динамические настройки симуляции (CVars)
struct SimulationSettings {
    float baseTxPower = 100000.0f;     // Мощность передатчиков на расстоянии 1 метр
    float signalThreshold = 0.05f;     // Порог чувствительности (отсечение шумов)
    float smoothing = 0.1f;            // Коэффициент сглаживания (Lerp)
    float noiseVariation = 0.15f;      // Шум эфира (0.15 = +/- 15%)
    float learningRate = 0.5f;         // Скорость градиентного спуска
    float baseUncertainty = 15.0f;     // Аппаратная погрешность (постоянный радиус ошибки)
    float wallAttenuation = 0.5f;      // Сколько сигнала пропускает стена (0.5 = 50%)
};

// глобальная переменная, которая безопасно объявляется прямо в заголовочном файле
inline SimulationSettings g_Settings;