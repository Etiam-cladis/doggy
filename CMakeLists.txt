cmake_minimum_required(VERSION 3.18)

project(doggy_t1)


set(CXX_FLAGS
  -g

 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native

 -std=c++17
 -rdynamic
 -pthread
 )



 set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(doggy/net)
