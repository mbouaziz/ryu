cmake_minimum_required(VERSION 3.10)
project(ryu VERSION 2.0 LANGUAGES C CXX)
include(GNUInstallDirs)

# ryu library
add_library(ryu
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


# generic_128
# Only builds on GCC/Clang/Intel/IntelLLVM due to __uint128_t. No MSVC.
if ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang"
        OR "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU"
        OR "${CMAKE_C_COMPILER_ID}" MATCHES "Intel")

    add_library(generic_128
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

# Benchmarks, built with -DRYU_BUILD_BENCHMARKS=ON (off by default). These
# need POSIX headers, so they are skipped on Windows.
option(RYU_BUILD_BENCHMARKS "Build the ryu benchmarks" OFF)
if (RYU_BUILD_BENCHMARKS AND NOT WIN32)
    add_library(mersenne STATIC third_party/mersenne/random.c)
    target_include_directories(mersenne PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mersenne)

    add_executable(ryu_printf_benchmark_c ryu/benchmark/benchmark_fixed.c)
    target_include_directories(ryu_printf_benchmark_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(ryu_printf_benchmark_c PRIVATE ryu mersenne m)

    add_library(double-conversion STATIC
            third_party/double-conversion/double-conversion/bignum.cc
            third_party/double-conversion/double-conversion/bignum-dtoa.cc
            third_party/double-conversion/double-conversion/cached-powers.cc
            third_party/double-conversion/double-conversion/diy-fp.cc
            third_party/double-conversion/double-conversion/double-conversion.cc
            third_party/double-conversion/double-conversion/fast-dtoa.cc
            third_party/double-conversion/double-conversion/fixed-dtoa.cc
            third_party/double-conversion/double-conversion/strtod.cc)
    target_include_directories(double-conversion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/double-conversion)

    add_executable(ryu_benchmark ryu/benchmark/benchmark.cc)
    target_include_directories(ryu_benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(ryu_benchmark PRIVATE ryu double-conversion m)
endif()
