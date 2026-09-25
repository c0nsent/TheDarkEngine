cmake_minimum_required(VERSION 4.2)

project(TheDarkEngine
        LANGUAGES CXX
        VERSION 0.0.1
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.5 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)

add_subdirectory(third-party/glad)
add_subdirectory(glow)

find_path(STB_INCLUDE_DIR
        NAMES stb_image.h
        PATH_SUFFIXES stb
)

if(NOT STB_INCLUDE_DIR)
    message(FATAL_ERROR "stb headers not found. Please install stb (e.g., 'sudo pacman -S stb')")
endif()

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_INCLUDE_DIR})

add_executable(
        ${PROJECT_NAME}
        main.cc
)

file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders_stamp
        DEPENDS ${SHADER_FILES}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/shaders_stamp
        COMMENT "Копирование шейдеров при изменении"
)
add_custom_target(copy_shaders DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shaders_stamp)
add_dependencies(${PROJECT_NAME} copy_shaders)

# Копирование текстур
file(GLOB_RECURSE TEXTURE_FILES "textures/*")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/textures_stamp
        DEPENDS ${TEXTURE_FILES}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/textures $<TARGET_FILE_DIR:${PROJECT_NAME}>/textures
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/textures_stamp
        COMMENT "Копирование текстур при изменении"
)
add_custom_target(copy_textures DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/textures_stamp)
add_dependencies(${PROJECT_NAME} copy_textures)

target_link_libraries(${PROJECT_NAME}
        glfw
        glm::glm
        imgui::imgui
        stb
        glad
        glow
)