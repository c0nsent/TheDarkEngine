#include "shader-program.hpp"

#include <glad/glad.h>

#include <stdexcept>
#include <utility>

//#include "uniform.hpp"


namespace glow
{
	auto ShaderProgram::get(const InfoType info) const noexcept -> i32
	{
		i32 requested{};
		glGetProgramiv(m_id, std::to_underlying(info), &requested);
		return requested;
	}


	ShaderProgram::ShaderProgram(Shaders &&shaders)
		: m_shaders{std::move(shaders)}
		, m_id{glCreateProgram()}
	{
		auto attachIfExist{[&] (const auto &shader) noexcept
		{
			if (not shader.isExists()) return;

			glAttachShader(m_id, shader.getId());
			shader.markForDeletion();
		}};

	    std::apply([&](const auto &... args)
	    {
	        (attachIfExist(args), ...);
	    }, m_shaders);

		glLinkProgram(m_id);

		if (not isLinked())
		{
			throw std::runtime_error{"Failed to link shader program: " + getInfoLog().value_or("No Log")};
		}
	}


	/*ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(m_id);
	}*/


	auto ShaderProgram::getId() const noexcept -> u32
	{
		return m_id;
	}


	auto ShaderProgram::isLinked() const noexcept -> bool
	{
		return get(InfoType::LinkStatus);
	}


	auto ShaderProgram::getLogLength() const noexcept -> isize
	{
		return get(InfoType::InfoLogLength);
	}


	auto ShaderProgram::getInfoLog() const -> std::optional<std::string>
	{
		const auto infoLogLength{getLogLength()};

		if (infoLogLength == 0) return std::nullopt;

		char infoLog[infoLogLength];
		glGetProgramInfoLog(m_id, infoLogLength, nullptr, infoLog);

		return std::string{infoLog};
	}


	template<class ShaderType> requires std::derived_from<ShaderType, detail::BaseShader>
	auto ShaderProgram::hasAttachedShader() const noexcept  -> bool
	{
		return std::get<ShaderType>(m_shaders).isExists();
	}


	template<class ShaderType> requires std::derived_from<ShaderType, detail::BaseShader>
	auto ShaderProgram::getAttachedShader() const noexcept -> const ShaderType &
	{
		return std::get<ShaderType>(m_shaders);
	}


    /*auto ShaderProgram::getUniform1f(const char *name) const noexcept -> Uniform1f
	{
	    return Uniform1f{m_id, name};
	}*/


    auto ShaderProgram::use() const noexcept -> void
	{
		glUseProgram(m_id);
	}
}
