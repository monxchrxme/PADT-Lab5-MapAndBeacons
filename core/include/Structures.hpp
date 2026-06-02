#pragma once

struct Point2D {
    double x;
    double y;
};

// Радио-путь (Отдельный луч, дошедший от Вышки к Приемнику)
struct RadioPath {
    double distance;         // Точная геометрическая длина пути (для фазы)
    Point2D arrivalVector;   // Нормализованный вектор прихода (для Азимута/AoA)
    double attenuation;      // Общий коэффициент пропускания пути (от 0.0 до 1.0)
    int bounceCount;         // 0 - прямой (LOS), 1 - отраженный луч
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

// Глобальные настройки (CVars) 
struct SimulationSettings {
    float baseTxPower = 100000.0f; 
    float signalThreshold = 0.001f; // Порог чувствительности приемника (Receiver Sensitivity)
    float smoothing = 0.1f;
    float frequencyGHz = 2.4f;      // Несущая частота (по умолчанию 2.4 ГГц)
    float hardwareJitter = 10.0f;   // Аппаратная погрешность (в пикселях)
};

// глобальная переменная, которая безопасно объявляется прямо в заголовочном файле
inline SimulationSettings g_Settings;