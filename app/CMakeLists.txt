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

target_include_directories(GeoLocationApp PRIVATE 
    ${CMAKE_SOURCE_DIR}/external/sfml/include
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui_sfml
)