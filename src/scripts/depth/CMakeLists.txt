add_executable(import_raw import_raw.cpp)
add_executable(spread_file_gen spread_file_gen.cpp)

target_link_libraries(import_raw obraw)
target_link_libraries(spread_file_gen ob config)
