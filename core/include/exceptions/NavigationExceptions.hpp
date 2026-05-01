#pragma once
#include <stdexcept>
#include <string>

// Базовый класс для всех ошибок навигации (позволяет отловить любую ошибку модуля одним catch)
class NavigationException : public std::runtime_error {
public:
    explicit NavigationException(const std::string& message) 
        : std::runtime_error(message) {}
};

class SignalLostException final : public NavigationException {
public:
    SignalLostException() 
        : NavigationException("Signal lost: insufficient data for localization.") {}
};

class TrilaterationMathException final : public NavigationException {
public:
    TrilaterationMathException() 
        : NavigationException("Mathematical error: failed to resolve coordinates from given signals.") {}
};