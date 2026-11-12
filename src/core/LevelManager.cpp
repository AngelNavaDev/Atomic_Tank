#include "core/LevelManager.hpp"
#include "core/AssetManager.hpp"
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

constexpr std::array<LevelPalette, 25> THEME_PALETTES = {{
    // Nivel 1: Costa Azul (Océano y colinas costeras)
    {
        {25, 75, 150, 255},   // skyTop
        {110, 180, 235, 255}, // skyBottom
        {35, 60, 95, 255},    // mountains
        {45, 90, 75, 255},    // midLayer
        {30, 65, 45, 255},    // groundBase
        {0, 215, 110, 255},   // groundSurface
        "SECTOR 01: BLUE COASTLINE"
    },
    // Nivel 2: Cañón Árido al Atardecer
    {
        {130, 45, 20, 255},
        {235, 135, 55, 255},
        {110, 45, 30, 255},
        {140, 70, 35, 255},
        {90, 45, 25, 255},
        {245, 160, 40, 255},
        "SECTOR 02: DUST CANYON"
    },
    // Nivel 3: Base Nocturna con Radar
    {
        {10, 15, 35, 255},
        {35, 45, 80, 255},
        {20, 25, 45, 255},
        {30, 40, 60, 255},
        {20, 25, 35, 255},
        {40, 190, 255, 255},
        "SECTOR 03: NIGHT RADAR BASE"
    },
    // Nivel 4: Tormenta Ártica
    {
        {60, 80, 100, 255},
        {145, 175, 195, 255},
        {80, 110, 130, 255},
        {110, 140, 160, 255},
        {65, 85, 105, 255},
        {120, 235, 255, 255},
        "SECTOR 04: ARCTIC VALKYRIE"
    },
    // Nivel 5: Fortaleza Volcánica
    {
        {65, 15, 15, 255},
        {185, 45, 25, 255},
        {55, 20, 20, 255},
        {85, 30, 25, 255},
        {45, 20, 20, 255},
        {255, 75, 30, 255},
        "SECTOR 05: RED STAR CITADEL"
    },
    // Nivel 6: Jungla y Ruinas Ancestrales
    {
        {20, 50, 30, 255},   // skyTop
        {125, 105, 45, 255}, // skyBottom (ámbar selva)
        {35, 55, 35, 255},   // pirámides
        {25, 75, 40, 255},   // follaje
        {25, 45, 30, 255},   // suelo base
        {110, 220, 70, 255}, // superficie
        "SECTOR 06: JUNGLE RUINS"
    },
    // Nivel 7: Megaciudad Neón Nocturna
    {
        {15, 12, 38, 255},   // skyTop
        {55, 20, 85, 255},   // skyBottom (púrpura neón)
        {25, 22, 45, 255},   // rascacielos lejanos
        {35, 30, 60, 255},   // rascacielos medios
        {20, 18, 30, 255},   // asfalto
        {0, 235, 255, 255},  // luz cian
        "SECTOR 07: NEON MEGACITY"
    },
    // Nivel 8: Plataforma Petrolera y Tormenta Marina
    {
        {40, 48, 58, 255},   // skyTop
        {85, 100, 115, 255}, // skyBottom
        {30, 38, 48, 255},   // horizonte marino
        {50, 60, 72, 255},   // plataformas
        {25, 32, 42, 255},   // mar profundo
        {255, 140, 30, 255}, // fuego de antorchas
        "SECTOR 08: OFFSHORE OIL RIG"
    },
    // Nivel 9: Complejo Químico Radiactivo
    {
        {18, 35, 22, 255},   // skyTop
        {45, 90, 50, 255},   // skyBottom (verde tóxico)
        {25, 45, 30, 255},   // torres de enfriamiento
        {40, 70, 48, 255},   // tuberías
        {22, 35, 25, 255},   // suelo contaminado
        {80, 255, 60, 255},  // fluorescencia
        "SECTOR 09: RADIOACTIVE COMPLEX"
    },
    // Nivel 10: Fortaleza del Cañón Ferroviario
    {
        {60, 18, 22, 255},   // skyTop
        {135, 42, 45, 255},  // skyBottom (rojo acero)
        {45, 20, 25, 255},   // búnkeres y puentes
        {70, 30, 35, 255},   // muros blindados
        {35, 18, 20, 255},   // balasto ferroviario
        {255, 80, 40, 255},  // rieles incandescentes
        "SECTOR 10: RAILWAY FORTRESS"
    },
    // Nivel 11: Fundición Subterránea y Magma
    {
        {24, 16, 20, 255},   // skyTop (roca basáltica oscura)
        {175, 45, 15, 255},  // skyBottom (resplandor de magma)
        {48, 22, 22, 255},   // columnas de basalto lejanas
        {65, 30, 25, 255},   // crisoles y hornos medios
        {35, 18, 16, 255},   // roca volcánica base
        {255, 110, 20, 255}, // lava incandescente superficie
        "SECTOR 11: MAGMA FOUNDRY"
    },
    // Nivel 12: Cañón de Cristales de Uranio
    {
        {18, 12, 42, 255},   // skyTop (índigo oscuro)
        {60, 25, 80, 255},   // skyBottom (violeta)
        {30, 20, 60, 255},   // monolitos de cristal lejanos
        {45, 32, 75, 255},   // torres mineras intermedias
        {22, 16, 36, 255},   // lecho de cuarzo oscuro base
        {0, 245, 210, 255},  // uranio turquesa superficie
        "SECTOR 12: URANIUM CANYON"
    },
    // Nivel 13: Cosmódromo Espacial
    {
        {6, 10, 26, 255},    // skyTop (estratosfera con estrellas)
        {22, 55, 140, 255},  // skyBottom (azul cobalto brillante)
        {18, 28, 55, 255},   // hangares y plataformas lejanas
        {35, 48, 80, 255},   // torres umbilicales de cohetes
        {14, 20, 36, 255},   // hormigón de cosmódromo
        {80, 180, 255, 255}, // luces de pista aeroespacial
        "SECTOR 13: SPACE COSMODROME"
    },
    // Nivel 14: Campo de Torres Tesla y Tormenta Eléctrica
    {
        {36, 38, 42, 255},   // skyTop (gris tormenta oscuro)
        {95, 85, 65, 255},   // skyBottom (ocre tormentoso)
        {45, 46, 50, 255},   // bobinas Tesla lejanas
        {60, 62, 68, 255},   // generadores de alta tensión
        {28, 30, 32, 255},   // suelo electrificado
        {140, 240, 255, 255},// arcos voltaicos cian
        "SECTOR 14: TESLA STORM FIELD"
    },
    // Nivel 15: Ascensor Orbital / Estratósfera
    {
        {4, 4, 10, 255},     // skyTop (negro espacial cósmico)
        {15, 85, 215, 255},  // skyBottom (atmósfera terrestre azul brillante)
        {20, 28, 45, 255},   // estaciones orbitales lejanas
        {35, 45, 70, 255},   // andamios del ascensor
        {10, 14, 24, 255},   // base de espaciopuerto
        {0, 220, 255, 255},  // guías magnéticas de hipervelocidad
        "SECTOR 15: ORBITAL ELEVATOR"
    },
    // Nivel 16: Astillero Naval
    {
        {22, 32, 44, 255},
        {44, 62, 82, 255},
        {36, 52, 69, 255},
        {26, 37, 51, 255},
        {16, 23, 33, 255},
        {96, 165, 250, 255},
        "SECTOR 16: NAVAL WAR SHIPYARD"
    },
    // Nivel 17: Valle de Reliquias Mecánicas
    {
        {46, 33, 24, 255},
        {89, 63, 41, 255},
        {66, 48, 33, 255},
        {48, 34, 23, 255},
        {33, 22, 14, 255},
        {217, 119, 6, 255},
        "SECTOR 17: MECHANICAL RELICS VALLEY"
    },
    // Nivel 18: Reactor Crio-Solar
    {
        {20, 36, 46, 255},
        {39, 68, 84, 255},
        {34, 59, 74, 255},
        {23, 41, 51, 255},
        {14, 26, 33, 255},
        {2, 132, 199, 255},
        "SECTOR 18: CRYO-SOLAR REACTOR"
    },
    // Nivel 19: Ciudadela Gótica
    {
        {33, 20, 28, 255},
        {69, 37, 56, 255},
        {51, 30, 43, 255},
        {36, 21, 30, 255},
        {23, 12, 19, 255},
        {225, 29, 72, 255},
        "SECTOR 19: GOTHIC CITADEL"
    },
    // Nivel 20: Cráter Lunar ICBM
    {
        {9, 10, 16, 255},
        {28, 32, 43, 255},
        {28, 32, 42, 255},
        {20, 23, 31, 255},
        {12, 14, 19, 255},
        {148, 163, 184, 255},
        "SECTOR 20: LUNAR ICBM CRATER"
    },
    // Nivel 21: Cinturón de Asteroides
    {
        {7, 8, 18, 255},
        {22, 25, 51, 255},
        {23, 26, 51, 255},
        {16, 18, 36, 255},
        {10, 11, 23, 255},
        {167, 139, 250, 255},
        "SECTOR 21: ASTEROID BELT VOID"
    },
    // Nivel 22: Marte Terraformado
    {
        {43, 28, 28, 255},
        {84, 52, 49, 255},
        {66, 40, 38, 255},
        {48, 29, 28, 255},
        {33, 19, 18, 255},
        {16, 185, 129, 255},
        "SECTOR 22: TERRAFORMED MARS"
    },
    // Nivel 23: Matriz Cibernética IA
    {
        {10, 22, 31, 255},
        {23, 48, 61, 255},
        {22, 46, 59, 255},
        {15, 32, 41, 255},
        {9, 20, 26, 255},
        {6, 182, 212, 255},
        "SECTOR 23: CYBERNETIC AI MATRIX"
    },
    // Nivel 24: Reactor de Antimateria
    {
        {18, 13, 36, 255},
        {39, 27, 74, 255},
        {35, 24, 64, 255},
        {24, 16, 43, 255},
        {15, 10, 28, 255},
        {217, 70, 239, 255},
        "SECTOR 24: ANTIMATTER REACTOR"
    },
    // Nivel 25: Ciudadela Apocalíptica Omega
    {
        {38, 14, 14, 255},
        {84, 28, 28, 255},
        {61, 21, 21, 255},
        {43, 14, 14, 255},
        {26, 8, 8, 255},
        {245, 158, 11, 255},
        "SECTOR 25: OMEGA DOOMSDAY CITADEL"
    }
}};

} // namespace

