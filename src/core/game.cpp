#include "core/game.hpp"
#include "ai/enemy_ai.hpp"
#include "world/collision.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>

namespace {
Direction directionFromVelocity(sf::Vector2f velocity) noexcept
{
    if(std::abs(velocity.x)>std::abs(velocity.y))
    {
        if(velocity.x<0.f)
            return Direction::Left;
        return Direction::Right;
    }
    if(velocity.y>0.f)
        return Direction::Down;
    return Direction::Up;
}

float speedFromVelocity(sf::Vector2f velocity) noexcept
{
    const float speed=std::sqrt(velocity.x*velocity.x+velocity.y*velocity.y);
    if(speed>0.f)
        return speed;
    return prototype::PlayerBulletSpeed;
}

void addAliveEnemyBlocks(std::vector<sf::FloatRect>& blocks,const std::vector<EnemyTank>& enemies)
{
    for(const auto& enemy:enemies)
    {
        if(enemy.isAlive())
            blocks.push_back(enemy.getBounds());
    }
}

void addAlivePlayerBlock(std::vector<sf::FloatRect>& blocks,const std::optional<PlayerTank>& player)
{
    if(player && player->isAlive())
        blocks.push_back(player->getBounds());
}

void addAliveBaseBlock(std::vector<sf::FloatRect>& blocks,const std::optional<Base>& base)
{
    if(base && base->isAlive())
        blocks.push_back(base->getBounds());
}

void addItemOccupiedBlocks(
    std::vector<sf::FloatRect>& blocks,
    const std::optional<PlayerTank>& playerOne,
    const std::optional<PlayerTank>& playerTwo,
    const std::optional<Base>& base,
    const std::vector<EnemyTank>& enemies
)
{
    addAlivePlayerBlock(blocks,playerOne);
    addAlivePlayerBlock(blocks,playerTwo);
    addAliveBaseBlock(blocks,base);
    addAliveEnemyBlocks(blocks,enemies);
}

void updatePlayerByInput(PlayerTank& player,float deltaTime,std::vector<Bullet>& bullets,const Map& map,const std::vector<sf::FloatRect>& blocks,const PlayerInputState& input)
{
    player.updateWithInput(deltaTime,bullets,map,blocks,input.moveUp,input.moveDown,input.moveLeft,input.moveRight,input.fire);
}

void applyTankCommand(EnemyTank& enemy,const TankCommand& command,std::vector<Bullet>& bullets)
{
    if(command.move)
        enemy.setMoveDirection(command.direction);
    else
    {
        enemy.setDirection(command.direction);
        enemy.stopMoving();
    }

    if(command.fire)
        enemy.tryFire(bullets);
}
}

Game::Game()
    : window_(sf::VideoMode({static_cast<unsigned int>(prototype::WindowWidth),
                             static_cast<unsigned int>(prototype::WindowHeight)}),
      "Retro Tank Battle"),
      map_(),
      itemSpawner_(map_),
      networkSession_(),
      hostDiscovery_(),
      mainMenu_(),
      playSetupMenu_(),
      hostSetupMenu_("assets/maps/coop"),
      joinMenu_(),
      pauseMenu_(),
      settingsMenu_(),
      resultMenu_(),
      hud_(),
      playerOne_(),
      playerTwo_(),
      base_(),
      enemies_(),
      enemyAis_(),
      bullets_(),
      items_(),
      localInput_(),
      remoteInput_(),
      hasTwoPlayers_(false),
      currentConfig_(),
      status_(GameStatus::MainMenu)
{
    window_.setFramerateLimit(60);
    playSetupMenu_.setTitle("Single Player","Configure map and enemy AI difficulty");
    hostSetupMenu_.setTitle("Host Multiplayer","Choose a cooperative map and AI difficulty");
}

