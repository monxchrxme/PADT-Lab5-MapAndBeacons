#include "entities/MobileBeacon.hpp"
#include "physics/SignalProcessor.hpp"
#include "math/AoASolver.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "sequences/mutable_array_sequence.hpp"
#include "MathUtils.hpp"

void MobileBeacon::updatePhysics(float dt, const IEnvironment* env) {
    // Проверяем, не ушел ли маяк слишком далеко от своей базы
    if (math::distance(realPosition_, anchorPos_) > patrolRadius_) {
        // Направляем вектор скорости ровно на базу (домой)
        velocity_.x = anchorPos_.x - realPosition_.x;
        velocity_.y = anchorPos_.y - realPosition_.y;
        math::normalize(velocity_);
        realPosition_.x += velocity_.x * speed_ * dt;
        realPosition_.y += velocity_.y * speed_ * dt;
    }
    // Вызываем стандартную физику из базового класса (шаг и отскок от стен)
    Entity::updatePhysics(dt, env);
}

void MobileBeacon::updateEstimation(const IEnvironment* env, const Sequence<MobileBeacon*>& activeBeacons) {
    if (!env) return;

    // 1. Опрос СТАЦИОНАРНЫХ ВЫШЕК (Безусловное доверие)

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

    // 2. КООПЕРАТИВНАЯ ЛОКАЛИЗАЦИЯ (Маяки помогают друг другу) 
    for (int i = 0; i < activeBeacons.get_length(); ++i) {
        const MobileBeacon* other = activeBeacons[i];
        // Самого себя не опрашиваем
        if (!other || other == this) continue;

        LocationResult otherEst = other->getEstimation();
        auto paths = env->computePaths(other->getRealPosition(), realPosition_, g_Settings.frequencyGHz);
        ProcessedSignal sig = SignalProcessor::processPaths(paths, otherEst.estimatedPos, g_Settings.frequencyGHz);

        if (sig.rssi > g_Settings.signalThreshold * 2.0) {
            // Если другой маяк потерялся (большой errorRadius), 
            // мы радикально занижаем мощность его сигнала. Решатель просто проигнорирует этот "мусор"
            sig.rssi /= (1.0 + (otherEst.errorRadius * otherEst.errorRadius * 0.01)); 
            validSignals.append(sig);
        }
    }

    // Запоминаем сигналы для отрисовки перед отправкой в решатель
    lastSignals_ = validSignals;

    // 3. Матричная Триангуляция (AoA)
    try {
        LocationResult rawEstimation = AoASolver::solve(validSignals);

        // Сглаживание (Lerp)
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
        // Если сигнал потерян погрешность начинает резко расти 
        estimation_.errorRadius += 2.0; 
        if (estimation_.errorRadius > g_Settings.maxErrorRadius) {
            estimation_.errorRadius = g_Settings.maxErrorRadius;
        }
    }
}

// СЕТЕВАЯ МАРШРУТИЗАЦИЯ (Flooding) 

void MobileBeacon::receivePacket(const MeshPacket& packet) {
    // 1. проверяем видели ли мы уже этот пакет
    for (int i = 0; i < seenPacketIds_.get_length(); ++i) {
        if (seenPacketIds_[i] == packet.packetId) return; 
    }
    seenPacketIds_.append(packet.packetId);

    // 2. Уменьшаем TTL
    MeshPacket forwardedPacket = packet;
    forwardedPacket.ttl -= 1;

    // 3. Если пакет еще жив, ставим в очередь на ретрансляцию соседям
    if (forwardedPacket.ttl > 0) {
        txQueue_.append(forwardedPacket);
    }
}

MutableArraySequence<MeshPacket> MobileBeacon::flushTxQueue() {
    // Копируем очередь, чтобы вернуть её
    MutableArraySequence<MeshPacket> copy = txQueue_;
    // Очищаем локальную очередь (создаем пустую)
    MutableArraySequence<MeshPacket> emptyQueue;
    txQueue_ = emptyQueue; 
    
    return copy;
}