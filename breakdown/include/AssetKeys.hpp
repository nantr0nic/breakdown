#pragma once

#include <string_view>

namespace Assets
{
    namespace Fonts
    {
        constexpr std::string_view MainFont = "MainFont";
        constexpr std::string_view ScoreFont = "ScoreFont";
    }
    namespace Textures
    {
        constexpr std::string_view ButtonRedX = "ButtonRedX";
        constexpr std::string_view ButtonLeftArrow = "ButtonLeftArrow";
        constexpr std::string_view ButtonRightArrow = "ButtonRightArrow";
        constexpr std::string_view ButtonBackground = "ButtonBackground";
    }
    namespace SoundBuffers
    {
        constexpr std::string_view BrickHit = "BrickHit";
        constexpr std::string_view PaddleHit = "PaddleHit";
        constexpr std::string_view WallHit = "WallHit";
        constexpr std::string_view NormBrickBreak = "NormBrickBreak";
        constexpr std::string_view GoldBrickBreak = "GoldBrickBreak";
        constexpr std::string_view StrongBrickBreak = "StrongBrickBreak";
    }
    namespace Musics
    {
        constexpr std::string_view MainSong = "MainSong";
    }
    namespace Configs
    {
        constexpr std::string_view Window = "WindowConfig";
        constexpr std::string_view Player = "Player";
        constexpr std::string_view Ball = "Ball";
        constexpr std::string_view Bricks = "Bricks";
        constexpr std::string_view Levels = "Levels";
    }
}