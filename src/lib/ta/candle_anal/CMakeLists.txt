add_library(candle_anal SHARED candle_anal.cpp)
target_link_libraries(candle_anal binance_candles config utils pip_levelizer)
