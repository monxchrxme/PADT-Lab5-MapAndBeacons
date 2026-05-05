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
    add_compile_options(/utf-8)
endif()

# include(FetchContent)
# 
# # 1. Скачиваем GoogleTest (через ZIP)
# FetchContent_Declare(
#         googletest
#         URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
# )
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# # 2. Скачиваем твою библиотеку Sequence (через ZIP)
# FetchContent_Declare(
#         sequence_lib
#         URL https://github.com/monxchrxme/PADT-Lab2-Sequences/archive/refs/heads/main.zip
# )
# FetchContent_MakeAvailable(sequence_lib)

# # 3. Скачиваем SFML (через ZIP)
# FetchContent_Declare(
#         sfml
#         URL https://github.com/SFML/SFML/archive/refs/tags/2.6.1.zip
# )
# FetchContent_MakeAvailable(sfml)

# # 4. Скачиваем ImGui-SFML (через ZIP)
# FetchContent_Declare(
#         imgui_sfml
#         URL https://github.com/SFML/imgui-sfml/archive/refs/tags/v2.6.zip
# )
# set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
# set(IMGUI_SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(imgui_sfml)

# Отключаем сборку лишнего мусора из библиотек
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
set(IMGUI_SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui CACHE PATH "" FORCE)

# ВЫКЛЮЧАЕМ ТЕСТЫ ДЛЯ СТОРОННИХ БИБЛИОТЕК 
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# Подключаем локальные библиотеки 
add_subdirectory(external/googletest)
add_subdirectory(external/sequence/src)
set(SFML_BUILD_AUDIO FALSE CACHE BOOL "Disable Audio" FORCE)
set(SFML_BUILD_NETWORK FALSE CACHE BOOL "Disable Network" FORCE)
add_subdirectory(external/sfml)
add_subdirectory(external/imgui_sfml)

# ВКЛЮЧАЕМ ТЕСТЫ ОБРАТНО ДЛЯ НАШЕГО ПРОЕКТА 
set(BUILD_TESTING ON CACHE BOOL "" FORCE)

# Включаем тесты
enable_testing()
include(GoogleTest)

# Подключаем наши папки с кодом
add_subdirectory(core)
add_subdirectory(modules/environment_map)
add_subdirectory(modules/navigation_beacons)
add_subdirectory(ui)
add_subdirectory(app)
# add_subdirectory(tests)