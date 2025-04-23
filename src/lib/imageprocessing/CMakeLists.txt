add_library(imagep SHARED core/image.cpp)
target_link_libraries(imagep PUBLIC config utils trade pip_levelizer)
