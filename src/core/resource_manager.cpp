#include "core/resource_manager.hpp"

#include <algorithm>

ResourceManager& ResourceManager::instance()
{
    static ResourceManager manager;
    return manager;
}

const sf::Texture* ResourceManager::texture(TextureId id)
{
    auto found=textures_.find(id);
    if(found!=textures_.end())
        return &found->second;

    if(loadTexture(id,texturePath(id)))
        return &textures_.find(id)->second;

    return nullptr;
}

const sf::Texture* ResourceManager::tileTexture(TileType type)
{
    if(type==TileType::Brick)
        return texture(TextureId::Brick);
    if(type==TileType::Steel)
        return texture(TextureId::Steel);
    if(type==TileType::Water)
        return texture(TextureId::Water);
    if(type==TileType::Grass)
        return texture(TextureId::Grass);

    return nullptr;
}

const sf::Texture* ResourceManager::enemyTexture(EnemyType type)
{
    if(type==EnemyType::Light)
        return texture(TextureId::EnemyLight);

    return texture(TextureId::EnemyHeavy);
}

void ResourceManager::playSound(SoundId id)
{
    auto found=soundBuffers_.find(id);
    if(found==soundBuffers_.end())
    {
        if(!loadSound(id,soundPath(id)))
            return;

        found=soundBuffers_.find(id);
    }

    activeSounds_.emplace_back(found->second);
    activeSounds_.back().setVolume(volume_);
    activeSounds_.back().play();
}

void ResourceManager::playUiSound(UISoundEvent event)
{
    if(event==UISoundEvent::MenuMove)
        playSound(SoundId::MenuMove);
    else if(event==UISoundEvent::MenuConfirm)
        playSound(SoundId::MenuConfirm);
    else if(event==UISoundEvent::MenuBack)
        playSound(SoundId::MenuBack);
    else if(event==UISoundEvent::PauseOpen)
        playSound(SoundId::PauseOpen);
    else if(event==UISoundEvent::PauseClose)
        playSound(SoundId::PauseClose);
    else if(event==UISoundEvent::Victory)
        playSound(SoundId::Victory);
    else if(event==UISoundEvent::Defeat)
        playSound(SoundId::Defeat);
}

void ResourceManager::setVolume(float volume) noexcept
{
    volume_=std::clamp(volume,0.f,100.f);
    for(auto& sound:activeSounds_)
        sound.setVolume(volume_);
}

float ResourceManager::volume() const noexcept{return volume_;}

void ResourceManager::update()
{
    activeSounds_.erase(
        std::remove_if(
            activeSounds_.begin(),
            activeSounds_.end(),
            [](const sf::Sound& sound)
            {
                return sound.getStatus()==sf::Sound::Status::Stopped;
            }
        ),
        activeSounds_.end()
    );
}

bool ResourceManager::loadTexture(TextureId id,const char* path)
{
    if(path==nullptr)
        return false;

    sf::Texture texture;
    if(!texture.loadFromFile(path))
        return false;

    textures_.emplace(id,std::move(texture));
    return true;
}

bool ResourceManager::loadSound(SoundId id,const char* path)
{
    if(path==nullptr)
        return false;

    sf::SoundBuffer buffer;
    if(!buffer.loadFromFile(path))
        return false;

    soundBuffers_.emplace(id,std::move(buffer));
    return true;
}

const char* ResourceManager::texturePath(TextureId id) const noexcept
{
    if(id==TextureId::Brick)
        return "assets/textures/brick.png";
    if(id==TextureId::Steel)
        return "assets/textures/steel.png";
    if(id==TextureId::Water)
        return "assets/textures/water.png";
    if(id==TextureId::Grass)
        return "assets/textures/grass.png";
    if(id==TextureId::Base)
        return "assets/textures/base.png";
    if(id==TextureId::PlayerTank)
        return "assets/textures/player_tank.png";
    if(id==TextureId::EnemyLight)
        return "assets/textures/enemy_light.png";
    if(id==TextureId::EnemyHeavy)
        return "assets/textures/enemy_heavy.png";

    return nullptr;
}

const char* ResourceManager::soundPath(SoundId id) const noexcept
{
    if(id==SoundId::MenuMove)
        return "assets/sounds/menu_move.wav";
    if(id==SoundId::MenuConfirm)
        return "assets/sounds/menu_confirm.wav";
    if(id==SoundId::MenuBack)
        return "assets/sounds/menu_back.wav";
    if(id==SoundId::PauseOpen)
        return "assets/sounds/pause_open.wav";
    if(id==SoundId::PauseClose)
        return "assets/sounds/pause_close.wav";
    if(id==SoundId::Victory)
        return "assets/sounds/victory.wav";
    if(id==SoundId::Defeat)
        return "assets/sounds/defeat.wav";
    if(id==SoundId::TankFire)
        return "assets/sounds/tank_fire.wav";
    if(id==SoundId::TankHit)
        return "assets/sounds/tank_hit.wav";
    if(id==SoundId::TankDestroy)
        return "assets/sounds/tank_destroy.wav";
    if(id==SoundId::BulletHitBrick)
        return "assets/sounds/bullet_hit_brick.wav";
    if(id==SoundId::BulletHitSteel)
        return "assets/sounds/bullet_hit_steel.wav";
    if(id==SoundId::BaseHit)
        return "assets/sounds/base_hit.wav";
    if(id==SoundId::BaseDestroy)
        return "assets/sounds/base_destroy.wav";
    if(id==SoundId::ItemPickHealth)
        return "assets/sounds/item_pick_health.wav";
    if(id==SoundId::ItemPickPower)
        return "assets/sounds/item_pick_power.wav";

    return nullptr;
}
