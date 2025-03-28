add_library(obtools SHARED ob_min_max_price.cpp)

target_link_libraries(obtools config utils ob)
