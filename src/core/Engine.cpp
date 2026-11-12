#include "core/Engine.hpp"
#include "core/AssetManager.hpp"
#include "core/AudioManager.hpp"
#include "math/Collision.hpp"
#include <iostream>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace {

struct SaveHeader {
    uint32_t magic{0x41544F4D}; // 'ATOM'
    uint32_t version{2};
    int maxUnlockedLevel{1};
    uint64_t highScore{0};
    int sfxVolume{80};
    int musicVolume{80};
    uint8_t screenShake{1};
    int touchOpacity{1};
    uint64_t totalEnemiesKilled{0};
    uint32_t totalBossesDefeated{0};
    uint32_t totalNukesUsed{0};
    float bestSurvivalSeconds{0.0f};
    int totalUpgradesPurchased{0};
    uint8_t campaignCompleted{0};
    uint8_t padding[16]{0};
};

constexpr uint8_t FONT_5X7[59][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';'
    {0x08, 0x14, 0x22, 0x41, 0x00}, // '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '='
    {0x00, 0x41, 0x22, 0x14, 0x08}, // '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}  // 'Z'
};

void renderArcadeText(
    SDL_Renderer* renderer, std::string_view str, float x, float y,
    float scale, SDL_Color color, bool center = false)
{
    if (str.empty()) return;
    const int len = static_cast<int>(str.size());

    const float charW = 5.0f * scale;
    const float charGap = 1.0f * scale;
    const float totalW = static_cast<float>(len) * (charW + charGap) - charGap;
    float curX = center ? (x - totalW * 0.5f) : x;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int i = 0; i < len; ++i) {
        char ch = str[i];
        if (ch >= 'a' && ch <= 'z') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
        if (ch >= 32 && ch <= 90) {
            const int idx = ch - 32;
            for (int col = 0; col < 5; ++col) {
                const uint8_t colBits = FONT_5X7[idx][col];
                for (int row = 0; row < 7; ++row) {
                    if ((colBits & (1 << row)) != 0) {
                        const SDL_FRect pixelRect{
                            curX + static_cast<float>(col) * scale,
                            y + static_cast<float>(row) * scale,
                            scale,
                            scale
                        };
                        SDL_RenderFillRectF(renderer, &pixelRect);
                    }
                }
            }
        }
        curX += charW + charGap;
    }
}

} // namespace

namespace Core {

Engine::~Engine() {
    shutdown();
}

bool Engine::init(const char* title, int windowWidth, int windowHeight) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "[ERROR] Fallo al inicializar SDL2: " << SDL_GetError() << '\n';
        return false;
    }

    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!m_window) {
        std::cerr << "[ERROR] Fallo al crear ventana: " << SDL_GetError() << '\n';
        SDL_Quit();
        return false;
    }

    // Políticas de GPU: OpenGL y filtrado lineal para eliminar pixel-dropping en escalado
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    m_renderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        std::cerr << "[ERROR] Fallo al crear renderer: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        SDL_Quit();
        return false;
    }

    if (SDL_RenderSetLogicalSize(m_renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT) != 0) {
        std::cerr << "[WARNING] Fallo al configurar resolución lógica: " << SDL_GetError() << '\n';
    }

    Core::AssetManager::getInstance().init(m_renderer);
    Core::AudioManager::getInstance().init();

    m_inputManager.init();
    loadProgress();
    resetGame();
    m_gameState = GameState::Loading;
    m_loadingTimer = 0.0f;

    std::cout << "[INFO] Motor inicializado correctamente (" << LOGICAL_WIDTH << "x" << LOGICAL_HEIGHT << " lógica).\n";
    std::cout << "[INFO] Mandos conectados: " << SDL_NumJoysticks() << '\n';

    m_isRunning = true;
    return true;
}

void Engine::resetGame() {
    m_lives = 3;
    m_score = 0;
    m_spendablePoints = 0;
    m_levelEnemiesKilled = 0;
    m_levelBombsIntercepted = 0;
    m_levelScoreEarned = 0;
    m_levelIntegrityBonus = 0;
    m_nextLifeScore = 100000;
    m_oneUpFlashTimer = 0.0f;

    m_nukesAvailable = 3;
    m_nukeFlashTimer = 0.0f;
    m_screenShakeTrauma = 0.0f;
    m_shakeOffsetX = 0.0f;
    m_shakeOffsetY = 0.0f;

    m_isRespawning = false;
    m_respawnTimer = 0.0f;
    m_isGameOver = false;
    m_isArmoryOpen = false;
    m_isCampaignWon = false;

    m_levelManager.init();

    m_tank.init(960.0f);
    m_tank.setDestroyed(false);
    m_tank.setInvulnerable(2.5f);
    m_tank.resetUpgrades();

    m_projectilePool.clear();
    m_enemyPool.clear();
    m_particlePool.clear();
    m_powerUpPool.clear();
    m_boss.reset();

    // Suministros espaciados (20 a 26 segundos)
    m_powerUpTimer = 22.0f;

    std::cout << "[SISTEMA] Campaña reiniciada: Sector 1, Vidas = 3, Salud = 100%, Puntos = 0, Nukes = 3.\n";
}

void Engine::jumpToLevel(int level) {
    m_levelManager.resetToLevel(level);
    m_tank.init(960.0f);
    m_tank.setDestroyed(false);
    m_tank.setInvulnerable(2.5f);

    m_projectilePool.clear();
    m_enemyPool.clear();
    m_particlePool.clear();
    m_powerUpPool.clear();
    m_boss.reset();

    m_levelEnemiesKilled = 0;
    m_levelBombsIntercepted = 0;
    m_levelScoreEarned = 0;
    m_levelIntegrityBonus = 0;

    m_isRespawning = false;
    m_respawnTimer = 0.0f;
    m_isGameOver = false;
    m_isArmoryOpen = false;
    m_isCampaignWon = false;
    m_powerUpTimer = 22.0f;

    std::cout << "[DEBUG] Salto directo al Nivel " << level << " (" << m_levelManager.getCurrentPalette().sectorName << ").\n";
}

void Engine::selectArmoryUpgrade(int option) {
    if (!m_isArmoryOpen) {
        return;
    }

    // Curación automática garantizada del 50% de la salud faltante al completar el sector
    m_tank.repairShield((m_tank.getMaxShield() - m_tank.getShield()) * 0.50f);

    switch (option) {
    case 1:
        buyArmoryUpgrade(6); // Titanium Plating I
        break;
    case 2:
        buyArmoryUpgrade(9); // Homing Pod I
        break;
    case 3:
        buyArmoryUpgrade(3); // Barrel Cooler I
        break;
    default:
        break;
    }

    m_isArmoryOpen = false;
    m_gameState = GameState::Playing;
    m_levelManager.advanceLevel();
    Core::AudioManager::getInstance().startMusic();
}

