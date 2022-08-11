cmake_minimum_required(VERSION 3.10)

# Project name and version
project(spatulastic VERSION 0.1)

# C++ Standard
set(CMAKE_CXX_STANDARAD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(cmakeconfig.h.in cmakeconfig.h)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

list(APPEND EXTRA_INCLUDES "include")
list(APPEND EXTRA_LIBS)

# Executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC ${EXTRA_LIBS})

target_include_directories(
        ${PROJECT_NAME} PUBLIC
        "${PROJECT_BINARY_DIR}"
        ${EXTRA_INCLUDES}
    )

