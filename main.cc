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
    //layout->setSize(300, 400);
    //auto foo { tgui::AutoLayout:: }
    layout->setAutoLayout(tgui::AutoLayout::Left);



    return layout;
}


static glm::vec3 lightPos{ 1.2f, 1.f, 2.f };

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

    auto colorPicker{ tgui::ColorPicker::create("Цвет источника света", tgui::Color::White) };

	auto renderer{ std::make_shared<tgui::LabelRenderer>( )};
	renderer.get()->setBackgroundColor({28, 29, 31});
	renderer.get()->setTextColor({210, 219, 222});

	mousePosLabel->setRenderer(renderer.get()->getData());
    deltaLabel->setRenderer(renderer.get()->getData());

    auto colorPickerRenderer{ colorPicker->getRenderer() };
    colorPickerRenderer->setBackgroundColor(renderer->getBackgroundColor());
    colorPickerRenderer->setTitleColor(renderer->getTextColor());
    colorPicker->setHeight(100);
    colorPicker->setWidth(200);

	layout->add(mousePosLabel);
    layout->add(deltaLabel);
    layout->add(colorPicker);

	const glow::ShaderProgram shaderProgram{std::make_tuple(
		glow::VertexShader{"shaders/shader.vert"},
		glow::FragmentShader{"shaders/shader.frag"},
		glow::GeometryShader{}
	)};

	const glow::ShaderProgram lightSourceSP{ std::make_tuple(
		glow::VertexShader{ "shaders/light-source.vert" },
		glow::FragmentShader{ "shaders/light-source.frag" },
		glow::GeometryShader{}
	)};

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

    glow::Error::printIfError();

	u32 vbo, cubeVao;
    glGenVertexArrays(1, &cubeVao);
	glGenBuffers(1, &vbo);
    glow::Error::printIfError();

	glBindVertexArray(cubeVao);

    glow::Error::printIfError();

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glNamedBufferData(
	    vbo,
	    vertices.size() * sizeof(vertices.front()),
	    vertices.data(),
	    GL_STATIC_DRAW
	);

    glow::Error::printIfError();

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

    glow::Error::printIfError();

	tde::Camera cam;

	sf::Clock deltaClock;
    sf::Time avgFrameTime;

	sf::Vector2i prevMouseDelta{0, 0};

    f32 rotation{};

    bool isCursorMode{false};

	while (win.isOpen())
	{
	    const auto delta{ deltaClock.restart()};
        avgFrameTime = countAvgFrameTime(delta);

	    while (const std::optional event{ win.pollEvent()})
	    {
	        gui.handleEvent(*event);

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

    	glClearColor(20.f/ 255, 20.f/255, 20.f/255, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderProgram.use();

	    shaderProgram.setVec3("objectColor", {1.f, 0.5f, 0.31f});
	    shaderProgram.setVec3("lightColor", {1.f, 1.0f, 1.f});
	    shaderProgram.setVec3("lightPos", lightPos);

	    const auto view{ cam.getPositionAndDirection(delta) };
	    shaderProgram.setMatrix("view", view);
		shaderProgram.setVec3("viewPos", cam.getPos());
		

	    const auto perspectiveProjection
	    { glm::perspective(
            glm::radians(cam.getFov()),
            static_cast<f32>(win.getSize().x)/static_cast<f32>(win.getSize().y),
            0.1f,
            100.f
        )};
	    shaderProgram.setMatrix("projection", perspectiveProjection);

	    auto model{ glm::mat4{1.f} };
	    rotation += delta.asSeconds() * 10;
	    model = glm::rotate(model, glm::radians(rotation), {0.f, 1.f, 0.f});
	    shaderProgram.setMatrix("model",  model);

	    glBindVertexArray(cubeVao);

	    glow::Error::printIfError();

	    glDrawArrays(GL_TRIANGLES, 0, 36);

	    lightSourceSP.use();
	    lightSourceSP.setMatrix("view", view);
        lightSourceSP.setMatrix("projection", perspectiveProjection);

		model = glm::translate(glm::mat4{1}, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightSourceSP.setMatrix("model", model);

	    glBindVertexArray(lightVAO);
    	glDrawArrays(GL_TRIANGLES, 0, 36);

		gui.draw();

		glow::Error::printIfError();

		win.display();
    }

	glDeleteProgram(shaderProgram.getId());
    glDeleteProgram(lightSourceSP.getId());
	glDeleteVertexArrays(1, &cubeVao);
	glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &lightVAO);

    return EXIT_SUCCESS;
}