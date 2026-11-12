#pragma once

namespace Input {

enum class InputSource {
    Touch,
    Gamepad
};

struct InputState {
    float moveAxisX{0.0f};
    float aimX{0.0f};
    float aimY{-1.0f};       // Apuntando arriba por defecto
    bool isFiring{false};
    bool triggerNuke{false};
    InputSource activeSource{InputSource::Touch};
};

} // namespace Input
