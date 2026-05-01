cmake_minimum_required(VERSION 3.20)
# Называем проект и указываем язык (CXX = C++)
project(Lab2_Sequences CXX)

# Жестко требуем стандарт C++23. Отключаем нестандартные расширения компиляторов (GNU-экстеншены и т.д.)
# для максимальной кроссплатформенности.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Генерируем базу данных команд компиляции (compile_commands.json)
# Это критически важно для корректной работы автодополнения (IntelliSense/clangd) в IDE.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Лечим конфликты рантайма для MSVC (актуально для Windows)
# Гарантирует, что и наш код, и gtest будут использовать одну и ту же версию стандартной библиотеки.
if(MSVC OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
            CACHE STRING "" FORCE
    )
endif()

# # Скачиваем и подключаем GoogleTest прямо из GitHub на этапе конфигурации CMake.
# include(FetchContent)
# FetchContent_Declare(
#         googletest
#         GIT_REPOSITORY https://github.com/google/googletest.git
#         GIT_TAG        v1.14.0
# )
# # Форсируем использование shared CRT для gtest (спасает от кучи ошибок линковки на Windows)
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# Включаем подсистему тестирования CTest в CMake
enable_testing()
# Подключаем макросы CMake для работы с GoogleTest (дает доступ к gtest_discover_tests)
include(GoogleTest)

# Подключаем исходный код (сначала src, чтобы цель seq_lib стала доступна для тестов)
add_subdirectory(src)
# Подключаем папку с тестами
add_subdirectory(tests)