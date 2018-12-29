# Required for target_sources
cmake_minimum_required(VERSION 3.1)

# Block in-source builds to prevent the directory from getting cluttered up
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# We're targeting our own operating system that CMake doesn't know about.
# Use a non-generic name would cause CMake to inject various flags which
# usually breaks builds
set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)

set(ARCH "x86_64" CACHE STRING "Architecture to build for")
message(STATUS "Architecture: " ${ARCH})

find_program(CMAKE_C_COMPILER ${ARCH}-quark-gcc)
if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "Compiler not found! Run the \
                         toolchain script in Pepper to build a toolchain. If \
                         you have already built it, make sure that you have run \
                         envsetup.sh before configuring CMake.")
endif()

message(STATUS "C Compiler: " ${CMAKE_C_COMPILER})

# Set SYSROOT so we know where to install the binaries
if(NOT DEFINED SYSROOT)
    set(SYSROOT "${CMAKE_SOURCE_DIR}/../sysroot")
endif()
message(STATUS "Sysroot: " ${SYSROOT})

project(Quark C)

# C11!
set(CMAKE_C_STANDARD 11)

# -fdiagnostics-color=always is needed because GCC thinks that we're piping
# it's output somewhere and tries to remove color by default
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fdiagnostics-color=always")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O2 -DQUARK_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DQUARK_RELEASE")
message(STATUS "CFLAGS: " ${CMAKE_C_FLAGS})

add_subdirectory(core)
add_subdirectory(extra)
