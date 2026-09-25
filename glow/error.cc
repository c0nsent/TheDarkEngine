#include "error.hpp"

#include <glad/glad.h>

#include <print>


namespace glow
{
	auto Error::getErrorCode() noexcept -> Code { return static_cast<Code>(glGetError()); }

	auto Error::clearAndGetLastErrorCode() noexcept -> Code
	{
		Code previous{getErrorCode()};
		Code last{previous};

		while (last != Code::NoError)
		{
			previous = last;
			last = getErrorCode();
		}

		return previous;
	}

	auto Error::clear() noexcept -> void { while (getErrorCode() != Code::NoError) {} }


	auto Error::print(const srcLoc loc) -> void { print(getErrorCode(), loc); }


	auto Error::printIfError(const srcLoc loc) -> void
	{
		if (const Code code{getErrorCode()}; code != Code::NoError) print(code, loc);
	}


	auto Error::print(const Code code, const srcLoc loc) -> void
	{
		std::println("[ {0} | {1} | {2}] Error: {3}", loc.file_name(), loc.function_name(), loc.line(), toString(code));
	}


	constexpr auto Error::toString(const Code code) -> const char *
	{
		switch (code)
		{
			case Code::NoError: return "No error";
			case Code::InvalidEnum: return "Invalid enum";
			case Code::InvalidValue: return "Invalid value";
			case Code::InvalidOperation: return "Invalid operation";
			case Code::StackOverflow: return "Stack overflow";
			case Code::StackUnderflow: return "Stack underflow";
			case Code::OutOfMemory: return "Out of memory";
			case Code::InvalidFrameBufferOperation: return "Invalid frame buffer operation";
		}

		return "Unknown error";
	}
}
