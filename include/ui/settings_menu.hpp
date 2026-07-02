#pragma once

#include <SFML/Graphics.hpp>

#include "ui/ui_types.hpp"

class SettingsMenu {
public:
    void setReturnTarget(SettingsReturnTarget target) noexcept;
    SettingsReturnTarget returnTarget() const noexcept;
    float volume() const noexcept;
    void setVolume(float value) noexcept;

    UIEventResult handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window) const;

private:
    float sliderValue(int index) const noexcept;
    void setSliderValue(int index,float value) noexcept;
    int sliderIndexAt(sf::Vector2f position) const noexcept;
    void updateSliderFromMouse(int index,float mouseX) noexcept;

    SettingsReturnTarget returnTarget_ = SettingsReturnTarget::MainMenu;
    int selectedIndex_ = 0;
    int draggingSlider_ = -1;
    float volume_ = 80.f;
};
