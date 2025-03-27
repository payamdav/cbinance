add_library(ob SHARED snapshot.cpp update.cpp ob.cpp)

target_link_libraries(ob config utils obraw)
