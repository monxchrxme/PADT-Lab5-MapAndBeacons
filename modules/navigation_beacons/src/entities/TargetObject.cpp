#include "entities/TargetObject.hpp"
#include "entities/MobileBeacon.hpp"
#include "physics/SignalProcessor.hpp"
#include "math/AoASolver.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "sequences/mutable_array_sequence.hpp"
#include "MathUtils.hpp"

void TargetObject::setDirection(double dx, double dy) {
    velocity_ = {dx, dy};
    math::normalize(velocity_);
}

void TargetObject::updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& activeBeacons) {
    if (!env) return;

    MutableArraySequence<ProcessedSignal> allSignals;

    // Сбор данных от стационарных вышек 
    MutableArraySequence<Point2D> staticTowers = env->getStaticTowers();
    for (int i = 0; i < staticTowers.get_length(); ++i) {
        Point2D towerPos = staticTowers[i];
        auto paths = env->computePaths(towerPos, realPosition_, g_Settings.frequencyGHz);
        ProcessedSignal sig = SignalProcessor::processPaths(paths, towerPos, g_Settings.frequencyGHz);

        if (sig.rssi > g_Settings.signalThreshold) {
            allSignals.append(sig);
        }
    }

    // Сбор данных от мобильных маяков 
    for (int i = 0; i < activeBeacons.get_length(); ++i) {
        const MobileBeacon* beacon = activeBeacons[i];
        if (!beacon) continue; 
        
        LocationResult beaconEst = beacon->getEstimation();
        // Сигнал отправялется от реальной позиции маяка
        auto paths = env->computePaths(beacon->getRealPosition(), realPosition_, g_Settings.frequencyGHz);
        ProcessedSignal sig = SignalProcessor::processPaths(paths, beacon->getEstimation().estimatedPos, g_Settings.frequencyGHz);

        // Маяки передают слабее вышек 
        if (sig.rssi > g_Settings.signalThreshold * 2.0) {
            // Чем больше маяк "заблудился", тем меньше мы ему верим
            // Делим RSSI на (1 + квадрат ошибки маяка)
            // Решатель AoASolver просто проигнорирует такой сигнал
            sig.rssi /= (1.0 + (beaconEst.errorRadius * beaconEst.errorRadius * 0.01)); 
            allSignals.append(sig);
        }
    }

    // Запоминаем сигналы для отрисовки перед отправкой в решатель
    lastSignals_ = allSignals;
    
    // Матричная Триангуляция и динамическое сглаживание 
    try {
        LocationResult rawEstimation = AoASolver::solve(allSignals);

        // Смотрим, насколько сильно новая точка "прыгнула" от текущей
        double jumpDistance = math::distance(estimation_.estimatedPos, rawEstimation.estimatedPos);
        // Если точка прыгнула слишком далеко (больше 50 пикселей) - это скорее всего помеха (выброс)
        // В этом случае урезаем Lerp в 10 раз, круг останется на месте по инерции
        double currentSmoothing = g_Settings.smoothing;
        if (jumpDistance > 50.0) {
            currentSmoothing *= 0.1; 
        }
        estimation_.estimatedPos.x = math::lerp(estimation_.estimatedPos.x, rawEstimation.estimatedPos.x, g_Settings.smoothing);
        estimation_.estimatedPos.y = math::lerp(estimation_.estimatedPos.y, rawEstimation.estimatedPos.y, g_Settings.smoothing);
        estimation_.errorRadius = math::lerp(estimation_.errorRadius, rawEstimation.errorRadius, g_Settings.smoothing);
        
    } catch (const NavigationException& /*e*/) {
        estimation_.errorRadius += 2.0; 
        if (estimation_.errorRadius > g_Settings.maxErrorRadius) {
            estimation_.errorRadius = g_Settings.maxErrorRadius;
        }
    }
}