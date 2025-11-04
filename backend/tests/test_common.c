/*
    libzint - the open source barcode library
    Copyright (C) 2019-2025 Robin Stuart <rstuart114@gmail.com>

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

/* Original */
static int is_sane_orig(const char test_string[], const unsigned char source[], const int length) {
    int i, j, lt = (int) strlen(test_string);

    for (i = 0; i < length; i++) {
        unsigned int latch = 0;
        for (j = 0; j < lt; j++) {
            if (source[i] == test_string[j]) {
                latch = 1;
                break;
            }
        }
        if (!(latch)) {
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    return 0;
}

static void test_to_int(const testCtx *const p_ctx) {

    struct item {
        const char *data;
        int length;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", -1, 0 },
        /*  1*/ { "1234", -1, 1234 },
        /*  2*/ { "-1234", -1, -1 },
        /*  3*/ { "A123A", -1, -1 },
        /*  4*/ { " ", -1, -1 },
        /*  5*/ { "999999999", -1, 999999999 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = z_to_int(ZCUCP(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_to_upper(const testCtx *const p_ctx) {

    struct item {
        const char *data;
        int length;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", -1, "" },
        /*  1*/ { "abcefghijklmnopqrstuvwxyz", -1, "ABCEFGHIJKLMNOPQRSTUVWXYZ" },
        /*  2*/ { ".a[b`cA~B\177C;\200", -1, ".A[B`CA~B\177C;\200" },
        /*  3*/ { "é", -1, "é" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length;

    unsigned char buf[512];

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        buf[0] = '\0';
        memcpy(buf, data[i].data, length);
        buf[length] = '\0';

        z_to_upper(buf, length);
        assert_zero(strcmp((const char *) buf, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                    i, buf, data[i].expected);
    }

    testFinish();
}

static void test_chr_cnt(const testCtx *const p_ctx) {

    struct item {
        const char *data;
        int length;
        char c;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", -1, 'a', 0 },
        /*  1*/ { "BDAaED", -1, 'a', 1 },
        /*  1*/ { "aBDAaaaEaDa", -1, 'a', 6 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = z_chr_cnt(ZCUCP(data[i].data), length, data[i].c);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_is_chr(const testCtx *const p_ctx) {

    struct item {
        int flg;
        int c;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { NEON_F, '1', 1 },
        /*  1*/ { NEON_F, 'a', 0 },
        /*  2*/ { NEON_F, 0xFF, 0 },
        /*  3*/ { NEON_F, 0xFFF, 0 },
        /*  4*/ { NEON_F, 0xFFFF, 0 },
        /*  5*/ { IS_UPR_F, '1', 0 },
        /*  6*/ { IS_UPR_F, 'a', 0 },
        /*  7*/ { IS_UPR_F, 'A', 1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        ret = z_is_chr(data[i].flg, data[i].c);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_not_sane(const testCtx *const p_ctx) {

    struct item {
        unsigned int flg;
        const char *data;
        int length;
        int ret;

        const char *orig_test;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { IS_SPC_F, " ", -1, 0, " " },
        /*  1*/ { IS_SPC_F, "\000", 0, 0, " " },
        /*  2*/ { IS_HSH_F, "#", -1, 0, "#" },
        /*  3*/ { IS_HSH_F, " ", -1, 1, "#" },
        /*  4*/ { IS_AST_F, "*", -1, 0, "*" },
        /*  5*/ { IS_AST_F, " ", -1, 1, "*" },
        /*  6*/ { IS_PLS_F, "+", -1, 0, "+" },
        /*  7*/ { IS_PLS_F, " ", -1, 1, "+" },
        /*  8*/ { IS_MNS_F, "-", -1, 0, "-" },
        /*  9*/ { IS_MNS_F, " ", -1, 1, "-" },
        /* 10*/ { IS_NUM_F, "0123456789", -1, 0, "0123456789" }, /* NEON */
        /* 11*/ { IS_NUM_F, "0123456789 ", -1, 11, "0123456789" },
        /* 12*/ { IS_NUM_F, "012345678A9", -1, 10, "0123456789" },
        /* 13*/ { IS_UPO_F, "GHIJKLMNOPQRSTUVWYZ", -1, 0, "GHIJKLMNOPQRSTUVWYZ" },
        /* 14*/ { IS_UPO_F, "FGHIJKLMNOPQRSTUVWYZ", -1, 1, "GHIJKLMNOPQRSTUVWYZ" },
        /* 15*/ { IS_LWO_F, "ghijklmnopqrstuvwyz", -1, 0, "ghijklmnopqrstuvwyz" },
        /* 16*/ { IS_LWO_F, "fghijklmnopqrstuvwyz", -1, 1, "ghijklmnopqrstuvwyz" },
        /* 17*/ { IS_UHX_F, "ABCDEF", -1, 0, "ABCDEF" },
        /* 18*/ { IS_UHX_F, "ABCDEf", -1, 6, "ABCDEF" },
        /* 19*/ { IS_LHX_F, "abcdef", -1, 0, "abcdef" },
        /* 20*/ { IS_LHX_F, "abcdeF", -1, 6, "abcdef" },
        /* 21*/ { IS_UPR_F, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 22*/ { IS_UPR_F, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ", -1, 27, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 23*/ { IS_UPR_F, "X", -1, 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 24*/ { IS_UPR_F, "x", -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 25*/ { IS_LWR_F, "abcdefghijklmnopqrstuvwxyz", -1, 0, "abcdefghijklmnopqrstuvwxyz" },
        /* 26*/ { IS_LWR_F, "abcdefghijklmnopqrstuvwxyz ", -1, 27, "abcdefghijklmnopqrstuvwxyz" },
        /* 27*/ { IS_LWR_F, "x", -1, 0, "abcdefghijklmnopqrstuvwxyz" },
        /* 28*/ { IS_LWR_F, "X", -1, 1, "abcdefghijklmnopqrstuvwxyz" },
        /* 29*/ { IS_UX__F, "X", -1, 0, "X" },
        /* 30*/ { IS_UX__F, "x", -1, 1, "X" },
        /* 31*/ { IS_LX__F, "x", -1, 0, "x" },
        /* 32*/ { IS_LX__F, "X", -1, 1, "x" },
        /* 33*/ { IS_C82_F, "!\"%&'(),./:;<=>?_", -1, 0, "!\"%&'(),./:;<=>?_" }, /* CSET82 punctuation less "*+-" */
        /* 34*/ { IS_C82_F, "!\"%&'(),./:;<=>?_ ", -1, 18, "!\"%&'(),./:;<=>?_" },
        /* 35*/ { IS_C82_F, "-", -1, 1, "!\"%&'(),./:;<=>?_" },
        /* 36*/ { IS_C82_F, "$", -1, 1, "!\"%&'(),./:;<=>?_" },
        /* 37*/ { IS_SIL_F, ".$/%", -1, 0, ".$/%" }, /* SILVER punctuation less " +-" */
        /* 38*/ { IS_SIL_F, ".$/% " , -1, 5, ".$/%" },
        /* 39*/ { IS_SIL_F, "-", -1, 1, ".$/%" },
        /* 40*/ { IS_CLI_F, "$:/.", -1, 0, "$:/." }, /* CALCIUM INNER punctuation less "+-" */
        /* 41*/ { IS_CLI_F, "$:/. ", -1, 5, "$:/." },
        /* 42*/ { IS_CLI_F, "+", -1, 1, "$:/." },
        /* 43*/ { IS_ARS_F, "ABCDEFGHJKLMNPRSTUVWXYZ", -1, 0, "ABCDEFGHJKLMNPRSTUVWXYZ" }, /* ARSENIC uppercase */
        /* 44*/ { IS_ARS_F, "ABCDEFGHJKLMNPRSTUVWXYZ ", -1, 24, "ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 45*/ { IS_ARS_F, "I", -1, 1, "ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 46*/ { IS_ARS_F, "O", -1, 1, "ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 47*/ { IS_NUM_F | IS_UHX_F, "0123456789ABCDEF", -1, 0, "0123456789ABCDEF" }, /* SSET */
        /* 48*/ { IS_NUM_F | IS_UHX_F, "0123456789ABCDEf", -1, 16, "0123456789ABCDEF" },
        /* 49*/ { IS_NUM_F | IS_PLS_F, "0123456789+", -1, 0, "0123456789+" }, /* SODIUM_PLS */
        /* 50*/ { IS_NUM_F | IS_PLS_F, "0123456789+-", -1, 12, "0123456789+" },
        /* 51*/ { IS_NUM_F | IS_UX__F, "0123456789X", -1, 0, "0123456789X" }, /* ISBNX_SANE */
        /* 52*/ { IS_NUM_F | IS_UX__F, "0123456789x", -1, 11, "0123456789X" },
        /* 53*/ { IS_NUM_F | IS_UX__F | IS_LX__F | IS_PLS_F, "0123456789Xx+", -1, 0, "0123456789Xx+" }, /* ISBNX_ADDON_SANE */
        /* 54*/ { IS_NUM_F | IS_UX__F | IS_LX__F | IS_PLS_F, "0123456789Xx+Y", -1, 14, "0123456789Xx+" },
        /* 55*/ { IS_NUM_F | IS_MNS_F, "0123456789-", -1, 0, "0123456789-" }, /* SODIUM_MNS */
        /* 56*/ { IS_NUM_F | IS_MNS_F, "0123456789-+", -1, 12, "0123456789-" },
        /* 57*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", -1, 0, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" }, /* CSET82 */
        /* 58*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, " ", -1, 1, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" },
        /* 59*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, "#", -1, 1, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" },
        /* 60*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, "$", -1, 1, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" },
        /* 61*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, "@", -1, 1, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" },
        /* 62*/ { IS_LWR_F | IS_C82_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SPC_F, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ ", -1, 0, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ " }, /* IS_ISOIEC_F */
        /* 63*/ { IS_LWR_F | IS_C82_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SPC_F, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ #", -1, 48, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ " },
        /* 64*/ { IS_LWR_F | IS_C82_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SPC_F, "$", -1, 1, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ " },
        /* 65*/ { IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F, "-. $/+%", -1, 0, "" },
        /* 66*/ { IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F, "-. $/!+%", -1, 6, "" },
        /* 67*/ { IS_NUM_F | IS_UPR_F | IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%", -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%" }, /* SILVER */
        /* 68*/ { IS_NUM_F | IS_UPR_F | IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%a", -1, 44, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%" },
        /* 69*/ { IS_NUM_F | IS_ARS_F, "0123456789ABCDEFGHJKLMNPRSTUVWXYZ", -1, 0, "0123456789ABCDEFGHJKLMNPRSTUVWXYZ" }, /* ARSENIC */
        /* 70*/ { IS_NUM_F | IS_ARS_F, "0123456789ABCDEFGHJKLMNPQRSTUVWXYZ", -1, 25, "0123456789ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 71*/ { IS_NUM_F | IS_ARS_F, "0123456789ABCDEFGHJKLMNPRSTUVWXYz", -1, 33, "0123456789ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 72*/ { IS_NUM_F | IS_UPR_F | IS_LWR_F | IS_SPC_F | IS_HSH_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #", -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #" }, /* GDSET */
        /* 73*/ { IS_NUM_F | IS_UPR_F | IS_LWR_F | IS_SPC_F | IS_HSH_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #!", -1, 65, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #" },
        /* 74*/ { IS_NUM_F | IS_MNS_F | IS_CLI_F | IS_PLS_F, "0123456789-$:/.+", -1, 0, "0123456789-$:/.+" }, /* CALCIUM_INNER */
        /* 75*/ { IS_NUM_F | IS_MNS_F | IS_CLI_F | IS_PLS_F, "0123456789-$:/.+ ", -1, 17, "0123456789-$:/.+" },
        /* 76*/ { IS_NUM_F | IS_MNS_F | IS_CLI_F | IS_PLS_F, "0123456789-$:/.+!", -1, 17, "0123456789-$:/.+" },
        /* 77*/ { IS_NUM_F | IS_UPR_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, /* KRSET */
        /* 78*/ { IS_NUM_F | IS_UPR_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYz", -1, 36, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 79*/ { IS_NUM_F | IS_UPR_F | IS_SPC_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ ", -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ " }, /* RUBIDIUM */
        /* 80*/ { IS_NUM_F | IS_UPR_F | IS_SPC_F, "0123456789aBCDEFGHIJKLMNOPQRSTUVWXYZ ", -1, 11, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ " },
        /* 81*/ { IS_NUM_F | IS_MNS_F | IS_UPR_F, "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 0, "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, /* SHKASUTSET */
        /* 82*/ { IS_NUM_F | IS_MNS_F | IS_UPR_F, "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYz", -1, 37, "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 83*/ { IS_NUM_F | IS_UPR_F | IS_SPC_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SIL_F | IS_CLI_F, "1234567890 $%*+-./:ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 0, "1234567890 $%*+-./:ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, /* QR_ALPHA */
        /* 84*/ { IS_NUM_F | IS_UPR_F | IS_SPC_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SIL_F | IS_CLI_F, "1234567890 $%*+-./:ABCDEFGHIJKLMNOPQRSTUVWXYz", -1, 45, "1234567890 $%*+-./:ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, length, ret;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = z_not_sane(data[i].flg, (const unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);

        if (data[i].orig_test[0]) {
            int orig_ret = is_sane_orig(data[i].orig_test, (const unsigned char *) data[i].data, length);
            if (orig_ret == 0) {
                assert_zero(ret, "i:%d orig_ret %d, ret %d != 0\n", i, orig_ret, ret);
            } else {
                assert_nonzero(ret, "i:%d orig_ret %d, ret %d == 0\n", i, orig_ret, ret);
            }
        }

        ret = 0;
        for (j = 0; j < length; j++) {
            if (!z_is_chr(data[i].flg, data[i].data[j])) {
                ret = j + 1;
                break;
            }
        }
        assert_equal(ret, data[i].ret, "i:%d z_is_chr() ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_not_sane_lookup(const testCtx *const p_ctx) {

    struct item {
        const char *test_string;
        int test_length;
        const char *data;
        int length;
        int ret;

        int posns[32];
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "1234567", -1, "7654321357", -1, 0, { 6, 5, 4, 3, 2, 1, 0, 2, 4, 6 } },
        /*  1*/ { "1234567", -1, "76543213578", -1, 11, {0} },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    int test_length;
    int posns[32] = {0}; /* Suppress clang -fsanitize=memory false positive */

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        test_length = data[i].test_length == -1 ? (int) strlen(data[i].test_string) : data[i].test_length;
        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = z_not_sane_lookup(data[i].test_string, test_length, (const unsigned char *) data[i].data, length,
                                posns);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);

        if (ret == 0) {
            int j;
            for (j = 0; j < length; j++) {
                assert_equal(posns[j], data[i].posns[j], "i:%d posns[%d] %d != expected posns[%d] %d\n",
                            i, j, posns[j], j, data[i].posns[j]);
            }
        }
    }

    testFinish();
}

static void test_errtxt(const testCtx *const p_ctx) {

    struct item {
        int debug_test;
        int error_number;
        int err_id;
        const char *msg;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 0, ZINT_ERROR_TOO_LONG, -1, "", "" },
        /*  1*/ { 0, ZINT_ERROR_TOO_LONG, 123, "", "123: " },
        /*  2*/ { 0, ZINT_ERROR_TOO_LONG, 0, "Data too long", "000: Data too long" },
        /*  3*/ { 0, ZINT_ERROR_TOO_LONG, 9, "Data too long", "009: Data too long" },
        /*  4*/ { 0, ZINT_ERROR_TOO_LONG, 99, "Data too long", "099: Data too long" },
        /*  5*/ { 0, ZINT_ERROR_TOO_LONG, 100, "Data too long", "100: Data too long" },
        /*  6*/ { 0, ZINT_ERROR_TOO_LONG, 999, "Data too long", "999: Data too long" },
        /*  7*/ { 0, ZINT_ERROR_TOO_LONG, 1000, "Data too long", "1000: Data too long" },
        /*  8*/ { 0, ZINT_ERROR_TOO_LONG, 9999, "Data too long", "9999: Data too long" },
        /*  9*/ { 0, ZINT_ERROR_TOO_LONG, 10000, "Data too long", "9999: Data too long" },
        /* 10*/ { 0, ZINT_ERROR_TOO_LONG, 99999, "Data too long", "9999: Data too long" },
        /* 11*/ { 1, ZINT_ERROR_TOO_LONG, 10000, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234", "9999: 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;
    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));
        if (data[i].debug_test) symbol->debug |= ZINT_DEBUG_TEST;

        ret = z_errtxt(data[i].error_number, symbol, data[i].err_id, data[i].msg);
        assert_equal(ret, data[i].error_number, "i:%d ret %d != %d\n", i, ret, data[i].error_number);
        assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->errtxt, data[i].expected);
    }

    testFinish();
}

static void test_errtxtf(const testCtx *const p_ctx) {

    struct item {
        int debug_test;
        int error_number;
        int err_id;
        const char *fmt;
        int num_args;
        int i_arg;
        const char *s_arg;
        double f_arg;
        int ret;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%0$d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid numbered format specifier" },
        /*  1*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%1d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid numbered format specifier" },
        /*  2*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%10$d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid numbered format specifier" },
        /*  3*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%10d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid numbered format specifier" },
        /*  4*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%00d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid numbered format specifier" },
        /*  5*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%000d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid numbered format specifier" },
        /*  6*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%001d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid numbered format specifier" },
        /*  7*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%0111d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid numbered format specifier" },
        /*  8*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%x", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: unknown format specifier ('%c','%d','%f','%g','%s' only)" },
        /*  9*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%1$10d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: unknown format specifier ('%c','%d','%f','%g','%s' only)" },
        /* 10*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%.0s", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid length precision" },
        /* 11*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%.ss", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid length precision" },
        /* 12*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%.10d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: invalid length precision" },
        /* 13*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%1$d %d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: mixed numbered and unnumbered format specifiers" },
        /* 14*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%d %d %d %d %s %d %d %d %d %d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: too many format specifiers (9 maximum)" },
        /* 15*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%1$d %2$d %3$d %4$d %5$s %6$d %7$d %8$d %9$d %9$d", 0, 0, NULL, 0, ZINT_ERROR_ENCODING_PROBLEM, "000: Internal error: too many format specifiers (9 maximum)" },
        /* 16*/ { 0, ZINT_ERROR_TOO_LONG, 123, "%d %d %d %d %s %d %d %d %d", 9, 4, "5max", -1, -1, "123: 2100000001 2100000002 3333 4 5max 2100000006 2100000007 2100000008 2100000009" },
        /* 17*/ { 0, ZINT_ERROR_TOO_LONG, 123, "%1$d %2$d %3$d %4$d %5$s %6$d %7$d %8$d %9$d", 9, 4, "5max", -1, -1, "123: 2100000001 2100000002 3333 4 5max 2100000006 2100000007 2100000008 2100000009" },
        /* 18*/ { 0, ZINT_ERROR_TOO_LONG, 123, "%9$d %8$d %7$d %6$d %5$s %4$d %3$d %2$d %1$d", 9, 4, "5max", -1, -1, "123: 2100000009 2100000008 2100000007 2100000006 5max 4 3333 2100000002 2100000001" },
        /* 19*/ { 0, ZINT_ERROR_TOO_LONG, 123, "%9$d %8$d %7$d %6$d %5$.3s %4$d %3$d %2$d %1$d", 9, 4, "5max", -1, -1, "123: 2100000009 2100000008 2100000007 2100000006 5ma 4 3333 2100000002 2100000001" },
        /* 20*/ { 0, ZINT_ERROR_TOO_LONG, 123, "%1$d %2$d %3$d %5$.*4$s %6$d %7$d %8$d %9$d", 9, 4, "45max", -1, -1, "123: 2100000001 2100000002 3333 45ma 2100000006 2100000007 2100000008 2100000009" },
        /* 21*/ { 0, ZINT_ERROR_TOO_LONG, 123, "%%%d %d %d %d %s %d %d %d %d", 9, 4, "5max", -1, -1, "123: %2100000001 2100000002 3333 4 5max 2100000006 2100000007 2100000008 2100000009" },
        /* 22*/ { 0, ZINT_ERROR_TOO_LONG, 123, "%%%d%%%% %%d %d%%%%%069d %d%% %%%s %d %d %%%% %d%d%%", 9, 4, "5max", -1, -1, "123: %2100000001%% %d 2100000002%%000000000000000000000000000000000000000000000000000000000000000003333 4% %5max 2100000006 2100000007 %% 21000000082100000009%" },
        /* 23*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%%%d%%%% %%d %d%%%%%069d %d%% %%%s %d %d %%%% %d %d%%", 9, 4, "5max", -1, -1, "123: %2100000001%% %d 2100000002%%000000000000000000000000000000000000000000000000000000000000000003333 4% %5max 2100000006 2100000007 %% 2100000008 2100000009" }, /* Truncated */
        /* 24*/ { 1, ZINT_ERROR_TOO_LONG, 123, "%%%d%%%% %%d %d%%%%%069d %d%% %%%s %d %d %%%% %d %d ", 9, 4, "5max", -1, -1, "123: %2100000001%% %d 2100000002%%000000000000000000000000000000000000000000000000000000000000000003333 4% %5max 2100000006 2100000007 %% 2100000008 2100000009" }, /* Truncated */
        /* 25*/ { 0, ZINT_ERROR_TOO_LONG, 123, "%d %011d %05d %05d %s %d %d %d %014d", 9, 4, "5max", -1, -1, "123: 2100000001 02100000002 03333 00004 5max 2100000006 2100000007 2100000008 00002100000009" },
        /* 26*/ { 0, ZINT_ERROR_TOO_LONG, 123, "", 0, 0, NULL, 0, -1, "123: " },
        /* 27*/ { 0, ZINT_ERROR_TOO_LONG, -1, "Gosh '%c' wow", 1, 1, NULL, -1, -1, "Gosh '\001' wow" },
        /* 28*/ { 0, ZINT_ERROR_TOO_LONG, 0, "Gosh '%c' wow", 1, 1, NULL, -1, -1, "000: Gosh '\001' wow" },
        /* 29*/ { 0, ZINT_ERROR_TOO_LONG, 99, "Gosh %d wow", 1, 1, NULL, -1, -1, "099: Gosh 1 wow" },
        /* 30*/ { 0, ZINT_ERROR_TOO_LONG, 99, "Gosh %02d wow", 1, 1, NULL, -1, -1, "099: Gosh 01 wow" },
        /* 31*/ { 0, ZINT_ERROR_TOO_LONG, 99, "Gosh %03d wow", 1, 99, NULL, -1, -1, "099: Gosh 099 wow" },
        /* 32*/ { 0, ZINT_ERROR_TOO_LONG, 99, "Gosh %012d wow", 1, 99, NULL, -1, -1, "099: Gosh 000000000099 wow" },
        /* 33*/ { 0, ZINT_ERROR_TOO_LONG, -1, "%099d", 1, 99, NULL, -1, -1, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000099" },
        /* 34*/ { 0, ZINT_ERROR_TOO_LONG, 9999, "Gosh %d%% wow", 1, 100, NULL, -1, -1, "9999: Gosh 100% wow" },
        /* 35*/ { 0, ZINT_ERROR_TOO_LONG, 10000, "Gosh %d%% wow", 1, 100, NULL, -1, -1, "9999: Gosh 100% wow" },
        /* 36*/ { 0, ZINT_ERROR_TOO_LONG, 99999, "Gosh %d%% wow", 1, 100, NULL, -1, -1, "9999: Gosh 100% wow" },
        /* 37*/ { 0, ZINT_ERROR_TOO_LONG, 9999, "%%%d%% wow", 1, 100, NULL, -1, -1, "9999: %100% wow" },
        /* 38*/ { 0, ZINT_ERROR_TOO_LONG, 9999, "Gosh %1$d wow", 1, 1, NULL, -1, -1, "9999: Gosh 1 wow" },
        /* 39*/ { 0, ZINT_ERROR_TOO_LONG, 9999, "Gosh %1$d wow %1$d", 1, 1, NULL, -1, -1, "9999: Gosh 1 wow 1" },
        /* 40*/ { 0, ZINT_ERROR_TOO_LONG, 9999, "Gosh %1$09d wow", 1, 10, NULL, -1, -1, "9999: Gosh 000000010 wow" },
        /* 41*/ { 0, ZINT_ERROR_TOO_LONG, 9999, "Gosh %1$03d wow %1$04d", 1, 10, NULL, -1, -1, "9999: Gosh 0010 wow 0010" }, /* TODO: incompat: last min field trumps but each should be respected */
        /* 42*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%f' wow", 1, -1, NULL, 3.1, -1, "123: Gosh '3.100000' wow" },
        /* 43*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.2f' wow", 1, -1, NULL, 3.1, -1, "123: Gosh '3.10' wow" },
        /* 44*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.3f' wow", 1, -1, NULL, 3.1, -1, "123: Gosh '3.100' wow" },
        /* 45*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.3f' wow", 1, -1, NULL, 3.1234, -1, "123: Gosh '3.123' wow" },
        /* 46*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.2f' wow", 1, -1, NULL, 3.9995, -1, "123: Gosh '4.00' wow" },
        /* 47*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.3f' wow", 1, -1, NULL, 3.9995, -1, "123: Gosh '3.999' wow" },
        /* 48*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%1$.3f' wow", 1, -1, NULL, 3.9995, -1, "123: Gosh '3.999' wow" },
        /* 49*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %g wow", 1, -1, NULL, 3.1, -1, "123: Gosh 3.1 wow" },
        /* 50*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %.1g wow", 1, -1, NULL, 3.1, -1, "123: Gosh 3 wow" },
        /* 51*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %.2g wow", 1, -1, NULL, 3.123, -1, "123: Gosh 3.1 wow" },
        /* 52*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %.3g wow", 1, -1, NULL, 3.123, -1, "123: Gosh 3.12 wow" },
        /* 53*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%s' wow", 1, -1, "gee", -1, -1, "123: Gosh 'gee' wow" },
        /* 54*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.1s' wow", 1, -1, "12345678901234567890", -1, -1, "123: Gosh '1' wow" },
        /* 55*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.9s' wow", 1, -1, "12345678901234567890", -1, -1, "123: Gosh '123456789' wow" },
        /* 56*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.10s' wow", 1, -1, "12345678901234567890", -1, -1, "123: Gosh '1234567890' wow" },
        /* 57*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.19s' wow", 1, -1, "12345678901234567890", -1, -1, "123: Gosh '1234567890123456789' wow" },
        /* 58*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%1$.1s' wow", 1, -1, "12345678901234567890", -1, -1, "123: Gosh '1' wow" },
        /* 59*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%1$.20s' wow", 1, -1, "12345678901234567890", -1, -1, "123: Gosh '12345678901234567890' wow" },
        /* 60*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.2s' wow", 1, -1, "gee", -1, -1, "123: Gosh 'ge' wow" },
        /* 61*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.3s' wow", 1, -1, "gee", -1, -1, "123: Gosh 'gee' wow" },
        /* 62*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.4s' wow", 1, -1, "gee", -1, -1, "123: Gosh 'gee' wow" },
        /* 63*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.9s' wow", 1, -1, "gee", -1, -1, "123: Gosh 'gee' wow" },
        /* 64*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.*s' wow", 2, 0, "gee", -1, -1, "123: Gosh '' wow" },
        /* 65*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.*s' wow", 2, 1, "gee", -1, -1, "123: Gosh 'g' wow" },
        /* 66*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.*s' wow", 2, 2, "gee", -1, -1, "123: Gosh 'ge' wow" },
        /* 67*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.*s' wow", 2, 3, "gee", -1, -1, "123: Gosh 'gee' wow" },
        /* 68*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.*s' wow", 2, 4, "gee", -1, -1, "123: Gosh 'gee' wow" },
        /* 69*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%.*s' wow", 2, 999, "gee", -1, -1, "123: Gosh 'gee' wow" },
        /* 70*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%2$.*1$s' wow", 2, 2, "gee", -1, -1, "123: Gosh 'ge' wow" },
        /* 71*/ { 1, ZINT_ERROR_TOO_LONG, 123, "Gosh %s wow", 1, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456", -1, -1, "123: Gosh 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456 wo" },
        /* 72*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh '%c' %g wow", 2, 'A', NULL, -12.1f, -1, "123: Gosh 'A' -12.1 wow" },
        /* 73*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %d %s wow", 2, 123456789, "cor", -1, -1, "123: Gosh 123456789 cor wow" },
        /* 74*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %1$d %2$s wow", 2, 123456789, "cor", -1, -1, "123: Gosh 123456789 cor wow" },
        /* 75*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %1$d %2$.1s wow", 2, 123456789, "cor", -1, -1, "123: Gosh 123456789 c wow" },
        /* 76*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$s %1$d wow", 2, 123456789, "cor", -1, -1, "123: Gosh cor 123456789 wow" },
        /* 77*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$g %1$d wow", 2, 1, NULL, 2, -1, "123: Gosh 2 1 wow" },
        /* 78*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$s %1$d wow second %2$s", 2, 123456789, "cor", -1, -1, "123: Gosh cor 123456789 wow second cor" },
        /* 79*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$.2s %1$d wow second %2$s", 2, 123456789, "cor", -1, -1, "123: Gosh co 123456789 wow second co" }, /* TODO: incompat: last length trumps but each should be respected */
        /* 80*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$.2s %1$d wow second %2$.1s", 2, 123456789, "cor", -1, -1, "123: Gosh c 123456789 wow second c" }, /* TODO: incompat: last length trumps */
        /* 81*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$.1s %1$d wow second %2$.2s", 2, 123456789, "cor", -1, -1, "123: Gosh co 123456789 wow second co" }, /* TODO: incompat: last length trumps */
        /* 82*/ { 1, ZINT_ERROR_TOO_LONG, -1, "%1$s %1$s", 1, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", -1, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890 123456789012345678901234567890123456789012345678" },
        /* 83*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %d %s %g wow", 3, 1, "cor", 3, -1, "123: Gosh 1 cor 3 wow" },
        /* 84*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %1$d %2$s %3$g wow", 3, 1, "cor", 3, -1, "123: Gosh 1 cor 3 wow" },
        /* 85*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %3$g %2$s %1$d wow", 3, 1, "cor", 3, -1, "123: Gosh 3 cor 1 wow" },
        /* 86*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %3$g %1$d %2$s wow", 3, 1, "cor", 3, -1, "123: Gosh 3 1 cor wow" },
        /* 87*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %3$g %1$d %2$s wow %2$s %1$d", 3, 1, "cor", 3, -1, "123: Gosh 3 1 cor wow cor 1" },
        /* 88*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$.*1$s %3$g wow", 3, 1, "cor", 3, -1, "123: Gosh c 3 wow" },
        /* 89*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$.*1$s %3$.3g wow", 3, 1, "cor", 3.1234, -1, "123: Gosh c 3.12 wow" },
        /* 90*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %2$.*1$s %3$g wow %2$s blimey", 3, 1, "cor", 3, -1, "123: Gosh c 3 wow c blimey" },
        /* 91*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %3$g %2$.*1$s wow", 3, 1, "cor", 3, -1, "123: Gosh 3 c wow" },
        /* 92*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %3$.2g %2$.*1$s wow", 3, 1, "cor", 3.67, -1, "123: Gosh 3.7 c wow" },
        /* 93*/ { 0, ZINT_ERROR_TOO_LONG, 123, "Gosh %3$.3g %2$.2s %1$04d wow", 3, 1, "cor", 3.3333, -1, "123: Gosh 3.33 co 0001 wow" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;
    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));
        if (data[i].debug_test) symbol->debug |= ZINT_DEBUG_TEST;

        if (data[i].num_args == 0) {
            ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt,
                            NULL /*suppress -Wformat-security*/);
        } else if (data[i].num_args == 1) {
            if (data[i].i_arg != -1) {
                ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt, data[i].i_arg);
            } else if (data[i].s_arg != NULL) {
                ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt, data[i].s_arg);
            } else {
                ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt, data[i].f_arg);
            }
        } else if (data[i].num_args == 2) {
            if (data[i].i_arg != -1) {
                if (data[i].s_arg != NULL) {
                    ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt, data[i].i_arg,
                                    data[i].s_arg);
                } else {
                    ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt, data[i].i_arg,
                                    data[i].f_arg);
                }
            } else {
                assert_nonnull(data[i].s_arg, "i:%d num_args:%d data[i].s_arg NULL", i, data[i].num_args);
                ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt, data[i].s_arg,
                                data[i].f_arg);
            }
        } else if (data[i].num_args == 3) {
            assert_nonnull(data[i].s_arg, "i:%d num_args:%d data[i].s_arg NULL", i, data[i].num_args);
            ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt, data[i].i_arg, data[i].s_arg,
                            data[i].f_arg);
        } else if (data[i].num_args == 9) { /* Special case max, assuming 4th arg "%d", 5th arg "%s" */
            assert_nonnull(data[i].s_arg, "i:%d num_args:%d data[i].s_arg NULL", i, data[i].num_args);
            ret = z_errtxtf(data[i].error_number, symbol, data[i].err_id, data[i].fmt, 2100000001, 2100000002, 3333,
                            data[i].i_arg, data[i].s_arg, 2100000006, 2100000007, 2100000008, 2100000009);
        } else {
            assert_nonnull(NULL, "i:%d num_args:%d > 3 && != 9\n", i, data[i].num_args);
        }
        if (data[i].ret == -1) {
            assert_equal(ret, data[i].error_number, "i:%d ret %d != %d (%s)\n",
                        i, ret, data[i].error_number, symbol->errtxt);
        } else {
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->errtxt, data[i].expected);
    }

    testFinish();
}

static void test_cnt_digits(const testCtx *const p_ctx) {

    struct item {
        const char *data;
        int length;
        int position;
        int max;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", -1, 0, -1, 0 },
        /*  1*/ { "asdf1", -1, 0, -1, 0 },
        /*  2*/ { "asdf1asdf", -1, 4, -1, 1 },
        /*  3*/ { "a12345asdf", -1, 1, -1, 5 },
        /*  4*/ { "a12345asdf", -1, 1, 4, 4},
        /*  5*/ { "a1234", -1, 1, 5, 4},
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = z_cnt_digits((const unsigned char *) data[i].data, length, data[i].position, data[i].max);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_is_valid_utf8(const testCtx *const p_ctx) {

    struct item {
        const char *data;
        int length;
        int ret;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", -1, 1, "" },
        /*  1*/ { "abcdefghijklmnopqrstuvwxyz", -1, 1, "" },
        /*  2*/ { "éa", -1, 1, "" },
        /*  3*/ { "a\000b", 3, 1, "Embedded nul" },
        /*  4*/ { "\357\273\277a", -1, 1, "Bom" },

        /*  5*/ { "a\xC2", -1, 0, "Missing 2nd byte" },
        /*  6*/ { "a\200b", -1, 0, "Orphan continuation 0x80" },
        /*  7*/ { "\300\201", -1, 0, "Overlong 0xC081" },
        /*  8*/ { "\355\240\200", -1, 0, "Surrogate 0xEDA080" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = z_is_valid_utf8((const unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_utf8_to_unicode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int length;
        int disallow_4byte;
        int ret;
        int ret_length;
        unsigned int expected_vals[20];
        const char *expected_errtxt;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", -1, 1, 0, 0, {0}, "", "" },
        /*  1*/ { "\000a\302\200\340\240\200", 7, 1, 0, 4, { 0, 'a', 0x80, 0x800 }, "", "NUL a C280 E0A080" },
        /*  2*/ { "\357\277\277", -1, 1, 0, 1, { 0xFFFF }, "", "EFBFBF" },
        /*  3*/ { "\360\220\200\200", -1, 1, ZINT_ERROR_INVALID_DATA, -1, {0}, "242: Unicode sequences of more than 3 bytes not supported", "Four-byte F0908080" },
        /*  4*/ { "a\200b", -1, 1, ZINT_ERROR_INVALID_DATA, -1, {0}, "240: Corrupt Unicode data", "Orphan continuation 0x80" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    unsigned int vals[20] = {0}; /* Suppress clang -fsanitize=memory false positive */
    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = z_utf8_to_unicode(symbol, (unsigned char *) data[i].data, vals, &ret_length, data[i].disallow_4byte);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n",
                        i, ret_length, data[i].ret_length);
            for (j = 0; j < ret_length; j++) {
                assert_equal(vals[j], data[i].expected_vals[j], "i:%d vals[%d] %04X != %04X\n",
                            i, j, vals[j], data[i].expected_vals[j]);
            }
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->errtxt, data[i].expected_errtxt);
    }

    testFinish();
}

/* Note transferred from "test_code128.c" */
static void test_hrt_cpy_iso8859_1(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int length;
        int ret;
        const char *expected;
        const char *comment;
    };
    /*
       NBSP U+00A0 (\240, 160), UTF-8 C2A0 (\302\240)
       ¡ U+00A1 (\241, 161), UTF-8 C2A1 (\302\241)
       é U+00E9 (\351, 233), UTF-8 C3A9
    */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", -1, 0, "", "" },
        /*  1*/ { "abc", -1, 0, "abc", "" },
        /*  2*/ { "\000A\001B\002\036\037C ~\177", 11, 0, " A B   C ~ ", "" },
        /*  3*/ { "~\177\200\201\237\240", -1, 0, "~    \302\240", "" },
        /*  4*/ { "\241\242\243\244\257\260", -1, 0, "¡¢£¤¯°", "" },
        /*  5*/ { "\276\277\300\337\377", -1, 0, "¾¿Àßÿ", "" },
        /*  6*/ { "\351", -1, 0, "é", "" },
        /*  7*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 0, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "127 \241" },
        /*  8*/ { "a\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 0, "a¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "a + 127 \241" },
        /*  9*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241a", -1, 0, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡a", "127 \241 + a" },
        /* 10*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 1, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "128 \241 (truncated)" },
        /* 11*/ { "a\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 1, "a¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "a + 128 \241 (truncated)" },
        /* 12*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241a", -1, 1, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "128 \241 + a (truncated)" },
        /* 13*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 1, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "129 \241 (truncated)" },
        /* 14*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 0, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "127 \351" },
        /* 15*/ { "a\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 0, "aééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "a + 127 \351" },
        /* 16*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351a", -1, 0, "éééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééa", "127 \351 + a" },
        /* 17*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 1, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "128 \351 (truncated)" },
        /* 18*/ { "a\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 1, "aééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "a + 128 \351 (truncated)" },
        /* 19*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351a", -1, 1, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "128 \351 + a (truncated)" },
        /* 20*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 1, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "129 \351 (truncated)" },
        /* 21*/ { "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "256 A (truncated)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;
    int expected_length;

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        expected_length = (int) strlen(data[i].expected);

        ret = z_hrt_cpy_iso8859_1(symbol, (unsigned char *) data[i].data, length);
        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            for (j = 0; j < symbol->text_length; j++) {
                fprintf(stderr, "symbol->text[%d] %2X\n", j, symbol->text[j]);
            }
        }
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_nonzero(testUtilIsValidUTF8(symbol->text, (int) z_ustrlen(symbol->text)),
                        "i:%d testUtilIsValidUTF8(%s) != 1\n", i, symbol->text);
        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d\n",
                    i, symbol->text_length, expected_length);
        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d symbol->text (%s) != expected (%s)\n",
                    i, symbol->text, data[i].expected);
    }

    testFinish();
}

static void test_hrt_cpy_nochk(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *cpy; /* z_hrt_cpy_nochk() */
        int cpy_length;

        const char cpy_chr; /* z_hrt_cpy_chr() */
        int cpy_chr_length;

        const char *cat; /* z_hrt_cat_nochk() */
        int cat_length;

        char cat_chr; /* z_hrt_cat_chr_nochk() */
        int cat_chr_length;

        const char *expected;
        int expected_length;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", -1, 0, 0, "", -1, 0, 0, "", -1 }, /* All zero */
        /*  1*/ { "AB\000C", 4, 0, 0, "", -1, 0, 0, "AB\000C", 4 }, /* z_hrt_cpy_nochk() */
        /*  2*/ { "", -1, '\000', 1, "", -1, 0, 0, "\000", 1 }, /* z_hrt_chr() (NUL char) */
        /*  3*/ { "", -1, '\000', 1, "XYZ", -1, 0, 0, "\000XYZ", 4 }, /* z_hrt_chr() + z_hrt_cat_nochk() */
        /*  4*/ { "", -1, '\000', 1, "", -1, '\000', 1, "\000\000", 2 }, /* z_hrt_chr() + z_hrt_cat_chr_nochk() (both NULL char) */
        /*  5*/ { "", -1, '\000', 1, "XYZ", -1, '\001', 1, "\000XYZ\001", 5 }, /* z_hrt_chr() + z_hrt_cat_chr_nochk() + z_hrt_cat_nochk() */
        /*  6*/ { "ABC\000", 4, 0, 0, "\000XYZ\177", 5, 0, 0, "ABC\000\000XYZ\177", 9 }, /* z_hrt_cpy_nochk() + z_hrt_cat_nochk() */
        /*  7*/ { "ABC\000", 4, 0, 0, "", -1, '\177', 1, "ABC\000\177", 5 }, /* z_hrt_cpy_nochk() + z_hrt_cat_chr_nochk() */
        /*  8*/ { "ABC\000", 4, 0, 0, "X\001Y\002Z", 5, '\003', 1, "ABC\000X\001Y\002Z\003", 10 }, /* z_hrt_cpy_nochk() + z_hrt_cat_chr_nochk() + z_hrt_cat_chr_nochk() */
        /*  9*/ { "1234567890123456789012345678901234567890123456789012345", -1, 0, 0, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", -1, 0, 0, "123456789012345678901234567890123456789012345678901234512345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", -1 }, /* z_hrt_cpy_nochk() + z_hrt_cat_nochk() - max 255 */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length;

    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;
    int expected_length;

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));

        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;

        if ((length = data[i].cpy_length == -1 ? (int) strlen(data[i].cpy) : data[i].cpy_length)) {
            z_hrt_cpy_nochk(symbol, TCU(data[i].cpy), length);
        }
        if (data[i].cpy_chr_length) {
            z_hrt_cpy_chr(symbol, data[i].cpy_chr);
        }
        if ((length = data[i].cat_length == -1 ? (int) strlen(data[i].cat) : data[i].cat_length)) {
            z_hrt_cat_nochk(symbol, TCU(data[i].cat), length);
        }
        if (data[i].cat_chr_length) {
            z_hrt_cat_chr_nochk(symbol, data[i].cat_chr);
        }

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            for (j = 0; j < symbol->text_length; j++) {
                fprintf(stderr, "symbol->text[%d] %2X\n", j, symbol->text[j]);
            }
        }

        assert_nonzero(testUtilIsValidUTF8(symbol->text, (int) z_ustrlen(symbol->text)),
                    "i:%d testUtilIsValidUTF8(%s) != 1\n", i, symbol->text);
        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d\n",
                    i, symbol->text_length, expected_length);
        assert_zero(memcmp(symbol->text, data[i].expected, expected_length), "i:%d memcmp(%s, %s, %d) != 0\n",
                    i, symbol->text, data[i].expected, expected_length);
    }

    testFinish();
}

static void test_hrt_cpy_cat_nochk(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *source;
        int length;
        char separator;
        const char *cat;
        int cat_length;

        const char *expected;
        int expected_length;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { NULL, -1, '\xFF', NULL, -1, "", -1 },
        /*  1*/ { "", 0, '\xFF', NULL, -1, "", -1 },
        /*  2*/ { "", 0, '\xFF', "", 0, "", -1 },
        /*  3*/ { NULL, 0, '\xFF', "", 0, "", -1 },
        /*  4*/ { "ABC", 3, ':', "DEF", 3, "ABC:DEF", -1 },
        /*  5*/ { "", 0, ':', "DEF", 3, ":DEF", -1 },
        /*  6*/ { "", 0, '\xFF', "DEF", 3, "DEF", -1 },
        /*  7*/ { "ABC", 3, ':', "", 0, "ABC:", -1 },
        /*  8*/ { "ABC", 3, '\xFF', "", 0, "ABC", -1 },
        /*  9*/ { "", 0, ':', "", 0, ":", -1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i;

    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;
    int expected_length;

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));

        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;

        z_hrt_cpy_cat_nochk(symbol, TCU(data[i].source), data[i].length, data[i].separator, TCU(data[i].cat),
                            data[i].cat_length);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            for (j = 0; j < symbol->text_length; j++) {
                fprintf(stderr, "symbol->text[%d] %2X\n", j, symbol->text[j]);
            }
        }

        assert_nonzero(testUtilIsValidUTF8(symbol->text, (int) z_ustrlen(symbol->text)),
                    "i:%d testUtilIsValidUTF8(%s) != 1\n", i, symbol->text);
        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d\n",
                    i, symbol->text_length, expected_length);
        assert_zero(memcmp(symbol->text, data[i].expected, expected_length), "i:%d memcmp(%s, %s, %d) != 0\n",
                    i, symbol->text, data[i].expected, expected_length);
    }

    testFinish();
}

static void test_hrt_printf_nochk(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *fmt;
        int num_args;
        const char *data1;
        const char *data2;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", 1, "", "", "" },
        /*  1*/ { "*%s*", 1, "gosh", "", "*gosh*" },
        /*  2*/ { "%.1s.%.2s", 2, "gosh", "wow", "g.wo" },
        /*  3*/ { "ABCDEFGHIJKLMNOPQRST%sABCDEFGHIJKLMNOPQRST%sABCDEFGHIJKLMNO", 2, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "ABCDEFGHIJKLMNOPQRST1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890ABCDEFGHIJKLMNOPQRST1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890ABCDEFGHIJKLMNO" }, /* Max 255 - 1 more and overflow */
    };
    const int data_size = ARRAY_SIZE(data);
    int i;

    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));

        if (data[i].num_args == 1) {
            z_hrt_printf_nochk(symbol, data[i].fmt, data[i].data1);
        } else if (data[i].num_args == 2) {
            z_hrt_printf_nochk(symbol, data[i].fmt, data[i].data1, data[i].data2);
        } else {
            assert_zero(1, "i:%d, bad num_args\n", i);
        }

        assert_zero(strcmp((const char *) symbol->text, data[i].expected), "i:%d strcmp(\"%s\", \"%s\") != 0\n",
                    i, symbol->text, data[i].expected);
    }

    testFinish();
}

static void test_hrt_conv_gs1_brackets_nochk(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", "" },
        /*  1*/ { "[", "(" }, /* Converts mismatched opening */
        /*  2*/ { "]", "]" }, /* But not mismatched closing */
        /*  3*/ { "[]", "()" },
        /*  4*/ { "(", "(" },
        /*  5*/ { ")", ")" },
        /*  6*/ { "[[]", "(()" },
        /*  7*/ { "[]]", "()]" },
        /*  8*/ { "[[]]", "(())" },
        /*  9*/ { "[[]][", "(())(" },
        /* 10*/ { "[[]]]", "(())]" },
        /* 11*/ { "[[]][]", "(())()" },
        /* 12*/ { "[[[[]]][]]", "(((()))())" },
        /* 13*/ { "[[[[]]]][]]", "(((())))()]" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length;

    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));

        length = (int) strlen(data[i].data);

        z_hrt_conv_gs1_brackets_nochk(symbol, TCU(data[i].data), length);

        assert_zero(strcmp((const char *) symbol->text, data[i].expected), "i:%d strcmp(\"%s\", \"%s\") != 0\n",
                    i, symbol->text, data[i].expected);
    }

    testFinish();
}

static void test_ct_cpy_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int seg_count;
        struct zint_seg segs[3];

        struct zint_seg expected_content_segs[3];
        int expected_content_seg_count;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 1, { { TU("A"), 0, 0 } }, { { TU("A"), 1, 3 } }, 1 },
        /*  1*/ { 1, { { TU("B\377C\377\000D"), 6, 0 } }, { { TU("B\377C\377\000D"), 6, 3 } }, 1 },
        /*  2*/ { 2, { { TU("A"), 0, 5 }, { TU("\000\355"), 2, 899 } }, { { TU("A"), 1, 5 }, { TU("\000\355"), 2, 899 } }, 2 },
        /*  3*/ { 3, { { TU("A"), 1, 5 }, { TU("ABCD"), 0, 900 }, { TU("\000\355"), 2, 899 } }, { { TU("A"), 1, 5 }, { TU("ABCD"), 4, 900 }, { TU("\000\355"), 2, 899 } }, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;

    struct zint_symbol s_symbol = {0};
    struct zint_symbol *symbol = &s_symbol;

    char escaped[4096];
    char escaped2[4096];

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {
        int expected_length;
        unsigned char *expected_source;
        int expected_eci;
        int seg_idx;

        if (testContinue(p_ctx, i)) continue;

        assert_nonzero(data[i].seg_count, "i:%d seg_count zero\n", i);

        ret = z_ct_cpy_segs(symbol, data[i].segs, data[i].seg_count);
        assert_zero(ret, "i:%d z_ct_cpy_segs %d != 0\n", i, ret);

        assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
        assert_equal(symbol->content_seg_count, data[i].seg_count, "i:%d content_seg_count %d != %d\n",
                    i, symbol->content_seg_count, data[i].seg_count);
        for (seg_idx = 0; seg_idx < data[i].seg_count; seg_idx++) {
            assert_nonnull(&symbol->content_segs[seg_idx], "i:%d content_segs[%d] NULL\n", i, seg_idx);
            assert_nonnull(symbol->content_segs[seg_idx].source, "i:%d content_segs[%d].source NULL\n", i, seg_idx);

            expected_length = data[i].expected_content_segs[seg_idx].length;
            expected_source = data[i].expected_content_segs[seg_idx].source;
            expected_eci = data[i].expected_content_segs[seg_idx].eci;

            assert_equal(symbol->content_segs[seg_idx].length, expected_length,
                        "i:%d content_segs[%d].length %d != expected_length %d\n",
                        i, seg_idx, symbol->content_segs[seg_idx].length, expected_length);
            assert_zero(memcmp(symbol->content_segs[seg_idx].source, expected_source, expected_length),
                        "i:%d content_segs[%d].source memcmp(%s, %s, %d) != 0\n", i, seg_idx,
                        testUtilEscape(ZCCP(symbol->content_segs[seg_idx].source), symbol->content_segs[seg_idx].length,
                                        escaped, sizeof(escaped)),
                        testUtilEscape(ZCCP(expected_source), expected_length, escaped2, sizeof(escaped2)),
                                        expected_length);
            assert_equal(symbol->content_segs[seg_idx].eci, expected_eci, "i:%d content_segs[%d].eci %d != expected_eci %d\n",
                        i, seg_idx, symbol->content_segs[seg_idx].eci, expected_eci);
        }

        ZBarcode_Clear(symbol);
    }

    testFinish();
}

static void test_ct_cpy(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *source;
        int length;
        char separator;
        const char *cat;
        int cat_length;

        const char *expected;
        int expected_length;
        int expected_eci;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "A", -1, '\0', "", -1, "A", -1, 3 },
        /*  1*/ { "A", -1, ':', "B", -1, "A:B", -1, 3 },
        /*  2*/ { "A", -1, '\xFF', "B", -1, "AB", -1, 3 },
        /*  3*/ { "A", -1, '\0', "B", -1, "A\000B", 3, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol s_symbol = {0};
    struct zint_symbol *symbol = &s_symbol;
    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {
        int cat_length;

        if (testContinue(p_ctx, i)) continue;

        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;

        length = data[i].length == -1 ? (int) strlen(data[i].source) : data[i].length;

        if ((cat_length = data[i].cat_length == -1 ? (int) strlen(data[i].cat) : data[i].cat_length)) {
            ret = z_ct_cpy_cat(symbol, TCU(data[i].source), length, data[i].separator, TCU(data[i].cat), cat_length);
            assert_zero(ret, "i:%d z_ct_cpy_cat %d != 0\n", i, ret);
        } else {
            ret = z_ct_cpy(symbol, TCU(data[i].source), length);
            assert_zero(ret, "i:%d z_ct_cpy %d != 0\n", i, ret);
        }

        assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
        assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
        assert_equal(symbol->content_segs[0].length, expected_length,
                    "i:%d content_segs[0].length %d != expected_length %d\n",
                    i, symbol->content_segs[0].length, expected_length);
        assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected, expected_length),
                    "i:%d content_segs[0].source memcmp(%s, %s, %d) != 0\n", i,
                    testUtilEscape((const char *) symbol->content_segs[0].source, symbol->content_segs[0].length,
                                    escaped, sizeof(escaped)),
                    testUtilEscape(data[i].expected, expected_length, escaped2, sizeof(escaped2)),
                    expected_length);
        assert_equal(symbol->content_segs[0].eci, data[i].expected_eci,
                    "i:%d content_segs[0].eci %d != expected_eci %d\n",
                    i, symbol->content_segs[0].eci, data[i].expected_eci);

        ZBarcode_Clear(symbol);
    }

    testFinish();
}

static void test_ct_cpy_iso8859_1(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *source;
        int length;

        const char *expected;
        int expected_length;
        int expected_eci;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "A", -1, "A", -1, 3 },
        /*  1*/ { "\000AB\177", 4, "\000AB\177", 4, 3 },
        /*  2*/ { "A\200", -1, "A\302\200", -1, 3 },
        /*  3*/ { "A\237\240\277\300B\377C", -1, "A\302\237\302\240\302\277\303\200B\303\277C", -1, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol s_symbol = {0};
    struct zint_symbol *symbol = &s_symbol;
    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;

        length = data[i].length == -1 ? (int) strlen(data[i].source) : data[i].length;

        ret = z_ct_cpy_iso8859_1(symbol, TCU(data[i].source), length);
        assert_zero(ret, "i:%d z_ct_cpy_iso8859_1 %d != 0\n", i, ret);

        assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
        assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
        assert_equal(symbol->content_segs[0].length, expected_length,
                    "i:%d content_segs[0].length %d != expected_length %d\n",
                    i, symbol->content_segs[0].length, expected_length);
        assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected, expected_length),
                    "i:%d content_segs[0].source memcmp(%s, %s, %d) != 0\n", i,
                    testUtilEscape((const char *) symbol->content_segs[0].source, symbol->content_segs[0].length,
                                    escaped, sizeof(escaped)),
                    testUtilEscape(data[i].expected, expected_length, escaped2, sizeof(escaped2)),
                    expected_length);
        assert_equal(symbol->content_segs[0].eci, data[i].expected_eci,
                    "i:%d content_segs[0].eci %d != expected_eci %d\n",
                    i, symbol->content_segs[0].eci, data[i].expected_eci);

        ZBarcode_Clear(symbol);
    }

    testFinish();
}

