#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Event.hpp>

#include "AppContext.hpp"
#include "State.hpp"
#include "Managers/StateManager.hpp"
#include "ECS/Components.hpp"
#include "ECS/EntityFactory.hpp"
#include "ECS/Systems.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"
#include "AssetKeys.hpp"

#include <memory>
#include <format>

//$ ----- MenuState Implementation ----- //
MenuState::MenuState(AppContext& context)
    : State(context)
{
    sf::Vector2u windowSize = m_AppContext.m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // Play button entity
    sf::Font* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (font)
    {
        EntityFactory::createButton(m_AppContext, *font, "Play", center,
            // lambda for when button is clicked
            [this]() {
                auto playState = std::make_unique<PlayState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(playState));
            }
        );
    }
    else 
    {
        logger::Error("Couldn't load font.");
    }

    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        // using ECS system here instead of previous SFML event response

        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);

    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };

    logger::Info("MenuState initialized.");
}

MenuState::~MenuState()
{
    // Clean up Menu UI entities
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}

void MenuState::update(sf::Time deltaTime)
{
    // Call the UI hover system here
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void MenuState::render()
{
    // Render menu here
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}


//$ ----- PlayState Implementation ----- //
PlayState::PlayState(AppContext& context)
    : State(context)
{
    sf::Vector2u windowSize = m_AppContext.m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // Create game entities
    m_DescentSpeed = EntityFactory::loadLevel(m_AppContext, context.m_LevelNumber);
    EntityFactory::createPlayer(m_AppContext);
    EntityFactory::createBall(m_AppContext);
    //EntityFactory::createBricks(*m_AppContext);

    // Create UI/HUD entities
    sf::Font* scoreFont = m_AppContext.m_ResourceManager->getResource<sf::Font>(
                                                           Assets::Fonts::ScoreFont);                                      
    if (!scoreFont)
    {
        logger::Error("Couldn't load ScoreFont! Score Display will not be created.");
    }
    else 
    {
        unsigned int scoreFontSize{ 32 };
        sf::Vector2f scorePosition({ center.x, windowSize.y - 20.0f });

        EntityFactory::createScoreDisplay(m_AppContext, *scoreFont, scoreFontSize, 
                                        sf::Color::White, scorePosition);
    }

    // Handle music stuff
    m_MainMusic = m_AppContext.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);

    // Start music
    if (m_MainMusic && m_MainMusic->getStatus() != sf::Music::Status::Playing)
    {
        m_MainMusic->setLooping(true);
        m_MainMusic->play();
        logger::Info("Playing MainSong");
    }
    else if (!m_MainMusic)
    {
        logger::Warn("MainSong not found, not playing music.");
    }
    else 
    {
        logger::Warn("MainSong not playing");
    }
    
    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        // "Global" Escape key
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
        // State-specific Pause key
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            auto pauseState = std::make_unique<PauseState>(m_AppContext);
            m_AppContext.m_StateManager->pushState(std::move(pauseState));
        }
        else if (event.scancode == sf::Keyboard::Scancode::F12)
        {
            m_ShowDebug = !m_ShowDebug;
            logger::Warn(std::format("Debug mode toggled: {}", m_ShowDebug ? "On" : "Off"));
        }
    };

    logger::Info("PlayState initialized.");
}

PlayState::~PlayState()
{
    // Clean up all game entities
    auto& registry = *m_AppContext.m_Registry;
    auto gameView = registry.view<RenderableTag>();
    registry.destroy(gameView.begin(), gameView.end());

    // Clean up all HUD entities
    auto hudView = registry.view<HUDTag>();
    registry.destroy(hudView.begin(), hudView.end());
}

void PlayState::update(sf::Time deltaTime)
{
    // Call game logic systems
    CoreSystems::handlePlayerInput(m_AppContext);
    CoreSystems::movementSystem(m_AppContext, deltaTime);
    CoreSystems::collisionSystem(m_AppContext, deltaTime);

    // Descent mechanic
    if (m_AppContext.m_LevelStarted)
    {
        float moveAmount = m_DescentSpeed * deltaTime.asSeconds();
        CoreSystems::moveBricksDown(*m_AppContext.m_Registry, moveAmount);
    }

}

void PlayState::render()
{
    // Call game rendering systems
    CoreSystems::renderSystem(
        *m_AppContext.m_Registry,
        *m_AppContext.m_MainWindow,
        m_ShowDebug
    );

    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}


//$ ----- PauseState Implementation -----
PauseState::PauseState(AppContext& context)
    : State(context)
{
    sf::Font* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);

    if (!font)
    {
        logger::Error("MainFont not found! Can't make pause text.");
    }
    else 
    {
        m_PauseText.emplace(*font, "Paused", 100);
        m_PauseText->setFillColor(sf::Color::Red);

        utils::centerOrigin(*m_PauseText);

        sf::Vector2u windowSize = m_AppContext.m_MainWindow->getSize();
        sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);
        m_PauseText->setPosition(center);
    }

    // Handle music stuff
    auto* music = m_AppContext.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    bool wasMusicPlaying = (music && music->getStatus() == sf::Music::Status::Playing);

    if (wasMusicPlaying)
    {
        music->pause();
    }

    // Lambda to handle pause
    m_StateEvents.onKeyPress = [this, music, wasMusicPlaying](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            if (wasMusicPlaying && music)
            {
                music->play();
            }
            m_AppContext.m_StateManager->popState();
            logger::Info("Game unpaused.");
        }
    };

    logger::Info("Game paused.");
}


void PauseState::update(sf::Time deltaTime)
{
    // Update pause logic here
}

