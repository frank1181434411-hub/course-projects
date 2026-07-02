#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

#include "ui/ui_types.hpp"

class JoinMenu {
public:
    JoinMenu();

    UIEventResult handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window) const;
    const std::string& selectedAddress() const noexcept;

private:
    std::vector<std::string> hosts_;
    int selectedIndex_=0;
};
