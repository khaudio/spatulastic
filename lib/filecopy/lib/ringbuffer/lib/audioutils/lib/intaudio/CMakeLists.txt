cmake_minimum_required(VERSION 3.10)

add_library(intaudio src/intaudio.cpp)

# C++ Standard
set(CMAKE_CXX_STANDARAD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

target_compile_options(intaudio
    PRIVATE
        -std=c++17
    PUBLIC
        "$<$<CONFIG:Debug>:/DO0;/DWall;/DWerror;/D_DEBUG>"
        "$<$<CONFIG:Release>:>"
)

target_include_directories(intaudio
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
