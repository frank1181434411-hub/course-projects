#include "ui/settings_menu.hpp"

#include <algorithm>
#include <string>

#include "ui/ui_common.hpp"

namespace
{
constexpr float SliderLeft=320.f;
constexpr float SliderTop=284.f;
constexpr float SliderWidth=220.f;
constexpr float SliderHeight=8.f;
constexpr float SliderKnobRadius=9.f;

sf::FloatRect sliderBounds()
{
    return {{SliderLeft-SliderKnobRadius,SliderTop-SliderKnobRadius},
        {SliderWidth+SliderKnobRadius*2.f,SliderHeight+SliderKnobRadius*2.f}};
}

constexpr sf::Vector2f BackButtonPosition{330.f,390.f};
constexpr sf::Vector2f BackButtonSize{140.f,42.f};
}

void SettingsMenu::setReturnTarget(SettingsReturnTarget target) noexcept
{
    returnTarget_ = target;
}

SettingsReturnTarget SettingsMenu::returnTarget() const noexcept
{
    return returnTarget_;
}

float SettingsMenu::volume() const noexcept{return volume_;}

void SettingsMenu::setVolume(float value) noexcept{volume_=std::clamp(value,0.f,100.f);}

UIEventResult SettingsMenu::handleEvent(const sf::Event& event)
{
    UIEventResult result;
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space || key->code == sf::Keyboard::Key::Escape) {
            result.action = MenuAction::Back;
            result.sound = UISoundEvent::MenuBack;
        }
    }

    if(const auto* pressed=event.getIf<sf::Event::MouseButtonPressed>())
    {
        if(pressed->button==sf::Mouse::Button::Left)
        {
            const sf::Vector2f position{
                static_cast<float>(pressed->position.x),
                static_cast<float>(pressed->position.y)
            };
            const int sliderIndex=sliderIndexAt(position);
            if(sliderIndex>=0)
            {
                draggingSlider_=sliderIndex;
                updateSliderFromMouse(draggingSlider_,position.x);
                result.sound=UISoundEvent::MenuMove;
            }
        }
    }

    if(const auto* moved=event.getIf<sf::Event::MouseMoved>())
    {
        if(draggingSlider_>=0)
            updateSliderFromMouse(draggingSlider_,static_cast<float>(moved->position.x));
    }

    if(const auto* released=event.getIf<sf::Event::MouseButtonReleased>())
    {
        if(released->button==sf::Mouse::Button::Left)
            draggingSlider_=-1;
    }

    return result;
}

void SettingsMenu::draw(sf::RenderWindow& window) const
{
    sf::RectangleShape overlay({800.f, 600.f});
    overlay.setFillColor(sf::Color(8, 12, 18, 150));
    window.draw(overlay);

    ui::drawPanel(window,{190.f,118.f},{420.f,380.f},sf::Color(14,20,30,240),sf::Color(91,132,187,160),2.f);
    ui::drawCenteredText(window,"Settings",38,{400.f,172.f},sf::Color(245,245,245));

    sf::Text label=ui::makeText("Volume",22,sf::Color::White);
    label.setPosition({232.f,SliderTop-14.f});
    window.draw(label);

    sf::RectangleShape track({SliderWidth,SliderHeight});
    track.setPosition({SliderLeft,SliderTop});
    track.setFillColor(sf::Color(53,68,90,255));
    window.draw(track);

    sf::RectangleShape filled({SliderWidth*(volume_/100.f),SliderHeight});
    filled.setPosition({SliderLeft,SliderTop});
    filled.setFillColor(sf::Color(219,178,77,230));
    window.draw(filled);

    sf::CircleShape knob(SliderKnobRadius);
    knob.setOrigin({SliderKnobRadius,SliderKnobRadius});
    knob.setPosition({SliderLeft+SliderWidth*(volume_/100.f),SliderTop+SliderHeight*0.5f});
    knob.setFillColor(sf::Color(245,222,160,255));
    knob.setOutlineColor(sf::Color(255,245,210,255));
    knob.setOutlineThickness(2.f);
    window.draw(knob);

    sf::Text percent=ui::makeText(std::to_string(static_cast<int>(volume_))+"%",18,sf::Color(190,214,240));
    percent.setPosition({552.f,SliderTop-11.f});
    window.draw(percent);

    sf::RectangleShape backButton(BackButtonSize);
    backButton.setPosition(BackButtonPosition);
    backButton.setFillColor(sf::Color(219,178,77,220));
    backButton.setOutlineColor(sf::Color(255,230,160,255));
    backButton.setOutlineThickness(2.f);
    window.draw(backButton);

    ui::drawCenteredText(window,"Back",24,{400.f,411.f},sf::Color::Black);
}

int SettingsMenu::sliderIndexAt(sf::Vector2f position) const noexcept
{
    if(sliderBounds().contains(position))
        return 0;
    return -1;
}

void SettingsMenu::updateSliderFromMouse(int index,float mouseX) noexcept
{
    if(index!=0)
        return;

    const float value=(mouseX-SliderLeft)/SliderWidth*100.f;
    setVolume(value);
}
