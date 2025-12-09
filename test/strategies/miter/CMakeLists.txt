# Copyright 2024-2025 keplertech.io
# SPDX-License-Identifier: GPL-3.0-only

add_executable(miterTests MiterTests.cpp)

target_link_libraries(miterTests
    naja_nl_dump
    naja_opt
    kepler_clauses
    formal_strategies
    gmock gtest_main
)

GTEST_DISCOVER_TESTS(miterTests)