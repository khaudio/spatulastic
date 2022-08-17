cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(hashlib2plus
    src/hl_md5.cpp
    src/hl_md5wrapper.cpp
    src/hl_sha1.cpp
    src/hl_sha1wrapper.cpp
    src/hl_sha256.cpp
    src/hl_sha256wrapper.cpp
    src/hl_sha2ext.cpp
    src/hl_sha384wrapper.cpp
    src/hl_sha512wrapper.cpp
    src/hl_wrapperfactory.cpp
)

target_include_directories(hashlib2plus
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/src>
)

target_link_libraries(hashlib2plus
    PRIVATE
        ${EXTRA_LIBS}
    PUBLIC
        ${EXTRA_LIBS}
)

