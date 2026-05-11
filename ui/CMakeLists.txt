# Собираем все .cpp файлы модуля UI
file(GLOB SOURCES "src/*.cpp")

add_library(ui STATIC ${SOURCES})

# Публичные заголовки
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# САМОЕ ГЛАВНОЕ: говорим модулю UI, что ему нужен SFML и твоя карта
target_link_libraries(ui PUBLIC 
    environment_map 
    navigation_beacons 
    sfml-graphics 
    sfml-window 
    sfml-system
)