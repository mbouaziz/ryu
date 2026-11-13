// Copyright 2026 Mehdi Bouaziz
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

// Randomized round-trip tests: d2s()/f2s() output must parse back to the
// exact same floating-point value, both with the C library strtod/strtof and
// with Ryu's own s2d_n()/s2f_n() parsers.

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ryu/ryu.h"
#include "ryu/ryu_parse.h"
#include "third_party/gtest/gtest.h"

static uint64_t nextRandom(uint64_t* state) {
  uint64_t x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  *state = x;
  return x;
}

static double int64Bits2Double(uint64_t bits) {
  double f;
  memcpy(&f, &bits, sizeof(double));
  return f;
}

static float int32Bits2Float(uint32_t bits) {
  float f;
  memcpy(&f, &bits, sizeof(float));
  return f;
}

static uint64_t doubleBits(double f) {
  uint64_t bits;
  memcpy(&bits, &f, sizeof(double));
  return bits;
}

static uint32_t floatBits(float f) {
  uint32_t bits;
  memcpy(&bits, &f, sizeof(float));
  return bits;
}

TEST(RoundTripTest, D2sStrtod) {
  uint64_t state = 0x243f6a8885a308d3ull;
  for (int i = 0; i < 1000000; ++i) {
    const double f = int64Bits2Double(nextRandom(&state));
    if (!isfinite(f)) {
      continue;
    }
    char* s = d2s(f);
    ASSERT_NE(nullptr, s);
    char* end = nullptr;
    const double g = strtod(s, &end);
    ASSERT_EQ(s + strlen(s), end);
    EXPECT_EQ(doubleBits(f), doubleBits(g)) << "s = " << s;
    free(s);
  }
}

TEST(RoundTripTest, D2sS2d) {
  uint64_t state = 0x13198a2e03707344ull;
  int checked = 0;
  for (int i = 0; i < 1000000; ++i) {
    const double f = int64Bits2Double(nextRandom(&state));
    if (!isfinite(f)) {
      continue;
    }
    char buffer[25]; // max d2s output including NUL
    const int len = d2s_buffered_n(f, buffer);
    buffer[len] = '\0';
    double g = 0.0;
    ASSERT_EQ(SUCCESS, s2d_n(buffer, len, &g)) << "s = " << buffer;
    EXPECT_EQ(doubleBits(f), doubleBits(g)) << "s = " << buffer;
    ++checked;
  }
  EXPECT_GT(checked, 900000);
}

TEST(RoundTripTest, D2sSpecialValues) {
  double g = 0.0;
  EXPECT_EQ(SUCCESS, s2d_n("0E0", 3, &g));
  EXPECT_EQ(0.0, g);
  EXPECT_EQ(SUCCESS, s2d_n("-0E0", 4, &g));
  EXPECT_EQ(0.0, g);
  EXPECT_EQ(doubleBits(-0.0), doubleBits(g));
}

TEST(RoundTripTest, F2sStrtof) {
  uint64_t state = 0xa4093822299f31d0ull;
  for (int i = 0; i < 1000000; ++i) {
    const float f = int32Bits2Float((uint32_t) nextRandom(&state));
    if (!isfinite(f)) {
      continue;
    }
    char* s = f2s(f);
    ASSERT_NE(nullptr, s);
    char* end = nullptr;
    const float g = strtof(s, &end);
    ASSERT_EQ(s + strlen(s), end);
    EXPECT_EQ(floatBits(f), floatBits(g)) << "s = " << s;
    free(s);
  }
}

TEST(RoundTripTest, F2sS2f) {
  uint64_t state = 0x082f31d0a4093822ull;
  int checked = 0;
  for (int i = 0; i < 1000000; ++i) {
    const float f = int32Bits2Float((uint32_t) nextRandom(&state));
    if (!isfinite(f)) {
      continue;
    }
    char buffer[16]; // max f2s output including NUL
    const int len = f2s_buffered_n(f, buffer);
    buffer[len] = '\0';
    float g = 0.0f;
    ASSERT_EQ(SUCCESS, s2f_n(buffer, len, &g)) << "s = " << buffer;
    EXPECT_EQ(floatBits(f), floatBits(g)) << "s = " << buffer;
    ++checked;
  }
  EXPECT_GT(checked, 900000);
}

// Small integers are a common special case (issue #144): verify them
// explicitly, including trailing-zero mantissas.
TEST(RoundTripTest, SmallIntegers) {
  uint64_t state = 0x94d049bb133111ebull;
  for (int i = 0; i < 100000; ++i) {
    const uint64_t m = nextRandom(&state) % (1ull << 53);
    const double f = (double) m;
    char buffer[25]; // max d2s output including NUL
    const int len = d2s_buffered_n(f, buffer);
    buffer[len] = '\0';
    double g = 0.0;
    ASSERT_EQ(SUCCESS, s2d_n(buffer, len, &g)) << "s = " << buffer;
    EXPECT_EQ(doubleBits(f), doubleBits(g)) << "s = " << buffer;
  }
}
