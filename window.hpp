#pragma once

#include "glow/basic-types.hpp"
using namespace glow::basicTypes;


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <expected>
#include <string>

namespace tde
{

    class Camera;

    enum Key : i32
    {
        Q = GLFW_KEY_Q,
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
        auto operator=(Window &&other) noexcept -> Window &;
        ~Window() noexcept;

        static auto create(
            u32 width,
            u32 height,
            std::string_view title
        ) noexcept -> std::expected<Window, std::string>;

        [[nodiscard]] auto shouldClose() const noexcept -> bool;
        [[nodiscard]] auto getKey(Key k) const noexcept -> KeyPressStatus;
        [[nodiscard]] auto createCamera() const noexcept -> std::expected<Camera, std::string>;

        [[nodiscard]] auto getRawPointer() const noexcept -> GLFWwindow *;
        void swapBuffer() const noexcept;
        static void pollEvents() noexcept;

    private:
        explicit Window(GLFWwindow *glfwWin) noexcept;

        GLFWwindow *m_window;
    };


}
