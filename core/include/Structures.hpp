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