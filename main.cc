#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <numeric>

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
#include <chrono>
#include <forward_list>
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


struct Camera
{
	glm::vec2 cursorOrigin;
	glm::vec2 cursorPos;
	glm::vec2 offset;
	glm::vec2 camPos;

	bool isButtonAlreadyPressed;
};



static void cursorPositionCallback(GLFWwindow *window, const f64 xPos, const f64 yPos)
{
	auto *const cam { static_cast<Camera *>(glfwGetWindowUserPointer(window)) };
	if (not cam) return;

	cam->cursorPos = {xPos, yPos};

	if (cam->isButtonAlreadyPressed)
	{
		cam->offset = cam->cursorPos - cam->cursorOrigin;
	}
}


static void mouseButtonCallback(GLFWwindow *window, const i32 button, const i32 action,	[[maybe_unused]] i32 mods)
{
	if (button != GLFW_MOUSE_BUTTON_RIGHT) return;

	auto *const cam{ static_cast<Camera *>(glfwGetWindowUserPointer(window)) };
	if (not cam ) return;

	if (action == GLFW_PRESS)
	{
		cam->isButtonAlreadyPressed = true;
		cam->cursorOrigin = cam->cursorPos;
	}
	else if (action == GLFW_RELEASE)
	{
		cam->isButtonAlreadyPressed = false;

		cam->camPos += cam->offset;
		cam->offset = {0, 0};
	}
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

	Camera camera { .cursorOrigin = {0, 0}, .cursorPos = {0, 0 }, .offset = {0, 0 }, .camPos =  {0, 0}};
	glfwSetWindowUserPointer(window, &camera);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

    f32 fov{45.f};
	auto width{WIDTH};
	auto height{HEIGHT};

    glEnable(GL_DEPTH_TEST);

	FpsCounter counter;
	std::chrono::microseconds duration{0};

	glm::vec3 camPos{0, 0, -3};

	while (not glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_W))
		{
			camPos.z += 0.1;
		}
		if (glfwGetKey(window, GLFW_KEY_S))
		{
			camPos.z -= 0.1;
		}
		if (glfwGetKey(window, GLFW_KEY_A))
		{
			camPos.x += 0.1;
		}
		if (glfwGetKey(window, GLFW_KEY_D))
		{
			camPos.x -= 0.1;
		}

		counter.addValue(duration);
		const auto start{ std::chrono::high_resolution_clock::now() };

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ig::NewFrame();
		ig::Begin(TITLE);

		ig::Text("%d fps", counter.count());

		ig::SeparatorText("Projection");
		ig::SliderFloat("FOV", &fov, 20.f, 360.f);
		ig::SliderInt("Width", &width, 720, 1920);
		ig::SliderInt("Height", &height, 400, 1080);

		ig::Separator();
		//TODO: Решить траблы с кириллицей в ImGui
		ig::SeparatorText(reinterpret_cast<const char*>(u8"View"));

		const auto label = std::string{"IsButtonAlreadyPressed: "} + (camera.isButtonAlreadyPressed ? "True" : "False");
		ig::Text(label.data());
		ig::Text("Cursor Origin: %f, %f", camera.cursorOrigin.x, camera.cursorOrigin.y);
		ig::Text("Offset: %f %f", camera.offset.x, camera.offset.y);
		ig::Text("CamPos: %f %f", camera.camPos.x, camera.camPos.y);

		ig::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const auto camDirection{ camera.camPos + camera.offset};

		auto view = glm::translate(glm::mat4{1.f}, camPos);
		view = glm::rotate(view, glm::radians(camDirection.y), {1.f, 0.f, 0.f});
		view = glm::rotate(view, glm::radians(camDirection.x), {0.f, 1.f, 0.f});
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