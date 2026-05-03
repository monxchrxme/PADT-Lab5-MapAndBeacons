#pragma once
#include "Structures.hpp"
#include "interfaces/sequence.hpp" 

class TrilaterationSolver {
public:
    // Статический метод, так как класс не хранит состояния (Pure Function)
    // Throws: SignalLostException
    // Point2D currentGuess - точка, от которой начнем поиск
    static LocationResult solve(const Sequence<SignalData>& signals, Point2D currentGuess);
};