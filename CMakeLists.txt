cmake_minimum_required(VERSION 3.10)

# Project name and version
project(spatulastic VERSION 0.1 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARAD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(include/cmakeconfig.h.in include/cmakeconfig.h)

# Executable
add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/filecopy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/progressbar)

list(APPEND EXTRA_LIBS filecopy progressbar)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -std=c++17
        "$<$<CONFIG:Debug>:/DO0;/DWall;/DWerror;/D_DEBUG>"
        "$<$<CONFIG:Release>:>"
    PUBLIC
        -std=c++17
        "$<$<CONFIG:Debug>:/DO0;/DWall;/DWerror;/D_DEBUG>"
        "$<$<CONFIG:Release>:>"
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/include>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${EXTRA_LIBS}
    PUBLIC
        ${EXTRA_LIBS}
)

