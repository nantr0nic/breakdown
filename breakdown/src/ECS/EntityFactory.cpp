#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <entt/entt.hpp>

#include "ECS/EntityFactory.hpp"
#include "ECS/Components.hpp"
#include "SFML/System/Vector2.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"
#include "AppContext.hpp"
#include "AssetKeys.hpp"

#include <string>
#include <utility>

// functions for the ECS system
namespace EntityFactory
{
    //$ --- Player ---
    // the player is a paddle, of course
    entt::entity createPlayer(AppContext& context)
    {
        auto& registry = *context.m_Registry;
        auto playerEntity = registry.create();

        // Load config values
        context.m_ConfigManager->loadConfig(Assets::Configs::Player, "config/Player.toml");

        float moveSpeed = context.m_ConfigManager->getConfigValue<float>(
                          Assets::Configs::Player, "player", "movementSpeed").value_or(350.0f);
        float paddleWidth = context.m_ConfigManager->getConfigValue<float>(
                            Assets::Configs::Player, "player", "paddleWidth").value_or(140.0f);
        float paddleHeight = context.m_ConfigManager->getConfigValue<float>(
                            Assets::Configs::Player, "player", "paddleHeight").value_or(20.0f);

        // Player paddle properties
        // Starting position
        auto windowCenterX = context.m_AppSettings.targetWidth / 2.0f;
        auto paddleYPosition = context.m_AppSettings.targetHeight - 50.0f;
        sf::Vector2f playerPosition = sf::Vector2f(windowCenterX, paddleYPosition);

        sf::Vector2f paddleSize = sf::Vector2f(paddleWidth, paddleHeight);
        sf::Color paddleColor = utils::loadColorFromConfig(*context.m_ConfigManager,
                                Assets::Configs::Player, "player", "paddleRGB");

        Paddle playerPaddle(paddleSize, paddleColor, playerPosition);

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

        // Load config values
        context.m_ConfigManager->loadConfig(Assets::Configs::Ball, "config/Ball.toml");

        float ballRadius = context.m_ConfigManager->getConfigValue<float>(
                           Assets::Configs::Ball, "ball", "ballRadius").value_or(25.0f);
        float ballSpeed = context.m_ConfigManager->getConfigValue<float>(
                          Assets::Configs::Ball, "ball", "ballSpeed").value_or(450.0f);

        sf::Color ballColor = utils::loadColorFromConfig(*context.m_ConfigManager,
                                Assets::Configs::Ball, "ball", "ballRGB");


        // Calculate ballStartingPosition from Player Position
        sf::Vector2f ballStartingPosition{ 0.0f, 0.0f };
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
        registry.emplace<Velocity>(ballEntity);
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

        // Placeholder values
        sf::Color color = sf::Color::White;
        int brickScoreValue = 0;
        int brickHealthValue = 0;
        int brickHealthMax = 0;

        switch (type)
        {
            case BrickType::Normal:
                brickScoreValue = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "normal", "scoreValue").value_or(5);
                brickHealthMax = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "normal", "healthMax").value_or(1);
                color = utils::loadColorFromConfig(*context.m_ConfigManager, 
                        Assets::Configs::Bricks, "normal", "normalRGB");
                break;
            case BrickType::Strong:
                brickScoreValue = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "strong", "scoreValue").value_or(10);
                brickHealthMax = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "strong", "healthMax").value_or(2);
                color = utils::loadColorFromConfig(*context.m_ConfigManager, 
                        Assets::Configs::Bricks, "strong", "strongRGB");
                break;
            case BrickType::Gold:
                brickScoreValue = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "gold", "scoreValue").value_or(20);
                brickHealthMax = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "gold", "healthMax").value_or(1);
                color = utils::loadColorFromConfig(*context.m_ConfigManager, 
                        Assets::Configs::Bricks, "gold", "goldRGB");
                break;
            case BrickType::Custom_1:
                brickScoreValue = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "custom_1", "scoreValue").value_or(0);
                brickHealthMax = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "custom_1", "healthMax").value_or(0);
                color = utils::loadColorFromConfig(*context.m_ConfigManager, 
                        Assets::Configs::Bricks, "custom_1", "custom_1RGB");
                break;
            case BrickType::Custom_2:
                brickScoreValue = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "custom_2", "scoreValue").value_or(0);
                brickHealthMax = context.m_ConfigManager->getConfigValue<int>(
                                  Assets::Configs::Bricks, "custom_2", "healthMax").value_or(0);
                color = utils::loadColorFromConfig(*context.m_ConfigManager, 
                        Assets::Configs::Bricks, "custom_2", "custom_2RGB");
                break;
            default:
                break;
        }

        brickHealthValue = brickHealthMax;

        registry.emplace<Brick>(brickEntity, size, color, position);
        registry.emplace<BrickScore>(brickEntity, brickScoreValue);
        registry.emplace<BrickHealth>(brickEntity, brickHealthValue, brickHealthMax);

        return brickEntity;
    }

    void createBricks(AppContext& context)
    {
        auto& registry = *context.m_Registry;
        auto& window = *context.m_MainWindow;
        sf::Vector2f windowSize = { context.m_AppSettings.targetWidth, 
                                    context.m_AppSettings.targetHeight };

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
                    createABrick(context, brickSize, brickPosition, BrickType::Strong);
                }
                else if (brick % 5 == 0)
                {
                    createABrick(context, brickSize, brickPosition, BrickType::Gold);
                }
                else
                {
                    createABrick(context, brickSize, brickPosition, BrickType::Normal);
                }
            }
        }
        logger::Info("Bricks created.");
    }

    float loadLevel(AppContext& context, int levelNumber)
    {
        std::string sectionName = std::format("level_{}", levelNumber);

        context.m_ConfigManager->loadConfig(Assets::Configs::Bricks, "config/Bricks.toml");

        float descentSpeed = context.m_ConfigManager->getConfigValue<float>(Assets::Configs::Levels,
            sectionName, "descentSpeed"
        ).value_or(0.0f);

        std::vector<std::string> layout = context.m_ConfigManager->getStringArray(
            Assets::Configs::Levels, sectionName, "layout"
        );

        if (layout.empty())
        {
            logger::Error("Failed to load level layout: " + sectionName);
            return 0.0f;
        }

        sf::Vector2f startPos{ 10.0f, 10.0f };

        float brickWidth = context.m_ConfigManager->getConfigValue<float>(
            Assets::Configs::Levels, sectionName, "brickWidth"
        ).value_or(120.0f);
        float brickHeight = context.m_ConfigManager->getConfigValue<float>(
            Assets::Configs::Levels, sectionName, "brickHeight"
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
                    case 'X':
                        type = BrickType::Custom_1;
                        break;
                    case 'Y':
                        type = BrickType::Custom_2;
                        break;
                    default:  
                        type = BrickType::Normal; 
                        break;
                }

                createABrick(context, brickSize, pos, type);
            }
        }
        
        logger::Info(std::format("Level {} loaded successfully. Level speed: {}", 
                                levelNumber, descentSpeed));

        return descentSpeed;
    }
    
    //$ ----- HUD ----- //
    entt::entity createScoreDisplay(AppContext &context, sf::Font& font, 
                                    unsigned int size, const sf::Color& color, 
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

    //$ ----- G/UI ----- //
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
        registry.emplace<UIBounds>(buttonEntity, buttonShape.shape.getGlobalBounds());

        // Clickable component
        registry.emplace<UIAction>(buttonEntity, std::move(action));
        
        return buttonEntity;
    }

    entt::entity createGUIButton(AppContext& context, sf::Texture& texture,
                                sf::Vector2f position,
                                std::function<void()> action,
                                ButtonNames buttonName)
    {
        auto& registry = *context.m_Registry;
        auto buttonEntity = registry.create();
        
        registry.emplace<MenuUITag>(buttonEntity);
        registry.emplace<GUIButtonTag>(buttonEntity);
        
        sf::Sprite buttonSprite(texture);
        buttonSprite.setPosition(position);
        registry.emplace<GUISprite>(buttonEntity, std::move(buttonSprite));
        
        // Bounds component
        registry.emplace<UIBounds>(buttonEntity, buttonSprite.getGlobalBounds());

        // Clickable component
        registry.emplace<UIAction>(buttonEntity, std::move(action));
        
        registry.emplace<GUIButtonName>(buttonEntity, buttonName); // to identify
        
        logger::Info("Button created");
        return buttonEntity;
    }
    
    entt::entity createButtonLabel(AppContext& context, const entt::entity buttonEntity,
                                sf::Font& font, const std::string& text, 
                                unsigned int size, const sf::Color& color)
    {
        auto& registry = *context.m_Registry;
        auto labelEntity = registry.create();
        registry.emplace<MenuUITag>(labelEntity);
        
        // We'll assume the label goes to the left (for now)
        auto& buttonBounds = registry.get<UIBounds>(buttonEntity);
        sf::FloatRect buttonRect = buttonBounds.rect;
        
        auto& labelText = registry.emplace<UIText>(labelEntity, sf::Text(font, text, size));
        labelText.text.setFillColor(color);
        
        sf::FloatRect textBounds = labelText.text.getLocalBounds();
        
        // set origin to RIGHT-CENTER of the text
        sf::Vector2f origin;
        origin.x = textBounds.position.x + textBounds.size.x; // far right edge
        origin.y = textBounds.position.y + (textBounds.size.y / 2.0f);
        labelText.text.setOrigin(origin);
        
        float labelPadding = 10.0f;
        
        sf::Vector2f position;
        position.x = buttonRect.position.x - labelPadding; // left of the button
        position.y = buttonRect.position.y + (buttonRect.size.y / 2.0f);
        
        labelText.text.setPosition(position);
        
        return labelEntity;
    }
    
    entt::entity createLabeledButton(AppContext &context, sf::Texture &texture, 
                                sf::Vector2f position, std::function<void ()> action,
                                sf::Font& font, const std::string& text, 
                                unsigned int size, const sf::Color& color, 
                                ButtonNames buttonName)
    {
        auto& registry = *context.m_Registry;
        auto buttonEntity = registry.create();
        
        registry.emplace<MenuUITag>(buttonEntity);
        registry.emplace<GUIButtonTag>(buttonEntity);
        
        sf::Sprite buttonSprite(texture);
        buttonSprite.setPosition(position);
        registry.emplace<GUISprite>(buttonEntity, std::move(buttonSprite));
        
        // Bounds component
        auto& buttonBounds = registry.emplace<UIBounds>(buttonEntity, 
                             buttonSprite.getGlobalBounds());
        sf::FloatRect buttonRect = buttonBounds.rect;

        // Clickable component
        registry.emplace<UIAction>(buttonEntity, std::move(action));
        
        registry.emplace<GUIButtonName>(buttonEntity, buttonName); // to identify
        
        // Label text
        auto& labelText = registry.emplace<UIText>(buttonEntity, sf::Text(font, text, size));
        labelText.text.setFillColor(color);
        
        sf::FloatRect textBounds = labelText.text.getLocalBounds();
        
        sf::Vector2f origin;
        origin.x = textBounds.position.x + textBounds.size.x; // far right edge
        origin.y = textBounds.position.y + (textBounds.size.y / 2.0f);
        labelText.text.setOrigin(origin);
        
        float labelPadding = 10.0f;
        
        sf::Vector2f labelPosition;
        labelPosition.x = buttonRect.position.x - labelPadding; // left of the button
        labelPosition.y = buttonRect.position.y + (buttonRect.size.y / 2.0f);
        
        labelText.text.setPosition(labelPosition);
        
        return buttonEntity;
    }
}