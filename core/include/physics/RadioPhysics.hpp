#pragma once
#include <cmath>

enum class TileType {
    EMPTY,    
    FOREST,   
    WATER,    
    WALL,      
    PATH,       
    TOWER_BASE   
};

class RadioPhysics {
public:
    // Статический метод вычисления коэффициента пропускания сквозь 1 тайл препятствия
    // Возвращает долю прошедшей амплитуды (от 0.0 до 1.0)
    [[nodiscard]] static double getTileTransmittance(TileType type, double frequencyGHz) {
        
        // Базовые потери свободного пространства на тайл уже учтены длиной пути
        // Здесь мы считаем только ВНЕСЕННЫЕ ПОТЕРИ материала (Penetration Loss)
        switch (type) {
            case TileType::EMPTY:
            case TileType::PATH:
            case TileType::TOWER_BASE:
                return 1.0; // Полностью радиопрозрачны

            case TileType::WALL: {
                // Модель ITU-R P.2040 для тяжелого бетона (Concrete)
                // e' = a * f^b, sigma = c * f^d
                const double a = 5.24;
                const double b = 0.0;
                const double c = 0.046;
                const double d = 0.7822;

                double epsilon = a * std::pow(frequencyGHz, b);
                double sigma = c * std::pow(frequencyGHz, d);

                // Упрощенная эвристика затухания на основе проводимости (sigma)
                // Чем выше частота, тем выше проводимость бетона -> сильнее потери (превращение в тепло)
                double loss = std::exp(-sigma * frequencyGHz * 0.1); 
                if (loss < 0.01) loss = 0.01;
                return loss;
            }

            case TileType::FOREST: {
                // Модель ITU-R P.833 (Затухание в растительности)
                double loss = std::exp(-0.05 * frequencyGHz); 
                return loss;
            }

            case TileType::WATER: {
                return 0.001; 
            }

            default:
                return 1.0;
        }
    }

    // Здесь позже добавим метод коэффициентов отражения Френеля для отраженных лучей
    [[nodiscard]] static double getReflectionCoefficient(TileType type, double angleRadians, double frequencyGHz) {
        return 0.5; // Пока заглушка для первого этапа
    }
};