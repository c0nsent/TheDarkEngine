#include "window.hpp"

#include <iostream>
#include <utility>

namespace tde
{
    Window::Window() noexcept
    {
    }

    Window::Window(Window&& other) noexcept
        : m_window{other.m_window}
    {
        other.m_window = nullptr;
    }

    Window::~Window() noexcept
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    auto Window::create(
        const u32 width,
        const u32 height,
        const std::string_view title
    ) noexcept -> std::expected<Window, std::string>
    {
        glfwSetErrorCallback([] (const i32 error, const char *description)
        {
            if (description == nullptr) description = "unknown error";

            std::cerr << "[GLFW] Error " << error << ": " << description << '\n';
        });

        if (not glfwInit()) return std::unexpected{ "Failed to initialize GLFW" };

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        auto glfwWindow {
            glfwCreateWindow(
                static_cast<i32>(width),
                static_cast<i32>(height),
                title.data(),
                glfwGetPrimaryMonitor(),
                nullptr
                )
        };

        Window window {};
        window.m_window = glfwWindow;

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

        return std::move(window);
    }

    auto Window::shouldClose() const noexcept -> bool
    {
        return glfwWindowShouldClose(this->m_window);
    }

    auto Window::getKey(const Key k) const noexcept -> KeyPressStatus
    {
        return static_cast<KeyPressStatus>(glfwGetKey(this->m_window, std::to_underlying(k)));
    }

    auto Window::getRawPointer() const noexcept -> GLFWwindow*
    {
        return this->m_window;
    }

    void Window::swapBuffer() const noexcept
    {
        glfwSwapBuffers(this->m_window);
    }

    void Window::pollEvents() noexcept
    {
        glfwPollEvents();
    }

    Window::Window(GLFWwindow* glfwWin) noexcept
        : m_window{ glfwWin }
    {
    }
}


