# Это графический модуль. Он знает про SFML и про оба логических модуля.

file(GLOB SOURCES "src/*.cpp")

add_library(ui STATIC ${SOURCES}
        include/NavigationRenderer.hpp
        src/NavigationRenderer.cpp)

target_include_directories(ui PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external/sfml/include 
)

# Здесь происходит магия объединения: UI зависит от карты, навигации и SFML.
# SFML состоит из нескольких модулей. Нам нужны графика, окно и система.
target_link_libraries(ui PUBLIC 
        core
        environment_map
        navigation_beacons
        sfml-graphics
        sfml-window
        sfml-system
        ImGui-SFML::ImGui-SFML # Подключаем графику и ImGui-SFML
)