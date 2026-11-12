#include "entities/Tank.hpp"
#include "entities/ProjectilePool.hpp"
#include "entities/ParticlePool.hpp"
#include "core/AssetManager.hpp"
#include "core/AudioManager.hpp"
#include "math/MathUtils.hpp"
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

namespace {

void drawSolidCircle(SDL_Renderer* renderer, float centerX, float centerY, float radius, SDL_Color color) {
    const int r = static_cast<int>(radius);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int dy = -r; dy <= r; ++dy) {
        const float halfWidth = std::sqrt(std::max(0.0f, radius * radius - static_cast<float>(dy * dy)));
        const SDL_FRect lineRect{centerX - halfWidth, centerY + static_cast<float>(dy), halfWidth * 2.0f, 1.5f};
        SDL_RenderFillRectF(renderer, &lineRect);
    }
}

} // namespace

Math::Vector2D Tank::getWingmanDronePosition() const noexcept {
    return {
        m_position.x - 45.0f + std::sin(m_animTimer * 2.5f) * 12.0f,
        (GROUND_Y - 95.0f) + std::sin(m_animTimer * 4.5f) * 6.0f
    };
}

Tank::Tank() {
    init(960.0f);
}

void Tank::init(float spawnX) {
    m_position = {spawnX, GROUND_Y};
    m_velocityX = 0.0f;
    m_aimDirX = 0.0f;
    m_aimDirY = -1.0f; // Apuntando verticalmente al cielo
    m_fireCooldown = 0.0f;
    m_wheelAngle = 0.0f;
    m_shield = m_maxShield;
    m_isDestroyed = false;
    m_invulnerableTimer = 0.0f;
    m_spreadShotTimer = 0.0f;
    m_rapidFireTimer = 0.0f;
    m_shieldOverchargeTimer = 0.0f;
    m_damageBoostTimer = 0.0f;
    m_megalaserFragments = 0;
    m_megalaserActiveTimer = 0.0f;
    m_homingCooldown = 0.0f;
    m_orbAngle = 0.0f;
    m_recoilOffset = 0.0f;
    m_animTimer = 0.0f;
    m_overdriveMeter = 0.0f;
    m_overdriveDuration = 0.0f;
}

void Tank::addMegalaserPiece() noexcept {
    m_megalaserFragments++;
    if (m_megalaserFragments >= 4) {
        m_megalaserFragments = 0;
        m_megalaserActiveTimer = 3.5f; // Rayo devastador durante 3.5 segundos
    }
}

void Tank::setChassisTier(int tier) noexcept {
    m_chassisUpgradeTier = tier;
    float newMax = 100.0f;
    if (tier == 1) newMax = 125.0f;
    else if (tier == 2) newMax = 125.0f;
    else if (tier >= 3) newMax = 175.0f;

    if (newMax > m_maxShield) {
        m_shield += (newMax - m_maxShield);
        m_maxShield = newMax;
    }
}

void Tank::resetUpgrades() noexcept {
    m_ammoUpgradeTier = 0;
    m_cannonUpgradeTier = 0;
    m_chassisUpgradeTier = 0;
    m_secondaryUpgradeTier = 0;
    m_pointDefenseCooldown = 0.0f;
    m_maxShield = 100.0f;
    m_shield = 100.0f;
    m_homingPodLevel = 0;
    m_hasDefenseOrbs = false;
    m_permanentDamageBonus = 0;
    m_overdriveMeter = 0.0f;
    m_overdriveDuration = 0.0f;
}

float Tank::getEffectiveFireRate() const noexcept {
    float rate = FIRE_RATE; // 0.065f
    if (m_cannonUpgradeTier >= 3) {
        rate = FIRE_RATE / (1.18f * 1.25f); // ~0.044f
    } else if (m_cannonUpgradeTier >= 1) {
        rate = FIRE_RATE / 1.18f; // ~0.055f
    }
    if (m_rapidFireTimer > 0.0f) {
        rate = 0.025f;
    }
    // Sobrecarga (Overdrive): cadencia de disparo aumenta +35% (intervalo * 1.0f / 1.35f)
    if (isOverdriveActive()) {
        rate *= (1.0f / 1.35f);
    }
    return rate;
}

