include_directories(${M5_PATH})
add_subdirectory(boost_spinlock)
add_subdirectory(linear_regression)
add_subdirectory(locked)
add_subdirectory(lockless)
add_subdirectory(ref_count)
add_subdirectory(string_match)

# add_subdirectory(tensor) # requires Eigen library
# add_subdirectory(tmi_boost) # repeated benchmark
