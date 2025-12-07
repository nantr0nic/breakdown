#include "ECS/Systems.hpp"
#include "ECS/Components.hpp"
#include "AppContext.hpp"

namespace CoreSystems
{
    //$ "Core" / game systems (maybe rename...)
    void handlePlayerInput(AppContext* m_AppContext)
    {
        auto &registry = *m_AppContext->m_Registry;
        auto &window = *m_AppContext->m_MainWindow;

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
        auto view = registry.view<RenderableRect, Velocity>();

        for (auto entity : view)
        {
            auto& rectShape = view.get<RenderableRect>(entity);
            const auto& velocity = view.get<Velocity>(entity);

            rectShape.shape.move(velocity.value * deltaTime.asSeconds());

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
    }

    
    void renderSystem(entt::registry& registry, sf::RenderWindow& window, bool showDebug)
    {
        // now renders anything with a sprite
        auto view = registry.view<RenderableRect>();
        for (auto entity : view)
        {
            const auto& rectShape = view.get<RenderableRect>(entity);
            window.draw(rectShape.shape);
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