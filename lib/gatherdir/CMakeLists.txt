cmake_minimum_required(VERSION 3.10)

add_library(gatherdir src/gatherdir.cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

target_include_directories(gatherdir
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

