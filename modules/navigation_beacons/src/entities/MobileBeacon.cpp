#include "entities/MobileBeacon.hpp"
#include "MathUtils.hpp"
#include "sequences/mutable_array_sequence.hpp" 
#include "math/TrilaterationSolver.hpp"

void MobileBeacon::updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& /*activeBeacons*/) {
    if (!env) return;

    // Получаем вышки по значению 
    MutableArraySequence<Point2D> staticTowers = env->getStaticTowers();
    // Массив для полезных сигналов
    MutableArraySequence<SignalData> validSignals;

    // pipeline сбора данных

    for (const auto& towerPos : staticTowers) {
        // Узнаем искаженный уровень сигнала через препятствия
        double rssi = env->calculateSignal(realPosition_, towerPos);
        if (rssi > g_Settings.signalThreshold) { // Сигналы слабее treshold отбрасываем как шум
            validSignals.append(SignalData{
                towerPos,
                rssi,
                0.0 // Стационарные вышки не имеют погрешности
            });
        }
    }

    // Вызываем математическое ядро трилатерации
    LocationResult rawEstimation = TrilaterationSolver::solve(validSignals, estimation_.estimatedPos);

    // Фильтр низких частот 
    // smoothing = 0.1 означает, что мы берем 90% от старой позиции и только 10% от новой
    // Это убирает резкие "дергания" от шума (Jitter), делая движение круга погрешности плавным 
    estimation_.estimatedPos.x = math::lerp(estimation_.estimatedPos.x, rawEstimation.estimatedPos.x, g_Settings.smoothing);
    estimation_.estimatedPos.y = math::lerp(estimation_.estimatedPos.y, rawEstimation.estimatedPos.y, g_Settings.smoothing);
    
    // Радиус ошибки тоже сглаживает, плюс добавляем базовую "неуверенность" от шума среды
    double targetError = rawEstimation.errorRadius + g_Settings.baseUncertainty;
    estimation_.errorRadius = math::lerp(estimation_.errorRadius, targetError, g_Settings.smoothing);
}