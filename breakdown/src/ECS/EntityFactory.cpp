#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include "ECS/EntityFactory.hpp"
#include "ECS/Components.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"
#include "AppContext.hpp"

#include <string>
#include <utility>
#include <cstdint>

// functions for the ECS system
namespace EntityFactory
{
    entt::entity createRectangle(AppContext& context, sf::Vector2f size,
                                sf::Color& color, sf::Vector2f position)
    {
        auto& registry = *context.m_Registry;

        auto rectEntity = registry.create();

        registry.emplace<Paddle>(rectEntity, size, color, position);

        return rectEntity;
    }

    //$ --- Player ---
    entt::entity createPlayer(AppContext& context)
    {
        auto& registry = *context.m_Registry;

        // Load config values
        context.m_ConfigManager->loadConfig("player", "config/Player.toml");
        float moveSpeed = context.m_ConfigManager->getConfigValue<float>(
                          "player", "player", "movementSpeed").value_or(350.0f);

        // Create player Entity in the registry
        auto playerEntity = registry.create();

        // Player paddle properties
        // Starting position
        auto windowCenterX = context.m_MainWindow->getSize().x / 2.0f;
        auto paddleYPosition = context.m_MainWindow->getSize().y - 50.0f;
        sf::Vector2f playerPosition = sf::Vector2f(windowCenterX, paddleYPosition);
        // Paddle start size
        sf::Vector2f paddleSize = sf::Vector2f(100.0f, 20.0f);

        Paddle playerPaddle(paddleSize, sf::Color::White, playerPosition);

        // Add all components that make a "player"
        registry.emplace<PlayerTag>(playerEntity);  // way to ID the player
        registry.emplace<RenderableTag>(playerEntity);
        registry.emplace<MovementSpeed>(playerEntity, moveSpeed);
        registry.emplace<Velocity>(playerEntity);
        registry.emplace<Paddle>(playerEntity, playerPaddle);
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
        auto view = registry.view<PlayerTag, Paddle>();
        for (auto entity : view)
        {
            const auto& playerShape = registry.get<Paddle>(entity).shape;
            const auto& playerPosition = playerShape.getPosition();
            float ballStartX = playerPosition.x;
            float ballStartY = playerPosition.y - playerShape.getSize().y / 2.0f - ballRadius;

            ballStartingPosition = sf::Vector2f(ballStartX, ballStartY);
        }

        Ball ballShape(ballRadius, ballColor, ballStartingPosition);

        registry.emplace<RenderableTag>(ballEntity);
        registry.emplace<Ball>(ballEntity, ballShape);
        auto& velocity = registry.emplace<Velocity>(ballEntity);
        // shoot the ball at start of game
        velocity.value = { 0.0f, -ballSpeed };
        
        registry.emplace<MovementSpeed>(ballEntity, ballSpeed);

        logger::Info("Ball created.");

        return ballEntity;
    }

    entt::entity createABrick(AppContext& context, sf::Vector2f size, 
                              sf::Color& color, sf::Vector2f position)
    {
        auto& registry = *context.m_Registry;
        auto brickEntity = registry.create();

        registry.emplace<Brick>(brickEntity, size, color, position);
        registry.emplace<RenderableTag>(brickEntity);

        return brickEntity;
    }

    void createBricks(AppContext& context)
    {
        auto& registry = *context.m_Registry;
        auto& window = *context.m_MainWindow;
        auto windowSize = window.getSize();

        sf::Vector2f spawnStartXY{ 20.0f, 10.0f };
        sf::Vector2f brickSize{ 60.0f, 20.0f };
        float brickSpacing{ 5.0f };

        float availableWidth = windowSize.x - spawnStartXY.x;
        float availableHeight = windowSize.y - (spawnStartXY.y * 2.0f);
        int bricksPerRow = static_cast<int>((availableWidth + brickSpacing) 
                                            / (brickSize.x + brickSpacing));
        int rows = 5;

        for (int row = 0; row < rows; ++row)
        {
            float rowOffsetX = 0.0f;
            int bricksInThisRow = bricksPerRow;

            // alternate number of bricks per row to give 'staggered' look
            if (row % 2 != 0)
            {
                rowOffsetX = (brickSize.x + brickSpacing) / 2.0f;
                bricksInThisRow = bricksPerRow - 1;
            }

            for (int brick = 0; brick < bricksInThisRow; ++brick)
            {
                sf::Vector2f brickPosition;
                
                // randomize brick color
                std::uint8_t randRed = static_cast<std::uint8_t>(
                                       context.m_RandomMachine->getInt(10, 250));
                std::uint8_t randGreen = static_cast<std::uint8_t>(
                                         context.m_RandomMachine->getInt(10, 250));
                std::uint8_t randBlue = static_cast<std::uint8_t>(
                                        context.m_RandomMachine->getInt(10, 250));
                sf::Color brickColor(randRed, randGreen, randBlue);

                brickPosition.x = spawnStartXY.x + rowOffsetX + 
                                  (brick * (brickSize.x + brickSpacing));
                brickPosition.y = spawnStartXY.y + (row * (brickSize.y + brickSpacing));

                auto brickEntity = createABrick(context, brickSize, brickColor, brickPosition);
            }
        }
        logger::Info("Bricks created.");
    }

    //$ ----- UI/HUD ----- //
    entt::entity createButton(AppContext& context, sf::Font& font,
                            const std::string& text, sf::Vector2f position,
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

    entt::entity createScoreDisplay(AppContext &context, sf::Font &font, 
                                    unsigned int size, sf::Color& color, 
                                    sf::Vector2f position)
    {
        auto& registry = *context.m_Registry;
        auto scoreEntity = registry.create();

        registry.emplace<HUDTag>(scoreEntity);
        registry.emplace<ScoreHUDTag>(scoreEntity);

        registry.emplace<CurrentScore>(scoreEntity, 0);

        auto& scoreText = registry.emplace<UIText>(scoreEntity, sf::Text(font, "Score: 0", size));
        scoreText.text.setFillColor(color);
        utils::centerOrigin(scoreText.text);
        scoreText.text.setPosition(position);

        logger::Info("Score Display created.");

        return scoreEntity;
    }
}