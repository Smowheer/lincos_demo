# based on
# https://github.com/bagage/AntTweakBar/blob/master/CMakeLists.txt
cmake_minimum_required(VERSION 2.8)
project(AntTweakBar)

################### Build the library ###################
if (APPLE)
    set (platform MACOSX)
elseif (UNIX)
    set (platform UNIX)
elseif(MSVC)
    set (platform WINDOWS)
else()
    message(SEND_ERROR "This platform (${CMAKE_SYSTEM_NAME}) is not handled yet! Consider a pull-request.")
endif()

set(ANTWB_SOURCES
    src/TwColors.cpp
    src/TwFonts.cpp
    src/TwOpenGL.cpp
    src/TwOpenGLCore.cpp
    src/TwBar.cpp
    src/TwMgr.cpp
    src/TwPrecomp.cpp
    src/LoadOGL.cpp
    src/LoadOGLCore.cpp
    src/TwEventGLFW.c
    src/TwEventGLUT.c
    src/TwEventSDL.c
    src/TwEventSDL12.c
    src/TwEventSDL13.c
    src/TwEventSFML.cpp
)

# platform dependent event handler
if(UNIX)
    set(ANTWB_SOURCES ${ANTWB_SOURCES} src/TwEventX11.c)
# on windows
elseif(NOT APPLE)
    set(ANTWB_SOURCES ${ANTWB_SOURCES} src/TwEventWin.c
                                    src/TwDirect3D10.cpp
                                    src/TwDirect3D9.cpp
                                    src/TwDirect3D11.cpp)
endif()

add_library(AntTweakBar STATIC ${ANTWB_SOURCES})
target_include_directories(AntTweakBar PUBLIC ${PROJECT_SOURCE_DIR}/include)

# must be public, otherwise msvc expects dll when including headers 
target_compile_definitions(AntTweakBar PUBLIC TW_STATIC _${platform})

# prevent header from causing automatic link to library with weird name
target_compile_definitions(AntTweakBar PUBLIC TW_NO_LIB_PRAGMA)
# compile flags
if(NOT MSVC)
    target_compile_options(AntTweakBar PRIVATE -Wall -fPIC -fno-strict-aliasing)
    target_compile_definitions(AntTweakBar PRIVATE __PLACEMENT_NEW_INLINE)
else()
    target_compile_options(AntTweakBar PRIVATE /W4)
    # export interface functions when generating dll
    # target_compile_definitions(AntTweakBar PRIVATE TW_EXPORTS)
endif()