#pragma once

#include <glm/glm.hpp>

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include "glow/basic-types.hpp"


namespace tde
{
    using namespace glow::basicTypes;

    class Camera
    {
        [[nodiscard]] static auto lookAt(
            const glm::vec3 &eye,
            const glm::vec3 &center,
            const glm::vec3 &up
        ) -> glm::mat4;

    public:

        Camera() noexcept=default;

        auto updateKeysState(const sf::Event::KeyPressed *pressed) -> void;
        auto updateKeysState(const sf::Event::KeyReleased *released) -> void;
        auto updateDirection(const sf::Event::MouseMovedRaw *moved, f32 sensitivity = 0.1f) -> void;
        auto updateFov(const sf::Event::MouseWheelScrolled *scrolled)  noexcept -> void;

        [[nodiscard]] auto getPositionAndDirection(sf::Time delta) -> glm::mat4;
        [[nodiscard]] auto getFov() const noexcept -> f32;
        [[nodiscard]] auto getPos() const noexcept -> glm::vec3;

    private:

        bool m_WPressed{ false };
        bool m_APressed{ false };
        bool m_SPressed{ false };
        bool m_DPressed{ false };

        glm::vec3 m_pos{ 0.f, 0.f, 3.f };
        glm::vec3 m_front{ 0.0f, 0.0f, -1.f };
        glm::vec3 m_up{ 0.f , 1.f, 0.f };

        f32 m_yaw{ -90.f};
        f32 m_pitch{ 0.f };

        f32 m_fov{45.f};
    };
}