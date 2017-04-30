cmake_minimum_required(VERSION 2.6)
project(BONSAIS)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

add_executable(bonsais bonsais.cpp BonsaiDCW.cpp BonsaiPR.cpp Basics.hpp FitVector.hpp)
