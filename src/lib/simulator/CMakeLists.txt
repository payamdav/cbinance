add_library(simulator SHARED simulator.cpp)
target_link_libraries(simulator PUBLIC trade)

