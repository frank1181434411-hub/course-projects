#include "ui/join_menu.hpp"

#include "ui/ui_common.hpp"

JoinMenu::JoinMenu()
    :hosts_{"127.0.0.1"},
    selectedIndex_(0)
{
}

UIEventResult JoinMenu::handleEvent(const sf::Event& event)
{
    UIEventResult result;
    if(const auto* key=event.getIf<sf::Event::KeyPressed>())
    {
        const int rowCount=static_cast<int>(hosts_.size())+1;
        if(key->code==sf::Keyboard::Key::Up || key->code==sf::Keyboard::Key::W)
        {
            selectedIndex_=(selectedIndex_+rowCount-1)%rowCount;
            result.sound=UISoundEvent::MenuMove;
        }
        else if(key->code==sf::Keyboard::Key::Down || key->code==sf::Keyboard::Key::S)
        {
            selectedIndex_=(selectedIndex_+1)%rowCount;
            result.sound=UISoundEvent::MenuMove;
        }
        else if(key->code==sf::Keyboard::Key::Enter || key->code==sf::Keyboard::Key::Space)
        {
            if(selectedIndex_<static_cast<int>(hosts_.size()))
                result.action=MenuAction::JoinSelectedHost;
            else
                result.action=MenuAction::Back;
            result.sound=UISoundEvent::MenuConfirm;
        }
        else if(key->code==sf::Keyboard::Key::Escape)
        {
            result.action=MenuAction::Back;
            result.sound=UISoundEvent::MenuBack;
        }
    }

    return result;
}

void JoinMenu::draw(sf::RenderWindow& window) const
{
    ui::drawPanel(window,{34.f,24.f},{732.f,552.f},sf::Color(12,16,26,232),sf::Color(80,115,172,140),2.f);
    ui::drawCenteredText(window,"Join Multiplayer",40,{400.f,90.f},sf::Color(245,245,245));
    ui::drawCenteredText(window,"Nearby hosts",18,{400.f,128.f},sf::Color(170,188,210));

    std::vector<std::string> items;
    for(const std::string& host:hosts_)
        items.push_back("Local Host - "+host);
    items.push_back("Back");

    ui::drawVerticalMenu(window,items,selectedIndex_,{400.f,292.f},62.f,28);
}

const std::string& JoinMenu::selectedAddress() const noexcept
{
    return hosts_[static_cast<std::size_t>(selectedIndex_)];
}
