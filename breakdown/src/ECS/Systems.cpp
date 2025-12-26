#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <entt/entt.hpp>

#include "ECS/Systems.hpp"
#include "ECS/Components.hpp"
#include "Managers/StateManager.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Vector2.hpp"
#include "State.hpp"
#include "AppContext.hpp"
#include "AssetKeys.hpp"
#include "Utilities/Logger.hpp"
#include "Utilities/Utils.hpp"


#include <memory>
#include <format>
#include <string_view>

namespace CoreSystems
{
    //$ "Core" / game systems (maybe rename...)
    void handlePlayerInput(AppContext& context)
    {
        auto& registry = context.m_Registry;
        bool levelStarted = context.m_AppData.levelStarted;

        auto paddleView = registry->view<PaddleTag, Velocity, MovementSpeed>();

        for (auto paddleEntity : paddleView)
        {
            auto& velocity = paddleView.get<Velocity>(paddleEntity);
            const auto& speed = paddleView.get<MovementSpeed>(paddleEntity);

            // Reset velocity
            velocity.value = { 0.0f, 0.0f };

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A))
            {
                velocity.value.x -= speed.value;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D))
            {
                velocity.value.x += speed.value;
            }

            if (!levelStarted && sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Space))
            {
                playSound(context, Assets::SoundBuffers::PaddleHit);

                context.m_AppData.levelStarted = true;
                logger::Info("Level started.");

                auto ballView = registry->view<Ball, Velocity, MovementSpeed>();
                for (auto ballEntity : ballView)
                {
                    auto& ballVelocity = ballView.get<Velocity>(ballEntity);
                    auto& ballSpeed = ballView.get<MovementSpeed>(ballEntity);
                    ballVelocity.value = { velocity.value.x, -ballSpeed.value };
                }
            }
        }
    }

    void movementSystem(AppContext& context, sf::Time deltaTime)
    {
        auto& registry = context.m_Registry;
        bool levelStarted = context.m_AppData.levelStarted;

        auto paddleView = registry->view<Paddle, Velocity>();
        for (auto paddleEntity : paddleView)
        {
            auto& paddleComp = paddleView.get<Paddle>(paddleEntity);
            const auto& velocity = paddleView.get<Velocity>(paddleEntity);

            paddleComp.shape.move(velocity.value * deltaTime.asSeconds());
        }

        if (levelStarted)
        {
            auto ballView = registry->view<Ball, Velocity>();
            for (auto ballEntity : ballView)
            {
                auto& ballShape = ballView.get<Ball>(ballEntity);
                const auto& velocity = ballView.get<Velocity>(ballEntity);

                ballShape.shape.move(velocity.value * deltaTime.asSeconds());
            }
        }
        else
        {
            auto paddleOnlyView = registry->view<Paddle>();
            sf::Vector2f paddlePosition{};
            sf::Vector2f paddleSize{};

            for (auto paddleEntity : paddleOnlyView)
            {
                auto& paddleComp = paddleOnlyView.get<Paddle>(paddleEntity);
                paddlePosition = paddleComp.shape.getPosition();
                paddleSize = paddleComp.shape.getSize();
                break;
            }

            auto ballView = registry->view<Ball>();
            for (auto ballEntity : ballView)
            {
                auto& ballComp = ballView.get<Ball>(ballEntity);
                float ballRadius = ballComp.shape.getRadius();

                float x = paddlePosition.x;
                float y = paddlePosition.y - paddleSize.y / 2.0f - ballRadius;

                ballComp.shape.setPosition({ x, y });
            }
        }

    }

    void collisionSystem(AppContext& context, sf::Time deltaTime)
    {
        auto& registry = context.m_Registry;
        auto& window = context.m_MainWindow;
        auto& stateManager = context.m_StateManager;

        sf::Vector2f windowSize = { context.m_AppSettings.targetWidth, 
                                    context.m_AppSettings.targetHeight };
        bool triggerGameOver = false;

        // Cache data structures
        struct CachedBrick { entt::entity entity; sf::FloatRect bounds; };
        static std::vector<CachedBrick> brickCache;
        static std::vector<sf::FloatRect> paddleBoundsList;

        // clear these each frame
        brickCache.clear();
        paddleBoundsList.clear();

        //$ --- Paddle Collision Logic--- //
        auto paddleView = registry->view<Paddle, Velocity>();
        for (auto paddleEntity : paddleView)
        {
            auto& paddleComp = paddleView.get<Paddle>(paddleEntity);

            //$ ----- Paddle vs Walls ----- //
            // Check for 'ConfineToWindow' and limit paddle to window
            if (auto* bounds = registry->try_get<ConfineToWindow>(paddleEntity))
            {
                auto paddleBounds = paddleComp.shape.getGlobalBounds();

                float currentY = paddleComp.shape.getPosition().y;
                float halfWidth = paddleBounds.size.x / 2.0f;

                // West Wall
                // left side of the paddle
                if (paddleBounds.position.x < 0.0f)
                {
                    float leftLimitPad = bounds->padLeft + halfWidth;
                    paddleComp.shape.setPosition({ leftLimitPad, currentY });
                }
                // East Wall
                // right side of the paddle
                if (paddleBounds.position.x + paddleBounds.size.x > windowSize.x)
                {
                    float rightLimitPad = windowSize.x - (bounds->padRight + halfWidth);
                    paddleComp.shape.setPosition({ rightLimitPad, currentY });
                }
            }

            paddleBoundsList.push_back(paddleComp.shape.getGlobalBounds());
        }

        //$ ----- Brick Logic + Cache ----- //
        auto brickView = registry->view<Brick>();
        for (auto brickEntity : brickView)
        {
            auto& brickComp = registry->get<Brick>(brickEntity);
            sf::FloatRect brickBounds = brickComp.shape.getGlobalBounds();

            //$ Brick hitting bottom of window
            if (brickBounds.position.y + brickBounds.size.y >= windowSize.y)
            {
                triggerGameOver = true;
                logger::Info("Brick hit the bottom of the window.");
            }

            //$ Brick hitting paddle
            for (const auto& paddleBounds : paddleBoundsList)
            {
                if (paddleBounds.findIntersection(brickBounds))
                {
                    triggerGameOver = true;
                    logger::Info("Brick hit the paddle.");
                }
            }

            // If not game over, add brick bounds to vector cache
            if (!triggerGameOver)
            {
                brickCache.push_back({ brickEntity, brickBounds });
            }
        }

        //$ Check for game over after checking paddle/brick, brick/window collisions!
        if (triggerGameOver)
        {
            auto gameoverState = std::make_unique<GameTransitionState>(context);
            stateManager->replaceState(std::move(gameoverState));
            logger::Info("Game Over triggered.");
            return;
        }

        //$ ----- Ball Collision Logic ----- //
        auto ballView = registry->view<Ball, Velocity, MovementSpeed>();
        for (auto ballEntity : ballView)
        {
            auto& ballComp = registry->get<Ball>(ballEntity);
            auto& ballVelocity = registry->get<Velocity>(ballEntity);
            float ballSpeed = registry->get<MovementSpeed>(ballEntity).value;

            sf::Vector2f ballPosition = ballComp.shape.getPosition(); // Center
            float ballRadius = ballComp.shape.getRadius();

            //$ ----- Ball vs Walls ----- //
            // West Wall
            if (ballPosition.x - ballRadius < 0.0f)
            {
                playSound(context, Assets::SoundBuffers::WallHit);
                ballPosition.x = ballRadius;
                ballVelocity.value.x *= -1.0f;
            }
            // East Wall
            if (ballPosition.x + ballRadius > windowSize.x)
            {
                playSound(context, Assets::SoundBuffers::WallHit);
                ballPosition.x = windowSize.x - ballRadius;
                ballVelocity.value.x *= -1.0f;
            }
            // North Wall
            if (ballPosition.y - ballRadius < 0.0f)
            {
                playSound(context, Assets::SoundBuffers::WallHit);
                ballPosition.y = ballRadius;
                ballVelocity.value.y *= -1.0f;
            }
            // South Wall (Game over)
            if (ballPosition.y + ballRadius > windowSize.y)
            {
                triggerGameOver = true;
                logger::Info("Ball hit bottom of window.");
            }

            // Update position after wall checks
            ballComp.shape.setPosition(ballPosition);

            // Update bounds for object collision checks
            sf::FloatRect ballBounds = ballComp.shape.getGlobalBounds(); // treats circle as square

            //$ ----- Ball vs Paddle ----- //
            for (const auto& paddleBounds : paddleBoundsList)
            {
                if (ballBounds.findIntersection(paddleBounds))
                {
                    playSound(context, Assets::SoundBuffers::PaddleHit);

                    // calculate offset (-1 to 1)
                    // (ball - center) / half of paddle width
                    float paddleCenterX = paddleBounds.position.x + paddleBounds.size.x / 2.0f;
                    float ballCenterX = ballPosition.x;
                    float relativeIntersectX = (ballCenterX - paddleCenterX) /
                                               (paddleBounds.size.x / 2.0f);
                    // define angle for reflection
                    sf::Angle rotation = sf::degrees(relativeIntersectX * 60.0f);
                    // create new velocity based on 'straight up'
                    sf::Vector2f upVelocity = { 0.0f, -1.0f };
                    // rotate it by our angle
                    sf::Vector2f rotatedDirection = upVelocity.rotatedBy(rotation);
                    // apply it
                    ballVelocity.value = rotatedDirection * ballSpeed;
                }
            }

            //$ ----- Ball vs Bricks ----- //
            //auto brickView = registry->view<Brick, BrickScore, BrickHealth, BrickType>();
            for (const auto& cachedBrick : brickCache)
            {
                // safety check
                if (!registry->valid(cachedBrick.entity))
                {
                    continue;
                }

                if (auto intersection = ballBounds.findIntersection(cachedBrick.bounds))
                {
                    playSound(context, Assets::SoundBuffers::BrickHit);

                    auto& brickShape = registry->get<Brick>(cachedBrick.entity);
                    auto brickType = registry->get<BrickType>(cachedBrick.entity);

                    // Check if hit top or bottom
                    if (intersection->size.x > intersection->size.y)
                    {
                        // Check overlap
                        // If ball is above brick, move up. If below, move down.
                        if (ballBounds.position.y < cachedBrick.bounds.position.y)
                        {
                            ballComp.shape.move({0.f, -intersection->size.y});
                        }
                        else
                        {
                            ballComp.shape.move({0.f, intersection->size.y});
                        }
                        // reflect along y axis
                        ballVelocity.value.y = -ballVelocity.value.y;
                    }
                    // else we've hit left or right side
                    else
                    {
                        // Check overlap
                        // If ball is to right/left of brick, move accordingly
                        if (ballBounds.position.x < cachedBrick.bounds.position.x)
                        {
                            ballComp.shape.move({-intersection->size.x, 0.f});
                        }
                        else
                        {
                            ballComp.shape.move({intersection->size.x, 0.f});
                        }
                        // reflect along x axis
                        ballVelocity.value.x = -ballVelocity.value.x;
                    }

                    // // Log brick collision for debug
                    // float brickY = brickShape.shape.getPosition().y;
                    // float brickX = brickShape.shape.getPosition().x;
                    // logger::Info(std::format("Brick hit at position: ({},{})", brickX, brickY));

                    // Handle brick health
                    bool destroyed = false;
                    auto& brickHealth = registry->get<BrickHealth>(cachedBrick.entity).current;
                    brickHealth -= 1;
                    if (brickHealth <= 0)
                    {
                        destroyed = true;
                    }

                    if (destroyed)
                    {
                        // play appropriate brick destruction sound
                        if (brickType == BrickType::Normal)
                        {
                            playSound(context, Assets::SoundBuffers::NormBrickBreak);
                        }
                        else if (brickType == BrickType::Strong)
                        {
                            playSound(context, Assets::SoundBuffers::StrongBrickBreak);
                        }
                        else if (brickType == BrickType::Gold)
                        {
                            playSound(context, Assets::SoundBuffers::GoldBrickBreak);
                        }
                        else
                        {
                            playSound(context, Assets::SoundBuffers::NormBrickBreak);
                        }

                        // handle scoring
                        auto brickScoreValue = registry->get<BrickScore>(cachedBrick.entity);
                        auto scoreView = registry->view<HUDTag, ScoreHUDTag, CurrentScore, UIText>();
                        for (auto scoreEntity : scoreView)
                        {
                            auto& scoreText = scoreView.get<UIText>(scoreEntity);
                            auto& scoreCurrentValue = scoreView.get<CurrentScore>(scoreEntity);
                            scoreCurrentValue.value += brickScoreValue.value;
                            scoreText.text.setString(std::format("Score: {}", scoreCurrentValue.value));
                        }

                        // remove the non-paddle rectangle we've collided with
                        registry->destroy(cachedBrick.entity);

                        if (registry->view<Brick>().empty())
                        {
                            if (context.m_AppData.levelNumber >= context.m_AppData.totalLevels)
                            {
                                logger::Info("Completed the last level.");

                                auto gameState = std::make_unique<GameTransitionState>(context,
                                                 TransitionType::GameWin);
                                stateManager->replaceState(std::move(gameState));
                            }
                            else
                            {
                                logger::Info("All bricks destroyed. Level complete!");

                                auto winState = std::make_unique<GameTransitionState>(context,
                                                 TransitionType::LevelWin);
                                stateManager->replaceState(std::move(winState));
                            }

                            return;
                        }
                    }
                    //! We will need to handle this differently once there are more than one type
                    //! of "strong" brick
                    else
                    {
                        if (brickType == BrickType::Strong)
                        {
                            brickShape.shape.setFillColor(utils::loadColorFromConfig(
                                *context.m_ConfigManager, Assets::Configs::Bricks,
                                "strongDamaged", "strongDamagedRGB"));
                        }
                    }
                }
            }
        }

        // Check if game is over
        if (triggerGameOver)
        {
            auto gameoverState = std::make_unique<GameTransitionState>(context);
            stateManager->replaceState(std::move(gameoverState));
            logger::Info("Game Over triggered.");
        }
    }

    void renderSystem(entt::registry& registry, sf::RenderWindow& window, bool showDebug)
    {
        // Draw all Rectangles
        auto rectView = registry.view<Paddle>();
        for (auto entity : rectView)
        {
            auto& rectComp = rectView.get<Paddle>(entity);
            window.draw(rectComp.shape);
        }

        // Draw all Bricks
        auto brickView = registry.view<Brick>();
        for (auto entity : brickView)
        {
            auto& brickComp = brickView.get<Brick>(entity);
            window.draw(brickComp.shape);
        }

        // Draw all Circles
        auto circleView = registry.view<Ball>();
        for (auto entity : circleView)
        {
            auto& circleComp = circleView.get<Ball>(entity);
            window.draw(circleComp.shape);
        }
    }

    void playSound(AppContext& context, std::string_view soundID)
    {
        // remove sounds that are done playing
        context.m_AppData.activeSounds.remove_if([](const sf::Sound& sound) {
            return sound.getStatus() == sf::Sound::Status::Stopped;
            });

        // fetch sound data (SoundBuffer)
        auto* sound = context.m_ResourceManager->getResource<sf::SoundBuffer>(soundID);
        if (!sound)
        {
            logger::Warn(std::format("Sound ID \"{}\" not found!", soundID));
            return;
        }

        // Add it to the list and play it
        context.m_AppData.activeSounds.emplace_back(*sound);
        context.m_AppData.activeSounds.back().play();
    }

    void moveBricksDown(entt::registry& registry, float amount)
    {
        auto brickView = registry.view<Brick>();
        for (auto bricks : brickView)
        {
            auto& brick = brickView.get<Brick>(bricks);
            brick.shape.move({ 0.0f, amount });
        }
    }
}

