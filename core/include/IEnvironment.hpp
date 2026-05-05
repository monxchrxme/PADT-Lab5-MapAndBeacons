#pragma once
#include "Structures.hpp"
#include "../../external/sequence/src/sequences/mutable_array_sequence.hpp"

class IEnvironment {
public:
    virtual ~IEnvironment() = default;

    // Проверяет, свободна ли точка от стен/препятствий (чтобы сущности не ходили сквозь стены)
    [[nodiscard]] virtual bool isPassable(Point2D point) const = 0;

    // Трассировка луча: вычисляет уровень сигнала (RSSI) между двумя точками с учетом препятствий
    [[nodiscard]] virtual double calculateSignal(Point2D source, Point2D target) const = 0;

    // Триггер для ленивой генерации чанков (вызывается при движении объектов)
    virtual void triggerLazyGeneration(Point2D currentPos) = 0;

    // Получить список сгенерированных стационарных вышек
    [[nodiscard]] virtual MutableArraySequence<Point2D> getStaticTowers() const = 0;
};