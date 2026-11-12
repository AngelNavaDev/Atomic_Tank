#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace Entities {

class Tank;
class ParticlePool;

enum class PowerUpType : uint8_t {
    SpreadShot = 0,
    RapidFire,
    RepairKit,
    ShieldOvercharge,
    DamageBoost,        // Munición de Uranio (daño = 20)
    MegalaserPiece      // Pieza de Megalaser (1/4 a 4/4)
};

struct PowerUp {
    float x{0.0f};
    float y{0.0f};
    float vy{85.0f};
    float swayTimer{0.0f};
    PowerUpType type{PowerUpType::SpreadShot};
    bool active{false};
    float groundLife{8.0f}; // Tiempo que permanece en el suelo
};

struct AlliedSupplyHeli {
    float x{0.0f};
    float y{110.0f};
    float vx{160.0f};
    int hp{40};
    int maxHp{40};
    float hitFlashTimer{0.0f};
    float rotorAngle{0.0f};
    bool active{false};
    bool hasDroppedCrate{false};
    PowerUpType pendingType{PowerUpType::SpreadShot};

    [[nodiscard]] SDL_FRect getBoundingBox() const noexcept {
        return {x - 38.0f, y - 16.0f, 76.0f, 32.0f};
    }
};

class PowerUpPool {
public:
    static constexpr size_t POOL_SIZE{16};

    PowerUpPool();
    ~PowerUpPool() = default;

    PowerUpPool(const PowerUpPool&) = delete;
    PowerUpPool& operator=(const PowerUpPool&) = delete;

    void update(float dt);
    void render(SDL_Renderer* renderer) const;

    bool spawnCrate(float x, float y, PowerUpType type);
    void triggerSupplyHeli(PowerUpType type, int direction = 1);
    bool checkFriendlyFire(float bx, float by, float br, int dmg, ParticlePool& particlePool, uint64_t& score, float& screenShake);

    void checkCollisionsWithTank(Tank& tank, ParticlePool& particlePool);
    void clear() noexcept;

    [[nodiscard]] size_t getActiveCount() const noexcept;
    [[nodiscard]] const AlliedSupplyHeli& getSupplyHeli() const noexcept { return m_supplyHeli; }

private:
    std::array<PowerUp, POOL_SIZE> m_powerUps{};
    AlliedSupplyHeli m_supplyHeli{};
    size_t m_nextIndex{0};
};

} // namespace Entities
