cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(ringbuffer src/ringbuffer.cpp)

target_include_directories(ringbuffer
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(ringbuffer
    PRIVATE
        RING_BUFFER_CHUNKSIZE=1024*1024*4
        $<$<CONFIG:Debug>:DEBUG_BUILD;_DEBUG>
        $<$<CONFIG:Release>:>
    
    PUBLIC
        RING_BUFFER_CHUNKSIZE=1024*1024*4
        $<$<CONFIG:Debug>:DEBUG_BUILD;_DEBUG>
        $<$<CONFIG:Release>:>

    INTERFACE
        RING_BUFFER_CHUNKSIZE=1024*1024*4
        $<$<CONFIG:Debug>:DEBUG_BUILD;_DEBUG>
        $<$<CONFIG:Release>:>
)