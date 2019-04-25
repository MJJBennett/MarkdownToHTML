cmake_minimum_required(VERSION 3.3)
project(MarkdownToHTML LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(MarkdownToHTML app.cpp)

target_include_directories(MarkdownToHTML PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

enable_testing()
add_subdirectory(test)
