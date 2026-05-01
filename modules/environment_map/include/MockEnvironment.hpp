#pragma once
#include "IEnvironment.hpp"
#include "sequences/mutable_array_sequence.hpp" 

class MockEnvironment final : public IEnvironment {
public:
    MockEnvironment() = default;
    ~MockEnvironment() override = default;

    // Запрет копирования (Rule of 5)
    MockEnvironment(const MockEnvironment&) = delete;
    MockEnvironment& operator=(const MockEnvironment&) = delete;

    [[nodiscard]] bool isPassable(Point2D /*point*/) const override {
        return true; // В заглушке препятствий нет
    }

    [[nodiscard]] double calculateSignal(Point2D /*source*/, Point2D /*target*/) const override {
        return 1.0; // Сигнал идеальный
    }

    void triggerLazyGeneration(Point2D /*currentPos*/) override {
        // Пока ничего не делаем
    }

    // Возвращаем конкретный тип по значению (RVO - Return Value Optimization предотвратит лишнее копирование)
    [[nodiscard]] MutableArraySequence<Point2D> getStaticTowers() const override {
        MutableArraySequence<Point2D> towers;
        // Для теста добавим одну вышку в центр (если в твоей библиотеке есть метод добавления, например Append)
        towers.append(Point2D{400.0, 300.0}); 
        return towers; 
    }
};