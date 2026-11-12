#pragma once

#include "input/InputState.hpp"
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

namespace Input {

class IInputProvider {
public:
    virtual ~IInputProvider() = default;

    virtual void handleEvent(const SDL_Event& event) = 0;
    virtual void update() = 0;
    [[nodiscard]] virtual const InputState& getState() const noexcept = 0;
};

} // namespace Input

