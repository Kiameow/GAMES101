cmake_minimum_required (VERSION 3.30.2)
project (Transformation)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

add_executable (Transformation main.cpp)
