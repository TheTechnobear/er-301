cmake_minimum_required(VERSION 3.15)

set(LODEPNG_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libs/lodepng)

set(LODEPNG_SOURCES
    ${LODEPNG_SOURCE_ROOT}/lodepng.cpp
)

add_library(lodepng STATIC ${LODEPNG_SOURCES})

target_include_directories(lodepng PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/lodepng>
)

target_compile_options(lodepng PRIVATE
    -std=gnu++11
    -Wall
    -fPIC
)