void PauseState::render()
{
    if (m_PauseText)
    {
        m_AppContext.m_MainWindow->draw(*m_PauseText);
    }
}

GameOverState::GameOverState(AppContext& context)
    : State(context)
{
    sf::Vector2u windowSize = m_AppContext.m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // "Try again" button entity
    sf::Font* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (font)
    {
        // make the "Try Again" button
        EntityFactory::createButton(
            m_AppContext,
            *font,
            "Try Again",
            center,
            [this]() {
                logger::Info("Try Again button pressed.");
                m_AppContext.m_LevelStarted = false;
                auto playState = std::make_unique<PlayState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(playState));
            }
        );

        // make the "Quit" button
        EntityFactory::createButton(
            m_AppContext,
            *font,
            "Quit",
            {center.x, center.y + 150.0f},
            [this]() {
                logger::Info("Quit button pressed.");
                m_AppContext.m_MainWindow->close();
            }
        );

        m_GameOverText.emplace(*font, "Game Over", 100);
        m_GameOverText->setFillColor(sf::Color::Red);

        utils::centerOrigin(*m_GameOverText);
        
        sf::Vector2f aboveButton(center.x, center.y - 150.0f);
        m_GameOverText->setPosition(aboveButton);
    }
    else 
    {
        logger::Error("Couldn't load font. Can't make Game Over button or text.");
    }

    // Handle music stuff
    auto* music = m_AppContext.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    bool wasMusicPlaying = (music && music->getStatus() == sf::Music::Status::Playing);

    if (wasMusicPlaying)
    {
        music->stop();
    }

    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };

    logger::Info("GameOverState initialized.");
}

GameOverState::~GameOverState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}

void GameOverState::update(sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void GameOverState::render()
{
    // Render button
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    // Render the text
    if (m_GameOverText)
    {
        m_AppContext.m_MainWindow->draw(*m_GameOverText);
    }
}

WinState::WinState(AppContext& context)
    : State(context)
{
    sf::Vector2u windowSize = m_AppContext.m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);
    
    bool nextLevelExists = (m_AppContext.m_LevelNumber < m_AppContext.m_TotalLevels ? true : false);

    // "Next Level" button entity
    sf::Font* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (font)
    {
        // make the "Next Level" button
        EntityFactory::createButton(
            m_AppContext,
            *font,
            "Next Level",
            center,
            [this, nextLevelExists]() {
                logger::Info("Next Level button pressed.");
                m_AppContext.m_LevelStarted = false;
                (nextLevelExists ? m_AppContext.m_LevelNumber++ : m_AppContext.m_LevelNumber);
                auto playState = std::make_unique<PlayState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(playState));
            }
        );

        // make the "Quit" button
        EntityFactory::createButton(
            m_AppContext,
            *font,
            "Quit",
            {center.x, center.y + 150.0f},
            [this]() {
                logger::Info("Quit button pressed.");
                m_AppContext.m_MainWindow->close();
            }
        );

        m_WinText.emplace(*font, "Level Complete!", 100);
        m_WinText->setFillColor(sf::Color::Green);

        utils::centerOrigin(*m_WinText);
        
        sf::Vector2f aboveButton(center.x, center.y - 150.0f);
        m_WinText->setPosition(aboveButton);
    }
    else 
    {
        logger::Error("Couldn't load font. Can't make Level Complete buttons or text.");
    }

    // Handle music stuff
    auto* music = m_AppContext.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    bool wasMusicPlaying = (music && music->getStatus() == sf::Music::Status::Playing);

    if (wasMusicPlaying)
    {
        music->stop();
    }

    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };

    logger::Info("WinState initialized.");
}

WinState::~WinState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}

void WinState::update(sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void WinState::render()
{
    // Render button
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    // Render the text
    if (m_WinText)
    {
        m_AppContext.m_MainWindow->draw(*m_WinText);
    }
}

GameCompleteState::GameCompleteState(AppContext& context)
    : State(context)
{
    sf::Vector2u windowSize = m_AppContext.m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // "Restart" button entity
    sf::Font* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (font)
    {
        // make the "Restart" button
        EntityFactory::createButton(
            m_AppContext,
            *font,
            "Restart",
            center,
            [this]() {
                logger::Info("Restart button pressed.");
                m_AppContext.m_LevelStarted = false;
                m_AppContext.m_LevelNumber = 1;
                auto playState = std::make_unique<PlayState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(playState));
            }
        );

        // make the "Quit" button
        EntityFactory::createButton(
            m_AppContext,
            *font,
            "Quit",
            {center.x, center.y + 150.0f},
            [this]() {
                logger::Info("Quit button pressed.");
                m_AppContext.m_MainWindow->close();
            }
        );

        m_GameCompleteText.emplace(*font, "You've completed the game!", 100);
        m_GameCompleteText->setFillColor(sf::Color::Yellow);

        utils::centerOrigin(*m_GameCompleteText);
        
        sf::Vector2f aboveButton(center.x, center.y - 150.0f);
        m_GameCompleteText->setPosition(aboveButton);
    }
    else 
    {
        logger::Error("Couldn't load font. Can't make Game Complete buttons or text.");
    }

    // Handle music stuff
    auto* music = m_AppContext.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    bool wasMusicPlaying = (music && music->getStatus() == sf::Music::Status::Playing);

    if (wasMusicPlaying)
    {
        music->stop();
    }

    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };

    logger::Info("GameCompleteState initialized.");
}

GameCompleteState::~GameCompleteState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}

void GameCompleteState::update(sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void GameCompleteState::render()
{
    // Render button
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    // Render the text
    if (m_GameCompleteText)
    {
        m_AppContext.m_MainWindow->draw(*m_GameCompleteText);
    }
}