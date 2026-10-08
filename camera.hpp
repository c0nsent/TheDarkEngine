#pragma once

#include "window.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace tde
{
    class Camera
    {
        friend Window;

    private:

        glm::vec3 m_camPos{ 0.f, 0.f, 3.f };
        glm::vec3 m_camFront{ 0.0f, 0.0f, -1.f };
        glm::vec3 m_camUp{ 0.f , 1.f, 0.f };

        f32 m_yaw{ -90.f};
        f32 pitch{ 0.f };
        glm::vec2 lastPos = { WIDTH / 2, HEIGHT / 2 };
        bool firstMouseInput = true;
        f32 zoom = 45.f;

    };
}