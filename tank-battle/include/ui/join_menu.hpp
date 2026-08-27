#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

#include "network/host_discovery.hpp"
#include "ui/ui_types.hpp"

class JoinMenu {
public:
    JoinMenu();

    UIEventResult handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window) const;
    void setHosts(const std::vector<DiscoveredHost>& hosts);
    const std::string& selectedAddress() const noexcept;
    unsigned short selectedPort() const noexcept;

private:
    std::vector<DiscoveredHost> hosts_;
    std::string emptyAddress_;
    int selectedIndex_=0;
};
