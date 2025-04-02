add_executable(import_raw import_raw.cpp)
target_link_libraries(import_raw obraw)

add_executable(obl_build obl_build.cpp)
target_link_libraries(obl_build ob config obl_creator)

add_executable(ob_checker ob_checker.cpp)
target_link_libraries(ob_checker ob config)

add_executable(ob_test ob_test.cpp)
target_link_libraries(ob_test ob config obtools obl_creator)

add_executable(obl_test obl_test.cpp)
target_link_libraries(obl_test ob obl config)
