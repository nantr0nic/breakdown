#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

// forward declarations
class AppContext;
struct BoundaryHits;

namespace CoreSystems
{
    //$ ----- Game Systems ----- //
    void handlePlayerInput(AppContext* m_AppContext);

    void movementSystem(entt::registry& registry, sf::Time deltaTime, sf::RenderWindow& window);

    void collisionSystem(entt::registry& registry, sf::Time deltaTime, sf::RenderWindow& window);

    void renderSystem(entt::registry& registry, sf::RenderWindow& window, bool showDebug);
}

namespace UISystems
{
    //$ ----- UI Systems -----

    void uiRenderSystem(entt::registry& registry, sf::RenderWindow& window);

    void uiClickSystem(entt::registry& registry, const sf::Event::MouseButtonPressed& event);

    void uiHoverSystem(entt::registry& registry, sf::RenderWindow& window);
}