void Engine::buyArmoryUpgrade(int upgradeIndex) {
    const int initialCredits = m_spendablePoints;
    switch (upgradeIndex) {
    // Fila 1 - Potencia de Munición (Progresiva 1->2->3)
    case 0: // AP Rounds I (+3 Daño) - Costo: 3,500
        if (m_tank.getAmmoTier() == 0 && m_spendablePoints >= 3500) {
            m_spendablePoints -= 3500;
            m_tank.setAmmoTier(1);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: AP Rounds I (+3 Daño). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 1: // Tungsten Core II (+3 Daño acumulativo +6) - Costo: 8,000
        if (m_tank.getAmmoTier() == 1 && m_spendablePoints >= 8000) {
            m_spendablePoints -= 8000;
            m_tank.setAmmoTier(2);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Tungsten Core II (+6 Daño total). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 2: // Uranium Shells III (+4 Daño acumulativo +10) - Costo: 16,000
        if (m_tank.getAmmoTier() == 2 && m_spendablePoints >= 16000) {
            m_spendablePoints -= 16000;
            m_tank.setAmmoTier(3);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Uranium Shells III (+10 Daño total). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;

    // Fila 2 - Sistema de Cañón (Progresiva 1->2->3)
    case 3: // Barrel Cooler I (+18% Cadencia de Fuego) - Costo: 4,000
        if (m_tank.getCannonTier() == 0 && m_spendablePoints >= 4000) {
            m_spendablePoints -= 4000;
            m_tank.setCannonTier(1);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Barrel Cooler I (+18% Cadencia). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 4: // Twin Cannon II (Activa disparo doble permanente en paralelo) - Costo: 12,000
        if (m_tank.getCannonTier() == 1 && m_spendablePoints >= 12000) {
            m_spendablePoints -= 12000;
            m_tank.setCannonTier(2);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Twin Cannon II (Doble Cañón Paralelo). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 5: // Gatling Overdrive III (+25% Cadencia adicional) - Costo: 22,000
        if (m_tank.getCannonTier() == 2 && m_spendablePoints >= 22000) {
            m_spendablePoints -= 22000;
            m_tank.setCannonTier(3);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Gatling Overdrive III (+25% Cadencia adicional). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;

    // Fila 3 - Chasis y Blindaje (Progresiva 1->2->3)
    case 6: // Titanium Plating I (+25 Max HP) - Costo: 3,000
        if (m_tank.getChassisTier() == 0 && m_spendablePoints >= 3000) {
            m_spendablePoints -= 3000;
            m_tank.setChassisTier(1);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Titanium Plating I (+25 Max HP). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 7: // Turbo Treads II (+25% Velocidad de traslación del tanque) - Costo: 7,500
        if (m_tank.getChassisTier() == 1 && m_spendablePoints >= 7500) {
            m_spendablePoints -= 7500;
            m_tank.setChassisTier(2);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Turbo Treads II (+25% Velocidad). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 8: // Reactive Armor III (+50 Max HP y reduce 15% todo el daño recibido) - Costo: 15,000
        if (m_tank.getChassisTier() == 2 && m_spendablePoints >= 15000) {
            m_spendablePoints -= 15000;
            m_tank.setChassisTier(3);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Reactive Armor III (+50 HP & -15% Daño). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;

    // Fila 4 - Armas Secundarias Automáticas (Progresiva 1->2->3)
    case 9: // Homing Pod I (Dispara 1 misil teledirigido cada 2.4s) - Costo: 6,000
        if (m_tank.getSecondaryTier() == 0 && m_spendablePoints >= 6000) {
            m_spendablePoints -= 6000;
            m_tank.setSecondaryTier(1);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Homing Pod I (1 Misil / 2.4s). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 10: // Dual Homing II (Dispara 2 misiles teledirigidos cada 1.7s) - Costo: 14,000
        if (m_tank.getSecondaryTier() == 1 && m_spendablePoints >= 14000) {
            m_spendablePoints -= 14000;
            m_tank.setSecondaryTier(2);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Dual Homing II (2 Misiles / 1.7s). Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 11: // Point-Defense Laser III (Destruye la bomba enemiga más cercana cada 2.0s) - Costo: 25,000
        if (m_tank.getSecondaryTier() == 2 && m_spendablePoints >= 25000) {
            m_spendablePoints -= 25000;
            m_tank.setSecondaryTier(3);
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Point-Defense Laser III. Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;

    // Fila 5 - Suministros Tácticos y Vidas (Consumibles / Repetibles)
    case 12: // Full Hull Repair (Restaura el 100% de HP actual) - Costo: 2,500
        if (m_spendablePoints >= 2500 && m_tank.getShield() < m_tank.getMaxShield()) {
            m_spendablePoints -= 2500;
            m_tank.repairShield(m_tank.getMaxShield());
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Reparación Total del Chasis. Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 13: // Tactical Nuke +1 (Añade +1 Bomba Nuke de emergencia) - Costo: 10,000
        if (m_spendablePoints >= 10000) {
            m_spendablePoints -= 10000;
            m_nukesAvailable++;
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] Comprado: Bomba Nuke de Emergencia (+1). Nukes: " << m_nukesAvailable << " | Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    case 14: // EXTRA LIFE (1-UP) (Añade +1 Vida permanente) - Costo: 45,000
        if (m_spendablePoints >= 45000) {
            m_spendablePoints -= 45000;
            m_lives++;
            Core::AudioManager::getInstance().playPowerUp();
            std::cout << "[ARMERÍA] ¡Comprado: 1-UP VIDA EXTRA! Vidas: " << m_lives << " | Créditos restantes: " << m_spendablePoints << '\n';
        }
        break;
    default:
        break;
    }

    if (m_spendablePoints < initialCredits) {
        m_totalUpgradesPurchased++;
        saveProgress();
    }
}

void Engine::shutdown() {
    Core::AudioManager::getInstance().shutdown();
    Core::AssetManager::getInstance().shutdown();
    m_inputManager.shutdown();

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
    m_isRunning = false;
}

void Engine::saveProgress() {
    SDL_RWops* file = SDL_RWFromFile("savegame.dat", "wb");
    if (!file) {
        std::cerr << "[WARNING] No se pudo abrir savegame.dat para escritura.\n";
        return;
    }
    SaveHeader header{};
    header.magic = 0x41544F4D;
    header.version = 2;
    header.maxUnlockedLevel = m_maxUnlockedLevel;
    header.highScore = m_highScore;
    header.sfxVolume = m_sfxVolume;
    header.musicVolume = m_musicVolume;
    header.screenShake = m_screenShakeEnabled ? 1 : 0;
    header.touchOpacity = m_touchOpacityLevel;
    header.totalEnemiesKilled = m_totalEnemiesKilled;
    header.totalBossesDefeated = m_totalBossesDefeated;
    header.totalNukesUsed = m_totalNukesUsed;
    header.bestSurvivalSeconds = m_bestSurvivalSeconds;
    header.totalUpgradesPurchased = m_totalUpgradesPurchased;
    header.campaignCompleted = (m_campaignCompleted || m_isCampaignWon) ? 1 : 0;
    SDL_RWwrite(file, &header, sizeof(SaveHeader), 1);
    SDL_RWclose(file);
    std::cout << "[SISTEMA] Progreso guardado: Nivel Máx = " << m_maxUnlockedLevel
              << " | HighScore = " << m_highScore
              << " | Derribos = " << m_totalEnemiesKilled
              << " | Jefes = " << m_totalBossesDefeated
              << " | Récord Supervivencia = " << static_cast<int>(m_bestSurvivalSeconds) << "s\n";
}

void Engine::loadProgress() {
    SDL_RWops* file = SDL_RWFromFile("savegame.dat", "rb");
    if (!file) {
        std::cout << "[SISTEMA] No se encontró savegame.dat. Inicializando nueva partida.\n";
        m_maxUnlockedLevel = 1;
        m_highScore = 0;
        m_sfxVolume = 80;
        m_musicVolume = 80;
        m_screenShakeEnabled = true;
        m_touchOpacityLevel = 1;
        m_totalEnemiesKilled = 0;
        m_totalBossesDefeated = 0;
        m_totalNukesUsed = 0;
        m_bestSurvivalSeconds = 0.0f;
        m_totalUpgradesPurchased = 0;
        m_campaignCompleted = false;
        m_inputManager.setTouchOpacityLevel(m_touchOpacityLevel);
        return;
    }
    SaveHeader header{};
    const size_t readCount = SDL_RWread(file, &header, sizeof(SaveHeader), 1);
    SDL_RWclose(file);

    if (readCount == 1 && header.magic == 0x41544F4D) {
        m_maxUnlockedLevel = std::clamp(header.maxUnlockedLevel, 1, Core::LevelManager::MAX_LEVELS);
        m_highScore = header.highScore;
        m_sfxVolume = std::clamp(header.sfxVolume, 0, 100);
        m_musicVolume = std::clamp(header.musicVolume, 0, 100);
        Core::AudioManager::getInstance().setSfxVolume(m_sfxVolume);
        Core::AudioManager::getInstance().setMusicVolume(m_musicVolume);
        m_screenShakeEnabled = (header.screenShake != 0);
        m_touchOpacityLevel = std::clamp(header.touchOpacity, 0, 2);
        m_inputManager.setTouchOpacityLevel(m_touchOpacityLevel);

        if (header.version >= 2) {
            m_totalEnemiesKilled = header.totalEnemiesKilled;
            m_totalBossesDefeated = header.totalBossesDefeated;
            m_totalNukesUsed = header.totalNukesUsed;
            m_bestSurvivalSeconds = header.bestSurvivalSeconds;
            m_totalUpgradesPurchased = header.totalUpgradesPurchased;
            m_campaignCompleted = (header.campaignCompleted != 0);
        }

        std::cout << "[SISTEMA] Progreso cargado: Nivel Máx = " << m_maxUnlockedLevel
                  << " | HighScore = " << m_highScore
                  << " | Derribos = " << m_totalEnemiesKilled
                  << " | Jefes = " << m_totalBossesDefeated
                  << " | Récord Supervivencia = " << static_cast<int>(m_bestSurvivalSeconds) << "s\n";
    } else {
        std::cerr << "[WARNING] Archivo savegame.dat inválido o corrupto.\n";
        m_maxUnlockedLevel = 1;
        m_highScore = 0;
        m_sfxVolume = 80;
        m_musicVolume = 80;
        Core::AudioManager::getInstance().setSfxVolume(m_sfxVolume);
        Core::AudioManager::getInstance().setMusicVolume(m_musicVolume);
    }
}

void Engine::processEvents() {
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_isRunning = false;
            return;
        }

        // =====================================================================
        // ESTADO 1: CARGA (Loading)
        // =====================================================================
        if (m_gameState == GameState::Loading) {
            if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN ||
                event.type == SDL_FINGERDOWN || event.type == SDL_CONTROLLERBUTTONDOWN) {
                m_gameState = GameState::MainMenu;
            }
            continue;
        }

        // =====================================================================
        // ESTADO 2: MENÚ PRINCIPAL (MainMenu)
        // =====================================================================
        if (m_gameState == GameState::MainMenu) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
                    m_mainMenuSelectedIndex = (m_mainMenuSelectedIndex + 5) % 6;
                } else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
                    m_mainMenuSelectedIndex = (m_mainMenuSelectedIndex + 1) % 6;
                } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    switch (m_mainMenuSelectedIndex) {
                    case 0:
                        m_isSurvivalMode = false;
                        jumpToLevel(m_maxUnlockedLevel);
                        m_gameState = GameState::Playing;
                        Core::AudioManager::getInstance().startMusic();
                        break;
                    case 1:
                        m_isSurvivalMode = true;
                        m_survivalTimer = 0.0f;
                        m_survivalBossTimer = 60.0f;
                        resetGame();
                        m_gameState = GameState::Playing;
                        Core::AudioManager::getInstance().startMusic();
                        break;
                    case 2:
                        m_levelSelectSelectedIndex = m_maxUnlockedLevel - 1;
                        m_gameState = GameState::LevelSelect;
                        break;
                    case 3:
                        m_previousState = GameState::MainMenu;
                        m_medalsSelectedIndex = 0;
                        m_gameState = GameState::Medals;
                        break;
                    case 4:
                        m_previousState = GameState::MainMenu;
                        m_settingsSelectedIndex = 0;
                        m_gameState = GameState::Settings;
                        break;
                    case 5:
                        m_isRunning = false;
                        break;
                    }
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    m_isRunning = false;
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                    m_mainMenuSelectedIndex = (m_mainMenuSelectedIndex + 5) % 6;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                    m_mainMenuSelectedIndex = (m_mainMenuSelectedIndex + 1) % 6;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A || event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    switch (m_mainMenuSelectedIndex) {
                    case 0:
                        m_isSurvivalMode = false;
                        jumpToLevel(m_maxUnlockedLevel);
                        m_gameState = GameState::Playing;
                        Core::AudioManager::getInstance().startMusic();
                        break;
                    case 1:
                        m_isSurvivalMode = true;
                        m_survivalTimer = 0.0f;
                        m_survivalBossTimer = 60.0f;
                        resetGame();
                        m_gameState = GameState::Playing;
                        Core::AudioManager::getInstance().startMusic();
                        break;
                    case 2:
                        m_levelSelectSelectedIndex = m_maxUnlockedLevel - 1;
                        m_gameState = GameState::LevelSelect;
                        break;
                    case 3:
                        m_previousState = GameState::MainMenu;
                        m_medalsSelectedIndex = 0;
                        m_gameState = GameState::Medals;
                        break;
                    case 4:
                        m_previousState = GameState::MainMenu;
                        m_settingsSelectedIndex = 0;
                        m_gameState = GameState::Settings;
                        break;
                    case 5:
                        m_isRunning = false;
                        break;
                    }
                }
            }

            // Mouse y Touch en Menú Principal (6 botones)
            float clickX = -1.0f;
            float clickY = -1.0f;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                clickX = static_cast<float>(event.button.x);
                clickY = static_cast<float>(event.button.y);
            } else if (event.type == SDL_FINGERDOWN) {
                clickX = event.tfinger.x * static_cast<float>(LOGICAL_WIDTH);
                clickY = event.tfinger.y * static_cast<float>(LOGICAL_HEIGHT);
            }

            if (clickX >= 0.0f) {
                constexpr float btnX = 690.0f;
                constexpr float btnW = 540.0f;
                constexpr float btnH = 56.0f;
                constexpr float btnYs[6] = {300.0f, 368.0f, 436.0f, 504.0f, 572.0f, 640.0f};

                for (int i = 0; i < 6; ++i) {
                    if (clickX >= btnX && clickX <= btnX + btnW && clickY >= btnYs[i] && clickY <= btnYs[i] + btnH) {
                        m_mainMenuSelectedIndex = i;
                        switch (i) {
                        case 0:
                            m_isSurvivalMode = false;
                            jumpToLevel(m_maxUnlockedLevel);
                            m_gameState = GameState::Playing;
                            Core::AudioManager::getInstance().startMusic();
                            break;
                        case 1:
                            m_isSurvivalMode = true;
                            m_survivalTimer = 0.0f;
                            m_survivalBossTimer = 60.0f;
                            resetGame();
                            m_gameState = GameState::Playing;
                            Core::AudioManager::getInstance().startMusic();
                            break;
                        case 2:
                            m_levelSelectSelectedIndex = m_maxUnlockedLevel - 1;
                            m_gameState = GameState::LevelSelect;
                            break;
                        case 3:
                            m_previousState = GameState::MainMenu;
                            m_medalsSelectedIndex = 0;
                            m_gameState = GameState::Medals;
                            break;
                        case 4:
                            m_previousState = GameState::MainMenu;
                            m_settingsSelectedIndex = 0;
                            m_gameState = GameState::Settings;
                            break;
                        case 5:
                            m_isRunning = false;
                            break;
                        }
                        break;
                    }
                }
            }
            continue;
        }

        // =====================================================================
        // ESTADO: MEDALLAS Y CONDECORACIONES (Medals)
        // =====================================================================
        if (m_gameState == GameState::Medals) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    m_gameState = GameState::MainMenu;
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B || event.cbutton.button == SDL_CONTROLLER_BUTTON_A || event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    m_gameState = GameState::MainMenu;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                const float cx = static_cast<float>(event.button.x);
                const float cy = static_cast<float>(event.button.y);
                if (cx >= 710.0f && cx <= 1210.0f && cy >= 740.0f && cy <= 800.0f) {
                    m_gameState = GameState::MainMenu;
                }
            } else if (event.type == SDL_FINGERDOWN) {
                const float cx = event.tfinger.x * static_cast<float>(LOGICAL_WIDTH);
                const float cy = event.tfinger.y * static_cast<float>(LOGICAL_HEIGHT);
                if (cx >= 710.0f && cx <= 1210.0f && cy >= 740.0f && cy <= 800.0f) {
                    m_gameState = GameState::MainMenu;
                }
            }
            continue;
        }

        // =====================================================================
        // ESTADO 3: SELECTOR DE NIVELES (LevelSelect)
        // =====================================================================
        if (m_gameState == GameState::LevelSelect) {
            constexpr float gridStartX = 637.0f;
            constexpr float gridStartY = 175.0f;
            constexpr float btnW = 110.0f;
            constexpr float btnH = 72.0f;
            constexpr float gapX = 24.0f;
            constexpr float gapY = 16.0f;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    m_gameState = GameState::MainMenu;
                    continue;
                } else if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) {
                    if (m_levelSelectSelectedIndex < 25) {
                        const int col = m_levelSelectSelectedIndex % 5;
                        if (col > 0) m_levelSelectSelectedIndex--;
                    }
                } else if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
                    if (m_levelSelectSelectedIndex < 25) {
                        const int col = m_levelSelectSelectedIndex % 5;
                        if (col < 4 && m_levelSelectSelectedIndex < 24) m_levelSelectSelectedIndex++;
                    }
                } else if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
                    if (m_levelSelectSelectedIndex == 25) {
                        m_levelSelectSelectedIndex = 22;
                    } else if (m_levelSelectSelectedIndex >= 5) {
                        m_levelSelectSelectedIndex -= 5;
                    }
                } else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
                    if (m_levelSelectSelectedIndex < 20) {
                        m_levelSelectSelectedIndex += 5;
                    } else {
                        m_levelSelectSelectedIndex = 25;
                    }
                } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    if (m_levelSelectSelectedIndex == 25) {
                        m_gameState = GameState::MainMenu;
                    } else {
                        const int targetLvl = m_levelSelectSelectedIndex + 1;
                        if (targetLvl <= m_maxUnlockedLevel) {
                            jumpToLevel(targetLvl);
                            m_gameState = GameState::Playing;
                            Core::AudioManager::getInstance().startMusic();
                        }
                    }
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
                    m_gameState = GameState::MainMenu;
                    continue;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
                    if (m_levelSelectSelectedIndex < 25 && (m_levelSelectSelectedIndex % 5) > 0) {
                        m_levelSelectSelectedIndex--;
                    }
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                    if (m_levelSelectSelectedIndex < 24 && (m_levelSelectSelectedIndex % 5) < 4) {
                        m_levelSelectSelectedIndex++;
                    }
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                    if (m_levelSelectSelectedIndex == 25) {
                        m_levelSelectSelectedIndex = 22;
                    } else if (m_levelSelectSelectedIndex >= 5) {
                        m_levelSelectSelectedIndex -= 5;
                    }
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                    if (m_levelSelectSelectedIndex < 20) {
                        m_levelSelectSelectedIndex += 5;
                    } else {
                        m_levelSelectSelectedIndex = 25;
                    }
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                    if (m_levelSelectSelectedIndex == 25) {
                        m_gameState = GameState::MainMenu;
                    } else {
                        const int targetLvl = m_levelSelectSelectedIndex + 1;
                        if (targetLvl <= m_maxUnlockedLevel) {
                            jumpToLevel(targetLvl);
                            m_gameState = GameState::Playing;
                            Core::AudioManager::getInstance().startMusic();
                        }
                    }
                }
            }

            // Mouse y Touch en Selector de Niveles
            float clickX = -1.0f;
            float clickY = -1.0f;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                clickX = static_cast<float>(event.button.x);
                clickY = static_cast<float>(event.button.y);
            } else if (event.type == SDL_FINGERDOWN) {
                clickX = event.tfinger.x * static_cast<float>(LOGICAL_WIDTH);
                clickY = event.tfinger.y * static_cast<float>(LOGICAL_HEIGHT);
            }

            if (clickX >= 0.0f) {
                for (int lvl = 1; lvl <= 25; ++lvl) {
                    const int col = (lvl - 1) % 5;
                    const int row = (lvl - 1) / 5;
                    const float bx = gridStartX + static_cast<float>(col) * (btnW + gapX);
                    const float by = gridStartY + static_cast<float>(row) * (btnH + gapY);

                    if (clickX >= bx && clickX <= bx + btnW && clickY >= by && clickY <= by + btnH) {
                        m_levelSelectSelectedIndex = lvl - 1;
                        if (lvl <= m_maxUnlockedLevel) {
                            jumpToLevel(lvl);
                            m_gameState = GameState::Playing;
                            Core::AudioManager::getInstance().startMusic();
                        }
                        break;
                    }
                }

                if (clickX >= 760.0f && clickX <= 1160.0f && clickY >= 650.0f && clickY <= 710.0f) {
                    m_levelSelectSelectedIndex = 25;
                    m_gameState = GameState::MainMenu;
                }
            }
            continue;
        }

        // =====================================================================
        // ESTADO 5: PAUSA (Paused)
        // =====================================================================
        if (m_gameState == GameState::Paused) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_p) {
                    m_gameState = GameState::Playing;
                    Core::AudioManager::getInstance().resumeMusic();
                    continue;
                } else if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
                    m_pauseSelectedIndex = (m_pauseSelectedIndex + 3) % 4;
                } else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
                    m_pauseSelectedIndex = (m_pauseSelectedIndex + 1) % 4;
                } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    switch (m_pauseSelectedIndex) {
                    case 0:
                        m_gameState = GameState::Playing;
                        Core::AudioManager::getInstance().resumeMusic();
                        break;
                    case 1:
                        jumpToLevel(m_levelManager.getCurrentLevel());
                        m_gameState = GameState::Playing;
                        Core::AudioManager::getInstance().startMusic();
                        break;
                    case 2:
                        m_previousState = GameState::Paused;
                        m_settingsSelectedIndex = 0;
                        m_gameState = GameState::Settings;
                        break;
                    case 3:
                        m_gameState = GameState::MainMenu;
                        Core::AudioManager::getInstance().stopMusic();
                        break;
                    }
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START || event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
                    m_gameState = GameState::Playing;
                    Core::AudioManager::getInstance().resumeMusic();
                    continue;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                    m_pauseSelectedIndex = (m_pauseSelectedIndex + 3) % 4;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                    m_pauseSelectedIndex = (m_pauseSelectedIndex + 1) % 4;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                    switch (m_pauseSelectedIndex) {
                    case 0:
                        m_gameState = GameState::Playing;
                        Core::AudioManager::getInstance().resumeMusic();
                        break;
                    case 1:
                        jumpToLevel(m_levelManager.getCurrentLevel());
                        m_gameState = GameState::Playing;
                        Core::AudioManager::getInstance().startMusic();
                        break;
                    case 2:
                        m_previousState = GameState::Paused;
                        m_settingsSelectedIndex = 0;
                        m_gameState = GameState::Settings;
                        break;
                    case 3:
                        m_gameState = GameState::MainMenu;
                        Core::AudioManager::getInstance().stopMusic();
                        break;
                    }
                }
            }

            // Mouse y Touch en Menú de Pausa
            float clickX = -1.0f;
            float clickY = -1.0f;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                clickX = static_cast<float>(event.button.x);
                clickY = static_cast<float>(event.button.y);
            } else if (event.type == SDL_FINGERDOWN) {
                clickX = event.tfinger.x * static_cast<float>(LOGICAL_WIDTH);
                clickY = event.tfinger.y * static_cast<float>(LOGICAL_HEIGHT);
            }

            if (clickX >= 720.0f && clickX <= 1200.0f) {
                if (clickY >= 280.0f && clickY <= 344.0f) {
                    m_pauseSelectedIndex = 0;
                    m_gameState = GameState::Playing;
                    Core::AudioManager::getInstance().resumeMusic();
                } else if (clickY >= 365.0f && clickY <= 429.0f) {
                    m_pauseSelectedIndex = 1;
                    jumpToLevel(m_levelManager.getCurrentLevel());
                    m_gameState = GameState::Playing;
                    Core::AudioManager::getInstance().startMusic();
                } else if (clickY >= 450.0f && clickY <= 514.0f) {
                    m_pauseSelectedIndex = 2;
                    m_previousState = GameState::Paused;
                    m_settingsSelectedIndex = 0;
                    m_gameState = GameState::Settings;
                } else if (clickY >= 535.0f && clickY <= 599.0f) {
                    m_pauseSelectedIndex = 3;
                    m_gameState = GameState::MainMenu;
                    Core::AudioManager::getInstance().stopMusic();
                }
            }
            continue;
        }

        // =====================================================================
        // ESTADO 7: AJUSTES (Settings)
        // =====================================================================
        if (m_gameState == GameState::Settings) {
            auto applySettingsChange = [this](int delta) {
                if (m_settingsSelectedIndex == 0) {
                    m_sfxVolume = std::clamp(m_sfxVolume + delta * 10, 0, 100);
                    Core::AudioManager::getInstance().setSfxVolume(m_sfxVolume);
                } else if (m_settingsSelectedIndex == 1) {
                    m_musicVolume = std::clamp(m_musicVolume + delta * 10, 0, 100);
                    Core::AudioManager::getInstance().setMusicVolume(m_musicVolume);
                } else if (m_settingsSelectedIndex == 2) {
                    m_screenShakeEnabled = !m_screenShakeEnabled;
                    if (!m_screenShakeEnabled) {
                        m_screenShakeTrauma = 0.0f;
                        m_shakeOffsetX = 0.0f;
                        m_shakeOffsetY = 0.0f;
                    }
                } else if (m_settingsSelectedIndex == 3) {
                    m_touchOpacityLevel = std::clamp(m_touchOpacityLevel + delta, 0, 2);
                    m_inputManager.setTouchOpacityLevel(m_touchOpacityLevel);
                }
            };

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    saveProgress();
                    m_gameState = m_previousState;
                    continue;
                } else if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
                    m_settingsSelectedIndex = (m_settingsSelectedIndex + 4) % 5;
                } else if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
                    m_settingsSelectedIndex = (m_settingsSelectedIndex + 1) % 5;
                } else if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) {
                    applySettingsChange(-1);
                } else if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
                    applySettingsChange(1);
                } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    if (m_settingsSelectedIndex == 2) {
                        applySettingsChange(1);
                    } else if (m_settingsSelectedIndex == 4) {
                        saveProgress();
                        m_gameState = m_previousState;
                    }
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
                    saveProgress();
                    m_gameState = m_previousState;
                    continue;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                    m_settingsSelectedIndex = (m_settingsSelectedIndex + 4) % 5;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                    m_settingsSelectedIndex = (m_settingsSelectedIndex + 1) % 5;
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
                    applySettingsChange(-1);
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                    applySettingsChange(1);
                } else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                    if (m_settingsSelectedIndex == 2) {
                        applySettingsChange(1);
                    } else if (m_settingsSelectedIndex == 4) {
                        saveProgress();
                        m_gameState = m_previousState;
                    }
                }
            }

            // Mouse y Touch en Ajustes
            float clickX = -1.0f;
            float clickY = -1.0f;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                clickX = static_cast<float>(event.button.x);
                clickY = static_cast<float>(event.button.y);
            } else if (event.type == SDL_FINGERDOWN) {
                clickX = event.tfinger.x * static_cast<float>(LOGICAL_WIDTH);
                clickY = event.tfinger.y * static_cast<float>(LOGICAL_HEIGHT);
            }

            if (clickX >= 0.0f) {
                if (clickY >= 226.0f && clickY <= 270.0f) {
                    if (clickX >= 860.0f && clickX <= 904.0f) {
                        m_settingsSelectedIndex = 0;
                        m_sfxVolume = std::max(0, m_sfxVolume - 10);
                        Core::AudioManager::getInstance().setSfxVolume(m_sfxVolume);
                    } else if (clickX >= 1250.0f && clickX <= 1294.0f) {
                        m_settingsSelectedIndex = 0;
                        m_sfxVolume = std::min(100, m_sfxVolume + 10);
                        Core::AudioManager::getInstance().setSfxVolume(m_sfxVolume);
                    }
                } else if (clickY >= 316.0f && clickY <= 360.0f) {
                    if (clickX >= 860.0f && clickX <= 904.0f) {
                        m_settingsSelectedIndex = 1;
                        m_musicVolume = std::max(0, m_musicVolume - 10);
                        Core::AudioManager::getInstance().setMusicVolume(m_musicVolume);
                    } else if (clickX >= 1250.0f && clickX <= 1294.0f) {
                        m_settingsSelectedIndex = 1;
                        m_musicVolume = std::min(100, m_musicVolume + 10);
                        Core::AudioManager::getInstance().setMusicVolume(m_musicVolume);
                    }
                } else if (clickY >= 404.0f && clickY <= 452.0f && clickX >= 920.0f && clickX <= 1200.0f) {
                    m_settingsSelectedIndex = 2;
                    m_screenShakeEnabled = !m_screenShakeEnabled;
                    if (!m_screenShakeEnabled) {
                        m_screenShakeTrauma = 0.0f;
                        m_shakeOffsetX = 0.0f;
                        m_shakeOffsetY = 0.0f;
                    }
                } else if (clickY >= 494.0f && clickY <= 542.0f) {
                    if (clickX >= 910.0f && clickX <= 1000.0f) {
                        m_settingsSelectedIndex = 3;
                        m_touchOpacityLevel = 0;
                        m_inputManager.setTouchOpacityLevel(0);
                    } else if (clickX >= 1010.0f && clickX <= 1100.0f) {
                        m_settingsSelectedIndex = 3;
                        m_touchOpacityLevel = 1;
                        m_inputManager.setTouchOpacityLevel(1);
                    } else if (clickX >= 1110.0f && clickX <= 1200.0f) {
                        m_settingsSelectedIndex = 3;
                        m_touchOpacityLevel = 2;
                        m_inputManager.setTouchOpacityLevel(2);
                    }
                } else if (clickY >= 620.0f && clickY <= 680.0f && clickX >= 740.0f && clickX <= 1180.0f) {
                    m_settingsSelectedIndex = 4;
                    saveProgress();
                    m_gameState = m_previousState;
                }
            }
            continue;
        }

        // =====================================================================
        // ESTADO 4: JUEGO EN CURSO (Playing)
        // =====================================================================
        if (m_gameState == GameState::Playing) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_p) {
                    m_pauseSelectedIndex = 0;
                    m_gameState = GameState::Paused;
                    Core::AudioManager::getInstance().pauseMusic();
                    continue;
                }
                // Atajos de Desarrollador
                if (event.key.keysym.sym >= SDLK_F1 && event.key.keysym.sym <= SDLK_F10) {
                    const int targetLvl = (event.key.keysym.sym - SDLK_F1) + 1;
                    jumpToLevel(targetLvl);
                    continue;
                } else if (event.key.keysym.sym == SDLK_PAGEUP) {
                    const int targetLvl = std::min(m_levelManager.getCurrentLevel() + 1, Core::LevelManager::MAX_LEVELS);
                    jumpToLevel(targetLvl);
                    continue;
                } else if (event.key.keysym.sym == SDLK_PAGEDOWN) {
                    const int targetLvl = std::max(m_levelManager.getCurrentLevel() - 1, 1);
                    jumpToLevel(targetLvl);
                    continue;
                } else if (event.key.keysym.sym == SDLK_b) {
                    m_levelManager.triggerBossQuickly();
                    std::cout << "[DEBUG] Misión acelerada al 99%. ¡Jefe inminente!\n";
                    continue;
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    m_pauseSelectedIndex = 0;
                    m_gameState = GameState::Paused;
                    Core::AudioManager::getInstance().pauseMusic();
                    continue;
                }
            }

            // Click o Touch en Botón de Pausa [1820..1890, 20..90]
            float clickX = -1.0f;
            float clickY = -1.0f;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                clickX = static_cast<float>(event.button.x);
                clickY = static_cast<float>(event.button.y);
            } else if (event.type == SDL_FINGERDOWN) {
                clickX = event.tfinger.x * static_cast<float>(LOGICAL_WIDTH);
                clickY = event.tfinger.y * static_cast<float>(LOGICAL_HEIGHT);
            }

            if (clickX >= 1820.0f && clickX <= 1890.0f && clickY >= 20.0f && clickY <= 90.0f) {
                m_pauseSelectedIndex = 0;
                m_gameState = GameState::Paused;
                Core::AudioManager::getInstance().pauseMusic();
                continue;
            }

            m_inputManager.processEvent(event, m_renderer);
            continue;
        }

        // =====================================================================
        // ESTADOS: Resumen de Nivel, Armería, Victoria y Game Over
        // =====================================================================
        if (m_gameState == GameState::LevelSummary) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE ||
                    event.key.keysym.sym == SDLK_KP_ENTER) {
                    m_gameState = GameState::Armory;
                    m_isArmoryOpen = true;
                    m_armorySelectedIndex = 0;
                    return;
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A ||
                    event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    m_gameState = GameState::Armory;
                    m_isArmoryOpen = true;
                    m_armorySelectedIndex = 0;
                    return;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                const float mx = static_cast<float>(event.button.x);
                const float my = static_cast<float>(event.button.y);
                // Botón [PROCEED TO ARMORY >>]: x = 660..1260, y = 735..807
                if (mx >= 660.0f && mx <= 1260.0f && my >= 735.0f && my <= 807.0f) {
                    m_gameState = GameState::Armory;
                    m_isArmoryOpen = true;
                    m_armorySelectedIndex = 0;
                    return;
                }
            } else if (event.type == SDL_FINGERDOWN) {
                const float fx = event.tfinger.x * 1920.0f;
                const float fy = event.tfinger.y * 1080.0f;
                if (fx >= 660.0f && fx <= 1260.0f && fy >= 735.0f && fy <= 807.0f) {
                    m_gameState = GameState::Armory;
                    m_isArmoryOpen = true;
                    m_armorySelectedIndex = 0;
                    return;
                }
            }
            continue;
        }

        if (m_gameState == GameState::Armory || m_isArmoryOpen) {
            if (event.type == SDL_KEYDOWN) {
                const auto sym = event.key.keysym.sym;
                if (sym == SDLK_UP || sym == SDLK_w) {
                    if (m_armorySelectedIndex == 15) {
                        m_armorySelectedIndex = 13; // Fila inferior central
                    } else if (m_armorySelectedIndex >= 3) {
                        m_armorySelectedIndex -= 3;
                    } else {
                        m_armorySelectedIndex = 15; // Botón Deploy
                    }
                } else if (sym == SDLK_DOWN || sym == SDLK_s) {
                    if (m_armorySelectedIndex == 15) {
                        m_armorySelectedIndex = 1; // Fila superior central
                    } else if (m_armorySelectedIndex <= 11) {
                        m_armorySelectedIndex += 3;
                    } else {
                        m_armorySelectedIndex = 15; // Botón Deploy
                    }
                } else if (sym == SDLK_LEFT || sym == SDLK_a) {
                    if (m_armorySelectedIndex != 15 && (m_armorySelectedIndex % 3) > 0) {
                        m_armorySelectedIndex--;
                    }
                } else if (sym == SDLK_RIGHT || sym == SDLK_d) {
                    if (m_armorySelectedIndex != 15 && (m_armorySelectedIndex % 3) < 2) {
                        m_armorySelectedIndex++;
                    }
                } else if (sym == SDLK_RETURN || sym == SDLK_SPACE || sym == SDLK_KP_ENTER) {
                    if (m_armorySelectedIndex == 15) {
                        // Desplegar al siguiente nivel
                        m_levelEnemiesKilled = 0;
                        m_levelBombsIntercepted = 0;
                        m_levelScoreEarned = 0;
                        m_levelIntegrityBonus = 0;
                        m_isArmoryOpen = false;
                        m_gameState = GameState::Playing;
                        m_levelManager.advanceLevel();
                        Core::AudioManager::getInstance().startMusic();
                        return;
                    } else if (m_armorySelectedIndex >= 0 && m_armorySelectedIndex < 15) {
                        buyArmoryUpgrade(m_armorySelectedIndex);
                    }
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                const auto btn = event.cbutton.button;
                if (btn == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                    if (m_armorySelectedIndex == 15) m_armorySelectedIndex = 13;
                    else if (m_armorySelectedIndex >= 3) m_armorySelectedIndex -= 3;
                    else m_armorySelectedIndex = 15;
                } else if (btn == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                    if (m_armorySelectedIndex == 15) m_armorySelectedIndex = 1;
                    else if (m_armorySelectedIndex <= 11) m_armorySelectedIndex += 3;
                    else m_armorySelectedIndex = 15;
                } else if (btn == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
                    if (m_armorySelectedIndex != 15 && (m_armorySelectedIndex % 3) > 0) m_armorySelectedIndex--;
                } else if (btn == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                    if (m_armorySelectedIndex != 15 && (m_armorySelectedIndex % 3) < 2) m_armorySelectedIndex++;
                } else if (btn == SDL_CONTROLLER_BUTTON_A || btn == SDL_CONTROLLER_BUTTON_START) {
                    if (m_armorySelectedIndex == 15) {
                        m_levelEnemiesKilled = 0;
                        m_levelBombsIntercepted = 0;
                        m_levelScoreEarned = 0;
                        m_levelIntegrityBonus = 0;
                        m_isArmoryOpen = false;
                        m_gameState = GameState::Playing;
                        m_levelManager.advanceLevel();
                        Core::AudioManager::getInstance().startMusic();
                        return;
                    } else if (m_armorySelectedIndex >= 0 && m_armorySelectedIndex < 15) {
                        buyArmoryUpgrade(m_armorySelectedIndex);
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                const float mx = static_cast<float>(event.button.x);
                const float my = static_cast<float>(event.button.y);

                // Botón Deploy: x = 560..1360, y = 865..933
                if (mx >= 560.0f && mx <= 1360.0f && my >= 865.0f && my <= 933.0f) {
                    m_levelEnemiesKilled = 0;
                    m_levelBombsIntercepted = 0;
                    m_levelScoreEarned = 0;
                    m_levelIntegrityBonus = 0;
                    m_isArmoryOpen = false;
                    m_gameState = GameState::Playing;
                    m_levelManager.advanceLevel();
                    Core::AudioManager::getInstance().startMusic();
                    return;
                }

                // Clic en alguna de las 15 tarjetas
                for (int i = 0; i < 15; ++i) {
                    const int col = i % 3;
                    const int row = i / 3;
                    const float cx = 80.0f + static_cast<float>(col) * 600.0f;
                    const float cy = 155.0f + static_cast<float>(row) * 140.0f;
                    if (mx >= cx && mx <= cx + 560.0f && my >= cy && my <= cy + 125.0f) {
                        m_armorySelectedIndex = i;
                        buyArmoryUpgrade(i);
                        break;
                    }
                }
            } else if (event.type == SDL_FINGERDOWN) {
                const float fx = event.tfinger.x * 1920.0f;
                const float fy = event.tfinger.y * 1080.0f;

                if (fx >= 560.0f && fx <= 1360.0f && fy >= 865.0f && fy <= 933.0f) {
                    m_levelEnemiesKilled = 0;
                    m_levelBombsIntercepted = 0;
                    m_levelScoreEarned = 0;
                    m_levelIntegrityBonus = 0;
                    m_isArmoryOpen = false;
                    m_gameState = GameState::Playing;
                    m_levelManager.advanceLevel();
                    Core::AudioManager::getInstance().startMusic();
                    return;
                }

                for (int i = 0; i < 15; ++i) {
                    const int col = i % 3;
                    const int row = i / 3;
                    const float cx = 80.0f + static_cast<float>(col) * 600.0f;
                    const float cy = 155.0f + static_cast<float>(row) * 140.0f;
                    if (fx >= cx && fx <= cx + 560.0f && fy >= cy && fy <= cy + 125.0f) {
                        m_armorySelectedIndex = i;
                        buyArmoryUpgrade(i);
                        break;
                    }
                }
            }
            continue;
        }

        if (m_gameState == GameState::Victory || m_isCampaignWon) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                const float mx = static_cast<float>(event.button.x);
                const float my = static_cast<float>(event.button.y);
                if (mx >= 760.0f && mx <= 1160.0f && my >= 640.0f && my <= 720.0f) {
                    resetGame();
                    m_gameState = GameState::MainMenu;
                    Core::AudioManager::getInstance().stopMusic();
                    return;
                }
            } else if (event.type == SDL_FINGERDOWN) {
                const float fx = event.tfinger.x * 1920.0f;
                const float fy = event.tfinger.y * 1080.0f;
                if (fx >= 760.0f && fx <= 1160.0f && fy >= 640.0f && fy <= 720.0f) {
                    resetGame();
                    m_gameState = GameState::MainMenu;
                    Core::AudioManager::getInstance().stopMusic();
                    return;
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_r || event.key.keysym.sym == SDLK_SPACE) {
                    resetGame();
                    m_gameState = GameState::MainMenu;
                    Core::AudioManager::getInstance().stopMusic();
                    return;
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A || event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    resetGame();
                    m_gameState = GameState::MainMenu;
                    Core::AudioManager::getInstance().stopMusic();
                    return;
                }
            }
            continue;
        }

        if (m_gameState == GameState::GameOver || m_isGameOver) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                const float mx = static_cast<float>(event.button.x);
                const float my = static_cast<float>(event.button.y);
                if (mx >= 760.0f && mx <= 1160.0f && my >= 520.0f && my <= 620.0f) {
                    resetGame();
                    m_gameState = GameState::Playing;
                    Core::AudioManager::getInstance().startMusic();
                    return;
                }
            } else if (event.type == SDL_FINGERDOWN) {
                const float fx = event.tfinger.x * 1920.0f;
                const float fy = event.tfinger.y * 1080.0f;
                if (fx >= 760.0f && fx <= 1160.0f && fy >= 520.0f && fy <= 620.0f) {
                    resetGame();
                    m_gameState = GameState::Playing;
                    Core::AudioManager::getInstance().startMusic();
                    return;
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_r || event.key.keysym.sym == SDLK_SPACE) {
                    resetGame();
                    m_gameState = GameState::Playing;
                    Core::AudioManager::getInstance().startMusic();
                    return;
                }
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A || event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                    resetGame();
                    m_gameState = GameState::Playing;
                    Core::AudioManager::getInstance().startMusic();
                    return;
                }
            }
        }
    }
}

void Engine::addScore(uint64_t points) {
    m_score += points;
    m_spendablePoints += static_cast<int>(points);
    m_levelScoreEarned += points;
    if (m_score > m_highScore) {
        m_highScore = m_score;
    }
    while (m_score >= m_nextLifeScore) {
        m_lives++;
        m_nextLifeScore += 100000;
        m_oneUpFlashTimer = 1.0f;
        Core::AudioManager::getInstance().playPowerUp();

        const auto pos = m_tank.getPosition();
        m_particlePool.spawnExplosion(pos.x, pos.y - 40.0f, 30, 220.0f, true);
        std::cout << "[1-UP] ¡Vida Extra Obtenida! Vidas: " << m_lives << " | Próxima vida a los " << m_nextLifeScore << " pts.\n";
    }
}

