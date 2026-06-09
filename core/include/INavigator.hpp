#pragma once
#include "Structures.hpp"
#include "interfaces/sequence.hpp"
#include "sequences/mutable_array_sequence.hpp" 

class TargetObject;
class MobileBeacon;

class INavigator {
public:
    virtual ~INavigator() = default;

    // Физика: сдвигает все маяки и объекты на шаг по времени (dt)
    virtual void updateEntities(float deltaTime) = 0;

    // МЕТОДЫ ДЛЯ UI И УПРАВЛЕНИЯ
    
    virtual void setTargetVelocity(double dx, double dy) = 0;
    
    [[nodiscard]] virtual const TargetObject* getTarget() const = 0;
    [[nodiscard]] virtual const Sequence<MobileBeacon*>& getBeacons() const = 0;
    // Возвращает активные сетевые соединения для отрисовки
    [[nodiscard]] virtual MutableArraySequence<NetworkLink> getActiveLinks() const = 0;
};