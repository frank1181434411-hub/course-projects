#include "ui/ui_common.hpp"

#include <algorithm>
#include <array>
#include <filesystem>

namespace ui {
namespace {
bool loadFontFromCandidates(sf::Font& font)
{
    const std::array<const char*, 6> candidates = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/verdana.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/meiryo.ttc"
    };

    for (const auto* candidate : candidates) {
        if (std::filesystem::exists(candidate) && font.openFromFile(candidate)) {
            return true;
        }
    }

    return false;
}
}

const sf::Font* defaultFont() noexcept
{
    static sf::Font font;
    static bool initialized = false;
    static bool loaded = false;

    if (!initialized) {
        initialized = true;
        loaded = loadFontFromCandidates(font);
    }

    return loaded ? &font : nullptr;
}

sf::Text makeText(const std::string& text, unsigned int size, sf::Color color)
{
    static sf::Font fallbackFont;
    const sf::Font* font = defaultFont();
    sf::Text result(font != nullptr ? *font : fallbackFont);
    result.setString(text);
    result.setCharacterSize(size);
    result.setFillColor(color);
    return result;
}

void centerOrigin(sf::Text& text)
{
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
}

void drawPanel(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size, sf::Color fill, sf::Color outline, float outlineThickness)
{
    sf::RectangleShape panel(size);
    panel.setPosition(position);
    panel.setFillColor(fill);
    panel.setOutlineColor(outline);
    panel.setOutlineThickness(outlineThickness);
    window.draw(panel);
}

void drawCenteredText(sf::RenderWindow& window, const std::string& text, unsigned int size, sf::Vector2f center, sf::Color color)
{
    sf::Text label = makeText(text, size, color);
    centerOrigin(label);
    label.setPosition(center);
    window.draw(label);
}

void drawVerticalMenu(sf::RenderWindow& window, const std::vector<std::string>& items, int selectedIndex, sf::Vector2f center, float rowSpacing, unsigned int textSize)
{
    const float totalHeight = rowSpacing * static_cast<float>(items.size() > 0 ? items.size() - 1 : 0);
    const float startY = center.y - totalHeight / 2.f;

    for (std::size_t i = 0; i < items.size(); ++i) {
        const float y = startY + rowSpacing * static_cast<float>(i);
        const bool selected = static_cast<int>(i) == selectedIndex;

        sf::Text label = makeText(items[i], textSize, selected ? sf::Color::Black : sf::Color::White);
        centerOrigin(label);
        label.setPosition({center.x, y});

        if (selected) {
            const sf::FloatRect bounds = label.getGlobalBounds();
            sf::RectangleShape background({bounds.size.x + 36.f, bounds.size.y + 18.f});
            background.setPosition({bounds.position.x - 18.f, bounds.position.y - 8.f});
            background.setFillColor(sf::Color(219, 178, 77, 220));
            background.setOutlineColor(sf::Color(255, 230, 160, 255));
            background.setOutlineThickness(2.f);
            window.draw(background);
        }

        window.draw(label);
    }
}

}
