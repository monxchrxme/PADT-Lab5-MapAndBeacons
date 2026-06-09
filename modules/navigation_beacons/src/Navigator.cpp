#include "Navigator.hpp"
#include "MathUtils.hpp"
#include "Structures.hpp"
#include <stdexcept>

Navigator::Navigator(const IEnvironment* env) : environment_(env) {
    if (!environment_) {
        throw std::invalid_argument("Navigator: Environment cannot be null.");
    }

    // Спавним главную цель в центре (400, 300) со скоростью 100 пикс/сек
    target_ = new TargetObject({600.0, 300.0}, 100.0f);

    // Спавним 3 мобильных маяка в разных частях карты
    // Скорость 50 пикс/сек, двигаются в разных направлениях
    beacons_.append(new MobileBeacon({100.0, 100.0}, {1.0, 0.5}, 50.0f));
    beacons_.append(new MobileBeacon({700.0, 500.0}, {-1.0, -0.5}, 50.0f));
    beacons_.append(new MobileBeacon({400.0, 100.0}, {0.0, 1.0}, 50.0f));
}

Navigator::~Navigator() {
    // Удаляем цель
    delete target_;
    // Уничтожаем все мобильные маяки, чтобы не было утечек памяти
    for (int i = 0; i < beacons_.get_length(); ++i) {
        delete beacons_[i];
    }
}

void Navigator::updateEntities(float dt) {
    // 0. Очистка старых линий связи (эффект затухания) 
    MutableArraySequence<NetworkLink> aliveLinks;
    for (int i = 0; i < activeLinks_.get_length(); ++i) {
        NetworkLink link = activeLinks_[i];
        link.lifeTime -= dt;
        if (link.lifeTime > 0.0f) {
            aliveLinks.append(link);
        }
    }
    activeLinks_ = aliveLinks;

    // 1. Обновляем все мобильные маяки 
    for (int i = 0; i < beacons_.get_length(); ++i) {
        MobileBeacon* beacon = beacons_[i];
        // Маяк шагает
        beacon->updatePhysics(dt, environment_);
        try {
            // Маяк вычисляет свое положение (опрашивая Карту)
            beacon->updateEstimation(environment_, beacons_); 
        } catch (...) {
            // Игнорируем ошибки соединения, маяк продолжит двигаться по физике, пока снова не поймает сигнал
        }
    }
    // Обновляем искомую цель
    target_->updatePhysics(dt, environment_);
    try {
        // Цель опрашивает и Карту (стационарные вышки), и мобильные маяки,
        // после чего внутри собирает все данные и запускает Трилатерацию
        target_->updateEstimation(environment_, beacons_);
    } catch (...) {
        // Игнорируем потерю сигнала у главной цели
    }

    // 3. ГЕНЕРАЦИЯ ПАКЕТОВ (Главная Цель транслируется в эфир)
    timeSinceLastPacket_ += dt;
    if (timeSinceLastPacket_ > 0.5f) {
        timeSinceLastPacket_ = 0.0f;
        packetIdCounter_++;

        MeshPacket newPacket{ packetIdCounter_, 3, target_->getEstimation().estimatedPos, target_->getEstimation().errorRadius };

        for (int i = 0; i < beacons_.get_length(); ++i) {
            if (math::distance(target_->getRealPosition(), beacons_[i]->getRealPosition()) < g_Settings.meshCommRadius) {
                beacons_[i]->receivePacket(newPacket);
                // ЖЕЛТАЯ СВЯЗЬ: Target -> Beacon
                activeLinks_.append(NetworkLink{target_->getRealPosition(), beacons_[i]->getRealPosition(), 0.3f, LinkType::TargetToBeacon});
            }
        }
    }

    // 4. РЕТРАНСЛЯЦИЯ MESH-СЕТИ (Доставка до Вышек) 
    MutableArraySequence<Point2D> towers = environment_->getStaticTowers(); // Запрашиваем вышки

    for (int i = 0; i < beacons_.get_length(); ++i) {
        auto packetsToTx = beacons_[i]->flushTxQueue();
        
        for (int p = 0; p < packetsToTx.get_length(); ++p) {
            
            // А) Пытаемся доставить пакет на Базовую Станцию (Стационарную вышку)
            bool deliveredToTower = false;
            for (int t = 0; t < towers.get_length(); ++t) {
                if (math::distance(beacons_[i]->getRealPosition(), towers[t]) < g_Settings.meshCommRadius) {
                    // ЗЕЛЕНАЯ СВЯЗЬ: Beacon -> Tower
                    activeLinks_.append(NetworkLink{beacons_[i]->getRealPosition(), towers[t], 0.3f, LinkType::BeaconToTower});
                    deliveredToTower = true;
                }
            }

            // Б) Если до вышки не достали, пересылаем другим маякам
            if (!deliveredToTower) {
                for (int j = 0; j < beacons_.get_length(); ++j) {
                    if (i == j) continue;

                    if (math::distance(beacons_[i]->getRealPosition(), beacons_[j]->getRealPosition()) < g_Settings.meshCommRadius) {
                        beacons_[j]->receivePacket(packetsToTx[p]);
                        // ГОЛУБАЯ СВЯЗЬ: Beacon -> Beacon
                        activeLinks_.append(NetworkLink{beacons_[i]->getRealPosition(), beacons_[j]->getRealPosition(), 0.3f, LinkType::BeaconToBeacon});
                    }
                }
            }
        }
    }
}