#pragma once

namespace glow
{
	namespace basicTypes
	{
		using f16 = _Float16  ;
		using f32 = float;
		using f64 = double;
		using f128 = _Float128;

		using u8 = unsigned char;
		using u16 = unsigned short int;
		using u32 = unsigned int;
		using u64 = unsigned long int;

		using i8 = signed char;
		using i16 = signed short int;
		using i32 = signed int;
		using i64 = signed long int;

		using usize = u32;
		using isize = i32;
	}
	using namespace glow::basicTypes;

	struct Color
	{
		f32 red{};
		f32 green{};
		f32 blue{};
		f32 alpha{1.f};
	};
}