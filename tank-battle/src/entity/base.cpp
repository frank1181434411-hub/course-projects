#include "entity/base.hpp"
#include "core/resource_manager.hpp"

Base::Base(float x, float y)
    : hp_(100),
      shape_()
{
    shape_.setSize({40.f, 40.f});
    shape_.setFillColor(sf::Color::Blue);
    shape_.setOrigin({20.f, 20.f});
    shape_.setPosition({x, y});
}

void Base::takeDamage(int damage) {
    hp_ -= damage;
    if (hp_ < 0) {
        hp_ = 0;
    }
}

void Base::setHp(int hp) noexcept
{
    hp_=hp;
    if(hp_<0)
        hp_=0;
    if(hp_>100)
        hp_=100;
}

void Base::draw(sf::RenderWindow& window) const {
    if (isAlive()) {
        const sf::Texture* texture=ResourceManager::instance().texture(TextureId::Base);
        if(texture!=nullptr)
        {
            sf::Sprite sprite(*texture);
            const sf::Vector2u size=texture->getSize();
            sprite.setOrigin({static_cast<float>(size.x)*0.5f,static_cast<float>(size.y)*0.5f});
            sprite.setPosition(shape_.getPosition());
            sprite.setScale({40.f/static_cast<float>(size.x),40.f/static_cast<float>(size.y)});
            window.draw(sprite);
            return;
        }

        window.draw(shape_);
    }
}
