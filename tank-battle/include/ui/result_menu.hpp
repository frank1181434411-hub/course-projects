#pragma once

#include <SFML/Graphics.hpp>

#include "core/game_status.hpp"
#include "ui/ui_types.hpp"

class ResultMenu {
public:
    void setResult(GameStatus result);
    UIEventResult handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window) const;

private:
    GameStatus result_ = GameStatus::Victory;
    int selectedIndex_ = 0;
};