LevelManager::LevelManager() {
    init();
}

void LevelManager::init() {
    m_currentLevel = 1;
    m_missionTimer = 0.0f;
    m_waveDuration = BASE_WAVE_DURATION + static_cast<float>(m_currentLevel) * 3.0f;
    m_levelCompleteTimer = 0.0f;
    m_mountainScroll = 0.0f;
    m_midLayerScroll = 0.0f;
    m_groundScroll = 0.0f;
    m_lightningTimer = 0.0f;
    m_ambientAnimTimer = 0.0f;
    updatePalette();
}

void LevelManager::resetToLevel(int level) {
    m_currentLevel = std::clamp(level, 1, MAX_LEVELS);
    m_missionTimer = 0.0f;
    m_waveDuration = BASE_WAVE_DURATION + static_cast<float>(m_currentLevel) * 3.0f;
    m_levelCompleteTimer = 0.0f;
    m_lightningTimer = 0.0f;
    m_ambientAnimTimer = 0.0f;
    updatePalette();
}

void LevelManager::updatePalette() {
    const int themeIndex = std::clamp(m_currentLevel - 1, 0, MAX_LEVELS - 1);
    m_currentPalette = THEME_PALETTES[themeIndex];
}

void LevelManager::update(float dt, bool bossActive) {
    m_ambientAnimTimer += dt;

    // 1. Avance continuo del desplazamiento horizontal (Parallax)
    m_mountainScroll += 40.0f * dt;
    if (m_mountainScroll >= 1920.0f) {
        m_mountainScroll -= 1920.0f;
    }

    m_midLayerScroll += 110.0f * dt;
    if (m_midLayerScroll >= 1920.0f) {
        m_midLayerScroll -= 1920.0f;
    }

    m_groundScroll += 260.0f * dt;
    if (m_groundScroll >= 1920.0f) {
        m_groundScroll -= 1920.0f;
    }

    // Efecto de relámpago en el Nivel 8 (Plataforma Petrolera) y Nivel 14 (Torres Tesla)
    const int sector = (m_currentLevel - 1) % 15 + 1;
    if (sector == 8 || sector == 14) {
        if (m_lightningTimer > 0.0f) {
            m_lightningTimer -= dt;
            if (m_lightningTimer < 0.0f) m_lightningTimer = 0.0f;
        } else {
            const float interval = (sector == 14) ? 4.5f : 6.5f;
            if (std::fmod(m_ambientAnimTimer, interval) < dt) {
                m_lightningTimer = 0.12f;
            }
        }
    }

    // 2. Progresión de la misión
    if (!bossActive && m_levelCompleteTimer <= 0.0f) {
        m_missionTimer += dt;
        if (m_missionTimer > m_waveDuration) {
            m_missionTimer = m_waveDuration;
        }
    }

    // 3. Temporizador de transición de nivel completado
    if (m_levelCompleteTimer > 0.0f) {
        m_levelCompleteTimer -= dt;
        if (m_levelCompleteTimer <= 0.0f) {
            m_levelCompleteTimer = 0.0f;
            advanceLevel();
        }
    }
}

