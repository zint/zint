/*
    libzint - the open source barcode library
    Copyright (C) 2019-2023 Robin Stuart <rstuart114@gmail.com>

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
/* SPDX-License-Identifier: BSD-3-Clause */

#include "testcommon.h"
#include "test_gb2312_tab.h"
#include "../eci.h"
/* For local "private" testing using previous libiconv adaptation, not included for licensing reasons */
#if 0
#define TEST_JUST_SAY_GNO
#endif
#ifdef TEST_JUST_SAY_GNO
#include "../just_say_gno/gb2312_gnu.c"
#endif

INTERNAL int u_gb2312_int_test(const unsigned int u, unsigned int *d);

/* As control convert to GB 2312 using simple table generated from unicode.org GB2312.TXT plus simple processing */
/* GB2312.TXT no longer on unicode.org site but available from https://haible.de/bruno/charsets/conversion-tables/GB2312.html */
static int u_gb2312_int2(unsigned int u, unsigned int *d) {
    int tab_length, start_i, end_i;
    int i;

    if (u < 0x80) {
        *d = (unsigned char) u;
        return 1;
    }
    /* Shortcut */
    if ((u > 0x0451 && u < 0x2015) || (u > 0x3229 && u < 0x4E00) || (u > 0x9FA0 && u < 0xFF01) || u > 0xFFE5) {
        return 0;
    }
    tab_length = ARRAY_SIZE(test_gb2312_tab);
    start_i = test_gb2312_tab_ind[u >> 10];
    end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    for (i = start_i; i < end_i; i += 2) {
        if (test_gb2312_tab[i + 1] == u) {
            *d = test_gb2312_tab[i] + 0x8080; /* Table in GB 2312 not EUC-CN */
            return 2;
        }
    }
    return 0;
}

#include <time.h>

#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)
#define TEST_PERF_RATIO(a1, a2) (a2 ? TEST_PERF_TIME(a1) / TEST_PERF_TIME(a2) : 0)

#ifdef TEST_JUST_SAY_GNO
#define TEST_INT_PERF_ITERATIONS    250
#endif

static void test_u_gb2312_int(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    int ret, ret2;
    unsigned int val, val2;
    unsigned int i;

#ifdef TEST_JUST_SAY_GNO
    int j;
    clock_t start;
    clock_t total = 0, total_gno = 0;
#else
    (void)debug;
#endif

    testStart("test_u_gb2312_int");

#ifdef TEST_JUST_SAY_GNO
    if ((debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        printf("test_u_gb2312_int perf iterations: %d\n", TEST_INT_PERF_ITERATIONS);
    }
#endif

    for (i = 0; i < 0xFFFE; i++) {
        if (i >= 0xD800 && i <= 0xDFFF) { /* UTF-16 surrogates */
            continue;
        }
        if (testContinue(p_ctx, i)) continue;
        val = val2 = 0;
        ret = u_gb2312_int_test(i, &val);
        ret2 = u_gb2312_int2(i, &val2);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
        }
#ifdef TEST_JUST_SAY_GNO
        /* `gb2312_wctomb_zint()` doesn't handle ASCII; and ignore duplicate mappings, no longer done */
        if (i >= 0x80 && i != 0xB7 && i != 0x2014) {
            if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
                val2 = 0;
                ret2 = gb2312_wctomb_zint(&val2, i);
            } else {
                for (j = 0; j < TEST_INT_PERF_ITERATIONS; j++) {
                    val = val2 = 0;

                    start = clock();
                    ret = u_gb2312_int_test(i, &val);
                    total += clock() - start;

                    start = clock();
                    ret2 = gb2312_wctomb_zint(&val2, i);
                    total_gno += clock() - start;
                }
            }

            assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
            if (ret2) {
                assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
            }
        }
#endif
    }

#ifdef TEST_JUST_SAY_GNO
    if ((debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        printf("test_u_gb2312_int perf totals: new % 8gms, gno % 8gms ratio %g\n",
                TEST_PERF_TIME(total), TEST_PERF_TIME(total_gno), TEST_PERF_RATIO(total, total_gno));
    }
#endif

    testFinish();
}

