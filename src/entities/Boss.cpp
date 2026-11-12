#include "entities/Boss.hpp"
#include "entities/EnemyPool.hpp"
#include "entities/ParticlePool.hpp"
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

Boss::Boss() {
    reset();
}

float Boss::nextRandomFloat() noexcept {
    m_rngState = m_rngState * 1664525u + 1013904223u;
    return static_cast<float>(m_rngState & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
}

void Boss::reset() noexcept {
    m_active = false;
    m_isDefeated = false;
    m_defeatTimer = 0.0f;
    m_explosionTimer = 0.0f;
    m_hitFlashTimer = 0.0f;
    m_attackTimer = 0.0f;
    m_secondaryAttackTimer = 0.0f;
    m_animTimer = 0.0f;
    m_smokeParticleTimer = 0.0f;
    m_stealthTimer = 0.0f;
    m_stealthAlpha = 255.0f;
    m_railgunTimer = 0.0f;
    m_railgunCharging = false;
    m_railgunTargetX = 960.0f;
    m_railgunFiring = false;
    m_railgunFireDuration = 0.0f;
    m_flameJetActive = false;
    m_flameJetTimer = 0.0f;
    m_flameJetDuration = 0.0f;
    m_flameJetX = 960.0f;
    m_lightningWarning = false;
    m_lightningWarningTimer = 0.0f;
    m_lightningFiring = false;
    m_lightningFireDuration = 0.0f;
    m_lightningTargetX = 960.0f;
    m_omegaLaserWarning = false;
    m_omegaLaserFiring = false;
    m_omegaLaserTimer = 0.0f;
    m_omegaLaserFireDuration = 0.0f;
    m_omegaLaserTargetX1 = 960.0f;
    m_omegaLaserTargetX2 = 960.0f;
    m_omegaIcbmTimer = 0.0f;
    m_omegaDroneTimer = 0.0f;
    m_x = 960.0f;
    m_y = -150.0f;
    m_targetY = 230.0f;
    m_baseVx = 160.0f;
    m_vx = 160.0f;
    m_width = 160.0f;
    m_height = 60.0f;
    m_level = 1;
    m_burstCount = 0;
    m_restTimer = 0.0f;
    m_hp = 2850;
    m_maxHp = 2850;
    m_type = BossType::IronVulture;
    m_name = "IRON VULTURE";
}

void Boss::spawn(int level) {
    reset();
    m_active = true;
    m_level = level;
    m_burstCount = 0;
    m_restTimer = 0.0f;
    m_x = 960.0f;
    m_y = -160.0f;

    if (level >= 25) {
        m_type = BossType::OmegaDoomsdayTitan;
        m_name = "FINAL BOSS: OMEGA DOOMSDAY TITAN";
        m_hp = 25000;
        m_maxHp = 25000;
        m_width = 380.0f;
        m_height = 115.0f;
        m_targetY = 210.0f;
        m_baseVx = 140.0f;
        m_vx = 140.0f;
        return;
    }

    const int bossIndex = std::clamp(level - 1, 0, 24);
    m_type = static_cast<BossType>(bossIndex);

    // Rebalanceo reglamentario para Niveles 1 al 24: maxHp = 1600 + (level * 480)
    const int scaledHp = 1600 + level * 480;
    m_hp = scaledHp;
    m_maxHp = scaledHp;

    switch (m_type) {
    case BossType::IronVulture:
        m_name = "BOSS: IRON VULTURE";
        m_width = 170.0f;
        m_height = 65.0f;
        m_targetY = 220.0f;
        m_baseVx = 170.0f;
        m_vx = 170.0f;
        break;
    case BossType::DesertGoliath:
        m_name = "BOSS: DESERT GOLIATH";
        m_width = 210.0f;
        m_height = 70.0f;
        m_targetY = 210.0f;
        m_baseVx = 130.0f;
        m_vx = 130.0f;
        break;
    case BossType::NightPhantom:
        m_name = "BOSS: NIGHT PHANTOM";
        m_width = 160.0f;
        m_height = 50.0f;
        m_targetY = 190.0f;
        m_baseVx = 300.0f;
        m_vx = 300.0f;
        break;
    case BossType::ArcticValkyrie:
        m_name = "BOSS: ARCTIC VALKYRIE";
        m_width = 220.0f;
        m_height = 75.0f;
        m_targetY = 230.0f;
        m_baseVx = 160.0f;
        m_vx = 160.0f;
        break;
    case BossType::RedStarCarrier:
        m_name = "BOSS: RED STAR ICBM CARRIER";
        m_width = 260.0f;
        m_height = 85.0f;
        m_targetY = 200.0f;
        m_baseVx = 120.0f;
        m_vx = 120.0f;
        break;
    case BossType::JungleAnaconda:
        m_name = "BOSS: JUNGLE ANACONDA";
        m_width = 190.0f;
        m_height = 60.0f;
        m_targetY = 220.0f;
        m_baseVx = 180.0f;
        m_vx = 180.0f;
        break;
    case BossType::CyberLeviathan:
        m_name = "BOSS: CYBER LEVIATHAN";
        m_width = 230.0f;
        m_height = 65.0f;
        m_targetY = 210.0f;
        m_baseVx = 220.0f;
        m_vx = 220.0f;
        break;
    case BossType::KrakenDreadnought:
        m_name = "BOSS: KRAKEN DREADNOUGHT";
        m_width = 280.0f;
        m_height = 80.0f;
        m_targetY = 240.0f;
        m_baseVx = 135.0f;
        m_vx = 135.0f;
        break;
    case BossType::BioHazardChimera:
        m_name = "BOSS: BIO-HAZARD CHIMERA";
        m_width = 250.0f;
        m_height = 75.0f;
        m_targetY = 215.0f;
        m_baseVx = 145.0f;
        m_vx = 145.0f;
        break;
    case BossType::DoomsdayExpress:
        m_name = "BOSS: DOOMSDAY EXPRESS";
        m_width = 300.0f;
        m_height = 85.0f;
        m_targetY = 220.0f;
        m_baseVx = 150.0f;
        m_vx = 150.0f;
        break;
    case BossType::VulcanForgeMaster:
        m_name = "BOSS: VULCAN FORGE-MASTER";
        m_width = 250.0f;
        m_height = 80.0f;
        m_targetY = 215.0f;
        m_baseVx = 145.0f;
        m_vx = 145.0f;
        m_flameJetTimer = 3.0f;
        break;
    case BossType::PrismOverlord:
        m_name = "BOSS: PRISM OVERLORD";
        m_width = 220.0f;
        m_height = 85.0f;
        m_targetY = 205.0f;
        m_baseVx = 170.0f;
        m_vx = 170.0f;
        break;
    case BossType::StarbreakerCarrier:
        m_name = "BOSS: STARBREAKER CARRIER";
        m_width = 290.0f;
        m_height = 80.0f;
        m_targetY = 210.0f;
        m_baseVx = 135.0f;
        m_vx = 135.0f;
        break;
    case BossType::TempestArchon:
        m_name = "BOSS: TEMPEST ARCHON";
        m_width = 240.0f;
        m_height = 75.0f;
        m_targetY = 215.0f;
        m_baseVx = 160.0f;
        m_vx = 160.0f;
        break;
    case BossType::AegisOrbitalCore:
        m_name = "BOSS: AEGIS ORBITAL CORE";
        m_width = 320.0f;
        m_height = 90.0f;
        m_targetY = 195.0f;
        m_baseVx = 120.0f;
        m_vx = 120.0f;
        break;
    case BossType::DreadnoughtAeroShip:
        m_name = "BOSS: NEPTUNE DREADNOUGHT";
        m_width = 290.0f;
        m_height = 85.0f;
        m_targetY = 225.0f;
        m_baseVx = 135.0f;
        m_vx = 135.0f;
        break;
    case BossType::ColossusScrapMech:
        m_name = "BOSS: RUST COLOSSUS";
        m_width = 240.0f;
        m_height = 80.0f;
        m_targetY = 220.0f;
        m_baseVx = 150.0f;
        m_vx = 150.0f;
        break;
    case BossType::CryoFusionMatrix:
        m_name = "BOSS: CRYO-SOLAR WRAITH";
        m_width = 220.0f;
        m_height = 70.0f;
        m_targetY = 210.0f;
        m_baseVx = 190.0f;
        m_vx = 190.0f;
        break;
    case BossType::GothicArchangel:
        m_name = "BOSS: GOTHIC ARCHANGEL";
        m_width = 250.0f;
        m_height = 75.0f;
        m_targetY = 215.0f;
        m_baseVx = 160.0f;
        m_vx = 160.0f;
        break;
    case BossType::LunarSiloDominator:
        m_name = "BOSS: LUNAR ICBM DOMINATOR";
        m_width = 310.0f;
        m_height = 90.0f;
        m_targetY = 205.0f;
        m_baseVx = 125.0f;
        m_vx = 125.0f;
        break;
    case BossType::AsteroidAnnihilator:
        m_name = "BOSS: VOID ANNIHILATOR";
        m_width = 270.0f;
        m_height = 80.0f;
        m_targetY = 220.0f;
        m_baseVx = 165.0f;
        m_vx = 165.0f;
        break;
    case BossType::MartianTerraformCore:
        m_name = "BOSS: ARES HARVESTER";
        m_width = 260.0f;
        m_height = 85.0f;
        m_targetY = 215.0f;
        m_baseVx = 140.0f;
        m_vx = 140.0f;
        break;
    case BossType::NexusCyberMind:
        m_name = "BOSS: NEXUS OVERMIND";
        m_width = 230.0f;
        m_height = 65.0f;
        m_targetY = 200.0f;
        m_baseVx = 210.0f;
        m_vx = 210.0f;
        break;
    case BossType::QuantumSingularityEngine:
        m_name = "BOSS: ANTIMATTER WARDEN";
        m_width = 280.0f;
        m_height = 85.0f;
        m_targetY = 210.0f;
        m_baseVx = 155.0f;
        m_vx = 155.0f;
        break;
    case BossType::OmegaDoomsdayTitan:
        m_name = "FINAL BOSS: OMEGA DOOMSDAY TITAN";
        m_width = 380.0f;
        m_height = 115.0f;
        m_targetY = 210.0f;
        m_baseVx = 140.0f;
        m_vx = 140.0f;
        break;
    }
}

bool Boss::takeDamage(int amount, ParticlePool& particlePool, float& screenShake) {
    if (!m_active || m_isDefeated) {
        return false;
    }

    m_hp -= amount;
    m_hitFlashTimer = 0.08f;
    screenShake = std::min(screenShake + 0.08f, 1.0f);

    if (m_hp <= 0) {
        m_hp = 0;
        m_isDefeated = true;
        m_defeatTimer = 3.0f;
        m_explosionTimer = 0.05f;

        particlePool.spawnExplosion(m_x, m_y, 45, 360.0f, true);
        screenShake = 1.0f;
        return true;
    }
    return false;
}

void Boss::registerAttackBurst() noexcept {
    m_burstCount++;
    const int threshold = (m_level <= 12) ? 2 : 3;
    if (m_burstCount >= threshold) {
        m_burstCount = 0;
        m_restTimer = (m_level <= 12) ? 2.4f : 1.8f;
        m_attackTimer = 0.0f;
        m_secondaryAttackTimer = 0.0f;
    }
}

int Boss::getSpreadCount() const noexcept {
    if (m_level <= 7) {
        return 3;
    }
    if (m_level <= 16) {
        return (m_level % 2 == 0) ? 4 : 5;
    }
    if (m_level <= 20) {
        return 5;
    }
    if (m_level <= 24) {
        return 6;
    }
    return 7;
}

void Boss::spawnSpreadSalvo(EnemyPool& enemyPool, float startX, float startY, float targetX, float targetY, float speed, int damage) {
    float dx = targetX - startX;
    float dy = (targetY - 30.0f) - startY;
    const float dist = std::hypot(dx, dy);
    if (dist > 1.0f) {
        dx /= dist;
        dy /= dist;
    } else {
        dx = 0.0f;
        dy = 1.0f;
    }

    const int count = getSpreadCount();
    const float spreadStep = (count <= 3) ? 0.18f : 0.13f;
    const float startAngle = -0.5f * static_cast<float>(count - 1) * spreadStep;

    for (int i = 0; i < count; ++i) {
        const float angle = startAngle + static_cast<float>(i) * spreadStep;
        const float cosA = std::cos(angle);
        const float sinA = std::sin(angle);
        const float vx = (dx * cosA - dy * sinA) * speed;
        const float vy = (dx * sinA + dy * cosA) * speed;
        enemyPool.spawnMissile(startX, startY, vx, vy, true, damage);
    }
}

void Boss::update(float dt, float tankX, float tankY, EnemyPool& enemyPool, ParticlePool& particlePool, float& screenShake) {
    if (!m_active) {
        return;
    }

    m_animTimer += dt;

    if (m_hitFlashTimer > 0.0f) {
        m_hitFlashTimer -= dt;
        if (m_hitFlashTimer < 0.0f) {
            m_hitFlashTimer = 0.0f;
        }
    }

    // -------------------------------------------------------------------------
    // A. SECUENCIA DE DERROTA (3.0 SEGUNDOS DE EXPLOSIONES MASIVAS)
    // -------------------------------------------------------------------------
    if (m_isDefeated) {
        m_defeatTimer -= dt;
        m_explosionTimer -= dt;

        m_x += (nextRandomFloat() - 0.5f) * 14.0f;
        m_y += 22.0f * dt;
        screenShake = std::max(screenShake, 0.75f);

        if (m_explosionTimer <= 0.0f) {
            m_explosionTimer = 0.07f;
            const float offsetX = (nextRandomFloat() - 0.5f) * m_width * 0.9f;
            const float offsetY = (nextRandomFloat() - 0.5f) * m_height * 0.8f;
            particlePool.spawnExplosion(m_x + offsetX, m_y + offsetY, 25, 290.0f, true);
        }

        if (m_defeatTimer <= 0.0f) {
            particlePool.spawnExplosion(m_x, m_y, 75, 480.0f, true);
            particlePool.spawnExplosion(m_x - 65.0f, m_y, 50, 420.0f, true);
            particlePool.spawnExplosion(m_x + 65.0f, m_y, 50, 420.0f, true);
            screenShake = 1.0f;
            m_active = false;
        }
        return;
    }

    // -------------------------------------------------------------------------
    // B. FASE DE FURIA (ENRAGE MODE AL < 25% DE SALUD EN JEFES 1-24 / < 30% EN TITAN)
    // -------------------------------------------------------------------------
    const bool enraged = isEnraged();
    float currentSpeed = m_baseVx;
    if (enraged) {
        if (m_type == BossType::OmegaDoomsdayTitan) {
            currentSpeed = m_baseVx * 1.45f; // +45% velocidad en Fase 3 Apocalypse Enrage
        } else {
            currentSpeed = m_baseVx * 1.35f;
        }
    }
    m_vx = (m_vx >= 0.0f ? 1.0f : -1.0f) * currentSpeed;

    // Emisión continua de humo y fuego si está encolerizado
    if (enraged) {
        m_smokeParticleTimer -= dt;
        if (m_smokeParticleTimer <= 0.0f) {
            m_smokeParticleTimer = (m_type == BossType::OmegaDoomsdayTitan) ? 0.04f : 0.06f;
            const float ox = (nextRandomFloat() - 0.5f) * m_width * 0.8f;
            const float oy = (nextRandomFloat() - 0.5f) * m_height * 0.6f;
            particlePool.spawnExplosion(m_x + ox, m_y + oy, 5, 120.0f, false);
        }
    }

    // -------------------------------------------------------------------------
    // C. ENTRADA CINEMÁTICA Y NAVEGACIÓN
    // -------------------------------------------------------------------------
    if (m_y < m_targetY) {
        m_y += 120.0f * dt;
        if (m_y > m_targetY) {
            m_y = m_targetY;
        }
    } else {
        const float floatOffset = std::sin(m_animTimer * 1.8f) * 22.0f * dt;
        m_y += floatOffset;
    }

    m_x += m_vx * dt;
    const float minX = 260.0f + m_width * 0.5f;
    const float maxX = 1660.0f - m_width * 0.5f;

    if (m_x >= maxX) {
        m_x = maxX;
        m_vx = -std::abs(m_vx);
    } else if (m_x <= minX) {
        m_x = minX;
        m_vx = std::abs(m_vx);
    }

    // -------------------------------------------------------------------------
    // D. PATRONES DE ATAQUE (SISTEMA ESCALONADO POR NIVEL Y COOLDOWN WINDOWS)
    // -------------------------------------------------------------------------
    const int bossProjDamage = (m_level == 10) ? 11 : ((m_level == 15) ? 14 : (5 + static_cast<int>(std::round(static_cast<float>(m_level) * 0.65f))));
    const int bossHeavyDamage = bossProjDamage + 4;
    const float bossBulletSpeed = 240.0f + (static_cast<float>(m_level) * 8.5f);
    const float baseFireInterval = std::max(0.85f, 2.85f - (static_cast<float>(m_level) * 0.08f));
    const float effectiveInterval = enraged ? (baseFireInterval * 0.80f) : baseFireInterval;
    const float effectiveSecondaryInterval = effectiveInterval * 1.5f;

    // Ventana de descanso (Boss Cooldown Window): Durante m_restTimer > 0, el jefe esquiva pero NO dispara
    if (m_restTimer > 0.0f) {
        m_restTimer -= dt;
        if (m_restTimer < 0.0f) {
            m_restTimer = 0.0f;
        }
        // Concluir haces continuos si estaban activos
        if (m_railgunFiring) {
            m_railgunFireDuration -= dt;
            if (m_railgunFireDuration <= 0.0f) m_railgunFiring = false;
        }
        if (m_flameJetActive) {
            m_flameJetDuration -= dt;
            m_flameJetX = m_x;
            if (m_flameJetDuration <= 0.0f) m_flameJetActive = false;
        }
        if (m_lightningFiring) {
            m_lightningFireDuration -= dt;
            if (m_lightningFireDuration <= 0.0f) m_lightningFiring = false;
        }
        if (m_omegaLaserFiring) {
            m_omegaLaserFireDuration -= dt;
            if (m_omegaLaserFireDuration <= 0.0f) m_omegaLaserFiring = false;
        }
        return;
    }

    m_attackTimer += dt;
    m_secondaryAttackTimer += dt;

    switch (m_type) {
    case BossType::IronVulture: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 20.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnBomb(m_x - 35.0f, m_y + 25.0f, -40.0f, 70.0f, 450.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x, m_y + 28.0f, 0.0f, 75.0f, 450.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 35.0f, m_y + 25.0f, 40.0f, 70.0f, 450.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::DesertGoliath: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 25.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnBomb(m_x - 50.0f, m_y + 30.0f, -40.0f, 60.0f, 470.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x, m_y + 30.0f, 0.0f, 60.0f, 470.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 50.0f, m_y + 30.0f, 40.0f, 60.0f, 470.0f, true, bossHeavyDamage);
            enemyPool.spawnEnemy(EnemyType::DroneSwarm, 1, m_y - 20.0f, m_level);
            enemyPool.spawnEnemy(EnemyType::DroneSwarm, -1, m_y - 20.0f, m_level);
            registerAttackBurst();
        }
        break;
    }

    case BossType::NightPhantom: {
        m_stealthTimer += dt * 1.5f;
        m_stealthAlpha = 80.0f + (std::sin(m_stealthTimer) * 0.5f + 0.5f) * 175.0f;

        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 15.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnBomb(m_x - 30.0f, m_y + 20.0f, m_vx * 0.4f, 90.0f, 520.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x, m_y + 22.0f, 0.0f, 100.0f, 520.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 30.0f, m_y + 20.0f, m_vx * 0.4f, 90.0f, 520.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::ArcticValkyrie: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 28.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnMine(m_x - 50.0f, m_y + 30.0f, bossHeavyDamage);
            enemyPool.spawnMine(m_x + 50.0f, m_y + 30.0f, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::RedStarCarrier: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 30.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.triggerIcbm(tankX);
            enemyPool.spawnBomb(m_x - 80.0f, m_y + 32.0f, -30.0f, 100.0f, 500.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 80.0f, m_y + 32.0f, 30.0f, 100.0f, 500.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::JungleAnaconda: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 15.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            const float dirSign = (m_vx >= 0.0f) ? -1.0f : 1.0f;
            const float tailX = m_x + dirSign * 140.0f;
            const float tailY = m_y + std::sin(m_animTimer * 3.6f + 3.4f) * 28.0f;
            enemyPool.spawnMine(tailX, tailY, bossHeavyDamage);
            enemyPool.spawnClusterBomb(m_x, m_y + 20.0f, m_vx * 0.35f, 80.0f, 420.0f, 0.85f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::CyberLeviathan: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 16.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnMissile(m_x - 50.0f, m_y + 24.0f, 0.0f, bossBulletSpeed * 1.1f, true, bossProjDamage);
            enemyPool.spawnMissile(m_x + 50.0f, m_y + 24.0f, 0.0f, bossBulletSpeed * 1.1f, true, bossProjDamage);
            enemyPool.spawnBomb(m_x, m_y + 24.0f, 0.0f, 105.0f, 520.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::KrakenDreadnought: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 28.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnEnemy(EnemyType::DroneSwarm, 1, m_y - 25.0f, m_level);
            enemyPool.spawnEnemy(EnemyType::DroneSwarm, -1, m_y - 25.0f, m_level);
            enemyPool.spawnBomb(m_x - 70.0f, m_y + 28.0f, -70.0f, 70.0f, 460.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 70.0f, m_y + 28.0f, 70.0f, 70.0f, 460.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::BioHazardChimera: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 25.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnClusterBomb(m_x - 60.0f, m_y + 25.0f, -35.0f, 85.0f, 390.0f, 0.9f, true, bossHeavyDamage);
            enemyPool.spawnClusterBomb(m_x + 60.0f, m_y + 25.0f,  35.0f, 85.0f, 390.0f, 0.9f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::DoomsdayExpress: {
        if (!m_railgunCharging && !m_railgunFiring) {
            m_railgunTimer -= dt;
            if (m_railgunTimer <= 0.0f && m_y >= m_targetY - 10.0f) {
                m_railgunCharging = true;
                m_railgunTimer = 1.0f;
                m_railgunTargetX = tankX;
            }
        } else if (m_railgunCharging) {
            m_railgunTimer -= dt;
            m_railgunTargetX += (tankX - m_railgunTargetX) * 3.5f * dt;
            if (m_railgunTimer <= 0.0f) {
                m_railgunCharging = false;
                m_railgunFiring = true;
                m_railgunFireDuration = 0.35f;
                m_railgunTimer = effectiveInterval * 2.5f;
                screenShake = std::min(screenShake + 0.60f, 1.0f);
                registerAttackBurst();
            }
        } else if (m_railgunFiring) {
            m_railgunFireDuration -= dt;
            if (m_railgunFireDuration <= 0.0f) {
                m_railgunFiring = false;
            }
        }

        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 35.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.triggerIcbm(tankX);
            enemyPool.spawnBomb(m_x - 50.0f, m_y + 35.0f, -25.0f, 95.0f, 480.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 50.0f, m_y + 35.0f,  25.0f, 95.0f, 480.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::VulcanForgeMaster: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 24.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnBomb(m_x - 60.0f, m_y + 30.0f, -40.0f, 90.0f, 480.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 60.0f, m_y + 30.0f,  40.0f, 90.0f, 480.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }

        if (!m_flameJetActive) {
            m_flameJetTimer -= dt;
            const bool nearTank = (std::abs(m_x - tankX) < 140.0f);
            if ((m_flameJetTimer <= 0.0f || (nearTank && m_flameJetTimer <= 1.0f)) && m_y >= m_targetY - 10.0f) {
                m_flameJetActive = true;
                m_flameJetDuration = 1.35f;
                m_flameJetTimer = effectiveInterval * 2.5f;
                m_flameJetX = m_x;
                screenShake = std::min(screenShake + 0.35f, 1.0f);
                registerAttackBurst();
            }
        } else {
            m_flameJetDuration -= dt;
            m_flameJetX = m_x;
            particlePool.spawnExplosion(m_flameJetX + (nextRandomFloat() - 0.5f) * 20.0f,
                                       m_y + 35.0f + nextRandomFloat() * 400.0f, 3, 110.0f, false);
            if (m_flameJetDuration <= 0.0f) {
                m_flameJetActive = false;
            }
        }
        break;
    }

    case BossType::PrismOverlord: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 20.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            for (int i = 0; i < 4; ++i) {
                const float angle = m_animTimer * 2.5f + static_cast<float>(i) * 1.5708f;
                const float nodeX = m_x + std::cos(angle) * 85.0f;
                const float nodeY = m_y + std::sin(angle) * 35.0f;
                float ndx = tankX - nodeX;
                float ndy = (tankY - 30.0f) - nodeY;
                const float ndist = std::hypot(ndx, ndy);
                if (ndist > 1.0f) {
                    ndx /= ndist;
                    ndy /= ndist;
                }
                enemyPool.spawnMissile(nodeX, nodeY, ndx * bossBulletSpeed, ndy * bossBulletSpeed, true, bossProjDamage);
            }
            registerAttackBurst();
        }
        break;
    }

    case BossType::StarbreakerCarrier: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 26.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnEnemy(EnemyType::DroneSwarm, 1, m_y - 25.0f, m_level);
            enemyPool.spawnEnemy(EnemyType::DroneSwarm, -1, m_y - 25.0f, m_level);
            enemyPool.spawnClusterBomb(m_x, m_y + 30.0f, 0.0f, 80.0f, 440.0f, 0.85f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::TempestArchon: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 25.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (!m_lightningWarning && !m_lightningFiring) {
            if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
                m_secondaryAttackTimer = 0.0f;
                m_lightningWarning = true;
                m_lightningWarningTimer = 0.60f;
                m_lightningTargetX = tankX;
            }
        } else if (m_lightningWarning) {
            m_lightningWarningTimer -= dt;
            m_lightningTargetX += (tankX - m_lightningTargetX) * 2.8f * dt;
            if (m_lightningWarningTimer <= 0.0f) {
                m_lightningWarning = false;
                m_lightningFiring = true;
                m_lightningFireDuration = 0.40f;
                screenShake = std::min(screenShake + 0.65f, 1.0f);
                registerAttackBurst();
            }
        } else if (m_lightningFiring) {
            m_lightningFireDuration -= dt;
            particlePool.spawnExplosion(m_lightningTargetX, 980.0f, 4, 180.0f, false);
            if (m_lightningFireDuration <= 0.0f) {
                m_lightningFiring = false;
            }
        }
        break;
    }

    case BossType::AegisOrbitalCore: {
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 30.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            const float x1 = std::clamp(tankX - 220.0f, 120.0f, 1800.0f);
            const float x2 = std::clamp(tankX, 120.0f, 1800.0f);
            const float x3 = std::clamp(tankX + 220.0f, 120.0f, 1800.0f);
            enemyPool.triggerMultiIcbm(x1, x2, x3, 598.0f);
            enemyPool.spawnClusterBomb(m_x - 60.0f, m_y + 35.0f, -30.0f, 100.0f, 500.0f, 0.85f, true, bossHeavyDamage);
            enemyPool.spawnClusterBomb(m_x + 60.0f, m_y + 35.0f,  30.0f, 100.0f, 500.0f, 0.85f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::DreadnoughtAeroShip: { // Nivel 16 (Astillero Naval)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 30.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnBomb(m_x - 50.0f, m_y + 35.0f, -30.0f, 100.0f, 500.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 50.0f, m_y + 35.0f,  30.0f, 100.0f, 500.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::ColossusScrapMech: { // Nivel 17 (Valle de Reliquias)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 32.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnClusterBomb(m_x, m_y + 35.0f, 0.0f, 100.0f, 480.0f, 0.85f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::CryoFusionMatrix: { // Nivel 18 (Reactor Crio-Solar)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 26.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnBomb(m_x - 60.0f, m_y + 30.0f, -35.0f, 100.0f, 460.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 60.0f, m_y + 30.0f,  35.0f, 100.0f, 460.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::GothicArchangel: { // Nivel 19 (Ciudadela Gótica)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 25.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnClusterBomb(m_x, m_y + 32.0f, 0.0f, 95.0f, 460.0f, 0.85f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::LunarSiloDominator: { // Nivel 20 (level % 5 == 0 -> Lluvia de Misiles Balísticos Verticales)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 35.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            const float x1 = std::clamp(tankX - 190.0f, 120.0f, 1800.0f);
            const float x2 = std::clamp(tankX, 120.0f, 1800.0f);
            const float x3 = std::clamp(tankX + 190.0f, 120.0f, 1800.0f);
            enemyPool.triggerMultiIcbm(x1, x2, x3, 620.0f);
            screenShake = std::min(screenShake + 0.35f, 1.0f);
            registerAttackBurst();
        }
        break;
    }

    case BossType::AsteroidAnnihilator: { // Nivel 21 (Cinturón de Asteroides)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 25.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnClusterBomb(m_x - 65.0f, m_y + 30.0f, -35.0f, 100.0f, 480.0f, 0.85f, true, bossHeavyDamage);
            enemyPool.spawnClusterBomb(m_x + 65.0f, m_y + 30.0f,  35.0f, 100.0f, 480.0f, 0.85f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::MartianTerraformCore: { // Nivel 22 (Marte Terraformado)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 32.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnBomb(m_x - 50.0f, m_y + 35.0f, -30.0f, 110.0f, 500.0f, true, bossHeavyDamage);
            enemyPool.spawnBomb(m_x + 50.0f, m_y + 35.0f,  30.0f, 110.0f, 500.0f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::NexusCyberMind: { // Nivel 23 (Matriz Cibernética IA)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 24.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnEnemy(EnemyType::DroneSwarm, 1, m_y - 20.0f, m_level);
            enemyPool.spawnEnemy(EnemyType::DroneSwarm, -1, m_y - 20.0f, m_level);
            registerAttackBurst();
        }
        break;
    }

    case BossType::QuantumSingularityEngine: { // Nivel 24 (Reactor de Antimateria)
        if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
            m_attackTimer = 0.0f;
            spawnSpreadSalvo(enemyPool, m_x, m_y + 35.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
            registerAttackBurst();
        }

        if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
            m_secondaryAttackTimer = 0.0f;
            enemyPool.spawnClusterBomb(m_x - 60.0f, m_y + 35.0f, -30.0f, 110.0f, 520.0f, 0.85f, true, bossHeavyDamage);
            enemyPool.spawnClusterBomb(m_x + 60.0f, m_y + 35.0f,  30.0f, 110.0f, 520.0f, 0.85f, true, bossHeavyDamage);
            registerAttackBurst();
        }
        break;
    }

    case BossType::OmegaDoomsdayTitan: {
        // JEFE FINAL SUPREMO (NIVEL 25) - 3 FASES DE COMBATE PROGRESIVAS
        const bool phase1 = (m_hp > 14300); // 100% a 65% HP
        const bool phase2 = (m_hp <= 14300 && m_hp > 6600); // 65% a 30% HP

        if (phase1) {
            if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
                m_attackTimer = 0.0f;
                spawnSpreadSalvo(enemyPool, m_x, m_y + 24.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
                registerAttackBurst();
            }

            if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
                m_secondaryAttackTimer = 0.0f;
                enemyPool.spawnClusterBomb(m_x - 75.0f, m_y + 35.0f, -40.0f, 75.0f, 420.0f, 0.9f, true, bossHeavyDamage);
                enemyPool.spawnClusterBomb(m_x + 75.0f, m_y + 35.0f,  40.0f, 75.0f, 420.0f, 0.9f, true, bossHeavyDamage);
                registerAttackBurst();
            }

            m_omegaDroneTimer += dt;
            if (m_omegaDroneTimer >= 4.5f && m_y >= m_targetY - 10.0f) {
                m_omegaDroneTimer = 0.0f;
                enemyPool.spawnEnemy(EnemyType::DroneSwarm, 1, m_y - 25.0f, 25);
                enemyPool.spawnEnemy(EnemyType::DroneSwarm, -1, m_y - 25.0f, 25);
            }
        } else if (phase2) {
            // FASE 2: Cañones Láser Duales dirigidos al tanque (Aviso 0.75s -> Disparo 0.65s)
            if (!m_omegaLaserWarning && !m_omegaLaserFiring) {
                m_omegaLaserTimer += dt;
                if (m_omegaLaserTimer >= 4.0f && m_y >= m_targetY - 10.0f) {
                    m_omegaLaserTimer = 0.0f;
                    m_omegaLaserWarning = true;
                    m_omegaLaserFireDuration = 0.75f;
                    m_omegaLaserTargetX1 = tankX - 45.0f;
                    m_omegaLaserTargetX2 = tankX + 45.0f;
                }
            } else if (m_omegaLaserWarning) {
                m_omegaLaserFireDuration -= dt;
                m_omegaLaserTargetX1 += ((tankX - 45.0f) - m_omegaLaserTargetX1) * 3.0f * dt;
                m_omegaLaserTargetX2 += ((tankX + 45.0f) - m_omegaLaserTargetX2) * 3.0f * dt;
                if (m_omegaLaserFireDuration <= 0.0f) {
                    m_omegaLaserWarning = false;
                    m_omegaLaserFiring = true;
                    m_omegaLaserFireDuration = 0.65f;
                    screenShake = std::min(screenShake + 0.60f, 1.0f);
                    registerAttackBurst();
                }
            } else if (m_omegaLaserFiring) {
                m_omegaLaserFireDuration -= dt;
                particlePool.spawnExplosion(m_omegaLaserTargetX1, 980.0f, 4, 160.0f, false);
                particlePool.spawnExplosion(m_omegaLaserTargetX2, 980.0f, 4, 160.0f, false);
                if (m_omegaLaserFireDuration <= 0.0f) {
                    m_omegaLaserFiring = false;
                }
            }

            m_omegaIcbmTimer += dt;
            if (m_omegaIcbmTimer >= 4.5f) {
                m_omegaIcbmTimer = 0.0f;
                enemyPool.triggerIcbm(tankX - 130.0f, 540.0f);
                enemyPool.triggerIcbm(tankX + 130.0f, 540.0f);
            }

            if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
                m_attackTimer = 0.0f;
                spawnSpreadSalvo(enemyPool, m_x, m_y + 24.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
                registerAttackBurst();
            }
        } else {
            // FASE 3: APOCALYPSE ENRAGE (< 30% HP)
            m_omegaIcbmTimer += dt;
            if (m_omegaIcbmTimer >= 3.8f) {
                m_omegaIcbmTimer = 0.0f;
                enemyPool.triggerMultiIcbm(tankX - 220.0f, tankX, tankX + 220.0f, 580.0f);
                enemyPool.triggerIcbm(tankX + ((m_rngState % 2 == 0) ? -360.0f : 360.0f), 580.0f);
            }

            if (m_attackTimer >= effectiveInterval && m_y >= m_targetY - 10.0f) {
                m_attackTimer = 0.0f;
                spawnSpreadSalvo(enemyPool, m_x, m_y + 25.0f, tankX, tankY, bossBulletSpeed, bossProjDamage);
                registerAttackBurst();
            }

            if (m_secondaryAttackTimer >= effectiveSecondaryInterval && m_y >= m_targetY - 10.0f) {
                m_secondaryAttackTimer = 0.0f;
                enemyPool.spawnBomb(m_x - 60.0f, m_y + 30.0f, -50.0f, 120.0f, 500.0f, true, bossHeavyDamage);
                enemyPool.spawnBomb(m_x + 60.0f, m_y + 30.0f,  50.0f, 120.0f, 500.0f, true, bossHeavyDamage);
                registerAttackBurst();
            }
        }
        break;
    }
    }
}

void Boss::render(SDL_Renderer* renderer) const {
    if (!m_active || !renderer) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const bool hit = (m_hitFlashTimer > 0.0f);
    const bool enraged = isEnraged();
    const Uint8 baseAlpha = (m_type == BossType::NightPhantom)
        ? static_cast<Uint8>(std::clamp(m_stealthAlpha, 60.0f, 255.0f))
        : 255;

    // Tinte rojizo/anaranjado parpadeante si está encolerizado
    const bool rageFlash = enraged && (static_cast<int>(m_animTimer * 10.0f) % 2 == 0);

    // =========================================================================
    // 0. RENDERIZADO DEL SPRITE COLOSAL DEL JEFE (PIXEL ART SVG ATLAS)
    // =========================================================================
    SDL_Texture* spriteTex = Core::AssetManager::getInstance().getSpriteSheet();
    if (spriteTex) {
        int spriteIndex = 10;
        switch (m_type) {
        case BossType::IronVulture:         spriteIndex = 10; break;
        case BossType::DesertGoliath:       spriteIndex = 11; break;
        case BossType::NightPhantom:        spriteIndex = 13; break; // Bombardero furtivo negro
        case BossType::ArcticValkyrie:      spriteIndex = 14; break;
        case BossType::RedStarCarrier:      spriteIndex = 20; break; // Fortaleza verde colosal
        case BossType::JungleAnaconda:      spriteIndex = 16; break;
        case BossType::CyberLeviathan:      spriteIndex = 17; break;
        case BossType::KrakenDreadnought:   spriteIndex = 18; break;
        case BossType::BioHazardChimera:    spriteIndex = 19; break;
        case BossType::DoomsdayExpress:     spriteIndex = 21; break;
        case BossType::VulcanForgeMaster:   spriteIndex = 23; break;
        case BossType::PrismOverlord:       spriteIndex = 12; break;
        case BossType::StarbreakerCarrier:  spriteIndex = 24; break;
        case BossType::TempestArchon:       spriteIndex = 15; break;
        case BossType::AegisOrbitalCore:    spriteIndex = 22; break; // Dirigible / Estación dorada
        case BossType::DreadnoughtAeroShip: spriteIndex = 5;  break; // Acorazado cañonero naval
        case BossType::ColossusScrapMech:   spriteIndex = 8;  break; // Coloso meca blindado
        case BossType::CryoFusionMatrix:    spriteIndex = 7;  break; // Nave crio-solar supersónica
        case BossType::GothicArchangel:     spriteIndex = 9;  break; // Fortaleza orbital alada
        case BossType::LunarSiloDominator:  spriteIndex = 3;  break; // Bombardero/Silo lunar colosal
        case BossType::AsteroidAnnihilator: spriteIndex = 4;  break; // Bombardero pesado del vacío
        case BossType::MartianTerraformCore:spriteIndex = 11; break; // Cosechador pesado marciano
        case BossType::NexusCyberMind:      spriteIndex = 6;  break; // Enjambre cibernético central
        case BossType::QuantumSingularityEngine: spriteIndex = 12; break; // Núcleo cuántico
        case BossType::OmegaDoomsdayTitan:  spriteIndex = 24; break; // Súper Titán Final Nuclear
        }

        const SDL_Rect srcRect = Core::AssetManager::getInstance().getAircraftSrc(spriteIndex);
        const float scaleMultW = (m_type == BossType::OmegaDoomsdayTitan) ? 1.25f : 1.10f;
        const float scaleMultH = (m_type == BossType::OmegaDoomsdayTitan) ? 1.45f : 1.30f;
        const int sW = static_cast<int>(m_width * scaleMultW);
        const int sH = static_cast<int>(m_height * scaleMultH);
        const SDL_Rect dstRect = {
            static_cast<int>(m_x - sW * 0.5f),
            static_cast<int>(m_y - sH * 0.5f),
            sW, sH
        };

        const double angle = (m_vx >= 0.0f) ? 90.0 : -90.0;
        if (hit) {
            SDL_SetTextureColorMod(spriteTex, 255, 80, 80);
        } else if (m_type == BossType::OmegaDoomsdayTitan && enraged) {
            // Fase 3: Parpadeo en rojo y dorado intenso
            const float pulse = (std::sin(m_animTimer * 12.0f) + 1.0f) * 0.5f;
            SDL_SetTextureColorMod(spriteTex, 255, static_cast<Uint8>(50.0f + pulse * 170.0f), 30);
        } else if (enraged && rageFlash) {
            SDL_SetTextureColorMod(spriteTex, 255, 130, 40);
        }

        if (m_type == BossType::NightPhantom) {
            SDL_SetTextureAlphaMod(spriteTex, baseAlpha);
        }

        SDL_RenderCopyEx(renderer, spriteTex, &srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);

        SDL_SetTextureColorMod(spriteTex, 255, 255, 255);
        SDL_SetTextureAlphaMod(spriteTex, 255);
    }

    // -------------------------------------------------------------------------
    // 1. BOSS 1: IRON VULTURE (SÚPER HELICÓPTERO BIMOTOR)
    // -------------------------------------------------------------------------
    if (m_type == BossType::IronVulture) {
        const SDL_FRect mainHull{m_x - 75.0f, m_y - 18.0f, 150.0f, 36.0f};
        Uint8 hr = hit ? 255 : (rageFlash ? 200 : 65);
        Uint8 hg = hit ? 255 : (rageFlash ? 50 : 85);
        Uint8 hb = hit ? 255 : (rageFlash ? 30 : 70);
        SDL_SetRenderDrawColor(renderer, hr, hg, hb, baseAlpha);
        SDL_RenderFillRectF(renderer, &mainHull);

        const SDL_FRect wingBar{m_x - 85.0f, m_y - 28.0f, 170.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 50, hit ? 255 : 65, hit ? 255 : 55, baseAlpha);
        SDL_RenderFillRectF(renderer, &wingBar);

        const SDL_FRect podL{m_x - 85.0f, m_y - 38.0f, 24.0f, 14.0f};
        const SDL_FRect podR{m_x + 61.0f, m_y - 38.0f, 24.0f, 14.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 35, hit ? 255 : 45, hit ? 255 : 40, baseAlpha);
        SDL_RenderFillRectF(renderer, &podL);
        SDL_RenderFillRectF(renderer, &podR);

        const float rotorSpan = 70.0f * std::abs(std::cos(m_animTimer * (enraged ? 35.0f : 25.0f)));
        const float safeSpan = std::max(12.0f, rotorSpan);
        const SDL_FRect bladeL{m_x - 73.0f - safeSpan * 0.5f, m_y - 42.0f, safeSpan, 5.0f};
        const SDL_FRect bladeR{m_x + 73.0f - safeSpan * 0.5f, m_y - 42.0f, safeSpan, 5.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 30, hit ? 255 : 30, hit ? 255 : 35, 230);
        SDL_RenderFillRectF(renderer, &bladeL);
        SDL_RenderFillRectF(renderer, &bladeR);

        const SDL_FRect cockpit{m_x - 22.0f, m_y - 8.0f, 44.0f, 16.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : (enraged ? 255 : 230), hit ? 255 : (enraged ? 40 : 170), hit ? 255 : 40, baseAlpha);
        SDL_RenderFillRectF(renderer, &cockpit);

        const SDL_FRect turret{m_x - 18.0f, m_y + 18.0f, 36.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 40, hit ? 255 : 45, hit ? 255 : 45, baseAlpha);
        SDL_RenderFillRectF(renderer, &turret);
    }
    // -------------------------------------------------------------------------
    // 2. BOSS 2: DESERT GOLIATH (FORTALEZA VOLANTE ACORAZADA)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::DesertGoliath) {
        const SDL_FRect wings{m_x - 105.0f, m_y - 20.0f, 210.0f, 24.0f};
        Uint8 wr = hit ? 255 : (rageFlash ? 220 : 155);
        Uint8 wg = hit ? 255 : (rageFlash ? 60 : 105);
        Uint8 wb = hit ? 255 : (rageFlash ? 30 : 55);
        SDL_SetRenderDrawColor(renderer, wr, wg, wb, baseAlpha);
        SDL_RenderFillRectF(renderer, &wings);

        const SDL_FRect hull{m_x - 55.0f, m_y - 30.0f, 110.0f, 60.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 130, hit ? 255 : 85, hit ? 255 : 45, baseAlpha);
        SDL_RenderFillRectF(renderer, &hull);

        for (int i = 0; i < 4; ++i) {
            const float ex = m_x - 85.0f + static_cast<float>(i) * 55.0f;
            const SDL_FRect engine{ex - 10.0f, m_y - 12.0f, 20.0f, 28.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 55, hit ? 255 : 45, hit ? 255 : 35, baseAlpha);
            SDL_RenderFillRectF(renderer, &engine);

            const SDL_FRect flame{ex - 6.0f, m_y - 18.0f, 12.0f, 6.0f};
            SDL_SetRenderDrawColor(renderer, 255, 140, 25, 230);
            SDL_RenderFillRectF(renderer, &flame);
        }

        const SDL_FRect gunL{m_x - 48.0f, m_y + 28.0f, 16.0f, 12.0f};
        const SDL_FRect gunR{m_x + 32.0f, m_y + 28.0f, 16.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 35, hit ? 255 : 30, hit ? 255 : 25, baseAlpha);
        SDL_RenderFillRectF(renderer, &gunL);
        SDL_RenderFillRectF(renderer, &gunR);
    }
    // -------------------------------------------------------------------------
    // 3. BOSS 3: NIGHT PHANTOM (BOMBARDERO FURTIVO SUPERSÓNICO)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::NightPhantom) {
        Uint8 pr = hit ? 255 : (rageFlash ? 180 : 25);
        Uint8 pg = hit ? 255 : (rageFlash ? 30 : 28);
        Uint8 pb = hit ? 255 : (rageFlash ? 20 : 35);
        const SDL_Color bodyCol{pr, pg, pb, baseAlpha};

        const SDL_Vertex deltaVerts[4] = {
            {{m_x, m_y + 25.0f},               bodyCol, {0.0f, 0.0f}},
            {{m_x - 80.0f, m_y - 25.0f},        bodyCol, {0.0f, 0.0f}},
            {{m_x, m_y - 12.0f},                bodyCol, {0.0f, 0.0f}},
            {{m_x + 80.0f, m_y - 25.0f},        bodyCol, {0.0f, 0.0f}}
        };
        const int deltaIdx[6] = {0, 1, 2, 0, 2, 3};
        SDL_RenderGeometry(renderer, nullptr, deltaVerts, 4, deltaIdx, 6);

        const SDL_FRect stealthCockpit{m_x - 8.0f, m_y - 5.0f, 16.0f, 18.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 240, hit ? 255 : 35, hit ? 255 : 35, baseAlpha);
        SDL_RenderFillRectF(renderer, &stealthCockpit);

        const SDL_FRect aftL{m_x - 30.0f, m_y - 28.0f, 12.0f, 6.0f};
        const SDL_FRect aftR{m_x + 18.0f, m_y - 28.0f, 12.0f, 6.0f};
        SDL_SetRenderDrawColor(renderer, 60, 160, 255, baseAlpha);
        SDL_RenderFillRectF(renderer, &aftL);
        SDL_RenderFillRectF(renderer, &aftR);
    }
    // -------------------------------------------------------------------------
    // 4. BOSS 4: ARCTIC VALKYRIE (FORTALEZA AÉREA DE DOBLE ROTOR)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::ArcticValkyrie) {
        const SDL_FRect hull{m_x - 80.0f, m_y - 25.0f, 160.0f, 50.0f};
        Uint8 vr = hit ? 255 : (rageFlash ? 190 : 90);
        Uint8 vg = hit ? 255 : (rageFlash ? 60 : 125);
        Uint8 vb = hit ? 255 : (rageFlash ? 40 : 155);
        SDL_SetRenderDrawColor(renderer, vr, vg, vb, baseAlpha);
        SDL_RenderFillRectF(renderer, &hull);

        const SDL_FRect tiltL{m_x - 110.0f, m_y - 20.0f, 32.0f, 38.0f};
        const SDL_FRect tiltR{m_x + 78.0f, m_y - 20.0f, 32.0f, 38.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 60, hit ? 255 : 85, hit ? 255 : 115, baseAlpha);
        SDL_RenderFillRectF(renderer, &tiltL);
        SDL_RenderFillRectF(renderer, &tiltR);

        const float rSpan = 84.0f * std::abs(std::cos(m_animTimer * (enraged ? 32.0f : 22.0f)));
        const float safeRSpan = std::max(14.0f, rSpan);
        const SDL_FRect bL{m_x - 94.0f - safeRSpan * 0.5f, m_y - 24.0f, safeRSpan, 6.0f};
        const SDL_FRect bR{m_x + 94.0f - safeRSpan * 0.5f, m_y - 24.0f, safeRSpan, 6.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 180, hit ? 255 : 225, hit ? 255 : 255, 230);
        SDL_RenderFillRectF(renderer, &bL);
        SDL_RenderFillRectF(renderer, &bR);

        const SDL_FRect cryoL{m_x - 45.0f, m_y + 22.0f, 22.0f, 12.0f};
        const SDL_FRect cryoR{m_x + 23.0f, m_y + 22.0f, 22.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 0, hit ? 255 : 205, hit ? 255 : 255, baseAlpha);
        SDL_RenderFillRectF(renderer, &cryoL);
        SDL_RenderFillRectF(renderer, &cryoR);
    }
    // -------------------------------------------------------------------------
    // 5. BOSS 5: RED STAR ICBM CARRIER (SÚPER ACORAZADO AÉREO NUCLEAR)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::RedStarCarrier) {
        const SDL_FRect hull{m_x - 130.0f, m_y - 32.0f, 260.0f, 64.0f};
        Uint8 rr = hit ? 255 : (rageFlash ? 210 : 65);
        Uint8 rg = hit ? 255 : (rageFlash ? 40 : 30);
        Uint8 rb = hit ? 255 : (rageFlash ? 30 : 30);
        SDL_SetRenderDrawColor(renderer, rr, rg, rb, baseAlpha);
        SDL_RenderFillRectF(renderer, &hull);

        const SDL_FRect plateT{m_x - 110.0f, m_y - 42.0f, 220.0f, 12.0f};
        const SDL_FRect plateB{m_x - 100.0f, m_y + 30.0f, 200.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 85, hit ? 255 : 35, hit ? 255 : 35, baseAlpha);
        SDL_RenderFillRectF(renderer, &plateT);
        SDL_RenderFillRectF(renderer, &plateB);

        const SDL_FRect silo{m_x - 30.0f, m_y - 15.0f, 60.0f, 30.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 35, hit ? 255 : 20, hit ? 255 : 20, baseAlpha);
        SDL_RenderFillRectF(renderer, &silo);

        const SDL_FRect starCore{m_x - 10.0f, m_y - 10.0f, 20.0f, 20.0f};
        SDL_SetRenderDrawColor(renderer, 240, 40, 40, baseAlpha);
        SDL_RenderFillRectF(renderer, &starCore);

        const SDL_FRect flak1{m_x - 90.0f, m_y + 18.0f, 20.0f, 10.0f};
        const SDL_FRect flak2{m_x + 70.0f, m_y + 18.0f, 20.0f, 10.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 45, hit ? 255 : 25, hit ? 255 : 25, baseAlpha);
        SDL_RenderFillRectF(renderer, &flak1);
        SDL_RenderFillRectF(renderer, &flak2);
    }
    // -------------------------------------------------------------------------
    // 6. BOSS 6: JUNGLE ANACONDA (SIERPE AÉREA ARTICULADA)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::JungleAnaconda) {
        // A. 4 Segmentos del cuerpo articulado oscilante
        const float dirSign = (m_vx >= 0.0f) ? -1.0f : 1.0f;
        for (int i = 4; i >= 1; --i) {
            const float segX = m_x + dirSign * (static_cast<float>(i) * 44.0f);
            const float segY = m_y + std::sin(m_animTimer * 3.6f + static_cast<float>(i) * 0.85f) * 32.0f;

            // Cuerpo del segmento
            const SDL_FRect segRect{segX - 20.0f, segY - 15.0f, 40.0f, 30.0f};
            Uint8 sr = hit ? 255 : (rageFlash ? 200 : static_cast<Uint8>(35 + i * 6));
            Uint8 sg = hit ? 255 : (rageFlash ? 60 : static_cast<Uint8>(95 + i * 5));
            Uint8 sb = hit ? 255 : (rageFlash ? 25 : 45);
            SDL_SetRenderDrawColor(renderer, sr, sg, sb, baseAlpha);
            SDL_RenderFillRectF(renderer, &segRect);

            // Escama dorsal de bronce
            const SDL_FRect dorsalScale{segX - 10.0f, segY - 20.0f, 20.0f, 10.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 175, hit ? 255 : 125, hit ? 255 : 45, baseAlpha);
            SDL_RenderFillRectF(renderer, &dorsalScale);

            // Núcleo energético ventral ámbar
            const SDL_FRect ventralCore{segX - 5.0f, segY - 3.0f, 10.0f, 8.0f};
            SDL_SetRenderDrawColor(renderer, 255, 185, 30, baseAlpha);
            SDL_RenderFillRectF(renderer, &ventralCore);
        }

        // B. Cabeza acorazada reptiliana
        const SDL_FRect headBox{m_x - 32.0f, m_y - 20.0f, 64.0f, 40.0f};
        Uint8 hr = hit ? 255 : (rageFlash ? 220 : 45);
        Uint8 hg = hit ? 255 : (rageFlash ? 50 : 115);
        Uint8 hb = hit ? 255 : (rageFlash ? 25 : 40);
        SDL_SetRenderDrawColor(renderer, hr, hg, hb, baseAlpha);
        SDL_RenderFillRectF(renderer, &headBox);

        // Morro triangular sólido
        const float faceDir = (m_vx >= 0.0f) ? 1.0f : -1.0f;
        const SDL_FRect snout{m_x + (faceDir > 0.0f ? 28.0f : -42.0f), m_y - 12.0f, 14.0f, 24.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 160, hit ? 255 : 115, hit ? 255 : 40, baseAlpha);
        SDL_RenderFillRectF(renderer, &snout);

        // Sensores oculares rojos
        const SDL_FRect eyeT{m_x + faceDir * 12.0f, m_y - 12.0f, 6.0f, 6.0f};
        const SDL_FRect eyeB{m_x + faceDir * 12.0f, m_y + 6.0f, 6.0f, 6.0f};
        SDL_SetRenderDrawColor(renderer, 255, 35, 35, baseAlpha);
        SDL_RenderFillRectF(renderer, &eyeT);
        SDL_RenderFillRectF(renderer, &eyeB);
    }
    // -------------------------------------------------------------------------
    // 7. BOSS 7: CYBER LEVIATHAN (CRUCERO SIGILOSO NEÓN)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::CyberLeviathan) {
        // Fuselaje angular negro obsidiana
        const SDL_FRect mainHull{m_x - 95.0f, m_y - 22.0f, 190.0f, 44.0f};
        Uint8 br = hit ? 255 : (rageFlash ? 180 : 18);
        Uint8 bg = hit ? 255 : (rageFlash ? 35 : 20);
        Uint8 bb = hit ? 255 : (rageFlash ? 30 : 26);
        SDL_SetRenderDrawColor(renderer, br, bg, bb, baseAlpha);
        SDL_RenderFillRectF(renderer, &mainHull);

        const SDL_FRect wingSwept{m_x - 115.0f, m_y - 8.0f, 230.0f, 20.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 28, hit ? 255 : 32, hit ? 255 : 40, baseAlpha);
        SDL_RenderFillRectF(renderer, &wingSwept);

        // Ribetes brillantes neón cian y magenta (anchura 4px)
        const SDL_FRect trimCyanL{m_x - 110.0f, m_y + 12.0f, 80.0f, 4.0f};
        const SDL_FRect trimCyanR{m_x + 30.0f, m_y + 12.0f, 80.0f, 4.0f};
        SDL_SetRenderDrawColor(renderer, 0, 245, 255, baseAlpha);
        SDL_RenderFillRectF(renderer, &trimCyanL);
        SDL_RenderFillRectF(renderer, &trimCyanR);

        const SDL_FRect trimMag{m_x - 65.0f, m_y - 24.0f, 130.0f, 4.0f};
        SDL_SetRenderDrawColor(renderer, 255, 30, 185, baseAlpha);
        SDL_RenderFillRectF(renderer, &trimMag);

        // Núcleos dobles de energía pulsantes
        const Uint8 pulseVal = static_cast<Uint8>(180 + 75 * std::abs(std::sin(m_animTimer * 10.0f)));
        const SDL_FRect coreL{m_x - 55.0f, m_y - 8.0f, 20.0f, 16.0f};
        const SDL_FRect coreR{m_x + 35.0f, m_y - 8.0f, 20.0f, 16.0f};
        SDL_SetRenderDrawColor(renderer, 0, 255, pulseVal, baseAlpha);
        SDL_RenderFillRectF(renderer, &coreL);
        SDL_RenderFillRectF(renderer, &coreR);

        // Propulsores de plasma de alta velocidad
        const SDL_FRect thrusterL{m_x - 40.0f, m_y - 28.0f, 14.0f, 6.0f};
        const SDL_FRect thrusterR{m_x + 26.0f, m_y - 28.0f, 14.0f, 6.0f};
        SDL_SetRenderDrawColor(renderer, 0, 210, 255, 240);
        SDL_RenderFillRectF(renderer, &thrusterL);
        SDL_RenderFillRectF(renderer, &thrusterR);
    }
    // -------------------------------------------------------------------------
    // 8. BOSS 8: KRAKEN DREADNOUGHT (EKRANOPLANO COLOSAL NAVAL)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::KrakenDreadnought) {
        // Casco naval acorazado masivo
        const SDL_FRect hull{m_x - 140.0f, m_y - 24.0f, 280.0f, 54.0f};
        Uint8 kr = hit ? 255 : (rageFlash ? 200 : 70);
        Uint8 kg = hit ? 255 : (rageFlash ? 50 : 76);
        Uint8 kb = hit ? 255 : (rageFlash ? 30 : 88);
        SDL_SetRenderDrawColor(renderer, kr, kg, kb, baseAlpha);
        SDL_RenderFillRectF(renderer, &hull);

        const SDL_FRect skirt{m_x - 125.0f, m_y + 24.0f, 250.0f, 16.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 46, hit ? 255 : 52, hit ? 255 : 62, baseAlpha);
        SDL_RenderFillRectF(renderer, &skirt);

        // 6 Turbinas frontales con llamarada
        for (int i = 0; i < 6; ++i) {
            const float tx = m_x - 110.0f + static_cast<float>(i) * 44.0f;
            const SDL_FRect enginePod{tx - 9.0f, m_y - 36.0f, 18.0f, 18.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 52, hit ? 255 : 56, hit ? 255 : 65, baseAlpha);
            SDL_RenderFillRectF(renderer, &enginePod);

            const SDL_FRect flame{tx - 5.0f, m_y - 44.0f, 10.0f, 8.0f};
            SDL_SetRenderDrawColor(renderer, 255, 135, 25, 240);
            SDL_RenderFillRectF(renderer, &flame);
        }

        // 3 Torretas navales (izq, centro, der)
        const SDL_FRect tBase1{m_x - 100.0f, m_y + 6.0f, 26.0f, 14.0f};
        const SDL_FRect tBase2{m_x - 13.0f,  m_y + 6.0f, 26.0f, 14.0f};
        const SDL_FRect tBase3{m_x + 74.0f,  m_y + 6.0f, 26.0f, 14.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 35, hit ? 255 : 40, hit ? 255 : 48, baseAlpha);
        SDL_RenderFillRectF(renderer, &tBase1);
        SDL_RenderFillRectF(renderer, &tBase2);
        SDL_RenderFillRectF(renderer, &tBase3);

        const SDL_FRect tGuns1{m_x - 96.0f, m_y + 20.0f, 18.0f, 12.0f};
        const SDL_FRect tGuns2{m_x - 9.0f,  m_y + 20.0f, 18.0f, 12.0f};
        const SDL_FRect tGuns3{m_x + 78.0f, m_y + 20.0f, 18.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 25, hit ? 255 : 28, hit ? 255 : 32, baseAlpha);
        SDL_RenderFillRectF(renderer, &tGuns1);
        SDL_RenderFillRectF(renderer, &tGuns2);
        SDL_RenderFillRectF(renderer, &tGuns3);

        // Puente de mando y torre de radar
        const SDL_FRect bridge{m_x - 25.0f, m_y - 18.0f, 50.0f, 22.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 38, hit ? 255 : 44, hit ? 255 : 54, baseAlpha);
        SDL_RenderFillRectF(renderer, &bridge);
    }
    // -------------------------------------------------------------------------
    // 9. BOSS 9: BIO-HAZARD CHIMERA (DOBLE ZEPELÍN QUÍMICO)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::BioHazardChimera) {
        // Cilindro izquierdo y derecho de zepelín
        const SDL_FRect cylL{m_x - 120.0f, m_y - 25.0f, 95.0f, 50.0f};
        const SDL_FRect cylR{m_x + 25.0f,  m_y - 25.0f, 95.0f, 50.0f};
        Uint8 zr = hit ? 255 : (rageFlash ? 200 : 75);
        Uint8 zg = hit ? 255 : (rageFlash ? 50 : 82);
        Uint8 zb = hit ? 255 : (rageFlash ? 25 : 70);
        SDL_SetRenderDrawColor(renderer, zr, zg, zb, baseAlpha);
        SDL_RenderFillRectF(renderer, &cylL);
        SDL_RenderFillRectF(renderer, &cylR);

        // Estructura central gantry reforzada
        const SDL_FRect bridge{m_x - 30.0f, m_y - 12.0f, 60.0f, 24.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 45, hit ? 255 : 48, hit ? 255 : 52, baseAlpha);
        SDL_RenderFillRectF(renderer, &bridge);

        // 4 Tanques químicos de ácido verde tóxico brillante
        const float vatPositions[4] = {m_x - 95.0f, m_x - 55.0f, m_x + 40.0f, m_x + 80.0f};
        for (float vx : vatPositions) {
            // Contenedor de vidrio templado
            const SDL_FRect vatFrame{vx - 10.0f, m_y - 10.0f, 20.0f, 24.0f};
            SDL_SetRenderDrawColor(renderer, 30, 45, 35, baseAlpha);
            SDL_RenderFillRectF(renderer, &vatFrame);

            // Ácido verde radiactivo pulsante
            const Uint8 acidGreen = static_cast<Uint8>(210 + 45 * std::abs(std::sin(m_animTimer * 8.0f + vx)));
            const SDL_FRect acidLiquid{vx - 7.0f, m_y - 7.0f, 14.0f, 18.0f};
            SDL_SetRenderDrawColor(renderer, 35, acidGreen, 60, baseAlpha);
            SDL_RenderFillRectF(renderer, &acidLiquid);
        }

        // Franjas de advertencia química amarilla en los morros
        const SDL_FRect warnL{m_x - 120.0f, m_y - 6.0f, 16.0f, 12.0f};
        const SDL_FRect warnR{m_x + 104.0f, m_y - 6.0f, 16.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, 255, 215, 30, baseAlpha);
        SDL_RenderFillRectF(renderer, &warnL);
        SDL_RenderFillRectF(renderer, &warnR);
    }
    // -------------------------------------------------------------------------
    // 10. BOSS 10: DOOMSDAY EXPRESS (TREN AÉREO NUCLEAR CON CAÑÓN DE RIEL)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::DoomsdayExpress) {
        // Haz de advertencia del Cañón de Riel (Railgun) durante la carga
        if (m_railgunCharging) {
            const float pulse = std::abs(std::sin(m_animTimer * 20.0f));
            const Uint8 beamAlpha = static_cast<Uint8>(130.0f + pulse * 125.0f);
            const SDL_FRect targetingLine{m_railgunTargetX - 2.0f, m_y + 35.0f, 4.0f, 980.0f - (m_y + 35.0f)};
            SDL_SetRenderDrawColor(renderer, 255, 35, 35, beamAlpha);
            SDL_RenderFillRectF(renderer, &targetingLine);
        }

        // Disparo hipersónico del Cañón de Riel
        if (m_railgunFiring) {
            const SDL_FRect beamAura{m_railgunTargetX - 18.0f, m_y + 35.0f, 36.0f, 980.0f - (m_y + 35.0f)};
            SDL_SetRenderDrawColor(renderer, 255, 35, 35, 190);
            SDL_RenderFillRectF(renderer, &beamAura);

            const SDL_FRect beamCore{m_railgunTargetX - 7.0f, m_y + 35.0f, 14.0f, 980.0f - (m_y + 35.0f)};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRectF(renderer, &beamCore);
        }

        // Cuerpo blindado de la locomotora nuclear
        const SDL_FRect locoBody{m_x - 150.0f, m_y - 28.0f, 300.0f, 58.0f};
        Uint8 lr = hit ? 255 : (rageFlash ? 210 : 42);
        Uint8 lg = hit ? 255 : (rageFlash ? 40 : 40);
        Uint8 lb = hit ? 255 : (rageFlash ? 25 : 46);
        SDL_SetRenderDrawColor(renderer, lr, lg, lb, baseAlpha);
        SDL_RenderFillRectF(renderer, &locoBody);

        const SDL_FRect roofPlates{m_x - 130.0f, m_y - 38.0f, 260.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 62, hit ? 255 : 58, hit ? 255 : 66, baseAlpha);
        SDL_RenderFillRectF(renderer, &roofPlates);

        // 3 Chimeneas expulsando fuego y humo
        const float stackX[3] = {m_x - 85.0f, m_x, m_x + 85.0f};
        for (float sx : stackX) {
            const SDL_FRect stack{sx - 8.0f, m_y - 52.0f, 16.0f, 16.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 32, hit ? 255 : 30, hit ? 255 : 36, baseAlpha);
            SDL_RenderFillRectF(renderer, &stack);

            const SDL_FRect fire{sx - 5.0f, m_y - 62.0f, 10.0f, 10.0f};
            SDL_SetRenderDrawColor(renderer, 255, 120, 20, 240);
            SDL_RenderFillRectF(renderer, &fire);
        }

        // Cañón de Riel (Railgun) central en la proa
        const SDL_FRect railgunMount{m_x - 22.0f, m_y + 18.0f, 44.0f, 22.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 25, hit ? 255 : 25, hit ? 255 : 30, baseAlpha);
        SDL_RenderFillRectF(renderer, &railgunMount);

        // Bobinas de aceleración magnética rojas
        const SDL_FRect coil1{m_x - 18.0f, m_y + 24.0f, 6.0f, 12.0f};
        const SDL_FRect coil2{m_x - 6.0f,  m_y + 24.0f, 6.0f, 12.0f};
        const SDL_FRect coil3{m_x + 6.0f,  m_y + 24.0f, 6.0f, 12.0f};
        const SDL_FRect coil4{m_x + 18.0f, m_y + 24.0f, 6.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, 255, 40, 40, baseAlpha);
        SDL_RenderFillRectF(renderer, &coil1);
        SDL_RenderFillRectF(renderer, &coil2);
        SDL_RenderFillRectF(renderer, &coil3);
        SDL_RenderFillRectF(renderer, &coil4);

        // Silos nucleares con señal de peligro radiactivo
        const SDL_FRect siloL{m_x - 130.0f, m_y - 15.0f, 32.0f, 20.0f};
        const SDL_FRect siloR{m_x + 98.0f,  m_y - 15.0f, 32.0f, 20.0f};
        SDL_SetRenderDrawColor(renderer, 255, 215, 30, baseAlpha);
        SDL_RenderFillRectF(renderer, &siloL);
        SDL_RenderFillRectF(renderer, &siloR);
    }
    // -------------------------------------------------------------------------
    // 11. BOSS 11: VULCAN FORGE-MASTER (EXCAVADORA TÉRMICA Y CRISOL DE MAGMA)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::VulcanForgeMaster) {
        // Chorro vertical de magma / fuego térmico descendente
        if (m_flameJetActive) {
            const float groundY = 980.0f;
            const float jetH = groundY - (m_y + 35.0f);

            // Resplandor exterior anaranjado
            const SDL_FRect aura{m_flameJetX - 22.0f, m_y + 35.0f, 44.0f, jetH};
            SDL_SetRenderDrawColor(renderer, 255, 65, 10, 160);
            SDL_RenderFillRectF(renderer, &aura);

            // Núcleo de magma ardiente
            const SDL_FRect core{m_flameJetX - 11.0f, m_y + 35.0f, 22.0f, jetH};
            SDL_SetRenderDrawColor(renderer, 255, 200, 40, 230);
            SDL_RenderFillRectF(renderer, &core);

            // Rayo de plasma blanco central
            const SDL_FRect plasma{m_flameJetX - 4.0f, m_y + 35.0f, 8.0f, jetH};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRectF(renderer, &plasma);
        }

        // Chasis principal de aleación refractaria
        const SDL_FRect hull{m_x - 120.0f, m_y - 25.0f, 240.0f, 52.0f};
        Uint8 vr = hit ? 255 : (rageFlash ? 210 : 60);
        Uint8 vg = hit ? 255 : (rageFlash ? 40 : 36);
        Uint8 vb = hit ? 255 : (rageFlash ? 20 : 32);
        SDL_SetRenderDrawColor(renderer, vr, vg, vb, baseAlpha);
        SDL_RenderFillRectF(renderer, &hull);

        // Blindaje superior con disipadores de calor
        const SDL_FRect topArmor{m_x - 95.0f, m_y - 36.0f, 190.0f, 12.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 45, hit ? 255 : 30, hit ? 255 : 28, baseAlpha);
        SDL_RenderFillRectF(renderer, &topArmor);

        // Crisol central de metal fundido con magma pulsante
        const SDL_FRect crucible{m_x - 38.0f, m_y - 12.0f, 76.0f, 36.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 30, hit ? 255 : 22, hit ? 255 : 20, baseAlpha);
        SDL_RenderFillRectF(renderer, &crucible);

        const Uint8 magmaG = static_cast<Uint8>(120.0f + 65.0f * std::abs(std::sin(m_animTimer * 9.0f)));
        const SDL_FRect magmaPool{m_x - 32.0f, m_y - 6.0f, 64.0f, 26.0f};
        SDL_SetRenderDrawColor(renderer, 255, magmaG, 20, baseAlpha);
        SDL_RenderFillRectF(renderer, &magmaPool);

        // Dos Rotores Trituradores laterales incandescentes
        const float rotorAngle = m_animTimer * (enraged ? 18.0f : 12.0f);
        const float cR = std::cos(rotorAngle);
        const float sR = std::sin(rotorAngle);

        const float rotorCenters[2] = {m_x - 110.0f, m_x + 110.0f};
        for (float rc : rotorCenters) {
            // Núcleo del rotor
            const SDL_FRect rHub{rc - 12.0f, m_y - 12.0f, 24.0f, 24.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 20, hit ? 255 : 20, hit ? 255 : 25, baseAlpha);
            SDL_RenderFillRectF(renderer, &rHub);

            // Dientes de trituración giratorios (aspas en cruz)
            for (int k = 0; k < 4; ++k) {
                const float a = static_cast<float>(k) * 1.5708f;
                const float dx = cR * std::cos(a) - sR * std::sin(a);
                const float dy = cR * std::sin(a) + sR * std::cos(a);
                const SDL_FRect tooth{rc + dx * 16.0f - 4.0f, m_y + dy * 16.0f - 4.0f, 8.0f, 8.0f};
                SDL_SetRenderDrawColor(renderer, 255, 140, 30, baseAlpha);
                SDL_RenderFillRectF(renderer, &tooth);
            }
        }
    }
    // -------------------------------------------------------------------------
    // 12. BOSS 12: PRISM OVERLORD (OCTAEDRO CRISTALINO Y PRISMAS ORBITALES)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::PrismOverlord) {
        // Enlaces de energía entre el núcleo y los 4 prismas orbitales
        for (int i = 0; i < 4; ++i) {
            const float angle = m_animTimer * 2.5f + static_cast<float>(i) * 1.5708f;
            const float nx = m_x + std::cos(angle) * 95.0f;
            const float ny = m_y + std::sin(angle) * 40.0f;

            // Haz de conexión sólido
            const SDL_FRect beamLink{(m_x + nx) * 0.5f - 2.0f, (m_y + ny) * 0.5f - 2.0f, 4.0f, 4.0f};
            SDL_SetRenderDrawColor(renderer, 0, 240, 255, 160);
            SDL_RenderFillRectF(renderer, &beamLink);

            // Nodo prisma orbital (diamante sólido)
            const SDL_FRect pNode{nx - 10.0f, ny - 10.0f, 20.0f, 20.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 0, hit ? 255 : 210, hit ? 255 : 255, baseAlpha);
            SDL_RenderFillRectF(renderer, &pNode);

            const SDL_FRect pCore{nx - 5.0f, ny - 5.0f, 10.0f, 10.0f};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, baseAlpha);
            SDL_RenderFillRectF(renderer, &pCore);
        }

        // Núcleo cristalino tipo octaedro facetado con SDL_RenderGeometry
        const SDL_Vertex topVert{{m_x, m_y - 44.0f}, {hit ? (Uint8)255 : (Uint8)180, hit ? (Uint8)255 : (Uint8)250, 255, baseAlpha}, {0,0}};
        const SDL_Vertex botVert{{m_x, m_y + 44.0f}, {hit ? (Uint8)255 : (Uint8)100, hit ? (Uint8)255 : (Uint8)140, 255, baseAlpha}, {0,0}};
        const SDL_Vertex lftVert{{m_x - 75.0f, m_y},  {hit ? (Uint8)255 : (Uint8)0,   hit ? (Uint8)255 : (Uint8)220, 255, baseAlpha}, {0,0}};
        const SDL_Vertex rgtVert{{m_x + 75.0f, m_y},  {hit ? (Uint8)255 : (Uint8)220, hit ? (Uint8)255 : (Uint8)100, 255, baseAlpha}, {0,0}};
        const SDL_Vertex midVert{{m_x, m_y},          {255, 255, 255, baseAlpha}, {0,0}};

        // Triángulo 1: Top-Left-Center
        const SDL_Vertex tri1[3] = {topVert, lftVert, midVert};
        SDL_RenderGeometry(renderer, nullptr, tri1, 3, nullptr, 0);

        // Triángulo 2: Top-Right-Center
        const SDL_Vertex tri2[3] = {topVert, rgtVert, midVert};
        SDL_RenderGeometry(renderer, nullptr, tri2, 3, nullptr, 0);

        // Triángulo 3: Bot-Left-Center
        const SDL_Vertex tri3[3] = {botVert, lftVert, midVert};
        SDL_RenderGeometry(renderer, nullptr, tri3, 3, nullptr, 0);

        // Triángulo 4: Bot-Right-Center
        const SDL_Vertex tri4[3] = {botVert, rgtVert, midVert};
        SDL_RenderGeometry(renderer, nullptr, tri4, 3, nullptr, 0);

        // Destello interior del prisma
        const SDL_FRect prismSparkle{m_x - 12.0f, m_y - 12.0f, 24.0f, 24.0f};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 230);
        SDL_RenderFillRectF(renderer, &prismSparkle);
    }
    // -------------------------------------------------------------------------
    // 13. BOSS 13: STARBREAKER CARRIER (PORTAAVIONES NODRIZA ORBITAL TITÁN)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::StarbreakerCarrier) {
        // Fuselaje principal blanco aeroespacial
        const SDL_FRect hull{m_x - 145.0f, m_y - 24.0f, 290.0f, 48.0f};
        Uint8 cr = hit ? 255 : (rageFlash ? 210 : 205);
        Uint8 cg = hit ? 255 : (rageFlash ? 50 : 215);
        Uint8 cb = hit ? 255 : (rageFlash ? 30 : 225);
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, baseAlpha);
        SDL_RenderFillRectF(renderer, &hull);

        // Cubierta central de despegue / pista oscura con balizas
        const SDL_FRect flightDeck{m_x - 90.0f, m_y - 10.0f, 180.0f, 22.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 40, hit ? 255 : 48, hit ? 255 : 62, baseAlpha);
        SDL_RenderFillRectF(renderer, &flightDeck);

        const SDL_FRect deckLine{m_x - 85.0f, m_y, 170.0f, 3.0f};
        SDL_SetRenderDrawColor(renderer, 0, 240, 255, baseAlpha);
        SDL_RenderFillRectF(renderer, &deckLine);

        // Alas de geometría oblicua
        const SDL_FRect wingL{m_x - 145.0f, m_y - 36.0f, 55.0f, 14.0f};
        const SDL_FRect wingR{m_x + 90.0f,  m_y - 36.0f, 55.0f, 14.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 160, hit ? 255 : 175, hit ? 255 : 190, baseAlpha);
        SDL_RenderFillRectF(renderer, &wingL);
        SDL_RenderFillRectF(renderer, &wingR);

        // Puente de mando avanzado
        const SDL_FRect bridge{m_x - 22.0f, m_y - 36.0f, 44.0f, 14.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 55, hit ? 255 : 65, hit ? 255 : 85, baseAlpha);
        SDL_RenderFillRectF(renderer, &bridge);

        const SDL_FRect bridgeGlass{m_x - 15.0f, m_y - 32.0f, 30.0f, 6.0f};
        SDL_SetRenderDrawColor(renderer, 255, 205, 40, baseAlpha);
        SDL_RenderFillRectF(renderer, &bridgeGlass);

        // Cuatro motores de iones de plasma cian
        const float engX[4] = {m_x - 110.0f, m_x - 55.0f, m_x + 35.0f, m_x + 90.0f};
        for (float ex : engX) {
            const SDL_FRect pod{ex, m_y - 44.0f, 20.0f, 10.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 45, hit ? 255 : 52, hit ? 255 : 68, baseAlpha);
            SDL_RenderFillRectF(renderer, &pod);

            const SDL_FRect ionPlume{ex + 3.0f, m_y - 54.0f, 14.0f, 10.0f};
            SDL_SetRenderDrawColor(renderer, 0, 210, 255, 240);
            SDL_RenderFillRectF(renderer, &ionPlume);
        }
    }
    // -------------------------------------------------------------------------
    // 14. BOSS 14: TEMPEST ARCHON (GENERADOR ATMOSFÉRICO TESLA TOROIDAL)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::TempestArchon) {
        // Columna de advertencia de Relámpago Tesla
        if (m_lightningWarning) {
            const float pulse = std::abs(std::sin(m_animTimer * 25.0f));
            const Uint8 guideAlpha = static_cast<Uint8>(120.0f + pulse * 135.0f);
            const SDL_FRect guideLine{m_lightningTargetX - 3.0f, m_y + 35.0f, 6.0f, 980.0f - (m_y + 35.0f)};
            SDL_SetRenderDrawColor(renderer, 80, 220, 255, guideAlpha);
            SDL_RenderFillRectF(renderer, &guideLine);
        }

        // Descarga de Relámpago Vertical colosal
        if (m_lightningFiring) {
            const float groundY = 980.0f;
            const float boltH = groundY - (m_y + 35.0f);

            // Halo eléctrico exterior
            const SDL_FRect boltAura{m_lightningTargetX - 22.0f, m_y + 35.0f, 44.0f, boltH};
            SDL_SetRenderDrawColor(renderer, 0, 160, 255, 200);
            SDL_RenderFillRectF(renderer, &boltAura);

            // Núcleo blanco del relámpago
            const SDL_FRect boltCore{m_lightningTargetX - 9.0f, m_y + 35.0f, 18.0f, boltH};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRectF(renderer, &boltCore);
        }

        // Toroide / Plataforma anular grafito
        const SDL_FRect toroide{m_x - 120.0f, m_y - 22.0f, 240.0f, 46.0f};
        Uint8 tr = hit ? 255 : (rageFlash ? 210 : 42);
        Uint8 tg = hit ? 255 : (rageFlash ? 40 : 45);
        Uint8 tb = hit ? 255 : (rageFlash ? 25 : 55);
        SDL_SetRenderDrawColor(renderer, tr, tg, tb, baseAlpha);
        SDL_RenderFillRectF(renderer, &toroide);

        // Tres bobinas Tesla toroidales con arcos de plasma cian
        const float coilPos[3] = {m_x - 85.0f, m_x, m_x + 85.0f};
        for (float cx : coilPos) {
            // Soporte de bobina
            const SDL_FRect coilBase{cx - 16.0f, m_y - 32.0f, 32.0f, 16.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 60, hit ? 255 : 65, hit ? 255 : 75, baseAlpha);
            SDL_RenderFillRectF(renderer, &coilBase);

            // Esfera/toroide superior
            const SDL_FRect sphere{cx - 12.0f, m_y - 44.0f, 24.0f, 14.0f};
            SDL_SetRenderDrawColor(renderer, 0, 240, 255, baseAlpha);
            SDL_RenderFillRectF(renderer, &sphere);

            // Núcleo blanco de descarga
            const SDL_FRect sparkCore{cx - 5.0f, m_y - 40.0f, 10.0f, 6.0f};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRectF(renderer, &sparkCore);
        }

        // Arcos voltaicos horizontales entre bobinas
        const SDL_FRect arc1{m_x - 70.0f, m_y - 38.0f, 55.0f, 3.0f};
        const SDL_FRect arc2{m_x + 15.0f, m_y - 38.0f, 55.0f, 3.0f};
        SDL_SetRenderDrawColor(renderer, 150, 240, 255, 220);
        SDL_RenderFillRectF(renderer, &arc1);
        SDL_RenderFillRectF(renderer, &arc2);
    }
    // -------------------------------------------------------------------------
    // 15. BOSS 15: AEGIS ORBITAL CORE (ESTACIÓN NUCLEAR CON LLUVIA ICBM)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::AegisOrbitalCore) {
        // Enormes alas de Paneles Solares Orbitales Dorados
        const SDL_FRect panelL{m_x - 160.0f, m_y - 20.0f, 75.0f, 42.0f};
        const SDL_FRect panelR{m_x + 85.0f,  m_y - 20.0f, 75.0f, 42.0f};
        SDL_SetRenderDrawColor(renderer, 240, 195, 45, baseAlpha);
        SDL_RenderFillRectF(renderer, &panelL);
        SDL_RenderFillRectF(renderer, &panelR);

        // Celosía oscura de los paneles fotovoltaicos
        SDL_SetRenderDrawColor(renderer, 30, 28, 25, baseAlpha);
        for (int p = 0; p < 3; ++p) {
            const SDL_FRect gridL{m_x - 155.0f + static_cast<float>(p) * 24.0f, m_y - 20.0f, 3.0f, 42.0f};
            const SDL_FRect gridR{m_x + 90.0f  + static_cast<float>(p) * 24.0f, m_y - 20.0f, 3.0f, 42.0f};
            SDL_RenderFillRectF(renderer, &gridL);
            SDL_RenderFillRectF(renderer, &gridR);
        }

        // Núcleo hexagonal acorazado central
        const SDL_FRect coreHull{m_x - 82.0f, m_y - 35.0f, 164.0f, 70.0f};
        Uint8 ar = hit ? 255 : (rageFlash ? 210 : 50);
        Uint8 ag = hit ? 255 : (rageFlash ? 40 : 52);
        Uint8 ab = hit ? 255 : (rageFlash ? 25 : 62);
        SDL_SetRenderDrawColor(renderer, ar, ag, ab, baseAlpha);
        SDL_RenderFillRectF(renderer, &coreHull);

        // Reactor nuclear central con resplandor pulsante
        const SDL_FRect reactorFrame{m_x - 30.0f, m_y - 14.0f, 60.0f, 28.0f};
        SDL_SetRenderDrawColor(renderer, hit ? 255 : 22, hit ? 255 : 24, hit ? 255 : 28, baseAlpha);
        SDL_RenderFillRectF(renderer, &reactorFrame);

        const Uint8 reactPulse = static_cast<Uint8>(180.0f + 75.0f * std::abs(std::sin(m_animTimer * 8.0f)));
        const SDL_FRect reactorCore{m_x - 22.0f, m_y - 8.0f, 44.0f, 16.0f};
        SDL_SetRenderDrawColor(renderer, reactPulse, 40, 40, baseAlpha);
        SDL_RenderFillRectF(renderer, &reactorCore);

        // 3 Compuertas de Silos Nucleares de Misiles ICBM en la parte superior
        const float siloX[3] = {m_x - 55.0f, m_x - 10.0f, m_x + 35.0f};
        for (float sx : siloX) {
            const SDL_FRect siloHatch{sx, m_y - 44.0f, 20.0f, 10.0f};
            SDL_SetRenderDrawColor(renderer, 255, 215, 30, baseAlpha);
            SDL_RenderFillRectF(renderer, &siloHatch);

            // Cabeza del misil ICBM asomándose
            const SDL_FRect icbmNose{sx + 4.0f, m_y - 50.0f, 12.0f, 6.0f};
            SDL_SetRenderDrawColor(renderer, 220, 45, 45, baseAlpha);
            SDL_RenderFillRectF(renderer, &icbmNose);
        }

        // Torretas cuádruples de penetración en la panza
        const float tX[4] = {m_x - 70.0f, m_x - 25.0f, m_x + 15.0f, m_x + 60.0f};
        for (float tx : tX) {
            const SDL_FRect barrel{tx, m_y + 35.0f, 8.0f, 14.0f};
            SDL_SetRenderDrawColor(renderer, hit ? 255 : 30, hit ? 255 : 35, hit ? 255 : 40, baseAlpha);
            SDL_RenderFillRectF(renderer, &barrel);
        }
    }
    // -------------------------------------------------------------------------
    // 16. BOSS 25: OMEGA DOOMSDAY TITAN (TITÁN SUPREMO FINAL)
    // -------------------------------------------------------------------------
    else if (m_type == BossType::OmegaDoomsdayTitan) {
        // 1. Reactor Nuclear Central Pulsante
        const float pulse = (std::sin(m_animTimer * 8.0f) + 1.0f) * 0.5f;
        const float auraW = 74.0f + pulse * 22.0f;
        const float auraH = 38.0f + pulse * 12.0f;
        const SDL_FRect reactorAura{m_x - auraW * 0.5f, m_y - auraH * 0.5f, auraW, auraH};
        SDL_SetRenderDrawColor(renderer, 0, 240, 210, static_cast<Uint8>(120 + pulse * 100.0f));
        SDL_RenderFillRectF(renderer, &reactorAura);

        const SDL_FRect reactorCore{m_x - 18.0f, m_y - 10.0f, 36.0f, 20.0f};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 250);
        SDL_RenderFillRectF(renderer, &reactorCore);

        // 2. Módulos / Torretas Orbitales Flotantes a los Flancos
        const float leftModX = m_x - 165.0f;
        const float rightModX = m_x + 130.0f;
        const float modY = m_y - 12.0f + std::sin(m_animTimer * 4.0f) * 6.0f;

        const SDL_FRect modL{leftModX, modY, 35.0f, 24.0f};
        const SDL_FRect modR{rightModX, modY, 35.0f, 24.0f};
        SDL_SetRenderDrawColor(renderer, 45, 50, 60, baseAlpha);
        SDL_RenderFillRectF(renderer, &modL);
        SDL_RenderFillRectF(renderer, &modR);

        const SDL_FRect pL{leftModX + 8.0f, modY + 5.0f, 19.0f, 14.0f};
        const SDL_FRect pR{rightModX + 8.0f, modY + 5.0f, 19.0f, 14.0f};
        SDL_SetRenderDrawColor(renderer, 255, 120, 30, 240);
        SDL_RenderFillRectF(renderer, &pL);
        SDL_RenderFillRectF(renderer, &pR);

        // 3. Disparo o Guía de Cañones Láser Duales
        if (m_omegaLaserWarning) {
            const SDL_FRect warn1{m_omegaLaserTargetX1 - 2.0f, m_y + 20.0f, 4.0f, 980.0f - (m_y + 20.0f)};
            const SDL_FRect warn2{m_omegaLaserTargetX2 - 2.0f, m_y + 20.0f, 4.0f, 980.0f - (m_y + 20.0f)};
            SDL_SetRenderDrawColor(renderer, 255, 40, 40, 160);
            SDL_RenderFillRectF(renderer, &warn1);
            SDL_RenderFillRectF(renderer, &warn2);
        } else if (m_omegaLaserFiring) {
            // Columnas dobles de plasma devastador
            const SDL_FRect beamA1{m_omegaLaserTargetX1 - 16.0f, m_y + 20.0f, 32.0f, 980.0f - (m_y + 20.0f)};
            const SDL_FRect beamA2{m_omegaLaserTargetX2 - 16.0f, m_y + 20.0f, 32.0f, 980.0f - (m_y + 20.0f)};
            SDL_SetRenderDrawColor(renderer, 255, 50, 30, 150);
            SDL_RenderFillRectF(renderer, &beamA1);
            SDL_RenderFillRectF(renderer, &beamA2);

            const SDL_FRect beamC1{m_omegaLaserTargetX1 - 6.0f, m_y + 20.0f, 12.0f, 980.0f - (m_y + 20.0f)};
            const SDL_FRect beamC2{m_omegaLaserTargetX2 - 6.0f, m_y + 20.0f, 12.0f, 980.0f - (m_y + 20.0f)};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 250);
            SDL_RenderFillRectF(renderer, &beamC1);
            SDL_RenderFillRectF(renderer, &beamC2);
        }
    }
}

