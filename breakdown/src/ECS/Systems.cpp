#include "ECS/Systems.hpp"
#include "ECS/Components.hpp"
#include "Managers/StateManager.hpp"
#include "AppContext.hpp"

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include <memory>

namespace CoreSystems
{
    //$ "Core" / game systems (maybe rename...)
    void handlePlayerInput(entt::registry& registry, const sf::RenderWindow& window)
    {
        auto view = registry.view<PlayerTag, 
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

        auto rectView = registry.view<RenderableRect, Velocity>();
        for (auto entity : rectView)
        {
            auto& rectShape = rectView.get<RenderableRect>(entity);
            const auto& velocity = rectView.get<Velocity>(entity);

            rectShape.shape.move(velocity.value * deltaTime.asSeconds());
        }

        auto ballView = registry.view<RenderableCircle, Velocity>();
        for (auto entity : ballView)
        {
            auto& ballShape = ballView.get<RenderableCircle>(entity);
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
        
        // view of renderable rects
        auto rectView = registry.view<RenderableRect, Velocity>();
        for (auto entity : rectView)
        {
            auto& rectShape = rectView.get<RenderableRect>(entity);

            // Check 'collision' with player paddle and west/east walls
            // Check for 'ConfineToWindow' and limit paddle to window
            if (auto* bounds = registry.try_get<ConfineToWindow>(entity))
            {
                auto rectBounds = rectShape.shape.getGlobalBounds();

                float currentY = rectShape.shape.getPosition().y;
                float rectLeft = rectBounds.position.x;
                float rectRight = rectBounds.position.x + rectBounds.size.x;
                float halfWidth = rectBounds.size.x / 2.0f;

                // West Wall
                if (rectLeft < 0.0f)
                {
                    float leftLimitPad = bounds->padLeft + halfWidth;
                    rectShape.shape.setPosition({ leftLimitPad, currentY });
                }
                // East Wall
                if (rectRight > windowSize.x)
                {
                    float rightLimitPad = windowSize.x - (bounds->padRight + halfWidth);
                    rectShape.shape.setPosition({ rightLimitPad, currentY });
                }
            }
        }

        // Check ball collisions
        auto ballView = registry.view<RenderableCircle, Velocity, MovementSpeed>();
        auto targetView = registry.view<RenderableRect>();
        for (auto ballEntity : ballView)
        {
            // Ball properties
            auto& ballShape = registry.get<RenderableCircle>(ballEntity);
            auto& ballVelocity = registry.get<Velocity>(ballEntity);
            sf::Vector2f ballPosition = ballShape.shape.getPosition(); // Center
            float r = ballShape.shape.getRadius();
            sf::FloatRect ballBounds = ballShape.shape.getGlobalBounds(); // treats circle as square
            float ballSpeed = registry.get<MovementSpeed>(ballEntity).value; 

            //$ ----- Wall collisions ----- //
            // West Wall
            if (ballPosition.x - r < 0.0f) 
            {
                ballPosition.x = r;                 
                ballVelocity.value.x *= -1.0f;
            }
            // East Wall
            if (ballPosition.x + r > windowSize.x) 
            {
                ballPosition.x = windowSize.x - r;
                ballVelocity.value.x *= -1.0f;
            }
            // North Wall
            if (ballPosition.y - r < 0.0f) 
            {
                ballPosition.y = r;
                ballVelocity.value.y *= -1.0f;
            }
            // South Wall (Game over)
            if (ballPosition.y + r > windowSize.y) 
            {
                auto gameoverState = std::make_unique<GameOverState>(m_AppContext);
                stateManager.replaceState(std::move(gameoverState));
            }

            ballShape.shape.setPosition(ballPosition);

            // Check for collision with rectangles
            for (auto targetEntity : targetView)
            {
                auto& targetShape = registry.get<RenderableRect>(targetEntity);
                sf::FloatRect targetBounds = targetShape.shape.getGlobalBounds();

                if (auto intersection = ballBounds.findIntersection(targetBounds))
                {
                    // if ball hits paddle, angle the reflection
                    if (registry.any_of<PlayerTag>(targetEntity))
                    {
                        float targetCenterX = targetShape.shape.getPosition().x;
                        float ballCenterX = ballPosition.x;
                        // calculate offset (-1 to 1)
                        // (ball - center) / half of paddle width
                        float relativeIntersectX = (ballCenterX - targetCenterX) / 
                                                   (targetBounds.size.x / 2.0f);
                        // define angle for reflection
                        sf::Angle rotation = sf::degrees(relativeIntersectX * 60.0f);
                        // create the new velocity based on 'straight up'
                        sf::Vector2f upVelocity = { 0.0f, -1.0f };
                        // rotate it by our angle
                        sf::Vector2f rotatedDirection = upVelocity.rotatedBy(rotation);
                        // apply it!
                        ballVelocity.value = upVelocity.rotatedBy(rotation) * ballSpeed;
                    }
                    // else we've hit a non-paddle rectangle (a brick)
                    else  
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

                        // remove the non-paddle rectangle we've collided with
                        float brickX = targetShape.shape.getPosition().x;
                        float brickY = targetShape.shape.getPosition().y;

                        logger::Info(std::format("Brick hit at position: ({},{})", brickX, brickY));
                        registry.destroy(targetEntity);
                    }
                }
            }
        }
    }
    
    void renderSystem(entt::registry& registry, sf::RenderWindow& window, bool showDebug)
    {
        // Draw all Rectangles
        auto rectView = registry.view<RenderableRect>();
        for (auto entity : rectView)
        {
            auto& rectComp = rectView.get<RenderableRect>(entity);
            window.draw(rectComp.shape);
        }

        // Draw all Circles
        auto circleView = registry.view<RenderableCircle>();
        for (auto entity : circleView)
        {
            auto& circleComp = circleView.get<RenderableCircle>(entity);
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
        for (auto entity : shapeView)
        {
            auto& uiShape = shapeView.get<UIShape>(entity);
            
            // Change color on hover
            if (registry.all_of<Hovered>(entity))
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
        for (auto entity : textView)
        {
            auto& uiText = textView.get<UIText>(entity);

            // Change text color on hover
            if (registry.all_of<Hovered>(entity))
            {
                uiText.text.setFillColor(sf::Color::White); // Hover text color
            }
            else
            {
                uiText.text.setFillColor(sf::Color(200, 200, 200)); // Normal text color
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