#include "entities/ParticlePool.hpp"
#include "core/AssetManager.hpp"
#include <cmath>
#include <algorithm>

namespace Entities {

ParticlePool::ParticlePool() {
    clear();
}

float ParticlePool::nextRandomFloat() noexcept {
    // Generador congruencial lineal de 32 bits (Zero-Allocation, determinista y seguro)
    m_rngState = m_rngState * 1664525u + 1013904223u;
    return static_cast<float>(m_rngState & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
}

void ParticlePool::clear() noexcept {
    for (auto& p : m_particles) {
        p.active = false;
        p.x = 0.0f;
        p.y = 0.0f;
        p.vx = 0.0f;
        p.vy = 0.0f;
        p.size = 8.0f;
        p.life = 0.0f;
        p.maxLife = 0.5f;
    }
    m_nextIndex = 0;

    for (auto& se : m_spriteExplosions) {
        se.active = false;
        se.x = 0.0f;
        se.y = 0.0f;
        se.timer = 0.0f;
        se.scale = 1.0f;
        se.currentFrame = 0;
        se.row = 0;
    }
    m_nextSpriteExplosionIndex = 0;

    for (auto& sc : m_shellCasings) {
        sc.active = false;
        sc.x = 0.0f;
        sc.y = 0.0f;
        sc.vx = 0.0f;
        sc.vy = 0.0f;
        sc.life = 0.0f;
        sc.maxLife = 1.8f;
        sc.bounces = 0;
    }
    m_nextShellCasingIndex = 0;

    for (auto& wp : m_weatherParticles) {
        wp.active = false;
        wp.x = 0.0f;
        wp.y = 0.0f;
        wp.vx = 0.0f;
        wp.vy = 0.0f;
        wp.size = 2.0f;
        wp.type = 0;
    }
    m_currentWeatherLevel = -1;
}

void ParticlePool::spawnShellCasing(float x, float y, float dirX, [[maybe_unused]] float dirY) {
    size_t chosenIdx = MAX_SHELL_CASINGS;
    for (size_t k = 0; k < MAX_SHELL_CASINGS; ++k) {
        const size_t idx = (m_nextShellCasingIndex + k) % MAX_SHELL_CASINGS;
        if (!m_shellCasings[idx].active) {
            chosenIdx = idx;
            m_nextShellCasingIndex = (idx + 1) % MAX_SHELL_CASINGS;
            break;
        }
    }

    if (chosenIdx == MAX_SHELL_CASINGS) {
        chosenIdx = m_nextShellCasingIndex;
        m_nextShellCasingIndex = (m_nextShellCasingIndex + 1) % MAX_SHELL_CASINGS;
    }

    auto& sc = m_shellCasings[chosenIdx];
    sc.active = true;
    sc.x = x;
    sc.y = y;
    sc.life = 0.0f;
    sc.maxLife = 1.6f + nextRandomFloat() * 0.4f;
    sc.bounces = 0;

    const float rand1 = nextRandomFloat();
    const float rand2 = nextRandomFloat();

    // Eyecta hacia atrás/arriba de la torreta con gravedad
    const float backwardDirX = (std::abs(dirX) > 0.05f) ? (-dirX) : ((rand2 > 0.5f) ? 1.0f : -1.0f);
    sc.vx = backwardDirX * (140.0f + 70.0f * rand1) + (rand2 - 0.5f) * 40.0f;
    sc.vy = -190.0f - 80.0f * rand1;
}

void ParticlePool::spawnExplosion(float x, float y, int count, float speed, bool isLarge) {
    // =========================================================================
    // 1. ANIMACIÓN DE EXPLOSIÓN EN SPRITE (Texture Atlas 8x2)
    // =========================================================================
    size_t chosenSpriteIdx = MAX_SPRITE_EXPLOSIONS;
    for (size_t k = 0; k < MAX_SPRITE_EXPLOSIONS; ++k) {
        const size_t idx = (m_nextSpriteExplosionIndex + k) % MAX_SPRITE_EXPLOSIONS;
        if (!m_spriteExplosions[idx].active) {
            chosenSpriteIdx = idx;
            m_nextSpriteExplosionIndex = (idx + 1) % MAX_SPRITE_EXPLOSIONS;
            break;
        }
    }

    if (chosenSpriteIdx == MAX_SPRITE_EXPLOSIONS) {
        chosenSpriteIdx = m_nextSpriteExplosionIndex;
        m_nextSpriteExplosionIndex = (m_nextSpriteExplosionIndex + 1) % MAX_SPRITE_EXPLOSIONS;
    }

    auto& se = m_spriteExplosions[chosenSpriteIdx];
    se.active = true;
    se.x = x;
    se.y = y;
    se.timer = 0.0f;
    se.currentFrame = 0;

    if (isLarge) {
        // Fila 0: Fuego colosal para jefes, tanques y aeronaves pesadas
        se.row = 0;
        se.scale = 1.8f;
    } else if (count >= 25) {
        // Fila 0: Fuego completo para destrucción regular de aeronaves
        se.row = 0;
        se.scale = 1.15f;
    } else {
        // Fila 1: Onda expansiva para proyectiles, bombas o impactos tácticos
        se.row = 1;
        se.scale = 0.95f;
    }

    // =========================================================================
    // 2. PARTÍCULAS GEOMÉTRICAS DE CHISPAS Y HUMO RADIAL
    // =========================================================================
    constexpr float TWO_PI = 6.2831853f;

    for (int i = 0; i < count; ++i) {
        // Encontrar siguiente ranura libre
        size_t chosenIdx = POOL_SIZE;
        for (size_t k = 0; k < POOL_SIZE; ++k) {
            const size_t idx = (m_nextIndex + k) % POOL_SIZE;
            if (!m_particles[idx].active) {
                chosenIdx = idx;
                m_nextIndex = (idx + 1) % POOL_SIZE;
                break;
            }
        }

        if (chosenIdx == POOL_SIZE) {
            // Si el pool está lleno, forzar reutilización de la ranura actual
            chosenIdx = m_nextIndex;
            m_nextIndex = (m_nextIndex + 1) % POOL_SIZE;
        }

        auto& p = m_particles[chosenIdx];
        p.active = true;
        p.x = x;
        p.y = y;

        // Distribución radial con dispersión angular y velocidad aleatoria
        const float baseAngle = (TWO_PI * static_cast<float>(i)) / static_cast<float>(count);
        const float angle = baseAngle + (nextRandomFloat() - 0.5f) * 0.5f;
        const float spd = speed * (0.35f + 0.85f * nextRandomFloat());

        p.vx = std::cos(angle) * spd;
        p.vy = std::sin(angle) * spd;

        if (isLarge) {
            p.size = 8.0f + 6.0f * nextRandomFloat(); // 8x8 a 14x14 px
            p.maxLife = 0.45f + 0.35f * nextRandomFloat();
        } else {
            p.size = 6.0f + 4.0f * nextRandomFloat(); // 6x6 a 10x10 px
            p.maxLife = 0.30f + 0.25f * nextRandomFloat();
        }
        p.life = p.maxLife;
    }
}

void ParticlePool::update(float dt) {
    constexpr float GRAVITY = 240.0f;     // Gravedad ligera para proyectiles de fuego y escombros
    constexpr float DRAG = 1.6f;          // Fricción con el aire
    constexpr float FRAME_DURATION = 0.0555f; // ~18 FPS (8 frames total = ~0.44s)

    // Actualizar animaciones de sprites de explosión
    for (auto& se : m_spriteExplosions) {
        if (!se.active) {
            continue;
        }

        se.timer += dt;
        se.currentFrame = static_cast<int>(se.timer / FRAME_DURATION);
        if (se.currentFrame >= 8) {
            se.active = false;
        }
    }

    // Actualizar partículas geométricas
    for (auto& p : m_particles) {
        if (!p.active) {
            continue;
        }

        p.life -= dt;
        if (p.life <= 0.0f) {
            p.active = false;
            continue;
        }

        p.vy += GRAVITY * dt;
        p.vx -= p.vx * DRAG * dt;
        p.vy -= p.vy * DRAG * dt;

        p.x += p.vx * dt;
        p.y += p.vy * dt;
    }

    // Actualizar casquillos de bala físicos eyectados
    constexpr float CASING_GRAVITY = 650.0f;
    for (auto& sc : m_shellCasings) {
        if (!sc.active) {
            continue;
        }

        sc.life += dt;
        if (sc.life >= sc.maxLife) {
            sc.active = false;
            continue;
        }

        sc.vy += CASING_GRAVITY * dt;
        sc.x += sc.vx * dt;
        sc.y += sc.vy * dt;

        // Rebote en el suelo a Y = 980
        if (sc.y >= 980.0f) {
            sc.y = 980.0f;
            if (sc.bounces == 0) {
                sc.vy = -sc.vy * 0.45f;
                sc.vx *= 0.60f;
                sc.bounces = 1;
            } else {
                sc.vx = 0.0f;
                sc.vy = 0.0f;
            }
        }
    }
}

void ParticlePool::updateWeather(float dt, int level) {
    if (m_currentWeatherLevel != level) {
        m_currentWeatherLevel = level;
        int weatherType = 3; // Polvo / viento ambiental
        if (level == 4 || level == 14 || level == 18) {
            weatherType = 0; // Nieve (Ártico / Criogénico)
        } else if (level == 6 || level == 8 || level == 16) {
            weatherType = 1; // Lluvia (Tormenta Marina / Jungla)
        } else if (level == 5 || level == 11 || level == 22 || level == 25) {
            weatherType = 2; // Ascuas (Volcán / Magma / Doomsday)
        }

        for (auto& wp : m_weatherParticles) {
            wp.active = true;
            wp.type = weatherType;
            wp.x = nextRandomFloat() * 1920.0f;
            wp.y = nextRandomFloat() * 1080.0f;

            if (weatherType == 0) {
                // Nieve
                wp.vx = 35.0f + 35.0f * nextRandomFloat();
                wp.vy = 75.0f + 55.0f * nextRandomFloat();
                wp.size = 3.0f + 2.5f * nextRandomFloat();
            } else if (weatherType == 1) {
                // Lluvia
                wp.vx = -100.0f - 40.0f * nextRandomFloat();
                wp.vy = 680.0f + 180.0f * nextRandomFloat();
                wp.size = 2.0f;
            } else if (weatherType == 2) {
                // Ascuas flotando hacia arriba desde el suelo
                wp.vx = -30.0f + 60.0f * nextRandomFloat();
                wp.vy = -110.0f - 90.0f * nextRandomFloat();
                wp.size = 3.0f + 2.5f * nextRandomFloat();
            } else {
                // Polvo / Viento ambiental
                wp.vx = 40.0f + 40.0f * nextRandomFloat();
                wp.vy = 12.0f + 16.0f * nextRandomFloat();
                wp.size = 2.0f;
            }
        }
    }

    for (auto& wp : m_weatherParticles) {
        if (!wp.active) continue;

        wp.x += wp.vx * dt;
        wp.y += wp.vy * dt;

        if (wp.type == 2) {
            // Ascuas suben
            if (wp.y < -15.0f) {
                wp.y = 1000.0f + nextRandomFloat() * 70.0f;
                wp.x = nextRandomFloat() * 1920.0f;
            }
        } else {
            // Nieve, lluvia, polvo caen
            if (wp.y > 1085.0f) {
                wp.y = -10.0f;
                wp.x = nextRandomFloat() * 1920.0f;
            }
        }

        if (wp.x < -20.0f) {
            wp.x = 1930.0f;
        } else if (wp.x > 1935.0f) {
            wp.x = -10.0f;
        }
    }
}

size_t ParticlePool::getActiveCount() const noexcept {
    size_t count = 0;
    for (const auto& p : m_particles) {
        if (p.active) {
            ++count;
        }
    }
    return count;
}

size_t ParticlePool::getActiveSpriteExplosionsCount() const noexcept {
    size_t count = 0;
    for (const auto& se : m_spriteExplosions) {
        if (se.active) {
            ++count;
        }
    }
    return count;
}

size_t ParticlePool::getActiveShellCasingsCount() const noexcept {
    size_t count = 0;
    for (const auto& sc : m_shellCasings) {
        if (sc.active) {
            ++count;
        }
    }
    return count;
}

void ParticlePool::renderShellCasings(SDL_Renderer* renderer) const {
    if (!renderer) return;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const auto& sc : m_shellCasings) {
        if (!sc.active) continue;

        Uint8 alpha = 255;
        if (sc.life > sc.maxLife - 0.5f) {
            const float fadeRatio = (sc.maxLife - sc.life) / 0.5f;
            alpha = static_cast<Uint8>(std::clamp(fadeRatio * 255.0f, 0.0f, 255.0f));
        }

        const SDL_FRect r{sc.x - 2.5f, sc.y - 1.5f, 5.0f, 3.0f};
        SDL_SetRenderDrawColor(renderer, 235, 195, 45, alpha);
        SDL_RenderFillRectF(renderer, &r);
    }
}

void ParticlePool::renderWeather(SDL_Renderer* renderer) const {
    if (!renderer) return;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const auto& wp : m_weatherParticles) {
        if (!wp.active) continue;

        if (wp.type == 0) {
            // Nieve
            const SDL_FRect flake{wp.x, wp.y, wp.size, wp.size};
            SDL_SetRenderDrawColor(renderer, 245, 250, 255, 215);
            SDL_RenderFillRectF(renderer, &flake);
        } else if (wp.type == 1) {
            // Lluvia rápida diagonal (2x10 px)
            const SDL_FRect drop{wp.x, wp.y, 2.0f, 10.0f};
            SDL_SetRenderDrawColor(renderer, 150, 210, 245, 190);
            SDL_RenderFillRectF(renderer, &drop);
        } else if (wp.type == 2) {
            // Ascuas de fuego
            const SDL_FRect ember{wp.x, wp.y, wp.size, wp.size};
            SDL_SetRenderDrawColor(renderer, 255, 130, 25, 220);
            SDL_RenderFillRectF(renderer, &ember);
        } else {
            // Polvo / Viento suave
            const SDL_FRect dust{wp.x, wp.y, wp.size, wp.size};
            SDL_SetRenderDrawColor(renderer, 210, 215, 190, 110);
            SDL_RenderFillRectF(renderer, &dust);
        }
    }
}

