#include "entities/ProjectilePool.hpp"
#include "core/AssetManager.hpp"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <array>
#include <string>

namespace Entities {

ProjectilePool::ProjectilePool() {
    clear();
}

void ProjectilePool::clear() noexcept {
    for (auto& proj : m_projectiles) {
        proj.active = false;
        proj.x = 0.0f;
        proj.y = 0.0f;
        proj.vx = 0.0f;
        proj.vy = 0.0f;
        proj.damage = 5;
        proj.isCritical = false;
        proj.isHoming = false;
        proj.isPiercing = false;
    }
    m_nextIndex = 0;
    m_muzzleFlashTimer = 0.0f;
}

bool ProjectilePool::spawn(float startX, float startY, float dirX, float dirY, int damage, bool isCritical, bool isHoming, bool isPiercing) {
    const float len = std::hypot(dirX, dirY);
    if (len > 0.0001f) {
        dirX /= len;
        dirY /= len;
    } else {
        dirX = 0.0f;
        dirY = -1.0f;
    }

    for (size_t i = 0; i < POOL_SIZE; ++i) {
        const size_t idx = (m_nextIndex + i) % POOL_SIZE;
        if (!m_projectiles[idx].active) {
            m_projectiles[idx].x = startX;
            m_projectiles[idx].y = startY;
            m_projectiles[idx].vx = dirX * PROJECTILE_SPEED;
            m_projectiles[idx].vy = dirY * PROJECTILE_SPEED;
            m_projectiles[idx].damage = damage;
            m_projectiles[idx].isCritical = isCritical;
            m_projectiles[idx].isHoming = isHoming;
            m_projectiles[idx].isPiercing = isPiercing;
            m_projectiles[idx].active = true;

            m_nextIndex = (idx + 1) % POOL_SIZE;

            m_muzzlePos = {startX, startY};
            m_muzzleDir = {dirX, dirY};
            m_muzzleFlashTimer = MUZZLE_FLASH_DURATION;

            return true;
        }
    }

    return false;
}

void ProjectilePool::updateHoming(float dt, float targetX, float targetY) {
    for (auto& proj : m_projectiles) {
        if (!proj.active || !proj.isHoming) {
            continue;
        }

        float dx = targetX - proj.x;
        float dy = targetY - proj.y;
        const float dist = std::hypot(dx, dy);
        if (dist > 1.0f) {
            dx /= dist;
            dy /= dist;
            // Guiado suave hacia el objetivo
            proj.vx += dx * 1600.0f * dt;
            proj.vy += dy * 1600.0f * dt;

            const float curSpeed = std::hypot(proj.vx, proj.vy);
            if (curSpeed > PROJECTILE_SPEED) {
                proj.vx = (proj.vx / curSpeed) * PROJECTILE_SPEED;
                proj.vy = (proj.vy / curSpeed) * PROJECTILE_SPEED;
            }
        }
    }
}

void ProjectilePool::update(float dt) {
    if (m_muzzleFlashTimer > 0.0f) {
        m_muzzleFlashTimer -= dt;
        if (m_muzzleFlashTimer < 0.0f) {
            m_muzzleFlashTimer = 0.0f;
        }
    }

    for (auto& proj : m_projectiles) {
        if (!proj.active) {
            continue;
        }

        proj.x += proj.vx * dt;
        proj.y += proj.vy * dt;

        // Desactivar únicamente cuando sale completamente de la pantalla lógica
        if (proj.x < BOUND_MIN_X || proj.x > BOUND_MAX_X ||
            proj.y < BOUND_MIN_Y || proj.y > BOUND_MAX_Y) {
            proj.active = false;
        }
    }
}

size_t ProjectilePool::getActiveCount() const noexcept {
    size_t count = 0;
    for (const auto& proj : m_projectiles) {
        if (proj.active) {
            ++count;
        }
    }
    return count;
}

void ProjectilePool::render(SDL_Renderer* renderer) const {
    if (!renderer) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // 1. Renderizado masivo y sólido de proyectiles trazadores (10x36 px)
    const int quadIndices[6]{0, 1, 2, 2, 3, 0};

    for (const auto& proj : m_projectiles) {
        if (!proj.active) {
            continue;
        }

        const float speed = std::hypot(proj.vx, proj.vy);
        const float dirX = (speed > 0.0001f) ? (proj.vx / speed) : 0.0f;
        const float dirY = (speed > 0.0001f) ? (proj.vy / speed) : -1.0f;

        // Vector perpendicular
        const float perpX = -dirY;
        const float perpY = dirX;

        if (proj.isHoming) {
            // Misil guiado de torreta pod: cuerpo blanco con aletas y cola de fuego
            const SDL_FRect missileCore{proj.x - 4.0f, proj.y - 4.0f, 8.0f, 8.0f};
            SDL_SetRenderDrawColor(renderer, 240, 245, 255, 255);
            SDL_RenderFillRectF(renderer, &missileCore);

            const SDL_FRect thrusterFlame{proj.x - dirX * 12.0f - 3.0f, proj.y - dirY * 12.0f - 3.0f, 6.0f, 6.0f};
            SDL_SetRenderDrawColor(renderer, 255, 120, 30, 230);
            SDL_RenderFillRectF(renderer, &thrusterFlame);
            continue;
        }

        // Proyectil trazador normal, crítico o perforante (Overdrive)
        const float halfWidth = proj.isPiercing ? 8.0f : (proj.isCritical ? 6.5f : 4.5f);
        const float length = proj.isPiercing ? 52.0f : (proj.isCritical ? 44.0f : 34.0f);

        const SDL_Color outerColor = proj.isPiercing
            ? SDL_Color{0, 220, 255, 255}    // Cyan eléctrico plasma perforante
            : (proj.isCritical
                ? SDL_Color{255, 40, 20, 255}    // Rojo incandescente para daño 15-20
                : SDL_Color{255, 120, 10, 255}); // Naranja encendido para daño normal

        const SDL_Color coreColor = proj.isPiercing
            ? SDL_Color{255, 255, 255, 255}  // Blanco brillante plasma
            : (proj.isCritical
                ? SDL_Color{255, 245, 120, 255}  // Dorado brillante núcleo
                : SDL_Color{255, 255, 210, 255}); // Amarillo/blanco brillante

        // Capa exterior
        const SDL_Vertex outerVerts[4]{
            {{proj.x - perpX * halfWidth, proj.y - perpY * halfWidth}, outerColor, {0.0f, 0.0f}},
            {{proj.x + perpX * halfWidth, proj.y + perpY * halfWidth}, outerColor, {0.0f, 0.0f}},
            {{proj.x + perpX * halfWidth - dirX * length, proj.y + perpY * halfWidth - dirY * length}, outerColor, {0.0f, 0.0f}},
            {{proj.x - perpX * halfWidth - dirX * length, proj.y - perpY * halfWidth - dirY * length}, outerColor, {0.0f, 0.0f}}
        };
        SDL_RenderGeometry(renderer, nullptr, outerVerts, 4, quadIndices, 6);

        // Capa interior (núcleo incandescente)
        const float coreHalfWidth = halfWidth * 0.45f;
        const float coreLength = length * 0.70f;
        const SDL_Vertex coreVerts[4]{
            {{proj.x - perpX * coreHalfWidth, proj.y - perpY * coreHalfWidth}, coreColor, {0.0f, 0.0f}},
            {{proj.x + perpX * coreHalfWidth, proj.y + perpY * coreHalfWidth}, coreColor, {0.0f, 0.0f}},
            {{proj.x + perpX * coreHalfWidth - dirX * coreLength, proj.y + perpY * coreHalfWidth - dirY * coreLength}, coreColor, {0.0f, 0.0f}},
            {{proj.x - perpX * coreHalfWidth - dirX * coreLength, proj.y - perpY * coreHalfWidth - dirY * coreLength}, coreColor, {0.0f, 0.0f}}
        };
        SDL_RenderGeometry(renderer, nullptr, coreVerts, 4, quadIndices, 6);
    }

    // 2. Destello en boca de cañón (Muzzle Flash)
    if (m_muzzleFlashTimer > 0.0f) {
        const float flashSize = (m_muzzleFlashTimer / MUZZLE_FLASH_DURATION) * 26.0f;
        const SDL_FRect flashRect{
            m_muzzlePos.x - flashSize * 0.5f,
            m_muzzlePos.y - flashSize * 0.5f,
            flashSize,
            flashSize
        };
        SDL_SetRenderDrawColor(renderer, 255, 235, 120, 240);
        SDL_RenderFillRectF(renderer, &flashRect);
    }
}

} // namespace Entities
