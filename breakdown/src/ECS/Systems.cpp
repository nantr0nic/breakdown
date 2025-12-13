#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <entt/entt.hpp>

#include "ECS/Systems.hpp"
#include "ECS/Components.hpp"
#include "Managers/StateManager.hpp"
#include "AppContext.hpp"

#include <memory>
#include <format>

namespace CoreSystems
{
    //$ "Core" / game systems (maybe rename...)
    void handlePlayerInput(entt::registry& registry, const sf::RenderWindow& window)
    {
        auto view = registry.view<PaddleTag, 
                                Velocity, 
                                MovementSpeed>();

        for (auto entity : view)
        {
            auto& velocity = view.get<Velocity>(entity);
            const auto& speed = view.get<MovementSpeed>(entity);

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
        }
    }

    void movementSystem(entt::registry& registry, sf::Time deltaTime, sf::RenderWindow& window)
    {
        // cache window size
        auto windowSize = window.getSize();

        auto rectView = registry.view<Paddle, Velocity>();
        for (auto entity : rectView)
        {
            auto& rectShape = rectView.get<Paddle>(entity);
            const auto& velocity = rectView.get<Velocity>(entity);

            rectShape.shape.move(velocity.value * deltaTime.asSeconds());
        }

        auto ballView = registry.view<Ball, Velocity>();
        for (auto entity : ballView)
        {
            auto& ballShape = ballView.get<Ball>(entity);
            const auto& velocity = ballView.get<Velocity>(entity);

            ballShape.shape.move(velocity.value * deltaTime.asSeconds());
        }

    }

