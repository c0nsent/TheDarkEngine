#include "glad/glad.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-OpenGL3.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glow/basic-types.hpp"
using namespace glow::basicTypes;
#include "glow/error.hpp"
#include "glow/shader-program.hpp"
#include "glow/shader.hpp"
#include "glow/utility.hpp"

#include "camera.hpp"

#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <print>


static auto countAvgFrameTime(const sf::Time delta) -> sf::Time
{
    static u64 counter{0};
    static sf::Time totalTime;

    counter++;
    totalTime += delta;

    return totalTime / static_cast<f32>(counter);
}


static auto createTexture(const std::filesystem::path &path) noexcept -> u32
{
    u32 textureId{0};

	sf::Image texData;
	if (!texData.loadFromFile(path))
	{
		return textureId;
	}

	texData.flipVertically();

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA, //sf::Image всегда содержит 32 битный RGBA
        static_cast<i32>(texData.getSize().x),
        static_cast<i32>(texData.getSize().y),
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        texData.getPixelsPtr()
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureId;
}


static auto createGui(sf::Window &win) -> std::unique_ptr<tgui::Gui>
{
	auto gui{ std::make_unique<tgui::Gui>(win) };

	gui->setTextSize(12);
	gui->setOpacity(0.8);
    gui->setRelativeViewport({0, 0, 0.25f , 1});
	const tgui::Font font{ "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf" };
	gui->setFont(font);

	return gui;
}


static auto createLayout(tgui::Gui &gui) -> tgui::VerticalLayout::Ptr
{
    auto layout{ tgui::VerticalLayout::create() };
    layout->setPosition(20, 20);
	gui.add(layout);
    return layout;
}


static auto createLabel(const tgui::VerticalLayout::Ptr &layout) -> tgui::Label::Ptr
{
	auto label{ tgui::Label::create() };
	layout->add(label);

	const auto renderer{ label->getRenderer() };
	renderer->setBackgroundColor({28, 29, 31});
	renderer->setTextColor({210, 219, 222});

	return label;
}


static auto createColorPicker(const tgui::VerticalLayout::Ptr &layout, const char *title, const tgui::Color color) -> tgui::ColorPicker::Ptr
{
	auto colorPicker{ tgui::ColorPicker::create(title, color) };

	auto colorPickerRenderer{ colorPicker->getRenderer() };
	colorPickerRenderer->setBackgroundColor({28, 29, 31});
	colorPickerRenderer->setTitleColor({210, 219, 222});
	//colorPicker->setHeight(100);
	//colorPicker->setWidth(200);

	layout->add(colorPicker);
	return colorPicker;
}


static void GLDebugMessageCallback(
	const u32 source,
	const u32 type,
	const u32 id,
	const u32 severity,
	i32,
	const char *msg,
	const void *)
{
    const char* strSource;
    const char* strType;
    const char* strSeverity;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
        strSource = "API";
        break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        strSource = "WINDOW SYSTEM";
        break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        strSource = "SHADER COMPILER";
        break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        strSource = "THIRD PARTY";
        break;

        case GL_DEBUG_SOURCE_APPLICATION:
        strSource = "APPLICATION";
        break;

        case GL_DEBUG_SOURCE_OTHER:
    	strSource = "OTHER";
    	break;

    	default:
        strSource = "UNKNOWN";
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        strType = "ERROR";
        break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        strType = "DEPRECATED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        strType = "UNDEFINED BEHAVIOR";
        break;

        case GL_DEBUG_TYPE_PORTABILITY:
        strType = "PORTABILITY";
        break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        strType = "PERFORMANCE";
        break;

        case GL_DEBUG_TYPE_OTHER:
        strType = "OTHER";
        break;

        case GL_DEBUG_TYPE_MARKER:
        strType = "MARKER";
        break;

        default:
        strType = "UNKNOWN";
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        strSeverity = "HIGH";
        break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        strSeverity = "MEDIUM";
        break;

        case GL_DEBUG_SEVERITY_LOW:
        strSeverity = "LOW";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        strSeverity = "NOTIFICATION";
        break;

        default:
        strSeverity = "UNKNOWN";
    }

	std::println(
		"{}: {} of {} severity, raised from {}: {}",
		id,
		strType,
		strSeverity,
		strSource,
		msg
	);
}


