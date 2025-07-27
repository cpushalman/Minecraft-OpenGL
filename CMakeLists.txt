cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)

# Set include and lib directories
include_directories(include)
link_directories(lib)

# Gather source files
file(GLOB SOURCES "src/*.cpp" "src/*.c")

# Create executable
add_executable(OpenGLProject ${SOURCES})

# Link with OpenGL and GLFW
target_link_libraries(OpenGLProject glfw3 opengl32)
