cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

set(SOURCE_FILES urcu.h new_urcu.c citrus.c test.c)

set_source_files_properties(citrus.c PROPERTIES COMPILE_OPTIONS "-Wno-pointer-sign;-Wno-incompatible-pointer-types;-Wno-implicit-function-declaration")
set_source_files_properties(new_urcu.c PROPERTIES COMPILE_OPTIONS "-Wno-pointer-sign;-Wno-incompatible-pointer-types;-Wno-implicit-function-declaration")
set_source_files_properties(test.c PROPERTIES COMPILE_OPTIONS "-Wno-pointer-sign;-Wno-incompatible-pointer-types;-Wno-implicit-function-declaration")

add_executable(synchrobench-mutex-skiplist ${SOURCE_FILES})

target_include_directories(synchrobench-mutex-skiplist PRIVATE
../../../src/atomic_ops
../../../include)

# set_property(TARGET synchrobench-mutex-skiplist PROPERTY C_STANDARD 99)
# string(REPLACE "-std=c11" "-std=gnu89" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
# string(REPLACE "-std=gnu11" "-std=gnu89" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(synchrobench-mutex-skiplist PRIVATE Threads::Threads)