static void test_ct_printf_256(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *fmt;
        int num_args;
        const char *data1;
        const char *data2;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "", 1, "", "", "" },
        /*  1*/ { "*%s*", 1, "gosh", "", "*gosh*" },
        /*  2*/ { "%.1s.%.2s", 2, "gosh", "wow", "g.wo" },
        /*  3*/ { "ABCDEFGHIJKLMNOPQRST%sABCDEFGHIJKLMNOPQRST%sABCDEFGHIJKLMNO", 2, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "ABCDEFGHIJKLMNOPQRST1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890ABCDEFGHIJKLMNOPQRST1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890ABCDEFGHIJKLMNO" }, /* Max 255 - 1 more and overflow */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;

    struct zint_symbol s_symbol = {0};
    struct zint_symbol *symbol = &s_symbol;
    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        expected_length = (int) strlen(data[i].expected);

        if (data[i].num_args == 1) {
            ret = z_ct_printf_256(symbol, data[i].fmt, data[i].data1);
            assert_zero(ret, "i:%d z_ct_printf_256 1 arg ret %d != 0\n", i, ret);
        } else if (data[i].num_args == 2) {
            ret = z_ct_printf_256(symbol, data[i].fmt, data[i].data1, data[i].data2);
            assert_zero(ret, "i:%d z_ct_printf_256 2 args ret %d != 0\n", i, ret);
        } else {
            assert_zero(1, "i:%d, bad num_args\n", i);
        }

        assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
        assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
        assert_equal(symbol->content_segs[0].length, expected_length,
                    "i:%d content_segs[0].length %d != expected_length %d\n",
                    i, symbol->content_segs[0].length, expected_length);
        assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected, expected_length),
                    "i:%d content_segs[0].source memcmp(%s, %s, %d) != 0\n", i,
                    testUtilEscape((const char *) symbol->content_segs[0].source, symbol->content_segs[0].length,
                                    escaped, sizeof(escaped)),
                    testUtilEscape(data[i].expected, expected_length, escaped2, sizeof(escaped2)),
                    expected_length);
        assert_equal(symbol->content_segs[0].eci, 3, "i:%d content_segs[0].eci %d != 3\n", i, symbol->content_segs[0].eci);

        ZBarcode_Clear(symbol);
    }

    testFinish();
}