static void test_gb2312_utf8(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[20];
        char *comment;
    };
    /*
       é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 2312 0xA8A6, UTF-8 C3A9
       β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in GB 2312 0xA6C2, UTF-8 CEB2
       ¤ U+00A4 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 2312 0xA1E8, UTF-8 C2A4
       ¥ U+00A5 in ISO 8859-1 0xA5, not in GB 2312, UTF-8 C2A5
       ・ U+30FB katakana middle dot, not in any ISO or Win page, in GB 2312 "GB2312.TXT" 0xA1A4, duplicate of mapping of U+00B7, UTF-8 E383BB
       · U+00B7 middle dot in ISO 8859-1 0xB7, in GB 2312 "GB 18030 subset" 0xA1A4, duplicate of mapping of U+30FB, UTF-8 C2B7
       ― U+2015 horizontal bar in ISO 8859-7 Greek and ISO 8859-10 Nordic, not in any Win page, in GB 2312 "GB2312.TXT" 0xA1AA, duplicate of mapping of U+2014, UTF-8 E28095
       — U+2014 em dash, not in any ISO, in Win 1250 and other Win, in GB 2312 "GB 18030 subset" 0xA1AA, duplicate of mapping of U+2015, UTF-8 E28094
    */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "é", -1, 0, 1, { 0xA8A6 }, "" },
        /*  1*/ { "β", -1, 0, 1, { 0xA6C2 }, "" },
        /*  2*/ { "¤", -1, 0, 1, { 0xA1E8 }, "" },
        /*  3*/ { "¥", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "" },
        /*  4*/ { "・", -1, 0, 1, { 0xA1A4 }, "GB2312.TXT mapping" },
        /*  5*/ { "·", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "No longer does GB 18030 subset mapping" },
        /*  6*/ { "―", -1, 0, 1, { 0xA1AA }, "GB2312.TXT mapping" },
        /*  7*/ { "—", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "No longer does GB 18030 subset mapping" },
        /*  8*/ { "aβc・―é―Z", -1, 0, 8, { 'a', 0xA6C2, 'c', 0xA1A4, 0xA1AA, 0xA8A6, 0xA1AA, 'Z' }, "" },
        /*  9*/ { "aβc・·—é—Z", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "No longer does GB 18030 mappings" },
        /* 10*/ { "\200", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Invalid UTF-8" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};
    unsigned int gbdata[20];

    testStart("test_gb2312_utf8");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = gb2312_utf8(&symbol, (unsigned char *) data[i].data, &ret_length, gbdata);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < (int) ret_length; j++) {
                assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] %04X != %04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
            }
        }
    }

    testFinish();
}

