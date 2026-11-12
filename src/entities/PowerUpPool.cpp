#include "entities/PowerUpPool.hpp"
#include "entities/Tank.hpp"
#include "entities/ParticlePool.hpp"
#include "math/Collision.hpp"
#include "core/AssetManager.hpp"
#include "core/AudioManager.hpp"
#include <cmath>
#include <algorithm>

namespace Entities {

PowerUpPool::PowerUpPool() {
    clear();
}

void PowerUpPool::clear() noexcept {
    for (auto& p : m_powerUps) {
        p.active = false;
        p.x = 0.0f;
        p.y = 0.0f;
        p.vy = 85.0f;
        p.swayTimer = 0.0f;
        p.groundLife = 8.0f;
    }
    m_supplyHeli.active = false;
    m_supplyHeli.hp = 40;
    m_supplyHeli.maxHp = 40;
    m_supplyHeli.hitFlashTimer = 0.0f;
    m_supplyHeli.hasDroppedCrate = false;
    m_nextIndex = 0;
}

void PowerUpPool::triggerSupplyHeli(PowerUpType type, int direction) {
    if (m_supplyHeli.active) {
        return;
    }

    m_supplyHeli.active = true;
    const int dir = (direction >= 0) ? 1 : -1;
    m_supplyHeli.x = (dir > 0) ? -90.0f : 2010.0f;
    m_supplyHeli.y = 110.0f;
    m_supplyHeli.vx = static_cast<float>(dir) * 165.0f;
    m_supplyHeli.hp = 40;
    m_supplyHeli.maxHp = 40;
    m_supplyHeli.hitFlashTimer = 0.0f;
    m_supplyHeli.rotorAngle = 0.0f;
    m_supplyHeli.hasDroppedCrate = false;
    m_supplyHeli.pendingType = type;
}

bool PowerUpPool::spawnCrate(float x, float y, PowerUpType type) {
    for (size_t i = 0; i < POOL_SIZE; ++i) {
        const size_t idx = (m_nextIndex + i) % POOL_SIZE;
        if (!m_powerUps[idx].active) {
            auto& p = m_powerUps[idx];
            p.active = true;
            p.x = std::clamp(x, 100.0f, 1820.0f);
            p.y = y;
            p.vy = 85.0f;
            p.swayTimer = 0.0f;
            p.type = type;
            p.groundLife = 8.0f;

            m_nextIndex = (idx + 1) % POOL_SIZE;
            return true;
        }
    }
    return false;
}

bool PowerUpPool::checkFriendlyFire(
    float bx, float by, float br, int dmg,
    ParticlePool& particlePool, uint64_t& score, float& screenShake)
{
    if (!m_supplyHeli.active) {
        return false;
    }

    const auto box = m_supplyHeli.getBoundingBox();
    if (Math::checkCircleAABB(bx, by, br, box.x, box.y, box.w, box.h)) {
        m_supplyHeli.hp -= dmg;
        m_supplyHeli.hitFlashTimer = 0.08f;
        particlePool.spawnExplosion(bx, by, 6, 120.0f, false);

        if (m_supplyHeli.hp <= 0) {
            m_supplyHeli.active = false;
            // Explosión de fuego amigo
            particlePool.spawnExplosion(m_supplyHeli.x, m_supplyHeli.y, 40, 320.0f, true);
            screenShake = std::min(screenShake + 0.40f, 1.0f);

            // Penalización reglamentaria: -1,500 puntos
            if (score >= 1500ULL) {
                score -= 1500ULL;
            } else {
                score = 0;
            }
        }
        return true; // Bala consumida
    }
    return false;
}

void PowerUpPool::update(float dt) {
    // 1. Cinemática del Helicóptero Aliado de Suministros
    if (m_supplyHeli.active) {
        m_supplyHeli.x += m_supplyHeli.vx * dt;
        m_supplyHeli.rotorAngle += 32.0f * dt;

        if (m_supplyHeli.hitFlashTimer > 0.0f) {
            m_supplyHeli.hitFlashTimer -= dt;
            if (m_supplyHeli.hitFlashTimer < 0.0f) {
                m_supplyHeli.hitFlashTimer = 0.0f;
            }
        }

        // Suelta la caja de suministros cerca del centro de la pantalla
        if (!m_supplyHeli.hasDroppedCrate) {
            if ((m_supplyHeli.vx > 0.0f && m_supplyHeli.x >= 920.0f) ||
                (m_supplyHeli.vx < 0.0f && m_supplyHeli.x <= 1000.0f)) {
                spawnCrate(m_supplyHeli.x, m_supplyHeli.y + 24.0f, m_supplyHeli.pendingType);
                m_supplyHeli.hasDroppedCrate = true;
            }
        }

        // Sale de la pantalla
        if ((m_supplyHeli.vx > 0.0f && m_supplyHeli.x > 2120.0f) ||
            (m_supplyHeli.vx < 0.0f && m_supplyHeli.x < -160.0f)) {
            m_supplyHeli.active = false;
        }
    }

    // 2. Cinemática de las Cajas con Paracaídas
    for (auto& p : m_powerUps) {
        if (!p.active) {
            continue;
        }

        constexpr float GROUND_TOUCH_Y = 980.0f - 11.0f;

        if (p.y < GROUND_TOUCH_Y) {
            // Descenso suave en paracaídas con balanceo senoidal
            p.swayTimer += 3.2f * dt;
            p.x += std::sin(p.swayTimer) * 55.0f * dt;
            p.y += p.vy * dt;
            p.x = std::clamp(p.x, 80.0f, 1840.0f);
        } else {
            // Posada sobre el suelo
            p.y = GROUND_TOUCH_Y;
            p.groundLife -= dt;
            if (p.groundLife <= 0.0f) {
                p.active = false;
            }
        }
    }
}

void PowerUpPool::checkCollisionsWithTank(Tank& tank, ParticlePool& particlePool) {
    if (tank.isDestroyed()) {
        return;
    }

    const auto tankBox = tank.getBoundingBox();

    for (auto& p : m_powerUps) {
        if (!p.active) {
            continue;
        }

        const SDL_FRect crateBox{p.x - 12.0f, p.y - 12.0f, 24.0f, 24.0f};

        if (Math::checkAABBAABB(crateBox.x, crateBox.y, crateBox.w, crateBox.h,
                                tankBox.x, tankBox.y, tankBox.w, tankBox.h)) {
            p.active = false;

            // Reproducir sonido de recolección de Power-Up
            Core::AudioManager::getInstance().playPowerUp();

            // Aplicar mejora al tanque
            switch (p.type) {
            case PowerUpType::SpreadShot:
                tank.activateSpreadShot(12.0f);
                break;
            case PowerUpType::RapidFire:
                tank.activateRapidFire(12.0f);
                break;
            case PowerUpType::RepairKit:
                tank.repairShield(35.0f);
                break;
            case PowerUpType::ShieldOvercharge:
                tank.activateShieldOvercharge(6.0f);
                break;
            case PowerUpType::DamageBoost:
                tank.activateDamageBoost(10.0f);
                break;
            case PowerUpType::MegalaserPiece:
                tank.addMegalaserPiece();
                break;
            }

            // Efecto de partículas de recolección
            particlePool.spawnExplosion(p.x, p.y, 25, 220.0f, false);
        }
    }
}

size_t PowerUpPool::getActiveCount() const noexcept {
    size_t count = 0;
    for (const auto& p : m_powerUps) {
        if (p.active) ++count;
    }
    return count;
}

void PowerUpPool::render(SDL_Renderer* renderer) const {
    if (!renderer) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_Texture* spriteTex = Core::AssetManager::getInstance().getSpriteSheet();
    const bool hasSprites = (spriteTex != nullptr);

    // =========================================================================
    // 1. RENDERIZADO DEL HELICÓPTERO ALIADO DE SUMINISTROS (ALLIED SUPPLY HELI)
    // =========================================================================
    if (m_supplyHeli.active) {
        const bool hit = (m_supplyHeli.hitFlashTimer > 0.0f);
        const float dir = (m_supplyHeli.vx >= 0.0f) ? 1.0f : -1.0f;
        const float hx = m_supplyHeli.x;
        const float hy = m_supplyHeli.y;

        // Aura suave esmeralda de auxilio aliado detrás del fuselaje
        const SDL_FRect auraOuter{hx - 56.0f, hy - 40.0f, 112.0f, 80.0f};
        SDL_SetRenderDrawColor(renderer, 46, 204, 113, 50);
        SDL_RenderFillRectF(renderer, &auraOuter);
        const SDL_FRect auraInner{hx - 44.0f, hy - 30.0f, 88.0f, 60.0f};
        SDL_SetRenderDrawColor(renderer, 39, 174, 96, 75);
        SDL_RenderFillRectF(renderer, &auraInner);

        if (hasSprites) {
            // Sprite del helicóptero del atlas (Fila 1, Columna 4 -> getAircraftSrc(4)) escalado a 96x96 px
            const SDL_Rect heliSrc = Core::AssetManager::getInstance().getAircraftSrc(4);
            const double angle = (dir > 0.0f) ? 90.0 : 270.0;
            const SDL_Rect dstRect{
                static_cast<int>(hx - 48.0f),
                static_cast<int>(hy - 48.0f),
                96,
                96
            };

            if (hit) {
                SDL_SetTextureColorMod(spriteTex, 255, 120, 120);
            } else {
                // Tinte aliado blanco verdoso
                SDL_SetTextureColorMod(spriteTex, 215, 255, 235);
            }
            SDL_SetTextureAlphaMod(spriteTex, 255);
            SDL_RenderCopyEx(renderer, spriteTex, &heliSrc, &dstRect, angle, nullptr, (dir < 0.0f) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);
            SDL_SetTextureColorMod(spriteTex, 255, 255, 255);
        } else {
            // Fallback procedural
            const SDL_FRect body{hx - 36.0f, hy - 14.0f, 72.0f, 28.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 240, hit ? 255 : 245, hit ? 255 : 250, 255);
            SDL_RenderFillRectF(renderer, &body);

            const SDL_FRect stripe{hx - 36.0f, hy - 3.0f, 72.0f, 6.0f};
            SDL_SetRenderDrawColor(renderer, 39, 174, 96, 255);
            SDL_RenderFillRectF(renderer, &stripe);

            const float boomX = (dir > 0.0f) ? (hx - 58.0f) : (hx + 36.0f);
            const SDL_FRect boom{boomX, hy - 6.0f, 24.0f, 10.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 210, hit ? 255 : 215, hit ? 255 : 220, 255);
            SDL_RenderFillRectF(renderer, &boom);

            const float tailRotorX = (dir > 0.0f) ? (hx - 60.0f) : (hx + 58.0f);
            const SDL_FRect tailRotor{tailRotorX, hy - 16.0f, 4.0f, 20.0f};
            SDL_SetRenderDrawColor(renderer, 60, 65, 70, 255);
            SDL_RenderFillRectF(renderer, &tailRotor);
        }

        // Aspas animadas superiores dibujadas con SDL_RenderFillRectF (alternando con sin(ticks * 0.05f))
        const float rotorSin = std::sin(static_cast<float>(SDL_GetTicks()) * 0.05f);
        const float bladeSpan = 22.0f + std::abs(rotorSin) * 72.0f;
        const SDL_FRect rotorMast{hx - 2.0f, hy - 36.0f, 4.0f, 10.0f};
        SDL_SetRenderDrawColor(renderer, 45, 50, 55, 255);
        SDL_RenderFillRectF(renderer, &rotorMast);

        const SDL_FRect rotorBlade{hx - bladeSpan * 0.5f, hy - 38.0f, bladeSpan, 4.0f};
        SDL_SetRenderDrawColor(renderer, 25, 30, 35, 235);
        SDL_RenderFillRectF(renderer, &rotorBlade);

        // Puntas amarillas de advertencia en las aspas
        const SDL_FRect tipL{hx - bladeSpan * 0.5f, hy - 38.0f, 6.0f, 4.0f};
        const SDL_FRect tipR{hx + bladeSpan * 0.5f - 6.0f, hy - 38.0f, 6.0f, 4.0f};
        SDL_SetRenderDrawColor(renderer, 240, 210, 40, 255);
        SDL_RenderFillRectF(renderer, &tipL);
        SDL_RenderFillRectF(renderer, &tipR);

        // Cruz médica / suministros pixel art distintiva verde y blanca en el centro de su fuselaje
        const SDL_FRect crossBg{hx - 8.0f, hy - 8.0f, 16.0f, 16.0f};
        SDL_SetRenderDrawColor(renderer, 250, 250, 255, 255);
        SDL_RenderFillRectF(renderer, &crossBg);

        const SDL_FRect crossH{hx - 6.0f, hy - 2.0f, 12.0f, 4.0f};
        const SDL_FRect crossV{hx - 2.0f, hy - 6.0f, 4.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, 39, 174, 96, 255);
        SDL_RenderFillRectF(renderer, &crossH);
        SDL_RenderFillRectF(renderer, &crossV);

        // Caja colgante si aún no la ha soltado
        if (!m_supplyHeli.hasDroppedCrate) {
            if (hasSprites) {
                const SDL_Rect crateSrc = Core::AssetManager::getInstance().getSupplyCrateSrc();
                const SDL_Rect dstCrate{
                    static_cast<int>(hx - 14.0f),
                    static_cast<int>(hy + 16.0f),
                    28,
                    28
                };
                SDL_RenderCopy(renderer, spriteTex, &crateSrc, &dstCrate);
            } else {
                const SDL_FRect crateBox{hx - 10.0f, hy + 16.0f, 20.0f, 20.0f};
                SDL_SetRenderDrawColor(renderer, 210, 150, 50, 255);
                SDL_RenderFillRectF(renderer, &crateBox);
            }
        }
    }

    // =========================================================================
    // 2. RENDERIZADO DE CAJAS MILITARES CON PARACAÍDAS
    // =========================================================================
    for (const auto& p : m_powerUps) {
        if (!p.active) {
            continue;
        }

        constexpr float GROUND_TOUCH_Y = 980.0f - 11.0f;
        const bool inAir = (p.y < GROUND_TOUCH_Y - 2.0f);

        Uint8 r = 180, g = 135, b = 45; // Por defecto
        switch (p.type) {
        case PowerUpType::SpreadShot:
            r = 0; g = 195; b = 255; // Azul cian
            break;
        case PowerUpType::RapidFire:
            r = 255; g = 140; b = 30; // Naranja fuego
            break;
        case PowerUpType::RepairKit:
            r = 0; g = 230; b = 100; // Verde médico
            break;
        case PowerUpType::ShieldOvercharge:
            r = 200; g = 255; b = 255; // Blanco/azul eléctrico
            break;
        case PowerUpType::DamageBoost:
            r = 255; g = 215; b = 0; // Dorado uranio
            break;
        case PowerUpType::MegalaserPiece:
            r = 230; g = 40; b = 255; // Púrpura megaláser
            break;
        }

        if (hasSprites) {
            const SDL_Rect crateSrc = Core::AssetManager::getInstance().getSupplyCrateSrc();

            if (inAir) {
                // Paracaídas cayendo y oscilando
                const double angle = std::sin(p.swayTimer) * 9.0;
                const SDL_Rect dstRect{
                    static_cast<int>(p.x - 26.0f),
                    static_cast<int>(p.y - 32.0f),
                    52,
                    52
                };

                // Resplandor del tipo de mejora debajo de la caja
                const SDL_FRect glowRect{p.x - 14.0f, p.y - 2.0f, 28.0f, 8.0f};
                SDL_SetRenderDrawColor(renderer, r, g, b, 140);
                SDL_RenderFillRectF(renderer, &glowRect);

                SDL_SetTextureColorMod(spriteTex, 255, 255, 255);
                SDL_SetTextureAlphaMod(spriteTex, 255);
                SDL_RenderCopyEx(renderer, spriteTex, &crateSrc, &dstRect, angle, nullptr, SDL_FLIP_NONE);

                // Indicador táctico de color en la caja
                const SDL_FRect badgeRect{p.x - 5.0f, p.y + 4.0f, 10.0f, 8.0f};
                SDL_SetRenderDrawColor(renderer, r, g, b, 240);
                SDL_RenderFillRectF(renderer, &badgeRect);
            } else {
                // En el suelo: caja táctica posada con halo pulsante
                const float pulse = (std::sin(p.groundLife * 8.0f) + 1.0f) * 0.5f;
                const Uint8 haloAlpha = static_cast<Uint8>(90 + pulse * 120.0f);

                const SDL_FRect haloRect{p.x - 18.0f, p.y - 6.0f, 36.0f, 16.0f};
                SDL_SetRenderDrawColor(renderer, r, g, b, haloAlpha);
                SDL_RenderFillRectF(renderer, &haloRect);

                // Caja sólida en el suelo
                const SDL_FRect crateBase{p.x - 14.0f, p.y - 12.0f, 28.0f, 24.0f};
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRectF(renderer, &crateBase);

                // Contorno y detalles
                const SDL_FRect rimT{p.x - 14.0f, p.y - 12.0f, 28.0f, 3.0f};
                const SDL_FRect rimB{p.x - 14.0f, p.y + 9.0f, 28.0f, 3.0f};
                SDL_SetRenderDrawColor(renderer, 20, 25, 30, 240);
                SDL_RenderFillRectF(renderer, &rimT);
                SDL_RenderFillRectF(renderer, &rimB);

                const SDL_FRect coreIcon{p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 250);
                SDL_RenderFillRectF(renderer, &coreIcon);
            }
        } else {
            // Fallback procedural
            if (inAir) {
                const float chuteY = p.y - 30.0f;
                const float chuteW = 42.0f;
                const float chuteH = 18.0f;

                const SDL_FRect canopyBase{p.x - chuteW * 0.5f, chuteY, chuteW, chuteH};
                SDL_SetRenderDrawColor(renderer, 240, 245, 250, 240);
                SDL_RenderFillRectF(renderer, &canopyBase);

                const SDL_FRect stripe1{p.x - 7.0f, chuteY, 14.0f, chuteH};
                SDL_SetRenderDrawColor(renderer, 225, 40, 40, 240);
                SDL_RenderFillRectF(renderer, &stripe1);

                const SDL_FRect cordL{p.x - chuteW * 0.45f, chuteY + chuteH, 2.0f, 12.0f};
                const SDL_FRect cordR{p.x + chuteW * 0.45f - 2.0f, chuteY + chuteH, 2.0f, 12.0f};
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
                SDL_RenderFillRectF(renderer, &cordL);
                SDL_RenderFillRectF(renderer, &cordR);
            }

            const SDL_FRect crateBase{p.x - 12.0f, p.y - 12.0f, 24.0f, 24.0f};
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderFillRectF(renderer, &crateBase);

            const SDL_FRect bT{p.x - 12.0f, p.y - 12.0f, 24.0f, 2.0f};
            const SDL_FRect bB{p.x - 12.0f, p.y + 10.0f, 24.0f, 2.0f};
            SDL_SetRenderDrawColor(renderer, 20, 25, 30, 240);
            SDL_RenderFillRectF(renderer, &bT);
            SDL_RenderFillRectF(renderer, &bB);

            const SDL_FRect coreIcon{p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 250);
            SDL_RenderFillRectF(renderer, &coreIcon);
        }
    }
}

} // namespace Entities