static void test_set_height(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int rows;
        int row_height[20];
        float height;
        float min_row_height;
        float default_height;
        float max_height;
        int no_errtxt;
        int ret;
        float expected_height;
        const char *expected_errtxt;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 0, { 0 }, 0, 0, 0, 0, 0, 0, 0.5, "", "" },
        /*  1*/ { 2, { 1, 1 }, 2, 0, 0, 0, 0, 0, 2, "", "zero_count == 0, fixed height only" },
        /*  2*/ { 2, { 1, 1 }, 2, 0, 0, 1, 1, ZINT_WARN_NONCOMPLIANT, 2, "", "zero_count == 0, height < max height" },
        /*  3*/ { 2, { 1, 1 }, 2, 0, 0, 1, 0, ZINT_WARN_NONCOMPLIANT, 2, "248: Height not compliant with standards (maximum 1)", "zero_count == 0, height < max height" },
        /*  4*/ { 2, { 2, 0 }, 2, 0, 0, 0, 0, 0, 2.5, "", "zero_count != 0, height 2" },
        /*  5*/ { 2, { 2, 0 }, 2, 1, 0, 0, 1, ZINT_WARN_NONCOMPLIANT, 2.5, "", "zero_count != 0, row_height < min_row_height" },
        /*  6*/ { 2, { 2, 0 }, 2, 1, 0, 0, 0, ZINT_WARN_NONCOMPLIANT, 2.5, "247: Height not compliant with standards (too small)", "zero_count != 0, row_height < min_row_height" },
        /*  7*/ { 2, { 2, 0 }, 0, 0, 20, 0, 0, 0, 22, "", "zero_count != 0, default_height 20" },
        /*  8*/ { 2, { 2, 0 }, 20, 0, 20, 0, 0, 0, 20, "", "zero_count != 0, height 20" },
        /*  9*/ { 2, { 2, 0 }, 0, 2, 0, 0, 0, 0, 4, "", "zero_count != 0, min_row_height 2" },
        /* 10*/ { 20, { 0 }, 0, 12.9000006, 258, 0, 0, 0, 258, "", "Was non-compliant before use of epsilson (CODABLOCKF)" },
        /* 11*/ { 1, { 0 }, 9.9, 9.90000057, 40, 0, 0, 0, 9.9, "", "Was non-compliant before use of epsilson (CODE93)" },
        /* 12*/ { 1, { 0 }, 9.89, 9.90000057, 40, 0, 0, ZINT_WARN_NONCOMPLIANT, 9.89, "247: Height not compliant with standards (too small)", "" },
        /* 13*/ { 1, { 0 }, 40.02, 10, 40, 40.01, 0, ZINT_WARN_NONCOMPLIANT, 40.02, "248: Height not compliant with standards (maximum 40.01)", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;

    struct zint_symbol s_symbol;
    struct zint_symbol *symbol = &s_symbol;

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        memset(symbol, 0, sizeof(*symbol));
        symbol->rows = data[i].rows;
        for (j = 0; j < ARRAY_SIZE(data[i].row_height); j++) {
            symbol->row_height[j] = data[i].row_height[j];
        }
        symbol->height = data[i].height;

        ret = z_set_height(symbol, data[i].min_row_height, data[i].default_height, data[i].max_height,
                            data[i].no_errtxt);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_equal(symbol->height, data[i].expected_height, "i:%d symbol->height %g != %g\n",
                    i, symbol->height, data[i].expected_height);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d errtxt %s != %s\n",
                    i, symbol->errtxt, data[i].expected_errtxt);
    }

    testFinish();
}

