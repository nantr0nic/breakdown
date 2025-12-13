#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "Utilities/Utils.hpp"

#include <functional>

//$ ----- Game Components ----- //

//$ Tags

struct PlayerTag {}; // Tag to identify the player entity
struct RenderableTag {}; // Tag to identify renderable entities

//$ Components

struct Velocity { sf::Vector2f value{ 0.0f, 0.0f }; };

struct MovementSpeed { float value{ 0.0f }; };

struct ConfineToWindow 
{
    float padLeft{ 1.0f };
    float padRight{ 1.0f };
};

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

//$ ----- Brick Components ----- //
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