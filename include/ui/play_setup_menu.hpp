#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

#include "ui/ui_types.hpp"

class PlaySetupMenu {
public:
    PlaySetupMenu();
    explicit PlaySetupMenu(std::string mapFolder);

    UIEventResult handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window) const;
    const GameStartConfig& config() const noexcept;
    void setTitle(std::string title,std::string subtitle);

private:
    void clampSelection();
    void rebuildMapOptions();

    std::string mapFolder_;
    std::string title_;
    std::string subtitle_;
    GameStartConfig config_;
    std::vector<std::string> mapOptions_;
    int selectedIndex_ = 0;
    int mapIndex_ = 0;
};
