add_library(vols SHARED vols.cpp)
target_link_libraries(vols timeseries binance_candles)
