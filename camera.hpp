#pragma once

#include "window.hpp"

#include <glm/glm.hpp>


namespace tde
{
    class Camera
    {

        friend Window;

    public:

        explicit Camera(const Window &win) noexcept;

    private:

        static void cursorPosCallback(GLFWwindow *window, f64 xPos, f64 yPos);
        static void scrollCallback(GLFWwindow *window, f64, f64 yOffset);
        static void keyCallback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods);

        glm::vec3 m_camPos{ 0.f, 0.f, 3.f };
        glm::vec3 m_camFront{ 0.0f, 0.0f, -1.f };
        glm::vec3 m_camUp{ 0.f , 1.f, 0.f };

        f32 m_yaw{ -90.f};
        f32 m_pitch{ 0.f };
        glm::vec2 m_lastPos{};
        bool m_firstMouseInput{true};
        f32 m_fov{45.f};

        const Window *const m_window;
    };
}