int Tank::calculateBulletDamage(bool& outIsCritical) noexcept {
    m_rngState = m_rngState * 1664525u + 1013904223u;

    int ammoBonus = 0;
    if (m_ammoUpgradeTier == 1) ammoBonus = 3;
    else if (m_ammoUpgradeTier == 2) ammoBonus = 6;
    else if (m_ammoUpgradeTier >= 3) ammoBonus = 10;

    const int base = 10 + static_cast<int>(m_rngState % 5u) + ammoBonus + m_permanentDamageBonus;

    if (hasDamageBoost()) {
        // x1.8 cuando recoge el Power-Up temporal de daño
        outIsCritical = true;
        return static_cast<int>(static_cast<float>(base) * 1.8f);
    }

    if (hasSpreadShot() || hasRapidFire()) {
        outIsCritical = true;
        return base + 2;
    }

    outIsCritical = false;
    return base;
}

Math::Vector2D Tank::getTurretPivot() const noexcept {
    return {m_position.x, GROUND_Y - 46.0f};
}

Math::Vector2D Tank::getBarrelTip() const noexcept {
    const auto pivot = getTurretPivot();
    return {pivot.x + m_aimDirX * BARREL_LENGTH, pivot.y + m_aimDirY * BARREL_LENGTH};
}

void Tank::update(float dt, const Input::InputState& input) {
    if (m_isDestroyed) {
        return;
    }

    if (m_invulnerableTimer > 0.0f) {
        m_invulnerableTimer = std::max(0.0f, m_invulnerableTimer - dt);
    }
    if (m_spreadShotTimer > 0.0f) {
        m_spreadShotTimer = std::max(0.0f, m_spreadShotTimer - dt);
    }
    if (m_rapidFireTimer > 0.0f) {
        m_rapidFireTimer = std::max(0.0f, m_rapidFireTimer - dt);
    }
    if (m_shieldOverchargeTimer > 0.0f) {
        m_shieldOverchargeTimer = std::max(0.0f, m_shieldOverchargeTimer - dt);
    }
    if (m_damageBoostTimer > 0.0f) {
        m_damageBoostTimer = std::max(0.0f, m_damageBoostTimer - dt);
    }
    if (m_megalaserActiveTimer > 0.0f) {
        m_megalaserActiveTimer = std::max(0.0f, m_megalaserActiveTimer - dt);
    }

    // Rotación de orbes tácticos de defensa
    if (m_hasDefenseOrbs) {
        m_orbAngle += 3.6f * dt;
    }

    // 1. Cinemática horizontal en el suelo con inercia suave
    const float currentMaxSpeed = (m_chassisUpgradeTier >= 2) ? (MAX_SPEED * 1.25f) : MAX_SPEED;
    const float targetVelocity = input.moveAxisX * currentMaxSpeed;
    const float accelRate = (std::abs(input.moveAxisX) > 0.01f) ? ACCELERATION : DECELERATION;

    if (m_velocityX < targetVelocity) {
        m_velocityX = std::min(m_velocityX + accelRate * dt, targetVelocity);
    } else if (m_velocityX > targetVelocity) {
        m_velocityX = std::max(m_velocityX - accelRate * dt, targetVelocity);
    }

    if (std::abs(m_velocityX) < 1.0f && std::abs(input.moveAxisX) < 0.01f) {
        m_velocityX = 0.0f;
    }

    m_position.x += m_velocityX * dt;

    // Límites de pantalla (60.0f a 1860.0f)
    if (m_position.x < MIN_BOUND_X) {
        m_position.x = MIN_BOUND_X;
        m_velocityX = 0.0f;
    } else if (m_position.x > MAX_BOUND_X) {
        m_position.x = MAX_BOUND_X;
        m_velocityX = 0.0f;
    }

    // Rotación de ruedas oruga sincronizada
    m_wheelAngle += (m_velocityX / 18.0f) * dt;

    // 2. Apuntado de torreta continuo y suave
    const float aimLen = std::hypot(input.aimX, input.aimY);
    if (aimLen > 0.001f) {
        float nx = input.aimX / aimLen;
        float ny = input.aimY / aimLen;
        if (ny > 0.0f) {
            ny = 0.0f;
            nx = (nx >= 0.0f) ? 1.0f : -1.0f;
        }
        m_aimDirX = nx;
        m_aimDirY = ny;
    }

    // 3. Cadencia de tiro y retroceso
    if (m_fireCooldown > 0.0f) {
        m_fireCooldown = std::max(0.0f, m_fireCooldown - dt);
    }
    if (m_recoilOffset > 0.0f) {
        m_recoilOffset = std::max(0.0f, m_recoilOffset - 40.0f * dt);
    }
}

