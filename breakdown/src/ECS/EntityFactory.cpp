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
        sf::Vector2f paddleSize = sf::Vector2f(140.0f, 20.0f);

        Paddle playerPaddle(paddleSize, sf::Color::White, playerPosition);

        // Add all components that make a "player"
        registry.emplace<PaddleTag>(playerEntity);  // way to ID the player
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
        auto view = registry.view<PaddleTag, Paddle>();
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
                              sf::Vector2f position, BrickType type)
    {
        auto& registry = *context.m_Registry;
        auto brickEntity = registry.create();

        registry.emplace<BrickTag>(brickEntity);
        registry.emplace<RenderableTag>(brickEntity);
        registry.emplace<BrickType>(brickEntity, type);

        sf::Color color = sf::Color::White;
        int brickScoreValue = 0;
        int brickHealthValue = 0;
        int brickHealthMax = 0;

        switch (type)
        {
            case BrickType::Normal:
                brickScoreValue = 5;
                brickHealthValue = 1;
                brickHealthMax = 1;
                color = BrickColors::Normal;
                break;
            case BrickType::Strong:
                brickScoreValue = 10;
                brickHealthValue = 2;
                brickHealthMax = 2;
                color = BrickColors::Strong;
                break;
            case BrickType::Gold:
                brickScoreValue = 20;
                brickHealthValue = 1;
                brickHealthMax = 1;
                color = BrickColors::Gold;
                break;
            default:
                break;
        }

        registry.emplace<Brick>(brickEntity, size, color, position);
        registry.emplace<BrickScore>(brickEntity, brickScoreValue);
        registry.emplace<BrickHealth>(brickEntity, brickHealthValue, brickHealthMax);

        return brickEntity;
    }

    void createBricks(AppContext& context)
    {
        auto& registry = *context.m_Registry;
        auto& window = *context.m_MainWindow;
        auto windowSize = window.getSize();

        sf::Vector2f spawnStartXY{ 20.0f, 10.0f };
        sf::Vector2f brickSize{ 120.0f, 40.0f };
        float brickSpacing{ 5.0f };

        float availableWidth = windowSize.x - spawnStartXY.x;
        float availableHeight = windowSize.y - (spawnStartXY.y * 2.0f);
        int bricksPerRow = static_cast<int>((availableWidth + brickSpacing) 
                                            / (brickSize.x + brickSpacing));
        int rows = 3;

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

                brickPosition.x = spawnStartXY.x + rowOffsetX + 
                                  (brick * (brickSize.x + brickSpacing));
                brickPosition.y = spawnStartXY.y + (row * (brickSize.y + brickSpacing));
                if (brick % 3 == 0)
                {
                    auto brickEntity = createABrick(context, brickSize, 
                                                    brickPosition, BrickType::Strong);
                }
                else if (brick % 5 == 0)
                {
                    auto brickEntity = createABrick(context, brickSize, 
                                                    brickPosition, BrickType::Gold);
                }
                else
                {
                    auto brickEntity = createABrick(context, brickSize, 
                                                    brickPosition, BrickType::Normal);
                }
            }
        }
        logger::Info("Bricks created.");
    }

    void loadLevel(AppContext& context, int levelNumber)
    {
        context.m_ConfigManager->loadConfig("levels", "config/Levels.toml");

        std::string sectionName = std::format("level_{}", levelNumber);

        std::vector<std::string> layout = context.m_ConfigManager->getStringArray(
            "levels", sectionName, "layout"
        );

        if (layout.empty())
        {
            logger::Error("Failed to load level layout: " + sectionName);
            return;
        }

        sf::Vector2f startPos{ 10.0f, 10.0f };

        float brickWidth = context.m_ConfigManager->getConfigValue<float>(
            "levels", sectionName, "brickWidth"
        ).value_or(120.0f);
        float brickHeight = context.m_ConfigManager->getConfigValue<float>(
            "levels", sectionName, "brickHeight"
        ).value_or(40.0f);

        sf::Vector2f brickSize{ brickWidth, brickHeight };
        float padding = 5.0f;

        for (size_t row = 0; row < layout.size(); ++row)
        {
            const std::string& rowStr = layout[row];
            for (size_t col = 0; col < rowStr.size(); ++col)
            {
                char typeChar = rowStr[col];

                // . and ' ' are empty spaces
                if (typeChar == '.' || typeChar == ' ')
                { 
                    continue;
                }

                sf::Vector2f pos{};
                pos.x = startPos.x + col * (brickSize.x + padding);
                pos.y = startPos.y + row * (brickSize.y + padding);

                // build bricks
                BrickType type = BrickType::Normal;
                switch (typeChar)
                {
                    case 'S': 
                        type = BrickType::Strong; 
                        break;
                    case 'G': 
                        type = BrickType::Gold;   
                        break;
                    case 'N': 
                        type = BrickType::Normal; 
                        break;
                    default:  
                        type = BrickType::Normal; 
                        break;
                }

                createABrick(context, brickSize, pos, type);
            }
        }
        
        logger::Info(std::format("Level {} loaded successfully.", levelNumber));
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