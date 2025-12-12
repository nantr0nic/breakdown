#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include <functional>

struct AppContext; // forward declaration

namespace EntityFactory
{
    //$ --- Game Play Entities --- //
    entt::entity createRectangle(AppContext& context,
                                sf::Vector2f size,
                                sf::Color& color,
                                sf::Vector2f position);

    entt::entity createPlayer(AppContext& context);

    entt::entity createBall(AppContext& context);

    entt::entity createABrick(AppContext& context,
                                sf::Vector2f size,
                                sf::Color& color,
                                sf::Vector2f position);

    void createBricks(AppContext& context);

    //$ --- UI Entities --- //
    entt::entity createButton(AppContext& context,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action);
}