void Game::initializeWorld(const GameStartConfig& config)
{
    currentConfig_=config;
    loadedMapPath_=config.mapPath;
    destroyedBricks_.clear();
    map_.clear();
    playerOne_.reset();
    playerTwo_.reset();
    base_.reset();
    enemies_.clear();
    enemyAis_.clear();
    bullets_.clear();
    items_.clear();
    localInput_={};
    remoteInput_={};
    hasTwoPlayers_=false;
    itemSpawner_.reset();

    const bool mapLoaded=map_.loadFromFile(config.mapPath);

    sf::Vector2f playerPosition(400.f,500.f);
    if(mapLoaded && map_.hasPlayerSpawn()) playerPosition=map_.playerSpawn();
    playerOne_.emplace(playerPosition.x,playerPosition.y);

    sf::Vector2f basePosition(400.f,550.f);
    if(mapLoaded && map_.hasBasePosition()) basePosition=map_.basePosition();
    base_.emplace(basePosition.x,basePosition.y);

    if(mapLoaded)
    {
        for(const EnemySpawn& spawn:map_.enemySpawns())
        {
            addEnemy(
                spawn.position.x,
                spawn.position.y,
                spawn.type,
                config.aiDifficulty
            );
        }
    }

    status_=GameStatus::Running;
    window_.setTitle("Retro Tank Battle");
}

bool Game::startHost(unsigned short port)
{
    if(port==0)
        return false;

    if(!networkSession_.startHost(port))
        return false;

    hostDiscovery_.beginAdvertising(port);
    return true;
}

bool Game::joinHost(const std::string& address,unsigned short port)
{
    if(address.empty()||port==0)
        return false;

    return networkSession_.connectToHost(address,port);
}
void Game::leaveNetworkGame()
{
    networkSession_.disconnect();
    hostDiscovery_.stopAdvertising();
    hostDiscovery_.stopListening();
}

void Game::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        const float deltaTime = clock.restart().asSeconds();
        processEvents();
        hostDiscovery_.update(deltaTime);
        if(status_==GameStatus::JoinSetup)
            joinMenu_.setHosts(hostDiscovery_.hosts());
        networkSession_.update();
        update(deltaTime);
        render();
    }
}

void Game::processEvents()
{
    while(const std::optional event=window_.pollEvent())
    {
        if(event->is<sf::Event::Closed>())
        {
            window_.close();
            continue;
        }

        if(status_==GameStatus::MainMenu)
            handleMenuAction(mainMenu_.handleEvent(*event).action);
        else if(status_==GameStatus::PlaySetup)
            handleMenuAction(playSetupMenu_.handleEvent(*event).action);
        else if(status_==GameStatus::HostSetup)
            handleMenuAction(hostSetupMenu_.handleEvent(*event).action);
        else if(status_==GameStatus::JoinSetup)
            handleMenuAction(joinMenu_.handleEvent(*event).action);
        else if(status_==GameStatus::Settings)
            handleMenuAction(settingsMenu_.handleEvent(*event).action);
        else if(status_==GameStatus::Paused)
            handleMenuAction(pauseMenu_.handleEvent(*event).action);
        else if(status_==GameStatus::Victory || status_==GameStatus::Defeat)
            handleMenuAction(resultMenu_.handleEvent(*event).action);
        else if(status_==GameStatus::Running)
        {
            if(const auto* keyPressed=event->getIf<sf::Event::KeyPressed>())
            {
                if(keyPressed->code==sf::Keyboard::Key::Escape &&
                   networkSession_.isOffline())
                {
                    status_=GameStatus::Paused;
                }
            }
        }
    }
}

