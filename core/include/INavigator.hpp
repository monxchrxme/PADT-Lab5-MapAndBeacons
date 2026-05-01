#pragma once
#include "Structures.hpp"
#include "interfaces/sequence.hpp"

class INavigator {
public:
    virtual ~INavigator() = default;

    // Математика трилатерации: вычисляет позицию на основе списка полученных сигналов.
    // Может выбросить SignalLostException, если данных не хватает.
    [[nodiscard]] virtual LocationResult estimateLocation(const Sequence<SignalData>& signals) const = 0;

    // Физика: сдвигает все маяки и объекты на шаг по времени (dt)
    virtual void updateEntities(float deltaTime) = 0;
};