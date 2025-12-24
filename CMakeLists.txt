cmake_minimum_required(VERSION 3.15)

project(lkeycpp VERSION 1.0 LANGUAGES CXX DESCRIPTION "Lowkey Programming Language Interpreter")

# Better defaults
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Include directories
include_directories(include)

# Main executable
add_executable(lkeycpp 
    src/main.cpp 
    src/Lexer.cpp 
    src/Parser.cpp 
    src/Interpreter.cpp
)

target_compile_options(lkeycpp PRIVATE ${CMAKE_CXX_FLAGS})

# Test executable
add_executable(run_tests 
    tests/test_runner.cpp 
    src/Lexer.cpp 
    src/Parser.cpp 
    src/Interpreter.cpp
)

target_compile_options(run_tests PRIVATE ${CMAKE_CXX_FLAGS})

# Testing
enable_testing()
add_test(NAME comprehensive_test COMMAND run_tests)

# Package configuration
include(GNUInstallDirs)
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})

# Installation
install(TARGETS lkeycpp RUNTIME DESTINATION ${INSTALL_BINDIR})