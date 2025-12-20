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
    void handlePlayerInput(AppContext* context);

    void movementSystem(AppContext* context, sf::Time deltaTime);

    void collisionSystem(AppContext* context, sf::Time deltaTime);

    void renderSystem(entt::registry& registry, sf::RenderWindow& window, bool showDebug);
}

namespace UISystems
{
    //$ ----- UI Systems -----

    void uiRenderSystem(entt::registry& registry, sf::RenderWindow& window);

    void uiClickSystem(entt::registry& registry, const sf::Event::MouseButtonPressed& event);

    void uiHoverSystem(entt::registry& registry, sf::RenderWindow& window);
}