cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

link_directories(gem5)

if(${CUSTOM_HOOK} OR ${STATIC_BINARY})
    message("Cmake for M5 library Building custom hook for m5 calls with path ${M5_PATH}")

    # https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#target-properties
    # https://cmake.org/cmake/help/v3.18/command/add_compile_options.html#command:add_compile_options
    # add_compile_options(-O2 -DM5OP_ADDR=0xFFFF0000 -Wno-implicit-function-declaration -no-pie -DM5OP_PIC -I./gem5)
    add_library(hooks_prospar OBJECT hooks_prospar.c)
    target_compile_options(hooks_prospar PRIVATE
        -Wno-implicit-function-declaration
        -fPIC
    )
    target_include_directories(hooks_prospar PRIVATE ${M5_PATH})

    # https://cmake.org/cmake/help/v3.12/manual/cmake-buildsystem.7.html#object-libraries
    add_library(m5op_x86 OBJECT m5op_x86.S)
    target_include_directories(m5op_x86 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(m5op_x86 PUBLIC
        -O2
        -DM5OP_ADDR=0xFFFF0000
        -DM5OP_PIC
        -fPIC
    )

    # target_include_directories(m5op_x86 PRIVATE M5_PATH)
    add_library(m5_mmap OBJECT m5_mmap.c)
    target_compile_options(m5_mmap PRIVATE
        -fPIC
        -DM5OP_ADDR=0xFFFF0000
    )
    target_include_directories(m5_mmap PRIVATE ${M5_PATH})

    # https://cmake.org/cmake/help/v3.5/manual/cmake-buildsystem.7.html#object-libraries
    add_library(libhooks_prospar STATIC $<TARGET_OBJECTS:hooks_prospar> $<TARGET_OBJECTS:m5op_x86> $<TARGET_OBJECTS:m5_mmap>)
endif()