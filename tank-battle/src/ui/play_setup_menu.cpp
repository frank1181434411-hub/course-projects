#include "ui/play_setup_menu.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <sstream>
#include <utility>

#include "ui/ui_common.hpp"

namespace {
std::string mapDisplayName(const std::string& path)
{
    const std::filesystem::path fsPath(path);
    return fsPath.stem().string();
}

std::string difficultyName(AIDifficulty difficulty)
{
    if(difficulty==AIDifficulty::Easy)
        return "Easy";

    if(difficulty==AIDifficulty::Normal)
        return "Normal";

    return "Hard";
}

AIDifficulty previousDifficulty(AIDifficulty difficulty)
{
    if(difficulty==AIDifficulty::Hard)
        return AIDifficulty::Normal;

    if(difficulty==AIDifficulty::Normal)
        return AIDifficulty::Easy;

    return AIDifficulty::Hard;
}

AIDifficulty nextDifficulty(AIDifficulty difficulty)
{
    if(difficulty==AIDifficulty::Easy)
        return AIDifficulty::Normal;

    if(difficulty==AIDifficulty::Normal)
        return AIDifficulty::Hard;

    return AIDifficulty::Easy;
}
}

PlaySetupMenu::PlaySetupMenu()
    :PlaySetupMenu("assets/maps/single")
{
}

PlaySetupMenu::PlaySetupMenu(std::string mapFolder)
    :mapFolder_(std::move(mapFolder)),
    title_("Single Player"),
    subtitle_("Configure map and enemy AI difficulty"),
    config_(),
    mapOptions_(),
    selectedIndex_(0),
    mapIndex_(0)
{
    rebuildMapOptions();
    clampSelection();
}

void PlaySetupMenu::setTitle(std::string title,std::string subtitle)
{
    title_=std::move(title);
    subtitle_=std::move(subtitle);
}

void PlaySetupMenu::rebuildMapOptions()
{
    mapOptions_.clear();

    const std::filesystem::path folder = mapFolder_;
    if (std::filesystem::exists(folder) && std::filesystem::is_directory(folder)) {
        for (const auto& entry : std::filesystem::directory_iterator(folder)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const auto extension = entry.path().extension().string();
            if (extension != ".txt") {
                continue;
            }

            mapOptions_.push_back(entry.path().generic_string());
        }
    }

    std::sort(mapOptions_.begin(), mapOptions_.end());
    if (mapOptions_.empty()) {
        mapOptions_.push_back((folder/"map_test.txt").generic_string());
    }

    if (config_.mapPath.empty()) {
        config_.mapPath = mapOptions_.front();
    }

    auto selected = std::find(mapOptions_.begin(), mapOptions_.end(), config_.mapPath);
    if (selected == mapOptions_.end()) {
        selected = mapOptions_.begin();
        config_.mapPath = *selected;
    }

    mapIndex_ = static_cast<int>(std::distance(mapOptions_.begin(), selected));
}

void PlaySetupMenu::clampSelection()
{
    if (selectedIndex_ < 0) {
        selectedIndex_ = 0;
    }
    if (selectedIndex_ > 3) {
        selectedIndex_ = 3;
    }

    if (mapOptions_.empty()) {
        rebuildMapOptions();
    }
    if (mapIndex_ < 0) {
        mapIndex_ = 0;
    }
    if (mapIndex_ >= static_cast<int>(mapOptions_.size())) {
        mapIndex_ = static_cast<int>(mapOptions_.size()) - 1;
    }
    config_.mapPath = mapOptions_[static_cast<std::size_t>(mapIndex_)];
}

