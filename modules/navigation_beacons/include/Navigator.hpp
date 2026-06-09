#pragma once
#include "INavigator.hpp"
#include "entities/TargetObject.hpp"
#include "entities/MobileBeacon.hpp"
#include "sequences/mutable_array_sequence.hpp"

class Navigator final : public INavigator {
private:
    const IEnvironment* environment_; // Невладеющий указатель на карту 
    
    // Владеющие указатели (обязаны сделать delete)
    TargetObject* target_;
    MutableArraySequence<MobileBeacon*> beacons_;

    // ДАННЫЕ ДЛЯ СЕТИ 
    MutableArraySequence<NetworkLink> activeLinks_; // Линии передачи для UI
    int packetIdCounter_ = 0;                       // Генератор уникальных ID
    float timeSinceLastPacket_ = 0.0f;              // Таймер отправки

public:
    explicit Navigator(const IEnvironment* env);
    ~Navigator() override;

    // Запрет копирования Оркестратора
    Navigator(const Navigator&) = delete;
    Navigator& operator=(const Navigator&) = delete;

    // Главный метод симуляции
    void updateEntities(float deltaTime) override;

    // Методы для взаимодействия (UI и Управление)

    void setTargetVelocity(double dx, double dy) override {
        if (target_) target_->setDirection(dx, dy);
    }
    // Геттеры для рендерера 
    [[nodiscard]] const TargetObject* getTarget() const override { return target_; }
    [[nodiscard]] const Sequence<MobileBeacon*>& getBeacons() const override { return beacons_; }
    [[nodiscard]] MutableArraySequence<NetworkLink> getActiveLinks() const override { return activeLinks_; }
};