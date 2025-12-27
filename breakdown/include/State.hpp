#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Event.hpp>

#include "AppContext.hpp"

#include <functional>
#include <optional>

struct StateEvents
{
	std::function<void(const sf::Event::KeyPressed&)> onKeyPress = [](const auto&){};
    std::function<void(const sf::Event::MouseButtonPressed&)> onMouseButtonPress = [](const auto&){};
};

enum class TransitionType
{
    LevelLoss,
    LevelWin,
    GameWin
};

class State
{
public:
    explicit State(AppContext& context) : m_AppContext(context) {}
    virtual ~State() = default;

    StateEvents& getEventHandlers() noexcept { return m_StateEvents; }
    const StateEvents& getEventHandlers() const noexcept { return m_StateEvents; }

    virtual void update(sf::Time deltaTime) = 0;
    virtual void render() = 0;

protected:
    AppContext& m_AppContext;
    StateEvents m_StateEvents;  // each state will have its own StateEvents instance
};

class MenuState : public State
{
public:
    explicit MenuState(AppContext& context);
    virtual ~MenuState() override;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_TitleText;
};

class SettingsMenuState : public State
{
public:
    explicit SettingsMenuState(AppContext& context);
    virtual ~SettingsMenuState() override;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    // Empty
};

class PlayState : public State
{
public:
    explicit PlayState(AppContext& context);
    virtual ~PlayState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    sf::Music* m_Music{ nullptr };
    bool m_ShowDebug{ false };

    // Descent mechanic data
    float m_DescentSpeed{ 10.0f };
};

class PauseState : public State
{
public:
    explicit PauseState(AppContext& context);
    virtual ~PauseState() override = default;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_PauseText;
};

class GameTransitionState : public State
{
public:
    explicit GameTransitionState(AppContext& context, 
                                TransitionType type = TransitionType::LevelLoss);
    virtual ~GameTransitionState() override;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_TransitionText;
};
