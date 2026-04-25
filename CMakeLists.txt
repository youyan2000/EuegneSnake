cmake_minimum_required(VERSION 3.10)
project(EuegneSnake VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/inc)
add_executable(EuegneSnake
    src/main.cpp
    src/SnakeGame.cpp
)

# 链接库不需要额外依赖，使用标准终端输出
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(EuegneSnake PRIVATE Threads::Threads)
endif()