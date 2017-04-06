cmake_minimum_required(VERSION 2.8)
project(bonsais)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O9")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

add_executable(bonsais bonsais.cpp BonsaiDCW.cpp BonsaiPR.cpp Basics.hpp FitVector.hpp)
