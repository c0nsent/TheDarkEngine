#include "camera.hpp"

#include <algorithm>
#include <glm/ext/matrix_transform.hpp>
#include <SFML/Window/Keyboard.hpp>


namespace tde
{
    auto Camera::lookAt(const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up) -> glm::mat4
    {
        using namespace glm;

        const auto f{ normalize(center - eye) };
        const auto r{ normalize(cross(f, up)) };
        const auto u{ cross(r, f) };

        return {
            r.x, u.x, -f.x, 0,
            r.y, u.y, -f.y, 0,
            r.z, u.z, -f.z, 0,
            -dot(r, eye), -dot(u, eye), dot(f, eye), 1
        };
    }

    auto Camera::updateKeysState(const sf::Event::KeyPressed *pressed) -> void
    {
        using namespace sf::Keyboard;

        switch (pressed->code)
        {
        case Key::W:
            m_WPressed = true;
            break;

        case Key::A:
            m_APressed = true;
            break;

        case Key::S:
            m_SPressed = true;
            break;

        case Key::D:
            m_DPressed = true;
            break;
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
            break;

        case Key::A:
            m_APressed = false;
            break;

        case Key::S:
            m_SPressed = false;
            break;

        case Key::D:
            m_DPressed = false;
        default:
            return;
        }
    }

    auto Camera::updateDirection(const sf::Event::MouseMovedRaw *moved, const f32 sensitivity) -> void
    {
        const glm::vec2 offset{
            static_cast<f32>(moved->delta.x) * sensitivity,
            static_cast<f32>(moved->delta.y) * sensitivity
        };

        m_yaw += offset.x;
        m_pitch = std::clamp(m_pitch - offset.y, -89.f, 89.f);

        const auto yawRad{ glm::radians(m_yaw) };
        const auto pitchRad { glm::radians(m_pitch) };
        const auto pitchCos { glm::cos(pitchRad) };

        const glm::vec3 direction {
            glm::cos(yawRad) * pitchCos,
            glm::sin(pitchRad),
            glm::sin(yawRad) * pitchCos
        };

        m_front = glm::normalize(direction);
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
            m_pos += camSpeed * m_front;
        if (m_SPressed)
            m_pos -= camSpeed * m_front;
        if (m_APressed)
            m_pos -= glm::normalize(glm::cross(m_front, m_up)) * camSpeed;
        if (m_DPressed)
            m_pos += glm::normalize(glm::cross(m_front, m_up)) * camSpeed;

        return lookAt(m_pos, m_pos + m_front, m_up);
    }

    auto Camera::getFov() const noexcept -> f32 { return m_fov; }
}
