#pragma once
#include <stdexcept>
#include <string>

// Базовый класс для всех ошибок карты
class EnvironmentException : public std::runtime_error {
public:
    explicit EnvironmentException(const std::string& message) 
        : std::runtime_error(message) {}
};

class ChunkGenerationException final : public EnvironmentException {
public:
    ChunkGenerationException() 
        : EnvironmentException("Environment error: failed to generate map chunk.") {}
};

class InvalidCoordinateException final : public EnvironmentException {
public:
    InvalidCoordinateException() 
        : EnvironmentException("Environment error: requested coordinates are out of valid bounds.") {}
};