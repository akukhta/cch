cmake_minimum_required(VERSION 3.28)
project(cch)

set(CMAKE_CXX_STANDARD 23)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(cch SHARED
        include/compression/RLECompression.h
        src/compression/RLECompression.cpp
        include/utilities/Utilities.h
        include/compression/LZWCompression.h
        src/compression/LZWCompression.cpp
        include/utilities/fbitstream.cpp
        include/utilities/fbitstream.h
        include/utilities/bitstream.h
        src/compression/HuffmanCompression.cpp
        include/compression/HuffmanCompression.h
        include/hash/djb2hash.h
        include/compression/ArithmeticCompression.h
        src/compression/ArithmeticCompression.cpp
        include/compression/LZSS.h
        src/compression/LZSS.cpp
        include/hash/MD5.h
        src/hash/MD5.cpp
        include/config/types.h
        include/hash/SHA256.h
        src/hash/SHA256.cpp
        include/hash/SHA512.h
        src/hash/SHA512.cpp
)


target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)