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
        char *data;
        int length;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", -1, 0 },
        /*  1*/ { "1234", -1, 1234 },
        /*  2*/ { "-1234", -1, -1 },
        /*  3*/ { "A123A", -1, -1 },
        /*  4*/ { " ", -1, -1 },
        /*  5*/ { "999999999", -1, 999999999 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_to_int");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = to_int((const unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_to_upper(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int length;
        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", -1, "" },
        /*  1*/ { "abcefghijklmnopqrstuvwxyz", -1, "ABCEFGHIJKLMNOPQRSTUVWXYZ" },
        /*  2*/ { ".a[b`cA~B\177C;\200", -1, ".A[B`CA~B\177C;\200" },
        /*  3*/ { "é", -1, "é" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length;

    unsigned char buf[512];

    testStart("test_to_upper");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        buf[0] = '\0';
        memcpy(buf, data[i].data, length);
        buf[length] = '\0';

        to_upper(buf, length);
        assert_zero(strcmp((const char *) buf, data[i].expected), "i:%d strcmp(%s,%s) != 0\n", i, buf, data[i].expected);
    }

    testFinish();
}

static void test_chr_cnt(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int length;
        char c;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", -1, 'a', 0 },
        /*  1*/ { "BDAaED", -1, 'a', 1 },
        /*  1*/ { "aBDAaaaEaDa", -1, 'a', 6 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_chr_cnt");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = chr_cnt((const unsigned char *) data[i].data, length, data[i].c);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_is_sane(const testCtx *const p_ctx) {

    struct item {
        unsigned int flg;
        char *data;
        int length;
        int ret;

        char *orig_test;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { IS_SPC_F, " ", -1, 1, " " },
        /*  1*/ { IS_SPC_F, "\000", 1, 0, " " },
        /*  2*/ { IS_HSH_F, "#", -1, 1, "#" },
        /*  3*/ { IS_HSH_F, " ", -1, 0, "#" },
        /*  4*/ { IS_AST_F, "*", -1, 1, "*" },
        /*  5*/ { IS_AST_F, " ", -1, 0, "*" },
        /*  6*/ { IS_PLS_F, "+", -1, 1, "+" },
        /*  7*/ { IS_PLS_F, " ", -1, 0, "+" },
        /*  8*/ { IS_MNS_F, "-", -1, 1, "-" },
        /*  9*/ { IS_MNS_F, " ", -1, 0, "-" },
        /* 10*/ { IS_NUM_F, "0123456789", -1, 1, "0123456789" }, /* NEON */
        /* 11*/ { IS_NUM_F, "0123456789 ", -1, 0, "0123456789" },
        /* 12*/ { IS_NUM_F, "012345678A9", -1, 0, "0123456789" },
        /* 13*/ { IS_UPO_F, "GHIJKLMNOPQRSTUVWYZ", -1, 1, "GHIJKLMNOPQRSTUVWYZ" },
        /* 14*/ { IS_UPO_F, "FGHIJKLMNOPQRSTUVWYZ", -1, 0, "GHIJKLMNOPQRSTUVWYZ" },
        /* 15*/ { IS_LWO_F, "ghijklmnopqrstuvwyz", -1, 1, "ghijklmnopqrstuvwyz" },
        /* 16*/ { IS_LWO_F, "fghijklmnopqrstuvwyz", -1, 0, "ghijklmnopqrstuvwyz" },
        /* 17*/ { IS_UHX_F, "ABCDEF", -1, 1, "ABCDEF" },
        /* 18*/ { IS_UHX_F, "ABCDEf", -1, 0, "ABCDEF" },
        /* 19*/ { IS_LHX_F, "abcdef", -1, 1, "abcdef" },
        /* 20*/ { IS_LHX_F, "abcdeF", -1, 0, "abcdef" },
        /* 21*/ { IS_UPR_F, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 22*/ { IS_UPR_F, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ", -1, 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 23*/ { IS_UPR_F, "X", -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 24*/ { IS_UPR_F, "x", -1, 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 25*/ { IS_LWR_F, "abcdefghijklmnopqrstuvwxyz", -1, 1, "abcdefghijklmnopqrstuvwxyz" },
        /* 26*/ { IS_LWR_F, "abcdefghijklmnopqrstuvwxyz ", -1, 0, "abcdefghijklmnopqrstuvwxyz" },
        /* 27*/ { IS_LWR_F, "x", -1, 1, "abcdefghijklmnopqrstuvwxyz" },
        /* 28*/ { IS_LWR_F, "X", -1, 0, "abcdefghijklmnopqrstuvwxyz" },
        /* 29*/ { IS_UX__F, "X", -1, 1, "X" },
        /* 30*/ { IS_UX__F, "x", -1, 0, "X" },
        /* 31*/ { IS_LX__F, "x", -1, 1, "x" },
        /* 32*/ { IS_LX__F, "X", -1, 0, "x" },
        /* 33*/ { IS_C82_F, "!\"%&'(),./:;<=>?_", -1, 1, "!\"%&'(),./:;<=>?_" }, /* CSET82 punctuation less "*+-" */
        /* 34*/ { IS_C82_F, "!\"%&'(),./:;<=>?_ ", -1, 0, "!\"%&'(),./:;<=>?_" },
        /* 35*/ { IS_C82_F, "-", -1, 0, "!\"%&'(),./:;<=>?_" },
        /* 36*/ { IS_C82_F, "$", -1, 0, "!\"%&'(),./:;<=>?_" },
        /* 37*/ { IS_SIL_F, ".$/%", -1, 1, ".$/%" }, /* SILVER punctuation less " +-" */
        /* 38*/ { IS_SIL_F, ".$/% " , -1, 0, ".$/%" },
        /* 39*/ { IS_SIL_F, "-", -1, 0, ".$/%" },
        /* 40*/ { IS_CLI_F, "$:/.", -1, 1, "$:/." }, /* CALCIUM INNER punctuation less "+-" */
        /* 41*/ { IS_CLI_F, "$:/. ", -1, 0, "$:/." },
        /* 42*/ { IS_CLI_F, "+", -1, 0, "$:/." },
        /* 43*/ { IS_ARS_F, "ABCDEFGHJKLMNPRSTUVWXYZ", -1, 1, "ABCDEFGHJKLMNPRSTUVWXYZ" }, /* ARSENIC uppercase */
        /* 44*/ { IS_ARS_F, "ABCDEFGHJKLMNPRSTUVWXYZ ", -1, 0, "ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 45*/ { IS_ARS_F, "I", -1, 0, "ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 46*/ { IS_ARS_F, "O", -1, 0, "ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 47*/ { IS_NUM_F | IS_UHX_F, "0123456789ABCDEF", -1, 1, "0123456789ABCDEF" }, /* SSET */
        /* 48*/ { IS_NUM_F | IS_UHX_F, "0123456789ABCDEf", -1, 0, "0123456789ABCDEF" },
        /* 49*/ { IS_NUM_F | IS_PLS_F, "0123456789+", -1, 1, "0123456789+" }, /* SODIUM_PLS */
        /* 50*/ { IS_NUM_F | IS_PLS_F, "0123456789+-", -1, 0, "0123456789+" },
        /* 51*/ { IS_NUM_F | IS_UX__F, "0123456789X", -1, 1, "0123456789X" }, /* ISBNX_SANE */
        /* 52*/ { IS_NUM_F | IS_UX__F, "0123456789x", -1, 0, "0123456789X" },
        /* 53*/ { IS_NUM_F | IS_UX__F | IS_LX__F | IS_PLS_F, "0123456789Xx+", -1, 1, "0123456789Xx+" }, /* ISBNX_ADDON_SANE */
        /* 54*/ { IS_NUM_F | IS_UX__F | IS_LX__F | IS_PLS_F, "0123456789Xx+Y", -1, 0, "0123456789Xx+" },
        /* 55*/ { IS_NUM_F | IS_MNS_F, "0123456789-", -1, 1, "0123456789-" }, /* SODIUM_MNS */
        /* 56*/ { IS_NUM_F | IS_MNS_F, "0123456789-+", -1, 0, "0123456789-" },
        /* 57*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", -1, 1, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" }, /* CSET82 */
        /* 58*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, " ", -1, 0, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" },
        /* 59*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, "#", -1, 0, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" },
        /* 60*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, "$", -1, 0, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" },
        /* 61*/ { IS_C82_F | IS_AST_F | IS_MNS_F | IS_PLS_F | IS_NUM_F | IS_UPR_F | IS_LWR_F, "@", -1, 0, "!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz" },
        /* 62*/ { IS_LWR_F | IS_C82_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SPC_F, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ ", -1, 1, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ " }, /* IS_ISOIEC_F */
        /* 63*/ { IS_LWR_F | IS_C82_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SPC_F, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ #", -1, 0, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ " },
        /* 64*/ { IS_LWR_F | IS_C82_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SPC_F, "$", -1, 0, "abcdefghijklmnopqrstuvwxyz!\"%&'()*+,-./:;<=>?_ " },
        /* 65*/ { IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F, "-. $/+%", -1, 1, "" },
        /* 66*/ { IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F, "-. $/!+%", -1, 0, "" },
        /* 67*/ { IS_NUM_F | IS_UPR_F | IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%", -1, 1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%" }, /* SILVER */
        /* 68*/ { IS_NUM_F | IS_UPR_F | IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%a", -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%" },
        /* 69*/ { IS_NUM_F | IS_ARS_F, "0123456789ABCDEFGHJKLMNPRSTUVWXYZ", -1, 1, "0123456789ABCDEFGHJKLMNPRSTUVWXYZ" }, /* ARSENIC */
        /* 70*/ { IS_NUM_F | IS_ARS_F, "0123456789ABCDEFGHJKLMNPQRSTUVWXYZ", -1, 0, "0123456789ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 71*/ { IS_NUM_F | IS_ARS_F, "0123456789ABCDEFGHJKLMNPRSTUVWXYz", -1, 0, "0123456789ABCDEFGHJKLMNPRSTUVWXYZ" },
        /* 72*/ { IS_NUM_F | IS_UPR_F | IS_LWR_F | IS_SPC_F | IS_HSH_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #", -1, 1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #" }, /* GDSET */
        /* 73*/ { IS_NUM_F | IS_UPR_F | IS_LWR_F | IS_SPC_F | IS_HSH_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #!", -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #" },
        /* 74*/ { IS_NUM_F | IS_MNS_F | IS_CLI_F | IS_PLS_F, "0123456789-$:/.+", -1, 1, "0123456789-$:/.+" }, /* CALCIUM_INNER */
        /* 75*/ { IS_NUM_F | IS_MNS_F | IS_CLI_F | IS_PLS_F, "0123456789-$:/.+ ", -1, 0, "0123456789-$:/.+" },
        /* 76*/ { IS_NUM_F | IS_MNS_F | IS_CLI_F | IS_PLS_F, "0123456789-$:/.+!", -1, 0, "0123456789-$:/.+" },
        /* 77*/ { IS_NUM_F | IS_UPR_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, /* KRSET */
        /* 78*/ { IS_NUM_F | IS_UPR_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYz", -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 79*/ { IS_NUM_F | IS_UPR_F | IS_SPC_F, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ ", -1, 1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ " }, /* RUBIDIUM */
        /* 80*/ { IS_NUM_F | IS_UPR_F | IS_SPC_F, "0123456789aBCDEFGHIJKLMNOPQRSTUVWXYZ ", -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ " },
        /* 81*/ { IS_NUM_F | IS_MNS_F | IS_UPR_F, "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 1, "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, /* SHKASUTSET */
        /* 82*/ { IS_NUM_F | IS_MNS_F | IS_UPR_F, "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYz", -1, 0, "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
        /* 83*/ { IS_NUM_F | IS_UPR_F | IS_SPC_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SIL_F | IS_CLI_F, "1234567890 $%*+-./:ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 1, "1234567890 $%*+-./:ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, /* QR_ALPHA */
        /* 84*/ { IS_NUM_F | IS_UPR_F | IS_SPC_F | IS_AST_F | IS_PLS_F | IS_MNS_F | IS_SIL_F | IS_CLI_F, "1234567890 $%*+-./:ABCDEFGHIJKLMNOPQRSTUVWXYz", -1, 0, "1234567890 $%*+-./:ABCDEFGHIJKLMNOPQRSTUVWXYZ" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, j, length, ret;

    testStart("test_is_sane");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = is_sane(data[i].flg, (const unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);

        if (data[i].orig_test[0]) {
            int orig_ret = is_sane_orig(data[i].orig_test, (const unsigned char *) data[i].data, length);
            if (orig_ret == 0) {
                assert_nonzero(ret, "i:%d orig_ret %d, ret %d == 0\n", i, orig_ret, ret);
            } else {
                assert_zero(ret, "i:%d orig_ret %d, ret %d != 0\n", i, orig_ret, ret);
            }
        }

        ret = 1;
        for (j = 0; j < length; j++) {
            if (!is_chr(data[i].flg, data[i].data[j])) {
                ret = 0;
                break;
            }
        }
        assert_equal(ret, data[i].ret, "i:%d is_chr() ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_is_sane_lookup(const testCtx *const p_ctx) {

    struct item {
        char *test_string;
        int test_length;
        char *data;
        int length;
        int ret;

        int posns[32];
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "1234567", -1, "7654321357", -1, 1, { 6, 5, 4, 3, 2, 1, 0, 2, 4, 6 } },
        /*  1*/ { "1234567", -1, "76543213578", -1, 0, {0} },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    int test_length;
    int posns[32];

    testStart("test_is_sane_lookup");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        test_length = data[i].test_length == -1 ? (int) strlen(data[i].test_string) : data[i].test_length;
        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = is_sane_lookup(data[i].test_string, test_length, (const unsigned char *) data[i].data, length, posns);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);

        if (ret) {
            int j;
            for (j = 0; j < length; j++) {
                assert_equal(posns[j], data[i].posns[j], "i:%d posns[%d] %d != expected posns[%d] %d\n", i, j, posns[j], j, data[i].posns[j]);
            }
        }
    }

    testFinish();
}

static void test_cnt_digits(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int length;
        int position;
        int max;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", -1, 0, -1, 0 },
        /*  1*/ { "asdf1", -1, 0, -1, 0 },
        /*  2*/ { "asdf1asdf", -1, 4, -1, 1 },
        /*  3*/ { "a12345asdf", -1, 1, -1, 5 },
        /*  4*/ { "a12345asdf", -1, 1, 4, 4},
        /*  5*/ { "a1234", -1, 1, 5, 4},
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_cnt_digits");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = cnt_digits((const unsigned char *) data[i].data, length, data[i].position, data[i].max);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_is_valid_utf8(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int length;
        int ret;
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
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
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_is_valid_utf8");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = is_valid_utf8((const unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_utf8_to_unicode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        int length;
        int disallow_4byte;
        int ret;
        int ret_length;
        unsigned int expected_vals[20];
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", -1, 1, 0, 0, {0}, "" },
        /*  1*/ { "\000a\302\200\340\240\200", 7, 1, 0, 4, { 0, 'a', 0x80, 0x800 }, "NUL a C280 E0A080" },
        /*  2*/ { "\357\277\277", -1, 1, 0, 1, { 0xFFFF }, "EFBFBF" },
        /*  3*/ { "\360\220\200\200", -1, 1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Four-byte F0908080" },
        /*  4*/ { "a\200b", -1, 1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Orphan continuation 0x80" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    unsigned int vals[20];
    struct zint_symbol symbol = {0};

    testStart("test_utf8_to_unicode");

    symbol.debug = debug;

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = utf8_to_unicode(&symbol, (unsigned char *) data[i].data, vals, &ret_length, data[i].disallow_4byte);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < ret_length; j++) {
                assert_equal(vals[j], data[i].expected_vals[j], "i:%d vals[%d] %04X != %04X\n", i, j, vals[j], data[i].expected_vals[j]);
            }
        }
    }

    testFinish();
}

/* Note transferred from "test_code128.c" */
static void test_hrt_cpy_iso8859_1(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        int length;
        int ret;
        char *expected;
        char *comment;
    };
    /*
       NBSP U+00A0 (\240, 160), UTF-8 C2A0 (\302\240)
       ¡ U+00A1 (\241, 161), UTF-8 C2A1 (\302\241)
       é U+00E9 (\351, 233), UTF-8 C3A9
    */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", -1, 0, "", "" },
        /*  1*/ { "abc", -1, 0, "abc", "" },
        /*  2*/ { "\000A\001B\002\036\037C ~\177", 11, 0, " A B   C ~ ", "" },
        /*  3*/ { "~\177\200\201\237\240", -1, 0, "~    \302\240", "" },
        /*  4*/ { "\241\242\243\244\257\260", -1, 0, "¡¢£¤¯°", "" },
        /*  5*/ { "\276\277\300\337\377", -1, 0, "¾¿Àßÿ", "" },
        /*  6*/ { "\351", -1, 0, "é", "" },
        /*  7*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 0, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "99 \241" },
        /*  8*/ { "a\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 0, "a¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "a + 99 \241" },
        /*  9*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241a", -1, 0, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡a", "99 \241 + a" },
        /* 10*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 1, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "100 \241 (truncated)" },
        /* 11*/ { "a\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 1, "a¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "a + 100 \241 (truncated)" },
        /* 12*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241a", -1, 1, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "100 \241 + a (truncated)" },
        /* 13*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 1, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "101 \241 (truncated)" },
        /* 14*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 0, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "99 \351" },
        /* 15*/ { "a\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 0, "aééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "a + 99 \351" },
        /* 16*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351a", -1, 0, "éééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééa", "99 \351 + a" },
        /* 17*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 1, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "100 \351 (truncated)" },
        /* 18*/ { "a\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 1, "aééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "a + 100 \351 (truncated)" },
        /* 19*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351a", -1, 1, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "100 \351 + a (truncated)" },
        /* 20*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 1, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "101 \351 (truncated)" },
        /* 21*/ { "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "200 A (truncated)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};

    testStart("test_hrt_cpy_iso8859_1");

    symbol.debug = debug;

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = hrt_cpy_iso8859_1(&symbol, (unsigned char *) data[i].data, length);
        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            for (j = 0; j < ret; j++) {
                fprintf(stderr, "symbol.text[%d] %2X\n", j, symbol.text[j]);
            }
        }
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_nonzero(testUtilIsValidUTF8(symbol.text, (int) ustrlen(symbol.text)), "i:%d testUtilIsValidUTF8(%s) != 1\n", i, symbol.text);
        assert_zero(strcmp((char *) symbol.text, data[i].expected), "i:%d symbol.text (%s) != expected (%s)\n", i, symbol.text, data[i].expected);
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
        char *expected_errtxt;
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 0, { 0 }, 0, 0, 0, 0, 0, 0, 0.5, "", "" },
        /*  1*/ { 2, { 1, 1 }, 2, 0, 0, 0, 0, 0, 2, "", "zero_count == 0, fixed height only" },
        /*  2*/ { 2, { 1, 1 }, 2, 0, 0, 1, 1, ZINT_WARN_NONCOMPLIANT, 2, "", "zero_count == 0, height < max height" },
        /*  3*/ { 2, { 1, 1 }, 2, 0, 0, 1, 0, ZINT_WARN_NONCOMPLIANT, 2, "248: Height not compliant with standards", "zero_count == 0, height < max height" },
        /*  4*/ { 2, { 2, 0 }, 2, 0, 0, 0, 0, 0, 2.5, "", "zero_count != 0, height 2" },
        /*  5*/ { 2, { 2, 0 }, 2, 1, 0, 0, 1, ZINT_WARN_NONCOMPLIANT, 2.5, "", "zero_count != 0, row_height < min_row_height" },
        /*  6*/ { 2, { 2, 0 }, 2, 1, 0, 0, 0, ZINT_WARN_NONCOMPLIANT, 2.5, "247: Height not compliant with standards", "zero_count != 0, row_height < min_row_height" },
        /*  7*/ { 2, { 2, 0 }, 0, 0, 20, 0, 0, 0, 22, "", "zero_count != 0, default_height 20" },
        /*  8*/ { 2, { 2, 0 }, 20, 0, 20, 0, 0, 0, 20, "", "zero_count != 0, height 20" },
        /*  9*/ { 2, { 2, 0 }, 0, 2, 0, 0, 0, 0, 4, "", "zero_count != 0, min_row_height 2" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, ret;

    struct zint_symbol symbol;

    testStart("set_height");

    symbol.debug = debug;

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        memset(&symbol, 0, sizeof(symbol));
        symbol.rows = data[i].rows;
        for (j = 0; j < ARRAY_SIZE(data[i].row_height); j++) {
            symbol.row_height[j] = data[i].row_height[j];
        }
        symbol.height = data[i].height;

        ret = set_height(&symbol, data[i].min_row_height, data[i].default_height, data[i].max_height, data[i].no_errtxt);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_equal(symbol.height, data[i].expected_height, "i:%d symbol.height %g != %g\n", i, symbol.height, data[i].expected_height);
        assert_zero(strcmp(symbol.errtxt, data[i].expected_errtxt), "i:%d errtxt %s != %s\n", i, symbol.errtxt, data[i].expected_errtxt);
    }

    testFinish();
}

INTERNAL void debug_test_codeword_dump_int(struct zint_symbol *symbol, const int *codewords, const int length);

static void test_debug_test_codeword_dump_int(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int codewords[50];
        int length;
        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { { 2147483647, -2147483646, 2147483647, 0, 2147483647, 2147483647, 2147483647, 2147483647, 123456 }, 10, "(10) 2147483647 -2147483646 2147483647 0 2147483647 2147483647 2147483647 2147483647 123456" },
        /*  1*/ { { 2147483647, -2147483646, 2147483647, 0, 2147483647, 2147483647, 2147483647, 2147483647, 1234567 }, 10, "(10) 2147483647 -2147483646 2147483647 0 2147483647 2147483647 2147483647 2147483647" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    struct zint_symbol symbol = {0};

    testStart("test_debug_test_codeword_dump_int");

    symbol.debug = debug;

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        debug_test_codeword_dump_int(&symbol, data[i].codewords, data[i].length);
        assert_nonzero(strlen(symbol.errtxt) < 92, "i:%d strlen(%s) >= 92 (%d)\n", i, symbol.errtxt, (int) strlen(symbol.errtxt));
        assert_zero(strcmp(symbol.errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0 (%d, %d)\n", i, symbol.errtxt, data[i].expected, (int) strlen(symbol.errtxt), (int) strlen(data[i].expected));
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_chr_cnt", test_chr_cnt },
        { "test_to_int", test_to_int },
        { "test_to_upper", test_to_upper },
        { "test_is_sane", test_is_sane },
        { "test_is_sane_lookup", test_is_sane_lookup },
        { "test_cnt_digits", test_cnt_digits },
        { "test_is_valid_utf8", test_is_valid_utf8 },
        { "test_utf8_to_unicode", test_utf8_to_unicode },
        { "test_hrt_cpy_iso8859_1", test_hrt_cpy_iso8859_1 },
        { "test_set_height", test_set_height },
        { "test_debug_test_codeword_dump_int", test_debug_test_codeword_dump_int },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
