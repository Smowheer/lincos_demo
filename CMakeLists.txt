cmake_minimum_required (VERSION 3.0.2)
# allows access to environment variables with the name 
project (InCGFramework)

#set the build type if its not set
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE )
endif()

#if user didnt set install dir, override it and write it to the cache -> Type and description necessary,
#overwrite variable, not just write it to cache
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE STRING "Install path prefix, prepended onto install directories." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# set build type dependent flags
if(UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2")
  set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi")
endif()

#if user didnt set install dir, override it and write it to the cache -> Type and description necessary,
#overwrite variable, not just write it to cache
# if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#     set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE STRING "Install path prefix, prepended onto install directories." FORCE)
# endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# installation
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
#before adding libraries, define the output paths
# MSVC & Xcode automatically create the build-type folders
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}")

# Location where cmake first looks for modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(GLOBAL_EXT_DIR ${CMAKE_SOURCE_DIR}/external)
include(AddSubLibrary)

# glfw
# configure glfw building
option(GLFW_BUILD_DOCS     OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS    OFF)
option(GLFW_INSTALL        OFF)
# add glfw build system
add_subdirectory(external/glfw-3.1.1)
# include glfw headers
include_directories(SYSTEM external/glfw-3.1.1/include)

# add glbindings
option(OPTION_BUILD_TOOLS OFF)
option(OPTION_BUILD_TESTS OFF)
option(OPTION_BUILD_GPU_TESTS OFF)
add_subdirectory(external/glbinding-2.1.4)
include_directories(SYSTEM external/glbinding-2.1.4/source/glbinding/include)

#AntTweakBar
add_subdirectory(${GLOBAL_EXT_DIR}/AntTweakBar)

# get lodepng components
set(LODEPNG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/lodepng-20161127")
file(GLOB LODEPNG_SOURCES "${LODEPNG_DIR}/*.cpp")
set(LODEPNG_INCLUCE_DIR ${LODEPNG_DIR})
# suppress al warnings for sources
if(NOT MSVC)
  set_source_files_properties(${LODEPNG_SOURCES} PROPERTIES COMPILE_FLAGS -w)
else()
  set_source_files_properties(${LODEPNG_SOURCES} PROPERTIES COMPILE_FLAGS /w)
endif()


# include glm
include_directories(SYSTEM ${GLOBAL_EXT_DIR}/glm-0.9.8.4)

# create framework helper library 
file(GLOB FRAMEWORK_SOURCES framework/source/*.cpp)
add_library(framework STATIC ${FRAMEWORK_SOURCES} ${LODEPNG_SOURCES})
target_include_directories(framework PUBLIC framework/include)
target_include_directories(framework SYSTEM PUBLIC ${LODEPNG_DIR})
target_link_libraries(framework glbinding glfw ${GLFW_LIBRARIES} AntTweakBar)

# activate C++ 11
if(NOT MSVC)
    add_definitions(-std=c++11)
    # all warnings, implicit lossy casts
    add_definitions(-Wall -Wconversion -Wextra -Wpedantic)
    # warn when members are unitialized
    add_definitions(-Weffc++)
    # force linking  with c++11 lib
    if(APPLE)
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
        add_definitions(-stdlib=libc++)
    endif()
else()
    # build in parallel, show warnings and suppress one caused by glbinding
    add_definitions(/MP /W3 /wd4251)
endif()

# generate executable targets for applications
include(GenerateExecutables)
generate_executables("./applications/source" LIBRARIES framework INCLUDES "./applications/include")


mark_as_advanced(GLFW_BUILD_DOCS GLFW_BUILD_TESTS GLFW_INSTALL GLFW_BUILD_EXAMPLES
 GLFW_DOCUMENT_INTERNALS GLFW_USE_EGL GLFW_USE_MIR GLFW_USE_WAYLAND GLFW_LIBRARIES
 LIB_SUFFIX BUILD_SHARED_LIBS)

mark_as_advanced(OPTION_BUILD_TESTS OPTION_BUILD_TOOLS OPTION_BUILD_GPU_TESTS
 OPTION_BUILD_WITH_BOOST_THREAD OPTION_SELF_CONTAINED OPTION_BUILD_DOCS
 OPTION_BUILD_EXAMPLES PYTHON)