#include "math/TrilaterationSolver.hpp"
#include "MathUtils.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include <cmath>

LocationResult TrilaterationSolver::solve(const Sequence<SignalData>& signals) {
    const int count = signals.get_length();
    
    // Если источников меньше 3-х, позицию вычислить невозможно математически
    if (count < 3) {
        throw SignalLostException();
    }

    // Начальное приближение - Центр масс всех вышек
    Point2D guess = {0.0, 0.0};
    for (int i = 0; i < count; ++i) {
        guess.x += signals[i].sourcePosition.x;
        guess.y += signals[i].sourcePosition.y;
    }
    guess.x /= count;
    guess.y /= count;

    // Метод Градиентного Спуска 
    const int maxIterations = 100;
    const double learningRate = 0.5; // Шаг спуска (скорость обучения)

    for (int iter = 0; iter < maxIterations; ++iter) {
        double gradX = 0.0;
        double gradY = 0.0;

        for (int i = 0; i < count; ++i) {
            SignalData sig = signals[i];
            double targetDist = math::rssiToDistance(sig.rssi);
            
            // Текущее расстояние от нашего "приближения" до вышки
            double dx = guess.x - sig.sourcePosition.x;
            double dy = guess.y - sig.sourcePosition.y;
            double currentDist = std::hypot(dx, dy);
            
            if (currentDist < 1e-5) currentDist = 1e-5; // Защита от деления на ноль

            // Ошибка: разница между тем, где мы думаем мы есть, и реальным радиусом
            double error = currentDist - targetDist;

            // Вычисляем вектор градиента (направление наискорейшего роста ошибки)
            gradX += error * (dx / currentDist);
            gradY += error * (dy / currentDist);
        }

        // Усредняем градиент
        gradX /= count;
        gradY /= count;

        // Делаем шаг в сторону УМЕНЬШЕНИЯ ошибки (против градиента)
        guess.x -= learningRate * gradX;
        guess.y -= learningRate * gradY;
    }

    // Вычисление итогового "Радиуса погрешности" 
    double totalErrorRadius = 0.0;
    for (int i = 0; i < count; ++i) {
        SignalData sig = signals[i];
        double targetDist = math::rssiToDistance(sig.rssi);
        double finalDist = math::distance(guess, sig.sourcePosition);
        
        // Погрешность = Ошибка вычисления дистанции + собственная погрешность самого маяка
        totalErrorRadius += std::abs(finalDist - targetDist) + sig.sourceError;
    }
    totalErrorRadius /= count;

    return LocationResult{guess, totalErrorRadius};
}