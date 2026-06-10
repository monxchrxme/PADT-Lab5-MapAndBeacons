#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <exception>
#include <algorithm> 

#include "EnvironmentManager.hpp"
#include "MapRenderer.hpp"
#include "exceptions/EnvironmentExceptions.hpp"
#include "exceptions/NavigationExceptions.hpp"
#include "Navigator.hpp"
#include "NavigationRenderer.hpp"

enum class AppState 
{
    MAIN_MENU,
    SIMULATION
};

void runApplication() 
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(1024, 768), "GeoLocation Simulation", sf::Style::Default, settings);
    window.setFramerateLimit(90);

    if (!ImGui::SFML::Init(window)) 
    {
        throw std::runtime_error("Failed to initialize ImGui-SFML");
    }

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear(); 
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
   (void)ImGui::SFML::UpdateFontTexture();

    //Базовые переменные
    sf::Clock deltaClock;
    NavigationRenderer renderer;
    MapRenderer mapRenderer; 

    IEnvironment* environment = nullptr;
    INavigator* navigator = nullptr;

    AppState currentState = AppState::MAIN_MENU;
    std::string errorMessage = "";

    float currentZoom = 1.0f;
    bool isCameraLocked = true;
    sf::Vector2i lastMousePos;
    sf::View view = window.getDefaultView();

    //Главный цикл
    while (window.isOpen()) 
    {
        sf::Event event{};
        while (window.pollEvent(event)) 
        {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) 
            {
                window.close();
                break;
            }

            if (currentState == AppState::SIMULATION) 
            {
                if (event.type == sf::Event::Resized) 
                {
                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                    view.setSize(event.size.width * currentZoom, event.size.height * currentZoom);
                }

                if (event.type == sf::Event::MouseWheelScrolled && !ImGui::GetIO().WantCaptureMouse) 
                {
                    if (event.mouseWheelScroll.delta > 0) 
                    {
                        currentZoom *= 0.8f;
                    }else 
                    {
                        currentZoom *= 1.2f;
                    }
                    currentZoom = std::clamp(currentZoom, 0.2f, 15.0f);
                    view.setSize(window.getSize().x * currentZoom, window.getSize().y * currentZoom);
                }

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) 
                {
                    if (!ImGui::GetIO().WantCaptureMouse) 
                    { 
                        isCameraLocked = false; 
                        lastMousePos = sf::Mouse::getPosition(window);
                    }
                }
            }
        }

        if (!window.isOpen()) 
        {
            break;
        }

        sf::Time dt = deltaClock.restart();
        ImGui::SFML::Update(window, dt);
        window.clear(sf::Color(20, 20, 20)); 

        if (currentState == AppState::MAIN_MENU) 
        {
            ImGui::SetNextWindowPos(ImVec2(window.getSize().x / 2.0f - 150.0f, window.getSize().y / 2.0f - 100.0f));
            ImGui::SetNextWindowSize(ImVec2(300.0f, 150.0f));
            ImGui::Begin("Главное Меню", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            
            if (ImGui::Button("Генерировать Новый Мир", ImVec2(280.0f, 40.0f))) 
            {
                try 
                {
                    std::remove("world_save.bin"); 
                    environment = new EnvironmentManager(EnvironmentManager::Mode::NEW_GAME);
                    environment->triggerLazyGeneration(Point2D{400.0, 300.0});
                    navigator = new Navigator(environment);
                    
                    currentState = AppState::SIMULATION;
                    errorMessage = "";
                    currentZoom = 1.0f; 
                } catch (const std::exception& e) 
                {
                    errorMessage = e.what();
                }
            }

            ImGui::Separator();

            if (ImGui::Button("Показать загруженное", ImVec2(280.0f, 40.0f))) 
            {
                try 
                {
                    auto* envManager = new EnvironmentManager(EnvironmentManager::Mode::LOAD_GAME);
                    envManager->loadWorldFromDisk(); 
                    
                    environment = envManager;
                    navigator = nullptr;
                    currentState = AppState::SIMULATION;
                    errorMessage = "";

                    isCameraLocked = false; 
                    MapBounds bounds = envManager->getWorldBounds();
                    if (bounds.maxX > bounds.minX) 
                    {
                        float worldWidth = static_cast<float>(bounds.maxX - bounds.minX);
                        float worldHeight = static_cast<float>(bounds.maxY - bounds.minY);
                        float centerX = static_cast<float>(bounds.minX + worldWidth / 2.0f);
                        float centerY = static_cast<float>(bounds.minY + worldHeight / 2.0f);
                        view.setCenter(centerX, centerY);

                        float zoomX = (worldWidth / window.getSize().x) * 1.1f;
                        float zoomY = (worldHeight / window.getSize().y) * 1.1f;
                        currentZoom = std::max(zoomX, zoomY); 
                        
                        view.setSize(window.getSize().x * currentZoom, window.getSize().y * currentZoom);
                    }
                } catch (const EnvironmentException& e) 
                {
                    errorMessage = e.what();
                    if (environment) 
                    { 
                        delete environment; 
                        environment = nullptr; 
                    }
                }
            }

            if (!errorMessage.empty()) 
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Ошибка:");
                ImGui::TextWrapped("%s", errorMessage.c_str());
            }

            ImGui::End();

        } else if (currentState == AppState::SIMULATION) 
        {
            if (!isCameraLocked && sf::Mouse::isButtonPressed(sf::Mouse::Left)) 
            {
                sf::Vector2i newMousePos = sf::Mouse::getPosition(window);
                sf::Vector2f delta = window.mapPixelToCoords(lastMousePos, view) - window.mapPixelToCoords(newMousePos, view);
                view.move(delta);
                lastMousePos = newMousePos;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) 
            {
                isCameraLocked = true;
            }

            double dx = 0.0; double dy = 0.0;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    
            {
                dy -= 1.0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  
            {
                dy += 1.0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  
            {
                dx -= 1.0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) 
            {
                dx += 1.0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) 
            {
                dx *= 3.0; 
                dy *= 3.0;
            }
            if (navigator) 
            {
                navigator->setTargetVelocity(dx, dy);
            }

            try 
            {
                if (navigator) 
                {
                    navigator->updateEntities(dt.asSeconds());
                }
                if (navigator && navigator->getTarget() && environment) 
                {
                    Point2D genPoint = isCameraLocked ? navigator->getTarget()->getRealPosition() 
                                                      : Point2D{view.getCenter().x, view.getCenter().y};
                    environment->triggerLazyGeneration(genPoint);            
                }
            } catch (...) { }

            //Интерфейс ImGui
            ImGui::Begin("Simulation Controls");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Separator();
            
            //Ползунки настроек 
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

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); 
            if (ImGui::Button("Назад в главное меню", ImVec2(ImGui::GetWindowWidth() * 0.9f, 30.0f))) 
            {
                if (navigator) 
                { 
                    delete navigator; 
                    navigator = nullptr; 
                }
                if (environment) 
                { 
                    delete environment; 
                    environment = nullptr; 
                }
                currentState = AppState::MAIN_MENU;
            }
            ImGui::PopStyleColor();

            ImGui::End();

            //Если всё еще в симуляции, то рисуем чанки
            if (currentState == AppState::SIMULATION) 
            {
                if (isCameraLocked && navigator && navigator->getTarget()) 
                {
                    Point2D pos = navigator->getTarget()->getRealPosition();
                    view.setCenter(static_cast<float>(pos.x), static_cast<float>(pos.y));
                }
                window.setView(view); 
                
                auto* concreteEnv = dynamic_cast<EnvironmentManager*>(environment);
                if (concreteEnv) 
                {
                    mapRenderer.render(window, concreteEnv);
                }
                
                if (navigator && environment) 
                {
                    renderer.render(window, navigator, environment);
                }
            }
            //Возврат камеры
            window.setView(window.getDefaultView());
        }

        ImGui::SFML::Render(window);
        window.display();
    } 

    //Очистка при закрытии окна
    ImGui::SFML::Shutdown();
    if (navigator) 
    {
        delete navigator;
    }
    if (environment) 
    {
        delete environment;
    }
}

int main() 
{
    try 
    {
        runApplication();
    } catch (const std::exception& e) 
    {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}