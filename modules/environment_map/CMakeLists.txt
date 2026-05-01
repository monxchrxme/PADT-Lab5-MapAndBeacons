# Собираем все .cpp файлы модуля (генерация, raycasting)
file(GLOB SOURCES "src/*.cpp")

add_library(environment_map STATIC ${SOURCES}
        include/MockEnvironment.hpp)

# Публичные заголовки модуля лежат в include/
# PUBLIC значит, что и сам модуль, и те, кто его прилинкуют, увидят эти .h файлы
target_include_directories(environment_map PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Модулю карты нужны интерфейсы из core
target_link_libraries(environment_map PUBLIC core)