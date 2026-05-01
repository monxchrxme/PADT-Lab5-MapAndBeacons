file(GLOB SOURCES "src/*.cpp")

add_library(navigation_beacons STATIC ${SOURCES})

target_include_directories(navigation_beacons PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(navigation_beacons PUBLIC core)