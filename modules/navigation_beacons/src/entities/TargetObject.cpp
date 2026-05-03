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
        if (rssi > 0.05) {
            allSignals.append(SignalData{towerPos, rssi, 0.0}); 
        }
    }

    // Сбор данных от мобильных маяков 
    for (int i = 0; i < activeBeacons.get_length(); ++i) {
        const MobileBeacon* beacon = activeBeacons[i];
        if (!beacon) continue; 
        
        // Мы берем не реальную позицию маяка, 
        // а ту, которую он сам вычислил и передал
        LocationResult beaconEstimation = beacon->getEstimation();
        // Сигнал идет от реальной позиции маяка до нас
        double rssi = env->calculateSignal(realPosition_, beacon->getRealPosition());
        
        if (rssi > 0.1) { // Мобильные передатчики слабее, порог жестче
            allSignals.append(SignalData{
                beaconEstimation.estimatedPos,   // Координаты, в которых маяк думает, что он находится
                rssi,
                beaconEstimation.errorRadius     // Погрешность, которую маяк накопил
            });
        }
    }

    // Трилатерация 
    estimation_ = TrilaterationSolver::solve(allSignals);
}