void Game::handleMenuAction(MenuAction action)
{
    if(action==MenuAction::None)
        return;

    if(action==MenuAction::Play)
        status_=GameStatus::PlaySetup;
    else if(action==MenuAction::HostMultiplayer)
        status_=GameStatus::HostSetup;
    else if(action==MenuAction::JoinMultiplayer)
    {
        hostDiscovery_.beginListening();
        status_=GameStatus::JoinSetup;
    }
    else if(action==MenuAction::OpenSettings)
    {
        if(status_==GameStatus::Paused)
            settingsMenu_.setReturnTarget(SettingsReturnTarget::PauseMenu);
        else
            settingsMenu_.setReturnTarget(SettingsReturnTarget::MainMenu);

        status_=GameStatus::Settings;
    }
    else if(action==MenuAction::Exit)
        window_.close();
    else if(action==MenuAction::StartGame&&status_==GameStatus::HostSetup)
    {
        constexpr unsigned short GamePort=54118;

        std::cout
            <<"[Game] Starting host on port "
            <<GamePort
            <<'\n';

        if(startHost(GamePort))
        {
            initializeWorld(hostSetupMenu_.config());
            std::cout<<"[Game] Host started successfully\n";
        }
        else
        {
            std::cerr<<"[Game] Host startup failed\n";
            status_=GameStatus::HostSetup;
        }
    }
    else if(action==MenuAction::StartGame)
        initializeWorld(playSetupMenu_.config());
    else if(action==MenuAction::StartHostGame)
    {
        constexpr unsigned short GamePort=54118;

        std::cout
            <<"[Game] Starting host on port "
            <<GamePort
            <<'\n';

        if(startHost(GamePort))
        {
            initializeWorld(hostSetupMenu_.config());
            std::cout<<"[Game] Host started successfully\n";
        }
        else
        {
            std::cerr<<"[Game] Host startup failed\n";
            status_=GameStatus::HostSetup;
        }
    }
    else if(action==MenuAction::JoinSelectedHost)
    {
        const std::string address=
            joinMenu_.selectedAddress();

        const unsigned short port=
            joinMenu_.selectedPort();

        std::cout
            <<"[Game] Joining "
            <<address
            <<':'
            <<port
            <<'\n';

        hostDiscovery_.stopListening();

        if(joinHost(address,port))
        {
            initializeWorld(hostSetupMenu_.config());
            std::cout<<"[Game] TCP connection succeeded\n";
        }
        else
        {
            std::cerr<<"[Game] TCP connection failed\n";

            hostDiscovery_.beginListening();
            status_=GameStatus::JoinSetup;
        }
    }
    else if(action==MenuAction::Back)
    {
        if(status_==GameStatus::PlaySetup||status_==GameStatus::HostSetup||status_==GameStatus::JoinSetup)
        {
            if(status_==GameStatus::JoinSetup)
                hostDiscovery_.stopListening();

            status_=GameStatus::MainMenu;
        }
        else if(status_==GameStatus::Settings)
        {
            if(settingsMenu_.returnTarget()==SettingsReturnTarget::PauseMenu)
            {
                status_=GameStatus::Paused;
            }
            else
                status_=GameStatus::MainMenu;
        }
    }
    else if(action==MenuAction::ContinueGame)
        status_=GameStatus::Running;
    else if(action==MenuAction::RestartGame)
        initializeWorld(currentConfig_);
    else if(action==MenuAction::QuitToMainMenu)
    {
        leaveNetworkGame();
        status_=GameStatus::MainMenu;
    }
}

void Game::update(float deltaTime)
{
    collectLocalInput(localInput_);

    if(networkSession_.isOffline())
        updateOffline(deltaTime);
    else if(networkSession_.isHost())
        updateHost(deltaTime);
    else if(networkSession_.isClient())
        updateClient(deltaTime);
}

