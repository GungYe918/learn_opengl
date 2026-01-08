cmake_minimum_required(VERSION 3.16)
project(cg101_ch3-5 LANGUAGES C CXX)

# C++ version 20으로 고정
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ch3-5
    src/main.cpp
    external/glad/src/glad.c
)

target_include_directories(ch3-5 PRIVATE
    external/glad/include
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

target_include_directories(ch3-5 PRIVATE ${GLFW_INCLUDE_DIRS})
target_link_libraries(ch3-5 PRIVATE ${GLFW_LIBRARIES} dl GL)