bool Tank::tryFire(ProjectilePool& projectilePool, ParticlePool* particlePool) {
    if (!canFire()) {
        return false;
    }

    const auto tip = getBarrelTip();
    const auto pivot = getTurretPivot();
    bool isCrit = false;
    const int dmg = calculateBulletDamage(isCrit);
    const bool isPiercing = isOverdriveActive();

    // Retroceso visual inmediato en dirección contraria al disparo
    m_recoilOffset = 5.0f;

    // Eyectar casquillo de bala físico hacia atrás/arriba
    if (particlePool) {
        particlePool->spawnShellCasing(pivot.x, pivot.y, m_aimDirX, m_aimDirY);
    }

    const float perpX = -m_aimDirY;
    const float perpY = m_aimDirX;

    if (hasSpreadShot()) {
        // Disparo en abanico de 5 proyectiles (-22°, -11°, 0°, +11°, +22°)
        constexpr float cos11 = 0.9816f;
        constexpr float sin11 = 0.1908f;
        constexpr float cos22 = 0.9272f;
        constexpr float sin22 = 0.3746f;

        // Proyectil central
        projectilePool.spawn(tip.x, tip.y, m_aimDirX, m_aimDirY, dmg, isCrit, false, isPiercing);

        // Proyectiles a +-11°
        const float l1x = m_aimDirX * cos11 - m_aimDirY * (-sin11);
        const float l1y = m_aimDirX * (-sin11) + m_aimDirY * cos11;
        projectilePool.spawn(tip.x, tip.y, l1x, l1y, dmg, isCrit, false, isPiercing);

        const float r1x = m_aimDirX * cos11 - m_aimDirY * sin11;
        const float r1y = m_aimDirX * sin11 + m_aimDirY * cos11;
        projectilePool.spawn(tip.x, tip.y, r1x, r1y, dmg, isCrit, false, isPiercing);

        // Proyectiles a +-22°
        const float l2x = m_aimDirX * cos22 - m_aimDirY * (-sin22);
        const float l2y = m_aimDirX * (-sin22) + m_aimDirY * cos22;
        projectilePool.spawn(tip.x, tip.y, l2x, l2y, dmg, isCrit, false, isPiercing);

        const float r2x = m_aimDirX * cos22 - m_aimDirY * sin22;
        const float r2y = m_aimDirX * sin22 + m_aimDirY * cos22;
        projectilePool.spawn(tip.x, tip.y, r2x, r2y, dmg, isCrit, false, isPiercing);
    } else if (hasTwinCannon()) {
        // Doble Cañón Táctico (Desbloqueado con Twin Cannon II - 2 proyectiles paralelos)
        const float b1x = tip.x + perpX * 6.0f;
        const float b1y = tip.y + perpY * 6.0f;
        const float b2x = tip.x - perpX * 6.0f;
        const float b2y = tip.y - perpY * 6.0f;

        projectilePool.spawn(b1x, b1y, m_aimDirX, m_aimDirY, dmg, isCrit, false, isPiercing);
        projectilePool.spawn(b2x, b2y, m_aimDirX, m_aimDirY, dmg, isCrit, false, isPiercing);
    } else {
        // Cañón Central Único Estándar
        projectilePool.spawn(tip.x, tip.y, m_aimDirX, m_aimDirY, dmg, isCrit, false, isPiercing);
    }

    resetFireCooldown();
    Core::AudioManager::getInstance().playShoot();
    return true;
}

