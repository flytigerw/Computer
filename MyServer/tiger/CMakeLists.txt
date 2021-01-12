cmake_minimum_required(VERSION 2.8)

project(tiger)


enable_language(ASM)

include(include/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined ")

include_directories(include/)
include_directories(/usr/local/include)

link_directories(${CMAKE_HOME_DIRECTORY}/lib)

#汇编
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(SRCS
    src/config.cc
    src/log.cc
    src/util.cc
    src/thread.cc
    src/MyGo/context/jump_x86_64_sysv_elf_gas.S 
    src/MyGo/context/make_x86_64_sysv_elf_gas.S
    src/MyGo/context/context.cpp
    )

set(
  LIBS 
  tiger
  yaml-cpp
  pthread
    )
add_library(tiger SHARED ${SRCS})

#add_executable(test_yaml tests/test_yaml.cc)
#add_dependencies(test_yaml tiger)
#force_redefine_file_macro_for_sources(test_yaml) 
#target_link_libraries(test_yaml ${LIBS})

#add_executable(test_logger tests/test_logger.cc)
#add_dependencies(test_logger tiger)
#force_redefine_file_macro_for_sources(test_logger) 
#target_link_libraries(test_logger ${LIBS})

#add_executable(test_thread tests/test_thread.cc)
#add_dependencies(test_thread tiger)
#force_redefine_file_macro_for_sources(test_thread) 
#target_link_libraries(test_thread ${LIBS})

add_executable(test_context tests/test_context.cc)
add_dependencies(test_context tiger)
force_redefine_file_macro_for_sources(test_context) 
target_link_libraries(test_context ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
