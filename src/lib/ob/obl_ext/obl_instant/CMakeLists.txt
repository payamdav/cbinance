add_library(obl_instant SHARED obl_instant.cpp)

target_link_libraries(obl_instant obl utils config vols binance_candles)

