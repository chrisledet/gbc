cmake_minimum_required(VERSION 3.21)
project(gbc C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(externals)

set(CMAKE_C_STANDARD 23)
file(GLOB SOURCES "src/*.c" "include/*.h")
include_directories(include)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(gbc ${SOURCES})

if (TARGET SDL2::SDL2main)
	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
