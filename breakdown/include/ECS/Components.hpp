#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "Utilities/Utils.hpp"

#include <cstdint>
#include <functional>

//$ ----- Game Components ----- //

//$ Game object entity tags
struct PaddleTag {};
struct BrickTag {};
struct BallTag {};
struct RenderableTag {};

//$ Generic Components
struct Velocity { sf::Vector2f value{ 0.0f, 0.0f }; };

struct MovementSpeed { float value{ 0.0f }; };

//$ Ball component
struct Ball 
{
    Ball(float radius, const sf::Color& color, sf::Vector2f position)
        : shape()
    {
        shape.setRadius(radius);
        shape.setFillColor(color);
        shape.setOrigin({ radius, radius });
        shape.setPosition(position);
    }

    sf::CircleShape shape; 
};

//$ Paddle component
struct Paddle
{
    Paddle(sf::Vector2f size, const sf::Color& color, sf::Vector2f position)
        : shape()
    {
        shape.setSize(size);
        shape.setFillColor(color);
        utils::centerOrigin(shape);
        shape.setPosition(position);
    }

    sf::RectangleShape shape;
};

struct ConfineToWindow 
{
    float padLeft{ 1.0f };
    float padRight{ 1.0f };
};

//$ ----- Brick Components ----- //
enum class BrickType { Normal, Strong, Gold };
namespace BrickColors 
{
    using uint8 = std::uint8_t;
    // Perhaps we can get these from a TOML file later?
    struct Normal 
    {
        uint8 r{ 66 }; uint8 g{ 170 }; uint8 b{ 139 };
        sf::Color color{ r, g, b };
    };
    struct Strong 
    {
        uint8 r{ 87 }; uint8 g{ 117 }; uint8 b{ 144 };
        sf::Color color{ r, g, b };
    };
    struct StrongDamaged 
    {
        uint8 r{ 76 }; uint8 g{ 144 }; uint8 b{ 142 };
        sf::Color color{ r, g, b };
    };
    struct Gold 
    {
        uint8 r{ 249 }; uint8 g{ 199 }; uint8 b{ 79 };
        sf::Color color{ r, g, b };
    };
}

struct Brick
{
    Brick(sf::Vector2f size, const sf::Color& color, sf::Vector2f position)
        : shape()
    {
        shape.setSize(size);
        shape.setFillColor(color);
        shape.setPosition(position);
    }

    sf::RectangleShape shape;
};

struct BrickScore { int value{ 5 }; };
struct BrickHealth { int current{ 1 }; int max{ 1 }; };
struct ShowDamage {};

//$ ----- Game Data ----- //
struct CurrentScore { int value{ 0 }; };
struct CurrentLives { int value{ 3 }; };

//$ ----- UI Components -----

// Tag to identify UI/HUD entities
struct MenuUITag {};
struct HUDTag{};
struct ScoreHUDTag {};
struct LivesHUDTag {};

struct UIText { sf::Text text; };

struct UIShape { sf::RectangleShape shape; };

// Defines the clickable/hoverable area 
struct Bounds { sf::FloatRect rect; };

struct Clickable { std::function<void()> action; };

struct Hovered {};