

cmake_minimum_required(VERSION 2.8)

project(yaml_test)

add_definitions(-std=c++11)

set(SRCS yaml_test.cpp)
add_executable(yamltest ${SRCS})

target_link_libraries(yamltest ${CMAKE_HOME_DIRECTORY}/lib/libyaml-cpp.so)
