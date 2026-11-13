#include "core/AudioManager.hpp"
#include <iostream>
#include <vector>
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

namespace Core {

namespace {

#pragma pack(push, 1)
struct WavHeader {
    char riff[4]{'R', 'I', 'F', 'F'};
    uint32_t fileSize{0};
    char wave[4]{'W', 'A', 'V', 'E'};
    char fmt[4]{'f', 'm', 't', ' '};
    uint32_t fmtSize{16};
    uint16_t audioFormat{1}; // PCM
    uint16_t numChannels{1}; // Mono
    uint32_t sampleRate{44100};
    uint32_t byteRate{44100 * 2};
    uint16_t blockAlign{2};
    uint16_t bitsPerSample{16};
    char data[4]{'d', 'a', 't', 'a'};
    uint32_t dataSize{0};
};
#pragma pack(pop)

constexpr float TWO_PI = 6.28318530717958647692f;
constexpr uint32_t SAMPLE_RATE = 44100;

// Generador pseudoaleatorio de ruido determinista y rápido
inline float pseudoNoise(uint32_t& rngState) {
    rngState = rngState * 1664525u + 1013904223u;
    return (static_cast<float>(rngState & 0xFFFFu) / 32767.5f) - 1.0f;
}

} // namespace

AudioManager::~AudioManager() {
    shutdown();
}

Mix_Chunk* AudioManager::createWavChunk(const int16_t* samples, size_t numSamples) {
    if (!samples || numSamples == 0) {
        return nullptr;
    }

    std::vector<int16_t> smoothed(samples, samples + numSamples);
    const size_t rampSamples = std::min<size_t>(128, numSamples / 4);
    for (size_t i = 0; i < rampSamples; ++i) {
        const float factor = static_cast<float>(i) / static_cast<float>(rampSamples);
        smoothed[i] = static_cast<int16_t>(static_cast<float>(smoothed[i]) * factor);
        smoothed[numSamples - 1 - i] = static_cast<int16_t>(static_cast<float>(smoothed[numSamples - 1 - i]) * factor);
    }

    const uint32_t dataBytes = static_cast<uint32_t>(numSamples * sizeof(int16_t));
    WavHeader header{};
    header.fileSize = 36 + dataBytes;
    header.dataSize = dataBytes;
    header.sampleRate = SAMPLE_RATE;
    header.byteRate = SAMPLE_RATE * 2;

    std::vector<uint8_t> wavBuffer(sizeof(WavHeader) + dataBytes);
    std::memcpy(wavBuffer.data(), &header, sizeof(WavHeader));
    std::memcpy(wavBuffer.data() + sizeof(WavHeader), smoothed.data(), dataBytes);

    SDL_RWops* rw = SDL_RWFromConstMem(wavBuffer.data(), static_cast<int>(wavBuffer.size()));
    if (!rw) {
        return nullptr;
    }

    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 1);
    return chunk;
}

