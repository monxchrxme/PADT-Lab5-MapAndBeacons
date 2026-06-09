#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <exception>

#include "EnvironmentManager.hpp"
#include "MapRenderer.hpp"
#include "exceptions/EnvironmentExceptions.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "Navigator.hpp"
#include "NavigationRenderer.hpp"

void runSimulation() {
    IEnvironment* environment = nullptr;
    INavigator* navigator = nullptr;

    try {
        environment = new EnvironmentManager();
        // Принудительно генерируем мир вокруг точки спавна объекта (400, 300)
        environment->triggerLazyGeneration(Point2D{400.0, 300.0});
        
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
    window.setFramerateLimit(90);

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
    MapRenderer mapRenderer; 

    float currentZoom = 1.0f; //стандартный масштаб

    // Главный цикл
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseWheelScrolled && !ImGui::GetIO().WantCaptureMouse) {
                if (event.mouseWheelScroll.delta > 0) {
                    currentZoom *= 0.8f; // Крутим вверх -> Приближаем
                } else {
                    currentZoom *= 1.2f; // Крутим вниз -> Отдаляем
                }
                // Ограничиваем зум (от х0.2 до х3.0)
                currentZoom = std::clamp(currentZoom, 0.2f, 3.0f);
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
            if (navigator->getTarget()) {
                environment->triggerLazyGeneration(navigator->getTarget()->getRealPosition());            }
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
        ImGui::SliderFloat("Частота (ГГц)", &g_Settings.frequencyGHz, 0.9f, 6.0f, "%.1f");
        ImGui::SliderFloat("Мощность (TxPower)", &g_Settings.baseTxPower, 10000.0f, 500000.0f, "%.0f");
        ImGui::SliderFloat("Порог приема", &g_Settings.signalThreshold, 0.001f, 0.1f, "%.3f");

        ImGui::Separator();
        ImGui::Text("DSP и Пеленгация");
        ImGui::SliderFloat("Сглаживание (Lerp)", &g_Settings.smoothing, 0.01f, 1.0f, "%.2f");
        ImGui::SliderFloat("Аппаратный шум (Jitter)", &g_Settings.hardwareJitter, 0.0f, 50.0f, "%.1f"); 
        
        ImGui::Separator();
        ImGui::Text("Отрисовка (Debug Draw)");
        ImGui::Checkbox("Показать Mesh-сеть", &g_Settings.showMeshNetwork);
        ImGui::SliderFloat("Радиус связи (Mesh)", &g_Settings.meshCommRadius, 50.0f, 800.0f, "%.0f");
        ImGui::Checkbox("Показать лучи пеленгации (AoA)", &g_Settings.showAoARays); 

        ImGui::Separator();
        if (ImGui::Button("Regenerate Map")) {
            environment->triggerLazyGeneration(Point2D{0, 0});
        }
        ImGui::End();

        // ОТРИСОВКА SFML
        window.clear(sf::Color(30, 30, 30));

        // Настраиваем камеру на объект
        if (navigator->getTarget()) {
            sf::View view = window.getDefaultView(); // Берем стандартный размер
            view.zoom(currentZoom);
            Point2D pos = navigator->getTarget()->getRealPosition();
            // Центрируем камеру точно на красной точке!
            view.setCenter(static_cast<float>(pos.x), static_cast<float>(pos.y));
            window.setView(view); // Применяем камеру к окному
        }

        // Динамически приводим интерфейс к конкретному классу, чтобы получить чанки
        auto* concreteEnv = dynamic_cast<EnvironmentManager*>(environment);
        if (concreteEnv) {
            mapRenderer.render(window, concreteEnv);
        }

        // TODO: NavigationRenderer.render(window, navigator);
        renderer.render(window, navigator, environment);

        // Возвращаем камеру по умолчанию, чтобы меню рисовалось на экране ровно
        window.setView(window.getDefaultView());

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