# Required for target_sources
cmake_minimum_required(VERSION 3.1)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)

set(ARCH "x86_64" CACHE STRING "Architecture to build for")
message(STATUS "Architecture: " ${ARCH})

set(SRC_ARCH ${ARCH})
if (SRC_ARCH STREQUAL "x86_64" OR SRC_ARCH STREQUAL "i686")
    set(SRC_ARCH "x86")
endif()
message(STATUS "Source architecture: " ${SRC_ARCH})

set(KERNEL_PATH "${CMAKE_SOURCE_DIR}/../quark")
if(NOT EXISTS ${KERNEL_PATH})
    message(FATAL_ERROR "Unable to locate kernel directory.")
endif()
message(STATUS "Kernel folder: " ${KERNEL_PATH})

set(ARCH_PATH "${KERNEL_PATH}/arch/${SRC_ARCH}")
if(NOT EXISTS ${ARCH_PATH})
    message(FATAL_ERROR "Unable to locate architecture directory. Perhaps your ARCH is wrong?")
endif()
message(STATUS "Architecture folder: " ${ARCH_PATH})

set(ARCH_INCLUDE_PATH "${KERNEL_PATH}/arch/include/${SRC_ARCH}")
if(NOT EXISTS ${ARCH_INCLUDE_PATH})
    message(FATAL_ERROR "Unable to locate architecture include directory. Perhaps your ARCH is wrong?")
endif()
message(STATUS "Architecture include folder: " ${ARCH_INCLUDE_PATH})

if(NOT DEFINED ${PLATFORM_FILE})
    set(PLATFORM_FILE ${ARCH_PATH}/module.cmake)
endif()
if(NOT EXISTS ${PLATFORM_FILE})
    message(FATAL_ERROR "Platform file does not exist: " ${PLATFORM_FILE})
endif()
message(STATUS "Platform file: " ${PLATFORM_FILE})

if(NOT DEFINED SYSROOT)
    set(SYSROOT "${CMAKE_SOURCE_DIR}/../hdd")
endif()
message(STATUS "Sysroot: " ${SYSROOT})

find_program(CXX ${ARCH}-quark-g++)
if(NOT CXX)
    message(FATAL_ERROR "Compiler not found! Run the \
                         toolchain script in Pepper to build a toolchain. If \
                         you have already built it, make sure that you have run \
                         envsetup.sh before configuring CMake.")
endif()

set(CMAKE_CXX_COMPILER ${CXX})
message(STATUS "CXX Compiler: " ${CXX})

project(Quark CXX)

include(${PLATFORM_FILE})

enable_language(ASM_NASM)

set(CMAKE_CXX_STANDARD 17)

# -mcmodel=large is necessary to disable GOT/PLT
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-field-initializers -ffreestanding -fno-builtin -fno-rtti -fno-exceptions -fno-stack-protector -fdiagnostics-color=always -mgeneral-regs-only")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -DQUARK_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DQUARK_RELEASE")
message(STATUS "CXXFLAGS: " ${CMAKE_CXX_FLAGS})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
message(STATUS "LDFLAGS: " ${CMAKE_EXE_LINKER_FLAGS})

set(INCLUDE_DIRS "${INCLUDE_DIRS} ${KERNEL_PATH}/include/")
include_directories(${INCLUDE_DIRS})
include_directories("${KERNEL_PATH}/include/libstdc++")
message(STATUS "Include directories: " ${INCLUDE_DIRS})

add_executable(test.ko test.cc)

add_subdirectory(ahci)

install(TARGETS test.ko DESTINATION ${SYSROOT}/lib/modules)
