#pragma once

#include "basic-types.hpp"

#include <source_location>


namespace glow
{
	class Error
	{
		using srcLoc = std::source_location;

	public:

		enum class Code : u16
		{
			NoError = 0,
			InvalidEnum = 0x0500,
			InvalidValue = 0x0501,
			InvalidOperation = 0x0502,
			StackOverflow = 0x0503,
			StackUnderflow = 0x0504,
			OutOfMemory = 0x0505,
			InvalidFrameBufferOperation = 0x0506,
		};

		[[nodiscard]] static auto getErrorCode() noexcept -> Code;
		[[nodiscard]] static auto clearAndGetLastErrorCode() noexcept -> Code;

		static auto clear() noexcept -> void;

		static auto print(srcLoc loc = srcLoc::current()) -> void;
		static auto printIfError(srcLoc loc = srcLoc::current()) -> void;

	private:

	    static auto print(Code code, srcLoc loc) -> void;
	    [[nodiscard]] static constexpr  auto toString(Code code) -> const char *;
	};
}
