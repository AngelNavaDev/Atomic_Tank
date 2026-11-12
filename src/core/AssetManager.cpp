#include "core/AssetManager.hpp"
#include <iostream>
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

SDL_Surface* AssetManager::loadSurfaceWithFallbacks(const char* filename) {
    std::string baseName(filename);
    if (baseName.size() > 4 &&
        (baseName.substr(baseName.size() - 4) == ".svg" || baseName.substr(baseName.size() - 4) == ".png")) {
        baseName = baseName.substr(0, baseName.size() - 4);
    }

    const std::array<std::string, 6> candidates = {{
        baseName + ".png",
        "assets/" + baseName + ".png",
        "../assets/" + baseName + ".png",
        baseName + ".svg",
        "assets/" + baseName + ".svg",
        "../assets/" + baseName + ".svg"
    }};

    for (const auto& path : candidates) {
        SDL_Surface* surf = IMG_Load(path.c_str());
        if (surf) {
            return surf;
        }
    }

    std::cerr << "[ERROR ASSET] No se pudo cargar: " << filename << " | SDL_image error: " << IMG_GetError() << '\n';
    return nullptr;
}

bool AssetManager::init(SDL_Renderer* renderer) {
    if (m_initialized) {
        return true;
    }

    if (!renderer) {
        std::cerr << "[ERROR ASSET] Renderer nulo proporcionado a AssetManager::init().\n";
        return false;
    }

    const int flags = IMG_INIT_PNG;
    if ((IMG_Init(flags) & flags) != flags) {
        std::cerr << "[AVISO ASSET] IMG_Init advertencia: " << IMG_GetError() << '\n';
    }

    // 1. Cargar Atlas de Sprites (Tanque, Municiones y 25 Aeronaves)
    SDL_Surface* spriteSurf = loadSurfaceWithFallbacks("planilla_sprites_tanque_aviones.svg");
    if (spriteSurf) {
        // Limpieza de píxeles en bordes de celdas por si vinieran con fondo blanco opaco
        if (spriteSurf->format->BytesPerPixel == 4) {
            Uint32* pixels = static_cast<Uint32*>(spriteSurf->pixels);
            const int pitch32 = spriteSurf->pitch / 4;
            for (int y = 0; y < spriteSurf->h; ++y) {
                for (int x = 0; x < spriteSurf->w; ++x) {
                    const int cx = x % 128;
                    const int cy = y % 128;
                    if (cx < 4 || cx > 124 || cy < 4 || cy > 124) {
                        Uint8 r, g, b, a;
                        SDL_GetRGBA(pixels[y * pitch32 + x], spriteSurf->format, &r, &g, &b, &a);
                        if (r > 250 && g > 250 && b > 250) {
                            pixels[y * pitch32 + x] = SDL_MapRGBA(spriteSurf->format, 0, 0, 0, 0);
                        }
                    }
                }
            }
        }

        m_spriteSheet = SDL_CreateTextureFromSurface(renderer, spriteSurf);
        if (m_spriteSheet) {
            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(m_spriteSheet, nullptr, nullptr, &texW, &texH);
            m_spriteCellW = texW / 5;
            m_spriteCellH = texH / 6;
            SDL_SetTextureScaleMode(m_spriteSheet, SDL_ScaleModeNearest);
            std::cout << "[ASSET] Planilla Sprites cargada (" << texW << "x" << texH 
                      << ") - Celdas: " << m_spriteCellW << "x" << m_spriteCellH << " (5x6).\n";
        }
        SDL_FreeSurface(spriteSurf);
    }

    // 2. Cargar Atlas de Explosiones (8 columnas x 2 filas)
    SDL_Surface* expSurf = loadSurfaceWithFallbacks("planilla_explosiones.svg");
    if (expSurf) {
        if (expSurf->format->BytesPerPixel == 4) {
            Uint32* pixels = static_cast<Uint32*>(expSurf->pixels);
            const int pitch32 = expSurf->pitch / 4;
            for (int y = 0; y < expSurf->h; ++y) {
                for (int x = 0; x < expSurf->w; ++x) {
                    const int cx = x % 128;
                    const int cy = y % 128;
                    if (cx < 4 || cx > 124 || cy < 4 || cy > 124) {
                        Uint8 r, g, b, a;
                        SDL_GetRGBA(pixels[y * pitch32 + x], expSurf->format, &r, &g, &b, &a);
                        if (r > 250 && g > 250 && b > 250) {
                            pixels[y * pitch32 + x] = SDL_MapRGBA(expSurf->format, 0, 0, 0, 0);
                        }
                    }
                }
            }
        }

        m_explosionSheet = SDL_CreateTextureFromSurface(renderer, expSurf);
        if (m_explosionSheet) {
            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(m_explosionSheet, nullptr, nullptr, &texW, &texH);
            m_explosionCellW = texW / 8;
            m_explosionCellH = texH / 2;
            SDL_SetTextureScaleMode(m_explosionSheet, SDL_ScaleModeNearest);
            std::cout << "[ASSET] Planilla Explosiones cargada (" << texW << "x" << texH 
                      << ") - Frames: " << m_explosionCellW << "x" << m_explosionCellH << " (8x2).\n";
        }
        SDL_FreeSurface(expSurf);
    }

    // 3. Cargar Atlas de Fondos Pixel Art (5 columnas x 5 filas = 25 niveles)
    SDL_Surface* bgSurf = loadSurfaceWithFallbacks("planilla_fondos_25_niveles.svg");
    if (bgSurf) {
        m_backgroundSheet = SDL_CreateTextureFromSurface(renderer, bgSurf);
        if (m_backgroundSheet) {
            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(m_backgroundSheet, nullptr, nullptr, &texW, &texH);
            m_bgCellW = texW / 5;
            m_bgCellH = texH / 5;
            SDL_SetTextureScaleMode(m_backgroundSheet, SDL_ScaleModeNearest);
            std::cout << "[ASSET] Planilla Fondos Pixel Art cargada (" << texW << "x" << texH
                      << ") - Celdas: " << m_bgCellW << "x" << m_bgCellH << " (5x5, 25 niveles).\n";
        }
        SDL_FreeSurface(bgSurf);
    }

    m_initialized = (m_spriteSheet != nullptr && m_explosionSheet != nullptr && m_backgroundSheet != nullptr);
    return m_initialized;
}

void AssetManager::shutdown() {
    if (m_spriteSheet) {
        SDL_DestroyTexture(m_spriteSheet);
        m_spriteSheet = nullptr;
    }
    if (m_explosionSheet) {
        SDL_DestroyTexture(m_explosionSheet);
        m_explosionSheet = nullptr;
    }
    if (m_backgroundSheet) {
        SDL_DestroyTexture(m_backgroundSheet);
        m_backgroundSheet = nullptr;
    }
    IMG_Quit();
    m_initialized = false;
    std::cout << "[ASSET] AssetManager apagado y texturas liberadas.\n";
}

} // namespace Core

