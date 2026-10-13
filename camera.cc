#include "camera.hpp"


#include <algorithm>
#include <glm/ext/matrix_transform.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>


namespace tde
{
    Camera::Camera() noexcept
        : m_lastPos{ sf::Mouse::getPosition().x, sf::Mouse::getPosition().y }
    {
    }

    auto Camera::calcPosition(const sf::Time delta) -> glm::mat4
    {
        const f32 camSpeed{ static_cast<f32>(delta.asMicroseconds())/ 10000000.f };

        using namespace sf::Keyboard;

        if (isKeyPressed(Key::W))
        {
            m_camPos += camSpeed * m_camFront;
        }
        if (isKeyPressed(Key::S))
        {
            m_camPos -= camSpeed * m_camFront;
        }
        if (isKeyPressed(Key::A))
        {
            m_camPos -= glm::normalize(glm::cross(m_camFront, m_camUp)) * camSpeed;
        }
        if (isKeyPressed(Key::D))
        {
            m_camPos += glm::normalize(glm::cross(m_camFront, m_camUp)) * camSpeed;
        }

        return glm::lookAt(m_camPos, m_camPos + m_camFront, m_camUp);
    }

    auto Camera::calcDirection() -> void
    {
        const glm::vec2 current{sf::Mouse::getPosition().x, sf::Mouse::getPosition().y};
        auto offset {current - m_lastPos};
        m_lastPos = current;

        constexpr f32 sensitivity{ 0.1f };
        offset *= sensitivity;

        m_yaw += offset.x;
        m_yaw += offset.y;

        m_pitch = std::clamp(m_pitch, -89.f, 89.f);

        glm::vec3 direction;
        direction.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        direction.y = glm::sin(glm::radians(m_pitch));
        direction.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        m_camFront = glm::normalize(direction);
    }
}
