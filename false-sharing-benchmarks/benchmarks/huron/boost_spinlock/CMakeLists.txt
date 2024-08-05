cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

if(${SE_MODE})
    link_directories(${GEM5_PATH}/util/m5/build/x86/out)
endif()

include_directories(${M5_PATH})

add_executable(huron-boost-spinlock spinlock_pool.cpp)
add_executable(huron-boost-spinlock-manual spinlock_pool_manual.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# target_link_libraries(huron-boost-spinlock PRIVATE Threads::Threads PRIVATE m5)
# target_link_libraries(huron-boost-spinlock-manual PRIVATE Threads::Threads PRIVATE m5)
if(${CUSTOM_HOOK})
    target_link_libraries(huron-boost-spinlock PRIVATE Threads::Threads PRIVATE libhooks_prospar)
    target_link_libraries(huron-boost-spinlock-manual PRIVATE Threads::Threads PRIVATE libhooks_prospar)
elseif(${STATIC_BINARY})
    target_link_libraries(huron-boost-spinlock PRIVATE Threads::Threads PRIVATE libhooks_prospar -static)
    target_link_libraries(huron-boost-spinlock-manual PRIVATE Threads::Threads PRIVATE libhooks_prospar -static)
elseif(${SE_MODE})
    target_link_libraries(huron-boost-spinlock PRIVATE Threads::Threads PRIVATE m5 -static)
    target_link_libraries(huron-boost-spinlock-manual PRIVATE Threads::Threads PRIVATE m5 -static)
else()
    target_link_libraries(huron-boost-spinlock PRIVATE Threads::Threads)
    target_link_libraries(huron-boost-spinlock-manual PRIVATE Threads::Threads)
endif()