UIEventResult PlaySetupMenu::handleEvent(const sf::Event& event)
{
    UIEventResult result;
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        const int rowCount = 4;
        if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) {
            selectedIndex_ = (selectedIndex_ + rowCount - 1) % rowCount;
            result.sound = UISoundEvent::MenuMove;
        } else if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) {
            selectedIndex_ = (selectedIndex_ + 1) % rowCount;
            result.sound = UISoundEvent::MenuMove;
        } else if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) {
            if (selectedIndex_ == 0) {
                mapIndex_ = (mapIndex_ + static_cast<int>(mapOptions_.size()) - 1) % static_cast<int>(mapOptions_.size());
                config_.mapPath = mapOptions_[static_cast<std::size_t>(mapIndex_)];
                result.sound = UISoundEvent::MenuMove;
            } else if (selectedIndex_ == 1) {
                config_.aiDifficulty = previousDifficulty(config_.aiDifficulty);
                result.sound = UISoundEvent::MenuMove;
            }
        } else if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) {
            if (selectedIndex_ == 0) {
                mapIndex_ = (mapIndex_ + 1) % static_cast<int>(mapOptions_.size());
                config_.mapPath = mapOptions_[static_cast<std::size_t>(mapIndex_)];
                result.sound = UISoundEvent::MenuMove;
            } else if (selectedIndex_ == 1) {
                config_.aiDifficulty = nextDifficulty(config_.aiDifficulty);
                result.sound = UISoundEvent::MenuMove;
            }
        } else if (key->code == sf::Keyboard::Key::Enter) {
            if (selectedIndex_ == 2) {
                clampSelection();
                result.action = MenuAction::StartGame;
                result.sound = UISoundEvent::MenuConfirm;
            } else if (selectedIndex_ == 3) {
                result.action = MenuAction::Back;
                result.sound = UISoundEvent::MenuBack;
            }
        }
    }

    clampSelection();
    return result;
}

void PlaySetupMenu::draw(sf::RenderWindow& window) const
{
    ui::drawPanel(window, {34.f, 24.f}, {732.f, 552.f}, sf::Color(12, 16, 26, 232), sf::Color(80, 115, 172, 140), 2.f);
    ui::drawCenteredText(window, title_, 40, {400.f, 90.f}, sf::Color(245, 245, 245));
    ui::drawCenteredText(window, subtitle_, 18, {400.f, 128.f}, sf::Color(170, 188, 210));

    const sf::Vector2f cardPos{120.f, 180.f};
    const sf::Vector2f cardSize{560.f, 270.f};
    ui::drawPanel(window, cardPos, cardSize, sf::Color(22, 30, 44, 240), sf::Color(84, 124, 176, 160), 2.f);

    const std::array<std::string, 4> labels = {
        "Map",
        "AI Difficulty",
        "Start",
        "Back"
    };

    for (std::size_t i = 0; i < labels.size(); ++i) {
        const bool selected = static_cast<int>(i) == selectedIndex_;
        const float rowY = 220.f + static_cast<float>(i) * 58.f;

        if (selected) {
            sf::RectangleShape highlight({500.f, 42.f});
            highlight.setPosition({150.f, rowY - 22.f});
            highlight.setFillColor(sf::Color(219, 178, 77, 220));
            window.draw(highlight);
        }

        sf::Text label = ui::makeText(labels[i], 24, selected ? sf::Color::Black : sf::Color::White);
        label.setPosition({170.f, rowY - 13.f});
        window.draw(label);

        if (i == 0) {
            sf::Text value = ui::makeText(mapDisplayName(config_.mapPath), 20, selected ? sf::Color::Black : sf::Color(190, 214, 240));
            value.setPosition({370.f, rowY - 11.f});
            window.draw(value);
        } else if (i == 1) {
            sf::Text value = ui::makeText(difficultyName(config_.aiDifficulty), 22, selected ? sf::Color::Black : sf::Color(190, 214, 240));
            value.setPosition({370.f, rowY - 11.f});
            window.draw(value);
        } else if (i == 2) {
            sf::Text hint = ui::makeText("Begin battle with the current config", 18, selected ? sf::Color::Black : sf::Color(170, 188, 210));
            hint.setPosition({370.f, rowY - 10.f});
            window.draw(hint);
        } else {
            sf::Text hint = ui::makeText("Return to main menu", 18, selected ? sf::Color::Black : sf::Color(170, 188, 210));
            hint.setPosition({370.f, rowY - 10.f});
            window.draw(hint);
        }
    }
}

const GameStartConfig& PlaySetupMenu::config() const noexcept
{
    return config_;
}
