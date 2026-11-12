#include "input/InputManager.hpp"
#include "math/MathUtils.hpp"
#include <iostream>
#include <cmath>

namespace Input {

namespace {

void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        const int halfWidth = static_cast<int>(std::sqrt(radius * radius - dy * dy));
        SDL_RenderDrawLine(renderer, centerX - halfWidth, centerY + dy, centerX + halfWidth, centerY + dy);
    }
}

void drawCircleOutline(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y) {
        SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);

        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

} // namespace

InputManager::~InputManager() {
    shutdown();
}

void InputManager::init() {
    m_moveThumbPos = LEFT_BASE_POS;
    m_aimThumbPos = RIGHT_BASE_POS;
    m_draggingLeftStick = false;
    m_draggingRightStick = false;
    openFirstAvailableController();
}

void InputManager::shutdown() {
    closeGameController();
}

void InputManager::openFirstAvailableController() {
    if (m_gameController) {
        return;
    }

    const int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i) {
        if (SDL_IsGameController(i)) {
            m_gameController = SDL_GameControllerOpen(i);
            if (m_gameController) {
                SDL_Joystick* joystick = SDL_GameControllerGetJoystick(m_gameController);
                m_controllerId = SDL_JoystickInstanceID(joystick);
                std::cout << "[INFO] Mando conectado: " << SDL_GameControllerName(m_gameController) << '\n';
                m_state.activeSource = InputSource::Gamepad;
                break;
            }
        }
    }
}

void InputManager::closeGameController() {
    if (m_gameController) {
        std::cout << "[INFO] Mando desconectado.\n";
        SDL_GameControllerClose(m_gameController);
        m_gameController = nullptr;
        m_controllerId = -1;
    }
}

void InputManager::beginFrame() {
    m_state.triggerNuke = false;
}

void InputManager::updateLeftStick(float x, float y) {
    float dx = x - LEFT_BASE_POS.x;
    float dy = y - LEFT_BASE_POS.y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > JOYSTICK_RADIUS) {
        dx = (dx / dist) * JOYSTICK_RADIUS;
        dy = (dy / dist) * JOYSTICK_RADIUS;
    }
    m_moveThumbPos = {LEFT_BASE_POS.x + dx, LEFT_BASE_POS.y + dy};
    m_state.moveAxisX = Math::clamp(dx / JOYSTICK_RADIUS, -1.0f, 1.0f);
}

void InputManager::updateRightStick(float x, float y) {
    float dx = x - RIGHT_BASE_POS.x;
    float dy = y - RIGHT_BASE_POS.y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > JOYSTICK_RADIUS) {
        dx = (dx / dist) * JOYSTICK_RADIUS;
        dy = (dy / dist) * JOYSTICK_RADIUS;
    }
    m_aimThumbPos = {RIGHT_BASE_POS.x + dx, RIGHT_BASE_POS.y + dy};

    if (dist > JOYSTICK_DEADZONE) {
        float normX = dx / dist;
        float normY = dy / dist;
        // Limitar al hemisferio superior (aimY <= 0.0f)
        if (normY > 0.0f) {
            normY = 0.0f;
            normX = (normX >= 0.0f) ? 1.0f : -1.0f;
        }
        m_state.aimX = normX;
        m_state.aimY = normY;
        m_state.isFiring = true;
    } else {
        m_state.isFiring = false;
    }
}

