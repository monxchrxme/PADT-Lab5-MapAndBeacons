#pragma once
#include "INavigator.hpp"
#include "exceptions/NavigationExceptions.hpp" 
#include <stdexcept>

class MockNavigator final : public INavigator {
private:
    const IEnvironment* environment_; // Невладеющий сырой указатель (Raw non-owning pointer)

public:
    explicit MockNavigator(const IEnvironment* env) : environment_(env) {
        if (!environment_) {
            throw std::invalid_argument("MockNavigator: IEnvironment pointer cannot be null.");
        }
    }
    
    ~MockNavigator() override = default;

    MockNavigator(const MockNavigator&) = delete;
    MockNavigator& operator=(const MockNavigator&) = delete;

    // Принимаем абстрактный Sequence по ссылке
    [[nodiscard]] LocationResult estimateLocation(const Sequence<SignalData>& /*signals*/) const override {
        // Возвращаем тестовую позицию
        return LocationResult{ Point2D{400.0, 300.0}, 50.0 };
    }

    void updateEntities(float /*deltaTime*/) override {
        // Сущности пока не двигаются
    }
};