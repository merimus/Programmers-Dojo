cmake_minimum_required(VERSION 3.8.0)
project(Mandelbrot VERSION 0.1.0)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
link_libraries(${OPENGL_gl_LIBRARY})

find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/imgui")

find_package(Boost REQUIRED)
include_directories(Boost::headers)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    link_libraries(OpenMP::OpenMP_CXX)
endif()

file(GLOB sources *.cpp imgui/*.cpp)
add_executable(Mandelbrot ${sources})
target_link_libraries(Mandelbrot GLEW::GLEW glfw)

configure_file(${CMAKE_SOURCE_DIR}/fragment.glsl ${CMAKE_CURRENT_BINARY_DIR}/fragment.glsl COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/vertex.glsl ${CMAKE_CURRENT_BINARY_DIR}/vertex.glsl COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/mandelbrot.glsl ${CMAKE_CURRENT_BINARY_DIR}/mandelbrot.glsl COPYONLY)

