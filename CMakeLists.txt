cmake_minimum_required(VERSION 3.29)
project(cpp_open_canvas)

set(CMAKE_CXX_STANDARD 20 -pedantic)

add_library(cppoc STATIC window.cpp window.h)
link_directories(/usr/lib/x86_64-linux-gnu/)
target_link_libraries(cppoc PRIVATE GLEW)
target_link_libraries(cppoc PRIVATE glfw)
target_link_libraries(cppoc PRIVATE GL)

add_executable(cpp_open_canvas main.cpp)
target_link_libraries(cpp_open_canvas PRIVATE cppoc)
