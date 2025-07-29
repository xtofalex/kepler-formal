# SPDX-FileCopyrightText: 2023 The Naja authors <https://github.com/najaeda/naja/blob/main/AUTHORS>
#
# SPDX-License-Identifier: Apache-2.0

add_executable(kepler_formal KeplerFormal.cpp)
target_include_directories(kepler_formal SYSTEM BEFORE PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(kepler_formal PUBLIC ${ARGPARSE_DIR})
target_link_libraries(kepler_formal
    naja_snl_pyloader
    naja_dnl naja_opt formal_strategies)
install(TARGETS kepler_formal DESTINATION ${CMAKE_INSTALL_BINDIR})
