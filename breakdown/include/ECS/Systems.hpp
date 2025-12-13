#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <entt/entt.hpp>

#include <AppContext.hpp>
#include <Managers/StateManager.hpp>
#include <ECS/Components.hpp>

namespace CoreSystems
{
    //$ ----- Game Systems ----- //
    void handlePlayerInput(entt::registry& registry, const sf::RenderWindow& window);

    void movementSystem(entt::registry& registry, sf::Time deltaTime, sf::RenderWindow& window);

    void collisionSystem(AppContext* m_AppContext, sf::Time deltaTime);

    void renderSystem(entt::registry& registry, sf::RenderWindow& window, bool showDebug);
}

namespace UISystems
{
    //$ ----- UI Systems -----

    void uiRenderSystem(entt::registry& registry, sf::RenderWindow& window);

    void uiClickSystem(entt::registry& registry, const sf::Event::MouseButtonPressed& event);

    void uiHoverSystem(entt::registry& registry, sf::RenderWindow& window);
}