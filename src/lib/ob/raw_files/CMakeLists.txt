add_library(obraw SHARED ob_raw_files.cpp)

target_link_libraries(obraw config utils config)
