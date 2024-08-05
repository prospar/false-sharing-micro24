cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# Add ASM compilation issue https://gitlab.kitware.com/cmake/cmake/-/issues/21480
project(false-sharing-benchmarks LANGUAGES C CXX ASM)

if(${SE_MODE})
    set(GEM5_PATH "/data/vipinpat/GEM5-FS/gem5-false-sharing" CACHE PATH "path to gem5 directory source to include m5")
    add_definitions(-DSE_MODE_BUILD)
endif()

# # changing value of option from command line:
# a) https://stackoverflow.com/questions/12896988/passing-the-argument-to-cmake-via-command-prompt
# b) https://stackoverflow.com/questions/5998186/adding-command-line-options-to-cmake
# # cmake option:https://cmake.org/cmake/help/latest/command/option.html
option(CUSTOM_HOOK "enable m5 library for stats call")
option(STATIC_BINARY "build static binary")

# https://cmake.org/cmake/help/latest/command/if.html
if(${CUSTOM_HOOK} OR ${STATIC_BINARY})
    set(M5_PATH "${CMAKE_CURRENT_SOURCE_DIR}/m5_library" CACHE PATH "path to m5 library")
    add_definitions(-DROI_TRACING) # https://cmake.org/cmake/help/v3.18/command/add_definitions.html
endif()

# gem5: using m5 in SE mode: <https://www.gem5.org/documentation/general_docs/m5ops/>
# using in FS mode: above link look for heading: "Using the “_addr” version of M5ops

# LDFLAGS  stored as CMAKE_EXE_LINKER_FLAGS_INIT, CMAKE_SHARED_LINKER_FLAGS_INIT, and CMAKE_MODULE_LINKER_FLAGS_INIT in cache file
set(CMAKE_C_STANDARD 11)

# -I${M5_PATH}/gem5 was required for the SE mode
if(${SE_MODE})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${GEM5_PATH}/include -std=c11 -Wall -Werror -g ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${GEM5_PATH}/include -std=c++11 -Wall -Werror -g ")
    add_definitions(-DSE_MODE_BUILD)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Werror -g ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -g ")
endif()

set(CMAKE_CXX_STANDARD 11)

# set LDFLAGS
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Disable entering/exiting messages
set(CMAKE_RULE_MESSAGES OFF)

set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR})

if(${CUSTOM_HOOK} OR ${STATIC_BINARY})
    add_subdirectory(m5_library)
endif()

add_subdirectory(benchmarks)
add_subdirectory(microbenchmarks)