static glm::vec3 lightPos{ 1.2f, 1.f, 2.f };
static auto lightSourceColor{ sf::Color::White };

auto main() -> int
{
	sf::Window win{
	    sf::VideoMode{{1920, 1080}},
	    "The Dark Engine",
	    sf::Style::Default,
	    sf::State::Fullscreen,
	    {
	    	.depthBits = 24,
			.stencilBits = 8,
			.antiAliasingLevel = 4,
			.majorVersion = 4,
			.minorVersion = 6,
			.attributeFlags = sf::ContextSettings::Attribute::Core,
	    }
	};

	if (not win.setActive())
	{
		std::cerr << "Failed to set current window as a target for rendering\n";
		return EXIT_FAILURE;
	}

    win.setMouseCursorVisible(false);

	const i32 version{ gladLoadGLLoader([] (const char *name)
	{
		return reinterpret_cast<void *>(sf::Context::getFunction(name));
	})};
	if (not version)
	{
		std::cerr << "Failed to load OpenGL functions\n";
		return EXIT_FAILURE;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GLDebugMessageCallback, nullptr);


	auto gui{createGui(win)};

    auto layout{createLayout(*gui)};
	auto mousePosLabel{ createLabel(layout) };
    auto deltaLabel{ createLabel(layout) };
	auto colorPicker{ createColorPicker(layout, "Источник цвета", tgui::Color::White) };

	const glow::ShaderProgram shaderProgram{std::make_tuple(
		glow::VertexShader{"shaders/shader.vert"},
		glow::FragmentShader{"shaders/shader.frag"},
		glow::GeometryShader{}
	)};

    shaderProgram.setColor("lightColor", lightSourceColor);

	const glow::ShaderProgram lightSourceSP{ std::make_tuple(
		glow::VertexShader{ "shaders/light-source.vert" },
		glow::FragmentShader{ "shaders/light-source.frag" },
		glow::GeometryShader{}
	)};

    lightSourceSP.setColor("lightColor", lightSourceColor);

	constexpr auto vertices{ std::to_array<f32>({
	    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	})};

	u32 vbo, cubeVao;
    glGenVertexArrays(1, &cubeVao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(cubeVao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glNamedBufferData(
	    vbo,
	    vertices.size() * sizeof(vertices.front()),
	    vertices.data(),
	    GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		false,
		6 * sizeof(vertices.front()),
		reinterpret_cast<void *>(0)
	);
	glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        false,
        6 * sizeof(vertices.front()),
        reinterpret_cast<void *>(3 * sizeof(vertices.front()))
    );
    glEnableVertexAttribArray(1);

	u32 lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(vertices.front()),
		reinterpret_cast<void *>(0)
	);
    glEnableVertexAttribArray(0);

	tde::Camera cam;

	sf::Clock deltaClock;
    sf::Time avgFrameTime;

	sf::Vector2i prevMouseDelta{0, 0};

    f32 rotation{};

    bool isCursorMode{false};
	{
		auto perspectiveProjection
		{ glm::perspective(
			glm::radians(cam.getFov()),
			static_cast<f32>(win.getSize().x) / static_cast<f32>(win.getSize().y),
			0.1f,
			100.f
		)};

		shaderProgram.setMatrix("projection", perspectiveProjection);
		lightSourceSP.setMatrix("projection", perspectiveProjection);
	}
	{
		auto model{ glm::mat4{1.f} };
		shaderProgram.setMatrix("model",  model);
	}
	while (win.isOpen())
	{
	    const auto delta{ deltaClock.restart()};
        avgFrameTime = countAvgFrameTime(delta);

	    while (const std::optional event{ win.pollEvent()})
	    {
	        gui->handleEvent(*event);

	    	if (const auto pressed{ event->getIf<sf::Event::KeyPressed>()}) {

	    	    using namespace sf::Keyboard;

	    	    cam.updateKeysState(pressed);

	    		if (pressed->code == Key::Escape) win.close();

	    	    if (pressed->code == Key::Tab)
	    	    {
	    	        isCursorMode = not isCursorMode;
	    	        win.setMouseCursorVisible(isCursorMode);
	    	    }
	    	}
	    	if (const auto moved{ event->getIf<sf::Event::MouseMovedRaw>()}; moved and not isCursorMode)
	    	{
	    	    mousePosLabel->setText(std::format("x: {}, y {}", moved->delta.x, moved->delta.y));

	    		const sf::Vector2i processedMouseDelta{
	    			glm::abs(moved->delta.x) / 50,
					glm::abs(moved->delta.y) / 50
				};

	    		if ( glm::abs(prevMouseDelta.x) < processedMouseDelta.x
					or glm::abs(prevMouseDelta.y) < processedMouseDelta.y ) [[unlikely]]
	    		{
	    			std::cerr << moved->delta.x << ' ' << moved->delta.y << '\n';
	    			continue;
	    		}
	    		prevMouseDelta = moved->delta;

	    		cam.updateDirection(moved);
	    	}
	        if (isCursorMode and event->is<sf::Event::MouseMoved>())
	        {
	            shaderProgram.setColor("lightColor", lightSourceColor);
	            lightSourceSP.setColor("lightColor", lightSourceColor);
	        }
	    	if (const auto released{ event->getIf<sf::Event::KeyReleased>() })
	    	{
	    		cam.updateKeysState(released);
	    	}
	    	if (const auto scrolled { event->getIf<sf::Event::MouseWheelScrolled>() })
	    	{
	    		cam.updateFov(scrolled);

	    		auto perspectiveProjection { glm::perspective(
				glm::radians(cam.getFov()),
				static_cast<f32>(win.getSize().x) / static_cast<f32>(win.getSize().y),
				0.1f,
				100.f
				)};

	    		shaderProgram.setMatrix("projection", perspectiveProjection);
	    		lightSourceSP.setMatrix("projection", perspectiveProjection);

	    	}
	        if (event->is<sf::Event::Closed>())
	        {
	        	std::cerr << "Window has closed\n";
	            win.close();
	        }
	    	if (const auto resized{ event->getIf<sf::Event::Resized>() })
	        {
	    		glow::viewport(resized->size);

	    		auto perspectiveProjection{ glm::perspective(
				glm::radians(cam.getFov()),
				static_cast<f32>(resized->size.x) / static_cast<f32>(resized->size.y),
				0.1f,
				100.f
				)};

	    		shaderProgram.setMatrix("projection", perspectiveProjection);
	    		lightSourceSP.setMatrix("projection", perspectiveProjection);
	        }
	    }

	    deltaLabel->setText(std::format("Frame Time: {} us | Average: {}",
	            delta.asMicroseconds(),
	            avgFrameTime.asMicroseconds()
	    ));

	    lightSourceColor = colorPicker->getColor();

		glow::clearScreen({20, 20, 20});

	    shaderProgram.setVec3("objectColor", {1.f, 0.5f, 0.31f});

	    const auto view{ cam.getPositionAndDirection(delta) };
	    shaderProgram.setMatrix("view", view);
		shaderProgram.setVec3("viewPos", cam.getPos());

		rotation += delta.asSeconds() * 10;

		auto model = glm::rotate(glm::mat4{1.f}, glm::radians(rotation), {0, 1, 0});

		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		const glm::vec3 currentLightPos{ model[3] };
		shaderProgram.setVec3("lightPos", currentLightPos);

	    glBindVertexArray(cubeVao);

	    glDrawArrays(GL_TRIANGLES, 0, 36);

	    lightSourceSP.setMatrix("view", view);
		lightSourceSP.setMatrix("model", model);

	    glBindVertexArray(lightVAO);
    	glDrawArrays(GL_TRIANGLES, 0, 36);

		gui->draw();
		win.display();
    }

	glDeleteVertexArrays(1, &cubeVao);
	glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &lightVAO);

    return EXIT_SUCCESS;
}