add_executable(GeoLocationApp main.cpp)

target_link_libraries(GeoLocationApp PRIVATE 
    ui 
    environment_map 
    navigation_beacons 
    core
    sfml-graphics
    sfml-window
    sfml-system
    ImGui-SFML::ImGui-SFML
)
