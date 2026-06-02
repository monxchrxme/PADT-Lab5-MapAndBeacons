#include "math/AoASolver.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include <cmath>

LocationResult AoASolver::solve(const Sequence<ProcessedSignal>& signals) {
    const int count = signals.get_length();

    // Для пересечения лучей (прямых) нужно минимум 2 источника
    if (count < 2) {
        throw SignalLostException();
    }

    // Составляем переопределенную СЛАУ: A * X = b
    // Каждое уравнение прямой: sin(θ) * X - cos(θ) * Y = sin(θ) * x_i - cos(θ) * y_i
    // Мы будем решать нормальное уравнение МНК: (A^T * A) * X = A^T * b

    // Элементы матрицы A^T * A (размер 2x2)
    double M00 = 0.0; // sum(sin^2)
    double M11 = 0.0; // sum(cos^2)
    double M01 = 0.0; // sum(-sin * cos)  (Матрица симметрична, поэтому M10 = M01)

    // Элементы вектора A^T * b (размер 2x1)
    double V0 = 0.0;
    double V1 = 0.0;

    for (int i = 0; i < count; ++i) {
        ProcessedSignal sig = signals[i];
        
        double s = std::sin(sig.azimuth);
        double c = std::cos(sig.azimuth);
        
        // Свободный член (b_i) для текущего луча
        double b_i = s * sig.sourcePos.x - c * sig.sourcePos.y;

        // Весовой коэффициент (Weighted Least Squares) 
        // Сильным сигналам доверяем больше
        double weight = sig.rssi; 

        // Накапливаем суммы с учетом веса
        M00 += weight * (s * s);
        M11 += weight * (c * c);
        M01 += weight * (-s * c);

        V0 += weight * (s * b_i);
        V1 += weight * (-c * b_i);
    }

    // Вычисляем детерминант матрицы 2x2
    double det = M00 * M11 - M01 * M01;

    // Если детерминант близок к нулю, значит все вышки выстроились в одну линию 
    // или лучи параллельны (плохая обусловленность матрицы - ill-conditioned)
    if (std::abs(det) < 1e-6) {
        throw TrilaterationMathException();
    }

    // Аналитическое обращение матрицы 2x2 и умножение на вектор: X = (A^T * A)^-1 * (A^T * b)
    Point2D estimatedPos;
    estimatedPos.x = (M11 * V0 - M01 * V1) / det;
    estimatedPos.y = (-M01 * V0 + M00 * V1) / det;

    // ОЦЕНКА ПОГРЕШНОСТИ (Residual Error)
    // В методе МНК радиус ошибки — это среднее перпендикулярное расстояние 
    // от найденной точки до каждого направленного луча (прямой)
    double totalErrorRadius = 0.0;
    
    for (int i = 0; i < count; ++i) {
        ProcessedSignal sig = signals[i];
        double s = std::sin(sig.azimuth);
        double c = std::cos(sig.azimuth);
        double b_i = s * sig.sourcePos.x - c * sig.sourcePos.y;
        
        // Геометрическое расстояние от точки (X, Y) до прямой A*x + B*y = C
        double distanceToLine = std::abs(s * estimatedPos.x - c * estimatedPos.y - b_i);
        totalErrorRadius += distanceToLine;
    }
    totalErrorRadius /= count;

    // Добавляем базовую неуверенность аппаратуры (Noise Floor)
    totalErrorRadius += g_Settings.hardwareJitter;

    return LocationResult{ estimatedPos, totalErrorRadius };
}