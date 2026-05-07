#pragma once
#include <exception>
#include <string>

class EnvironmentException : public std::exception {
protected:
    std::string message;
public:
    explicit EnvironmentException(const char* msg) : message(msg) {}
    explicit EnvironmentException(const std::string& msg) : message(msg) {}
    
    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }
};

class OutOfBoundsException : public EnvironmentException {
public:
    explicit OutOfBoundsException(const char* msg = "Coordinates out of bounds") 
        : EnvironmentException(msg) {}
};

class ChunkGenerationException : public EnvironmentException {
public:
    explicit ChunkGenerationException(const char* msg = "Failed to generate map chunk") 
        : EnvironmentException(msg) {}
};