void Game::updateOffline(float deltaTime)
{
    if (status_ != GameStatus::Running || !playerOne_ || !base_) {
        return;
    }

    std::vector<sf::FloatRect> playerTankBlocks;
    if(base_->isAlive())
        playerTankBlocks.push_back(base_->getBounds());
    addAliveEnemyBlocks(playerTankBlocks,enemies_);
    playerOne_->update(deltaTime,bullets_,map_,playerTankBlocks);

    std::vector<PlayerTank> aiPlayers;
    if(playerOne_)
        aiPlayers.push_back(*playerOne_);
    AIContext aiContext{map_,aiPlayers,*base_,enemies_,bullets_};

    for (std::size_t i=0;i<enemies_.size();++i) {
        auto& enemy=enemies_[i];
        std::vector<sf::FloatRect> enemyTankBlocks;
        if(playerOne_->isAlive())
            enemyTankBlocks.push_back(playerOne_->getBounds());
        if(base_->isAlive())
            enemyTankBlocks.push_back(base_->getBounds());
        for(const auto& other:enemies_)
        {
            if(other.getId()!=enemy.getId() && other.isAlive())
                enemyTankBlocks.push_back(other.getBounds());
        }
        const TankCommand command=enemyAis_[i]->decide(enemy,aiContext,deltaTime);
        applyTankCommand(enemy,command,bullets_);
        enemy.update(deltaTime,bullets_,map_,enemyTankBlocks);
    }

    std::vector<sf::FloatRect> itemOccupiedBlocks;
    addItemOccupiedBlocks(
        itemOccupiedBlocks,
        playerOne_,
        playerTwo_,
        base_,
        enemies_
    );
    itemSpawner_.update(deltaTime,items_,itemOccupiedBlocks);

    for (auto& item : items_) {
        if (item.isAlive() && playerOne_->getBounds().findIntersection(item.getBounds()).has_value()) {
            playerOne_->collectItem(item);
        }
    }

    for (auto bullet = bullets_.begin(); bullet != bullets_.end();) {
        bullet->update(deltaTime);
        bool hit = false;

        int hitTileX=0;
        int hitTileY=0;
        if(Collision::findBulletHitTile(map_,bullet->getBounds(),hitTileX,hitTileY))
        {
            const Tile* tile=map_.tileAt(hitTileX,hitTileY);
            if(tile!=nullptr && tile->type()==TileType::Brick)
            {
                map_.setTile(hitTileX,hitTileY,TileType::Empty);
                destroyedBricks_.insert({hitTileX,hitTileY});
            }
            hit=true;
        }
        else if(Collision::hitsMapBounds(map_,bullet->getBounds()))
            hit=true;

        if (!hit && bullet->getFaction() == Faction::Player) {
            for (auto& enemy : enemies_) {
                if (enemy.isAlive() && bullet->getBounds().findIntersection(enemy.getBounds()).has_value()) {
                    enemy.takeDamage(bullet->getDamage());
                    hit = true;
                    break;
                }
            }
        } else if (!hit && bullet->getFaction() == Faction::Enemy) {
            if (playerOne_->isAlive() && bullet->getBounds().findIntersection(playerOne_->getBounds()).has_value()) {
                playerOne_->takeDamage(bullet->getDamage());
                hit = true;
            } else if (base_->isAlive() && bullet->getBounds().findIntersection(base_->getBounds()).has_value()) {
                base_->takeDamage(bullet->getDamage());
                hit = true;
            }
        }

        if (hit || !bullet->isAlive()) {
            bullet = bullets_.erase(bullet);
        } else {
            ++bullet;
        }
    }

    removeDeadEnemies();

    for (auto item = items_.begin(); item != items_.end();) {
        if (!item->isAlive()) {
            item = items_.erase(item);
        } else {
            ++item;
        }
    }

    updateStatus();
}

