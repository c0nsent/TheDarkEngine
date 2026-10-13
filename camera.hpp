#pragma once

#include <glm/glm.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include "glow/basic-types.hpp"


namespace tde
{
    using namespace glow::basicTypes;

    class Camera
    {

    public:

        Camera() noexcept;
        auto calcPosition(sf::Time delta) -> glm::mat4;
        auto calcDirection() -> void;

    private:

        glm::vec3 m_camPos{ 0.f, 0.f, 3.f };
        glm::vec3 m_camFront{ 0.0f, 0.0f, -1.f };
        glm::vec3 m_camUp{ 0.f , 1.f, 0.f };

        f32 m_yaw{ -90.f};
        f32 m_pitch{ 0.f };
        glm::vec2 m_lastPos;
        bool m_firstMouseInput{true};
    };
}