void Engine::onTankDestroyed() {
    if (m_isRespawning || m_isGameOver) {
        return;
    }

    m_isRespawning = true;
    m_respawnTimer = 2.0f;
    m_lives--;

    Core::AudioManager::getInstance().playExplosion(true);

    const auto pos = m_tank.getPosition();

    m_particlePool.spawnExplosion(pos.x, pos.y - 25.0f, 50, 420.0f, true);
    m_particlePool.spawnExplosion(pos.x - 35.0f, pos.y - 15.0f, 32, 340.0f, true);
    m_particlePool.spawnExplosion(pos.x + 35.0f, pos.y - 15.0f, 32, 340.0f, true);
    m_screenShakeTrauma = 1.0f;

    for (auto& ep : m_enemyPool.getProjectiles()) {
        if (ep.active && std::abs(ep.x - pos.x) < 320.0f && ep.y > 550.0f) {
            ep.active = false;
            m_particlePool.spawnExplosion(ep.x, ep.y, 14, 180.0f, false);
        }
    }

    m_tank.setDestroyed(true);
    if (m_isSurvivalMode && m_survivalTimer > m_bestSurvivalSeconds) {
        m_bestSurvivalSeconds = m_survivalTimer;
    }
    std::cout << "[COMBATE] ¡TANQUE DESTRUIDO! Vidas restantes: " << m_lives << '\n';
}

void Engine::update(float dt) {
    // Micro Hit-Stop para impacto cinemático contundente
    if (m_hitStopTimer > 0.0f) {
        m_hitStopTimer = std::max(0.0f, m_hitStopTimer - dt);
        return;
    }

    if (m_gameState == GameState::Loading) {
        m_loadingTimer += dt;
        if (m_loadingTimer >= 1.8f) {
            m_gameState = GameState::MainMenu;
        }
        return;
    }

    if (m_gameState == GameState::MainMenu || m_gameState == GameState::LevelSelect || m_gameState == GameState::Medals) {
        m_levelManager.update(dt * 0.35f, false);
        m_particlePool.update(dt);
        m_particlePool.updateWeather(dt, 1);
        return;
    }

    if (m_gameState == GameState::Paused || m_gameState == GameState::Settings) {
        return; // Pausado: física totalmente congelada
    }

    if (m_gameState == GameState::GameOver || m_gameState == GameState::LevelSummary || m_gameState == GameState::Armory || m_gameState == GameState::Victory) {
        m_particlePool.update(dt);
        return;
    }

    // A partir de aquí: GameState::Playing
    if (!m_screenShakeEnabled) {
        m_screenShakeTrauma = 0.0f;
        m_shakeOffsetX = 0.0f;
        m_shakeOffsetY = 0.0f;
    }

    const int effectiveLevel = m_isSurvivalMode
        ? std::clamp(1 + static_cast<int>(m_survivalTimer / 30.0f), 1, 25)
        : m_levelManager.getCurrentLevel();

    // Actualización de efectos climáticos dinámicos por nivel
    m_particlePool.updateWeather(dt, effectiveLevel);

    m_inputManager.update();
    const auto& inputState = m_inputManager.getState();

    // 1. Respawn tras destrucción
    if (m_isRespawning) {
        m_respawnTimer -= dt;
        if (m_respawnTimer <= 0.0f) {
            m_isRespawning = false;
            if (m_lives > 0) {
                m_tank.init(960.0f);
                m_tank.setDestroyed(false);
                m_tank.setInvulnerable(3.0f);
                std::cout << "[INFO] Tanque reaparecido en el centro con escudo temporal.\n";
            } else {
                m_isGameOver = true;
                m_gameState = GameState::GameOver;
                Core::AudioManager::getInstance().stopMusic();
                if (m_score > m_highScore) {
                    m_highScore = m_score;
                }
                if (m_isSurvivalMode && m_survivalTimer > m_bestSurvivalSeconds) {
                    m_bestSurvivalSeconds = m_survivalTimer;
                }
                saveProgress();
                std::cout << "[JUEGO] GAME OVER. Puntuación final: " << m_score << '\n';
            }
        }
    }

    // 2. Detonación de Bomba Nuclear (Nuke)
    if (inputState.triggerNuke && m_nukesAvailable > 0 && !m_tank.isDestroyed()) {
        m_nukesAvailable--;
        m_totalNukesUsed++;
        m_hitStopTimer = 0.065f;
        m_nukeFlashTimer = 0.40f;
        Core::AudioManager::getInstance().playNuke();
        m_enemyPool.destroyAllWithNuke(m_particlePool, m_screenShakeTrauma);
        if (m_boss.isActive()) {
            if (m_boss.takeDamage(120, m_particlePool, m_screenShakeTrauma)) {
                m_hitStopTimer = 0.065f;
                m_totalBossesDefeated++;
                Core::AudioManager::getInstance().playExplosion(true);
                if (m_isSurvivalMode) {
                    m_boss.reset();
                    addScore(5000);
                    m_tank.repairShield(30.0f);
                    m_powerUpPool.spawnCrate(m_boss.getPositionX() - 40.0f, m_boss.getPositionY(), Entities::PowerUpType::DamageBoost);
                    m_powerUpPool.spawnCrate(m_boss.getPositionX() + 40.0f, m_boss.getPositionY(), Entities::PowerUpType::ShieldOvercharge);
                    if (m_survivalTimer > m_bestSurvivalSeconds) {
                        m_bestSurvivalSeconds = m_survivalTimer;
                    }
                    saveProgress();
                } else {
                    Core::AudioManager::getInstance().stopMusic();
                    m_levelManager.onBossDefeated();
                    addScore(5000);
                    m_tank.repairShield((m_tank.getMaxShield() - m_tank.getShield()) * 0.50f);
                    if (m_levelManager.getCurrentLevel() + 1 > m_maxUnlockedLevel) {
                        m_maxUnlockedLevel = std::min(m_levelManager.getCurrentLevel() + 1, Core::LevelManager::MAX_LEVELS);
                    }
                    saveProgress();
                    if (m_levelManager.getCurrentLevel() == 25) {
                        m_isCampaignWon = true;
                        m_campaignCompleted = true;
                        m_gameState = GameState::Victory;
                        saveProgress();
                        std::cout << "[VICTORIA TOTAL] ¡OMEGA DOOMSDAY TITAN DESTRUIDO! ¡CAMPAÑA COMPLETADA!\n";
                    } else {
                        m_levelIntegrityBonus = static_cast<int>(m_tank.getShield()) * 10;
                        addScore(m_levelIntegrityBonus);
                        m_gameState = GameState::LevelSummary;
                        Core::AudioManager::getInstance().playPowerUp();
                    }
                }
            }
        }
        std::cout << "[COMBATE] ¡BOMBA NUCLEAR DETONADA! Nukes restantes: " << m_nukesAvailable << '\n';
    }

    // 3. Progresión del Nivel y Parallax / Modo Supervivencia
    if (m_isSurvivalMode) {
        m_levelManager.update(dt * 0.75f, false);
        m_survivalTimer += dt;
        m_survivalBossTimer -= dt;
        if (m_survivalBossTimer <= 0.0f) {
            m_survivalBossTimer = 60.0f;
            if (!m_boss.isActive()) {
                const int bossLvl = std::clamp((static_cast<int>(m_survivalTimer / 60.0f) % 25) + 1, 1, 25);
                m_boss.spawn(bossLvl);
                Core::AudioManager::getInstance().playAlert();
                std::cout << "[SUPERVIVENCIA] ¡Aparece Boss en oleada! Nivel: " << bossLvl << " | Nombre: " << m_boss.getName() << '\n';
            }
        }
    } else {
        m_levelManager.update(dt, m_boss.isActive());
        // 4. Activación del Boss al completarse la misión
        if (m_levelManager.isWavePhaseDone() && !m_boss.isActive() && !m_levelManager.isLevelCompleteTransition()) {
            m_boss.spawn(m_levelManager.getCurrentLevel());
            Core::AudioManager::getInstance().playAlert();
            std::cout << "[ALERTA] ¡BOSS DEL NIVEL " << m_levelManager.getCurrentLevel() << " DETECTADO: " << m_boss.getName() << "!\n";
        }
    }

    // 5. Helicóptero Aliado de Suministros (Espaciado de 20.0f a 26.0f segundos)
    m_powerUpTimer -= dt;
    if (m_powerUpTimer <= 0.0f) {
        m_shakeRngState = m_shakeRngState * 1664525u + 1013904223u;
        m_powerUpTimer = 20.0f + static_cast<float>(m_shakeRngState % 600u) * 0.01f;
        const auto pType = static_cast<Entities::PowerUpType>((m_shakeRngState / 11u) % 6u);
        const int heliDir = (m_shakeRngState % 2 == 0) ? 1 : -1;
        m_powerUpPool.triggerSupplyHeli(pType, heliDir);
        std::cout << "[SUMINISTROS] ¡Helicóptero aliado en camino con suministros tácticos!\n";
    }

    // 6. Cinemática del Tanque (con eyector de casquillos y sobrecarga)
    m_tank.update(dt, inputState, m_projectilePool, m_particlePool);

    // 7. Guiado de misiles homing del tanque
    if (m_tank.getHomingPodLevel() > 0) {
        if (m_boss.isActive() && !m_boss.isDefeated()) {
            const auto bossBox = m_boss.getBoundingBox();
            m_projectilePool.updateHoming(dt, bossBox.x + bossBox.w * 0.5f, bossBox.y + bossBox.h * 0.5f);
        } else {
            // Guiado hacia el enemigo regular más cercano
            float bestDistSq = 999999999.0f;
            float targetX = 960.0f;
            float targetY = 200.0f;
            const auto tankPos = m_tank.getPosition();

            for (const auto& e : m_enemyPool.getEnemies()) {
                if (e.active) {
                    const float d = (e.x - tankPos.x) * (e.x - tankPos.x) + (e.y - tankPos.y) * (e.y - tankPos.y);
                    if (d < bestDistSq) {
                        bestDistSq = d;
                        targetX = e.x;
                        targetY = e.y;
                    }
                }
            }
            m_projectilePool.updateHoming(dt, targetX, targetY);
        }
    }

    // 7B. Láser de Defensa de Punto (Nivel 3 de Armas Secundarias)
    if (m_tank.hasPointDefenseLaser() && m_tank.getPointDefenseCooldown() <= 0.0f) {
        float closestDistSq = 550.0f * 550.0f;
        Entities::EnemyProjectile* targetBomb = nullptr;
        const auto laserOrigin = m_tank.hasWingmanDrone() ? m_tank.getWingmanDronePosition() : m_tank.getTurretPivot();

        for (auto& ep : m_enemyPool.getProjectiles()) {
            if (ep.active) {
                const float dx = ep.x - laserOrigin.x;
                const float dy = ep.y - laserOrigin.y;
                const float d2 = dx * dx + dy * dy;
                if (d2 < closestDistSq) {
                    closestDistSq = d2;
                    targetBomb = &ep;
                }
            }
        }

        if (targetBomb != nullptr) {
            targetBomb->active = false;
            m_tank.resetPointDefenseCooldown();
            Core::AudioManager::getInstance().playExplosion(false);
            m_particlePool.spawnExplosion(targetBomb->x, targetBomb->y, 16, 190.0f, false);
            // Traza de energía del haz defensivo desde el dron o la torreta
            for (int i = 0; i < 7; ++i) {
                const float t = static_cast<float>(i) / 6.0f;
                m_particlePool.spawnExplosion(laserOrigin.x + (targetBomb->x - laserOrigin.x) * t,
                                              laserOrigin.y + (targetBomb->y - laserOrigin.y) * t,
                                              2, 50.0f, false);
            }
            m_levelBombsIntercepted++;
            addScore(50);
        }
    }

    // 8. Ecosistema de Enemigos y Bombas (En supervivencia los aviones siguen apareciendo con Boss)
    const auto tankPos = m_tank.getPosition();
    m_enemyPool.update(dt, tankPos.x, tankPos.y, effectiveLevel, m_isSurvivalMode ? false : m_boss.isActive());

    // 9. Actualización del Boss
    m_boss.update(dt, tankPos.x, tankPos.y, m_enemyPool, m_particlePool, m_screenShakeTrauma);

    // 10. Actualización de Power-Ups y Helicóptero Aliado
    m_powerUpPool.update(dt);

    // 11. Balas del Jugador
    m_projectilePool.update(dt);

    // 12. Resolución de Colisiones y Daño
    resolveCollisions();

    // 13. Simulación de Partículas y Explosiones
    m_particlePool.update(dt);

    // 14. Amortiguamiento paramétrico de Screen Shake (Trauma Decay)
    if (m_screenShakeTrauma > 0.0f) {
        m_screenShakeTrauma -= dt * 1.5f;
        if (m_screenShakeTrauma < 0.0f) {
            m_screenShakeTrauma = 0.0f;
        }

        const float shakeAmount = (m_screenShakeTrauma * m_screenShakeTrauma) * 22.0f;
        m_shakeRngState = m_shakeRngState * 1664525u + 1013904223u;
        const float rx = (static_cast<float>(m_shakeRngState & 0xFFFFu) / 32767.5f) - 1.0f;
        m_shakeRngState = m_shakeRngState * 1664525u + 1013904223u;
        const float ry = (static_cast<float>(m_shakeRngState & 0xFFFFu) / 32767.5f) - 1.0f;

        m_shakeOffsetX = rx * shakeAmount;
        m_shakeOffsetY = ry * shakeAmount;
    } else {
        m_shakeOffsetX = 0.0f;
        m_shakeOffsetY = 0.0f;
    }

    // 15. Temporizadores de destellos visuales
    if (m_nukeFlashTimer > 0.0f) {
        m_nukeFlashTimer -= dt;
        if (m_nukeFlashTimer < 0.0f) m_nukeFlashTimer = 0.0f;
    }
    if (m_oneUpFlashTimer > 0.0f) {
        m_oneUpFlashTimer -= dt;
        if (m_oneUpFlashTimer < 0.0f) m_oneUpFlashTimer = 0.0f;
    }
}

