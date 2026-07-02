#include "entity/item.hpp"

Item::Item(sf::Vector2f pos,ItemType type)
    :type_(type),
    isAlive_(true),
    age_(0.f),
    shape_()
{
    shape_.setSize(
        {
            DefaultSize,
            DefaultSize
        }
    );

    shape_.setOrigin(
        {
            DefaultSize*0.5f,
            DefaultSize*0.5f
        }
    );

    shape_.setPosition(pos);

    if(type==ItemType::HealthPack)
        shape_.setFillColor(sf::Color::Red);
    else
        shape_.setFillColor(sf::Color::Magenta);
}

void Item::draw(sf::RenderWindow& window) const
{
    if(isAlive_)
        window.draw(shape_);
}

void Item::updateAge(float deltaTime) noexcept
{
    if(isAlive_&&deltaTime>0.f)
        age_+=deltaTime;
}