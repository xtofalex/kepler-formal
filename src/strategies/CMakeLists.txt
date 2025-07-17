cmake_minimum_required(VERSION 3.10)
project(formal_strategies_lib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create a static library target
add_library(formal_strategies STATIC
    miter/MiterStrategy.cpp
)

# Make headers accessible to other targets
target_include_directories(formal_strategies PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/miter)
target_link_libraries(formal_strategies kepler_clauses formal_structures naja_dnl)
