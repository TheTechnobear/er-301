cmake_minimum_required(VERSION 3.15)

set(PERCUSSA_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/percussa)
set(OD_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/od)
set(LIBS_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(SWIG_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(SWIG_INCLUDES
    -I${CMAKE_CURRENT_SOURCE_DIR}
    -I${CMAKE_CURRENT_SOURCE_DIR}/od
    -I${CMAKE_CURRENT_SOURCE_DIR}/percussa
    -I${CMAKE_CURRENT_SOURCE_DIR}/hal
    -I${CMAKE_CURRENT_SOURCE_DIR}/percussa/hal
    -I${CMAKE_CURRENT_SOURCE_DIR}/arch/darwin
    -I${CMAKE_CURRENT_SOURCE_DIR}/arch/linux
    -I${LIBS_SOURCE_ROOT}/lua54
    -I${LIBS_SOURCE_ROOT}/lodepng
    -I${LIBS_SOURCE_ROOT}/miniz
)

find_package(SWIG)
if(SWIG_FOUND)
    include(${SWIG_USE_FILE})
endif()

set(SWIG_APP_SWG_CPP ${SWIG_OUTPUT_DIR}/app_swig.cpp)

add_custom_command(
    OUTPUT ${SWIG_APP_SWG_CPP}
    COMMAND ${SWIG_EXECUTABLE} -lua -no-old-metatable-bindings -fvirtual -fcompact -c++ ${SWIG_INCLUDES} -o ${SWIG_APP_SWG_CPP} ${OD_SOURCE_ROOT}/glue/app.cpp.swig
    DEPENDS ${OD_SOURCE_ROOT}/glue/app.cpp.swig
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating app_swig.cpp from app.cpp.swig"
)

set(PERCUSSA_COMMON_SOURCES
    ${PERCUSSA_SOURCE_ROOT}/app/Bootstrap.cpp
    ${PERCUSSA_SOURCE_ROOT}/app/CardState.cpp
    ${PERCUSSA_SOURCE_ROOT}/app/EncoderProxy.cpp
    ${PERCUSSA_SOURCE_ROOT}/hal/adc.c
    ${PERCUSSA_SOURCE_ROOT}/hal/audio.c
    ${PERCUSSA_SOURCE_ROOT}/hal/card.cpp
    ${PERCUSSA_SOURCE_ROOT}/hal/concurrency/EventFlags.cpp
    ${PERCUSSA_SOURCE_ROOT}/hal/concurrency/Mutex.cpp
    ${PERCUSSA_SOURCE_ROOT}/hal/concurrency/Thread.cpp
    ${PERCUSSA_SOURCE_ROOT}/hal/dir.c
    ${PERCUSSA_SOURCE_ROOT}/hal/display.cpp
    ${PERCUSSA_SOURCE_ROOT}/hal/encoder.cpp
    ${PERCUSSA_SOURCE_ROOT}/hal/events.cpp
    ${PERCUSSA_SOURCE_ROOT}/hal/fft.c
    ${PERCUSSA_SOURCE_ROOT}/hal/fileops.c
    ${PERCUSSA_SOURCE_ROOT}/hal/gpio.c
    ${PERCUSSA_SOURCE_ROOT}/hal/log.c
    ${PERCUSSA_SOURCE_ROOT}/hal/modulation.c
    ${PERCUSSA_SOURCE_ROOT}/hal/pwm.c
    ${PERCUSSA_SOURCE_ROOT}/hal/rng.c
    ${PERCUSSA_SOURCE_ROOT}/hal/simd.c
    ${PERCUSSA_SOURCE_ROOT}/hal/timing.c
    ${PERCUSSA_SOURCE_ROOT}/hal/uart.c
    ${PERCUSSA_SOURCE_ROOT}/hal/usb.cpp
    ${PERCUSSA_SOURCE_ROOT}/runtime/Runtime.cpp
    ${PERCUSSA_SOURCE_ROOT}/support/CommandLine.cpp
    ${PERCUSSA_SOURCE_ROOT}/support/KeyValueStore.cpp
    ${PERCUSSA_SOURCE_ROOT}/support/tls.c
    ${PERCUSSA_SOURCE_ROOT}/ui/BiButtonWidget.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/ButtonWidget.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/DisplayWidget.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/EncoderWidget.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/LedWidget.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/MainDisplayWidget.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/PanelRenderer.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/SubDisplayWidget.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/ToggleWidget.cpp
    ${PERCUSSA_SOURCE_ROOT}/ui/olive_impl.c
    ${PERCUSSA_SOURCE_ROOT}/od/glue/AppInterpreter.cpp
    ${PERCUSSA_SOURCE_ROOT}/od/glue/Interpreter.cpp
    ${PERCUSSA_SOURCE_ROOT}/od/config.c
    ${SWIG_APP_SWG_CPP}
)

set(PERCUSSA_PANEL_SOURCES "")
if(PERCUSSA_PANEL STREQUAL "ssp")
    list(APPEND PERCUSSA_PANEL_SOURCES
        ${PERCUSSA_SOURCE_ROOT}/panel/Panel.cpp
        ${PERCUSSA_SOURCE_ROOT}/panel/ssp/SspController.cpp
        ${PERCUSSA_SOURCE_ROOT}/panel/ssp/SspPanel.cpp
    )
elseif(PERCUSSA_PANEL STREQUAL "xmx")
    list(APPEND PERCUSSA_PANEL_SOURCES
        ${PERCUSSA_SOURCE_ROOT}/panel/Panel.cpp
        ${PERCUSSA_SOURCE_ROOT}/panel/xmx/XmxController.cpp
        ${PERCUSSA_SOURCE_ROOT}/panel/xmx/XmxPanel.cpp
    )
endif()

set(PERCUSSA_PLATFORM_SOURCES "")
if(PERCUSSA_PLATFORM STREQUAL "host-sdl")
    list(APPEND PERCUSSA_PLATFORM_SOURCES
        ${PERCUSSA_SOURCE_ROOT}/platform/HostSdlPlatform.cpp
    )
    list(APPEND PERCUSSA_PLATFORM_SOURCES
        ${LIBS_SOURCE_ROOT}/SDL_FontCache/SDL_FontCache.c
    )
elseif(PERCUSSA_PLATFORM STREQUAL "fbdev")
    list(APPEND PERCUSSA_PLATFORM_SOURCES
        ${PERCUSSA_SOURCE_ROOT}/platform/FbdevPlatform.cpp
        ${PERCUSSA_SOURCE_ROOT}/platform/fbdev/Framebuffer.cpp
        ${PERCUSSA_SOURCE_ROOT}/platform/fbdev/HardwareInput.cpp
    )
    if(PERCUSSA_PANEL STREQUAL "ssp")
        list(APPEND PERCUSSA_PLATFORM_SOURCES
            ${PERCUSSA_SOURCE_ROOT}/platform/fbdev/ButtonMap_SSP.cpp
        )
    elseif(PERCUSSA_PANEL STREQUAL "xmx")
        list(APPEND PERCUSSA_PLATFORM_SOURCES
            ${PERCUSSA_SOURCE_ROOT}/platform/fbdev/ButtonMap_XMX.cpp
        )
    endif()
elseif(PERCUSSA_PLATFORM STREQUAL "plugin")
    list(APPEND PERCUSSA_PLATFORM_SOURCES
        ${PERCUSSA_SOURCE_ROOT}/platform/PluginPlatform.cpp
    )
endif()

set(PERCUSSA_ARCH_SOURCES "")
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND PERCUSSA_ARCH_SOURCES
        ${PERCUSSA_SOURCE_ROOT}/hal/card_macos.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/darwin/hal/heap.c
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND PERCUSSA_ARCH_SOURCES
        ${PERCUSSA_SOURCE_ROOT}/hal/card_linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/linux/hal/heap.c
    )
    if(NOT CMAKE_CROSSCOMPILING)
        list(APPEND PERCUSSA_ARCH_SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/arch/linux/hal/fileops.c
        )
    endif()
endif()

set(PERCUSSA_ALL_SOURCES
    ${PERCUSSA_COMMON_SOURCES}
    ${PERCUSSA_PANEL_SOURCES}
    ${PERCUSSA_PLATFORM_SOURCES}
    ${PERCUSSA_ARCH_SOURCES}
    ${PERCUSSA_SOURCE_ROOT}/main.cpp
)

add_executable(percussa ${PERCUSSA_ALL_SOURCES})

target_include_directories(percussa PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PERCUSSA_SOURCE_ROOT}
    ${PERCUSSA_SOURCE_ROOT}/od/glue
    ${LIBS_SOURCE_ROOT}/SDL_FontCache
    ${LIBS_SOURCE_ROOT}/rtaudio
    ${LIBS_SOURCE_ROOT}/lodepng
    ${LIBS_SOURCE_ROOT}/miniz
    ${LIBS_SOURCE_ROOT}/lua54
)

