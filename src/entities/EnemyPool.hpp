#pragma once

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <array>
#include <cstddef>
#include <cstdint>

namespace Entities {

class ParticlePool;
class PowerUpPool;

enum class EnemyType : uint8_t {
    ScoutPlane = 0,     // Caza veloz, bombas de caída libre
    AttackHeli,         // Vuelo oscilatorio senoidal, ráfagas dirigidas
    HeavyBomber,        // Gran resistencia, alfombra de bombas
    DiveFighter,        // Caza en picada hacia la posición X del tanque
    StealthJet,         // Caza sigiloso negro que se vuelve translúcido
    Gunship,            // Avión artillero fuertemente blindado, ráfagas triples
    DroneSwarm,         // Drones pequeños y ágiles en formación rápida
    MineDropper,        // Despliega minas aéreas flotantes lentas
    LaserSatellite,     // Satélite en la estratosfera con rayo láser vertical
    ArmoredZeppelin     // Zepelín titánico con torretas defensivas
};

struct Enemy {
    EnemyType type{EnemyType::ScoutPlane};
    float x{0.0f};
    float y{0.0f};
    float startY{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    float width{75.0f};
    float height{28.0f};
    int hp{2};
    int maxHp{2};
    int direction{1};           // +1 = derecha, -1 = izquierda
    float hitFlashTimer{0.0f};
    float attackTimer{0.0f};
    float secondaryTimer{0.0f};
    float animTimer{0.0f};
    bool hasDroppedBomb{false};
    bool divePullingUp{false};  // Para DiveFighter
    float stealthAlpha{255.0f}; // Para StealthJet
    float laserTimer{0.0f};     // Para LaserSatellite
    bool isLaserFiring{false};  // Para LaserSatellite
    bool active{false};
};

struct EnemyProjectile {
    float x{0.0f};
    float y{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    float radius{8.0f};
    float gravity{0.0f};
    bool isMissile{false};
    bool isMine{false};         // Mina flotante lenta
    bool isClusterBomb{false};  // Bomba de racimo
    float splitTimer{0.0f};     // Tiempo antes de dividirse en 3
    float lifeTimer{0.0f};
    bool isBossProjectile{false};
    int damage{10};
    bool active{false};
};

struct IcbmMissile {
    float x{0.0f};
    float y{0.0f};
    float vy{520.0f};
    int hp{40};
    int maxHp{40};
    float warningTimer{0.0f};   // 0.8s de alerta roja antes de caer
    float hitFlashTimer{0.0f};
    bool active{false};
    bool isDescending{false};
};

class EnemyPool {
public:
    static constexpr size_t MAX_ENEMIES{64};
    static constexpr size_t MAX_PROJECTILES{200};
    static constexpr size_t MAX_ICBMS{6};
    static constexpr float SPAWN_INTERVAL{2.6f};

    EnemyPool();
    ~EnemyPool() = default;

    EnemyPool(const EnemyPool&) = delete;
    EnemyPool& operator=(const EnemyPool&) = delete;

    void update(float dt, float tankX, float tankY, int currentLevel, bool bossActive);
    void render(SDL_Renderer* renderer) const;

    void spawnEnemy(EnemyType type, int direction, float startY, int currentLevel = 1);
    bool spawnBomb(float x, float y, float vx, float vy, float gravity, bool isBoss = false, int damage = 10);
    bool spawnMissile(float x, float y, float vx, float vy, bool isBoss = false, int damage = 10);
    bool spawnMine(float x, float y, int damage = 10);
    bool spawnClusterBomb(float x, float y, float vx, float vy, float gravity, float splitDelay = 1.0f, bool isBoss = false, int damage = 10);
    bool triggerIcbm(float targetX, float speed = 520.0f);
    void triggerMultiIcbm(float targetX1, float targetX2, float targetX3, float speed = 598.0f);

    void clear() noexcept;
    void destroyAllWithNuke(ParticlePool& particlePool, float& screenShake);

    [[nodiscard]] std::array<Enemy, MAX_ENEMIES>& getEnemies() noexcept { return m_enemies; }
    [[nodiscard]] const std::array<Enemy, MAX_ENEMIES>& getEnemies() const noexcept { return m_enemies; }

    [[nodiscard]] std::array<EnemyProjectile, MAX_PROJECTILES>& getProjectiles() noexcept { return m_projectiles; }
    [[nodiscard]] const std::array<EnemyProjectile, MAX_PROJECTILES>& getProjectiles() const noexcept { return m_projectiles; }

    [[nodiscard]] std::array<IcbmMissile, MAX_ICBMS>& getIcbms() noexcept { return m_icbms; }
    [[nodiscard]] const std::array<IcbmMissile, MAX_ICBMS>& getIcbms() const noexcept { return m_icbms; }

    [[nodiscard]] IcbmMissile& getIcbm() noexcept { return m_icbms[0]; }
    [[nodiscard]] const IcbmMissile& getIcbm() const noexcept { return m_icbms[0]; }

    [[nodiscard]] size_t getActiveEnemyCount() const noexcept;
    [[nodiscard]] size_t getActiveProjectileCount() const noexcept;

    // Utilidad de cálculo de puntuación reglamentaria (tope 1,500 pts)
    [[nodiscard]] static uint64_t calculateKillScore(EnemyType type, int level) noexcept;

private:
    float nextRandomFloat() noexcept;

    std::array<Enemy, MAX_ENEMIES> m_enemies{};
    std::array<EnemyProjectile, MAX_PROJECTILES> m_projectiles{};
    std::array<IcbmMissile, MAX_ICBMS> m_icbms{};

    float m_spawnTimer{0.5f};
    float m_icbmCooldown{9.0f};
    uint32_t m_rngState{987654321u};
    size_t m_nextEnemyIndex{0};
    size_t m_nextProjectileIndex{0};
    uint32_t m_enemySpawnCounter{0};
    int m_currentLevel{1};
};

} // namespace Entities
