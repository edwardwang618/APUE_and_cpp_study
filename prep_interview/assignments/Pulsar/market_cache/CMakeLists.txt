cmake_minimum_required(VERSION 3.14)
project(MarketDataCache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release build by default for benchmarking
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

add_executable(market_cache
    src/main.cpp
    src/market_data_cache.cpp
)

target_include_directories(market_cache PRIVATE include)

# Optional: build tests
add_executable(tests
    tests/test_cache.cpp
    src/market_data_cache.cpp
)
target_include_directories(tests PRIVATE include)