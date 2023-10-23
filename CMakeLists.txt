cmake_minimum_required(VERSION 3.20)  # Використовуємо CMake версії 3.20 або вище, щоб підтримувати C++20

project(MyScalarProductProject)

# Вказуємо компілятору використовувати стандарт C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MyScalarProductProject main.cpp)
