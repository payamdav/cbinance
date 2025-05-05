cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LANGUAGE CXX)
set(CMAKE_BUILD_PARALLEL_LEVEL 4)
set(CMAKE_GENERATOR "Ninja")
set(CMAKE_LINKER_TYPE LLD)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


project(cbinance)

add_subdirectory(src/lib/utils)
add_subdirectory(src/lib/config)
add_subdirectory(src/lib/market_info)
add_subdirectory(src/lib/ta/timeseries)
add_subdirectory(src/lib/ta/pip_levelizer)
add_subdirectory(src/lib/ta/vols)
add_subdirectory(src/lib/binance_candles)
add_subdirectory(src/lib/statistics)
add_subdirectory(src/lib/ob/raw_files)
add_subdirectory(src/lib/ob)
add_subdirectory(src/lib/ob/obtools)
add_subdirectory(src/lib/ob/obl_creator)
add_subdirectory(src/lib/ob/obl)
add_subdirectory(src/lib/trade)
add_subdirectory(src/lib/ta/zigzag)
add_subdirectory(src/lib/ob/obl_ext/obl_instant) # Must be after obl, as it links to it
add_subdirectory(src/lib/ob/obl_ext/obl_sum_up_sum_prev) # Must be after obl, as it links to it
add_subdirectory(src/lib/imageprocessing)
add_subdirectory(src/lib/ta/candle_anal)
add_subdirectory(src/lib/market)
add_subdirectory(src/lib/simulator)
# add_subdirectory(src/lib/ta/volume_profiler)
add_subdirectory(src/scripts/depth)
add_subdirectory(src/scripts/trades)
add_subdirectory(src/scripts/chart_data)
add_subdirectory(src/scripts/tests)

add_executable(cbinance main.cpp)
# target_link_libraries(cbinance )


