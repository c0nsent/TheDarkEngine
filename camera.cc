#include "camera.hpp"

#include <algorithm>


namespace tde
{
    Camera::Camera(const Window &win) noexcept
        : m_window{ &win }
    {
        i32 width, height;
        glfwGetWindowSize(win.getRawPointer(), &width, &height);
        m_lastPos = { width, height };

        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(win.getRawPointer(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        glfwSetWindowUserPointer(win.getRawPointer(), this);
        glfwSetCursorPosCallback(win.getRawPointer(), cursorPosCallback);
        glfwSetScrollCallback(win.getRawPointer(), scrollCallback);
        glfwSetKeyCallback(win.getRawPointer(), keyCallback);
    }

    void Camera::cursorPosCallback(GLFWwindow* window, f64 xPos, f64 yPos)
    {
        auto cam{ static_cast<Camera *>(glfwGetWindowUserPointer(window)) };

        if (cam->m_firstMouseInput)
        {
            cam->m_lastPos = { xPos, yPos };
            cam->m_firstMouseInput = false;
        }

        glm::vec2 offset{ xPos - cam->m_lastPos.x, cam->m_lastPos.y - yPos };
        cam->m_lastPos = { xPos, yPos };

        constexpr f32 sensitivity{ 0.1f };
        offset *= sensitivity;

        cam->m_yaw += offset.x;
        cam->m_pitch += offset.y;

        cam->m_pitch = std::clamp(cam->m_pitch, -89.f, 89.f);


        const glm::vec3 direction {
            glm::cos(glm::radians(cam->m_yaw)) * glm::cos(glm::radians(cam->m_pitch)),
            glm::sin(glm::radians(cam->m_pitch)),
            glm::sin(glm::radians(cam->m_yaw)) * glm::cos(glm::radians(cam->m_pitch))
        };

        cam->m_camFront = glm::normalize(direction);
    }

    void Camera::scrollCallback(GLFWwindow* window, f64, const f64 yOffset)
    {
        auto cam{ static_cast<Camera *>(glfwGetWindowUserPointer(window)) };

        cam->m_fov -= static_cast<f32>(yOffset);
        cam->m_fov = std::clamp(cam->m_fov, 1.f, 45.f);
    }

    void Camera::keyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
    {
        auto cam{ static_cast<Camera *>(glfwGetWindowUserPointer(window)) };

        static f32 lastFrame{};
        const f32 currentFrame{ static_cast<f32>(glfwGetTime()) };
        static f32 deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        const f32 cameraSpeed = 2.5f * deltaTime;

        if (cam->m_window->getKey(tde::Key::W))
        {
            cam->m_camPos += cameraSpeed * cam->m_camFront;
        }
    }
}
