#pragma once
#include "Structures.hpp"
#include "interfaces/sequence.hpp"
#include "MathUtils.hpp"
#include <complex>
#include <cmath>
#include <numbers>

class SignalProcessor {
public:
    // Принимает массив путей от одной вышки и возвращает один итоговый скомбинированный сигнал
    static ProcessedSignal processPaths(const Sequence<RadioPath>& paths, Point2D txPos, double freqGHz) {
        
        // Физические параметры среды (считаем 1 пиксель = 1 метр)
        // Скорость света = 3 * 10^8 м/с. Частота в Гц = freqGHz * 10^9
        // Длина волны (lambda) = 0.3 / freqGHz
        const double lambda = 0.3 / freqGHz;
        const double k = 2.0 * std::numbers::pi / lambda; // Волновое число

        // Инициализируем 3 виртуальные антенны (I/Q аккумуляторы)
        std::complex<double> E_center(0.0, 0.0);
        std::complex<double> E_x(0.0, 0.0);
        std::complex<double> E_y(0.0, 0.0);

        // Разнос антенн на четверть волны (чтобы избежать фазового алиасинга > 180 градусов)
        const double offset = lambda / 4.0;

        // Обрабатываем каждый пришедший луч (LOS и переотражения)
        for(int i = 0; i < paths.get_length(); ++i) {
            RadioPath path = paths[i];

            // Амплитуда луча = (sqrt(Power) / d) * attenuation
            if (path.distance < 1.0) path.distance = 1.0;
            double amplitude = (std::sqrt(g_Settings.baseTxPower) / path.distance) * path.attenuation;

            // Базовая фаза на центральной антенне
            double phase = k * path.distance;

            // Вектор прихода луча (направляющие косинусы)
            double vx = path.arrivalVector.x;
            double vy = path.arrivalVector.y;

            // Сложение фазоров (Суперпозиция электромагнитных волн)
            E_center += std::polar(amplitude, phase);

            // Фаза на антенне X (луч проходит доп. расстояние offset * vx)
            double phase_x = phase + k * (vx * offset);
            E_x += std::polar(amplitude, phase_x);

            // Фаза на антенне Y (луч проходит доп. расстояние offset * vy)
            double phase_y = phase + k * (vy * offset);
            E_y += std::polar(amplitude, phase_y);
        }

        // Вычисление итоговой мощности (RSSI)
        // Квадрат модуля комплексного числа (автоматически учитывает интерференционные ямы)
        double finalRSSI = std::norm(E_center);

        // Если сигнал заглушен в ноль, возвращаем пустую структуру
        if (finalRSSI < g_Settings.signalThreshold) {
            return ProcessedSignal{txPos, finalRSSI, 0.0};
        }

        // ПЕЛЕНГАЦИЯ (Angle of Arrival)
        // Вычисляем разность фаз между центральной и смещенными антеннами
        // arg(E_x * conj(E_center)) дает точную разность фаз
        double dPhi_x = std::arg(E_x * std::conj(E_center));
        double dPhi_y = std::arg(E_y * std::conj(E_center));

        // Имитируем тепловой шум АЦП на микро-антеннах (Аппаратная погрешность)
        // Чем меньше g_Settings.hardwareJitter, тем точнее компас
        double jitterX = math::randomDouble(-g_Settings.hardwareJitter, g_Settings.hardwareJitter) * 0.01;
        double jitterY = math::randomDouble(-g_Settings.hardwareJitter, g_Settings.hardwareJitter) * 0.01;

        // Вычисляем финальный Азимут (от -Pi до Pi)
        double azimuth = std::atan2(dPhi_y + jitterY, dPhi_x + jitterX);

        return ProcessedSignal{ txPos, finalRSSI, azimuth };
    }
};