void Engine::resolveCollisions() {
    auto& playerBullets = m_projectilePool.getProjectiles();
    auto& enemies = m_enemyPool.getEnemies();
    auto& enemyProjectiles = m_enemyPool.getProjectiles();
    auto& icbms = m_enemyPool.getIcbms();
    const auto tankBox = m_tank.getBoundingBox();

    // -------------------------------------------------------------------------
    // 0. RAYO DEVASTADOR MEGALASER (SI ESTÁ ACTIVO)
    // -------------------------------------------------------------------------
    if (m_tank.isMegalaserActive()) {
        const float beamX = m_tank.getBarrelTip().x;
        constexpr float BEAM_HALF_WIDTH = 22.0f;

        // Daño a aviones regulares en la columna del haz
        for (auto& enemy : enemies) {
            if (enemy.active && std::abs(enemy.x - beamX) <= (enemy.width * 0.5f + BEAM_HALF_WIDTH)) {
                enemy.hp -= 2;
                enemy.hitFlashTimer = 0.08f;
                m_particlePool.spawnExplosion(enemy.x, enemy.y, 4, 140.0f, false);
                if (enemy.hp <= 0) {
                    enemy.active = false;
                    Core::AudioManager::getInstance().playExplosion(false);
                    m_particlePool.spawnExplosion(enemy.x, enemy.y, 35, 320.0f, true);
                    m_levelEnemiesKilled++;
                    addScore(Entities::EnemyPool::calculateKillScore(enemy.type, m_levelManager.getCurrentLevel()));
                }
            }
        }

        // Daño al Boss en la columna del haz
        if (m_boss.isActive() && !m_boss.isDefeated()) {
            const auto bossBox = m_boss.getBoundingBox();
            if (beamX >= bossBox.x && beamX <= bossBox.x + bossBox.w) {
                if (m_boss.takeDamage(2, m_particlePool, m_screenShakeTrauma)) {
                    m_hitStopTimer = 0.065f;
                    m_totalBossesDefeated++;
                    Core::AudioManager::getInstance().playExplosion(true);
                    if (m_isSurvivalMode) {
                        m_boss.reset();
                        addScore(5000);
                        m_tank.repairShield(30.0f);
                        m_powerUpPool.spawnCrate(m_boss.getPositionX() - 40.0f, m_boss.getPositionY(), Entities::PowerUpType::DamageBoost);
                        m_powerUpPool.spawnCrate(m_boss.getPositionX() + 40.0f, m_boss.getPositionY(), Entities::PowerUpType::ShieldOvercharge);
                        if (m_survivalTimer > m_bestSurvivalSeconds) {
                            m_bestSurvivalSeconds = m_survivalTimer;
                        }
                        saveProgress();
                    } else {
                        Core::AudioManager::getInstance().stopMusic();
                        m_levelManager.onBossDefeated();
                        addScore(5000);
                        m_tank.repairShield((m_tank.getMaxShield() - m_tank.getShield()) * 0.50f);
                        if (m_levelManager.getCurrentLevel() + 1 > m_maxUnlockedLevel) {
                            m_maxUnlockedLevel = std::min(m_levelManager.getCurrentLevel() + 1, Core::LevelManager::MAX_LEVELS);
                        }
                        saveProgress();
                        if (m_levelManager.getCurrentLevel() == 25) {
                            m_isCampaignWon = true;
                            m_campaignCompleted = true;
                            m_gameState = GameState::Victory;
                            saveProgress();
                        } else {
                            m_levelIntegrityBonus = static_cast<int>(m_tank.getShield()) * 10;
                            addScore(m_levelIntegrityBonus);
                            m_gameState = GameState::LevelSummary;
                            Core::AudioManager::getInstance().playPowerUp();
                        }
                    }
                }
            }
        }

        // Daño a los ICBMs en la columna del haz
        for (auto& icbm : icbms) {
            if (icbm.active && icbm.isDescending && std::abs(icbm.x - beamX) <= 24.0f) {
                icbm.hp -= 3;
                if (icbm.hp <= 0) {
                    icbm.active = false;
                    Core::AudioManager::getInstance().playExplosion(false);
                    m_particlePool.spawnExplosion(icbm.x, icbm.y, 45, 380.0f, true);
                    m_levelBombsIntercepted++;
                    addScore(1500);
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // 0B. ORBES TÁCTICOS DE DEFENSA DEL TANQUE (VAPORIZAN BOMBAS)
    // -------------------------------------------------------------------------
    if (m_tank.hasDefenseOrbs()) {
        const auto pivot = m_tank.getTurretPivot();
        const float angle = m_tank.getOrbAngle();
        constexpr float ORB_RADIUS = 46.0f;

        const float ox1 = pivot.x + std::cos(angle) * ORB_RADIUS;
        const float oy1 = pivot.y + std::sin(angle) * ORB_RADIUS * 0.5f;

        const float ox2 = pivot.x + std::cos(angle + 3.14159f) * ORB_RADIUS;
        const float oy2 = pivot.y + std::sin(angle + 3.14159f) * ORB_RADIUS * 0.5f;

        for (auto& ep : enemyProjectiles) {
            if (!ep.active) continue;

            const float d1 = (ep.x - ox1) * (ep.x - ox1) + (ep.y - oy1) * (ep.y - oy1);
            const float d2 = (ep.x - ox2) * (ep.x - ox2) + (ep.y - oy2) * (ep.y - oy2);

            if (d1 <= (18.0f * 18.0f) || d2 <= (18.0f * 18.0f)) {
                ep.active = false;
                Core::AudioManager::getInstance().playExplosion(false);
                m_particlePool.spawnExplosion(ep.x, ep.y, 14, 160.0f, false);
                m_levelBombsIntercepted++;
                addScore(50);
            }
        }
    }

    // -------------------------------------------------------------------------
    // A. BALAS DEL JUGADOR VS ENEMIGOS, BOMBAS, ICBM, BOSS Y HELICÓPTERO ALIADO
    // -------------------------------------------------------------------------
    for (auto& bullet : playerBullets) {
        if (!bullet.active) {
            continue;
        }

        constexpr float BULLET_RADIUS = 5.0f;
        bool bulletConsumed = false;

        // 1. Fuego Amigo contra Helicóptero Aliado de Suministros (-1,500 pts si se destruye)
        if (m_powerUpPool.checkFriendlyFire(bullet.x, bullet.y, BULLET_RADIUS, bullet.damage,
                                            m_particlePool, m_score, m_screenShakeTrauma)) {
            bullet.active = false;
            continue;
        }

        // 2. Balas vs Boss (Vida Masiva y Enrage)
        if (m_boss.isActive() && !m_boss.isDefeated()) {
            const auto bossBox = m_boss.getBoundingBox();
            if (Math::checkCircleAABB(bullet.x, bullet.y, BULLET_RADIUS, bossBox.x, bossBox.y, bossBox.w, bossBox.h)) {
                bullet.active = false;
                bulletConsumed = true;
                Core::AudioManager::getInstance().playHit();

                // Destello proporcional al daño (rojo/oro para daño 15-20)
                if (bullet.isCritical || bullet.damage >= 15) {
                    m_particlePool.spawnExplosion(bullet.x, bullet.y, 10, 220.0f, false);
                } else {
                    m_particlePool.spawnExplosion(bullet.x, bullet.y, 5, 120.0f, false);
                }

                if (m_boss.takeDamage(bullet.damage, m_particlePool, m_screenShakeTrauma)) {
                    m_hitStopTimer = 0.065f;
                    m_totalBossesDefeated++;
                    Core::AudioManager::getInstance().playExplosion(true);
                    if (m_isSurvivalMode) {
                        m_boss.reset();
                        addScore(5000);
                        m_tank.repairShield(30.0f);
                        m_powerUpPool.spawnCrate(m_boss.getPositionX() - 40.0f, m_boss.getPositionY(), Entities::PowerUpType::DamageBoost);
                        m_powerUpPool.spawnCrate(m_boss.getPositionX() + 40.0f, m_boss.getPositionY(), Entities::PowerUpType::ShieldOvercharge);
                        if (m_survivalTimer > m_bestSurvivalSeconds) {
                            m_bestSurvivalSeconds = m_survivalTimer;
                        }
                        saveProgress();
                    } else {
                        Core::AudioManager::getInstance().stopMusic();
                        m_levelManager.onBossDefeated();
                        addScore(5000);
                        // Curación táctica del 50% de la salud faltante
                        m_tank.repairShield((m_tank.getMaxShield() - m_tank.getShield()) * 0.50f);
                        if (m_levelManager.getCurrentLevel() + 1 > m_maxUnlockedLevel) {
                            m_maxUnlockedLevel = std::min(m_levelManager.getCurrentLevel() + 1, Core::LevelManager::MAX_LEVELS);
                        }
                        saveProgress();
                        if (m_levelManager.getCurrentLevel() == 25) {
                            m_isCampaignWon = true;
                            m_campaignCompleted = true;
                            m_gameState = GameState::Victory;
                            saveProgress();
                            std::cout << "[VICTORIA TOTAL] ¡OMEGA DOOMSDAY TITAN DESTRUIDO! ¡CAMPAÑA COMPLETADA!\n";
                        } else {
                            m_levelIntegrityBonus = static_cast<int>(m_tank.getShield()) * 10;
                            addScore(m_levelIntegrityBonus);
                            m_gameState = GameState::LevelSummary;
                            Core::AudioManager::getInstance().playPowerUp();
                            std::cout << "[VICTORIA] ¡Boss destruido! +5,000 pts. Bonus Integridad: +" << m_levelIntegrityBonus << " pts. Mostrando Resumen...\n";
                        }
                    }
                }
                continue;
            }
        }

        // 3. Balas vs Misiles Balísticos ICBM (40 HP)
        for (auto& icbm : icbms) {
            if (icbm.active && icbm.isDescending) {
                if (Math::checkCircleAABB(bullet.x, bullet.y, BULLET_RADIUS, icbm.x - 8.0f, icbm.y - 24.0f, 16.0f, 44.0f)) {
                    bullet.active = false;
                    bulletConsumed = true;
                    Core::AudioManager::getInstance().playHit();
                    icbm.hp -= bullet.damage;
                    icbm.hitFlashTimer = 0.08f;
                    m_particlePool.spawnExplosion(bullet.x, bullet.y, 8, 160.0f, false);

                    if (icbm.hp <= 0) {
                        icbm.active = false;
                        Core::AudioManager::getInstance().playExplosion(false);
                        m_particlePool.spawnExplosion(icbm.x, icbm.y, 45, 380.0f, true);
                        m_screenShakeTrauma = std::min(m_screenShakeTrauma + 0.40f, 1.0f);
                        m_levelBombsIntercepted++;
                        addScore(1500);
                    }
                    break;
                }
            }
        }

        if (bulletConsumed) {
            continue;
        }

        // 4. Balas vs 10 Arquetipos de Aviones y Helicópteros
        for (auto& enemy : enemies) {
            if (!enemy.active) {
                continue;
            }

            const float rx = enemy.x - enemy.width * 0.5f;
            const float ry = enemy.y - enemy.height * 0.5f;

            if (Math::checkCircleAABB(bullet.x, bullet.y, BULLET_RADIUS, rx, ry, enemy.width, enemy.height)) {
                if (!bullet.isPiercing) {
                    bullet.active = false;
                    bulletConsumed = true;
                }
                Core::AudioManager::getInstance().playHit();
                enemy.hp -= bullet.damage;
                enemy.hitFlashTimer = 0.06f;

                // Destello proporcional al daño infligido
                if (bullet.isPiercing || bullet.isCritical || bullet.damage >= 15) {
                    m_particlePool.spawnExplosion(bullet.x, bullet.y, 10, 220.0f, false);
                } else {
                    m_particlePool.spawnExplosion(bullet.x, bullet.y, 5, 120.0f, false);
                }

                if (enemy.hp <= 0) {
                    enemy.active = false;
                    Core::AudioManager::getInstance().playExplosion(false);
                    const bool isHeavy = (enemy.type == Entities::EnemyType::HeavyBomber ||
                                          enemy.type == Entities::EnemyType::Gunship ||
                                          enemy.type == Entities::EnemyType::ArmoredZeppelin);

                    const int pCount = isHeavy ? 45 : 24;
                    const float expSpeed = isHeavy ? 380.0f : 270.0f;
                    m_particlePool.spawnExplosion(enemy.x, enemy.y, pCount, expSpeed, true);
                    m_screenShakeTrauma = std::min(m_screenShakeTrauma + (isHeavy ? 0.30f : 0.18f), 1.0f);

                    m_levelEnemiesKilled++;
                    m_totalEnemiesKilled++;
                    m_tank.addOverdrive(8.0f);
                    const uint64_t scoreReward = Entities::EnemyPool::calculateKillScore(
                        enemy.type, m_levelManager.getCurrentLevel());
                    addScore(scoreReward);

                    // Probabilidad del 4% de soltar caja de suministros al derribar aviones pesados
                    if (isHeavy) {
                        m_shakeRngState = m_shakeRngState * 1664525u + 1013904223u;
                        if ((m_shakeRngState % 100u) < 4u) {
                            const auto pType = static_cast<Entities::PowerUpType>((m_shakeRngState / 13u) % 6u);
                            m_powerUpPool.spawnCrate(enemy.x, enemy.y, pType);
                        }
                    }
                }
                break;
            }
        }

        if (bulletConsumed) {
            continue;
        }

        // 5. Balas vs Bombas, Misiles y Minas Enemigas (Interceptación = 50 pts)
        for (auto& ep : enemyProjectiles) {
            if (!ep.active) {
                continue;
            }

            if (Math::checkCircleCircle(bullet.x, bullet.y, BULLET_RADIUS, ep.x, ep.y, ep.radius)) {
                bullet.active = false;
                ep.active = false;
                Core::AudioManager::getInstance().playExplosion(false);

                m_particlePool.spawnExplosion(ep.x, ep.y, 14, 180.0f, false);
                m_screenShakeTrauma = std::min(m_screenShakeTrauma + 0.10f, 1.0f);
                m_levelBombsIntercepted++;
                addScore(50);
                break;
            }
        }
    }

    // -------------------------------------------------------------------------
    // B. BOMBAS, MISILES, MINAS E ICBM VS SUELO Y TANQUE
    // -------------------------------------------------------------------------
    // 1. Misiles balísticos ICBM impactando
    for (auto& icbm : icbms) {
        if (icbm.active && icbm.isDescending) {
            if (icbm.y >= Entities::Tank::GROUND_Y - 20.0f) {
                icbm.active = false;
                m_particlePool.spawnExplosion(icbm.x, Entities::Tank::GROUND_Y, 65, 460.0f, true);
                m_screenShakeTrauma = 1.0f;

                if (!m_tank.isDestroyed() && std::abs(icbm.x - (tankBox.x + tankBox.w * 0.5f)) < 150.0f) {
                    const int currentLvl = m_levelManager.getCurrentLevel();
                    const float icbmDmg = (currentLvl <= 5) ? 15.0f : (15.0f + static_cast<float>(currentLvl - 5) * 0.75f);
                    m_tank.takeDamage(icbmDmg);
                    if (m_tank.getShield() <= 0.0f) {
                        onTankDestroyed();
                    }
                }
            }
        }
    }

    // 2. Proyectiles enemigos regulares o de Boss
    for (auto& ep : enemyProjectiles) {
        if (!ep.active) {
            continue;
        }

        // Impacto contra el suelo
        if (ep.y >= Entities::Tank::GROUND_Y) {
            ep.active = false;
            m_particlePool.spawnExplosion(ep.x, Entities::Tank::GROUND_Y, 16, 200.0f, false);
            m_screenShakeTrauma = std::min(m_screenShakeTrauma + 0.12f, 1.0f);
            continue;
        }

        // Impacto contra el Tanque
        if (!m_tank.isDestroyed() && Math::checkCircleAABB(ep.x, ep.y, ep.radius, tankBox.x, tankBox.y, tankBox.w, tankBox.h)) {
            ep.active = false;

            if (!m_tank.isInvulnerable() && !m_tank.isShieldOvercharged()) {
                float dmg = static_cast<float>(ep.damage);
                if (dmg <= 0.0f) {
                    const int currentLvl = m_levelManager.getCurrentLevel();
                    if (ep.isBossProjectile || m_boss.isActive()) {
                        const int bossProjDmg = (currentLvl == 10) ? 11 : ((currentLvl == 15) ? 14 : (5 + static_cast<int>(std::round(static_cast<float>(currentLvl) * 0.65f))));
                        dmg = static_cast<float>(ep.isMissile ? bossProjDmg : (bossProjDmg + 4));
                    } else {
                        const float lvlRatio = static_cast<float>(std::clamp(currentLvl, 1, 24) - 1) / 23.0f;
                        const float basePlaneDmg = 8.0f + lvlRatio * 8.0f; // 8.0f en lvl 1 a 16.0f en lvl 24
                        dmg = ep.isMissile ? (basePlaneDmg + 4.0f) : basePlaneDmg;
                    }
                }
                m_tank.takeDamage(dmg);
                m_particlePool.spawnExplosion(ep.x, ep.y, 28, 300.0f, true);
                m_screenShakeTrauma = std::min(m_screenShakeTrauma + 0.45f, 1.0f);

                if (m_tank.getShield() <= 0.0f) {
                    onTankDestroyed();
                }
            } else {
                m_particlePool.spawnExplosion(ep.x, ep.y, 14, 220.0f, false);
            }
        }
    }

    // 3. Daño por rayo láser de satélite orbital
    for (const auto& enemy : enemies) {
        if (enemy.active && enemy.type == Entities::EnemyType::LaserSatellite && enemy.isLaserFiring) {
            if (!m_tank.isDestroyed() && std::abs(enemy.x - (tankBox.x + tankBox.w * 0.5f)) < 26.0f) {
                if (!m_tank.isInvulnerable() && !m_tank.isShieldOvercharged()) {
                    m_tank.takeDamage(45.0f * FIXED_DELTA_TIME);
                    m_particlePool.spawnExplosion(tankBox.x + tankBox.w * 0.5f, Entities::Tank::GROUND_Y - 20.0f, 4, 120.0f, false);
                    if (m_tank.getShield() <= 0.0f) {
                        onTankDestroyed();
                    }
                }
            }
        }
    }

    // 4. Daño por Cañón de Riel (Railgun) de Doomsday Express (Jefe 10)
    const int currentLvl = m_levelManager.getCurrentLevel();
    const int bossBaseDmg = (currentLvl == 10) ? 11 : ((currentLvl == 15) ? 14 : (5 + static_cast<int>(std::round(static_cast<float>(currentLvl) * 0.65f))));
    const float bossBeamDmgPerSec = static_cast<float>(bossBaseDmg + 4) * 3.5f;

    if (m_boss.isActive() && !m_boss.isDefeated() && m_boss.isRailgunFiring()) {
        const float beamX = m_boss.getRailgunX();
        if (!m_tank.isDestroyed() && std::abs(beamX - (tankBox.x + tankBox.w * 0.5f)) < 30.0f) {
            if (!m_tank.isInvulnerable() && !m_tank.isShieldOvercharged()) {
                m_tank.takeDamage(bossBeamDmgPerSec * 1.1f * FIXED_DELTA_TIME);
                m_particlePool.spawnExplosion(beamX, Entities::Tank::GROUND_Y - 15.0f, 6, 180.0f, false);
                if (m_tank.getShield() <= 0.0f) {
                    onTankDestroyed();
                }
            }
        }
    }

    // 5. Daño por Chorro de Magma de Vulcan Forge-Master (Jefe 11)
    if (m_boss.isActive() && !m_boss.isDefeated() && m_boss.isFlameJetActive()) {
        const float jetX = m_boss.getFlameJetX();
        if (!m_tank.isDestroyed() && std::abs(jetX - (tankBox.x + tankBox.w * 0.5f)) < 36.0f) {
            if (!m_tank.isInvulnerable() && !m_tank.isShieldOvercharged()) {
                m_tank.takeDamage(bossBeamDmgPerSec * 0.9f * FIXED_DELTA_TIME);
                m_particlePool.spawnExplosion(jetX, Entities::Tank::GROUND_Y - 15.0f, 5, 140.0f, false);
                if (m_tank.getShield() <= 0.0f) {
                    onTankDestroyed();
                }
            }
        }
    }

    // 6. Daño por Relámpago Vertical de Tempest Archon (Jefe 14)
    if (m_boss.isActive() && !m_boss.isDefeated() && m_boss.isLightningFiring()) {
        const float boltX = m_boss.getLightningX();
        if (!m_tank.isDestroyed() && std::abs(boltX - (tankBox.x + tankBox.w * 0.5f)) < 32.0f) {
            if (!m_tank.isInvulnerable() && !m_tank.isShieldOvercharged()) {
                m_tank.takeDamage(bossBeamDmgPerSec * 1.15f * FIXED_DELTA_TIME);
                m_particlePool.spawnExplosion(boltX, Entities::Tank::GROUND_Y - 15.0f, 6, 200.0f, false);
                if (m_tank.getShield() <= 0.0f) {
                    onTankDestroyed();
                }
            }
        }
    }

    // 7. Daño por Rayos Láser Duales de OMEGA DOOMSDAY TITAN (Jefe 25)
    if (m_boss.isActive() && !m_boss.isDefeated() && m_boss.isOmegaLaserFiring()) {
        const float beamX1 = m_boss.getOmegaLaserX1();
        const float beamX2 = m_boss.getOmegaLaserX2();
        const float tankCenterX = tankBox.x + tankBox.w * 0.5f;
        if (!m_tank.isDestroyed() && (std::abs(beamX1 - tankCenterX) < 28.0f || std::abs(beamX2 - tankCenterX) < 28.0f)) {
            if (!m_tank.isInvulnerable() && !m_tank.isShieldOvercharged()) {
                m_tank.takeDamage(bossBeamDmgPerSec * 1.05f * FIXED_DELTA_TIME);
                m_particlePool.spawnExplosion(tankCenterX, Entities::Tank::GROUND_Y - 15.0f, 6, 180.0f, false);
                if (m_tank.getShield() <= 0.0f) {
                    onTankDestroyed();
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // C. CAJAS DE SUMINISTROS (POWER-UPS) VS TANQUE
    // -------------------------------------------------------------------------
    m_powerUpPool.checkCollisionsWithTank(m_tank, m_particlePool);
}

void Engine::render7SegmentDigit(
    SDL_Renderer* renderer, float x, float y, int digit,
    SDL_Color litColor, SDL_Color unlitColor)
{
    constexpr uint8_t MASKS[10] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
    };
    const uint8_t mask = (digit >= 0 && digit <= 9) ? MASKS[digit] : 0x00;

    const float w = 16.0f;
    const float h = 26.0f;
    const float t = 3.0f;
    const float halfH = (h - 3.0f * t) * 0.5f;

    const SDL_FRect segs[7] = {
        {x + t, y, w - 2.0f * t, t},
        {x + w - t, y + t, t, halfH},
        {x + w - t, y + 2.0f * t + halfH, t, halfH},
        {x + t, y + h - t, w - 2.0f * t, t},
        {x, y + 2.0f * t + halfH, t, halfH},
        {x, y + t, t, halfH},
        {x + t, y + t + halfH, w - 2.0f * t, t}
    };

    for (int s = 0; s < 7; ++s) {
        const bool on = (mask & (1 << s)) != 0;
        const SDL_Color& c = on ? litColor : unlitColor;
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRectF(renderer, &segs[s]);
    }
}

void Engine::renderScore(SDL_Renderer* renderer, float centerX, float y, uint64_t score) {
    char buffer[8];
    uint64_t temp = score % 10000000ULL;
    for (int i = 6; i >= 0; --i) {
        buffer[i] = static_cast<char>('0' + (temp % 10));
        temp /= 10;
    }

    const float digitW = 16.0f;
    const float digitH = 26.0f;
    const float digitGap = 6.0f;
    const float totalW = 7.0f * digitW + 6.0f * digitGap;
    const float startX = centerX - totalW * 0.5f;

    const SDL_FRect bgRect{startX - 10.0f, y - 6.0f, totalW + 20.0f, digitH + 18.0f};
    SDL_SetRenderDrawColor(renderer, 20, 25, 30, 235);
    SDL_RenderFillRectF(renderer, &bgRect);

    const SDL_FRect borderTop{startX - 10.0f, y - 6.0f, totalW + 20.0f, 2.0f};
    const SDL_FRect borderBot{startX - 10.0f, y + digitH + 10.0f, totalW + 20.0f, 2.0f};
    SDL_SetRenderDrawColor(renderer, 70, 85, 100, 255);
    SDL_RenderFillRectF(renderer, &borderTop);
    SDL_RenderFillRectF(renderer, &borderBot);

    const bool is1Up = (m_oneUpFlashTimer > 0.0f);
    const SDL_Color litColor = is1Up ? SDL_Color{80, 255, 130, 255} : SDL_Color{255, 225, 45, 255};
    const SDL_Color unlitColor{40, 35, 20, 110};

    for (int i = 0; i < 7; ++i) {
        const int d = buffer[i] - '0';
        render7SegmentDigit(renderer, startX + static_cast<float>(i) * (digitW + digitGap), y, d, litColor, unlitColor);
    }

    const float progressRatio = static_cast<float>(score % 100000ULL) / 100000.0f;
    const SDL_FRect progBg{startX, y + digitH + 4.0f, totalW, 3.0f};
    SDL_SetRenderDrawColor(renderer, 35, 40, 45, 255);
    SDL_RenderFillRectF(renderer, &progBg);

    const SDL_FRect progFill{startX, y + digitH + 4.0f, totalW * progressRatio, 3.0f};
    SDL_SetRenderDrawColor(renderer, is1Up ? 80 : 0, is1Up ? 255 : 210, is1Up ? 130 : 255, 255);
    SDL_RenderFillRectF(renderer, &progFill);
}

void Engine::renderHUD() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // =========================================================================
    // 1. BARRA DE SALUD / ESCUDO DEL TANQUE
    // =========================================================================
    const float currentShield = m_tank.getShield();
    const float maxShield = m_tank.getMaxShield();
    const float shieldRatio = std::clamp(currentShield / maxShield, 0.0f, 1.0f);

    const SDL_FRect frameRect{50.0f, 32.0f, 310.0f, 26.0f};
    SDL_SetRenderDrawColor(m_renderer, 30, 35, 40, 240);
    SDL_RenderFillRectF(m_renderer, &frameRect);

    const SDL_FRect borderTop{50.0f, 32.0f, 310.0f, 2.0f};
    const SDL_FRect borderBot{50.0f, 56.0f, 310.0f, 2.0f};
    const SDL_FRect borderLeft{50.0f, 32.0f, 2.0f, 26.0f};
    const SDL_FRect borderRight{358.0f, 32.0f, 2.0f, 26.0f};
    SDL_SetRenderDrawColor(m_renderer, 80, 95, 110, 255);
    SDL_RenderFillRectF(m_renderer, &borderTop);
    SDL_RenderFillRectF(m_renderer, &borderBot);
    SDL_RenderFillRectF(m_renderer, &borderLeft);
    SDL_RenderFillRectF(m_renderer, &borderRight);

    const SDL_FRect emptyBar{54.0f, 36.0f, 302.0f, 18.0f};
    SDL_SetRenderDrawColor(m_renderer, 60, 20, 20, 255);
    SDL_RenderFillRectF(m_renderer, &emptyBar);

    if (shieldRatio > 0.001f) {
        const SDL_FRect fillBar{54.0f, 36.0f, 302.0f * shieldRatio, 18.0f};
        Uint8 r = 0; Uint8 g = 220; Uint8 b = 100;
        if (shieldRatio <= 0.25f) {
            r = 240; g = 40; b = 40;
        } else if (shieldRatio <= 0.50f) {
            r = 250; g = 190; b = 30;
        }
        SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
        SDL_RenderFillRectF(m_renderer, &fillBar);
    }

    SDL_SetRenderDrawColor(m_renderer, 20, 25, 30, 200);
    for (int i = 1; i < 5; ++i) {
        const SDL_FRect tick{54.0f + static_cast<float>(i) * (302.0f / 5.0f), 36.0f, 2.0f, 18.0f};
        SDL_RenderFillRectF(m_renderer, &tick);
    }

    // =========================================================================
    // 1B. BARRA DE SOBRECARGA (OVERDRIVE)
    // =========================================================================
    const SDL_FRect odFrame{50.0f, 60.0f, 310.0f, 8.0f};
    SDL_SetRenderDrawColor(m_renderer, 25, 30, 36, 240);
    SDL_RenderFillRectF(m_renderer, &odFrame);

    const SDL_FRect odBorder{50.0f, 60.0f, 310.0f, 1.0f};
    SDL_SetRenderDrawColor(m_renderer, 70, 85, 100, 255);
    SDL_RenderFillRectF(m_renderer, &odBorder);

    const bool isOverdrive = m_tank.isOverdriveActive();
    const float odRatio = isOverdrive
        ? std::clamp(m_tank.getOverdriveDuration() / 6.0f, 0.0f, 1.0f)
        : std::clamp(m_tank.getOverdriveMeter() / 100.0f, 0.0f, 1.0f);

    if (odRatio > 0.001f) {
        const SDL_FRect odFill{52.0f, 61.0f, 306.0f * odRatio, 6.0f};
        if (isOverdrive) {
            static uint32_t flashCounter = 0;
            flashCounter++;
            if ((flashCounter / 6) % 2 == 0) {
                SDL_SetRenderDrawColor(m_renderer, 255, 220, 30, 255);
            } else {
                SDL_SetRenderDrawColor(m_renderer, 0, 240, 255, 255);
            }
        } else {
            SDL_SetRenderDrawColor(m_renderer, 0, 190, 220, 220);
        }
        SDL_RenderFillRectF(m_renderer, &odFill);
    }

    // =========================================================================
    // 2. CONTADOR DE BOMBAS NUCLEARES (NUKES)
    // =========================================================================
    const SDL_FRect nukeBox{50.0f, 72.0f, 140.0f, 30.0f};
    SDL_SetRenderDrawColor(m_renderer, 25, 30, 35, 230);
    SDL_RenderFillRectF(m_renderer, &nukeBox);

    const SDL_FRect nukeBorder{50.0f, 72.0f, 140.0f, 2.0f};
    SDL_SetRenderDrawColor(m_renderer, 70, 85, 95, 255);
    SDL_RenderFillRectF(m_renderer, &nukeBorder);

    for (int i = 0; i < 3; ++i) {
        const float iconX = 62.0f + static_cast<float>(i) * 44.0f;
        const float iconY = 77.0f;

        if (i < m_nukesAvailable) {
            const SDL_FRect wBody{iconX + 2.0f, iconY + 4.0f, 14.0f, 14.0f};
            SDL_SetRenderDrawColor(m_renderer, 255, 215, 35, 255);
            SDL_RenderFillRectF(m_renderer, &wBody);

            const SDL_FRect wTip{iconX + 4.0f, iconY, 10.0f, 4.0f};
            SDL_SetRenderDrawColor(m_renderer, 255, 235, 80, 255);
            SDL_RenderFillRectF(m_renderer, &wTip);

            const SDL_FRect wFins{iconX, iconY + 14.0f, 18.0f, 4.0f};
            SDL_SetRenderDrawColor(m_renderer, 220, 45, 45, 255);
            SDL_RenderFillRectF(m_renderer, &wFins);

            const SDL_FRect wCore{iconX + 6.0f, iconY + 7.0f, 6.0f, 6.0f};
            SDL_SetRenderDrawColor(m_renderer, 30, 30, 35, 255);
            SDL_RenderFillRectF(m_renderer, &wCore);
        } else {
            const SDL_FRect wEmpty{iconX + 2.0f, iconY + 4.0f, 14.0f, 14.0f};
            SDL_SetRenderDrawColor(m_renderer, 55, 60, 65, 140);
            SDL_RenderFillRectF(m_renderer, &wEmpty);

            const SDL_FRect wEmptyFins{iconX, iconY + 14.0f, 18.0f, 4.0f};
            SDL_SetRenderDrawColor(m_renderer, 45, 50, 55, 140);
            SDL_RenderFillRectF(m_renderer, &wEmptyFins);
        }
    }

    // =========================================================================
    // 3. ICONOS DE VIDAS RESTANTES (MINI TANQUES)
    // =========================================================================
    const SDL_FRect livesBox{50.0f, 108.0f, 140.0f, 26.0f};
    SDL_SetRenderDrawColor(m_renderer, 25, 30, 35, 230);
    SDL_RenderFillRectF(m_renderer, &livesBox);

    const SDL_FRect livesBorder{50.0f, 108.0f, 140.0f, 2.0f};
    SDL_SetRenderDrawColor(m_renderer, 70, 85, 95, 255);
    SDL_RenderFillRectF(m_renderer, &livesBorder);

    for (int i = 0; i < std::min(m_lives, 4); ++i) {
        const float tx = 58.0f + static_cast<float>(i) * 32.0f;
        const float ty = 114.0f;

        const SDL_FRect tTracks{tx, ty + 8.0f, 24.0f, 6.0f};
        SDL_SetRenderDrawColor(m_renderer, 20, 25, 30, 255);
        SDL_RenderFillRectF(m_renderer, &tTracks);

        const SDL_FRect tChassis{tx + 2.0f, ty + 3.0f, 20.0f, 6.0f};
        SDL_SetRenderDrawColor(m_renderer, 0, 210, 100, 255);
        SDL_RenderFillRectF(m_renderer, &tChassis);

        const SDL_FRect tTurret{tx + 7.0f, ty, 10.0f, 4.0f};
        SDL_SetRenderDrawColor(m_renderer, 0, 240, 120, 255);
        SDL_RenderFillRectF(m_renderer, &tTurret);

        const SDL_FRect tBarrel{tx + 14.0f, ty - 1.0f, 8.0f, 2.0f};
        SDL_SetRenderDrawColor(m_renderer, 80, 255, 160, 255);
        SDL_RenderFillRectF(m_renderer, &tBarrel);
    }

    // =========================================================================
    // 4. INDICADORES DE MEJORAS TÁCTICAS ACTIVAS
    // =========================================================================
    float badgeX = 50.0f;
    if (m_tank.hasSpreadShot()) {
        const SDL_FRect badge{badgeX, 140.0f, 68.0f, 18.0f};
        SDL_SetRenderDrawColor(m_renderer, 0, 195, 255, 230);
        SDL_RenderFillRectF(m_renderer, &badge);
        badgeX += 74.0f;
    }
    if (m_tank.hasRapidFire()) {
        const SDL_FRect badge{badgeX, 140.0f, 64.0f, 18.0f};
        SDL_SetRenderDrawColor(m_renderer, 255, 140, 30, 230);
        SDL_RenderFillRectF(m_renderer, &badge);
        badgeX += 70.0f;
    }
    if (m_tank.hasDamageBoost()) {
        const SDL_FRect badge{badgeX, 140.0f, 74.0f, 18.0f};
        SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 240); // Dorado uranio
        SDL_RenderFillRectF(m_renderer, &badge);
        badgeX += 80.0f;
    }
    if (m_tank.isShieldOvercharged()) {
        const SDL_FRect badge{badgeX, 140.0f, 74.0f, 18.0f};
        SDL_SetRenderDrawColor(m_renderer, 140, 255, 240, 240);
        SDL_RenderFillRectF(m_renderer, &badge);
        badgeX += 80.0f;
    }
    if (m_tank.isMegalaserActive()) {
        const SDL_FRect badge{badgeX, 140.0f, 96.0f, 18.0f};
        SDL_SetRenderDrawColor(m_renderer, 230, 40, 255, 240); // Megalaser activo
        SDL_RenderFillRectF(m_renderer, &badge);
        badgeX += 102.0f;
    } else if (m_tank.getMegalaserFragments() > 0) {
        // Indicador de fragmentos de megaláser (1/4 a 3/4)
        const SDL_FRect badge{badgeX, 140.0f, 60.0f, 18.0f};
        SDL_SetRenderDrawColor(m_renderer, 150, 40, 190, 200);
        SDL_RenderFillRectF(m_renderer, &badge);
        badgeX += 66.0f;
    }

    // =========================================================================
    // 5. MARCADOR DE PUNTOS SUPERIOR CENTRAL (7 DÍGITOS + PROGRESO 100K)
    // =========================================================================
    renderScore(m_renderer, 960.0f, 32.0f, m_score);

    // =========================================================================
    // 6. BARRA DE PROGRESO DE MISIÓN (95s+) O BARRA DE VIDA DEL BOSS / MODO SUPERVIVENCIA
    // =========================================================================
    if (m_boss.isActive()) {
        m_boss.renderHealthBar(m_renderer);
    } else if (m_isSurvivalMode) {
        // Cronómetro prominente de supervivencia en la parte superior central
        const SDL_FRect sPanel{780.0f, 76.0f, 360.0f, 48.0f};
        SDL_SetRenderDrawColor(m_renderer, 15, 22, 30, 235);
        SDL_RenderFillRectF(m_renderer, &sPanel);

        const SDL_FRect sBorderT{780.0f, 76.0f, 360.0f, 2.0f};
        const SDL_FRect sBorderB{780.0f, 122.0f, 360.0f, 2.0f};
        SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 240);
        SDL_RenderFillRectF(m_renderer, &sBorderT);
        SDL_RenderFillRectF(m_renderer, &sBorderB);

        const int survSecs = static_cast<int>(m_survivalTimer);
        const int sMin = survSecs / 60;
        const int sSec = survSecs % 60;
        char survBuf[32];
        snprintf(survBuf, sizeof(survBuf), "SUPERVIVENCIA: %02d:%02d", sMin, sSec);
        renderArcadeText(m_renderer, survBuf, 960.0f, 85.0f, 2.4f, SDL_Color{255, 225, 60, 255}, true);

        const int bestSecs = static_cast<int>(m_bestSurvivalSeconds);
        const int bMin = bestSecs / 60;
        const int bSec = bestSecs % 60;
        char bestBuf[32];
        snprintf(bestBuf, sizeof(bestBuf), "RECORD: %02d:%02d", bMin, bSec);
        renderArcadeText(m_renderer, bestBuf, 960.0f, 108.0f, 1.6f, SDL_Color{0, 225, 245, 220}, true);
    } else {
        m_levelManager.renderMissionBar(m_renderer);
    }

    // =========================================================================
    // 7. BANNER DE NIVEL COMPLETADO (TRANSICIÓN DE 3.0 SEGUNDOS)
    // =========================================================================
    if (!m_isSurvivalMode) {
        m_levelManager.renderLevelCompleteBanner(m_renderer);
    }

    // =========================================================================
    // 8. INDICADOR TÁCTICO DE NIVEL ACTUAL (ESQUINA SUPERIOR DERECHA)
    // =========================================================================
    const int currentLvl = m_isSurvivalMode
        ? std::clamp(1 + static_cast<int>(m_survivalTimer / 30.0f), 1, 25)
        : m_levelManager.getCurrentLevel();
    const float badgeW = 190.0f;
    const float badgeH = 46.0f;
    const float badgeStartX = 1920.0f - 50.0f - badgeW; // 1680.0f
    const float badgeStartY = 32.0f;

    // Fondo del marco táctico militar
    const SDL_FRect levelFrame{badgeStartX, badgeStartY, badgeW, badgeH};
    SDL_SetRenderDrawColor(m_renderer, 22, 28, 35, 240);
    SDL_RenderFillRectF(m_renderer, &levelFrame);

    // Bordes dobles con acento cian militar
    const SDL_FRect bTop{badgeStartX, badgeStartY, badgeW, 2.0f};
    const SDL_FRect bBot{badgeStartX, badgeStartY + badgeH - 2.0f, badgeW, 2.0f};
    const SDL_FRect bLeft{badgeStartX, badgeStartY, 2.0f, badgeH};
    const SDL_FRect bRight{badgeStartX + badgeW - 2.0f, badgeStartY, 2.0f, badgeH};
    SDL_SetRenderDrawColor(m_renderer, 0, 210, 230, 240);
    SDL_RenderFillRectF(m_renderer, &bTop);
    SDL_RenderFillRectF(m_renderer, &bBot);
    SDL_RenderFillRectF(m_renderer, &bLeft);
    SDL_RenderFillRectF(m_renderer, &bRight);

    // Letras "LVL" geométricas
    const float txtX = badgeStartX + 14.0f;
    const float txtY = badgeStartY + 14.0f;
    // 'L'
    const SDL_FRect l1{txtX, txtY, 3.0f, 16.0f};
    const SDL_FRect l2{txtX, txtY + 13.0f, 10.0f, 3.0f};
    // 'V'
    const SDL_FRect v1{txtX + 13.0f, txtY, 3.0f, 12.0f};
    const SDL_FRect v2{txtX + 21.0f, txtY, 3.0f, 12.0f};
    const SDL_FRect v3{txtX + 16.0f, txtY + 11.0f, 5.0f, 5.0f};
    // 'L'
    const SDL_FRect l3{txtX + 27.0f, txtY, 3.0f, 16.0f};
    const SDL_FRect l4{txtX + 27.0f, txtY + 13.0f, 10.0f, 3.0f};

    SDL_SetRenderDrawColor(m_renderer, 160, 205, 225, 255);
    SDL_RenderFillRectF(m_renderer, &l1);
    SDL_RenderFillRectF(m_renderer, &l2);
    SDL_RenderFillRectF(m_renderer, &v1);
    SDL_RenderFillRectF(m_renderer, &v2);
    SDL_RenderFillRectF(m_renderer, &v3);
    SDL_RenderFillRectF(m_renderer, &l3);
    SDL_RenderFillRectF(m_renderer, &l4);

    // Separador vertical
    const SDL_FRect vDiv{txtX + 44.0f, badgeStartY + 8.0f, 2.0f, 30.0f};
    SDL_SetRenderDrawColor(m_renderer, 50, 75, 95, 255);
    SDL_RenderFillRectF(m_renderer, &vDiv);

    // Dos dígitos 7-segmentos para el nivel
    const SDL_Color digitLit{0, 240, 255, 255};
    const SDL_Color digitUnlit{15, 45, 55, 120};
    const int tenDigit = currentLvl / 10;
    const int unitDigit = currentLvl % 10;

    render7SegmentDigit(m_renderer, txtX + 54.0f, badgeStartY + 10.0f, tenDigit, digitLit, digitUnlit);
    render7SegmentDigit(m_renderer, txtX + 76.0f, badgeStartY + 10.0f, unitDigit, digitLit, digitUnlit);

    // Franjas de color temáticas del sector actual
    const auto& palette = m_levelManager.getCurrentPalette();
    const SDL_FRect palStripe{txtX + 102.0f, badgeStartY + 11.0f, 22.0f, 10.0f};
    SDL_SetRenderDrawColor(m_renderer, palette.groundSurface.r, palette.groundSurface.g, palette.groundSurface.b, 255);
    SDL_RenderFillRectF(m_renderer, &palStripe);

    const SDL_FRect palStripe2{txtX + 102.0f, badgeStartY + 25.0f, 22.0f, 10.0f};
    SDL_SetRenderDrawColor(m_renderer, palette.skyTop.r, palette.skyTop.g, palette.skyTop.b, 255);
    SDL_RenderFillRectF(m_renderer, &palStripe2);
}

void Engine::renderLevelSummaryScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // 1. Fondo oscurecido táctico
    const SDL_FRect darkOverlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 10, 16, 22, 235);
    SDL_RenderFillRectF(m_renderer, &darkOverlay);

    // 2. Panel central militar
    const SDL_FRect panel{420.0f, 130.0f, 1080.0f, 790.0f};
    SDL_SetRenderDrawColor(m_renderer, 18, 26, 36, 250);
    SDL_RenderFillRectF(m_renderer, &panel);

    // Bordes del panel
    const SDL_FRect pTop{420.0f, 130.0f, 1080.0f, 4.0f};
    const SDL_FRect pBot{420.0f, 916.0f, 1080.0f, 4.0f};
    const SDL_FRect pLeft{420.0f, 130.0f, 4.0f, 790.0f};
    const SDL_FRect pRight{1496.0f, 130.0f, 4.0f, 790.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 220, 130, 255);
    SDL_RenderFillRectF(m_renderer, &pTop);
    SDL_RenderFillRectF(m_renderer, &pBot);
    SDL_RenderFillRectF(m_renderer, &pLeft);
    SDL_RenderFillRectF(m_renderer, &pRight);

    // Esquinas doradas de fijación táctica
    const SDL_FRect c1{424.0f, 134.0f, 24.0f, 24.0f};
    const SDL_FRect c2{1472.0f, 134.0f, 24.0f, 24.0f};
    const SDL_FRect c3{424.0f, 892.0f, 24.0f, 24.0f};
    const SDL_FRect c4{1472.0f, 892.0f, 24.0f, 24.0f};
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 30, 255);
    SDL_RenderFillRectF(m_renderer, &c1);
    SDL_RenderFillRectF(m_renderer, &c2);
    SDL_RenderFillRectF(m_renderer, &c3);
    SDL_RenderFillRectF(m_renderer, &c4);

    // Cabecera superior
    const SDL_FRect headerBg{450.0f, 160.0f, 1020.0f, 75.0f};
    SDL_SetRenderDrawColor(m_renderer, 25, 38, 52, 255);
    SDL_RenderFillRectF(m_renderer, &headerBg);

    char titleBuf[64];
    std::snprintf(titleBuf, sizeof(titleBuf), "SECTOR %d CLEARED - MISSION REPORT", m_levelManager.getCurrentLevel());
    renderArcadeText(m_renderer, titleBuf, 960.0f, 185.0f, 3.0f, SDL_Color{0, 255, 160, 255}, true);

    const auto& pal = m_levelManager.getCurrentPalette();
    renderArcadeText(m_renderer, pal.sectorName, 960.0f, 252.0f, 2.2f, SDL_Color{140, 190, 230, 255}, true);

    // Línea separadora
    const SDL_FRect sep1{460.0f, 285.0f, 1000.0f, 2.0f};
    SDL_SetRenderDrawColor(m_renderer, 50, 75, 95, 255);
    SDL_RenderFillRectF(m_renderer, &sep1);

    // 4 Filas de estadísticas
    struct StatRow {
        const char* label;
        char val[32];
        SDL_Color valColor;
    };

    StatRow stats[4];
    stats[0].label = "ENEMIES DESTROYED";
    std::snprintf(stats[0].val, sizeof(stats[0].val), "%d", m_levelEnemiesKilled);
    stats[0].valColor = SDL_Color{0, 230, 255, 255};

    stats[1].label = "PROJECTILES INTERCEPTED";
    std::snprintf(stats[1].val, sizeof(stats[1].val), "%d", m_levelBombsIntercepted);
    stats[1].valColor = SDL_Color{0, 230, 255, 255};

    stats[2].label = "TANK INTEGRITY BONUS";
    std::snprintf(stats[2].val, sizeof(stats[2].val), "+%d PTS", m_levelIntegrityBonus);
    stats[2].valColor = SDL_Color{60, 255, 120, 255};

    stats[3].label = "MISSION SCORE EARNED";
    std::snprintf(stats[3].val, sizeof(stats[3].val), "+%llu PTS", static_cast<unsigned long long>(m_levelScoreEarned));
    stats[3].valColor = SDL_Color{255, 220, 50, 255};

    for (int i = 0; i < 4; ++i) {
        const float ry = 310.0f + static_cast<float>(i) * 75.0f;
        const SDL_FRect rowBg{460.0f, ry - 6.0f, 1000.0f, 58.0f};
        SDL_SetRenderDrawColor(m_renderer, (i % 2 == 0) ? 24 : 20, (i % 2 == 0) ? 34 : 28, (i % 2 == 0) ? 46 : 38, 255);
        SDL_RenderFillRectF(m_renderer, &rowBg);

        // Barra decorativa izquierda
        const SDL_FRect rowStripe{460.0f, ry - 6.0f, 6.0f, 58.0f};
        SDL_SetRenderDrawColor(m_renderer, stats[i].valColor.r, stats[i].valColor.g, stats[i].valColor.b, 255);
        SDL_RenderFillRectF(m_renderer, &rowStripe);

        renderArcadeText(m_renderer, stats[i].label, 485.0f, ry + 12.0f, 2.3f, SDL_Color{220, 230, 240, 255}, false);
        renderArcadeText(m_renderer, stats[i].val, 1435.0f, ry + 10.0f, 2.6f, stats[i].valColor, true);
    }

    // Separador dorado
    const SDL_FRect goldSep{460.0f, 615.0f, 1000.0f, 3.0f};
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 30, 255);
    SDL_RenderFillRectF(m_renderer, &goldSep);

    // Caja destacada de Créditos Disponibles
    const SDL_FRect credBox{460.0f, 630.0f, 1000.0f, 68.0f};
    SDL_SetRenderDrawColor(m_renderer, 36, 44, 28, 255);
    SDL_RenderFillRectF(m_renderer, &credBox);

    const SDL_FRect credBorder{460.0f, 630.0f, 1000.0f, 2.0f};
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 30, 255);
    SDL_RenderFillRectF(m_renderer, &credBorder);

    renderArcadeText(m_renderer, "TOTAL AVAILABLE CREDITS", 485.0f, 650.0f, 2.5f, SDL_Color{255, 220, 40, 255}, false);

    char totalCredBuf[32];
    std::snprintf(totalCredBuf, sizeof(totalCredBuf), "%d PTS", m_spendablePoints);
    renderArcadeText(m_renderer, totalCredBuf, 1435.0f, 646.0f, 3.0f, SDL_Color{255, 235, 70, 255}, true);

    // Botón Táctico [PROCEED TO ARMORY >>]
    const SDL_FRect procBtn{660.0f, 735.0f, 600.0f, 72.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 160, 80, 255);
    SDL_RenderFillRectF(m_renderer, &procBtn);

    const SDL_FRect procBtnBorderT{660.0f, 735.0f, 600.0f, 3.0f};
    const SDL_FRect procBtnBorderB{660.0f, 804.0f, 600.0f, 3.0f};
    const SDL_FRect procBtnBorderL{660.0f, 735.0f, 3.0f, 72.0f};
    const SDL_FRect procBtnBorderR{1257.0f, 735.0f, 3.0f, 72.0f};
    SDL_SetRenderDrawColor(m_renderer, 60, 255, 140, 255);
    SDL_RenderFillRectF(m_renderer, &procBtnBorderT);
    SDL_RenderFillRectF(m_renderer, &procBtnBorderB);
    SDL_RenderFillRectF(m_renderer, &procBtnBorderL);
    SDL_RenderFillRectF(m_renderer, &procBtnBorderR);

    renderArcadeText(m_renderer, "[ PROCEED TO ARMORY >> ]", 960.0f, 758.0f, 2.7f, SDL_Color{255, 255, 255, 255}, true);
    renderArcadeText(m_renderer, "PRESS ENTER / GAMEPAD A OR CLICK TO CONTINUE", 960.0f, 825.0f, 1.8f, SDL_Color{140, 165, 185, 255}, true);
}

