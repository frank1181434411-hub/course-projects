#include "ui/pause_menu.hpp"

#include <vector>

#include "ui/ui_common.hpp"

UIEventResult PauseMenu::handleEvent(const sf::Event& event)
{
    UIEventResult result;
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        const std::vector<MenuAction> actions = {
            MenuAction::ContinueGame,
            MenuAction::OpenSettings,
            MenuAction::QuitToMainMenu
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

void PauseMenu::draw(sf::RenderWindow& window) const
{
    sf::RectangleShape overlay({800.f, 600.f});
    overlay.setFillColor(sf::Color(8, 12, 18, 145));
    window.draw(overlay);

    ui::drawPanel(window, {210.f, 110.f}, {380.f, 340.f}, sf::Color(14, 20, 30, 240), sf::Color(91, 132, 187, 160), 2.f);
    ui::drawCenteredText(window, "Paused", 40, {400.f, 168.f}, sf::Color(245, 245, 245));
    ui::drawVerticalMenu(window, {"Continue", "Setting", "Quit"}, selectedIndex_, {400.f, 302.f}, 64.f, 28);
}
