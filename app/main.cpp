#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <exception>

#include "MockEnvironment.hpp"
#include "exceptions/EnvironmentExceptions.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "Navigator.hpp"

void runSimulation() {
    IEnvironment* environment = nullptr;
    INavigator* navigator = nullptr;

    try {
        environment = new MockEnvironment();
        navigator = new Navigator(environment);
    } catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << '\n';
        delete navigator;
        delete environment;
        throw;
    }

    // Инициализация графики (SFML + ImGui)
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    
    sf::RenderWindow window(sf::VideoMode(800, 600), "GeoLocation Simulation", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) {
        delete navigator;
        delete environment;
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }

    sf::Clock deltaClock;

    // Главный цикл
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        sf::Time dt = deltaClock.restart();
        ImGui::SFML::Update(window, dt);

        // ЛОГИКА 
        try {
            navigator->updateEntities(dt.asSeconds());
        } catch (const NavigationException& e) {
            // Ловим только ошибки навигации (например, SignalLostException)
            std::cerr << "Navigation Warning: " << e.what() << '\n';
        } catch (const EnvironmentException& e) {
            // Ловим ошибки карты
            std::cerr << "Environment Warning: " << e.what() << '\n';
        }

        // ИНТЕРФЕЙС ImGui
        ImGui::Begin("Simulation Controls");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        if (ImGui::Button("Regenerate Map")) {
            std::cout << "[Debug] Map regeneration triggered.\n";
            environment->triggerLazyGeneration(Point2D{0, 0});
        }
        ImGui::End();

        // ОТРИСОВКА SFML
        window.clear(sf::Color(30, 30, 30));

        // TODO: MapRenderer.render(window, environment);
        // TODO: NavigationRenderer.render(window, navigator);

        // Тестовая отрисовка искомого объекта (красный круг в центре)
        sf::CircleShape targetShape(20.0f);
        targetShape.setFillColor(sf::Color::Red);
        targetShape.setPosition(380.0f, 280.0f);
        window.draw(targetShape);

        ImGui::SFML::Render(window);
        window.display();
    }

    // Безопасная очистка ресурсов при закрытии программы
    ImGui::SFML::Shutdown();
    delete navigator;
    delete environment;
}

int main() {
    try {
        runSimulation();
    } catch (const std::exception& e) {
        std::cerr << "FATAL CRASH (Uncaught Exception): " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "FATAL CRASH: Unknown exception occurred.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}