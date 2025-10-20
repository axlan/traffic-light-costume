cmake_minimum_required(VERSION 3.5)
project(traffic_light_costume_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add test executable that builds the RMC parser and a small unit test harness.
add_executable(rmc_test
    lib/RMCParser/RMCParser.cpp
    tests/rmc_test.cpp
)

# Ensure the parser header is discoverable
target_include_directories(rmc_test PRIVATE ${CMAKE_SOURCE_DIR}/lib/RMCParser)

enable_testing()
add_test(NAME rmc_test COMMAND rmc_test)
