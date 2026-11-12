#include "core/AudioManager.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>

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

    const uint32_t dataBytes = static_cast<uint32_t>(numSamples * sizeof(int16_t));
    WavHeader header{};
    header.fileSize = 36 + dataBytes;
    header.dataSize = dataBytes;
    header.sampleRate = SAMPLE_RATE;
    header.byteRate = SAMPLE_RATE * 2;

    std::vector<uint8_t> wavBuffer(sizeof(WavHeader) + dataBytes);
    std::memcpy(wavBuffer.data(), &header, sizeof(WavHeader));
    std::memcpy(wavBuffer.data() + sizeof(WavHeader), samples, dataBytes);

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

    if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16SYS, 2, 1024) != 0) {
        std::cerr << "[WARNING] Fallo al inicializar SDL2_mixer (" << Mix_GetError() << "). El juego continuará en silencio.\n";
        m_initialized = false;
        return false;
    }

    // 16 canales: Canal 0 reservado exclusivamente para la música
    Mix_AllocateChannels(16);
    Mix_ReserveChannels(1);

    uint32_t rng = 987654321u;

    // -------------------------------------------------------------------------
    // 1. DISPARO DEL TANQUE (playShoot): 55ms (280Hz -> 70Hz + ruido metálico)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.055f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float phase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float env = (1.0f - t) * (1.0f - t);
            const float freq = 280.0f * (1.0f - t) + 70.0f * t;

            phase += TWO_PI * freq / static_cast<float>(SAMPLE_RATE);
            if (phase > TWO_PI) phase -= TWO_PI;

            const float squareWave = (std::sin(phase) >= 0.0f) ? 1.0f : -1.0f;
            const float noise = pseudoNoise(rng);
            const float mixed = (squareWave * 0.60f + noise * 0.40f) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 26000.0f, -32767.0f, 32767.0f));
        }
        m_shootChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 2. EXPLOSIÓN NORMAL (playExplosion(false)): 350ms (ruido filtrado + golpe)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.35f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float filteredNoise = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float env = std::pow(1.0f - t, 2.2f);
            const float rawNoise = pseudoNoise(rng);

            filteredNoise += 0.14f * (rawNoise - filteredNoise);
            const float subThump = std::sin(t * 55.0f * TWO_PI) * (1.0f - t);
            const float mixed = (filteredNoise * 0.72f + subThump * 0.28f) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 28000.0f, -32767.0f, 32767.0f));
        }
        m_explosionChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 3. EXPLOSIÓN GRANDE / JEFE (playExplosion(true)): 700ms (estruendo profundo)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.70f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float filteredNoise = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float env = std::pow(1.0f - t, 1.6f);
            const float rawNoise = pseudoNoise(rng);

            filteredNoise += 0.08f * (rawNoise - filteredNoise);
            const float subRumble = std::sin(t * 36.0f * TWO_PI) * (1.0f - t * 0.7f);
            const float mixed = (filteredNoise * 0.68f + subRumble * 0.32f) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 30000.0f, -32767.0f, 32767.0f));
        }
        m_largeExplosionChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 4. IMPACTO METÁLICO (playHit): 35ms (520Hz -> 180Hz)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.035f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float phase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float env = (1.0f - t) * (1.0f - t);
            const float freq = 520.0f * (1.0f - t) + 180.0f * t;

            phase += TWO_PI * freq / static_cast<float>(SAMPLE_RATE);
            if (phase > TWO_PI) phase -= TWO_PI;

            const float tone = std::sin(phase) + 0.35f * std::sin(phase * 2.7f);
            const float noise = pseudoNoise(rng) * 0.25f;
            const float mixed = (tone * 0.75f + noise) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 24000.0f, -32767.0f, 32767.0f));
        }
        m_hitChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 5. POWER-UP (playPowerUp): 280ms (Arpegio ascendente 440, 554, 659, 880Hz)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.28f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        const float freqs[4] = {440.0f, 554.37f, 659.25f, 880.0f};
        const size_t noteSamples = numSamples / 4;
        float phase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const size_t noteIdx = std::min(size_t{3}, i / noteSamples);
            const float freq = freqs[noteIdx];
            const size_t subI = i % noteSamples;
            const float tNote = static_cast<float>(subI) / static_cast<float>(noteSamples);
            const float env = 1.0f - tNote * 0.45f;

            phase += TWO_PI * freq / static_cast<float>(SAMPLE_RATE);
            if (phase > TWO_PI) phase -= TWO_PI;

            // Onda rica con segundo armónico
            const float tone = std::sin(phase) * 0.75f + std::sin(phase * 2.0f) * 0.25f;
            const float mixed = tone * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 22000.0f, -32767.0f, 32767.0f));
        }
        m_powerUpChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 6. ALERTA ICBM / BOSS (playAlert): 400ms (Sirena militar 800Hz <-> 1200Hz)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(0.40f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float phase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float sec = static_cast<float>(i) / static_cast<float>(SAMPLE_RATE);
            // LFO a 6.5 Hz oscilando entre 800 y 1200 Hz
            const float lfo = (std::sin(sec * TWO_PI * 6.5f) >= 0.0f) ? 1200.0f : 800.0f;

            phase += TWO_PI * lfo / static_cast<float>(SAMPLE_RATE);
            if (phase > TWO_PI) phase -= TWO_PI;

            const float squareTone = (std::sin(phase) >= 0.0f) ? 0.70f : -0.70f;
            const float env = 1.0f - (static_cast<float>(i) / static_cast<float>(numSamples)) * 0.25f;

            samples[i] = static_cast<int16_t>(std::clamp(squareTone * env * 22000.0f, -32767.0f, 32767.0f));
        }
        m_alertChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 7. DETONACIÓN NUKE (playNuke): 1.2s (Estruendo nuclear profundo con choque)
    // -------------------------------------------------------------------------
    {
        constexpr size_t numSamples = static_cast<size_t>(1.20f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);
        float filteredNoise = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(numSamples);
            const float rawNoise = pseudoNoise(rng);

            // Filtro dinámico: paso bajo que se va cerrando a frecuencias sub-graves
            const float filterAlpha = 0.12f * (1.0f - t * 0.7f);
            filteredNoise += filterAlpha * (rawNoise - filteredNoise);

            const float subBass = std::sin(t * (35.0f - t * 12.0f) * TWO_PI);
            const float env = std::pow(1.0f - t, 1.4f);
            const float mixed = (filteredNoise * 0.70f + subBass * 0.30f) * env;

            samples[i] = static_cast<int16_t>(std::clamp(mixed * 31000.0f, -32767.0f, 32767.0f));
        }
        m_nukeChunk = createWavChunk(samples.data(), numSamples);
    }

    // -------------------------------------------------------------------------
    // 8. LOOP MUSICAL ARCADE (125 BPM, Mi Menor, 16 Tiempos = 7.68 segundos)
    // -------------------------------------------------------------------------
    {
        // 125 BPM -> 0.48s por negra -> 16 negras = 7.68s -> 338,688 muestras
        constexpr size_t numSamples = static_cast<size_t>(7.68f * SAMPLE_RATE);
        std::vector<int16_t> samples(numSamples);

        // Frecuencias de bajo en Mi menor: E2 (82.4Hz), G2 (98.0Hz), A2 (110.0Hz), B2 (123.5Hz), D3 (146.8Hz)
        constexpr float E2 = 82.41f;
        constexpr float G2 = 97.99f;
        constexpr float A2 = 110.00f;
        constexpr float B2 = 123.47f;
        constexpr float D3 = 146.83f;

        // Secuencia de 16 semicorcheas que se repite con variaciones (64 semicorcheas en total)
        const float bassline[64] = {
            E2, E2, G2, E2,   A2, E2, G2, B2,   E2, E2, G2, E2,   D3, B2, A2, G2,
            E2, E2, G2, E2,   A2, E2, G2, B2,   E2, E2, G2, E2,   B2, A2, G2, E2,
            E2, E2, G2, E2,   A2, E2, G2, B2,   E2, E2, G2, E2,   D3, B2, A2, G2,
            E2, E2, G2, E2,   A2, E2, G2, B2,   E2, E2, G2, E2,   G2, A2, B2, D3
        };

        const size_t samplesPerStep = numSamples / 64; // ~5292 muestras por semicorchea
        float bassPhase = 0.0f;
        float subPhase = 0.0f;

        for (size_t i = 0; i < numSamples; ++i) {
            const size_t step = std::min(size_t{63}, i / samplesPerStep);
            const size_t stepSample = i % samplesPerStep;
            const float stepT = static_cast<float>(stepSample) / static_cast<float>(samplesPerStep);
            const float noteFreq = bassline[step];

            // 1. Sintetizador de Bajo: Onda diente de sierra + sub-oscilador cuadrado con envolvente de caída
            bassPhase += TWO_PI * noteFreq / static_cast<float>(SAMPLE_RATE);
            if (bassPhase > TWO_PI) bassPhase -= TWO_PI;

            subPhase += TWO_PI * (noteFreq * 0.5f) / static_cast<float>(SAMPLE_RATE);
            if (subPhase > TWO_PI) subPhase -= TWO_PI;

            // Diente de sierra aproximada
            const float saw = (bassPhase / 3.14159f) - 1.0f;
            const float subSquare = (std::sin(subPhase) >= 0.0f) ? 0.5f : -0.5f;
            const float bassEnv = (1.0f - stepT * 0.85f);
            const float bassOut = (saw * 0.65f + subSquare * 0.35f) * bassEnv;

            // 2. Bombo rítmico (Kick) en cada tiempo fuerte (pasos 0, 4, 8, 12, etc.)
            float kickOut = 0.0f;
            if (step % 4 == 0) {
                const float kickT = static_cast<float>(stepSample) / static_cast<float>(samplesPerStep * 2);
                if (kickT < 1.0f) {
                    const float kickFreq = 120.0f * (1.0f - kickT * 0.8f);
                    const float kickPhase = TWO_PI * kickFreq * kickT;
                    kickOut = std::sin(kickPhase) * (1.0f - kickT);
                }
            }

            // 3. Hi-Hat metálico sutil en contratiempo (pasos impares)
            float hiHatOut = 0.0f;
            if (step % 2 == 1 && stepSample < samplesPerStep / 3) {
                const float hhEnv = 1.0f - static_cast<float>(stepSample) / static_cast<float>(samplesPerStep / 3);
                hiHatOut = pseudoNoise(rng) * hhEnv * 0.18f;
            }

            const float totalMix = (bassOut * 0.60f + kickOut * 0.30f + hiHatOut);
            samples[i] = static_cast<int16_t>(std::clamp(totalMix * 18000.0f, -32767.0f, 32767.0f));
        }

        m_musicChunk = createWavChunk(samples.data(), numSamples);
    }

    m_initialized = true;
    setSfxVolume(m_sfxVolume);
    setMusicVolume(m_musicVolume);

    std::cout << "[INFO] Motor de Audio y Síntesis Procedural inicializado (44.1 kHz, 16-bit, 16 canales).\n";
    return true;
}

void AudioManager::shutdown() {
    if (!m_initialized) {
        return;
    }

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

void AudioManager::playShoot() {
    if (!m_initialized || !m_shootChunk) return;
    Mix_PlayChannel(-1, m_shootChunk, 0);
}

void AudioManager::playExplosion(bool large) {
    if (!m_initialized) return;
    Mix_Chunk* chunk = large ? m_largeExplosionChunk : m_explosionChunk;
    if (chunk) {
        Mix_PlayChannel(-1, chunk, 0);
    }
}

void AudioManager::playHit() {
    if (!m_initialized || !m_hitChunk) return;
    Mix_PlayChannel(-1, m_hitChunk, 0);
}

void AudioManager::playPowerUp() {
    if (!m_initialized || !m_powerUpChunk) return;
    Mix_PlayChannel(-1, m_powerUpChunk, 0);
}

void AudioManager::playAlert() {
    if (!m_initialized || !m_alertChunk) return;
    Mix_PlayChannel(-1, m_alertChunk, 0);
}

void AudioManager::playNuke() {
    if (!m_initialized || !m_nukeChunk) return;
    Mix_PlayChannel(-1, m_nukeChunk, 0);
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