void ParticlePool::render(SDL_Renderer* renderer) const {
    if (!renderer) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // =========================================================================
    // 1. RENDERIZADO DE ANIMACIONES DE EXPLOSIONES (SPRITES 8 FRAMES)
    // =========================================================================
    SDL_Texture* explTex = Core::AssetManager::getInstance().getExplosionSheet();
    if (explTex) {
        for (const auto& se : m_spriteExplosions) {
            if (!se.active || se.currentFrame < 0 || se.currentFrame >= 8) {
                continue;
            }

            const SDL_Rect srcRect = Core::AssetManager::getInstance().getExplosionSrc(se.currentFrame, se.row);
            const float w = static_cast<float>(srcRect.w) * se.scale;
            const float h = static_cast<float>(srcRect.h) * se.scale;

            const SDL_FRect dstRect{
                se.x - w * 0.5f,
                se.y - h * 0.5f,
                w,
                h
            };

            SDL_RenderCopyF(renderer, explTex, &srcRect, &dstRect);
        }
    }

    // =========================================================================
    // 2. RENDERIZADO DE CHISPAS Y HUMO RADIAL GEOMÉTRICO
    // =========================================================================
    for (const auto& p : m_particles) {
        if (!p.active) {
            continue;
        }

        const float ratio = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);

        Uint8 r = 255;
        Uint8 g = 255;
        Uint8 b = 255;
        Uint8 a = 255;

        // Transición de color del fuego: Blanco brillante -> Amarillo -> Naranja -> Humo gris
        if (ratio > 0.70f) {
            // Núcleo inicial blanco incandescente
            r = 255;
            g = 255;
            b = static_cast<Uint8>(180 + 75 * ((ratio - 0.70f) / 0.30f));
            a = 255;
        } else if (ratio > 0.40f) {
            // Llamarada amarillo-dorada
            const float t = (ratio - 0.40f) / 0.30f;
            r = 255;
            g = static_cast<Uint8>(180 + 75 * t);
            b = static_cast<Uint8>(30 * t);
            a = 240;
        } else if (ratio > 0.18f) {
            // Naranja fuego
            const float t = (ratio - 0.18f) / 0.22f;
            r = static_cast<Uint8>(200 + 55 * t);
            g = static_cast<Uint8>(80 + 100 * t);
            b = 20;
            a = static_cast<Uint8>(180 + 40 * t);
        } else {
            // Humo gris desvaneciéndose en el aire
            const float t = ratio / 0.18f;
            r = 80;
            g = 80;
            b = 85;
            a = static_cast<Uint8>(150 * t);
        }

        // Ligera expansión del volumen de humo al enfriarse
        const float renderSize = p.size * (0.85f + 0.35f * (1.0f - ratio));
        const SDL_FRect pRect{
            p.x - renderSize * 0.5f,
            p.y - renderSize * 0.5f,
            renderSize,
            renderSize
        };

        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderFillRectF(renderer, &pRect);
    }

    // 3. Renderizado de Casquillos Físicos Eyectados
    renderShellCasings(renderer);

    // 4. Efectos Climáticos Ambientales
    renderWeather(renderer);
}

} // namespace Entities

