#pragma once

#include <SFML/Graphics.hpp>

#include "entity/player_tank.hpp"
#include "network/network_types.hpp"

class Hud {
public:
    void draw(sf::RenderWindow& window, const PlayerTank* playerOne, const PlayerTank* playerTwo, NetworkMode mode) const;
};
