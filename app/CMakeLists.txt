add_executable(GeoLocationApp main.cpp)

# main.cpp достаточно знать только про UI (рендереры) и Core (интерфейсы).
# UI транзитивно подтянет SFML и логические модули при сборке.
target_link_libraries(GeoLocationApp PRIVATE ui)