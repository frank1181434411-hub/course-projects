#pragma once

#include <SFML/Graphics.hpp>

#include <random>
#include <vector>

#include "entity/item.hpp"
#include "world/map.hpp"

class ItemSpawner
{
public:
    explicit ItemSpawner(const Map& map);

    void update(
        float deltaTime,
        std::vector<Item>& items,
        const std::vector<sf::FloatRect>& occupiedAreas
    );

    void reset() noexcept
    {
        healthSpawnTimer_=HealthFirstDelay;
        damageSpawnTimer_=DamageFirstDelay;
    }

    void setSpawnInterval(float seconds)
    {
        spawnInterval_=seconds;
    }

    void setLifeTime(float seconds)
    {
        lifeTime_=seconds;
    }

private:
    static constexpr float HealthFirstDelay=15.f;
    static constexpr float DamageFirstDelay=5.f;

    void updateSpawnTimer(
        float& timer,
        float deltaTime,
        ItemType type,
        std::vector<Item>& items,
        const std::vector<sf::FloatRect>& occupiedAreas
    );

    bool hasAliveItemType(
        const std::vector<Item>& items,
        ItemType type
    ) const noexcept;

    bool isValidSpawnPos(
        int x,
        int y,
        const std::vector<Item>& items,
        const std::vector<sf::FloatRect>& occupiedAreas
    ) const;

    sf::Vector2f findRandomSpawnPos(
        const std::vector<Item>& items,
        const std::vector<sf::FloatRect>& occupiedAreas
    );

    const Map& map_;
    std::mt19937 random_;

    float healthSpawnTimer_=HealthFirstDelay;
    float damageSpawnTimer_=DamageFirstDelay;
    float spawnInterval_=15.f;
    float lifeTime_=10.f;
};
