add_executable(trade_test trade_test.cpp)
target_link_libraries(trade_test trade config pip_levelizer utils)

add_executable(import_trades import_trades.cpp)
target_link_libraries(import_trades trade config)

add_executable(set_min_max_price set_min_max_price.cpp)
target_link_libraries(set_min_max_price trade config market_info)

add_executable(trades_to_tls trades_to_tls.cpp)
target_link_libraries(trades_to_tls trade config)
