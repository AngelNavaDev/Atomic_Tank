#pragma once

#include "input/InputManager.hpp"
#include "core/LevelManager.hpp"
#include "entities/Tank.hpp"
#include "entities/ProjectilePool.hpp"
#include "entities/EnemyPool.hpp"
#include "entities/ParticlePool.hpp"
#include "entities/PowerUpPool.hpp"
#include "entities/Boss.hpp"
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <cstdint>

namespace Core {

enum class GameState {
    Loading,
    MainMenu,
    LevelSelect,
    Playing,
    Paused,
    LevelSummary,
    Armory,
    Settings,
    Medals,
    GameOver,
    Victory
};

class Engine {
public:
    static constexpr int LOGICAL_WIDTH{1920};
    static constexpr int LOGICAL_HEIGHT{1080};
    static constexpr float FIXED_DELTA_TIME{1.0f / 60.0f};
    static constexpr float MAX_FRAME_TIME{0.25f};

    Engine() = default;
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool init(const char* title, int windowWidth, int windowHeight);
    void run();
    void shutdown();

private:
    void processEvents();
    void update(float dt);
    void resolveCollisions();
    void onTankDestroyed();
    void addScore(uint64_t points);
    void resetGame();
    void jumpToLevel(int level);
    void selectArmoryUpgrade(int option);
    void buyArmoryUpgrade(int upgradeIndex);

    void saveProgress();
    void loadProgress();

    void render();
    void renderHUD();
    void renderScore(SDL_Renderer* renderer, float centerX, float y, uint64_t score);
    void render7SegmentDigit(SDL_Renderer* renderer, float x, float y, int digit, SDL_Color litColor, SDL_Color unlitColor);
    void renderLoadingScreen();
    void renderMainMenuScreen();
    void renderLevelSelectScreen();
    void renderPauseScreen();
    void renderSettingsScreen();
    void renderPauseButton();
    void renderArcadeTitle(float centerX, float centerY);
    void renderGameOverScreen();
    void renderLevelSummaryScreen();
    void renderArmoryScreen();
    void renderCampaignVictoryScreen();
    void renderMedalsScreen();

    SDL_Window* m_window{nullptr};
    SDL_Renderer* m_renderer{nullptr};
    bool m_isRunning{false};

    // Máquina de estados del juego
    GameState m_gameState{GameState::Loading};
    GameState m_previousState{GameState::MainMenu};
    float m_loadingTimer{0.0f};

    // Progreso persistente y récord
    int m_maxUnlockedLevel{1};
    uint64_t m_highScore{0};
    uint64_t m_totalEnemiesKilled{0};
    uint32_t m_totalBossesDefeated{0};
    uint32_t m_totalNukesUsed{0};
    float m_bestSurvivalSeconds{0.0f};
    int m_totalUpgradesPurchased{0};
    bool m_campaignCompleted{false};

    // Modo Supervivencia (Survival Mode)
    bool m_isSurvivalMode{false};
    float m_survivalTimer{0.0f};
    float m_survivalBossTimer{60.0f};

    // Parámetros de Ajustes (Settings)
    int m_sfxVolume{80};
    int m_musicVolume{80};
    bool m_screenShakeEnabled{true};
    int m_touchOpacityLevel{1};

    // Índices de selección para navegación en menús (Gamepad / Teclado)
    int m_mainMenuSelectedIndex{0};
    int m_levelSelectSelectedIndex{0};
    int m_pauseSelectedIndex{0};
    int m_settingsSelectedIndex{0};
    int m_armorySelectedIndex{0};
    int m_medalsSelectedIndex{0};

    // Micro Hit-Stop
    float m_hitStopTimer{0.0f};

    // Estadísticas por Nivel y Moneda de Sesión
    int m_levelEnemiesKilled{0};
    int m_levelBombsIntercepted{0};
    uint64_t m_levelScoreEarned{0};
    int m_levelIntegrityBonus{0};
    int m_spendablePoints{0};

    // Sub-sistemas del motor
    Input::InputManager m_inputManager{};
    LevelManager m_levelManager{};

    // Entidades del juego
    Entities::Tank m_tank{};
    Entities::ProjectilePool m_projectilePool{};
    Entities::EnemyPool m_enemyPool{};
    Entities::ParticlePool m_particlePool{};
    Entities::PowerUpPool m_powerUpPool{};
    Entities::Boss m_boss{};

    // Generador automático de suministros (20s a 26s)
    float m_powerUpTimer{22.0f};

    // Pantalla de Armería entre niveles
    bool m_isArmoryOpen{false};

    // Pantalla de Victoria Final de Campaña (Nivel 25)
    bool m_isCampaignWon{false};

    // Vidas, Puntuación y Game Over
    int m_lives{3};
    uint64_t m_score{0};
    uint64_t m_nextLifeScore{100000};
    float m_oneUpFlashTimer{0.0f};

    bool m_isRespawning{false};
    float m_respawnTimer{0.0f};
    bool m_isGameOver{false};

    // Estado de juego, Nuke y Screen Shake
    int m_nukesAvailable{3};
    float m_nukeFlashTimer{0.0f};
    float m_screenShakeTrauma{0.0f};
    float m_shakeOffsetX{0.0f};
    float m_shakeOffsetY{0.0f};
    uint32_t m_shakeRngState{54321u};
};

} // namespace Core
