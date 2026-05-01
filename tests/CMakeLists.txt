# Исходники тестов Студента 1 и Студента 2
add_executable(run_tests
        map_tests/test_generation.cpp
        map_tests/test_raycasting.cpp
        beacons_tests/test_trilateration.cpp
        beacons_tests/test_navigation.cpp
)

# Линкуем оба модуля и GTest
target_link_libraries(run_tests PRIVATE
        environment_map
        navigation_beacons
        GTest::gtest
        GTest::gtest_main
)

gtest_discover_tests(run_tests)