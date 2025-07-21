# SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

message(STATUS "CMake version: ${CMAKE_VERSION}")

project(kepler-formal
  VERSION 1.0.0
  HOMEPAGE_URL https://github.com/keplertech/kepler-formal
)

set(ARGPARSE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/naja/thirdparty/argparse-3.1/include)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)
add_subdirectory(thirdparty)
add_subdirectory(test)

find_package(TBB REQUIRED)
