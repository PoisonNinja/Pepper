# Required for target_sources
cmake_minimum_required(VERSION 3.1)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)

find_program(CLANG clang)
if(NOT CLANG)
    message(FATAL_ERROR "clang not found! Run the \
                         toolchain script in Pepper to build a toolchain. If \
                         you have already built it, make sure that you have run \
                         envsetup.sh before configuring CMake.")
endif()

find_program(LD.LLD ld.lld)
if(NOT LD.LLD)
    message(FATAL_ERROR "ld.lld not found! Run the \
                         toolchain script in Pepper to build a toolchain. If \
                         you have already built it, make sure that you have run \
                         envsetup.sh before configuring CMake.")
endif()

# Skip compiler checks since we're targeting ELF binaries, and thus those
# would fail on MacOS with the error `file was built for unsupported file format`,
# which makes sense since MacOS uses MachO. Unfortunately, the way that CMake
# does the compiler checks means that it performs the compiler checks without
# letting us inject flags to force Clang to generate ELF binaries
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_C_COMPILER ${CLANG})
set(CMAKE_C_LINKER ${LD.LLD})

message(STATUS "C Compiler: " ${CLANG})
message(STATUS "C Linker: " ${LD.LLD})

enable_language(ASM_NASM)

if(NOT DEFINED SYSROOT)
    set(SYSROOT "${CMAKE_SOURCE_DIR}/../hdd")
endif()
message(STATUS "Sysroot: " ${SYSROOT})

project(Quark C)

# Rudimentary compiler checks
if (${CMAKE_C_COMPILER_ID} STREQUAL "AppleClang")
    # AppleClang is screwed up, it can only produce MachO binaries
    message(FATAL_ERROR "The active compiler is AppleClang. Unfortunately, \
                         AppleClang can't produce ELF binaries. You need to \
                         build LLVM using the \
                         toolchain script provided in Pepper. If you have \
                         already done so, make sure you executed envsetup.sh. \
                         This only needs to be done once during configuration.")
endif()

if (${CMAKE_C_COMPILER_VERSION} VERSION_LESS 5.0)
    message(FATAL_ERROR "Your version of Clang is too old! Rerun the toolchain \
                         script to build a newer version.")
endif()

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=-Wall -Wextra -fcolor-diagnostics")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -DQUARK_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DQUARK_RELEASE")
message(STATUS "CFLAGS: " ${CMAKE_C_FLAGS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "LDFLAGS: " ${CMAKE_EXE_LINKER_FLAGS})

set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINKER} <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

add_subdirectory(core)
