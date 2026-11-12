#pragma once

#include "math/Vector2D.hpp"
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <cstdint>
#include <string_view>

namespace Entities {

class EnemyPool;
class ParticlePool;

enum class BossType : uint8_t {
    IronVulture = 0,    // Nivel 1: Súper helicóptero bimotor (HP 2850)
    DesertGoliath,      // Nivel 2: Fortaleza volante acorazada (HP 3500)
    NightPhantom,       // Nivel 3: Bombardero furtivo supersónico (HP 4150)
    ArcticValkyrie,     // Nivel 4: Fortaleza aérea con dos rotores gigantes (HP 4800)
    RedStarCarrier,     // Nivel 5: Súper acorazado aéreo nuclear (HP 5450)
    JungleAnaconda,     // Nivel 6: Sierpe aérea articulada (HP 6100)
    CyberLeviathan,     // Nivel 7: Crucero sigiloso neón (HP 6750)
    KrakenDreadnought,  // Nivel 8: Ekranoplano colosal naval (HP 7400)
    BioHazardChimera,   // Nivel 9: Doble zepelín químico (HP 8050)
    DoomsdayExpress,    // Nivel 10: Tren aéreo nuclear con Railgun (HP 8700)
    VulcanForgeMaster,  // Nivel 11: Excavadora térmica con rotores y magma (HP 9350)
    PrismOverlord,      // Nivel 12: Octaedro cristalino con 4 prismas orbitales (HP 10000)
    StarbreakerCarrier, // Nivel 13: Portaaviones nodriza orbital aeroespacial (HP 10650)
    TempestArchon,      // Nivel 14: Generador atmosférico Tesla toroidal (HP 11300)
    AegisOrbitalCore,   // Nivel 15: Estación nuclear orbital con lluvia ICBM (HP 11950)
    DreadnoughtAeroShip,// Nivel 16: Acorazado aéreo de astillero naval (HP 12600)
    ColossusScrapMech,  // Nivel 17: Coloso meca oxidado del valle de reliquias (HP 13250)
    CryoFusionMatrix,   // Nivel 18: Matriz flotante de reactor crio-solar (HP 13900)
    GothicArchangel,    // Nivel 19: Fortaleza alada gótica con crucetas lanzamisiles (HP 14550)
    LunarSiloDominator, // Nivel 20: Plataforma lunar con silo ICBM integrado (HP 15200)
    AsteroidAnnihilator,// Nivel 21: Destructor pesado del cinturón de asteroides (HP 15850)
    MartianTerraformCore,// Nivel 22: Cosechador bio-atmosférico marciano (HP 16500)
    NexusCyberMind,     // Nivel 23: Mente colmena cibernética con drones de asalto (HP 17150)
    QuantumSingularityEngine, // Nivel 24: Reactor de antimateria con anillos de aceleración (HP 17800)
    OmegaDoomsdayTitan  // Nivel 25: Titán Final Supremo Nuclear (HP 22000)
};

class Boss {
public:
    Boss();
    ~Boss() = default;

    Boss(const Boss&) = delete;
    Boss& operator=(const Boss&) = delete;

    void reset() noexcept;
    void spawn(int level);
    void update(float dt, float tankX, float tankY, EnemyPool& enemyPool, ParticlePool& particlePool, float& screenShake);
    void render(SDL_Renderer* renderer) const;
    void renderHealthBar(SDL_Renderer* renderer) const;

    [[nodiscard]] bool isActive() const noexcept { return m_active; }
    [[nodiscard]] bool isDefeated() const noexcept { return m_isDefeated; }
    [[nodiscard]] bool isEnraged() const noexcept {
        if (!m_active || m_isDefeated) return false;
        if (m_type == BossType::OmegaDoomsdayTitan) {
            return m_hp < static_cast<int>(static_cast<float>(m_maxHp) * 0.30f); // Fase 3 (< 30% HP Apocalypse Enrage)
        }
        return m_hp < static_cast<int>(static_cast<float>(m_maxHp) * 0.25f);     // Enrage al < 25% para Niveles 1 al 24
    }
    [[nodiscard]] BossType getType() const noexcept { return m_type; }
    [[nodiscard]] std::string_view getName() const noexcept { return m_name; }
    [[nodiscard]] int getHp() const noexcept { return m_hp; }
    [[nodiscard]] int getMaxHp() const noexcept { return m_maxHp; }
    [[nodiscard]] float getPositionX() const noexcept { return m_x; }
    [[nodiscard]] float getPositionY() const noexcept { return m_y; }
    [[nodiscard]] float getX() const noexcept { return m_x; }
    [[nodiscard]] float getY() const noexcept { return m_y; }

