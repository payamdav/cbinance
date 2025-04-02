add_library(obl SHARED snapshot.cpp update.cpp ob.cpp)

target_link_libraries(obl config utils pip_levelizer)