void Engine::renderArmoryScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // 1. Fondo oscurecido táctico
    const SDL_FRect darkOverlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 10, 16, 22, 245);
    SDL_RenderFillRectF(m_renderer, &darkOverlay);

    // 2. Cabecera superior táctica
    const SDL_FRect headerBg{80.0f, 25.0f, 1760.0f, 105.0f};
    SDL_SetRenderDrawColor(m_renderer, 20, 30, 42, 250);
    SDL_RenderFillRectF(m_renderer, &headerBg);

    const SDL_FRect hBorderT{80.0f, 25.0f, 1760.0f, 3.0f};
    const SDL_FRect hBorderB{80.0f, 127.0f, 1760.0f, 3.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 220, 130, 255);
    SDL_RenderFillRectF(m_renderer, &hBorderT);
    SDL_RenderFillRectF(m_renderer, &hBorderB);

    renderArcadeText(m_renderer, "TACTICAL ARMORY - 15 UPGRADE MATRIX", 960.0f, 44.0f, 3.2f, SDL_Color{0, 255, 160, 255}, true);

    char credBuf[64];
    std::snprintf(credBuf, sizeof(credBuf), "TOTAL AVAILABLE CREDITS: %d PTS", m_spendablePoints);
    renderArcadeText(m_renderer, credBuf, 960.0f, 88.0f, 2.5f, SDL_Color{255, 220, 40, 255}, true);

    // 3. Definición estática de las 15 Mejoras
    struct ArmoryItemDef {
        int index;
        const char* name;
        const char* desc;
        int cost;
        int row;
        int col;
    };

    static constexpr ArmoryItemDef ITEMS[15] = {
        // Fila 1 - Munición
        {0, "AP ROUNDS I", "+3 BULLET DAMAGE", 3500, 0, 0},
        {1, "TUNGSTEN CORE II", "+3 DAMAGE (+6 CUMULATIVE)", 8000, 0, 1},
        {2, "URANIUM SHELLS III", "+4 DAMAGE (+10 CUMULATIVE)", 16000, 0, 2},

        // Fila 2 - Cañón
        {3, "BARREL COOLER I", "+18% FIRE RATE", 4000, 1, 0},
        {4, "TWIN CANNON II", "DUAL PARALLEL CANNONS", 12000, 1, 1},
        {5, "GATLING OVERDRIVE III", "+25% EXTRA FIRE RATE", 22000, 1, 2},

        // Fila 3 - Chasis y Blindaje
        {6, "TITANIUM PLATING I", "+25 MAX HP", 3000, 2, 0},
        {7, "TURBO TREADS II", "+25% TANK SPEED", 7500, 2, 1},
        {8, "REACTIVE ARMOR III", "+50 HP & -15% DAMAGE TAKEN", 15000, 2, 2},

        // Fila 4 - Armas Secundarias
        {9, "HOMING POD I", "1 HOMING MISSILE / 2.4S", 6000, 3, 0},
        {10, "DUAL HOMING II", "2 HOMING MISSILES / 1.7S", 14000, 3, 1},
        {11, "POINT-DEFENSE LASER III", "ZAPS CLOSEST BOMB / 2.0S", 25000, 3, 2},

        // Fila 5 - Suministros Tácticos & Vidas (Consumibles)
        {12, "FULL HULL REPAIR", "RESTORES 100% TANK HP", 2500, 4, 0},
        {13, "TACTICAL NUKE +1", "ADDS +1 EMERGENCY NUKE", 10000, 4, 1},
        {14, "EXTRA LIFE (1-UP)", "ADDS +1 PERMANENT LIFE", 45000, 4, 2}
    };

    enum class Status { Locked, Available, Bought };

    auto getItemStatus = [&](int idx) -> Status {
        switch (idx) {
        case 0: return (m_tank.getAmmoTier() >= 1) ? Status::Bought : Status::Available;
        case 1:
            if (m_tank.getAmmoTier() >= 2) return Status::Bought;
            return (m_tank.getAmmoTier() == 1) ? Status::Available : Status::Locked;
        case 2:
            if (m_tank.getAmmoTier() >= 3) return Status::Bought;
            return (m_tank.getAmmoTier() == 2) ? Status::Available : Status::Locked;

        case 3: return (m_tank.getCannonTier() >= 1) ? Status::Bought : Status::Available;
        case 4:
            if (m_tank.getCannonTier() >= 2) return Status::Bought;
            return (m_tank.getCannonTier() == 1) ? Status::Available : Status::Locked;
        case 5:
            if (m_tank.getCannonTier() >= 3) return Status::Bought;
            return (m_tank.getCannonTier() == 2) ? Status::Available : Status::Locked;

        case 6: return (m_tank.getChassisTier() >= 1) ? Status::Bought : Status::Available;
        case 7:
            if (m_tank.getChassisTier() >= 2) return Status::Bought;
            return (m_tank.getChassisTier() == 1) ? Status::Available : Status::Locked;
        case 8:
            if (m_tank.getChassisTier() >= 3) return Status::Bought;
            return (m_tank.getChassisTier() == 2) ? Status::Available : Status::Locked;

        case 9: return (m_tank.getSecondaryTier() >= 1) ? Status::Bought : Status::Available;
        case 10:
            if (m_tank.getSecondaryTier() >= 2) return Status::Bought;
            return (m_tank.getSecondaryTier() == 1) ? Status::Available : Status::Locked;
        case 11:
            if (m_tank.getSecondaryTier() >= 3) return Status::Bought;
            return (m_tank.getSecondaryTier() == 2) ? Status::Available : Status::Locked;

        case 12: return Status::Available;
        case 13: return Status::Available;
        case 14: return Status::Available;
        default: return Status::Locked;
        }
    };

    // 4. Renderizado de las 15 Tarjetas (5 filas x 3 columnas)
    for (int i = 0; i < 15; ++i) {
        const auto& it = ITEMS[i];
        const float cx = 80.0f + static_cast<float>(it.col) * 600.0f;
        const float cy = 155.0f + static_cast<float>(it.row) * 140.0f;
        constexpr float cw = 560.0f;
        constexpr float ch = 125.0f;

        const Status st = getItemStatus(i);
        const bool isSelected = (m_armorySelectedIndex == i);

        // Fondo de tarjeta
        const SDL_FRect cBase{cx, cy, cw, ch};
        if (isSelected) {
            SDL_SetRenderDrawColor(m_renderer, 32, 44, 58, 255);
        } else if (st == Status::Bought) {
            SDL_SetRenderDrawColor(m_renderer, 18, 32, 26, 245);
        } else if (st == Status::Locked) {
            SDL_SetRenderDrawColor(m_renderer, 16, 20, 26, 240);
        } else {
            SDL_SetRenderDrawColor(m_renderer, 20, 28, 38, 245);
        }
        SDL_RenderFillRectF(m_renderer, &cBase);

        // Bordes de la tarjeta
        const SDL_FRect cT{cx, cy, cw, isSelected ? 3.0f : 2.0f};
        const SDL_FRect cB{cx, cy + ch - (isSelected ? 3.0f : 2.0f), cw, isSelected ? 3.0f : 2.0f};
        const SDL_FRect cL{cx, cy, isSelected ? 3.0f : 2.0f, ch};
        const SDL_FRect cR{cx + cw - (isSelected ? 3.0f : 2.0f), cy, isSelected ? 3.0f : 2.0f, ch};

        if (isSelected) {
            SDL_SetRenderDrawColor(m_renderer, 255, 220, 50, 255); // Resaltado dorado
        } else if (st == Status::Bought) {
            SDL_SetRenderDrawColor(m_renderer, 0, 200, 100, 255);
        } else if (st == Status::Locked) {
            SDL_SetRenderDrawColor(m_renderer, 55, 65, 75, 255);
        } else {
            SDL_SetRenderDrawColor(m_renderer, 0, 160, 220, 255);
        }
        SDL_RenderFillRectF(m_renderer, &cT);
        SDL_RenderFillRectF(m_renderer, &cB);
        SDL_RenderFillRectF(m_renderer, &cL);
        SDL_RenderFillRectF(m_renderer, &cR);

        // Icono temático del slot (izquierda)
        const SDL_FRect iconBox{cx + 12.0f, cy + 18.0f, 46.0f, 46.0f};
        SDL_SetRenderDrawColor(m_renderer, 28, 36, 48, 255);
        SDL_RenderFillRectF(m_renderer, &iconBox);

        if (it.row == 0) { // Munición
            const SDL_FRect b1{cx + 24.0f, cy + 24.0f, 8.0f, 32.0f};
            const SDL_FRect b2{cx + 36.0f, cy + 24.0f, 8.0f, 32.0f};
            SDL_SetRenderDrawColor(m_renderer, 255, 210, 40, 255);
            SDL_RenderFillRectF(m_renderer, &b1);
            SDL_RenderFillRectF(m_renderer, &b2);
        } else if (it.row == 1) { // Cañones
            const SDL_FRect bar1{cx + 22.0f, cy + 28.0f, 10.0f, 28.0f};
            const SDL_FRect bar2{cx + 38.0f, cy + 28.0f, 10.0f, 28.0f};
            SDL_SetRenderDrawColor(m_renderer, 0, 210, 255, 255);
            SDL_RenderFillRectF(m_renderer, &bar1);
            SDL_RenderFillRectF(m_renderer, &bar2);
        } else if (it.row == 2) { // Chasis/Blindaje
            const SDL_FRect shld{cx + 20.0f, cy + 24.0f, 30.0f, 32.0f};
            SDL_SetRenderDrawColor(m_renderer, 0, 220, 110, 255);
            SDL_RenderFillRectF(m_renderer, &shld);
        } else if (it.row == 3) { // Secundarias
            const SDL_FRect msl{cx + 30.0f, cy + 22.0f, 10.0f, 36.0f};
            SDL_SetRenderDrawColor(m_renderer, 255, 90, 60, 255);
            SDL_RenderFillRectF(m_renderer, &msl);
        } else { // Suministros
            const SDL_FRect crossH{cx + 20.0f, cy + 36.0f, 30.0f, 10.0f};
            const SDL_FRect crossV{cx + 30.0f, cy + 26.0f, 10.0f, 30.0f};
            SDL_SetRenderDrawColor(m_renderer, 255, 215, 30, 255);
            SDL_RenderFillRectF(m_renderer, &crossH);
            SDL_RenderFillRectF(m_renderer, &crossV);
        }

        // Título del Upgrade
        SDL_Color nameCol = SDL_Color{240, 250, 255, 255};
        if (st == Status::Bought) nameCol = SDL_Color{140, 255, 180, 255};
        else if (st == Status::Locked) nameCol = SDL_Color{120, 130, 140, 255};
        renderArcadeText(m_renderer, it.name, cx + 68.0f, cy + 18.0f, 2.0f, nameCol, false);

        // Descripción técnica
        SDL_Color descCol = (st == Status::Locked) ? SDL_Color{90, 100, 110, 255} : SDL_Color{145, 175, 205, 255};
        renderArcadeText(m_renderer, it.desc, cx + 68.0f, cy + 44.0f, 1.4f, descCol, false);

        // Badge de estado y precio a la derecha
        constexpr float badgeW = 160.0f;
        constexpr float badgeH = 34.0f;
        const float badgeX = cx + cw - badgeW - 14.0f;
        const float badgeY = cy + ch - badgeH - 12.0f;
        const SDL_FRect badgeRect{badgeX, badgeY, badgeW, badgeH};

        if (st == Status::Bought) {
            SDL_SetRenderDrawColor(m_renderer, 0, 130, 65, 255);
            SDL_RenderFillRectF(m_renderer, &badgeRect);
            renderArcadeText(m_renderer, "[ COMPRADO ]", badgeX + badgeW * 0.5f, badgeY + 10.0f, 1.7f, SDL_Color{220, 255, 230, 255}, true);
        } else if (st == Status::Locked) {
            SDL_SetRenderDrawColor(m_renderer, 35, 42, 50, 255);
            SDL_RenderFillRectF(m_renderer, &badgeRect);
            renderArcadeText(m_renderer, "[ BLOQUEADO ]", badgeX + badgeW * 0.5f, badgeY + 10.0f, 1.7f, SDL_Color{120, 130, 140, 255}, true);
        } else {
            const bool canAfford = (m_spendablePoints >= it.cost);
            if (canAfford) {
                SDL_SetRenderDrawColor(m_renderer, 160, 120, 20, 255);
            } else {
                SDL_SetRenderDrawColor(m_renderer, 90, 30, 30, 255);
            }
            SDL_RenderFillRectF(m_renderer, &badgeRect);

            char costStr[32];
            std::snprintf(costStr, sizeof(costStr), "[ %d PTS ]", it.cost);
            renderArcadeText(m_renderer, costStr, badgeX + badgeW * 0.5f, badgeY + 10.0f, 1.7f,
                             canAfford ? SDL_Color{255, 230, 60, 255} : SDL_Color{255, 140, 140, 255}, true);
        }
    }

    // 5. Botón Inferior Táctico: [DEPLOY TO NEXT MISSION >>]
    const bool isDeploySelected = (m_armorySelectedIndex == 15);
    const SDL_FRect deployBtn{560.0f, 865.0f, 800.0f, 68.0f};
    if (isDeploySelected) {
        SDL_SetRenderDrawColor(m_renderer, 20, 170, 90, 255);
    } else {
        SDL_SetRenderDrawColor(m_renderer, 12, 120, 65, 255);
    }
    SDL_RenderFillRectF(m_renderer, &deployBtn);

    const SDL_FRect depBorderT{560.0f, 865.0f, 800.0f, isDeploySelected ? 4.0f : 2.0f};
    const SDL_FRect depBorderB{560.0f, 929.0f, 800.0f, isDeploySelected ? 4.0f : 2.0f};
    const SDL_FRect depBorderL{560.0f, 865.0f, isDeploySelected ? 4.0f : 2.0f, 68.0f};
    const SDL_FRect depBorderR{1356.0f, 865.0f, isDeploySelected ? 4.0f : 2.0f, 68.0f};
    if (isDeploySelected) {
        SDL_SetRenderDrawColor(m_renderer, 255, 230, 50, 255);
    } else {
        SDL_SetRenderDrawColor(m_renderer, 60, 255, 140, 255);
    }
    SDL_RenderFillRectF(m_renderer, &depBorderT);
    SDL_RenderFillRectF(m_renderer, &depBorderB);
    SDL_RenderFillRectF(m_renderer, &depBorderL);
    SDL_RenderFillRectF(m_renderer, &depBorderR);

    renderArcadeText(m_renderer, "[ DEPLOY TO NEXT MISSION >> ]", 960.0f, 887.0f, 2.8f,
                     isDeploySelected ? SDL_Color{255, 255, 255, 255} : SDL_Color{230, 250, 235, 255}, true);

    // Barra de Estado Actual del Tanque
    char statusBuf[128];
    std::snprintf(statusBuf, sizeof(statusBuf),
                  "TANK HP: %d/%d  |  AMMO: T%d  |  CANNONS: T%d  |  CHASSIS: T%d  |  NUKES: %d  |  LIVES: %d",
                  static_cast<int>(m_tank.getShield()), static_cast<int>(m_tank.getMaxShield()),
                  m_tank.getAmmoTier(), m_tank.getCannonTier(), m_tank.getChassisTier(),
                  m_nukesAvailable, m_lives);
    renderArcadeText(m_renderer, statusBuf, 960.0f, 955.0f, 1.8f, SDL_Color{160, 190, 220, 255}, true);
    renderArcadeText(m_renderer, "ARROWS / WASD: NAVIGATE  |  ENTER / GAMEPAD A: BUY / DEPLOY  |  CLICK CARD TO BUY",
                     960.0f, 985.0f, 1.6f, SDL_Color{120, 140, 160, 255}, true);
}

