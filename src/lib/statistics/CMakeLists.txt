add_library(statistics SHARED basic_statistics.cpp)
target_link_libraries(statistics PUBLIC config)
