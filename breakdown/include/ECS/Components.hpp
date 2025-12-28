#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "Utilities/Utils.hpp"

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
enum class BrickType { Normal, Strong, Gold, Custom_1, Custom_2 };

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

//$ ----- Game Data ----- //
struct CurrentScore { int value{ 0 }; };

//$ ----- UI Components ----- //
enum class UITags { None, Menu, Settings, Transition };

// Tag to identify UI/HUD entities
struct MenuUITag {};
struct SettingsUITag {};
struct TransUITag {};

struct HUDTag {};
struct ScoreHUDTag {};
struct GUIButtonTag {};

struct UIHover {};

struct UIText { sf::Text text; };

struct UIShape { sf::RectangleShape shape; };

struct UIBounds { sf::FloatRect rect; };

struct UIAction { std::function<void()> action; };

struct UIToggleCond { std::function<bool()> shouldShowOverlay; };

struct GUISprite { sf::Sprite sprite; };

struct GUIRedX { sf::Sprite sprite; };

struct GUITexture { sf::Texture texture; };
