file(GLOB SOURCES "src/*.cpp")

add_library(navigation_beacons STATIC ${SOURCES}
        include/entities/Entity.hpp
        src/entities/Entity.cpp
        include/entities/MobileBeacon.hpp
        src/entities/MobileBeacon.cpp
        include/entities/TargetObject.hpp
        src/entities/TargetObject.cpp
        include/math/TrilaterationSolver.hpp
        src/math/TrilaterationSolver.cpp
        include/Navigator.hpp
        src/Navigator.cpp)

target_include_directories(navigation_beacons PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(navigation_beacons PUBLIC core)