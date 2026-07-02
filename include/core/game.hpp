#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ai/ai_controller.hpp"
#include "core/game_status.hpp"
#include "entity/base.hpp"
#include "entity/bullet.hpp"
#include "entity/enemy_tank.hpp"
#include "entity/item.hpp"
#include "entity/player_tank.hpp"
#include "network/network_session.hpp"
#include "ui/hud.hpp"
#include "ui/join_menu.hpp"
#include "ui/main_menu.hpp"
#include "ui/pause_menu.hpp"
#include "ui/play_setup_menu.hpp"
#include "ui/result_menu.hpp"
#include "ui/settings_menu.hpp"
#include "world/item_spawner.hpp"
#include "world/map.hpp"

class Game {
public:
    Game();
    void run();
    void startHost(unsigned short port);
    void joinHost(const std::string& address,unsigned short port);
    void leaveNetworkGame();

private:
    void initializeWorld(const GameStartConfig& config);
    void processEvents();
    void handleMenuAction(MenuAction action);
    void update(float deltaTime);
    void updateOffline(float deltaTime);
    void updateHost(float deltaTime);
    void updateClient(float deltaTime);
    void render();
    void updateStatus();
    void collectLocalInput(PlayerInputState& input);
    void applySnapshot(const GameSnapshot& snapshot);
    GameSnapshot createSnapshot() const;
    void addEnemy(float x,float y,EnemyType type,AIDifficulty difficulty);
    void removeDeadEnemies();

    sf::RenderWindow window_;
    Map map_;
    ItemSpawner itemSpawner_;
    NetworkSession networkSession_;
    MainMenu mainMenu_;
    PlaySetupMenu playSetupMenu_;
    PlaySetupMenu hostSetupMenu_;
    JoinMenu joinMenu_;
    PauseMenu pauseMenu_;
    SettingsMenu settingsMenu_;
    ResultMenu resultMenu_;
    Hud hud_;
    std::optional<PlayerTank> playerOne_;
    std::optional<PlayerTank> playerTwo_;
    std::optional<Base> base_;
    std::vector<EnemyTank> enemies_;
    std::vector<std::unique_ptr<AIController>> enemyAis_;
    std::vector<Bullet> bullets_;
    std::vector<Item> items_;
    PlayerInputState localInput_;
    PlayerInputState remoteInput_;
    bool hasTwoPlayers_=false;
    GameStartConfig currentConfig_;
    GameStatus status_;
};
