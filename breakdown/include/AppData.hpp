#pragma once

#include <SFML/Audio.hpp>

#include <list>

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
    bool muteMusic{ false };
    bool muteSFX{ false };
    float volumeMusic{ 100.0f };
    float volumeSFX{ 100.0f };
    
    // Resolution target settings
    float targetWidth{ 1280.0f };
    float targetHeight{ 720.0f };
};