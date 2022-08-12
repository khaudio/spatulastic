cmake_minimum_required(VERSION 3.10)

add_library(filecopy src/filecopy.cpp)

# C++ Standard
set(CMAKE_CXX_STANDARAD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(lib/ringbuffer)

list(APPEND EXTRA_LIBS ringbuffer)

target_compile_options(filecopy
    PRIVATE
        -std=c++17
        "$<$<CONFIG:Debug>:/DO0;/DWall;/DWerror;/D_DEBUG>"
        "$<$<CONFIG:Release>:>"
    PUBLIC
        -std=c++17
        "$<$<CONFIG:Debug>:/DO0;/DWall;/DWerror;/D_DEBUG>"
        "$<$<CONFIG:Release>:>"
)

target_include_directories(filecopy
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/include>
)

target_link_libraries(filecopy
    PRIVATE
        ${EXTRA_LIBS}
    PUBLIC
        ${EXTRA_LIBS}
)

