#pragma once

#include <SDL2/SDL.h>
#include <algorithm>
#include <cstdint>
#include <string_view>

namespace Core {

struct LevelPalette {
    SDL_Color skyTop;
    SDL_Color skyBottom;
    SDL_Color mountains;
    SDL_Color midLayer;
    SDL_Color groundBase;
    SDL_Color groundSurface;
    std::string_view sectorName;
};

class LevelManager {
public:
    static constexpr int MAX_LEVELS{25};
    static constexpr float BASE_WAVE_DURATION{95.0f}; // 95s base + (level * 3s)
    static constexpr float LEVEL_COMPLETE_DURATION{3.0f};

    LevelManager();
    ~LevelManager() = default;

    LevelManager(const LevelManager&) = delete;
    LevelManager& operator=(const LevelManager&) = delete;

    void init();
    void resetToLevel(int level);
    void update(float dt, bool bossActive);

    void onBossDefeated();
    void advanceLevel();
    void triggerBossQuickly() noexcept {
        m_missionTimer = m_waveDuration * 0.99f;
    }

    void renderParallaxBackground(SDL_Renderer* renderer) const;
    void renderGround(SDL_Renderer* renderer) const;
    void renderMissionBar(SDL_Renderer* renderer) const;
    void renderLevelCompleteBanner(SDL_Renderer* renderer) const;

    [[nodiscard]] int getCurrentLevel() const noexcept { return m_currentLevel; }
    [[nodiscard]] float getMissionProgress() const noexcept {
        return (m_waveDuration > 0.0f) ? std::clamp(m_missionTimer / m_waveDuration, 0.0f, 1.0f) : 1.0f;
    }
    [[nodiscard]] bool isWavePhaseDone() const noexcept {
        return m_missionTimer >= m_waveDuration;
    }
    [[nodiscard]] bool isLevelCompleteTransition() const noexcept {
        return m_levelCompleteTimer > 0.0f;
    }
    [[nodiscard]] const LevelPalette& getCurrentPalette() const noexcept {
        return m_currentPalette;
    }

private:
    void updatePalette();

    int m_currentLevel{1};
    float m_missionTimer{0.0f};
    float m_waveDuration{98.0f}; // 95.0f + level * 3.0f
    float m_levelCompleteTimer{0.0f};

    // Desplazamiento horizontal continuo (Parallax)
    float m_mountainScroll{0.0f};
    float m_midLayerScroll{0.0f};
    float m_groundScroll{0.0f};

    // Efectos climáticos y ambientales
    float m_lightningTimer{0.0f};
    float m_ambientAnimTimer{0.0f};

    LevelPalette m_currentPalette{};
};

} // namespace Core
