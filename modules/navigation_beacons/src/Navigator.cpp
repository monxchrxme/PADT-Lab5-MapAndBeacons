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
}

Navigator::~Navigator() {
    // Удаляем цель
    delete target_;
    // Уничтожаем все мобильные маяки, чтобы не было утечек памяти
    for (int i = 0; i < beacons_.get_length(); ++i) {
        delete beacons_[i];
    }
}

void Navigator::checkAndSpawnBeacons() {
    const double SECTOR_SIZE = 1500.0; // Размер одного сектора спавна
    
    Point2D targetPos = target_->getRealPosition();
    int currentSectorX = static_cast<int>(std::floor(targetPos.x / SECTOR_SIZE));
    int currentSectorY = static_cast<int>(std::floor(targetPos.y / SECTOR_SIZE));

    // Проверяем сам сектор и 8 секторов вокруг него (квадрат 3x3)
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int sx = currentSectorX + dx;
            int sy = currentSectorY + dy;

            // Проверяем, спавнили ли мы уже маяк в этом секторе
            bool alreadySpawned = false;
            for (int i = 0; i < spawnedSectors_.get_length(); ++i) {
                if (spawnedSectors_[i].x == sx && spawnedSectors_[i].y == sy) {
                    alreadySpawned = true;
                    break;
                }
            }

            if (!alreadySpawned) {
                spawnedSectors_.append(SectorCoord{sx, sy}); // Запоминаем сектор

                // Пытаемся найти проходимую точку в этом секторе (даем 10 попыток)
                for (int attempt = 0; attempt < 10; ++attempt) {
                    double randX = (sx * SECTOR_SIZE) + math::randomDouble(50.0, SECTOR_SIZE - 50.0);
                    double randY = (sy * SECTOR_SIZE) + math::randomDouble(50.0, SECTOR_SIZE - 50.0);
                    Point2D spawnPos{randX, randY};

                    // Опрашиваем Карту: можно ли тут стоять (не вода ли это и не стена)
                    if (environment_->isPassable(spawnPos)) {
                        double vx = math::randomDouble(-1.0, 1.0);
                        double vy = math::randomDouble(-1.0, 1.0);
                        // Спавним маяк
                        beacons_.append(new MobileBeacon(spawnPos, {vx, vy}, 50.0f, 400.0));
                        break; // Маяк создан, переходим к следующему сектору
                    }
                }
            }
        }
    }
}

void Navigator::updateEntities(float dt) {
    checkAndSpawnBeacons();

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

    // Радиус в котором маяки живут, все что дальше - засыпает (для экономии CPU)
    const double ACTIVE_RADIUS = 1200.0; 

    // 1. Обновляем все мобильные маяки 
    for (int i = 0; i < beacons_.get_length(); ++i) {
        MobileBeacon* beacon = beacons_[i];

        // Hibernation: Если маяк слишком далеко от Главной Цели - пропускаем
        if (math::distance(target_->getRealPosition(), beacon->getRealPosition()) > ACTIVE_RADIUS) {
            continue; 
        }

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

    // 3. Генерация пакетов  (Главная Цель транслируется в эфир)
    timeSinceLastPacket_ += dt;
    if (timeSinceLastPacket_ > 0.5f) {
        timeSinceLastPacket_ = 0.0f;
        packetIdCounter_++;

        MeshPacket newPacket{ packetIdCounter_, 3, target_->getEstimation().estimatedPos, target_->getEstimation().errorRadius };

        for (int i = 0; i < beacons_.get_length(); ++i) {
            if (math::distance(target_->getRealPosition(), beacons_[i]->getRealPosition()) < g_Settings.meshCommRadius) {
                beacons_[i]->receivePacket(newPacket);
                // Желтая связь: Target -> Beacon
                activeLinks_.append(NetworkLink{target_->getRealPosition(), beacons_[i]->getRealPosition(), 0.3f, LinkType::TargetToBeacon});
            }
        }
    }

    // 4. Ретрансляция Mesh-сети (Доставка до Вышек) 
    MutableArraySequence<Point2D> towers = environment_->getStaticTowers(); // Запрашиваем вышки

    for (int i = 0; i < beacons_.get_length(); ++i) {
        auto packetsToTx = beacons_[i]->flushTxQueue();
        
        for (int p = 0; p < packetsToTx.get_length(); ++p) {
            
            // А) Пытаемся доставить пакет на Базовую Станцию (Стационарную вышку)
            bool deliveredToTower = false;
            for (int t = 0; t < towers.get_length(); ++t) {
                if (math::distance(beacons_[i]->getRealPosition(), towers[t]) < g_Settings.meshCommRadius) {
                    // Зеленая связь: Beacon -> Tower
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
                        // Голубая связь: Beacon -> Beacon
                        activeLinks_.append(NetworkLink{beacons_[i]->getRealPosition(), beacons_[j]->getRealPosition(), 0.3f, LinkType::BeaconToBeacon});
                    }
                }
            }
        }
    }
}