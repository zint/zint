/*
    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"
#include "../large.h"

#if defined(__MINGW32__)
#  if __WORDSIZE == 32
#    define LX_FMT "I32"
#  else
#    define LX_FMT "I64"
#  endif
#  if defined(__clang__)
#    pragma GCC diagnostic ignored "-Wformat-non-iso"
#  elif defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wformat" /* Unfortunately doesn't seem to be way to only avoid non-ISO warnings */
#  endif
#elif defined(_MSC_VER) || __WORDSIZE == 32
#  define LX_FMT "ll"
#else
#  define LX_FMT "l"
#endif

#define LI(l, h) { l, h }

int clz_u64(uint64_t x);

static void test_clz_u64(int index) {

    struct item {
        uint64_t s;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 0x0, 64 },
        /*  1*/ { 0x1, 63 },
        /*  2*/ { 0x2, 62 },
        /*  3*/ { 0x3, 62 },
        /*  4*/ { 0x4, 61 },
        /*  5*/ { 0x5, 61 },
        /*  6*/ { 0x6, 61 },
        /*  7*/ { 0x7, 61 },
        /*  8*/ { 0x8, 60 },
        /*  9*/ { 0x9, 60 },
        /* 10*/ { 0xA, 60 },
        /* 11*/ { 0xB, 60 },
        /* 12*/ { 0xC, 60 },
        /* 13*/ { 0xD, 60 },
        /* 14*/ { 0xE, 60 },
        /* 15*/ { 0xF, 60 },
        /* 16*/ { 0x10, 59 },
        /* 17*/ { 0x11, 59 },
        /* 18*/ { 0x12, 59 },
        /* 19*/ { 0x13, 59 },
        /* 20*/ { 0x14, 59 },
        /* 21*/ { 0x15, 59 },
        /* 22*/ { 0x16, 59 },
        /* 23*/ { 0x17, 59 },
        /* 24*/ { 0x18, 59 },
        /* 25*/ { 0x19, 59 },
        /* 26*/ { 0x1A, 59 },
        /* 27*/ { 0x1B, 59 },
        /* 28*/ { 0x1C, 59 },
        /* 29*/ { 0x1D, 59 },
        /* 30*/ { 0x1E, 59 },
        /* 31*/ { 0x1F, 59 },
        /* 32*/ { 0x20, 58 },
        /* 33*/ { 0x21, 58 },
        /* 34*/ { 0x22, 58 },
        /* 35*/ { 0x23, 58 },
        /* 36*/ { 0x2F, 58 },
        /* 37*/ { 0x40, 57 },
        /* 38*/ { 0x49, 57 },
        /* 39*/ { 0x50, 57 },
        /* 40*/ { 0x5F, 57 },
        /* 41*/ { 0x60, 57 },
        /* 42*/ { 0x7F, 57 },
        /* 43*/ { 0x80, 56 },
        /* 44*/ { 0x88, 56 },
        /* 45*/ { 0xA1, 56 },
        /* 46*/ { 0xFF, 56 },
        /* 47*/ { 0x100, 55 },
        /* 48*/ { 0x165, 55 },
        /* 49*/ { 0x1FF, 55 },
        /* 50*/ { 0x384, 54 },
        /* 51*/ { 0x555, 53 },
        /* 52*/ { 0xBCD, 52 },
        /* 53*/ { 0x1FFF, 51 },
        /* 54*/ { 0x2E06, 50 },
        /* 55*/ { 0x7040, 49 },
        /* 56*/ { 0x8001, 48 },
        /* 57*/ { 0xC0FF, 48 },
        /* 58*/ { 0x1C0FF, 47 },
        /* 59*/ { 0x2211E, 46 },
        /* 60*/ { 0x44220, 45 },
        /* 61*/ { 0x50505, 45 },
        /* 62*/ { 0x88888, 44 },
        /* 63*/ { 0x111111, 43 },
        /* 64*/ { 0x222222, 42 },
        /* 65*/ { 0x444444, 41 },
        /* 66*/ { 0xFF00FF, 40 },
        /* 67*/ { 0x10B8392, 39 },
        /* 68*/ { 0x2FFFFFF, 38 },
        /* 69*/ { 0x4040404, 37 },
        /* 70*/ { 0x7777777, 37 },
        /* 71*/ { 0xF0F0F0F, 36 },
        /* 72*/ { 0x194F0311, 35 },
        /* 73*/ { 0x33333333, 34 },
        /* 74*/ { 0x55555555, 33 },
        /* 75*/ { 0xAAAAAAAA, 32 },
        /* 76*/ { 0x100000000, 31 },
        /* 77*/ { 0x2FFFFFFFF, 30 },
        /* 78*/ { 0x304050607, 30 },
        /* 79*/ { 0x707070707, 29 },
        /* 80*/ { 0x999999999, 28 },
        /* 81*/ { 0xEEEEEEEEE, 28 },
        /* 82*/ { 0x1000000001, 27 },
        /* 83*/ { 0x2D2D2D2D2D, 26 },
        /* 84*/ { 0x68034DAE71, 25 },
        /* 85*/ { 0xF462103784, 24 },
        /* 86*/ { 0x1CCCCCCCCCC, 23 },
        /* 87*/ { 0x2F462103784, 22 },
        /* 88*/ { 0x4F4E4D4C4B4, 21 },
        /* 89*/ { 0x9FFFFFFFFFF, 20 },
        /* 90*/ { 0x100000000000, 19 },
        /* 91*/ { 0x369D03178212, 18 },
        /* 92*/ { 0x666000666000, 17 },
        /* 93*/ { 0xFFFFFFFFFFFF, 16 },
        /* 94*/ { 0x123456789ABCD, 15 },
        /* 95*/ { 0x3FFFFFFFFFFFF, 14 },
        /* 96*/ { 0x429C8174831A0, 13 },
        /* 97*/ { 0xBBBBBBBBBBBBB, 12 },
        /* 98*/ { 0x11111111111111, 11 },
        /* 99*/ { 0x24242424242424, 10 },
        /*100*/ { 0x44444444444444, 9 },
        /*101*/ { 0x567890ABCDEF01, 9 },
        /*102*/ { 0xFFFFFFFFFFFFFF, 8 },
        /*103*/ { 0x100000000000100, 7 },
        /*104*/ { 0x37F037F047F037F, 6 },
        /*105*/ { 0x60123456789ABCD, 5 },
        /*106*/ { 0xDDDDDDDDDDDDDDD, 4 },
        /*107*/ { 0xFEDCBA987654321, 4 },
        /*108*/ { 0xFFFFFFFFFFFFFFF, 4 },
        /*109*/ { 0x1000000000000000, 3 },
        /*110*/ { 0x1000000000000001, 3 },
        /*111*/ { 0x2000000000010000, 2 },
        /*112*/ { 0x3100000000000000, 2 },
        /*113*/ { 0x4001000000000000, 1 },
        /*114*/ { 0x5000000100000000, 1 },
        /*115*/ { 0x6000001000000000, 1 },
        /*116*/ { 0x7000000001000000, 1 },
        /*117*/ { 0x8000100000000000, 0 },
        /*118*/ { 0x9000000000001000, 0 },
        /*119*/ { 0xA000000000000010, 0 },
        /*120*/ { 0xB000000000000000, 0 },
        /*121*/ { 0xC010000000000000, 0 },
        /*122*/ { 0xD000010000000000, 0 },
        /*123*/ { 0xE111111111111111, 0 },
        /*124*/ { 0xF000000000000000, 0 },
        /*125*/ { 0xFFFFFFFFFFFFFFFF, 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, ret;

    testStart("test_clz_u64");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        ret = clz_u64(data[i].s);
        assert_equal(ret, data[i].ret, "i:%d 0x%" LX_FMT "X ret %d != %d\n", i, data[i].s, ret, data[i].ret);
    }

    testFinish();
}