void Game::updateHost(float deltaTime)
{
    
    if(status_!=GameStatus::Running || !playerOne_ || !base_)
        return;

    if(!hasTwoPlayers_)
    {
        localInput_={};
        remoteInput_={};

        if(!networkSession_.pollWelcome())
            return;

        sf::Vector2f position=playerOne_->getPosition();

        if(map_.hasSecondPlayerSpawn())
            position=map_.secondPlayerSpawn();
        else
            position.x-=60.f;

        playerTwo_.emplace(position.x,position.y);
        playerTwo_->setColor(sf::Color::Cyan);

        hasTwoPlayers_=true;

        hostDiscovery_.stopAdvertising();

        networkSession_.sendSnapshot(createSnapshot());

        return;
    }

PlayerInputState remoteInput;

if(networkSession_.pollRemoteInput(remoteInput))
    remoteInput_=remoteInput;

    std::vector<sf::FloatRect> playerOneBlocks;
    if(base_->isAlive())
        playerOneBlocks.push_back(base_->getBounds());
    addAliveEnemyBlocks(playerOneBlocks,enemies_);
    if(playerTwo_ && playerTwo_->isAlive())
        playerOneBlocks.push_back(playerTwo_->getBounds());
    updatePlayerByInput(*playerOne_,deltaTime,bullets_,map_,playerOneBlocks,localInput_);

    if(playerTwo_)
    {
        std::vector<sf::FloatRect> playerTwoBlocks;
        if(base_->isAlive())
            playerTwoBlocks.push_back(base_->getBounds());
        addAliveEnemyBlocks(playerTwoBlocks,enemies_);
        if(playerOne_->isAlive())
            playerTwoBlocks.push_back(playerOne_->getBounds());
        updatePlayerByInput(*playerTwo_,deltaTime,bullets_,map_,playerTwoBlocks,remoteInput_);
    }

    std::vector<PlayerTank> aiPlayers;
    if(playerOne_)
        aiPlayers.push_back(*playerOne_);
    if(playerTwo_)
        aiPlayers.push_back(*playerTwo_);
    AIContext aiContext{map_,aiPlayers,*base_,enemies_,bullets_};

    for(std::size_t i=0;i<enemies_.size();++i)
    {
        auto& enemy=enemies_[i];
        std::vector<sf::FloatRect> enemyTankBlocks;
        if(playerOne_->isAlive())
            enemyTankBlocks.push_back(playerOne_->getBounds());
        if(playerTwo_ && playerTwo_->isAlive())
            enemyTankBlocks.push_back(playerTwo_->getBounds());
        if(base_->isAlive())
            enemyTankBlocks.push_back(base_->getBounds());
        for(const auto& other:enemies_)
        {
            if(other.getId()!=enemy.getId() && other.isAlive())
                enemyTankBlocks.push_back(other.getBounds());
        }
        const TankCommand command=enemyAis_[i]->decide(enemy,aiContext,deltaTime);
        applyTankCommand(enemy,command,bullets_);
        enemy.update(deltaTime,bullets_,map_,enemyTankBlocks);
    }

    std::vector<sf::FloatRect> itemOccupiedBlocks;
    addItemOccupiedBlocks(
        itemOccupiedBlocks,
        playerOne_,
        playerTwo_,
        base_,
        enemies_
    );
    itemSpawner_.update(deltaTime,items_,itemOccupiedBlocks);

    for(auto& item:items_)
    {
        if(!item.isAlive())
            continue;
        bool picked=false;
        if(playerOne_->isAlive() && playerOne_->getBounds().findIntersection(item.getBounds()).has_value())
        {
            playerOne_->collectItem(item);
            picked=true;
        }
        if(!picked && playerTwo_ && playerTwo_->isAlive() && playerTwo_->getBounds().findIntersection(item.getBounds()).has_value())
            playerTwo_->collectItem(item);
    }

    for(auto bullet=bullets_.begin();bullet!=bullets_.end();)
    {
        bullet->update(deltaTime);
        bool hit=false;

        int hitTileX=0;
        int hitTileY=0;
        if(Collision::findBulletHitTile(map_,bullet->getBounds(),hitTileX,hitTileY))
        {
            const Tile* tile=map_.tileAt(hitTileX,hitTileY);
            if(tile!=nullptr && tile->type()==TileType::Brick)
            {
                map_.setTile(hitTileX,hitTileY,TileType::Empty);
                destroyedBricks_.insert({hitTileX,hitTileY});
                networkSession_.sendTileChanged(hitTileX,hitTileY,static_cast<int>(TileType::Empty));
            }
            hit=true;
        }
        else if(Collision::hitsMapBounds(map_,bullet->getBounds()))
            hit=true;

        if(!hit && bullet->getFaction()==Faction::Player)
        {
            for(auto& enemy:enemies_)
            {
                if(enemy.isAlive() && bullet->getBounds().findIntersection(enemy.getBounds()).has_value())
                {
                    enemy.takeDamage(bullet->getDamage());
                    hit=true;
                    break;
                }
            }
        }
        else if(!hit && bullet->getFaction()==Faction::Enemy)
        {
            if(playerOne_->isAlive() && bullet->getBounds().findIntersection(playerOne_->getBounds()).has_value())
            {
                playerOne_->takeDamage(bullet->getDamage());
                hit=true;
            }
            else if(playerTwo_ && playerTwo_->isAlive() && bullet->getBounds().findIntersection(playerTwo_->getBounds()).has_value())
            {
                playerTwo_->takeDamage(bullet->getDamage());
                hit=true;
            }
            else if(base_->isAlive() && bullet->getBounds().findIntersection(base_->getBounds()).has_value())
            {
                base_->takeDamage(bullet->getDamage());
                hit=true;
            }
        }

        if(hit || !bullet->isAlive())
            bullet=bullets_.erase(bullet);
        else
            ++bullet;
    }

    removeDeadEnemies();

    for(auto item=items_.begin();item!=items_.end();)
    {
        if(!item->isAlive())
            item=items_.erase(item);
        else
            ++item;
    }

    updateStatus();
    networkSession_.sendSnapshot(createSnapshot());
}

