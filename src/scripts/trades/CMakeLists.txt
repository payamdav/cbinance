add_executable(trade_test trade_test.cpp)
target_link_libraries(trade_test trade config pip_levelizer utils)

add_executable(import_trades import_trades.cpp)
target_link_libraries(import_trades trade config)
