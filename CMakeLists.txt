cmake_minimum_required(VERSION 3.21)
project(gbc C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(externals)

set(CMAKE_C_STANDARD 23)
file(GLOB SOURCES "src/*.c" "include/*.h")
include_directories(include)
add_executable(gbc ${SOURCES})
target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
