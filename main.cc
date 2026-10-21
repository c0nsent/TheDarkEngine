#include "glad/glad.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Image.hpp>


#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-OpenGL3.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glow/basic-types.hpp"
#include "glow/error.hpp"
#include "glow/shader-program.hpp"
#include "glow/shader.hpp"
#include "glow/utility.hpp"

using namespace glow::basicTypes;

#include "camera.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <filesystem>


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


static auto setupGui(tgui::Gui &gui) -> void
{
	gui.setTextSize(12);
	gui.setOpacity(0.8);

	const tgui::Font font{ "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf" };
	gui.setFont(font);
}


static auto createLayout(tgui::Gui &gui) -> std::shared_ptr<tgui::VerticalLayout>
{
    auto layout{ tgui::VerticalLayout::create() };
    layout->setPosition(20, 20);
    layout->setSize(200, 100);

    return layout;
}


auto main() -> int
{
	constexpr sf::ContextSettings settings {
		.depthBits = 24,
		.stencilBits = 8,
		.antiAliasingLevel = 4,
		.majorVersion = 4,
		.minorVersion = 6,
		.attributeFlags = sf::ContextSettings::Attribute::Core,
	};

	sf::Window win{
	    sf::VideoMode{{1920, 1080}},
	    "The Dark Engine",
	    sf::Style::Default,
	    sf::State::Fullscreen,
	    settings
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

	tgui::Gui gui{ win };
	setupGui(gui);

    auto layout = createLayout(gui);
    gui.add(layout);

	const auto pos{sf::Mouse::getPosition()};
	const auto label{ std::format("x: {} y: {}", pos.x, pos.y) };
	auto mousePosLabel{ tgui::Label::create(label) };

    auto deltaLabel{ tgui::Label::create( "Delta time: 0") };

	std::shared_ptr<tgui::LabelRenderer> labelStyle{ mousePosLabel->getSharedRenderer() };
	labelStyle->setBackgroundColor({28, 29, 31});
	labelStyle->setTextColor({210, 219, 222});

    deltaLabel->setRenderer(labelStyle->getData());

	layout->add(mousePosLabel);
    layout->add(deltaLabel);

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


	const auto texture{createTexture("assets/life-during-wartime.jpg")};

	if (not texture)
	{
		std::cerr << "Failed to load texture: " << "assets/life-during-wartime.jpg";
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

	tde::Camera cam;

	sf::Clock deltaClock;
    sf::Time avgFrameTime;

    f32 rotation{};

	size_t counter{0};

	while (win.isOpen())
	{
	    const auto delta{ deltaClock.restart()};
        avgFrameTime = countAvgFrameTime(delta);

	    while (const std::optional event{ win.pollEvent()})
	    {
	        gui.handleEvent(*event);

	    	if (const auto pressed{ event->getIf<sf::Event::KeyPressed>()}) {
	    		cam.updateKeysState(pressed);

	    		if (pressed->code == sf::Keyboard::Key::Escape) win.close();
	    	}
	    	if (const auto moved{ event->getIf<sf::Event::MouseMovedRaw>() })
	    	{
	    		if (counter < 8)
	    		{
	    			counter++;
	    			continue;
	    		}

	    		cam.updateDirection(moved);
	    	}
	    	if (const auto released{ event->getIf<sf::Event::KeyReleased>() })
	    	{
	    		cam.updateKeysState(released);
	    	}
	    	if (const auto scrolled { event->getIf<sf::Event::MouseWheelScrolled>() })
	    	{
	    		cam.updateFov(scrolled);
	    	}
	        if (event->is<sf::Event::Closed>())
	        {
	        	std::cerr << "Window has closed\n";
	            win.close();
	        }
	    	if (const auto resized{ event->getIf<sf::Event::Resized>() })
	        {
	    		glow::viewport(resized->size);
	        }
	    }

	    deltaLabel->setText(std::format("Frame Time: {} us | Average: {}",
	            delta.asMicroseconds(),
	            avgFrameTime.asMicroseconds()
	    ));

    	glClearColor(0.2f, 0.3f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderProgram.use();

		shaderProgram.setMatrix("view", cam.getPositionAndDirection(delta));

    	glow::Error::printIfError();

    	const auto perspectiveProjection
    	{ glm::perspective(
			glm::radians(cam.getFov()),
			static_cast<f32>(win.getSize().x)/static_cast<f32>(win.getSize().y),
			0.1f,
			100.f
		)};
		shaderProgram.setMatrix("projection", perspectiveProjection);

    	glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_2D, texture);

    	glBindVertexArray(vao);

    	for (u32 i{0}; i < 10; i++)
    	{
    		auto model{ glm::translate(glm::mat4{1.f}, cubePositions[i]) };

    	    rotation += delta.asSeconds();
    		const f32 rotationPos = (i % 2 == 0) ? 20 * static_cast<f32>(i) : static_cast<f32>(i) * rotation;
    		model = glm::rotate(model, glm::radians(rotationPos), {1.f, 0.3f, 0.5f});
    		shaderProgram.setMatrix("model", model);

    		glDrawArrays(GL_TRIANGLES, 0, 36);
    	}

		gui.draw();

		glow::Error::printIfError();

		win.display();
    }

	glDeleteProgram(shaderProgram.getId());
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

    //TODO: Некорректно удаляются виджеты в TGUI

    return EXIT_SUCCESS;
}
