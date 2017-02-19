cmake_minimum_required(VERSION 2.8)
project(bonsais)

set(CMAKE_CXX_FLAGS "-Wall -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

# SDSL
link_directories(~/lib)
include_directories(~/include)

add_executable(bonsais bonsais.cpp Bonsai.cpp BonsaiPlus.cpp Basics.hpp)
target_link_libraries(bonsais sdsl)
