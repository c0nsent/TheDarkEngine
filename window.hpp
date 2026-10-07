#pragma once

#include "glow/basic-types.hpp"
using namespace glow::basicTypes;


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <expected>
#include <string>

namespace tde
{
    enum Key : i32
    {
        W = GLFW_KEY_W,
        A = GLFW_KEY_A,
        S = GLFW_KEY_S,
        D = GLFW_KEY_D,
        Escape = GLFW_KEY_ESCAPE,
    };

    enum KeyPressStatus
    {
        Pressed = GLFW_PRESS,
        Released = GLFW_RELEASE,
    };


    class Window
    {
    public:

        Window() noexcept;
        Window(const Window &other) noexcept = delete;
        Window(Window &&other) noexcept;
        ~Window() noexcept;

        static auto create(
            u32 width,
            u32 height,
            std::string_view title
        ) noexcept -> std::expected<Window, std::string>;

        [[nodiscard]] auto shouldClose() const noexcept -> bool;
        [[nodiscard]] auto getKey(Key k) const noexcept -> KeyPressStatus;

        [[nodiscard]] auto getRawPointer() const noexcept -> GLFWwindow *;
        void swapBuffer() const noexcept;
        static void pollEvents() noexcept;

    private:
        explicit Window(GLFWwindow *glfwWin) noexcept;

        GLFWwindow *m_window;
    };


}
