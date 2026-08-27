#include "ui/main_menu.hpp"

#include <vector>

#include "ui/ui_common.hpp"

UIEventResult MainMenu::handleEvent(const sf::Event& event)
{
    UIEventResult result;
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        const std::vector<MenuAction> actions = {
            MenuAction::Play,
            MenuAction::HostMultiplayer,
            MenuAction::JoinMultiplayer,
            MenuAction::OpenSettings,
            MenuAction::Exit
        };

        if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) {
            selectedIndex_ = (selectedIndex_ + static_cast<int>(actions.size()) - 1) % static_cast<int>(actions.size());
            result.sound = UISoundEvent::MenuMove;
        } else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) {
            selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(actions.size());
            result.sound = UISoundEvent::MenuMove;
        } else if (key->code == sf::Keyboard::Key::Enter) {
            result.action = actions[static_cast<std::size_t>(selectedIndex_)];
            result.sound = UISoundEvent::MenuConfirm;
        }
    }

    return result;
}

void MainMenu::draw(sf::RenderWindow& window) const
{
    ui::drawPanel(window, {44.f, 28.f}, {712.f, 544.f}, sf::Color(13, 18, 28, 230), sf::Color(70, 110, 160, 120), 2.f);
    ui::drawCenteredText(window, "Tank Battle", 44, {400.f, 120.f}, sf::Color(245, 245, 245));

    ui::drawVerticalMenu(
        window,
        {"Single Player", "Host Multiplayer", "Join Multiplayer", "Setting", "Exit"},
        selectedIndex_,
        {400.f, 328.f},
        58.f,
        28
    );
}