static void test_load(int index) {

    struct item {
        large_int t;
        large_int s;
        large_int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), LI(0, 0), LI(0, 0) },
        /*  1*/ { LI(1, 1), LI(0, 0), LI(0, 0) },
        /*  2*/ { LI(0, 0), LI(1, 2), LI(1, 2) },
        /*  3*/ { LI(1, 1), LI(2, 3), LI(2, 3) },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char t_dump[35];
    char expected_dump[35];

    testStart("test_load");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        large_load(&data[i].t, &data[i].s);

        assert_equalu64(data[i].t.lo, data[i].expected.lo, "i:%d lo 0x%" LX_FMT "X (%s) != expected lo 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.lo, large_dump(&data[i].t, t_dump), data[i].expected.lo, large_dump(&data[i].expected, expected_dump));
        assert_equalu64(data[i].t.hi, data[i].expected.hi, "i:%d hi 0x%" LX_FMT "X (%s) != expected hi 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.hi, large_dump(&data[i].t, t_dump), data[i].expected.hi, large_dump(&data[i].expected, expected_dump));
    }

    testFinish();
}

static void test_load_str_u64(int index) {

    struct item {
        large_int t;
        const char *s;
        int length;
        large_int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), "0", -1, LI(0, 0) },
        /*  1*/ { LI(0, 1), "1", -1, LI(1, 0) },
        /*  2*/ { LI(1, 1), "4294967296", -1, LI(4294967296, 0) },
        /*  3*/ { LI(1, 1), "18446744073709551615", -1, LI(0xFFFFFFFFFFFFFFFF, 0) },
        /*  4*/ { LI(1, 1), "18446744073709551616", -1, LI(0, 0) }, // Overflow 18446744073709551616 == 2^64 == 0
        /*  5*/ { LI(2, 2), "123", 2, LI(12, 0) }, // Only reads up to length
        /*  6*/ { LI(2, 2), "123A1X", -1, LI(123, 0) }, // Only reads decimal
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char t_dump[35];
    char expected_dump[35];

    testStart("test_load_str_u64");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        large_load_str_u64(&data[i].t, (unsigned char *) data[i].s, data[i].length == -1 ? (int) strlen(data[i].s) : data[i].length);

        assert_equalu64(data[i].t.lo, data[i].expected.lo, "i:%d lo 0x%" LX_FMT "X (%s) != expected lo 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.lo, large_dump(&data[i].t, t_dump), data[i].expected.lo, large_dump(&data[i].expected, expected_dump));
        assert_equalu64(data[i].t.hi, data[i].expected.hi, "i:%d hi 0x%" LX_FMT "X (%s) != expected hi 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.hi, large_dump(&data[i].t, t_dump), data[i].expected.hi, large_dump(&data[i].expected, expected_dump));
    }

    testFinish();
}

static void test_add_u64(int index) {

    struct item {
        large_int t;
        uint64_t s;
        large_int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), 0, LI(0, 0) },
        /*  1*/ { LI(1, 0), 0, LI(1, 0) },
        /*  2*/ { LI(0, 1), 0, LI(0, 1) },
        /*  3*/ { LI(1, 0), 1, LI(2, 0) },
        /*  4*/ { LI(1, 1), 0xFFFFFFFFFFFFFFFF, LI(0, 2) },
        /*  5*/ { LI(1, 1), 2, LI(3, 1) },
        /*  6*/ { LI(0xFFFFFFFFFFFFFFFF, 1), 1, LI(0, 2) },
        /*  7*/ { LI(0xFFFFFFFFFFFFFFFE, 100), 4, LI(2, 101) },
        /*  8*/ { LI(0xFFFFFFFFFFFFFF0E, 0xFFFFFE), 0xFF, LI(0xD, 0xFFFFFF) },
        /*  9*/ { LI(0xFFFFFFFF00000001, 0xFFFFFFFF), 0xFFFFFFFF, LI(0, 0x100000000) },
        /* 10*/ { LI(0x0000000000000001, 0xFFFFFFFFFFFFFFFF), 0xFFFFFFFFFFFFFFFF, LI(0, 0) }, // Overflow
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char t_dump[35];
    char expected_dump[35];

    testStart("test_add_u64");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        large_add_u64(&data[i].t, data[i].s);

        assert_equalu64(data[i].t.lo, data[i].expected.lo, "i:%d lo 0x%" LX_FMT "X (%s) != expected lo 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.lo, large_dump(&data[i].t, t_dump), data[i].expected.lo, large_dump(&data[i].expected, expected_dump));
        assert_equalu64(data[i].t.hi, data[i].expected.hi, "i:%d hi 0x%" LX_FMT "X (%s) != expected hi 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.hi, large_dump(&data[i].t, t_dump), data[i].expected.hi, large_dump(&data[i].expected, expected_dump));
    }

    testFinish();
}