INTERNAL void z_debug_test_codeword_dump_int(struct zint_symbol *symbol, const int *codewords, const int length);

static void test_debug_test_codeword_dump_int(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int codewords[50];
        int length;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { { 2147483647, -2147483646, 2147483647, 0, 2147483647, 2147483647, 2147483647, 2147483647, 123456 }, 10, "(10) 2147483647 -2147483646 2147483647 0 2147483647 2147483647 2147483647 2147483647 123456" },
        /*  1*/ { { 2147483647, -2147483646, 2147483647, 0, 2147483647, 2147483647, 2147483647, 2147483647, 1234567 }, 10, "(10) 2147483647 -2147483646 2147483647 0 2147483647 2147483647 2147483647 2147483647" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i;

    struct zint_symbol s_symbol = {0};
    struct zint_symbol *symbol = &s_symbol;

    testStart(p_ctx->func_name);

    symbol->debug = debug;

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        z_debug_test_codeword_dump_int(symbol, data[i].codewords, data[i].length);
        assert_nonzero(strlen(symbol->errtxt) < 92, "i:%d strlen(%s) >= 92 (%d)\n",
                    i, symbol->errtxt, (int) strlen(symbol->errtxt));
        assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0 (%d, %d)\n",
                    i, symbol->errtxt, data[i].expected, (int) strlen(symbol->errtxt),
                    (int) strlen(data[i].expected));
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_to_int", test_to_int },
        { "test_to_upper", test_to_upper },
        { "test_chr_cnt", test_chr_cnt },
        { "test_is_chr", test_is_chr },
        { "test_not_sane", test_not_sane },
        { "test_not_sane_lookup", test_not_sane_lookup },
        { "test_errtxt", test_errtxt },
        { "test_errtxtf", test_errtxtf },
        { "test_cnt_digits", test_cnt_digits },
        { "test_is_valid_utf8", test_is_valid_utf8 },
        { "test_utf8_to_unicode", test_utf8_to_unicode },
        { "test_hrt_cpy_iso8859_1", test_hrt_cpy_iso8859_1 },
        { "test_hrt_cpy_nochk", test_hrt_cpy_nochk },
        { "test_hrt_cpy_cat_nochk", test_hrt_cpy_cat_nochk },
        { "test_hrt_printf_nochk", test_hrt_printf_nochk },
        { "test_hrt_conv_gs1_brackets_nochk", test_hrt_conv_gs1_brackets_nochk },
        { "test_ct_cpy_segs", test_ct_cpy_segs },
        { "test_ct_cpy", test_ct_cpy },
        { "test_ct_cpy_iso8859_1", test_ct_cpy_iso8859_1 },
        { "test_ct_printf_256", test_ct_printf_256 },
        { "test_set_height", test_set_height },
        { "test_debug_test_codeword_dump_int", test_debug_test_codeword_dump_int },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
