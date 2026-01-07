cmake_minimum_required(VERSION 3.16)
project(cg101_ch1 LANGUAGES C CXX)

# C++ version 20으로 고정
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ch1
    src/main.cpp
    external/glad/src/glad.c
)

target_include_directories(ch1 PRIVATE
    external/glad/include
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

target_include_directories(ch1 PRIVATE ${GLFW_INCLUDE_DIRS})
target_link_libraries(ch1 PRIVATE ${GLFW_LIBRARIES} dl GL)
