#include "entities/TargetObject.hpp"
#include "entities/MobileBeacon.hpp"
#include "physics/SignalProcessor.hpp"
#include "math/AoASolver.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "sequences/mutable_array_sequence.hpp"

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
        
        // Сигнал отправялется от реальной позиции маяка
        auto paths = env->computePaths(beacon->getRealPosition(), realPosition_, g_Settings.frequencyGHz);
        ProcessedSignal sig = SignalProcessor::processPaths(paths, beacon->getEstimation().estimatedPos, g_Settings.frequencyGHz);

        // Маяки передают слабее вышек 
        if (sig.rssi > g_Settings.signalThreshold * 2.0) {
            // Добавляем неуверенность маяка к нашему сигналу
            sig.rssi /= (1.0 + beacon->getEstimation().errorRadius * 0.01); 
            allSignals.append(sig);
        }
    }

    // Триангуляция
    try {
        LocationResult rawEstimation = AoASolver::solve(allSignals);

        estimation_.estimatedPos.x = math::lerp(estimation_.estimatedPos.x, rawEstimation.estimatedPos.x, g_Settings.smoothing);
        estimation_.estimatedPos.y = math::lerp(estimation_.estimatedPos.y, rawEstimation.estimatedPos.y, g_Settings.smoothing);
        estimation_.errorRadius = math::lerp(estimation_.errorRadius, rawEstimation.errorRadius, g_Settings.smoothing);
        
    } catch (const NavigationException& /*e*/) {
        estimation_.errorRadius += 2.0; 
    }
}