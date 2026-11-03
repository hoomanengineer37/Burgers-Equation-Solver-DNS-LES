# Burgers' equation DNS/LES solver
#
# Build (Release):
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build
#   ./build/burgers --config cases/dns.cfg
#
# Or with presets (see CMakePresets.json):
#   cmake --preset release && cmake --build --preset release

cmake_minimum_required(VERSION 3.16)

project(BurgersSolver
    VERSION 2.0.0
    DESCRIPTION "1D Burgers equation solver: DNS and Smagorinsky LES"
    LANGUAGES CXX)

# ---------------------------------------------------------------- options

option(BURGERS_BUILD_TESTS          "Build the unit tests (tests/)"                ON)
option(BURGERS_WARNINGS_AS_ERRORS   "Treat compiler warnings as errors"            OFF)
option(BURGERS_ENABLE_SANITIZERS    "Build with AddressSanitizer + UBSan (GCC/Clang)" OFF)

# Default to an optimized build when the user does not choose one
# (single-config generators such as Makefiles and Ninja only).
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# Standard C++17 everywhere (-std=c++17, not the GNU dialect)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile_commands.json for editors and clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------- shared settings

# Interface target carrying warning and sanitizer flags, so every target
# (library, app, tests) is compiled the same way.
add_library(burgers_project_options INTERFACE)
target_compile_features(burgers_project_options INTERFACE cxx_std_17)

if(MSVC)
    target_compile_options(burgers_project_options INTERFACE /W4 /permissive-)
    if(BURGERS_WARNINGS_AS_ERRORS)
        target_compile_options(burgers_project_options INTERFACE /WX)
    endif()
else()
    target_compile_options(burgers_project_options INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
    if(BURGERS_WARNINGS_AS_ERRORS)
        target_compile_options(burgers_project_options INTERFACE -Werror)
    endif()
endif()

if(BURGERS_ENABLE_SANITIZERS)
    if(MSVC)
        message(WARNING "BURGERS_ENABLE_SANITIZERS is only supported with GCC/Clang; ignored")
    else()
        target_compile_options(burgers_project_options INTERFACE
            -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
        target_link_options(burgers_project_options INTERFACE -fsanitize=address,undefined)
    endif()
endif()

# ---------------------------------------------------------------- library

# The physics code, shared by the application and the tests.
add_library(burgers_core
    src/BurgersSolver.cpp
    src/Config.cpp
    src/Filter.cpp
    src/Grid.cpp
    src/IO.cpp
    src/SGSModel.cpp)
add_library(burgers::core ALIAS burgers_core)

target_include_directories(burgers_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(burgers_core PUBLIC burgers_project_options)

# std::filesystem needs an extra library on GCC 8
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    target_link_libraries(burgers_core PUBLIC stdc++fs)
endif()

# ---------------------------------------------------------------- application

add_executable(burgers apps/main.cpp)
target_link_libraries(burgers PRIVATE burgers::core)

# ---------------------------------------------------------------- tests

if(BURGERS_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------- install

include(GNUInstallDirs)
install(TARGETS burgers RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY cases/ DESTINATION ${CMAKE_INSTALL_DATADIR}/burgers/cases)