if(PERCUSSA_PLATFORM STREQUAL "host-sdl")
    target_compile_definitions(percussa PRIVATE
        PERCUSSA_PLATFORM_HOST_SDL
    )
elseif(PERCUSSA_PLATFORM STREQUAL "fbdev")
    target_compile_definitions(percussa PRIVATE
        PERCUSSA_PLATFORM_FBDEV
    )
elseif(PERCUSSA_PLATFORM STREQUAL "plugin")
    target_compile_definitions(percussa PRIVATE
        PERCUSSA_PLATFORM_PLUGIN
    )
endif()

if(PERCUSSA_PANEL STREQUAL "ssp")
    target_compile_definitions(percussa PRIVATE TARGET_SSP)
elseif(PERCUSSA_PANEL STREQUAL "xmx")
    target_compile_definitions(percussa PRIVATE TARGET_XMX)
endif()

target_compile_definitions(percussa PRIVATE
    BUILDOPT_LUA_USE_REALLOC
    BUILDOPT_TESTING
    FIRMWARE_VERSION=\"0.0.0\"
    BUILD_PROFILE=\"testing\"
)

if(CMAKE_CROSSCOMPILING)
    target_compile_definitions(percussa PRIVATE
        _GNU_SOURCE
        __LINUX_ALSA__
    )
    target_include_directories(percussa PRIVATE
        ${CMAKE_SYSROOT}/usr/include
    )
    if(PERCUSSA_PLATFORM STREQUAL "host-sdl")
        target_include_directories(percussa PRIVATE
            ${CMAKE_SYSROOT}/usr/include/SDL2
        )
    endif()
