#include "core/Engine.hpp"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Core::Engine engine{};
    if (!engine.init("Atomic Weapon", 1280, 720)) {
        return 1;
    }

    engine.run();

    return 0;
}
