cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(filecopy src/filecopy.cpp)

add_subdirectory(lib/ringbuffer)

list(APPEND EXTRA_LIBS ringbuffer)

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