endif()

set(RTAUDIO_SOURCES
    ${LIBS_SOURCE_ROOT}/rtaudio/RtAudio.cpp
    ${LIBS_SOURCE_ROOT}/rtaudio/rtaudio_c.cpp
)

add_library(rtaudio OBJECT ${RTAUDIO_SOURCES})
target_include_directories(rtaudio PRIVATE
    ${LIBS_SOURCE_ROOT}/rtaudio
)
target_compile_options(rtaudio PRIVATE
    -std=gnu++11
)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(rtaudio PRIVATE
        -Wno-vla-cxx-extension
    )
endif()
target_compile_definitions(rtaudio PRIVATE
    __RTAUDIO_DEBUG__
)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(rtaudio PRIVATE
        __MACOSX_CORE__
    )
endif()

target_sources(percussa PRIVATE
    $<TARGET_OBJECTS:rtaudio>
)

set_target_properties(percussa PROPERTIES
    LINKER_LANGUAGE CXX
)

target_link_libraries(percussa PRIVATE
    od
    lua54
    miniz
    lodepng
)

find_package(Threads REQUIRED)
target_link_libraries(percussa PRIVATE
    Threads::Threads
)

if(FFTW_STAGE_ROOT)
    target_include_directories(percussa PRIVATE ${FFTW_STAGE_ROOT}/include)
    target_link_libraries(percussa PRIVATE ${FFTW_STAGE_ROOT}/lib/libfftw3f.a)
endif()

