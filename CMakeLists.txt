cmake_minimum_required(VERSION 3.14) 
project(Payload)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- CRITICAL FIX: FORCING STATIC LINKING (/MT) ---

# 1. Apply the standard static runtime library setting.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

# 2. AGGRESSIVELY OVERRIDE: Add the /MT flag directly to all compile options.
# This ensures the compiler links the runtime statically, eliminating MSVCP140D.dll, etc.
add_compile_options("/MT") 

# --- END CRITICAL FIX ---

# BUILD EXECUTABLE
add_executable(Payload src/main.cpp)