#pragma once
#include "IEnvironment.hpp"
#include "sequences/mutable_array_sequence.hpp" 
#include "Structures.hpp"
#include "MathUtils.hpp"
#include <cmath> 

class MockEnvironment final : public IEnvironment {
public:
    MockEnvironment() = default;
    ~MockEnvironment() override = default;

    // Запрет копирования (Rule of 5)
    MockEnvironment(const MockEnvironment&) = delete;
    MockEnvironment& operator=(const MockEnvironment&) = delete;

    [[nodiscard]] bool isPassable(Point2D p) const override {
        // Ограничиваем карту размером окна
        if (p.x < 20.0 || p.x > 780.0 || p.y < 20.0 || p.y > 580.0) return false;
        
        // ТЕСТОВАЯ СТЕНА (Бетонный блок по центру) 
        if (p.x > 380.0 && p.x < 420.0 && p.y > 200.0 && p.y < 400.0) return false;

        return true; 
    }

    void triggerLazyGeneration(Point2D /*currentPos*/) override {
        // Пока ничего не делаем
    }

    [[nodiscard]] double calculateSignal(Point2D source, Point2D target) const override {
        double dist = std::hypot(target.x - source.x, target.y - source.y);
        if (dist < 1.0) dist = 1.0;
        
        double signal = g_Settings.baseTxPower / (dist * dist);

        // Имитируем "шум эфира"
        double noiseFactor = math::randomDouble(1.0 - g_Settings.noiseVariation, 1.0 + g_Settings.noiseVariation);
        signal *= noiseFactor;

        // RAYCASTING (Проверка пересечения луча со стеной)
        // Если источник и цель по разные стороны от x=400
        if ((source.x < 400.0 && target.x > 400.0) || (source.x > 400.0 && target.x < 400.0)) {
            // Находим точку пересечения луча с линией x=400
            double intersectY = source.y + (400.0 - source.x) * (target.y - source.y) / (target.x - source.x);
            // Если точка пересечения попадает в нашу стену (y от 200 до 400)
            if (intersectY > 200.0 && intersectY < 400.0) {
                // Стена пропускает только часть сигнала (зависит от настроек)
                signal *= g_Settings.wallAttenuation; 
            }
        }
        return signal;
    }

    [[nodiscard]] MutableArraySequence<Point2D> getStaticTowers() const override {
        MutableArraySequence<Point2D> towers;
        // Расставляем 4 стационарные вышки (как спутники GPS) по краям нашей зоны 800x600
        towers.append(Point2D{100.0, 100.0}); // Левый верхний
        towers.append(Point2D{700.0, 100.0}); // Правый верхний
        towers.append(Point2D{100.0, 500.0}); // Левый нижний
        towers.append(Point2D{700.0, 500.0}); // Правый нижний
        return towers; 
    }
};