void Engine::renderGameOverScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    const SDL_FRect darkOverlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 10, 15, 20, 220);
    SDL_RenderFillRectF(m_renderer, &darkOverlay);

    const SDL_FRect panel{560.0f, 200.0f, 800.0f, 500.0f};
    SDL_SetRenderDrawColor(m_renderer, 25, 30, 35, 250);
    SDL_RenderFillRectF(m_renderer, &panel);

    const SDL_FRect pBorderTop{560.0f, 200.0f, 800.0f, 4.0f};
    const SDL_FRect pBorderBot{560.0f, 696.0f, 800.0f, 4.0f};
    const SDL_FRect pBorderLeft{560.0f, 200.0f, 4.0f, 500.0f};
    const SDL_FRect pBorderRight{1356.0f, 200.0f, 4.0f, 500.0f};
    SDL_SetRenderDrawColor(m_renderer, 180, 50, 50, 255);
    SDL_RenderFillRectF(m_renderer, &pBorderTop);
    SDL_RenderFillRectF(m_renderer, &pBorderBot);
    SDL_RenderFillRectF(m_renderer, &pBorderLeft);
    SDL_RenderFillRectF(m_renderer, &pBorderRight);

    const SDL_FRect bannerBg{600.0f, 240.0f, 720.0f, 60.0f};
    SDL_SetRenderDrawColor(m_renderer, 160, 30, 30, 255);
    SDL_RenderFillRectF(m_renderer, &bannerBg);

    const SDL_FRect bannerStripe{600.0f, 296.0f, 720.0f, 4.0f};
    SDL_SetRenderDrawColor(m_renderer, 255, 220, 50, 255);
    SDL_RenderFillRectF(m_renderer, &bannerStripe);

    if (m_isSurvivalMode) {
        renderArcadeText(m_renderer, "MODO SUPERVIVENCIA FINALIZADO", 960.0f, 258.0f, 2.6f, SDL_Color{255, 255, 255, 255}, true);

        const int sSecs = static_cast<int>(m_survivalTimer);
        char sBuf[64];
        std::snprintf(sBuf, sizeof(sBuf), "TIEMPO RESISTIDO: %02d:%02d", sSecs / 60, sSecs % 60);
        renderArcadeText(m_renderer, sBuf, 960.0f, 430.0f, 2.4f, SDL_Color{255, 220, 50, 255}, true);

        const int bSecs = static_cast<int>(m_bestSurvivalSeconds);
        char bBuf[64];
        std::snprintf(bBuf, sizeof(bBuf), "MEJOR RECORD: %02d:%02d", bSecs / 60, bSecs % 60);
        renderArcadeText(m_renderer, bBuf, 960.0f, 465.0f, 2.0f, SDL_Color{0, 230, 255, 255}, true);
    } else {
        renderArcadeText(m_renderer, "MISION FALLIDA - FIN DE PARTIDA", 960.0f, 258.0f, 2.6f, SDL_Color{255, 255, 255, 255}, true);
    }

    renderScore(m_renderer, 960.0f, 350.0f, m_score);

    const SDL_FRect btnRect{760.0f, 520.0f, 400.0f, 100.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 170, 75, 255);
    SDL_RenderFillRectF(m_renderer, &btnRect);

    const SDL_FRect btnBorderT{760.0f, 520.0f, 400.0f, 4.0f};
    const SDL_FRect btnBorderB{760.0f, 616.0f, 400.0f, 4.0f};
    const SDL_FRect btnBorderL{760.0f, 520.0f, 4.0f, 100.0f};
    const SDL_FRect btnBorderR{1156.0f, 520.0f, 4.0f, 100.0f};
    SDL_SetRenderDrawColor(m_renderer, 60, 255, 140, 255);
    SDL_RenderFillRectF(m_renderer, &btnBorderT);
    SDL_RenderFillRectF(m_renderer, &btnBorderB);
    SDL_RenderFillRectF(m_renderer, &btnBorderL);
    SDL_RenderFillRectF(m_renderer, &btnBorderR);

    const SDL_Vertex playTri[3]{
        {{935.0f, 545.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}},
        {{935.0f, 595.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}},
        {{975.0f, 570.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}}
    };
    const int triIdx[3]{0, 1, 2};
    SDL_RenderGeometry(m_renderer, nullptr, playTri, 3, triIdx, 3);
}

void Engine::renderMedalsScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // 1. Fondo parallax y capa oscura
    m_levelManager.renderParallaxBackground(m_renderer);
    m_levelManager.renderGround(m_renderer);

    const SDL_FRect overlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 10, 16, 24, 235);
    SDL_RenderFillRectF(m_renderer, &overlay);

    // 2. Encabezado ceremonial
    renderArcadeText(m_renderer, "SALA DE CONDECORACIONES Y EXPEDIENTE MILITAR", 960.0f, 50.0f, 3.4f, SDL_Color{0, 230, 255, 255}, true);
    renderArcadeText(m_renderer, "DISTINCIONES OFICIALES POR EFICACIA EN COMBATE Y DEFENSA GLOBAL", 960.0f, 92.0f, 2.0f, SDL_Color{160, 205, 225, 220}, true);

    // 3. Barra de Estadísticas de Servicio
    const SDL_FRect statBar{180.0f, 124.0f, 1560.0f, 44.0f};
    SDL_SetRenderDrawColor(m_renderer, 20, 30, 42, 245);
    SDL_RenderFillRectF(m_renderer, &statBar);

    const SDL_FRect sbT{180.0f, 124.0f, 1560.0f, 2.0f};
    const SDL_FRect sbB{180.0f, 166.0f, 1560.0f, 2.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 210, 230, 240);
    SDL_RenderFillRectF(m_renderer, &sbT);
    SDL_RenderFillRectF(m_renderer, &sbB);

    char statBuf1[64], statBuf2[64], statBuf3[64], statBuf4[64];
    std::snprintf(statBuf1, sizeof(statBuf1), "DERRIBOS: %llu", static_cast<unsigned long long>(m_totalEnemiesKilled));
    std::snprintf(statBuf2, sizeof(statBuf2), "JEFES: %u", m_totalBossesDefeated);
    std::snprintf(statBuf3, sizeof(statBuf3), "NUKES: %u", m_totalNukesUsed);

    const int bSecs = static_cast<int>(m_bestSurvivalSeconds);
    std::snprintf(statBuf4, sizeof(statBuf4), "SUPERVIVENCIA: %02d:%02d", bSecs / 60, bSecs % 60);

    renderArcadeText(m_renderer, statBuf1, 360.0f, 138.0f, 2.0f, SDL_Color{255, 255, 255, 255}, true);
    renderArcadeText(m_renderer, statBuf2, 740.0f, 138.0f, 2.0f, SDL_Color{255, 255, 255, 255}, true);
    renderArcadeText(m_renderer, statBuf3, 1120.0f, 138.0f, 2.0f, SDL_Color{255, 255, 255, 255}, true);
    renderArcadeText(m_renderer, statBuf4, 1520.0f, 138.0f, 2.0f, SDL_Color{255, 220, 50, 255}, true);

    // 4. Las 6 Medallas Militares (2 filas x 3 columnas)
    struct MedalDef {
        const char* title;
        const char* desc;
        bool unlocked;
        char progress[32];
    };

    MedalDef medals[6];
    // 1. 100 Derribos
    medals[0].title = "100 DERRIBOS AEREOS";
    medals[0].desc = "DESTRUIR 100 AERONAVES HOSTILES";
    medals[0].unlocked = (m_totalEnemiesKilled >= 100);
    std::snprintf(medals[0].progress, sizeof(medals[0].progress), "%llu / 100", static_cast<unsigned long long>(std::min<uint64_t>(m_totalEnemiesKilled, 100)));

    // 2. Cazador de Jefes
    medals[1].title = "CAZADOR DE JEFES";
    medals[1].desc = "DERROTAR 5 SUPERJEFES ENEMIGOS";
    medals[1].unlocked = (m_totalBossesDefeated >= 5);
    std::snprintf(medals[1].progress, sizeof(medals[1].progress), "%u / 5", std::min<uint32_t>(m_totalBossesDefeated, 5));

    // 3. Superviviente 3 Minutos
    medals[2].title = "SUPERVIVIENTE 3 MINUTOS";
    medals[2].desc = "RESISTIR 180s EN MODO SUPERVIVENCIA";
    medals[2].unlocked = (m_bestSurvivalSeconds >= 180.0f);
    std::snprintf(medals[2].progress, sizeof(medals[2].progress), "%02d:%02d / 03:00", bSecs / 60, bSecs % 60);

    // 4. Maestro del Arsenal
    medals[3].title = "MAESTRO DEL ARSENAL";
    medals[3].desc = "ADQUIRIR 10 MEJORAS EN LA ARMERIA";
    medals[3].unlocked = (m_totalUpgradesPurchased >= 10);
    std::snprintf(medals[3].progress, sizeof(medals[3].progress), "%d / 10", std::min(m_totalUpgradesPurchased, 10));

    // 5. Interceptor Nuclear
    medals[4].title = "INTERCEPTOR NUCLEAR";
    medals[4].desc = "ACTIVAR O DETONAR 10 BOMBAS NUKES";
    medals[4].unlocked = (m_totalNukesUsed >= 10);
    std::snprintf(medals[4].progress, sizeof(medals[4].progress), "%u / 10", std::min<uint32_t>(m_totalNukesUsed, 10));

    // 6. Veterano Nivel 25
    medals[5].title = "VETERANO NIVEL 25";
    medals[5].desc = "COMPLETAR LA CAMPANA HASTA NIVEL 25";
    medals[5].unlocked = (m_campaignCompleted || m_isCampaignWon || m_maxUnlockedLevel >= 25);
    std::snprintf(medals[5].progress, sizeof(medals[5].progress), "%d / 25", std::min(m_maxUnlockedLevel, 25));

    constexpr float cardW = 500.0f;
    constexpr float cardH = 210.0f;
    constexpr float colXs[3] = {180.0f, 710.0f, 1240.0f};
    constexpr float rowYs[2] = {190.0f, 430.0f};

    for (int i = 0; i < 6; ++i) {
        const int col = i % 3;
        const int row = i / 3;
        const float cx = colXs[col];
        const float cy = rowYs[row];
        const bool unl = medals[i].unlocked;

        // Base de la insignia
        const SDL_FRect cardBase{cx, cy, cardW, cardH};
        if (unl) {
            SDL_SetRenderDrawColor(m_renderer, 28, 38, 48, 250);
        } else {
            SDL_SetRenderDrawColor(m_renderer, 16, 20, 26, 240);
        }
        SDL_RenderFillRectF(m_renderer, &cardBase);

        // Bordes dorados (desbloqueado) o acero oscuro (bloqueado)
        const float bThick = unl ? 3.0f : 2.0f;
        const SDL_FRect bT{cx, cy, cardW, bThick};
        const SDL_FRect bB{cx, cy + cardH - bThick, cardW, bThick};
        const SDL_FRect bL{cx, cy, bThick, cardH};
        const SDL_FRect bR{cx + cardW - bThick, cy, bThick, cardH};

        const SDL_Color bColor = unl ? SDL_Color{255, 215, 0, 255} : SDL_Color{60, 75, 88, 200};
        SDL_SetRenderDrawColor(m_renderer, bColor.r, bColor.g, bColor.b, bColor.a);
        SDL_RenderFillRectF(m_renderer, &bT);
        SDL_RenderFillRectF(m_renderer, &bB);
        SDL_RenderFillRectF(m_renderer, &bL);
        SDL_RenderFillRectF(m_renderer, &bR);

        // Estrella o Emblema
        if (unl) {
            const float sx = cx + 45.0f;
            const float sy = cy + 45.0f;
            const SDL_Vertex starPoly[4]{
                {{sx, sy - 18.0f}, {255, 220, 40, 255}, {0.0f, 0.0f}},
                {{sx + 18.0f, sy}, {255, 200, 30, 255}, {0.0f, 0.0f}},
                {{sx, sy + 18.0f}, {255, 180, 20, 255}, {0.0f, 0.0f}},
                {{sx - 18.0f, sy}, {255, 200, 30, 255}, {0.0f, 0.0f}}
            };
            const int sIdx[6]{0, 1, 2, 0, 2, 3};
            SDL_RenderGeometry(m_renderer, nullptr, starPoly, 4, sIdx, 6);

            const SDL_FRect sCore{sx - 5.0f, sy - 5.0f, 10.0f, 10.0f};
            SDL_SetRenderDrawColor(m_renderer, 255, 255, 240, 255);
            SDL_RenderFillRectF(m_renderer, &sCore);
        } else {
            const float sx = cx + 45.0f;
            const float sy = cy + 45.0f;
            const SDL_FRect lockBox{sx - 12.0f, sy - 8.0f, 24.0f, 20.0f};
            SDL_SetRenderDrawColor(m_renderer, 70, 80, 95, 255);
            SDL_RenderFillRectF(m_renderer, &lockBox);
            const SDL_FRect lockShackle{sx - 8.0f, sy - 18.0f, 16.0f, 10.0f};
            SDL_SetRenderDrawColor(m_renderer, 95, 110, 125, 255);
            SDL_RenderFillRectF(m_renderer, &lockShackle);
        }

        // Título de la Condecoración
        const SDL_Color titleCol = unl ? SDL_Color{255, 225, 60, 255} : SDL_Color{140, 155, 170, 220};
        renderArcadeText(m_renderer, medals[i].title, cx + 80.0f, cy + 28.0f, 2.3f, titleCol, false);

        // Descripción del Requisito
        const SDL_Color descCol = unl ? SDL_Color{220, 235, 245, 240} : SDL_Color{100, 115, 130, 200};
        renderArcadeText(m_renderer, medals[i].desc, cx + 80.0f, cy + 68.0f, 1.7f, descCol, false);

        // Estado y Progreso
        if (unl) {
            const SDL_FRect stBar{cx + 25.0f, cy + 140.0f, cardW - 50.0f, 38.0f};
            SDL_SetRenderDrawColor(m_renderer, 35, 60, 45, 240);
            SDL_RenderFillRectF(m_renderer, &stBar);
            renderArcadeText(m_renderer, "[ CONDECORACION OTORGADA ]", cx + cardW * 0.5f, cy + 152.0f, 2.1f, SDL_Color{0, 255, 140, 255}, true);
        } else {
            const SDL_FRect stBar{cx + 25.0f, cy + 140.0f, cardW - 50.0f, 38.0f};
            SDL_SetRenderDrawColor(m_renderer, 24, 28, 35, 240);
            SDL_RenderFillRectF(m_renderer, &stBar);

            char progBuf[64];
            std::snprintf(progBuf, sizeof(progBuf), "EN CURSO: %s", medals[i].progress);
            renderArcadeText(m_renderer, progBuf, cx + cardW * 0.5f, cy + 152.0f, 2.0f, SDL_Color{255, 180, 50, 230}, true);
        }
    }

    // 5. Botón Volver al Menú Principal
    constexpr float backX = 710.0f;
    constexpr float backY = 675.0f;
    constexpr float backW = 500.0f;
    constexpr float backH = 58.0f;

    const SDL_FRect backBase{backX, backY, backW, backH};
    SDL_SetRenderDrawColor(m_renderer, 24, 38, 52, 250);
    SDL_RenderFillRectF(m_renderer, &backBase);

    const SDL_FRect bbT{backX, backY, backW, 3.0f};
    const SDL_FRect bbB{backX, backY + backH - 3.0f, backW, 3.0f};
    const SDL_FRect bbL{backX, backY, 3.0f, backH};
    const SDL_FRect bbR{backX + backW - 3.0f, backY, 3.0f, backH};
    SDL_SetRenderDrawColor(m_renderer, 0, 220, 245, 255);
    SDL_RenderFillRectF(m_renderer, &bbT);
    SDL_RenderFillRectF(m_renderer, &bbB);
    SDL_RenderFillRectF(m_renderer, &bbL);
    SDL_RenderFillRectF(m_renderer, &bbR);

    renderArcadeText(m_renderer, "<-- VOLVER AL MENU PRINCIPAL [ESC / ENTER]", 960.0f, backY + 18.0f, 2.4f, SDL_Color{255, 255, 255, 255}, true);
}

