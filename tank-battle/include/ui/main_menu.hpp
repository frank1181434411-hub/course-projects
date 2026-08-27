#pragma once

#include <SFML/Graphics.hpp>

#include "ui/ui_types.hpp"

class MainMenu {
public:
    UIEventResult handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window) const;

private:
    int selectedIndex_ = 0;
};
