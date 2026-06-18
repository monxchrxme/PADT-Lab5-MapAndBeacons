#include "entities/TargetObject.hpp"
#include "entities/MobileBeacon.hpp"
#include "physics/SignalProcessor.hpp"
#include "math/AoASolver.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "sequences/mutable_array_sequence.hpp"
#include "MathUtils.hpp"

void TargetObject::setDirection(double dx, double dy) {
    if (dx == 0.0 && dy == 0.0) {
        velocity_ = {0.0, 0.0};
        return;
    }
    //Сохраняем "силу" нажатия
    double magnitude = std::sqrt(dx*dx + dy*dy);
    
    // Нормализуем направление
    velocity_ = {dx / magnitude, dy / magnitude};
    
    velocity_.x *= magnitude;
    velocity_.y *= magnitude;
}

void TargetObject::updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& activeBeacons) {
    if (!env) return;

    // Очищаем старые пути каждый кадр
    lastPhysicalPaths_ = MutableArraySequence<RadioPath>();

    MutableArraySequence<ProcessedSignal> allSignals;

    // Сбор данных от стационарных вышек 
    MutableArraySequence<Point2D> staticTowers = env->getStaticTowers();
    // Вычисляем радиус, дальше которого сигнал от вышки будет ниже установленной погрешности
    double maxHearingRadius = std::sqrt(g_Settings.baseTxPower / g_Settings.signalThreshold);
    if (maxHearingRadius > 1500.0) {
        maxHearingRadius = 1500.0; 
    }
    for (int i = 0; i < staticTowers.get_length(); ++i) {
        Point2D towerPos = staticTowers[i];
        if (math::distance(realPosition_, towerPos) > maxHearingRadius) {
            continue; 
        }
        auto paths = env->computePaths(towerPos, realPosition_, g_Settings.frequencyGHz);
        for(int p = 0; p < paths.get_length(); ++p) {
            lastPhysicalPaths_.append(paths[p]);
        }
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
        for(int p = 0; p < paths.get_length(); ++p) {
            lastPhysicalPaths_.append(paths[p]);
        }
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