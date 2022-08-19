cmake_minimum_required(VERSION 3.10)

project(spatulastic VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CXX_EXTENSIONS ON)
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

configure_file(include/cmakeconfig.h.in include/cmakeconfig.h)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/filecopy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/progressbar)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/hashlib2plus/trunk)

list(APPEND LIBRARIES filecopy progressbar hashlib2plus)

add_executable(${PROJECT_NAME} src/main.cpp)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -lpthread
        $<$<CONFIG:Debug>:-O0 -g -Wall>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g>
    
    PUBLIC
        -lpthread
        $<$<CONFIG:Debug>:-O0 -g -Wall>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g>

    INTERFACE
        -lpthread
        $<$<CONFIG:Debug>:-O0 -g -Wall>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g>
)

target_compile_definitions(${PROJECT_NAME}
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

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${LIBRARIES}

    PUBLIC
        ${LIBRARIES}
    
    INTERFACE
        ${LIBRARIES}
)