static void test_gb2312_utf8_to_eci(const testCtx *const p_ctx) {

    struct item {
        int eci;
        int full_multibyte;
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[20];
        char *comment;
    };
    /*
       é U+00E9 in ISO 8859-1 0xE9, Win 1250 plus other Win, in GRIDMATRIX Chinese mode first byte range 0xA1..A9, 0xB0..F7
       β U+03B2 in ISO 8859-7 Greek 0xE2 (but not other ISO 8859 or Win page)
       ¥ U+00A5 in ISO 8859-1 0xA5, in first byte range 0xA1..A9, 0xB0..F7
       NBSP U+00A0 in ISO 8859-1 0xA0, outside first byte and second byte range 0xA1..FE, UTF-8 C2A0 (\302\240)
       ¡ U+00A1 in ISO 8859-1 0xA1, in first byte range
       © U+00A9 in ISO 8859-1 0xA9, in first byte range
       ª U+00AA in ISO 8859-1 0xAA, outside first byte range
       ¯ U+00AF in ISO 8859-1 0xAF, outside first byte range
       ° U+00B0 in ISO 8859-1 0xB0, in first byte range
       ÷ U+00F7 in ISO 8859-1 0xF7, in first byte range
       ø U+00F8 in ISO 8859-1 0xF8, outside first byte range
       ÿ U+00FF in ISO 8859-1 0xFF, outside first byte and second byte range
    */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 3, 0, "é", -1, 0, 1, { 0xE9 }, "Not full multibyte" },
        /*  1*/ { 3, 1, "é", -1, 0, 1, { 0xE9 }, "First byte in range but only one byte" },
        /*  2*/ { 3, 0, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Not in ECI 3 (ISO 8859-1)" },
        /*  3*/ { 3, 1, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Not in ECI 3 (ISO 8859-1)" },
        /*  4*/ { 9, 0, "β", -1, 0, 1, { 0xE2 }, "In ECI 9 (ISO 8859-7)" },
        /*  5*/ { 9, 1, "β", -1, 0, 1, { 0xE2 }, "In ECI 9 (ISO 8859-7)" },
        /*  6*/ { 3, 0, "¥", -1, 0, 1, { 0xA5 }, "Not full multibyte" },
        /*  7*/ { 3, 1, "¥", -1, 0, 1, { 0xA5 }, "First byte in range but only one byte" },
        /*  8*/ { 3, 0, "¡é", -1, 0, 2, { 0xA1, 0xE9 }, "Not full multibyte" },
        /*  9*/ { 3, 1, "¡é", -1, 0, 1, { 0xA1E9 }, "In GRIDMATRIX Chinese mode range" },
        /* 10*/ { 3, 0, "¡\302\240", -1, 0, 2, { 0xA1, 0xA0 }, "Not full multibyte" },
        /* 11*/ { 3, 1, "¡\302\240", -1, 0, 2, { 0xA1, 0xA0 }, "First byte in range but not second" },
        /* 12*/ { 3, 0, "©é", -1, 0, 2, { 0xA9, 0xE9 }, "Not full multibyte" },
        /* 13*/ { 3, 1, "©é", -1, 0, 1, { 0xA9E9 }, "In GRIDMATRIX Chinese mode range" },
        /* 14*/ { 3, 0, "©ÿ", -1, 0, 2, { 0xA9, 0xFF }, "Not full multibyte" },
        /* 15*/ { 3, 1, "©ÿ", -1, 0, 2, { 0xA9, 0xFF }, "First byte in range but not second" },
        /* 16*/ { 3, 0, "éaéé©ªª©¯é°°é÷éø", -1, 0, 16, { 0xE9, 0x61, 0xE9, 0xE9, 0xA9, 0xAA, 0xAA, 0xA9, 0xAF, 0xE9, 0xB0, 0xB0, 0xE9, 0xF7, 0xE9, 0xF8 }, "" },
        /* 17*/ { 3, 1, "éaéé©ªª©¯é°°é÷éø", -1, 0, 10, { 0xE9, 0x61, 0xE9E9, 0xA9AA, 0xAA, 0xA9AF, 0xE9B0, 0xB0E9, 0xF7E9, 0xF8 }, "" },
        /* 18*/ { 20, 0, "\\\\", -1, 0, 4, { 0x81, 0x5F, 0x81, 0x5F }, "Shift JIS reverse solidus (backslash) mapping from ASCII to double byte" },
        /* 19*/ { 20, 1, "\\\\", -1, 0, 4, { 0x81, 0x5F, 0x81, 0x5F }, "Shift JIS outside GB 2312 Hanzi mode range" },
        /* 20*/ { 20, 0, "爍", -1, 0, 2, { 0xE0, 0xA1 }, "Shift JIS U+720D" },
        /* 21*/ { 20, 1, "爍", -1, 0, 1, { 0xE0A1 }, "Shift JIS in GB 2312 Hanzi mode range" },
        /* 22*/ { 25, 0, "12", -1, 0, 4, { 0x00, 0x31, 0x00, 0x32 }, "UCS-2BE ASCII" },
        /* 23*/ { 25, 0, "", -1, 0, 4, { 0x00, 0x81, 0x00, 0x81 }, "UCS-2BE U+0081" },
        /* 24*/ { 25, 1, "", -1, 0, 4, { 0x00, 0x81, 0x00, 0x81 }, "UCS-2BE outside GB 2312 Hanzi mode range" },
        /* 25*/ { 25, 0, "ꆩꆩ", -1, 0, 4, { 0xA1, 0xA9, 0xA1, 0xA9 }, "UCS-2BE U+A1A9" },
        /* 26*/ { 25, 1, "ꆩꆩ", -1, 0, 2, { 0xA1A9, 0xA1A9 }, "UCS-2BE in GB 2312 Hanzi mode range" },
        /* 27*/ { 25, 0, "膀膀", -1, 0, 4, { 0x81, 0x80, 0x81, 0x80 }, "UCS-2BE U+8180" },
        /* 28*/ { 25, 1, "膀膀", -1, 0, 4, { 0x81, 0x80, 0x81, 0x80 }, "UCS-2BE outside GB 2312 Hanzi mode range (but in GB 18030 range)" },
        /* 29*/ { 28, 0, "¢¢", -1, 0, 4, { 0xA2, 0x46, 0xA2, 0x46 }, "Big5 U+00A2" },
        /* 30*/ { 28, 1, "¢¢", -1, 0, 4, { 0xA2, 0x46, 0xA2, 0x46 }, "Big5 outside GB 2312 Hanzi mode range (but in GB 18030 range)" },
        /* 31*/ { 28, 0, "陛", -1, 0, 2, { 0xB0, 0xA1 }, "Big5 U+965B" },
        /* 32*/ { 28, 1, "陛", -1, 0, 1, { 0xB0A1 }, "Big5 in GB 2312 Hanzi mode range" },
        /* 33*/ { 29, 0, "¨¨", -1, 0, 2, { 0xA1A7, 0xA1A7 }, "GB 2312 U+00A8" },
        /* 34*/ { 29, 1, "¨¨", -1, 0, 2, { 0xA1A7, 0xA1A7 }, "GB 2312" },
        /* 35*/ { 29, 0, "崂", -1, 0, 1, { 0xE1C0 }, "GB 2312 U+5D02" },
        /* 36*/ { 29, 1, "崂", -1, 0, 1, { 0xE1C0 }, "GB 2312" },
        /* 37*/ { 29, 0, "・", -1, 0, 1, { 0xA1A4 }, "GB 2312 U+30FB" },
        /* 38*/ { 29, 1, "・", -1, 0, 1, { 0xA1A4 }, "GB 2312" },
        /* 39*/ { 29, 0, "釦", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "GB 18030 U+91E6 not in GB 2312" },
        /* 40*/ { 30, 0, "¡¡", -1, 0, 4, { 0x22 + 0x80, 0x2E + 0x80, 0x22 + 0x80, 0x2E + 0x80 }, "EUC-KR U+00A1 (0xA2AE)" },
        /* 41*/ { 30, 1, "¡¡", -1, 0, 2, { 0x222E + 0x8080, 0x222E + 0x8080 }, "EUC-KR 0xA2AE in GB 2312 Hanzi mode range" },
        /* 42*/ { 30, 0, "詰", -1, 0, 2, { 0x7D + 0x80, 0x7E + 0x80 }, "EUC-KR U+8A70 (0xFDFE)" },
        /* 43*/ { 30, 1, "詰", -1, 0, 2, { 0x7D + 0x80, 0x7E + 0x80 }, "EUC-KR 0xFDFE > 0xF7FE so not in GB 2312 Hanzi mode range" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    unsigned int gbdata[20];

    testStart("test_gb2312_utf8_to_eci");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = gb2312_utf8_to_eci(data[i].eci, (unsigned char *) data[i].data, &ret_length, gbdata, data[i].full_multibyte);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < (int) ret_length; j++) {
                assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] 0x%04X != 0x%04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
            }
        }
    }

    testFinish();
}

