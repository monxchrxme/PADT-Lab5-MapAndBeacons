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
        if (rssi > 0.05) { // Сигналы слабее 5% отбрасываем как шум
            validSignals.append(SignalData{
                towerPos,
                rssi,
                0.0 // Стационарные вышки не имеют погрешности
            });
        }
    }

    // Вызываем математическое ядро трилатерации
    estimation_ = TrilaterationSolver::solve(validSignals);
}