cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

if(${SE_MODE})
    link_directories(${GEM5_PATH}/util/m5/build/x86/out)
endif()
include_directories(${M5_PATH})

set_source_files_properties(string_match_pthreads.c PROPERTIES COMPILE_OPTIONS "-Wno-unused-variable;-fgnu89-inline")
set_source_files_properties(string_match_pthreads_manual.c PROPERTIES COMPILE_OPTIONS "-Wno-unused-variable;-fgnu89-inline")

add_executable(huron-string-match string_match_pthreads.c)
add_executable(huron-string-match-manual string_match_pthreads_manual.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(${CUSTOM_HOOK})
    target_link_libraries(huron-string-match PRIVATE Threads::Threads PRIVATE libhooks_prospar)
    target_link_libraries(huron-string-match-manual PRIVATE Threads::Threads PRIVATE libhooks_prospar)
elseif(${STATIC_BINARY})
    target_link_libraries(huron-string-match PRIVATE Threads::Threads PRIVATE libhooks_prospar -static)
    target_link_libraries(huron-string-match-manual PRIVATE Threads::Threads PRIVATE libhooks_prospar -static)
elseif(${SE_MODE})
    target_link_libraries(huron-string-match PRIVATE Threads::Threads PRIVATE m5 -static)
    target_link_libraries(huron-string-match-manual PRIVATE Threads::Threads PRIVATE m5 -static)
else()
    target_link_libraries(huron-string-match PRIVATE Threads::Threads)
    target_link_libraries(huron-string-match-manual PRIVATE Threads::Threads)
endif()