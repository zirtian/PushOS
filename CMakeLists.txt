cmake_minimum_required(VERSION 3.17)
project(PushOS LANGUAGES CXX)

file(GLOB SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(pushos ${SRCS})
target_include_directories(pushos PRIVATE "${CMAKE_SOURCE_DIR}/include/")