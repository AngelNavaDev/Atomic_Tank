#include "entities/EnemyPool.hpp"
#include "entities/ParticlePool.hpp"
#include "core/AssetManager.hpp"
#include "core/AudioManager.hpp"
#include <cmath>
#include <algorithm>

namespace Entities {

EnemyPool::EnemyPool() {
    clear();
}

float EnemyPool::nextRandomFloat() noexcept {
    m_rngState = m_rngState * 1664525u + 1013904223u;
    return static_cast<float>(m_rngState & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
}

void EnemyPool::clear() noexcept {
    for (auto& enemy : m_enemies) {
        enemy.active = false;
        enemy.x = 0.0f;
        enemy.y = 0.0f;
        enemy.startY = 0.0f;
        enemy.vx = 0.0f;
        enemy.vy = 0.0f;
        enemy.width = 75.0f;
        enemy.height = 28.0f;
        enemy.hp = 0;
        enemy.maxHp = 0;
        enemy.direction = 1;
        enemy.hitFlashTimer = 0.0f;
        enemy.attackTimer = 0.0f;
        enemy.secondaryTimer = 0.0f;
        enemy.animTimer = 0.0f;
        enemy.hasDroppedBomb = false;
        enemy.divePullingUp = false;
        enemy.stealthAlpha = 255.0f;
        enemy.laserTimer = 0.0f;
        enemy.isLaserFiring = false;
    }
    for (auto& proj : m_projectiles) {
        proj.active = false;
        proj.x = 0.0f;
        proj.y = 0.0f;
        proj.vx = 0.0f;
        proj.vy = 0.0f;
        proj.radius = 8.0f;
        proj.gravity = 0.0f;
        proj.isMissile = false;
        proj.isMine = false;
        proj.isClusterBomb = false;
        proj.splitTimer = 0.0f;
        proj.lifeTimer = 0.0f;
        proj.isBossProjectile = false;
        proj.damage = 10;
    }
    for (auto& icbm : m_icbms) {
        icbm.active = false;
        icbm.isDescending = false;
        icbm.warningTimer = 0.0f;
        icbm.hitFlashTimer = 0.0f;
        icbm.hp = 40;
        icbm.maxHp = 40;
    }

    m_nextEnemyIndex = 0;
    m_nextProjectileIndex = 0;
    m_spawnTimer = 0.5f;
    m_icbmCooldown = 8.0f;
    m_enemySpawnCounter = 0;
}

uint64_t EnemyPool::calculateKillScore(EnemyType type, int level) noexcept {
    uint64_t baseScore = 200;
    switch (type) {
    case EnemyType::ScoutPlane:     baseScore = 150; break;
    case EnemyType::AttackHeli:     baseScore = 250; break;
    case EnemyType::HeavyBomber:    baseScore = 400; break;
    case EnemyType::DiveFighter:    baseScore = 250; break;
    case EnemyType::StealthJet:     baseScore = 350; break;
    case EnemyType::Gunship:        baseScore = 450; break;
    case EnemyType::DroneSwarm:     baseScore = 150; break;
    case EnemyType::MineDropper:    baseScore = 300; break;
    case EnemyType::LaserSatellite: baseScore = 500; break;
    case EnemyType::ArmoredZeppelin:baseScore = 700; break;
    }

    const float levelMultiplier = 1.0f + 0.12f * static_cast<float>(std::max(1, level) - 1);
    const uint64_t scaledScore = static_cast<uint64_t>(static_cast<float>(baseScore) * levelMultiplier);

    // Tope estricto de 1,500 pts por avión en dificultad máxima
    return std::min(scaledScore, 1500ULL);
}

void EnemyPool::spawnEnemy(EnemyType type, int direction, float startY, int currentLevel) {
    size_t chosenIdx = MAX_ENEMIES;
    for (size_t i = 0; i < MAX_ENEMIES; ++i) {
        const size_t idx = (m_nextEnemyIndex + i) % MAX_ENEMIES;
        if (!m_enemies[idx].active) {
            chosenIdx = idx;
            m_nextEnemyIndex = (idx + 1) % MAX_ENEMIES;
            break;
        }
    }

    if (chosenIdx == MAX_ENEMIES) {
        return; // Pool lleno
    }

    auto& e = m_enemies[chosenIdx];
    e.type = type;
    e.active = true;
    e.direction = (direction >= 0) ? 1 : -1;
    e.hitFlashTimer = 0.0f;
    e.attackTimer = 0.0f;
    e.secondaryTimer = 0.0f;
    e.animTimer = 0.0f;
    e.hasDroppedBomb = false;
    e.divePullingUp = false;
    e.stealthAlpha = 255.0f;
    e.laserTimer = 0.0f;
    e.isLaserFiring = false;
    e.startY = startY;
    e.y = startY;

    const float dir = static_cast<float>(e.direction);

    // Multiplicador de dificultad progresivo (Niveles 1 al 24)
    const float hpScale = 0.85f + (static_cast<float>(std::clamp(currentLevel, 1, 24)) * 0.04f);

    switch (type) {
    case EnemyType::ScoutPlane:
        e.width = 86.0f;
        e.height = 32.0f;
        e.hp = std::max(18, static_cast<int>(26.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 380.0f;
        e.vy = 0.0f;
        e.x = (dir > 0.0f) ? -90.0f : 2010.0f;
        break;

    case EnemyType::AttackHeli:
        e.width = 98.0f;
        e.height = 46.0f;
        e.hp = std::max(50, static_cast<int>(75.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 240.0f; // Avance continuo sin detenerse jamás
        e.vy = 0.0f;
        e.x = (dir > 0.0f) ? -105.0f : 2025.0f;
        break;

    case EnemyType::HeavyBomber:
        e.width = 150.0f;
        e.height = 53.0f;
        e.hp = std::max(95, static_cast<int>(145.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 170.0f;
        e.vy = 0.0f;
        e.x = (dir > 0.0f) ? -155.0f : 2075.0f;
        break;

    case EnemyType::DiveFighter:
        e.width = 83.0f;
        e.height = 30.0f;
        e.hp = std::max(24, static_cast<int>(36.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 320.0f;
        e.vy = 120.0f;
        e.x = (dir > 0.0f) ? -88.0f : 2008.0f;
        break;

    case EnemyType::StealthJet:
        e.width = 92.0f;
        e.height = 32.0f;
        e.hp = std::max(28, static_cast<int>(40.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 420.0f;
        e.vy = 0.0f;
        e.x = (dir > 0.0f) ? -95.0f : 2015.0f;
        break;

    case EnemyType::Gunship:
        e.width = 132.0f;
        e.height = 48.0f;
        e.hp = std::max(85, static_cast<int>(130.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 160.0f;
        e.vy = 0.0f;
        e.x = (dir > 0.0f) ? -135.0f : 2055.0f;
        break;

    case EnemyType::DroneSwarm:
        e.width = 46.0f;
        e.height = 21.0f;
        e.hp = std::max(16, static_cast<int>(24.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 340.0f;
        e.vy = 0.0f;
        e.x = (dir > 0.0f) ? -52.0f : 1972.0f;
        break;

    case EnemyType::MineDropper:
        e.width = 104.0f;
        e.height = 41.0f;
        e.hp = std::max(45, static_cast<int>(65.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 180.0f;
        e.vy = 0.0f;
        e.x = (dir > 0.0f) ? -110.0f : 2030.0f;
        break;

    case EnemyType::LaserSatellite:
        e.width = 109.0f;
        e.height = 37.0f;
        e.hp = std::max(55, static_cast<int>(80.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 140.0f;
        e.vy = 0.0f;
        e.y = 100.0f; // Vuelo en la estratosfera alta
        e.x = (dir > 0.0f) ? -115.0f : 2035.0f;
        break;

    case EnemyType::ArmoredZeppelin:
        e.width = 218.0f;
        e.height = 67.0f;
        e.hp = std::max(120, static_cast<int>(175.0f * hpScale));
        e.maxHp = e.hp;
        e.vx = dir * 130.0f;
        e.vy = 0.0f;
        e.x = (dir > 0.0f) ? -225.0f : 2145.0f;
        break;
    }
}

bool EnemyPool::spawnBomb(float x, float y, float vx, float vy, float gravity, bool isBoss, int damage) {
    for (size_t i = 0; i < MAX_PROJECTILES; ++i) {
        const size_t idx = (m_nextProjectileIndex + i) % MAX_PROJECTILES;
        if (!m_projectiles[idx].active) {
            auto& p = m_projectiles[idx];
            p.active = true;
            p.x = x;
            p.y = y;
            p.vx = vx;
            p.vy = vy;
            p.radius = 8.0f;
            p.gravity = gravity;
            p.isMissile = false;
            p.isMine = false;
            p.isClusterBomb = false;
            p.isBossProjectile = isBoss;
            p.damage = damage;
            p.lifeTimer = 0.0f;

            m_nextProjectileIndex = (idx + 1) % MAX_PROJECTILES;
            return true;
        }
    }
    return false;
}

bool EnemyPool::spawnMissile(float x, float y, float vx, float vy, bool isBoss, int damage) {
    for (size_t i = 0; i < MAX_PROJECTILES; ++i) {
        const size_t idx = (m_nextProjectileIndex + i) % MAX_PROJECTILES;
        if (!m_projectiles[idx].active) {
            auto& p = m_projectiles[idx];
            p.active = true;
            p.x = x;
            p.y = y;
            p.vx = vx;
            p.vy = vy;
            p.radius = 7.0f;
            p.gravity = 0.0f;
            p.isMissile = true;
            p.isMine = false;
            p.isClusterBomb = false;
            p.isBossProjectile = isBoss;
            p.damage = damage;
            p.lifeTimer = 0.0f;

            m_nextProjectileIndex = (idx + 1) % MAX_PROJECTILES;
            return true;
        }
    }
    return false;
}

bool EnemyPool::spawnMine(float x, float y, int damage) {
    for (size_t i = 0; i < MAX_PROJECTILES; ++i) {
        const size_t idx = (m_nextProjectileIndex + i) % MAX_PROJECTILES;
        if (!m_projectiles[idx].active) {
            auto& p = m_projectiles[idx];
            p.active = true;
            p.x = x;
            p.y = y;
            p.vx = (nextRandomFloat() - 0.5f) * 20.0f;
            p.vy = 35.0f; // Descenso muy lento
            p.radius = 10.0f;
            p.gravity = 0.0f;
            p.isMissile = false;
            p.isMine = true;
            p.isClusterBomb = false;
            p.isBossProjectile = false;
            p.damage = damage;
            p.lifeTimer = 12.0f;

            m_nextProjectileIndex = (idx + 1) % MAX_PROJECTILES;
            return true;
        }
    }
    return false;
}

bool EnemyPool::spawnClusterBomb(float x, float y, float vx, float vy, float gravity, float splitDelay, bool isBoss, int damage) {
    for (size_t i = 0; i < MAX_PROJECTILES; ++i) {
        const size_t idx = (m_nextProjectileIndex + i) % MAX_PROJECTILES;
        if (!m_projectiles[idx].active) {
            auto& p = m_projectiles[idx];
            p.active = true;
            p.x = x;
            p.y = y;
            p.vx = vx;
            p.vy = vy;
            p.radius = 9.0f;
            p.gravity = gravity;
            p.isMissile = false;
            p.isMine = false;
            p.isClusterBomb = true;
            p.isBossProjectile = isBoss;
            p.damage = damage;
            p.splitTimer = splitDelay;
            p.lifeTimer = 0.0f;

            m_nextProjectileIndex = (idx + 1) % MAX_PROJECTILES;
            return true;
        }
    }
    return false;
}

bool EnemyPool::triggerIcbm(float targetX, float speed) {
    for (auto& icbm : m_icbms) {
        if (!icbm.active) {
            icbm.active = true;
            icbm.isDescending = false;
            icbm.x = std::clamp(targetX, 150.0f, 1770.0f);
            icbm.y = 0.0f;
            icbm.vy = speed;
            icbm.hp = 40;
            icbm.maxHp = 40;
            icbm.warningTimer = 0.8f; // 0.8s de advertencia previa
            icbm.hitFlashTimer = 0.0f;
            Core::AudioManager::getInstance().playAlert();
            return true;
        }
    }
    return false;
}

void EnemyPool::triggerMultiIcbm(float targetX1, float targetX2, float targetX3, float speed) {
    triggerIcbm(targetX1, speed);
    triggerIcbm(targetX2, speed);
    triggerIcbm(targetX3, speed);
}

void EnemyPool::update(float dt, float tankX, float tankY, int currentLevel, bool bossActive) {
    m_currentLevel = currentLevel;

    // -------------------------------------------------------------------------
    // 1. SPAWNER AUTOMÁTICO DE OLEADAS Y ROTACIÓN TÁCTICA DE ESCUADRONES
    // -------------------------------------------------------------------------
    // Reducción del 25% en la velocidad de bombas y proyectiles enemigos en niveles 1 al 12
    const float projSpeedScale = (currentLevel <= 12)
        ? 0.75f
        : (0.75f + (static_cast<float>(std::min(currentLevel, 24) - 12) / 12.0f) * 0.25f);

    const float speedMult = ((currentLevel >= 9) ? 1.15f : 1.0f) * projSpeedScale;

    if (!bossActive) {
        m_spawnTimer -= dt;
        if (m_spawnTimer <= 0.0f) {
            // Intervalo de Spawn progresivo y respirable: 2.6f base en nivel 1
            m_spawnTimer = std::max(1.45f, 2.6f - (static_cast<float>(currentLevel) * 0.045f));

            EnemyType type = EnemyType::ScoutPlane;
            if (currentLevel >= 20) {
                // Niveles 20 a 25: Máxima élite aérea combinada (StealthJet, LaserSatellite, Gunship, ArmoredZeppelin)
                static constexpr EnemyType poolLvl20[8] = {
                    EnemyType::StealthJet, EnemyType::LaserSatellite,
                    EnemyType::ArmoredZeppelin, EnemyType::Gunship,
                    EnemyType::DiveFighter, EnemyType::DroneSwarm,
                    EnemyType::HeavyBomber, EnemyType::LaserSatellite
                };
                type = poolLvl20[m_enemySpawnCounter % 8];
            } else if (currentLevel >= 16) {
                // Niveles 16 a 19: Combina Gunship, StealthJet, DroneSwarm, DiveFighter, HeavyBomber
                static constexpr EnemyType poolLvl16[8] = {
                    EnemyType::Gunship, EnemyType::StealthJet,
                    EnemyType::DiveFighter, EnemyType::DroneSwarm,
                    EnemyType::LaserSatellite, EnemyType::HeavyBomber,
                    EnemyType::Gunship, EnemyType::ArmoredZeppelin
                };
                type = poolLvl16[m_enemySpawnCounter % 8];
            } else if (currentLevel >= 14) {
                // Niveles 14 y 15: Combina LaserSatellite, ArmoredZeppelin, DroneSwarm y HeavyBomber
                static constexpr EnemyType poolLvl14[8] = {
                    EnemyType::LaserSatellite, EnemyType::ArmoredZeppelin,
                    EnemyType::DroneSwarm, EnemyType::HeavyBomber,
                    EnemyType::ArmoredZeppelin, EnemyType::LaserSatellite,
                    EnemyType::DroneSwarm, EnemyType::HeavyBomber
                };
                type = poolLvl14[m_enemySpawnCounter % 8];
            } else if (currentLevel >= 11) {
                // Niveles 11 a 13: Combina StealthJet, DiveFighter, Gunship, MineDropper y AttackHeli
                static constexpr EnemyType poolLvl11[10] = {
                    EnemyType::StealthJet, EnemyType::DiveFighter,
                    EnemyType::Gunship, EnemyType::MineDropper,
                    EnemyType::AttackHeli, EnemyType::Gunship,
                    EnemyType::StealthJet, EnemyType::DiveFighter,
                    EnemyType::MineDropper, EnemyType::AttackHeli
                };
                type = poolLvl11[m_enemySpawnCounter % 10];
            } else if (currentLevel >= 9) {
                // Niveles 9 y 10: Priorizar StealthJet, LaserSatellite, ArmoredZeppelin, Gunship
                static constexpr EnemyType poolLvl9[8] = {
                    EnemyType::StealthJet, EnemyType::LaserSatellite,
                    EnemyType::ArmoredZeppelin, EnemyType::Gunship,
                    EnemyType::StealthJet, EnemyType::ArmoredZeppelin,
                    EnemyType::LaserSatellite, EnemyType::Gunship
                };
                type = poolLvl9[m_enemySpawnCounter % 8];
            } else if (currentLevel >= 6) {
                // Niveles 6 a 8: Priorizar DiveFighter, Gunship, DroneSwarm, MineDropper, HeavyBomber
                static constexpr EnemyType poolLvl6[8] = {
                    EnemyType::DiveFighter, EnemyType::Gunship,
                    EnemyType::DroneSwarm, EnemyType::MineDropper,
                    EnemyType::HeavyBomber, EnemyType::DiveFighter,
                    EnemyType::Gunship, EnemyType::DroneSwarm
                };
                type = poolLvl6[m_enemySpawnCounter % 8];
            } else {
                // Niveles 1 a 5: Desbloqueo progresivo
                int maxArchetype = 2; // Niveles 1-3
                if (currentLevel >= 4) {
                    maxArchetype = 4;
                }
                const uint32_t choice = m_enemySpawnCounter % (static_cast<uint32_t>(maxArchetype) + 1);
                type = static_cast<EnemyType>(choice);
            }
            m_enemySpawnCounter++;

            const int dir = (nextRandomFloat() > 0.5f) ? 1 : -1;
            const float startY = 120.0f + nextRandomFloat() * 380.0f;

            spawnEnemy(type, dir, startY, currentLevel);

            // Si es DroneSwarm, genera 2 drones escoltas adicionales en formación V (escuadra de 3)
            if (type == EnemyType::DroneSwarm) {
                spawnEnemy(EnemyType::DroneSwarm, dir, startY - 35.0f, currentLevel);
                spawnEnemy(EnemyType::DroneSwarm, dir, startY + 35.0f, currentLevel);
            }
        }

        // Activación de proyectil balístico ICBM en niveles múltiplos de 5 (5, 10, 15, 20, 25)
        if (currentLevel % 5 == 0) {
            m_icbmCooldown -= dt;
            if (m_icbmCooldown <= 0.0f) {
                // Nivel 20 y 25 caen a 620 px/s y con intervalos más agresivos (2.3s)
                const float icbmSpeed = (currentLevel >= 20) ? 620.0f : ((currentLevel >= 15) ? 598.0f : 520.0f);
                const float baseInterval = (currentLevel >= 20) ? 2.3f : ((currentLevel >= 15) ? 2.6f : ((currentLevel == 10) ? 3.2f : 4.5f));
                m_icbmCooldown = baseInterval + nextRandomFloat() * 1.6f;
                triggerIcbm(tankX, icbmSpeed);
            }
        }
    }

    // -------------------------------------------------------------------------
    // 2. ACTUALIZACIÓN DE MISILES BALÍSTICOS ICBM
    // -------------------------------------------------------------------------
    for (auto& icbm : m_icbms) {
        if (!icbm.active) {
            continue;
        }

        if (icbm.hitFlashTimer > 0.0f) {
            icbm.hitFlashTimer -= dt;
            if (icbm.hitFlashTimer < 0.0f) icbm.hitFlashTimer = 0.0f;
        }

        if (!icbm.isDescending) {
            icbm.warningTimer -= dt;
            if (icbm.warningTimer <= 0.0f) {
                icbm.isDescending = true;
                icbm.y = -30.0f;
            }
        } else {
            icbm.y += icbm.vy * dt;
            if (icbm.y > 1050.0f) {
                icbm.active = false;
            }
        }
    }

    // -------------------------------------------------------------------------
    // 3. CINEMÁTICA Y COMPORTAMIENTO DE LOS 10 ARQUETIPOS
    // -------------------------------------------------------------------------
    for (auto& e : m_enemies) {
        if (!e.active) {
            continue;
        }

        if (e.hitFlashTimer > 0.0f) {
            e.hitFlashTimer -= dt;
            if (e.hitFlashTimer < 0.0f) e.hitFlashTimer = 0.0f;
        }

        e.animTimer += dt;
        e.attackTimer += dt;
        e.secondaryTimer += dt;

        // REGLA CRÍTICA: NUNCA SE DETIENE EN X (|vx| >= 130.0f SIEMPRE)
        e.x += e.vx * dt;

        const float lvlRatio = static_cast<float>(std::clamp(currentLevel, 1, 24) - 1) / 23.0f;
        const int planeBombDmg = static_cast<int>(8.0f + lvlRatio * 8.0f);
        const int planeMissileDmg = planeBombDmg + 4;

        switch (e.type) {
        case EnemyType::ScoutPlane: {
            // Suelta 1 bomba precisa al pasar cerca de la vertical del tanque
            if (!e.hasDroppedBomb && std::abs(e.x - tankX) < 140.0f) {
                spawnBomb(e.x, e.y + 14.0f, e.vx * 0.45f, 60.0f * projSpeedScale, 500.0f * projSpeedScale, false, planeBombDmg);
                e.hasDroppedBomb = true;
            }
            break;
        }

        case EnemyType::AttackHeli: {
            // Onda senoidal vertical continua mientras cruza la pantalla a toda velocidad
            e.y = e.startY + std::sin(e.animTimer * 4.0f) * 60.0f;

            // Ráfaga de misiles dirigidos cada 1.8 segundos
            if (e.attackTimer >= 1.8f) {
                e.attackTimer = 0.0f;
                float dx = tankX - e.x;
                float dy = (tankY - 30.0f) - e.y;
                const float dist = std::hypot(dx, dy);
                if (dist > 1.0f) {
                    dx /= dist;
                    dy /= dist;
                }
                spawnMissile(e.x, e.y + 14.0f, dx * 440.0f * projSpeedScale, dy * 440.0f * projSpeedScale, false, planeMissileDmg);
            }
            break;
        }

        case EnemyType::HeavyBomber: {
            // Alfombra de bombas continuas cada 1.3s (o bomba de racimo en niveles >= 6)
            if (e.attackTimer >= 1.3f) {
                e.attackTimer = 0.0f;
                if (e.x > 80.0f && e.x < 1840.0f) {
                    if (currentLevel >= 6 && (m_enemySpawnCounter % 3 == 0)) {
                        spawnClusterBomb(e.x, e.y + 22.0f, e.vx * 0.35f, 60.0f * speedMult, 400.0f * projSpeedScale, 0.9f, false, planeBombDmg + 2);
                    } else {
                        spawnBomb(e.x, e.y + 22.0f, e.vx * 0.35f, 50.0f * speedMult, 440.0f * projSpeedScale, false, planeBombDmg);
                    }
                }
            }
            break;
        }

        case EnemyType::DiveFighter: {
            // Picada hacia el tanque y recuperación de altitud
            if (!e.divePullingUp) {
                e.y += 180.0f * dt;
                if (e.y >= 680.0f || std::abs(e.x - tankX) < 100.0f) {
                    e.divePullingUp = true;
                    spawnBomb(e.x, e.y + 10.0f, e.vx * 0.6f, 120.0f * speedMult, 550.0f * projSpeedScale, false, planeBombDmg);
                }
            } else {
                e.y -= 190.0f * dt;
            }
            break;
        }

        case EnemyType::StealthJet: {
            // Fading periódico de camuflaje de sigilo
            e.stealthAlpha = 70.0f + (std::sin(e.animTimer * 3.5f) * 0.5f + 0.5f) * 185.0f;

            if (e.attackTimer >= 1.5f) {
                e.attackTimer = 0.0f;
                float dx = tankX - e.x;
                float dy = (tankY - 30.0f) - e.y;
                const float dist = std::hypot(dx, dy);
                if (dist > 1.0f) {
                    dx /= dist;
                    dy /= dist;
                }
                spawnMissile(e.x, e.y + 12.0f, dx * 510.0f * speedMult, dy * 510.0f * speedMult, false, planeMissileDmg);
            }
            break;
        }

        case EnemyType::Gunship: {
            // Ráfaga triple dirigida cada 2.0s
            if (e.attackTimer >= 2.0f) {
                e.attackTimer = 0.0f;
                float dx = tankX - e.x;
                float dy = (tankY - 30.0f) - e.y;
                const float dist = std::hypot(dx, dy);
                if (dist > 1.0f) {
                    dx /= dist;
                    dy /= dist;
                }
                spawnMissile(e.x - 20.0f, e.y + 18.0f, dx * 430.0f * speedMult, dy * 430.0f * speedMult, false, planeMissileDmg);
                spawnMissile(e.x, e.y + 22.0f, dx * 450.0f * speedMult, dy * 450.0f * speedMult, false, planeMissileDmg);
                spawnMissile(e.x + 20.0f, e.y + 18.0f, dx * 430.0f * speedMult, dy * 430.0f * speedMult, false, planeMissileDmg);
            }
            break;
        }

        case EnemyType::DroneSwarm: {
            // Movimiento ágil con zig-zag vertical suave
            e.y = e.startY + std::sin(e.animTimer * 6.0f) * 25.0f;

            if (e.attackTimer >= 1.6f && std::abs(e.x - tankX) < 180.0f) {
                e.attackTimer = 0.0f;
                spawnBomb(e.x, e.y + 8.0f, 0.0f, 140.0f * speedMult, 520.0f * projSpeedScale, false, planeBombDmg);
            }
            break;
        }

        case EnemyType::MineDropper: {
            // Despliega minas aéreas flotantes lentas
            if (e.attackTimer >= 1.8f) {
                e.attackTimer = 0.0f;
                if (e.x > 120.0f && e.x < 1800.0f) {
                    spawnMine(e.x, e.y + 20.0f, planeBombDmg + 2);
                }
            }
            break;
        }

        case EnemyType::LaserSatellite: {
            // Secuencia del rayo láser orbital
            e.laserTimer += dt;
            if (e.laserTimer >= 2.8f) {
                e.isLaserFiring = true;
                if (e.laserTimer >= 3.6f) {
                    e.isLaserFiring = false;
                    e.laserTimer = 0.0f;
                }
            }
            break;
        }

        case EnemyType::ArmoredZeppelin: {
            // Fortaleza volante: lanza bombas pesadas y misiles defensivos
            if (e.attackTimer >= 1.5f) {
                e.attackTimer = 0.0f;
                spawnBomb(e.x - 40.0f, e.y + 28.0f, -30.0f, 60.0f * speedMult, 420.0f * projSpeedScale, false, planeBombDmg);
                spawnBomb(e.x + 40.0f, e.y + 28.0f, 30.0f, 60.0f * speedMult, 420.0f * projSpeedScale, false, planeBombDmg);
            }
            if (e.secondaryTimer >= 2.5f) {
                e.secondaryTimer = 0.0f;
                float dx = tankX - e.x;
                float dy = (tankY - 30.0f) - e.y;
                const float dist = std::hypot(dx, dy);
                if (dist > 1.0f) {
                    dx /= dist;
                    dy /= dist;
                }
                spawnMissile(e.x, e.y + 30.0f, dx * 460.0f * speedMult, dy * 460.0f * speedMult, false, planeMissileDmg);
            }
            break;
        }
        }

        // Desactivación al cruzar totalmente la pantalla
        if (e.direction > 0 && e.x > 2180.0f) {
            e.active = false;
        } else if (e.direction < 0 && e.x < -240.0f) {
            e.active = false;
        }
    }

    // -------------------------------------------------------------------------
    // 4. PROYECTILES Y BOMBAS ENEMIGAS
    // -------------------------------------------------------------------------
    for (auto& p : m_projectiles) {
        if (!p.active) {
            continue;
        }

        if (p.isMine) {
            p.lifeTimer -= dt;
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            if (p.lifeTimer <= 0.0f || p.y >= 980.0f) {
                p.active = false;
            }
        } else if (p.isClusterBomb) {
            p.splitTimer -= dt;
            p.vy += p.gravity * dt;
            p.x += p.vx * dt;
            p.y += p.vy * dt;

            // División en el aire en 3 bombas en abanico
            if (p.splitTimer <= 0.0f) {
                p.active = false;
                const bool bossProj = p.isBossProjectile;
                const int dmg = p.damage;
                spawnBomb(p.x - 10.0f, p.y, -140.0f * speedMult, 260.0f * speedMult, 400.0f, bossProj, dmg);
                spawnBomb(p.x,         p.y,    0.0f,            280.0f * speedMult, 400.0f, bossProj, dmg);
                spawnBomb(p.x + 10.0f, p.y,  140.0f * speedMult, 260.0f * speedMult, 400.0f, bossProj, dmg);
                continue;
            }

            if (p.x < -60.0f || p.x > 1980.0f || p.y < -60.0f || p.y > 1050.0f) {
                p.active = false;
            }
        } else {
            p.vy += p.gravity * dt;
            p.x += p.vx * dt;
            p.y += p.vy * dt;

            if (p.x < -60.0f || p.x > 1980.0f || p.y < -60.0f || p.y > 1050.0f) {
                p.active = false;
            }
        }
    }
}

void EnemyPool::destroyAllWithNuke(ParticlePool& particlePool, float& screenShake) {
    for (auto& e : m_enemies) {
        if (e.active) {
            e.active = false;
            const int pCount = (e.type == EnemyType::HeavyBomber || e.type == EnemyType::ArmoredZeppelin) ? 45 : 26;
            particlePool.spawnExplosion(e.x, e.y, pCount, 360.0f, true);
        }
    }

    for (auto& p : m_projectiles) {
        if (p.active) {
            p.active = false;
            particlePool.spawnExplosion(p.x, p.y, 16, 200.0f, false);
        }
    }

    for (auto& icbm : m_icbms) {
        if (icbm.active) {
            icbm.active = false;
            particlePool.spawnExplosion(icbm.x, icbm.y, 40, 380.0f, true);
        }
    }

    screenShake = 1.0f;
}

size_t EnemyPool::getActiveEnemyCount() const noexcept {
    size_t count = 0;
    for (const auto& e : m_enemies) {
        if (e.active) ++count;
    }
    return count;
}

size_t EnemyPool::getActiveProjectileCount() const noexcept {
    size_t count = 0;
    for (const auto& p : m_projectiles) {
        if (p.active) ++count;
    }
    return count;
}

void EnemyPool::render(SDL_Renderer* renderer) const {
    if (!renderer) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // =========================================================================
    // 1. MISIL BALÍSTICO ICBM (ALERTA Y PROYECTIL CAYENDO)
    // =========================================================================
    for (const auto& icbm : m_icbms) {
        if (!icbm.active) {
            continue;
        }

        if (!icbm.isDescending) {
            // A. Alerta roja parpadeante en la parte superior (0.8s)
            const float pulse = std::abs(std::sin(icbm.warningTimer * 20.0f));
            const Uint8 alertAlpha = static_cast<Uint8>(150.0f + pulse * 105.0f);

            // Caja de advertencia
            const SDL_FRect alertBox{icbm.x - 22.0f, 60.0f, 44.0f, 44.0f};
            SDL_SetRenderDrawColor(renderer, 240, 30, 30, alertAlpha);
            SDL_RenderFillRectF(renderer, &alertBox);

            // Borde
            const SDL_FRect alertBorder{icbm.x - 24.0f, 58.0f, 48.0f, 48.0f};
            SDL_SetRenderDrawColor(renderer, 255, 230, 40, alertAlpha);
            SDL_RenderFillRectF(renderer, &alertBorder);

            // Signo de exclamación "!" blanco
            const SDL_FRect barTop{icbm.x - 3.0f, 66.0f, 6.0f, 20.0f};
            const SDL_FRect dotBot{icbm.x - 3.0f, 92.0f, 6.0f, 6.0f};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRectF(renderer, &barTop);
            SDL_RenderFillRectF(renderer, &dotBot);

            // Línea de trayectoria vertical tenue
            const SDL_FRect trailLine{icbm.x - 2.0f, 108.0f, 4.0f, 870.0f};
            SDL_SetRenderDrawColor(renderer, 240, 40, 40, 45);
            SDL_RenderFillRectF(renderer, &trailLine);
        } else {
            // B. Misil balístico cayendo a 520 px/s (o 598 px/s en nivel 15)
            const bool hit = (icbm.hitFlashTimer > 0.0f);
            SDL_Texture* spriteTex = Core::AssetManager::getInstance().getSpriteSheet();

            if (spriteTex) {
                const SDL_Rect srcRect = Core::AssetManager::getInstance().getMissileSrc();
                const SDL_Rect dstRect = {
                    static_cast<int>(icbm.x - 18.0f),
                    static_cast<int>(icbm.y - 32.0f),
                    36, 64
                };
                if (hit) {
                    SDL_SetTextureColorMod(spriteTex, 255, 80, 80);
                }
                // Rotado 180° porque en el atlas el misil apunta hacia arriba y aquí desciende
                SDL_RenderCopyEx(renderer, spriteTex, &srcRect, &dstRect, 180.0, nullptr, SDL_FLIP_NONE);
                if (hit) {
                    SDL_SetTextureColorMod(spriteTex, 255, 255, 255);
                }

                // Llamarada del propulsor en la parte superior
                const SDL_FRect flame{icbm.x - 5.0f, icbm.y - 42.0f, 10.0f, 12.0f};
                SDL_SetRenderDrawColor(renderer, 255, 140, 30, 230);
                SDL_RenderFillRectF(renderer, &flame);
            } else {
                // Fallback geométrico sólido
                const SDL_FRect warhead{icbm.x - 7.0f, icbm.y + 20.0f, 14.0f, 16.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 240, hit ? 255 : 40, hit ? 255 : 40, 255);
                SDL_RenderFillRectF(renderer, &warhead);

                const SDL_FRect hull{icbm.x - 8.0f, icbm.y - 24.0f, 16.0f, 44.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 220, hit ? 255 : 225, hit ? 255 : 230, 255);
                SDL_RenderFillRectF(renderer, &hull);

                const SDL_FRect finL{icbm.x - 14.0f, icbm.y - 24.0f, 6.0f, 12.0f};
                const SDL_FRect finR{icbm.x + 8.0f, icbm.y - 24.0f, 6.0f, 12.0f};
                SDL_SetRenderDrawColor(renderer, 60, 65, 70, 255);
                SDL_RenderFillRectF(renderer, &finL);
                SDL_RenderFillRectF(renderer, &finR);

                const SDL_FRect flame{icbm.x - 5.0f, icbm.y - 36.0f, 10.0f, 12.0f};
                SDL_SetRenderDrawColor(renderer, 255, 140, 30, 230);
                SDL_RenderFillRectF(renderer, &flame);
            }
        }
    }

    // =========================================================================
    // 2. PROYECTILES, BOMBAS Y MINAS FLOTANTES
    // =========================================================================
    SDL_Texture* spriteTex = Core::AssetManager::getInstance().getSpriteSheet();

    for (const auto& p : m_projectiles) {
        if (!p.active) {
            continue;
        }

        if (p.isMine) {
            // Mina aérea flotante con púas y sensor parpadeante
            const SDL_FRect mineBody{p.x - 8.0f, p.y - 8.0f, 16.0f, 16.0f};
            SDL_SetRenderDrawColor(renderer, 50, 55, 60, 255);
            SDL_RenderFillRectF(renderer, &mineBody);

            const SDL_FRect mineSpikeH{p.x - 12.0f, p.y - 2.0f, 24.0f, 4.0f};
            const SDL_FRect mineSpikeV{p.x - 2.0f, p.y - 12.0f, 4.0f, 24.0f};
            SDL_SetRenderDrawColor(renderer, 220, 45, 45, 255);
            SDL_RenderFillRectF(renderer, &mineSpikeH);
            SDL_RenderFillRectF(renderer, &mineSpikeV);

            // Sensor central parpadeante
            const SDL_FRect sensor{p.x - 3.0f, p.y - 3.0f, 6.0f, 6.0f};
            SDL_SetRenderDrawColor(renderer, 255, 220, 40, 240);
            SDL_RenderFillRectF(renderer, &sensor);
        } else if (p.isMissile) {
            // Misil guiado: Celda (Col 3, Fila 0) rotada en dirección de velocidad
            if (spriteTex) {
                const SDL_Rect srcRect = Core::AssetManager::getInstance().getMissileSrc();
                const double angle = std::atan2(p.vy, p.vx) * 180.0 / 3.14159265358979323846 + 90.0;
                const SDL_Rect dstRect = {
                    static_cast<int>(p.x - 9.0f),
                    static_cast<int>(p.y - 16.0f),
                    18, 32
                };
                SDL_RenderCopyEx(renderer, spriteTex, &srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);

                // Llamarada del motor
                const float spd = std::hypot(p.vx, p.vy);
                const float dx = (spd > 0.001f) ? (p.vx / spd) : 0.0f;
                const float dy = (spd > 0.001f) ? (p.vy / spd) : 1.0f;
                const SDL_FRect flame{p.x - dx * 18.0f - 2.5f, p.y - dy * 18.0f - 2.5f, 5.0f, 5.0f};
                SDL_SetRenderDrawColor(renderer, 255, 140, 30, 230);
                SDL_RenderFillRectF(renderer, &flame);
            } else {
                // Fallback geométrico
                const float spd = std::hypot(p.vx, p.vy);
                const float dx = (spd > 0.001f) ? (p.vx / spd) : 0.0f;
                const float dy = (spd > 0.001f) ? (p.vy / spd) : 1.0f;
                const float px = -dy;
                const float py = dx;

                const SDL_Color missileColor{220, 225, 230, 255};
                const SDL_Vertex missileVerts[4]{
                    {{p.x - dx * 16.0f + px * 3.0f, p.y - dy * 16.0f + py * 3.0f}, missileColor, {0.0f, 0.0f}},
                    {{p.x - dx * 16.0f - px * 3.0f, p.y - dy * 16.0f - py * 3.0f}, missileColor, {0.0f, 0.0f}},
                    {{p.x - px * 3.0f,              p.y - py * 3.0f},              missileColor, {0.0f, 0.0f}},
                    {{p.x + px * 3.0f,              p.y + py * 3.0f},              missileColor, {0.0f, 0.0f}}
                };
                const int quadIdx[6]{0, 1, 2, 2, 3, 0};
                SDL_RenderGeometry(renderer, nullptr, missileVerts, 4, quadIdx, 6);

                const SDL_Color tipColor{240, 40, 40, 255};
                const SDL_Vertex tipVerts[3]{
                    {{p.x + dx * 6.0f, p.y + dy * 6.0f}, tipColor, {0.0f, 0.0f}},
                    {{p.x - px * 3.0f, p.y - py * 3.0f}, tipColor, {0.0f, 0.0f}},
                    {{p.x + px * 3.0f, p.y + py * 3.0f}, tipColor, {0.0f, 0.0f}}
                };
                const int triIdx[3]{0, 1, 2};
                SDL_RenderGeometry(renderer, nullptr, tipVerts, 3, triIdx, 3);
            }
        } else {
            // Bomba o Bomba de Racimo: Celda (Col 2, Fila 0)
            if (spriteTex) {
                const SDL_Rect srcRect = Core::AssetManager::getInstance().getBombSrc();
                const double angle = std::atan2(p.vy, p.vx) * 180.0 / 3.14159265358979323846 - 90.0;
                const int w = p.isClusterBomb ? 26 : 20;
                const int h = p.isClusterBomb ? 34 : 28;
                const SDL_Rect dstRect = {
                    static_cast<int>(p.x - w * 0.5f),
                    static_cast<int>(p.y - h * 0.5f),
                    w, h
                };
                if (p.isClusterBomb) {
                    SDL_SetTextureColorMod(spriteTex, 255, 215, 60); // Tinte amarillento para racimo
                }
                SDL_RenderCopyEx(renderer, spriteTex, &srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);
                if (p.isClusterBomb) {
                    SDL_SetTextureColorMod(spriteTex, 255, 255, 255);
                }
            } else {
                // Fallback geométrico
                const SDL_FRect finRect{p.x - 7.0f, p.y - 10.0f, 14.0f, 4.0f};
                SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
                SDL_RenderFillRectF(renderer, &finRect);

                const SDL_FRect bodyRect{p.x - 5.0f, p.y - 6.0f, 10.0f, 14.0f};
                SDL_SetRenderDrawColor(renderer, 45, 50, 55, 255);
                SDL_RenderFillRectF(renderer, &bodyRect);

                const SDL_FRect bandRect{p.x - 5.0f, p.y - 1.0f, 10.0f, 3.0f};
                SDL_SetRenderDrawColor(renderer, 255, 220, 40, 255);
                SDL_RenderFillRectF(renderer, &bandRect);
            }
        }
    }

    // =========================================================================
    // 3. RENDERIZADO DE LOS 10 ARQUETIPOS DE ENEMIGOS (PIXEL ART SVG / FALLBACK)
    // =========================================================================
    for (const auto& e : m_enemies) {
        if (!e.active) {
            continue;
        }

        const bool hit = (e.hitFlashTimer > 0.0f);
        const Uint8 alpha = (e.type == EnemyType::StealthJet)
            ? static_cast<Uint8>(std::clamp(e.stealthAlpha, 60.0f, 255.0f))
            : 255;

        int spriteIndex = 0;
        int spriteW = 76;
        int spriteH = 76;

        switch (e.type) {
        case EnemyType::ScoutPlane:     spriteIndex = 0; spriteW = 74; spriteH = 74; break;
        case EnemyType::HeavyBomber:    spriteIndex = 1; spriteW = 90; spriteH = 90; break;
        case EnemyType::AttackHeli:     spriteIndex = 2; spriteW = 78; spriteH = 78; break;
        case EnemyType::StealthJet:     spriteIndex = 3; spriteW = 76; spriteH = 76; break;
        case EnemyType::DiveFighter:    spriteIndex = 4; spriteW = 74; spriteH = 74; break;
        case EnemyType::Gunship:        spriteIndex = 5; spriteW = 90; spriteH = 90; break;
        case EnemyType::LaserSatellite: spriteIndex = 6; spriteW = 86; spriteH = 86; break;
        case EnemyType::ArmoredZeppelin:spriteIndex = 7; spriteW = 140; spriteH = 90; break;
        case EnemyType::DroneSwarm:     spriteIndex = 8; spriteW = 48; spriteH = 48; break;
        case EnemyType::MineDropper:    spriteIndex = 9; spriteW = 86; spriteH = 86; break;
        }

        if (spriteTex) {
            const SDL_Rect srcRect = Core::AssetManager::getInstance().getAircraftSrc(spriteIndex);
            // Los aviones miran hacia ARRIBA (0 grados) en el atlas. Rotamos al vector (+90°):
            const double angle = std::atan2(e.vy, e.vx) * 180.0 / 3.14159265358979323846 + 90.0;
            const SDL_Rect dstRect = {
                static_cast<int>(e.x - spriteW * 0.5f),
                static_cast<int>(e.y - spriteH * 0.5f),
                spriteW, spriteH
            };

            if (hit) {
                SDL_SetTextureColorMod(spriteTex, 255, 80, 80);
            }
            if (alpha < 255) {
                SDL_SetTextureAlphaMod(spriteTex, alpha);
            }

            SDL_RenderCopyEx(renderer, spriteTex, &srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);

            if (hit) {
                SDL_SetTextureColorMod(spriteTex, 255, 255, 255);
            }
            if (alpha < 255) {
                SDL_SetTextureAlphaMod(spriteTex, 255);
            }

            // Rayo láser orbital de LaserSatellite si está activo
            if (e.type == EnemyType::LaserSatellite && e.isLaserFiring) {
                const SDL_FRect beamCore{e.x - 6.0f, e.y + 8.0f, 12.0f, 980.0f - (e.y + 8.0f)};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 245);
                SDL_RenderFillRectF(renderer, &beamCore);

                const SDL_FRect beamAura{e.x - 14.0f, e.y + 8.0f, 28.0f, 980.0f - (e.y + 8.0f)};
                SDL_SetRenderDrawColor(renderer, 245, 40, 40, 160);
                SDL_RenderFillRectF(renderer, &beamAura);
            }
        } else {
            // Fallback geométrico existente
            switch (e.type) {
            case EnemyType::ScoutPlane: {
                const SDL_FRect body{e.x - 26.0f, e.y - 5.0f, 52.0f, 10.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 85, hit ? 255 : 105, hit ? 255 : 125, alpha);
                SDL_RenderFillRectF(renderer, &body);
                break;
            }
            case EnemyType::AttackHeli: {
                const SDL_FRect cabin{e.x - 24.0f, e.y - 9.0f, 48.0f, 20.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 120, hit ? 255 : 95, hit ? 255 : 60, 255);
                SDL_RenderFillRectF(renderer, &cabin);
                break;
            }
            case EnemyType::HeavyBomber: {
                const SDL_FRect hull{e.x - 55.0f, e.y - 11.0f, 110.0f, 22.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 70, hit ? 255 : 85, hit ? 255 : 75, 255);
                SDL_RenderFillRectF(renderer, &hull);
                break;
            }
            case EnemyType::DiveFighter: {
                const SDL_FRect body{e.x - 24.0f, e.y - 5.0f, 48.0f, 10.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 180, hit ? 255 : 80, hit ? 255 : 30, 255);
                SDL_RenderFillRectF(renderer, &body);
                break;
            }
            case EnemyType::StealthJet: {
                const SDL_FRect body{e.x - 30.0f, e.y - 6.0f, 60.0f, 12.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 30, hit ? 255 : 32, hit ? 255 : 36, alpha);
                SDL_RenderFillRectF(renderer, &body);
                break;
            }
            case EnemyType::Gunship: {
                const SDL_FRect body{e.x - 45.0f, e.y - 10.0f, 90.0f, 20.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 85, hit ? 255 : 80, hit ? 255 : 95, 255);
                SDL_RenderFillRectF(renderer, &body);
                break;
            }
            case EnemyType::DroneSwarm: {
                const SDL_FRect body{e.x - 15.0f, e.y - 4.0f, 30.0f, 8.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 120, hit ? 255 : 140, hit ? 255 : 160, 255);
                SDL_RenderFillRectF(renderer, &body);
                break;
            }
            case EnemyType::MineDropper: {
                const SDL_FRect body{e.x - 35.0f, e.y - 10.0f, 70.0f, 20.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 100, hit ? 255 : 90, hit ? 255 : 60, 255);
                SDL_RenderFillRectF(renderer, &body);
                break;
            }
            case EnemyType::LaserSatellite: {
                const SDL_FRect body{e.x - 25.0f, e.y - 8.0f, 50.0f, 16.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 65, hit ? 255 : 75, hit ? 255 : 90, 255);
                SDL_RenderFillRectF(renderer, &body);
                break;
            }
            case EnemyType::ArmoredZeppelin: {
                const SDL_FRect envelope{e.x - 85.0f, e.y - 20.0f, 170.0f, 40.0f};
                SDL_SetRenderDrawColor(renderer, hit ? 255 : 75, hit ? 255 : 75, hit ? 255 : 85, 255);
                SDL_RenderFillRectF(renderer, &envelope);
                break;
            }
            }
        }

        // Micro barra de vida táctica para enemigos regulares dañados
        if (e.hp < e.maxHp && e.hp > 0) {
            const float barW = 40.0f;
            const float barH = 5.0f;
            const float barX = e.x - barW * 0.5f;
            const float barY = e.y - (e.height * 0.5f + 10.0f);

            // Fondo negro
            const SDL_FRect bgRect{barX, barY, barW, barH};
            SDL_SetRenderDrawColor(renderer, 15, 15, 20, 230);
            SDL_RenderFillRectF(renderer, &bgRect);

            // Relleno proporcional (verde > 50%, amarillo > 25%, rojo <= 25%)
            const float ratio = std::clamp(static_cast<float>(e.hp) / static_cast<float>(std::max(1, e.maxHp)), 0.0f, 1.0f);
            const float fillW = (barW - 2.0f) * ratio;
            if (fillW > 0.0f) {
                const SDL_FRect fillRect{barX + 1.0f, barY + 1.0f, fillW, barH - 2.0f};
                if (ratio > 0.5f) {
                    SDL_SetRenderDrawColor(renderer, 46, 204, 113, 255);
                } else if (ratio > 0.25f) {
                    SDL_SetRenderDrawColor(renderer, 241, 196, 15, 255);
                } else {
                    SDL_SetRenderDrawColor(renderer, 231, 76, 60, 255);
                }
                SDL_RenderFillRectF(renderer, &fillRect);
            }
        }
    }
}

} // namespace Entities
