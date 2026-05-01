cmake_minimum_required(VERSION 3.20)
project(GeoLocationSim CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Экспорт для IDE (Clangd / IntelliSense)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Фикс для MSVC
if(MSVC OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
            CACHE STRING "" FORCE
    )
endif()

include(FetchContent)

# 1. Скачиваем GoogleTest
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 2. Скачиваем библиотеку Sequence
FetchContent_Declare(
        sequence_lib
        GIT_REPOSITORY https://github.com/monxchrxme/PADT-Lab2-Sequences.git
        GIT_TAG        main 
)
FetchContent_MakeAvailable(sequence_lib)

# 3. Скачиваем SFML (Графика)
FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        2.6.1
)
FetchContent_MakeAvailable(sfml)

# 4. Скачиваем ImGui-SFML (Официальная связка ImGui и SFML)
FetchContent_Declare(
        imgui_sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG        v2.6 # Ветка, совместимая с SFML 2.6
)
# Отключаем сборку примеров ImGui-SFML, чтобы компиляция шла быстрее
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
set(IMGUI_SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(imgui_sfml)

# Включаем тесты
enable_testing()
include(GoogleTest)

# Подключаем наши папки с кодом
add_subdirectory(core)
add_subdirectory(modules/environment_map)
add_subdirectory(modules/navigation_beacons)
add_subdirectory(ui)
add_subdirectory(app)
add_subdirectory(tests)