    [[nodiscard]] SDL_FRect getBoundingBox() const noexcept {
        return {m_x - m_width * 0.5f, m_y - m_height * 0.5f, m_width, m_height};
    }

    [[nodiscard]] bool isRailgunFiring() const noexcept { return m_railgunFiring; }
    [[nodiscard]] float getRailgunX() const noexcept { return m_railgunTargetX; }

    [[nodiscard]] bool isFlameJetActive() const noexcept { return m_flameJetActive; }
    [[nodiscard]] float getFlameJetX() const noexcept { return m_flameJetX; }

    [[nodiscard]] bool isLightningWarning() const noexcept { return m_lightningWarning; }
    [[nodiscard]] bool isLightningFiring() const noexcept { return m_lightningFiring; }
    [[nodiscard]] float getLightningX() const noexcept { return m_lightningTargetX; }

    [[nodiscard]] bool isOmegaLaserWarning() const noexcept { return m_omegaLaserWarning; }
    [[nodiscard]] bool isOmegaLaserFiring() const noexcept { return m_omegaLaserFiring; }
    [[nodiscard]] float getOmegaLaserX1() const noexcept { return m_omegaLaserTargetX1; }
    [[nodiscard]] float getOmegaLaserX2() const noexcept { return m_omegaLaserTargetX2; }

    [[nodiscard]] int getLevel() const noexcept { return m_level; }
    [[nodiscard]] int getBurstCount() const noexcept { return m_burstCount; }
    [[nodiscard]] float getRestTimer() const noexcept { return m_restTimer; }
    [[nodiscard]] bool isResting() const noexcept { return m_restTimer > 0.0f; }

    void registerAttackBurst() noexcept;
    [[nodiscard]] int getSpreadCount() const noexcept;
    void spawnSpreadSalvo(EnemyPool& enemyPool, float startX, float startY, float targetX, float targetY, float speed, int damage);

    bool takeDamage(int amount, ParticlePool& particlePool, float& screenShake);

private:
    float nextRandomFloat() noexcept;

    bool m_active{false};
    BossType m_type{BossType::IronVulture};
    std::string_view m_name{"IRON VULTURE"};

    float m_x{960.0f};
    float m_y{-150.0f};
    float m_targetY{230.0f};
    float m_vx{160.0f};
    float m_baseVx{160.0f};
    float m_width{160.0f};
    float m_height{60.0f};

    int m_level{1};
    int m_burstCount{0};
    float m_restTimer{0.0f};
    int m_hp{2850};
    int m_maxHp{2850};
    float m_hitFlashTimer{0.0f};
    float m_attackTimer{0.0f};
    float m_secondaryAttackTimer{0.0f};
    float m_animTimer{0.0f};
    float m_smokeParticleTimer{0.0f};

    // Sigilo para Night Phantom
    float m_stealthTimer{0.0f};
    float m_stealthAlpha{255.0f};

    // Cañón de Riel para Doomsday Express
    float m_railgunTimer{0.0f};
    bool m_railgunCharging{false};
    float m_railgunTargetX{960.0f};
    bool m_railgunFiring{false};
    float m_railgunFireDuration{0.0f};

    // Chorro de Magma para Vulcan Forge-Master (Nivel 11)
    bool m_flameJetActive{false};
    float m_flameJetTimer{0.0f};
    float m_flameJetDuration{0.0f};
    float m_flameJetX{960.0f};

    // Relámpago Tesla Atmosférico para Tempest Archon (Nivel 14)
    bool m_lightningWarning{false};
    float m_lightningWarningTimer{0.0f};
    bool m_lightningFiring{false};
    float m_lightningFireDuration{0.0f};
    float m_lightningTargetX{960.0f};

    // Cañones Láser Duales y Misiles para Omega Doomsday Titan (Nivel 25)
    bool m_omegaLaserWarning{false};
    bool m_omegaLaserFiring{false};
    float m_omegaLaserTimer{0.0f};
    float m_omegaLaserFireDuration{0.0f};
    float m_omegaLaserTargetX1{960.0f};
    float m_omegaLaserTargetX2{960.0f};
    float m_omegaIcbmTimer{0.0f};
    float m_omegaDroneTimer{0.0f};

    // Secuencia de derrota
    bool m_isDefeated{false};
    float m_defeatTimer{0.0f};
    float m_explosionTimer{0.0f};

    uint32_t m_rngState{135792468u};
};

} // namespace Entities
