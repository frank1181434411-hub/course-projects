#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <map>
#include <vector>

#include "entity/enemy_type.hpp"
#include "ui/ui_types.hpp"
#include "world/tile.hpp"

enum class TextureId
{
    Brick,
    Steel,
    Water,
    Grass,
    Base,
    PlayerTank,
    EnemyLight,
    EnemyHeavy
};

enum class SoundId
{
    MenuMove,
    MenuConfirm,
    MenuBack,
    PauseOpen,
    PauseClose,
    Victory,
    Defeat,
    TankFire,
    TankHit,
    TankDestroy,
    BulletHitBrick,
    BulletHitSteel,
    BaseHit,
    BaseDestroy,
    ItemPickHealth,
    ItemPickPower
};

class ResourceManager
{
public:
    static ResourceManager& instance();

    const sf::Texture* texture(TextureId id);
    const sf::Texture* tileTexture(TileType type);
    const sf::Texture* enemyTexture(EnemyType type);

    void playSound(SoundId id);
    void playUiSound(UISoundEvent event);
    void setVolume(float volume) noexcept;
    float volume() const noexcept;
    void update();

private:
    ResourceManager()=default;

    bool loadTexture(TextureId id,const char* path);
    bool loadSound(SoundId id,const char* path);
    const char* texturePath(TextureId id) const noexcept;
    const char* soundPath(SoundId id) const noexcept;

    std::map<TextureId,sf::Texture> textures_;
    std::map<SoundId,sf::SoundBuffer> soundBuffers_;
    std::vector<sf::Sound> activeSounds_;
    float volume_=80.f;
};
