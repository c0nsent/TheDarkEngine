#include "glad/glad.h"

#include <SFML/Window.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "glow/basic-types.hpp"
#include "glow/error.hpp"
#include "glow/shader-program.hpp"
#include "glow/shader.hpp"

using namespace glow::basicTypes;

#include <array>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <optional>


constexpr i32 WIDTH{1920};
constexpr i32 HEIGHT{1080};
constexpr auto TITLE{"The Dark Engine"};


static auto createTexture(const char *path) noexcept -> u32
{
    u32 textureId{0};

    i32 width, height, nrChannels;
    u8 *imageData{stbi_load(path, &width, &height, &nrChannels, 0)};
    if (not imageData)
    {
        return textureId;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const i32 format{nrChannels == 3 ? GL_RGB : GL_RGBA};

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        imageData
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);

    return textureId;
}


static void setMatrixValue(const glow::ShaderProgram &sp, const char *matrixName, const glm::mat4 &matrix) noexcept
{
    glUniformMatrix4fv(
        glGetUniformLocation(sp.getId(), matrixName),
        1,
        GL_FALSE,
        glm::value_ptr(matrix)
    );
}


int main()
{
    constexpr sf::ContextSettings settings {
        .depthBits = 24,
        .stencilBits = 8,
        .antiAliasingLevel = 4,
        .majorVersion = 4,
        .minorVersion = 6,
    };


    sf::Window win{ sf::VideoMode{{WIDTH, HEIGHT}}, TITLE, sf::Style::Default, sf::State::Windowed, settings };
    win.setVerticalSyncEnabled(true);

    if (not win.setActive())
    {
        std::cerr << "Failed to set current window as a target for rendering\n";
        return EXIT_FAILURE;
    }

    const int version{ gladLoadGLLoader([] (const char *name) -> void *
    {
        return reinterpret_cast<void *>(sf::Context::getFunction(name));
    })};
    if (not version)
    {
        std::cerr << "Failed to load OpenGL functions\n";
        return EXIT_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);


	const glow::ShaderProgram shaderProgram{std::make_tuple(
		glow::VertexShader{"shaders/shader.vert"},
		glow::FragmentShader{"shaders/shader.frag"},
		glow::GeometryShader{}
	)};

	constexpr auto vertices{ std::to_array<f32>({
	    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

       -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
       -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

       -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
       -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
       -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

       -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
       -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

       -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
       -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
       -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	})};

    constexpr auto cubePositions { std::to_array<glm::vec3>({
		{ 0.0f,  0.0f,  0.0f},
		{ 2.0f,  5.0f, -15.0f},
		{-1.5f, -2.2f, -2.5f},
		{-3.8f, -2.0f, -12.3f},
		{2.4f, -0.4f, -3.5f},
		{-1.7f,  3.0f, -7.5f},
		{ 1.3f, -2.0f, -2.5f},
		{1.5f,  2.0f, -2.5f},
		{ 1.5f,  0.2f, -1.5f},
		{-1.3f,  1.0f, -1.5f}
	})};

	stbi_set_flip_vertically_on_load(true);

	const auto obamaTexture{createTexture("textures/obama.png")};

	if (not obamaTexture)
	{
		std::cerr << "Failed to load texture: " << "textures/obama.png";
		return EXIT_FAILURE;
	}

    glow::Error::printIfError();

	u32 vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);

    glow::Error::printIfError();

	glBindVertexArray(vao);

    glow::Error::printIfError();

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glNamedBufferData(vbo, vertices.size() * sizeof(vertices.front()), vertices.data(), GL_STATIC_DRAW);

    glow::Error::printIfError();

    glow::Error::printIfError();

	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(vertices.front()), reinterpret_cast<void *>(0));
	glEnableVertexAttribArray(0);

    glow::Error::printIfError();

	glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(vertices.front()), reinterpret_cast<void*>(3 * sizeof(vertices.front())));
	glEnableVertexAttribArray(1);

    glow::Error::printIfError();

	shaderProgram.use();
	glUniform1i(glGetUniformLocation(shaderProgram.getId(), "texture1"), 0);


	const auto res{ win.getSize() };

	f32 yaw{ -90.f };
	f32 pitch{ 0.f };
	glm::vec2 lastPos{ res.x / 2, res.y / 2 };
	bool firstMouseInput{true};
	f32 fov{45.f};

    bool running{ true };
	sf::Clock clock;

	tde::Camera cam;
	while (running)
    {

    	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
    	{
    		//win.close();
    	}

        while (const auto event{ win.pollEvent()})
        {
            if (event->is<sf::Event::Closed>())
            {
                running = false;
            }
            else if (const auto *resized{ event->getIf<sf::Event::Resized>() })
            {
                glViewport(
                	0,
                	0,
                	static_cast<i32>(resized->size.x),
                	static_cast<i32>(resized->size.y)
                );
            }
        }

    	glClearColor(0.2f, 0.3f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    	cam.calcDirection();
		auto view {cam.calcPosition(clock.getElapsedTime())};

		setMatrixValue(shaderProgram, "view", view);

    	glow::Error::printIfError();

    	const auto perspectiveProjection
    	{ glm::perspective(
			glm::radians(fov),
			static_cast<f32>(win.getSize().x)/static_cast<f32>(win.getSize().y),
			0.1f,
			100.f
		)};
    	setMatrixValue(shaderProgram, "projection", perspectiveProjection);

    	glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_2D, obamaTexture);

    	shaderProgram.use();
    	glBindVertexArray(vao);
    	for (u32 i{0}; i < 10; i++)
    	{
    		auto model = glm::translate(glm::mat4{1.f}, cubePositions[i]);
    		const f32 rotation = (i % 2 == 0) ? 20 * static_cast<f32>(i) : i * (static_cast<f32>(clock.getElapsedTime().asMicroseconds()) / 100000); // здесь была дельта
    		model = glm::rotate(model, glm::radians(rotation), {1.f, 0.3f, 0.5f});
    		setMatrixValue(shaderProgram, "model", model);

    		glDrawArrays(GL_TRIANGLES, 0, 36);
    	}

    	glow::Error::printIfError();

        win.display();
    }

	glDeleteProgram(shaderProgram.getId());
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

    return EXIT_SUCCESS;
}
