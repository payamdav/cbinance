add_library(market_info SHARED market_info.cpp)
target_link_libraries(market_info PUBLIC config utils)
