#pragma once

#include "basic-types.hpp"

#include <optional>
#include <string>


namespace glow
{
	namespace detail
	{
		class BaseShader
		{
		protected:

			enum class InfoType : u16
			{
				Type = 0x8B4F,
				DeleteStatus = 0x8B80,
				CompileStatus = 0x8B81,
				InfoLogLength = 0x8B84,
				SourceLength = 0x8B88,
			};

			enum class ShaderType : u16
			{
				Fragment = 0x8B30,
				Vertex = 0x8B31,
				Geometry = 0x8DD9,
			};

			static constexpr u32 NONE{0};

			[[nodiscard]] auto get(InfoType info) const noexcept -> i32;

			 auto compile(const char *path) const -> void;

		public:

			[[nodiscard]] auto isMarkedForDeletion() const noexcept -> bool;
			[[nodiscard]] auto isCompiled() const noexcept -> bool;
			[[nodiscard]] auto getInfoLogLength() const noexcept -> isize;
			[[nodiscard]] auto getSourceLength() const noexcept -> usize;

			[[nodiscard]] auto isExists() const noexcept -> bool;
			[[nodiscard]] auto getId() const noexcept -> u32;
			[[nodiscard]] auto getInfoLog() const -> std::optional<std::string>;
			auto deleteShader() const noexcept -> void;

			static void swap(BaseShader &lhs, BaseShader &rhs ) noexcept;

			BaseShader(BaseShader &&rhs) noexcept;
			auto operator=(BaseShader &&rhs) noexcept -> BaseShader &;

			~BaseShader();

			BaseShader(const BaseShader &) = delete;
			BaseShader &operator=(const BaseShader &) = delete;

		protected:

			BaseShader() = default;
			explicit BaseShader(ShaderType type) noexcept;

			u32 m_id;
		};
	}


	class VertexShader final : public detail::BaseShader
	{
	public:

		VertexShader() noexcept;
		explicit VertexShader(const char *srcPath);
	};


	class FragmentShader final : public detail::BaseShader
	{
	public:

		FragmentShader() noexcept;
		explicit FragmentShader(const char *srcPath);
	};


	class GeometryShader final : public detail::BaseShader
	{
	public:

		GeometryShader() noexcept;
		explicit GeometryShader(const char *srcPath);
	};
}