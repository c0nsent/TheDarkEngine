#pragma once

#include "glow/basic-types.hpp"
using namespace glow::basicTypes;


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <expected>
#include <string>


namespace tde
{
    class Window
    {

    public:

        Window() noexcept = default;
        Window(const Window &other) noexcept = default;
        ~Window() noexcept;

        static auto create(
            u32 width,
            u32 height,
            std::string_view title
        ) noexcept -> std::expected<Window, std::string>;

        auto getRawPointer() const noexcept -> GLFWwindow *;
        void swapBuffer() const noexcept;
        static void pollEvents() noexcept;

    private:
        explicit Window(GLFWwindow *glfwWin) noexcept;

        GLFWwindow *m_window;
    };


}