static void test_sub_u64(int index) {

    struct item {
        large_int t;
        uint64_t s;
        large_int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), 0, LI(0, 0) },
        /*  1*/ { LI(1, 0), 0, LI(1, 0) },
        /*  2*/ { LI(0, 1), 0, LI(0, 1) },
        /*  3*/ { LI(1, 0), 1, LI(0, 0) },
        /*  4*/ { LI(0xFFFFFFFFFFFFFFFF, 1), 0xFFFFFFFFFFFFFFFF, LI(0, 1) },
        /*  5*/ { LI(0xFFFFFFFFFFFFFFFE, 1), 0xFFFFFFFFFFFFFFFF, LI(0xFFFFFFFFFFFFFFFF, 0) },
        /*  6*/ { LI(1, 1), 0xFFFFFFFFFFFFFFFF, LI(2, 0) },
        /*  7*/ { LI(1, 1), 0xFFFFFFFFFFFFFFFE, LI(3, 0) },
        /*  8*/ { LI(2, 0xFFFFFFFFFFFFFFFF), 0xFFFFFFFFFFFFFFFF, LI(3, 0xFFFFFFFFFFFFFFFE) },
        /*  9*/ { LI(2, 0xFFFFFFFFFFFFFFFF), 0xFFFFFFFFFFFFFFFE, LI(4, 0xFFFFFFFFFFFFFFFE) },
        /* 10*/ { LI(0, 0), 1, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char t_dump[35];
    char expected_dump[35];

    testStart("test_sub_u64");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        large_sub_u64(&data[i].t, data[i].s);

        assert_equalu64(data[i].t.lo, data[i].expected.lo, "i:%d lo 0x%" LX_FMT "X (%s) != expected lo 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.lo, large_dump(&data[i].t, t_dump), data[i].expected.lo, large_dump(&data[i].expected, expected_dump));
        assert_equalu64(data[i].t.hi, data[i].expected.hi, "i:%d hi 0x%" LX_FMT "X (%s) != expected hi 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.hi, large_dump(&data[i].t, t_dump), data[i].expected.hi, large_dump(&data[i].expected, expected_dump));
    }

    testFinish();
}

static void test_mul_u64(int index) {

    struct item {
        large_int t;
        uint64_t s;
        large_int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), 0, LI(0, 0) },
        /*  1*/ { LI(1, 0), 0, LI(0, 0) },
        /*  2*/ { LI(0, 1), 0, LI(0, 0) },
        /*  3*/ { LI(1, 0), 1, LI(1, 0) },
        /*  4*/ { LI(0, 1), 1, LI(0, 1) },
        /*  5*/ { LI(1, 1), 2, LI(2, 2) },
        /*  6*/ { LI(3, 0x64), 3, LI(9, 0x12C) },
        /*  7*/ { LI(0x5555555555555555, 0x5555555555555555), 3, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /*  8*/ { LI(432, 518), 4, LI(1728, 2072) },
        /*  9*/ { LI(0x4000000000000000, 0), 4, LI(0, 1) },
        /* 10*/ { LI(0xFFFFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFFF), 4, LI(0xFFFFFFFFFFFFFFFC, 0xFFFFFFFFFFFFFFFF) },
        /* 11*/ { LI(0x3333333333333333, 0x111), 5, LI(0xFFFFFFFFFFFFFFFF, 0x555) },
        /* 12*/ { LI(0x3333333333333334, 0x111), 5, LI(4, 0x556) },
        /* 13*/ { LI(0x2222222222222222, 0x2222222222222222), 8, LI(0x1111111111111110, 0x1111111111111111) }, // Overflow
        /* 14*/ { LI(432, 518), 10, LI(4320, 5180) },
        /* 15*/ { LI(0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCC), 20, LI(0xFFFFFFFFFFFFFFF0, 0xFFFFFFFFFFFFFFFF) },
        /* 16*/ { LI(432, 518), 100, LI(43200, 51800) },
        /* 17*/ { LI(0x123456789ABCDEF0, 0x123456789ABCDE), 0xE0F, LI(0xEDCBA98765423010, 0xFFEDCBA987653601) },
        /* 18*/ { LI(0xFFFFFFFFFFFFFFFF, 1), 2, LI(0xFFFFFFFFFFFFFFFE, 3) },
        /* 19*/ { LI(0xFFFFFFFFFFFFFFFF, 1), 10, LI(0xFFFFFFFFFFFFFFF6, 19) },
        /* 20*/ { LI(0xFFFFFFFFFFFFFFFF, 1), 0x1234567890ABCDEF, LI(0xEDCBA9876F543211, 0x2468ACF121579BDD) },
        /* 21*/ { LI(0xFFFFFFFFFFFFFFFF, 0xCF), 0x123456789ABCDEF, LI(0xFEDCBA9876543211, 0xECA8641FDB97522F) },
        /* 22*/ { LI(0xFFFFFFFFFFFFFFFF, 0x123456), 0x123456789AB, LI(0xFFFFFEDCBA987655, 0x14B66E5D4C2C851C) },
        /* 23*/ { LI(0xFFFFFFFFFFFFFFFF, 0), 0xFFFFFFFFFFFFFFFF, LI(1, 0xFFFFFFFFFFFFFFFE) },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char t_dump[35];
    char expected_dump[35];

    testStart("test_mul_u64");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        large_mul_u64(&data[i].t, data[i].s);

        assert_equalu64(data[i].t.lo, data[i].expected.lo, "i:%d lo 0x%" LX_FMT "X (%s) != expected lo 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.lo, large_dump(&data[i].t, t_dump), data[i].expected.lo, large_dump(&data[i].expected, expected_dump));
        assert_equalu64(data[i].t.hi, data[i].expected.hi, "i:%d hi 0x%" LX_FMT "X (%s) != expected hi 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.hi, large_dump(&data[i].t, t_dump), data[i].expected.hi, large_dump(&data[i].expected, expected_dump));
    }

    testFinish();
}

static void test_div_u64(int index) {

    uint64_t r;
    struct item {
        large_int t;
        uint64_t s;
        uint64_t expected_r;
        large_int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), 1, 0, LI(0, 0) },
        /*  1*/ { LI(1, 0), 1, 0, LI(1, 0) },
        /*  2*/ { LI(4990000, 0), 509999, 400009, LI(9, 0) },
        /*  3*/ { LI(3, 0), 2, 1, LI(1, 0) }, // BEGIN divmnu64.c.txt (first 3 errors)
        /*  4*/ { LI(3, 0), 3, 0, LI(1, 0) },
        /*  5*/ { LI(3, 0), 4, 3, LI(0, 0) },
        /*  6*/ { LI(0, 0), 0xFFFFFFFF, 0, LI(0, 0) },
        /*  7*/ { LI(0xFFFFFFFF, 0), 1, 0, LI(0xFFFFFFFF, 0) },
        /*  8*/ { LI(0xFFFFFFFF, 0), 0xFFFFFFFF, 0, LI(1, 0) },
        /*  9*/ { LI(0xFFFFFFFF, 0), 3, 0, LI(0x55555555, 0) },
        /* 10*/ { LI(0xFFFFFFFFFFFFFFFF, 0), 1, 0, LI(0xFFFFFFFFFFFFFFFF, 0) },
        /* 11*/ { LI(0xFFFFFFFFFFFFFFFF, 0), 0xFFFFFFFF, 0, LI(0x100000001, 0) },
        /* 12*/ { LI(0xFFFFFFFEFFFFFFFF, 0), 0xFFFFFFFF, 0xFFFFFFFE, LI(0xFFFFFFFF, 0) },
        /* 13*/ { LI(0x0000123400005678, 0), 0x00009ABC, 0x6BD0, LI(0x1E1DBA76, 0) },
        /* 14*/ { LI(0, 0), 0x100000000, 0, LI(0, 0) },
        /* 15*/ { LI(0x700000000, 0), 0x300000000, 0x100000000, LI(2, 0) },
        /* 16*/ { LI(0x700000005, 0), 0x300000000, 0x100000005, LI(2, 0) },
        /* 17*/ { LI(0x600000000, 0), 0x200000000, 0, LI(3, 0) },
        /* 18*/ { LI(0x80000000, 0), 0x40000001, 0x3FFFFFFF, LI(1, 0) },
        /* 19*/ { LI(0x8000000000000000, 0), 0x40000001, 8, LI(0x1FFFFFFF8, 0) },
        /* 20*/ { LI(0x8000000000000000, 0), 0x4000000000000001, 0x3FFFFFFFFFFFFFFF, LI(1, 0) },
        /* 21*/ { LI(0x0000BCDE0000789A, 0), 0x0000BCDE0000789A, 0, LI(1, 0) },
        /* 22*/ { LI(0x0000BCDE0000789B, 0), 0x0000BCDE0000789A, 1, LI(1, 0) },
        /* 23*/ { LI(0x0000BCDE00007899, 0), 0x0000BCDE0000789A, 0x0000BCDE00007899, LI(0, 0) },
        /* 24*/ { LI(0x0000FFFF0000FFFF, 0), 0x0000FFFF0000FFFF, 0, LI(1, 0) },
        /* 25*/ { LI(0x0000FFFF0000FFFF, 0), 0x100000000, 0x0000FFFF, LI(0x0000FFFF, 0) },
        /* 26*/ { LI(0x00004567000089AB, 0x00000123), 0x100000000, 0x000089AB, LI(0x0000012300004567, 0) },
        /* 27*/ { LI(0x0000FFFE00000000, 0x00008000), 0x000080000000FFFF, 0x7FFF0000FFFF, LI(0xFFFFFFFF, 0) }, // END divmnu64.c.txt (last 6 96-bit divisor); shows that first qhat0 can = b + 1
        /* 28*/ { LI(0, 0x80000000FFFE0000), 0x80000000FFFF0000, 0x1FFFE00000000, LI(0xFFFFFFFFFFFE0000, 0) }, // Shows that first qhat1 can = b + 1
        /* 29*/ { LI(0xFFFE000000000000, 0x80000000), 0x80000000FFFF0000, 0x7FFF0000FFFF0000, LI(0xFFFFFFFF, 0) }, // First qhat0 = b + 1
        /* 30*/ { LI(0x7FFF800000000000, 0), 0x800000000001, 0x7FFFFFFF0002, LI(0xFFFE, 0) }, // "add back" examples won't trigger as divisor only 2 digits (in 2**32 base)
        /* 31*/ { LI(0, 0x7FFF800000000000), 0x800000000001, 0x1FFFE0000, LI(0xFFFFFFFE00020000, 0xFFFE) },
        /* 32*/ { LI(0x0000000000000003, 0x80000000), 0x2000000000000001, 0x1FFFFFFC00000004, LI(0x3FFFFFFFF, 0) },
        /* 33*/ { LI(0x0000000000000003, 0x00008000), 0x2000000000000001, 0x1FFFFFFFFFFC0004, LI(0x3FFFF, 0) },
        /* 34*/ { LI(0x8000000000000003, 0), 0x2000000000000001, 0x2000000000000000, LI(3, 0) },
        /* 35*/ { LI(0x0000000000000000, 0x00007FFF00008000), 0x8000000000000001, 0x7FFF0001FFFF0001, LI(0xFFFE0000FFFF, 0) },
        /* 36*/ { LI(0xFFFFFFFFFFFFFFFF, 0), 0xFFFFFFFFFFFFFFFF, 0, LI(1, 0) },
        /* 37*/ { LI(0, 1), 1, 0, LI(0, 1) },
        /* 38*/ { LI(1, 1), 1, 0, LI(1, 1) },
        /* 39*/ { LI(1, 1), 2, 1, LI(0x8000000000000000, 0) },
        /* 40*/ { LI(2, 1), 2, 0, LI(0x8000000000000001, 0) },
        /* 41*/ { LI(3, 1), 3, 1, LI(0x5555555555555556, 0) },
        /* 42*/ { LI(0, 1), 3, 1, LI(0x5555555555555555, 0) },
        /* 43*/ { LI(1, 1), 3, 2, LI(0x5555555555555555, 0) },
        /* 44*/ { LI(0, 7), 3, 1, LI(0x5555555555555555, 2) },
        /* 45*/ { LI(5, 7), 3, 0, LI(0x5555555555555557, 2) },
        /* 46*/ { LI(0, 10), 3, 1, LI(0x5555555555555555, 3) },
        /* 47*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFE), 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFE, LI(0xFFFFFFFF, 0) },
        /* 48*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 3, 0, LI(0x5555555555555555, 0x5555555555555555) },
        /* 49*/ { LI(0x0000000100000000, 0), 3, 1, LI(0x55555555, 0) },
        /* 50*/ { LI(0, 0x0000000100000000), 3, 1, LI(0x5555555555555555, 0x55555555) },
        /* 51*/ { LI(0x0000000100000000, 0x0000000100000000), 3, 2, LI(0x55555555AAAAAAAA, 0x55555555) },
        /* 52*/ { LI(0x0000000100000001, 0x0000000100000001), 3, 1, LI(0xAAAAAAAB00000000, 0x55555555) },
        /* 53*/ { LI(0x7C54A8E022961911, 0x92940F87B13D9529), 0xF09B387392497535, 0x3A5BBDA3D6EBF8D7, LI(0x9BF4CCB73D412892, 0) },
        /* 54*/ { LI(0x57BC33D2FAB596C9, 0x63F589EB9FB6C96), 0x90B9A0DB6A5F, 0x43555745A6D4, LI(0x957BE34B1882E2B, 0xB0D) },
        /* 55*/ { LI(0x1000000000000000, 0x1000000000000000), 0x10000000, 0, LI(0x100000000, 0x100000000) },
        /* 56*/ { LI(0x1000000000000001, 0x1000000000000001), 0x10000000, 1, LI(0x1100000000, 0x100000000) },
        /* 57*/ { LI(0x1000000000000001, 0x1000000000000001), 0x10000001, 0x1111, LI(0x110FFFFEEF0, 0xFFFFFFF0) },
        /* 58*/ { LI(0x99999999999999BF, 0x9999999999999999), 0x5555555567, 0x3331D64530, LI(0xCC6D6666667A2699, 0x1CCCCCC) },
        /* 59*/ { LI(0, 0xFFFFFFFFFFFFFFFF), 0x200000000, 0, LI(0xFFFFFFFF80000000, 0x7FFFFFFF) },
        /* 60*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 0x200000000, 0x1FFFFFFFF, LI(0xFFFFFFFFFFFFFFFF, 0x7FFFFFFF) },
        /* 61*/ { LI(0, 0xFFFFFFFFFFFFFFFF), 0x300000000, 0, LI(0x5555555500000000, 0x55555555) },
        /* 62*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 0x300000000, 0xFFFFFFFF, LI(0x5555555555555555, 0x55555555) },
        /* 63*/ { LI(0x0000000300000004, 0x0000000100000002), 0x200000003, 0x40000001, LI(0x8000000040000001, 0) },
        /* 64*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 0x2000000000000000, 0x1FFFFFFFFFFFFFFF, LI(0xFFFFFFFFFFFFFFFF, 7) },
        /* 65*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 0x3000000000000000, 0xFFFFFFFFFFFFFFF, LI(0x5555555555555555, 5) },
        /* 66*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 0xFFFFFFFFFFFFFFFF, 0, LI(1, 1) },
        /* 67*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFEFFFFFFFF), 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFEFFFFFFFF, LI(0xFFFFFFFF00000000, 0) },
        /* 68*/ { LI(0x00000F70677372AE, 0), 0x453AF5, 0, LI(0x391736, 0) }, // Divisor 0x453AF5 (4537077) used by RSS14
        /* 69*/ { LI(0x00000F70677372AF, 0), 0x453AF5, 1, LI(0x391736, 0) },
        /* 70*/ { LI(0x00000F70677372B0, 0), 0x453AF5, 2, LI(0x391736, 0) },
        /* 71*/ { LI(0x453AF4, 0), 0x453AF5, 0x453AF4, LI(0, 0) },
        /* 72*/ { LI(0x453AF5, 0), 0x453AF5, 0, LI(1, 0) },
        /* 73*/ { LI(0x453AF6, 0), 0x453AF5, 1, LI(1, 0) },
        /* 74*/ { { 0x453AF5 * 10 - 1, 0 }, 0x453AF5, 0x453AF4, LI(9, 0) },
        /* 75*/ { LI(0x000003A03166E0CE, 0), 0x1EB983, 0x1EB982, LI(0x1E35C4, 0) }, // Divisor 0x1EB983 (2013571) used by RSS_LTD
        /* 76*/ { LI(0x000003A03166E0CF, 0), 0x1EB983, 0, LI(0x1E35C5, 0) },
        /* 77*/ { LI(0x000003A03166E0D0, 0), 0x1EB983, 1, LI(0x1E35C5, 0) },
        /* 78*/ { LI(0x93BB793904CAFFFF, 0x13F50B74), 32, 0x1F, LI(0xA49DDBC9C82657FF, 0x9FA85B) }, // Divisor 32 used by MAILMARK
        /* 79*/ { LI(0x93BB793904CB0000, 0x13F50B74), 32, 0, LI(0xA49DDBC9C8265800, 0x9FA85B) },
        /* 80*/ { LI(0x93BB793904CB0001, 0x13F50B74), 32, 1, LI(0xA49DDBC9C8265800, 0x9FA85B) },
        /* 81*/ { LI(0x93BB793904CAFFFF, 0x13F50B74), 30, 0x1D, LI(0x8D752EB519C27FFF, 0xAA4D2E) }, // Divisor 30 used by MAILMARK
        /* 82*/ { LI(0x93BB793904CB0000, 0x13F50B74), 30, 0, LI(0x8D752EB519C28000, 0xAA4D2E) },
        /* 83*/ { LI(0x93BB793904CB0001, 0x13F50B74), 30, 1, LI(0x8D752EB519C28000, 0xAA4D2E) },
        /* 84*/ { LI(0x4ABC16A2E5C005FF, 0x16907B2A2), 636,  635, LI(0xD70F9761AA390E7F, 0x9151FD) }, // Divisor 636 used by ONECODE
        /* 85*/ { LI(0x4ABC16A2E5C00600, 0x16907B2A2), 636, 0, LI(0xD70F9761AA390E80, 0x9151FD) },
        /* 86*/ { LI(0x4ABC16A2E5C00601, 0x16907B2A2), 636, 1, LI(0xD70F9761AA390E80, 0x9151FD) },
        /* 87*/ { LI(0x4ABC16A2E5C00734, 0x16907B2A2), 1365, 1364, LI(0xD93B96FDAE65FA60, 0x43B5AC) }, // Divisor 1365 used by ONECODE
        /* 88*/ { LI(0x4ABC16A2E5C00735, 0x16907B2A2), 1365, 0, LI(0xD93B96FDAE65FA61, 0x43B5AC) },
        /* 89*/ { LI(0x4ABC16A2E5C00736, 0x16907B2A2), 1365, 1, LI(0xD93B96FDAE65FA61, 0x43B5AC) },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char t_dump[35];
    char expected_dump[35];

    testStart("test_div_u64");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        r = large_div_u64(&data[i].t, data[i].s);

        assert_equalu64(r, data[i].expected_r, "i:%d r %" LX_FMT "u (0x%" LX_FMT "X) != expected_r %" LX_FMT "u (0x%" LX_FMT "X)\n",
                        i, r, r, data[i].expected_r, data[i].expected_r);
        assert_equalu64(data[i].t.lo, data[i].expected.lo, "i:%d lo 0x%" LX_FMT "X (%s) != expected lo 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.lo, large_dump(&data[i].t, t_dump), data[i].expected.lo, large_dump(&data[i].expected, expected_dump));
        assert_equalu64(data[i].t.hi, data[i].expected.hi, "i:%d hi 0x%" LX_FMT "X (%s) != expected hi 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.hi, large_dump(&data[i].t, t_dump), data[i].expected.hi, large_dump(&data[i].expected, expected_dump));
    }

    testFinish();
}

static void test_unset_bit(int index) {

    struct item {
        large_int t;
        int s;
        large_int expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), 0, LI(0, 0) },
        /*  1*/ { LI(0, 0xFFFFFFFFFFFFFFFF), 0, LI(0, 0xFFFFFFFFFFFFFFFF) },
        /*  2*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 0, LI(0xFFFFFFFFFFFFFFFE, 0xFFFFFFFFFFFFFFFF) },
        /*  3*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 1, LI(0xFFFFFFFFFFFFFFFD, 0xFFFFFFFFFFFFFFFF) },
        /*  4*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 2, LI(0xFFFFFFFFFFFFFFFB, 0xFFFFFFFFFFFFFFFF) },
        /*  5*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 3, LI(0xFFFFFFFFFFFFFFF7, 0xFFFFFFFFFFFFFFFF) },
        /*  6*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 4, LI(0xFFFFFFFFFFFFFFEF, 0xFFFFFFFFFFFFFFFF) },
        /*  7*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 5, LI(0xFFFFFFFFFFFFFFDF, 0xFFFFFFFFFFFFFFFF) },
        /*  8*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 6, LI(0xFFFFFFFFFFFFFFBF, 0xFFFFFFFFFFFFFFFF) },
        /*  9*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 7, LI(0xFFFFFFFFFFFFFF7F, 0xFFFFFFFFFFFFFFFF) },
        /* 10*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 8, LI(0xFFFFFFFFFFFFFEFF, 0xFFFFFFFFFFFFFFFF) },
        /* 11*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 9, LI(0xFFFFFFFFFFFFFDFF, 0xFFFFFFFFFFFFFFFF) },
        /* 12*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 10, LI(0xFFFFFFFFFFFFFBFF, 0xFFFFFFFFFFFFFFFF) },
        /* 13*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 11, LI(0xFFFFFFFFFFFFF7FF, 0xFFFFFFFFFFFFFFFF) },
        /* 14*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 12, LI(0xFFFFFFFFFFFFEFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 15*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 13, LI(0xFFFFFFFFFFFFDFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 16*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 14, LI(0xFFFFFFFFFFFFBFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 17*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 15, LI(0xFFFFFFFFFFFF7FFF, 0xFFFFFFFFFFFFFFFF) },
        /* 18*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 16, LI(0xFFFFFFFFFFFEFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 19*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 17, LI(0xFFFFFFFFFFFDFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 20*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 18, LI(0xFFFFFFFFFFFBFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 21*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 19, LI(0xFFFFFFFFFFF7FFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 22*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 20, LI(0xFFFFFFFFFFEFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 23*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 21, LI(0xFFFFFFFFFFDFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 24*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 22, LI(0xFFFFFFFFFFBFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 25*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 23, LI(0xFFFFFFFFFF7FFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 26*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 24, LI(0xFFFFFFFFFEFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 27*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 25, LI(0xFFFFFFFFFDFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 28*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 26, LI(0xFFFFFFFFFBFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 29*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 27, LI(0xFFFFFFFFF7FFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 30*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 28, LI(0xFFFFFFFFEFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 31*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 29, LI(0xFFFFFFFFDFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 32*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 30, LI(0xFFFFFFFFBFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 33*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 31, LI(0xFFFFFFFF7FFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 34*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 32, LI(0xFFFFFFFEFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 35*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 33, LI(0xFFFFFFFDFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 36*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 34, LI(0xFFFFFFFBFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 37*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 35, LI(0xFFFFFFF7FFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 38*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 36, LI(0xFFFFFFEFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 39*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 37, LI(0xFFFFFFDFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 40*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 38, LI(0xFFFFFFBFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 41*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 39, LI(0xFFFFFF7FFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 42*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 40, LI(0xFFFFFEFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 43*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 41, LI(0xFFFFFDFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 44*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 42, LI(0xFFFFFBFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 45*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 43, LI(0xFFFFF7FFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 46*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 44, LI(0xFFFFEFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 47*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 45, LI(0xFFFFDFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 48*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 46, LI(0xFFFFBFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 49*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 47, LI(0xFFFF7FFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 50*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 48, LI(0xFFFEFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 51*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 49, LI(0xFFFDFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 52*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 50, LI(0xFFFBFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 53*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 51, LI(0xFFF7FFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 54*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 52, LI(0xFFEFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 55*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 53, LI(0xFFDFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 56*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 54, LI(0xFFBFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 57*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 55, LI(0xFF7FFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 58*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 56, LI(0xFEFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 59*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 57, LI(0xFDFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 60*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 58, LI(0xFBFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 61*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 59, LI(0xF7FFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 62*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 60, LI(0xEFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 63*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 61, LI(0xDFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 64*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 62, LI(0xBFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 65*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 63, LI(0x7FFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
        /* 66*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 64, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE) },
        /* 67*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 65, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFD) },
        /* 68*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 66, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFB) },
        /* 69*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 67, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFF7) },
        /* 70*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 68, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFEF) },
        /* 71*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 69, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFDF) },
        /* 72*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 70, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFBF) },
        /* 73*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 71, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF7F) },
        /* 74*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 72, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFEFF) },
        /* 75*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 73, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFDFF) },
        /* 76*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 74, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFBFF) },
        /* 77*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 75, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFF7FF) },
        /* 78*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 76, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFEFFF) },
        /* 79*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 77, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFDFFF) },
        /* 80*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 78, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFBFFF) },
        /* 81*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 79, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFF7FFF) },
        /* 82*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 80, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFEFFFF) },
        /* 83*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 81, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFDFFFF) },
        /* 84*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 82, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFBFFFF) },
        /* 85*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 83, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFF7FFFF) },
        /* 86*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 84, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFEFFFFF) },
        /* 87*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 85, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFDFFFFF) },
        /* 88*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 86, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFBFFFFF) },
        /* 89*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 87, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFF7FFFFF) },
        /* 90*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 88, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFEFFFFFF) },
        /* 91*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 89, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFDFFFFFF) },
        /* 92*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 90, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFBFFFFFF) },
        /* 93*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 91, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFF7FFFFFF) },
        /* 94*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 92, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFEFFFFFFF) },
        /* 95*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 93, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFDFFFFFFF) },
        /* 96*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 94, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFBFFFFFFF) },
        /* 97*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 95, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFF7FFFFFFF) },
        /* 98*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 96, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFEFFFFFFFF) },
        /* 99*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 97, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFDFFFFFFFF) },
        /*100*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 98, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFBFFFFFFFF) },
        /*101*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 99, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFF7FFFFFFFF) },
        /*102*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 100, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFEFFFFFFFFF) },
        /*103*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 101, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFDFFFFFFFFF) },
        /*104*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 102, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFBFFFFFFFFF) },
        /*105*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 103, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFF7FFFFFFFFF) },
        /*106*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 104, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFEFFFFFFFFFF) },
        /*107*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 105, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFDFFFFFFFFFF) },
        /*108*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 106, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFBFFFFFFFFFF) },
        /*109*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 107, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFF7FFFFFFFFFF) },
        /*110*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 108, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFEFFFFFFFFFFF) },
        /*111*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 109, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFDFFFFFFFFFFF) },
        /*112*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 110, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFBFFFFFFFFFFF) },
        /*113*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 111, LI(0xFFFFFFFFFFFFFFFF, 0xFFFF7FFFFFFFFFFF) },
        /*114*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 112, LI(0xFFFFFFFFFFFFFFFF, 0xFFFEFFFFFFFFFFFF) },
        /*115*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 113, LI(0xFFFFFFFFFFFFFFFF, 0xFFFDFFFFFFFFFFFF) },
        /*116*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 114, LI(0xFFFFFFFFFFFFFFFF, 0xFFFBFFFFFFFFFFFF) },
        /*117*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 115, LI(0xFFFFFFFFFFFFFFFF, 0xFFF7FFFFFFFFFFFF) },
        /*118*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 116, LI(0xFFFFFFFFFFFFFFFF, 0xFFEFFFFFFFFFFFFF) },
        /*119*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 117, LI(0xFFFFFFFFFFFFFFFF, 0xFFDFFFFFFFFFFFFF) },
        /*120*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 118, LI(0xFFFFFFFFFFFFFFFF, 0xFFBFFFFFFFFFFFFF) },
        /*121*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 119, LI(0xFFFFFFFFFFFFFFFF, 0xFF7FFFFFFFFFFFFF) },
        /*122*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 120, LI(0xFFFFFFFFFFFFFFFF, 0xFEFFFFFFFFFFFFFF) },
        /*123*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 121, LI(0xFFFFFFFFFFFFFFFF, 0xFDFFFFFFFFFFFFFF) },
        /*124*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 122, LI(0xFFFFFFFFFFFFFFFF, 0xFBFFFFFFFFFFFFFF) },
        /*125*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 123, LI(0xFFFFFFFFFFFFFFFF, 0xF7FFFFFFFFFFFFFF) },
        /*126*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 124, LI(0xFFFFFFFFFFFFFFFF, 0xEFFFFFFFFFFFFFFF) },
        /*127*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 125, LI(0xFFFFFFFFFFFFFFFF, 0xDFFFFFFFFFFFFFFF) },
        /*128*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 126, LI(0xFFFFFFFFFFFFFFFF, 0xBFFFFFFFFFFFFFFF) },
        /*129*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 127, LI(0xFFFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF) },
        /*130*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 128, LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF) },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char t_dump[35];
    char expected_dump[35];

    testStart("test_unset_bit");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        large_unset_bit(&data[i].t, data[i].s);

        assert_equalu64(data[i].t.lo, data[i].expected.lo, "i:%d lo 0x%" LX_FMT "X (%s) != expected lo 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.lo, large_dump(&data[i].t, t_dump), data[i].expected.lo, large_dump(&data[i].expected, expected_dump));
        assert_equalu64(data[i].t.hi, data[i].expected.hi, "i:%d hi 0x%" LX_FMT "X (%s) != expected hi 0x%" LX_FMT "X (%s)\n",
                        i, data[i].t.hi, large_dump(&data[i].t, t_dump), data[i].expected.hi, large_dump(&data[i].expected, expected_dump));
    }

    testFinish();
}

static void test_uint_array(int index) {

    struct item {
        large_int t;
        int size;
        int bits;
        unsigned int expected[130];
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), 0, 0, { 0 } },
        /*  1*/ { LI(1, 1), 16, 0, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1 } },
        /*  2*/ { LI(1, 1), 15, 0, { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1 } },
        /*  3*/ { LI(0x1122334455667788, 0x99AABBCCDDEEFF01), 16, 0, { 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 } },
        /*  4*/ { LI(0x1122334455667788, 0x99AABBCCDDEEFF01), 13, 0, { 0xCC, 0xDD, 0xEE, 0xFF, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 } },
        /*  5*/ { LI(0x1122334455667788, 0x99AABBCCDDEEFF01), 6, 0, { 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 } },
        /*  6*/ { LI(1, 1), 128, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
        /*  7*/ { LI(1, 1), 64, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
        /*  8*/ { LI(1, 1), 43, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
        /*  9*/ { LI(1, 1), 32, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, } },
        /* 10*/ { LI(1, 1), 26, 5, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
        /* 11*/ { LI(0xAAAAAAAAAAAAAAAA, 0), 13, 5, { 0xA, 0x15, 0xA, 0x15, 0xA, 0x15, 0xA, 0x15, 0xA, 0x15, 0xA, 0x15, 0xA } },
        /* 12*/ { LI(0x5555555555555555, 0), 13, 5, { 0x5, 0xA, 0x15, 0xA, 0x15, 0xA, 0x15, 0xA, 0x15, 0xA, 0x15, 0xA, 0x15 } },
        /* 13*/ { LI(0x1122334455667788, 0x99AABBCCDDEEFF01), 26, 5, { 0x4, 0x19, 0x15, 0xA, 0x1D, 0x1C, 0x19, 0x17, 0xF, 0xE, 0x1F, 0x1C, 0x0, 0x11, 0x2, 0x8, 0x11, 0x13, 0x8, 0x11, 0xA, 0x16, 0xC, 0x1D, 0x1C, 0x8 } },
        /* 14*/ { LI(1, 1), 22, 6, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
        /* 15*/ { LI(1, 1), 19, 7, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
        /* 16*/ { LI(1, 1), 15, 9, { 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 1 } },
        /* 17*/ { LI(1, 1), 13, 10, { 0, 0, 0, 0, 0, 0, 0x10, 0, 0, 0, 0, 0, 1 } },
        /* 18*/ { LI(1, 1), 12, 11, { 0, 0, 0, 0, 0, 0, 0x200, 0, 0, 0, 0, 1 } },
        /* 19*/ { LI(1, 1), 11, 12, { 0, 0, 0, 0, 0, 0x10, 0, 0, 0, 0, 1 } },
        /* 20*/ { LI(1, 1), 10, 13, { 0, 0, 0, 0, 0, 0x1000, 0, 0, 0, 1 } },
        /* 21*/ { LI(1, 1), 10, 14, { 0, 0, 0, 0, 0, 0x100, 0, 0, 0, 1 } },
        /* 22*/ { LI(1, 1), 9, 15, { 0, 0, 0, 0, 0x10, 0, 0, 0, 1 } },
        /* 23*/ { LI(1, 1), 8, 16, { 0, 0, 0, 1, 0, 0, 0, 1 } },
        /* 24*/ { LI(1, 1), 8, 17, { 0, 0, 0, 0, 0x2000, 0, 0, 1 } },
        /* 25*/ { LI(1, 1), 8, 18, { 0, 0, 0, 0, 0x400, 0, 0, 1 } },
        /* 26*/ { LI(1, 1), 7, 19, { 0, 0, 0, 0x80, 0, 0, 1 } },
        /* 27*/ { LI(1, 1), 5, 31, { 0, 0, 4, 0, 1 } },
        /* 28*/ { LI(1, 1), 4, 32, { 0, 1, 0, 1 } },
        /* 29*/ { LI(1, 1), 4, 33, { 0, 1, 0, 1 } }, // Bits > 32 ignored and treated as 32
        /* 30*/ { LI(0xF0F0F0F0F0F0F0F0, 0xE0F0F0F0F0F0F0F0), 129, 1, { 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 } }, // Leading zeroes
        /* 31*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), 130, 1, { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } }, // Leading zeroes
        /* 32*/ { LI(0xFFFFFFFFFFFFFFFF, 0xEFFFFFFFFFFFFFFF), 127, 1, { 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } }, // Truncated
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char t_dump[35];
    char uint_dump[130 * 17 + 1];
    char uint_expected_dump[130 * 17 + 1];
    char uchar_dump[130 * 3 + 1];
    char uchar_expected_dump[130 * 3 + 1];

    unsigned int uint_array[130];
    unsigned char uchar_array[130];
    unsigned char uchar_expected_array[130];

    testStart("test_uint_array");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        memset(uint_array, 0, sizeof(uint_array));

        large_uint_array(&data[i].t, uint_array, data[i].size, data[i].bits);

        assert_zero(memcmp(uint_array, data[i].expected, data[i].size * sizeof(unsigned int)), "i:%d %s uint memcmp != 0\n  actual: %s\nexpected: %s\n",
                        i, large_dump(&data[i].t, t_dump), testUtilUIntArrayDump(uint_array, data[i].size, uint_dump, sizeof(uint_dump)),
                        testUtilUIntArrayDump(data[i].expected, data[i].size, uint_expected_dump, sizeof(uint_expected_dump)));

        if (data[i].bits <= 8) {
            int j;
            memset(uchar_array, 0, sizeof(uchar_array));
            for (j = 0; j < data[i].size; j++) {
                uchar_expected_array[j] = data[i].expected[j];
            }

            large_uchar_array(&data[i].t, uchar_array, data[i].size, data[i].bits);

            assert_zero(memcmp(uchar_array, uchar_expected_array, data[i].size), "i:%d %s uchar memcmp != 0\n  actual: %s\nexpected: %s\n",
                            i, large_dump(&data[i].t, t_dump), testUtilUCharArrayDump(uchar_array, data[i].size, uchar_dump, sizeof(uchar_dump)),
                            testUtilUCharArrayDump(uchar_expected_array, data[i].size, uchar_expected_dump, sizeof(uchar_expected_dump)));
        }
    }

    testFinish();
}

static void test_dump(int index) {

    struct item {
        large_int t;
        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { LI(0, 0), "0x0" },
        /*  1*/ { LI(1, 0), "0x1" },
        /*  2*/ { LI(0x12, 0), "0x12" },
        /*  3*/ { LI(0x123, 0), "0x123" },
        /*  4*/ { LI(0x1234, 0), "0x1234" },
        /*  5*/ { LI(0x12345, 0), "0x12345" },
        /*  6*/ { LI(0x123456, 0), "0x123456" },
        /*  7*/ { LI(0x1234567, 0), "0x1234567" },
        /*  8*/ { LI(0x12345678, 0), "0x12345678" },
        /*  9*/ { LI(0x123456789, 0), "0x123456789" },
        /* 10*/ { LI(0x1234567890, 0), "0x1234567890" },
        /* 11*/ { LI(0x12345678901, 0), "0x12345678901" },
        /* 12*/ { LI(0x123456789012, 0), "0x123456789012" },
        /* 13*/ { LI(0x1234567890123, 0), "0x1234567890123" },
        /* 14*/ { LI(0x12345678901234, 0), "0x12345678901234" },
        /* 15*/ { LI(0x123456789012345, 0), "0x123456789012345" },
        /* 16*/ { LI(0x1234567890123456, 0), "0x1234567890123456" },
        /* 17*/ { LI(0x1234567890123456, 1), "0x11234567890123456" },
        /* 18*/ { LI(0x1234567890123456, 0x12), "0x121234567890123456" },
        /* 19*/ { LI(0x1234567890123456, 0x123), "0x1231234567890123456" },
        /* 20*/ { LI(0x1234567890123456, 0x1234), "0x12341234567890123456" },
        /* 21*/ { LI(0x1234567890123456, 0x12345), "0x123451234567890123456" },
        /* 22*/ { LI(0x1234567890123456, 0x123456), "0x1234561234567890123456" },
        /* 23*/ { LI(0x1234567890123456, 0x1234567), "0x12345671234567890123456" },
        /* 24*/ { LI(0x1234567890123456, 0x12345678), "0x123456781234567890123456" },
        /* 25*/ { LI(0x1234567890123456, 0x123456789), "0x1234567891234567890123456" },
        /* 26*/ { LI(0x1234567890123456, 0x1234567890), "0x12345678901234567890123456" },
        /* 27*/ { LI(0x1234567890123456, 0x12345678901), "0x123456789011234567890123456" },
        /* 28*/ { LI(0x1234567890123456, 0x123456789012), "0x1234567890121234567890123456" },
        /* 29*/ { LI(0x1234567890123456, 0x1234567890123), "0x12345678901231234567890123456" },
        /* 30*/ { LI(0x1234567890123456, 0x12345678901234), "0x123456789012341234567890123456" },
        /* 31*/ { LI(0x1234567890123456, 0x123456789012345), "0x1234567890123451234567890123456" },
        /* 32*/ { LI(0x1234567890123456, 0x1234567890123456), "0x12345678901234561234567890123456" },
        /* 33*/ { LI(0, 1), "0x10000000000000000" },
        /* 34*/ { LI(1, 1), "0x10000000000000001" },
        /* 35*/ { LI(0, 0xFFFFFFFF), "0xFFFFFFFF0000000000000000" },
        /* 36*/ { LI(0, 0xFFFFFFF000000000), "0xFFFFFFF0000000000000000000000000" },
        /* 37*/ { LI(0, 0xFFFFFFFFFFFFFFFF), "0xFFFFFFFFFFFFFFFF0000000000000000" },
        /* 38*/ { LI(0xFFFFFFFF, 0xFFFFFFFF), "0xFFFFFFFF00000000FFFFFFFF" },
        /* 39*/ { LI(0xFFFFFFFF, 0xFFFFFFFF00000000), "0xFFFFFFFF0000000000000000FFFFFFFF" },
        /* 40*/ { LI(0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF), "0xFFFFFFFFFFFFFFFF00000000FFFFFFFF" },
        /* 41*/ { LI(0xFFFFFFFF00000000, 0xFFFFFFFF), "0xFFFFFFFFFFFFFFFF00000000" },
        /* 42*/ { LI(0xFFFFFFFF00000000, 0xFFFFFFFF00000000), "0xFFFFFFFF00000000FFFFFFFF00000000" },
        /* 43*/ { LI(0xFFFFFFFF00000000, 0xFFFFFFFFFFFFFFFF), "0xFFFFFFFFFFFFFFFFFFFFFFFF00000000" },
        /* 44*/ { LI(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF), "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    char dump[35];

    testStart("test_dump");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        memset(dump, 0, sizeof(dump));

        large_dump(&data[i].t, dump);

        assert_zero(strcmp(dump, data[i].expected), "i:%d { %" LX_FMT "X, %" LX_FMT "X } strcmp(%s, %s) != 0\n",
                        i, data[i].t.lo, data[i].t.hi, dump, data[i].expected);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_clz_u64", test_clz_u64, 1, 0, 0 },
        { "test_load", test_load, 1, 0, 0 },
        { "test_load_str_u64", test_load_str_u64, 1, 0, 0 },
        { "test_add_u64", test_add_u64, 1, 0, 0 },
        { "test_sub_u64", test_sub_u64, 1, 0, 0 },
        { "test_mul_u64", test_mul_u64, 1, 0, 0 },
        { "test_div_u64", test_div_u64, 1, 0, 0 },
        { "test_unset_bit", test_unset_bit, 1, 0, 0 },
        { "test_uint_array", test_uint_array, 1, 0, 0 },
        { "test_dump", test_dump, 1, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