INTERNAL void gb2312_cpy_test(const unsigned char source[], int *p_length, unsigned int *ddata,
                const int full_multibyte);

static void test_gb2312_cpy(const testCtx *const p_ctx) {

    struct item {
        int full_multibyte;
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[20];
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 0, "\351", -1, 0, 1, { 0xE9 }, "Not full multibyte" },
        /*  1*/ { 1, "\351", -1, 0, 1, { 0xE9 }, "In GRIDMATRIX Chinese mode first-byte range but only one byte" },
        /*  2*/ { 0, "\351\241", -1, 0, 2, { 0xE9, 0xA1 }, "Not full multibyte" },
        /*  3*/ { 1, "\351\241", -1, 0, 1, { 0xE9A1 }, "In GRIDMATRIX Chinese range" },
        /*  4*/ { 0, "\241", -1, 0, 1, { 0xA1 }, "Not full multibyte" },
        /*  5*/ { 1, "\241", -1, 0, 1, { 0xA1 }, "In first-byte range but only one byte" },
        /*  6*/ { 0, "\241\241", -1, 0, 2, { 0xA1, 0xA1 }, "Not full multibyte" },
        /*  7*/ { 1, "\241\241", -1, 0, 1, { 0xA1A1 }, "In range" },
        /*  8*/ { 0, "\241\240\241\376\367\376\367\377", -1, 0, 8, { 0xA1, 0xA0, 0xA1, 0xFE, 0xF7, 0xFE, 0xF7, 0xFF }, "" },
        /*  9*/ { 1, "\241\240\241\376\367\376\367\377", -1, 0, 6, { 0xA1, 0xA0, 0xA1FE, 0xF7FE, 0xF7, 0xFF }, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length;

    unsigned int gbdata[20];

    testStart("test_gb2312_cpy");

    for (i = 0; i < data_size; i++) {
        int ret_length;
        int j;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        gb2312_cpy_test((unsigned char *) data[i].data, &ret_length, gbdata, data[i].full_multibyte);
        assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
        for (j = 0; j < (int) ret_length; j++) {
            assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] %04X != %04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
        }
    }

    testFinish();
}

