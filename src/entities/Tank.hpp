#pragma once

#include "input/InputState.hpp"
#include "math/Vector2D.hpp"
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <algorithm>

namespace Entities {

class ProjectilePool;
class ParticlePool;

class Tank {
public:
    static constexpr float GROUND_Y{980.0f};
    static constexpr float MAX_SPEED{650.0f};
    static constexpr float ACCELERATION{3200.0f};
    static constexpr float DECELERATION{2400.0f};
    static constexpr float MIN_BOUND_X{60.0f};
    static constexpr float MAX_BOUND_X{1860.0f};
    static constexpr float BARREL_LENGTH{72.0f};
    static constexpr float FIRE_RATE{0.065f};

    Tank();
    ~Tank() = default;

    Tank(const Tank&) = delete;
    Tank& operator=(const Tank&) = delete;

    void init(float spawnX);
    void update(float dt, const Input::InputState& input);
    void update(float dt, const Input::InputState& input, ProjectilePool& projectilePool);
    void update(float dt, const Input::InputState& input, ProjectilePool& projectilePool, ParticlePool& particlePool);
    void update(float dt, const Input::InputState& input, ProjectilePool& projectilePool, ParticlePool* particlePool);
    void render(SDL_Renderer* renderer) const;

    [[nodiscard]] const Math::Vector2D& getPosition() const noexcept { return m_position; }
    [[nodiscard]] float getVelocityX() const noexcept { return m_velocityX; }
    [[nodiscard]] Math::Vector2D getTurretPivot() const noexcept;
    [[nodiscard]] Math::Vector2D getBarrelTip() const noexcept;
    [[nodiscard]] Math::Vector2D getMuzzlePosition() const noexcept { return getBarrelTip(); }
    [[nodiscard]] Math::Vector2D getAimDirection() const noexcept { return {m_aimDirX, m_aimDirY}; }
    [[nodiscard]] bool canFire() const noexcept { return !m_isDestroyed && (m_fireCooldown <= 0.0f); }

    [[nodiscard]] float getShield() const noexcept { return m_shield; }
    [[nodiscard]] float getMaxShield() const noexcept { return m_maxShield; }
    void takeDamage(float amount) noexcept {
        if (isInvulnerable() || isShieldOvercharged() || m_isDestroyed) {
            return;
        }
        m_overdriveMeter = std::max(0.0f, m_overdriveMeter - 25.0f);
        if (m_chassisUpgradeTier >= 3) {
            amount *= 0.85f; // Reactive Armor III reduce daño un 15%
        }
        m_shield = (m_shield > amount) ? (m_shield - amount) : 0.0f;
    }
    void repairShield(float amount = 100.0f) noexcept {
        m_shield = std::min(m_shield + amount, m_maxShield);
    }

    [[nodiscard]] bool isDestroyed() const noexcept { return m_isDestroyed; }
    void setDestroyed(bool destroyed) noexcept { m_isDestroyed = destroyed; }

    [[nodiscard]] bool isInvulnerable() const noexcept { return m_invulnerableTimer > 0.0f; }
    void setInvulnerable(float duration) noexcept { m_invulnerableTimer = duration; }

    // Medidor de Sobrecarga (Overdrive / Hiper-Modo)
    void addOverdrive(float amount) noexcept {
        if (m_isDestroyed) return;
        if (m_overdriveDuration > 0.0f) return;
        m_overdriveMeter = std::min(100.0f, m_overdriveMeter + amount);
        if (m_overdriveMeter >= 100.0f) {
            m_overdriveMeter = 0.0f;
            m_overdriveDuration = 6.0f;
        }
    }
    [[nodiscard]] float getOverdriveMeter() const noexcept { return m_overdriveMeter; }
    [[nodiscard]] bool isOverdriveActive() const noexcept { return m_overdriveDuration > 0.0f; }
    [[nodiscard]] float getOverdriveDuration() const noexcept { return m_overdriveDuration; }

    // Dron Escolta Visual (Wingman Drone)
    [[nodiscard]] bool hasWingmanDrone() const noexcept {
        return (m_secondaryUpgradeTier > 0) || (m_homingPodLevel > 0);
    }
    [[nodiscard]] Math::Vector2D getWingmanDronePosition() const noexcept;

    // Power-Ups
    void activateSpreadShot(float dur = 12.0f) noexcept { m_spreadShotTimer = dur; }
    void activateRapidFire(float dur = 12.0f) noexcept { m_rapidFireTimer = dur; }
    void activateShieldOvercharge(float dur = 6.0f) noexcept { m_shieldOverchargeTimer = dur; }
    void activateDamageBoost(float dur = 10.0f) noexcept { m_damageBoostTimer = dur; }
    void addMegalaserPiece() noexcept;

