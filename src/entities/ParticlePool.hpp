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

struct Particle {
    float x{0.0f};
    float y{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    float size{8.0f};
    float life{0.0f};
    float maxLife{0.5f};
    bool active{false};
};

struct SpriteExplosion {
    float x{0.0f};
    float y{0.0f};
    float timer{0.0f};
    float scale{1.0f};
    int currentFrame{0};
    int row{0};          // 0 = Fuego completa, 1 = Onda expansiva de impacto
    bool active{false};
};

struct ShellCasing {
    float x{0.0f};
    float y{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    float life{0.0f};
    float maxLife{1.8f};
    int bounces{0};
    bool active{false};
};

struct WeatherParticle {
    float x{0.0f};
    float y{0.0f};
    float vx{0.0f};
    float vy{0.0f};
    float size{2.0f};
    int type{0};         // 0: Nieve, 1: Lluvia, 2: Ascuas, 3: Polvo
    bool active{false};
};

class ParticlePool {
public:
    static constexpr size_t POOL_SIZE{600};
    static constexpr size_t MAX_SPRITE_EXPLOSIONS{64};
    static constexpr size_t MAX_SHELL_CASINGS{128};
    static constexpr size_t MAX_WEATHER_PARTICLES{80};

    ParticlePool();
    ~ParticlePool() = default;

    ParticlePool(const ParticlePool&) = delete;
    ParticlePool& operator=(const ParticlePool&) = delete;

    void update(float dt);
    void updateWeather(float dt, int level);
    void render(SDL_Renderer* renderer) const;
    void renderWeather(SDL_Renderer* renderer) const;
    void renderShellCasings(SDL_Renderer* renderer) const;

    void spawnExplosion(float x, float y, int count, float speed, bool isLarge);
    void spawnShellCasing(float x, float y, float dirX, float dirY);
    void clear() noexcept;

    [[nodiscard]] size_t getActiveCount() const noexcept;
    [[nodiscard]] size_t getActiveSpriteExplosionsCount() const noexcept;
    [[nodiscard]] size_t getActiveShellCasingsCount() const noexcept;

private:
    float nextRandomFloat() noexcept;

    std::array<Particle, POOL_SIZE> m_particles{};
    std::array<SpriteExplosion, MAX_SPRITE_EXPLOSIONS> m_spriteExplosions{};
    std::array<ShellCasing, MAX_SHELL_CASINGS> m_shellCasings{};
    std::array<WeatherParticle, MAX_WEATHER_PARTICLES> m_weatherParticles{};
    size_t m_nextIndex{0};
    size_t m_nextSpriteExplosionIndex{0};
    size_t m_nextShellCasingIndex{0};
    int m_currentWeatherLevel{-1};
    uint32_t m_rngState{123456789u};
};

} // namespace Entities

