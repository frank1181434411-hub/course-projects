#include "ui/result_menu.hpp"

#include <vector>

#include "ui/ui_common.hpp"

void ResultMenu::setResult(GameStatus result)
{
    if(result_==result)
        return;

    result_ = result;
    selectedIndex_ = 0;
}

UIEventResult ResultMenu::handleEvent(const sf::Event& event)
{
    UIEventResult result;
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        const std::vector<MenuAction> actions = {
            MenuAction::RestartGame,
            MenuAction::QuitToMainMenu,
            MenuAction::Exit
        };

        if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) {
            selectedIndex_ = (selectedIndex_ + static_cast<int>(actions.size()) - 1) % static_cast<int>(actions.size());
            result.sound = UISoundEvent::MenuMove;
        } else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) {
            selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(actions.size());
            result.sound = UISoundEvent::MenuMove;
        } else if (key->scancode == sf::Keyboard::Scancode::NumpadEnter) {
            result.action = actions[static_cast<std::size_t>(selectedIndex_)];
            result.sound = UISoundEvent::MenuConfirm;
        }
    }

    return result;
}

void ResultMenu::draw(sf::RenderWindow& window) const
{
    sf::RectangleShape overlay({800.f, 600.f});
    overlay.setFillColor(sf::Color(8, 12, 18, 165));
    window.draw(overlay);

    ui::drawPanel(window, {176.f, 108.f}, {448.f, 348.f}, sf::Color(14, 20, 30, 242), sf::Color(91, 132, 187, 165), 2.f);

    const std::string title = result_ == GameStatus::Victory ? "Victory" : "Defeat";
    ui::drawCenteredText(window, title, 42, {400.f, 168.f}, result_ == GameStatus::Victory ? sf::Color(255, 214, 112) : sf::Color(255, 123, 123));
    ui::drawCenteredText(window, "Battle complete", 18, {400.f, 208.f}, sf::Color(170, 188, 210));
    ui::drawVerticalMenu(window, {"Restart", "Back To Menu", "Exit"}, selectedIndex_, {400.f, 304.f}, 66.f, 28);
}
