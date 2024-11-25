/*
    libzint - the open source barcode library
    Copyright (C) 2019-2024 Robin Stuart <rstuart114@gmail.com>

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

static void test_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_1;
        int option_2;
        char *pattern;
        int length;
        char *primary;
        int ret;
        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, "1", 138, "", 0, 33, 30, 1, "Mode 4 (138 agrees with ISO/IEC 16023:2000)" },
        /*  1*/ { -1, -1, "1", 139, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /*  2*/ { -1, -1, "1", 144, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /*  3*/ { -1, -1, "1", 145, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "Absolute max" },
        /*  4*/ { -1, -1, "A", 93, "", 0, 33, 30, 1, "" },
        /*  5*/ { -1, -1, "A", 94, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /*  6*/ { -1, -1, "\001", 91, "", 0, 33, 30, 1, "" },
        /*  7*/ { -1, -1, "\001", 92, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /*  8*/ { -1, -1, "\200", 91, "", 0, 33, 30, 1, "" },
        /*  9*/ { -1, -1, "\200", 92, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 10*/ { 2, -1, "1", 126, "123456789123123", 0, 33, 30, 1, "" },
        /* 11*/ { 2, -1, "1", 127, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 12*/ { 2, -1, "A", 84, "123456789123123", 0, 33, 30, 1, "" },
        /* 13*/ { 2, -1, "A", 85, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 14*/ { 2, 96 + 1, "1", 109, "123456789123123", 0, 33, 30, 1, "" },
        /* 15*/ { 2, 96 + 1, "1", 110, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 16*/ { 2, 96 + 1, "1", 136, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "Absolute max with SCM vv" },
        /* 17*/ { 3, -1, "1", 126, "ABCDEF123123", 0, 33, 30, 1, "" },
        /* 18*/ { 3, -1, "1", 127, "ABCDEF123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 19*/ { 3, -1, "A", 84, "ABCDEF123123", 0, 33, 30, 1, "" },
        /* 20*/ { 3, -1, "A", 85, "ABCDEF123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 21*/ { 3, 96 + 1, "1", 109, "ABCDEF123123", 0, 33, 30, 1, "" },
        /* 22*/ { 3, 96 + 1, "1", 110, "ABCDEF123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 23*/ { 0, -1, "1", 126, "123456789123123", 0, 33, 30, 1, "Mode 2" },
        /* 24*/ { 0, -1, "1", 127, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 25*/ { 0, -1, "1", 126, "ABCDEF123123", 0, 33, 30, 1, "Mode 3" },
        /* 26*/ { 0, -1, "1", 127, "ABCDEF123123", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 27*/ { 5, -1, "1", 113, "", 0, 33, 30, 1, "Extra EEC" },
        /* 28*/ { 5, -1, "1", 114, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 29*/ { 5, -1, "A", 77, "", 0, 33, 30, 1, "" },
        /* 30*/ { 5, -1, "A", 78, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 31*/ { 6, -1, "1", 138, "", 0, 33, 30, 1, "" },
        /* 32*/ { 6, -1, "1", 139, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
        /* 33*/ { 6, -1, "A", 93, "", 0, 33, 30, 1, "" },
        /* 34*/ { 6, -1, "A", 94, "", ZINT_ERROR_TOO_LONG, -1, -1, 1, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[256];
    char escaped[1024];
    char cmp_buf[32768];
    char cmp_msg[1024];
    const char expected_errtxt[] = "Error 553: Input too long, requires too many codewords (maximum 144)";

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_MAXICODE, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);
        strcpy(symbol->primary, data[i].primary);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        } else {
            assert_zero(strcmp(symbol->errtxt, expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, expected_errtxt);
        }

        if (ret < ZINT_ERROR) {
            if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, debug)) {
                if (!data[i].bwipp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    char modules_dump[33 * 33 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data_buf, length, symbol->primary, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                }
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data_buf, length, debug)) {
                int cmp_len, ret_len;
                char modules_dump[33 * 33 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilZXingCPP(i, symbol, data_buf, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data_buf, length, symbol->primary, escaped, &ret_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        struct zint_structapp structapp;
        char *data;
        int length;
        char *primary;
        int ret;
        int expected_width;
        char *expected;
        int bwipp_cmp;
        int zxingcpp_cmp;
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 01 21 21 21 21 21 21 21 21 08 0E 19 2B 20 0C 24 06 32 1C 21 21 21 21 21 21 21 21", 1, 1, "" },
        /*  1*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_DATA, 0, "Error 548: Primary Message empty", 1, 1, "" },
        /*  2*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "A123456", ZINT_ERROR_INVALID_DATA, 0, "Error 555: Non-numeric postcode in Primary Message", 1, 1, "" },
        /*  3*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "1123456", 0, 30, "(144) 12 00 00 00 00 10 30 1E 20 1C 1A 3D 1C 0D 1B 15 3C 17 3C 08 01 21 21 21 21 21 21 21", 1, 1, "1-digit postcode" },
        /*  4*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "1 123456", 0, 30, "(144) 12 00 00 00 00 10 30 1E 20 1C 1A 3D 1C 0D 1B 15 3C 17 3C 08 01 21 21 21 21 21 21 21", 1, 0, "1-digit postcode; ZXing-C++ test can't handle space" },
        /*  5*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "123456789123456", 0, 30, "(144) 12 05 0D 2F 35 11 32 1E 20 1C 0D 1D 3B 12 22 3F 30 14 23 1A 01 21 21 21 21 21 21 21", 1, 1, "9-digit postcode" },
        /*  6*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "1234567890123456", ZINT_ERROR_INVALID_DATA, 0, "Error 551: Primary Message length 16 wrong (7 to 15 only)", 1, 1, "10-digit postcode" },
        /*  7*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "1123456", 0, 30, "(144) 12 00 00 00 00 10 30 1E 20 1C 1A 3D 1C 0D 1B 15 3C 17 3C 08 01 21 21 21 21 21 21 21", 1, 1, "1-digit postcode" },
        /*  8*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "123456789123456", 0, 30, "(144) 12 05 0D 2F 35 11 32 1E 20 1C 0D 1D 3B 12 22 3F 30 14 23 1A 01 21 21 21 21 21 21 21", 1, 1, "9-digit postcode" },
        /*  9*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "1234567890123456", ZINT_ERROR_INVALID_DATA, 0, "Error 551: Primary Message length 16 wrong (7 to 15 only)", 1, 1, "10-digit postcode" },
        /* 10*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "123456", ZINT_ERROR_INVALID_DATA, 0, "Error 551: Primary Message length 6 wrong (7 to 15 only)", 1, 1, "0-digit postcode" },
        /* 11*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "12345678123456", 0, 30, "(144) 22 13 21 31 0B 00 32 1E 20 1C 04 14 07 30 10 07 08 28 1D 09 01 21 21 21 21 21 21 21", 1, 1, "8-digit postcode" },
        /* 12*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "12345678123456", 0, 30, "(144) 22 13 21 31 0B 00 32 1E 20 1C 04 14 07 30 10 07 08 28 1D 09 01 21 21 21 21 21 21 21", 1, 1, "8-digit postcode" },
        /* 13*/ { UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_DATA, 0, "Error 548: Primary Message empty", 1, 1, "" },
        /* 14*/ { UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, "A", -1, "A123456", 0, 30, "(144) 03 08 08 08 08 18 30 1E 20 1C 22 35 1C 0F 02 1A 26 04 10 31 01 21 21 21 21 21 21 21", 1, 1, "1-alphanumeric postcode" },
        /* 15*/ { UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, "A", -1, "1123456", 0, 30, "(144) 03 08 08 08 08 18 3C 1E 20 1C 13 37 07 2C 26 2D 18 29 3F 2C 01 21 21 21 21 21 21 21", 1, 0, "1-digit postcode; ZXing-C++ test can't handle padding" },
        /* 16*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "A123456", 0, 30, "(144) 03 08 08 08 08 18 30 1E 20 1C 22 35 1C 0F 02 1A 26 04 10 31 01 21 21 21 21 21 21 21", 1, 1, "1-alphanumeric postcode" },
        /* 17*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "ABCDEF123456", 0, 30, "(144) 23 11 01 31 20 10 30 1E 20 1C 3C 1D 22 03 19 15 0F 20 0F 2A 01 21 21 21 21 21 21 21", 1, 1, "6-alphanumeric postcode" },
        /* 18*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "ABCDEFG123456", 0, 30, "(144) 23 11 01 31 20 10 30 1E 20 1C 3C 1D 22 03 19 15 0F 20 0F 2A 01 21 21 21 21 21 21 21", 1, 0, "7-alphanumeric postcode truncated; ZXing-C++ test can't handle truncation" },
        /* 19*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "ABCDE123456", 0, 30, "(144) 03 18 01 31 20 10 30 1E 20 1C 0F 38 38 1A 39 10 2F 37 22 12 01 21 21 21 21 21 21 21", 1, 1, "5-alphanumeric postcode" },
        /* 20*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "AAAAAA   840001", 0, 30, "(144) 13 10 10 10 10 10 00 12 07 00 17 36 2E 38 04 29 16 0D 27 16 01 21 21 21 21 21 21 21", 1, 0, "6-alphanumeric postcode with padding; ZXing-C++ test can't handle padding" },
        /* 21*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "AAAAA A840001", 0, 30, "(144) 03 18 10 10 10 10 00 12 07 00 19 07 29 31 26 01 23 2C 2E 07 01 21 21 21 21 21 21 21", 1, 0, "7-alphanumeric with embedded padding truncated; ZXing-C++ test can't handle truncation" },
        /* 22*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "AA\015AAA840001", ZINT_ERROR_INVALID_DATA, 0, "Error 556: Invalid character in postcode in Primary Message", 1, 1, "Alphanumeric postcode with CR" },
        /* 23*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "A#%-/A840001", 0, 30, "(144) 13 30 1B 1B 39 18 00 12 07 00 3F 1E 25 07 2A 1E 14 3C 28 2D 01 21 21 21 21 21 21 21", 1, 1, "Alphanumeric postcode with non-control Code A chars" },
        /* 24*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "1A23456", ZINT_ERROR_INVALID_DATA, 0, "Error 552: Non-numeric country code or service class in Primary Message", 1, 1, "Non-numeric country code" },
        /* 25*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "12345678912345A", ZINT_ERROR_INVALID_DATA, 0, "Error 552: Non-numeric country code or service class in Primary Message", 1, 1, "Non-numeric service class" },
        /* 26*/ { UNICODE_MODE, -1, 0, -1, { 0, 0, "" }, "A", -1, "123456789123456", 0, 30, "(144) 12 05 0D 2F 35 11 32 1E 20 1C 0D 1D 3B 12 22 3F 30 14 23 1A 01 21 21 21 21 21 21 21", 1, 1, "Auto-determine mode 2" },
        /* 27*/ { UNICODE_MODE, -1, 0, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_DATA, 0, "Error 554: Primary Message empty", 1, 1, "Auto-determine mode 2/3 requires primary message" },
        /* 28*/ { UNICODE_MODE, -1, 0, -1, { 0, 0, "" }, "A", -1, "A23456123456", 0, 30, "(144) 23 1D 0D 3D 2C 1C 30 1E 20 1C 24 35 30 31 2A 0D 17 14 16 3D 01 21 21 21 21 21 21 21", 1, 1, "Auto-determine mode 3" },
        /* 29*/ { UNICODE_MODE, -1, -1, 100, { 0, 0, "" }, "A", -1, "123456123456", 0, 30, "(144) 02 10 22 07 00 20 31 1E 20 1C 0E 29 13 1B 0D 26 36 25 3B 22 3B 2A 29 3B 28 1E 30 31", 1, 1, "SCM prefix version" },
        /* 30*/ { UNICODE_MODE, -1, -1, 101, { 0, 0, "" }, "A", -1, "123456123456", ZINT_ERROR_INVALID_OPTION, 0, "Error 557: SCM prefix version '101' out of range (1 to 100)", 1, 1, "SCM prefix version" },
        /* 31*/ { UNICODE_MODE, 25, -1, 96 + 1, { 0, 0, "" }, "A", -1, "123456123456", ZINT_ERROR_INVALID_OPTION, 0, "Error 547: SCM prefix can not be used with ECI 25 (ECI must be ASCII compatible)", 1, 1, "SCM prefix version UTF-16BE" },
        /* 32*/ { UNICODE_MODE, 33, -1, 96 + 1, { 0, 0, "" }, "A", -1, "123456123456", ZINT_ERROR_INVALID_OPTION, 0, "Error 547: SCM prefix can not be used with ECI 33 (ECI must be ASCII compatible)", 1, 1, "SCM prefix version UTF-16LE" },
        /* 33*/ { UNICODE_MODE, 34, -1, 96 + 1, { 0, 0, "" }, "A", -1, "123456123456", ZINT_ERROR_INVALID_OPTION, 0, "Error 547: SCM prefix can not be used with ECI 34 (ECI must be ASCII compatible)", 1, 1, "SCM prefix version UTF-32BE" },
        /* 34*/ { UNICODE_MODE, 35, -1, 96 + 1, { 0, 0, "" }, "A", -1, "123456123456", ZINT_ERROR_INVALID_OPTION, 0, "Error 547: SCM prefix can not be used with ECI 35 (ECI must be ASCII compatible)", 1, 1, "SCM prefix version UTF-32LE" },
        /* 35*/ { UNICODE_MODE, 3, 2, 96 + 1, { 0, 0, "" }, "A", -1, "999999999840333", 0, 30, "(144) 32 3F 09 2B 39 1E 02 12 37 14 2F 1E 03 29 3E 1B 2D 2A 06 20 3B 2A 29 3B 28 1E 30 31", 1, 1, "SCM prefix version ISO/IEC 8859-1" },
        /* 36*/ { UNICODE_MODE, 170, 2, 96 + 1, { 0, 0, "" }, "A", -1, "1234567123456", 0, 30, "(144) 32 21 16 0B 01 30 31 1E 20 1C 13 22 04 28 0E 21 13 1E 3A 14 3B 2A 29 3B 28 1E 30 31", 1, 1, "SCM prefix version ASCII Invariant" },
        /* 37*/ { UNICODE_MODE, 20, 2, 96 + 1, { 0, 0, "" }, "テ", -1, "12345678123456", 0, 30, "(144) 22 13 21 31 0B 00 32 1E 20 1C 04 14 07 30 10 07 08 28 1D 09 3B 2A 29 3B 28 1E 30 31", 1, 1, "SCM prefix version Shift JIS" },
        /* 38*/ { UNICODE_MODE | ESCAPE_MODE, 3, 2, -1, { 0, 0, "" }, "[)>\\R01\\G96A", -1, "999999999840333", 0, 30, "(144) 32 3F 09 2B 39 1E 02 12 37 14 2F 1E 03 29 3E 1B 2D 2A 06 20 3B 2A 29 3B 28 1E 30 31", 1, 1, "Manual SCM prefix ISO/IEC 8859-1" },
        /* 39*/ { UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 03 01 21 21 21 21 21 21 2F 14 23 21 05 24 27 00 24 0C 21 21 21 21 21 21 21 21", 1, 1, "" },
        /* 40*/ { UNICODE_MODE, 31, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 1F 01 21 21 21 21 21 21 00 2F 0E 09 39 3B 24 1A 21 05 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x1F" },
        /* 41*/ { UNICODE_MODE, 32, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 20 20 01 21 21 21 21 21 3D 15 0F 30 0D 22 24 35 22 06 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x20" },
        /* 42*/ { UNICODE_MODE, 1023, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 2F 3F 01 21 21 21 21 21 2E 27 23 1D 35 19 21 04 3A 26 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x3FF" },
        /* 43*/ { UNICODE_MODE, 1024, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 30 10 00 01 21 21 21 21 11 2F 15 10 1D 29 06 35 14 2B 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x400" },
        /* 44*/ { UNICODE_MODE, 32767, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 37 3F 3F 01 21 21 21 21 3E 15 12 01 07 30 39 27 04 2B 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x7FFF" },
        /* 45*/ { UNICODE_MODE, 32768, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 38 08 00 00 01 21 21 21 10 30 3A 04 26 23 0E 21 3D 0F 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x8000" },
        /* 46*/ { UNICODE_MODE, 65535, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 38 0F 3F 3F 01 21 21 21 1C 0E 1D 39 3B 0D 38 25 00 30 21 21 21 21 21 21 21 21", 1, 1, "ECI 0xFFFF" },
        /* 47*/ { UNICODE_MODE, 65536, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 38 10 00 00 01 21 21 21 2B 1F 24 06 38 2E 17 1B 10 2F 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x10000" },
        /* 48*/ { UNICODE_MODE, 131071, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 38 1F 3F 3F 01 21 21 21 0F 05 09 04 2F 3A 17 09 36 31 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x1FFFF" },
        /* 49*/ { UNICODE_MODE, 999999, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 3B 34 08 3F 01 21 21 21 26 3B 2B 23 08 17 32 05 26 35 21 21 21 21 21 21 21 21", 1, 1, "Max ECI" },
        /* 50*/ { UNICODE_MODE, -1, 1, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 550: Mode '1' out of range (2 to 6)", 1, 1, "" },
        /* 51*/ { UNICODE_MODE, -1, 7, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 550: Mode '7' out of range (2 to 6)", 1, 1, "" },
        /* 52*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\015", -1, "", 0, 30, "(144) 04 00 21 21 21 21 21 21 21 21 37 32 10 01 24 1B 10 11 38 0C 21 21 21 21 21 21 21 21", 1, 1, "" },
        /* 53*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\015\034\035\036 ", -1, "", 0, 30, "(144) 04 00 1C 1D 1E 20 21 21 21 21 3E 18 0B 14 22 27 2D 3A 18 22 21 21 21 21 21 21 21 21", 1, 1, "Code Set A" },
        /* 54*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "abc\034\035\036 ,./:", -1, "", 0, 30, "(144) 04 3F 01 02 03 1C 1D 1E 2F 30 21 1F 2B 2A 35 0A 00 10 36 1D 31 32 33 21 21 21 21 21", 1, 1, "Code Set B" },
        /* 55*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\001\002\003\015\034\035\036 ", -1, "", 0, 30, "(144) 04 3E 3E 01 02 03 0D 20 21 22 3D 0F 31 3A 0C 0C 34 26 27 31 3B 1C 1C 1C 1C 1C 1C 1C", 1, 1, "Code Set E" },
        /* 56*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "ÀÀÀ\034\035\036 ", -1, "", 0, 30, "(144) 04 3C 3C 00 00 00 1C 1D 1E 3B 3C 05 39 07 18 15 25 36 28 11 3A 21 21 21 21 21 21 21", 1, 1, "Code Set C" },
        /* 57*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "ààà\034\035\036 ", -1, "", 0, 30, "(144) 04 3D 3D 00 00 00 1C 1D 1E 3B 06 26 23 19 32 1E 0C 1A 05 11 3A 21 21 21 21 21 21 21", 1, 1, "Code Set D" },
        /* 58*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\001\034\001\035\001\036\001a:b", -1, "", 0, 30, "(144) 04 3E 3E 01 20 01 21 01 22 01 27 0B 35 01 08 0D 16 02 17 1A 3F 01 33 02 21 21 21 21", 1, 1, "" },
        /* 59*/ { UNICODE_MODE, -1, -1, -1, { 1, 2, "" }, "A", -1, "", 0, 30, "(144) 04 21 01 01 21 21 21 21 21 21 09 0B 26 03 37 0E 25 27 07 1E 21 21 21 21 21 21 21 21", 1, 1, "" },
        /* 60*/ { UNICODE_MODE, -1, -1, -1, { 0, 2, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 559: Structured Append index '0' out of range (1 to count 2)", 1, 1, "" },
        /* 61*/ { UNICODE_MODE, -1, -1, -1, { 1, 1, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 558: Structured Append count '1' out of range (2 to 8)", 1, 1, "" },
        /* 62*/ { UNICODE_MODE, -1, -1, -1, { 1, 9, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 558: Structured Append count '9' out of range (2 to 8)", 1, 1, "" },
        /* 63*/ { UNICODE_MODE, -1, -1, -1, { 3, 2, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 559: Structured Append index '3' out of range (1 to count 2)", 1, 1, "" },
        /* 64*/ { UNICODE_MODE, -1, -1, -1, { 1, 2, "A" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 549: Structured Append ID not available for MaxiCode", 1, 1, "" },
        /* 65*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "b..A", -1, "", 0, 30, "(144) 04 3B 02 2E 2E 01 21 21 21 21 11 1C 30 14 2D 3E 16 0E 0C 31 21 21 21 21 21 21 21 21", 1, 1, "" },
        /* 66*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A123456789b123456789bbbA", -1, "", 0, 30, "(144) 04 01 1F 07 16 3C 34 15 3F 02 10 1B 28 22 20 32 37 0C 0B 2A 1F 07 16 3C 34 15 02 02", 1, 1, "" },
        /* 67*/ { ESCAPE_MODE, -1, -1, -1, { 0, 0, "" }, "\\d192\\d224\\d224\\d224\\d192\\d224\\d224\\d224\\d192\\d224\\d224\\d224\\d192\\d224\\d224\\d224\\d192\\d224\\d224\\d224\\d192", -1, "", 0, 30, "(144) 04 3C 00 3D 3D 00 00 00 3C 00 34 39 0E 35 1D 25 00 1B 28 03 00 00 00 3C 00 00 00 00", 1, 1, "BWIPP PR #279" },
        /* 68*/ { ESCAPE_MODE, -1, 2, -1, { 0, 0, "" }, "1Z34567890\\GUPSN\\G102562\\G034\\G\\G1/1\\G\\GY\\G2201 Second St\\GFt Myers\\GFL\\R\\E", -1, "339010000840001", 0, 30, "(144) 02 34 21 13 03 15 02 12 07 00 0C 03 00 38 24 04 0B 1F 2F 21 31 1A 33 34 35 36 37 38", 1, 1, "" },
        /* 69*/ { DATA_MODE | ESCAPE_MODE, -1, -1, -1, { 0, 0, "" }, "ABabcdeAabcdABCabcdABabc\\d192\\d192 \\d192\\d224\\d224\\d028\\d224\\d001\\d001\\d001\\d029\\d00112345678a123456789aABCDa\\d192\\d224\\d001\\d192\\d001\\d224\\d030\\d004", -1, "", 0, 30, "(144) 04 01 02 3F 01 02 03 04 05 3B 25 28 3F 32 0D 10 0D 0F 35 11 01 01 02 03 04 39 01 02", 1, 1, "Exercises all latches & no. of shifts" },
        /* 70*/ { UNICODE_MODE, 1023, 3, 96 + 1, { 2, 3, "" }, "A", -1, "P144275001", 0, 30, "(144) 03 08 08 0D 1D 0C 34 04 05 00 13 29 0C 17 0F 15 2E 38 00 0B 21 0A 3B 2A 29 3B 28 1E", 1, 1, "ECI, Structured Append, SCM" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_MAXICODE, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }
        strcpy(symbol->primary, data[i].primary);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, { %d, %d, \"%s\" }, \"%s\", %d, \"%s\", %s, %d, \"%s\", %d, %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, data[i].option_2,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length, data[i].primary,
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].bwipp_cmp, data[i].zxingcpp_cmp, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        char modules_dump[33 * 33 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, symbol->primary, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    if (!data[i].zxingcpp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[33 * 33 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, symbol->primary, escaped, &ret_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        struct zint_structapp structapp;
        char *data;
        int length;
        char *primary;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { -1, -1, -1, { 0, 0, "" }, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", -1, "", 0, 33, 30, 1, "ISO/IEC 16023:2000 Figure 2 (and L1), same",
                    "011111010000001000001000100111"
                    "000100000001000000001010000000"
                    "001011001100100110110010010010"
                    "100000010001100010010000000000"
                    "001011000000101000001010110011"
                    "111010001000001011001000111100"
                    "100000000110000010010000000000"
                    "000010100010010010001001111100"
                    "111011100000001000000110000000"
                    "000000011011000000010100011000"
                    "101111000001010110001100000011"
                    "001110001010000000111010001110"
                    "000111100000000000100001011000"
                    "100010000000000000000111001000"
                    "100000001000000000011000001000"
                    "000010111000000000000010000010"
                    "111000001000000000001000001101"
                    "011000000000000000001000100100"
                    "000000101100000000001001010001"
                    "101010001000000000100111001100"
                    "001000011000000000011100001010"
                    "000000000000000000110000100000"
                    "101011001010100001000101010001"
                    "100011110010101001101010001010"
                    "011010000000000101011010011111"
                    "000001110011111111111100010100"
                    "001110100111000101011000011100"
                    "110111011100100001101001010110"
                    "000001011011101010010111001100"
                    "111000110111100010001111011110"
                    "101111010111111000010110111001"
                    "001001101111101101101010011100"
                    "001011000000111101100100001000"
                },
        /*  1*/ { -1, 4, -1, { 0, 0, "" }, "MaxiCode (19 chars)", -1, "", 0, 33, 30, 1, "ISO/IEC 16023:2000 Figure H1 **NOT SAME** different encodation (figure uses '3 Shift A' among other differences)",
                    "000111011111010000001010110111"
                    "101101010001110001000000110110"
                    "101110001010111100100111111011"
                    "010101010101010101010101010100"
                    "000000000000000000000000000011"
                    "101010101010101010101010101010"
                    "010101010101010101010101010111"
                    "000000000000000000000000000010"
                    "101010101010101010101010101011"
                    "010101011111111100000001010100"
                    "000000000011110110001000000001"
                    "101010101110000000111010101010"
                    "010101100010000000001101010110"
                    "000000101000000000001000000010"
                    "101010000000000000011010101001"
                    "010101010000000000001101010110"
                    "000000001000000000001000000010"
                    "101010110000000000001010101010"
                    "010101101100000000010101010111"
                    "000000100000000000000000000000"
                    "101010010110000000000110101011"
                    "010101010110000000001001010110"
                    "000000000110001011000000000011"
                    "101010100110111001010010101010"
                    "010101010101010101011100100010"
                    "000000000000000000000000101100"
                    "101010101010101010101010101101"
                    "011000111100100001011011100100"
                    "001010011100001010001111100100"
                    "111010000011101011110001010100"
                    "000111001100100000010011000110"
                    "100000101101100000001000111110"
                    "011101000100000001001111101001"
                },
        /*  2*/ { ESCAPE_MODE, 2, 96 + 1, { 0, 0, "" }, "1Z00004951\\GUPSN\\G06X610\\G159\\G1234567\\G1/1\\G\\GY\\G634 ALPHA DR\\GPITTSBURGH\\GPA\\R\\E", -1, "152382802840001", 0, 33, 30, 1, "ISO/IEC 16023:2000 Figure B2 **NOT SAME** uses different encodation (figure precedes PAD chars with Latch B)",
                    "110101110110111110111111101111"
                    "010101010111000011011000010010"
                    "110110110001001010100110010001"
                    "111000101010101111111111111110"
                    "001111000010110010011000000011"
                    "001001110010101010100000000000"
                    "111011111110111111101111111110"
                    "100110000011001001110000001010"
                    "010001100010101010101001110001"
                    "110111100011010000011011111100"
                    "001100110011110000001110101001"
                    "101110101000000001011111011010"
                    "101010000000000000010110111111"
                    "111101100000000000011011100010"
                    "101010010000000000000110011101"
                    "001000010000000000011100011110"
                    "010011001000000000001000001010"
                    "000000101000000000001010000010"
                    "000100111100000000001110101010"
                    "000010101100000000001000110000"
                    "100000111010000000011101100000"
                    "101000100000000000110110100000"
                    "001000001110100101100110100101"
                    "011001110010101001100000001000"
                    "000010100010110001010101111010"
                    "100111000011111000001101101010"
                    "110010001001010010100000001101"
                    "000001000110110100111010111100"
                    "010111010010100100001111011010"
                    "110111001110101010101101110100"
                    "011011110001010100010111010011"
                    "000111101001100001111000010110"
                    "000100101000110000000111110011"
                },
        /*  3*/ { ESCAPE_MODE, 2, 96 + 1, { 0, 0, "" }, "1Z00004951\\GUPSN\\G06X610\\G159\\G1234567\\G1/1\\G\\GY\\G634 ALPHA DR\\GPITTSBURGH\\GPA\\R\\E", -1, "15238840001", 0, 33, 30, 1, "OkapiBarcode zero-pad postcode lacking +4 (US 840 only), ISO/IEC 16023:2000 Annex B.1.4a",
                    "110101110110111110111111101111"
                    "010101010111000011011000010010"
                    "110110110001001010100110010001"
                    "111000101010101111111111111110"
                    "001111000010110010011000000011"
                    "001001110010101010100000000000"
                    "111011111110111111101111111110"
                    "100110000011001001110000001010"
                    "010001100010101010101001110001"
                    "110111101111010000011011111100"
                    "001100111111000000001010101001"
                    "101110100000000001010011011010"
                    "101010010010000000010110111111"
                    "111101110000000000010011100010"
                    "101010110000000000000110011101"
                    "001000010000000000001100011110"
                    "010011001000000000001000001010"
                    "000000101000000000001010000010"
                    "000100001100000000000010101010"
                    "000010010100000000000100110000"
                    "100000111100000000010001100000"
                    "101000101000000000111110100000"
                    "001000001110100101011010100101"
                    "011001111110011001010100001000"
                    "000010100010110001010101111010"
                    "100111000011111000001101101010"
                    "110010001001010010100000001101"
                    "000001000110110100111010111100"
                    "010111010010100100001111011010"
                    "110111001110101010101101110100"
                    "011011110001010100010111010011"
                    "000111101001100001111000010110"
                    "000100101000110000000111110011"
                },
        /*  4*/ { -1, 3, -1, { 0, 0, "" }, "CEN", -1, "B1050056999", 0, 33, 30, 1, "ISO/IEC 16023:2000 B.1 Example (primary only given, data arbitrary); verified manually against tec-it",
                    "000000010101010101010101010111"
                    "001011000000000000000000000010"
                    "111001101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101010"
                    "010101010101010101010101010100"
                    "000000000000000000000000000010"
                    "101010101010101010101010101000"
                    "010101010111100000100101010110"
                    "000000000001110000010100000010"
                    "101010101000000011010010101010"
                    "010101111100000000100001010110"
                    "000000001100000000011000000010"
                    "101010100100000000011110101001"
                    "010101011000000000000001010110"
                    "000000101000000000001000000001"
                    "101010110000000000001010101010"
                    "010101010000000000011101010101"
                    "000000101000000000011100000000"
                    "101010111100000000001010101001"
                    "010101011110000000011101010110"
                    "000000001110000001111000000011"
                    "101010101011011001000110101010"
                    "010101010101010101011100100010"
                    "000000000000000000000011011100"
                    "101010101010101010101001110100"
                    "111011101110000110101011010110"
                    "100001011111111101000011100111"
                    "110100001000001101100010100110"
                    "110110111111011110000011011111"
                    "010010001001110010000101000010"
                    "010001011010000011010010011100"
                },
        /*  5*/ { UNICODE_MODE | ESCAPE_MODE, -1, -1, { 0, 0, "" }, "Comité Européen de Normalisation\034rue de Stassart 36\034B-1050 BRUXELLES\034TEL +3225196811", -1, "", 0, 33, 30, 1, "ISO/IEC 16023:2000 Example F.5 **NOT SAME** uses different encodation (2 Shift A among other things)",
                    "110010100010110000000100000111"
                    "011010001100110110111110101100"
                    "111010011100101010011100101101"
                    "000000100010000000001000001100"
                    "111101100000011100110011110110"
                    "011110001011100010100111011110"
                    "101010000100000111101000101010"
                    "110010101110101101001000000010"
                    "000110101100101111110010111111"
                    "111101111011111000111110100000"
                    "100000110101111010011110000000"
                    "011100110100000000101100011000"
                    "100001000110000000101011111100"
                    "110011101100000000111000001000"
                    "000110111100000000011010001011"
                    "101010010000000000001000011100"
                    "011000001000000000001000000000"
                    "001001010000000000001101000010"
                    "000000100100000000010000101010"
                    "101111101100000000010010001110"
                    "100000101110000000011110110000"
                    "101101011010000001110000001010"
                    "110001000110001011000111101001"
                    "111111000110000001110000100010"
                    "001000010001010101010111010000"
                    "110100110100000000000010001010"
                    "101111001110101010101101100111"
                    "011010000110010001100100101010"
                    "011100000011000100100111111110"
                    "010100011110011011111011000100"
                    "000000100010100101010010010010"
                    "001111010001110000011001100110"
                    "000100001011101000111010000010"
                },
        /*  6*/ { -1, -1, -1, { 0, 0, "" }, "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999", -1, "", 0, 33, 30, 1, "Numeric compaction, verified manually against TEC-IT",
                    "101101010111101101010111101111"
                    "110110111011110110111011110100"
                    "111101001111111101001111111100"
                    "010111101101010111101101010110"
                    "111001111110111001111110111000"
                    "000111111111000111111111000110"
                    "011110110101011110110101011111"
                    "101111011011101111011011101100"
                    "111111110100111111110100111101"
                    "101101011111110100101001111010"
                    "111110111111011010010110011111"
                    "111111000000000000000101111100"
                    "110101001100000000110001111010"
                    "011011101100000000001110111100"
                    "110100101100000000010111111110"
                    "101101111000000000001101011110"
                    "111110001000000000001011100100"
                    "111111101000000000001100011110"
                    "110101111000000000011001111000"
                    "011011001100000000111010111110"
                    "110100110110000000111011111110"
                    "111011010010000000100101011100"
                    "011111101110111111111011100101"
                    "111111110110011001111000011100"
                    "010101111011010101111010001110"
                    "101110111101101110110110000000"
                    "010011111111010011110011000000"
                    "101010001001110111000101100100"
                    "110111101010011001100011111000"
                    "111000110011011001001101000110"
                    "111010010010001000101000010101"
                    "101111000100011110110011011100"
                    "011111000110100011001101000000"
                },
        /*  7*/ { -1, 5, -1, { 0, 0, "" }, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\037\237\240\242\243\244\245\246\247\251\255\256\266\225\226\227\230\231\232\233\234\235\236", 51, "", 0, 33, 30, 1, "Mode 5 set E",
                    "000000000000000000101010101011"
                    "100101010111111111000000001000"
                    "110010011100100111001001110011"
                    "010101010101011010101010101000"
                    "010110101010001101010110101010"
                    "100011011000110101100011011000"
                    "010101010101111111111111111100"
                    "010111111111000000001010101010"
                    "011100100111001001110010011110"
                    "101010101011110000001010111100"
                    "111111111111110010010011010110"
                    "000000000110000010001000100010"
                    "101010110010000000001110101011"
                    "111111111000000000001011111100"
                    "000000110100000000001100000010"
                    "101010010000000000001010101000"
                    "111111001000000000001011111101"
                    "000000100000000000001000000010"
                    "101010100000000000001010101001"
                    "111111000000000000000011111110"
                    "000000100100000000011100000000"
                    "001011110100000001110111111000"
                    "000111000010110101110111101111"
                    "001110100010110001001010110010"
                    "110011000100101011100101101100"
                    "101111101011010011111111100110"
                    "010001100101111110101001111101"
                    "001000011101100111100101110010"
                    "110100001111010010010010001111"
                    "100010011100101000100000101100"
                    "110100100100100001111100000111"
                    "011111011011101100100101001010"
                    "000011101111000101100100011101"
                },
        /*  8*/ { -1, 6, -1, { 0, 0, "" }, "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377\241\250\253\257\260\264\267\270\273\277\212\213\214\215\216\217\220\221\222\223\224", -1, "", 0, 33, 30, 1, "Mode 6 set D",
                    "000000000000000000101010101011"
                    "100101010111111111000000001010"
                    "110010011100100111001001110001"
                    "010101010101010110101010101010"
                    "101010100000000001010110101000"
                    "110110001101100001100011011000"
                    "010101010101010111111111111111"
                    "010101011111111100000000101000"
                    "001001110010011100100111001001"
                    "010111110111110000000011111110"
                    "000001011011110010011101101011"
                    "101001101110000001000100110110"
                    "010101110000000000000001010110"
                    "000000101000000000000000000010"
                    "101010110100000000000110101010"
                    "010101010000000000001001010100"
                    "000000001000000000001000000011"
                    "101010100000000000001010101010"
                    "010101101100000000001001010111"
                    "000000101000000000001000000010"
                    "101010010110000000011010101001"
                    "010101011100000000111101010100"
                    "000000001110110111011000000011"
                    "101010100110110001100110101010"
                    "010101010101010101011001011100"
                    "000000000000000000001000000000"
                    "101010101010101010101010100010"
                    "101100011100101000011001110100"
                    "011100110000010001111110111110"
                    "101101100000111001011101001010"
                    "001100111110101100001101010101"
                    "011111001010000101000011000110"
                    "101111010010011100100011110010"
                },
        /*  9*/ { -1, 6, -1, { 0, 0, "" }, "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\252\254\261\262\263\265\271\272\274\275\276\200\201\202\203\204\205\206\207\210\211", -1, "", 0, 33, 30, 1, "Mode 6 set C",
                    "000000000000000000101010101011"
                    "100101010111111111000000001010"
                    "110010011100100111001001110001"
                    "010101010101010110101010101010"
                    "101010100000000001010110101000"
                    "110110001101100001100011011000"
                    "010101010101010111111111111111"
                    "010101011111111100000000101000"
                    "001001110010011100100111001001"
                    "010111111111110000000111111110"
                    "000001010111110010010001101011"
                    "101001101010000001001100110110"
                    "010101100000000000000101010110"
                    "000000011000000000000100000010"
                    "101010110100000000000010101010"
                    "010101010000000000001001010100"
                    "000000001000000000001000000011"
                    "101010100000000000001010101010"
                    "010101110000000000001001010111"
                    "000000111100000000001100000010"
                    "101010110000000000010110101001"
                    "010101010100000000101101010100"
                    "000000001010110101011000000011"
                    "101010100010110001010110101010"
                    "010101010101010101011001011100"
                    "000000000000000000001000000000"
                    "101010101010101010101010100010"
                    "101100011100101000011001110100"
                    "011100110000010001111110111110"
                    "101101100000111001011101001010"
                    "001100111110101100001101010101"
                    "011111001010000101000011000110"
                    "101111010010011100100011110010"
                },
        /* 10*/ { UNICODE_MODE | ESCAPE_MODE, 4, -1, { 0, 0, "" }, "`abcdefghijklmnopqrstuvwxyz\034\\G\\R{}~\177;<=>?[\\\\]^_ ,./:@!|", -1, "", 0, 33, 30, 1, "Mode 4 Set B",
                    "000000000000000010101010101011"
                    "010101011111111100000000101010"
                    "001001110010011100100111001000"
                    "010101010101101010101010101010"
                    "101010000000111111010101101000"
                    "011000110100011000011000110110"
                    "010101010101010101111111111110"
                    "100101010111111111000000001010"
                    "110010011100100111001001110010"
                    "010101011111000000000101111110"
                    "000000001101001010011000101000"
                    "101010101000000000100110011000"
                    "010101010000000000001001010101"
                    "000000111000000000001000000000"
                    "101010111100000000011010101010"
                    "010101010000000000000101010110"
                    "000000001000000000001000000010"
                    "101010100000000000001010101010"
                    "010101110100000000001101010101"
                    "000000010100000000101100000000"
                    "101010110100000000001010101000"
                    "010101010000000001010001010110"
                    "000000000010111111001000000010"
                    "101010101010110001011110101000"
                    "010101010101010101010010101000"
                    "000000000000000000000101100100"
                    "101010101010101010100010010110"
                    "001101100000000100111111111100"
                    "011101101001110010100010011010"
                    "111001101111100110001001101000"
                    "000001100000101010001100101010"
                    "100100110011010101001011100100"
                    "001000101111010000111000010101"
                },
        /* 11*/ { UNICODE_MODE | ESCAPE_MODE, 4, -1, { 0, 0, "" }, "\\rABCDEFGHIJKLMNOPQRSTUVWXYZ\034\\G\\R \"#$%&'()*+,-./0123456789:", -1, "", 0, 33, 30, 1, "Mode 4 Set A",
                    "000000000000001010101010101011"
                    "010101111111110000000010101010"
                    "100111001001110010011100100101"
                    "010101010101011010101010101000"
                    "101010100000001111110101011011"
                    "110110001101000110000110001100"
                    "010101010101010111100010111101"
                    "010101011111111100111010111000"
                    "001001110010011100111101000000"
                    "010101010011000000000001111000"
                    "000000001001001010010100011001"
                    "101010100010000000010010011010"
                    "010101000000000000000001010111"
                    "000000110100000000100100000010"
                    "101010110000000000001010101001"
                    "010101010000000000001101010110"
                    "000000001000000000001000000000"
                    "101010100000000000001010101000"
                    "010101001100000000001101010100"
                    "000000111000000000101100000000"
                    "101010000010000000011110101001"
                    "010101010000000000100101010110"
                    "000000001110000001101000000000"
                    "101010100011000001001110101000"
                    "010101010101010101011100010100"
                    "000000000000000000001011000100"
                    "101010101010101010101011110001"
                    "100101110110110010101111111100"
                    "101101011111001110100110100100"
                    "010001101100100011100111010100"
                    "010000101010001111010111011001"
                    "010010011001101010010001111100"
                    "100111100111001110011000011011"
                },
        /* 12*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "ABCDabcdAabcABabcABCabcABCDaABCabABCabcABCéa", -1, "", 0, 33, 30, 1, "Mode 4 LCHB SHA 2SHA 3SHA LCHA SHB LCHB 3SHA 3SHA SHD",
                    "110000000011000000000011000011"
                    "010000000001000000000001000000"
                    "111010011100100110011110100101"
                    "000000001100000000110000000000"
                    "000000000110000000110000000010"
                    "110110101100110110111101101100"
                    "110000110000000000001100000000"
                    "110000010000000000000100000000"
                    "111001101001111001111010011110"
                    "010101010011000000001000001100"
                    "000000000001000010001000011111"
                    "101010100110000000010010101000"
                    "010101000010000000000001010101"
                    "000000001000000000101000000000"
                    "101010100000000000010010101010"
                    "010101011000000000001101010110"
                    "000000001000000000001000000001"
                    "101010001000000000001010101000"
                    "010101001100000000001001010111"
                    "000000111000000000011000000010"
                    "101010101000000000001110101000"
                    "010101010110000000010101010100"
                    "000000001110001111001100000001"
                    "101010101111000001101110101000"
                    "010101010101010101011101000110"
                    "000000000000000000000000000110"
                    "101010101010101010101101100100"
                    "000010001010001010000101010000"
                    "101000000111100010010001111100"
                    "101001111000010101010110110010"
                    "011101111110011111011100001000"
                    "110001000010011110111101111000"
                    "011010011011111110001000000010"
                },
        /* 13*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "ÀÁÂÃ1", -1, "", 0, 33, 30, 1, "Mode 4 LCKC LCHA",
                    "010101010101010101010101010111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101010"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101011111110000111001010100"
                    "000000001111110010001000000000"
                    "101010101010000000011110101010"
                    "010101000100000000000101010110"
                    "000000110000000000000100000000"
                    "101010011000000000001110101000"
                    "010101110000000000001001010110"
                    "000000101000000000001000000010"
                    "101010000000000000001110101000"
                    "010101001100000000000001010111"
                    "000000100100000000001000000010"
                    "101010010000000000010110101011"
                    "010101011100000000001101010100"
                    "000000001010110101111100000011"
                    "101010101010110001111110101010"
                    "010101010101010101011111010111"
                    "000000000000000000001111010110"
                    "101010101010101010100000000010"
                    "111111110000111100000101000000"
                    "010110101111000011110101010100"
                    "101010100000010101011111101010"
                    "111101011010000011110101101010"
                    "010110101111010110101010111100"
                    "010100000000010110101010010100"
                },
        /* 14*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "ÀÁÂÃ123456789", -1, "", 0, 33, 30, 1, "Mode 4 LCKC NS",
                    "111110110101010101010101010111"
                    "111010010000000000000000000000"
                    "000010011010101010101010101000"
                    "010101010101010101010101010100"
                    "000000000000000000000000000001"
                    "101010101010101010101010101010"
                    "010101010101010101010101010101"
                    "000000000000000000000000000000"
                    "101010101010101010101010101001"
                    "010101010111110000110001010110"
                    "000000000011110010010000000010"
                    "101010100110000000111010101010"
                    "010101011000000000001001010101"
                    "000000101000000000001000000000"
                    "101010000100000000000110101011"
                    "010101010000000000001101010110"
                    "000000001000000000001000000000"
                    "101010100000000000001010101000"
                    "010101100000000000001001010100"
                    "000000001000000000001100000010"
                    "101010110000000000011010101001"
                    "010101010100000000011001010100"
                    "000000001110110101111000000000"
                    "101010101010110001000110101010"
                    "010101010101010101010000101101"
                    "000000000000000000001110011100"
                    "101010101010101010100111100010"
                    "011101100110110110100011110010"
                    "010001101101001001100100011011"
                    "100011011110110000101000000010"
                    "101101111000011101010000100101"
                    "000000111100011110100001110000"
                    "101000000010100111001011110101"
                },
        /* 15*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "àáâã1", -1, "", 0, 33, 30, 1, "Mode 4 LCKD LCHA",
                    "010101010101010101010101010111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101010"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101010111110000111101010100"
                    "000000000011110010000100000000"
                    "101010101110000000010110101010"
                    "010101010100000000000001010110"
                    "000000000000000000000000000000"
                    "101010011000000000001010101000"
                    "010101110000000000001001010110"
                    "000000101000000000001000000010"
                    "101010000000000000001110101000"
                    "010101010000000000000001010111"
                    "000000110000000000001100000010"
                    "101010110110000000011010101011"
                    "010101010100000000011101010100"
                    "000000001110110111111100000011"
                    "101010101110110001001110101010"
                    "010101010101010101011111010111"
                    "000000000000000000001111010110"
                    "101010101010101010100000000010"
                    "111111110000111100000101000000"
                    "010110101111000011110101010100"
                    "101010100000010101011111101010"
                    "111101011010000011110101101010"
                    "010110101111010110101010111100"
                    "010100000000010110101010010100"
                },
        /* 16*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "¢£¤¥1", -1, "", 0, 33, 30, 1, "Mode 4 LCKE LCHA",
                    "010101010101010101010101010111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101010"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101010111110100110001010100"
                    "000000000101110110000100000000"
                    "101010101010000000011110101010"
                    "010101010110000000001001010110"
                    "000000010000000000111000000000"
                    "101010011000000000001110101000"
                    "010101111000000000001001010110"
                    "000000101000000000001000000010"
                    "101010000000000000001110101000"
                    "010101111000000000001101010111"
                    "000000111100000000111000000010"
                    "101010001100000000111110101011"
                    "010101011110000001011101010100"
                    "000000001010110001011100000011"
                    "101010101010110101101010101010"
                    "010101010101010101011111010111"
                    "000000000000000000001111010110"
                    "101010101010101010100000000010"
                    "111111110000111100000101000000"
                    "010110101111000011110101010100"
                    "101010100000010101011111101010"
                    "111101011010000011110101101010"
                    "010110101111010110101010111100"
                    "010100000000010110101010010100"
                },
        /* 17*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "¢£¤¥123456789", -1, "", 0, 33, 30, 1, "Mode 4 LCKE NS",
                    "111110101010101010101010101011"
                    "111010111111111111111111111100"
                    "000010000000000000000000000011"
                    "101010101010101010101010101010"
                    "111111111111111111111111111101"
                    "000000000000000000000000000010"
                    "101010101010101010101010101011"
                    "111111111111111111111111111100"
                    "000000000000000000000000000011"
                    "101010101111110100111010101000"
                    "111111111001110110011111111101"
                    "000000000110000000111000000000"
                    "101010001010000000000110101011"
                    "111111001000000000110111111110"
                    "000000000100000000000100000011"
                    "101010011000000000001110101010"
                    "111111001000000000001011111101"
                    "000000100000000000001000000000"
                    "101010010100000000000110101001"
                    "111111010000000000111111111100"
                    "000000101100000000110000000001"
                    "101010100110000001001010101010"
                    "111111111110110001011011111101"
                    "000000001010110101010000000010"
                    "101010101010101010100110111001"
                    "111111111111111111110110001100"
                    "000000000000000000001100010001"
                    "001110001011100101010100110000"
                    "010000101010111000100010110100"
                    "111011010110010000010100011000"
                    "111100010011001010110101010101"
                    "011100101001010011011100111100"
                    "101101110111011101011010011101"
                },
        /* 18*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "ABCDE12abcde1ÀÁÂÃ¢£¤¥1àáâãabcde123A123456789àáâã¢£¤¥abc", -1, "", 0, 33, 30, 1, "Mode 4 mixed sets",
                    "000000001111111100000000111111"
                    "000010101100111100000000111100"
                    "011100101110000000100111010111"
                    "001100000000111111110101010110"
                    "001100000000111100010101101010"
                    "101111011000101010011000110110"
                    "000000001111111100100010111110"
                    "000010101100000000111010111000"
                    "011100101110011110111101000010"
                    "000000001011001100010111111010"
                    "000000000001000010000111111000"
                    "110110001110000000011110101000"
                    "111101000010000000001101010101"
                    "111110110000000000100110010100"
                    "010101101000000000010111001001"
                    "010100110000000000001100001110"
                    "000000101000000000001000001110"
                    "101011110000000000001101101110"
                    "010101101000000000000001010101"
                    "000000000100000000010000000010"
                    "101010100000000000000110101001"
                    "010101010100000000000101010100"
                    "000000001010001011100000000011"
                    "101010101011000001011010101000"
                    "010101010101010101011001101001"
                    "000000000000000000001010110000"
                    "101010101010101010100111000000"
                    "001010111101110100110011111010"
                    "011101101010101100110000010101"
                    "011100101111011110010010111010"
                    "100101101011010111110011101011"
                    "100010001001110010101010111000"
                    "111010110110001100101000010001"
                },
        /* 19*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "A123456789B123456789C123456789D123456789E123456789F123456789G123456789H123456789I123456789J123456789K123456789L123456789MNO123456789", -1, "", 0, 33, 30, 1, "Mode 4 spaced NSs",
                    "001011111000100010111110001011"
                    "101011101000111010111010101100"
                    "110100001011111101000010001101"
                    "100010111110001000101111100010"
                    "111010101110101110101011101011"
                    "110110000001111110100000011110"
                    "001011111000100010111110001010"
                    "101011101010111010111010011110"
                    "110100001011111101000010001110"
                    "100010110111101100010011100000"
                    "110110100001111010011111101010"
                    "111010000100000000101100011100"
                    "001011101010000000001011100010"
                    "101011111100000000110110100110"
                    "110100001100000000010000100100"
                    "101111011000000000000110001000"
                    "101011001000000000001010101100"
                    "100000001000000000001001111110"
                    "001000101100000000000110111101"
                    "011110001100000000111010111010"
                    "111111100010000000000101000001"
                    "111110000010000000001010001010"
                    "101110101010000011011011111011"
                    "000001110111001001101011001010"
                    "100000001000101111100001000111"
                    "101111111110101110101001010000"
                    "101001111111010000100110111011"
                    "101100110110010001111000100000"
                    "111101100110001110010001110001"
                    "000101001110100101000001111000"
                    "101101001001111010110101100000"
                    "011101000011011001001111111000"
                    "111010010011100000100100111111"
                },
        /* 20*/ { UNICODE_MODE, 4, -1, { 3, 7, "" }, "THIS IS A 91 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, APPENDED, MAXICODE SYMBOL...", -1, "", 0, 33, 30, 1, "Mode 4 Structured Append",
                    "010001111101000000100000100011"
                    "000000010000000100000000101000"
                    "001000101000110010011011001000"
                    "000100011000100100000000011000"
                    "110000001010000010101100000010"
                    "100010000010110010001111000100"
                    "001010000000011000001001000010"
                    "100000001010001001001000100110"
                    "101111101110000000100000011010"
                    "000100010011101000000110000010"
                    "110000000111100010001100111100"
                    "100010001010000000000011000010"
                    "000000001010000000010101110101"
                    "111010110000000000010100101100"
                    "110010111100000000001100000011"
                    "000010010000000000001010000100"
                    "111000001000000000001000000000"
                    "011000010000000000001100100000"
                    "000000001100000000011001010000"
                    "101010010100000000000111001100"
                    "001000000000000000001000001001"
                    "000000000010000000000000100000"
                    "101011000110011011000001010001"
                    "100011111010000001000010001000"
                    "011010000000000101011111110010"
                    "000001110011111111111010100000"
                    "001110100111000101011001001100"
                    "011010010100110111100101011100"
                    "101101001001011111000110110111"
                    "110001110110110001000011001010"
                    "011100001000111100110111011110"
                    "010101011101100110111011100100"
                    "011001000011110011011110111010"
                },
        /* 21*/ { UNICODE_MODE, 3, -1, { 1, 8, "" }, "COMMISSION FOR EUROPEAN NORMALIZATION, RUE DE STASSART 36, B-1050 BRUXELLES", -1, "B1050056999", 0, 33, 30, 1, "Mode 3 Structured Append",
                    "010000000000001010000000010011"
                    "001000111111010000011111001000"
                    "101111111010101111101101000101"
                    "000001000000100010100001100010"
                    "111100110010001100101000001110"
                    "110100011010001101101000011100"
                    "100000000010001000000001011010"
                    "001100110101001001111111000010"
                    "011010001001100010110100000111"
                    "100100000111100000100101001000"
                    "000010100001110000010100101010"
                    "110010001000000011010000101000"
                    "100010111100000000100010001001"
                    "100000001100000000011000000000"
                    "001011100100000000011111100100"
                    "010111011000000000000011011010"
                    "001110101000000000001000001000"
                    "000001110000000000001011000000"
                    "000111010000000000011111111101"
                    "001100101000000000011100100010"
                    "011010111100000000001000100000"
                    "000010101110000000011110000110"
                    "111001101110000001111000000000"
                    "001000101011011001000101010000"
                    "000010010101010101010000111101"
                    "111000000000000000001110100000"
                    "001011101010101010101000100010"
                    "001100110110100101101100010110"
                    "111100110000110110000010011001"
                    "011100011110101010001100000000"
                    "011001100010001111111000101001"
                    "011111110000111010001010001100"
                    "110010001001001011011111100111"
                },
        /* 22*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "am.//ab,\034TA# z\015!", -1, "", 0, 33, 30, 1, "Was test_best_supported_set",
                    "101110000101101100110101010111"
                    "110110000011010100100000000010"
                    "001000101111011100101010101010"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101010"
                    "010101010101010101010101010100"
                    "000000000000000000000000000010"
                    "101010101010101010101010101011"
                    "010101010111001100000101010110"
                    "000000001011000010000000000001"
                    "101010101100000000100110101010"
                    "010101001100000000101101010110"
                    "000000100000000000010000000010"
                    "101010110000000000010010101011"
                    "010101011000000000000101010100"
                    "000000001000000000001000000000"
                    "101010001000000000001010101000"
                    "010101010000000000001101010101"
                    "000000001100000000000000000010"
                    "101010110010000000010110101011"
                    "010101010100000001111001010110"
                    "000000001110110111111100000000"
                    "101010100110111101011010101000"
                    "010101010101010101011101100010"
                    "000000000000000000000101101000"
                    "101010101010101010100011011000"
                    "000000100000111000011101111100"
                    "110001011111000111110010001000"
                    "001011000001011000000110100000"
                    "110010000110011010101100011010"
                    "011011001001000010000110011100"
                    "000010111111111111001110001110"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAXICODE, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }
        strcpy(symbol->primary, data[i].primary);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, { %d, %d, \"%s\" }, \"%s\", %d, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    data[i].primary, testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, data[i].primary, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[17984 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, data[i].primary, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        struct zint_structapp structapp;
        struct zint_seg segs[3];
        char *primary;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, "", 0, 33, 30, 1, "ISO 16023:2000 4.15.4 example",
                    "010101010101010101010101010111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101010"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101010011010100000101010100"
                    "000000000011111110001000000000"
                    "101010101110000000101010101010"
                    "010101110110000000110101010110"
                    "000000110000000000100100000000"
                    "101010011000000000010010101000"
                    "010101111000000000001001010110"
                    "000000001000000000001000000010"
                    "101010001000000000001110101000"
                    "010101111100000000000001010111"
                    "000000010000000000110100000010"
                    "101010000110000000010110101011"
                    "010101011010000001110001010100"
                    "000000000010110101000100000011"
                    "101010100111110001001110101010"
                    "010101010101010101011111010111"
                    "000000000000000000001111010110"
                    "101010101010101010100000000010"
                    "111111110000111100000101000000"
                    "010110101111000011110101010100"
                    "101010100000010101011111101010"
                    "111101011010000011110101101010"
                    "010110101111010110101010111100"
                    "010100000000010110101010010100"
                },
        /*  1*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, "", ZINT_WARN_USES_ECI, 33, 30, 1, "ISO 16023:2000 4.15.4 example auto-ECI",
                    "010101010101010101010101010111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101010"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101010011010100000101010100"
                    "000000000011111110001000000000"
                    "101010101110000000101010101010"
                    "010101110110000000110101010110"
                    "000000110000000000100100000000"
                    "101010011000000000010010101000"
                    "010101111000000000001001010110"
                    "000000001000000000001000000010"
                    "101010001000000000001110101000"
                    "010101111100000000000001010111"
                    "000000010000000000110100000010"
                    "101010000110000000010110101011"
                    "010101011010000001110001010100"
                    "000000000010110101000100000011"
                    "101010100111110001001110101010"
                    "010101010101010101011111010111"
                    "000000000000000000001111010110"
                    "101010101010101010100000000010"
                    "111111110000111100000101000000"
                    "010110101111000011110101010100"
                    "101010100000010101011111101010"
                    "111101011010000011110101101010"
                    "010110101111010110101010111100"
                    "010100000000010110101010010100"
                },
        /*  2*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, "", 0, 33, 30, 1, "ISO 16023:2000 4.15.4 example inverted",
                    "010101010101010101010101010111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101010"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101010011000000110101010100"
                    "000000000111100010011000000000"
                    "101010100110000000011010101010"
                    "010101010110000000110101010110"
                    "000000000000000000111100000000"
                    "101010101000000000001110101000"
                    "010101110000000000001101010110"
                    "000000001000000000001000000010"
                    "101010111000000000001110101000"
                    "010101101000000000010101010111"
                    "000000100100000000111000000010"
                    "101010011010000000110110101011"
                    "010101010110000001111001010100"
                    "000000001010101111101100000011"
                    "101010101110010101110010101010"
                    "010101010101010101011111010111"
                    "000000000000000000001111010110"
                    "101010101010101010100000000010"
                    "111111110000111100000101000000"
                    "010110101111000011110101010100"
                    "101010100000010101011111101010"
                    "111101011010000011110101101010"
                    "010110101111010110101010111100"
                    "010100000000010110101010010100"
                },
        /*  3*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, "", ZINT_WARN_USES_ECI, 33, 30, 1, "ISO 16023:2000 4.15.4 example inverted auto-ECI",
                    "010101010101010101010101010111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101010"
                    "010101010101010101010101010110"
                    "000000000000000000000000000001"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101010011000000110101010100"
                    "000000000111100010011000000000"
                    "101010100110000000011010101010"
                    "010101010110000000110101010110"
                    "000000000000000000111100000000"
                    "101010101000000000001110101000"
                    "010101110000000000001101010110"
                    "000000001000000000001000000010"
                    "101010111000000000001110101000"
                    "010101101000000000010101010111"
                    "000000100100000000111000000010"
                    "101010011010000000110110101011"
                    "010101010110000001111001010100"
                    "000000001010101111101100000011"
                    "101010101110010101110010101010"
                    "010101010101010101011111010111"
                    "000000000000000000001111010110"
                    "101010101010101010100000000010"
                    "111111110000111100000101000000"
                    "010110101111000011110101010100"
                    "101010100000010101011111101010"
                    "111101011010000011110101101010"
                    "010110101111010110101010111100"
                    "010100000000010110101010010100"
                },
        /*  4*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("Pixel 4a 128 GB:$439.97"), -1, 3 }, { TU("Pixel 4a 128 GB:￥3149.79"), -1, 29 }, { TU("Pixel 4a 128 GB:444,90 €"), -1, 17 } }, "", 0, 33, 30, 1, "AIM ITS/04-023:2022 Annex A example (shortened)",
                    "011111000111111101000011011111"
                    "001001000000000100100001101010"
                    "000011100010010000110101000001"
                    "110000100011101010111101111110"
                    "111110010111001101100111010000"
                    "110010001011001011111001101110"
                    "011111000111111101000011110110"
                    "001001000000000100100001111000"
                    "000011100010010000110101011100"
                    "011111111011001000000111011110"
                    "110110000101000110011000011100"
                    "011000101100000000100011000100"
                    "111110001110000000011010101110"
                    "100101101100000000001000001110"
                    "111011001100000000001110001100"
                    "011100010000000000000000100010"
                    "001111001000000000001010010111"
                    "001100110000000000001010001000"
                    "111100011100000000011001111110"
                    "100100110000000000110100000010"
                    "001110000110000000011100100110"
                    "111111001110000001011000011100"
                    "101001000110101011001110000100"
                    "000001010110011101111011000000"
                    "110111110111010101011111010000"
                    "101101000011010000000101101100"
                    "000010000001001010100001001111"
                    "001010001101110010110101001000"
                    "111110111100101100111101000110"
                    "010000000001111110111001010000"
                    "110101000010011011010111000110"
                    "100010110010010000000010010100"
                    "100011010011101111001011000001"
                },
        /*  5*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("$439.97"), -1, 3 }, { TU("￥3149.79"), -1, 29 }, { TU("444,90 €"), -1, 17 } }, "", 0, 33, 30, 1, "AIM ITS/04-023:2022 Annex A example price only",
                    "101011011101111111110111111011"
                    "011111101101000010011110010100"
                    "111001110100111000100111101110"
                    "010101010111011111011111111010"
                    "000000000111000001111010100011"
                    "101010100001000010000000001010"
                    "010101010101010101010101010111"
                    "000000000000000000000000000010"
                    "101010101010101010101010101000"
                    "010101010111001100100001010100"
                    "000000001001000110011100000010"
                    "101010100010000000011110101010"
                    "010101011110000000001101010100"
                    "000000101000000000010000000000"
                    "101010001100000000000110101000"
                    "010101111000000000001001010110"
                    "000000101000000000001000000010"
                    "101010011000000000001110101010"
                    "010101110100000000001101010101"
                    "000000001000000000011000000000"
                    "101010010010000000101010101000"
                    "010101010100000001111101010100"
                    "000000000110101111101000000011"
                    "101010100110011101000110101000"
                    "010101010101010101011001001110"
                    "000000000000000000000000000100"
                    "101010101010101010100101100101"
                    "001001110101111010110011011100"
                    "001100001100101000100101110110"
                    "011101100101000100110011000100"
                    "000100101101010011010011101111"
                    "010111011011100000011000101000"
                    "001111010010000100010110011110"
                },
        /*  6*/ { DATA_MODE, -1, -1, { 0, 0, "" }, { { TU("\266"), 1, 0 }, { TU("\266"), 1, 7 }, { TU("\266"), 1, 0 } }, "", 0, 33, 30, 1, "Standard example + extra seg, data mode",
                    "101010101010101010101010101011"
                    "111111111111111111111111111100"
                    "000000000000000000000000000011"
                    "101010101010101010101010101010"
                    "111111111111111111111111111111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101011"
                    "111111111111111111111111111110"
                    "000000000000000000000000000011"
                    "101010100011110100111010101000"
                    "111111110011111110000011111111"
                    "000000001110000000111000000010"
                    "101010110110000000101110101010"
                    "111111111100000000110011111100"
                    "000000001100000000010100000011"
                    "101010010000000000001110101010"
                    "111111001000000000001011111110"
                    "000000000000000000001000000010"
                    "101010101000000000010010101001"
                    "111111110100000000100011111110"
                    "000000110000000000111100000010"
                    "101010100100000001110110101010"
                    "111111111110111101101111111101"
                    "000000000011111001100100000010"
                    "101010101010101010101010010110"
                    "111111111111111111110101111110"
                    "000000000000000000000101101010"
                    "000001010101000010100101111110"
                    "101001011111101011111010000010"
                    "111111110000101011110101101010"
                    "000000001111111101011111000010"
                    "111100001111010101010101111110"
                    "000010100101010111111010101010"
                },
        /*  7*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("αβ"), -1, 0 }, { TU("ÿ"), -1, 0 }, { TU("貫やぐ禁"), -1, 20 } }, "", ZINT_WARN_USES_ECI, 33, 30, 1, "Auto-ECI",
                    "101001111110111100111101111111"
                    "011011111100001100001111110010"
                    "110011000010011001010101100010"
                    "010101010101010101010101111000"
                    "000000000000000000000000011010"
                    "101010101010101010101010010100"
                    "010101010101010101010101010101"
                    "000000000000000000000000000010"
                    "101010101010101010101010101011"
                    "010101010111000000110101010110"
                    "000000001011010010000000000000"
                    "101010100000000000111010101000"
                    "010101100100000000110101010111"
                    "000000011000000000011100000000"
                    "101010010000000000010010101010"
                    "010101010000000000001101010110"
                    "000000001000000000001000000011"
                    "101010000000000000001010101010"
                    "010101100000000000010001010100"
                    "000000000000000000010100000010"
                    "101010110010000000110010101000"
                    "010101011000000001001001010110"
                    "000000000010101011101100000000"
                    "101010100010011101101110101000"
                    "010101010101010101011111011101"
                    "000000000000000000000110001000"
                    "101010101010101010100100010011"
                    "001001001011101100110001101010"
                    "100011101101100100001001100101"
                    "110000000110011111100001111000"
                    "000011110000100000101100001100"
                    "101111001101000010111111110000"
                    "001001000010100001000011101011"
                },
        /*  8*/ { UNICODE_MODE, -1, -1, { 1, 2, "" }, { { TU("αβ"), -1, 9 }, { TU("ÿ"), -1, 3 }, { TU("貫やぐ禁"), -1, 20 } }, "", 0, 33, 30, 1, "Structured Append",
                    "000110100111111011110011110111"
                    "001001101111110000110000111100"
                    "110011001100001001100101010101"
                    "010101010101010101011110111110"
                    "000000000000000000000110001101"
                    "101010101010101010100101001010"
                    "010101010101010101010101010111"
                    "000000000000000000000000000010"
                    "101010101010101010101010101000"
                    "010101010011001100000001010110"
                    "000000000101001110001000000011"
                    "101010101010000000101110101000"
                    "010101111000000000110001010101"
                    "000000110100000000100000000010"
                    "101010001100000000011010101011"
                    "010101011000000000000101010100"
                    "000000001000000000001000000010"
                    "101010001000000000001010101000"
                    "010101100000000000011101010110"
                    "000000001000000000001000000010"
                    "101010101110000000011010101001"
                    "010101010110000000010101010110"
                    "000000001110011011011000000010"
                    "101010100011000001100010101010"
                    "010101010101010101011010110111"
                    "000000000000000000001111011010"
                    "101010101010101010101101010010"
                    "111001010001101011001010011100"
                    "001101001101101110010010110011"
                    "101000100011001111100000101000"
                    "100000000110100010001110000100"
                    "101000101011101111110001101000"
                    "000010101010000010000100110100"
                },
        /*  9*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("ab"), -1, 3 }, { TU("ABCD"), -1, 4 }, { TU(""), 0, 0 } }, "", 0, 33, 30, 1, "Code Set B then A",
                    "000000010101010101010101010111"
                    "000010000000000000000000000010"
                    "011100101010101010101010101001"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101010"
                    "010101010101010101010101010100"
                    "000000000000000000000000000000"
                    "101010101010101010101010101011"
                    "010101010111000000110001010100"
                    "000000001011000010000100000000"
                    "101010100000000000111110101010"
                    "010101110010000000111001010110"
                    "000000000000000000101000000010"
                    "101010011000000000010110101011"
                    "010101011000000000000001010110"
                    "000000001000000000001000000001"
                    "101010101000000000001010101000"
                    "010101000000000000001001010101"
                    "000000001000000000001100000010"
                    "101010001110000000111010101001"
                    "010101010010000001111101010100"
                    "000000000110101111011000000010"
                    "101010101010010001101010101010"
                    "010101010101010101011110000100"
                    "000000000000000000001100111010"
                    "101010101010101010101100000000"
                    "110000011010110101000110100010"
                    "100100110111110111001111101011"
                    "011111111011111001110100101100"
                    "101000010001001011101111001010"
                    "011011011101010101001101001000"
                    "001010010110100000100111101000"
                },
        /* 10*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("\004\004\004\004"), -1, 3 }, { TU("ABCD"), -1, 4 }, { TU("abcd"), -1, 5 } }, "", 0, 33, 30, 0, "Code Set E then A then B; BWIPP different encodation, LCHA before 2nd ECI not after, same codeword count, same result",
                    "001100000000100011000000000111"
                    "100100000010011011000000100000"
                    "000110011100111011100111001010"
                    "010101010101010101010101010100"
                    "000000000000000000000000000011"
                    "101010101010101010101010101010"
                    "010101010101010101010101010111"
                    "000000000000000000000000000010"
                    "101010101010101010101010101000"
                    "010101010111000000000001010110"
                    "000000001001001010011100000011"
                    "101010100100000000000110101010"
                    "010101101010000000110101010100"
                    "000000100100000000111000000010"
                    "101010111100000000000010101001"
                    "010101010000000000000001010110"
                    "000000001000000000001000000001"
                    "101010100000000000001010101010"
                    "010101110100000000010101010110"
                    "000000111000000000110100000000"
                    "101010000110000000101010101011"
                    "010101010000000001010101010110"
                    "000000000110100011101100000011"
                    "101010100011011101111110101010"
                    "010101010101010101010101101000"
                    "000000000000000000001001000010"
                    "101010101010101010101010110111"
                    "100010010101100011100111000100"
                    "110110101110100010100011010011"
                    "100001111011001010101000111000"
                    "101100010110101111011111100011"
                    "111010100101000101001011011000"
                    "011111000111001010010000111100"
                },
        /* 11*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("12345678"), -1, 3 }, { TU("91234567901"), -1, 4 }, { TU("12345678"), -1, 5 } }, "", 0, 33, 30, 1, "Digits spanning ECIs",
                    "111000111110001111011010001111"
                    "010110010011111100001101100010"
                    "001100101011100111111011101001"
                    "010101010101011111111111111110"
                    "000000000000000110101010000010"
                    "101010101010100011011000110110"
                    "010101010101010101010101010111"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101011111001100010001010100"
                    "000000000001001010011000000010"
                    "101010100000000000101010101000"
                    "010101011110000000011001010110"
                    "000000011000000000001100000010"
                    "101010101100000000011110101010"
                    "010101111000000000000101010100"
                    "000000101000000000001000000000"
                    "101010101000000000001110101000"
                    "010101010100000000000101010110"
                    "000000001100000000100000000000"
                    "101010001110000000101110101001"
                    "010101011100000001010101010110"
                    "000000001010101111111000000011"
                    "101010101010011101101110101000"
                    "010101010101010101011010001010"
                    "000000000000000000000110111110"
                    "101010101010101010100011111111"
                    "011111100011010101000000000110"
                    "111111010001111000000100101010"
                    "011101100000010110110001000100"
                    "101110111111001001111001110000"
                    "010101101100001010110111001100"
                    "101011011010100110010010100111"
                },
        /* 12*/ { UNICODE_MODE, -1, 96 + 1, { 0, 0, "" }, { { TU("12345678"), -1, 3 }, { TU("91234567901"), -1, 4 }, { TU("12345678"), -1, 5 } }, "9999840012", 0, 33, 30, 1, "SCM prefix mode 2",
                    "110101110110111110111110001111"
                    "010101010111000011011001000000"
                    "110110110001001010100111111001"
                    "110010111100101111111111111110"
                    "111111000110010110101010000010"
                    "011011101000110011011000110100"
                    "110110100011111111111111110100"
                    "000011011000000010101010010010"
                    "111110111010011100100111001001"
                    "010101010011010000011101010110"
                    "000000000001100000000100000001"
                    "101010101110000001011010101000"
                    "010101100010000000000001010100"
                    "000000100000000000001000000000"
                    "101010110000000000001110101000"
                    "010101110000000000011101010110"
                    "000000101000000000001000000011"
                    "101010000000000000011010101010"
                    "010101011100000000000101010110"
                    "000000111000000000000100000010"
                    "101010100110000000001010101011"
                    "010101011000000001101101010100"
                    "000000001010000011010100000010"
                    "101010100110000001010110101000"
                    "010101010101010101010010100111"
                    "000000000000000000001111100100"
                    "101010101010101010100000011011"
                    "001001001110011001111101100010"
                    "110000110000100111010010101000"
                    "110001111010010001001101000010"
                    "100110000010010001101011000111"
                    "110011010111010010100011101000"
                    "111010011111011100110001010110"
                },
        /* 13*/ { UNICODE_MODE, 3, 96 + 1, { 0, 0, "" }, { { TU("12345678"), -1, 3 }, { TU("91234567901"), -1, 4 }, { TU("12345678"), -1, 5 } }, "B999840012", 0, 33, 30, 1, "SCM prefix mode 3",
                    "110101110110111110111110001111"
                    "010101010111000011011001000000"
                    "110110110001001010100111111001"
                    "110010111100101111111111111110"
                    "111111000110010110101010000010"
                    "011011101000110011011000110100"
                    "110110100011111111111111110100"
                    "000011011000000010101010010010"
                    "111110111010011100100111001001"
                    "010101011011100000011101010110"
                    "000000000101010000001000000001"
                    "101010100100000011011010101000"
                    "010101000000000000110001010100"
                    "000000100000000000110000000000"
                    "101010000000000000000110101000"
                    "010101111000000000001101010110"
                    "000000101000000000001000000011"
                    "101010000000000000001010101010"
                    "010101100100000000010101010110"
                    "000000101000000000110100000010"
                    "101010110100000000001110101011"
                    "010101011110000000001001010100"
                    "000000000010000101110100000010"
                    "101010100111011001110010101000"
                    "010101010101010101010010100111"
                    "000000000000000000001111100100"
                    "101010101010101010100000011011"
                    "001001001110011001111101100010"
                    "110000110000100111010010101000"
                    "110001111010010001001101000010"
                    "100110000010010001101011000111"
                    "110011010111010010100011101000"
                    "111010011111011100110001010110"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_encode_segs", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, BARCODE_MAXICODE, data[i].input_mode, -1 /*eci*/,
                    data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, NULL, 0, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }
        strcpy(symbol->primary, data[i].primary);
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            char escaped1[4096];
            char escaped2[4096];
            int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
            int length1 = data[i].segs[1].length == -1 ? (int) ustrlen(data[i].segs[1].source) : data[i].segs[1].length;
            int length2 = data[i].segs[2].length == -1 ? (int) ustrlen(data[i].segs[2].source) : data[i].segs[2].length;
            printf("        /*%3d*/ { %s, %d, %d, { %d, %d, \"%s\" }, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    data[i].primary, testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwippSegs(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].segs, seg_count, data[i].primary, cmp_buf, sizeof(cmp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwippSegs ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, (const char *) data[i].segs[0].source, data[i].segs[0].length, debug)) {
                    if (data[i].input_mode == DATA_MODE) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) {
                            printf("i:%d multiple segments in DATA_MODE not currently supported for ZXing-C++ testing (%s)\n",
                                    i, testUtilBarcodeName(symbol->symbology));
                        }
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[17984 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, (const char *) data[i].segs[0].source, data[i].segs[0].length,
                                modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmpSegs(symbol, cmp_msg, cmp_buf, cmp_len, data[i].segs, seg_count,
                                data[i].primary, escaped, &ret_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPPCmpSegs %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* #181 Nico Gunkel OSS-Fuzz */
static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int eci;
        char *data;
        int length;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /* 0*/ { -1, "\223\223\223\223\223\200\000\060\060\020\122\104\060\343\000\000\040\104\104\104\104\177\377\040\000\324\336\000\000\000\000\104\060\060\060\060\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\060\104\104\000\000\000\040\104\104\104\104\177\377\377\377\324\336\000\000\000\000\104\377\104\001\104\104\104\104\104\104\233\233\060\060\060\060\060\060\060\060\060\325\074", 107, ZINT_ERROR_TOO_LONG }, /* Original OSS-Fuzz triggering data */
        /* 1*/ { -1, "AaAaAaAaAaAaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA123456789", -1, ZINT_ERROR_TOO_LONG }, /* Add 6 lowercase a's so 6 SHIFTS inserted so 6 + 138 (max input len) = 144 and numbers come at end of buffer */
        /* 2*/ { -1, "AaAaAaAaAaAaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA123456789A", -1, ZINT_ERROR_TOO_LONG },
        /* 3*/ { -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", -1, ZINT_ERROR_TOO_LONG },
        /* 4*/ { 32768, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 5*/ { -1, "AaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAa", -1, ZINT_ERROR_TOO_LONG },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_fuzz", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAXICODE, -1 /*input_mode*/, data[i].eci, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITER_MILLES   10
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)
#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)

/* Not a real test, just performance indicator */
static void test_perf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        char *primary;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_MAXICODE, UNICODE_MODE | ESCAPE_MODE, -1, -1,
                    "1Z34567890\\GUPSN\\G102562\\G034\\G\\G1/1\\G\\GY\\G2201 Second St\\GFt Myers\\GFL\\R\\E",
                    "339010000840001", 0, 33, 30, "Mode 2" },
        /*  1*/ { BARCODE_MAXICODE, UNICODE_MODE, 4, -1, "MaxiCode (19 chars)", "", 0, 33, 30, "Mode 4 small" },
        /*  2*/ { BARCODE_MAXICODE, DATA_MODE | ESCAPE_MODE, 4, -1, "ABCDabcdAabcABabcABCabcABCDaABCabABCabcABC\\d233a", "", 0, 33, 30, "Mode 4 medium" },
        /*  3*/ { BARCODE_MAXICODE, DATA_MODE | ESCAPE_MODE, 4, -1,
                    "ABabcdeAabcdABCabcdABabc\\d192\\d192 \\d192\\d224\\d224\\d028\\d224\\d001\\d001\\d001\\d029\\d00112345678a123456789aABCDa\\d192\\d224\\d001\\d192\\d001\\d224\\d030\\d004",
                    "", 0, 33, 30, "Mode 4 latches" },
        /*  4*/ { BARCODE_MAXICODE, UNICODE_MODE, 4, -1,
                    "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "", 0, 33, 30, "Mode 4 txt max" },
        /*  5*/ { BARCODE_MAXICODE, UNICODE_MODE, 4, -1,
                    "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999",
                    "", 0, 33, 30, "Mode 4 num max" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start;
    clock_t total_create = 0, total_encode = 0, total_buffer = 0;
    clock_t diff_create, diff_encode, diff_buffer;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
            strcpy(symbol->primary, data[i].primary);

            start = clock();
            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            #if 0
            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            #endif

            ZBarcode_Delete(symbol);
        }

        printf("%*s: encode % 8gms, buffer % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
    }
    if (p_ctx->index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_create));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_encode_segs", test_encode_segs },
        { "test_fuzz", test_fuzz },
        { "test_perf", test_perf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
