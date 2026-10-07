#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <numeric>

namespace ig=ImGui;

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glow/basic-types.hpp"
#include "glow/error.hpp"
#include "glow/shader-program.hpp"
#include "glow/shader.hpp"

#include "window.hpp"

using namespace glow::basicTypes;

#include <array>
#include <iostream>
#include <chrono>
#include <algorithm>

constexpr i32 WIDTH{1920};
constexpr i32 HEIGHT{1080};
constexpr auto TITLE{"The Dark Engine"};


class FpsCounter
{
public:

	FpsCounter() noexcept
		: m_frameTimes{} , m_sum{0} , index{0}
	{
		using namespace std::chrono_literals;

		for (auto &frameTime : m_frameTimes)
		{
			frameTime = 0us;
		}
	};

	auto addValue(const std::chrono::microseconds frameTime)
	{
		m_sum -= m_frameTimes[index];
		m_sum += frameTime;

		m_frameTimes[index] = frameTime;

		index = (index == m_frameTimes.size() - 1) ?  0 : index + 1;
	}

	[[nodiscard]] auto count() const noexcept -> u32
	{
		using namespace std::chrono_literals;

		const auto avg{ m_sum / m_frameTimes.size() };
		if ( avg == 0us) [[unlikely]] return 0;

		return 1s / avg;
	}

private:

	std::array<std::chrono::microseconds, 100> m_frameTimes;
	std::chrono::microseconds m_sum;
	size_t index;
};


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


glm::vec3 cameraPos{ 0.f, 0.f, 3.f };
glm::vec3 cameraFront{ 0.0f, 0.0f, -1.f };
glm::vec3 cameraUp{ 0.f , 1.f, 0.f };

f32 yaw{ -90.f };
f32 pitch{ 0.f };
glm::vec2 lastPos = { WIDTH / 2, HEIGHT / 2 };
bool firstMouseInput = true;
f32 zoom = 45.f;

static void cursorPositionCallback(GLFWwindow *window, const f64 xPos, const f64 yPos)
{
	if (firstMouseInput)
	{
		lastPos = { xPos, yPos };
		firstMouseInput = false;
	}

	glm::vec2 offset{ xPos - lastPos.x, lastPos.y - yPos };
	lastPos = { xPos, yPos };

	constexpr f32 sensitivity{ 0.1f };
	offset *= sensitivity;

	yaw += offset.x;
	pitch += offset.y;

	pitch = std::clamp(pitch, -89.f, 89.f);


	glm::vec3 direction;
	direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
	direction.y = glm::sin(glm::radians(pitch));
	direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow *window, double xOffset, double yOffset)
{
	zoom -= static_cast<f32>(yOffset);

	zoom = std::clamp(zoom, 1.f , 45.f);
}


static void mouseButtonCallback(GLFWwindow *window, const i32 button, const i32 action,	[[maybe_unused]] i32 mods)
{

}


auto main() -> int
{
	auto result{ tde::Window::create(WIDTH, HEIGHT, TITLE)};
	if (not result)
	{
		std::cerr << result.error() << std::endl;
	}

	auto window = *result;


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	if (not gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD.\n";
		return 1;
	};

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


	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetScrollCallback(window, scroll_callback);

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	auto width{WIDTH};
	auto height{HEIGHT};

	FpsCounter counter;
	std::chrono::microseconds duration{0};


	f32 deltaTime{}, lastFrame{};

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	while (not glfwWindowShouldClose(window))
	{
		const f32 currentFrame{ static_cast<f32>(glfwGetTime())};
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		const f32 cameraSpeed = 2.5f * deltaTime;

		if (glfwGetKey(window, GLFW_KEY_W))
		{/*
			camPos.z += 0.1;*/
			cameraPos += cameraSpeed * cameraFront;
		}
		if (glfwGetKey(window, GLFW_KEY_S))
		{
			cameraPos -= cameraSpeed * cameraFront;
		}
		if (glfwGetKey(window, GLFW_KEY_A))
		{
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D))
		{
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}

		counter.addValue(duration);
		const auto start{ std::chrono::high_resolution_clock::now() };

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ig::NewFrame();
		ig::Begin(TITLE);

		ig::Text("%d fps", counter.count());

		ig::SeparatorText("Projection");
		ig::SliderFloat("FOV", &zoom, 20.f, 360.f);
		ig::SliderInt("Width", &width, 720, 1920);
		ig::SliderInt("Height", &height, 400, 1080);

		ig::Separator();
		//TODO: Решить траблы с кириллицей в ImGui
		ig::SeparatorText(reinterpret_cast<const char*>(u8"View"));

		ig::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const auto view{ glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp) };

		setMatrixValue(shaderProgram, "view", view);

		const auto perspectiveProjection
		{ glm::perspective(
			glm::radians(zoom),
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
			auto model = glm::translate(glm::mat4{1.f}, cubePositions[i]);

			const f32 rotation = (i % 2 == 0) ? 20 * static_cast<f32>(i) : 10 * glfwGetTime() * i;

			model = glm::rotate(model, glm::radians(rotation), {1.f, 0.3f, 0.5f});

			setMatrixValue(shaderProgram, "model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glow::Error::printIfError();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		const auto end{ std::chrono::high_resolution_clock::now() };
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		glfwSwapBuffers(window);
		glfwPollEvents();
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