bool AudioManager::init() {
    if (m_initialized) {
        return true;
    }

    if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16SYS, 2, 2048) != 0) {
        std::cerr << "[WARNING] Fallo al inicializar SDL2_mixer (" << Mix_GetError() << "). El juego continuará en silencio.\n";
        m_initialized = false;
        return false;
    }

    // 16 canales con asignación dedicada para evitar saturación/solapamiento:
    // Canal 0: Música | Canal 1: Disparo | Canal 2: Impacto | Canales 3-4: Explosión
    // Canal 5: Explosión Grande | Canal 6: PowerUp | Canal 7: Alerta | Canal 8: Nuke
    Mix_AllocateChannels(16);
    Mix_ReserveChannels(9);

    uint32_t rng = 987654321u;

    // -------------------------------------------------------------------------
    // 1. DISPARO DEL TANQUE (playShoot): 48ms (240Hz -> 95Hz suave, sin saturar)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.048f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float phase = 0.0f;
        float filteredNoise = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float env = std::pow(1.0f - t, 2.0f);
            const float freq = 240.0f * (1.0f - t) + 95.0f * t;

            phase += TWO_PI * freq / static_cast<float>(SAMPLE_RATE);
            if (phase > TWO_PI) phase -= TWO_PI;

            const float tone = std::sin(phase) * 0.75f + std::sin(phase * 2.0f) * 0.25f;
            filteredNoise += 0.25f * (pseudoNoise(rng) - filteredNoise);
            const float mixed = (tone * 0.72f + filteredNoise * 0.28f) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 7500.0f, -32767.0f, 32767.0f));
        }
        m_shootChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 2. EXPLOSIÓN NORMAL (playExplosion(false)): 260ms (ruido filtrado + golpe)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.26f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float filteredNoise = 0.0f;
        float lpOut = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float env = std::pow(1.0f - t, 2.3f);
            const float rawNoise = pseudoNoise(rng);

            filteredNoise += 0.14f * (rawNoise - filteredNoise);
            lpOut += 0.30f * (filteredNoise - lpOut);
            const float punch = std::sin(t * 95.0f * TWO_PI) * (1.0f - t);
            const float mixed = (lpOut * 0.70f + punch * 0.30f) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 8500.0f, -32767.0f, 32767.0f));
        }
        m_explosionChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 3. EXPLOSIÓN GRANDE / JEFE (playExplosion(true)): 480ms
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.48f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float filteredNoise = 0.0f;
        float lpOut = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float env = std::pow(1.0f - t, 1.8f);
            const float rawNoise = pseudoNoise(rng);

            filteredNoise += 0.10f * (rawNoise - filteredNoise);
            lpOut += 0.25f * (filteredNoise - lpOut);
            const float rumble = std::sin(t * 80.0f * TWO_PI) * (1.0f - t * 0.7f);
            const float mixed = (lpOut * 0.68f + rumble * 0.32f) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 9500.0f, -32767.0f, 32767.0f));
        }
        m_largeExplosionChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 4. IMPACTO METÁLICO (playHit): 28ms (460Hz -> 190Hz limpio)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.028f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float phase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float env = (1.0f - t) * (1.0f - t);
            const float freq = 460.0f * (1.0f - t) + 190.0f * t;

            phase += TWO_PI * freq / static_cast<float>(SAMPLE_RATE);
            if (phase > TWO_PI) phase -= TWO_PI;

            const float tone = std::sin(phase) * 0.80f + std::sin(phase * 2.0f) * 0.20f;
            const float mixed = tone * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 6000.0f, -32767.0f, 32767.0f));
        }
        m_hitChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 5. POWER-UP (playPowerUp): 260ms (Arpegio ascendente 440, 554, 659, 880Hz)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.26f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        const float freqs[4] = {440.0f, 554.37f, 659.25f, 880.0f};
        const size_t noteSamples = numSamples / 4;
        float phase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const size_t noteIdx = std::min(size_t{3}, i / noteSamples);
            const float freq = freqs[noteIdx];
            const size_t subI = i % noteSamples;
            const float tNote = static_cast<float>(subI) / static_cast<float>(noteSamples);
            const float attack = std::min(1.0f, tNote * 12.0f);
            const float env = attack * (1.0f - tNote * 0.50f);

            phase += TWO_PI * freq / static_cast<float>(SAMPLE_RATE);
            if (phase > TWO_PI) phase -= TWO_PI;

            const float tone = std::sin(phase) * 0.80f + std::sin(phase * 2.0f) * 0.20f;
            const float mixed = tone * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 7500.0f, -32767.0f, 32767.0f));
        }
        m_powerUpChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 6. ALERTA ICBM / BOSS (playAlert): 320ms (Sirena senoidal suave)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.32f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float phase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float sec = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
            const float freq = 780.0f + 180.0f * std::sin(sec * TWO_PI * 6.0f);

            phase += TWO_PI * freq / static_cast<float>(SAMPLE_RATE);
            if (phase > TWO_PI) phase -= TWO_PI;

            const float tone = std::sin(phase) * 0.80f + std::sin(phase * 2.0f) * 0.20f;
            const float env = (1.0f - t * 0.35f);

            samples[i] = static_cast<int16_t>(std::clamp(tone * env * 7000.0f, -32767.0f, 32767.0f));
        }
        m_alertChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 7. DETONACIÓN NUKE (playNuke): 850ms (Estruendo profundo controlado)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.85f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float filteredNoise = 0.0f;
        float lpOut = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float rawNoise = pseudoNoise(rng);

            const float filterAlpha = 0.12f * (1.0f - t * 0.65f);
            filteredNoise += filterAlpha * (rawNoise - filteredNoise);
            lpOut += 0.25f * (filteredNoise - lpOut);

            const float bassTone = std::sin(t * 75.0f * TWO_PI) * (1.0f - t * 0.5f);
            const float env = std::pow(1.0f - t, 1.6f);
            const float mixed = (lpOut * 0.70f + bassTone * 0.30f) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 10500.0f, -32767.0f, 32767.0f));
        }
        m_nukeChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 8. LOOP MUSICAL ARCADE (125 BPM, Mi Menor, 16 Tiempos = 7.68 segundos)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(7.68f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);

        // Octava media-baja limpia (E3..D4) para no saturar altavoces móviles
        constexpr float E3 = 164.81f;
        constexpr float G3 = 196.00f;
        constexpr float A3 = 220.00f;
        constexpr float B3 = 246.94f;
        constexpr float D4 = 293.66f;

        const float bassline[64] = {
            E3, E3, G3, E3,   A3, E3, G3, B3,   E3, E3, G3, E3,   D4, B3, A3, G3,
            E3, E3, G3, E3,   A3, E3, G3, B3,   E3, E3, G3, E3,   B3, A3, G3, E3,
            E3, E3, G3, E3,   A3, E3, G3, B3,   E3, E3, G3, E3,   D4, B3, A3, G3,
            E3, E3, G3, E3,   A3, E3, G3, B3,   E3, E3, G3, E3,   G3, A3, B3, D4
        };

        const size_t samplesPerStep = numSamples / 64;
        float bassPhase = 0.0f;
        float subPhase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const size_t step = std::min(size_t{63}, i / samplesPerStep);
            const size_t stepSample = i % samplesPerStep;
            const float stepT = static_cast<float>(stepSample) / static_cast<float>(samplesPerStep);
            const float noteFreq = bassline[step];

            bassPhase += TWO_PI * noteFreq / static_cast<float>(SAMPLE_RATE);
            if (bassPhase > TWO_PI) bassPhase -= TWO_PI;

            subPhase += TWO_PI * (noteFreq * 0.5f) / static_cast<float>(SAMPLE_RATE);
            if (subPhase > TWO_PI) subPhase -= TWO_PI;

            // Envolvente suave de ataque y caída en cada nota para eliminar clics
            const float attack = std::min(1.0f, stepT * 18.0f);
            const float release = std::max(0.0f, 1.0f - stepT * 0.95f);
            const float bassEnv = attack * release;

            const float synthWave = std::sin(bassPhase) * 0.65f + std::sin(bassPhase * 2.0f) * 0.20f + std::sin(subPhase) * 0.15f;
            const float bassOut = synthWave * bassEnv;

            float kickOut = 0.0f;
            if (step % 4 == 0) {
                const float kickT = static_cast<float>(stepSample) / static_cast<float>(samplesPerStep * 2);
                if (kickT < 1.0f) {
                    const float kickFreq = 135.0f * (1.0f - kickT * 0.65f);
                    const float kickPhase = TWO_PI * kickFreq * kickT;
                    kickOut = std::sin(kickPhase) * (1.0f - kickT) * (1.0f - kickT);
                }
            }

            float hiHatOut = 0.0f;
            if (step % 2 == 1 && stepSample < samplesPerStep / 4) {
                const float hhEnv = 1.0f - static_cast<float>(stepSample) / static_cast<float>(samplesPerStep / 4);
                hiHatOut = pseudoNoise(rng) * hhEnv * hhEnv * 0.10f;
            }

            const float totalMix = (bassOut * 0.58f + kickOut * 0.28f + hiHatOut);
            samples[i] = static_cast<int16_t>(std::clamp(totalMix * 5500.0f, -32767.0f, 32767.0f));
        }

        m_musicChunk = createWavChunk(samples.data(), numSamples);
    }

    m_initialized = true;
    setSfxVolume(m_sfxVolume);
    setMusicVolume(m_musicVolume);

    std::cout << "[INFO] Motor de Audio y Síntesis Procedural inicializado (44.1 kHz, 16-bit, canales dedicados).\n";
    return true;
}

void AudioManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    stopAllSfx();
    stopMusic();

    if (m_shootChunk) { Mix_FreeChunk(m_shootChunk); m_shootChunk = nullptr; }
    if (m_explosionChunk) { Mix_FreeChunk(m_explosionChunk); m_explosionChunk = nullptr; }
    if (m_largeExplosionChunk) { Mix_FreeChunk(m_largeExplosionChunk); m_largeExplosionChunk = nullptr; }
    if (m_hitChunk) { Mix_FreeChunk(m_hitChunk); m_hitChunk = nullptr; }
    if (m_powerUpChunk) { Mix_FreeChunk(m_powerUpChunk); m_powerUpChunk = nullptr; }
    if (m_alertChunk) { Mix_FreeChunk(m_alertChunk); m_alertChunk = nullptr; }
    if (m_nukeChunk) { Mix_FreeChunk(m_nukeChunk); m_nukeChunk = nullptr; }
    if (m_musicChunk) { Mix_FreeChunk(m_musicChunk); m_musicChunk = nullptr; }

    Mix_CloseAudio();
    m_initialized = false;
}

void AudioManager::stopAllSfx() {
    if (!m_initialized) return;
    for (int ch = 1; ch < 16; ++ch) {
        Mix_HaltChannel(ch);
    }
}

