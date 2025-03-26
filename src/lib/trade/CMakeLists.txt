add_library(trade SHARED trade.cpp trades.cpp)
target_link_libraries(trade PUBLIC pip_levelizer)

