#include "shader.hpp"

#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <utility>


namespace glow
{
	namespace detail
	{
		auto BaseShader::get(const InfoType info) const noexcept -> i32
		{
			i32 requested;
			glGetShaderiv(m_id, std::to_underlying(info), &requested);

			return requested;
		}


		auto BaseShader::compile(const char *path) const -> void
		{
			std::ifstream file{path};

			if (not file)
				throw std::runtime_error{std::string{"Cannot open shader source file by path: "} + path};

			const std::string source{
				std::istreambuf_iterator{file},
				std::istreambuf_iterator<char>{}
			};

			const auto cStr{ source.c_str() };
			const auto sourceLength{ static_cast<GLint>(source.size()) };

			glShaderSource(m_id, 1, &cStr,  &sourceLength);
			glCompileShader(m_id);

			if (isCompiled()) return;

			auto log{getInfoLog().value_or("No log")};
			throw std::runtime_error{std::string{"Shader compilation failed: "} +  path + '\n' + log};
		}


		auto BaseShader::isMarkedForDeletion() const noexcept -> bool
		{
			return get(InfoType::DeleteStatus);
		}


		auto BaseShader::isCompiled() const noexcept -> bool
		{
			return get(InfoType::CompileStatus);
		}


		auto BaseShader::getInfoLogLength() const noexcept -> isize
		{
			return get(InfoType::InfoLogLength);
		}


		auto BaseShader::getSourceLength() const noexcept -> usize
		{
			return get(InfoType::SourceLength);
		}


		auto BaseShader::isExists() const noexcept -> bool
		{
			return m_id != NONE;
		}


		auto BaseShader::getId() const noexcept -> u32
		{
			return m_id;
		}


		auto BaseShader::getInfoLog() const -> std::optional<std::string>
		{
			if (m_id == NONE) return std::nullopt;

			constexpr GLsizei maxLogLength{256}; //Я так чувствую

			std::string infoLog(static_cast<usize>(maxLogLength), '\0');
			GLsizei logLength{};
			glGetShaderInfoLog(m_id, maxLogLength, &logLength, infoLog.data());
			infoLog.resize(static_cast<usize>(logLength));

			return infoLog;
		}


		void BaseShader::markForDeletion() const noexcept
		{
			glDeleteShader(m_id);
		}


		void BaseShader::swap(BaseShader &lhs, BaseShader &rhs) noexcept
		{
			const auto temp{lhs.getId()};
			lhs.m_id = rhs.m_id;
			rhs.m_id = temp;
		}


		BaseShader::BaseShader(BaseShader &&rhs) noexcept : BaseShader{}
		{
			swap(*this, rhs);
		}


		auto BaseShader::operator=(BaseShader &&rhs) noexcept -> BaseShader &
		{
			swap(*this, rhs);

			return *this;
		}


		BaseShader::~BaseShader()
		{
			if (m_id != NONE) glDeleteShader(m_id);
		}

		BaseShader::BaseShader() noexcept : m_id{NONE}{}

		BaseShader::BaseShader(const ShaderType type) noexcept
			: m_id{glCreateShader(std::to_underlying(type))}
		{
		}
	}


	VertexShader::VertexShader(const char *srcPath)
		: BaseShader{ShaderType::Vertex}
	{
		compile(srcPath);
	}

	FragmentShader::FragmentShader() noexcept : BaseShader() {}


	FragmentShader::FragmentShader(const char *srcPath)
		: BaseShader{ShaderType::Fragment}
	{
		compile(srcPath);
	}

	GeometryShader::GeometryShader() noexcept : BaseShader{} {}


	GeometryShader::GeometryShader(const char *srcPath)
	: BaseShader{ShaderType::Geometry}
	{
		compile(srcPath);
	}
}
