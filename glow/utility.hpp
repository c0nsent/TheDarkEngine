#pragma once

#include <SFML/System/Vector2.hpp>

#include "basic-types.hpp"


namespace glow {
    auto viewport(const sf::Vector2u &origin, const sf::Vector2u &res) noexcept -> void;

    auto viewport(const sf::Vector2u &res) noexcept -> void;
}
