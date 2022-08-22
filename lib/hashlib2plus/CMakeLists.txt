cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(hashlib2plus
    trunk/src/hl_md5.cpp
    trunk/src/hl_md5wrapper.cpp
    trunk/src/hl_sha1.cpp
    trunk/src/hl_sha1wrapper.cpp
    trunk/src/hl_sha256.cpp
    trunk/src/hl_sha256wrapper.cpp
    trunk/src/hl_sha2ext.cpp
    trunk/src/hl_sha384wrapper.cpp
    trunk/src/hl_sha512wrapper.cpp
    trunk/src/hl_wrapperfactory.cpp
)

target_include_directories(hashlib2plus
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/trunk/src

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/trunk/src
    
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/trunk/src
)
