cmake_minimum_required(VERSION 3.15)

set(MINIZ_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libs/miniz)

set(MINIZ_SOURCES
    ${MINIZ_SOURCE_ROOT}/miniz.c
)

add_library(miniz STATIC ${MINIZ_SOURCES})

target_include_directories(miniz PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/miniz>
)

target_compile_options(miniz PRIVATE
    -std=gnu11
    -Wall
    -fPIC
)