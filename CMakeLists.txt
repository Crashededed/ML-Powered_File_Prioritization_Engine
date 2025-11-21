cmake_minimum_required(VERSION 3.10)

# Project Name
project(ADDPP_Payload)

# Use C++ 17 (Standard for modern file system access)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- CRITICAL: Static Linking Configuration ---
# This replaces the "Runtime Library -> /MT" setting you did in the VS IDE.
# It ensures your .exe runs on the VM without missing DLL errors.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    add_compile_options(/W4) # Enable warnings to help catch bugs
endif()

# Create the executable from main.cpp
add_executable(Payload main.cpp)