project(glow
        VERSION 0.0.2
        LANGUAGES CXX
        DESCRIPTION "OpenGL Lightweight Object Wrapper"
)

set(CMAKE_CXX_STANDARD 23)

add_library(glow SHARED
        basic-types.hpp
        shader.cc shader.hpp
        shader-program.cc shader-program.hpp
        error.cc error.hpp
)

target_link_libraries(glow PRIVATE
        glad
)
