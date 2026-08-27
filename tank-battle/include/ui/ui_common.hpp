#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

namespace ui {

const sf::Font* defaultFont() noexcept;
sf::Text makeText(const std::string& text, unsigned int size, sf::Color color = sf::Color::White);
void centerOrigin(sf::Text& text);
void drawPanel(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size, sf::Color fill, sf::Color outline, float outlineThickness = 2.f);
void drawCenteredText(sf::RenderWindow& window, const std::string& text, unsigned int size, sf::Vector2f center, sf::Color color = sf::Color::White);
void drawVerticalMenu(sf::RenderWindow& window, const std::vector<std::string>& items, int selectedIndex, sf::Vector2f center, float rowSpacing, unsigned int textSize = 28);

}
