#pragma once

#include "entities/Projectile.hpp"
#include "math/Vector2D.hpp"
#include <SDL2/SDL.h>
#include <array>
#include <cstddef>

namespace Entities {

class ProjectilePool {
public:
    static constexpr size_t POOL_SIZE{500};
    static constexpr float PROJECTILE_SPEED{1750.0f};
    static constexpr float MUZZLE_FLASH_DURATION{0.06f};

    // Límites de desactivación lógica
    static constexpr float BOUND_MIN_X{-50.0f};
    static constexpr float BOUND_MAX_X{1970.0f};
    static constexpr float BOUND_MIN_Y{-50.0f};
    static constexpr float BOUND_MAX_Y{1130.0f};

    ProjectilePool();
    ~ProjectilePool() = default;

    ProjectilePool(const ProjectilePool&) = delete;
    ProjectilePool& operator=(const ProjectilePool&) = delete;

    void update(float dt);
    void updateHoming(float dt, float targetX, float targetY);
    void render(SDL_Renderer* renderer) const;

    bool spawn(float startX, float startY, float dirX, float dirY, int damage = 5, bool isCritical = false, bool isHoming = false, bool isPiercing = false);
    void clear() noexcept;

    [[nodiscard]] size_t getActiveCount() const noexcept;

    [[nodiscard]] std::array<Projectile, POOL_SIZE>& getProjectiles() noexcept { return m_projectiles; }
    [[nodiscard]] const std::array<Projectile, POOL_SIZE>& getProjectiles() const noexcept { return m_projectiles; }

private:
    std::array<Projectile, POOL_SIZE> m_projectiles{};
    size_t m_nextIndex{0};

    // Estado del destello en boca de cañón (muzzle flash)
    float m_muzzleFlashTimer{0.0f};
    Math::Vector2D m_muzzlePos{0.0f, 0.0f};
    Math::Vector2D m_muzzleDir{0.0f, -1.0f};
};

} // namespace Entities