    void collisionSystem(AppContext* m_AppContext, sf::Time deltaTime)
    {
        auto& registry = *m_AppContext->m_Registry;
        auto& window = *m_AppContext->m_MainWindow;
        auto& stateManager = *m_AppContext->m_StateManager;

        // cache window size
        auto windowSize = window.getSize();
        
        //$ --- Paddle vs Window Walls --- //
        // Confine paddle to window (wall collision)
        auto paddleView = registry.view<Paddle, Velocity>();
        for (auto entity : paddleView)
        {
            auto& paddleShape = paddleView.get<Paddle>(entity);

            // Check for 'ConfineToWindow' and limit paddle to window
            if (auto* bounds = registry.try_get<ConfineToWindow>(entity))
            {
                auto paddleBounds = paddleShape.shape.getGlobalBounds();

                float currentY = paddleShape.shape.getPosition().y;
                float paddleLeft = paddleBounds.position.x;
                float paddleRight = paddleBounds.position.x + paddleBounds.size.x;
                float halfWidth = paddleBounds.size.x / 2.0f;

                // West Wall
                if (paddleLeft < 0.0f)
                {
                    float leftLimitPad = bounds->padLeft + halfWidth;
                    paddleShape.shape.setPosition({ leftLimitPad, currentY });
                }
                // East Wall
                if (paddleRight > windowSize.x)
                {
                    float rightLimitPad = windowSize.x - (bounds->padRight + halfWidth);
                    paddleShape.shape.setPosition({ rightLimitPad, currentY });
                }
            }
        }

        //$ ----- Ball Collision Logic ----- //
        auto ballView = registry.view<Ball, Velocity, MovementSpeed>();
        for (auto ballEntity : ballView)
        {
            // Ball properties
            auto& ballComp = registry.get<Ball>(ballEntity);
            auto& ballVelocity = registry.get<Velocity>(ballEntity);
            float ballSpeed = registry.get<MovementSpeed>(ballEntity).value; 

            sf::Vector2f ballPosition = ballComp.shape.getPosition(); // Center
            float ballRadius = ballComp.shape.getRadius();

            //$ ----- Ball vs Walls ----- //
            // West Wall
            if (ballPosition.x - ballRadius < 0.0f) 
            {
                ballPosition.x = ballRadius;                 
                ballVelocity.value.x *= -1.0f;
            }
            // East Wall
            if (ballPosition.x + ballRadius > windowSize.x) 
            {
                ballPosition.x = windowSize.x - ballRadius;
                ballVelocity.value.x *= -1.0f;
            }
            // North Wall
            if (ballPosition.y - ballRadius < 0.0f) 
            {
                ballPosition.y = ballRadius;
                ballVelocity.value.y *= -1.0f;
            }
            // South Wall (Game over)
            if (ballPosition.y + ballRadius > windowSize.y) 
            {
                auto gameoverState = std::make_unique<GameOverState>(m_AppContext);
                stateManager.replaceState(std::move(gameoverState));
            }

            // Update position after wall checks
            ballComp.shape.setPosition(ballPosition);

            // Update bounds for object collision checks
            sf::FloatRect ballBounds = ballComp.shape.getGlobalBounds(); // treats circle as square
        

            //$ ----- Ball vs Paddle ----- //
            auto paddleView = registry.view<Paddle>();
            for (auto paddleEntity : paddleView)
            {
                auto& paddleShape = registry.get<Paddle>(paddleEntity);
                sf::FloatRect paddleBounds = paddleShape.shape.getGlobalBounds();

                if (ballBounds.findIntersection(paddleBounds))
                {
                    float paddleCenterX = paddleShape.shape.getPosition().x;
                    float ballCenterX = ballPosition.x;
                    // calculate offset (-1 to 1)
                    // (ball - center) / half of paddle width
                    float relativeIntersectX = (ballCenterX - paddleCenterX) / 
                                                (paddleBounds.size.x / 2.0f);
                    // define angle for reflection
                    sf::Angle rotation = sf::degrees(relativeIntersectX * 60.0f);
                    // create the new velocity based on 'straight up'
                    sf::Vector2f upVelocity = { 0.0f, -1.0f };
                    // rotate it by our angle
                    sf::Vector2f rotatedDirection = upVelocity.rotatedBy(rotation);
                    // apply it!
                    ballVelocity.value = upVelocity.rotatedBy(rotation) * ballSpeed;
                }
            }
                    
            //$ ----- Ball vs Bricks ----- //
            auto brickView = registry.view<Brick, BrickScore, BrickHealth, BrickType>();
            for (auto brickEntity : brickView)
            {
                // Handle collision/bounce
                auto& brickShape = brickView.get<Brick>(brickEntity);
                sf::FloatRect brickBounds = brickShape.shape.getGlobalBounds();

                if (auto intersection = ballBounds.findIntersection(brickBounds))
                {
                    // Check if hit top or bottom
                    if (intersection->size.x > intersection->size.y)
                    {
                        // reflect along y axis
                        ballVelocity.value.y = -ballVelocity.value.y;
                    }
                    // else we've hit left or right side
                    else
                    {
                        // reflect along x axis
                        ballVelocity.value.x = -ballVelocity.value.x;
                    }

                    // // Log brick collision for debug
                    // float brickY = brickShape.shape.getPosition().y;
                    // float brickX = brickShape.shape.getPosition().x;
                    // logger::Info(std::format("Brick hit at position: ({},{})", brickX, brickY));

                    // Handle brick health
                    bool destroyed = false;
                    auto& brickHealth = brickView.get<BrickHealth>(brickEntity).current;
                    brickHealth -= 1;
                    if (brickHealth <= 0)
                    {
                        destroyed = true;
                    }
                    
                    if (destroyed)
                    {
                        // increment score
                        auto scoreView = registry.view<HUDTag, ScoreHUDTag, CurrentScore, UIText>();
                        for (auto scoreEntity : scoreView)
                        {
                            auto& scoreText = scoreView.get<UIText>(scoreEntity);
                            auto& scoreCurrentValue = scoreView.get<CurrentScore>(scoreEntity);
                            auto& brickScoreValue = brickView.get<BrickScore>(brickEntity);

                            scoreCurrentValue.value += brickScoreValue.value;

                            scoreText.text.setString(std::format("Score: {}", scoreCurrentValue.value));
                        }

                        // remove the non-paddle rectangle we've collided with
                        registry.destroy(brickEntity);
                    }
                    //! We will need to handle this differently once there are more than one type
                    //! of "strong" brick
                    else 
                    {
                        BrickType brickType = brickView.get<BrickType>(brickEntity);
                        if (brickType == BrickType::Strong)
                        {
                            brickShape.shape.setFillColor(BrickColors::StrongDamaged);
                        }
                    }
                }
            }
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
}

namespace UISystems
{
    //$ --- UI Systems Implementation ---

    void uiHoverSystem(entt::registry& registry, sf::RenderWindow& window)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        auto view = registry.view<Bounds>();

        for (auto entity : view)
        {
            const auto& bounds = view.get<Bounds>(entity);
            if (bounds.rect.contains(mousePos))
            {
                registry.emplace_or_replace<Hovered>(entity);
            }
            else if (registry.all_of<Hovered>(entity))
            {
                registry.remove<Hovered>(entity);
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
            if (registry.all_of<Hovered>(shapeEntity))
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
            if (registry.any_of<Clickable, Bounds>(textEntity))
            {
                if (registry.all_of<Hovered>(textEntity))
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
    }

    void uiClickSystem(entt::registry& registry, const sf::Event::MouseButtonPressed& event)
    {
        if (event.button == sf::Mouse::Button::Left)
        {
            auto view = registry.view<Hovered, Clickable>();
            for (auto entity : view)
            {
                auto& clickable = view.get<Clickable>(entity);
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