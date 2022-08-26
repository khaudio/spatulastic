cmake_minimum_required(VERSION 3.10)

add_library(timer src/timer.cpp)

set(CMAKE_CXX_STANDARAD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

target_include_directories(timer
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
