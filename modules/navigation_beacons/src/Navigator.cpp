#include "Navigator.hpp"
#include "math/TrilaterationSolver.hpp"
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

LocationResult Navigator::estimateLocation(const Sequence<SignalData>& signals) const {
    // Передаем центр экрана {400.0, 300.0} как начальную точку для поиска
    return TrilaterationSolver::solve(signals, Point2D{400.0, 300.0});
}

void Navigator::updateEntities(float dt) {
    // Обновляем все мобильные маяки 
    for (int i = 0; i < beacons_.get_length(); ++i) {
        MobileBeacon* beacon = beacons_[i];
        
        // Маяк шагает
        beacon->updatePhysics(dt, environment_);

        try {
            // Маяк вычисляет свое положение (опрашивая Карту)
            beacon->updateEstimation(environment_, beacons_); 
        } catch (...) {
            // Игнорируем ошибки соединения, маяк продолжит двигаться по физике, пока снова не поймает сигнал.
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
}