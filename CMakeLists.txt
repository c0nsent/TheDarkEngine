cmake_minimum_required(VERSION 4.2)

project(TheDarkEngine
        LANGUAGES CXX
        VERSION 0.0.2
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm REQUIRED)
find_package(
        SFML REQUIRED COMPONENTS
        Graphics
        System
)

find_package(TGUI REQUIRED)

add_subdirectory(third-party/glad)
add_subdirectory(glow)

add_executable(
        ${PROJECT_NAME}
        camera.cc camera.hpp
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
file(GLOB_RECURSE TEXTURE_FILES "assets/*")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/textures_stamp
        DEPENDS ${TEXTURE_FILES}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/textures_stamp
        COMMENT "Копирование текстур при изменении"
)
add_custom_target(copy_textures DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/textures_stamp)
add_dependencies(${PROJECT_NAME} copy_textures)

target_link_libraries(${PROJECT_NAME}
        SFML::System
        SFML::Graphics
        glad
        glow
        glm::glm
        TGUI::TGUI
)