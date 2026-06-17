# Полиморфные абстрактные типы данных (Лабораторная работа №5 - MapAndBeacons)



## Сборка и запуск

Компилятор C++23 (MSVC, GCC 10+, Clang 12+)

### 1. Конфигурация

**1. Конфигурация и сборка проекта**

```bash
# Клонирование репозитория
git clone https://github.com/monxchrxme/PADT-Lab5-MapAndBeacons.git
cd MapAndBeacons

# Конфигурация (выполняется единожды или при изменении CMakeLists.txt)
cmake -B build -DCMAKE_BUILD_TYPE=Release          
# Компиляция проекта     
cmake --build build --config Release     
```

### 2. Запуск приложения

```bash
./build/app/Release/GeoLocationApp.exe  
```

## Структура проекта

```
.
├── app/
│   ├── CMakeLists.txt
│   └── main.cpp                          # Точка входа в приложение
├── core/                                 # Ядро системы и общие утилиты
│   ├── include/
│   │   ├── exceptions/                   # Кастомные исключения
│   │   │   ├── EnvironmentExceptions.hpp
│   │   │   └── NavigationExceptions.hpp
│   │   ├── physics/                      # Физика радиоволн и обработка сигналов
│   │   │   ├── RadioPhysics.hpp
│   │   │   └── SignalProcessor.hpp
│   │   ├── IEnvironment.hpp              # Интерфейсы карты 
│   │   ├── INavigator.hpp                # Интерфейсы навигатора 
│   │   ├── MathUtils.hpp                 # Вспомогательные математические функции
│   │   └── Structures.hpp                # Общие структуры данных
│   └── CMakeLists.txt
├── external/                             # Внешние зависимости и библиотеки
│   ├── googletest/
│   ├── imgui/
│   ├── imgui_sfml/
│   ├── sequence/
│   └── sfml/
├── modules/                              # Основные функциональные модули
│   ├── environment_map/                  # Модуль карты окружения
│   │   ├── include/
│   │   │   ├── Chunk.hpp                 # Логика чанков карты
│   │   │   └── EnvironmentManager.hpp
│   │   ├── src/
│   │   │   ├── Chunk.cpp
│   │   │   ├── dummy_map.cpp
│   │   │   └── EnvironmentManager.cpp
│   │   └── CMakeLists.txt
│   └── navigation_beacons/               # Модуль навигационных маяков
│       ├── include/
│       │   ├── entities/                 # Сущности (Маяки, Объекты)
│       │   │   ├── Entity.hpp
│       │   │   ├── MobileBeacon.hpp
│       │   │   └── TargetObject.hpp
│       │   ├── math/                     # Математика навигации (AoA Solver)
│       │   │   └── AoASolver.hpp
│       │   └── Navigator.hpp
│       ├── src/
│       │   ├── entities/
│       │   │   ├── Entity.cpp
│       │   │   ├── MobileBeacon.cpp
│       │   │   └── TargetObject.cpp
│       │   ├── math/
│       │   │   └── AoASolver.cpp
│       │   └── Navigator.cpp
│       └── CMakeLists.txt
├── tests/                                # Юнит-тесты проекта
│   ├── beacons_tests/
│   │   ├── test_navigation.cpp
│   │   └── test_trilateration.cpp        
│   ├── map_tests/
│   │   ├── test_generation.cpp           
│   │   └── test_raycasting.cpp           
│   └── CMakeLists.txt
├── ui/                                   # Графический интерфейс пользователя
│   ├── include/
│   │   ├── MapRenderer.hpp               # Отрисовка карты окружения
│   │   └── NavigationRenderer.hpp        # Визуализация маяков и лучей
│   ├── src/
│   │   ├── MapRenderer.cpp
│   │   └── NavigationRenderer.cpp
│   └── CMakeLists.txt
├── .gitignore
├── CMakeLists.txt                        # Корневой файл сборки CMake
├── imgui.ini                             # Конфигурация окон ImGui
├── RadioWaves.md                         # Отчет по радиоволнам и маякам 
├── Map.md                                # Отчет по карте
├── README.md                             # Документация проекта
└── world_save.bin                        # Файл сохранения состояния мира
```