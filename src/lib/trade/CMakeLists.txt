add_library(trade SHARED trade.cpp tl.cpp)
target_link_libraries(trade PUBLIC pip_levelizer config utils)

