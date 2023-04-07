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
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { -1, -1, "1", 138, "", 0, 33, 30 }, /* Mode 4 (138 agrees with ISO/IEC 16023:2000) */
        /*  1*/ { -1, -1, "1", 139, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { -1, -1, "1", 145, "", ZINT_ERROR_TOO_LONG, -1, -1 }, /* Absolute max */
        /*  3*/ { -1, -1, "A", 93, "", 0, 33, 30 },
        /*  4*/ { -1, -1, "A", 94, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  5*/ { -1, -1, "\001", 90, "", 0, 33, 30 },
        /*  6*/ { -1, -1, "\001", 91, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  7*/ { -1, -1, "\200", 90, "", 0, 33, 30 },
        /*  8*/ { -1, -1, "\200", 91, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  9*/ { 2, -1, "1", 126, "123456789123123", 0, 33, 30 },
        /* 10*/ { 2, -1, "1", 127, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 11*/ { 2, -1, "A", 84, "123456789123123", 0, 33, 30 },
        /* 12*/ { 2, -1, "A", 85, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 13*/ { 2, 96, "1", 109, "123456789123123", 0, 33, 30 },
        /* 14*/ { 2, 96, "1", 110, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 15*/ { 2, 96, "1", 136, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1 }, /* Absolute max with SCM vv */
        /* 16*/ { 3, -1, "1", 126, "ABCDEF123123", 0, 33, 30 },
        /* 17*/ { 3, -1, "1", 127, "ABCDEF123123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 18*/ { 3, -1, "A", 84, "ABCDEF123123", 0, 33, 30 },
        /* 19*/ { 3, -1, "A", 85, "ABCDEF123123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 20*/ { 3, 96, "1", 109, "ABCDEF123123", 0, 33, 30 },
        /* 21*/ { 3, 96, "1", 110, "ABCDEF123123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 22*/ { 0, -1, "1", 126, "123456789123123", 0, 33, 30 }, /* Mode 2 */
        /* 23*/ { 0, -1, "1", 127, "123456789123123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 24*/ { 0, -1, "1", 126, "ABCDEF123123", 0, 33, 30 }, /* Mode 3 */
        /* 25*/ { 0, -1, "1", 127, "ABCDEF123123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 26*/ { 5, -1, "1", 113, "", 0, 33, 30 }, /* Extra EEC */
        /* 27*/ { 5, -1, "1", 114, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 28*/ { 5, -1, "A", 77, "", 0, 33, 30 },
        /* 29*/ { 5, -1, "A", 78, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 30*/ { 6, -1, "1", 138, "", 0, 33, 30 },
        /* 31*/ { 6, -1, "1", 139, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 32*/ { 6, -1, "A", 93, "", 0, 33, 30 },
        /* 33*/ { 6, -1, "A", 94, "", ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[256];

    testStart("test_large");

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

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
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
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 01 21 21 21 21 21 21 21 21 08 0E 19 2B 20 0C 24 06 32 1C 21 21 21 21 21 21 21 21", 1, 1, "" },
        /*  1*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_DATA, 0, "Error 551: Invalid length for Primary Message", 1, 1, "" },
        /*  2*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "A123456", ZINT_ERROR_INVALID_DATA, 0, "Error 555: Non-numeric postcode in Primary Message", 1, 1, "" },
        /*  3*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "1123456", 0, 30, "(144) 12 00 00 00 00 10 30 1E 20 1C 1A 3D 1C 0D 1B 15 3C 17 3C 08 01 21 21 21 21 21 21 21", 1, 1, "1-digit postcode" },
        /*  4*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "1 123456", 0, 30, "(144) 12 00 00 00 00 10 30 1E 20 1C 1A 3D 1C 0D 1B 15 3C 17 3C 08 01 21 21 21 21 21 21 21", 1, 0, "1-digit postcode; ZXing-C++ test can't handle space" },
        /*  5*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "123456789123456", 0, 30, "(144) 12 05 0D 2F 35 11 32 1E 20 1C 0D 1D 3B 12 22 3F 30 14 23 1A 01 21 21 21 21 21 21 21", 1, 1, "9-digit postcode" },
        /*  6*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "1234567890123456", ZINT_ERROR_INVALID_DATA, 0, "Error 551: Invalid length for Primary Message", 1, 1, "10-digit postcode" },
        /*  7*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "1123456", 0, 30, "(144) 12 00 00 00 00 10 30 1E 20 1C 1A 3D 1C 0D 1B 15 3C 17 3C 08 01 21 21 21 21 21 21 21", 0, 1, "1-digit postcode; BWIPP requires mode" },
        /*  8*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "123456789123456", 0, 30, "(144) 12 05 0D 2F 35 11 32 1E 20 1C 0D 1D 3B 12 22 3F 30 14 23 1A 01 21 21 21 21 21 21 21", 0, 1, "9-digit postcode; BWIPP requires mode" },
        /*  9*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "1234567890123456", ZINT_ERROR_INVALID_DATA, 0, "Error 551: Invalid length for Primary Message", 1, 1, "10-digit postcode" },
        /* 10*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "123456", ZINT_ERROR_INVALID_DATA, 0, "Error 551: Invalid length for Primary Message", 1, 1, "0-digit postcode" },
        /* 11*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", -1, "12345678123456", 0, 30, "(144) 22 13 21 31 0B 00 32 1E 20 1C 04 14 07 30 10 07 08 28 1D 09 01 21 21 21 21 21 21 21", 0, 1, "8-digit postcode; BWIPP requires mode" },
        /* 12*/ { UNICODE_MODE, -1, 2, -1, { 0, 0, "" }, "A", -1, "12345678123456", 0, 30, "(144) 22 13 21 31 0B 00 32 1E 20 1C 04 14 07 30 10 07 08 28 1D 09 01 21 21 21 21 21 21 21", 1, 1, "8-digit postcode" },
        /* 13*/ { UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_DATA, 0, "Error 551: Invalid length for Primary Message", 1, 1, "" },
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
        /* 26*/ { UNICODE_MODE, -1, 0, -1, { 0, 0, "" }, "A", -1, "123456789123456", 0, 30, "(144) 12 05 0D 2F 35 11 32 1E 20 1C 0D 1D 3B 12 22 3F 30 14 23 1A 01 21 21 21 21 21 21 21", 0, 1, "Auto-determine mode 2; BWIPP requires mode for mode 2" },
        /* 27*/ { UNICODE_MODE, -1, 0, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_DATA, 0, "Error 554: Primary Message empty", 1, 1, "Auto-determine mode 2/3 requires primary message" },
        /* 28*/ { UNICODE_MODE, -1, 0, -1, { 0, 0, "" }, "A", -1, "A23456123456", 0, 30, "(144) 23 1D 0D 3D 2C 1C 30 1E 20 1C 24 35 30 31 2A 0D 17 14 16 3D 01 21 21 21 21 21 21 21", 1, 1, "Auto-determine mode 3" },
        /* 29*/ { UNICODE_MODE, -1, -1, 100, { 0, 0, "" }, "A", -1, "123456123456", 0, 30, "(144) 02 10 22 07 00 20 31 1E 20 1C 0E 29 13 1B 0D 26 36 25 3B 22 3B 2A 29 3B 28 1E 30 31", 0, 1, "SCM prefix version" },
        /* 30*/ { UNICODE_MODE, -1, -1, 101, { 0, 0, "" }, "A", -1, "123456123456", ZINT_ERROR_INVALID_OPTION, 0, "Error 557: Invalid SCM prefix version", 1, 1, "SCM prefix version" },
        /* 31*/ { UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 03 01 21 21 21 21 21 21 2F 14 23 21 05 24 27 00 24 0C 21 21 21 21 21 21 21 21", 1, 1, "" },
        /* 32*/ { UNICODE_MODE, 31, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 1F 01 21 21 21 21 21 21 00 2F 0E 09 39 3B 24 1A 21 05 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x1F" },
        /* 33*/ { UNICODE_MODE, 32, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 20 20 01 21 21 21 21 21 3D 15 0F 30 0D 22 24 35 22 06 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x20" },
        /* 34*/ { UNICODE_MODE, 1023, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 2F 3F 01 21 21 21 21 21 2E 27 23 1D 35 19 21 04 3A 26 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x3FF" },
        /* 35*/ { UNICODE_MODE, 1024, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 30 10 00 01 21 21 21 21 11 2F 15 10 1D 29 06 35 14 2B 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x400" },
        /* 36*/ { UNICODE_MODE, 32767, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 37 3F 3F 01 21 21 21 21 3E 15 12 01 07 30 39 27 04 2B 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x7FFF" },
        /* 37*/ { UNICODE_MODE, 32768, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 38 08 00 00 01 21 21 21 10 30 3A 04 26 23 0E 21 3D 0F 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x8000" },
        /* 38*/ { UNICODE_MODE, 65535, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 38 0F 3F 3F 01 21 21 21 1C 0E 1D 39 3B 0D 38 25 00 30 21 21 21 21 21 21 21 21", 1, 1, "ECI 0xFFFF" },
        /* 39*/ { UNICODE_MODE, 65536, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 38 10 00 00 01 21 21 21 2B 1F 24 06 38 2E 17 1B 10 2F 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x10000" },
        /* 40*/ { UNICODE_MODE, 131071, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 38 1F 3F 3F 01 21 21 21 0F 05 09 04 2F 3A 17 09 36 31 21 21 21 21 21 21 21 21", 1, 1, "ECI 0x1FFFF" },
        /* 41*/ { UNICODE_MODE, 999999, -1, -1, { 0, 0, "" }, "A", -1, "", 0, 30, "(144) 04 1B 3B 34 08 3F 01 21 21 21 26 3B 2B 23 08 17 32 05 26 35 21 21 21 21 21 21 21 21", 1, 1, "Max ECI" },
        /* 42*/ { UNICODE_MODE, -1, 1, -1, { 0, 0, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 550: Invalid MaxiCode Mode", 1, 1, "" },
        /* 43*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\015", -1, "", 0, 30, "(144) 04 00 21 21 21 21 21 21 21 21 37 32 10 01 24 1B 10 11 38 0C 21 21 21 21 21 21 21 21", 1, 0, "ZXing-C++ test can't handle LF" },
        /* 44*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\001\034\001\035\001\036\001a:b", -1, "", 0, 30, "(144) 04 3E 3E 01 20 01 21 01 22 01 27 0B 35 01 08 0D 16 02 17 1A 3F 01 33 02 21 21 21 21", 1, 1, "" },
        /* 45*/ { UNICODE_MODE, -1, -1, -1, { 1, 2, "" }, "A", -1, "", 0, 30, "(144) 04 21 01 01 21 21 21 21 21 21 09 0B 26 03 37 0E 25 27 07 1E 21 21 21 21 21 21 21 21", 1, 1, "" },
        /* 46*/ { UNICODE_MODE, -1, -1, -1, { 0, 2, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 559: Structured Append index out of range (1-2)", 1, 1, "" },
        /* 47*/ { UNICODE_MODE, -1, -1, -1, { 1, 1, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 558: Structured Append count out of range (2-8)", 1, 1, "" },
        /* 48*/ { UNICODE_MODE, -1, -1, -1, { 1, 9, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 558: Structured Append count out of range (2-8)", 1, 1, "" },
        /* 49*/ { UNICODE_MODE, -1, -1, -1, { 3, 2, "" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 559: Structured Append index out of range (1-2)", 1, 1, "" },
        /* 50*/ { UNICODE_MODE, -1, -1, -1, { 1, 2, "A" }, "A", -1, "", ZINT_ERROR_INVALID_OPTION, 0, "Error 549: Structured Append ID not available for MaxiCode", 1, 1, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStart("test_input");

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
    struct item data[] = {
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
        /*  1*/ { -1, 4, -1, { 0, 0, "" }, "MaxiCode (19 chars)", -1, "", 0, 33, 30, 0, "ISO/IEC 16023:2000 Figure H1 **NOT SAME** different encodation (figure uses '3 Shift A' among other differences); BWIPP different encodation again",
                    "001101011111011100000010101111"
                    "101100010001001100010000001100"
                    "101100001010001111001001111101"
                    "010101010101010101010101010100"
                    "000000000000000000000000000111"
                    "101010101010101010101010101000"
                    "010101010101010101010101010111"
                    "000000000000000000000000000010"
                    "101010101010101010101010101000"
                    "010101011111111100000001010100"
                    "000000000011110110001000000000"
                    "101010101110000000111010101000"
                    "010101100010000000001101010101"
                    "000000101000000000001000000000"
                    "101010000000000000011010101000"
                    "010101010000000000001101010100"
                    "000000001000000000001000000011"
                    "101010110000000000001010101010"
                    "010101101100000000010101010111"
                    "000000100000000000000000000000"
                    "101010010110000000000110101011"
                    "010101010110000000001001010100"
                    "000000000110001011000000000010"
                    "101010100110111001010010101000"
                    "010101010101010101010000101111"
                    "000000000000000000001100100000"
                    "101010101010101010100101000001"
                    "000011000111010110101100010000"
                    "111001111110111110011000111111"
                    "000001110010000010110001100100"
                    "000111000000001111011000010010"
                    "010110010110001110100000010100"
                    "010011110011000001010111100111"
                },
        /*  2*/ { DATA_MODE | ESCAPE_MODE, 2, 96, { 0, 0, "" }, "1Z00004951\\GUPSN\\G06X610\\G159\\G1234567\\G1/1\\G\\GY\\G634 ALPHA DR\\GPITTSBURGH\\GPA\\R\\E", -1, "152382802840001", 0, 33, 30, 0, "ISO/IEC 16023:2000 Figure B2 **NOT SAME** uses different encodation (figure precedes PAD chars with Latch B); BWIPP different encodation again",
                    "110101110110111110111111101111"
                    "010101010111000011011000010010"
                    "110110110001001010101010010011"
                    "111000101010101111111111111100"
                    "001111000010110010011000000011"
                    "001001110010101010100000000000"
                    "111011111110111111101111111110"
                    "100110000011001001110000001010"
                    "010001100010101010101001110001"
                    "110111100011010000011011111100"
                    "001100110011110000001110101001"
                    "101110101000000001011111011000"
                    "101010000000000000010110111100"
                    "111101100000000000011011100010"
                    "101010010000000000000110011101"
                    "001000010000000000011100011110"
                    "010011001000000000001000001010"
                    "000000101000000000001010000010"
                    "000100111100000000001110101010"
                    "000010101100000000001000110010"
                    "100000111010000000011101100011"
                    "101000100000000000110110100000"
                    "001000001110100101100110100101"
                    "011001110010101001100000001000"
                    "000010100010110001010101011010"
                    "100111000011111000001001011000"
                    "110010001001010010101100011101"
                    "001001110101110100011001110010"
                    "011111010011101100111101010011"
                    "111111101111101010101101111000"
                    "101001110101110111010111000011"
                    "010110101101000001111000100110"
                    "110110100000010000001011110011"
                },
        /*  3*/ { -1, 3, -1, { 0, 0, "" }, "CEN", -1, "B1050056999", 0, 33, 30, 1, "ISO/IEC 16023:2000 B.1 Example (primary only given, data arbitrary); verified manually against tec-it",
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
        /*  4*/ { UNICODE_MODE | ESCAPE_MODE, -1, -1, { 0, 0, "" }, "Comité Européen de Normalisation\034rue de Stassart 36\034B-1050 BRUXELLES\034TEL +3225196811", -1, "", 0, 33, 30, 0, "ISO/IEC 16023:2000 Example F.5 **NOT SAME** uses different encodation (2 Shift A among other things); BWIPP different encodation again",
                    "010010100010110000000100001111"
                    "001010001100110110111110100110"
                    "001010011100101010011100100000"
                    "000000100010000000001000000110"
                    "111101100000011100110011110001"
                    "011110001011100010100111010010"
                    "101010000100001101101000101001"
                    "110010101110100100001000000000"
                    "000110101100100000110010111110"
                    "111101111111111000110110100000"
                    "100000111001111010010010000011"
                    "011100111100000000101100011010"
                    "100001101110000000101111111101"
                    "110011000100000000111100001010"
                    "000110010000000000010110001010"
                    "101010010000000000001000011100"
                    "011000001000000000001000000010"
                    "001001010000000000001101000000"
                    "000000010000000000010100101000"
                    "101111110100000000011110001100"
                    "100000000010000000011010110011"
                    "101001010010000001011100001010"
                    "001101000010001011110111101010"
                    "111111001010000001100100100000"
                    "001000001101010101010010111001"
                    "111001000000000000001010100000"
                    "010001101010101010101010110001"
                    "011011000011100001011001101100"
                    "101100000001111010000001100011"
                    "110001001100011100110111011010"
                    "011110010010101101110100000100"
                    "001011110011100001001001101100"
                    "000010111011111010110011000011"
                },
        /*  5*/ { -1, -1, -1, { 0, 0, "" }, "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999", -1, "", 0, 33, 30, 1, "Numeric compaction, verified manually against tec-it",
                    "010111101101010111101101010111"
                    "111011110110111011110110111010"
                    "001111111101001111111101001100"
                    "101101010111101101010111101100"
                    "111110111001111110111001111100"
                    "111111000111111111000111111110"
                    "110101011110110101011110110110"
                    "011011101111011011101111011000"
                    "110100111111110100111111110101"
                    "010111101111111100110111010100"
                    "111001111011011110011111101100"
                    "000111111110000000111011110000"
                    "011110100110000000000111010101"
                    "101111001000000000110101101110"
                    "111111110100000000000011010001"
                    "010111111000000000001110110110"
                    "111001101000000000001011111001"
                    "000111010000000000001011111100"
                    "011110011000000000011011010101"
                    "101111000100000000010001101100"
                    "111111100110000000100111010010"
                    "010101110100000001010110110110"
                    "101110011010101111111011111011"
                    "110001110111110101111011111110"
                    "111011010101111111110000111011"
                    "111101101110110101010001001000"
                    "111111010011111010101111110011"
                    "000101011000111100010001000010"
                    "011110001101100001011010110010"
                    "101110000100010011000001001000"
                    "100000001010110100100110001100"
                    "111010101011001101111001011010"
                    "011110011111000011101011111011"
                },
        /*  6*/ { -1, 5, -1, { 0, 0, "" }, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\037\237\240\242\243\244\245\246\247\251\255\256\266\225\226\227\230\231\232\233\234\235\236", 51, "", 0, 33, 30, 1, "Mode 5 set E",
                    "000000000000000000101010101011"
                    "100101010111111111000000001010"
                    "110010011100100111001001110010"
                    "010101010101011010101010101010"
                    "010110101010001101010110101001"
                    "100011011000110101100011011000"
                    "010101010101111111111111111110"
                    "010111111111000000001010101010"
                    "011100100111001001110010011101"
                    "010101011011110000001011111110"
                    "000000001111110010010001010110"
                    "101010100110000010001001100000"
                    "010101110010000000001101010111"
                    "000000111000000000001000000010"
                    "101010110100000000001110101011"
                    "010101010000000000001001010100"
                    "000000001000000000001000000001"
                    "101010100000000000001010101000"
                    "010101100000000000001001010110"
                    "000000000000000000000000000000"
                    "101010100100000000011110101010"
                    "101100110100000001110101110110"
                    "011000000010110101110111000011"
                    "110111000010110001001000011010"
                    "100001101111010001110110101000"
                    "111110011001100100010110010010"
                    "100011110000001110111011000001"
                    "111001000110000011101000011000"
                    "101110110100100001100011011111"
                    "101100010011001000110000101100"
                    "110101001101000000111101001111"
                    "100111110000101000000001110100"
                    "100101010010100000010101000111"
                },
        /*  7*/ { -1, 6, -1, { 0, 0, "" }, "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377\241\250\253\257\260\264\267\270\273\277\212\213\214\215\216\217\220\221\222\223\224", -1, "", 0, 33, 30, 1, "Mode 6 set D",
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
        /*  8*/ { -1, 6, -1, { 0, 0, "" }, "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\252\254\261\262\263\265\271\272\274\275\276\200\201\202\203\204\205\206\207\210\211", -1, "", 0, 33, 30, 1, "Mode 6 set C",
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
        /*  9*/ { UNICODE_MODE | ESCAPE_MODE, 4, -1, { 0, 0, "" }, "`abcdefghijklmnopqrstuvwxyz\034\\G\\R{}~\177;<=>?[\\\\]^_ ,./:@!|", -1, "", 0, 33, 30, 1, "Mode 4 Set B",
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
        /* 10*/ { UNICODE_MODE | ESCAPE_MODE, 4, -1, { 0, 0, "" }, "\\rABCDEFGHIJKLMNOPQRSTUVWXYZ\034\\G\\R \"#$%&'()*+,-./0123456789:", -1, "", 0, 33, 30, 1, "Mode 4 Set A",
                    "000000000000001010101010101011"
                    "010101111111110000000010101010"
                    "100111001001110010011100100101"
                    "010101010101011010101010101010"
                    "101010100000001111110101011011"
                    "110110001101000110000110001110"
                    "010101010101010110000100000011"
                    "010101011111111111001110101100"
                    "001001110010011111001101100110"
                    "010101010011000000000001111100"
                    "000000001001001010010100010100"
                    "101010100010000000010010011010"
                    "010101000000000000000001010100"
                    "000000110100000000100100000000"
                    "101010110000000000001010101011"
                    "010101010000000000001101010100"
                    "000000001000000000001000000010"
                    "101010100000000000001010101000"
                    "010101001100000000001101010100"
                    "000000111000000000101100000000"
                    "101010000010000000011110101001"
                    "010101010000000000100101010110"
                    "000000001110000001101000000001"
                    "101010100011000001001110101000"
                    "010101010101010101010011100001"
                    "000000000000000000000100010110"
                    "101010101010101010100100011110"
                    "101100000000000010001010010000"
                    "110010011000011000011011011001"
                    "111000110101100100110110101000"
                    "001001100010110011111101010101"
                    "110111010000000001010000100110"
                    "101001001010011101111100111011"
                },
        /* 11*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "ABCDabcdAabcABabcABCabcABCDaABCabABCabcABCéa", -1, "", 0, 33, 30, 1, "Mode 4 LCHB SHA 2SHA 3SHA LCHA SHB LCHB 3SHA 3SHA SHD",
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
        /* 12*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "ÀÁÂÃ1", -1, "", 0, 33, 30, 1, "Mode 4 LCKC LCHA",
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
        /* 13*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "ÀÁÂÃ123456789", -1, "", 0, 33, 30, 1, "Mode 4 LCKC NS",
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
        /* 14*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "àáâã1", -1, "", 0, 33, 30, 1, "Mode 4 LCKD LCHA",
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
        /* 15*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "¢£¤¥1", -1, "", 0, 33, 30, 1, "Mode 4 LCKE LCHA",
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
        /* 16*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "¢£¤¥123456789", -1, "", 0, 33, 30, 1, "Mode 4 LCKE NS",
                    "111110110101010101010101010111"
                    "111010010000000000000000000000"
                    "000010011010101010101010101000"
                    "010101010101010101010101010100"
                    "000000000000000000000000000001"
                    "101010101010101010101010101010"
                    "010101010101010101010101010101"
                    "000000000000000000000000000000"
                    "101010101010101010101010101001"
                    "010101011111110100111001010110"
                    "000000001001110110011100000010"
                    "101010100110000000111010101010"
                    "010101001010000000000101010101"
                    "000000001000000000110100000000"
                    "101010000100000000000110101011"
                    "010101011000000000001101010110"
                    "000000001000000000001000000000"
                    "101010100000000000001010101000"
                    "010101010100000000000101010100"
                    "000000010000000000111100000010"
                    "101010101100000000110010101001"
                    "010101010110000001001001010100"
                    "000000001110110001011000000000"
                    "101010101010110101010010101010"
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
        /* 17*/ { UNICODE_MODE, 4, -1, { 0, 0, "" }, "ABCDE12abcde1ÀÁÂÃ¢£¤¥1àáâãabcde123A123456789àáâã¢£¤¥abc", -1, "", 0, 33, 30, 1, "Mode 4 mixed sets",
                    "000000001111111100000000111111"
                    "000010100100111100000000111100"
                    "011100101110000000100111010100"
                    "001100000000111111110101010110"
                    "001100000000111100010101101001"
                    "101111011000101010011000110110"
                    "000000001111111100100010111110"
                    "000010101100000000111010111000"
                    "011100101110011110111101000010"
                    "000000001011001100010111111010"
                    "000000000001000010000111111000"
                    "110110001110000000011110101010"
                    "111101000010000000001101010101"
                    "111110110000000000100110010100"
                    "010101101000000000010111001001"
                    "010100110000000000001100001110"
                    "000000101000000000001000001110"
                    "101011110000000000001101101110"
                    "010101101000000000000001010101"
                    "000000000100000000010000000000"
                    "101010100000000000000110101000"
                    "010101010100000000000101010100"
                    "000000001010001011100000000011"
                    "101010101011000001011010101000"
                    "010101010101010101010101001001"
                    "000000000000000000000010100000"
                    "101010101010101010101111110001"
                    "001110011111110100100011111000"
                    "011001101011100100010000010101"
                    "010000011110010110000011110010"
                    "110111100111000100111111001011"
                    "100011000001110011101110101000"
                    "001001110010111101100100010001"
                },
        /* 18*/ { UNICODE_MODE, 4, -1, { 3, 7, "" }, "THIS IS A 91 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, APPENDED, MAXICODE SYMBOL...", -1, "", 0, 33, 30, 1, "Mode 4 Structured Append",
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
        /* 19*/ { UNICODE_MODE, 3, -1, { 1, 8, "" }, "COMMISSION FOR EUROPEAN NORMALIZATION, RUE DE STASSART 36, B-1050 BRUXELLES", -1, "B1050056999", 0, 33, 30, 1, "Mode 3 Structured Append",
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
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStart("test_encode");

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
    struct item data[] = {
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
        /*  4*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("Pixel 4a 128 GB:$439.97"), -1, 3 }, { TU("Pixel 4a 128 GB:￥3149.79"), -1, 29 }, { TU("Pixel 4a 128 GB:444,90 €"), -1, 17 } }, "", 0, 33, 30, 0, "AIM ITS/04-023:2022 Annex A example (shortened)",
                    "011100110111111101000011011111"
                    "001000110000000100100001101000"
                    "000010110010010000110101000010"
                    "110000100011101010111101111100"
                    "011110010111001101100111010010"
                    "000010001011001011111001101100"
                    "011100110111111101000011110111"
                    "001000110000000100100001111000"
                    "000010110010010000110101011110"
                    "011111110011001000000111011100"
                    "110110000101000110011000011110"
                    "011000100000000000101011000100"
                    "111110101110000000011110101100"
                    "100101100100000000001100001100"
                    "111011100000000000000110001101"
                    "011100010000000000000000100010"
                    "000111001000000000001010010110"
                    "000000110000000000001010001010"
                    "110011010100000000011001111111"
                    "100011101100000000111000000000"
                    "001011100010000000011000100100"
                    "111111000010000001111000011110"
                    "101001000110101011100010000110"
                    "000001010110011101000111000010"
                    "110111110111010101010010100001"
                    "101101000011010000000001011000"
                    "000010000001001010100100111001"
                    "101010010101100010111000111000"
                    "000111001101011101010101101000"
                    "110111101110001101100111010100"
                    "101000010100101000011010011110"
                    "100000110001100001110001101000"
                    "010000101110100111010101100101"
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
                    "010101010101010101010101010111"
                    "110000000000000000000000000010"
                    "111010101010101010101010101011"
                    "010101010101010101010101010110"
                    "000000000000000000000000000000"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000010"
                    "101010101010101010101010101001"
                    "010101010011010100111101010110"
                    "000000000011111110000100000010"
                    "101010100010000000111010101010"
                    "010101001110000000110001010100"
                    "000000001100000000101000000000"
                    "101010000100000000010010101000"
                    "010101011000000000001101010110"
                    "000000001000000000001000000010"
                    "101010001000000000001010101000"
                    "010101001000000000000001010101"
                    "000000111100000000110100000000"
                    "101010011010000000011010101000"
                    "010101011010000001010101010100"
                    "000000001010110101000100000011"
                    "101010101111110001011110101010"
                    "010101010101010101010011100111"
                    "000000000000000000000111010110"
                    "101010101010101010100100000010"
                    "110011010010110000000111001110"
                    "011010011101001011010111010101"
                    "101010000011010101001111100110"
                    "001110010010000001111001111010"
                    "000111101111000100101110001100"
                    "000100001000100111100110010100"
                },
        /*  7*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("αβ"), -1, 0 }, { TU("ÿ"), -1, 0 }, { TU("貫やぐ禁"), -1, 20 } }, "", ZINT_WARN_USES_ECI, 33, 30, 1, "Auto-ECI",
                    "011010110111101111110011111111"
                    "100110111111001100110011001110"
                    "001100101100100001100100010111"
                    "010101010101010101101111110100"
                    "000000000000000000101100111110"
                    "101010101010101010010000100110"
                    "010101010101010101010101010111"
                    "000000000000000000000000000000"
                    "101010101010101010101010101000"
                    "010101010111000000110101010100"
                    "000000000011010010001100000010"
                    "101010101100000000111010101010"
                    "010101101100000000110101010110"
                    "000000011100000000011100000000"
                    "101010001000000000011110101000"
                    "010101011000000000001101010110"
                    "000000001000000000001000000011"
                    "101010001000000000001010101010"
                    "010101010000000000001001010100"
                    "000000010100000000000000000000"
                    "101010110010000000110110101000"
                    "010101010010000001110101010110"
                    "000000000010101011100100000001"
                    "101010101011010001100010101000"
                    "010101010101010101011010000110"
                    "000000000000000000001001001100"
                    "101010101010101010100101111110"
                    "110111000101110010100001011100"
                    "111110011010010010000011001011"
                    "111000110110111111111000010100"
                    "101001001111001011110110101001"
                    "101010010100011001011101100110"
                    "111011110000111001101101111000"
                },
        /*  8*/ { UNICODE_MODE, -1, -1, { 1, 2, "" }, { { TU("αβ"), -1, 9 }, { TU("ÿ"), -1, 3 }, { TU("貫やぐ禁"), -1, 20 } }, "", 0, 33, 30, 1, "Structured Append",
                    "001101101011011110111111001111"
                    "001110011011111100110011001110"
                    "111000110010110010000110010011"
                    "010101010101011011111101111110"
                    "000000000000001011001111110011"
                    "101010101010100100001001010100"
                    "010101010101010101010101010111"
                    "000000000000000000000000000010"
                    "101010101010101010101010101011"
                    "010101010011000000111101010110"
                    "000000001101000010010000000011"
                    "101010101010000000100010101000"
                    "010101111000000000111101010111"
                    "000000000100000000100000000000"
                    "101010001100000000011010101010"
                    "010101011000000000000101010110"
                    "000000001000000000001000000000"
                    "101010001000000000001110101000"
                    "010101100000000000011001010100"
                    "000000111000000000001000000010"
                    "101010100110000000010110101011"
                    "010101011010000000001001010110"
                    "000000001010011011100000000010"
                    "101010101011000001000010101000"
                    "010101010101010101011111010011"
                    "000000000000000000000100001000"
                    "101010101010101010100101111111"
                    "111101010111011110110011111100"
                    "000011111110111101111100110001"
                    "001101101111000110000011100010"
                    "111110010111110110000111100100"
                    "101010011010110110110010010100"
                    "100101010111100011100010101000"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStart("test_encode_segs");

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
                        ret = testUtilBwippSegs(i, symbol, -1, data[i].option_2, -1, data[i].segs, seg_count, NULL, cmp_buf, sizeof(cmp_buf));
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

static void test_best_supported_set(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        int ret;
        float w;
        float h;
        int ret_vector;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /* 0*/ { "am.//ab,\034TA# z\015!", 0, 100, 100, 0, 33, 30, "Different encodation than BWIPP, same number of codewords",
                    "111010000101111000111101010111"
                    "111110000000010100111000000000"
                    "110000101100110100111010101011"
                    "010101010101010101010101010100"
                    "000000000000000000000000000000"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000000"
                    "101010101010101010101010101011"
                    "010101010111001100000101010110"
                    "000000001011000010000000000010"
                    "101010101100000000100110101010"
                    "010101001100000000101101010101"
                    "000000100000000000010000000010"
                    "101010110000000000010010101010"
                    "010101011000000000000101010110"
                    "000000001000000000001000000010"
                    "101010001000000000001010101000"
                    "010101010000000000001101010101"
                    "000000001100000000000000000010"
                    "101010110010000000010110101010"
                    "010101010100000001111001010100"
                    "000000001110110111111100000011"
                    "101010100110111101011010101010"
                    "010101010101010101010011101000"
                    "000000000000000000001101100000"
                    "101010101010101010100000100110"
                    "101001001101110001001011010000"
                    "100100110110001010011000011100"
                    "011011000001011011100100100110"
                    "111001100000101101000111001000"
                    "111100000110000011011101001110"
                    "010100101001110111101010110010"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped_data[1024];

    testStart("test_best_supported_set");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAXICODE, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (p_ctx->generate) {
            printf("        /*%2d*/ { \"%s\", %d, %.0f, %.0f, %d, %d, %d, \"%s\",\n",
                    i, testUtilEscape(data[i].data, length, escaped_data, sizeof(escaped_data)), ret,
                    data[i].w, data[i].h, data[i].ret_vector, symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ",  "\n");
            printf("                },\n");
        } else {
            int width, row;

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
            assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
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
    struct item data[] = {
        /* 0*/ { -1, "\223\223\223\223\223\200\000\060\060\020\122\104\060\343\000\000\040\104\104\104\104\177\377\040\000\324\336\000\000\000\000\104\060\060\060\060\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\060\104\104\000\000\000\040\104\104\104\104\177\377\377\377\324\336\000\000\000\000\104\377\104\001\104\104\104\104\104\104\233\233\060\060\060\060\060\060\060\060\060\325\074", 107, ZINT_ERROR_TOO_LONG }, /* Original OSS-Fuzz triggering data */
        /* 1*/ { -1, "AaAaAaAaAaAaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA123456789", -1, ZINT_ERROR_TOO_LONG }, /* Add 6 lowercase a's so 6 SHIFTS inserted so 6 + 138 (max input len) = 144 and numbers come at end of buffer */
        /* 2*/ { -1, "AaAaAaAaAaAaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA123456789A", -1, ZINT_ERROR_TOO_LONG },
        /* 3*/ { -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", -1, ZINT_ERROR_TOO_LONG },
        /* 4*/ { 32768, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 5*/ { -1, "AaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAaAa", -1, ZINT_ERROR_TOO_LONG },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_fuzz");

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

#define TEST_PERF_ITERATIONS    1000

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
    struct item data[] = {
        /*  0*/ { BARCODE_MAXICODE, UNICODE_MODE | ESCAPE_MODE, -1, -1,
                    "1Z34567890\\GUPSN\\G102562\\G034\\G\\G1/1\\G\\GY\\G2201 Second St\\GFt Myers\\GFL\\R\\E",
                    "339010000840001", 0, 33, 30, "Mode 2" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    clock_t start, total_encode = 0, total_buffer = 0, diff_encode, diff_buffer;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_encode = diff_buffer = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            struct zint_symbol *symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
            strcpy(symbol->primary, data[i].primary);

            start = clock();
            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

            ZBarcode_Delete(symbol);
        }

        printf("%s: diff_encode %gms, diff_buffer %gms\n", data[i].comment, diff_encode * 1000.0 / CLOCKS_PER_SEC, diff_buffer * 1000.0 / CLOCKS_PER_SEC);

        total_encode += diff_encode;
        total_buffer += diff_buffer;
    }
    if (p_ctx->index != -1) {
        printf("totals: encode %gms, buffer %gms\n", total_encode * 1000.0 / CLOCKS_PER_SEC, total_buffer * 1000.0 / CLOCKS_PER_SEC);
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_encode_segs", test_encode_segs },
        { "test_best_supported_set", test_best_supported_set },
        { "test_fuzz", test_fuzz },
        { "test_perf", test_perf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
