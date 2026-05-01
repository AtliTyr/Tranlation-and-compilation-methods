cmake_minimum_required(VERSION 3.15)

project(syntaxAnalyzer)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(syntaxAnal syntaxAnalyzer.cpp)