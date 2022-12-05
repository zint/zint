/*
    libzint - the open source barcode library
    Copyright (C) 2021-2022 Robin Stuart <rstuart114@gmail.com>

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
#include "test_big5_tab.h"
/* For local "private" testing using previous libiconv adaptation, not included for licensing reasons */
#if 0
#define TEST_JUST_SAY_GNO
#endif
#ifdef TEST_JUST_SAY_GNO
#include "../just_say_gno/big5_gnu.h"
#endif

INTERNAL int u_big5_test(const unsigned int u, unsigned char *dest);

/* Version of `u_big5()` taking unsigned int destination for backward-compatible testing */
static int u_big5_int(unsigned int u, unsigned int *d) {
    unsigned char dest[2];
    int ret = u_big5_test(u, dest);
    if (ret) {
        *d = ret == 1 ? dest[0] : ((dest[0] << 8) | dest[1]);
    }
    return ret;
}

/* As control convert to Big5 using simple table generated from
   https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/OTHER/BIG5.TXT plus simple processing */
static int u_big5_int2(unsigned int u, unsigned int *dest) {
    int tab_length = ARRAY_SIZE(test_big5_tab);
    int start_i = test_big5_tab_ind[u >> 10];
    int end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    int i;
    if (u < 0x80) {
        *dest = u;
        return 1;
    }
    for (i = start_i; i < end_i; i += 2) {
        if (test_big5_tab[i + 1] == u) {
            *dest = test_big5_tab[i];
            return *dest > 0xFF ? 2 : 1;
        }
    }
    return 0;
}

#include <time.h>

#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)
#define TEST_PERF_RATIO(a1, a2) (a2 ? TEST_PERF_TIME(a1) / TEST_PERF_TIME(a2) : 0)

#ifdef TEST_JUST_SAY_GNO
#define TEST_INT_PERF_ITERATIONS    100
#endif

static void test_u_big5_int(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    unsigned int i;
    int ret, ret2;
    unsigned int val, val2;

#ifdef TEST_JUST_SAY_GNO
    int j;
    clock_t start;
    clock_t total = 0, total_gno = 0;
#else
    (void)debug;
#endif

    testStart("test_u_big5_int");

#ifdef TEST_JUST_SAY_GNO
    if ((debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        printf("test_u_big5_int perf iterations: %d\n", TEST_INT_PERF_ITERATIONS);
    }
#endif

    for (i = 0; i < 0xFFFE; i++) {
        if (i >= 0xD800 && i < 0xE000) { /* UTF-16 surrogates */
            continue;
        }
        if (testContinue(p_ctx, i)) continue;
        val = val2 = 0;
        ret = u_big5_int(i, &val);
        ret2 = u_big5_int2(i, &val2);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
        }
#ifdef TEST_JUST_SAY_GNO
        if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
            val2 = 0;
            ret2 = big5_wctomb_zint(&val2, i);
        } else {
            for (j = 0; j < TEST_INT_PERF_ITERATIONS; j++) {
                val = val2 = 0;

                start = clock();
                ret = u_big5_int(i, &val);
                total += clock() - start;

                start = clock();
                ret2 = big5_wctomb_zint(&val2, i);
                total_gno += clock() - start;
            }
        }

        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
        }
#endif
    }

#ifdef TEST_JUST_SAY_GNO
    if ((debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        printf("test_u_big5_int perf totals: new % 8gms, gno % 8gms ratio %g\n",
                TEST_PERF_TIME(total), TEST_PERF_TIME(total_gno), TEST_PERF_RATIO(total, total_gno));
    }
#endif

    testFinish();
}

/* Convert UTF-8 string to Big5 and place in array of ints */
static int big5_utf8(struct zint_symbol *symbol, const unsigned char source[], int *p_length,
                unsigned int *b5data) {
    int error_number;
    unsigned int i, length;
    unsigned int *utfdata = (unsigned int *) z_alloca(sizeof(unsigned int) * (*p_length + 1));

    error_number = utf8_to_unicode(symbol, source, utfdata, p_length, 0 /*disallow_4byte*/);
    if (error_number != 0) {
        return error_number;
    }

    for (i = 0, length = *p_length; i < length; i++) {
        if (!u_big5_int(utfdata[i], b5data + i)) {
            strcpy(symbol->errtxt, "800: Invalid character in input data");
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    return 0;
}

static void test_big5_utf8(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_b5data[20];
        char *comment;
    };
    /*
       ＿ U+FF3F fullwidth low line, not in ISO/Win, in Big5 0xA1C4, UTF-8 EFBCBF
       ╴ U+2574 drawings box light left, not in ISO/Win, not in original Big5 but in "Big5-2003" as 0xA15A, UTF-8 E295B4
    */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "＿", -1, 0, 1, { 0xA1C4 }, "" },
        /*  1*/ { "╴", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};
    unsigned int b5data[20];

    testStart("test_big5_utf8");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = big5_utf8(&symbol, (unsigned char *) data[i].data, &ret_length, b5data);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < ret_length; j++) {
                assert_equal(b5data[j], data[i].expected_b5data[j], "i:%d b5data[%d] %04X != %04X\n", i, j, b5data[j], data[i].expected_b5data[j]);
            }
        }
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_u_big5_int", test_u_big5_int },
        { "test_big5_utf8", test_big5_utf8 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
