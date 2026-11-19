cmake_minimum_required(VERSION 3.17)
project(PushOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(pushos ${SRCS})
target_include_directories(pushos PRIVATE "${CMAKE_SOURCE_DIR}/include/")