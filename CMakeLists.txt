cmake_minimum_required(VERSION 3.14)
project(GraphTransit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

# ---------------------------------------------------------------------
# Core library: everything except main.cpp, so it can be linked into
# both the main executable and the test executable.
# ---------------------------------------------------------------------
add_library(graphtransit_core
    src/Graph.cpp
    src/BusNetwork.cpp
    src/GraphTransformer.cpp
    src/Algorithms.cpp
    src/Tarjan.cpp
    src/FlowNetwork.cpp
    src/RouteAlgorithms.cpp
)

target_include_directories(graphtransit_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---------------------------------------------------------------------
# Main demonstration executable.
# ---------------------------------------------------------------------
add_executable(graph_transit
    src/main.cpp
)

target_link_libraries(graph_transit PRIVATE graphtransit_core)

# ---------------------------------------------------------------------
# Test executable.
# ---------------------------------------------------------------------
enable_testing()

add_executable(graph_transit_tests
    tests/test_main.cpp
)

target_link_libraries(graph_transit_tests PRIVATE graphtransit_core)

add_test(NAME graph_transit_tests COMMAND graph_transit_tests)