cmake_minimum_required(VERSION 3.10)
project(ryu VERSION 2.0 LANGUAGES C CXX)
include(GNUInstallDirs)

# ryu library
add_library(ryu STATIC
        ryu/f2s.c
        ryu/f2s_full_table.h
        ryu/f2s_intrinsics.h
        ryu/d2s.c
        ryu/d2fixed.c
        ryu/d2fixed_full_table.h
        ryu/d2s_full_table.h
        ryu/d2s_small_table.h
        ryu/d2s_intrinsics.h
        ryu/digit_table.h
        ryu/common.h
        ryu/ryu.h)

# This directory is the include root because the headers are in ryu/ and are included as "ryu/*.h"
target_include_directories(ryu PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# add alias so the project can be used with add_subdirectory
add_library(ryu::ryu ALIAS ryu)

# Specify what to install if using CMake to install ryu.
install(TARGETS ryu LIBRARY)
install(FILES ryu/ryu.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ryu)

# ryu_parse library (string to float parsing)
add_library(ryu_parse STATIC
        ryu/s2d.c
        ryu/s2f.c
        ryu/ryu_parse.h
        ryu/common.h
        ryu/d2s_full_table.h
        ryu/d2s_small_table.h
        ryu/d2s_intrinsics.h
        ryu/f2s_full_table.h
        ryu/f2s_intrinsics.h)

target_include_directories(ryu_parse PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

add_library(ryu::ryu_parse ALIAS ryu_parse)

install(TARGETS ryu_parse LIBRARY)
install(FILES ryu/ryu_parse.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ryu)


# generic_128
# Only builds on GCC/Clang/Intel/IntelLLVM due to __uint128_t. No MSVC.
if ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang"
        OR "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU"
        OR "${CMAKE_C_COMPILER_ID}" MATCHES "Intel")

    add_library(generic_128 STATIC
            ryu/generic_128.c
            ryu/generic_128.h
            ryu/ryu_generic_128.h)

    target_include_directories(generic_128 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

    add_library(ryu::generic_128 ALIAS generic_128)

    install(TARGETS generic_128 LIBRARY)
    install(FILES ryu/ryu_generic_128.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ryu)

    set(RYU_GENERIC_128_AVAILABLE ON CACHE BOOL "generic_128 available." FORCE)
else()
    set(RYU_GENERIC_128_AVAILABLE OFF CACHE BOOL "generic_128 not available on this platform." FORCE)
endif()

# Tests, runnable with `ctest` after configuring with -DRYU_BUILD_TESTS=ON (default).
option(RYU_BUILD_TESTS "Build the ryu tests" ON)
if (RYU_BUILD_TESTS)
    enable_testing()

    add_library(gtest STATIC third_party/gtest/gtest-all.cc)
    target_include_directories(gtest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(Threads REQUIRED)
    target_link_libraries(gtest PUBLIC Threads::Threads)

    add_library(gtest_main STATIC third_party/gtest/gtest_main.cc)
    target_link_libraries(gtest_main PUBLIC gtest)

    foreach(test common_test d2s_intrinsics_test d2s_table_test d2s_test f2s_test d2fixed_test)
        add_executable(${test} ryu/tests/${test}.cc)
        target_link_libraries(${test} PRIVATE ryu gtest_main)
        add_test(NAME ${test} COMMAND ${test})
    endforeach()

    foreach(test s2d_test s2f_test roundtrip_test)
        add_executable(${test} ryu/tests/${test}.cc)
        target_link_libraries(${test} PRIVATE ryu ryu_parse gtest_main)
        add_test(NAME ${test} COMMAND ${test})
    endforeach()

    if (RYU_GENERIC_128_AVAILABLE)
        add_executable(generic_128_test ryu/tests/generic_128_test.cc)
        target_link_libraries(generic_128_test PRIVATE generic_128 gtest_main)
        add_test(NAME generic_128_test COMMAND generic_128_test)
    endif()
endif()