void InputManager::processEvent(const SDL_Event& event, SDL_Renderer* renderer) {
    (void)renderer;

    switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED: {
            if (!m_gameController && SDL_IsGameController(event.cdevice.which)) {
                m_gameController = SDL_GameControllerOpen(event.cdevice.which);
                if (m_gameController) {
                    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(m_gameController);
                    m_controllerId = SDL_JoystickInstanceID(joystick);
                    std::cout << "[INFO] Mando conectado (Hot-Swap): " << SDL_GameControllerName(m_gameController) << '\n';
                    m_state.activeSource = InputSource::Gamepad;
                }
            }
            break;
        }

        case SDL_CONTROLLERDEVICEREMOVED: {
            if (m_gameController && event.cdevice.which == m_controllerId) {
                closeGameController();
                m_state.activeSource = InputSource::Touch;
            }
            break;
        }

        // ------------------ TOQUES TÁCTILES ANDROID ------------------
        case SDL_FINGERDOWN: {
            m_state.activeSource = InputSource::Touch;
            const float x = event.tfinger.x * 1920.0f;
            const float y = event.tfinger.y * 1080.0f;

            if (isInsideNukeButton(x, y)) {
                m_state.triggerNuke = true;
            } else if (x < 960.0f && !m_moveActive) {
                m_moveActive = true;
                m_moveFingerId = event.tfinger.fingerId;
                updateLeftStick(x, y);
            } else if (x >= 960.0f && !m_aimActive) {
                m_aimActive = true;
                m_aimFingerId = event.tfinger.fingerId;
                updateRightStick(x, y);
            }
            break;
        }

        case SDL_FINGERMOTION: {
            m_state.activeSource = InputSource::Touch;
            const float x = event.tfinger.x * 1920.0f;
            const float y = event.tfinger.y * 1080.0f;

            // Actualizar basándose ÚNICAMENTE en el ID del dedo asignado, sin volver a comprobar x < 960 o x >= 960
            if (m_moveActive && event.tfinger.fingerId == m_moveFingerId) {
                updateLeftStick(x, y);
            }
            if (m_aimActive && event.tfinger.fingerId == m_aimFingerId) {
                updateRightStick(x, y);
            }
            break;
        }

        case SDL_FINGERUP: {
            if (m_moveActive && event.tfinger.fingerId == m_moveFingerId) {
                m_moveActive = false;
                m_moveFingerId = -1;
                m_moveThumbPos = LEFT_BASE_POS;
                if (!m_keyLeft && !m_keyRight) {
                    m_state.moveAxisX = 0.0f;
                }
            }
            if (m_aimActive && event.tfinger.fingerId == m_aimFingerId) {
                m_aimActive = false;
                m_aimFingerId = -1;
                m_aimThumbPos = RIGHT_BASE_POS;
                m_state.isFiring = false;
            }
            break;
        }

        // ------------------ RATÓN EN PC (BLOQUEO DE CAPTURA AL ARRASTRAR) ------------------
        case SDL_MOUSEBUTTONDOWN: {
            if (event.button.which == SDL_TOUCH_MOUSEID) {
                return;
            }

            if (event.button.button == SDL_BUTTON_LEFT) {
                m_state.activeSource = InputSource::Touch;
                const float x = static_cast<float>(event.button.x);
                const float y = static_cast<float>(event.button.y);

                if (isInsideNukeButton(x, y)) {
                    m_state.triggerNuke = true;
                    m_draggingLeftStick = false;
                    m_draggingRightStick = false;
                } else if (x < 960.0f) {
                    m_draggingLeftStick = true;
                    m_draggingRightStick = false;
                    m_moveActive = true;
                    m_moveFingerId = MOUSE_FINGER_ID;
                    updateLeftStick(x, y);
                } else {
                    m_draggingRightStick = true;
                    m_draggingLeftStick = false;
                    m_aimActive = true;
                    m_aimFingerId = MOUSE_FINGER_ID;
                    updateRightStick(x, y);
                }
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                m_state.triggerNuke = true;
                m_state.activeSource = InputSource::Touch;
            }
            break;
        }

        case SDL_MOUSEMOTION: {
            if (event.motion.which == SDL_TOUCH_MOUSEID) {
                return;
            }

            const float mouseX = static_cast<float>(event.motion.x);
            const float mouseY = static_cast<float>(event.motion.y);

            // Si se está arrastrando el stick derecho, actualizar SIEMPRE sin importar
            // si mouseX cruza a la izquierda de 960 o pasa sobre el tanque
            if (m_draggingRightStick) {
                m_state.activeSource = InputSource::Touch;
                updateRightStick(mouseX, mouseY);
            } else if (m_draggingLeftStick) {
                m_state.activeSource = InputSource::Touch;
                updateLeftStick(mouseX, mouseY);
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            if (event.button.which == SDL_TOUCH_MOUSEID) {
                return;
            }

            if (event.button.button == SDL_BUTTON_LEFT) {
                if (m_draggingLeftStick) {
                    m_draggingLeftStick = false;
                    m_moveActive = false;
                    m_moveFingerId = -1;
                    m_moveThumbPos = LEFT_BASE_POS;
                    if (!m_keyLeft && !m_keyRight) {
                        m_state.moveAxisX = 0.0f;
                    }
                }
                if (m_draggingRightStick) {
                    m_draggingRightStick = false;
                    m_aimActive = false;
                    m_aimFingerId = -1;
                    m_aimThumbPos = RIGHT_BASE_POS;
                    m_state.isFiring = false;
                }
            }
            break;
        }

        // ------------------ TECLADO EN PC ------------------
        case SDL_KEYDOWN: {
            if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT) {
                m_keyLeft = true;
                m_state.activeSource = InputSource::Touch;
            } else if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT) {
                m_keyRight = true;
                m_state.activeSource = InputSource::Touch;
            } else if (event.key.keysym.sym == SDLK_SPACE) {
                m_state.triggerNuke = true;
                m_state.activeSource = InputSource::Touch;
            }
            break;
        }

        case SDL_KEYUP: {
            if (event.key.keysym.sym == SDLK_a || event.key.keysym.sym == SDLK_LEFT) {
                m_keyLeft = false;
                if (!m_keyRight && !m_moveActive) {
                    m_state.moveAxisX = 0.0f;
                }
            } else if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_RIGHT) {
                m_keyRight = false;
                if (!m_keyLeft && !m_moveActive) {
                    m_state.moveAxisX = 0.0f;
                }
            }
            break;
        }

        default:
            break;
    }
}