void Engine::renderCampaignVictoryScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // 1. Fondo oscurecido con tinte ceremonial profundo
    const SDL_FRect darkOverlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 8, 14, 22, 238);
    SDL_RenderFillRectF(m_renderer, &darkOverlay);

    // 2. Panel ceremonial militar central (1060 x 620)
    const SDL_FRect panel{430.0f, 150.0f, 1060.0f, 620.0f};
    SDL_SetRenderDrawColor(m_renderer, 18, 25, 34, 250);
    SDL_RenderFillRectF(m_renderer, &panel);

    // Borde exterior en Oro Puro (Espesor 4px)
    const SDL_FRect pBorderTop{430.0f, 150.0f, 1060.0f, 4.0f};
    const SDL_FRect pBorderBot{430.0f, 766.0f, 1060.0f, 4.0f};
    const SDL_FRect pBorderLeft{430.0f, 150.0f, 4.0f, 620.0f};
    const SDL_FRect pBorderRight{1486.0f, 150.0f, 4.0f, 620.0f};
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 255);
    SDL_RenderFillRectF(m_renderer, &pBorderTop);
    SDL_RenderFillRectF(m_renderer, &pBorderBot);
    SDL_RenderFillRectF(m_renderer, &pBorderLeft);
    SDL_RenderFillRectF(m_renderer, &pBorderRight);

    // Borde interior esmeralda/cian (Espesor 2px, inset 8px)
    const SDL_FRect inBorderTop{438.0f, 158.0f, 1044.0f, 2.0f};
    const SDL_FRect inBorderBot{438.0f, 758.0f, 1044.0f, 2.0f};
    const SDL_FRect inBorderLeft{438.0f, 158.0f, 2.0f, 602.0f};
    const SDL_FRect inBorderRight{1480.0f, 158.0f, 2.0f, 602.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 220, 160, 180);
    SDL_RenderFillRectF(m_renderer, &inBorderTop);
    SDL_RenderFillRectF(m_renderer, &inBorderBot);
    SDL_RenderFillRectF(m_renderer, &inBorderLeft);
    SDL_RenderFillRectF(m_renderer, &inBorderRight);

    // Escuadras decorativas en las 4 esquinas del panel
    constexpr float cornerSize = 22.0f;
    constexpr float cornerThick = 4.0f;
    SDL_SetRenderDrawColor(m_renderer, 255, 225, 60, 255);
    // Superior izquierda
    const SDL_FRect c1h{430.0f, 150.0f, cornerSize, cornerThick};
    const SDL_FRect c1v{430.0f, 150.0f, cornerThick, cornerSize};
    // Superior derecha
    const SDL_FRect c2h{1490.0f - cornerSize, 150.0f, cornerSize, cornerThick};
    const SDL_FRect c2v{1490.0f - cornerThick, 150.0f, cornerThick, cornerSize};
    // Inferior izquierda
    const SDL_FRect c3h{430.0f, 770.0f - cornerThick, cornerSize, cornerThick};
    const SDL_FRect c3v{430.0f, 770.0f - cornerSize, cornerThick, cornerSize};
    // Inferior derecha
    const SDL_FRect c4h{1490.0f - cornerSize, 770.0f - cornerThick, cornerSize, cornerThick};
    const SDL_FRect c4v{1490.0f - cornerThick, 770.0f - cornerSize, cornerThick, cornerSize};
    SDL_RenderFillRectF(m_renderer, &c1h);
    SDL_RenderFillRectF(m_renderer, &c1v);
    SDL_RenderFillRectF(m_renderer, &c2h);
    SDL_RenderFillRectF(m_renderer, &c2v);
    SDL_RenderFillRectF(m_renderer, &c3h);
    SDL_RenderFillRectF(m_renderer, &c3v);
    SDL_RenderFillRectF(m_renderer, &c4h);
    SDL_RenderFillRectF(m_renderer, &c4v);

    // 3. Banner superior ceremonial: "VICTORY"
    const SDL_FRect bannerBg{470.0f, 185.0f, 980.0f, 85.0f};
    SDL_SetRenderDrawColor(m_renderer, 42, 32, 12, 255); // Bronce imperial
    SDL_RenderFillRectF(m_renderer, &bannerBg);

    const SDL_FRect bStripeT{470.0f, 185.0f, 980.0f, 4.0f};
    const SDL_FRect bStripeB{470.0f, 266.0f, 980.0f, 4.0f};
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 255);
    SDL_RenderFillRectF(m_renderer, &bStripeT);
    SDL_RenderFillRectF(m_renderer, &bStripeB);

    // Letras "VICTORY" geométricas de alto impacto en oro brillante
    const SDL_Color goldText{255, 225, 50, 255};
    const float letY = 208.0f;
    const float letH = 40.0f;
    const float stroke = 5.0f;

    // V (x=835)
    const SDL_FRect v_l{835.0f, letY, stroke, letH - 6.0f};
    const SDL_FRect v_r{851.0f, letY, stroke, letH - 6.0f};
    const SDL_FRect v_b{838.0f, letY + letH - 6.0f, 15.0f, 6.0f};
    // I (x=870)
    const SDL_FRect i_stem{877.0f, letY, stroke, letH};
    const SDL_FRect i_top{870.0f, letY, 18.0f, stroke};
    const SDL_FRect i_bot{870.0f, letY + letH - stroke, 18.0f, stroke};
    // C (x=900)
    const SDL_FRect c_l{900.0f, letY, stroke, letH};
    const SDL_FRect c_t{900.0f, letY, 20.0f, stroke};
    const SDL_FRect c_b{900.0f, letY + letH - stroke, 20.0f, stroke};
    // T (x=930)
    const SDL_FRect t_bar{930.0f, letY, 22.0f, stroke};
    const SDL_FRect t_stem{939.0f, letY, stroke, letH};
    // O (x=962)
    const SDL_FRect o_l{962.0f, letY, stroke, letH};
    const SDL_FRect o_r{980.0f, letY, stroke, letH};
    const SDL_FRect o_t{962.0f, letY, 22.0f, stroke};
    const SDL_FRect o_b{962.0f, letY + letH - stroke, 22.0f, stroke};
    // R (x=994)
    const SDL_FRect r_l{994.0f, letY, stroke, letH};
    const SDL_FRect r_t{994.0f, letY, 20.0f, stroke};
    const SDL_FRect r_r{1010.0f, letY, stroke, 20.0f};
    const SDL_FRect r_m{994.0f, letY + 18.0f, 20.0f, stroke};
    const SDL_FRect r_leg{1008.0f, letY + 22.0f, stroke + 1.0f, 18.0f};
    // Y (x=1026)
    const SDL_FRect y_tl{1026.0f, letY, stroke, 18.0f};
    const SDL_FRect y_tr{1042.0f, letY, stroke, 18.0f};
    const SDL_FRect y_m{1026.0f, letY + 16.0f, 20.0f, stroke};
    const SDL_FRect y_stem{1034.0f, letY + 18.0f, stroke, 22.0f};

    SDL_SetRenderDrawColor(m_renderer, goldText.r, goldText.g, goldText.b, goldText.a);
    SDL_RenderFillRectF(m_renderer, &v_l);
    SDL_RenderFillRectF(m_renderer, &v_r);
    SDL_RenderFillRectF(m_renderer, &v_b);
    SDL_RenderFillRectF(m_renderer, &i_stem);
    SDL_RenderFillRectF(m_renderer, &i_top);
    SDL_RenderFillRectF(m_renderer, &i_bot);
    SDL_RenderFillRectF(m_renderer, &c_l);
    SDL_RenderFillRectF(m_renderer, &c_t);
    SDL_RenderFillRectF(m_renderer, &c_b);
    SDL_RenderFillRectF(m_renderer, &t_bar);
    SDL_RenderFillRectF(m_renderer, &t_stem);
    SDL_RenderFillRectF(m_renderer, &o_l);
    SDL_RenderFillRectF(m_renderer, &o_r);
    SDL_RenderFillRectF(m_renderer, &o_t);
    SDL_RenderFillRectF(m_renderer, &o_b);
    SDL_RenderFillRectF(m_renderer, &r_l);
    SDL_RenderFillRectF(m_renderer, &r_t);
    SDL_RenderFillRectF(m_renderer, &r_r);
    SDL_RenderFillRectF(m_renderer, &r_m);
    SDL_RenderFillRectF(m_renderer, &r_leg);
    SDL_RenderFillRectF(m_renderer, &y_tl);
    SDL_RenderFillRectF(m_renderer, &y_tr);
    SDL_RenderFillRectF(m_renderer, &y_m);
    SDL_RenderFillRectF(m_renderer, &y_stem);

    // Separadores laterales ornamentales
    const SDL_FRect dashL{650.0f, 225.0f, 140.0f, 4.0f};
    const SDL_FRect dashR{1100.0f, 225.0f, 140.0f, 4.0f};
    SDL_RenderFillRectF(m_renderer, &dashL);
    SDL_RenderFillRectF(m_renderer, &dashR);

    // 4. Sub-banner: "CAMPAIGN COMPLETED - GLOBAL THREAT ANNIHILATED"
    const SDL_FRect subBanner{500.0f, 290.0f, 920.0f, 36.0f};
    SDL_SetRenderDrawColor(m_renderer, 15, 42, 28, 255); // Verde militar
    SDL_RenderFillRectF(m_renderer, &subBanner);

    const SDL_FRect subBorder{500.0f, 324.0f, 920.0f, 2.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 240, 140, 255);
    SDL_RenderFillRectF(m_renderer, &subBorder);

    // 5 Estrellas de Victoria (Medallas Militares)
    for (int s = 0; s < 5; ++s) {
        const float sx = 840.0f + static_cast<float>(s) * 60.0f;
        const float sy = 358.0f;
        const SDL_Vertex starPoly[4]{
            {{sx, sy - 14.0f}, {255, 220, 50, 255}, {0.0f, 0.0f}},
            {{sx + 14.0f, sy}, {255, 200, 30, 255}, {0.0f, 0.0f}},
            {{sx, sy + 14.0f}, {255, 180, 20, 255}, {0.0f, 0.0f}},
            {{sx - 14.0f, sy}, {255, 200, 30, 255}, {0.0f, 0.0f}}
        };
        const int sIdx[6]{0, 1, 2, 0, 2, 3};
        SDL_RenderGeometry(m_renderer, nullptr, starPoly, 4, sIdx, 6);

        // Núcleo brillante
        const SDL_FRect sCore{sx - 4.0f, sy - 4.0f, 8.0f, 8.0f};
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 220, 255);
        SDL_RenderFillRectF(m_renderer, &sCore);
    }

    // 5. Marco para Puntuación Final de Campaña
    const SDL_FRect scoreBox{680.0f, 410.0f, 560.0f, 85.0f};
    SDL_SetRenderDrawColor(m_renderer, 25, 34, 46, 255);
    SDL_RenderFillRectF(m_renderer, &scoreBox);

    const SDL_FRect scBTop{680.0f, 410.0f, 560.0f, 3.0f};
    const SDL_FRect scBBot{680.0f, 492.0f, 560.0f, 3.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 210, 240, 255);
    SDL_RenderFillRectF(m_renderer, &scBTop);
    SDL_RenderFillRectF(m_renderer, &scBBot);

    // Renderizar Puntuación en 7 Segmentos
    renderScore(m_renderer, 960.0f, 440.0f, m_score);

    // Insignias de Estadísticas Militares
    // Izquierda: "SECTOR 25 CLEARED"
    const SDL_FRect badgeL{490.0f, 525.0f, 340.0f, 52.0f};
    SDL_SetRenderDrawColor(m_renderer, 22, 30, 40, 250);
    SDL_RenderFillRectF(m_renderer, &badgeL);
    const SDL_FRect bLBorder{490.0f, 525.0f, 340.0f, 2.0f};
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 255);
    SDL_RenderFillRectF(m_renderer, &bLBorder);

    // Derecha: "RANK: SUPREME TITAN SLAYER"
    const SDL_FRect badgeR{1090.0f, 525.0f, 340.0f, 52.0f};
    SDL_SetRenderDrawColor(m_renderer, 22, 30, 40, 250);
    SDL_RenderFillRectF(m_renderer, &badgeR);
    const SDL_FRect bRBorder{1090.0f, 525.0f, 340.0f, 2.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 230, 255, 255);
    SDL_RenderFillRectF(m_renderer, &bRBorder);

    // 6. Botón de Reinicio de Campaña ("PLAY AGAIN / RESTART")
    const SDL_FRect btnRect{760.0f, 640.0f, 400.0f, 80.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 160, 75, 255);
    SDL_RenderFillRectF(m_renderer, &btnRect);

    const SDL_FRect btnBorderT{760.0f, 640.0f, 400.0f, 4.0f};
    const SDL_FRect btnBorderB{760.0f, 716.0f, 400.0f, 4.0f};
    const SDL_FRect btnBorderL{760.0f, 640.0f, 4.0f, 80.0f};
    const SDL_FRect btnBorderR{1156.0f, 640.0f, 4.0f, 80.0f};
    SDL_SetRenderDrawColor(m_renderer, 255, 220, 50, 255);
    SDL_RenderFillRectF(m_renderer, &btnBorderT);
    SDL_RenderFillRectF(m_renderer, &btnBorderB);
    SDL_RenderFillRectF(m_renderer, &btnBorderL);
    SDL_RenderFillRectF(m_renderer, &btnBorderR);

    // Triángulo Play
    const SDL_Vertex playTri[3]{
        {{940.0f, 660.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}},
        {{940.0f, 700.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}},
        {{975.0f, 680.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}}
    };
    const int triIdx[3]{0, 1, 2};
    SDL_RenderGeometry(m_renderer, nullptr, playTri, 3, triIdx, 3);
}

void Engine::renderArcadeTitle(float centerX, float centerY) {
    // Sombra de impacto
    renderArcadeText(m_renderer, "ATOMIC WEAPON", centerX + 4.0f, centerY + 4.0f, 7.0f, SDL_Color{8, 12, 16, 255}, true);

    // Contorno oscuro táctico
    renderArcadeText(m_renderer, "ATOMIC WEAPON", centerX - 2.0f, centerY, 7.0f, SDL_Color{20, 32, 24, 255}, true);
    renderArcadeText(m_renderer, "ATOMIC WEAPON", centerX + 2.0f, centerY, 7.0f, SDL_Color{20, 32, 24, 255}, true);
    renderArcadeText(m_renderer, "ATOMIC WEAPON", centerX, centerY - 2.0f, 7.0f, SDL_Color{20, 32, 24, 255}, true);
    renderArcadeText(m_renderer, "ATOMIC WEAPON", centerX, centerY + 2.0f, 7.0f, SDL_Color{20, 32, 24, 255}, true);

    // Frente brillante en Oro / Amarillo Militar
    renderArcadeText(m_renderer, "ATOMIC WEAPON", centerX, centerY, 7.0f, SDL_Color{255, 220, 40, 255}, true);

    // Franjas de peligro (Hazard Stripes) a la izquierda y derecha
    for (int s = 0; s < 4; ++s) {
        const float sxL = centerX - 380.0f + static_cast<float>(s) * 16.0f;
        const float sxR = centerX + 320.0f + static_cast<float>(s) * 16.0f;
        const SDL_FRect stripeL{sxL, centerY + 18.0f, 10.0f, 18.0f};
        const SDL_FRect stripeR{sxR, centerY + 18.0f, 10.0f, 18.0f};
        SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 255);
        SDL_RenderFillRectF(m_renderer, &stripeL);
        SDL_RenderFillRectF(m_renderer, &stripeR);
    }

    // Subtítulo militar táctico
    renderArcadeText(m_renderer, "HEAVY ANTI-AIR DEFENSE FORCE", centerX, centerY + 58.0f, 2.2f, SDL_Color{0, 230, 255, 240}, true);

    // Línea divisoria decorativa
    const SDL_FRect divider{centerX - 360.0f, centerY + 80.0f, 720.0f, 3.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 210, 230, 240);
    SDL_RenderFillRectF(m_renderer, &divider);
}

void Engine::renderLoadingScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Fondo táctico oscuro
    const SDL_FRect bgRect{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 12, 18, 26, 255);
    SDL_RenderFillRectF(m_renderer, &bgRect);

    // Rejilla sutil de radar táctico en el fondo
    SDL_SetRenderDrawColor(m_renderer, 22, 34, 48, 90);
    for (float x = 0.0f; x < static_cast<float>(LOGICAL_WIDTH); x += 80.0f) {
        const SDL_FRect vLine{x, 0.0f, 2.0f, static_cast<float>(LOGICAL_HEIGHT)};
        SDL_RenderFillRectF(m_renderer, &vLine);
    }
    for (float y = 0.0f; y < static_cast<float>(LOGICAL_HEIGHT); y += 80.0f) {
        const SDL_FRect hLine{0.0f, y, static_cast<float>(LOGICAL_WIDTH), 2.0f};
        SDL_RenderFillRectF(m_renderer, &hLine);
    }

    // Título grande "ATOMIC WEAPON"
    renderArcadeTitle(960.0f, 200.0f);

    // Tanque central
    SDL_Texture* tankTex = Core::AssetManager::getInstance().getSpriteSheet();
    if (tankTex) {
        const SDL_Rect chassisSrc = Core::AssetManager::getInstance().getTankChassisSrc();
        const SDL_Rect barrelSrc = Core::AssetManager::getInstance().getTankBarrelSrc();

        // Chasis del tanque
        const SDL_FRect chassisDst{960.0f - 80.0f, 430.0f, 160.0f, 96.0f};
        SDL_RenderCopyF(m_renderer, tankTex, &chassisSrc, &chassisDst);

        // Tubo del cañón apuntando en ángulo táctico
        const SDL_FRect barrelDst{960.0f - 15.0f, 420.0f, 85.0f, 32.0f};
        const SDL_FPoint pivot{10.0f, 16.0f};
        SDL_RenderCopyExF(m_renderer, tankTex, &barrelSrc, &barrelDst, -35.0, &pivot, SDL_FLIP_NONE);
    }

    // Barra de carga progresiva (0% a 100% durante 1.8f segundos)
    constexpr float barW = 680.0f;
    constexpr float barH = 34.0f;
    constexpr float barX = 960.0f - barW * 0.5f;
    constexpr float barY = 660.0f;

    // Marco exterior de la barra
    const SDL_FRect barFrame{barX, barY, barW, barH};
    SDL_SetRenderDrawColor(m_renderer, 18, 25, 35, 250);
    SDL_RenderFillRectF(m_renderer, &barFrame);

    const SDL_FRect bTop{barX, barY, barW, 3.0f};
    const SDL_FRect bBot{barX, barY + barH - 3.0f, barW, 3.0f};
    const SDL_FRect bLeft{barX, barY, 3.0f, barH};
    const SDL_FRect bRight{barX + barW - 3.0f, barY, 3.0f, barH};
    SDL_SetRenderDrawColor(m_renderer, 0, 220, 140, 255);
    SDL_RenderFillRectF(m_renderer, &bTop);
    SDL_RenderFillRectF(m_renderer, &bBot);
    SDL_RenderFillRectF(m_renderer, &bLeft);
    SDL_RenderFillRectF(m_renderer, &bRight);

    // Relleno progresivo
    const float progressRatio = std::clamp(m_loadingTimer / 1.8f, 0.0f, 1.0f);
    const float fillW = (barW - 8.0f) * progressRatio;
    if (fillW > 0.0f) {
        const SDL_FRect fillBar{barX + 4.0f, barY + 4.0f, fillW, barH - 8.0f};
        SDL_SetRenderDrawColor(m_renderer, 0, 230, 120, 255);
        SDL_RenderFillRectF(m_renderer, &fillBar);
    }

    // Separadores de segmento cada 10%
    SDL_SetRenderDrawColor(m_renderer, 15, 24, 32, 220);
    for (int i = 1; i < 10; ++i) {
        const SDL_FRect tick{barX + 4.0f + static_cast<float>(i) * ((barW - 8.0f) / 10.0f), barY + 4.0f, 2.0f, barH - 8.0f};
        SDL_RenderFillRectF(m_renderer, &tick);
    }

    // Texto descriptivo
    renderArcadeText(m_renderer, "INICIALIZANDO PROTOCOLOS DE DEFENSA MILITAR", 960.0f, 620.0f, 2.4f, SDL_Color{0, 230, 255, 255}, true);

    const int pct = static_cast<int>(progressRatio * 100.0f);
    char pctBuffer[16];
    pctBuffer[0] = static_cast<char>('0' + (pct / 100));
    pctBuffer[1] = static_cast<char>('0' + ((pct / 10) % 10));
    pctBuffer[2] = static_cast<char>('0' + (pct % 10));
    pctBuffer[3] = '%';
    pctBuffer[4] = '\0';
    renderArcadeText(m_renderer, (pct >= 100) ? "100%" : (pctBuffer[1] != '0' ? &pctBuffer[1] : &pctBuffer[2]), 960.0f, 710.0f, 2.8f, SDL_Color{255, 225, 45, 255}, true);

    renderArcadeText(m_renderer, "PREPARANDO ARSENAL TACTICO Y REJILLAS DE RADAR", 960.0f, 755.0f, 1.9f, SDL_Color{140, 180, 210, 200}, true);
}

void Engine::renderMainMenuScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Fondo Parallax y Suelo en movimiento suave
    m_levelManager.renderParallaxBackground(m_renderer);
    m_levelManager.renderGround(m_renderer);

    // Capa oscura translúcida para legibilidad
    const SDL_FRect overlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 10, 16, 24, 215);
    SDL_RenderFillRectF(m_renderer, &overlay);

    // Título Principal "ATOMIC WEAPON"
    renderArcadeTitle(960.0f, 110.0f);

    // Panel de HIGH SCORE
    renderArcadeText(m_renderer, "RECORD DE COMBATE - HIGH SCORE", 960.0f, 220.0f, 2.2f, SDL_Color{0, 220, 240, 255}, true);
    renderScore(m_renderer, 960.0f, 246.0f, m_highScore);

    // 6 Botones Grandes Interactivos
    constexpr float btnX = 690.0f;
    constexpr float btnW = 540.0f;
    constexpr float btnH = 56.0f;
    constexpr float btnYs[6] = {300.0f, 368.0f, 436.0f, 504.0f, 572.0f, 640.0f};

    const char* labels[6] = {
        (m_maxUnlockedLevel > 1) ? "CONTINUAR CAMPANA" : "NUEVA PARTIDA",
        "MODO SUPERVIVENCIA",
        "SELECCIONAR NIVEL",
        "MEDALLAS Y RECORDS",
        "AJUSTES",
        "SALIR DEL JUEGO"
    };

    for (int i = 0; i < 6; ++i) {
        const float by = btnYs[i];
        const bool isSelected = (i == m_mainMenuSelectedIndex);

        // Fondo del botón
        const SDL_FRect btnBase{btnX, by, btnW, btnH};
        if (isSelected) {
            SDL_SetRenderDrawColor(m_renderer, 32, 48, 62, 250);
        } else {
            SDL_SetRenderDrawColor(m_renderer, 18, 25, 34, 240);
        }
        SDL_RenderFillRectF(m_renderer, &btnBase);

        // Bordes
        const float bThick = isSelected ? 4.0f : 2.0f;
        const SDL_FRect bT{btnX, by, btnW, bThick};
        const SDL_FRect bB{btnX, by + btnH - bThick, btnW, bThick};
        const SDL_FRect bL{btnX, by, bThick, btnH};
        const SDL_FRect bR{btnX + btnW - bThick, by, bThick, btnH};

        const SDL_Color bColor = isSelected ? SDL_Color{255, 215, 0, 255} : SDL_Color{50, 85, 110, 220};
        SDL_SetRenderDrawColor(m_renderer, bColor.r, bColor.g, bColor.b, bColor.a);
        SDL_RenderFillRectF(m_renderer, &bT);
        SDL_RenderFillRectF(m_renderer, &bB);
        SDL_RenderFillRectF(m_renderer, &bL);
        SDL_RenderFillRectF(m_renderer, &bR);

        // Flechas selectoras para el botón activo
        if (isSelected) {
            renderArcadeText(m_renderer, ">>", btnX + 24.0f, by + 18.0f, 2.8f, SDL_Color{255, 220, 50, 255});
            renderArcadeText(m_renderer, "<<", btnX + btnW - 54.0f, by + 18.0f, 2.8f, SDL_Color{255, 220, 50, 255});
        }

        // Texto del botón
        const SDL_Color txtColor = isSelected ? SDL_Color{255, 255, 245, 255} : SDL_Color{180, 210, 230, 230};
        renderArcadeText(m_renderer, labels[i], 960.0f, by + 18.0f, 3.0f, txtColor, true);

        // Subtexto
        if (i == 0 && m_maxUnlockedLevel > 1) {
            renderArcadeText(m_renderer, "DESDE SECTOR DESBLOQUEADO", 960.0f, by + 39.0f, 1.4f, SDL_Color{0, 240, 150, 255}, true);
        } else if (i == 1) {
            renderArcadeText(m_renderer, "OLEADAS INFINITAS Y JEFES CADA 60s", 960.0f, by + 39.0f, 1.3f, SDL_Color{255, 180, 40, 230}, true);
        }
    }

    // Pie de pantalla con instrucciones
    renderArcadeText(m_renderer, "CONTROLES: TOUCH / MOUSE / FLECHAS (W/S) / ENTER O BOTON A", 960.0f, 980.0f, 2.0f, SDL_Color{120, 160, 185, 200}, true);
}