namespace UISystems
{
    //$ --- UI Systems Implementation ---

    void uiHoverSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        auto view = registry.view<UIBounds>();

        for (auto entity : view)
        {
            const auto& bounds = view.get<UIBounds>(entity);
            if (bounds.rect.contains(mousePos))
            {
                registry.emplace_or_replace<UIHover>(entity);
            }
            else if (registry.all_of<UIHover>(entity))
            {
                registry.remove<UIHover>(entity);
            }
        }
    }

    void uiRenderSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        // Render shapes
        auto shapeView = registry.view<UIShape>();
        for (auto shapeEntity : shapeView)
        {
            auto& uiShape = shapeView.get<UIShape>(shapeEntity);

            // Change color on hover
            if (registry.all_of<UIHover>(shapeEntity))
            {
                uiShape.shape.setFillColor(sf::Color(100, 100, 255)); // Hover color
            }
            else
            {
                uiShape.shape.setFillColor(sf::Color::Blue); // Normal color
            }

            window.draw(uiShape.shape);
        }

        // Render text
        auto textView = registry.view<UIText>();
        for (auto textEntity : textView)
        {
            auto& uiText = textView.get<UIText>(textEntity);

            // Change text color on hover for interactive UI
            if (registry.any_of<UIAction, UIBounds>(textEntity))
            {
                if (registry.all_of<UIHover>(textEntity))
                {
                    uiText.text.setFillColor(sf::Color::White); // Hover text color
                }
                else
                {
                    uiText.text.setFillColor(sf::Color(200, 200, 200)); // Normal text color
                }
            }

            window.draw(uiText.text);
        }

        // Render UI buttons
        auto buttonView = registry.view<GUISprite>();
        for (auto buttonEntity : buttonView)
        {
            auto& button = buttonView.get<GUISprite>(buttonEntity);
            window.draw(button.sprite);
        }
    }

    void uiClickSystem(entt::registry& registry, const sf::Event::MouseButtonPressed& event)
    {
        if (event.button == sf::Mouse::Button::Left)
        {
            auto view = registry.view<UIHover, UIAction>();
            for (auto entity : view)
            {
                auto& clickable = view.get<UIAction>(entity);
                if (clickable.action)
                {
                    clickable.action();
                }
            }
        }
        else
        {
            // empty
        }
    }
}
