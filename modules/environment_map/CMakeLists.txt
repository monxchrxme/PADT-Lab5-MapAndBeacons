# Собираем все .cpp файлы модуля (генерация, raycasting)
file(GLOB SOURCES "src/*.cpp")

add_library(environment_map STATIC ${SOURCES} include/MockEnvironment.hpp)

# Указываем компилятору, ГДЕ искать заголовочные файлы
target_include_directories(environment_map PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external/sequence/src   
)

# УБРАЛИ sequence отсюда! Оставляем только core
target_link_libraries(environment_map PUBLIC core)