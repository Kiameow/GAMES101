cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/include)

add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)

target_include_directories(Rasterizer PRIVATE ${OpenCV_INCLUDE_DIRS})


# 链接OpenCV库

target_link_libraries(Rasterizer ${OpenCV_LIBS})
