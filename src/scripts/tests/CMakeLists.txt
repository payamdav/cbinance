add_executable(obl_instant_test obl_instant_test.cpp)
target_link_libraries(obl_instant_test obl utils obl_instant statistics)

add_executable(candles_test candles_test.cpp)
target_link_libraries(candles_test binance_candles utils pip_levelizer vols config)

add_subdirectory(imagep)