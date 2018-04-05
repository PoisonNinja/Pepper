# Required for target_sources
cmake_minimum_required(VERSION 3.1)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)

find_program(C x86_64-pepper-gcc)
if(NOT C)
    message(FATAL_ERROR "Compiler not found! Run the \
                         toolchain script in Pepper to build a toolchain. If \
                         you have already built it, make sure that you have run \
                         envsetup.sh before configuring CMake.")
endif()

# Skip compiler checks since we're targeting ELF binaries, and thus those
# would fail on MacOS with the error `file was built for unsupported file format`,
# which makes sense since MacOS uses MachO. Unfortunately, the way that CMake
# does the compiler checks means that it performs the compiler checks without
# letting us inject flags to force Clang to generate ELF binaries
set(CMAKE_C_COMPILER ${C})

message(STATUS "C Compiler: " ${CLANG})

enable_language(ASM_NASM)

if(NOT DEFINED SYSROOT)
    set(SYSROOT "${CMAKE_SOURCE_DIR}/../hdd")
endif()
message(STATUS "Sysroot: " ${SYSROOT})

project(Quark C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fdiagnostics-color=always")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -DQUARK_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DQUARK_RELEASE")
message(STATUS "CFLAGS: " ${CMAKE_C_FLAGS})

add_subdirectory(core)
add_subdirectory(extra)
