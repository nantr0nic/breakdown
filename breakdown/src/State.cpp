#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Event.hpp>

#include "AppContext.hpp"
#include "AppData.hpp"
#include "State.hpp"
#include "Managers/StateManager.hpp"
#include "ECS/Components.hpp"
#include "ECS/EntityFactory.hpp"
#include "ECS/Systems.hpp"
#include "SFML/Graphics/Text.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"
#include "AssetKeys.hpp"

#include <memory>
#include <format>

//$ ----- MenuState Implementation ----- //
MenuState::MenuState(AppContext& context)
    : State(context)
{
    sf::Vector2f windowSize = { context.m_AppSettings.targetWidth,
                                context.m_AppSettings.targetHeight };
    sf::Vector2f center = { windowSize.x / 2.0f, windowSize.y / 2.0f };

    // Title text
    sf::Font* titleFont = context.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::ScoreFont);
    if (!titleFont)
    {
        logger::Error("Couldn't load ScoreFont. Not drawing title.");
    }
    else
    {
        m_TitleText.emplace(*titleFont, "Breakdown", 120);
        utils::centerOrigin(*m_TitleText);
        m_TitleText->setPosition({ center.x, center.y - 150.0f });
        m_TitleText->setFillColor(sf::Color(250, 250, 250));
        m_TitleText->setStyle(sf::Text::Style::Italic);
    }
    // Main menu buttons
    sf::Font* buttonFont = context.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (!buttonFont)
    {
        logger::Error("Couldn't load MainFont. Not drawing text to buttons.");
    }
    else
    {
        EntityFactory::createButton(context, *buttonFont, "Play", center,
            [this]() {
                auto playState = std::make_unique<PlayState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(playState));
            }
        );
        EntityFactory::createButton(context, *buttonFont, "Settings",
            {center.x, center.y + 150.0f},
            [this]() {
                auto settingsState = std::make_unique<SettingsMenuState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(settingsState));
            }
        );
    }

    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };

    logger::Info("MenuState initialized.");
}

MenuState::~MenuState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}

void MenuState::update(sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void MenuState::render()
{
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    // Render the text
    if (m_TitleText)
    {
        m_AppContext.m_MainWindow->draw(*m_TitleText);
    }
}

SettingsMenuState::SettingsMenuState(AppContext& context)
    : State(context)
{
    sf::Vector2f windowSize = { context.m_AppSettings.targetWidth,
                                context.m_AppSettings.targetHeight };
    sf::Vector2f center = { windowSize.x / 2.0f, windowSize.y / 2.0f };

    auto* font = context.m_ResourceManager->getResource<sf::Font>(
                                                        Assets::Fonts::ScoreFont);
    auto* buttonBackground = context.m_ResourceManager->getResource<sf::Texture>(
                                                            Assets::Textures::ButtonBackground);
    auto* music = context.m_ResourceManager->getResource<sf::Music>(
                                                            Assets::Musics::MainSong);
    if (font && buttonBackground && music)
    {
        // Mute music button
        auto toggleMusicMute = [this]() { m_AppContext.m_AppSettings.toggleMusicMute(); };
        auto muteMusicButton = EntityFactory::createLabeledButton(context, *buttonBackground,
                                center, toggleMusicMute, *font, UITags::Settings, "Mute Music",
                                36, sf::Color::White, ButtonNames::MuteMusic);

        sf::Vector2f backButtonSize = { 150.0f, 50.0f };
        auto backButton = EntityFactory::createButton(context, *font, "Back",
            {center.x, center.y + 150.0f},
            [this]() {
                auto menuState = std::make_unique<MenuState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(menuState));
            },
            UITags::Settings,
            backButtonSize
        );
    }
    else
    {
        logger::Error("Failed to load resources for SettingsMenuState");
    }

    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };

    logger::Info("SettingsMenuState initialized.");
}

SettingsMenuState::~SettingsMenuState()
{
    // Clean up Menu UI entities
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<SettingsUITag>();
    registry.destroy(view.begin(), view.end());
}

void SettingsMenuState::update(sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    UISystems::uiSettingsChecks(m_AppContext);
}

void SettingsMenuState::render()
{
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}