    [[nodiscard]] bool isShieldOvercharged() const noexcept { return m_shieldOverchargeTimer > 0.0f; }
    [[nodiscard]] bool hasSpreadShot() const noexcept { return m_spreadShotTimer > 0.0f; }
    [[nodiscard]] bool hasRapidFire() const noexcept { return m_rapidFireTimer > 0.0f; }
    [[nodiscard]] bool hasDamageBoost() const noexcept { return m_damageBoostTimer > 0.0f; }
    [[nodiscard]] bool isMegalaserActive() const noexcept { return m_megalaserActiveTimer > 0.0f; }
    [[nodiscard]] int getMegalaserFragments() const noexcept { return m_megalaserFragments; }

    // Sistema de 15 Mejoras de Armería
    void setAmmoTier(int t) noexcept { m_ammoUpgradeTier = t; }
    [[nodiscard]] int getAmmoTier() const noexcept { return m_ammoUpgradeTier; }

    void setCannonTier(int t) noexcept { m_cannonUpgradeTier = t; }
    [[nodiscard]] int getCannonTier() const noexcept { return m_cannonUpgradeTier; }
    [[nodiscard]] bool hasTwinCannon() const noexcept { return m_cannonUpgradeTier >= 2; }

    void setChassisTier(int t) noexcept;
    [[nodiscard]] int getChassisTier() const noexcept { return m_chassisUpgradeTier; }

    void setSecondaryTier(int t) noexcept { m_secondaryUpgradeTier = t; }
    [[nodiscard]] int getSecondaryTier() const noexcept { return m_secondaryUpgradeTier; }
    [[nodiscard]] bool hasPointDefenseLaser() const noexcept { return m_secondaryUpgradeTier >= 3; }
    [[nodiscard]] float getPointDefenseCooldown() const noexcept { return m_pointDefenseCooldown; }
    void resetPointDefenseCooldown() noexcept { m_pointDefenseCooldown = 2.0f; }

    void resetUpgrades() noexcept;

    // Métodos heredados/compatibilidad
    void upgradeArmor() noexcept { setChassisTier(1); }
    void upgradeHomingPod() noexcept { setSecondaryTier(std::min(m_secondaryUpgradeTier + 1, 2)); }
    void upgradeFlakAndDamage() noexcept { m_permanentDamageBonus += 1; m_hasDefenseOrbs = true; }

    [[nodiscard]] int getHomingPodLevel() const noexcept { return m_secondaryUpgradeTier; }
    [[nodiscard]] int getPermanentDamageBonus() const noexcept { return m_permanentDamageBonus; }
    [[nodiscard]] bool hasDefenseOrbs() const noexcept { return m_hasDefenseOrbs; }
    [[nodiscard]] float getOrbAngle() const noexcept { return m_orbAngle; }

    [[nodiscard]] SDL_FRect getBoundingBox() const noexcept {
        return {m_position.x - 65.0f, GROUND_Y - 48.0f, 130.0f, 48.0f};
    }

    bool tryFire(ProjectilePool& projectilePool, ParticlePool* particlePool = nullptr);
    [[nodiscard]] float getEffectiveFireRate() const noexcept;
    void resetFireCooldown() noexcept {
        m_fireCooldown = getEffectiveFireRate();
    }

    // Calcula el daño del proyectil según el estado actual
    [[nodiscard]] int calculateBulletDamage(bool& outIsCritical) noexcept;

private:
    Math::Vector2D m_position{960.0f, GROUND_Y};
    float m_velocityX{0.0f};
    float m_aimDirX{0.0f};
    float m_aimDirY{-1.0f};
    float m_fireCooldown{0.0f};
    float m_wheelAngle{0.0f};
    float m_shield{100.0f};
    float m_maxShield{100.0f};
    bool m_isDestroyed{false};
    float m_invulnerableTimer{0.0f};
    float m_recoilOffset{0.0f};
    float m_animTimer{0.0f};

    // Medidor y Duración de Sobrecarga
    float m_overdriveMeter{0.0f};
    float m_overdriveDuration{0.0f};

    // Temporizadores de Mejoras Temporales
    float m_spreadShotTimer{0.0f};
    float m_rapidFireTimer{0.0f};
    float m_shieldOverchargeTimer{0.0f};
    float m_damageBoostTimer{0.0f};
    int m_megalaserFragments{0};
    float m_megalaserActiveTimer{0.0f};

    // Tiers de Mejoras Permanentes de Sesión (15 Upgrades)
    int m_ammoUpgradeTier{0};
    int m_cannonUpgradeTier{0};
    int m_chassisUpgradeTier{0};
    int m_secondaryUpgradeTier{0};
    float m_pointDefenseCooldown{0.0f};

    int m_permanentDamageBonus{0};
    int m_homingPodLevel{0};
    float m_homingCooldown{0.0f};
    bool m_hasDefenseOrbs{false};
    float m_orbAngle{0.0f};

    uint32_t m_rngState{777111u};
};

} // namespace Entities
