cmake_minimum_required(VERSION 3.10)

add_library(audioutils src/audioutils.cpp)

# C++ Standard
set(CMAKE_CXX_STANDARAD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(lib/intaudio)

list(APPEND EXTRA_LIBS intaudio)

target_compile_options(audioutils
    PRIVATE
        -std=c++17
        "$<$<CONFIG:Debug>:/DO0;/DWall;/DWerror;/D_DEBUG>"
        "$<$<CONFIG:Release>:>"
    PUBLIC
        -std=c++17
        "$<$<CONFIG:Debug>:/DO0;/DWall;/DWerror;/D_DEBUG>"
        "$<$<CONFIG:Release>:>"
)

target_include_directories(audioutils
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/include>
)

target_link_libraries(audioutils
    PRIVATE
        ${EXTRA_LIBS}
    PUBLIC
        ${EXTRA_LIBS}
)

