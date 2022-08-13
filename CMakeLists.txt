cmake_minimum_required(VERSION 3.10)

project(spatulastic VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug Release)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -D_DEBUG -O0 -Wall -DRING_BUFFER_CHUNKSIZE=1024*1024 -lpthread"
)

configure_file(include/cmakeconfig.h.in include/cmakeconfig.h)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/filecopy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/progressbar)

list(APPEND EXTRA_LIBS filecopy progressbar)

add_executable(${PROJECT_NAME} src/main.cpp)

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
        ${EXTRA_LIBS}

    PUBLIC
        ${EXTRA_LIBS}
    
    INTERFACE
        ${EXTRA_LIBS}
)

