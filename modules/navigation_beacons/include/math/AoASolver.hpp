#pragma once
#include "Structures.hpp"
#include "interfaces/sequence.hpp"

class AoASolver {
public:
    // Принимает массив обработанных сигналов (где уже есть вычисленный Азимут)
    // Throws: SignalLostException (если сигналов < 2)
    // Throws: TrilaterationMathException (если матрица вырождена/параллельные лучи)
    static LocationResult solve(const Sequence<ProcessedSignal>& signals);
};