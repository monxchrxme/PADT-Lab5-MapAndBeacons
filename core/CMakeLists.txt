# Здесь лежат только .h файлы с интерфейсами (IMap, INavigator). Это будет INTERFACE библиотека.

# Создаем header-only библиотеку
add_library(core INTERFACE)

# Тот, кто линкует core, должен искать заголовки в папке include/
target_include_directories(core INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Если в core.h используется Sequence<T>, нужно прокинуть зависимость.
# ЗАМЕТКА: Укажи здесь реальное имя таргета из твоего репозитория Sequence (seq_lib)
target_link_libraries(core INTERFACE seq_lib)