void Tank::update(float dt, const Input::InputState& input, ProjectilePool& projectilePool) {
    update(dt, input, projectilePool, static_cast<ParticlePool*>(nullptr));
}

void Tank::update(float dt, const Input::InputState& input, ProjectilePool& projectilePool, ParticlePool& particlePool) {
    update(dt, input, projectilePool, &particlePool);
}

void Tank::update(float dt, const Input::InputState& input, ProjectilePool& projectilePool, ParticlePool* particlePool) {
    update(dt, input);
    if (input.isFiring) {
        tryFire(projectilePool, particlePool);
    }

    if (m_pointDefenseCooldown > 0.0f) {
        m_pointDefenseCooldown -= dt;
    }

    // Disparo automático de torreta de misiles teledirigidos
    const int homingTier = (m_secondaryUpgradeTier > 0) ? m_secondaryUpgradeTier : m_homingPodLevel;
    if (homingTier > 0) {
        m_homingCooldown -= dt;
        if (m_homingCooldown <= 0.0f) {
            const auto spawnPos = hasWingmanDrone() ? getWingmanDronePosition() : getBarrelTip();
            const float perpX = -m_aimDirY;
            const float perpY = m_aimDirX;

            if (homingTier >= 2) {
                // Dual Homing II: 2 misiles teledirigidos cada 1.7s
                m_homingCooldown = 1.7f;
                projectilePool.spawn(spawnPos.x + perpX * 10.0f, spawnPos.y + perpY * 10.0f, m_aimDirX, m_aimDirY, 16, true, true);
                projectilePool.spawn(spawnPos.x - perpX * 10.0f, spawnPos.y - perpY * 10.0f, m_aimDirX, m_aimDirY, 16, true, true);
            } else {
                // Homing Pod I: 1 misil teledirigido cada 2.4s
                m_homingCooldown = 2.4f;
                projectilePool.spawn(spawnPos.x, spawnPos.y - 12.0f, m_aimDirX, m_aimDirY, 16, true, true);
            }
        }
    }
}

