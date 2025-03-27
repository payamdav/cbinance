add_executable(import_raw import_raw.cpp)
target_link_libraries(import_raw obraw)

add_executable(ob_checker ob_checker.cpp)
target_link_libraries(ob_checker ob config)

add_executable(ob_test ob_test.cpp)
target_link_libraries(ob_test ob config)
