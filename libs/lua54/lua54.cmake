cmake_minimum_required(VERSION 3.15)

set(LUA54_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libs/lua54)

set(LUA54_SOURCES
    ${LUA54_SOURCE_ROOT}/lapi.c
    ${LUA54_SOURCE_ROOT}/lauxlib.c
    ${LUA54_SOURCE_ROOT}/lbaselib.c
    ${LUA54_SOURCE_ROOT}/lcode.c
    ${LUA54_SOURCE_ROOT}/ldblib.c
    ${LUA54_SOURCE_ROOT}/ldebug.c
    ${LUA54_SOURCE_ROOT}/ldo.c
    ${LUA54_SOURCE_ROOT}/ldump.c
    ${LUA54_SOURCE_ROOT}/lfunc.c
    ${LUA54_SOURCE_ROOT}/lgc.c
    ${LUA54_SOURCE_ROOT}/linit.c
    ${LUA54_SOURCE_ROOT}/liolib.c
    ${LUA54_SOURCE_ROOT}/llex.c
    ${LUA54_SOURCE_ROOT}/lmathlib.c
    ${LUA54_SOURCE_ROOT}/lmem.c
    ${LUA54_SOURCE_ROOT}/loadlib.c
    ${LUA54_SOURCE_ROOT}/lobject.c
    ${LUA54_SOURCE_ROOT}/lopcodes.c
    ${LUA54_SOURCE_ROOT}/lparser.c
    ${LUA54_SOURCE_ROOT}/lstate.c
    ${LUA54_SOURCE_ROOT}/lstring.c
    ${LUA54_SOURCE_ROOT}/lstrlib.c
    ${LUA54_SOURCE_ROOT}/ltable.c
    ${LUA54_SOURCE_ROOT}/ltablib.c
    ${LUA54_SOURCE_ROOT}/ltm.c
    ${LUA54_SOURCE_ROOT}/lctype.c
    ${LUA54_SOURCE_ROOT}/lcorolib.c
    ${LUA54_SOURCE_ROOT}/loslib.c
    ${LUA54_SOURCE_ROOT}/lundump.c
    ${LUA54_SOURCE_ROOT}/lutf8lib.c
    ${LUA54_SOURCE_ROOT}/lvm.c
    ${LUA54_SOURCE_ROOT}/lzio.c
)

add_library(lua54 STATIC ${LUA54_SOURCES})

target_include_directories(lua54 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/lua54>
)

target_compile_options(lua54 PRIVATE
    -std=gnu11
    -Wall
    -fPIC
)

target_compile_definitions(lua54 PRIVATE
    LUA_USE_REALLOC
)