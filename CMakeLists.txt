cmake_minimum_required(VERSION 3.10)

# Project Name
project(ADDPP_Payload)

# Use C++ 17 (Standard for modern file system access)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- CRITICAL: Static Linking Configuration ---
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    add_compile_options(/W4)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Create the executable from main.cpp in src/
add_executable(Payload src/main.cpp)

# Copy data files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/data/Text.txt 
     DESTINATION ${CMAKE_BINARY_DIR})