#define TEST_PERF_ITER_MILLES   100
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)

/* Not a real test, just performance indicator */
static void test_perf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        int ret;

        char *comment;
    };
    struct item data[] = {
        /*  0*/ { "1234567890", 0, "10 numerics" },
        /*  1*/ { "条码北京條碼པེ་ཅིང།バーコード바코드", 0, "Small various code pages" },
        /*  2*/ { "Summer Palace Ticket for 6 June 2015 13:00;2015年6月6日夜01時00分PM頤和園のチケット;2015년6월6일13시오후여름궁전티켓.2015年6月6号下午13:00的颐和园门票;", 0, "Small mixed ASCII/Hanzi" },
        /*  3*/ { "汉信码标准\015\012中国物品编码中心\015\012北京网路畅想科技发展有限公司\015\012张成海、赵楠、黄燕滨、罗秋科、王毅、张铎、王越\015\012施煜、边峥、修兴强\015\012汉信码标准\015\012中国物品编码中心\015\012北京网路畅想科技发展有限公司", 0, "Bigger mixed" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};
    int ret_length;
#ifdef TEST_JUST_SAY_GNO
    int ret_length2;
#endif
    unsigned int ddata[8192];
    unsigned char dest[8192];
    int ret2 = 0;
#ifdef TEST_JUST_SAY_GNO
    unsigned int ddata2[8192];
    unsigned char dest2[8192];
#endif

    clock_t start;
    clock_t total = 0, total_gno = 0, total_eci = 0, total_eci_gno = 0;
    clock_t diff, diff_gno, diff_eci, diff_eci_gno;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        length = (int) strlen(data[i].data);

        diff = diff_gno = diff_eci = diff_eci_gno = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            ret_length = length;

            start = clock();
            ret = gb2312_utf8(&symbol, (unsigned char *) data[i].data, &ret_length, ddata);
            diff += clock() - start;

#ifdef TEST_JUST_SAY_GNO
            ret_length2 = length;
            start = clock();
            ret2 = gb2312_utf8_wctomb(&symbol, (unsigned char *) data[i].data, &ret_length2, ddata2);
            diff_gno += clock() - start;
#endif

            ret_length = length;

            start = clock();
            (void)utf8_to_eci(29, (unsigned char *) data[i].data, dest, &ret_length);
            diff_eci += clock() - start;

#ifdef TEST_JUST_SAY_GNO
            ret_length2 = length;
            start = clock();
            (void)utf8_to_eci_wctomb(29, (unsigned char *) data[i].data, dest2, &ret_length2);
            diff_eci_gno += clock() - start;
#endif
        }
        assert_equal(ret, ret2, "i:%d ret %d != ret2 %d\n", (int) i, ret, ret2);

        printf("%*s: new % 8gms, gno % 8gms ratio % 9g | eci % 8gms, gno % 8gms ratio %g\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff), TEST_PERF_TIME(diff_gno), TEST_PERF_RATIO(diff, diff_gno),
                TEST_PERF_TIME(diff_eci), TEST_PERF_TIME(diff_eci_gno), TEST_PERF_RATIO(diff_eci, diff_eci_gno));

        total += diff;
        total_gno += diff_gno;
        total_eci += diff_eci;
        total_eci_gno += diff_eci_gno;
    }
    if (p_ctx->index == -1) {
        printf("%*s: new % 8gms, gno % 8gms ratio % 9g | eci % 8gms, gno % 8gms ratio %g\n", comment_max, "totals",
                TEST_PERF_TIME(total), TEST_PERF_TIME(total_gno), TEST_PERF_RATIO(total, total_gno),
                TEST_PERF_TIME(total_eci), TEST_PERF_TIME(total_eci_gno), TEST_PERF_RATIO(total_eci, total_eci_gno));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_u_gb2312_int", test_u_gb2312_int },
        { "test_gb2312_utf8", test_gb2312_utf8 },
        { "test_gb2312_utf8_to_eci", test_gb2312_utf8_to_eci },
        { "test_gb2312_cpy", test_gb2312_cpy },
        { "test_perf", test_perf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
