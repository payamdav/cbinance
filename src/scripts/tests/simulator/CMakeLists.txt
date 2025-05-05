add_executable(simulator_test simulator_test.cpp)
target_link_libraries(simulator_test utils config simulator trade pip_levelizer market)
