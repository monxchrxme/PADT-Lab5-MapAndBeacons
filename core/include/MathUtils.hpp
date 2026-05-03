#pragma once
#include "Structures.hpp"
#include <cmath>
#include <random>

namespace math {
    // Вычисление евклидова расстояния между двумя точками
    [[nodiscard]] inline double distance(Point2D a, Point2D b) {
        return std::hypot(b.x - a.x, b.y - a.y);
    }

    // Нормализация вектора 
    inline void normalize(Point2D& vector) {
        const double mag = std::hypot(vector.x, vector.y);
        if (mag > 1e-9) { 
            vector.x /= mag;
            vector.y /= mag;
        } else {
            vector.x = 0.0;
            vector.y = 0.0;
        }
    }

    // Вращение 2D вектора на заданный угол в радианах (Матрица поворота)
    inline void rotate(Point2D& vector, double angleRadians) {
        const double cosA = std::cos(angleRadians);
        const double sinA = std::sin(angleRadians);
        const double newX = vector.x * cosA - vector.y * sinA;
        const double newY = vector.x * sinA + vector.y * cosA;
        vector.x = newX;
        vector.y = newY;
    }

    // Генерация случайного числа 
    [[nodiscard]] inline double randomDouble(double min, double max) {
        thread_local std::mt19937 gen{std::random_device{}()};
        std::uniform_real_distribution<double> dist(min, max);
        return dist(gen);
    }

    // Функция перевода RSSI в дистанцию
    [[nodiscard]] inline double rssiToDistance(double rssi, double txPower) {
    if (rssi <= 0.0001) return 99999.0; // Сигнала нет, расстояние бесконечно
    return std::sqrt(txPower / rssi); 
    }   

    // Линейная интерполяция (Lerp) для сглаживания значений
    [[nodiscard]] inline double lerp(double start, double end, double t) {
        return start + t * (end - start);
    }
}