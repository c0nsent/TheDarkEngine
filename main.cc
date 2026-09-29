#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace ig=ImGui;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glow/basic-types.hpp"
#include "glow/error.hpp"
#include "glow/shader-program.hpp"
#include "glow/shader.hpp"

using namespace glow::basicTypes;

#include <array>
#include <iostream>


constexpr i32 WIDTH{1920};
constexpr i32 HEIGHT{1080};
constexpr auto TITLE{"The Dark Engine"};


static void glfwErrorCallback(const i32 error, const char *description) noexcept
{
	std::cerr << "[GLFW] Error " << error << ": "
		<< (description ? description : "unknown error") << '\n';
}


static void framebufferSizeCallback(GLFWwindow *, const i32 width, const i32 height) noexcept
{
	glViewport(0, 0, width, height);
}


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

glm::f64vec2 cursorPosition{ 0, 0 };


static void cursorPositionCallback(GLFWwindow *window, const f64 xPos, const f64 yPos)
{
	cursorPosition = { xPos, yPos };
}

auto main() -> int
{
	glfwSetErrorCallback(glfwErrorCallback);

	if (not glfwInit()) return EXIT_FAILURE;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window{glfwCreateWindow(WIDTH, HEIGHT, TITLE, glfwGetPrimaryMonitor(), nullptr)};
	if (window == nullptr)
	{
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwGetCursorPos(window, &cursorPosition.x, &cursorPosition.y);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	/*ImGuiIO &io{ImGui::GetIO()};
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;*/

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	if (not gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD.\n";
		return 1;
	}

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

    constexpr glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

	stbi_set_flip_vertically_on_load(true);

    glow::Error::printIfError();

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

	/*
	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}
	*/

	glfwSetCursorPosCallback(window, cursorPositionCallback);

    f32 fov{45.f};
	auto width{WIDTH};
	auto height{HEIGHT};

	glm::f64vec2 cursorOrigin;
	glm::vec2 cameraMoveInDegrees;
	glm::vec2 camVelocity;
	glm::vec2 prevCameraMove;

	bool firstIteration;

    glEnable(GL_DEPTH_TEST);

	while (not glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			if ( not firstIteration)
			{
				camVelocity = (cursorPosition - cursorOrigin);

				std::cerr << cursorOrigin.x << ", " << cursorOrigin.y << std::endl;
				std::cerr << cursorPosition.x << " " << cursorPosition.y << std::endl;

				cameraMoveInDegrees = { camVelocity.x / 10, camVelocity.y / 10 };
			}
			else
			{
				firstIteration = false;
				cursorOrigin = cursorPosition;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
		{
			firstIteration = true;
			prevCameraMove += cameraMoveInDegrees;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ig::NewFrame();
		ig::Begin(TITLE);
		ig::SeparatorText("Projection");
		ig::SliderFloat("FOV", &fov, 20.f, 360.f);
		ig::SliderInt("Width", &width, 720, 1920);
		ig::SliderInt("Height", &height, 400, 1080);

		ig::Separator();
		//TODO: Решить траблы с кириллицей в ImGui
		ig::SeparatorText(reinterpret_cast<const char*>(u8"View"));
		/*ig::SliderFlo("Horizontal movement", &cameraMoveInDegrees.y, -100.f, 100.f);
		ig::SliderFloat("Vertical movement", &cameraVerticalMove, -100.f, 100.f);*/
		ig::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        auto view = glm::translate(glm::mat4{1.f}, {0.f, 0.f, -3.f});
		view = glm::rotate(view, glm::radians(prevCameraMove.y + cameraMoveInDegrees.y), {1.f, 0.f, 0.f});
		view = glm::rotate(view, glm::radians(prevCameraMove.x + cameraMoveInDegrees.x), {0.f, 1.f, 0.f});
		setMatrixValue(shaderProgram, "view", view);

	    const auto perspectiveProjection
	    { glm::perspective(
			glm::radians(fov),
            static_cast<f32>(width)/static_cast<f32>(height),
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
            auto model = glm::mat4{1.f};
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(20.f * i), {1.f, 0.3f, 0.5f});

        	setMatrixValue(shaderProgram, "model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


		glow::Error::printIfError();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();

		//std::cerr << "Iteration completed" << std::endl;
	}

	glDeleteProgram(shaderProgram.getId());
	glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}