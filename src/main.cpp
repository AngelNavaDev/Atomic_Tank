#include "core/Engine.hpp"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Core::Engine engine{};
    if (!engine.init("Atomic Tank", 1920, 1080)) {
        return 1;
    }

    engine.run();

    return 0;
}