void LevelManager::onBossDefeated() {
    m_levelCompleteTimer = LEVEL_COMPLETE_DURATION;
}

void LevelManager::advanceLevel() {
    if (m_currentLevel < MAX_LEVELS) {
        m_currentLevel++;
    } else {
        m_currentLevel = 1;
    }
    m_missionTimer = 0.0f;
    m_waveDuration = BASE_WAVE_DURATION + static_cast<float>(m_currentLevel) * 3.0f;
    m_levelCompleteTimer = 0.0f;
    updatePalette();
}

void LevelManager::renderParallaxBackground(SDL_Renderer* renderer) const {
    const int sectorIndex = std::clamp(m_currentLevel, 1, MAX_LEVELS);

    // =========================================================================
    // 1. CAPA BASE: ATLAS DE FONDOS PIXEL ART (25 NIVELES, SEAMLESS TILING)
    // =========================================================================
    SDL_Texture* bgAtlas = Core::AssetManager::getInstance().getBackgroundSheet();
    if (bgAtlas) {
        const SDL_Rect srcRect = Core::AssetManager::getInstance().getLevelBackgroundRect(m_currentLevel);
        constexpr float tileW = 960.0f;
        constexpr float tileH = 1080.0f;
        const float scroll = std::fmod(m_mountainScroll * 0.75f, tileW);

        for (int i = -1; i <= 2; ++i) {
            const SDL_Rect dstRect = {
                static_cast<int>(static_cast<float>(i) * tileW - scroll),
                0,
                static_cast<int>(tileW),
                static_cast<int>(tileH)
            };
            SDL_RenderCopy(renderer, bgAtlas, &srcRect, &dstRect);
        }
    } else {
        // Fallback procedural de degradado
        SDL_Color top = m_currentPalette.skyTop;
        SDL_Color bot = m_currentPalette.skyBottom;
        const SDL_Vertex skyVerts[4] = {
            {{0.0f, 0.0f},       top, {0.0f, 0.0f}},
            {{1920.0f, 0.0f},    top, {0.0f, 0.0f}},
            {{1920.0f, 980.0f},  bot, {0.0f, 0.0f}},
            {{0.0f, 980.0f},     bot, {0.0f, 0.0f}}
        };
        const int skyIndices[6] = {0, 1, 2, 2, 3, 0};
        SDL_RenderGeometry(renderer, nullptr, skyVerts, 4, skyIndices, 6);
    }

    // =========================================================================
    // 2. EFECTOS CLIMÁTICOS Y RELÁMPAGOS
    // =========================================================================
    if ((sectorIndex == 8 || sectorIndex == 14) && m_lightningTimer > 0.0f) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        const SDL_Rect flashRect{0, 0, 1920, 1080};
        SDL_SetRenderDrawColor(renderer, 220, 240, 255, 120);
        SDL_RenderFillRect(renderer, &flashRect);
    }

    // =========================================================================
    // 3. CAPA LEJANA (PARALLAX A 40 px/s): MODELADO SEGÚN EL SECTOR
    // =========================================================================
    const SDL_Color& farColor = m_currentPalette.mountains;
    const float farOffset = m_mountainScroll;

    for (int rep = 0; rep < 2; ++rep) {
        const float bX = static_cast<float>(rep) * 1920.0f - farOffset;

        if (sectorIndex == 6) {
            // Nivel 6 (Jungla y Ruinas): Pirámides escalonadas mayas / ziggurats
            SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            for (int i = 0; i < 4; ++i) {
                const float px = bX + static_cast<float>(i) * 480.0f + 120.0f;
                // Escalón 1 (Base)
                const SDL_FRect s1{px - 140.0f, 820.0f, 280.0f, 160.0f};
                // Escalón 2 (Medio)
                const SDL_FRect s2{px - 95.0f, 720.0f, 190.0f, 100.0f};
                // Escalón 3 (Superior)
                const SDL_FRect s3{px - 55.0f, 640.0f, 110.0f, 80.0f};
                // Santuario en la cima
                const SDL_FRect shrine{px - 25.0f, 590.0f, 50.0f, 50.0f};
                SDL_RenderFillRectF(renderer, &s1);
                SDL_RenderFillRectF(renderer, &s2);
                SDL_RenderFillRectF(renderer, &s3);
                SDL_RenderFillRectF(renderer, &shrine);
            }
        } else if (sectorIndex == 7) {
            // Nivel 7 (Megaciudad Neón): Siluetas de rascacielos con balizas rojas
            SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            for (int i = 0; i < 9; ++i) {
                const float sx = bX + static_cast<float>(i) * 220.0f;
                const float sw = 130.0f + static_cast<float>((i % 3) * 30);
                const float sh = 380.0f + static_cast<float>((i % 4) * 60);
                const SDL_FRect bldg{sx, 980.0f - sh, sw, sh};
                SDL_RenderFillRectF(renderer, &bldg);

                // Antena y baliza roja en la cúspide
                const SDL_FRect ant{sx + sw * 0.5f - 2.0f, 980.0f - sh - 30.0f, 4.0f, 30.0f};
                SDL_RenderFillRectF(renderer, &ant);
                const SDL_FRect beacon{sx + sw * 0.5f - 4.0f, 980.0f - sh - 36.0f, 8.0f, 6.0f};
                SDL_SetRenderDrawColor(renderer, 255, 30, 30, 240);
                SDL_RenderFillRectF(renderer, &beacon);
                SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            }
        } else if (sectorIndex == 8) {
            // Nivel 8 (Tormenta Marina): Acorazados y petroleros en el horizonte marino
            SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            for (int i = 0; i < 3; ++i) {
                const float shipX = bX + static_cast<float>(i) * 640.0f + 160.0f;
                // Casco naval
                const SDL_FRect shipHull{shipX - 180.0f, 880.0f, 360.0f, 100.0f};
                const SDL_FRect bridge{shipX - 40.0f, 810.0f, 80.0f, 70.0f};
                const SDL_FRect mast{shipX - 4.0f, 740.0f, 8.0f, 70.0f};
                SDL_RenderFillRectF(renderer, &shipHull);
                SDL_RenderFillRectF(renderer, &bridge);
                SDL_RenderFillRectF(renderer, &mast);
            }
        } else if (sectorIndex == 9) {
            // Nivel 9 (Complejo Químico): Torres hiperbólicas de enfriamiento nuclear
            SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            for (int i = 0; i < 4; ++i) {
                const float twX = bX + static_cast<float>(i) * 480.0f + 140.0f;
                // Base ancha
                const SDL_FRect baseTower{twX - 110.0f, 800.0f, 220.0f, 180.0f};
                // Cuello curvado
                const SDL_FRect neckTower{twX - 85.0f, 680.0f, 170.0f, 120.0f};
                // Borde superior
                const SDL_FRect lipTower{twX - 95.0f, 660.0f, 190.0f, 20.0f};
                SDL_RenderFillRectF(renderer, &baseTower);
                SDL_RenderFillRectF(renderer, &neckTower);
                SDL_RenderFillRectF(renderer, &lipTower);

                // Tanque esférico
                const SDL_FRect sphereTank{twX + 130.0f, 830.0f, 100.0f, 100.0f};
                SDL_RenderFillRectF(renderer, &sphereTank);
            }
        } else if (sectorIndex == 10) {
            // Nivel 10 (Fortaleza Ferroviaria): Puentes de vigas de hierro y reflectores
            SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            for (int i = 0; i < 4; ++i) {
                const float bridgeX = bX + static_cast<float>(i) * 500.0f;
                // Viga horizontal masiva
                const SDL_FRect girder{bridgeX, 760.0f, 480.0f, 35.0f};
                SDL_RenderFillRectF(renderer, &girder);
                // Pilares
                const SDL_FRect p1{bridgeX + 40.0f, 795.0f, 30.0f, 185.0f};
                const SDL_FRect p2{bridgeX + 410.0f, 795.0f, 30.0f, 185.0f};
                SDL_RenderFillRectF(renderer, &p1);
                SDL_RenderFillRectF(renderer, &p2);
            }

            // Haces de reflectores antiaéreos cruzando el cielo
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            const float sweep = std::sin(m_ambientAnimTimer * 1.5f) * 200.0f;
            const SDL_Vertex beamVerts[3] = {
                {{bX + 480.0f, 780.0f},                     {255, 230, 180, 70}, {0.0f, 0.0f}},
                {{bX + 480.0f + sweep - 120.0f, 0.0f},      {255, 230, 180, 0},  {0.0f, 0.0f}},
                {{bX + 480.0f + sweep + 120.0f, 0.0f},      {255, 230, 180, 0},  {0.0f, 0.0f}}
            };
            const int triIdx[3] = {0, 1, 2};
            SDL_RenderGeometry(renderer, nullptr, beamVerts, 3, triIdx, 3);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        } else if (sectorIndex == 11) {
            // Nivel 11 (Fundición Subterránea): Columnas basálticas y cascadas de lava brillante
            SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            for (int i = 0; i < 5; ++i) {
                const float colX = bX + static_cast<float>(i) * 380.0f + 80.0f;
                const SDL_FRect pillar{colX - 45.0f, 320.0f, 90.0f, 660.0f};
                SDL_RenderFillRectF(renderer, &pillar);

                // Cascada vertical de lava
                const float pulseLava = std::sin(m_ambientAnimTimer * 4.0f + static_cast<float>(i)) * 25.0f;
                const Uint8 lavaG = static_cast<Uint8>(130.0f + pulseLava);
                const SDL_FRect lavaFall{colX - 14.0f, 420.0f, 28.0f, 560.0f};
                SDL_SetRenderDrawColor(renderer, 255, lavaG, 20, 240);
                SDL_RenderFillRectF(renderer, &lavaFall);

                const SDL_FRect lavaCore{colX - 6.0f, 440.0f, 12.0f, 540.0f};
                SDL_SetRenderDrawColor(renderer, 255, 230, 80, 255);
                SDL_RenderFillRectF(renderer, &lavaCore);
                SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            }
        } else if (sectorIndex == 12) {
            // Nivel 12 (Cañón de Uranio): Formaciones de cristales puntiagudos inclinados
            for (int i = 0; i < 6; ++i) {
                const float crysX = bX + static_cast<float>(i) * 320.0f + 140.0f;
                const float crysH = 260.0f + static_cast<float>((i % 3) * 60);
                const float tilt = ((i % 2 == 0) ? 35.0f : -35.0f);

                const SDL_Color cColor = (i % 2 == 0) ? SDL_Color{0, 225, 255, 210} : SDL_Color{230, 45, 195, 190};
                const SDL_Vertex crysVerts[3] = {
                    {{crysX + tilt, 980.0f - crysH}, cColor, {0.0f, 0.0f}},
                    {{crysX - 45.0f, 980.0f},         farColor, {0.0f, 0.0f}},
                    {{crysX + 45.0f, 980.0f},         farColor, {0.0f, 0.0f}}
                };
                const int triIdx[3] = {0, 1, 2};
                SDL_RenderGeometry(renderer, nullptr, crysVerts, 3, triIdx, 3);

                const SDL_FRect derrick{crysX + 100.0f, 980.0f - 180.0f, 30.0f, 180.0f};
                SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
                SDL_RenderFillRectF(renderer, &derrick);
            }
        } else if (sectorIndex == 13) {
            // Nivel 13 (Cosmódromo): Estrellas en la estratosfera y cohetes espaciales gigantes
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            for (int s = 0; s < 12; ++s) {
                const float starX = bX + static_cast<float>(s * 160 + 35);
                const float starY = 40.0f + static_cast<float>((s * 29) % 240);
                const Uint8 starAlpha = static_cast<Uint8>(150 + 105 * std::abs(std::sin(m_ambientAnimTimer * 2.5f + static_cast<float>(s))));
                const SDL_FRect starDot{starX, starY, 4.0f, 4.0f};
                SDL_SetRenderDrawColor(renderer, 240, 245, 255, starAlpha);
                SDL_RenderFillRectF(renderer, &starDot);
            }
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

            SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
            for (int i = 0; i < 3; ++i) {
                const float rx = bX + static_cast<float>(i) * 640.0f + 200.0f;
                const SDL_FRect tower{rx - 38.0f, 520.0f, 26.0f, 460.0f};
                SDL_RenderFillRectF(renderer, &tower);

                const SDL_FRect rocketBody{rx, 560.0f, 32.0f, 420.0f};
                SDL_SetRenderDrawColor(renderer, 220, 230, 245, 255);
                SDL_RenderFillRectF(renderer, &rocketBody);

                const SDL_Vertex coneVerts[3] = {
                    {{rx + 16.0f, 510.0f}, {240, 60, 60, 255}, {0.0f, 0.0f}},
                    {{rx,         560.0f}, {220, 230, 245, 255}, {0.0f, 0.0f}},
                    {{rx + 32.0f, 560.0f}, {220, 230, 245, 255}, {0.0f, 0.0f}}
                };
                const int triIdx[3] = {0, 1, 2};
                SDL_RenderGeometry(renderer, nullptr, coneVerts, 3, triIdx, 3);

                const SDL_FRect hangar{rx + 120.0f, 840.0f, 220.0f, 140.0f};
                SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
                SDL_RenderFillRectF(renderer, &hangar);
            }
        } else if (sectorIndex == 14) {
            // Nivel 14 (Torres Tesla): Enormes torres con bobinas esféricas en la punta
            for (int i = 0; i < 4; ++i) {
                const float tx = bX + static_cast<float>(i) * 480.0f + 160.0f;
                SDL_SetRenderDrawColor(renderer, farColor.r, farColor.g, farColor.b, 255);
                const SDL_FRect mast{tx - 16.0f, 560.0f, 32.0f, 420.0f};
                SDL_RenderFillRectF(renderer, &mast);

                const SDL_FRect coilSphere{tx - 32.0f, 520.0f, 64.0f, 40.0f};
                const Uint8 spark = static_cast<Uint8>(180 + 75 * std::abs(std::sin(m_ambientAnimTimer * 16.0f + static_cast<float>(i))));
                SDL_SetRenderDrawColor(renderer, 100, spark, 255, 255);
                SDL_RenderFillRectF(renderer, &coilSphere);

                const SDL_FRect ringCore{tx - 20.0f, 532.0f, 40.0f, 16.0f};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRectF(renderer, &ringCore);
            }
        } else if (sectorIndex == 15) {
            // Nivel 15 (Ascensor Orbital): Mega-columna vertical y paneles solares espaciales
            const float colX = bX + 960.0f;
            const SDL_FRect mainSpine{colX - 35.0f, 0.0f, 70.0f, 980.0f};
            SDL_SetRenderDrawColor(renderer, 45, 55, 75, 255);
            SDL_RenderFillRectF(renderer, &mainSpine);

            const SDL_FRect magRail{colX - 8.0f, 0.0f, 16.0f, 980.0f};
            SDL_SetRenderDrawColor(renderer, 0, 240, 255, 220);
            SDL_RenderFillRectF(renderer, &magRail);

            const float carY = std::fmod(m_ambientAnimTimer * 90.0f, 980.0f);
            const SDL_FRect climberCar{colX - 25.0f, 980.0f - carY, 50.0f, 70.0f};
            SDL_SetRenderDrawColor(renderer, 240, 245, 255, 255);
            SDL_RenderFillRectF(renderer, &climberCar);

            for (int s = 0; s < 3; ++s) {
                const float stX = bX + static_cast<float>(s) * 640.0f + 160.0f;
                const float stY = 160.0f + static_cast<float>((s % 2) * 80);
                const SDL_FRect hub{stX - 20.0f, stY - 14.0f, 40.0f, 28.0f};
                SDL_SetRenderDrawColor(renderer, 200, 210, 230, 255);
                SDL_RenderFillRectF(renderer, &hub);

                const SDL_FRect panL{stX - 90.0f, stY - 10.0f, 65.0f, 20.0f};
                const SDL_FRect panR{stX + 25.0f, stY - 10.0f, 65.0f, 20.0f};
                SDL_SetRenderDrawColor(renderer, 255, 200, 40, 240);
                SDL_RenderFillRectF(renderer, &panL);
                SDL_RenderFillRectF(renderer, &panR);
            }
        } else {
            // Niveles 1 al 5: Montañas clásicas triangulares
            for (int i = 0; i < 6; ++i) {
                const float peakX = bX + static_cast<float>(i) * 320.0f + 160.0f;
                const float peakY = 560.0f + static_cast<float>((i % 3) * 50);

                const SDL_Vertex mntTri[3] = {
                    {{peakX, peakY},                      farColor, {0.0f, 0.0f}},
                    {{peakX - 220.0f, 980.0f},            farColor, {0.0f, 0.0f}},
                    {{peakX + 220.0f, 980.0f},            farColor, {0.0f, 0.0f}}
                };
                const int triIdx[3] = {0, 1, 2};
                SDL_RenderGeometry(renderer, nullptr, mntTri, 3, triIdx, 3);
            }
        }
    }

    // =========================================================================
    // C. CAPA MEDIA (PARALLAX A 110 px/s): MODELADO SEGÚN EL SECTOR
    // =========================================================================
    const SDL_Color& midCol = m_currentPalette.midLayer;
    const float midOffset = m_midLayerScroll;

    for (int rep = 0; rep < 2; ++rep) {
        const float bX = static_cast<float>(rep) * 1920.0f - midOffset;

        if (sectorIndex == 6) {
            // Nivel 6: Árboles gigantes de jungla y copas frondosas
            SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
            for (int i = 0; i < 8; ++i) {
                const float tx = bX + static_cast<float>(i) * 240.0f + 50.0f;
                // Tronco
                const SDL_FRect trunk{tx - 12.0f, 800.0f, 24.0f, 180.0f};
                SDL_RenderFillRectF(renderer, &trunk);
                // 3 Bloques superpuestos de copa redondeada
                const SDL_FRect leaf1{tx - 70.0f, 750.0f, 140.0f, 70.0f};
                const SDL_FRect leaf2{tx - 55.0f, 700.0f, 110.0f, 60.0f};
                const SDL_FRect leaf3{tx - 35.0f, 660.0f, 70.0f, 50.0f};
                SDL_RenderFillRectF(renderer, &leaf1);
                SDL_RenderFillRectF(renderer, &leaf2);
                SDL_RenderFillRectF(renderer, &leaf3);
            }
        } else if (sectorIndex == 7) {
            // Nivel 7: Rascacielos con cuadrícula de ventanas neón encendidas
            for (int i = 0; i < 8; ++i) {
                const float sx = bX + static_cast<float>(i) * 240.0f;
                const float sw = 100.0f + static_cast<float>((i % 3) * 25);
                const float sh = 200.0f + static_cast<float>((i % 4) * 45);

                const SDL_FRect bldg{sx, 980.0f - sh, sw, sh};
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                SDL_RenderFillRectF(renderer, &bldg);

                // Cuadrículas de ventanas neón encendidas
                for (int wy = 0; wy < 5; ++wy) {
                    for (int wx = 0; wx < 3; ++wx) {
                        const SDL_FRect win{sx + 14.0f + static_cast<float>(wx) * 24.0f,
                                            980.0f - sh + 20.0f + static_cast<float>(wy) * 26.0f,
                                            12.0f, 14.0f};
                        if ((i + wx + wy) % 3 == 0) {
                            SDL_SetRenderDrawColor(renderer, 0, 240, 255, 230); // Cian
                        } else if ((i + wx + wy) % 3 == 1) {
                            SDL_SetRenderDrawColor(renderer, 255, 220, 50, 230); // Amarillo
                        } else {
                            SDL_SetRenderDrawColor(renderer, 255, 40, 180, 230); // Magenta
                        }
                        SDL_RenderFillRectF(renderer, &win);
                    }
                }
            }
        } else if (sectorIndex == 8) {
            // Nivel 8: Plataformas petroleras offshore con antorchas de fuego en la cima
            for (int i = 0; i < 5; ++i) {
                const float rigX = bX + static_cast<float>(i) * 380.0f + 60.0f;
                // Piernas estructurales
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                const SDL_FRect legL{rigX - 45.0f, 820.0f, 14.0f, 160.0f};
                const SDL_FRect legR{rigX + 35.0f, 820.0f, 14.0f, 160.0f};
                const SDL_FRect deck{rigX - 60.0f, 800.0f, 124.0f, 25.0f};
                const SDL_FRect derrick{rigX - 16.0f, 700.0f, 32.0f, 100.0f};
                SDL_RenderFillRectF(renderer, &legL);
                SDL_RenderFillRectF(renderer, &legR);
                SDL_RenderFillRectF(renderer, &deck);
                SDL_RenderFillRectF(renderer, &derrick);

                // Antorcha de gas ardiendo en la cima
                const float flameH = 16.0f + std::sin(m_ambientAnimTimer * 12.0f + static_cast<float>(i)) * 6.0f;
                const SDL_FRect flare{rigX - 6.0f, 700.0f - flameH, 12.0f, flameH};
                SDL_SetRenderDrawColor(renderer, 255, 140, 20, 240);
                SDL_RenderFillRectF(renderer, &flare);
            }
        } else if (sectorIndex == 9) {
            // Nivel 9: Tuberías industriales y vapor verde radiactivo
            for (int i = 0; i < 6; ++i) {
                const float px = bX + static_cast<float>(i) * 320.0f;
                // Tubería horizontal
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                const SDL_FRect pipeH{px, 860.0f, 320.0f, 16.0f};
                SDL_RenderFillRectF(renderer, &pipeH);
                // Soportes verticales
                const SDL_FRect sup{px + 40.0f, 876.0f, 18.0f, 104.0f};
                SDL_RenderFillRectF(renderer, &sup);

                // Anillo de advertencia amarillo en la tubería
                const SDL_FRect hazard{px + 120.0f, 860.0f, 16.0f, 16.0f};
                SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                SDL_RenderFillRectF(renderer, &hazard);

                // Nubes de vapor verde subiendo
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                const float puffY = 830.0f - std::fmod(m_ambientAnimTimer * 40.0f + static_cast<float>(i * 30), 80.0f);
                const SDL_FRect vaporPuff{px + 115.0f, puffY, 26.0f, 20.0f};
                SDL_SetRenderDrawColor(renderer, 70, 240, 50, 140);
                SDL_RenderFillRectF(renderer, &vaporPuff);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
        } else if (sectorIndex == 10) {
            // Nivel 10: Búnkeres y radar militar giratorio
            for (int i = 0; i < 6; ++i) {
                const float bx = bX + static_cast<float>(i) * 320.0f;
                // Muro fortificado
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                const SDL_FRect bunker{bx, 870.0f, 180.0f, 110.0f};
                SDL_RenderFillRectF(renderer, &bunker);

                // Aspillera de disparo
                const SDL_FRect slit{bx + 30.0f, 900.0f, 60.0f, 12.0f};
                SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
                SDL_RenderFillRectF(renderer, &slit);

                // Plato de radar militar giratorio
                const float dishW = 38.0f * std::cos(m_ambientAnimTimer * 3.0f + static_cast<float>(i));
                const float safeW = std::max(8.0f, std::abs(dishW));
                const SDL_FRect radarDish{bx + 90.0f - safeW * 0.5f, 850.0f, safeW, 14.0f};
                SDL_SetRenderDrawColor(renderer, 200, 210, 225, 255);
                SDL_RenderFillRectF(renderer, &radarDish);
            }
        } else if (sectorIndex == 11) {
            // Nivel 11: Crisoles gigantes de metal fundido y hornos de fundición
            for (int i = 0; i < 5; ++i) {
                const float fx = bX + static_cast<float>(i) * 380.0f + 60.0f;
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                const SDL_FRect beam{fx - 70.0f, 760.0f, 240.0f, 20.0f};
                SDL_RenderFillRectF(renderer, &beam);

                const SDL_FRect chainL{fx - 25.0f, 780.0f, 8.0f, 40.0f};
                const SDL_FRect chainR{fx + 25.0f, 780.0f, 8.0f, 40.0f};
                SDL_RenderFillRectF(renderer, &chainL);
                SDL_RenderFillRectF(renderer, &chainR);

                const SDL_FRect crucible{fx - 35.0f, 820.0f, 70.0f, 50.0f};
                SDL_SetRenderDrawColor(renderer, 45, 25, 20, 255);
                SDL_RenderFillRectF(renderer, &crucible);

                const SDL_FRect moltenMetal{fx - 28.0f, 824.0f, 56.0f, 16.0f};
                SDL_SetRenderDrawColor(renderer, 255, 160, 30, 255);
                SDL_RenderFillRectF(renderer, &moltenMetal);

                const SDL_FRect furnace{fx + 80.0f, 870.0f, 120.0f, 110.0f};
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                SDL_RenderFillRectF(renderer, &furnace);

                const SDL_FRect furnaceDoor{fx + 105.0f, 915.0f, 70.0f, 65.0f};
                SDL_SetRenderDrawColor(renderer, 255, 90, 20, 255);
                SDL_RenderFillRectF(renderer, &furnaceDoor);
            }
        } else if (sectorIndex == 12) {
            // Nivel 12: Torres mineras de extracción y cristales de uranio intermedios
            for (int i = 0; i < 5; ++i) {
                const float mx = bX + static_cast<float>(i) * 380.0f + 80.0f;
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                const SDL_FRect towerLeg1{mx - 25.0f, 780.0f, 12.0f, 200.0f};
                const SDL_FRect towerLeg2{mx + 15.0f, 780.0f, 12.0f, 200.0f};
                const SDL_FRect towerTop{mx - 35.0f, 760.0f, 70.0f, 20.0f};
                SDL_RenderFillRectF(renderer, &towerLeg1);
                SDL_RenderFillRectF(renderer, &towerLeg2);
                SDL_RenderFillRectF(renderer, &towerTop);

                const SDL_FRect crysCore{mx + 60.0f, 890.0f, 45.0f, 90.0f};
                SDL_SetRenderDrawColor(renderer, 0, 245, 210, 230);
                SDL_RenderFillRectF(renderer, &crysCore);
            }
        } else if (sectorIndex == 13) {
            // Nivel 13: Radares parabólicos de seguimiento y hangares de ensamblaje
            for (int i = 0; i < 4; ++i) {
                const float px = bX + static_cast<float>(i) * 480.0f + 120.0f;
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                const SDL_FRect mast{px - 10.0f, 840.0f, 20.0f, 140.0f};
                SDL_RenderFillRectF(renderer, &mast);

                const float angle = m_ambientAnimTimer * 2.2f + static_cast<float>(i);
                const float wSpan = 60.0f * std::abs(std::cos(angle));
                const SDL_FRect dish{px - wSpan * 0.5f, 810.0f, std::max(10.0f, wSpan), 24.0f};
                SDL_SetRenderDrawColor(renderer, 210, 225, 240, 255);
                SDL_RenderFillRectF(renderer, &dish);

                const SDL_FRect hangar{px + 70.0f, 870.0f, 160.0f, 110.0f};
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                SDL_RenderFillRectF(renderer, &hangar);

                const SDL_FRect strobe{px + 145.0f, 862.0f, 10.0f, 8.0f};
                SDL_SetRenderDrawColor(renderer, 255, 40, 40, 255);
                SDL_RenderFillRectF(renderer, &strobe);
            }
        } else if (sectorIndex == 14) {
            // Nivel 14: Generadores de alta tensión con arcos eléctricos
            for (int i = 0; i < 5; ++i) {
                const float gx = bX + static_cast<float>(i) * 380.0f + 70.0f;
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                const SDL_FRect genBody{gx - 40.0f, 860.0f, 80.0f, 120.0f};
                SDL_RenderFillRectF(renderer, &genBody);

                for (int f = 0; f < 4; ++f) {
                    const SDL_FRect fin{gx - 36.0f, 875.0f + static_cast<float>(f) * 16.0f, 72.0f, 6.0f};
                    SDL_SetRenderDrawColor(renderer, 35, 38, 42, 255);
                    SDL_RenderFillRectF(renderer, &fin);
                }

                const SDL_FRect termL{gx - 25.0f, 840.0f, 10.0f, 20.0f};
                const SDL_FRect termR{gx + 15.0f, 840.0f, 10.0f, 20.0f};
                SDL_SetRenderDrawColor(renderer, 180, 190, 200, 255);
                SDL_RenderFillRectF(renderer, &termL);
                SDL_RenderFillRectF(renderer, &termR);

                if (static_cast<int>(m_ambientAnimTimer * 20.0f + static_cast<float>(i * 3)) % 3 == 0) {
                    const SDL_FRect arc{gx - 15.0f, 842.0f, 30.0f, 8.0f};
                    SDL_SetRenderDrawColor(renderer, 100, 240, 255, 240);
                    SDL_RenderFillRectF(renderer, &arc);
                }
            }
        } else if (sectorIndex == 15) {
            // Nivel 15: Andamios del ascensor y módulos habitacionales orbitales
            for (int i = 0; i < 4; ++i) {
                const float dx = bX + static_cast<float>(i) * 480.0f + 60.0f;
                SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
                const SDL_FRect gantry{dx, 870.0f, 240.0f, 24.0f};
                const SDL_FRect legL{dx + 20.0f, 894.0f, 16.0f, 86.0f};
                const SDL_FRect legR{dx + 200.0f, 894.0f, 16.0f, 86.0f};
                SDL_RenderFillRectF(renderer, &gantry);
                SDL_RenderFillRectF(renderer, &legL);
                SDL_RenderFillRectF(renderer, &legR);

                const SDL_FRect pod{dx + 60.0f, 830.0f, 120.0f, 40.0f};
                SDL_SetRenderDrawColor(renderer, 200, 215, 230, 255);
                SDL_RenderFillRectF(renderer, &pod);

                for (int w = 0; w < 4; ++w) {
                    const SDL_FRect win{dx + 72.0f + static_cast<float>(w) * 26.0f, 842.0f, 14.0f, 14.0f};
                    SDL_SetRenderDrawColor(renderer, 0, 220, 255, 240);
                    SDL_RenderFillRectF(renderer, &win);
                }
            }
        } else {
            // Niveles 1 al 5: Siluetas de estructuras convencionales
            SDL_SetRenderDrawColor(renderer, midCol.r, midCol.g, midCol.b, 255);
            for (int i = 0; i < 8; ++i) {
                const float structX = bX + static_cast<float>(i) * 240.0f;
                const float structW = 70.0f + static_cast<float>((i % 4) * 25);
                const float structH = 90.0f + static_cast<float>((i % 3) * 45);

                const SDL_FRect sRect{structX, 980.0f - structH, structW, structH};
                SDL_RenderFillRectF(renderer, &sRect);
            }
        }
    }
}

