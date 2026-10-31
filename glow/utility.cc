#include "utility.hpp"

#include <glad/glad.h>


namespace glow {
    auto viewport(const sf::Vector2u& origin, const sf::Vector2u& res) noexcept -> void
    {
        glViewport(
            static_cast<i32>(origin.x),
            static_cast<i32>(origin.y),
            static_cast<i32>(res.x),
            static_cast<i32>(res.y)
        );
    }


    auto viewport(const sf::Vector2u& res) noexcept -> void
    {
        glViewport(
            0,
            0,
            static_cast<i32>(res.x),
            static_cast<i32>(res.y)
        );
    }


    auto clearScreen(const sf::Color& color) noexcept -> void
    {
        glClearColor(
            static_cast<f32>(color.r) / 255.f,
            static_cast<f32>(color.g) / 255.f,
            static_cast<f32>(color.b) / 255.f,
            static_cast<f32>(color.a) / 255.f
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}