if(PERCUSSA_PLATFORM STREQUAL "host-sdl")
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        execute_process(
            COMMAND brew --prefix sdl2
            OUTPUT_VARIABLE SDL2_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(SDL2_PREFIX)
            target_include_directories(percussa PRIVATE ${SDL2_PREFIX}/include/SDL2)
            target_include_directories(percussa PRIVATE ${SDL2_PREFIX}/include)
        endif()
        find_library(SDL2_LIBRARY SDL2)
        if(SDL2_LIBRARY)
            target_link_libraries(percussa PRIVATE ${SDL2_LIBRARY})
        endif()
        execute_process(
            COMMAND brew --prefix sdl2_ttf
            OUTPUT_VARIABLE SDL2_TTF_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(SDL2_TTF_PREFIX)
            target_include_directories(percussa PRIVATE ${SDL2_TTF_PREFIX}/include)
        endif()
        find_library(SDL2_TTF_LIBRARY SDL2_ttf)
        if(SDL2_TTF_LIBRARY)
            target_link_libraries(percussa PRIVATE ${SDL2_TTF_LIBRARY})
        endif()
        execute_process(
            COMMAND brew --prefix fftw
            OUTPUT_VARIABLE FFTW_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(FFTW_PREFIX)
            target_include_directories(percussa PRIVATE ${FFTW_PREFIX}/include)
        endif()
        find_library(FFTW3F_LIBRARY fftw3f)
        if(FFTW3F_LIBRARY)
            target_link_libraries(percussa PRIVATE ${FFTW3F_LIBRARY})
        endif()
        if(NOT FFTW3F_LIBRARY AND FFTW_STAGE_ROOT)
            target_include_directories(percussa PRIVATE ${FFTW_STAGE_ROOT}/include)
            target_link_libraries(percussa PRIVATE ${FFTW_STAGE_ROOT}/lib/libfftw3f.a)
        endif()
        target_link_libraries(percussa PRIVATE
            "-framework CoreAudio"
            "-framework CoreFoundation"
        )
    elseif(CMAKE_CROSSCOMPILING)
        target_link_libraries(percussa PRIVATE
            ${CMAKE_SYSROOT}/usr/lib/libSDL2.so
            ${CMAKE_SYSROOT}/usr/lib/libSDL2_ttf.so
        )
        if(FFTW_STAGE_ROOT)
            target_include_directories(percussa PRIVATE ${FFTW_STAGE_ROOT}/include)
            target_link_libraries(percussa PRIVATE ${FFTW_STAGE_ROOT}/lib/libfftw3f.a)
        endif()
    else()
        if(SDL2_FOUND)
            target_link_libraries(percussa PRIVATE ${SDL2_LIBRARIES})
        endif()
        if(SDL2_ttf_FOUND)
            target_link_libraries(percussa PRIVATE ${SDL2_ttf_LIBRARIES})
        endif()
        find_package(PkgConfig)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(FFTW3F fftw3f)
            if(FFTW3F_FOUND)
                target_include_directories(percussa PRIVATE ${FFTW3F_INCLUDE_DIRS})
                target_link_libraries(percussa PRIVATE ${FFTW3F_LIBRARIES})
            endif()
        endif()
        if(NOT FFTW3F_FOUND AND FFTW_STAGE_ROOT)
            target_include_directories(percussa PRIVATE ${FFTW_STAGE_ROOT}/include)
            target_link_libraries(percussa PRIVATE ${FFTW_STAGE_ROOT}/lib/libfftw3f.a)
        endif()
    endif()
else()
    if(FFTW_STAGE_ROOT)
        target_include_directories(percussa PRIVATE ${FFTW_STAGE_ROOT}/include)
        target_link_libraries(percussa PRIVATE ${FFTW_STAGE_ROOT}/lib/libfftw3f.a)
    endif()
    if(CMAKE_CROSSCOMPILING)
        target_link_libraries(percussa PRIVATE asound)
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(percussa PRIVATE
        "-lc++"
        "-lc++abi"
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
    target_link_libraries(percussa PRIVATE -rdynamic)
endif()

target_link_libraries(percussa PRIVATE -lm -ldl)