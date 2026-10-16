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

    auto Camera::updateKeysState(const sf::Event::KeyPressed *pressed) -> void
    {
        using namespace sf::Keyboard;

        switch (pressed->code)
        {
        case Key::W:
            m_WPressed = true;
            return;

        case Key::A:
            m_APressed = true;
            return;

        case Key::S:
            m_SPressed = true;
            return;

        case Key::D:
            m_DPressed = true;
        default:
            return;
        }
    }

    auto Camera::updateKeysState(const sf::Event::KeyReleased *released) -> void
    {
        using namespace sf::Keyboard;

        switch (released->code)
        {
        case Key::W:
            m_WPressed = false;
            return;

        case Key::A:
            m_APressed = false;
            return;

        case Key::S:
            m_SPressed = false;
            return;

        case Key::D:
            m_DPressed = false;
        default:
            return;
        }
    }

    auto Camera::updateDirection(const sf::Event::MouseMoved *moved) -> void
    {
        const glm::vec2 current{moved->position.x, moved->position.y};

        auto offset {current - m_lastPos};
        m_lastPos = current;

        constexpr f32 sensitivity{ 0.1f };
        offset *= sensitivity;

        m_yaw += offset.x;
        m_pitch = std::clamp(m_pitch - offset.y, -89.f, 89.f);

        glm::vec3 direction;
        direction.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        direction.y = glm::sin(glm::radians(m_pitch));
        direction.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
        m_camFront = glm::normalize(direction);
    }

    auto Camera::updateFov(const sf::Event::MouseWheelScrolled *scrolled) noexcept -> void
    {
        m_fov = std::clamp( m_fov - scrolled->delta, 30.f, 90.f  );
    }

    auto Camera::getPositionAndDirection(const sf::Time delta) -> glm::mat4
    {
        const f32 camSpeed{ 2.5f * delta.asSeconds()};

        using namespace sf::Keyboard;

        if (m_WPressed)
        {
            m_camPos += camSpeed * m_camFront;
        }
        if (m_SPressed)
        {
            m_camPos -= camSpeed * m_camFront;
        }
        if (m_APressed)
        {
            m_camPos -= glm::normalize(glm::cross(m_camFront, m_camUp)) * camSpeed;
        }
        if (m_DPressed)
        {
            m_camPos += glm::normalize(glm::cross(m_camFront, m_camUp)) * camSpeed;
        }

        return glm::lookAt(m_camPos, m_camPos + m_camFront, m_camUp);
    }

    auto Camera::getFov() const noexcept -> f32
    {
        return m_fov;
    }
}
