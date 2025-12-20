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

class State
{
public:
    State(AppContext& context) : m_AppContext(context) {}
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
    MenuState(AppContext& context);
    virtual ~MenuState() override;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    // Empty
};

class PlayState : public State
{
public:
    PlayState(AppContext& context);
    virtual ~PlayState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

    bool getLevelStarted() const { return m_LevelStarted; }
    void setLevelStarted(bool value) { m_LevelStarted = value; }

private:
    sf::Music* m_MainMusic{ nullptr };
    bool m_LevelStarted{ false };
    bool m_ShowDebug{ false };
};

class PauseState : public State
{
public:
    PauseState(AppContext& context);
    virtual ~PauseState() override = default;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_PauseText;
};

class GameOverState : public State
{
public:
    GameOverState(AppContext& context);
    virtual ~GameOverState() override;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_GameOverText;
};

class WinState: public State
{
public:
    WinState(AppContext& context);
    virtual ~WinState() override;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_WinText;
};

class GameCompleteState : public State
{
public:
    GameCompleteState(AppContext& context);
    virtual ~GameCompleteState() override;

    virtual void update(sf::Time /* deltaTime */) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_GameCompleteText;
};