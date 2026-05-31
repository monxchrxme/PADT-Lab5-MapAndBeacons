#include <gtest/gtest.h>
#include "../../core/include/Structures.hpp"
#include "../../modules/environment_map/include/EnvironmentManager.hpp"

// Проверяем, что менеджер корректно создается и не течет по памяти
TEST(EnvironmentTest, InitializationAndMemory) {
    auto* env = new EnvironmentManager();
    // Если деструктор написан криво (утечка памяти или двойное удаление), 
    // тест упадет при вызове delete.
    delete env; 
    SUCCEED();
}

// Проверяем логику генерации стартового чанка
TEST(EnvironmentTest, BasicChunkGeneration) {
    EnvironmentManager env;
    
    // Точка 0,0 должна быть сгенерирована в конструкторе и быть проходимой (Трава)
    EXPECT_TRUE(env.isPassable(Point2D{0.0, 0.0}));
    
    // Проверяем, что башня сгенерировалась
    auto towers = env.getStaticTowers();
    EXPECT_GT(towers.get_length(), 0); // Длина массива должна быть > 0
}

// Проверяем ленивую генерацию
TEST(EnvironmentTest, LazyGenerationOutBounds) {
    EnvironmentManager env;
    
    // Точка 1000, 1000 изначально не существует.
    // Запрашиваем генерацию:
    env.triggerLazyGeneration(Point2D{1000.0, 1000.0});
    
    // Теперь она должна быть проходимой
    EXPECT_TRUE(env.isPassable(Point2D{1000.0, 1000.0}));
}