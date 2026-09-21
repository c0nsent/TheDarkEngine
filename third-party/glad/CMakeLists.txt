project(glad
		LANGUAGES C
		VERSION 4.6
)

add_library(glad SHARED)
target_sources(glad PRIVATE src/glad.c)
target_include_directories(glad PUBLIC include)