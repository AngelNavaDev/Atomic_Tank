#pragma once

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_mixer.h>
#endif
#include <cstdint>
#include <cstddef>

namespace Core {

class AudioManager {
public:
    static AudioManager& getInstance() noexcept {
        static AudioManager instance;
        return instance;
    }

    bool init();
    void shutdown();

    void playShoot();
    void playExplosion(bool large = false);
    void playHit();
    void playPowerUp();
    void playAlert();
    void playNuke();
    void stopAllSfx();

    void startMusic();
    void stopMusic();
    void pauseMusic();
    void resumeMusic();

    void setSfxVolume(int volume);   // Rango 0 .. 100
    void setMusicVolume(int volume); // Rango 0 .. 100

    [[nodiscard]] int getSfxVolume() const noexcept { return m_sfxVolume; }
    [[nodiscard]] int getMusicVolume() const noexcept { return m_musicVolume; }
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    AudioManager() = default;
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    Mix_Chunk* createWavChunk(const int16_t* samples, size_t numSamples);

    bool m_initialized{false};
    int m_sfxVolume{80};
    int m_musicVolume{80};

    uint32_t m_lastShootTick{0};
    uint32_t m_lastHitTick{0};
    uint32_t m_lastExplosionTick{0};
    uint32_t m_lastLargeExplosionTick{0};
    uint32_t m_lastPowerUpTick{0};
    uint32_t m_lastAlertTick{0};
    int m_explosionChannelToggle{0};

    Mix_Chunk* m_shootChunk{nullptr};
    Mix_Chunk* m_explosionChunk{nullptr};
    Mix_Chunk* m_largeExplosionChunk{nullptr};
    Mix_Chunk* m_hitChunk{nullptr};
    Mix_Chunk* m_powerUpChunk{nullptr};
    Mix_Chunk* m_alertChunk{nullptr};
    Mix_Chunk* m_nukeChunk{nullptr};
    Mix_Chunk* m_musicChunk{nullptr};
};

} // namespace Core

