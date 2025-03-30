add_library(obl_creator SHARED obl_creator.cpp snapshot_converter.cpp)
target_link_libraries(obl_creator config utils ob pip_levelizer)
