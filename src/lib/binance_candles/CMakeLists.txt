add_library(binance_candles SHARED binance_candles.cpp)
target_link_libraries(binance_candles PUBLIC utils)
