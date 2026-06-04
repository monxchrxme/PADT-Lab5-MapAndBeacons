#pragma once
#include "Structures.hpp"
#include "../../external/sequence/src/sequences/mutable_array_sequence.hpp"

class IEnvironment {
public:
    virtual ~IEnvironment() = default;

    [[nodiscard]] virtual bool isPassable(Point2D point) const = 0;

    // Навигатор запрашивает все пути между двумя точками на заданной частоте
    // Карта использует DDA и RadioPhysics для генерации прямого и отраженных лучей
    [[nodiscard]] virtual MutableArraySequence<RadioPath> computePaths(Point2D tx, Point2D rx, double frequencyGHz) const = 0;

    // Триггер для ленивой генерации чанков (вызывается при движении объектов)
    virtual void triggerLazyGeneration(Point2D currentPos) = 0;

    // Получить список сгенерированных стационарных вышек
    [[nodiscard]] virtual MutableArraySequence<Point2D> getStaticTowers() const = 0;
};