cmake_minimum_required (VERSION 3.20)
project(sonda VERSION 1.0 LANGUAGES CXX)

add_subdirectory(src)
enable_testing()
add_subdirectory(test)
