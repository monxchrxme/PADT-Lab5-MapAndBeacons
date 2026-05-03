#pragma once
#include "Structures.hpp"
#include "interfaces/sequence.hpp" 

class TrilaterationSolver {
public:
    // Статический метод, так как класс не хранит состояния (Pure Function)
    // Throws: SignalLostException
    static LocationResult solve(const Sequence<SignalData>& signals);
};