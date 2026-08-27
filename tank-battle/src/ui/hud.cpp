#include "ui/hud.hpp"
#include "ui/ui_common.hpp"

#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>

namespace {
std::string powerUpText(const PlayerTank* player)
{
    if (player == nullptr || !player->isAlive()) {
        return "None";
    }

    if (!player->hasDamageBuff()) {
        return "None";
    }

    std::ostringstream stream;
    stream.setf(std::ios::fixed);
    stream.precision(1);
    stream << "Ammo Boost: " << std::max(0.f, player->getDamageBuffRemaining()) << "s";
    return stream.str();
}
}

void Hud::draw(sf::RenderWindow& window,const PlayerTank* playerOne,const PlayerTank* playerTwo,const Base* base,NetworkMode mode) const
{
    const bool showSecond = mode != NetworkMode::Offline;

    auto drawCard = [&](const PlayerTank* player, const std::string& title, sf::Vector2f position) {
        ui::drawPanel(window, position, {220.f, 100.f}, sf::Color(18, 24, 36, 185), sf::Color(95, 136, 186, 120), 1.5f);

        sf::Text label = ui::makeText(title, 22, sf::Color(255, 224, 122));
        label.setPosition({position.x + 14.f, position.y + 10.f});
        window.draw(label);

        const int hp = player != nullptr ? player->getHp() : 0;
        const int maxHp = player != nullptr ? player->getMaxHp() : 0;
        const int lives = player != nullptr && player->isAlive() ? 1 : 0;

        sf::Text hpText = ui::makeText("HP: " + std::to_string(hp) + "/" + std::to_string(maxHp), 18, sf::Color::White);
        hpText.setPosition({position.x + 14.f, position.y + 38.f});
        window.draw(hpText);

        sf::Text livesText = ui::makeText("Lives: " + std::to_string(lives), 18, sf::Color::White);
        livesText.setPosition({position.x + 14.f, position.y + 58.f});
        window.draw(livesText);

        sf::Text buffText = ui::makeText("Power Up: " + powerUpText(player), 18, sf::Color::White);
        buffText.setPosition({position.x + 14.f, position.y + 78.f});
        window.draw(buffText);
    };

    if (playerOne != nullptr) {
        drawCard(playerOne, "P1", {10.f, 10.f});
    }

    if (showSecond) {
        drawCard(playerTwo, "P2", {570.f, 10.f});
    }

    if(base!=nullptr)
    {
        const float windowWidth=static_cast<float>(window.getSize().x);

        const sf::Vector2f panelPosition
        {
            windowWidth/2.f-120.f,10.f
        };

        ui::drawPanel(window,panelPosition,{240.f,52.f},sf::Color(18,24,36,210),sf::Color(95,136,186,160),1.5f);

        const int hp=base->getHp();

        sf::Text baseText=ui::makeText("BASE HP: "+std::to_string(hp),22,hp>0?sf::Color(255,224,122):sf::Color(230,70,70));

        const sf::FloatRect bounds=
            baseText.getLocalBounds();

        baseText.setOrigin({bounds.position.x+bounds.size.x/2.f,bounds.position.y+bounds.size.y/2.f});

        baseText.setPosition({windowWidth/2.f,panelPosition.y+26.f});

        window.draw(baseText);
    }

    if (mode == NetworkMode::Host && playerTwo == nullptr) {
        const sf::Vector2u windowSize = window.getSize();
        const sf::Vector2f screenSize{
            static_cast<float>(windowSize.x),
            static_cast<float>(windowSize.y)
        };
        const sf::Vector2f center{
            screenSize.x / 2.f,
            screenSize.y / 2.f
        };

        sf::RectangleShape shade(screenSize);
        shade.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(shade);

        ui::drawPanel(
            window,
            {center.x - 210.f, center.y - 60.f},
            {420.f, 120.f},
            sf::Color(18, 24, 36, 240),
            sf::Color(219, 178, 77),
            2.f
        );

        ui::drawCenteredText(
            window,
            "Waiting for client...",
            32,
            center,
            sf::Color(255, 224, 122)
        );
    }
}