void Tank::render(SDL_Renderer* renderer) const {
    if (m_isDestroyed) {
        return;
    }

    const auto pivot = getTurretPivot();
    const auto tip = getBarrelTip();

    // ----------------- 0. RAYO DEVASTADOR MEGALASER (SI ESTÁ ACTIVO) -----------------
    if (isMegalaserActive()) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // Núcleo blanco intenso (18 px)
        const SDL_FRect laserCore{tip.x - 9.0f, 0.0f, 18.0f, tip.y};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRectF(renderer, &laserCore);

        // Aura azul eléctrico de energía (42 px)
        const SDL_FRect laserAura{tip.x - 21.0f, 0.0f, 42.0f, tip.y};
        SDL_SetRenderDrawColor(renderer, 0, 215, 255, 160);
        SDL_RenderFillRectF(renderer, &laserAura);

        // Destello en la boca del cañón
        const SDL_FRect laserMuzzle{tip.x - 24.0f, tip.y - 24.0f, 48.0f, 48.0f};
        SDL_SetRenderDrawColor(renderer, 180, 240, 255, 240);
        SDL_RenderFillRectF(renderer, &laserMuzzle);
    }

    // ----------------- 1. MIRA TÁCTICA CON CUADROS SÓLIDOS (CERO SDL_RenderDrawLine) -----------------
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 230, 80, 160);

    for (float dist = 30.0f; dist <= 2000.0f; dist += 45.0f) {
        const float cx = tip.x + m_aimDirX * dist;
        const float cy = tip.y + m_aimDirY * dist;
        const SDL_FRect dotRect{cx - 3.0f, cy - 3.0f, 6.0f, 6.0f};
        SDL_RenderFillRectF(renderer, &dotRect);
    }

    // ----------------- 2. RENDERIZADO DEL TANQUE Y CAÑÓN (PIXEL ART SVG / FALLBACK) -----------------
    SDL_Texture* spriteTex = Core::AssetManager::getInstance().getSpriteSheet();
    const bool isFlashing = isInvulnerable() && (static_cast<int>(m_invulnerableTimer * 14.0f) % 2 == 0);

    if (spriteTex) {
        if (isFlashing) {
            SDL_SetTextureAlphaMod(spriteTex, 120);
        } else {
            SDL_SetTextureAlphaMod(spriteTex, 255);
        }

        // A. TUBO DEL CAÑÓN INDEPENDIENTE (Col 1, Fila 0) - SE DIBUJA PRIMERO CON RETROCESO
        const double angleDeg = std::atan2(m_aimDirY, m_aimDirX) * 180.0 / 3.14159265358979323846;
        const SDL_Rect srcBarrel = Core::AssetManager::getInstance().getTankBarrelSrc();
        const SDL_Point center = {8, 15}; // Pivote anclado en extremo izquierdo dentro de la torreta
        const float recoilX = m_aimDirX * m_recoilOffset;
        const float recoilY = m_aimDirY * m_recoilOffset;

        if (hasTwinCannon()) {
            const float perpX = -m_aimDirY;
            const float perpY = m_aimDirX;
            // Cañón 1 (+5px perpendicular)
            const SDL_Rect dstBarrel1 = {
                static_cast<int>(pivot.x + perpX * 5.0f - center.x - recoilX),
                static_cast<int>(pivot.y + perpY * 5.0f - center.y - recoilY),
                80, 24
            };
            SDL_RenderCopyEx(renderer, spriteTex, &srcBarrel, &dstBarrel1, angleDeg, &center, SDL_FLIP_NONE);

            // Cañón 2 (-5px perpendicular)
            const SDL_Rect dstBarrel2 = {
                static_cast<int>(pivot.x - perpX * 5.0f - center.x - recoilX),
                static_cast<int>(pivot.y - perpY * 5.0f - center.y - recoilY),
                80, 24
            };
            SDL_RenderCopyEx(renderer, spriteTex, &srcBarrel, &dstBarrel2, angleDeg, &center, SDL_FLIP_NONE);
        } else {
            const SDL_Rect dstBarrel = {
                static_cast<int>(pivot.x - center.x - recoilX),
                static_cast<int>(pivot.y - center.y - recoilY),
                80, 30
            };
            SDL_RenderCopyEx(renderer, spriteTex, &srcBarrel, &dstBarrel, angleDeg, &center, SDL_FLIP_NONE);
        }

        // B. CHASIS Y CÚPULA DEL TANQUE (Col 0, Fila 0) - SE DIBUJA ENCIMA
        const SDL_Rect srcChassis = Core::AssetManager::getInstance().getTankChassisSrc();
        // El centro de la cúpula está en (55, 21) relativo al origen (9, 29) de srcChassis
        const SDL_Rect dstChassis = {
            static_cast<int>(pivot.x - 55.0f),
            static_cast<int>(pivot.y - 21.0f),
            110, 75
        };
        SDL_RenderCopy(renderer, spriteTex, &srcChassis, &dstChassis);
        SDL_SetTextureAlphaMod(spriteTex, 255);

        // Torreta de misiles pod si está instalada (Homing Pod)
        if (m_secondaryUpgradeTier > 0 || m_homingPodLevel > 0) {
            const SDL_FRect podRect{pivot.x - 30.0f, pivot.y - 14.0f, 14.0f, 16.0f};
            SDL_SetRenderDrawColor(renderer, 40, 45, 55, 255);
            SDL_RenderFillRectF(renderer, &podRect);

            const SDL_FRect podCap{pivot.x - 28.0f, pivot.y - 18.0f, 10.0f, 5.0f};
            SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
            SDL_RenderFillRectF(renderer, &podCap);
        }

        // Emisor óptico de Point-Defense Laser III si está instalado
        if (hasPointDefenseLaser()) {
            const SDL_FRect lensRect{pivot.x + 18.0f, pivot.y - 12.0f, 10.0f, 12.0f};
            SDL_SetRenderDrawColor(renderer, 30, 40, 50, 255);
            SDL_RenderFillRectF(renderer, &lensRect);

            const SDL_FRect lensCore{pivot.x + 20.0f, pivot.y - 10.0f, 6.0f, 8.0f};
            SDL_SetRenderDrawColor(renderer, 0, 230, 255, 255);
            SDL_RenderFillRectF(renderer, &lensCore);
        }
    } else {
        // Fallback geométrico sólido si el asset aún no estuviera listo
        const float perpX = -m_aimDirY;
        const float perpY = m_aimDirX;
        const float halfWidth = 6.0f;

        const float offX = perpX * halfWidth;
        const float offY = perpY * halfWidth;

        const SDL_Color barrelColor{80, 160, 100, 255};
        const SDL_Vertex vertices[4]{
            {{pivot.x + offX, pivot.y + offY}, barrelColor, {0.0f, 0.0f}},
            {{pivot.x - offX, pivot.y - offY}, barrelColor, {0.0f, 0.0f}},
            {{tip.x - offX, tip.y - offY}, barrelColor, {0.0f, 0.0f}},
            {{tip.x + offX, tip.y + offY}, barrelColor, {0.0f, 0.0f}}
        };
        const int indices[6]{0, 1, 2, 2, 3, 0};

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderGeometry(renderer, nullptr, vertices, 4, indices, 6);

        const float hullX = m_position.x - 65.0f;
        const float hullY = GROUND_Y - 46.0f;
        const SDL_FRect hullRect{hullX, hullY, 130.0f, 22.0f};
        SDL_SetRenderDrawColor(renderer, 50, 90, 65, 255);
        SDL_RenderFillRectF(renderer, &hullRect);

        drawSolidCircle(renderer, pivot.x, pivot.y, 19.0f, SDL_Color{45, 80, 55, 255});
        drawSolidCircle(renderer, pivot.x, pivot.y, 8.0f, SDL_Color{30, 55, 40, 255});

        const float treadX = m_position.x - 72.0f;
        const float treadY = GROUND_Y - 26.0f;
        const SDL_FRect treadRect{treadX, treadY, 144.0f, 26.0f};
        SDL_SetRenderDrawColor(renderer, 25, 30, 35, 255);
        SDL_RenderFillRectF(renderer, &treadRect);

        // 5 Ruedas de rodadura con radios rotatorios sólidos
        for (int i = 0; i < 5; ++i) {
            const float wheelCenterX = treadX + 16.0f + static_cast<float>(i) * 28.0f;
            const float wheelCenterY = treadY + 13.0f;

            drawSolidCircle(renderer, wheelCenterX, wheelCenterY, 10.0f, SDL_Color{50, 60, 70, 255});
            drawSolidCircle(renderer, wheelCenterX, wheelCenterY, 4.0f, SDL_Color{80, 95, 110, 255});

            // Radio giratorio sólido (cuadro de 3x3 rotado o alineado)
            const float spokeAngle = m_wheelAngle + static_cast<float>(i) * 0.4f;
            const float spokeX = wheelCenterX + std::cos(spokeAngle) * 6.0f;
            const float spokeY = wheelCenterY + std::sin(spokeAngle) * 6.0f;
            const SDL_FRect spokePoint{spokeX - 1.5f, spokeY - 1.5f, 3.0f, 3.0f};
            SDL_SetRenderDrawColor(renderer, 180, 210, 230, 255);
            SDL_RenderFillRectF(renderer, &spokePoint);
        }
    }

    // ----------------- 6. ORBES TÁCTICOS DE DEFENSA (ARMERÍA OPCIÓN 3) -----------------
    if (m_hasDefenseOrbs) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        constexpr float ORB_RADIUS = 46.0f;

        const float ox1 = pivot.x + std::cos(m_orbAngle) * ORB_RADIUS;
        const float oy1 = pivot.y + std::sin(m_orbAngle) * ORB_RADIUS * 0.5f;

        const float ox2 = pivot.x + std::cos(m_orbAngle + 3.14159f) * ORB_RADIUS;
        const float oy2 = pivot.y + std::sin(m_orbAngle + 3.14159f) * ORB_RADIUS * 0.5f;

        drawSolidCircle(renderer, ox1, oy1, 7.0f, SDL_Color{0, 220, 255, 230});
        drawSolidCircle(renderer, ox1, oy1, 3.0f, SDL_Color{255, 255, 255, 255});

        drawSolidCircle(renderer, ox2, oy2, 7.0f, SDL_Color{0, 220, 255, 230});
        drawSolidCircle(renderer, ox2, oy2, 3.0f, SDL_Color{255, 255, 255, 255});
    }

    // ----------------- 7. ESCUDO DE ENERGÍA Y SOBRECARGA -----------------
    if (isShieldOvercharged() || isInvulnerable()) {
        const bool isOvercharge = isShieldOvercharged();
        const float timer = isOvercharge ? m_shieldOverchargeTimer : m_invulnerableTimer;
        const float pulse = std::abs(std::sin(timer * 12.0f));
        const Uint8 alpha = static_cast<Uint8>(80 + 90 * pulse);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        const SDL_FRect shieldBubble{m_position.x - 78.0f, GROUND_Y - 58.0f, 156.0f, 60.0f};
        if (isOvercharge) {
            SDL_SetRenderDrawColor(renderer, 0, 240, 255, alpha);
        } else {
            SDL_SetRenderDrawColor(renderer, 70, 200, 255, alpha);
        }
        SDL_RenderFillRectF(renderer, &shieldBubble);

        const SDL_FRect shieldBorder{m_position.x - 78.0f, GROUND_Y - 58.0f, 156.0f, 4.0f};
        SDL_SetRenderDrawColor(renderer, 220, 255, 255, 240);
        SDL_RenderFillRectF(renderer, &shieldBorder);
    }

    // ----------------- 8. DRON ESCOLTA FÍSICO (WINGMAN DRONE) -----------------
    if (hasWingmanDrone()) {
        const auto dronePos = getWingmanDronePosition();
        if (spriteTex) {
            const SDL_Rect droneSrc = Core::AssetManager::getInstance().getSpriteCellSrc(3, 3);
            const SDL_FRect droneDst{dronePos.x - 22.0f, dronePos.y - 22.0f, 44.0f, 44.0f};
            SDL_RenderCopyF(renderer, spriteTex, &droneSrc, &droneDst);
        } else {
            const SDL_FRect droneHull{dronePos.x - 16.0f, dronePos.y - 8.0f, 32.0f, 16.0f};
            SDL_SetRenderDrawColor(renderer, 70, 85, 105, 255);
            SDL_RenderFillRectF(renderer, &droneHull);
        }

        // Propulsores gemelos cian brillantes
        const SDL_FRect th1{dronePos.x - 13.0f, dronePos.y + 12.0f, 5.0f, 5.0f};
        const SDL_FRect th2{dronePos.x + 8.0f, dronePos.y + 12.0f, 5.0f, 5.0f};
        SDL_SetRenderDrawColor(renderer, 0, 240, 255, 230);
        SDL_RenderFillRectF(renderer, &th1);
        SDL_RenderFillRectF(renderer, &th2);
    }

    // ----------------- 9. AURA RESPLANDECIENTE DE SOBRECARGA (OVERDRIVE) -----------------
    if (isOverdriveActive()) {
        const float pulse = std::abs(std::sin(m_animTimer * 14.0f));
        const Uint8 auraAlpha = static_cast<Uint8>(90 + 90 * pulse);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // Capa exterior dorada de energía
        const SDL_FRect auraOuter{m_position.x - 74.0f, GROUND_Y - 56.0f, 148.0f, 58.0f};
        SDL_SetRenderDrawColor(renderer, 255, 215, 30, auraAlpha);
        SDL_RenderFillRectF(renderer, &auraOuter);

        // Capa interior cian relampagueante
        const SDL_FRect auraInner{m_position.x - 68.0f, GROUND_Y - 50.0f, 136.0f, 48.0f};
        SDL_SetRenderDrawColor(renderer, 0, 240, 255, static_cast<Uint8>(auraAlpha * 0.75f));
        SDL_RenderFillRectF(renderer, &auraInner);

        // Destellos de energía en la torreta
        const SDL_FRect muzzleAura{pivot.x - 18.0f, pivot.y - 18.0f, 36.0f, 36.0f};
        SDL_SetRenderDrawColor(renderer, 255, 255, 200, static_cast<Uint8>(auraAlpha * 0.9f));
        SDL_RenderFillRectF(renderer, &muzzleAura);
    }
}

} // namespace Entities
