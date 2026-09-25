#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

#include <array>
#include <iostream>


using namespace glow::basicTypes;

constexpr i32 WIDTH{620};
constexpr i32 HEIGHT{480};
constexpr auto TITLE{"The Dark Engine"};


static void errorCallback(const i32 error, const char *description)
{
	std::cerr << "Error: " << error << " " << description << std::endl;
}


static void framebufferSizeCallback(GLFWwindow *, const i32 width, const i32 height)
{
	glViewport(0, 0, width, height);
}


static void processInput(GLFWwindow *window)
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


auto main() -> int
{
	glfwSetErrorCallback(errorCallback);

	if (not glfwInit())
	{
		std::cerr << "Failed to initialize GLFW\n";
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow *window{glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr)};
	if (window == nullptr)
	{
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSwapInterval(1);

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
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f,	1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f,	0.0f, 1.0f,
	})};

	constexpr auto indices{ std::to_array<u32>({
		0, 1, 3,
		1, 2, 3,
	})};

	stbi_set_flip_vertically_on_load(true);

    glow::Error::printIfError();

	const auto obamaTexture{createTexture("textures/obama.png")};

    glow::Error::printIfError();

	u32 vbo, vao, ebo;
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);

    glow::Error::printIfError();

	glBindVertexArray(vao);

    glow::Error::printIfError();

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glNamedBufferData(vbo, vertices.size() * sizeof(vertices.front()), vertices.data(), GL_STATIC_DRAW);

    glow::Error::printIfError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glNamedBufferData(ebo, indices.size() * sizeof(indices.front()), indices.data(), GL_STATIC_DRAW);

    glow::Error::printIfError();

	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(indices.front()), reinterpret_cast<void *>(0));
	glEnableVertexAttribArray(0);

    glow::Error::printIfError();

	glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(indices.front()), reinterpret_cast<void*>(3 * sizeof(indices.front())));
	glEnableVertexAttribArray(1);

    glow::Error::printIfError();

	shaderProgram.use();
	glUniform1i(glGetUniformLocation(shaderProgram.getId(), "texture1"), 0);

	f32 rotationScaler{0.f};

	while (not glfwWindowShouldClose(window))
	{
		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		ImGui::Begin(TITLE);
		ImGui::SliderFloat("Speed", &rotationScaler, 0.f, 1.f);
		ImGui::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		/*glm::mat4 trans{ 1.f };
		trans = glm::rotate(trans, glm::radians(90.f), glm::vec3{0.f, 0.f, 1.f});
		trans = glm::scale(trans, glm::vec3{ 0.5, 0.5, 0.5 });*/

		glm::mat4 trans{1.f};
		//trans = glm::translate(trans, glm::vec3(0.5, -0.5, 0.0));
		trans = glm::rotate(trans, static_cast<f32>(glfwGetTime()) * rotationScaler, glm::vec3(0.0f, 0.0, 1.f));

		const auto transLoc= glGetUniformLocation(shaderProgram.getId(), "transform");
		glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obamaTexture);

		shaderProgram.use();
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glow::Error::printIfError();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgram.getId());
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}