void Game::updateClient(float)
{
    GameSnapshot snapshot;

    if(networkSession_.pollSnapshot(snapshot))
        applySnapshot(snapshot);

    int tileX=0;
    int tileY=0;
    int tileType=0;

    while(networkSession_.pollTileChanged(
        tileX,
        tileY,
        tileType
    ))
    {
        map_.setTile(
            tileX,
            tileY,
            static_cast<TileType>(tileType)
        );
    }

    networkSession_.sendInput(localInput_);
}

void Game::collectLocalInput(PlayerInputState& input)
{
    input.moveUp=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
    input.moveDown=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    input.moveLeft=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    input.moveRight=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    input.fire=sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
}

void Game::applySnapshot(const GameSnapshot& snapshot)
{
    if(!snapshot.mapPath.empty() && snapshot.mapPath!=loadedMapPath_)
    {
        if(!map_.loadFromFile(snapshot.mapPath))
        {
            std::cerr << "[Game] Failed to load network map: " << snapshot.mapPath <<'\n';
            return;
        }
        loadedMapPath_=snapshot.mapPath;
        currentConfig_.mapPath=snapshot.mapPath;
        sf::Vector2f basePosition{400.f,550.f};
        if(map_.hasBasePosition()) basePosition=map_.basePosition();
        base_.emplace(basePosition.x,basePosition.y);
    }

    destroyedBricks_.clear();

    for(const auto& position:snapshot.destroyedBricks)
    {
        destroyedBricks_.insert(position);
        map_.setTile(position.first,position.second,TileType::Empty);
    }

    status_=static_cast<GameStatus>(snapshot.status);

    if(!playerOne_)
        playerOne_.emplace(snapshot.playerOne.x,snapshot.playerOne.y);
    playerOne_->setPosition({snapshot.playerOne.x,snapshot.playerOne.y});
    playerOne_->setDirection(static_cast<Direction>(snapshot.playerOne.direction));
    playerOne_->setHp(snapshot.playerOne.hp);

    if(snapshot.playerTwo.alive && !playerTwo_)
    {
        playerTwo_.emplace(snapshot.playerTwo.x,snapshot.playerTwo.y);
        playerTwo_->setColor(sf::Color::Cyan);
        hasTwoPlayers_=true;
    }
    if(playerTwo_)
    {
        playerTwo_->setPosition({snapshot.playerTwo.x,snapshot.playerTwo.y});
        playerTwo_->setDirection(static_cast<Direction>(snapshot.playerTwo.direction));
        playerTwo_->setHp(snapshot.playerTwo.hp);
    }

    enemies_.clear();
    enemyAis_.clear();
    for(const auto& enemySnapshot:snapshot.enemies)
    {
        const EnemyType type=
        static_cast<EnemyType>(enemySnapshot.enemyType);

        addEnemy(enemySnapshot.x,enemySnapshot.y,type,currentConfig_.aiDifficulty);
        enemies_.back().setDirection(static_cast<Direction>(enemySnapshot.direction));
        enemies_.back().setHp(enemySnapshot.hp);
    }

    bullets_.clear();
    for(const auto& bulletSnapshot:snapshot.bullets)
    {
        if(!bulletSnapshot.alive)
            continue;
        const sf::Vector2f velocity{bulletSnapshot.dirX,bulletSnapshot.dirY};
        const Direction direction=directionFromVelocity(velocity);
        const float speed=speedFromVelocity(velocity);
        bullets_.emplace_back(sf::Vector2f{bulletSnapshot.x,bulletSnapshot.y},direction,static_cast<Faction>(bulletSnapshot.faction),bulletSnapshot.damage,speed,bulletSnapshot.ownerId);
    }

    items_.clear();
    for(const auto& itemSnapshot:snapshot.items)
    {
        if(itemSnapshot.alive)
            items_.emplace_back(sf::Vector2f{itemSnapshot.x,itemSnapshot.y},static_cast<ItemType>(itemSnapshot.type));
    }

    if(base_)
        base_->setHp(snapshot.baseHp);
}

