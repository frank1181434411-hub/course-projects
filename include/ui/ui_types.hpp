#pragma once

#include <string>

enum class MenuAction {
    None,
    Play,
    HostMultiplayer,
    JoinMultiplayer,
    OpenSettings,
    Exit,
    StartGame,
    StartHostGame,
    JoinSelectedHost,
    Back,
    ContinueGame,
    RestartGame,
    QuitToMainMenu
};

enum class UISoundEvent {
    None,
    MenuMove,
    MenuConfirm,
    MenuBack,
    PauseOpen,
    PauseClose,
    Victory,
    Defeat
};

enum class SettingsReturnTarget {
    MainMenu,
    PauseMenu
};

enum class AIDifficulty {
    Easy,
    Normal,
    Hard
};

struct UIEventResult {
    MenuAction action = MenuAction::None;
    UISoundEvent sound = UISoundEvent::None;
};

struct GameStartConfig {
    std::string mapPath = "assets/maps/single/map_test.txt";
    AIDifficulty aiDifficulty = AIDifficulty::Hard;
};
