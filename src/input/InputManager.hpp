#pragma once

#include "input/InputState.hpp"
#include "math/Vector2D.hpp"
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

namespace Input {

class InputManager {
public:
    InputManager() = default;
    ~InputManager();

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void init();
    void shutdown();

    void beginFrame();
    void processEvent(const SDL_Event& event, SDL_Renderer* renderer = nullptr);
    void update();

    [[nodiscard]] const InputState& getState() const noexcept { return m_state; }
    void renderTouchUI(SDL_Renderer* renderer);

    void setTouchOpacityLevel(int level) noexcept {
        m_touchOpacityLevel = (level < 0) ? 0 : ((level > 2) ? 2 : level);
    }
    [[nodiscard]] int getTouchOpacityLevel() const noexcept { return m_touchOpacityLevel; }

    // Parámetros de joysticks de base fija y deadzones
    static constexpr float JOYSTICK_RADIUS{130.0f};
    static constexpr float JOYSTICK_DEADZONE{18.0f};
    static constexpr float GAMEPAD_RADIAL_DEADZONE{0.18f};
    static constexpr SDL_FingerID MOUSE_FINGER_ID{-999999};

    // Botón Nuke circular táctico (arriba a la derecha del joystick de disparo)
    static constexpr Math::Vector2D NUKE_BUTTON_CENTER{1780.0f, 600.0f};
    static constexpr float NUKE_BUTTON_RADIUS{55.0f};

    // Posiciones 100% fijas e inmutables de las bases
    static constexpr Math::Vector2D LEFT_BASE_POS{280.0f, 820.0f};
    static constexpr Math::Vector2D RIGHT_BASE_POS{1640.0f, 820.0f};

    [[nodiscard]] static bool isInsideNukeButton(float x, float y) noexcept {
        const float dx = x - NUKE_BUTTON_CENTER.x;
        const float dy = y - NUKE_BUTTON_CENTER.y;
        return (dx * dx + dy * dy) <= (NUKE_BUTTON_RADIUS * NUKE_BUTTON_RADIUS);
    }

private:
    void openFirstAvailableController();
    void closeGameController();

    void updateLeftStick(float x, float y);
    void updateRightStick(float x, float y);

    InputState m_state{};

    // Mando (Gamepad)
    SDL_GameController* m_gameController{nullptr};
    SDL_JoystickID m_controllerId{-1};

    // Joystick izquierdo fijo (Movimiento X)
    bool m_moveActive{false};
    SDL_FingerID m_moveFingerId{-1};
    Math::Vector2D m_moveThumbPos{LEFT_BASE_POS};

    // Joystick derecho fijo (Apuntado y Disparo)
    bool m_aimActive{false};
    SDL_FingerID m_aimFingerId{-1};
    Math::Vector2D m_aimThumbPos{RIGHT_BASE_POS};

    // Banderas de bloqueo y captura de arrastre de ratón en PC
    bool m_draggingLeftStick{false};
    bool m_draggingRightStick{false};

    // Espejo de teclado
    bool m_keyLeft{false};
    bool m_keyRight{false};

    // Nivel de opacidad táctil (0 = Baja, 1 = Media, 2 = Alta)
    int m_touchOpacityLevel{1};
};

} // namespace Input
