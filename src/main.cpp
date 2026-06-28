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