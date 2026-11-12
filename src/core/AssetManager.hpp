#pragma once

#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#endif
#include <cstdint>

namespace Core {

class AssetManager {
public:
    static AssetManager& getInstance() noexcept {
        static AssetManager instance;
        return instance;
    }

    bool init(SDL_Renderer* renderer);
    void shutdown();

    [[nodiscard]] bool isLoaded() const noexcept { return m_initialized; }
    [[nodiscard]] SDL_Texture* getSpriteSheet() const noexcept { return m_spriteSheet; }
    [[nodiscard]] SDL_Texture* getExplosionSheet() const noexcept { return m_explosionSheet; }
    [[nodiscard]] SDL_Texture* getBackgroundSheet() const noexcept { return m_backgroundSheet; }
    [[nodiscard]] SDL_Texture* getBackgroundAtlasTexture() const noexcept { return m_backgroundSheet; }

    [[nodiscard]] int getSpriteCellW() const noexcept { return m_spriteCellW; }
    [[nodiscard]] int getSpriteCellH() const noexcept { return m_spriteCellH; }

    [[nodiscard]] int getExplosionCellW() const noexcept { return m_explosionCellW; }
    [[nodiscard]] int getExplosionCellH() const noexcept { return m_explosionCellH; }

    [[nodiscard]] int getBackgroundCellW() const noexcept { return m_bgCellW; }
    [[nodiscard]] int getBackgroundCellH() const noexcept { return m_bgCellH; }

    // Recorte exacto de la celda de fondo para un nivel (1..25, cuadrícula de 5x5)
    [[nodiscard]] SDL_Rect getLevelBackgroundRect(int level) const noexcept {
        if (level < 1) level = 1;
        if (level > 25) level = 25;
        const int idx = level - 1;
        const int col = idx % 5;
        const int row = idx / 5;
        return {col * m_bgCellW, row * m_bgCellH, m_bgCellW, m_bgCellH};
    }

    // Recortes exactos para la Fila 0 (Jugador, Proyectiles, Suministros)
    [[nodiscard]] SDL_Rect getTankChassisSrc() const noexcept {
        return {9, 29, 110, 75};
    }

    [[nodiscard]] SDL_Rect getTankBarrelSrc() const noexcept {
        return {136, 49, 80, 30};
    }

    [[nodiscard]] SDL_Rect getBombSrc() const noexcept {
        return {296, 32, 48, 64};
    }

    [[nodiscard]] SDL_Rect getMissileSrc() const noexcept {
        return {424, 32, 48, 64};
    }

    [[nodiscard]] SDL_Rect getSupplyCrateSrc() const noexcept {
        return {544, 36, 64, 60};
    }

    // Celda genérica del atlas de sprites (cols 0..4, rows 0..5)
    [[nodiscard]] SDL_Rect getSpriteCellSrc(int col, int row) const noexcept {
        return {col * m_spriteCellW, row * m_spriteCellH, m_spriteCellW, m_spriteCellH};
    }

    // Aeronaves (Filas 1 a 5, 25 aeronaves, índices 0 al 24)
    [[nodiscard]] SDL_Rect getAircraftSrc(int aircraftIndex) const noexcept {
        if (aircraftIndex < 0) aircraftIndex = 0;
        if (aircraftIndex >= 25) aircraftIndex = 24;
        const int col = aircraftIndex % 5;
        const int row = 1 + (aircraftIndex / 5);
        return {col * m_spriteCellW, row * m_spriteCellH, m_spriteCellW, m_spriteCellH};
    }

    // Cuadrícula de explosiones (8 columnas x 2 filas)
    // row 0: explosión de fuego completa (8 frames)
    // row 1: onda expansiva de impacto (8 frames)
    [[nodiscard]] SDL_Rect getExplosionSrc(int frame, int row = 0) const noexcept {
        const int f = (frame >= 0 && frame < 8) ? frame : 0;
        const int r = (row == 1) ? 1 : 0;
        return {f * m_explosionCellW, r * m_explosionCellH, m_explosionCellW, m_explosionCellH};
    }

private:
    AssetManager() = default;
    ~AssetManager() = default;

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    SDL_Surface* loadSurfaceWithFallbacks(const char* filename);

    SDL_Texture* m_spriteSheet{nullptr};
    SDL_Texture* m_explosionSheet{nullptr};
    SDL_Texture* m_backgroundSheet{nullptr};

    int m_spriteCellW{128};
    int m_spriteCellH{128};
    int m_explosionCellW{128};
    int m_explosionCellH{128};
    int m_bgCellW{256};
    int m_bgCellH{128};

    bool m_initialized{false};
};

} // namespace Core

