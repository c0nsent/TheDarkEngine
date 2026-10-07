#include "window.hpp"

#include <iostream>

namespace tde
{
    auto Window::create(
        const u32 width,
        const u32 height,
        const std::string_view title
    ) noexcept -> std::expected<Window, std::string>
    {
        glfwSetErrorCallback([] (const i32 error, const char *description)
        {
            std::cerr << "[GLFW] Error " << error << ": "
                << (description ? description : "unknown error") << '\n';
        });

        if (not glfwInit()) return std::unexpected{ "Failed to initialize GLFW" };

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        Window window {
            glfwCreateWindow(
                static_cast<i32>(width),
                static_cast<i32>(height),
                title.data(),
                glfwGetPrimaryMonitor(),
                nullptr
                )
        };

        if (not window.m_window)
        {
            glfwTerminate();
            return  std::unexpected{"Failed to create window"};
        }

        glfwMakeContextCurrent(window.m_window);
        glfwSetFramebufferSizeCallback(window.m_window,
            [] (GLFWwindow *, const i32 width, const i32 height)
            {
                glViewport(0, 0, width, height);
            }
        );


        if (not gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            return std::unexpected{"Failed to initialize GLAD"};
        };

        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(window.m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        return window;
    }

    void Window::swapBuffer() const noexcept
    {
        glfwSwapBuffers(this->m_window);
    }

    void Window::pollEvents() const noexcept
    {
        glfwPollEvents();
    }

    Window::Window(GLFWwindow* glfwWin) noexcept
        : m_window{ glfwWin }
    {}
}


