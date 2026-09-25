#pragma once

#include "basic-types.hpp"
#include "shader.hpp"
//#include "uniform.hpp"

#include <tuple>


namespace glow
{
	class ShaderProgram final
	{
	    using Shaders = std::tuple<VertexShader, FragmentShader, GeometryShader>;

		enum class InfoType : u16
		{
			DeleteStatus = 0x8B80,
			LinkStatus = 0x8B82,
			ValidateStatus = 0x8B83,
			InfoLogLength = 0x8B84,
			AttachedShaders = 0x8B85,
		};

		[[nodiscard]] auto get(InfoType info) const noexcept -> i32;

	public:

		/*struct CreateInfo final
		{
			VertexShader vertexShader;
			FragmentShader fragmentShader;
			GeometryShader geometryShader;
		};*/



		explicit ShaderProgram(Shaders &&shaders);
		//ShaderProgram(const char *vertexPath, const char *fragmentShader);

		//~ShaderProgram();

		ShaderProgram() = delete;
		ShaderProgram(const ShaderProgram &) = delete;
		ShaderProgram(ShaderProgram &&) = delete;
		ShaderProgram &operator=(const ShaderProgram &) = delete;

		[[nodiscard]] auto getId() const noexcept -> u32;
		[[nodiscard]] auto isLinked() const noexcept -> bool;
		[[nodiscard]] auto getLogLength() const noexcept -> isize;
		[[nodiscard]] auto getInfoLog() const -> std::optional<std::string>;

		template<class ShaderType> requires std::derived_from<ShaderType, detail::BaseShader>
		[[nodiscard]] auto hasAttachedShader() const noexcept -> bool;

		template<class ShaderType> requires std::derived_from<ShaderType, detail::BaseShader>
		[[nodiscard]] auto getAttachedShader() const noexcept ->  const ShaderType &;

	    //[[nodiscard]] auto getUniform1f(const char *name) const noexcept -> Uniform1f;

		auto use() const noexcept -> void;

	private:

		Shaders m_shaders;

		u32 m_id;
	};
}