void Engine::renderLevelSelectScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Fondo oscuro
    const SDL_FRect overlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 10, 16, 24, 240);
    SDL_RenderFillRectF(m_renderer, &overlay);

    // Encabezado
    renderArcadeText(m_renderer, "SELECCION DE SECTOR MILITAR", 960.0f, 65.0f, 4.0f, SDL_Color{0, 230, 255, 255}, true);
    renderArcadeText(m_renderer, "SELECCIONA CUALQUIER MISION DESBLOQUEADA (1 AL 25)", 960.0f, 115.0f, 2.4f, SDL_Color{160, 205, 225, 230}, true);

    // Cuadrícula 5x5 de botones para Niveles 1 al 25
    constexpr float gridStartX = 637.0f;
    constexpr float gridStartY = 165.0f;
    constexpr float btnW = 110.0f;
    constexpr float btnH = 72.0f;
    constexpr float gapX = 24.0f;
    constexpr float gapY = 16.0f;

    for (int lvl = 1; lvl <= 25; ++lvl) {
        const int col = (lvl - 1) % 5;
        const int row = (lvl - 1) / 5;
        const float bx = gridStartX + static_cast<float>(col) * (btnW + gapX);
        const float by = gridStartY + static_cast<float>(row) * (btnH + gapY);

        const bool isUnlocked = (lvl <= m_maxUnlockedLevel);
        const bool isSelected = ((lvl - 1) == m_levelSelectSelectedIndex);

        const SDL_FRect btnBase{bx, by, btnW, btnH};
        if (isUnlocked) {
            SDL_SetRenderDrawColor(m_renderer, 20, 38, 30, 245);
        } else {
            SDL_SetRenderDrawColor(m_renderer, 16, 20, 24, 230);
        }
        SDL_RenderFillRectF(m_renderer, &btnBase);

        // Bordes
        const float bThick = isSelected ? 3.0f : 2.0f;
        const SDL_FRect bT{bx, by, btnW, bThick};
        const SDL_FRect bB{bx, by + btnH - bThick, btnW, bThick};
        const SDL_FRect bL{bx, by, bThick, btnH};
        const SDL_FRect bR{bx + btnW - bThick, by, bThick, btnH};

        SDL_Color bColor;
        if (isSelected) {
            bColor = SDL_Color{255, 220, 40, 255};
        } else if (isUnlocked) {
            bColor = SDL_Color{0, 220, 120, 255};
        } else {
            bColor = SDL_Color{45, 52, 60, 200};
        }
        SDL_SetRenderDrawColor(m_renderer, bColor.r, bColor.g, bColor.b, bColor.a);
        SDL_RenderFillRectF(m_renderer, &bT);
        SDL_RenderFillRectF(m_renderer, &bB);
        SDL_RenderFillRectF(m_renderer, &bL);
        SDL_RenderFillRectF(m_renderer, &bR);

        if (isUnlocked) {
            renderArcadeText(m_renderer, "SECTOR", bx + btnW * 0.5f, by + 10.0f, 1.5f, SDL_Color{140, 200, 180, 240}, true);

            // Dos dígitos 7-segmentos
            const int tenDigit = lvl / 10;
            const int unitDigit = lvl % 10;
            render7SegmentDigit(m_renderer, bx + 34.0f, by + 26.0f, tenDigit, SDL_Color{0, 240, 255, 255}, SDL_Color{15, 45, 55, 120});
            render7SegmentDigit(m_renderer, bx + 58.0f, by + 26.0f, unitDigit, SDL_Color{0, 240, 255, 255}, SDL_Color{15, 45, 55, 120});

            // Indicador de Boss (Jefe cada 5 niveles)
            if (lvl % 5 == 0) {
                const SDL_FRect bossBadge{bx + btnW - 18.0f, by + 4.0f, 14.0f, 14.0f};
                SDL_SetRenderDrawColor(m_renderer, 240, 45, 45, 255);
                SDL_RenderFillRectF(m_renderer, &bossBadge);
                renderArcadeText(m_renderer, "B", bx + btnW - 11.0f, by + 5.0f, 1.4f, SDL_Color{255, 255, 255, 255}, true);
            }

            // Franja inferior con color temático del sector
            const SDL_FRect stripe{bx + 12.0f, by + 60.0f, btnW - 24.0f, 4.0f};
            SDL_SetRenderDrawColor(m_renderer, 0, 210, 140, 255);
            SDL_RenderFillRectF(m_renderer, &stripe);
        } else {
            // Icono geométrico de candado cerrado
            const SDL_FRect lockBody{bx + 45.0f, by + 34.0f, 20.0f, 18.0f};
            SDL_SetRenderDrawColor(m_renderer, 110, 120, 130, 255);
            SDL_RenderFillRectF(m_renderer, &lockBody);

            const SDL_FRect lockShackle{bx + 48.0f, by + 22.0f, 14.0f, 14.0f};
            SDL_SetRenderDrawColor(m_renderer, 140, 155, 170, 255);
            SDL_RenderFillRectF(m_renderer, &lockShackle);

            const SDL_FRect lockHole{bx + 51.0f, by + 25.0f, 8.0f, 11.0f};
            SDL_SetRenderDrawColor(m_renderer, 16, 20, 24, 230);
            SDL_RenderFillRectF(m_renderer, &lockHole);

            const SDL_FRect keyhole{bx + 53.0f, by + 40.0f, 4.0f, 6.0f};
            SDL_SetRenderDrawColor(m_renderer, 20, 25, 30, 255);
            SDL_RenderFillRectF(m_renderer, &keyhole);

            // Nivel bloqueado en color atenuado
            char lvlBuf[4];
            lvlBuf[0] = static_cast<char>('0' + (lvl / 10));
            lvlBuf[1] = static_cast<char>('0' + (lvl % 10));
            lvlBuf[2] = '\0';
            renderArcadeText(m_renderer, lvlBuf, bx + btnW * 0.5f, by + 56.0f, 1.4f, SDL_Color{80, 90, 100, 200}, true);
        }
    }

    // Botón "VOLVER AL MENÚ"
    const bool isBackSelected = (m_levelSelectSelectedIndex == 25);
    const SDL_FRect backBase{760.0f, 650.0f, 400.0f, 58.0f};
    SDL_SetRenderDrawColor(m_renderer, isBackSelected ? 35 : 20, isBackSelected ? 50 : 28, isBackSelected ? 65 : 36, 245);
    SDL_RenderFillRectF(m_renderer, &backBase);

    const SDL_FRect bkTop{760.0f, 650.0f, 400.0f, 3.0f};
    const SDL_FRect bkBot{760.0f, 705.0f, 400.0f, 3.0f};
    const SDL_Color bkColor = isBackSelected ? SDL_Color{255, 215, 0, 255} : SDL_Color{0, 215, 120, 240};
    SDL_SetRenderDrawColor(m_renderer, bkColor.r, bkColor.g, bkColor.b, bkColor.a);
    SDL_RenderFillRectF(m_renderer, &bkTop);
    SDL_RenderFillRectF(m_renderer, &bkBot);

    renderArcadeText(m_renderer, "<-- VOLVER AL MENU", 960.0f, 668.0f, 2.8f, isBackSelected ? SDL_Color{255, 255, 240, 255} : SDL_Color{180, 220, 235, 240}, true);
}

void Engine::renderPauseButton() {
    // Botón táctico de pausa [1820, 20, 70, 70]
    const SDL_FRect btnBase{1820.0f, 20.0f, 70.0f, 70.0f};
    SDL_SetRenderDrawColor(m_renderer, 18, 25, 34, 230);
    SDL_RenderFillRectF(m_renderer, &btnBase);

    const SDL_FRect bT{1820.0f, 20.0f, 70.0f, 3.0f};
    const SDL_FRect bB{1820.0f, 87.0f, 70.0f, 3.0f};
    const SDL_FRect bL{1820.0f, 20.0f, 3.0f, 70.0f};
    const SDL_FRect bR{1887.0f, 20.0f, 3.0f, 70.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 210, 230, 240);
    SDL_RenderFillRectF(m_renderer, &bT);
    SDL_RenderFillRectF(m_renderer, &bB);
    SDL_RenderFillRectF(m_renderer, &bL);
    SDL_RenderFillRectF(m_renderer, &bR);

    // Símbolo de pausa '||' (dos barras gruesas sólidas)
    const SDL_FRect bar1{1838.0f, 32.0f, 10.0f, 46.0f};
    const SDL_FRect bar2{1862.0f, 32.0f, 10.0f, 46.0f};
    SDL_SetRenderDrawColor(m_renderer, 230, 250, 255, 255);
    SDL_RenderFillRectF(m_renderer, &bar1);
    SDL_RenderFillRectF(m_renderer, &bar2);
}

void Engine::renderPauseScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Fondo oscurecido manteniendo el mundo visible detrás
    const SDL_FRect darkOverlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 8, 12, 16, 215);
    SDL_RenderFillRectF(m_renderer, &darkOverlay);

    // Panel modal central
    const SDL_FRect modalBase{620.0f, 160.0f, 680.0f, 560.0f};
    SDL_SetRenderDrawColor(m_renderer, 18, 26, 35, 250);
    SDL_RenderFillRectF(m_renderer, &modalBase);

    const SDL_FRect mT{620.0f, 160.0f, 680.0f, 4.0f};
    const SDL_FRect mB{620.0f, 716.0f, 680.0f, 4.0f};
    const SDL_FRect mL{620.0f, 160.0f, 4.0f, 560.0f};
    const SDL_FRect mR{1296.0f, 160.0f, 4.0f, 560.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 220, 160, 255);
    SDL_RenderFillRectF(m_renderer, &mT);
    SDL_RenderFillRectF(m_renderer, &mB);
    SDL_RenderFillRectF(m_renderer, &mL);
    SDL_RenderFillRectF(m_renderer, &mR);

    // Encabezado
    const SDL_FRect headerBar{640.0f, 180.0f, 640.0f, 60.0f};
    SDL_SetRenderDrawColor(m_renderer, 28, 38, 48, 255);
    SDL_RenderFillRectF(m_renderer, &headerBar);
    renderArcadeText(m_renderer, "MISION EN PAUSA", 960.0f, 196.0f, 3.8f, SDL_Color{255, 220, 50, 255}, true);

    // 4 Botones de Pausa
    constexpr float btnX = 720.0f;
    constexpr float btnW = 480.0f;
    constexpr float btnH = 60.0f;
    constexpr float btnYs[4] = {270.0f, 345.0f, 420.0f, 495.0f};
    const char* labels[4] = {
        "REANUDAR PARTIDA",
        "REINICIAR SECTOR",
        "AJUSTES",
        "SALIR AL MENU"
    };

    for (int i = 0; i < 4; ++i) {
        const float by = btnYs[i];
        const bool isSelected = (i == m_pauseSelectedIndex);

        const SDL_FRect btnBase{btnX, by, btnW, btnH};
        SDL_SetRenderDrawColor(m_renderer, isSelected ? 35 : 22, isSelected ? 50 : 30, isSelected ? 65 : 40, 250);
        SDL_RenderFillRectF(m_renderer, &btnBase);

        const float bThick = isSelected ? 3.0f : 2.0f;
        const SDL_FRect bT{btnX, by, btnW, bThick};
        const SDL_FRect bB{btnX, by + btnH - bThick, btnW, bThick};
        const SDL_Color bColor = isSelected ? SDL_Color{255, 215, 0, 255} : SDL_Color{60, 95, 120, 200};
        SDL_SetRenderDrawColor(m_renderer, bColor.r, bColor.g, bColor.b, bColor.a);
        SDL_RenderFillRectF(m_renderer, &bT);
        SDL_RenderFillRectF(m_renderer, &bB);

        if (isSelected) {
            renderArcadeText(m_renderer, ">>", btnX + 24.0f, by + 20.0f, 2.8f, SDL_Color{255, 220, 50, 255});
            renderArcadeText(m_renderer, "<<", btnX + btnW - 50.0f, by + 20.0f, 2.8f, SDL_Color{255, 220, 50, 255});
        }

        const SDL_Color txtColor = isSelected ? SDL_Color{255, 255, 240, 255} : SDL_Color{180, 215, 235, 230};
        renderArcadeText(m_renderer, labels[i], 960.0f, by + 20.0f, 3.0f, txtColor, true);
    }

    renderArcadeText(m_renderer, "PRESIONA ESC / P O BOTON START PARA REANUDAR", 960.0f, 620.0f, 1.8f, SDL_Color{130, 175, 205, 220}, true);
}

void Engine::renderSettingsScreen() {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Fondo oscurecido
    const SDL_FRect darkOverlay{0.0f, 0.0f, static_cast<float>(LOGICAL_WIDTH), static_cast<float>(LOGICAL_HEIGHT)};
    SDL_SetRenderDrawColor(m_renderer, 8, 14, 20, 235);
    SDL_RenderFillRectF(m_renderer, &darkOverlay);

    // Modal de Ajustes
    const SDL_FRect modalBase{480.0f, 120.0f, 960.0f, 680.0f};
    SDL_SetRenderDrawColor(m_renderer, 16, 24, 32, 250);
    SDL_RenderFillRectF(m_renderer, &modalBase);

    const SDL_FRect mT{480.0f, 120.0f, 960.0f, 4.0f};
    const SDL_FRect mB{480.0f, 796.0f, 960.0f, 4.0f};
    const SDL_FRect mL{480.0f, 120.0f, 4.0f, 680.0f};
    const SDL_FRect mR{1436.0f, 120.0f, 4.0f, 680.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 215, 230, 255);
    SDL_RenderFillRectF(m_renderer, &mT);
    SDL_RenderFillRectF(m_renderer, &mB);
    SDL_RenderFillRectF(m_renderer, &mL);
    SDL_RenderFillRectF(m_renderer, &mR);

    // Encabezado
    renderArcadeText(m_renderer, "AJUSTES DE CONFIGURACION", 960.0f, 150.0f, 3.8f, SDL_Color{255, 220, 50, 255}, true);
    const SDL_FRect divider{540.0f, 195.0f, 840.0f, 3.0f};
    SDL_SetRenderDrawColor(m_renderer, 0, 215, 230, 200);
    SDL_RenderFillRectF(m_renderer, &divider);

    // -------------------------------------------------------------------------
    // Fila 0: Volumen de Efectos (SFX)
    // -------------------------------------------------------------------------
    const bool isRow0Selected = (m_settingsSelectedIndex == 0);
    renderArcadeText(m_renderer, "VOLUMEN EFECTOS SFX", 520.0f, 234.0f, 2.4f, isRow0Selected ? SDL_Color{255, 220, 50, 255} : SDL_Color{200, 225, 240, 255});

    // Botón [-]
    const SDL_FRect sfxMinus{860.0f, 222.0f, 44.0f, 44.0f};
    SDL_SetRenderDrawColor(m_renderer, 30, 42, 54, 250);
    SDL_RenderFillRectF(m_renderer, &sfxMinus);
    renderArcadeText(m_renderer, "-", 882.0f, 232.0f, 3.0f, SDL_Color{255, 255, 255, 255}, true);

    // 10 Barras de Volumen
    for (int i = 0; i < 10; ++i) {
        const float barX = 916.0f + static_cast<float>(i) * 32.0f;
        const bool isLit = (i * 10 < m_sfxVolume);
        const SDL_FRect bar{barX, 224.0f, 26.0f, 40.0f};
        if (isLit) {
            SDL_SetRenderDrawColor(m_renderer, 0, 230, 140, 255);
        } else {
            SDL_SetRenderDrawColor(m_renderer, 35, 45, 55, 180);
        }
        SDL_RenderFillRectF(m_renderer, &bar);
    }

    // Botón [+]
    const SDL_FRect sfxPlus{1246.0f, 222.0f, 44.0f, 44.0f};
    SDL_SetRenderDrawColor(m_renderer, 30, 42, 54, 250);
    SDL_RenderFillRectF(m_renderer, &sfxPlus);
    renderArcadeText(m_renderer, "+", 1268.0f, 232.0f, 3.0f, SDL_Color{255, 255, 255, 255}, true);

    // -------------------------------------------------------------------------
    // Fila 1: Volumen de Música
    // -------------------------------------------------------------------------
    const bool isRow1Selected = (m_settingsSelectedIndex == 1);
    renderArcadeText(m_renderer, "VOLUMEN MUSICA", 520.0f, 324.0f, 2.4f, isRow1Selected ? SDL_Color{255, 220, 50, 255} : SDL_Color{200, 225, 240, 255});

    // Botón [-]
    const SDL_FRect musMinus{860.0f, 312.0f, 44.0f, 44.0f};
    SDL_SetRenderDrawColor(m_renderer, 30, 42, 54, 250);
    SDL_RenderFillRectF(m_renderer, &musMinus);
    renderArcadeText(m_renderer, "-", 882.0f, 322.0f, 3.0f, SDL_Color{255, 255, 255, 255}, true);

    // 10 Barras de Volumen
    for (int i = 0; i < 10; ++i) {
        const float barX = 916.0f + static_cast<float>(i) * 32.0f;
        const bool isLit = (i * 10 < m_musicVolume);
        const SDL_FRect bar{barX, 314.0f, 26.0f, 40.0f};
        if (isLit) {
            SDL_SetRenderDrawColor(m_renderer, 0, 215, 255, 255);
        } else {
            SDL_SetRenderDrawColor(m_renderer, 35, 45, 55, 180);
        }
        SDL_RenderFillRectF(m_renderer, &bar);
    }

    // Botón [+]
    const SDL_FRect musPlus{1246.0f, 312.0f, 44.0f, 44.0f};
    SDL_SetRenderDrawColor(m_renderer, 30, 42, 54, 250);
    SDL_RenderFillRectF(m_renderer, &musPlus);
    renderArcadeText(m_renderer, "+", 1268.0f, 322.0f, 3.0f, SDL_Color{255, 255, 255, 255}, true);

    // -------------------------------------------------------------------------
    // Fila 2: Sacudida de Pantalla (Screen Shake)
    // -------------------------------------------------------------------------
    const bool isRow2Selected = (m_settingsSelectedIndex == 2);
    renderArcadeText(m_renderer, "SACUDIDA DE PANTALLA", 520.0f, 414.0f, 2.4f, isRow2Selected ? SDL_Color{255, 220, 50, 255} : SDL_Color{200, 225, 240, 255});

    const SDL_FRect shakeToggle{916.0f, 402.0f, 374.0f, 44.0f};
    if (m_screenShakeEnabled) {
        SDL_SetRenderDrawColor(m_renderer, 0, 160, 75, 255);
        SDL_RenderFillRectF(m_renderer, &shakeToggle);
        const SDL_FRect shB{916.0f, 402.0f, 374.0f, 2.0f};
        SDL_SetRenderDrawColor(m_renderer, 60, 255, 140, 255);
        SDL_RenderFillRectF(m_renderer, &shB);
        renderArcadeText(m_renderer, "ACTIVADO  (ON)", 1103.0f, 414.0f, 2.5f, SDL_Color{255, 255, 255, 255}, true);
    } else {
        SDL_SetRenderDrawColor(m_renderer, 150, 35, 35, 255);
        SDL_RenderFillRectF(m_renderer, &shakeToggle);
        const SDL_FRect shB{916.0f, 402.0f, 374.0f, 2.0f};
        SDL_SetRenderDrawColor(m_renderer, 255, 90, 90, 255);
        SDL_RenderFillRectF(m_renderer, &shB);
        renderArcadeText(m_renderer, "DESACTIVADO (OFF)", 1103.0f, 414.0f, 2.5f, SDL_Color{255, 255, 255, 255}, true);
    }

    // -------------------------------------------------------------------------
    // Fila 3: Opacidad de UI Táctil (Baja, Media, Alta)
    // -------------------------------------------------------------------------
    const bool isRow3Selected = (m_settingsSelectedIndex == 3);
    renderArcadeText(m_renderer, "OPACIDAD HUD TACTIL", 520.0f, 504.0f, 2.4f, isRow3Selected ? SDL_Color{255, 220, 50, 255} : SDL_Color{200, 225, 240, 255});

    const char* opacLabels[3] = {"BAJA", "MEDIA", "ALTA"};
    for (int i = 0; i < 3; ++i) {
        const float ox = 916.0f + static_cast<float>(i) * 128.0f;
        const bool isActive = (m_touchOpacityLevel == i);
        const SDL_FRect opBtn{ox, 492.0f, 118.0f, 44.0f};

        if (isActive) {
            SDL_SetRenderDrawColor(m_renderer, 0, 160, 85, 255);
            SDL_RenderFillRectF(m_renderer, &opBtn);
            const SDL_FRect opBorder{ox, 492.0f, 118.0f, 3.0f};
            SDL_SetRenderDrawColor(m_renderer, 60, 255, 140, 255);
            SDL_RenderFillRectF(m_renderer, &opBorder);
        } else {
            SDL_SetRenderDrawColor(m_renderer, 28, 38, 48, 250);
            SDL_RenderFillRectF(m_renderer, &opBtn);
            const SDL_FRect opBorder{ox, 492.0f, 118.0f, 2.0f};
            SDL_SetRenderDrawColor(m_renderer, 60, 85, 110, 200);
            SDL_RenderFillRectF(m_renderer, &opBorder);
        }

        renderArcadeText(m_renderer, opacLabels[i], ox + 59.0f, 504.0f, 2.4f, isActive ? SDL_Color{255, 255, 255, 255} : SDL_Color{160, 190, 210, 220}, true);
    }

    // -------------------------------------------------------------------------
    // Fila 4: Botón "VOLVER"
    // -------------------------------------------------------------------------
    const bool isBackSelected = (m_settingsSelectedIndex == 4);
    const SDL_FRect backBase{740.0f, 620.0f, 440.0f, 60.0f};
    SDL_SetRenderDrawColor(m_renderer, isBackSelected ? 35 : 22, isBackSelected ? 50 : 30, isBackSelected ? 65 : 40, 250);
    SDL_RenderFillRectF(m_renderer, &backBase);

    const SDL_FRect bkTop{740.0f, 620.0f, 440.0f, isBackSelected ? 4.0f : 2.0f};
    const SDL_FRect bkBot{740.0f, 676.0f, 440.0f, isBackSelected ? 4.0f : 2.0f};
    const SDL_Color bkColor = isBackSelected ? SDL_Color{255, 215, 0, 255} : SDL_Color{0, 215, 120, 240};
    SDL_SetRenderDrawColor(m_renderer, bkColor.r, bkColor.g, bkColor.b, bkColor.a);
    SDL_RenderFillRectF(m_renderer, &bkTop);
    SDL_RenderFillRectF(m_renderer, &bkBot);

    if (isBackSelected) {
        renderArcadeText(m_renderer, ">>", 764.0f, 638.0f, 2.8f, SDL_Color{255, 220, 50, 255});
        renderArcadeText(m_renderer, "<<", 1146.0f, 638.0f, 2.8f, SDL_Color{255, 220, 50, 255});
    }

    renderArcadeText(m_renderer, "<-- GUARDAR Y VOLVER", 960.0f, 638.0f, 2.8f, isBackSelected ? SDL_Color{255, 255, 240, 255} : SDL_Color{180, 220, 235, 240}, true);
}

void Engine::render() {
    // 1. Limpiar el exterior (letterbox) en negro puro
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    switch (m_gameState) {
    case GameState::Loading:
        renderLoadingScreen();
        break;

    case GameState::MainMenu:
        renderMainMenuScreen();
        break;

    case GameState::LevelSelect:
        renderLevelSelectScreen();
        break;

    case GameState::Medals:
        renderMedalsScreen();
        break;

    case GameState::Playing:
    case GameState::Paused:
    case GameState::Settings:
    case GameState::LevelSummary:
    case GameState::Armory:
    case GameState::GameOver:
    case GameState::Victory: {
        // En Settings desde el Menú Principal, mostrar el menú de fondo
        if (m_gameState == GameState::Settings && m_previousState == GameState::MainMenu) {
            renderMainMenuScreen();
            renderSettingsScreen();
            break;
        }

        // En los estados de juego y menús superpuestos, renderizar el mundo
        const float shakeX = (m_gameState == GameState::Playing && m_screenShakeEnabled) ? m_shakeOffsetX : 0.0f;
        const float shakeY = (m_gameState == GameState::Playing && m_screenShakeEnabled) ? m_shakeOffsetY : 0.0f;

        const SDL_Rect worldViewport{
            static_cast<int>(shakeX),
            static_cast<int>(shakeY),
            LOGICAL_WIDTH,
            LOGICAL_HEIGHT
        };
        SDL_RenderSetViewport(m_renderer, &worldViewport);

        // Fondo de Parallax Scrolling Dinámico
        m_levelManager.renderParallaxBackground(m_renderer);

        // Suelo Táctico y Marcas de Terreno en Avance
        m_levelManager.renderGround(m_renderer);

        // Entidades del juego
        m_tank.render(m_renderer);
        m_powerUpPool.render(m_renderer);
        m_enemyPool.render(m_renderer);
        m_boss.render(m_renderer);
        m_projectilePool.render(m_renderer);
        m_particlePool.render(m_renderer);

        // Restaurar Viewport para la capa HUD y Overlays
        const SDL_Rect hudViewport{0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT};
        SDL_RenderSetViewport(m_renderer, &hudViewport);

        // Destello visual expansivo de Bomba Nuclear (Nuke)
        if (m_nukeFlashTimer > 0.0f) {
            const float alphaRatio = m_nukeFlashTimer / 0.40f;
            const Uint8 alpha = static_cast<Uint8>(std::clamp(alphaRatio * 255.0f, 0.0f, 255.0f));
            const SDL_Rect logicalBounds{0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT};
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, alpha);
            SDL_RenderFillRect(m_renderer, &logicalBounds);
        }

        // Destello de 1-UP
        if (m_oneUpFlashTimer > 0.0f) {
            const float alphaRatio = m_oneUpFlashTimer / 1.0f;
            const Uint8 alpha = static_cast<Uint8>(std::clamp(alphaRatio * 110.0f, 0.0f, 110.0f));
            const SDL_Rect logicalBounds{0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT};
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(m_renderer, 80, 255, 120, alpha);
            SDL_RenderFillRect(m_renderer, &logicalBounds);
        }

        // Capa de HUD Táctico
        renderHUD();

        // Renderizado según sub-estado
        if (m_gameState == GameState::Playing) {
            renderPauseButton();
            m_inputManager.renderTouchUI(m_renderer);
        } else if (m_gameState == GameState::Paused) {
            renderPauseScreen();
        } else if (m_gameState == GameState::Settings) {
            renderSettingsScreen();
        } else if (m_gameState == GameState::LevelSummary) {
            renderLevelSummaryScreen();
        } else if (m_gameState == GameState::Armory) {
            renderArmoryScreen();
        } else if (m_gameState == GameState::GameOver) {
            renderGameOverScreen();
        } else if (m_gameState == GameState::Victory) {
            renderCampaignVictoryScreen();
        }
        break;
    }
    }

    // Presentar frame
    SDL_RenderPresent(m_renderer);
}

void Engine::run() {
    const uint64_t performanceFrequency{SDL_GetPerformanceFrequency()};
    uint64_t lastCounter{SDL_GetPerformanceCounter()};
    float accumulator{0.0f};

    while (m_isRunning) {
        m_inputManager.beginFrame();
        processEvents();

        const uint64_t currentCounter{SDL_GetPerformanceCounter()};
        float frameTime{static_cast<float>(currentCounter - lastCounter) / static_cast<float>(performanceFrequency)};
        lastCounter = currentCounter;

        if (frameTime > MAX_FRAME_TIME) {
            frameTime = MAX_FRAME_TIME;
        }

        accumulator += frameTime;

        while (accumulator >= FIXED_DELTA_TIME) {
            update(FIXED_DELTA_TIME);
            accumulator -= FIXED_DELTA_TIME;
        }

        render();
    }
}

} // namespace Core
