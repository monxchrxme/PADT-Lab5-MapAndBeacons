#include "entities/MobileBeacon.hpp"
#include "physics/SignalProcessor.hpp"
#include "math/AoASolver.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "sequences/mutable_array_sequence.hpp"

void MobileBeacon::updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& /*activeBeacons*/) {
    if (!env) return;

    // Получаем вышки по значению 
    MutableArraySequence<Point2D> staticTowers = env->getStaticTowers();
    // Массив для полезных сигналов
    MutableArraySequence<ProcessedSignal> validSignals;

    // pipeline сбора данных

    for (int i = 0; i < staticTowers.get_length(); ++i) {
        Point2D towerPos = staticTowers[i];
        // Запрашиваем пути у Карты на текущей частоте
        auto paths = env->computePaths(towerPos, realPosition_, g_Settings.frequencyGHz);
        // Пропускаем пути через DSP-процессор (считаем фазы, азимут, интерференцию)
        ProcessedSignal sig = SignalProcessor::processPaths(paths, towerPos, g_Settings.frequencyGHz);
        if (sig.rssi > g_Settings.signalThreshold) {
            validSignals.append(sig);
        }
    }

    // Матричная Триангуляция (AoA)
    try {
        LocationResult rawEstimation = AoASolver::solve(validSignals);

        // Сглаживание (Lerp)
        estimation_.estimatedPos.x = math::lerp(estimation_.estimatedPos.x, rawEstimation.estimatedPos.x, g_Settings.smoothing);
        estimation_.estimatedPos.y = math::lerp(estimation_.estimatedPos.y, rawEstimation.estimatedPos.y, g_Settings.smoothing);
        estimation_.errorRadius = math::lerp(estimation_.errorRadius, rawEstimation.errorRadius, g_Settings.smoothing);
        
    } catch (const NavigationException& /*e*/) {
        // Если сигнал потерян погрешность начинает резко расти 
        estimation_.errorRadius += 2.0; 
    }
}