void InputManager::update() {
    // 1. Procesar teclado si alguna tecla está activa
    if (m_keyLeft || m_keyRight) {
        m_state.moveAxisX = (m_keyRight ? 1.0f : 0.0f) - (m_keyLeft ? 1.0f : 0.0f);
        m_state.activeSource = InputSource::Touch;
    }

    // 2. Procesar Gamepad de manera estrictamente aislada
    if (m_gameController) {
        bool gamepadHasInput = false;

        // Movimiento X (Stick Izquierdo)
        const int16_t rawLeftX = SDL_GameControllerGetAxis(m_gameController, SDL_CONTROLLER_AXIS_LEFTX);
        const float normLeftX = static_cast<float>(rawLeftX) / 32767.0f;
        if (std::abs(normLeftX) > GAMEPAD_RADIAL_DEADZONE) {
            m_state.moveAxisX = normLeftX;
            gamepadHasInput = true;
        }

        // Movimiento X (D-Pad)
        if (SDL_GameControllerGetButton(m_gameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
            m_state.moveAxisX = -1.0f;
            gamepadHasInput = true;
        } else if (SDL_GameControllerGetButton(m_gameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            m_state.moveAxisX = 1.0f;
            gamepadHasInput = true;
        }

        // Apuntado (Stick Derecho)
        const int16_t rawRightX = SDL_GameControllerGetAxis(m_gameController, SDL_CONTROLLER_AXIS_RIGHTX);
        const int16_t rawRightY = SDL_GameControllerGetAxis(m_gameController, SDL_CONTROLLER_AXIS_RIGHTY);
        const float normRightX = static_cast<float>(rawRightX) / 32767.0f;
        const float normRightY = static_cast<float>(rawRightY) / 32767.0f;
        const float stickDist = std::sqrt(normRightX * normRightX + normRightY * normRightY);

        if (stickDist > GAMEPAD_RADIAL_DEADZONE) {
            float normX = normRightX / stickDist;
            float normY = normRightY / stickDist;
            if (normY > 0.0f) {
                normY = 0.0f;
                normX = (normX >= 0.0f) ? 1.0f : -1.0f;
            }
            m_state.aimX = normX;
            m_state.aimY = normY;
            m_state.isFiring = true;
            gamepadHasInput = true;
        }

        // Fuego Continuo (Gatillo Derecho RT / R2)
        const int16_t rawTriggerR = SDL_GameControllerGetAxis(m_gameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        if (rawTriggerR > 8000) {
            m_state.isFiring = true;
            gamepadHasInput = true;
        }

        // Bomba Nuclear (Gatillo Izquierdo LT, LB o Botón A / Cruz)
        const int16_t rawTriggerL = SDL_GameControllerGetAxis(m_gameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        if (rawTriggerL > 8000 ||
            SDL_GameControllerGetButton(m_gameController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) ||
            SDL_GameControllerGetButton(m_gameController, SDL_CONTROLLER_BUTTON_A)) {
            m_state.triggerNuke = true;
            gamepadHasInput = true;
        }

        if (gamepadHasInput) {
            m_state.activeSource = InputSource::Gamepad;
        } else if (m_state.activeSource == InputSource::Gamepad) {
            m_state.moveAxisX = 0.0f;
            m_state.isFiring = false;
        }
    }
}

void InputManager::renderTouchUI(SDL_Renderer* renderer) {
    if (m_state.activeSource == InputSource::Gamepad) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const float opMult = (m_touchOpacityLevel == 0) ? 0.45f : ((m_touchOpacityLevel == 1) ? 1.0f : 1.55f);
    auto a = [opMult](Uint8 base) -> Uint8 {
        return static_cast<Uint8>(std::clamp(static_cast<float>(base) * opMult, 0.0f, 255.0f));
    };

    // 1. Botón táctico circular de Bomba Atómica (Nuke) en (1780.0f, 600.0f), radio 55
    const int nukeX = static_cast<int>(NUKE_BUTTON_CENTER.x);
    const int nukeY = static_cast<int>(NUKE_BUTTON_CENTER.y);
    const int nukeR = static_cast<int>(NUKE_BUTTON_RADIUS);

    SDL_SetRenderDrawColor(renderer, 35, 45, 38, a(200));
    drawFilledCircle(renderer, nukeX, nukeY, nukeR);

    SDL_SetRenderDrawColor(renderer, 255, 185, 25, a(240));
    drawCircleOutline(renderer, nukeX, nukeY, nukeR);
    drawCircleOutline(renderer, nukeX, nukeY, nukeR - 1);

    const float angles[3]{ -1.570796f, 0.523599f, 2.617994f }; // -90°, 30°, 150°
    SDL_SetRenderDrawColor(renderer, 255, 185, 25, a(230));
    for (float baseAngle : angles) {
        for (float ang = baseAngle - 0.35f; ang <= baseAngle + 0.35f; ang += 0.04f) {
            const int x1 = nukeX + static_cast<int>(std::cos(ang) * 18.0f);
            const int y1 = nukeY + static_cast<int>(std::sin(ang) * 18.0f);
            const int x2 = nukeX + static_cast<int>(std::cos(ang) * 44.0f);
            const int y2 = nukeY + static_cast<int>(std::sin(ang) * 44.0f);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 185, 25, a(255));
    drawFilledCircle(renderer, nukeX, nukeY, 14);
    SDL_SetRenderDrawColor(renderer, 30, 35, 30, a(255));
    drawFilledCircle(renderer, nukeX, nukeY, 7);

    // 2. Joystick Izquierdo (Base fija en 280, 820 con radio 130)
    const int leftBaseX = static_cast<int>(LEFT_BASE_POS.x);
    const int leftBaseY = static_cast<int>(LEFT_BASE_POS.y);
    const int joyRadius = static_cast<int>(JOYSTICK_RADIUS);

    const Uint8 leftBaseAlpha = a(m_moveActive ? 90 : 35);
    const Uint8 leftBorderAlpha = a(m_moveActive ? 220 : 70);
    SDL_SetRenderDrawColor(renderer, 50, 100, 180, leftBaseAlpha);
    drawFilledCircle(renderer, leftBaseX, leftBaseY, joyRadius);
    SDL_SetRenderDrawColor(renderer, 120, 190, 255, leftBorderAlpha);
    drawCircleOutline(renderer, leftBaseX, leftBaseY, joyRadius);
    drawCircleOutline(renderer, leftBaseX, leftBaseY, joyRadius - 1);

    SDL_SetRenderDrawColor(renderer, 100, 160, 230, a(m_moveActive ? 70 : 25));
    SDL_RenderDrawLine(renderer, leftBaseX - 40, leftBaseY, leftBaseX + 40, leftBaseY);
    SDL_RenderDrawLine(renderer, leftBaseX, leftBaseY - 40, leftBaseX, leftBaseY + 40);

    const int leftThumbX = static_cast<int>(m_moveThumbPos.x);
    const int leftThumbY = static_cast<int>(m_moveThumbPos.y);
    const Uint8 leftThumbAlpha = a(m_moveActive ? 200 : 60);
    const Uint8 leftThumbBorderAlpha = a(m_moveActive ? 255 : 100);
    SDL_SetRenderDrawColor(renderer, 140, 200, 255, leftThumbAlpha);
    drawFilledCircle(renderer, leftThumbX, leftThumbY, 45);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, leftThumbBorderAlpha);
    drawCircleOutline(renderer, leftThumbX, leftThumbY, 45);
    drawCircleOutline(renderer, leftThumbX, leftThumbY, 44);

    // 3. Joystick Derecho (Base fija en 1640, 820 con radio 130)
    const int rightBaseX = static_cast<int>(RIGHT_BASE_POS.x);
    const int rightBaseY = static_cast<int>(RIGHT_BASE_POS.y);

    const Uint8 rightBaseAlpha = a(m_aimActive ? 90 : 35);
    const Uint8 rightBorderAlpha = a(m_aimActive ? 220 : 70);
    SDL_SetRenderDrawColor(renderer, 180, 60, 60, rightBaseAlpha);
    drawFilledCircle(renderer, rightBaseX, rightBaseY, joyRadius);
    SDL_SetRenderDrawColor(renderer, 255, 130, 130, rightBorderAlpha);
    drawCircleOutline(renderer, rightBaseX, rightBaseY, joyRadius);
    drawCircleOutline(renderer, rightBaseX, rightBaseY, joyRadius - 1);

    if (m_aimActive) {
        SDL_SetRenderDrawColor(renderer, 255, 200, 100, a(190));
        SDL_RenderDrawLine(renderer, rightBaseX, rightBaseY, static_cast<int>(m_aimThumbPos.x), static_cast<int>(m_aimThumbPos.y));
    }

    const int rightThumbX = static_cast<int>(m_aimThumbPos.x);
    const int rightThumbY = static_cast<int>(m_aimThumbPos.y);
    const Uint8 rightThumbAlpha = a(m_aimActive ? 200 : 60);
    const Uint8 rightThumbBorderAlpha = a(m_aimActive ? 255 : 100);
    SDL_SetRenderDrawColor(renderer, 255, 140, 140, rightThumbAlpha);
    drawFilledCircle(renderer, rightThumbX, rightThumbY, 45);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, rightThumbBorderAlpha);
    drawCircleOutline(renderer, rightThumbX, rightThumbY, 45);
    drawCircleOutline(renderer, rightThumbX, rightThumbY, 44);
}

} // namespace Input