void LevelManager::renderGround(SDL_Renderer* renderer) const {
    const SDL_Rect groundBase{0, 980, 1920, 100};
    SDL_SetRenderDrawColor(renderer,
        m_currentPalette.groundBase.r,
        m_currentPalette.groundBase.g,
        m_currentPalette.groundBase.b,
        255
    );
    SDL_RenderFillRect(renderer, &groundBase);

    const SDL_Rect groundSurface{0, 980, 1920, 4};
    SDL_SetRenderDrawColor(renderer,
        m_currentPalette.groundSurface.r,
        m_currentPalette.groundSurface.g,
        m_currentPalette.groundSurface.b,
        255
    );
    SDL_RenderFillRect(renderer, &groundSurface);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 70);
    for (int i = 0; i < 20; ++i) {
        const float dashX = std::fmod(static_cast<float>(i) * 120.0f - m_groundScroll, 1920.0f);
        const float actualX = (dashX < 0.0f) ? (dashX + 1920.0f) : dashX;
        const SDL_FRect mark{actualX, 995.0f + static_cast<float>((i % 3) * 20), 28.0f, 4.0f};
        SDL_RenderFillRectF(renderer, &mark);
    }
}

void LevelManager::renderMissionBar(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const float barX = 770.0f;
    const float barY = 70.0f;
    const float barW = 380.0f;
    const float barH = 14.0f;

    const SDL_FRect bgRect{barX - 32.0f, barY - 4.0f, barW + 64.0f, barH + 8.0f};
    SDL_SetRenderDrawColor(renderer, 20, 25, 30, 235);
    SDL_RenderFillRectF(renderer, &bgRect);

    const SDL_FRect borderT{barX - 32.0f, barY - 4.0f, barW + 64.0f, 2.0f};
    const SDL_FRect borderB{barX - 32.0f, barY + barH + 2.0f, barW + 64.0f, 2.0f};
    SDL_SetRenderDrawColor(renderer, 70, 85, 100, 255);
    SDL_RenderFillRectF(renderer, &borderT);
    SDL_RenderFillRectF(renderer, &borderB);

    const SDL_FRect emptyBar{barX, barY, barW, barH};
    SDL_SetRenderDrawColor(renderer, 35, 45, 55, 255);
    SDL_RenderFillRectF(renderer, &emptyBar);

    const float progress = getMissionProgress();
    const SDL_FRect fillBar{barX, barY, barW * progress, barH};
    if (progress >= 1.0f) {
        SDL_SetRenderDrawColor(renderer, 240, 50, 50, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 210, 110, 255);
    }
    SDL_RenderFillRectF(renderer, &fillBar);

    const float tankIconX = barX + barW * progress - 10.0f;
    const SDL_FRect tankIcon{tankIconX, barY - 3.0f, 16.0f, 8.0f};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRectF(renderer, &tankIcon);

    const SDL_FRect bossIcon{barX + barW + 8.0f, barY - 4.0f, 18.0f, 18.0f};
    SDL_SetRenderDrawColor(renderer, 235, 40, 40, 255);
    SDL_RenderFillRectF(renderer, &bossIcon);

    const SDL_FRect skullEye1{barX + barW + 11.0f, barY, 4.0f, 4.0f};
    const SDL_FRect skullEye2{barX + barW + 19.0f, barY, 4.0f, 4.0f};
    SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
    SDL_RenderFillRectF(renderer, &skullEye1);
    SDL_RenderFillRectF(renderer, &skullEye2);
}

