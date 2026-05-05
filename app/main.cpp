#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <exception>

#include "EnvironmentManager.hpp"
#include "exceptions/EnvironmentExceptions.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "Navigator.hpp"
#include "NavigationRenderer.hpp"

void runSimulation() {
    IEnvironment* environment = nullptr;
    INavigator* navigator = nullptr;

    try {
        environment = new EnvironmentManager();
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

    // ФИКС ДЛЯ РУССКОГО ЯЗЫКА

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear(); 
    
    // Загружаем Arial из системы. 16.0f - размер шрифта.
    // передаем GetGlyphRangesCyrillic(), чтобы ImGui загрузил русские буквы
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    
    if (!ImGui::SFML::UpdateFontTexture()) {
        std::cerr << "Warning: Failed to update ImGui font texture.\n";
    }

    sf::Clock deltaClock;

    NavigationRenderer renderer;

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

        // УПРАВЛЕНИЕ С КЛАВИАТУРЫ (WASD)
        double dx = 0.0;
        double dy = 0.0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    dy -= 1.0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  dy += 1.0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  dx -= 1.0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dx += 1.0;
        
        // Передаем вектор направления в навигатор
        navigator->setTargetVelocity(dx, dy);

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
        ImGui::Separator();
        
        ImGui::Text("Физика эфира");
        ImGui::SliderFloat("Мощность (TxPower)", &g_Settings.baseTxPower, 10000.0f, 500000.0f, "%.0f");
        ImGui::SliderFloat("Шум среды (+/- %)", &g_Settings.noiseVariation, 0.0f, 0.5f, "%.2f");
        ImGui::SliderFloat("Порог приема", &g_Settings.signalThreshold, 0.01f, 0.2f, "%.3f");
        
        ImGui::Separator();
        ImGui::Text("Математика Навигации");
        ImGui::SliderFloat("Сглаживание (Lerp)", &g_Settings.smoothing, 0.01f, 1.0f, "%.2f");
        ImGui::SliderFloat("Градиентный шаг", &g_Settings.learningRate, 0.05f, 2.0f, "%.2f");
        ImGui::SliderFloat("Аппаратная погрешность", &g_Settings.baseUncertainty, 0.0f, 50.0f, "%.1f");
        ImGui::SliderFloat("Пропускаемость стены", &g_Settings.wallAttenuation, 0.05f, 1.0f, "%.2f");

        ImGui::Separator();
        if (ImGui::Button("Regenerate Map")) {
            environment->triggerLazyGeneration(Point2D{0, 0});
        }
        ImGui::End();

        // ОТРИСОВКА SFML
        window.clear(sf::Color(30, 30, 30));

        // TODO: MapRenderer.render(window, environment);
        // TODO: NavigationRenderer.render(window, navigator);

        window.clear(sf::Color(30, 30, 30));
        renderer.render(window, navigator, environment);

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