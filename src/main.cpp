#include "core/Engine.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    try {
        Engine engine(WIDTH, HEIGHT);
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*

#include <cstdlib>
#include <iostream>

#include "tools/Benchmark.hpp"
#include "scene/World.hpp"
#include "worldgen/TerrainGenerator.hpp"
#include "scene/TerrainParams.hpp"

int main() {
    try {
        TerrainParams params;
        params.worldSize = 2048;
        params.seed = 12345;

        Benchmark benchmark("Terrain Generation", 5);

        benchmark.run([&]() {
            World world(params.worldSize);
            TerrainGenerator generator(params);

            generator.generateBase(world);
        });

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

*/