#include "entities/TargetObject.hpp"
#include "entities/MobileBeacon.hpp"
#include "MathUtils.hpp"
#include "sequences/mutable_array_sequence.hpp" 
#include "math/TrilaterationSolver.hpp"

void TargetObject::setDirection(double dx, double dy) {
    velocity_ = {dx, dy};
    math::normalize(velocity_);
}

void TargetObject::updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& activeBeacons) {
    if (!env) return;

    MutableArraySequence<SignalData> allSignals;

    // Сбор данных от стационарных вышек 
    MutableArraySequence<Point2D> staticTowers = env->getStaticTowers();
   
    for (int i = 0; i < staticTowers.get_length(); ++i) {
        Point2D towerPos = staticTowers[i];
        double rssi = env->calculateSignal(realPosition_, towerPos);
        if (rssi > g_Settings.signalThreshold) {
            allSignals.append(SignalData{towerPos, rssi, 0.0}); 
        }
    }

    // Сбор данных от мобильных маяков 
    for (int i = 0; i < activeBeacons.get_length(); ++i) {
        const MobileBeacon* beacon = activeBeacons[i];
        if (!beacon) continue; 
        
        // Берем не реальную позицию маяка, 
        // а ту, которую он сам вычислил и передал
        LocationResult beaconEstimation = beacon->getEstimation();
        // НО сигнал идет от реальной позиции маяка до TargetObject
        double rssi = env->calculateSignal(realPosition_, beacon->getRealPosition());
        //TODO: подумать над значением на которое порог жестче 
        if (rssi > g_Settings.signalThreshold + 0.1) { // Мобильные передатчики слабее, порог жестче 
            allSignals.append(SignalData{
                beaconEstimation.estimatedPos,   // Координаты, в которых маяк думает, что он находится
                rssi,
                beaconEstimation.errorRadius     // Погрешность, которую маяк накопил
            });
        }
    }

    // Трилатерация (с учетом предыдущей позиции)
    LocationResult rawEstimation = TrilaterationSolver::solve(allSignals, estimation_.estimatedPos);

    // Фильтр низких частот 
    // smoothing = 0.1 означает, что мы берем 90% от старой позиции и только 10% от новой
    // Это убирает резкие "дергания" от шума (Jitter), делая движение круга плавным 
    estimation_.estimatedPos.x = math::lerp(estimation_.estimatedPos.x, rawEstimation.estimatedPos.x, g_Settings.smoothing);
    estimation_.estimatedPos.y = math::lerp(estimation_.estimatedPos.y, rawEstimation.estimatedPos.y, g_Settings.smoothing);
    
    // Радиус ошибки тоже сглаживает, плюс добавляем базовую "неуверенность" от шума среды
    double targetError = rawEstimation.errorRadius + g_Settings.baseUncertainty;; // добавляем радиус для компенсации аппаратной погрешности
    estimation_.errorRadius = math::lerp(estimation_.errorRadius, targetError, g_Settings.smoothing);
}