GameSnapshot Game::createSnapshot() const
{
    GameSnapshot snapshot;

    snapshot.mapPath=currentConfig_.mapPath;
    snapshot.status=static_cast<int>(status_);

    if(playerOne_)
    {
        snapshot.playerOne.id=playerOne_->getId();
        snapshot.playerOne.x=playerOne_->getPosition().x;
        snapshot.playerOne.y=playerOne_->getPosition().y;
        snapshot.playerOne.direction=static_cast<int>(playerOne_->getDirection());
        snapshot.playerOne.hp=playerOne_->getHp();
        snapshot.playerOne.alive=playerOne_->isAlive();
        snapshot.playerOne.enemyType=-1;
    }

    if(playerTwo_)
    {
        snapshot.playerTwo.id=playerTwo_->getId();
        snapshot.playerTwo.x=playerTwo_->getPosition().x;
        snapshot.playerTwo.y=playerTwo_->getPosition().y;
        snapshot.playerTwo.direction=static_cast<int>(playerTwo_->getDirection());
        snapshot.playerTwo.hp=playerTwo_->getHp();
        snapshot.playerTwo.alive=playerTwo_->isAlive();
        snapshot.playerTwo.enemyType=-1;
    }

    for(const auto& enemy:enemies_)
    {
        TankSnapshot enemySnapshot;
        enemySnapshot.id=enemy.getId();
        enemySnapshot.x=enemy.getPosition().x;
        enemySnapshot.y=enemy.getPosition().y;
        enemySnapshot.direction=static_cast<int>(enemy.getDirection());
        enemySnapshot.hp=enemy.getHp();
        enemySnapshot.alive=enemy.isAlive();
        enemySnapshot.enemyType=static_cast<int>(enemy.getType());
        snapshot.enemies.push_back(enemySnapshot);
    }

    for(const auto& bullet:bullets_)
    {
        if(!bullet.isAlive())
            continue;
        const sf::Vector2f velocity=bullet.getVelocity();
        BulletSnapshot bulletSnapshot;
        bulletSnapshot.x=bullet.getPosition().x;
        bulletSnapshot.y=bullet.getPosition().y;
        bulletSnapshot.faction=static_cast<int>(bullet.getFaction());
        bulletSnapshot.damage=bullet.getDamage();
        bulletSnapshot.ownerId=bullet.getOwnerId();
        bulletSnapshot.alive=true;
        bulletSnapshot.dirX=velocity.x;
        bulletSnapshot.dirY=velocity.y;
        snapshot.bullets.push_back(bulletSnapshot);
    }

    for(const auto& item:items_)
    {
        ItemSnapshot itemSnapshot;
        const sf::FloatRect bounds=item.getBounds();
        itemSnapshot.x=bounds.position.x+bounds.size.x/2.f;
        itemSnapshot.y=bounds.position.y+bounds.size.y/2.f;
        itemSnapshot.type=static_cast<int>(item.getType());
        itemSnapshot.alive=item.isAlive();
        snapshot.items.push_back(itemSnapshot);
    }

    if(base_)
    {
        snapshot.baseHp=base_->getHp();
        snapshot.baseAlive=base_->isAlive();
    }

    snapshot.destroyedBricks.assign(destroyedBricks_.begin(),destroyedBricks_.end());

    return snapshot;

}