void Boss::renderHealthBar(SDL_Renderer* renderer) const {
    if (!m_active || !renderer) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const float barX = 580.0f;
    const float barY = 96.0f;
    const float barW = 760.0f;
    const float barH = 22.0f;

    const bool enraged = isEnraged();

    // Fondo del marco de Boss
    const SDL_FRect bgRect{barX - 44.0f, barY - 4.0f, barW + 88.0f, barH + 8.0f};
    SDL_SetRenderDrawColor(renderer, 25, 20, 25, 245);
    SDL_RenderFillRectF(renderer, &bgRect);

    // Borde carmesí o dorado si está encolerizado
    const SDL_FRect borderT{barX - 44.0f, barY - 4.0f, barW + 88.0f, 2.0f};
    const SDL_FRect borderB{barX - 44.0f, barY + barH + 2.0f, barW + 88.0f, 2.0f};
    if (enraged) {
        SDL_SetRenderDrawColor(renderer, 255, 120, 20, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
    }
    SDL_RenderFillRectF(renderer, &borderT);
    SDL_RenderFillRectF(renderer, &borderB);

    // Barra de fondo vacía
    const SDL_FRect emptyBar{barX, barY, barW, barH};
    SDL_SetRenderDrawColor(renderer, 45, 25, 30, 255);
    SDL_RenderFillRectF(renderer, &emptyBar);

    // Relleno de vida activa
    const float ratio = std::clamp(static_cast<float>(m_hp) / static_cast<float>(m_maxHp), 0.0f, 1.0f);
    const SDL_FRect fillBar{barX, barY, barW * ratio, barH};

    const bool hit = (m_hitFlashTimer > 0.0f);
    if (hit) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    } else if (m_type == BossType::OmegaDoomsdayTitan) {
        if (ratio <= 0.30f) {
            // Fase 3 Apocalypse Enrage: oro y rojo fuego
            const float pulse = (std::sin(m_animTimer * 10.0f) + 1.0f) * 0.5f;
            SDL_SetRenderDrawColor(renderer, 255, static_cast<Uint8>(50.0f + pulse * 170.0f), 20, 255);
        } else if (ratio <= 0.65f) {
            SDL_SetRenderDrawColor(renderer, 255, 140, 20, 255); // Naranja Fase 2
        } else {
            SDL_SetRenderDrawColor(renderer, 235, 45, 45, 255);  // Rojo Fase 1
        }
    } else if (enraged) {
        SDL_SetRenderDrawColor(renderer, 255, 80, 20, 255); // Naranja fuego enrage
    } else {
        SDL_SetRenderDrawColor(renderer, 235, 45, 45, 255);
    }
    SDL_RenderFillRectF(renderer, &fillBar);

    // Divisiones tácticas: para Omega Doomsday Titan marcas doradas en 65% y 30%
    if (m_type == BossType::OmegaDoomsdayTitan) {
        const SDL_FRect tick65{barX + barW * 0.65f - 1.0f, barY - 2.0f, 3.0f, barH + 4.0f};
        const SDL_FRect tick30{barX + barW * 0.30f - 1.0f, barY - 2.0f, 3.0f, barH + 4.0f};
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Oro para separador de fases
        SDL_RenderFillRectF(renderer, &tick65);
        SDL_RenderFillRectF(renderer, &tick30);
    } else {
        SDL_SetRenderDrawColor(renderer, 20, 15, 20, 200);
        for (int i = 1; i < 10; ++i) {
            const SDL_FRect tick{barX + static_cast<float>(i) * (barW / 10.0f), barY, 2.0f, barH};
            SDL_RenderFillRectF(renderer, &tick);
        }
    }

    // Indicador táctico ENRAGED a la derecha si aplica
    if (enraged) {
        const SDL_FRect rageBadge{barX + barW + 6.0f, barY, 32.0f, barH};
        SDL_SetRenderDrawColor(renderer, 255, 60, 0, 240);
        SDL_RenderFillRectF(renderer, &rageBadge);
    }

    // Icono de cráneo de advertencia al inicio
    const SDL_FRect skull{barX - 34.0f, barY + 1.0f, 20.0f, 20.0f};
    SDL_SetRenderDrawColor(renderer, enraged ? 255 : 240, enraged ? 140 : 50, 40, 255);
    SDL_RenderFillRectF(renderer, &skull);

    const SDL_FRect eye1{barX - 30.0f, barY + 5.0f, 4.0f, 4.0f};
    const SDL_FRect eye2{barX - 22.0f, barY + 5.0f, 4.0f, 4.0f};
    SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
    SDL_RenderFillRectF(renderer, &eye1);
    SDL_RenderFillRectF(renderer, &eye2);
}

} // namespace Entities