//$ ----- PlayState Implementation ----- //
PlayState::PlayState(AppContext& context)
    : State(context)
{
    // Create game entities
    m_DescentSpeed = EntityFactory::loadLevel(context, context.m_AppData.levelNumber);
    EntityFactory::createPlayer(context);
    EntityFactory::createBall(context);

    // Create UI/HUD entities
    sf::Vector2f windowSize = { context.m_AppSettings.targetWidth,
                                context.m_AppSettings.targetHeight };
    sf::Vector2f center = { windowSize.x / 2.0f, windowSize.y / 2.0f };

    sf::Font* scoreFont = context.m_ResourceManager->getResource<sf::Font>(
                                                           Assets::Fonts::ScoreFont);
    if (!scoreFont)
    {
        logger::Error("Couldn't load ScoreFont! Score Display will not be created.");
    }
    else
    {
        unsigned int scoreFontSize{ 32 };
        sf::Vector2f scorePosition({ center.x, windowSize.y - 20.0f });

        EntityFactory::createScoreDisplay(context, *scoreFont, scoreFontSize,
                                        sf::Color::White, scorePosition);
    }

    // Handle Music
    m_Music = context.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    if (!m_Music)
    {
        logger::Error("Couldn't load MainSong! Music will not be played.");
    }
    else
    {
        if (context.m_AppSettings.musicMuted)
        {
            logger::Info("Music muted, not playing MainSong.");
        }
        else
        {
            m_Music->setLooping(true);
            m_Music->play();
            logger::Info("Playing MainSong");
        }
    }

    // Handle Input
    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event) {
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
    if (m_AppContext.m_AppData.levelStarted)
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
    sf::Font* font = context.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);

    if (!font)
    {
        logger::Error("MainFont not found! Can't make pause text.");
    }
    else
    {
        m_PauseText.emplace(*font, "Paused", 100);
        m_PauseText->setFillColor(sf::Color::Red);

        utils::centerOrigin(*m_PauseText);

        sf::Vector2f center(context.m_AppSettings.targetWidth / 2.0f,
                            context.m_AppSettings.targetHeight / 2.0f);
        m_PauseText->setPosition(center);
    }

    // Handle music
    auto* music = context.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    bool isMusicPlaying = (music && music->getStatus() == sf::Music::Status::Playing);

    if (isMusicPlaying && music)
    {
        music->pause();
    }

    // Lambda to handle pause
    m_StateEvents.onKeyPress = [this, music, isMusicPlaying](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            if (isMusicPlaying && music)
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

GameTransitionState::GameTransitionState(AppContext& context, TransitionType type)
    : State(context)
{
    sf::Vector2f windowSize = { context.m_AppSettings.targetWidth,
                                context.m_AppSettings.targetHeight };
    sf::Vector2f center = { windowSize.x / 2.0f, windowSize.y / 2.0f };

    // Transition text position
    sf::Vector2f textPosition(center.x, center.y - 200.0f);
    // Button positions
    sf::Vector2f topButtonPos = { center.x, center.y - 70.0f };
    sf::Vector2f middleButtonPos = { center.x, center.y + 50} ;
    sf::Vector2f bottomButtonPos = { center.x, center.y + 200.0f };

    bool nextLevelExists = (context.m_AppData.levelNumber < context.m_AppData.totalLevels
                            ? true : false);

    std::string stateMessage{};
    sf::Color stateMessageColor = sf::Color::White;
    std::string topButtonText{};

    switch (type)
    {
        case TransitionType::LevelLoss:
            stateMessage = "Oops! Level lost.";
            stateMessageColor = sf::Color::Red;
            topButtonText = "Try Again";
            break;
        case TransitionType::LevelWin:
            stateMessage = "Level Complete!";
            stateMessageColor = sf::Color::Green;
            topButtonText = "Next Level";
            break;
        case TransitionType::GameWin:
            stateMessage = "You beat the game! Woo!";
            stateMessageColor = sf::Color::Yellow;
            topButtonText = "Restart";
            break;
        default:
            logger::Error("Invalid transition type.");
            break;
    }

    // Button entities
    auto* font = context.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (!font)
    {
        logger::Error("Couldn't load font. Can't make transition state buttons or text.");
    }
    else
    {
        // Set button tag to TransUITag
        UITags buttonTag = UITags::Transition;

        // Create top button
        switch (type)
        {
            case TransitionType::LevelLoss:
                EntityFactory::createButton(
                    context,
                    *font,
                    topButtonText,
                    topButtonPos,
                    [this]() {
                        logger::Info("Try Again button pressed.");
                        m_AppContext.m_AppData.levelStarted = false;
                        auto playState = std::make_unique<PlayState>(m_AppContext);
                        m_AppContext.m_StateManager->replaceState(std::move(playState));
                    },
                    buttonTag
                );
                break;
            case TransitionType::LevelWin:
                EntityFactory::createButton(
                    context,
                    *font,
                    topButtonText,
                    topButtonPos,
                    [this, nextLevelExists]() {
                        logger::Info("Next Level button pressed.");
                        m_AppContext.m_AppData.levelStarted = false;
                        if (nextLevelExists)
                        {
                            m_AppContext.m_AppData.levelNumber++;
                        }
                        auto playState = std::make_unique<PlayState>(m_AppContext);
                        m_AppContext.m_StateManager->replaceState(std::move(playState));
                    },
                    buttonTag
                );
                break;
            case TransitionType::GameWin:
                EntityFactory::createButton(
                    context,
                    *font,
                    topButtonText,
                    topButtonPos,
                    [this]() {
                        logger::Info("Restart button pressed.");
                        m_AppContext.m_AppData.reset();
                        auto playState = std::make_unique<PlayState>(m_AppContext);
                        m_AppContext.m_StateManager->replaceState(std::move(playState));
                    },
                    buttonTag
                );
                break;
        }

        // make the "Main Menu" button
        EntityFactory::createButton(
            context,
            *font,
            "Main Menu",
            middleButtonPos,
            [this]() {
                logger::Info("Main menu button pressed.");
                m_AppContext.m_AppData.reset();
                auto menuState = std::make_unique<MenuState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(menuState));
            },
            buttonTag
        );

        // make the "Quit" button
        EntityFactory::createButton(
            context,
            *font,
            "Quit",
            bottomButtonPos,
            [this]() {
                logger::Info("Quit button pressed.");
                m_AppContext.m_MainWindow->close();
            },
            buttonTag
        );

        // Transition state text
        m_TransitionText.emplace(*font, stateMessage, 100);
        m_TransitionText->setFillColor(stateMessageColor);
        utils::centerOrigin(*m_TransitionText);
        m_TransitionText->setPosition(textPosition);
    }

    // Handle music stuff
    auto* music = context.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    bool wasMusicPlaying = (music && music->getStatus() == sf::Music::Status::Playing);

    if (wasMusicPlaying)
    {
        music->stop();
    }

    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };

    logger::Info("Game transition state initialized.");
}

GameTransitionState::~GameTransitionState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<TransUITag>();
    registry.destroy(view.begin(), view.end());
}

void GameTransitionState::update(sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void GameTransitionState::render()
{
    // Render button
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    // Render the text
    if (m_TransitionText)
    {
        m_AppContext.m_MainWindow->draw(*m_TransitionText);
    }
}
