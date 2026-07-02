#pragma once

#include <SFML/Graphics.hpp>

enum class ItemType
{
    HealthPack,
    DamagePack
};

class Item
{
public:
    static constexpr float DefaultSize=20.f;

    Item(sf::Vector2f pos,ItemType type);

    void draw(sf::RenderWindow& window) const;
    void updateAge(float deltaTime) noexcept;

    bool isAlive() const noexcept
    {
        return isAlive_;
    }

    bool isExpired(float lifeTime) const noexcept
    {
        return age_>=lifeTime;
    }

    void pickUp() noexcept
    {
        isAlive_=false;
    }

    ItemType getType() const noexcept
    {
        return type_;
    }

    sf::FloatRect getBounds() const
    {
        return shape_.getGlobalBounds();
    }

private:
    ItemType type_;
    bool isAlive_;
    float age_;
    sf::RectangleShape shape_;
};