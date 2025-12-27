#pragma once

#include <SFML/Audio.hpp>

#include "Utilities/Logger.hpp"

#include <list>
#include <format>

struct AppData
{
    // Game level data
    bool levelStarted{ false };
    int levelNumber{ 1 };
    int totalLevels{ 1 };
    
    // Game audio storage: holds sounds while they are playing
    std::list<sf::Sound> activeSounds;
    
    void reset()
    {
        levelStarted = false;
        levelNumber = 1;
        activeSounds.clear();
    }
};

struct AppSettings
{
    // Audio settings
    bool musicMuted{ false };
    bool sfxMuted{ false };
    float musicVolume{ 100.0f };
    float sfxVolume{ 100.0f };
    
    void toggleMusicMute()
    {
        musicMuted = !musicMuted;
        logger::Info(std::format("Music muted: {}", musicMuted ? "true" : "false"));
    }
    
    // Resolution target settings
    float targetWidth{ 1280.0f };
    float targetHeight{ 720.0f };
};