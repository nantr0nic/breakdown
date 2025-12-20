#pragma once

#include <SFML/Graphics.hpp>

struct SpritePadding
{
    float left{ 0.0f };
    float right{ 0.0f };
    float top{ 0.0f };
    float bottom{ 0.0f };
};

namespace utils
{
    template <typename T>
    void centerOrigin(T& item)
    {
        sf::FloatRect bounds = item.getLocalBounds();
        item.setOrigin(bounds.getCenter());
    }

    SpritePadding getSpritePadding(const sf::Sprite& sprite);
}