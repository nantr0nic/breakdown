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

    //$ --- HUD Entities --- //
    entt::entity createScoreDisplay(AppContext& context,
                                    sf::Font& font,
                                    unsigned int size,
                                    const sf::Color& color,
                                    sf::Vector2f position);

    //$ --- G/UI Entities --- //
    entt::entity createButton(AppContext& context,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action,
                            UITags tag = UITags::Menu,
                            sf::Vector2f size = {250.0f, 100.0f});

    entt::entity createGUIButton(AppContext& context,
                                sf::Texture& texture,
                                sf::Vector2f position,
                                std::function<void()> action,
                                UITags tag = UITags::Menu);

    entt::entity createButtonLabel(AppContext& context,
                                   const entt::entity buttonEntity,
                                   sf::Font& font, const std::string& text,
                                   unsigned int size = 32,
                                   const sf::Color& color = sf::Color::White,
                                   UITags tag = UITags::Menu);

    entt::entity createLabeledButton(AppContext& context, 
                                    sf::Texture& texture,
                                    sf::Vector2f position,
                                    std::function<void()> action,
                                    sf::Font& font,
                                    UITags tag = UITags::Menu,
                                    const std::string& text = "",
                                    unsigned int size = 32,
                                    const sf::Color& color = sf::Color::White);
}
