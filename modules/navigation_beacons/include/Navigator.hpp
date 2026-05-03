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

public:
    explicit Navigator(const IEnvironment* env);
    ~Navigator() override;

    // Запрет копирования Оркестратора
    Navigator(const Navigator&) = delete;
    Navigator& operator=(const Navigator&) = delete;

    // Реализация интерфейса INavigator
    [[nodiscard]] LocationResult estimateLocation(const Sequence<SignalData>& signals) const override;
    // Главный метод симуляции
    void updateEntities(float deltaTime) override;

    // Методы для взаимодействия (UI и Управление)

    void setTargetVelocity(double dx, double dy) {
        if (target_) target_->setDirection(dx, dy);
    }
    // Геттеры для рендерера 
    [[nodiscard]] const TargetObject* getTarget() const { return target_; }
    [[nodiscard]] const Sequence<MobileBeacon*>& getBeacons() const { return beacons_; }
};