cmake_minimum_required(VERSION 3.28)
project(cch)

set(CMAKE_CXX_STANDARD 23)

add_library(cch SHARED
        include/compression/RLECompression.h
        src/compression/RLECompression.cpp
        include/compression/CompressionInfo.h
        include/utilities/Utilities.h
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)