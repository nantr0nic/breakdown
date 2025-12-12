#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include "ECS/EntityFactory.hpp"
#include "ECS/Components.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"
#include "AppContext.hpp"
#include "AssetKeys.hpp"

#include <string>
#include <utility>

// functions for the ECS system
namespace EntityFactory
{
    entt::entity createRectangle(AppContext& context, 
                                sf::Vector2f size,
                                sf::Color& color,
                                sf::Vector2f position)
    {
        auto& registry = *context.m_Registry;

        auto rectEntity = registry.create();

        registry.emplace<RenderableRect>(rectEntity, size, color, position);

        return rectEntity;
    }

    //$ --- Player ---
    entt::entity createPlayer(AppContext& context)
    {
        auto& registry = *context.m_Registry;

        // Load config values
        context.m_ConfigManager->loadConfig("player", "config/Player.toml");
        float moveSpeed = context.m_ConfigManager->getConfigValue<float>("player", "player", "movementSpeed").value_or(350.0f);

        // Create player Entity in the registry
        auto playerEntity = registry.create();

        // Player paddle properties
        // Starting position
        auto windowCenterX = context.m_MainWindow->getSize().x / 2.0f;
        auto paddleYPosition = context.m_MainWindow->getSize().y - 50.0f;
        sf::Vector2f playerPosition = sf::Vector2f(windowCenterX, paddleYPosition);
        // Paddle start size
        sf::Vector2f paddleSize = sf::Vector2f(100.0f, 20.0f);

        RenderableRect playerPaddle(paddleSize, sf::Color::White, playerPosition);

        // Add all components that make a "player"
        registry.emplace<PlayerTag>(playerEntity);  // way to ID the player
        registry.emplace<RenderableTag>(playerEntity);
        registry.emplace<MovementSpeed>(playerEntity, moveSpeed);
        registry.emplace<Velocity>(playerEntity);
        registry.emplace<RenderableRect>(playerEntity, playerPaddle);
        registry.emplace<ConfineToWindow>(playerEntity, 1.0f, 1.0f);

        logger::Info("Player paddle created.");

        return playerEntity;
    }

    entt::entity createBall(AppContext& context)
    {
        auto& registry = *context.m_Registry;
        auto ballEntity = registry.create();

        float ballRadius{ 10.0f };
        sf::Color ballColor{ sf::Color::White };
        sf::Vector2f ballStartingPosition{ 0.0f, 0.0f };
        float ballSpeed{ 450.0f };

        // Calculate ballStartingPosition from Player Position
        auto view = registry.view<PlayerTag, RenderableRect>();
        for (auto entity : view)
        {
            const auto& playerShape = registry.get<RenderableRect>(entity).shape;
            const auto& playerPosition = playerShape.getPosition();
            float ballStartX = playerPosition.x;
            float ballStartY = playerPosition.y - playerShape.getSize().y / 2.0f - ballRadius;

            ballStartingPosition = sf::Vector2f(ballStartX, ballStartY);
        }

        RenderableCircle ballShape(ballRadius, ballColor, ballStartingPosition);

        registry.emplace<RenderableTag>(ballEntity);
        registry.emplace<RenderableCircle>(ballEntity, ballShape);
        auto& velocity = registry.emplace<Velocity>(ballEntity);
        // shoot the ball at start of game
        velocity.value = { 0.0f, -ballSpeed };
        
        registry.emplace<MovementSpeed>(ballEntity, ballSpeed);

        logger::Info("Ball created.");

        return ballEntity;
    }

    //$ --- UI ---
    entt::entity createButton(AppContext& context,
                            sf::Font& font,
                            const std::string& text,
                            sf::Vector2f position,
                            std::function<void()> action)
    {
        auto& registry = *context.m_Registry;

        auto buttonEntity = registry.create();
        registry.emplace<MenuUITag>(buttonEntity); // Tag for easy cleanup

        // Shape component
        auto& buttonShape = registry.emplace<UIShape>(buttonEntity);
        buttonShape.shape.setSize({250.f, 100.f});
        buttonShape.shape.setFillColor(sf::Color::Blue);
        utils::centerOrigin(buttonShape.shape);
        buttonShape.shape.setPosition(position);

        // Text component
        auto& buttonText = registry.emplace<UIText>(
            buttonEntity,
            sf::Text(font, text, 50)
        );
        utils::centerOrigin(buttonText.text);
        buttonText.text.setPosition(position);
        buttonText.text.setFillColor(sf::Color(200, 200, 200));

        // Bounds component
        registry.emplace<Bounds>(buttonEntity, buttonShape.shape.getGlobalBounds());

        // Clickable component
        registry.emplace<Clickable>(buttonEntity, std::move(action));
        
        return buttonEntity;
    }
}