#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "glow/basic-types.hpp"
#include "glow/shader-program.hpp"
using namespace glow::basicTypes;
#include "glow/shader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>


constexpr u32 WIDTH = 800;
constexpr u32 HEIGHT = 600;

auto errorCallback(const i32 error, const char *description) -> void
{
    std::cerr << "Error: " << error << " " << description << std::endl;
}


auto framebufferSizeCallback(GLFWwindow *, const i32 width, const i32 height) -> void
{
    glViewport(0, 0, width, height);
}




auto processInput(GLFWwindow *window) -> void
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}


static auto createTexture(const char *path) -> u32
{
    u32 textureId;
    glGenTextures(1, &textureId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    i32 width, height, nrChannels;
    u8 *imageData{stbi_load(path, &width, &height, &nrChannels, 0)};

    const i32 format{nrChannels == 3 ? GL_RGB : GL_RGBA};

    if (not imageData)
        throw std::runtime_error{"Failed to load image\n"};

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);

    return textureId;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto *window = glfwCreateWindow(WIDTH, HEIGHT, "The Dark Engine", nullptr, nullptr);
    if (not window)
    {
        std::cerr << "Failed to create a window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    glow::ShaderProgram shaderProgram{std::make_tuple(
        glow::VertexShader{"shaders/shader.vert"},
        glow::FragmentShader{/*"fragment.fs"*/},
        glow::GeometryShader{}
    )};

    constexpr auto vertices { std::to_array<f32>({
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  //
    })};

    constexpr auto indices { std::to_array<i32>({
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    })};

    u32 vbo, vao, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), reinterpret_cast<void*>(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);

    const auto texture1{ createTexture("textures/obama.png") };
    const auto texture2 {createTexture("textures/container.png")};

    shaderProgram.use();

    glUniform1i(glGetUniformLocation(shaderProgram.getId(), "texture1"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram.getId(), "texture2"), 1);

    while (not glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
        transform = glm::rotate(transform, static_cast<float>(glfwGetTime()), glm::vec3(0.f, 0.f, 1.f));

        shaderProgram.use();

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    /*glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);*/

    glfwTerminate();
}