void Game::addEnemy(float x,float y,EnemyType type,AIDifficulty difficulty)
{
    enemies_.emplace_back(x,y,type);
    const std::uint32_t seed=static_cast<std::uint32_t>(enemies_.back().getId()*9973+enemyAis_.size()+1);
    if(difficulty==AIDifficulty::Easy)
        enemyAis_.push_back(std::make_unique<EasyAI>(seed));
    else if(difficulty==AIDifficulty::Normal)
        enemyAis_.push_back(std::make_unique<NormalAI>(seed));
    else
        enemyAis_.push_back(std::make_unique<HardAI>(seed));
}

void Game::removeDeadEnemies()
{
    for(std::size_t i=0;i<enemies_.size();)
    {
        if(!enemies_[i].isAlive())
        {
            enemies_.erase(enemies_.begin()+static_cast<std::ptrdiff_t>(i));
            if(i<enemyAis_.size())
                enemyAis_.erase(enemyAis_.begin()+static_cast<std::ptrdiff_t>(i));
        }
        else
            ++i;
    }

    while(enemyAis_.size()>enemies_.size())
        enemyAis_.pop_back();
}

void Game::render()
{
    window_.clear(sf::Color::Black);

    if(status_==GameStatus::MainMenu)
    {
        mainMenu_.draw(window_);
        window_.display();
        return;
    }

    if(status_==GameStatus::PlaySetup)
    {
        playSetupMenu_.draw(window_);
        window_.display();
        return;
    }

    if(status_==GameStatus::HostSetup)
    {
        hostSetupMenu_.draw(window_);
        window_.display();
        return;
    }

    if(status_==GameStatus::JoinSetup)
    {
        joinMenu_.draw(window_);
        window_.display();
        return;
    }

    if(status_==GameStatus::Settings &&
       settingsMenu_.returnTarget()==SettingsReturnTarget::MainMenu)
    {
        settingsMenu_.draw(window_);
        window_.display();
        return;
    }

    map_.drawLayer(window_,0);
    if(base_) base_->draw(window_);
    for (const auto& item : items_) {
        item.draw(window_);
    }

    if(playerOne_) playerOne_->draw(window_);
    if(playerTwo_) playerTwo_->draw(window_);
    for (const auto& enemy : enemies_) {
        enemy.draw(window_);
    }

    for (const auto& bullet : bullets_) {
        bullet.draw(window_);
    }

    map_.drawLayer(window_,1);

    hud_.draw(
        window_,
        playerOne_ ? &*playerOne_ : nullptr,
        playerTwo_ ? &*playerTwo_ : nullptr,
        networkSession_.mode()
    );

    if(status_==GameStatus::Paused)
        pauseMenu_.draw(window_);
    else if(status_==GameStatus::Settings)
        settingsMenu_.draw(window_);
    else if(status_==GameStatus::Victory || status_==GameStatus::Defeat)
    {
        resultMenu_.setResult(status_);
        resultMenu_.draw(window_);
    }

    window_.display();
}

void Game::updateStatus() {
    if(!playerOne_ || !base_) return;

    bool playersDefeated=!playerOne_->isAlive();
    if(playerTwo_)
        playersDefeated=!playerOne_->isAlive() && !playerTwo_->isAlive();

    if (!base_->isAlive() || playersDefeated) {
        status_ = GameStatus::Defeat;
        window_.setTitle("Retro Tank Battle - Defeat");
    } else if (enemies_.empty()) {
        status_ = GameStatus::Victory;
        window_.setTitle("Retro Tank Battle - Victory");
    }
}
