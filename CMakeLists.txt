cmake_minimum_required(VERSION 3.10)

project(lkeycpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

include_directories(include)

add_executable(lkeycpp src/main.cpp src/Lexer.cpp src/Parser.cpp src/Interpreter.cpp)
