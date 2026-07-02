cmake_minimum_required(VERSION 3.28)

project(TankBattle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(TANK_BATTLE_BUILD_TESTS "Build Tank Battle tests" OFF)
option(TANK_BATTLE_ENABLE_RL "Enable reinforcement learning features" OFF)
option(TANK_BATTLE_ENABLE_DEBUG_UI "Enable debug UI" OFF)

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
)

FetchContent_MakeAvailable(SFML)

add_executable(TankBattle
    include/ai/ai_context.hpp
    include/ai/ai_controller.hpp
    include/ai/enemy_ai.hpp
    include/ai/tank_command.hpp
    include/core/game_status.hpp
    include/network/network_message.hpp
    include/network/network_session.hpp
    include/network/network_types.hpp
    include/ui/hud.hpp
    include/ui/join_menu.hpp
    include/ui/main_menu.hpp
    include/ui/pause_menu.hpp
    include/ui/play_setup_menu.hpp
    include/ui/result_menu.hpp
    include/ui/settings_menu.hpp
    include/ui/ui_common.hpp
    include/ui/ui_types.hpp
    include/world/item_spawner.hpp
    src/main.cpp
    src/ai/enemy_ai.cpp
    src/core/game.cpp
    src/core/resource_manager.cpp
    src/entity/base.cpp
    src/entity/bullet.cpp
    src/entity/enemy_tank.cpp
    src/entity/item.cpp
    src/entity/player_tank.cpp
    src/entity/tank.cpp
    src/network/network_message.cpp
    src/network/network_session.cpp
    src/ui/hud.cpp
    src/ui/join_menu.cpp
    src/ui/main_menu.cpp
    src/ui/pause_menu.cpp
    src/ui/play_setup_menu.cpp
    src/ui/result_menu.cpp
    src/ui/settings_menu.cpp
    src/ui/ui_common.cpp
    src/world/collision.cpp
    src/world/item_spawner.cpp
    src/world/map.cpp
    src/world/tile.cpp
)

target_include_directories(TankBattle PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(TankBattle PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_definitions(TankBattle PRIVATE
    $<$<BOOL:${TANK_BATTLE_ENABLE_RL}>:TANK_BATTLE_ENABLE_RL>
    $<$<BOOL:${TANK_BATTLE_ENABLE_DEBUG_UI}>:TANK_BATTLE_ENABLE_DEBUG_UI>
)

target_link_libraries(TankBattle PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Network
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET TankBattle POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                "$<TARGET_FILE_DIR:TankBattle>/assets"
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config")
    add_custom_command(TARGET TankBattle POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/config"
                "$<TARGET_FILE_DIR:TankBattle>/config"
    )
endif()

if(TANK_BATTLE_BUILD_TESTS)
    add_executable(TankBattleAITest
        include/ai/ai_context.hpp
        include/ai/ai_controller.hpp
        include/ai/enemy_ai.hpp
        include/ai/tank_command.hpp
        tests/ai_test.cpp
        src/ai/enemy_ai.cpp
        src/entity/base.cpp
        src/entity/bullet.cpp
        src/entity/enemy_tank.cpp
        src/entity/item.cpp
        src/entity/player_tank.cpp
        src/entity/tank.cpp
        src/world/collision.cpp
        src/world/map.cpp
        src/world/tile.cpp
    )

    target_include_directories(TankBattleAITest PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )

    target_compile_options(TankBattleAITest PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    target_link_libraries(TankBattleAITest PRIVATE
        SFML::Graphics
        SFML::Audio
        SFML::Network
    )

    add_executable(TankBattleNetworkTest
        include/network/network_message.hpp
        include/network/network_session.hpp
        include/network/network_types.hpp
        tests/network_test.cpp
        src/network/network_message.cpp
        src/network/network_session.cpp
    )

    target_include_directories(TankBattleNetworkTest PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )

    target_compile_options(TankBattleNetworkTest PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    target_link_libraries(TankBattleNetworkTest PRIVATE
        SFML::Network
        SFML::System
    )
endif()
