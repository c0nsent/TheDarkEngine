project(glow
        VERSION 0.0.2
        LANGUAGES CXX
        DESCRIPTION "OpenGL Lightweight Object Wrapper"
)

set(CMAKE_CXX_STANDARD 23)

add_library(glow SHARED
        basic-types.hpp
        shader.cc shader.hpp
)

target_link_libraries(glow PUBLIC
        glad
)