void LevelManager::renderLevelCompleteBanner(SDL_Renderer* renderer) const {
    if (m_levelCompleteTimer <= 0.0f) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const SDL_FRect bannerBg{460.0f, 420.0f, 1000.0f, 120.0f};
    SDL_SetRenderDrawColor(renderer, 15, 25, 30, 245);
    SDL_RenderFillRectF(renderer, &bannerBg);

    const SDL_FRect bT{460.0f, 420.0f, 1000.0f, 4.0f};
    const SDL_FRect bB{460.0f, 536.0f, 1000.0f, 4.0f};
    SDL_SetRenderDrawColor(renderer, 255, 215, 30, 255);
    SDL_RenderFillRectF(renderer, &bT);
    SDL_RenderFillRectF(renderer, &bB);

    const SDL_FRect accent{500.0f, 470.0f, 920.0f, 16.0f};
    SDL_SetRenderDrawColor(renderer, 0, 220, 120, 255);
    SDL_RenderFillRectF(renderer, &accent);

    const SDL_FRect beaconL{475.0f, 465.0f, 16.0f, 26.0f};
    const SDL_FRect beaconR{1429.0f, 465.0f, 16.0f, 26.0f};
    SDL_SetRenderDrawColor(renderer, 255, 235, 70, 255);
    SDL_RenderFillRectF(renderer, &beaconL);
    SDL_RenderFillRectF(renderer, &beaconR);
}

} // namespace Core