void AudioManager::playShoot() {
    if (!m_initialized || !m_shootChunk || m_sfxVolume <= 0) return;
    const uint32_t now = SDL_GetTicks();
    if (now - m_lastShootTick < 55u) {
        return;
    }
    m_lastShootTick = now;
    Mix_PlayChannel(1, m_shootChunk, 0);
}

void AudioManager::playExplosion(bool large) {
    if (!m_initialized || m_sfxVolume <= 0) return;
    const uint32_t now = SDL_GetTicks();
    if (large) {
        if (!m_largeExplosionChunk || (now - m_lastLargeExplosionTick < 120u)) {
            return;
        }
        m_lastLargeExplosionTick = now;
        Mix_PlayChannel(5, m_largeExplosionChunk, 0);
    } else {
        if (!m_explosionChunk || (now - m_lastExplosionTick < 65u)) {
            return;
        }
        m_lastExplosionTick = now;
        const int ch = 3 + (m_explosionChannelToggle & 1);
        m_explosionChannelToggle ^= 1;
        Mix_PlayChannel(ch, m_explosionChunk, 0);
    }
}

void AudioManager::playHit() {
    if (!m_initialized || !m_hitChunk || m_sfxVolume <= 0) return;
    const uint32_t now = SDL_GetTicks();
    if (now - m_lastHitTick < 50u) {
        return;
    }
    m_lastHitTick = now;
    Mix_PlayChannel(2, m_hitChunk, 0);
}

void AudioManager::playPowerUp() {
    if (!m_initialized || !m_powerUpChunk || m_sfxVolume <= 0) return;
    const uint32_t now = SDL_GetTicks();
    if (now - m_lastPowerUpTick < 80u) {
        return;
    }
    m_lastPowerUpTick = now;
    Mix_PlayChannel(6, m_powerUpChunk, 0);
}

void AudioManager::playAlert() {
    if (!m_initialized || !m_alertChunk || m_sfxVolume <= 0) return;
    const uint32_t now = SDL_GetTicks();
    if (now - m_lastAlertTick < 250u) {
        return;
    }
    m_lastAlertTick = now;
    Mix_PlayChannel(7, m_alertChunk, 0);
}

void AudioManager::playNuke() {
    if (!m_initialized || !m_nukeChunk || m_sfxVolume <= 0) return;
    Mix_PlayChannel(8, m_nukeChunk, 0);
}

void AudioManager::startMusic() {
    if (!m_initialized || !m_musicChunk) return;
    // El canal 0 está reservado exclusivamente para la música ambiental
    if (Mix_Playing(0) == 0) {
        Mix_PlayChannel(0, m_musicChunk, -1);
    } else if (Mix_Paused(0) != 0) {
        Mix_Resume(0);
    }
}

void AudioManager::stopMusic() {
    if (!m_initialized) return;
    Mix_HaltChannel(0);
}

void AudioManager::pauseMusic() {
    if (!m_initialized) return;
    Mix_Pause(0);
}

void AudioManager::resumeMusic() {
    if (!m_initialized) return;
    Mix_Resume(0);
}

void AudioManager::setSfxVolume(int volume) {
    m_sfxVolume = std::clamp(volume, 0, 100);
    const int mixVol = (m_sfxVolume * MIX_MAX_VOLUME) / 100;
    // Configura el volumen para todos los canales de efectos (canales 1 al 15)
    for (int ch = 1; ch < 16; ++ch) {
        Mix_Volume(ch, mixVol);
    }
}

void AudioManager::setMusicVolume(int volume) {
    m_musicVolume = std::clamp(volume, 0, 100);
    const int mixVol = (m_musicVolume * MIX_MAX_VOLUME) / 100;
    // Configura el volumen del canal 0 (música)
    Mix_Volume(0, mixVol);
}

} // namespace Core

