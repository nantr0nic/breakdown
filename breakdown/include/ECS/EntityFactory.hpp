#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include "AppContext.hpp"
#include "Components.hpp"

#include <functional>

namespace EntityFactory
{
    //$ --- Game Play Entities --- //
    entt::entity createPlayer(AppContext& context);

    entt::entity createBall(AppContext& context);

    entt::entity createABrick(AppContext& context,
                                sf::Vector2f size,
                                sf::Vector2f position,
                                BrickType type = BrickType::Normal);

    void createBricks(AppContext& context);

    float loadLevel(AppContext& context, int levelNumber);

    //$ --- UI Entities --- //
    entt::entity createButton(AppContext& context,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action);
    
    entt::entity createGUIButton(AppContext& context,
                                sf::Texture& texture,
                                sf::Vector2f position,
                                std::function<void()> action,
                                ButtonNames buttonName);
    
    entt::entity createGUIButtonLabel(AppContext& context,
                                sf::Font& font,
                                const std::string& text,
                                sf::Vector2f position);

    entt::entity createScoreDisplay(AppContext& context, 
                                    sf::Font& font,
                                    unsigned int size,
                                    const sf::Color& color, 
                                    sf::Vector2f position);

}