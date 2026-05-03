#include "math/TrilaterationSolver.hpp"
#include "MathUtils.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include <cmath>

LocationResult TrilaterationSolver::solve(const Sequence<SignalData>& signals, Point2D currentGuess) {
    const int count = signals.get_length();
    
    // Если источников меньше 3-х, позицию вычислить невозможно математически
    if (count < 3) {
        throw SignalLostException();
    }

    // Начальное приближение - это то, где мы были в прошлом кадре 
    Point2D guess = currentGuess;

    // Метод Градиентного Спуска 
    const int maxIterations = 100;

    for (int iter = 0; iter < maxIterations; ++iter) {
        double gradX = 0.0;
        double gradY = 0.0;

        for (int i = 0; i < count; ++i) {
            SignalData sig = signals[i];
            double targetDist = math::rssiToDistance(sig.rssi, g_Settings.baseTxPower);
            
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
        guess.x -= g_Settings.learningRate  * gradX;
        guess.y -= g_Settings.learningRate  * gradY;
    }

    // Вычисление итогового "Радиуса погрешности" 
    double totalErrorRadius = 0.0;
    for (int i = 0; i < count; ++i) {
        SignalData sig = signals[i];
        double targetDist = math::rssiToDistance(sig.rssi, g_Settings.baseTxPower);
        double finalDist = math::distance(guess, sig.sourcePosition);
        
        // Погрешность = Ошибка вычисления дистанции + собственная погрешность самого маяка
        totalErrorRadius += std::abs(finalDist - targetDist) + sig.sourceError;
    }
    totalErrorRadius /= count;

    return LocationResult{guess, totalErrorRadius};
}