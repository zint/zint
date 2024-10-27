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
        int symbology;
        int option_1;
        int option_2;
        int option_3;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, 0, -1, -1, "A", 1850, 0, 32, 562, "" },
        /*  1*/ { BARCODE_PDF417, 0, -1, -1, "A", 1851, ZINT_ERROR_TOO_LONG, -1, -1, "Error 464: Input too long, requires too many codewords (maximum 928)" },
        /*  2*/ { BARCODE_PDF417, 0, -1, -1, "\200", 1108, 0, 32, 562, "" },
        /*  3*/ { BARCODE_PDF417, 0, -1, -1, "\200", 1109, ZINT_ERROR_TOO_LONG, -1, -1, "Error 464: Input too long, requires too many codewords (maximum 928)" },
        /*  4*/ { BARCODE_PDF417, 0, -1, -1, "1", 2710, 0, 32, 562, "" },
        /*  5*/ { BARCODE_PDF417, 0, -1, -1, "1", 2711, ZINT_ERROR_TOO_LONG, -1, -1, "Error 463: Input length 2711 too long (maximum 2710)" },
        /*  6*/ { BARCODE_PDF417, 0, -1, 59, "A", 1850, ZINT_ERROR_TOO_LONG, -1, -1, "Error 465: Input too long, requires too many codewords (maximum 928)" },
        /*  7*/ { BARCODE_PDF417, 0, 1, 3, "A", 1850, ZINT_ERROR_TOO_LONG, -1, -1, "Error 745: Input too long for number of columns '1'" },
        /*  8*/ { BARCODE_PDF417, 0, -1, 3, "A", 1850, ZINT_WARN_INVALID_OPTION, 32, 562, "Warning 746: Number of rows increased from 3 to 32" },
        /*  9*/ { BARCODE_PDF417, 0, 30, -1, "A", 1850, ZINT_ERROR_TOO_LONG, -1, -1, "Error 747: Input too long, requires too many codewords (maximum 928)" },
        /* 10*/ { BARCODE_MICROPDF417, 0, -1, -1, "A", 250, 0, 44, 99, "" },
        /* 11*/ { BARCODE_MICROPDF417, 0, -1, -1, "A", 251, ZINT_ERROR_TOO_LONG, -1, -1, "Error 467: Input too long, requires 127 codewords (maximum 126)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[4096];

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].length != -1) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
            assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));
        } else {
            strcpy(data_buf, data[i].pattern);
        }

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, symbol->errtxt);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        /* FAST_MODE */

        length = testUtilSetSymbol(symbol, data[i].symbology, FAST_MODE, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, symbol->errtxt);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_1;
        int option_2;
        int option_3;
        int warn_level;
        struct zint_structapp structapp;
        char *data;
        int ret_encode;
        int ret_vector;

        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
        int compare_previous;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, 0, { 0, 0, "" }, "12345", 0, 0, 6, 103, "", -1 }, /* ECC auto-set to 2, cols auto-set to 2 */
        /*  1*/ { BARCODE_PDF417, -1, -1, 928, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 466: Number of rows '928' out of range (3 to 90)", -1 }, /* Option 3 no longer ignored */
        /*  2*/ { BARCODE_PDF417, -1, -1, 1, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 466: Number of rows '1' out of range (3 to 90)", -1 }, /* Option 3 no longer ignored */
        /*  3*/ { BARCODE_PDF417, 3, -1, -1, 0, { 0, 0, "" }, "12345", 0, 0, 7, 120, "", -1 }, /* ECC 3, cols auto-set to 3 */
        /*  4*/ { BARCODE_PDF417, 3, 2, -1, 0, { 0, 0, "" }, "12345", 0, 0, 10, 103, "", -1 }, /* ECC 3, cols 2 */
        /*  5*/ { BARCODE_PDF417, 8, 2, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 86, 171, "Warning 748: Number of columns increased from 2 to 6", -1 }, /* ECC 8, cols 2, used to fail, now auto-upped to 3 with warning */
        /*  6*/ { BARCODE_PDF417, 8, 2, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, 0, 86, 171, "Error 748: Number of columns increased from 2 to 6", -1 },
        /*  7*/ { BARCODE_PDF417, 7, 2, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 87, 120, "Warning 748: Number of columns increased from 2 to 3", -1 }, /* ECC 7, cols 2 auto-upped to 3 but now with warning */
        /*  8*/ { BARCODE_PDF417, 7, 2, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, 0, 87, 120, "Error 748: Number of columns increased from 2 to 3", -1 },
        /*  9*/ { BARCODE_PDF417, -1, 10, -1, 0, { 0, 0, "" }, "12345", 0, 0, 3, 239, "", -1 }, /* ECC auto-set to 2, cols 10 */
        /* 10*/ { BARCODE_PDF417, 9, -1, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 6, 103, "Warning 460: Error correction level '9' out of range (0 to 8), ignoring", -1 }, /* Invalid ECC, auto-set */
        /* 11*/ { BARCODE_PDF417, -1, 31, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 6, 103, "Warning 461: Number of columns '31' out of range (1 to 30), ignoring", 0 }, /* Invalid cols, auto-set */
        /* 12*/ { BARCODE_PDF417, -1, -1, 2, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 466: Number of rows '2' out of range (3 to 90)", -1 }, /* Invalid rows, error */
        /* 13*/ { BARCODE_PDF417, -1, -1, 91, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 466: Number of rows '91' out of range (3 to 90)", -1 }, /* Invalid rows, error */
        /* 14*/ { BARCODE_PDF417, 9, -1, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 460: Error correction level '9' out of range (0 to 8)", -1 }, /* Invalid ECC */
        /* 15*/ { BARCODE_PDF417, -1, 31, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 461: Number of columns '31' out of range (1 to 30)", -1 }, /* Invalid cols */
        /* 16*/ { BARCODE_PDF417, -1, 30, 31, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 475: Columns x rows value '930' out of range (1 to 928)", -1 }, /* Rows * cols (930) > 928 */
        /* 17*/ { BARCODE_PDF417, -1, 1, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI", ZINT_WARN_INVALID_OPTION, 0, 65, 120, "Warning 748: Number of columns increased from 1 to 3", -1 }, /* Cols 1 too small, used to fail, now auto-upped to 3 with warning */
        /* 18*/ { BARCODE_PDF417, -1, -1, 4, 0, { 0, 0, "" }, "12345", 0, 0, 4, 120, "", -1 }, /* Specify rows 4 (cols 3) */
        /* 19*/ { BARCODE_PDF417, -1, 3, 4, 0, { 0, 0, "" }, "12345", 0, 0, 4, 120, "", 0 }, /* Specify cols 3 & rows 4 */
        /* 20*/ { BARCODE_PDF417, -1, -1, 90, 0, { 0, 0, "" }, "12345", 0, 0, 90, 86, "", -1 }, /* Specify rows 90 (cols 1) */
        /* 21*/ { BARCODE_PDF417, 0, -1, 3, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", 0, 0, 3, 579, "", -1 }, /* Specify rows 3, max cols 30 */
        /* 22*/ { BARCODE_PDF417, 0, 30, 3, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", 0, 0, 3, 579, "", 0 }, /* Specify rows 3, cols 30 */
        /* 23*/ { BARCODE_PDF417, 0, 29, 3, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_WARN_INVALID_OPTION, 0, 4, 562, "Warning 746: Number of rows increased from 3 to 4", -1 }, /* Specify rows 3, cols 29, rows auto-upped to 4 */
        /* 24*/ { BARCODE_MICROPDF417, -1, 5, -1, 0, { 0, 0, "" }, "12345", ZINT_WARN_INVALID_OPTION, 0, 11, 38, "Warning 468: Number of columns '5' out of range (1 to 4), ignoring", -1 }, /* Invalid cols, auto-set to 1 */
        /* 25*/ { BARCODE_MICROPDF417, -1, 5, -1, WARN_FAIL_ALL, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 471: Number of columns '5' out of range (1 to 4)", -1 }, /* Invalid cols */
        /* 26*/ { BARCODE_MICROPDF417, -1, 5, 3, 0, { 0, 0, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 476: Cannot specify rows for MicroPDF417", -1 }, /* Rows option not available */
        /* 27*/ { BARCODE_MICROPDF417, -1, 1, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM", ZINT_WARN_INVALID_OPTION, 0, 17, 55, "Warning 470: Input too long for number of columns '1', ignoring", -1 }, /* Cols 1 too small, auto-upped to 2 with warning */
        /* 28*/ { BARCODE_MICROPDF417, -1, 1, -1, WARN_FAIL_ALL, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM", ZINT_ERROR_INVALID_OPTION, ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 469: Input too long for number of columns '1', requires 21 codewords (maximum 20)", -1 }, /* Cols 1 too small */
        /* 29*/ { BARCODE_MICROPDF417, -1, 2, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWX", ZINT_WARN_INVALID_OPTION, 0, 15, 99, "Warning 470: Input too long for number of columns '2', ignoring", -1 }, /* Cols 2 too small, auto-upped to 4 with warning */
        /* 30*/ { BARCODE_MICROPDF417, -1, 2, -1, WARN_FAIL_ALL, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWX", ZINT_ERROR_INVALID_OPTION, ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 469: Input too long for number of columns '2', requires 38 codewords (maximum 37)", -1 }, /* Cols 2 too small */
        /* 31*/ { BARCODE_MICROPDF417, -1, 3, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKL", ZINT_WARN_INVALID_OPTION, 0, 32, 99, "Warning 470: Input too long for number of columns '3', ignoring", -1 }, /* Cols 3 too small, auto-upped to 4 with warning */
        /* 32*/ { BARCODE_MICROPDF417, -1, 3, -1, WARN_FAIL_ALL, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKL", ZINT_ERROR_INVALID_OPTION, ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 469: Input too long for number of columns '3', requires 83 codewords (maximum 82)", -1 }, /* Cols 3 too small */
        /* 33*/ { BARCODE_PDF417, -1, 1, -1, 0, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Number of columns increased from 1 to 2", -1 }, /* Cols 1 auto-upped to 2 just fits, now with warning */
        /* 34*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH", ZINT_WARN_INVALID_OPTION, 0, 67, 120, "Warning 748: Number of columns increased from 1 to 3", -1 }, /* Cols 1 too small with Structured Append, used to fail, now auto-upped to 3 with warning */
        /* 35*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Number of columns increased from 1 to 2", -1 }, /* Cols 1 with Structured Append auto-upped to 2 just fits, now with warning */
        /* 36*/ { BARCODE_PDF417, -1, 1, -1, 0, { 2, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", ZINT_WARN_INVALID_OPTION, 0, 65, 120, "Warning 748: Number of columns increased from 1 to 3", -1 }, /* Cols 1 too small with Structured Append as last symbol (uses extra terminating codeword), used to fail, now auto-upped to 3 with warning */
        /* 37*/ { BARCODE_PDF417, -1, 1, -1, 0, { 2, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Number of columns increased from 1 to 2", -1 }, /* Cols 1 with Structured Append as last symbol just fits with 1 less character pair when auto-upped to 2, now with warning */
        /* 38*/ { BARCODE_PDF417, -1, 1, -1, 0, { 3, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 741: Structured Append index '3' out of range (1 to count 2)", -1 },
        /* 39*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 1, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 740: Structured Append count '1' out of range (2 to 99999)", -1 },
        /* 40*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 100000, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 740: Structured Append count '100000' out of range (2 to 99999)", -1 },
        /* 41*/ { BARCODE_PDF417, -1, 1, -1, 0, { 0, 2, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 741: Structured Append index '0' out of range (1 to count 2)", -1 },
        /* 42*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "1" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Number of columns increased from 1 to 2", -1 }, /* Now with warning */
        /* 43*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "123123" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOP", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Number of columns increased from 1 to 2", -1 }, /* Now with warning */
        /* 44*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "123123123123123123123123123123" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Number of columns increased from 1 to 2", -1 }, /* Now with warning */
        /* 45*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "1231231231231231231231231231231" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 742: Structured Append ID length 31 too long (30 digit maximum)", -1 },
        /* 46*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "23123123123123123123123123123" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", ZINT_WARN_INVALID_OPTION, 0, 89, 103, "Warning 748: Number of columns increased from 1 to 2", -1 }, /* Now with warning */
        /* 47*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "A" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 743: Invalid Structured Append ID (digits only)", -1 },
        /* 48*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "900" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQR", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 744: Structured Append ID triplet 1 value '900' out of range (000 to 899)", -1 },
        /* 49*/ { BARCODE_PDF417, -1, 1, -1, 0, { 1, 2, "123123123123123123123123901123" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 744: Structured Append ID triplet 9 value '901' out of range (000 to 899)", -1 },
        /* 50*/ { BARCODE_MICROPDF417, -1, -1, -1, 0, { 1, 2, "1231231231231231231231231231231" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 742: Structured Append ID length 31 too long (30 digit maximum)", -1 }, /* Micro PDF417 same error checking code */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_symbol previous_symbol;

    testStartSymbol("test_options", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].warn_level) {
            symbol->warn_level = data[i].warn_level;
        }
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

        if (data[i].option_3 != -1) {
            assert_equal(symbol->option_3, data[i].option_3, "i:%d symbol->option_3 %d != %d\n", i, symbol->option_3, data[i].option_3); /* Unchanged */
        } else {
            assert_zero(symbol->option_3, "i:%d symbol->option_3 %d != 0\n", i, symbol->option_3);
        }

        assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, symbol->errtxt);
        assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (p_ctx->index == -1 && data[i].compare_previous != -1) {
            ret = testUtilSymbolCmp(symbol, &previous_symbol);
            assert_equal(!ret, !data[i].compare_previous, "i:%d testUtilSymbolCmp !ret %d != %d\n", i, ret, data[i].compare_previous);
        }
        memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int output_options;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, UNICODE_MODE, READER_INIT, "A", 0, 6, 103, "(12) 4 921 29 900 209 917 46 891 522 472 822 385", "Outputs Test Alpha flag 900" },
        /*  1*/ { BARCODE_MICROPDF417, UNICODE_MODE, READER_INIT, "A", 0, 11, 38, "(11) 921 900 29 900 179 499 922 262 777 478 300", "Outputs Test Alpha flag 900" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_reader_init", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[32768];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        struct zint_structapp structapp;
        char *data;
        int ret;
        int expected_eci;
        int expected_rows;
        int expected_width;
        char *expected;
        int bwipp_cmp;
        char *comment;
    };
    /* é U+00E9 (\351, 233), UTF-8 C3A9 */
    /* β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page) (\342, 226), UTF-8 CEB2 */
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "é", 0, 0, 6, 103, "(12) 4 913 233 900 398 878 279 350 217 295 231 77", 1, "" },
        /*  1*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "é", 0, 0, 6, 103, "(12) 4 913 233 900 398 878 279 350 217 295 231 77", 1, "" },
        /*  2*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 3, -1, -1, { 0, 0, "" }, "é", 0, 3, 7, 103, "(14) 6 927 3 913 233 900 162 81 551 529 607 384 164 108", 1, "" },
        /*  3*/ { BARCODE_PDF417, UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "é", 0, 3, 7, 103, "(14) 6 927 3 913 233 900 162 81 551 529 607 384 164 108", 1, "" },
        /*  4*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 26, -1, -1, { 0, 0, "" }, "é", 0, 26, 7, 103, "(14) 6 927 26 901 195 169 574 701 519 908 84 241 360 642", 1, "" },
        /*  5*/ { BARCODE_PDF417, UNICODE_MODE, 26, -1, -1, { 0, 0, "" }, "é", 0, 26, 7, 103, "(14) 6 927 26 901 195 169 574 701 519 908 84 241 360 642", 1, "" },
        /*  6*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 9, -1, -1, { 0, 0, "" }, "β", 0, 9, 7, 103, "(14) 6 927 9 913 226 900 487 92 418 278 838 500 576 84", 1, "" },
        /*  7*/ { BARCODE_PDF417, UNICODE_MODE, 9, -1, -1, { 0, 0, "" }, "β", 0, 9, 7, 103, "(14) 6 927 9 913 226 900 487 92 418 278 838 500 576 84", 1, "" },
        /*  8*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, 9, 7, 103, "Warning (14) 6 927 9 913 226 900 487 92 418 278 838 500 576 84", 1, "" },
        /*  9*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, 9, 7, 103, "Warning (14) 6 927 9 913 226 900 487 92 418 278 838 500 576 84", 1, "" },
        /* 10*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 3, -1, -1, { 0, 0, "" }, "β", ZINT_ERROR_INVALID_DATA, 3, 0, 0, "Error 244: Invalid character in input for ECI '3'", 1, "" },
        /* 11*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 899, -1, -1, { 0, 0, "" }, "A", 0, 899, 6, 103, "(12) 4 927 899 29 567 272 3 384 796 210 839 746", 0, "BWIPP BYTE1" },
        /* 12*/ { BARCODE_PDF417, UNICODE_MODE, 899, -1, -1, { 0, 0, "" }, "A", 0, 899, 6, 103, "(12) 4 927 899 29 567 272 3 384 796 210 839 746", 0, "BWIPP BYTE1" },
        /* 13*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 900, -1, -1, { 0, 0, "" }, "A", 0, 900, 7, 103, "(14) 6 926 0 0 29 900 205 526 248 527 915 616 104 416", 0, "BWIPP BYTE1" },
        /* 14*/ { BARCODE_PDF417, UNICODE_MODE, 900, -1, -1, { 0, 0, "" }, "A", 0, 900, 7, 103, "(14) 6 926 0 0 29 900 205 526 248 527 915 616 104 416", 0, "BWIPP BYTE1" },
        /* 15*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 810899, -1, -1, { 0, 0, "" }, "A", 0, 810899, 7, 103, "(14) 6 926 899 899 29 900 67 632 76 482 536 212 354 560", 0, "BWIPP BYTE1" },
        /* 16*/ { BARCODE_PDF417, UNICODE_MODE, 810899, -1, -1, { 0, 0, "" }, "A", 0, 810899, 7, 103, "(14) 6 926 899 899 29 900 67 632 76 482 536 212 354 560", 0, "BWIPP BYTE1" },
        /* 17*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 810900, -1, -1, { 0, 0, "" }, "A", 0, 810900, 6, 103, "(12) 4 925 0 29 682 61 599 519 256 742 797 153", 0, "BWIPP BYTE1" },
        /* 18*/ { BARCODE_PDF417, UNICODE_MODE, 810900, -1, -1, { 0, 0, "" }, "A", 0, 810900, 6, 103, "(12) 4 925 0 29 682 61 599 519 256 742 797 153", 0, "BWIPP BYTE1" },
        /* 19*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 811799, -1, -1, { 0, 0, "" }, "A", 0, 811799, 6, 103, "(12) 4 925 899 29 447 238 770 706 58 39 749 743", 0, "BWIPP BYTE1" },
        /* 20*/ { BARCODE_PDF417, UNICODE_MODE, 811799, -1, -1, { 0, 0, "" }, "A", 0, 811799, 6, 103, "(12) 4 925 899 29 447 238 770 706 58 39 749 743", 0, "BWIPP BYTE1" },
        /* 21*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, 811800, -1, -1, { 0, 0, "" }, "A", ZINT_ERROR_INVALID_OPTION, 811800, 0, 0, "Error 472: ECI code '811800' out of range (0 to 811799)", 1, "" },
        /* 22*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "é", 0, 0, 11, 38, "(11) 901 233 900 900 310 142 569 141 677 300 494", 1, "" },
        /* 23*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "é", 0, 0, 11, 38, "(11) 901 233 900 900 310 142 569 141 677 300 494", 1, "" },
        /* 24*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 3, -1, -1, { 0, 0, "" }, "é", 0, 3, 11, 38, "(11) 927 3 901 233 657 863 824 246 172 292 833", 1, "" },
        /* 25*/ { BARCODE_MICROPDF417, UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "é", 0, 3, 11, 38, "(11) 927 3 901 233 657 863 824 246 172 292 833", 1, "" },
        /* 26*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 26, -1, 3, { 0, 0, "" }, "é", 0, 26, 6, 82, "(18) 927 26 901 195 169 900 288 96 509 365 709 784 713 403 219 81 851 866", 1, "" },
        /* 27*/ { BARCODE_MICROPDF417, UNICODE_MODE, 26, -1, 3, { 0, 0, "" }, "é", 0, 26, 6, 82, "(18) 927 26 901 195 169 900 288 96 509 365 709 784 713 403 219 81 851 866", 1, "" },
        /* 28*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 9, -1, -1, { 0, 0, "" }, "β", 0, 9, 11, 38, "(11) 927 9 901 226 806 489 813 191 671 146 327", 1, "" },
        /* 29*/ { BARCODE_MICROPDF417, UNICODE_MODE, 9, -1, -1, { 0, 0, "" }, "β", 0, 9, 11, 38, "(11) 927 9 901 226 806 489 813 191 671 146 327", 1, "" },
        /* 30*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, 9, 11, 38, "Warning (11) 927 9 901 226 806 489 813 191 671 146 327", 1, "" },
        /* 31*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, 9, 11, 38, "Warning (11) 927 9 901 226 806 489 813 191 671 146 327", 1, "" },
        /* 32*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, 9, 11, 38, "Warning (11) 927 9 901 226 806 489 813 191 671 146 327", 1, "" },
        /* 33*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, 9, 11, 38, "Warning (11) 927 9 901 226 806 489 813 191 671 146 327", 1, "" },
        /* 34*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 3, -1, -1, { 0, 0, "" }, "β", ZINT_ERROR_INVALID_DATA, 3, 0, 0, "Error 244: Invalid character in input for ECI '3'", 1, "" },
        /* 35*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 899, -1, -1, { 0, 0, "" }, "A", 0, 899, 11, 38, "(11) 927 899 900 29 533 437 884 3 617 241 747", 0, "BWIPP BYTE" },
        /* 36*/ { BARCODE_MICROPDF417, UNICODE_MODE, 899, -1, -1, { 0, 0, "" }, "A", 0, 899, 11, 38, "(11) 927 899 900 29 533 437 884 3 617 241 747", 0, "BWIPP BYTE" },
        /* 37*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 900, -1, 3, { 0, 0, "" }, "A", 0, 900, 6, 82, "(18) 926 0 0 900 29 900 913 543 414 141 214 886 461 1 419 422 54 495", 0, "BWIPP BYTE" },
        /* 38*/ { BARCODE_MICROPDF417, UNICODE_MODE, 900, -1, 3, { 0, 0, "" }, "A", 0, 900, 6, 82, "(18) 926 0 0 900 29 900 913 543 414 141 214 886 461 1 419 422 54 495", 0, "BWIPP BYTE" },
        /* 39*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 810899, -1, 3, { 0, 0, "" }, "A", 0, 810899, 6, 82, "(18) 926 899 899 900 29 900 351 555 241 509 787 583 3 326 41 628 534 151", 0, "BWIPP BYTE" },
        /* 40*/ { BARCODE_MICROPDF417, UNICODE_MODE, 810899, -1, 3, { 0, 0, "" }, "A", 0, 810899, 6, 82, "(18) 926 899 899 900 29 900 351 555 241 509 787 583 3 326 41 628 534 151", 0, "BWIPP BYTE" },
        /* 41*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 810900, -1, 1, { 0, 0, "" }, "A", 0, 810900, 11, 38, "(11) 925 0 900 29 233 533 43 483 708 659 704", 0, "BWIPP BYTE" },
        /* 42*/ { BARCODE_MICROPDF417, UNICODE_MODE, 810900, -1, 1, { 0, 0, "" }, "A", 0, 810900, 11, 38, "(11) 925 0 900 29 233 533 43 483 708 659 704", 0, "BWIPP BYTE" },
        /* 43*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, 811800, -1, -1, { 0, 0, "" }, "A", ZINT_ERROR_INVALID_OPTION, 811800, 0, 0, "Error 472: ECI code '811800' out of range (0 to 811799)", 0, "BWIPP BYTE" },
        /* 44*/ { BARCODE_HIBC_PDF, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, ",", ZINT_ERROR_INVALID_DATA, 0, 0, 0, "Error 203: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)", 1, "" },
        /* 45*/ { BARCODE_HIBC_MICPDF, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, ",", ZINT_ERROR_INVALID_DATA, 0, 0, 0, "Error 203: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)", 1, "" },
        /* 46*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "AB{}  C#+  de{}  {}F  12{}  G{}  H", 0, 0, 12, 120, "(36) 28 1 865 807 896 782 855 626 807 94 865 807 896 808 776 839 176 808 32 776 839 806 208", 0, "BWIPP different encodation (3 codewords shorter)" },
        /* 47*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "AB{}  C#+  de{}  {}F  12{}  G{}  H", 0, 0, 11, 120, "(33) 25 1 896 897 806 88 470 806 813 149 809 836 809 809 837 178 806 32 776 839 806 209 809", 0, "BWIPP different encodation, same codeword count" },
        /* 48*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "{}  #+ de{}  12{}  {}  H", 0, 0, 10, 120, "(30) 22 865 807 896 808 470 807 94 865 807 896 808 32 776 839 806 865 807 896 787 900 900", 0, "BWIPP different encodation" },
        /* 49*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "{}  #+ de{}  12{}  {}  H", 0, 0, 9, 120, "(27) 19 896 897 866 795 626 813 149 809 838 806 32 776 839 806 896 897 806 239 325 656 122", 0, "BWIPP different encodation" },
        /* 50*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "A", 0, 0, 5, 103, "(10) 2 29 478 509 903 637 74 490 760 21", 0, "BWIPP BYTE1" },
        /* 51*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A", 0, 0, 5, 103, "(10) 2 29 478 509 903 637 74 490 760 21", 0, "BWIPP BYTE1" },
        /* 52*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 0, -1, { 0, 0, "" }, "A", 0, 0, 4, 86, "(4) 2 29 347 502", 0, "BWIPP BYTE1" },
        /* 53*/ { BARCODE_PDF417, UNICODE_MODE, -1, 0, -1, { 0, 0, "" }, "A", 0, 0, 4, 86, "(4) 2 29 347 502", 0, "BWIPP BYTE1" },
        /* 54*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 1, -1, { 0, 0, "" }, "A", 0, 0, 6, 86, "(6) 2 29 752 533 551 139", 0, "BWIPP BYTE1" },
        /* 55*/ { BARCODE_PDF417, UNICODE_MODE, -1, 1, -1, { 0, 0, "" }, "A", 0, 0, 6, 86, "(6) 2 29 752 533 551 139", 0, "BWIPP BYTE1" },
        /* 56*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 2, -1, { 0, 0, "" }, "A", 0, 0, 5, 103, "(10) 2 29 478 509 903 637 74 490 760 21", 0, "BWIPP BYTE1" },
        /* 57*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 3, -1, { 0, 0, "" }, "A", 0, 0, 9, 103, "(18) 2 29 290 888 64 789 390 182 22 197 347 41 298 467 387 917 455 196", 0, "BWIPP BYTE1" },
        /* 58*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 4, -1, { 0, 0, "" }, "A", 0, 0, 12, 120, "(36) 4 29 900 900 702 212 753 721 695 584 222 459 110 594 813 465 718 912 667 349 852 602", 0, "BWIPP BYTE1" },
        /* 59*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 5, -1, { 0, 0, "" }, "A", 0, 0, 14, 154, "(70) 6 29 900 900 900 900 774 599 527 418 850 374 921 763 922 772 572 661 584 902 578 696", 0, "BWIPP BYTE1" },
        /* 60*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 6, -1, { 0, 0, "" }, "A", 0, 0, 19, 188, "(133) 5 29 900 900 900 113 261 822 368 600 652 404 869 860 902 184 702 611 323 195 794 566", 0, "BWIPP BYTE1" },
        /* 61*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 7, -1, { 0, 0, "" }, "A", 0, 0, 29, 222, "(261) 5 29 900 900 900 384 614 456 20 422 177 78 492 215 859 765 864 755 572 621 891 97 538", 0, "BWIPP BYTE1" },
        /* 62*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 8, -1, { 0, 0, "" }, "A", 0, 0, 40, 290, "(520) 8 29 900 900 900 900 900 900 255 576 871 499 885 500 866 196 784 681 589 448 428 108", 0, "BWIPP BYTE1" },
        /* 63*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 8, -1, { 1, 4, "017053" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 928 111 100 17 53 923 1 111 104 903 71", 1, "H.4 example" },
        /* 64*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 1, 4, "017053" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 928 111 100 17 53 923 1 111 104 903 71", 1, "H.4 example" },
        /* 65*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 8, -1, { 4, 4, "017053" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 928 111 103 17 53 923 1 111 104 922 772 754", 1, "H.4 example last segment" },
        /* 66*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 4, 4, "017053" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 928 111 103 17 53 923 1 111 104 922 772 754", 1, "H.4 example last segment" },
        /* 67*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 8, -1, { 2, 4, "" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 900 900 928 111 101 923 1 111 104 583", 1, "No ID" },
        /* 68*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 2, 4, "" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 900 900 928 111 101 923 1 111 104 583", 1, "No ID" },
        /* 69*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, 8, -1, { 99998, 99999, "12345" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 928 222 197 123 45 923 1 222 199 198", 1, "IDs '123', '045'" },
        /* 70*/ { BARCODE_PDF417, UNICODE_MODE, -1, 8, -1, { 99998, 99999, "12345" }, "A", 0, 0, 41, 290, "(533) 21 29 900 900 900 900 900 900 900 900 900 900 928 222 197 123 45 923 1 222 199 198", 1, "IDs '123', '045'" },
        /* 71*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 1, 4, "017053" }, "A", 0, 0, 6, 99, "(24) 900 29 900 928 111 100 17 53 923 1 111 104 430 136 328 218 796 853 32 421 712 477 363", 1, "H.4 example" },
        /* 72*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 1, 4, "017053" }, "A", 0, 0, 6, 99, "(24) 900 29 900 928 111 100 17 53 923 1 111 104 430 136 328 218 796 853 32 421 712 477 363", 1, "H.4 example" },
        /* 73*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 4, 4, "017053" }, "A", 0, 0, 6, 99, "(24) 900 29 928 111 103 17 53 923 1 111 104 922 837 837 774 835 701 445 926 428 285 851 334", 1, "H.4 example last segment" },
        /* 74*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 3, 4, "" }, "A", 0, 0, 17, 38, "(17) 900 29 900 928 111 102 923 1 111 104 343 717 634 693 618 860 618", 1, "No ID" },
        /* 75*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 3, 4, "" }, "A", 0, 0, 17, 38, "(17) 900 29 900 928 111 102 923 1 111 104 343 717 634 693 618 860 618", 1, "No ID" },
        /* 76*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 99999, 99999, "100200300" }, "A", 0, 0, 11, 55, "(22) 900 29 928 222 198 100 200 300 923 1 222 199 922 693 699 895 719 637 154 478 399 638", 1, "IDs '100', '200', '300'" },
        /* 77*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 99999, 99999, "100200300" }, "A", 0, 0, 11, 55, "(22) 900 29 928 222 198 100 200 300 923 1 222 199 922 693 699 895 719 637 154 478 399 638", 1, "IDs '100', '200', '300'" },
        /* 78*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "123456", 0, 0, 7, 103, "(14) 6 902 1 348 256 900 759 577 359 263 64 409 852 154", 0, "BWIPP BYTE" },
        /* 79*/ { BARCODE_PDF417, DATA_MODE, -1, -1, -1, { 0, 0, "" }, "123456", 0, 0, 7, 103, "(14) 6 841 63 125 209 900 889 730 746 451 470 371 667 636", 1, "" },
        /* 80*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "12345678901234567890", 0, 0, 9, 103, "(18) 10 902 211 358 354 304 269 753 190 900 327 902 163 367 231 586 808 731", 1, "" },
        /* 81*/ { BARCODE_PDF417, DATA_MODE, -1, -1, -1, { 0, 0, "" }, "12345678901234567890", 0, 0, 9, 103, "(18) 10 902 211 358 354 304 269 753 190 900 327 902 163 367 231 586 808 731", 1, "" },
        /* 82*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, 0, 12, 137, "(48) 40 902 491 81 137 450 302 67 15 174 492 862 667 475 869 12 434 685 326 422 57 117 339", 1, "" },
        /* 83*/ { BARCODE_PDF417, DATA_MODE, -1, -1, -1, { 0, 0, "" }, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, 0, 12, 137, "(48) 40 902 491 81 137 450 302 67 15 174 492 862 667 475 869 12 434 685 326 422 57 117 339", 1, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;
    int last_fast_num_cwds = 0; /* Keep clang-tidy happy */

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

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, %d, { %d, %d, \"%s\" }, \"%s\", %s, %d, %d, %d, \"%s\", %d, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, data[i].option_2,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->eci, symbol->rows, symbol->width, symbol->errtxt, data[i].bwipp_cmp, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d symbol->eci %d != %d (%s)\n", i, symbol->eci, data[i].expected_eci, data[i].data);
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (ret == 0 && p_ctx->index == -1) {
                    if (i && (data[i - 1].input_mode & FAST_MODE) && !(data[i].input_mode & FAST_MODE)
                            && strcmp(data[i - 1].data, data[i].data) == 0) {
                        int num_cwds;
                        assert_equal(sscanf(symbol->errtxt, "(%d)", &num_cwds), 1, "i:%d num_cwds sscanf != 1 (%s)\n", i, symbol->errtxt);
                        assert_nonzero(last_fast_num_cwds >= num_cwds, "i:%d last_fast_num_cwds %d < num_cwds %d\n", i, last_fast_num_cwds, num_cwds);
                        if (num_cwds < last_fast_num_cwds && (debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) {
                            printf("i:%d diff %d\n", i, num_cwds - last_fast_num_cwds);
                        }
                    }
                    if (data[i].input_mode & FAST_MODE) {
                        assert_equal(sscanf(symbol->errtxt, "(%d)", &last_fast_num_cwds), 1, "i:%d last_fast sscanf != 1 (%s)\n", i, symbol->errtxt);
                    }
                }
                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        char modules_dump[32768];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[32768];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
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
        int symbology;
        int eci;
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 2, -1, "PDF417 Symbology Standard", 0, 10, 103, 0, "ISO 15438:2015 Figure 1, same, BWIPP uses different encodation, same codeword count",
                    "1111111101010100011101010011100000111010110011110001110111011001100011110101011110000111111101000101001"
                    "1111111101010100011111010100110000110100001110001001111010001010000011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101100110111100001110111111000101011010100111110000111111101000101001"
                    "1111111101010100010101111101111100100000100100000101000101000100000010101111001111000111111101000101001"
                    "1111111101010100011010111000100000111100100000101001001000011111011011010111000100000111111101000101001"
                    "1111111101010100011110101111010000100111100001010001100111110010010011110101111001000111111101000101001"
                    "1111111101010100010100111001110000101110001110100001111001101000111011010011101111000111111101000101001"
                    "1111111101010100011010111111011110111101011001100001010011111101110011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110000101001111101101111100010001010100110011111000111111101000101001"
                    "1111111101010100010100011000001100100010111101111001100011100011001011010001100011100111111101000101001"
                },
        /*  1*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 2, -1, "PDF417 Symbology Standard", 0, 10, 103, 0, "ISO 15438:2015 Figure 1, same, BWIPP uses different encodation, same codeword count",
                    "1111111101010100011101010011100000111010110011110001110111011001100011110101011110000111111101000101001"
                    "1111111101010100011111010100110000110100001110001001111010001010000011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101100110111100001110111111000101011010100111110000111111101000101001"
                    "1111111101010100010101111101111100100000100100000101000101000100000010101111001111000111111101000101001"
                    "1111111101010100011010111000100000111100100000101001001000011111011011010111000100000111111101000101001"
                    "1111111101010100011110101111010000100111100001010001100111110010010011110101111001000111111101000101001"
                    "1111111101010100010100111001110000101110001110100001111001101000111011010011101111000111111101000101001"
                    "1111111101010100011010111111011110111101011001100001010011111101110011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110000101001111101101111100010001010100110011111000111111101000101001"
                    "1111111101010100010100011000001100100010111101111001100011100011001011010001100011100111111101000101001"
                },
        /*  2*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 2, -1, "PDF417", 0, 5, 103, 1, "ISO 15438:2015 Annex Q example for generating ECC",
                    "1111111101010100011110101011110000110101000110000001110111011001100011110101011110000111111101000101001"
                    "1111111101010100011111101010011100110100001110001001111010001010000011111101010111000111111101000101001"
                    "1111111101010100011101010111111000101100110011110001100011111001001011101010011111100111111101000101001"
                    "1111111101010100010101111001111000101011101110000001100001101000100010101111001111000111111101000101001"
                    "1111111101010100011101011100011000100001101011111101111110110001011011101011100110000111111101000101001"
                },
        /*  3*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 2, -1, "PDF417", 0, 5, 103, 1, "ISO 15438:2015 Annex Q example for generating ECC",
                    "1111111101010100011110101011110000110101000110000001110111011001100011110101011110000111111101000101001"
                    "1111111101010100011111101010011100110100001110001001111010001010000011111101010111000111111101000101001"
                    "1111111101010100011101010111111000101100110011110001100011111001001011101010011111100111111101000101001"
                    "1111111101010100010101111001111000101011101110000001100001101000100010101111001111000111111101000101001"
                    "1111111101010100011101011100011000100001101011111101111110110001011011101011100110000111111101000101001"
                },
        /*  4*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 0, 1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ", 0, 17, 86, 1, "Text Compaction Alpha",
                    "11111111010101000111110101001111101101011001110000011101010111000000111111101000101001"
                    "11111111010101000111111010101110001111110101011100011110101000100000111111101000101001"
                    "11111111010101000110101011111000001010011001111100011101010111111000111111101000101001"
                    "11111111010101000111010010111000001101000001011000011010111101111100111111101000101001"
                    "11111111010101000111010111001100001010000111110011011110101110001110111111101000101001"
                    "11111111010101000111110101110000101011001110011111011110101111010000111111101000101001"
                    "11111111010101000101001110001110001100100001101110010100111011100000111111101000101001"
                    "11111111010101000101011111000011001101100111100100010101111110001110111111101000101001"
                    "11111111010101000101001101111100001001100001000111011010011011111100111111101000101001"
                    "11111111010101000110100011101111001001100111100111010100011000110000111111101000101001"
                    "11111111010101000111010011101100001001100101111110011110100111001110111111101000101001"
                    "11111111010101000110100010011111001000001010111100010100010001111000111111101000101001"
                    "11111111010101000110100000101100001000110011001000010100001100001100111111101000101001"
                    "11111111010101000111101000101000001110001100111010011111010001001100111111101000101001"
                    "11111111010101000111101000011110101100011110010011011101000011111010111111101000101001"
                    "11111111010101000110010110111000001100011000100001011100101000111000111111101000101001"
                    "11111111010101000101000111100100001110000101100010010100011110000100111111101000101001"
                },
        /*  5*/ { BARCODE_PDF417, -1, UNICODE_MODE, 0, 1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ", 0, 17, 86, 1, "Text Compaction Alpha",
                    "11111111010101000111110101001111101101011001110000011101010111000000111111101000101001"
                    "11111111010101000111111010101110001111110101011100011110101000100000111111101000101001"
                    "11111111010101000110101011111000001010011001111100011101010111111000111111101000101001"
                    "11111111010101000111010010111000001101000001011000011010111101111100111111101000101001"
                    "11111111010101000111010111001100001010000111110011011110101110001110111111101000101001"
                    "11111111010101000111110101110000101011001110011111011110101111010000111111101000101001"
                    "11111111010101000101001110001110001100100001101110010100111011100000111111101000101001"
                    "11111111010101000101011111000011001101100111100100010101111110001110111111101000101001"
                    "11111111010101000101001101111100001001100001000111011010011011111100111111101000101001"
                    "11111111010101000110100011101111001001100111100111010100011000110000111111101000101001"
                    "11111111010101000111010011101100001001100101111110011110100111001110111111101000101001"
                    "11111111010101000110100010011111001000001010111100010100010001111000111111101000101001"
                    "11111111010101000110100000101100001000110011001000010100001100001100111111101000101001"
                    "11111111010101000111101000101000001110001100111010011111010001001100111111101000101001"
                    "11111111010101000111101000011110101100011110010011011101000011111010111111101000101001"
                    "11111111010101000110010110111000001100011000100001011100101000111000111111101000101001"
                    "11111111010101000101000111100100001110000101100010010100011110000100111111101000101001"
                },
        /*  6*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 1, -1, "abcdefghijklmnopqrstuvwxyz ", 0, 19, 86, 1, "Text Compaction Lower",
                    "11111111010101000110101000110000001101011001110000011101010111000000111111101000101001"
                    "11111111010101000111110101001100001100000101110010011111010100011000111111101000101001"
                    "11111111010101000110101011111000001111101011110110011010100111110000111111101000101001"
                    "11111111010101000111101001011110001010001110111000011010111101111100111111101000101001"
                    "11111111010101000110101110001000001010001111000001011010111000010000111111101000101001"
                    "11111111010101000111110101110000101110010011111001011110101111001000111111101000101001"
                    "11111111010101000110100111000111101011001111000111010100111011100000111111101000101001"
                    "11111111010101000110101111110111101111101100100100011110100101000000111111101000101001"
                    "11111111010101000101001101111100001011100100000011010100110011111000111111101000101001"
                    "11111111010101000101000111001110001100010001001100010100011000110000111111101000101001"
                    "11111111010101000110100111001000001110001001110011011010011100010000111111101000101001"
                    "11111111010101000110100010011111001101111000100110010100010000111100111111101000101001"
                    "11111111010101000111010000010111001000110111101110010100001100001100111111101000101001"
                    "11111111010101000111111010001011101100011011110010011110100010001000111111101000101001"
                    "11111111010101000111101000011110101000001100101110011010000010111110111111101000101001"
                    "11111111010101000111001011011110001000010010000001011100101000111000111111101000101001"
                    "11111111010101000101000111100010001111010000101000010100011110000010111111101000101001"
                    "11111111010101000111111001011101101010000001001111010010111001111110111111101000101001"
                    "11111111010101000111011010000110001000100111001110011110110100111000111111101000101001"
                },
        /*  7*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 1, -1, "abcdefghijklmnopqrstuvwxyz ", 0, 19, 86, 1, "Text Compaction Lower",
                    "11111111010101000110101000110000001101011001110000011101010111000000111111101000101001"
                    "11111111010101000111110101001100001100000101110010011111010100011000111111101000101001"
                    "11111111010101000110101011111000001111101011110110011010100111110000111111101000101001"
                    "11111111010101000111101001011110001010001110111000011010111101111100111111101000101001"
                    "11111111010101000110101110001000001010001111000001011010111000010000111111101000101001"
                    "11111111010101000111110101110000101110010011111001011110101111001000111111101000101001"
                    "11111111010101000110100111000111101011001111000111010100111011100000111111101000101001"
                    "11111111010101000110101111110111101111101100100100011110100101000000111111101000101001"
                    "11111111010101000101001101111100001011100100000011010100110011111000111111101000101001"
                    "11111111010101000101000111001110001100010001001100010100011000110000111111101000101001"
                    "11111111010101000110100111001000001110001001110011011010011100010000111111101000101001"
                    "11111111010101000110100010011111001101111000100110010100010000111100111111101000101001"
                    "11111111010101000111010000010111001000110111101110010100001100001100111111101000101001"
                    "11111111010101000111111010001011101100011011110010011110100010001000111111101000101001"
                    "11111111010101000111101000011110101000001100101110011010000010111110111111101000101001"
                    "11111111010101000111001011011110001000010010000001011100101000111000111111101000101001"
                    "11111111010101000101000111100010001111010000101000010100011110000010111111101000101001"
                    "11111111010101000111111001011101101010000001001111010010111001111110111111101000101001"
                    "11111111010101000111011010000110001000100111001110011110110100111000111111101000101001"
                },
        /*  8*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 2, 2, -1, "abcdefgABCDEFG", 0, 9, 103, 1, "Text Compaction Lower Alpha",
                    "1111111101010100011111010101111100110101000001100001000001010000010011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111101011100111001110100111001100011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111110010111101101000001110100110010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111101010111100001010011100111000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111110100010011001101000000111001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100111111001110101011111000011010011110101111101100111111101000101001"
                    "1111111101010100011101001110111110110010000101100001001110111101100011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111111011010110001011100111111010010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100011101100011101010111011111100011111010011101000111111101000101001"
                },
        /*  9*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 2, -1, "abcdefgABCDEFG", 0, 9, 103, 1, "Text Compaction Lower Alpha",
                    "1111111101010100011111010101111100110101000001100001000001010000010011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111101011100111001110100111001100011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111110010111101101000001110100110010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111101010111100001010011100111000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111110100010011001101000000111001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100111111001110101011111000011010011110101111101100111111101000101001"
                    "1111111101010100011101001110111110110010000101100001001110111101100011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111111011010110001011100111111010010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100011101100011101010111011111100011111010011101000111111101000101001"
                },
        /* 10*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 4, -1, "0123456&\015\011,:#-.$/+%*=^ 789", 0, 5, 137, 1, "Text Compaction Mixed",
                    "11111111010101000111101010111100001110101100111100010000110111001100110101111001111101010001110111000011101010011100000111111101000101001"
                    "11111111010101000111111010100111001010001111000001011101101111001100110110011110010001110010000011010011111101010111000111111101000101001"
                    "11111111010101000110101001111100001100111010000111011011110010110000100000101011110001101111101010000011101010011111100111111101000101001"
                    "11111111010101000101011110011110001000010000100001010010011000011000110010000100110001000011000110010010101111101111100111111101000101001"
                    "11111111010101000111010111000110001001111001001111010000101111101100100011110010111101001111110110111011101011100110000111111101000101001"
                },
        /* 11*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "0123456&\015\011,:#-.$/+%*=^ 789", 0, 5, 137, 1, "Text Compaction Mixed",
                    "11111111010101000111101010111100001110101100111100010000110111001100110101111001111101010001110111000011101010011100000111111101000101001"
                    "11111111010101000111111010100111001010001111000001011101101111001100110110011110010001110010000011010011111101010111000111111101000101001"
                    "11111111010101000110101001111100001100111010000111011011110010110000100000101011110001101111101010000011101010011111100111111101000101001"
                    "11111111010101000101011110011110001000010000100001010010011000011000110010000100110001000011000110010010101111101111100111111101000101001"
                    "11111111010101000111010111000110001001111001001111010000101111101100100011110010111101001111110110111011101011100110000111111101000101001"
                },
        /* 12*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 3, 2, -1, ";<>@[\\]_'~!\015\011,:\012-.$/\"|*()?{", 0, 16, 103, 1, "Text Compaction Punctuation",
                    "1111111101010100011111010100111110111010110011110001000111011100100011110101011110000111111101000101001"
                    "1111111101010100011111010100001100111111010101110001101011111101111011110101000100000111111101000101001"
                    "1111111101010100011101010111111000101000001000111101011011001111000011010100001111100111111101000101001"
                    "1111111101010100011101001011100000110000110010100001100100001101110010101111001111000111111101000101001"
                    "1111111101010100011101011100000110110110011110010001110010000011010011110101110001110111111101000101001"
                    "1111111101010100011110101111010000110011101000011101101111001011000011101011111001000111111101000101001"
                    "1111111101010100010100111000111000110001101000100001000110011001000011010011101111000111111101000101001"
                    "1111111101010100011110100100100000111000110011101001110000010111011010101111110001110111111101000101001"
                    "1111111101010100011010011011111100110011111010100001001111000010010011111101001110110111111101000101001"
                    "1111111101010100011010001110111100110101000110000001100011010010000011010001100011100111111101000101001"
                    "1111111101010100011101001110000110110100111110011101001000011110100011110100111001110111111101000101001"
                    "1111111101010100010100010001111000110010011011111101000101000001111010100011001111100111111101000101001"
                    "1111111101010100011010000010110000110001011001110001100100010011000010100001100000110111111101000101001"
                    "1111111101010100011110100010000010110000010001110101111010011000110011111010001001100111111101000101001"
                    "1111111101010100011101000011111010111111010001101001011000010011100010010101111000000111111101000101001"
                    "1111111101010100011001011011100000110011001100001101100100101100000011110010100011110111111101000101001"
                },
        /* 13*/ { BARCODE_PDF417, -1, UNICODE_MODE, 3, 2, -1, ";<>@[\\]_'~!\015\011,:\012-.$/\"|*()?{", 0, 16, 103, 1, "Text Compaction Punctuation",
                    "1111111101010100011111010100111110111010110011110001000111011100100011110101011110000111111101000101001"
                    "1111111101010100011111010100001100111111010101110001101011111101111011110101000100000111111101000101001"
                    "1111111101010100011101010111111000101000001000111101011011001111000011010100001111100111111101000101001"
                    "1111111101010100011101001011100000110000110010100001100100001101110010101111001111000111111101000101001"
                    "1111111101010100011101011100000110110110011110010001110010000011010011110101110001110111111101000101001"
                    "1111111101010100011110101111010000110011101000011101101111001011000011101011111001000111111101000101001"
                    "1111111101010100010100111000111000110001101000100001000110011001000011010011101111000111111101000101001"
                    "1111111101010100011110100100100000111000110011101001110000010111011010101111110001110111111101000101001"
                    "1111111101010100011010011011111100110011111010100001001111000010010011111101001110110111111101000101001"
                    "1111111101010100011010001110111100110101000110000001100011010010000011010001100011100111111101000101001"
                    "1111111101010100011101001110000110110100111110011101001000011110100011110100111001110111111101000101001"
                    "1111111101010100010100010001111000110010011011111101000101000001111010100011001111100111111101000101001"
                    "1111111101010100011010000010110000110001011001110001100100010011000010100001100000110111111101000101001"
                    "1111111101010100011110100010000010110000010001110101111010011000110011111010001001100111111101000101001"
                    "1111111101010100011101000011111010111111010001101001011000010011100010010101111000000111111101000101001"
                    "1111111101010100011001011011100000110011001100001101100100101100000011110010100011110111111101000101001"
                },
        /* 14*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 4, 2, -1, "\015\015\015\015\010\015", 0, 20, 103, 1, "Text Compaction Punctuation 1 Mixed -> Byte",
                    "1111111101010100011010100011000000110101000011000001100011100011010011110101011110000111111101000101001"
                    "1111111101010100011110101101100000111110101100011101110111101111101011111010100011000111111101000101001"
                    "1111111101010100011101010111111000110100111111001001001111000101000011010110111111000111111101000101001"
                    "1111111101010100011110100101111000111000101100111101000011000110010010101111001111000111111101000101001"
                    "1111111101010100011110101111011110111001000110000101111101010000110011010111000010000111111101000101001"
                    "1111111101010100011110101111010000100001111001101101000111100101000011010111111010000111111101000101001"
                    "1111111101010100011010011100011110110001001001100001110100001001110011010011101111000111111101000101001"
                    "1111111101010100011111010010001100111000101100001001111001010010000011110100101000000111111101000101001"
                    "1111111101010100011010011011111100101000110111110001001110000001011011111010011100010111111101000101001"
                    "1111111101010100010100011100111000110101000011000001100011010100000011010001100011100111111101000101001"
                    "1111111101010100011010011110110000111101000111011101011100010011111011010011100010000111111101000101001"
                    "1111111101010100010100010001111000111110110011110101011110000010001011111010001110010111111101000101001"
                    "1111111101010100011101000001011100100101100000110001100010000110111010100001100000110111111101000101001"
                    "1111111101010100011101000110010000111100110011000101111001000110011011110100010001000111111101000101001"
                    "1111111101010100011101000011111010101110111000111101100010101111100011001010011111000111111101000101001"
                    "1111111101010100011100101101111000111110101011111001110001100010011011110010100011110111111101000101001"
                    "1111111101010100011111010000101100101111100100111001100001011101000010100011110000010111111101000101001"
                    "1111111101010100010010111011111100100011000010011101001001000011110011001011111101000111111101000101001"
                    "1111111101010100011101101000011000111010111011111001011001111000111011111011010011110111111101000101001"
                    "1111111101010100011110100000110110100111111011011101101110001111101010100001111101100111111101000101001"
                },
        /* 15*/ { BARCODE_PDF417, -1, UNICODE_MODE, 4, 2, -1, "\015\015\015\015\010\015", 0, 20, 103, 0, "Text Compaction Punctuation 1 Mixed; BWIPP same as FAST_MODE",
                    "1111111101010100011010100011000000110101000011000001110001110110110011110101011110000111111101000101001"
                    "1111111101010100011110101101100000101100101111110001111001000110110011111010100011000111111101000101001"
                    "1111111101010100011101010111111000100111111001110101010100001111000011010110111111000111111101000101001"
                    "1111111101010100011110100101111000110001010011000001000011000110010010101111001111000111111101000101001"
                    "1111111101010100011110101111011110111110011010001001011100010111110011010111000010000111111101000101001"
                    "1111111101010100011110101111010000111110110011110101001001011110000011010111111010000111111101000101001"
                    "1111111101010100011010011100011110110010100001100001110001100100110011010011101111000111111101000101001"
                    "1111111101010100011111010010001100111001001110001101111000100110110011110100101000000111111101000101001"
                    "1111111101010100011010011011111100101110000011011101001111011101110011111010011100010111111101000101001"
                    "1111111101010100010100011100111000110001000010011001100111101110100011010001100011100111111101000101001"
                    "1111111101010100011010011110110000110110000111101001110001100111010011010011100010000111111101000101001"
                    "1111111101010100010100010001111000111101001111101101100111100100110011111010001110010111111101000101001"
                    "1111111101010100011101000001011100100010111100111101110001100001011010100001100000110111111101000101001"
                    "1111111101010100011101000110010000101000111110001101010000111110011011110100010001000111111101000101001"
                    "1111111101010100011101000011111010111000100111110101100011100100111011001010011111000111111101000101001"
                    "1111111101010100011100101101111000110111001100001001000010000101000011110010100011110111111101000101001"
                    "1111111101010100011111010000101100111100100110110001100100111100110010100011110000010111111101000101001"
                    "1111111101010100010010111011111100100010100111100001101110000101110011001011111101000111111101000101001"
                    "1111111101010100011101101000011000110100000100001101001110000111010011111011010011110111111101000101001"
                    "1111111101010100011110100000110110111000000100110101111100100001011010100001111101100111111101000101001"
                },
        /* 16*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 4, 3, -1, "??????ABCDEFG??????abcdef??????%%%%%%", 0, 19, 120, 1, "Text Compaction Punctuation Alpha Punctuation Lower Punctuation Mixed",
                    "111111110101010001101010001100000011010111001111000100011101110010001100111000110010011111010101111100111111101000101001"
                    "111111110101010001111010100000010011111001110011010111110011100110101101111100101111011111010100011000111111101000101001"
                    "111111110101010001010100111100000011111010111101100101000100000111101111100101111011010101101111100000111111101000101001"
                    "111111110101010001111010010111100010001110111001000110011100011001001100111000110010011010111100111110111111101000101001"
                    "111111110101010001110101111011100011111001110011010100111110100011101111110101011100011010111000010000111111101000101001"
                    "111111110101010001111101011110110010100110011111000101000001000111101000000110100111011101011111000010111111101000101001"
                    "111111110101010001101001110001111011001110001100100110011100011001001100111000110010011101001110111110111111101000101001"
                    "111111110101010001111010010001000010001111001011110111100010000010101111000100000101011110100101000000111111101000101001"
                    "111111110101010001111110100110010010001110110001110100001001011110001001111001101100010100111001111110111111101000101001"
                    "111111110101010001010001110011100011000110110000110110001011100111101100111001100001010100011000011000111111101000101001"
                    "111111110101010001010011110100000011011110111111010100000101111000101110110111001000011010011100010000111111101000101001"
                    "111111110101010001101000100011111011000101101111110110111110010001001011010011100000010100011101111110111111101000101001"
                    "111111110101010001110100000101110010010001000000100100000100110110001100110001010000011010000111011110111111101000101001"
                    "111111110101010001111101000110111010011110001011110111110101000011001111001001000010011110100010001000111111101000101001"
                    "111111110101010001010000010111100010111100010001000111001001111101001110011111101010010010100111100000111111101000101001"
                    "111111110101010001110010110111100010100011000110000110001110001100101000010010010000010010100001000000111111101000101001"
                    "111111110101010001111010000101000010000010111101000111010001101000001111000001001001010100011110000010111111101000101001"
                    "111111110101010001111100101110010010011100000100110100111010000110001100110100001111011100101111100010111111101000101001"
                    "111111110101010001110110100001100010011001111001110101111011110001001011011100011000011011010001000000111111101000101001"
                },
        /* 17*/ { BARCODE_PDF417, -1, UNICODE_MODE, 4, 3, -1, "??????ABCDEFG??????abcdef??????%%%%%%", 0, 19, 120, 1, "Text Compaction Punctuation Alpha Punctuation Lower Punctuation Mixed",
                    "111111110101010001101010001100000011010111001111000100011101110010001100111000110010011111010101111100111111101000101001"
                    "111111110101010001111010100000010011111001110011010111110011100110101101111100101111011111010100011000111111101000101001"
                    "111111110101010001010100111100000011111010111101100101000100000111101111100101111011010101101111100000111111101000101001"
                    "111111110101010001111010010111100010001110111001000110011100011001001100111000110010011010111100111110111111101000101001"
                    "111111110101010001110101111011100011111001110011010100111110100011101111110101011100011010111000010000111111101000101001"
                    "111111110101010001111101011110110010100110011111000101000001000111101000000110100111011101011111000010111111101000101001"
                    "111111110101010001101001110001111011001110001100100110011100011001001100111000110010011101001110111110111111101000101001"
                    "111111110101010001111010010001000010001111001011110111100010000010101111000100000101011110100101000000111111101000101001"
                    "111111110101010001111110100110010010001110110001110100001001011110001001111001101100010100111001111110111111101000101001"
                    "111111110101010001010001110011100011000110110000110110001011100111101100111001100001010100011000011000111111101000101001"
                    "111111110101010001010011110100000011011110111111010100000101111000101110110111001000011010011100010000111111101000101001"
                    "111111110101010001101000100011111011000101101111110110111110010001001011010011100000010100011101111110111111101000101001"
                    "111111110101010001110100000101110010010001000000100100000100110110001100110001010000011010000111011110111111101000101001"
                    "111111110101010001111101000110111010011110001011110111110101000011001111001001000010011110100010001000111111101000101001"
                    "111111110101010001010000010111100010111100010001000111001001111101001110011111101010010010100111100000111111101000101001"
                    "111111110101010001110010110111100010100011000110000110001110001100101000010010010000010010100001000000111111101000101001"
                    "111111110101010001111010000101000010000010111101000111010001101000001111000001001001010100011110000010111111101000101001"
                    "111111110101010001111100101110010010011100000100110100111010000110001100110100001111011100101111100010111111101000101001"
                    "111111110101010001110110100001100010011001111001110101111011110001001011011100011000011011010001000000111111101000101001"
                },
        /* 18*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, ";;;;;é;;;;;", 0, 10, 103, 1, "",
                    "1111111101010100011101010011100000110101101110000001000111011100100011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111110101011000001111101010110000011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101011100011111101001111110011101011010100011111000111111101000101001"
                    "1111111101010100010101111101111100111101100101110001000111011100100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111110101011000001111101010110000011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101011100011111101100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000111011110011010001111010010111100011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111111011010011001111000001010001011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110111100110011101001101000001110010100110001111100111111101000101001"
                    "1111111101010100010100011000001100100010001110111001001010000010000011010001100011100111111101000101001"
                },
        /* 19*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, ";;;;;é;;;;;", 0, 10, 103, 1, "",
                    "1111111101010100011101010011100000110101101110000001000111011100100011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111110101011000001111101010110000011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101011100011111101001111110011101011010100011111000111111101000101001"
                    "1111111101010100010101111101111100111101100101110001000111011100100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111110101011000001111101010110000011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101011100011111101100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000111011110011010001111010010111100011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111111011010011001111000001010001011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110111100110011101001101000001110010100110001111100111111101000101001"
                    "1111111101010100010100011000001100100010001110111001001010000010000011010001100011100111111101000101001"
                },
        /* 20*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 3, -1, "12345678", 0, 3, 120, 1, "Numeric Compaction (minimum)",
                    "111111110101010001110101011100000011111010100111110100111101111010001010000001010000011111010101111100111111101000101001"
                    "111111110101010001111010100010000011100010000110100111000101110001101011111000100111011111010101100000111111101000101001"
                    "111111110101010001010100111100000010011001100011110101011000011111001000011000101110010101000111100000111111101000101001"
                },
        /* 21*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 3, -1, "12345678", 0, 3, 120, 1, "Numeric Compaction (minimum)",
                    "111111110101010001110101011100000011111010100111110100111101111010001010000001010000011111010101111100111111101000101001"
                    "111111110101010001111010100010000011100010000110100111000101110001101011111000100111011111010101100000111111101000101001"
                    "111111110101010001010100111100000010011001100011110101011000011111001000011000101110010101000111100000111111101000101001"
                },
        /* 22*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 2, 3, -1, "12345678901234", 0, 5, 120, 1, "Numeric Compaction",
                    "111111110101010001111010101111000011101010001110000100111101111010001001011100001110011111010101111100111111101000101001"
                    "111111110101010001111110101000111011010000001110010111111011010011001111010100000010011111101010111000111111101000101001"
                    "111111110101010001010100111100000010111000110011100101110011000011101110001111110101011101010001111110111111101000101001"
                    "111111110101010001010111100111100010001100001100010100001100011101101110101100111100011010111100111110111111101000101001"
                    "111111110101010001110101110000110011000000101110010110001001110000101011001000111111011101011100110000111111101000101001"
                },
        /* 23*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "12345678901234", 0, 5, 120, 1, "Numeric Compaction",
                    "111111110101010001111010101111000011101010001110000100111101111010001001011100001110011111010101111100111111101000101001"
                    "111111110101010001111110101000111011010000001110010111111011010011001111010100000010011111101010111000111111101000101001"
                    "111111110101010001010100111100000010111000110011100101110011000011101110001111110101011101010001111110111111101000101001"
                    "111111110101010001010111100111100010001100001100010100001100011101101110101100111100011010111100111110111111101000101001"
                    "111111110101010001110101110000110011000000101110010110001001110000101011001000111111011101011100110000111111101000101001"
                },
        /* 24*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 2, 3, -1, "1234567890123456789012345678901234567890123", 0, 9, 120, 1, "Numeric Compaction 43 consecutive",
                    "111111110101010001111101010111110011010110001110000100111101111010001101001101110000011111010101111100111111101000101001"
                    "111111110101010001111010100001000011010011100001000110100111101100001110000101100001011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111010111000010110010010011111001000011010000111010101000011110000111111101000101001"
                    "111111110101010001101011110011111011100010001001110110011011000110001001000110011000011010111100111110111111101000101001"
                    "111111110101010001101011100000100010010111101000000111000010111011001001101111101000011110101110011100111111101000101001"
                    "111111110101010001111101011110110010001111001000100110110010111100001100011111001001011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100110010001110111101001001110111000011101001110111110111111101000101001"
                    "111111110101010001111110100101110010100111100001000110000101111001101110010110000100010101111110111000111111101000101001"
                    "111111110101010001111110100110010011100100111110100100111110011000101001111000010001011111010011101000111111101000101001"
                },
        /* 25*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "1234567890123456789012345678901234567890123", 0, 9, 120, 1, "Numeric Compaction 43 consecutive",
                    "111111110101010001111101010111110011010110001110000100111101111010001101001101110000011111010101111100111111101000101001"
                    "111111110101010001111010100001000011010011100001000110100111101100001110000101100001011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111010111000010110010010011111001000011010000111010101000011110000111111101000101001"
                    "111111110101010001101011110011111011100010001001110110011011000110001001000110011000011010111100111110111111101000101001"
                    "111111110101010001101011100000100010010111101000000111000010111011001001101111101000011110101110011100111111101000101001"
                    "111111110101010001111101011110110010001111001000100110110010111100001100011111001001011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100110010001110111101001001110111000011101001110111110111111101000101001"
                    "111111110101010001111110100101110010100111100001000110000101111001101110010110000100010101111110111000111111101000101001"
                    "111111110101010001111110100110010011100100111110100100111110011000101001111000010001011111010011101000111111101000101001"
                },
        /* 26*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 2, 3, -1, "12345678901234567890123456789012345678901234", 0, 9, 120, 1, "Numeric Compaction 44 consecutive",
                    "111111110101010001111101010111110011010110001110000100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111010100001000011101001100100000111010001100001001110010000001101011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000010101000011110000111111101000101001"
                    "111111110101010001101011110011111010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001101011100000100011111000010100110110001011100100001011111001011100011110101110011100111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001100011111001001011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100100001011011000001000100000100100011101001110111110111111101000101001"
                    "111111110101010001111110100101110011100110011101000100110001011111101101001110000001010101111110111000111111101000101001"
                    "111111110101010001111110100110010010111001100011100101000110111110001001100001000111011111010011101000111111101000101001"
                },
        /* 27*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "12345678901234567890123456789012345678901234", 0, 9, 120, 1, "Numeric Compaction 44 consecutive",
                    "111111110101010001111101010111110011010110001110000100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111010100001000011101001100100000111010001100001001110010000001101011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000010101000011110000111111101000101001"
                    "111111110101010001101011110011111010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001101011100000100011111000010100110110001011100100001011111001011100011110101110011100111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001100011111001001011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100100001011011000001000100000100100011101001110111110111111101000101001"
                    "111111110101010001111110100101110011100110011101000100110001011111101101001110000001010101111110111000111111101000101001"
                    "111111110101010001111110100110010010111001100011100101000110111110001001100001000111011111010011101000111111101000101001"
                },
        /* 28*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 2, 3, -1, "123456789012345678901234567890123456789012345", 0, 9, 120, 1, "Numeric Compaction 45 consecutive",
                    "111111110101010001111101010111110011010110001110000100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111010100001000011101001100100000111010001100001001110010000001101011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000010101000011110000111111101000101001"
                    "111111110101010001101011110011111010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001101011100000100011111000010100110110001011100100001011111001011100011110101110011100111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001010110011111000011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100100001001000010001000010001001000011101001110111110111111101000101001"
                    "111111110101010001111110100101110011110001001101100111101001110011101101111010111110010101111110111000111111101000101001"
                    "111111110101010001111110100110010011011110011001110110011100100011101100100100011111011111010011101000111111101000101001"
                },
        /* 29*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 2, 3, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567", 0, 14, 120, 1, "Numeric Compaction 87 consecutive",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111101001110100011101011111010000111111101000101001"
                    "111111110101010001101001110011110011000100011011100111101110000101101101101000010000011101001110111110111111101000101001"
                    "111111110101010001111101001011000010011000111110100101110000101111101100000100011101010101111110011100111111101000101001"
                    "111111110101010001111110100110010010000110011011110110011111010001001000011110110110010100110000111110111111101000101001"
                    "111111110101010001010001110111000011011110111000010100110001100000101111101011111101010100011000011000111111101000101001"
                    "111111110101010001110100111000110010111101101111100111110000101001101011111101011000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010100000100010111101011111001100100011010001101111110111111101000101001"
                    "111111110101010001010000010100000011110111001001100110100000100110001110111100011010011010000111011110111111101000101001"
                    "111111110101010001111101000100011011100101110011000111100011001101001000001011110001011110100010010000111111101000101001"
                },
        /* 30*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567", 0, 14, 120, 1, "Numeric Compaction 87 consecutive",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111101001110100011101011111010000111111101000101001"
                    "111111110101010001101001110011110011000100011011100111101110000101101101101000010000011101001110111110111111101000101001"
                    "111111110101010001111101001011000010011000111110100101110000101111101100000100011101010101111110011100111111101000101001"
                    "111111110101010001111110100110010010000110011011110110011111010001001000011110110110010100110000111110111111101000101001"
                    "111111110101010001010001110111000011011110111000010100110001100000101111101011111101010100011000011000111111101000101001"
                    "111111110101010001110100111000110010111101101111100111110000101001101011111101011000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010100000100010111101011111001100100011010001101111110111111101000101001"
                    "111111110101010001010000010100000011110111001001100110100000100110001110111100011010011010000111011110111111101000101001"
                    "111111110101010001111101000100011011100101110011000111100011001101001000001011110001011110100010010000111111101000101001"
                },
        /* 31*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 2, 3, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", 0, 14, 120, 1, "Numeric Compaction 88 consecutive",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111011111010111011101011111010000111111101000101001"
                    "111111110101010001101001110011110011101100001001100110001001100011101110100110001111011101001110111110111111101000101001"
                    "111111110101010001111101001011000010100111110001100111110110010100001101100111100001010101111110011100111111101000101001"
                    "111111110101010001111110100110010011001000100111110101111110111001001011111011001000010100110000111110111111101000101001"
                    "111111110101010001010001110111000011111001010111110100000100011011001110110000010110010100011000011000111111101000101001"
                    "111111110101010001110100111000110011100010011001000100100011110100001011111101011000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010100011110100000101001110011011100011010001101111110111111101000101001"
                    "111111110101010001010000010100000011100010110011110111011001100111001110011010000110011010000111011110111111101000101001"
                    "111111110101010001111101000100011011110010110000110111011100111100101111010000110011011110100010010000111111101000101001"
                },
        /* 32*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", 0, 14, 120, 1, "Numeric Compaction 88 consecutive",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111011111010111011101011111010000111111101000101001"
                    "111111110101010001101001110011110011101100001001100110001001100011101110100110001111011101001110111110111111101000101001"
                    "111111110101010001111101001011000010100111110001100111110110010100001101100111100001010101111110011100111111101000101001"
                    "111111110101010001111110100110010011001000100111110101111110111001001011111011001000010100110000111110111111101000101001"
                    "111111110101010001010001110111000011111001010111110100000100011011001110110000010110010100011000011000111111101000101001"
                    "111111110101010001110100111000110011100010011001000100100011110100001011111101011000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010100011110100000101001110011011100011010001101111110111111101000101001"
                    "111111110101010001010000010100000011100010110011110111011001100111001110011010000110011010000111011110111111101000101001"
                    "111111110101010001111101000100011011110010110000110111011100111100101111010000110011011110100010010000111111101000101001"
                },
        /* 33*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 2, 3, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 0, 14, 120, 1, "Numeric Compaction 89 consecutive",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111011111010111011101011111010000111111101000101001"
                    "111111110101010001101001110011110011101100001001100110001001100011101110100110001111011101001110111110111111101000101001"
                    "111111110101010001111101001011000010100111110001100111110110010100001101100111100001010101111110011100111111101000101001"
                    "111111110101010001111110100110010011001000100111110101111110111001001011111011001000010100110000111110111111101000101001"
                    "111111110101010001010001110111000011111001010111110100000100011011001110110000010110010100011000011000111111101000101001"
                    "111111110101010001110100111000110011100010011001000100100011110100001110101100010000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010110010001011111001111010011110001011010001101111110111111101000101001"
                    "111111110101010001010000010100000011100110011001110100111011110110001100010000100110011010000111011110111111101000101001"
                    "111111110101010001111101000100011011100101110001100110001001110100001010001111000001011110100010010000111111101000101001"
                },
        /* 34*/ { BARCODE_PDF417, -1, UNICODE_MODE, 2, 3, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 0, 14, 120, 1, "Numeric Compaction 89 consecutive -> ML(1) NUM(88)",
                    "111111110101010001111010100111100011111010111111010100111101111010001000100011000011011111010101111100111111101000101001"
                    "111111110101010001111110101000111011101001100100000111010001100001001110010000001101011111101010011100111111101000101001"
                    "111111110101010001010100111100000011111100010110100101001100001111101010110011111000011101010001111110111111101000101001"
                    "111111110101010001111101011111101010010111100111100100110011100001101000111011101000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011111000010100110110001011100100001011111001011100011101011100011000111111101000101001"
                    "111111110101010001111101011110110010101101111100000101110001100111001111011111010111011101011111010000111111101000101001"
                    "111111110101010001101001110011110011101100001001100110001001100011101110100110001111011101001110111110111111101000101001"
                    "111111110101010001111101001011000010100111110001100111110110010100001101100111100001010101111110011100111111101000101001"
                    "111111110101010001111110100110010011001000100111110101111110111001001011111011001000010100110000111110111111101000101001"
                    "111111110101010001010001110111000011111001010111110100000100011011001110110000010110010100011000011000111111101000101001"
                    "111111110101010001110100111000110011100010011001000100100011110100001110101100010000011101001110011000111111101000101001"
                    "111111110101010001101000100011111011000111110010010110010001011111001111010011110001011010001101111110111111101000101001"
                    "111111110101010001010000010100000011100110011001110100111011110110001100010000100110011010000111011110111111101000101001"
                    "111111110101010001111101000100011011100101110001100110001001110100001010001111000001011110100010010000111111101000101001"
                },
        /* 35*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 0, 3, -1, "AB{}  C#+  de{}  {}F  12{}  G{}  H", 0, 10, 120, 0, "Text Compaction newtable, BWIPP uses PUNCT_SHIFT better for less codewords",
                    "111111110101010001110101001110000011010111000111100111101010111100001000111011100100011111010101111100111111101000101001"
                    "111111110101010001111101010110000011100000101100010100111110100111001110001100011101011111010100110000111111101000101001"
                    "111111110101010001010100111100000010111111001110100100001101011100001001111101101000011010101111100000111111101000101001"
                    "111111110101010001010111110111110010100011101110000100011101110010001100000101001100011010111100111110111111101000101001"
                    "111111110101010001101011100100000010011111010011100110000010111010001111101111011101011010111000100000111111101000101001"
                    "111111110101010001111101011110110010111111011100100101101000011100001100111110110110011111010111000010111111101000101001"
                    "111111110101010001010011100111000011010111100111110110001100001000101100001101110111011101001110111110111111101000101001"
                    "111111110101010001101011111000111011100000101100100110100000011101001111101111011101011010111111011110111111101000101001"
                    "111111110101010001111110100110010010111111011100100110001111001011001011001100111100010100110111110000111111101000101001"
                    "111111110101010001010001100000110010000110001100100110011100110100001100100100110000010100011000011000111111101000101001"
                },
        /* 36*/ { BARCODE_PDF417, -1, UNICODE_MODE, 0, 3, -1, "AB{}  C#+  de{}  {}F  12{}  G{}  H", 0, 9, 120, 0, "Text Compaction newtable; BWIPP same except ML before spaces instead of after",
                    "111111110101010001111101010111110011010111001111000111101010111100001100000100001011011111010101111100111111101000101001"
                    "111111110101010001111010100100000010011111010001110111000001011001001110100110000010011110101001000000111111101000101001"
                    "111111110101010001010100111100000010001101011100000110001111001011001000111100100001010101001111000000111111101000101001"
                    "111111110101010001101011110011111011001010001100000100000101000010001000001001101100011010111100111110111111101000101001"
                    "111111110101010001111010111001110011100000101110110111000001011101101000111010111110011110101110011100111111101000101001"
                    "111111110101010001111101011110110010110100000111000110001111001011001111101011110110011111010111101100111111101000101001"
                    "111111110101010001110100111011111011000110000100010110000110111011101000001010010000011101001110111110111111101000101001"
                    "111111110101010001010111111011100011010000001110010111000001011101101111110001110101010101111110111000111111101000101001"
                    "111111110101010001111110100110010010001111010000100110001010001111101010001101111100011111101001100100111111101000101001"
                },
        /* 37*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 4, -1, "\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction",
                    "11111111010101000111010101110000001101010000110000010000010000100010101000001001000001010000010010000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001111101000100011011111010001000110111110100010001101011111101011000011111010101100000111111101000101001"
                    "11111111010101000110101001111100001100011110101100011001101011110000100000111010110001011110011100111010101000111100000111111101000101001"
                },
        /* 38*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction",
                    "11111111010101000111010101110000001101010000110000010000010000100010101000001001000001010000010010000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001111101000100011011111010001000110111110100010001101011111101011000011111010101100000111111101000101001"
                    "11111111010101000110101001111100001100011110101100011001101011110000100000111010110001011110011100111010101000111100000111111101000101001"
                },
        /* 39*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 4, -1, "\177\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "11111111010101000111010101110000001101010000110000011000111000110100111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001110010000111011010100111110000110111101001100001101011111101011000011111010101100000111111101000101001"
                    "11111111010101000110101001111100001011111011101100010000001110100110110000011010111101111010111100001010101000111100000111111101000101001"
                },
        /* 40*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "11111111010101000111010101110000001101010000110000011000111000110100111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001110010000111011010100111110000110111101001100001101011111101011000011111010101100000111111101000101001"
                    "11111111010101000110101001111100001011111011101100010000001110100110110000011010111101111010111100001010101000111100000111111101000101001"
                },
        /* 41*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 4, -1, "\177\177\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction",
                    "11111111010101000111010101110000001101010000110000010000010000100010111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001110010000111011010100111110000110111101001100001101111101000100011011111010101100000111111101000101001"
                    "11111111010101000110101001111100001011000110011110010110001000111000100011010000111001001100100001110010101000111100000111111101000101001"
                },
        /* 42*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177\177\177", 0, 3, 137, 1, "Byte Compaction",
                    "11111111010101000111010101110000001101010000110000010000010000100010111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111101010001000001110010000111011010100111110000110111101001100001101111101000100011011111010101100000111111101000101001"
                    "11111111010101000110101001111100001011000110011110010110001000111000100011010000111001001100100001110010101000111100000111111101000101001"
                },
        /* 43*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 4, -1, "\177\177\177\177\177\177\177\177\177\177\177", 0, 4, 137, 1, "Byte Compaction",
                    "11111111010101000111101010111100001101011011100000010000010000100010111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111110101001100001110010000111011010100111110000110111101001100001101111101000100011011111101010111000111111101000101001"
                    "11111111010101000110101001111100001010000001011110010100000010111100101000000101111001010000001011110011010100111110000111111101000101001"
                    "11111111010101000101011110011110001010001000001000011011000010100000111000110001001101100111000110010010101111101111100111111101000101001"
                },
        /* 44*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177\177\177\177\177\177\177", 0, 4, 137, 1, "Byte Compaction",
                    "11111111010101000111101010111100001101011011100000010000010000100010111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111110101001100001110010000111011010100111110000110111101001100001101111101000100011011111101010111000111111101000101001"
                    "11111111010101000110101001111100001010000001011110010100000010111100101000000101111001010000001011110011010100111110000111111101000101001"
                    "11111111010101000101011110011110001010001000001000011011000010100000111000110001001101100111000110010010101111101111100111111101000101001"
                },
        /* 45*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 1, 4, -1, "\177\177\177\177\177\177\177\177\177\177\177\177", 0, 4, 137, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "11111111010101000111101010111100001101011011100000011000111000110100111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111110101001100001110010000111011010100111110000110111101001100001101111001010010000011111101010111000111111101000101001"
                    "11111111010101000110101001111100001001110000100110010011000100001110101000011001111101101000101111100011010100111110000111111101000101001"
                    "11111111010101000101011110011110001101000100011000010011000111001100110001100001000101110100010111000010101111101111100111111101000101001"
                },
        /* 46*/ { BARCODE_PDF417, -1, UNICODE_MODE, 1, 4, -1, "\177\177\177\177\177\177\177\177\177\177\177\177", 0, 4, 137, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "11111111010101000111101010111100001101011011100000011000111000110100111001001100111101000010100001000011101010011100000111111101000101001"
                    "11111111010101000111110101001100001110010000111011010100111110000110111101001100001101111001010010000011111101010111000111111101000101001"
                    "11111111010101000110101001111100001001110000100110010011000100001110101000011001111101101000101111100011010100111110000111111101000101001"
                    "11111111010101000101011110011110001101000100011000010011000111001100110001100001000101110100010111000010101111101111100111111101000101001"
                },
        /* 47*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, 5, -1, "1\177", 0, 3, 154, 1, "Byte Compaction, 1 Numeric, 1 Byte",
                    "1111111101010100011101010111000000111010100011100001000001000010001011010011011100000101000001001000001000011000110010011110101001111000111111101000101001"
                    "1111111101010100011110101000010000101111110101100001011111101011000011101001110110000111000010110100001110000110011101011111010101100000111111101000101001"
                    "1111111101010100011101010011111100100111100000100101101100010011110010111100000110110101101110111110001001101011100000010101000011110000111111101000101001"
                },
        /* 48*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, 5, -1, "1\177", 0, 3, 154, 0, "Byte Compaction, 1 Numeric, 1 Byte; BWIPP same as FAST_MODE",
                    "1111111101010100011101010111000000111010100011100001000001001100011011110011110010100101000001001000001000011000110010011110101001111000111111101000101001"
                    "1111111101010100011110101000010000101111110101100001011111101011000011011011110001000100011100101111101110100111000110011111010101100000111111101000101001"
                    "1111111101010100011101010011111100100110011011110001001100010000111010000011110010100100000100010111101011111011010000010101000011110000111111101000101001"
                },
        /* 49*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, 5, -1, "ABCDEF1234567890123\177\177\177\177VWXYZ", 0, 6, 154, 1, "Text, Numeric, Byte, Text",
                    "1111111101010100011110101011110000110101110111100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011110101000010000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011111101010111000111111101000101001"
                    "1111111101010100011101010011111100110011111101100101010000001011110010100000010111100101000000101111001010000001011110010101000011110000111111101000101001"
                    "1111111101010100010101111001111000100001100011001001000110111101110011110111101101100110111100111000101000011000110010011111010111111010111111101000101001"
                    "1111111101010100011010111000001000101111110101100001011111101011000011001011111001110111100100100100001011111101011000011101011100110000111111101000101001"
                    "1111111101010100011111010111100110110111110110011001101001011111000010101110011111100100100001000111101011000000101110011110101111101100111111101000101001"
                },
        /* 50*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, 5, -1, "ABCDEF1234567890123\177\177\177\177VWXYZ", 0, 6, 154, 1, "Text, Numeric, Byte(7), Text",
                    "1111111101010100011110101011110000110101110111100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011110101000010000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011111101010111000111111101000101001"
                    "1111111101010100011101010011111100110011111101100101010000001011110010100000010111100101000000101111001010000001011110010101000011110000111111101000101001"
                    "1111111101010100010101111001111000100001100011001001000110111101110011110111101101100110111100111000101000011000110010011111010111111010111111101000101001"
                    "1111111101010100011010111000001000101111110101100001011111101011000011001011111001110111100100100100001011111101011000011101011100110000111111101000101001"
                    "1111111101010100011111010111100110110111110110011001101001011111000010101110011111100100100001000111101011000000101110011110101111101100111111101000101001"
                },
        /* 51*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 6, 5, -1, "ABCDEF1234567890123\177\177\177\177VWXYZ", 0, 30, 154, 1, "ECC 6: Text, Numeric, Byte, Text",
                    "1111111101010100010101000001000000110101110111100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011110101100011000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011111010100001100111111101000101001"
                    "1111111101010100011101010011111100110011111101100101010000001011110010100000010111100101000000101111001010000001011110011111101011000010111111101000101001"
                    "1111111101010100011101001001110000100001100011001001000110111101110011110111101101100110111100111000101000011000110010011111010111111010111111101000101001"
                    "1111111101010100010101111000010000101111110101100001011111101011000011110001010000010100001001111000101011110011011111011101011100000110111111101000101001"
                    "1111111101010100011111010111100110100111011000111001111010001111010011001111001011000111001111110100101101110000101110011101001011111100111111101000101001"
                    "1111111101010100010100111100111100110011000010000101010000100000100011110011110101000110111100111001001101000110111000011010011100111100111111101000101001"
                    "1111111101010100011111010011011100101000111100100001101001110010000010000001011110100110111001111101001001100010111111011110100100100000111111101000101001"
                    "1111111101010100011010011001111110111111001011101101011111000110100011010101111100000100001110010001101010111001111110011101001111100100111111101000101001"
                    "1111111101010100010100011110111100111101111010001001101000110001110011110100101111000111101100101110001001001000010000010100011101110000111111101000101001"
                    "1111111101010100010100111100001000111000010110010001110011000111001011100101100000100100111011111101001111110111001010011101001110000110111111101000101001"
                    "1111111101010100010100010000011110111001000111110101001111001100011010100001101111100100110000001011101011100110000111010100001011110000111111101000101001"
                    "1111111101010100011101000001001110100001011000001101101111000111001010100001110001110100010100001000001010000110110000010100000101000000111111101000101001"
                    "1111111101010100011101000111011000110011111101011101101001111011000010001110111111010111110111011000101111011101110010011110100010000010111111101000101001"
                    "1111111101010100010100000100111100110000111110100101101111010001100010001111000100100101110000010011001000101000111100011111100101101000111111101000101001"
                    "1111111101010100011001011001110000100111000111010001011000011010000011001100011001100100010100010000001010111110111110010010100000100000111111101000101001"
                    "1111111101010100011101000011001000101111110100110001111101011011100010011101001111100110000001001110101111100010101100010100011111001100111111101000101001"
                    "1111111101010100011111001011100010110000111010111001101010000111110011111010111000100101111011001100001000011101000110010110100111000000111111101000101001"
                    "1111111101010100011101101000001100101110011100010001111001111010100010110011110111000100101000001000001011110011110100011110110100011100111111101000101001"
                    "1111111101010100010100000111101000111110001010011001111100011001001010110111111000110111100100100100001111110001101011011111010000010110111111101000101001"
                    "1111111101010100010110110111100000101111000010100001000111101000010011011010000111100100011010011100001001111000001001010110111011111000111111101000101001"
                    "1111111101010100010110111000011000101111001111001001000011011100011011100001101000110110011000011001101101001000001100010110111001100000111111101000101001"
                    "1111111101010100011100101100001000110110011110000101110010011101100011110100011000110111110110010100001011100010011111011111001010000110111111101000101001"
                    "1111111101010100011110010011110010100011111001101001001111000110011010001110110011100110000101011111001111110101101000010110010000111000111111101000101001"
                    "1111111101010100011001000100110000110100111001111001011100111101100011010000100011000110011011000110001101011001110000011011001000000100111111101000101001"
                    "1111111101010100011001011111001110110110111110011001110001001100100010010001111010000111101000100100001011111101101110010010111100100000111111101000101001"
                    "1111111101010100011111100100011010110100000101111101011001110111110010111000110111000110001001111110101111011011111001010110001001110000111111101000101001"
                    "1111111101010100010010001110001110100110000110010001100001001100111011101001110111110110111100011100101110101001110000010010001110111000111111101000101001"
                    "1111111101010100011100100110100000110101111011000001100110011110100010000010001111010111001011001000001001000011110100011110110110100000111111101000101001"
                    "1111111101010100010110001110111110111110001011100101111101100111101010000111010110000110110010111100001111101011100100010110000011011110111111101000101001"
                },
        /* 52*/ { BARCODE_PDF417, -1, UNICODE_MODE, 6, 5, -1, "ABCDEF1234567890123\177\177\177\177VWXYZ", 0, 30, 154, 1, "ECC 6: Text, Numeric, Byte, Text",
                    "1111111101010100010101000001000000110101110111100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011110101100011000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011111010100001100111111101000101001"
                    "1111111101010100011101010011111100110011111101100101010000001011110010100000010111100101000000101111001010000001011110011111101011000010111111101000101001"
                    "1111111101010100011101001001110000100001100011001001000110111101110011110111101101100110111100111000101000011000110010011111010111111010111111101000101001"
                    "1111111101010100010101111000010000101111110101100001011111101011000011110001010000010100001001111000101011110011011111011101011100000110111111101000101001"
                    "1111111101010100011111010111100110100111011000111001111010001111010011001111001011000111001111110100101101110000101110011101001011111100111111101000101001"
                    "1111111101010100010100111100111100110011000010000101010000100000100011110011110101000110111100111001001101000110111000011010011100111100111111101000101001"
                    "1111111101010100011111010011011100101000111100100001101001110010000010000001011110100110111001111101001001100010111111011110100100100000111111101000101001"
                    "1111111101010100011010011001111110111111001011101101011111000110100011010101111100000100001110010001101010111001111110011101001111100100111111101000101001"
                    "1111111101010100010100011110111100111101111010001001101000110001110011110100101111000111101100101110001001001000010000010100011101110000111111101000101001"
                    "1111111101010100010100111100001000111000010110010001110011000111001011100101100000100100111011111101001111110111001010011101001110000110111111101000101001"
                    "1111111101010100010100010000011110111001000111110101001111001100011010100001101111100100110000001011101011100110000111010100001011110000111111101000101001"
                    "1111111101010100011101000001001110100001011000001101101111000111001010100001110001110100010100001000001010000110110000010100000101000000111111101000101001"
                    "1111111101010100011101000111011000110011111101011101101001111011000010001110111111010111110111011000101111011101110010011110100010000010111111101000101001"
                    "1111111101010100010100000100111100110000111110100101101111010001100010001111000100100101110000010011001000101000111100011111100101101000111111101000101001"
                    "1111111101010100011001011001110000100111000111010001011000011010000011001100011001100100010100010000001010111110111110010010100000100000111111101000101001"
                    "1111111101010100011101000011001000101111110100110001111101011011100010011101001111100110000001001110101111100010101100010100011111001100111111101000101001"
                    "1111111101010100011111001011100010110000111010111001101010000111110011111010111000100101111011001100001000011101000110010110100111000000111111101000101001"
                    "1111111101010100011101101000001100101110011100010001111001111010100010110011110111000100101000001000001011110011110100011110110100011100111111101000101001"
                    "1111111101010100010100000111101000111110001010011001111100011001001010110111111000110111100100100100001111110001101011011111010000010110111111101000101001"
                    "1111111101010100010110110111100000101111000010100001000111101000010011011010000111100100011010011100001001111000001001010110111011111000111111101000101001"
                    "1111111101010100010110111000011000101111001111001001000011011100011011100001101000110110011000011001101101001000001100010110111001100000111111101000101001"
                    "1111111101010100011100101100001000110110011110000101110010011101100011110100011000110111110110010100001011100010011111011111001010000110111111101000101001"
                    "1111111101010100011110010011110010100011111001101001001111000110011010001110110011100110000101011111001111110101101000010110010000111000111111101000101001"
                    "1111111101010100011001000100110000110100111001111001011100111101100011010000100011000110011011000110001101011001110000011011001000000100111111101000101001"
                    "1111111101010100011001011111001110110110111110011001110001001100100010010001111010000111101000100100001011111101101110010010111100100000111111101000101001"
                    "1111111101010100011111100100011010110100000101111101011001110111110010111000110111000110001001111110101111011011111001010110001001110000111111101000101001"
                    "1111111101010100010010001110001110100110000110010001100001001100111011101001110111110110111100011100101110101001110000010010001110111000111111101000101001"
                    "1111111101010100011100100110100000110101111011000001100110011110100010000010001111010111001011001000001001000011110100011110110110100000111111101000101001"
                    "1111111101010100010110001110111110111110001011100101111101100111101010000111010110000110110010111100001111101011100100010110000011011110111111101000101001"
                },
        /* 53*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, 5, -1, "ABCDEF1234567890123\177\177\177\177YZ1234567890123", 0, 7, 154, 0, "Text, Numeric, Byte, 2 Text, Numeric; BWIPP different encodation",
                    "1111111101010100011111010101111100101011100011100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011111010100011000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011110101001000000111111101000101001"
                    "1111111101010100011101010011111100110011111101100101010000001011110010100000010111100101000000101111001010000001011110011010100011111000111111101000101001"
                    "1111111101010100011010111100111110100001100011001001100111001110011010011110111101000111101011001111101101000010000110011111010111111010111111101000101001"
                    "1111111101010100011010111000010000111111000111010101100000101110001011110010110110000101111110101100001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011111010111100110110001111100100101100011111001001010011100101100000110011101011100001111010111110110011110101111000010111111101000101001"
                    "1111111101010100011101001110111110110001001100011101010000111000111011101010011100000110100010001100001110111100110001011010011100111100111111101000101001"
                },
        /* 54*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, 5, -1, "ABCDEF1234567890123\177\177\177\177YZ1234567890123", 0, 6, 154, 1, "Text, Numeric, Byte, 2 Text, Numeric",
                    "1111111101010100011110101011110000110101110111100001111010101111000010100111001110000110100000101100001001111011110100011110101001111000111111101000101001"
                    "1111111101010100011110101000010000111101011001100001010011110000100011111100011101010110000010111000101111001011011000011111101010111000111111101000101001"
                    "1111111101010100011101010011111100100000111100101001111011011111010010011100001001100100110001000011101110100011111001010101000011110000111111101000101001"
                    "1111111101010100010101111001111000101101111011100001001111011110100011110101100111110110100001000011001000001001101100011111010111111010111111101000101001"
                    "1111111101010100011010111000001000110000010111000101111001011011000010110001111101000111010011100011001110110111010000011101011100110000111111101000101001"
                    "1111111101010100011111010111100110100001100110111101100110101111000010110011011110000111011100010111101001001001111000011110101111101100111111101000101001"
                },
        /* 55*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZ", 0, 14, 154, 1, "ECC 3",
                    "1111111101010100011110101001111000111010011001111001111010101111000010100111001110000110100000101100001111011010000111011110101001111000111111101000101001"
                    "1111111101010100011110101000001000100101111001000001110110111100110011101100111100110111000101100100001110001001110011011111101010011100111111101000101001"
                    "1111111101010100011101010011111100110111100010011001000001000101111010011110011000110101111100011001001111101011110110010101000001111000111111101000101001"
                    "1111111101010100011111010111111010101000111011100001110010110111100010010011000011000101100111100011101011000011000010011111010111111010111111101000101001"
                    "1111111101010100011010111000000100111001000001101001001101011111100010011001011111100111110111001100101100001011110110011101011100011000111111101000101001"
                    "1111111101010100011111010111100110110111110001000101110101011111100010100110011111000101000001000111101011011001111000011110101111100110111111101000101001"
                    "1111111101010100011010011100111100110010001001100001100100001101110010001011000110000110001000100110001001100011001000011010011100111100111111101000101001"
                    "1111111101010100011111010010011000110011001111010001111001100001101011000110111100100110000100011101001111010111001110010101111110011100111111101000101001"
                    "1111111101010100011010011001111110101000100000111101111100101111011011100100111110010101100001000011101001101110111110010100111011111100111111101000101001"
                    "1111111101010100010100011101110000111101110101100001001100111100111011100110000100110110001101000100001000110011001000010100011101110000111111101000101001"
                    "1111111101010100011010011100000100111000110011101001011111101011000010111111010110000101111110101100001111010100001000011101001110011000111111101000101001"
                    "1111111101010100010100010000011110100100110001111101011011000111100011000111100101100110010101111100001111101000111010010100011000111110111111101000101001"
                    "1111111101010100010100000101000000111101100100011101000011011010000011001000000101100110100000010011001010000110000011010100000101000000111111101000101001"
                    "1111111101010100011101000110100000100111101001111001111100100101100011110101101100000111100010011001101111000100000101011110100010010000111111101000101001"
                },
        /* 56*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZABCDEFGHIJKLMOPQRSTUVWXYZ", 0, 14, 154, 1, "ECC 3",
                    "1111111101010100011110101001111000111010011001111001111010101111000010100111001110000110100000101100001111011010000111011110101001111000111111101000101001"
                    "1111111101010100011110101000001000100101111001000001110110111100110011101100111100110111000101100100001110001001110011011111101010011100111111101000101001"
                    "1111111101010100011101010011111100110111100010011001000001000101111010011110011000110101111100011001001111101011110110010101000001111000111111101000101001"
                    "1111111101010100011111010111111010101000111011100001110010110111100010010011000011000101100111100011101011000011000010011111010111111010111111101000101001"
                    "1111111101010100011010111000000100111001000001101001001101011111100010011001011111100111110111001100101100001011110110011101011100011000111111101000101001"
                    "1111111101010100011111010111100110110111110001000101110101011111100010100110011111000101000001000111101011011001111000011110101111100110111111101000101001"
                    "1111111101010100011010011100111100110010001001100001100100001101110010001011000110000110001000100110001001100011001000011010011100111100111111101000101001"
                    "1111111101010100011111010010011000110011001111010001111001100001101011000110111100100110000100011101001111010111001110010101111110011100111111101000101001"
                    "1111111101010100011010011001111110101000100000111101111100101111011011100100111110010101100001000011101001101110111110010100111011111100111111101000101001"
                    "1111111101010100010100011101110000111101110101100001001100111100111011100110000100110110001101000100001000110011001000010100011101110000111111101000101001"
                    "1111111101010100011010011100000100111000110011101001011111101011000010111111010110000101111110101100001111010100001000011101001110011000111111101000101001"
                    "1111111101010100010100010000011110100100110001111101011011000111100011000111100101100110010101111100001111101000111010010100011000111110111111101000101001"
                    "1111111101010100010100000101000000111101100100011101000011011010000011001000000101100110100000010011001010000110000011010100000101000000111111101000101001"
                    "1111111101010100011101000110100000100111101001111001111100100101100011110101101100000111100010011001101111000100000101011110100010010000111111101000101001"
                },
        /* 57*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, 8, 29, 32, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 562, 1, "Max codewords (with padding)",
                    "1111111101010100011010100000110000111000100110111101111010101111000010100111001110000110100000101100001111011010000111011001000100110000110010000110111001110001011001111011110111010110000100110011110011101110011000010011011000110100010000100011001100100001100111001110011010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011010111000111100111111101000101001"
                    "1111111101010100011110101100000110101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011110101000001000111111101000101001"
                    "1111111101010100011111010111000100110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011111010111001000111111101000101001"
                    "1111111101010100011110100100111100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010100110000110000111111101000101001"
                    "1111111101010100011010111110111000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011010111000000100111111101000101001"
                    "1111111101010100010100100000111100110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011010010001111100111111101000101001"
                    "1111111101010100010100111100011110100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011010001100111000111111101000101001"
                    "1111111101010100011110100110001100101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011111010010011000111111101000101001"
                    "1111111101010100011101000101111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011010011111100010111111101000101001"
                    "1111111101010100011010000101100000100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010100001100011000111111101000101001"
                    "1111111101010100010100111110011000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011010011100000100111111101000101001"
                    "1111111101010100010100001100111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001010100001000011110111111101000101001"
                    "1111111101010100010100000100010000100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011110010100111100111111101000101001"
                    "1111111101010100011101000111000110101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011101000110100000111111101000101001"
                    "1111111101010100010010110000111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100001010111110010011000000101110110001111101001001100111111011001010000011100010110101100001101111001011110001100011011010110111111000100111110011000101111100101110100010001111000101000100010001101111101011110000011011011011110101100000111001110100111101011110001100110011001001011111000110010100001111101000110100001110011001011001111110111111101000101001"
                    "1111111101010100011100101100111100100110011010000001001011000011000010110001110110000101000110000110001000110111000110010110001111001110111010010000111001000001010100000011010000100011000101110011100010001010000010000010010100011000110000110000110100010001101101101100000011001001000011000110000100001011001100001100101000010000101000100000101110011111011101001000110000110010000010100000100100111011100100001101100010000001011110011110010010100110000111011001000110011110111011111001010111110100111001110010001101000111011110011111011010111100111111101000101001"
                    "1111111101010100011101000011101100110000001011101001011111010011100011111000110001010100010001111010001000110111110100010001101111100010111001011001000001101101111100110011111011001010000100100111111011101101110101111110011101000011010000110111101111110101111101101001000011100100111101110111110110001001001111110110101100011010111100110000101111100010011101111001101110011011100100110010000101011110000100001100001000011101011100100011100110110110011110001001110101100010000010011111011011110110001011111011101011111101101110010100011111000110111111101000101001"
                    "1111111101010100010110100000111000101111010000100001000001110100011010001111000010100110111110000101001000101011110000010100001100111110111111011001110101110100011111001011101111010011100111101011111001101001000010011110011111100101101000111011110100011101100111110100100010111000110011100110001111001011001000011110100100010000111101001000110100111111010001011110000100100010001111101100100100101100001111101011110000101000010111111011101000111001011111000101110100111110001010110011100111110111110010111001001011110000100010011101101000111110111111101000101001"
                    "1111111101010100011001001011000000111110101011111001110011110011001010001000110000110110011011100111001011100011100010010110111110111100100011000011101101100010000100011010001000010010000110110000100010001100010001001100011100111000100010110010000110111001101000011001110010111011100100000101000000101000001100001000110111011001101001000000110110010010000001110100001101111011100110010000110110010111101111101101000100001100010111000011101000110011001001000001101111001110010011101011100111110111110110010111101111101100101111011001001101110000111111101000101001"
                    "1111111101010100011110100000010010110001100111100101011111010001110011100000100011010101110111111001001111010001000010011101101110010000110101110010000001101000111010000010110101111110000101111100101110001100101110000001010111101100111110111001001110110001010001111110111011100011011101000110000100111101101110010011000010011100001101110100111101000010100001011000111111011010111011111100010110101111100011101111101110111011011111100101001110101110011011111101010001111101100010001111101011100110100011100100001101000011100010011110100000100100111111101000101001"
                    "1111111101010100011111011011110100100011110101000001010010111100000010001010000011110110111110001000101110110010011111011001111011001110110000011101011101011101110111100011111000101110100101100000101110001000110100001110011011110100000110101111101100001001000111000001011011000111100010110101110011000111001111000101111010010010111011111100110111100010001101101111000101100010000010101111000111111001011010001000111110011010010000111100101000101100111001111101001111001000010011111101001100100101111011100011101011100100000110010110111000111110111111101000101001"
                    "1111111101010100010010011101110000111001000100011101011000111100111011010000111011110111001100010110001100111001101000011010000010001100110110010000100001001000110110000011011011100111000110111000110001001000010000100100010001100110000100100100111100111101001111011110010011001101110011100110011000010100001011000110000010011110001101011100110001101100001101111101101001111010110000111011000111110101111110101001110000111010011000100010000110110000010010011001110100000100111011000110100000100100110011110111001100001100001001011011001000100000111111101000101001"
                    "1111111101010100011100101110110000110011011110100001111110111010100010111011001111110100110001111101001100001001110001011111010100110000111001100111001001001111101011100011110010001101100111110010101100001111000100100001011111010100011000111000001001100101111100110100010010110001111110110111110110110011001111101000100110010001100101111110101001111101100001111101000010011011111001000010110101110111111001001111101100110011010011110101111000111011001110000101110010110000001010111101000111100110001000111000101001011110001000011110010100000100111111101000101001"
                    "1111111101010100011001000100111110100111010001100001001101000001110010100110011111000110100001001111101100011010011110010011000010011100110111100001011001111010011110001011001111000010110101101001110000001000111100010100011101110100011110110011111001000101111001001111001010111100000100100100111001011000001110011010111110010110000110111100111111010011001001100111101000110011011101000111000101111000100010001101110000010111010000111011001110100111100001010001101000111111010010000001001011110100010010000111101101110110011110011001000101111100111111101000101001"
                    "1111111101010100011100100010011100110001110011000101100001100010100011000010001101110111010101110000001001000011011000010110111100011100110100001000110001010000110000110010111001110100000110110010001000001000101000100000010001101100001000111000110010001101101000010110000010011100111101100110101000001100001110110000101100011011001000000100111011100001000101000011011000010011000110100010000110010111001111001110110001001100011000010101100000111010001100111101000001000011011010110111011000000111011000001001101100010100110000010110011100110000111111101000101001"
                    "1111111101010100011111011010100000101101111110011001111001010001000011000100111101100111100101100011001111000101101100010001101111101000111101011000011001111000010110011011110000010010010111100101101100001110100011000010010111110100001110110001001110000101110100111100111011110101101100000111010011100011001101011110000110010011110110111110111100101110111001001111010011110011100101111011100111101110001110101111101000100110011000100111010000111101110111000101101000111000010010011111100101100111000101100000101100010001110010011001011110011000111111101000101001"
                    "1111111101010100010110001101111000111011110101110001100101111110010011000001101011110110111001000011101000010100011110010111000001001100110011110001011001011010000111000011111000100111010100110101110000001100110010011110011111010011101000111111010110000101101111100100100010000011110101000100100010000111101100011111010010010110000001011100110001101001111001000011100110111010110110111100000100111011101111001010000100011110011110010011110100111100010111101001100111101001100011100100101111110101100100000111001000111110011010010110001000001110111111101000101001"
                    "1111111101010100010110011110001110100111001111011001100011011100111010000101000010000110100001110111101111011110100100010000110111101110110110110110000001111011000100111010001000011101110100011000011001001110001100001011011110110000101110101110111001000001101100110110000011110011110010010110100010001100001100100000100011010110001110011000111000101000111001100111000110100010010111100111100100001001100001101010000100010000010111100011110100111000111011011001001000011000011011000110010100000111001100000101101101110011001000010010000100100000111111101000101001"
                    "1111111101010100011110010011001100101100010111111001111001001100011011101001100010000100111101101111101110100001110011011010000111010000111111011101101101001100011111001010100111111001110100010111100100001011000100111111011011111010011110110011011110100001110110000011101011001101111001000101100011111010001110110111001000011111000101001100101000111100100001100100011100100011101001101000000101000111100100001101111110100111010001111100101110100101111100001101000101111101100011011100011111010101110000101111101011010001111110011110010010010000111111101000101001"
                    "1111111101010100010001010001111000110101100111111001110100011111010010111100011000110110001110010011101000110101110000011111100100011010101100111011111001011001101111000011000111110100100101111011011000001010111011111100010011100011011100110111000101110001100011111001001010011010011100000100111001000001101111010111100010010111100010001000100111001000001101101000001011111010111001000110000110111101011000001011100110111000011101010111111000110011110001011001001110000010011010011100110111000100111000010001101111110101100100011100010101111110111111101000101001"
                    "1111111101010100010010000110110000110011010000010001010000100001000011010110111000000111011000001001101100010001001100011001100001010000111100010010111101101111011110011010001000110000110110110001000001001110110101100000011000110001001000110010001101110001010010000010000011111011110101100100010000010100001000010110001100011100010110111100110110010000001001111101100101111010000100001010000110010001100111001101110001101000011101000010111000111001000101110001000001000100100010101110001110000101000110000110001110011010000110011101100001000110111111101000101001"
                    "1111111101010100011011011111001100101111001001111001101110011111010011100110011100100110000100011101001111010011000110011110101110001110100010011110000101100101110000010010111101000111100111010011010000001101011111000111011110101101100000110001001111001101110100000011010010001000111101000111100010010100001111010110011000011001011111011100101000111101000001111001000001010011111001010001100111011100011110101110010000001101011011000011110010110000101110010001001001111001000010100001111100110110100111000010001011010011111100011011011110010000111111101000101001"
                },
        /* 58*/ { BARCODE_PDF417, -1, UNICODE_MODE, 8, 29, 32, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 562, 1, "Max codewords (with padding)",
                    "1111111101010100011010100000110000111000100110111101111010101111000010100111001110000110100000101100001111011010000111011001000100110000110010000110111001110001011001111011110111010110000100110011110011101110011000010011011000110100010000100011001100100001100111001110011010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011010111000111100111111101000101001"
                    "1111111101010100011110101100000110101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011110101000001000111111101000101001"
                    "1111111101010100011111010111000100110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011111010111001000111111101000101001"
                    "1111111101010100011110100100111100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010100110000110000111111101000101001"
                    "1111111101010100011010111110111000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011010111000000100111111101000101001"
                    "1111111101010100010100100000111100110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011010010001111100111111101000101001"
                    "1111111101010100010100111100011110100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011010001100111000111111101000101001"
                    "1111111101010100011110100110001100101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011111010010011000111111101000101001"
                    "1111111101010100011101000101111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011010011111100010111111101000101001"
                    "1111111101010100011010000101100000100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010100001100011000111111101000101001"
                    "1111111101010100010100111110011000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011010011100000100111111101000101001"
                    "1111111101010100010100001100111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001010100001000011110111111101000101001"
                    "1111111101010100010100000100010000100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010010000110001100100100001100011001001000011000110010011110010100111100111111101000101001"
                    "1111111101010100011101000111000110101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000010111111010110000101111110101100001011111101011000011101000110100000111111101000101001"
                    "1111111101010100010010110000111110110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100011111001001011000111110010010110001111100100101100001010111110010011000000101110110001111101001001100111111011001010000011100010110101100001101111001011110001100011011010110111111000100111110011000101111100101110100010001111000101000100010001101111101011110000011011011011110101100000111001110100111101011110001100110011001001011111000110010100001111101000110100001110011001011001111110111111101000101001"
                    "1111111101010100011100101100111100100110011010000001001011000011000010110001110110000101000110000110001000110111000110010110001111001110111010010000111001000001010100000011010000100011000101110011100010001010000010000010010100011000110000110000110100010001101101101100000011001001000011000110000100001011001100001100101000010000101000100000101110011111011101001000110000110010000010100000100100111011100100001101100010000001011110011110010010100110000111011001000110011110111011111001010111110100111001110010001101000111011110011111011010111100111111101000101001"
                    "1111111101010100011101000011101100110000001011101001011111010011100011111000110001010100010001111010001000110111110100010001101111100010111001011001000001101101111100110011111011001010000100100111111011101101110101111110011101000011010000110111101111110101111101101001000011100100111101110111110110001001001111110110101100011010111100110000101111100010011101111001101110011011100100110010000101011110000100001100001000011101011100100011100110110110011110001001110101100010000010011111011011110110001011111011101011111101101110010100011111000110111111101000101001"
                    "1111111101010100010110100000111000101111010000100001000001110100011010001111000010100110111110000101001000101011110000010100001100111110111111011001110101110100011111001011101111010011100111101011111001101001000010011110011111100101101000111011110100011101100111110100100010111000110011100110001111001011001000011110100100010000111101001000110100111111010001011110000100100010001111101100100100101100001111101011110000101000010111111011101000111001011111000101110100111110001010110011100111110111110010111001001011110000100010011101101000111110111111101000101001"
                    "1111111101010100011001001011000000111110101011111001110011110011001010001000110000110110011011100111001011100011100010010110111110111100100011000011101101100010000100011010001000010010000110110000100010001100010001001100011100111000100010110010000110111001101000011001110010111011100100000101000000101000001100001000110111011001101001000000110110010010000001110100001101111011100110010000110110010111101111101101000100001100010111000011101000110011001001000001101111001110010011101011100111110111110110010111101111101100101111011001001101110000111111101000101001"
                    "1111111101010100011110100000010010110001100111100101011111010001110011100000100011010101110111111001001111010001000010011101101110010000110101110010000001101000111010000010110101111110000101111100101110001100101110000001010111101100111110111001001110110001010001111110111011100011011101000110000100111101101110010011000010011100001101110100111101000010100001011000111111011010111011111100010110101111100011101111101110111011011111100101001110101110011011111101010001111101100010001111101011100110100011100100001101000011100010011110100000100100111111101000101001"
                    "1111111101010100011111011011110100100011110101000001010010111100000010001010000011110110111110001000101110110010011111011001111011001110110000011101011101011101110111100011111000101110100101100000101110001000110100001110011011110100000110101111101100001001000111000001011011000111100010110101110011000111001111000101111010010010111011111100110111100010001101101111000101100010000010101111000111111001011010001000111110011010010000111100101000101100111001111101001111001000010011111101001100100101111011100011101011100100000110010110111000111110111111101000101001"
                    "1111111101010100010010011101110000111001000100011101011000111100111011010000111011110111001100010110001100111001101000011010000010001100110110010000100001001000110110000011011011100111000110111000110001001000010000100100010001100110000100100100111100111101001111011110010011001101110011100110011000010100001011000110000010011110001101011100110001101100001101111101101001111010110000111011000111110101111110101001110000111010011000100010000110110000010010011001110100000100111011000110100000100100110011110111001100001100001001011011001000100000111111101000101001"
                    "1111111101010100011100101110110000110011011110100001111110111010100010111011001111110100110001111101001100001001110001011111010100110000111001100111001001001111101011100011110010001101100111110010101100001111000100100001011111010100011000111000001001100101111100110100010010110001111110110111110110110011001111101000100110010001100101111110101001111101100001111101000010011011111001000010110101110111111001001111101100110011010011110101111000111011001110000101110010110000001010111101000111100110001000111000101001011110001000011110010100000100111111101000101001"
                    "1111111101010100011001000100111110100111010001100001001101000001110010100110011111000110100001001111101100011010011110010011000010011100110111100001011001111010011110001011001111000010110101101001110000001000111100010100011101110100011110110011111001000101111001001111001010111100000100100100111001011000001110011010111110010110000110111100111111010011001001100111101000110011011101000111000101111000100010001101110000010111010000111011001110100111100001010001101000111111010010000001001011110100010010000111101101110110011110011001000101111100111111101000101001"
                    "1111111101010100011100100010011100110001110011000101100001100010100011000010001101110111010101110000001001000011011000010110111100011100110100001000110001010000110000110010111001110100000110110010001000001000101000100000010001101100001000111000110010001101101000010110000010011100111101100110101000001100001110110000101100011011001000000100111011100001000101000011011000010011000110100010000110010111001111001110110001001100011000010101100000111010001100111101000001000011011010110111011000000111011000001001101100010100110000010110011100110000111111101000101001"
                    "1111111101010100011111011010100000101101111110011001111001010001000011000100111101100111100101100011001111000101101100010001101111101000111101011000011001111000010110011011110000010010010111100101101100001110100011000010010111110100001110110001001110000101110100111100111011110101101100000111010011100011001101011110000110010011110110111110111100101110111001001111010011110011100101111011100111101110001110101111101000100110011000100111010000111101110111000101101000111000010010011111100101100111000101100000101100010001110010011001011110011000111111101000101001"
                    "1111111101010100010110001101111000111011110101110001100101111110010011000001101011110110111001000011101000010100011110010111000001001100110011110001011001011010000111000011111000100111010100110101110000001100110010011110011111010011101000111111010110000101101111100100100010000011110101000100100010000111101100011111010010010110000001011100110001101001111001000011100110111010110110111100000100111011101111001010000100011110011110010011110100111100010111101001100111101001100011100100101111110101100100000111001000111110011010010110001000001110111111101000101001"
                    "1111111101010100010110011110001110100111001111011001100011011100111010000101000010000110100001110111101111011110100100010000110111101110110110110110000001111011000100111010001000011101110100011000011001001110001100001011011110110000101110101110111001000001101100110110000011110011110010010110100010001100001100100000100011010110001110011000111000101000111001100111000110100010010111100111100100001001100001101010000100010000010111100011110100111000111011011001001000011000011011000110010100000111001100000101101101110011001000010010000100100000111111101000101001"
                    "1111111101010100011110010011001100101100010111111001111001001100011011101001100010000100111101101111101110100001110011011010000111010000111111011101101101001100011111001010100111111001110100010111100100001011000100111111011011111010011110110011011110100001110110000011101011001101111001000101100011111010001110110111001000011111000101001100101000111100100001100100011100100011101001101000000101000111100100001101111110100111010001111100101110100101111100001101000101111101100011011100011111010101110000101111101011010001111110011110010010010000111111101000101001"
                    "1111111101010100010001010001111000110101100111111001110100011111010010111100011000110110001110010011101000110101110000011111100100011010101100111011111001011001101111000011000111110100100101111011011000001010111011111100010011100011011100110111000101110001100011111001001010011010011100000100111001000001101111010111100010010111100010001000100111001000001101101000001011111010111001000110000110111101011000001011100110111000011101010111111000110011110001011001001110000010011010011100110111000100111000010001101111110101100100011100010101111110111111101000101001"
                    "1111111101010100010010000110110000110011010000010001010000100001000011010110111000000111011000001001101100010001001100011001100001010000111100010010111101101111011110011010001000110000110110110001000001001110110101100000011000110001001000110010001101110001010010000010000011111011110101100100010000010100001000010110001100011100010110111100110110010000001001111101100101111010000100001010000110010001100111001101110001101000011101000010111000111001000101110001000001000100100010101110001110000101000110000110001110011010000110011101100001000110111111101000101001"
                    "1111111101010100011011011111001100101111001001111001101110011111010011100110011100100110000100011101001111010011000110011110101110001110100010011110000101100101110000010010111101000111100111010011010000001101011111000111011110101101100000110001001111001101110100000011010010001000111101000111100010010100001111010110011000011001011111011100101000111101000001111001000001010011111001010001100111011100011110101110010000001101011011000011110010110000101110010001001001111001000010100001111100110110100111000010001011010011111100011011011110010000111111101000101001"
                },
        /* 59*/ { BARCODE_PDF417COMP, -1, UNICODE_MODE | FAST_MODE, 1, 2, -1, "PDF417 APK\012", 0, 6, 69, 0, "ISO 15438:2015 Figure G.1, same, BWIPP uses different encodation (AL before <SP>), same codeword count",
                    "111111110101010001111010101111000011010100001100000111011101100110001"
                    "111111110101010001111010100010000011010000111000100111101000101000001"
                    "111111110101010001110101011111100010110011011110000100111110011000101"
                    "111111110101010001010111100111100011001100100000010110000110001010001"
                    "111111110101010001111010111000111011111011000100100101110111111000101"
                    "111111110101010001111010111101000010011111001110110100011100011011101"
                },
        /* 60*/ { BARCODE_PDF417COMP, -1, UNICODE_MODE, 1, 2, -1, "PDF417 APK\012", 0, 6, 69, 0, "ISO 15438:2015 Figure G.1, same, BWIPP uses different encodation (AL before <SP>), same codeword count",
                    "111111110101010001111010101111000011010100001100000111011101100110001"
                    "111111110101010001111010100010000011010000111000100111101000101000001"
                    "111111110101010001110101011111100010110011011110000100111110011000101"
                    "111111110101010001010111100111100011001100100000010110000110001010001"
                    "111111110101010001111010111000111011111011000100100101110111111000101"
                    "111111110101010001111010111101000010011111001110110100011100011011101"
                },
        /* 61*/ { BARCODE_PDF417COMP, -1, UNICODE_MODE | FAST_MODE, 4, 4, -1, "ABCDEFG", 0, 10, 103, 1, "",
                    "1111111101010100011101010011100000110101000011000001111010101111000010100111001110000110100000101100001"
                    "1111111101010100011110101000000100110100000011100101011111101011000010111111010110000101111110101100001"
                    "1111111101010100011010100111110000101111001100011001000001111010100010011111001100100111001011111001001"
                    "1111111101010100010101111101111100100001011000110001100011100110010011011011100111000101111000111100101"
                    "1111111101010100011101011110111000111110011010000101110000010110010011100100011001000100011111001011101"
                    "1111111101010100011110101111001000111111010111011001110111111010100010100000010011110100111100110001101"
                    "1111111101010100010100111001110000100001101101000001101100110000110011011000110011000101000000101000001"
                    "1111111101010100011110100100010000111101001000010001111101011011100011100100001100100101001111100011001"
                    "1111111101010100010100110011111000100110000110111101100111000010111010010001011110000110011111010001001"
                    "1111111101010100010100011000001100110001101010000001100011000110011011001001101110000111110111110101001"
                },
        /* 62*/ { BARCODE_PDF417COMP, -1, UNICODE_MODE, 4, 4, -1, "ABCDEFG", 0, 10, 103, 1, "",
                    "1111111101010100011101010011100000110101000011000001111010101111000010100111001110000110100000101100001"
                    "1111111101010100011110101000000100110100000011100101011111101011000010111111010110000101111110101100001"
                    "1111111101010100011010100111110000101111001100011001000001111010100010011111001100100111001011111001001"
                    "1111111101010100010101111101111100100001011000110001100011100110010011011011100111000101111000111100101"
                    "1111111101010100011101011110111000111110011010000101110000010110010011100100011001000100011111001011101"
                    "1111111101010100011110101111001000111111010111011001110111111010100010100000010011110100111100110001101"
                    "1111111101010100010100111001110000100001101101000001101100110000110011011000110011000101000000101000001"
                    "1111111101010100011110100100010000111101001000010001111101011011100011100100001100100101001111100011001"
                    "1111111101010100010100110011111000100110000110111101100111000010111010010001011110000110011111010001001"
                    "1111111101010100010100011000001100110001101010000001100011000110011011001001101110000111110111110101001"
                },
        /* 63*/ { BARCODE_HIBC_PDF, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "H123ABC01234567890D", 0, 8, 120, 0, "BWIPP uses different encodation, same codeword count but zint half-pad shorter",
                    "111111110101010001111101010111110011101011001111000100000100010010001110001110100010011111010101111100111111101000101001"
                    "111111110101010001111110101000111011110000010001010110101111110111101111100011101101011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111010111101100100001111000101001100101000011111011101010001111110111111101000101001"
                    "111111110101010001101011110011111010000100000101000110001110110010001100100101100000011010111100111110111111101000101001"
                    "111111110101010001110101110000110010111111010110000110100111100110001011111101011000011110101110011100111111101000101001"
                    "111111110101010001111101011110110011000111110010010110010100111110001000001001001111011101011111010000111111101000101001"
                    "111111110101010001110100111011111010100110001100000110100011100111101111010010111100011101001110111110111111101000101001"
                    "111111110101010001111101001011000011100001001100100111010000011001001111011000110100010101111110111000111111101000101001"
                },
        /* 64*/ { BARCODE_HIBC_PDF, -1, UNICODE_MODE, -1, 3, -1, "H123ABC01234567890D", 0, 8, 120, 0, "BWIPP uses different encodation, same codeword count but zint half-pad shorter",
                    "111111110101010001111101010111110011101011001111000100000100010010001110001110100010011111010101111100111111101000101001"
                    "111111110101010001111110101000111011110000010001010110101111110111101111100011101101011110101001000000111111101000101001"
                    "111111110101010001010100111100000011111010111101100100001111000101001100101000011111011101010001111110111111101000101001"
                    "111111110101010001101011110011111010000100000101000110001110110010001100100101100000011010111100111110111111101000101001"
                    "111111110101010001110101110000110010111111010110000110100111100110001011111101011000011110101110011100111111101000101001"
                    "111111110101010001111101011110110011000111110010010110010100111110001000001001001111011101011111010000111111101000101001"
                    "111111110101010001110100111011111010100110001100000110100011100111101111010010111100011101001110111110111111101000101001"
                    "111111110101010001111101001011000011100001001100100111010000011001001111011000110100010101111110111000111111101000101001"
                },
        /* 65*/ { BARCODE_HIBC_PDF, -1, UNICODE_MODE | FAST_MODE, 1, 3, -1, "A123BJC5D6E71", 0, 6, 120, 1, "BWIPP example",
                    "111111110101010001111010101111000011110101101111100100000100010010001000011011100110011111010101111100111111101000101001"
                    "111111110101010001111010100010000011110000010001010110101111110111101111000001000101011111101010111000111111101000101001"
                    "111111110101010001010100111100000010110001100011110101111110111101101000111100011011010101000111100000111111101000101001"
                    "111111110101010001010111100111100011100011101001000100001101111011101110001110100010011010111100111110111111101000101001"
                    "111111110101010001111010111000111011010111110011100110100000011100101111110101000111011101011100110000111111101000101001"
                    "111111110101010001111101011110110010011100110011100100011110110011001011001011100000011110101111000100111111101000101001"
                },
        /* 66*/ { BARCODE_HIBC_PDF, -1, UNICODE_MODE, 1, 3, -1, "A123BJC5D6E71", 0, 6, 120, 1, "BWIPP example",
                    "111111110101010001111010101111000011110101101111100100000100010010001000011011100110011111010101111100111111101000101001"
                    "111111110101010001111010100010000011110000010001010110101111110111101111000001000101011111101010111000111111101000101001"
                    "111111110101010001010100111100000010110001100011110101111110111101101000111100011011010101000111100000111111101000101001"
                    "111111110101010001010111100111100011100011101001000100001101111011101110001110100010011010111100111110111111101000101001"
                    "111111110101010001111010111000111011010111110011100110100000011100101111110101000111011101011100110000111111101000101001"
                    "111111110101010001111101011110110010011100110011100100011110110011001011001011100000011110101111000100111111101000101001"
                },
        /* 67*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 1, -1, "ABCDEFGHIJKLMNOPQRSTUV", 0, 20, 38, 1, "ISO 24728:2006 Figure 1 1st 1x20, same",
                    "11110101001000011000110010011110101001"
                    "11100101001111110101011100011100101001"
                    "11101101001010011001111100011101101001"
                    "11101001001101000001011000011101001001"
                    "11101001101010000111110011011101001101"
                    "11101011101011001110011111011101011101"
                    "11101011001100100001101110011101011001"
                    "11101010001101100111100100011101010001"
                    "11001010001001100001000111011001010001"
                    "11001011001001100111100111011001011001"
                    "11001011101001100101111110011001011101"
                    "11001001101000001010111100011001001101"
                    "11001101101111001100100111011001101101"
                    "11101101101011100111111010011101101101"
                    "11100101101110011110100111011100101101"
                    "11000101101110001010011100011000101101"
                    "11000101001100010111100110011000101001"
                    "11001101001000011001000111011001101001"
                    "11011101001111011111011010011011101001"
                    "11011001001100010001110100011011001001"
                },
        /* 68*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "ABCDEFGHIJKLMNOPQRSTUV", 0, 20, 38, 1, "ISO 24728:2006 Figure 1 1st 1x20, same",
                    "11110101001000011000110010011110101001"
                    "11100101001111110101011100011100101001"
                    "11101101001010011001111100011101101001"
                    "11101001001101000001011000011101001001"
                    "11101001101010000111110011011101001101"
                    "11101011101011001110011111011101011101"
                    "11101011001100100001101110011101011001"
                    "11101010001101100111100100011101010001"
                    "11001010001001100001000111011001010001"
                    "11001011001001100111100111011001011001"
                    "11001011101001100101111110011001011101"
                    "11001001101000001010111100011001001101"
                    "11001101101111001100100111011001101101"
                    "11101101101011100111111010011101101101"
                    "11100101101110011110100111011100101101"
                    "11000101101110001010011100011000101101"
                    "11000101001100010111100110011000101001"
                    "11001101001000011001000111011001101001"
                    "11011101001111011111011010011011101001"
                    "11011001001100010001110100011011001001"
                },
        /* 69*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 20, 55, 1, "ISO 24728:2006 Figure 1 2nd 2x20, same",
                    "1111010100100001100011001001111010101111000011110101001"
                    "1110010100110101111110111101111101000100110011100101001"
                    "1110110100101101100111100001011001110011111011101101001"
                    "1110100100110010000110111001110001011001111011101001001"
                    "1110100110111001000001101001001101011111100011101001101"
                    "1110101110110111100101100001000001010111100011101011101"
                    "1110101100100011001100100001100111001110011011101011001"
                    "1110101000111111010101110001101011111101111011101010001"
                    "1100101000101000001000111101011011001111000011001010001"
                    "1100101100110010001001100001100100001101110011001011001"
                    "1100101110110110011110010001110010000011010011001011101"
                    "1100100110110011101000011101101111001011000011001001101"
                    "1100110110110001101000100001000110011001000011001101101"
                    "1110110110111000110011101001111110101011100011101101101"
                    "1110010110101001100111110001000110010011100011100101101"
                    "1100010110110110011011000001100110100010000011000101101"
                    "1100010100111111001010011101110000100110100011000101001"
                    "1100110100111001111100101101111110010011010011001101001"
                    "1101110100111010110011110001000001001101100011011101001"
                    "1101100100111100110110100001001001111001000011011001001"
                },
        /* 70*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 20, 55, 1, "ISO 24728:2006 Figure 1 2nd 2x20, same",
                    "1111010100100001100011001001111010101111000011110101001"
                    "1110010100110101111110111101111101000100110011100101001"
                    "1110110100101101100111100001011001110011111011101101001"
                    "1110100100110010000110111001110001011001111011101001001"
                    "1110100110111001000001101001001101011111100011101001101"
                    "1110101110110111100101100001000001010111100011101011101"
                    "1110101100100011001100100001100111001110011011101011001"
                    "1110101000111111010101110001101011111101111011101010001"
                    "1100101000101000001000111101011011001111000011001010001"
                    "1100101100110010001001100001100100001101110011001011001"
                    "1100101110110110011110010001110010000011010011001011101"
                    "1100100110110011101000011101101111001011000011001001101"
                    "1100110110110001101000100001000110011001000011001101101"
                    "1110110110111000110011101001111110101011100011101101101"
                    "1110010110101001100111110001000110010011100011100101101"
                    "1100010110110110011011000001100110100010000011000101101"
                    "1100010100111111001010011101110000100110100011000101001"
                    "1100110100111001111100101101111110010011010011001101001"
                    "1101110100111010110011110001000001001101100011011101001"
                    "1101100100111100110110100001001001111001000011011001001"
                },
        /* 71*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMN", 0, 20, 82, 1, "ISO 24728:2006 Figure 1 3rd 3x20",
                    "1100100010100001100011001001011110010111101010111100001010011100111000011100101101"
                    "1110100010111110100010011001011110110101000011111001101001011110010000011000101101"
                    "1110110010111100010111101001001110110110111011001111001001100001000111011000101001"
                    "1100110010100110011110011101001110100111001100001001101100011010001000011001101001"
                    "1101110010110000101111011001001100100111000110011101001111110101011100011011101001"
                    "1101111010101001100111110001001100110101000001000111101011011001111000011011001001"
                    "1100111010110010001001100001001000110110010000110111001110001011001111011011001101"
                    "1110111010111001000001101001001000010100110101111110001001100101111110011011011101"
                    "1110011010100000101011110001011000010110111110101000001101111100010001011011011001"
                    "1111011010111101010111100001011100010101001110011100001101000001011000011011010001"
                    "1111001010101000011111001101011100110100101111001000001110110111100110011010010001"
                    "1110001010110111011001111001011100100101111100110010001110011111010110011010110001"
                    "1100001010111100111101010001011101100111100111100101001111101110100111011010111001"
                    "1100011010111001101110010001001101100101001111000001001111010001100110011010111101"
                    "1100010010110010001111110101000101100110011100001011101111001000111101011010011101"
                    "1110010010100110001100000101000101000110111101110001001101100110110000011010011001"
                    "1111010010100111010001111101001101000100110111110100001111001110111010011010001001"
                    "1111010110101100100000011101011101000110001101000111101000000100101111011010001101"
                    "1111010100101111011110100001011001000111110011010111101011110111110110011010000101"
                    "1110010100110010001111011001011001100111000010111011001110001011100110011011000101"
                },
        /* 72*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMN", 0, 20, 82, 1, "ISO 24728:2006 Figure 1 3rd 3x20",
                    "1100100010100001100011001001011110010111101010111100001010011100111000011100101101"
                    "1110100010111110100010011001011110110101000011111001101001011110010000011000101101"
                    "1110110010111100010111101001001110110110111011001111001001100001000111011000101001"
                    "1100110010100110011110011101001110100111001100001001101100011010001000011001101001"
                    "1101110010110000101111011001001100100111000110011101001111110101011100011011101001"
                    "1101111010101001100111110001001100110101000001000111101011011001111000011011001001"
                    "1100111010110010001001100001001000110110010000110111001110001011001111011011001101"
                    "1110111010111001000001101001001000010100110101111110001001100101111110011011011101"
                    "1110011010100000101011110001011000010110111110101000001101111100010001011011011001"
                    "1111011010111101010111100001011100010101001110011100001101000001011000011011010001"
                    "1111001010101000011111001101011100110100101111001000001110110111100110011010010001"
                    "1110001010110111011001111001011100100101111100110010001110011111010110011010110001"
                    "1100001010111100111101010001011101100111100111100101001111101110100111011010111001"
                    "1100011010111001101110010001001101100101001111000001001111010001100110011010111101"
                    "1100010010110010001111110101000101100110011100001011101111001000111101011010011101"
                    "1110010010100110001100000101000101000110111101110001001101100110110000011010011001"
                    "1111010010100111010001111101001101000100110111110100001111001110111010011010001001"
                    "1111010110101100100000011101011101000110001101000111101000000100101111011010001101"
                    "1111010100101111011110100001011001000111110011010111101011110111110110011010000101"
                    "1110010100110010001111011001011001100111000010111011001110001011100110011011000101"
                },
        /* 73*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZAB", 0, 20, 99, 1, "ISO 24728:2006 Figure 1 4th 4x20, same",
                    "110010001010000110001100100111101010111100001011110010101001110011100001101000001011000011100101101"
                    "111010001010100001111100110100101111001000001011110110111011011110011001101100111100100011000101101"
                    "111011001010011000010001110110011101000011101001110110110111100101100001000001010111100011000101001"
                    "110011001010001100110010000110011100111001101001110100111101010111100001010011100111000011001101001"
                    "110111001011111010001001100101000011111001101001100100100101111001000001110110111100110011011101001"
                    "110111101011011101100111100100110000100011101001100110110011101000011101101111001011000011011001001"
                    "110011101011000110100010000100011001100100001001000110110011100111001101111010101111000011011001101"
                    "111011101011010111111011110111110100010011001001000010101000011111001101001011110010000011011011101"
                    "111001101011110001011110100110111011001111001011000010100110000100011101100111010000111011011011001"
                    "111101101011100110000100110110001101000100001011100010100011001100100001100111001110011011011010001"
                    "111100101011111101010111000110101111110111101011100110111110100010011001010000111110011011010010001"
                    "111000101010110011100111110111100010111101001011100100110111011001111001001100001000111011010110001"
                    "110000101010011001111001110111001100001001101011101100110001101000100001000110011001000011010111001"
                    "110001101011100011001110100111111010101110001001101100111101001110111001011111011001111011010111101"
                    "110001001011110101111101100101111001110011101000101100101110110111000001000010000101111011010011101"
                    "111001001010000010100000100111000111010010001000101000110010000010110001101100010000001011010011001"
                    "111101001011110111001110100110000010011100101001101000111101100110001001011110100001111011010001001"
                    "111101011010000001010011110100110110001111001011101000110000111100101101111010011110010011010001101"
                    "111101010011100011101010000110001011101111001011001000111110111101011001100101110111100011010000101"
                    "111001010010001000001111010111100010100001001011001100100111101101111101001110100111110011011000101"
                },
        /* 74*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZAB", 0, 20, 99, 1, "ISO 24728:2006 Figure 1 4th 4x20, same",
                    "110010001010000110001100100111101010111100001011110010101001110011100001101000001011000011100101101"
                    "111010001010100001111100110100101111001000001011110110111011011110011001101100111100100011000101101"
                    "111011001010011000010001110110011101000011101001110110110111100101100001000001010111100011000101001"
                    "110011001010001100110010000110011100111001101001110100111101010111100001010011100111000011001101001"
                    "110111001011111010001001100101000011111001101001100100100101111001000001110110111100110011011101001"
                    "110111101011011101100111100100110000100011101001100110110011101000011101101111001011000011011001001"
                    "110011101011000110100010000100011001100100001001000110110011100111001101111010101111000011011001101"
                    "111011101011010111111011110111110100010011001001000010101000011111001101001011110010000011011011101"
                    "111001101011110001011110100110111011001111001011000010100110000100011101100111010000111011011011001"
                    "111101101011100110000100110110001101000100001011100010100011001100100001100111001110011011011010001"
                    "111100101011111101010111000110101111110111101011100110111110100010011001010000111110011011010010001"
                    "111000101010110011100111110111100010111101001011100100110111011001111001001100001000111011010110001"
                    "110000101010011001111001110111001100001001101011101100110001101000100001000110011001000011010111001"
                    "110001101011100011001110100111111010101110001001101100111101001110111001011111011001111011010111101"
                    "110001001011110101111101100101111001110011101000101100101110110111000001000010000101111011010011101"
                    "111001001010000010100000100111000111010010001000101000110010000010110001101100010000001011010011001"
                    "111101001011110111001110100110000010011100101001101000111101100110001001011110100001111011010001001"
                    "111101011010000001010011110100110110001111001011101000110000111100101101111010011110010011010001101"
                    "111101010011100011101010000110001011101111001011001000111110111101011001100101110111100011010000101"
                    "111001010010001000001111010111100010100001001011001100100111101101111101001110100111110011011000101"
                },
        /* 75*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 1, -1, "123456789012345", 0, 14, 38, 1, "Number Compaction",
                    "11101110101011111101001100011101110101"
                    "11100110101110101011111100011100110101"
                    "11110110101000001011001100011110110101"
                    "11110010101111100100110111011110010101"
                    "11100010101000111110110010011100010101"
                    "11000010101010000010100000011000010101"
                    "11000110101110001100111010011000110101"
                    "11000100101000100001001111011000100101"
                    "11100100101011110011110010011100100101"
                    "11110100101110011000011101011110100101"
                    "11110101101000101000001111011110101101"
                    "11110101001111001010011110011110101001"
                    "11100101001101011110000110011100101001"
                    "11101101001101000111111001011101101001"
                },
        /* 76*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "123456789012345", 0, 14, 38, 1, "Number Compaction",
                    "11101110101011111101001100011101110101"
                    "11100110101110101011111100011100110101"
                    "11110110101000001011001100011110110101"
                    "11110010101111100100110111011110010101"
                    "11100010101000111110110010011100010101"
                    "11000010101010000010100000011000010101"
                    "11000110101110001100111010011000110101"
                    "11000100101000100001001111011000100101"
                    "11100100101011110011110010011100100101"
                    "11110100101110011000011101011110100101"
                    "11110101101000101000001111011110101101"
                    "11110101001111001010011110011110101001"
                    "11100101001101011110000110011100101001"
                    "11101101001101000111111001011101101001"
                },
        /* 77*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 1, -1, "+12345678901", 0, 14, 38, 0, "Single mixed + Number Compaction; BWIPP different encodation",
                    "11101110101011111101011000011101110101"
                    "11100110101101111110001101011100110101"
                    "11110110101001111011110100011110110101"
                    "11110010101101000111100011011110010101"
                    "11100010101111110100011001011100010101"
                    "11000010101100100000101100011000010101"
                    "11000110101101001110000001011000110101"
                    "11000100101001111011000110011000100101"
                    "11100100101010001110001110011100100101"
                    "11110100101110101111001110011110100101"
                    "11110101101110011110101110011110101101"
                    "11110101001001011000110000011110101001"
                    "11100101001101001110001000011100101001"
                    "11101101001000010011011111011101101001"
                },
        /* 78*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "+12345678901", 0, 14, 38, 0, "Single mixed + Number Compaction; BWIPP different encodation",
                    "11101110101011111101011000011101110101"
                    "11100110101101111110001101011100110101"
                    "11110110101001111011110100011110110101"
                    "11110010101101000111100011011110010101"
                    "11100010101111110100011001011100010101"
                    "11000010101100100000101100011000010101"
                    "11000110101101001110000001011000110101"
                    "11000100101001111011000110011000100101"
                    "11100100101010001110001110011100100101"
                    "11110100101110101111001110011110100101"
                    "11110101101110011110101110011110101101"
                    "11110101001001011000110000011110101001"
                    "11100101001101001110001000011100101001"
                    "11101101001000010011011111011101101001"
                },
        /* 79*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 2, -1, "\177\177\177", 0, 8, 55, 1, "Byte Compaction",
                    "1100100010100000100001000101010000010010000011001000101"
                    "1110100010111110100010001101111101000100011011101000101"
                    "1110110010110001111100100101100011111001001011101100101"
                    "1100110010100001100011001001000011000110010011001100101"
                    "1101110010111001000111011001011011111101100011011100101"
                    "1101111010111010011111101101001110010011000011011110101"
                    "1100111010111001111001100101000001001101100011001110101"
                    "1110111010111000101111011101110001000011010011101110101"
                },
        /* 80*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "\177\177\177", 0, 8, 55, 1, "Byte Compaction",
                    "1100100010100000100001000101010000010010000011001000101"
                    "1110100010111110100010001101111101000100011011101000101"
                    "1110110010110001111100100101100011111001001011101100101"
                    "1100110010100001100011001001000011000110010011001100101"
                    "1101110010111001000111011001011011111101100011011100101"
                    "1101111010111010011111101101001110010011000011011110101"
                    "1100111010111001111001100101000001001101100011001110101"
                    "1110111010111000101111011101110001000011010011101110101"
                },
        /* 81*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 2, -1, "\177\177\177\177\177\177", 0, 8, 55, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "1100100010110001110001101001110010011001111011001000101"
                    "1110100010100010001111010001110010000111011011101000101"
                    "1110110010101000011001111101101000101111100011101100101"
                    "1100110010100001100011001001000011000110010011001100101"
                    "1101110010101110001001111101101000111000001011011100101"
                    "1101111010100011001101111001010010000111100011011110101"
                    "1100111010100100010000100001110111101100001011001110101"
                    "1110111010111110011010100001101100001111010011101110101"
                },
        /* 82*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "\177\177\177\177\177\177", 0, 8, 55, 1, "Byte Compaction, mod 6 == 0 (924 emitted)",
                    "1100100010110001110001101001110010011001111011001000101"
                    "1110100010100010001111010001110010000111011011101000101"
                    "1110110010101000011001111101101000101111100011101100101"
                    "1100110010100001100011001001000011000110010011001100101"
                    "1101110010101110001001111101101000111000001011011100101"
                    "1101111010100011001101111001010010000111100011011110101"
                    "1100111010100100010000100001110111101100001011001110101"
                    "1110111010111110011010100001101100001111010011101110101"
                },
        /* 83*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFG\177\177\177", 0, 8, 82, 1, "Text & Byte Compaction",
                    "1100111010100001100011001001000010110111101010111100001010011100111000011001110101"
                    "1110111010111110100010011001000010010110100000011100101101111110101110011101110101"
                    "1110011010101000000101111001000011010101000000101111001010000001011110011100110101"
                    "1111011010100001100011001001000111010100100001010000001111001110100011011110110101"
                    "1111001010111000001001100101000110010111010011000010001110100110000100011110010101"
                    "1110001010101111010001000001000100010101100010000011101000000111001011011100010101"
                    "1100001010111110111010111001001100010110011100011000101101100001100110011000010101"
                    "1100011010110100011100001001001110010110110000111101001100011011110010011000110101"
                },
        /* 84*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFG\177\177\177", 0, 8, 82, 1, "Text & Byte Compaction",
                    "1100111010100001100011001001000010110111101010111100001010011100111000011001110101"
                    "1110111010111110100010011001000010010110100000011100101101111110101110011101110101"
                    "1110011010101000000101111001000011010101000000101111001010000001011110011100110101"
                    "1111011010100001100011001001000111010100100001010000001111001110100011011110110101"
                    "1111001010111000001001100101000110010111010011000010001110100110000100011110010101"
                    "1110001010101111010001000001000100010101100010000011101000000111001011011100010101"
                    "1100001010111110111010111001001100010110011100011000101101100001100110011000010101"
                    "1100011010110100011100001001001110010110110000111101001100011011110010011000110101"
                },
        /* 85*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "\177\177\177abcd123456789", 0, 8, 99, 1, "Byte & Text & Numeric Compaction",
                    "110011101010000010000100010101000001001000001000010110101000001001000001010000010010000011001110101"
                    "111011101010111111010110000110000010111001001000010010111101011100111001010011111000011011101110101"
                    "111001101011111010111101100101000100000111101000011010111110010111101101110010011111001011100110101"
                    "111101101011001000010011000100001100011001001000111010100001100011001001000011000110010011110110101"
                    "111100101010111111010110000101111110101100001000110010100100001111001001001110010111110011110010101"
                    "111000101010010101111000000111111011011101001000100010101100000110111101000100011011111011100010101"
                    "110000101011100011001101110100001101110110001001100010100010110000110001101100110000110011000010101"
                    "110001101011101110011110100101000011111001101001110010110001011110011001110011011101000011000110101"
                },
        /* 86*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "\177\177\177abcd123456789", 0, 6, 99, 0, "Byte & Text & Numeric Compaction -> BYT(8) NUM(8); BWIPP same as FAST_MODE",
                    "110010001010000010000100010111001001100111101011001110100001010000100001100010010000110011001000101"
                    "111010001011110111001110100110010111100001101001001110110100111000001001011111101001100011101000101"
                    "111011001011101010111111000100001001101111101001101110110001111101000101001000000101111011101100101"
                    "110011001010001110011110110101001110011100001000101110100010000110110001000001000011011011001100101"
                    "110111001011101100011100010101001111100001101000100110111001100111010001111011000110100011011100101"
                    "110111101010111101000000100111011111100101001000110110101101001110000001011100010011000011011110101"
                },
        /* 87*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "\177\177\177abcdef12345", 0, 6, 99, 1, "Byte & Text & Numeric Compaction",
                    "110010001010000010000100010101000001001000001011001110101000001001000001010000010010000011001000101"
                    "111010001010111111010110000110000010111001001001001110111101011100111001110100111001100011101000101"
                    "111011001010110100000111000111110101111011001001101110101000100000111101101101000001111011101100101"
                    "110011001010011011000010000110010001000001101000101110101101110110000001010000011011000011001100101"
                    "110111001011001011101000000101001111110011101000100110111010111101110001111101110011001011011100101"
                    "110111101010011100110011100110100011111101001000110110100110100000111001100100100111110011011110101"
                },
        /* 88*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "\177\177\177abcd123456789", 0, 6, 99, 0, "Byte & Text & Numeric Compaction; BWIPP same as FAST_MODE",
                    "110010001010000010000100010111001001100111101011001110100001010000100001100010010000110011001000101"
                    "111010001011110111001110100110010111100001101001001110110100111000001001011111101001100011101000101"
                    "111011001011101010111111000100001001101111101001101110110001111101000101001000000101111011101100101"
                    "110011001010001110011110110101001110011100001000101110100010000110110001000001000011011011001100101"
                    "110111001011101100011100010101001111100001101000100110111001100111010001111011000110100011011100101"
                    "110111101010111101000000100111011111100101001000110110101101001110000001011100010011000011011110101"
                },
        /* 89*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "\177\177\177abcdefgh1234567890123", 0, 8, 99, 1, "Byte & Text & Numeric Compaction",
                    "110011101010000010000100010101000001001000001000010110101000001001000001010000010010000011001110101"
                    "111011101010111111010110000110000010111001001000010010111101011100111001110100111001100011101110101"
                    "111001101011111001011110110101100110011110001000011010100001111000101001111110101100010011100110101"
                    "111101101011010000100001100100000100110110001000111010100000101101100001001001110011100011110110101"
                    "111100101010111111010110000101111110101100001000110010101111101011100001110011000111010011110010101"
                    "111000101010111111011110110110111111000110101000100010100111000000101101001111101101000011100010101"
                    "110000101011000011010000100100000101101100001001100010101110111110111001111001110010110011000010101"
                    "110001101011101110111100010100100011110100001001110010100000101111000101111001010010000011000110101"
                },
        /* 90*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "\177\177\177abcdefgh1234567890123", 0, 8, 99, 1, "Byte(6) & Text & Numeric Compaction",
                    "110011101010000010000100010101000001001000001000010110101000001001000001010000010010000011001110101"
                    "111011101010111111010110000110000010111001001000010010111101011100111001110100111001100011101110101"
                    "111001101011111001011110110101100110011110001000011010100001111000101001111110101100010011100110101"
                    "111101101011010000100001100100000100110110001000111010100000101101100001001001110011100011110110101"
                    "111100101010111111010110000101111110101100001000110010101111101011100001110011000111010011110010101"
                    "111000101010111111011110110110111111000110101000100010100111000000101101001111101101000011100010101"
                    "110000101011000011010000100100000101101100001001100010101110111110111001111001110010110011000010101"
                    "110001101011101110111100010100100011110100001001110010100000101111000101111001010010000011000110101"
                },
        /* 91*/ { BARCODE_HIBC_MICPDF, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "H123ABC01234567890D", 0, 8, 99, 0, "BWIPP uses different encodation, same codeword count but zint full-pad shorter",
                    "110011101010000110001100100100000100010010001000010110111000111010001001000001001100011011001110101"
                    "111011101011010111111011110111110001110110101000010010111101011100111001011111101001100011101110101"
                    "111001101011001010000111110100011110101000001000011010100111110001101001011011000111100011100110101"
                    "111101101010000110001100100101000010001000001000111010100001100011001001000011000110010011110110101"
                    "111100101010111111010110000101111110101100001000110010111001001100001001111110001101011011110010101"
                    "111000101010001100101110000110010010011111001000100010100111100001101101111110101100010011100010101"
                    "110000101010110110001000000111000101100111101001100010110111101110000101100010101100000011000010101"
                    "110001101011110110000011010111100100001101101001110010101101011111100001111001000110011011000110101"
                },
        /* 92*/ { BARCODE_HIBC_MICPDF, -1, UNICODE_MODE, -1, 4, -1, "H123ABC01234567890D", 0, 8, 99, 0, "BWIPP uses different encodation, same codeword count but zint full-pad shorter",
                    "110011101010000110001100100100000100010010001000010110111000111010001001000001001100011011001110101"
                    "111011101011010111111011110111110001110110101000010010111101011100111001011111101001100011101110101"
                    "111001101011001010000111110100011110101000001000011010100111110001101001011011000111100011100110101"
                    "111101101010000110001100100101000010001000001000111010100001100011001001000011000110010011110110101"
                    "111100101010111111010110000101111110101100001000110010111001001100001001111110001101011011110010101"
                    "111000101010001100101110000110010010011111001000100010100111100001101101111110101100010011100010101"
                    "110000101010110110001000000111000101100111101001100010110111101110000101100010101100000011000010101"
                    "110001101011110110000011010111100100001101101001110010101101011111100001111001000110011011000110101"
                },
        /* 93*/ { BARCODE_HIBC_MICPDF, -1, UNICODE_MODE | FAST_MODE, -1, 1, -1, "/EAH783", 0, 17, 38, 1, "HIBC Provider Applications Standard (PAS) example",
                    "11001101001100011111001001011001101001"
                    "11011101001000001000100100011011101001"
                    "11011001001000100011110100011011001001"
                    "11011001101111010000111101011011001101"
                    "11011011101101100100010000011011011101"
                    "11011011001111001010000100011011011001"
                    "11011010001010000110011111011011010001"
                    "11010010001101001100001110011010010001"
                    "11010110001011111101011000011010110001"
                    "11010111001100011111001001011010111001"
                    "11010111101100110001000010011010111101"
                    "11010011101100001000111001011010011101"
                    "11010011001011110111000111011010011001"
                    "11010001001110001100100011011010001001"
                    "11010001101110010000110100011010001101"
                    "11010000101101100100001111011010000101"
                    "11011000101110111000100010011011000101"
                },
        /* 94*/ { BARCODE_HIBC_MICPDF, -1, UNICODE_MODE, -1, 1, -1, "/EAH783", 0, 17, 38, 1, "HIBC Provider Applications Standard (PAS) example",
                    "11001101001100011111001001011001101001"
                    "11011101001000001000100100011011101001"
                    "11011001001000100011110100011011001001"
                    "11011001101111010000111101011011001101"
                    "11011011101101100100010000011011011101"
                    "11011011001111001010000100011011011001"
                    "11011010001010000110011111011011010001"
                    "11010010001101001100001110011010010001"
                    "11010110001011111101011000011010110001"
                    "11010111001100011111001001011010111001"
                    "11010111101100110001000010011010111101"
                    "11010011101100001000111001011010011101"
                    "11010011001011110111000111011010011001"
                    "11010001001110001100100011011010001001"
                    "11010001101110010000110100011010001101"
                    "11010000101101100100001111011010000101"
                    "11011000101110111000100010011011000101"
                },
        /* 95*/ { BARCODE_PDF417, 9, DATA_MODE, -1, -1, -1, "\342", 0, 7, 103, 1, "β",
                    "1111111101010100011111010101111100110101000110000001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111110101000011001011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000110110010011110001100011111001001011010100011111000111111101000101001"
                    "1111111101010100011010111100111110100110011100110001010001100001100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000110110001111000101111110010010111011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100011110001000101000110010111000011110101111000010111111101000101001"
                    "1111111101010100011101001110111110101110001110001001010001101100000011010011101111000111111101000101001"
                },
        /* 96*/ { BARCODE_PDF417, 9, DATA_MODE, -1, -1, -1, "\342", 0, 7, 103, 1, "β",
                    "1111111101010100011111010101111100110101000110000001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111110101000011001011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000110110010011110001100011111001001011010100011111000111111101000101001"
                    "1111111101010100011010111100111110100110011100110001010001100001100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000110110001111000101111110010010111011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100011110001000101000110010111000011110101111000010111111101000101001"
                    "1111111101010100011101001110111110101110001110001001010001101100000011010011101111000111111101000101001"
                },
        /* 97*/ { BARCODE_MICROPDF417, 9, DATA_MODE, -1, 1, -1, "\342\343", 0, 14, 38, 1, "βγ",
                    "11101110101001111110010110011101110101"
                    "11100110101101010000111110011100110101"
                    "11110110101000001000010001011110110101"
                    "11110010101111001011001100011110010101"
                    "11100010101110110010011111011100010101"
                    "11000010101000011000110010011000010101"
                    "11000110101011111101011000011000110101"
                    "11000100101001111000101000011000100101"
                    "11100100101000100000101000011100100101"
                    "11110100101101110010111111011110100101"
                    "11110101101101100101111000011110101101"
                    "11110101001101011100111100011110101001"
                    "11100101001011100101111100011100101001"
                    "11101101001101001001111100011101101001"
                },
        /* 98*/ { BARCODE_MICROPDF417, 9, DATA_MODE, -1, 1, -1, "\342\343", 0, 14, 38, 1, "βγ",
                    "11101110101001111110010110011101110101"
                    "11100110101101010000111110011100110101"
                    "11110110101000001000010001011110110101"
                    "11110010101111001011001100011110010101"
                    "11100010101110110010011111011100010101"
                    "11000010101000011000110010011000010101"
                    "11000110101011111101011000011000110101"
                    "11000100101001111000101000011000100101"
                    "11100100101000100000101000011100100101"
                    "11110100101101110010111111011110100101"
                    "11110101101101100101111000011110101101"
                    "11110101001101011100111100011110101001"
                    "11100101001011100101111100011100101001"
                    "11101101001101001001111100011101101001"
                },
        /* 99*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 1, -1, "12345678", 0, 11, 38, 1, "1 columns x 11 rows, variant 1",
                    "11001000101001111011110100011100110101"
                    "11101000101110100011000001011110110101"
                    "11101100101000011010011100011110010101"
                    "11001100101100110011000011011100010101"
                    "11011100101111100001010110011000010101"
                    "11011110101101100100111100011000110101"
                    "11001110101011110011111011011000100101"
                    "11101110101000110111110010011100100101"
                    "11100110101011011110111111011110100101"
                    "11110110101001000001000010011110101101"
                    "11110010101110001001110110011110101001"
                },
        /*100*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "12345678", 0, 11, 38, 1, "1 columns x 11 rows, variant 1",
                    "11001000101001111011110100011100110101"
                    "11101000101110100011000001011110110101"
                    "11101100101000011010011100011110010101"
                    "11001100101100110011000011011100010101"
                    "11011100101111100001010110011000010101"
                    "11011110101101100100111100011000110101"
                    "11001110101011110011111011011000100101"
                    "11101110101000110111110010011100100101"
                    "11100110101011011110111111011110100101"
                    "11110110101001000001000010011110101101"
                    "11110010101110001001110110011110101001"
                },
        /*101*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 1, -1, "123456789012345678901234567890", 0, 20, 38, 1, "1 columns x 20 rows, variant 4",
                    "11110101001001111011110100011110101001"
                    "11100101001111101010011000011100101001"
                    "11101101001111110010011001011101101001"
                    "11101001001001110011110110011101001001"
                    "11101001101111100010100110011101001101"
                    "11101011101010000010111100011101011101"
                    "11101011001011000111100111011101011001"
                    "11101010001110100000111011011101010001"
                    "11001010001110011101001111011001010001"
                    "11001011001100101110011110011001011001"
                    "11001011101100111011111010011001011101"
                    "11001001101011011000111100011001001101"
                    "11001101101100110000010010011001101101"
                    "11101101101100100011110110011101101101"
                    "11100101101001111110111001011100101101"
                    "11000101101001110011100100011000101101"
                    "11000101001010000111100100011000101001"
                    "11001101001011111101110001011001101001"
                    "11011101001011110111101000011011101001"
                    "11011001001010001111000010011011001001"
                },
        /*102*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "123456789012345678901234567890", 0, 20, 38, 1, "1 columns x 20 rows, variant 4",
                    "11110101001001111011110100011110101001"
                    "11100101001111101010011000011100101001"
                    "11101101001111110010011001011101101001"
                    "11101001001001110011110110011101001001"
                    "11101001101111100010100110011101001101"
                    "11101011101010000010111100011101011101"
                    "11101011001011000111100111011101011001"
                    "11101010001110100000111011011101010001"
                    "11001010001110011101001111011001010001"
                    "11001011001100101110011110011001011001"
                    "11001011101100111011111010011001011101"
                    "11001001101011011000111100011001001101"
                    "11001101101100110000010010011001101101"
                    "11101101101100100011110110011101101101"
                    "11100101101001111110111001011100101101"
                    "11000101101001110011100100011000101101"
                    "11000101001010000111100100011000101001"
                    "11001101001011111101110001011001101001"
                    "11011101001011110111101000011011101001"
                    "11011001001010001111000010011011001001"
                },
        /*103*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 1, -1, "1234567890123456789012345678901234567890", 0, 24, 38, 1, "1 columns x 24 rows, variant 5",
                    "11100110101000011110001010011110100101"
                    "11110110101101001000011000011110101101"
                    "11110010101101000011100010011110101001"
                    "11100010101001000011011111011100101001"
                    "11000010101011000011001000011101101001"
                    "11000110101100010111000100011101001001"
                    "11000100101110110101111100011101001101"
                    "11100100101101011100011110011101011101"
                    "11110100101001001111000001011101011001"
                    "11110101101110001110101111011101010001"
                    "11110101001101100001000010011001010001"
                    "11100101001110111011110100011001011001"
                    "11101101001111110100111011011001011101"
                    "11101001001000010000110011011001001101"
                    "11101001101111010000010010011001101101"
                    "11101011101100011111001001011101101101"
                    "11101011001000010011100111011100101101"
                    "11101010001111101100001001011000101101"
                    "11001010001011101101110000011000101001"
                    "11001011001000010011011000011001101001"
                    "11001011101100110111100100011011101001"
                    "11001001101001110011011100011011001001"
                    "11001101101000101101100000011011001101"
                    "11101101101111000010010010011011011101"
                },
        /*104*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "1234567890123456789012345678901234567890", 0, 24, 38, 1, "1 columns x 24 rows, variant 5",
                    "11100110101000011110001010011110100101"
                    "11110110101101001000011000011110101101"
                    "11110010101101000011100010011110101001"
                    "11100010101001000011011111011100101001"
                    "11000010101011000011001000011101101001"
                    "11000110101100010111000100011101001001"
                    "11000100101110110101111100011101001101"
                    "11100100101101011100011110011101011101"
                    "11110100101001001111000001011101011001"
                    "11110101101110001110101111011101010001"
                    "11110101001101100001000010011001010001"
                    "11100101001110111011110100011001011001"
                    "11101101001111110100111011011001011101"
                    "11101001001000010000110011011001001101"
                    "11101001101111010000010010011001101101"
                    "11101011101100011111001001011101101101"
                    "11101011001000010011100111011100101101"
                    "11101010001111101100001001011000101101"
                    "11001010001011101101110000011000101001"
                    "11001011001000010011011000011001101001"
                    "11001011101100110111100100011011101001"
                    "11001001101001110011011100011011001001"
                    "11001101101000101101100000011011001101"
                    "11101101101111000010010010011011011101"
                },
        /*105*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 1, -1, "12345678901234567890123456789012345678901234567890", 0, 28, 38, 1, "1 columns x 28 rows, variant 6",
                    "11101011001001111011110100011100101101"
                    "11101010001100010111110111011000101101"
                    "11001010001110100111110001011000101001"
                    "11001011001010000011000110011001101001"
                    "11001011101110010000001101011011101001"
                    "11001001101111110001011010011011001001"
                    "11001101101010011110111100011011001101"
                    "11101101101111110101101111011011011101"
                    "11100101101101101000111100011011011001"
                    "11000101101001100111000011011011010001"
                    "11000101001101111101001111011010010001"
                    "11001101001000111001011000011010110001"
                    "11011101001100010001000011011010111001"
                    "11011001001011111001011100011010111101"
                    "11011001101010110111110000011010011101"
                    "11011011101110111010100000011010011001"
                    "11011011001111110101011100011010001001"
                    "11011010001000011100001011011010001101"
                    "11010010001010001100011000011010000101"
                    "11010110001011111101011000011011000101"
                    "11010111001110111110101100011001000101"
                    "11010111101101011110111110011101000101"
                    "11010011101011110100111100011101100101"
                    "11010011001011101110111100011001100101"
                    "11010001001000101100001100011011100101"
                    "11010001101111101000100011011011110101"
                    "11010000101001111001100110011001110101"
                    "11011000101110101011100000011101110101"
                },
        /*106*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 1, -1, "12345678901234567890123456789012345678901234567890", 0, 28, 38, 1, "1 columns x 28 rows, variant 6",
                    "11101011001001111011110100011100101101"
                    "11101010001100010111110111011000101101"
                    "11001010001110100111110001011000101001"
                    "11001011001010000011000110011001101001"
                    "11001011101110010000001101011011101001"
                    "11001001101111110001011010011011001001"
                    "11001101101010011110111100011011001101"
                    "11101101101111110101101111011011011101"
                    "11100101101101101000111100011011011001"
                    "11000101101001100111000011011011010001"
                    "11000101001101111101001111011010010001"
                    "11001101001000111001011000011010110001"
                    "11011101001100010001000011011010111001"
                    "11011001001011111001011100011010111101"
                    "11011001101010110111110000011010011101"
                    "11011011101110111010100000011010011001"
                    "11011011001111110101011100011010001001"
                    "11011010001000011100001011011010001101"
                    "11010010001010001100011000011010000101"
                    "11010110001011111101011000011011000101"
                    "11010111001110111110101100011001000101"
                    "11010111101101011110111110011101000101"
                    "11010011101011110100111100011101100101"
                    "11010011001011101110111100011001100101"
                    "11010001001000101100001100011011100101"
                    "11010001101111101000100011011011110101"
                    "11010000101001111001100110011001110101"
                    "11011000101110101011100000011101110101"
                },
        /*107*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTU", 0, 11, 55, 1, "2 columns x 11 rows, variant 8",
                    "1100100010100001100011001001111010101111000011100110101"
                    "1110100010110101111110111101111101000100110011110110101"
                    "1110110010101101100111100001011001110011111011110010101"
                    "1100110010110010000110111001110001011001111011100010101"
                    "1101110010111001000001101001001101011111100011000010101"
                    "1101111010110111100101100001100001101001111011000110101"
                    "1100111010100001100011001001110010000101110011000100101"
                    "1110111010111101110111010001111100001010110011100100101"
                    "1110011010110010001001111101101100101111000011110100101"
                    "1111011010111000110100001101000101110111000011110101101"
                    "1111001010110001011100000101000100011110001011110101001"
                },
        /*108*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTU", 0, 11, 55, 1, "2 columns x 11 rows, variant 8",
                    "1100100010100001100011001001111010101111000011100110101"
                    "1110100010110101111110111101111101000100110011110110101"
                    "1110110010101101100111100001011001110011111011110010101"
                    "1100110010110010000110111001110001011001111011100010101"
                    "1101110010111001000001101001001101011111100011000010101"
                    "1101111010110111100101100001100001101001111011000110101"
                    "1100111010100001100011001001110010000101110011000100101"
                    "1110111010111101110111010001111100001010110011100100101"
                    "1110011010110010001001111101101100101111000011110100101"
                    "1111011010111000110100001101000101110111000011110101101"
                    "1111001010110001011100000101000100011110001011110101001"
                },
        /*109*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZA", 0, 14, 55, 1, "2 columns x 14 rows, variant 9",
                    "1110111010101111110101100001111110101011100011101110101"
                    "1110011010101001100111110001010000010001111011100110101"
                    "1111011010111101101000011101100100010011000011110110101"
                    "1111001010111011011110011001101100111100100011110010101"
                    "1110001010100110000100011101100111010000111011100010101"
                    "1100001010111001100001001101100011010001000011000010101"
                    "1100011010110000101111011001110001100111010011000110101"
                    "1100010010101011100011111101100011111001001011000100101"
                    "1110010010100001100011001001000011000110010011100100101"
                    "1111010010101111110101100001110111000111101011110100101"
                    "1111010110101111110111001001111001011110100011110101101"
                    "1111010100101010000010000001110100001001110011110101001"
                    "1110010100101111100110111101110110011110011011100101001"
                    "1110110100100000011010111001100111101000011011101101001"
                },
        /*110*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZA", 0, 14, 55, 1, "2 columns x 14 rows, variant 9",
                    "1110111010101111110101100001111110101011100011101110101"
                    "1110011010101001100111110001010000010001111011100110101"
                    "1111011010111101101000011101100100010011000011110110101"
                    "1111001010111011011110011001101100111100100011110010101"
                    "1110001010100110000100011101100111010000111011100010101"
                    "1100001010111001100001001101100011010001000011000010101"
                    "1100011010110000101111011001110001100111010011000110101"
                    "1100010010101011100011111101100011111001001011000100101"
                    "1110010010100001100011001001000011000110010011100100101"
                    "1111010010101111110101100001110111000111101011110100101"
                    "1111010110101111110111001001111001011110100011110101101"
                    "1111010100101010000010000001110100001001110011110101001"
                    "1110010100101111100110111101110110011110011011100101001"
                    "1110110100100000011010111001100111101000011011101101001"
                },
        /*111*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL", 0, 17, 55, 1, "2 columns x 17 rows, variant 10",
                    "1100110100110001111100100101110101011111100011001101001"
                    "1101110100101001110011100001101000001011000011011101001"
                    "1101100100101000011111001101001011110010000011011001001"
                    "1101100110111100010111101001101110110011110011011001101"
                    "1101101110111101110101100001001100111100111011011011101"
                    "1101101100100110010111111001111101110011001011011011001"
                    "1101101000110111110101000001101111100010001011011010001"
                    "1101001000111101010111100001010011100111000011010010001"
                    "1101011000111110100010011001010000111110011011010110001"
                    "1101011100101100111001111101111000101111010011010111001"
                    "1101011110100001100011001001000011000110010011010111101"
                    "1101001110101111110101100001011111101011000011010011101"
                    "1101001100101110100000110001001111001010000011010011001"
                    "1101000100111001000110111101110011000010011011010001001"
                    "1101000110111111001010111001111000101100110011010001101"
                    "1101000010101110011101111001010010011110000011010000101"
                    "1101100010100101111101111101000110000011010011011000101"
                },
        /*112*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL", 0, 17, 55, 1, "2 columns x 17 rows, variant 10",
                    "1100110100110001111100100101110101011111100011001101001"
                    "1101110100101001110011100001101000001011000011011101001"
                    "1101100100101000011111001101001011110010000011011001001"
                    "1101100110111100010111101001101110110011110011011001101"
                    "1101101110111101110101100001001100111100111011011011101"
                    "1101101100100110010111111001111101110011001011011011001"
                    "1101101000110111110101000001101111100010001011011010001"
                    "1101001000111101010111100001010011100111000011010010001"
                    "1101011000111110100010011001010000111110011011010110001"
                    "1101011100101100111001111101111000101111010011010111001"
                    "1101011110100001100011001001000011000110010011010111101"
                    "1101001110101111110101100001011111101011000011010011101"
                    "1101001100101110100000110001001111001010000011010011001"
                    "1101000100111001000110111101110011000010011011010001001"
                    "1101000110111111001010111001111000101100110011010001101"
                    "1101000010101110011101111001010010011110000011010000101"
                    "1101100010100101111101111101000110000011010011011000101"
                },
        /*113*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", 0, 23, 55, 1, "2 columns x 23 rows, variant 12",
                    "1110011010110001111100100101110101011111100011110100101"
                    "1111011010101001110011100001101000001011000011110101101"
                    "1111001010101000011111001101001011110010000011110101001"
                    "1110001010111100010111101001101110110011110011100101001"
                    "1100001010111101110101100001001100111100111011101101001"
                    "1100011010100110010111111001111101110011001011101001001"
                    "1100010010110111110101000001101111100010001011101001101"
                    "1110010010111101010111100001010011100111000011101011101"
                    "1111010010111110100010011001010000111110011011101011001"
                    "1111010110101100111001111101111000101111010011101010001"
                    "1111010100111000101100111101111011101011000011001010001"
                    "1110010100100110101111110001001100101111110011001011001"
                    "1110110100100000101011110001101111101010000011001011101"
                    "1110100100110011100111001101111010101111000011001001101"
                    "1110100110110101111110111101111101000100110011001101101"
                    "1110101110111110010011100101100011111001001011101101101"
                    "1110101100100001100011001001110111100110010011100101101"
                    "1110101000110111011111001001110000100011010011000101101"
                    "1100101000100011010011100001011000100000111011000101001"
                    "1100101100111010001001110001101000101100000011001101001"
                    "1100101110111010000001101001011001111100010011011101001"
                    "1100100110101100011011110001111101101111010011011001001"
                    "1100110110100001000010000101100010001000011011011001101"
                },
        /*114*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", 0, 23, 55, 1, "2 columns x 23 rows, variant 12",
                    "1110011010110001111100100101110101011111100011110100101"
                    "1111011010101001110011100001101000001011000011110101101"
                    "1111001010101000011111001101001011110010000011110101001"
                    "1110001010111100010111101001101110110011110011100101001"
                    "1100001010111101110101100001001100111100111011101101001"
                    "1100011010100110010111111001111101110011001011101001001"
                    "1100010010110111110101000001101111100010001011101001101"
                    "1110010010111101010111100001010011100111000011101011101"
                    "1111010010111110100010011001010000111110011011101011001"
                    "1111010110101100111001111101111000101111010011101010001"
                    "1111010100111000101100111101111011101011000011001010001"
                    "1110010100100110101111110001001100101111110011001011001"
                    "1110110100100000101011110001101111101010000011001011101"
                    "1110100100110011100111001101111010101111000011001001101"
                    "1110100110110101111110111101111101000100110011001101101"
                    "1110101110111110010011100101100011111001001011101101101"
                    "1110101100100001100011001001110111100110010011100101101"
                    "1110101000110111011111001001110000100011010011000101101"
                    "1100101000100011010011100001011000100000111011000101001"
                    "1100101100111010001001110001101000101100000011001101001"
                    "1100101110111010000001101001011001111100010011011101001"
                    "1100100110101100011011110001111101101111010011011001001"
                    "1100110110100001000010000101100010001000011011011001101"
                },
        /*115*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ", 0, 26, 55, 1, "2 columns x 26 rows, variant 13",
                    "1100101000110001111100100101110101011111100011000101001"
                    "1100101100101001110011100001101000001011000011001101001"
                    "1100101110101000011111001101001011110010000011011101001"
                    "1100100110111100010111101001101110110011110011011001001"
                    "1100110110111101110101100001001100111100111011011001101"
                    "1110110110100110010111111001111101110011001011011011101"
                    "1110010110110111110101000001101111100010001011011011001"
                    "1100010110111101010111100001010011100111000011011010001"
                    "1100010100111110100010011001010000111110011011010010001"
                    "1100110100101100111001111101111000101111010011010110001"
                    "1101110100111000101100111101111011101011000011010111001"
                    "1101100100100110101111110001001100101111110011010111101"
                    "1101100110100000101011110001101111101010000011010011101"
                    "1101101110110011100111001101111010101111000011010011001"
                    "1101101100110101111110111101111101000100110011010001001"
                    "1101101000101101100111100001011001110011111011010001101"
                    "1101001000110010000110111001110001011001111011010000101"
                    "1101011000111001000001101001111001101100100011011000101"
                    "1101011100110001111100100101101001000011111011001000101"
                    "1101011110100100001101100001011100011100010011101000101"
                    "1101001110100010011110100001111110110100110011101100101"
                    "1101001100111101011111001101100011101001110011001100101"
                    "1101000100110111011110011101011100011100010011011100101"
                    "1101000110100111011011111101110110011101000011011110101"
                    "1101000010100010111011111101010100011110000011001110101"
                    "1101100010110001000001011001110101000111000011101110101"
                },
        /*116*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 2, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ", 0, 26, 55, 1, "2 columns x 26 rows, variant 13",
                    "1100101000110001111100100101110101011111100011000101001"
                    "1100101100101001110011100001101000001011000011001101001"
                    "1100101110101000011111001101001011110010000011011101001"
                    "1100100110111100010111101001101110110011110011011001001"
                    "1100110110111101110101100001001100111100111011011001101"
                    "1110110110100110010111111001111101110011001011011011101"
                    "1110010110110111110101000001101111100010001011011011001"
                    "1100010110111101010111100001010011100111000011011010001"
                    "1100010100111110100010011001010000111110011011010010001"
                    "1100110100101100111001111101111000101111010011010110001"
                    "1101110100111000101100111101111011101011000011010111001"
                    "1101100100100110101111110001001100101111110011010111101"
                    "1101100110100000101011110001101111101010000011010011101"
                    "1101101110110011100111001101111010101111000011010011001"
                    "1101101100110101111110111101111101000100110011010001001"
                    "1101101000101101100111100001011001110011111011010001101"
                    "1101001000110010000110111001110001011001111011010000101"
                    "1101011000111001000001101001111001101100100011011000101"
                    "1101011100110001111100100101101001000011111011001000101"
                    "1101011110100100001101100001011100011100010011101000101"
                    "1101001110100010011110100001111110110100110011101100101"
                    "1101001100111101011111001101100011101001110011001100101"
                    "1101000100110111011110011101011100011100010011011100101"
                    "1101000110100111011011111101110110011101000011011110101"
                    "1101000010100010111011111101010100011110000011001110101"
                    "1101100010110001000001011001110101000111000011101110101"
                },
        /*117*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJ", 0, 6, 82, 1, "3 columns x 6 rows, variant 14",
                    "1100100010100001100011001001011001110111101010111100001010011100111000011001000101"
                    "1110100010111110100010011001001001110101000011111001101001011110010000011101000101"
                    "1110110010110100010001111101001101110110000101001111101001000100111100011101100101"
                    "1100110010111101101001110001000101110100100011101110001011110011110001011001100101"
                    "1101110010111010011110011101000100110111011100111100101110100111001100011011100101"
                    "1101111010100001111010000101000110110100110100000011101100100111111001011011110101"
                },
        /*118*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJ", 0, 6, 82, 1, "3 columns x 6 rows, variant 14",
                    "1100100010100001100011001001011001110111101010111100001010011100111000011001000101"
                    "1110100010111110100010011001001001110101000011111001101001011110010000011101000101"
                    "1110110010110100010001111101001101110110000101001111101001000100111100011101100101"
                    "1100110010111101101001110001000101110100100011101110001011110011110001011001100101"
                    "1101110010111010011110011101000100110111011100111100101110100111001100011011100101"
                    "1101111010100001111010000101000110110100110100000011101100100111111001011011110101"
                },
        /*119*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTU", 0, 10, 82, 1, "3 columns x 10 rows, variant 16",
                    "1100010010110001111100100101001111010111010101111110001010011001111100011000100101"
                    "1110010010110100000101100001011111010111101101000011101100100010011000011100100101"
                    "1111010010111011011110011001011110010110110011110010001110010000011010011110100101"
                    "1111010110110011101000011101011110110110111100101100001100001101001111011110101101"
                    "1111010100100001100011001001001110110100001100011001001101110001101000011110101001"
                    "1110010100110000100111010001001110100111010001110110001101011100000100011100101001"
                    "1110110100110100110011111101001100100110011100001011101000110010000111011101101001"
                    "1110100100101011110011110001001100110100001001001000001101101000001000011101001001"
                    "1110100110101011111001100001001000110101110010011111001111101100101000011101001101"
                    "1110101110110111101100111001001000010110111110000101001001110001101110011101011101"
                },
        /*120*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTU", 0, 10, 82, 1, "3 columns x 10 rows, variant 16",
                    "1100010010110001111100100101001111010111010101111110001010011001111100011000100101"
                    "1110010010110100000101100001011111010111101101000011101100100010011000011100100101"
                    "1111010010111011011110011001011110010110110011110010001110010000011010011110100101"
                    "1111010110110011101000011101011110110110111100101100001100001101001111011110101101"
                    "1111010100100001100011001001001110110100001100011001001101110001101000011110101001"
                    "1110010100110000100111010001001110100111010001110110001101011100000100011100101001"
                    "1110110100110100110011111101001100100110011100001011101000110010000111011101101001"
                    "1110100100101011110011110001001100110100001001001000001101101000001000011101001001"
                    "1110100110101011111001100001001000110101110010011111001111101100101000011101001101"
                    "1110101110110111101100111001001000010110111110000101001001110001101110011101011101"
                },
        /*121*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 12, 82, 1, "3 columns x 12 rows, variant 17",
                    "1110101100100001100011001001011000010111101010111100001010011100111000011101011001"
                    "1110101000111110100010011001011100010101000011111001101001011110010000011101010001"
                    "1100101000111100010111101001011100110110111011001111001001100001000111011001010001"
                    "1100101100100110011110011101011100100111001100001001101100011010001000011001011001"
                    "1100101110110000101111011001011101100111000110011101001111110101011100011001011101"
                    "1100100110101001100111110001001101100110001111100100101100011111001001011001001101"
                    "1100110110110011010000100001000101100100011101110010001101100100000010011001101101"
                    "1110110110111101011000110001000101000111101110111010001111110111010100011101101101"
                    "1110010110100011010111000001001101000101110000001011001110101111101000011100101101"
                    "1100010110101100001110001101011101000110111000111001101110001011011110011000101101"
                    "1100010100111101100011000101011001000110110001111000101100011001111001011000101001"
                    "1100110100101010000011110001011001100111001111101011001111010111100010011001101001"
                },
        /*122*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 12, 82, 1, "3 columns x 12 rows, variant 17",
                    "1110101100100001100011001001011000010111101010111100001010011100111000011101011001"
                    "1110101000111110100010011001011100010101000011111001101001011110010000011101010001"
                    "1100101000111100010111101001011100110110111011001111001001100001000111011001010001"
                    "1100101100100110011110011101011100100111001100001001101100011010001000011001011001"
                    "1100101110110000101111011001011101100111000110011101001111110101011100011001011101"
                    "1100100110101001100111110001001101100110001111100100101100011111001001011001001101"
                    "1100110110110011010000100001000101100100011101110010001101100100000010011001101101"
                    "1110110110111101011000110001000101000111101110111010001111110111010100011101101101"
                    "1110010110100011010111000001001101000101110000001011001110101111101000011100101101"
                    "1100010110101100001110001101011101000110111000111001101110001011011110011000101101"
                    "1100010100111101100011000101011001000110110001111000101100011001111001011000101001"
                    "1100110100101010000011110001011001100111001111101011001111010111100010011001101001"
                },
        /*123*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI", 0, 15, 82, 1, "3 columns x 15 rows, variant 18",
                    "1101110100100001100011001001011000100111101010111100001010011100111000011011101001"
                    "1101100100111110100010011001011000110101000011111001101001011110010000011011001001"
                    "1101100110111100010111101001010000110110111011001111001001100001000111011011001101"
                    "1101101110100110011110011101010001110111001100001001101100011010001000011011011101"
                    "1101101100110000101111011001010001100111000110011101001111110101011100011011011001"
                    "1101101000101001100111110001010011100101000001000111101011011001111000011011010001"
                    "1101001000110110011100111001010011000100001100011001001000011000110010011010010001"
                    "1101011000101111110101100001010111000101111110101100001011111101011000011010110001"
                    "1101011100110111100100001101010110000111011010011111001101111001011000011010111001"
                    "1101011110110011100110000101010010000100010110000011001011110001111001011010111101"
                    "1101001110111010001100100001011010000100111110100011101011010001111110011010011101"
                    "1101001100110011110010110001001010000110011111101100101000111000101100011010011001"
                    "1101000100100001100110010001001011000100000100001010001111010000101111011010001001"
                    "1101000110111110101101110001001011100101111110101100001111100110100001011010001101"
                    "1101000010111011101011110001011011100100011100000101101011000110001111011010000101"
                },
        /*124*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI", 0, 15, 82, 1, "3 columns x 15 rows, variant 18",
                    "1101110100100001100011001001011000100111101010111100001010011100111000011011101001"
                    "1101100100111110100010011001011000110101000011111001101001011110010000011011001001"
                    "1101100110111100010111101001010000110110111011001111001001100001000111011011001101"
                    "1101101110100110011110011101010001110111001100001001101100011010001000011011011101"
                    "1101101100110000101111011001010001100111000110011101001111110101011100011011011001"
                    "1101101000101001100111110001010011100101000001000111101011011001111000011011010001"
                    "1101001000110110011100111001010011000100001100011001001000011000110010011010010001"
                    "1101011000101111110101100001010111000101111110101100001011111101011000011010110001"
                    "1101011100110111100100001101010110000111011010011111001101111001011000011010111001"
                    "1101011110110011100110000101010010000100010110000011001011110001111001011010111101"
                    "1101001110111010001100100001011010000100111110100011101011010001111110011010011101"
                    "1101001100110011110010110001001010000110011111101100101000111000101100011010011001"
                    "1101000100100001100110010001001011000100000100001010001111010000101111011010001001"
                    "1101000110111110101101110001001011100101111110101100001111100110100001011010001101"
                    "1101000010111011101011110001011011100100011100000101101011000110001111011010000101"
                },
        /*125*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", 0, 26, 82, 1, "3 columns x 26 rows, variant 20",
                    "1100100010100001100011001001000011010111101010111100001010011100111000011110100101"
                    "1110100010111110100010011001000111010101000011111001101001011110010000011110101101"
                    "1110110010111100010111101001000110010110111011001111001001100001000111011110101001"
                    "1100110010100110011110011101000100010111001100001001101100011010001000011100101001"
                    "1101110010110000101111011001001100010111000110011101001111110101011100011101101001"
                    "1101111010101001100111110001001110010101000001000111101011011001111000011101001001"
                    "1100111010110010001001100001001111010110010000110111001110001011001111011101001101"
                    "1110111010111001000001101001011111010100110101111110001001100101111110011101011101"
                    "1110011010100000101011110001011110010110111110101000001101111100010001011101011001"
                    "1111011010111101010111100001011110110101001110011100001101000001011000011101010001"
                    "1111001010101000011111001101001110110100101111001000001110110111100110011001010001"
                    "1110001010110111011001111001001110100100110000100011101100111010000111011001011001"
                    "1100001010111001100001001101001100100100001100011001001000011000110010011001011101"
                    "1100011010101111110101100001001100110101111110101100001011111101011000011001001101"
                    "1100010010110001111100100101001000110110001111100100101100011111001001011001101101"
                    "1110010010100001100011001001001000010101000000100000101001111011110100011101101101"
                    "1111010010100111001011111001011000010111101101100100001111010111000111011100101101"
                    "1111010110101101001110000001011100010110011111101101001100011101000111011000101101"
                    "1111010100111001110001010001011100110100111000111010001111001101000111011000101001"
                    "1110010100111000101100001001011100100111000110011101001100011011110001011001101001"
                    "1110110100100111000010011001011101100100000110110111101010011011111000011011101001"
                    "1110100100110010000110011101001101100111001111011000101110010001011100011011001001"
                    "1110100110110001011110001101000101100111100001011001101111001101101000011011001101"
                    "1110101110110011110010001101000101000100111101001000001101111101100110011011011101"
                    "1110101100100110011010000001001101000111011110001100101100110000100100011011011001"
                    "1110101000111100010100100001011101000101111101001110001111100100010011011011010001"
                },
        /*126*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", 0, 26, 82, 1, "3 columns x 26 rows, variant 20",
                    "1100100010100001100011001001000011010111101010111100001010011100111000011110100101"
                    "1110100010111110100010011001000111010101000011111001101001011110010000011110101101"
                    "1110110010111100010111101001000110010110111011001111001001100001000111011110101001"
                    "1100110010100110011110011101000100010111001100001001101100011010001000011100101001"
                    "1101110010110000101111011001001100010111000110011101001111110101011100011101101001"
                    "1101111010101001100111110001001110010101000001000111101011011001111000011101001001"
                    "1100111010110010001001100001001111010110010000110111001110001011001111011101001101"
                    "1110111010111001000001101001011111010100110101111110001001100101111110011101011101"
                    "1110011010100000101011110001011110010110111110101000001101111100010001011101011001"
                    "1111011010111101010111100001011110110101001110011100001101000001011000011101010001"
                    "1111001010101000011111001101001110110100101111001000001110110111100110011001010001"
                    "1110001010110111011001111001001110100100110000100011101100111010000111011001011001"
                    "1100001010111001100001001101001100100100001100011001001000011000110010011001011101"
                    "1100011010101111110101100001001100110101111110101100001011111101011000011001001101"
                    "1100010010110001111100100101001000110110001111100100101100011111001001011001101101"
                    "1110010010100001100011001001001000010101000000100000101001111011110100011101101101"
                    "1111010010100111001011111001011000010111101101100100001111010111000111011100101101"
                    "1111010110101101001110000001011100010110011111101101001100011101000111011000101101"
                    "1111010100111001110001010001011100110100111000111010001111001101000111011000101001"
                    "1110010100111000101100001001011100100111000110011101001100011011110001011001101001"
                    "1110110100100111000010011001011101100100000110110111101010011011111000011011101001"
                    "1110100100110010000110011101001101100111001111011000101110010001011100011011001001"
                    "1110100110110001011110001101000101100111100001011001101111001101101000011011001101"
                    "1110101110110011110010001101000101000100111101001000001101111101100110011011011101"
                    "1110101100100110011010000001001101000111011110001100101100110000100100011011011001"
                    "1110101000111100010100100001011101000101111101001110001111100100010011011011010001"
                },
        /*127*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 82, 1, "3 columns x 26 rows, variant 20",
                    "1110110100110001111100100101011101100111010101111110001010011001111100011011101001"
                    "1110100100110100000101100001001101100111101101000011101100100010011000011011001001"
                    "1110100110111011011110011001000101100110110011110010001110010000011010011011001101"
                    "1110101110110011101000011101000101000110111100101100001000001010111100011011011101"
                    "1110101100100011001100100001001101000110011100111001101111010101111000011011011001"
                    "1110101000110101111110111101011101000111110100010011001010000111110011011011010001"
                    "1100101000101100111001111101011001000111100010111101001101110110011110011010010001"
                    "1100101100111101110101100001011001100100110011110011101110011000010011011010110001"
                    "1100101110111110111001100101011000100110000101111011001110001100111010011010111001"
                    "1100100110111010101111110001011000110101001100111110001010000010001111011010111101"
                    "1100110110111101101000011101010000110110010001001100001100100001101110011010011101"
                    "1110110110110110011110010001010001110111001000001101001001101011111100011010011001"
                    "1110010110110111100101100001010001100100000101011110001101111101010000011010001001"
                    "1100010110110011100111001101010011100111101010111100001010011100111000011010001101"
                    "1100010100111110100010011001010011000101000011111001101001011110010000011010000101"
                    "1100110100111100010111101001010111000110111011001111001001100001000111011011000101"
                    "1101110100100110011110011101010110000111001100001001101100011010001000011001000101"
                    "1101100100110000101111011001010010000111000110011101001011111101011000011101000101"
                    "1101100110110001111100100101011010000110001111100100101100011111001001011101100101"
                    "1101101110100001100011001001001010000111101111101101001110110000100011011001100101"
                    "1101101100100001011111001101001011000111010000111001101110011011101000011011100101"
                    "1101101000110011111101100101001011100101111100011101101100100100111110011011110101"
                    "1101001000101000010001000001011011100100110111001100001110111010100000011001110101"
                    "1101011000101011111100111001011011110111110001110110101111110101001110011101110101"
                    "1101011100100011001001110001011001110100011110101000001100111110010010011100110101"
                    "1101011110111011100010000101001001110101101111001110001001100111000011011110110101"
                    "1101001110100111111010011001001101110100011001111101001111000100110011011110010101"
                    "1101001100100011101011000001000101110111010111110010001000010010011110011100010101"
                    "1101000100111000010010011101000100110100001011000001101000101111001111011000010101"
                    "1101000110111110010000101101000110110111001101111001101111110110001011011000110101"
                    "1101000010110010100001111101000010110100111100110011001111010000111101011000100101"
                    "1101100010101110011101000001000010010110000100010001101110001101000011011100100101"
                },
        /*128*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 82, 1, "3 columns x 26 rows, variant 20",
                    "1110110100110001111100100101011101100111010101111110001010011001111100011011101001"
                    "1110100100110100000101100001001101100111101101000011101100100010011000011011001001"
                    "1110100110111011011110011001000101100110110011110010001110010000011010011011001101"
                    "1110101110110011101000011101000101000110111100101100001000001010111100011011011101"
                    "1110101100100011001100100001001101000110011100111001101111010101111000011011011001"
                    "1110101000110101111110111101011101000111110100010011001010000111110011011011010001"
                    "1100101000101100111001111101011001000111100010111101001101110110011110011010010001"
                    "1100101100111101110101100001011001100100110011110011101110011000010011011010110001"
                    "1100101110111110111001100101011000100110000101111011001110001100111010011010111001"
                    "1100100110111010101111110001011000110101001100111110001010000010001111011010111101"
                    "1100110110111101101000011101010000110110010001001100001100100001101110011010011101"
                    "1110110110110110011110010001010001110111001000001101001001101011111100011010011001"
                    "1110010110110111100101100001010001100100000101011110001101111101010000011010001001"
                    "1100010110110011100111001101010011100111101010111100001010011100111000011010001101"
                    "1100010100111110100010011001010011000101000011111001101001011110010000011010000101"
                    "1100110100111100010111101001010111000110111011001111001001100001000111011011000101"
                    "1101110100100110011110011101010110000111001100001001101100011010001000011001000101"
                    "1101100100110000101111011001010010000111000110011101001011111101011000011101000101"
                    "1101100110110001111100100101011010000110001111100100101100011111001001011101100101"
                    "1101101110100001100011001001001010000111101111101101001110110000100011011001100101"
                    "1101101100100001011111001101001011000111010000111001101110011011101000011011100101"
                    "1101101000110011111101100101001011100101111100011101101100100100111110011011110101"
                    "1101001000101000010001000001011011100100110111001100001110111010100000011001110101"
                    "1101011000101011111100111001011011110111110001110110101111110101001110011101110101"
                    "1101011100100011001001110001011001110100011110101000001100111110010010011100110101"
                    "1101011110111011100010000101001001110101101111001110001001100111000011011110110101"
                    "1101001110100111111010011001001101110100011001111101001111000100110011011110010101"
                    "1101001100100011101011000001000101110111010111110010001000010010011110011100010101"
                    "1101000100111000010010011101000100110100001011000001101000101111001111011000010101"
                    "1101000110111110010000101101000110110111001101111001101111110110001011011000110101"
                    "1101000010110010100001111101000010110100111100110011001111010000111101011000100101"
                    "1101100010101110011101000001000010010110000100010001101110001101000011011100100101"
                },
        /*129*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 38, 82, 1, "3 columns x 38 rows, variant 22",
                    "1100010010110001111100100101000101100111010101111110001010011001111100011010011101"
                    "1110010010110100000101100001000101000111101101000011101100100010011000011010011001"
                    "1111010010111011011110011001001101000110110011110010001110010000011010011010001001"
                    "1111010110110011101000011101011101000110111100101100001000001010111100011010001101"
                    "1111010100100011001100100001011001000110011100111001101111010101111000011010000101"
                    "1110010100110101111110111101011001100111110100010011001010000111110011011011000101"
                    "1110110100101100111001111101011000100111100010111101001101110110011110011001000101"
                    "1110100100111101110101100001011000110100110011110011101110011000010011011101000101"
                    "1110100110111110111001100101010000110110000101111011001110001100111010011101100101"
                    "1110101110111010101111110001010001110101001100111110001010000010001111011001100101"
                    "1110101100111101101000011101010001100110010001001100001100100001101110011011100101"
                    "1110101000110110011110010001010011100111001000001101001001101011111100011011110101"
                    "1100101000110111100101100001010011000100000101011110001101111101010000011001110101"
                    "1100101100110011100111001101010111000111101010111100001010011100111000011101110101"
                    "1100101110111110100010011001010110000101000011111001101001011110010000011100110101"
                    "1100100110111100010111101001010010000110111011001111001001100001000111011110110101"
                    "1100110110100110011110011101011010000111001100001001101100011010001000011110010101"
                    "1110110110110000101111011001001010000111000110011101001111110101011100011100010101"
                    "1110010110101001100111110001001011000101000001000111101011011001111000011000010101"
                    "1100010110110010001001100001001011100110010000110111001110001011001111011000110101"
                    "1100010100111001000001101001011011100100110101111110001001100101111110011000100101"
                    "1100110100100000101011110001011011110110111110101000001101111100010001011100100101"
                    "1101110100100001100011001001011001110100001100011001001000011000110010011110100101"
                    "1101100100101111110101100001001001110110110011110001001110000101100100011110101101"
                    "1101100110101111000001001001001101110111111001011000101000110001011100011110101001"
                    "1101101110110111000110010001000101110101001111001111001100100101100000011100101001"
                    "1101101100101100011111000101000100110101111110011011101011100011111101011101101001"
                    "1101101000101111010000001001000110110101110011101111001001110011101111011101001001"
                    "1101001000111110111010011101000010110110110011100111001100100001001100011101001101"
                    "1101011000100101111000010001000010010100111111001011001101001110010000011101011101"
                    "1101011100100011101000110001000011010110011100001011101011101001100000011101011001"
                    "1101011110110110000110011001000111010111000011010001101001000100001000011101010001"
                    "1101001110110101110000001001000110010110010011111011101111010001100110011001010001"
                    "1101001100111110001011101001000100010100011001001110001011111011100011011001011001"
                    "1101000100100100001100001101001100010111011000010001101001011100111000011001011101"
                    "1101000110110000010011100101001110010101100111110001001111010111000111011001001101"
                    "1101000010110100101111100001001111010101000000100111101011101111011111011001101101"
                    "1101100010111011101100001101011111010100000101000010001011110111100010011101101101"
                },
        /*130*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 38, 82, 1, "3 columns x 38 rows, variant 22",
                    "1100010010110001111100100101000101100111010101111110001010011001111100011010011101"
                    "1110010010110100000101100001000101000111101101000011101100100010011000011010011001"
                    "1111010010111011011110011001001101000110110011110010001110010000011010011010001001"
                    "1111010110110011101000011101011101000110111100101100001000001010111100011010001101"
                    "1111010100100011001100100001011001000110011100111001101111010101111000011010000101"
                    "1110010100110101111110111101011001100111110100010011001010000111110011011011000101"
                    "1110110100101100111001111101011000100111100010111101001101110110011110011001000101"
                    "1110100100111101110101100001011000110100110011110011101110011000010011011101000101"
                    "1110100110111110111001100101010000110110000101111011001110001100111010011101100101"
                    "1110101110111010101111110001010001110101001100111110001010000010001111011001100101"
                    "1110101100111101101000011101010001100110010001001100001100100001101110011011100101"
                    "1110101000110110011110010001010011100111001000001101001001101011111100011011110101"
                    "1100101000110111100101100001010011000100000101011110001101111101010000011001110101"
                    "1100101100110011100111001101010111000111101010111100001010011100111000011101110101"
                    "1100101110111110100010011001010110000101000011111001101001011110010000011100110101"
                    "1100100110111100010111101001010010000110111011001111001001100001000111011110110101"
                    "1100110110100110011110011101011010000111001100001001101100011010001000011110010101"
                    "1110110110110000101111011001001010000111000110011101001111110101011100011100010101"
                    "1110010110101001100111110001001011000101000001000111101011011001111000011000010101"
                    "1100010110110010001001100001001011100110010000110111001110001011001111011000110101"
                    "1100010100111001000001101001011011100100110101111110001001100101111110011000100101"
                    "1100110100100000101011110001011011110110111110101000001101111100010001011100100101"
                    "1101110100100001100011001001011001110100001100011001001000011000110010011110100101"
                    "1101100100101111110101100001001001110110110011110001001110000101100100011110101101"
                    "1101100110101111000001001001001101110111111001011000101000110001011100011110101001"
                    "1101101110110111000110010001000101110101001111001111001100100101100000011100101001"
                    "1101101100101100011111000101000100110101111110011011101011100011111101011101101001"
                    "1101101000101111010000001001000110110101110011101111001001110011101111011101001001"
                    "1101001000111110111010011101000010110110110011100111001100100001001100011101001101"
                    "1101011000100101111000010001000010010100111111001011001101001110010000011101011101"
                    "1101011100100011101000110001000011010110011100001011101011101001100000011101011001"
                    "1101011110110110000110011001000111010111000011010001101001000100001000011101010001"
                    "1101001110110101110000001001000110010110010011111011101111010001100110011001010001"
                    "1101001100111110001011101001000100010100011001001110001011111011100011011001011001"
                    "1101000100100100001100001101001100010111011000010001101001011100111000011001011101"
                    "1101000110110000010011100101001110010101100111110001001111010111000111011001001101"
                    "1101000010110100101111100001001111010101000000100111101011101111011111011001101101"
                    "1101100010111011101100001101011111010100000101000010001011110111100010011101101101"
                },
        /*131*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 44, 82, 1, "3 columns x 44 rows, variant 23",
                    "1100100010100001100011001001011000010111101010111100001010011100111000011010001001"
                    "1110100010111110100010011001011100010101000011111001101001011110010000011010001101"
                    "1110110010111100010111101001011100110110111011001111001001100001000111011010000101"
                    "1100110010100110011110011101011100100111001100001001101100011010001000011011000101"
                    "1101110010110000101111011001011101100111000110011101001111110101011100011001000101"
                    "1101111010101001100111110001001101100101000001000111101011011001111000011101000101"
                    "1100111010110010001001100001000101100110010000110111001110001011001111011101100101"
                    "1110111010111001000001101001000101000100110101111110001001100101111110011001100101"
                    "1110011010100000101011110001001101000110111110101000001101111100010001011011100101"
                    "1111011010111101010111100001011101000101001110011100001101000001011000011011110101"
                    "1111001010101000011111001101011001000100101111001000001110110111100110011001110101"
                    "1110001010110111011001111001011001100100110000100011101100111010000111011101110101"
                    "1100001010111001100001001101011000100110001101000100001000110011001000011100110101"
                    "1100011010111000110011101001011000110111111010101110001101011111101111011110110101"
                    "1100010010101000001000111101010000110101101100111100001011001110011111011110010101"
                    "1110010010110010000110111001010001110111000101100111101111011101011000011100010101"
                    "1111010010100110101111110001010001100100110010111111001111101110011001011000010101"
                    "1111010110110111110101000001010011100110111110001000101110101011111100011000110101"
                    "1111010100101001110011100001010011000110100000101100001111011010000111011000100101"
                    "1110010100100101111001000001010111000111011011110011001101100111100100011100100101"
                    "1110110100100110000100011101010110000110011101000011101101111001011000011110100101"
                    "1110100100110001101000100001010010000100011001100100001100111001110011011110101101"
                    "1110100110111111010101110001011010000110101111110111101111101000100110011110101001"
                    "1110101110101101100111100001001010000101100111001111101111000101111010011100101001"
                    "1110101100111000101100111101001011000111101110101100001001100111100111011101101001"
                    "1110101000100110010111111001001011100111110111001100101100001011110110011101001001"
                    "1100101000110111110001000101011011100110001111100100101100011111001001011101001101"
                    "1100101100100001100011001001011011110111100100100111101110010000101110011101011101"
                    "1100101110111100010001101101011001110111011100111101001111100010101100011101011001"
                    "1100100110111001100101111101001001110110010100001111101011000010000111011101010001"
                    "1100110110100011000110100001001101110101100110001000001110001001000111011001010001"
                    "1110110110101101001111110001000101110111011000111101101111000101000010011001011001"
                    "1110010110100111100000100101000100110111000100111110101001100010000111011001011101"
                    "1100010110100010000100100001000110110111010011001111001000101000010000011001001101"
                    "1100010100101111000100111101000010110110001011100100001111000001010001011001101101"
                    "1100110100101110100110000001000010010101111011000011001000110011001111011101101101"
                    "1101110100110001110110010001000011010100011011100110001001100001100100011100101101"
                    "1101100100100111110001011101000111010100011110101111001110100110000100011000101101"
                    "1101100110101100110111100001000110010111001001111100101011110010001000011000101001"
                    "1101101110111011000001011001000100010101000111000111001001110001110010011001101001"
                    "1101101100101111101100111101001100010111101000100000101110100111100111011011101001"
                    "1101101000110111100001011001001110010111110100001110101001100100001110011011001001"
                    "1101001000111000100010011101001111010110000010010011001111011101000110011011001101"
                    "1101011000100100011111011001011111010111011100111100101001011110100000011011011101"
                },
        /*132*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 3, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 44, 82, 1, "3 columns x 44 rows, variant 23",
                    "1100100010100001100011001001011000010111101010111100001010011100111000011010001001"
                    "1110100010111110100010011001011100010101000011111001101001011110010000011010001101"
                    "1110110010111100010111101001011100110110111011001111001001100001000111011010000101"
                    "1100110010100110011110011101011100100111001100001001101100011010001000011011000101"
                    "1101110010110000101111011001011101100111000110011101001111110101011100011001000101"
                    "1101111010101001100111110001001101100101000001000111101011011001111000011101000101"
                    "1100111010110010001001100001000101100110010000110111001110001011001111011101100101"
                    "1110111010111001000001101001000101000100110101111110001001100101111110011001100101"
                    "1110011010100000101011110001001101000110111110101000001101111100010001011011100101"
                    "1111011010111101010111100001011101000101001110011100001101000001011000011011110101"
                    "1111001010101000011111001101011001000100101111001000001110110111100110011001110101"
                    "1110001010110111011001111001011001100100110000100011101100111010000111011101110101"
                    "1100001010111001100001001101011000100110001101000100001000110011001000011100110101"
                    "1100011010111000110011101001011000110111111010101110001101011111101111011110110101"
                    "1100010010101000001000111101010000110101101100111100001011001110011111011110010101"
                    "1110010010110010000110111001010001110111000101100111101111011101011000011100010101"
                    "1111010010100110101111110001010001100100110010111111001111101110011001011000010101"
                    "1111010110110111110101000001010011100110111110001000101110101011111100011000110101"
                    "1111010100101001110011100001010011000110100000101100001111011010000111011000100101"
                    "1110010100100101111001000001010111000111011011110011001101100111100100011100100101"
                    "1110110100100110000100011101010110000110011101000011101101111001011000011110100101"
                    "1110100100110001101000100001010010000100011001100100001100111001110011011110101101"
                    "1110100110111111010101110001011010000110101111110111101111101000100110011110101001"
                    "1110101110101101100111100001001010000101100111001111101111000101111010011100101001"
                    "1110101100111000101100111101001011000111101110101100001001100111100111011101101001"
                    "1110101000100110010111111001001011100111110111001100101100001011110110011101001001"
                    "1100101000110111110001000101011011100110001111100100101100011111001001011101001101"
                    "1100101100100001100011001001011011110111100100100111101110010000101110011101011101"
                    "1100101110111100010001101101011001110111011100111101001111100010101100011101011001"
                    "1100100110111001100101111101001001110110010100001111101011000010000111011101010001"
                    "1100110110100011000110100001001101110101100110001000001110001001000111011001010001"
                    "1110110110101101001111110001000101110111011000111101101111000101000010011001011001"
                    "1110010110100111100000100101000100110111000100111110101001100010000111011001011101"
                    "1100010110100010000100100001000110110111010011001111001000101000010000011001001101"
                    "1100010100101111000100111101000010110110001011100100001111000001010001011001101101"
                    "1100110100101110100110000001000010010101111011000011001000110011001111011101101101"
                    "1101110100110001110110010001000011010100011011100110001001100001100100011100101101"
                    "1101100100100111110001011101000111010100011110101111001110100110000100011000101101"
                    "1101100110101100110111100001000110010111001001111100101011110010001000011000101001"
                    "1101101110111011000001011001000100010101000111000111001001110001110010011001101001"
                    "1101101100101111101100111101001100010111101000100000101110100111100111011011101001"
                    "1101101000110111100001011001001110010111110100001110101001100100001110011011001001"
                    "1101001000111000100010011101001111010110000010010011001111011101000110011011001101"
                    "1101011000100100011111011001011111010111011100111100101001011110100000011011011101"
                },
        /*133*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFG", 0, 4, 99, 1, "4 columns x 4 rows, variant 24",
                    "110100111010111111010110000111111010101110001001110110110101111110111101111101000100110011010010001"
                    "110100110011111001001110010110001111100100101001110100110001111100100101100011111001001011010110001"
                    "110100010011000110100010000101101111101111001001100100110100110000111001110011100101000011010111001"
                    "110100011011111011001001000110100111000010001001100110101011111101110001110110111100110011010111101"
                },
        /*134*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFG", 0, 4, 99, 1, "4 columns x 4 rows, variant 24",
                    "110100111010111111010110000111111010101110001001110110110101111110111101111101000100110011010010001"
                    "110100110011111001001110010110001111100100101001110100110001111100100101100011111001001011010110001"
                    "110100010011000110100010000101101111101111001001100100110100110000111001110011100101000011010111001"
                    "110100011011111011001001000110100111000010001001100110101011111101110001110110111100110011010111101"
                },
        /*135*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRS", 0, 6, 99, 1, "4 columns x 6 rows, variant 25",
                    "110010001010000110001100100111101010111100001011001110101001110011100001101000001011000011001000101"
                    "111010001010100001111100110100101111001000001001001110111011011110011001101100111100100011101000101"
                    "111011001010011000010001110110011101000011101001101110101111001000000101100011111001001011101100101"
                    "110011001010010001000000100110100001000011001000101110110111011000010001110010111011111011001100101"
                    "110111001011000110011110010101101111100001001000100110111110001010110001111101100010001011011100101"
                    "110111101010001110110001110100000100101111001000110110110111100000101101111000101111001011011110101"
                },
        /*136*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRS", 0, 6, 99, 1, "4 columns x 6 rows, variant 25",
                    "110010001010000110001100100111101010111100001011001110101001110011100001101000001011000011001000101"
                    "111010001010100001111100110100101111001000001001001110111011011110011001101100111100100011101000101"
                    "111011001010011000010001110110011101000011101001101110101111001000000101100011111001001011101100101"
                    "110011001010010001000000100110100001000011001000101110110111011000010001110010111011111011001100101"
                    "110111001011000110011110010101101111100001001000100110111110001010110001111101100010001011011100101"
                    "110111101010001110110001110100000100101111001000110110110111100000101101111000101111001011011110101"
                },
        /*137*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJK", 0, 10, 99, 1, "4 columns x 10 rows, variant 27",
                    "110001001011000111110010010111010101111110001001111010101001100111110001010000010001111011000100101"
                    "111001001011110110100001110110010001001100001011111010110010000110111001110001011001111011100100101"
                    "111101001011100100000110100100110101111110001011110010100110010111111001111101110011001011110100101"
                    "111101011011011111010100000110111110001000101011110110111010101111110001010011001111100011110101101"
                    "111101010011010000010110000111101101000011101001110110110010001001100001101100001000010011110101001"
                    "111001010010111111010110000101111110101100001001110100101111110101100001011111101011000011100101001"
                    "111011010010001110111011110111110101110001001001100100100110001000011101110010111111011011101101001"
                    "111010010010001000100000010100100010000100001001100110110110000100010001110001001000111011101001001"
                    "111010011011101100000111010110111111010111001001000110100010011111011001111010110000110011101001101"
                    "111010111011001111110110010100011101000011001001000010101111100110010001011100001100111011101011101"
                },
        /*138*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJK", 0, 10, 99, 1, "4 columns x 10 rows, variant 27",
                    "110001001011000111110010010111010101111110001001111010101001100111110001010000010001111011000100101"
                    "111001001011110110100001110110010001001100001011111010110010000110111001110001011001111011100100101"
                    "111101001011100100000110100100110101111110001011110010100110010111111001111101110011001011110100101"
                    "111101011011011111010100000110111110001000101011110110111010101111110001010011001111100011110101101"
                    "111101010011010000010110000111101101000011101001110110110010001001100001101100001000010011110101001"
                    "111001010010111111010110000101111110101100001001110100101111110101100001011111101011000011100101001"
                    "111011010010001110111011110111110101110001001001100100100110001000011101110010111111011011101101001"
                    "111010010010001000100000010100100010000100001001100110110110000100010001110001001000111011101001001"
                    "111010011011101100000111010110111111010111001001000110100010011111011001111010110000110011101001101"
                    "111010111011001111110110010100011101000011001001000010101111100110010001011100001100111011101011101"
                },
        /*139*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 12, 99, 1, "4 columns x 12 rows, variant 28",
                    "111010110010000110001100100111101010111100001011000010101001110011100001101000001011000011101011001"
                    "111010100010100001111100110100101111001000001011100010111011011110011001101100111100100011101010001"
                    "110010100010011000010001110110011101000011101011100110110111100101100001000001010111100011001010001"
                    "110010110010001100110010000110011100111001101011100100111101010111100001010011100111000011001011001"
                    "110010111011111010001001100101000011111001101011101100100101111001000001110110111100110011001011101"
                    "110010011011011101100111100100110000100011101001101100110011101000011101101111001011000011001001101"
                    "110011011011000110100010000100011001100100001000101100110011100111001101111010101111000011001101101"
                    "111011011011010111111011110101111110101100001000101000111101100111001101011100100011111011101101101"
                    "111001011011101100100111110100010110001111101001101000111101011110000101110010111110001011100101101"
                    "110001011011000001000010110111010101110000001011101000111110100101111101101100011100111011000101101"
                    "110001010010001111001011110111101101100010001011001000111100111011101001111001100011001011000101001"
                    "110011010011101111110100100101011100111111001011001100100001111000101001001111101110110011001101001"
                },
        /*140*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 12, 99, 1, "4 columns x 12 rows, variant 28",
                    "111010110010000110001100100111101010111100001011000010101001110011100001101000001011000011101011001"
                    "111010100010100001111100110100101111001000001011100010111011011110011001101100111100100011101010001"
                    "110010100010011000010001110110011101000011101011100110110111100101100001000001010111100011001010001"
                    "110010110010001100110010000110011100111001101011100100111101010111100001010011100111000011001011001"
                    "110010111011111010001001100101000011111001101011101100100101111001000001110110111100110011001011101"
                    "110010011011011101100111100100110000100011101001101100110011101000011101101111001011000011001001101"
                    "110011011011000110100010000100011001100100001000101100110011100111001101111010101111000011001101101"
                    "111011011011010111111011110101111110101100001000101000111101100111001101011100100011111011101101101"
                    "111001011011101100100111110100010110001111101001101000111101011110000101110010111110001011100101101"
                    "110001011011000001000010110111010101110000001011101000111110100101111101101100011100111011000101101"
                    "110001010010001111001011110111101101100010001011001000111100111011101001111001100011001011000101001"
                    "110011010011101111110100100101011100111111001011001100100001111000101001001111101110110011001101001"
                },
        /*141*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI", 0, 15, 99, 1, "4 columns x 15 rows, variant 29",
                    "110111010010000110001100100111101010111100001011000100101001110011100001101000001011000011011101001"
                    "110110010010100001111100110100101111001000001011000110111011011110011001101100111100100011011001001"
                    "110110011010011000010001110110011101000011101010000110110111100101100001000001010111100011011001101"
                    "110110111010001100110010000110011100111001101010001110111101010111100001010011100111000011011011101"
                    "110110110011111010001001100101000011111001101010001100100101111001000001110110111100110011011011001"
                    "110110100011011101100111100100110000100011101010011100110011101000011101101111001011000011011010001"
                    "110100100011000110100010000100011001100100001010011000110011100111001101111010101111000011010010001"
                    "110101100011010111111011110111110100010011001010111000101000011111001101111110110100110011010110001"
                    "110101110011000111110010010110001111100100101010110000110001111100100101100011111001001011010111001"
                    "110101111010000110001100100100001100011001001010010000100001100011001001110110100110000011010111101"
                    "110100111010111100101111000100111110100111001011010000111001101110100001011001111100010011010011101"
                    "110100110010000110010011100111001011111000101001010000110011100100111001110011111101001011010011001"
                    "110100010011000111011001000110100110000111001001011000111001000010111001111001101000111011010001001"
                    "110100011010111010000111110110001000111001001001011100111101100000110101100100011101000011010001101"
                    "110100001010011110100100000100111110011010001011011100111011111101001001011101100011100011010000101"
                },
        /*142*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI", 0, 15, 99, 1, "4 columns x 15 rows, variant 29",
                    "110111010010000110001100100111101010111100001011000100101001110011100001101000001011000011011101001"
                    "110110010010100001111100110100101111001000001011000110111011011110011001101100111100100011011001001"
                    "110110011010011000010001110110011101000011101010000110110111100101100001000001010111100011011001101"
                    "110110111010001100110010000110011100111001101010001110111101010111100001010011100111000011011011101"
                    "110110110011111010001001100101000011111001101010001100100101111001000001110110111100110011011011001"
                    "110110100011011101100111100100110000100011101010011100110011101000011101101111001011000011011010001"
                    "110100100011000110100010000100011001100100001010011000110011100111001101111010101111000011010010001"
                    "110101100011010111111011110111110100010011001010111000101000011111001101111110110100110011010110001"
                    "110101110011000111110010010110001111100100101010110000110001111100100101100011111001001011010111001"
                    "110101111010000110001100100100001100011001001010010000100001100011001001110110100110000011010111101"
                    "110100111010111100101111000100111110100111001011010000111001101110100001011001111100010011010011101"
                    "110100110010000110010011100111001011111000101001010000110011100100111001110011111101001011010011001"
                    "110100010011000111011001000110100110000111001001011000111001000010111001111001101000111011010001001"
                    "110100011010111010000111110110001000111001001001011100111101100000110101100100011101000011010001101"
                    "110100001010011110100100000100111110011010001011011100111011111101001001011101100011100011010000101"
                },
        /*143*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 20, 99, 1, "4 columns x 20 rows, variant 30",
                    "110010001010000110001100100111101010111100001011110010101001110011100001101000001011000011100101101"
                    "111010001010100001111100110100101111001000001011110110111011011110011001101100111100100011000101101"
                    "111011001010011000010001110110011101000011101001110110110111100101100001000001010111100011000101001"
                    "110011001010001100110010000110011100111001101001110100111101010111100001010011100111000011001101001"
                    "110111001011111010001001100101000011111001101001100100100101111001000001110110111100110011011101001"
                    "110111101011011101100111100100110000100011101001100110110011101000011101101111001011000011011001001"
                    "110011101011000110100010000100011001100100001001000110110011100111001101111010101111000011011001101"
                    "111011101011010111111011110111110100010011001001000010101000011111001101001011110010000011011011101"
                    "111001101011110001011110100110111011001111001011000010100110000100011101100111010000111011011011001"
                    "111101101011100110000100110110001101000100001011100010100011001100100001100111001110011011011010001"
                    "111100101010111111010110000101111110101100001011100110101111110101100001011111101011000011010010001"
                    "111000101011000111110010010110001111100100101011100100110001111100100101100011111001001011010110001"
                    "110000101010000110001100100100001100011001001011101100100001100011001001000011000110010011010111001"
                    "110001101010111111010110000101111110101100001001101100111110110010010001101111101001111011010111101"
                    "110001001011111101011100110111100010111100101000101100100011011011110001010010000011110011010011101"
                    "111001001010000100001100110110011100011010001000101000110111101111001101100001001100111011010011001"
                    "111101001010111101000011110111100011001101001001101000110100001110000101101111110100111011010001001"
                    "111101011010111100000110110100110001100111101011101000101100010000011101000001111011011011010001101"
                    "111101010011001100001010000100010001000000101011001000100001100110100001110111001000100011010000101"
                    "111001010011100101100100000111000110111000101011001100111110010010001101110001011101100011011000101"
                },
        /*144*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 20, 99, 1, "4 columns x 20 rows, variant 30",
                    "110010001010000110001100100111101010111100001011110010101001110011100001101000001011000011100101101"
                    "111010001010100001111100110100101111001000001011110110111011011110011001101100111100100011000101101"
                    "111011001010011000010001110110011101000011101001110110110111100101100001000001010111100011000101001"
                    "110011001010001100110010000110011100111001101001110100111101010111100001010011100111000011001101001"
                    "110111001011111010001001100101000011111001101001100100100101111001000001110110111100110011011101001"
                    "110111101011011101100111100100110000100011101001100110110011101000011101101111001011000011011001001"
                    "110011101011000110100010000100011001100100001001000110110011100111001101111010101111000011011001101"
                    "111011101011010111111011110111110100010011001001000010101000011111001101001011110010000011011011101"
                    "111001101011110001011110100110111011001111001011000010100110000100011101100111010000111011011011001"
                    "111101101011100110000100110110001101000100001011100010100011001100100001100111001110011011011010001"
                    "111100101010111111010110000101111110101100001011100110101111110101100001011111101011000011010010001"
                    "111000101011000111110010010110001111100100101011100100110001111100100101100011111001001011010110001"
                    "110000101010000110001100100100001100011001001011101100100001100011001001000011000110010011010111001"
                    "110001101010111111010110000101111110101100001001101100111110110010010001101111101001111011010111101"
                    "110001001011111101011100110111100010111100101000101100100011011011110001010010000011110011010011101"
                    "111001001010000100001100110110011100011010001000101000110111101111001101100001001100111011010011001"
                    "111101001010111101000011110111100011001101001001101000110100001110000101101111110100111011010001001"
                    "111101011010111100000110110100110001100111101011101000101100010000011101000001111011011011010001101"
                    "111101010011001100001010000100010001000000101011001000100001100110100001110111001000100011010000101"
                    "111001010011100101100100000111000110111000101011001100111110010010001101110001011101100011011000101"
                },
        /*145*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 26, 99, 1, "4 columns x 26 rows, variant 31",
                    "110010001010000110001100100111101010111100001000011010101001110011100001101000001011000011110100101"
                    "111010001010100001111100110100101111001000001000111010111011011110011001101100111100100011110101101"
                    "111011001010011000010001110110011101000011101000110010110111100101100001000001010111100011110101001"
                    "110011001010001100110010000110011100111001101000100010111101010111100001010011100111000011100101001"
                    "110111001011111010001001100101000011111001101001100010100101111001000001110110111100110011101101001"
                    "110111101011011101100111100100110000100011101001110010110011101000011101101111001011000011101001001"
                    "110011101011000110100010000100011001100100001001111010110011100111001101111010101111000011101001101"
                    "111011101011010111111011110111110100010011001011111010101000011111001101001011110010000011101011101"
                    "111001101011110001011110100110111011001111001011110010100110000100011101100111010000111011101011001"
                    "111101101011100110000100110110001101000100001011110110100011001100100001100111001110011011101010001"
                    "111100101011111101010111000110101111110111101001110110111110100010011001010000111110011011001010001"
                    "111000101010110011100111110111100010111101001001110100110111011001111001001100001000111011001011001"
                    "110000101010011001111001110111001100001001101001100100110001101000100001000110011001000011001011101"
                    "110001101011100011001110100111111010101110001001100110110101111110111101111101000100110011001001101"
                    "110001001010110110011110000101100111001111101001000110111100010111101001101110110011110011001101101"
                    "111001001011110111010110000100110011110011101001000010111001100001001101100011010001000011101101101"
                    "111101001011000010111101100111000110011101001011000010101111110101100001011111101011000011100101101"
                    "111101011011000111110010010110001111100100101011100010110001111100100101100011111001001011000101101"
                    "111101010011000111011000010101100001110011001011100110101100111011000001000010010000100011000101001"
                    "111001010011100100111000110110000100011100101011100100111100101010000001001111100100111011001101001"
                    "111011010010110100011100000101101000000111001011101100100000110010011101100111110001010011011101001"
                    "111010010010010000110000110110000100010110001001101100100001000110011001010011101110000011011001001"
                    "111010011011110001100110100110001000011100101000101100110000010111010001111101100110011011011001101"
                    "111010111010011111001100100100010100001111001000101000100111100000101001001111100111011011011011101"
                    "111010110011000110000100100100100001001000001001101000100000100011001101000011001100001011011011001"
                    "111010100011110100010010000111000001101110101011101000111001001110011001101111101001111011011010001"
                },
        /*146*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 26, 99, 1, "4 columns x 26 rows, variant 31",
                    "110010001010000110001100100111101010111100001000011010101001110011100001101000001011000011110100101"
                    "111010001010100001111100110100101111001000001000111010111011011110011001101100111100100011110101101"
                    "111011001010011000010001110110011101000011101000110010110111100101100001000001010111100011110101001"
                    "110011001010001100110010000110011100111001101000100010111101010111100001010011100111000011100101001"
                    "110111001011111010001001100101000011111001101001100010100101111001000001110110111100110011101101001"
                    "110111101011011101100111100100110000100011101001110010110011101000011101101111001011000011101001001"
                    "110011101011000110100010000100011001100100001001111010110011100111001101111010101111000011101001101"
                    "111011101011010111111011110111110100010011001011111010101000011111001101001011110010000011101011101"
                    "111001101011110001011110100110111011001111001011110010100110000100011101100111010000111011101011001"
                    "111101101011100110000100110110001101000100001011110110100011001100100001100111001110011011101010001"
                    "111100101011111101010111000110101111110111101001110110111110100010011001010000111110011011001010001"
                    "111000101010110011100111110111100010111101001001110100110111011001111001001100001000111011001011001"
                    "110000101010011001111001110111001100001001101001100100110001101000100001000110011001000011001011101"
                    "110001101011100011001110100111111010101110001001100110110101111110111101111101000100110011001001101"
                    "110001001010110110011110000101100111001111101001000110111100010111101001101110110011110011001101101"
                    "111001001011110111010110000100110011110011101001000010111001100001001101100011010001000011101101101"
                    "111101001011000010111101100111000110011101001011000010101111110101100001011111101011000011100101101"
                    "111101011011000111110010010110001111100100101011100010110001111100100101100011111001001011000101101"
                    "111101010011000111011000010101100001110011001011100110101100111011000001000010010000100011000101001"
                    "111001010011100100111000110110000100011100101011100100111100101010000001001111100100111011001101001"
                    "111011010010110100011100000101101000000111001011101100100000110010011101100111110001010011011101001"
                    "111010010010010000110000110110000100010110001001101100100001000110011001010011101110000011011001001"
                    "111010011011110001100110100110001000011100101000101100110000010111010001111101100110011011011001101"
                    "111010111010011111001100100100010100001111001000101000100111100000101001001111100111011011011011101"
                    "111010110011000110000100100100100001001000001001101000100000100011001101000011001100001011011011001"
                    "111010100011110100010010000111000001101110101011101000111001001110011001101111101001111011011010001"
                },
        /*147*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 99, 1, "4 columns x 32 rows, variant 32",
                    "111011010011000111110010010111010101111110001011101100101001100111110001010000010001111011011101001"
                    "111010010011110110100001110110010001001100001001101100110010000110111001110001011001111011011001001"
                    "111010011011100100000110100100110101111110001000101100100110010111111001111101110011001011011001101"
                    "111010111011011111010100000110111110001000101000101000111010101111110001010011001111100011011011101"
                    "111010110011010000010110000111101101000011101001101000110010001001100001100100001101110011011011001"
                    "111010100011011001111001000111001000001101001011101000100110101111110001001100101111110011011010001"
                    "110010100010000010101111000110111110101000001011001000110111110001000101110101011111100011010010001"
                    "110010110010100111001110000110100000101100001011001100111101101000011101100100010011000011010110001"
                    "110010111011101101111001100110110011110010001011000100111001000001101001001101011111100011010111001"
                    "110010011011011110010110000100000101011110001011000110110111110101000001101111100010001011010111101"
                    "110011011011110101011110000101001110011100001010000110110100000101100001111011010000111011010011101"
                    "111011011010010111100100000111011011110011001010001110110110011110010001110010000011010011010011001"
                    "111001011011001110100001110110111100101100001010001100100000101011110001101111101010000011010001001"
                    "110001011011001110011100110111101010111100001010011100101001110011100001101000001011000011010001101"
                    "110001010010100001111100110100101111001000001010011000111011011110011001101100111100100011010000101"
                    "110011010010011000010001110110011101000011101010111000110111100101100001000001010111100011011000101"
                    "110111010010001100110010000110011100111001101010110000111101010111100001010011100111000011001000101"
                    "110110010011111010001001100101000011111001101010010000100101111001000001110110111100110011101000101"
                    "110110011011011101100111100100110000100011101011010000110011101000011101101111001011000011101100101"
                    "110110111011000110100010000100011001100100001001010000110011100111001101000011000110010011001100101"
                    "110110110010111111010110000101111110101100001001011000101111110101100001011111101011000011011100101"
                    "110110100011000111110010010110001111100100101001011100110001111100100101100011111001001011011110101"
                    "110100100010000110001100100100001100011001001011011100110111100111000101100001100010100011001110101"
                    "110101100011111010000101100111001000001101001011011110111100011011010001111101010001100011101110101"
                    "110101110010000001110010110101110100001100001011001110101001001111000001111110101100100011100110101"
                    "110101111011000010110011100101101111100111101001001110110011000100001001001101110110000011110110101"
                    "110100111011110100011011000100101111100110001001101110100100111100001001111000100100100011110010101"
                    "110100110011100111101001110111100101111101101000101110100000100100111101111110101100010011100010101"
                    "110100010010011000011010000100001100011001001000100110100110000110100001101100100100000011000010101"
                    "110100011010110111111000110111110000101011001000110110111010001100000101111000110110001011000110101"
                    "110100001010110000100001110101110000011011101000010110101110000010110001101111110011010011000100101"
                    "110110001010010000001001000110011101100001001000010010111110010101111101110001000100111011100100101"
                },
        /*148*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 32, 99, 1, "4 columns x 32 rows, variant 32",
                    "111011010011000111110010010111010101111110001011101100101001100111110001010000010001111011011101001"
                    "111010010011110110100001110110010001001100001001101100110010000110111001110001011001111011011001001"
                    "111010011011100100000110100100110101111110001000101100100110010111111001111101110011001011011001101"
                    "111010111011011111010100000110111110001000101000101000111010101111110001010011001111100011011011101"
                    "111010110011010000010110000111101101000011101001101000110010001001100001100100001101110011011011001"
                    "111010100011011001111001000111001000001101001011101000100110101111110001001100101111110011011010001"
                    "110010100010000010101111000110111110101000001011001000110111110001000101110101011111100011010010001"
                    "110010110010100111001110000110100000101100001011001100111101101000011101100100010011000011010110001"
                    "110010111011101101111001100110110011110010001011000100111001000001101001001101011111100011010111001"
                    "110010011011011110010110000100000101011110001011000110110111110101000001101111100010001011010111101"
                    "110011011011110101011110000101001110011100001010000110110100000101100001111011010000111011010011101"
                    "111011011010010111100100000111011011110011001010001110110110011110010001110010000011010011010011001"
                    "111001011011001110100001110110111100101100001010001100100000101011110001101111101010000011010001001"
                    "110001011011001110011100110111101010111100001010011100101001110011100001101000001011000011010001101"
                    "110001010010100001111100110100101111001000001010011000111011011110011001101100111100100011010000101"
                    "110011010010011000010001110110011101000011101010111000110111100101100001000001010111100011011000101"
                    "110111010010001100110010000110011100111001101010110000111101010111100001010011100111000011001000101"
                    "110110010011111010001001100101000011111001101010010000100101111001000001110110111100110011101000101"
                    "110110011011011101100111100100110000100011101011010000110011101000011101101111001011000011101100101"
                    "110110111011000110100010000100011001100100001001010000110011100111001101000011000110010011001100101"
                    "110110110010111111010110000101111110101100001001011000101111110101100001011111101011000011011100101"
                    "110110100011000111110010010110001111100100101001011100110001111100100101100011111001001011011110101"
                    "110100100010000110001100100100001100011001001011011100110111100111000101100001100010100011001110101"
                    "110101100011111010000101100111001000001101001011011110111100011011010001111101010001100011101110101"
                    "110101110010000001110010110101110100001100001011001110101001001111000001111110101100100011100110101"
                    "110101111011000010110011100101101111100111101001001110110011000100001001001101110110000011110110101"
                    "110100111011110100011011000100101111100110001001101110100100111100001001111000100100100011110010101"
                    "110100110011100111101001110111100101111101101000101110100000100100111101111110101100010011100010101"
                    "110100010010011000011010000100001100011001001000100110100110000110100001101100100100000011000010101"
                    "110100011010110111111000110111110000101011001000110110111010001100000101111000110110001011000110101"
                    "110100001010110000100001110101110000011011101000010110101110000010110001101111110011010011000100101"
                    "110110001010010000001001000110011101100001001000010010111110010101111101110001000100111011100100101"
                },
        /*149*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 38, 99, 1, "4 columns x 38 rows, variant 33",
                    "110001001011000111110010010111010101111110001000101100101001100111110001010000010001111011010011101"
                    "111001001011110110100001110110010001001100001000101000110010000110111001110001011001111011010011001"
                    "111101001011100100000110100100110101111110001001101000100110010111111001111101110011001011010001001"
                    "111101011011011111010100000110111110001000101011101000111010101111110001010011001111100011010001101"
                    "111101010011010000010110000111101101000011101011001000110010001001100001100100001101110011010000101"
                    "111001010011011001111001000111001000001101001011001100100110101111110001001100101111110011011000101"
                    "111011010010000010101111000110111110101000001011000100110111110001000101110101011111100011001000101"
                    "111010010010100111001110000110100000101100001011000110111101101000011101100100010011000011101000101"
                    "111010011011101101111001100110110011110010001010000110111001000001101001001101011111100011101100101"
                    "111010111011011110010110000100000101011110001010001110110111110101000001101111100010001011001100101"
                    "111010110011110101011110000101001110011100001010001100110100000101100001111011010000111011011100101"
                    "111010100010010111100100000111011011110011001010011100110110011110010001110010000011010011011110101"
                    "110010100011001110100001110110111100101100001010011000100000101011110001101111101010000011001110101"
                    "110010110011001110011100110111101010111100001010111000101001110011100001101000001011000011101110101"
                    "110010111010100001111100110100101111001000001010110000111011011110011001101100111100100011100110101"
                    "110010011010011000010001110110011101000011101010010000110111100101100001000001010111100011110110101"
                    "110011011010001100110010000110011100111001101011010000111101010111100001010011100111000011110010101"
                    "111011011011111010001001100101000011111001101001010000100101111001000001110110111100110011100010101"
                    "111001011011011101100111100100110000100011101001011000110011101000011101101111001011000011000010101"
                    "110001011011000110100010000100011001100100001001011100110011100111001101111010101111000011000110101"
                    "110001010011010111111011110111110100010011001011011100101000011111001101001011110010000011000100101"
                    "110011010011110001011110100110111011001111001011011110100110000100011101100111010000111011100100101"
                    "110111010011100110000100110110001101000100001011001110100011001100100001100111001110011011110100101"
                    "110110010010111111010110000101111110101100001001001110101111110101100001011111101011000011110101101"
                    "110110011011000111110010010110001111100100101001101110110001111100100101100011111001001011110101001"
                    "110110111010000110001100100100001100011001001000101110100001100011001001000011000110010011100101001"
                    "110110110010111111010110000101111110101100001000100110101111110101100001011111101011000011101101001"
                    "110110100011011111101101000100001100101110001000110110110110010111100001001111011000110011101001001"
                    "110100100011100011101000100110000010001001101000010110111101111010000101000001000011011011101001101"
                    "110101100011100010001110110110110001111000101000010010101111101000011101001000011110010011101011101"
                    "110101110011010100001111100101111101110011001000011010101110000010110001011000100000111011101011001"
                    "110101111010000100010100000110001101100001101000111010111101110100001101011101111001100011101010001"
                    "110100111011101001110000110100100111101000001000110010111111001010111001110100000011010011001010001"
                    "110100110010001111010000100111111010011000101000100010101001000011110001011110010001000011001011001"
                    "110100010011110111101001000110011110111000101001100010110011001010000001101110110000100011001011101"
                    "110100011011001011110000110100110001111100101001110010101110011011111101001011110100000011001001101"
                    "110100001011001111001011000110011111010001001001111010100100101111000001011100000100011011001101101"
                    "110110001011011101111001110111101101100111101011111010110100011001110001100110100001000011101101101"
                },
        /*150*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 38, 99, 1, "4 columns x 38 rows, variant 33",
                    "110001001011000111110010010111010101111110001000101100101001100111110001010000010001111011010011101"
                    "111001001011110110100001110110010001001100001000101000110010000110111001110001011001111011010011001"
                    "111101001011100100000110100100110101111110001001101000100110010111111001111101110011001011010001001"
                    "111101011011011111010100000110111110001000101011101000111010101111110001010011001111100011010001101"
                    "111101010011010000010110000111101101000011101011001000110010001001100001100100001101110011010000101"
                    "111001010011011001111001000111001000001101001011001100100110101111110001001100101111110011011000101"
                    "111011010010000010101111000110111110101000001011000100110111110001000101110101011111100011001000101"
                    "111010010010100111001110000110100000101100001011000110111101101000011101100100010011000011101000101"
                    "111010011011101101111001100110110011110010001010000110111001000001101001001101011111100011101100101"
                    "111010111011011110010110000100000101011110001010001110110111110101000001101111100010001011001100101"
                    "111010110011110101011110000101001110011100001010001100110100000101100001111011010000111011011100101"
                    "111010100010010111100100000111011011110011001010011100110110011110010001110010000011010011011110101"
                    "110010100011001110100001110110111100101100001010011000100000101011110001101111101010000011001110101"
                    "110010110011001110011100110111101010111100001010111000101001110011100001101000001011000011101110101"
                    "110010111010100001111100110100101111001000001010110000111011011110011001101100111100100011100110101"
                    "110010011010011000010001110110011101000011101010010000110111100101100001000001010111100011110110101"
                    "110011011010001100110010000110011100111001101011010000111101010111100001010011100111000011110010101"
                    "111011011011111010001001100101000011111001101001010000100101111001000001110110111100110011100010101"
                    "111001011011011101100111100100110000100011101001011000110011101000011101101111001011000011000010101"
                    "110001011011000110100010000100011001100100001001011100110011100111001101111010101111000011000110101"
                    "110001010011010111111011110111110100010011001011011100101000011111001101001011110010000011000100101"
                    "110011010011110001011110100110111011001111001011011110100110000100011101100111010000111011100100101"
                    "110111010011100110000100110110001101000100001011001110100011001100100001100111001110011011110100101"
                    "110110010010111111010110000101111110101100001001001110101111110101100001011111101011000011110101101"
                    "110110011011000111110010010110001111100100101001101110110001111100100101100011111001001011110101001"
                    "110110111010000110001100100100001100011001001000101110100001100011001001000011000110010011100101001"
                    "110110110010111111010110000101111110101100001000100110101111110101100001011111101011000011101101001"
                    "110110100011011111101101000100001100101110001000110110110110010111100001001111011000110011101001001"
                    "110100100011100011101000100110000010001001101000010110111101111010000101000001000011011011101001101"
                    "110101100011100010001110110110110001111000101000010010101111101000011101001000011110010011101011101"
                    "110101110011010100001111100101111101110011001000011010101110000010110001011000100000111011101011001"
                    "110101111010000100010100000110001101100001101000111010111101110100001101011101111001100011101010001"
                    "110100111011101001110000110100100111101000001000110010111111001010111001110100000011010011001010001"
                    "110100110010001111010000100111111010011000101000100010101001000011110001011110010001000011001011001"
                    "110100010011110111101001000110011110111000101001100010110011001010000001101110110000100011001011101"
                    "110100011011001011110000110100110001111100101001110010101110011011111101001011110100000011001001101"
                    "110100001011001111001011000110011111010001001001111010100100101111000001011100000100011011001101101"
                    "110110001011011101111001110111101101100111101011111010110100011001110001100110100001000011101101101"
                },
        /*151*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE | FAST_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 44, 99, 1, "4 columns x 44 rows, variant 34",
                    "110010001010000110001100100111101010111100001011000010101001110011100001101000001011000011010001001"
                    "111010001010100001111100110100101111001000001011100010111011011110011001101100111100100011010001101"
                    "111011001010011000010001110110011101000011101011100110110111100101100001000001010111100011010000101"
                    "110011001010001100110010000110011100111001101011100100111101010111100001010011100111000011011000101"
                    "110111001011111010001001100101000011111001101011101100100101111001000001110110111100110011001000101"
                    "110111101011011101100111100100110000100011101001101100110011101000011101101111001011000011101000101"
                    "110011101011000110100010000100011001100100001000101100110011100111001101111010101111000011101100101"
                    "111011101011010111111011110111110100010011001000101000101000011111001101001011110010000011001100101"
                    "111001101011110001011110100110111011001111001001101000100110000100011101100111010000111011011100101"
                    "111101101011100110000100110110001101000100001011101000100011001100100001100111001110011011011110101"
                    "111100101011111101010111000110101111110111101011001000111110100010011001010000111110011011001110101"
                    "111000101010110011100111110111100010111101001011001100110111011001111001001100001000111011101110101"
                    "110000101010011001111001110111001100001001101011000100110001101000100001000110011001000011100110101"
                    "110001101011100011001110100111111010101110001011000110110101111110111101111101000100110011110110101"
                    "110001001010110110011110000101100111001111101010000110111100010111101001101110110011110011110010101"
                    "111001001011110111010110000100110011110011101010001110111001100001001101100011010001000011100010101"
                    "111101001011000010111101100111000110011101001010001100111111010101110001101011111101111011000010101"
                    "111101011010100000100011110101101100111100001010011100101100111001111101111000101111010011000110101"
                    "111101010011100010110011110111101110101100001010011000100110011110011101110011000010011011000100101"
                    "111001010011111011100110010110000101111011001010111000111000110011101001111110101011100011100100101"
                    "111011010010100110011111000101000001000111101010110000101101100111100001011001110011111011110100101"
                    "111010010011001000011011100111000101100111101010010000111101110101100001001100111100111011110101101"
                    "111010011010011001011111100111110111001100101011010000110000101111011001110001100111010011110101001"
                    "111010111011101010111111000101001100111110001001010000101000001000111101011011001111000011100101001"
                    "111010110011001000100110000110010000110111001001011000111000101100111101111011101011000011101101001"
                    "111010100010011010111111000100110010111111001001011100111110111001100101100001011110110011101001001"
                    "110010100011011111000100010111010101111110001011011100101001100111110001010000010001111011101001101"
                    "110010110011110110100001110110010001001100001011011110110010000110111001110001011001111011101011101"
                    "110010111011100100000110100100110101111110001011001110100110010111111001111101110011001011101011001"
                    "110010011011011111010100000110111110001000101001001110110001111100100101100011111001001011101010001"
                    "110011011010000110001100100100001100011001001001101110100001100011001001000011000110010011001010001"
                    "111011011010111111010110000101111110101100001000101110111000100001101001111011110111101011001011001"
                    "111001011010101100011111000101111000010010001000100110101000100011110001101110110011110011001011101"
                    "110001011010001011000110000111001000110111101000110110111101010111100001011000011100110011001001101"
                    "110001010011001001110100000111100010100000101000010110111100111011101001111001010000001011001101101"
                    "110011010010011110110001100100011101101110001000010010100101000001111001111010111110110011101101101"
                    "110111010011100011001011000110000110110011001000011010101111001111010001100001001100111011100101101"
                    "110110010011110001000110110111101001000010001000111010110001001110100001011111010000111011000101101"
                    "110110011010001101001110000101110000010001101000110010100111000110011101000011010000111011000101001"
                    "110110111010000010100010000110001110110000101000100010110110110110000001000011000110100011001101001"
                    "110110110011110001100110010111101001110111001001100010111000100110100001001110100111110011011101001"
                    "110110100011000101101111110100100001101111101001110010110000011101011101001011011111000011011001001"
                    "110100100011100011101100110111010111001111101001111010100001000110110001110100110111100011011001101"
                    "110101100011001000011101000111110100101100001011111010110101111101110001101101111010000011011011101"
                },
        /*152*/ { BARCODE_MICROPDF417, -1, UNICODE_MODE, -1, 4, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 44, 99, 1, "4 columns x 44 rows, variant 34",
                    "110010001010000110001100100111101010111100001011000010101001110011100001101000001011000011010001001"
                    "111010001010100001111100110100101111001000001011100010111011011110011001101100111100100011010001101"
                    "111011001010011000010001110110011101000011101011100110110111100101100001000001010111100011010000101"
                    "110011001010001100110010000110011100111001101011100100111101010111100001010011100111000011011000101"
                    "110111001011111010001001100101000011111001101011101100100101111001000001110110111100110011001000101"
                    "110111101011011101100111100100110000100011101001101100110011101000011101101111001011000011101000101"
                    "110011101011000110100010000100011001100100001000101100110011100111001101111010101111000011101100101"
                    "111011101011010111111011110111110100010011001000101000101000011111001101001011110010000011001100101"
                    "111001101011110001011110100110111011001111001001101000100110000100011101100111010000111011011100101"
                    "111101101011100110000100110110001101000100001011101000100011001100100001100111001110011011011110101"
                    "111100101011111101010111000110101111110111101011001000111110100010011001010000111110011011001110101"
                    "111000101010110011100111110111100010111101001011001100110111011001111001001100001000111011101110101"
                    "110000101010011001111001110111001100001001101011000100110001101000100001000110011001000011100110101"
                    "110001101011100011001110100111111010101110001011000110110101111110111101111101000100110011110110101"
                    "110001001010110110011110000101100111001111101010000110111100010111101001101110110011110011110010101"
                    "111001001011110111010110000100110011110011101010001110111001100001001101100011010001000011100010101"
                    "111101001011000010111101100111000110011101001010001100111111010101110001101011111101111011000010101"
                    "111101011010100000100011110101101100111100001010011100101100111001111101111000101111010011000110101"
                    "111101010011100010110011110111101110101100001010011000100110011110011101110011000010011011000100101"
                    "111001010011111011100110010110000101111011001010111000111000110011101001111110101011100011100100101"
                    "111011010010100110011111000101000001000111101010110000101101100111100001011001110011111011110100101"
                    "111010010011001000011011100111000101100111101010010000111101110101100001001100111100111011110101101"
                    "111010011010011001011111100111110111001100101011010000110000101111011001110001100111010011110101001"
                    "111010111011101010111111000101001100111110001001010000101000001000111101011011001111000011100101001"
                    "111010110011001000100110000110010000110111001001011000111000101100111101111011101011000011101101001"
                    "111010100010011010111111000100110010111111001001011100111110111001100101100001011110110011101001001"
                    "110010100011011111000100010111010101111110001011011100101001100111110001010000010001111011101001101"
                    "110010110011110110100001110110010001001100001011011110110010000110111001110001011001111011101011101"
                    "110010111011100100000110100100110101111110001011001110100110010111111001111101110011001011101011001"
                    "110010011011011111010100000110111110001000101001001110110001111100100101100011111001001011101010001"
                    "110011011010000110001100100100001100011001001001101110100001100011001001000011000110010011001010001"
                    "111011011010111111010110000101111110101100001000101110111000100001101001111011110111101011001011001"
                    "111001011010101100011111000101111000010010001000100110101000100011110001101110110011110011001011101"
                    "110001011010001011000110000111001000110111101000110110111101010111100001011000011100110011001001101"
                    "110001010011001001110100000111100010100000101000010110111100111011101001111001010000001011001101101"
                    "110011010010011110110001100100011101101110001000010010100101000001111001111010111110110011101101101"
                    "110111010011100011001011000110000110110011001000011010101111001111010001100001001100111011100101101"
                    "110110010011110001000110110111101001000010001000111010110001001110100001011111010000111011000101101"
                    "110110011010001101001110000101110000010001101000110010100111000110011101000011010000111011000101001"
                    "110110111010000010100010000110001110110000101000100010110110110110000001000011000110100011001101001"
                    "110110110011110001100110010111101001110111001001100010111000100110100001001110100111110011011101001"
                    "110110100011000101101111110100100001101111101001110010110000011101011101001011011111000011011001001"
                    "110100100011100011101100110111010111001111101001111010100001000110110001110100110111100011011001101"
                    "110101100011001000011101000111110100101100001011111010110101111101110001101101111010000011011011101"
                },
        /*153*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "123\035", 0, 7, 103, 1, "MR #151 NUM BYTE1",
                    "1111111101010100011111010101111100110101000110000001000001000010001011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111010111100011101010111100001000011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101001001111000001010111000111111011010100011111000111111101000101001"
                    "1111111101010100011010111100111110110011110111000101100001011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111110110001001001000100001111001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110001100101111001000111100010010011110101111000010111111101000101001"
                    "1111111101010100011101001110111110100100001100001101100011000010100011010011101111000111111101000101001"
                },
        /*154*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "123\035", 0, 7, 103, 0, "MR #151 NUM BYTE1; BWIPP same as FAST_MODE",
                    "1111111101010100011111010101111100110101000110000001000001001100011011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110101111110111101011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101011100011111101100011111001001011010100011111000111111101000101001"
                    "1111111101010100011010111100111110101100011000001001111011000010111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111100110011010001010011111100111011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111110010011101001011101001100000011110101111000010111111101000101001"
                    "1111111101010100011101001110111110100110001100000101110111101101000011010011101111000111111101000101001"
                },
        /*155*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "+123456789012", 0, 8, 103, 1, "",
                    "1111111101010100011111010101111100110101000011000001000001000100100011110101011110000111111101000101001"
                    "1111111101010100011111101010001110111101011100111001110100111001100011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111110010111101101110010011111001011101010001111110111111101000101001"
                    "1111111101010100011010111100111110110010001100011101101011110011111010101111001111000111111101000101001"
                    "1111111101010100011101011100001100110000100111000101011111100100011011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110101111110001001100111110001001011101011111010000111111101000101001"
                    "1111111101010100011101001110111110110000110110110001001101111101111011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111111011101101101111110100010111010101111110111000111111101000101001"
                },
        /*156*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "+123456789012", 0, 8, 103, 1, "",
                    "1111111101010100011111010101111100110101000011000001000001000100100011110101011110000111111101000101001"
                    "1111111101010100011111101010001110111101011100111001110100111001100011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111110010111101101110010011111001011101010001111110111111101000101001"
                    "1111111101010100011010111100111110110010001100011101101011110011111010101111001111000111111101000101001"
                    "1111111101010100011101011100001100110000100111000101011111100100011011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110101111110001001100111110001001011101011111010000111111101000101001"
                    "1111111101010100011101001110111110110000110110110001001101111101111011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111111011101101101111110100010111010101111110111000111111101000101001"
                },
        /*157*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "+1234567890123", 0, 8, 103, 0, "BWIPP different encodation",
                    "1111111101010100011111010101111100110101000011000001000001000100100011110101011110000111111101000101001"
                    "1111111101010100011111101010001110101111110100110001111010110011000011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000010111100001011111101110100011101010001111110111111101000101001"
                    "1111111101010100011010111100111110100000101101100001001001110011100010101111001111000111111101000101001"
                    "1111111101010100011101011100001100111111001010111001001101111100001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100000111100100101011100100000011011101011111010000111111101000101001"
                    "1111111101010100011101001110111110110110000100001001110100100111000011010011101111000111111101000101001"
                    "1111111101010100011111010010110000100001011110000101000111110101110010101111110111000111111101000101001"
                },
        /*158*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "+1234567890123", 0, 8, 103, 0, "BWIPP different encodation",
                    "1111111101010100011111010101111100110101000011000001000001000100100011110101011110000111111101000101001"
                    "1111111101010100011111101010001110101111110100110001111010110011000011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000010111100001011111101110100011101010001111110111111101000101001"
                    "1111111101010100011010111100111110100000101101100001001001110011100010101111001111000111111101000101001"
                    "1111111101010100011101011100001100111111001010111001001101111100001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100000111100100101011100100000011011101011111010000111111101000101001"
                    "1111111101010100011101001110111110110110000100001001110100100111000011010011101111000111111101000101001"
                    "1111111101010100011111010010110000100001011110000101000111110101110010101111110111000111111101000101001"
                },
        /*159*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "90044030118100801265*D_2D+1.02+31351440315981", 0, 11, 120, 0, "BWIPP different encodation",
                    "111111110101010001110101001110000011010111001111000100111101111010001110001010111000011111010101111100111111101000101001"
                    "111111110101010001111110101000111010011011111000100111100101010000001001001111110111011111010100110000111111101000101001"
                    "111111110101010001010100111100000010011111000011010100001100101110001100000110101111011101010001111110111111101000101001"
                    "111111110101010001010111110111110010000110001100100110011110111001001101000011000111011010111100111110111111101000101001"
                    "111111110101010001110101110000110011100101110011000111010011000001001010011111000011011010111000100000111111101000101001"
                    "111111110101010001111101011110110010111100011000110100001000101111001110100111110010011101011111010000111111101000101001"
                    "111111110101010001010011100111000010011110111101000101101100010000001111011010011100011101001110111110111111101000101001"
                    "111111110101010001111101001011000011111011010001000110011111010111101000101111010000011010111111011110111111101000101001"
                    "111111110101010001111110100110010011000111110010010111101011110100001101111100100001010100110000111110111111101000101001"
                    "111111110101010001010001100000110010001110111001000101111000111100101100110011100111010100011000011000111111101000101001"
                    "111111110101010001110100111000110010100111100001000110100000111101101101011110000011011010011100100000111111101000101001"
                },
        /*160*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "90044030118100801265*D_2D+1.02+31351440315981", 0, 11, 120, 0, "BWIPP different encodation",
                    "111111110101010001110101001110000011010111001111000100111101111010001110001010111000011111010101111100111111101000101001"
                    "111111110101010001111110101000111010011011111000100111100101010000001001001111110111011111010100110000111111101000101001"
                    "111111110101010001010100111100000010011111000011010100001100101110001100000110101111011101010001111110111111101000101001"
                    "111111110101010001010111110111110010000110001100100110011110111001001101000011000111011010111100111110111111101000101001"
                    "111111110101010001110101110000110011100101110011000111010011000001001010011111000011011010111000100000111111101000101001"
                    "111111110101010001111101011110110010111100011000110100001000101111001110100111110010011101011111010000111111101000101001"
                    "111111110101010001010011100111000011010001100011100110100011101111001111101011111101011101001110111110111111101000101001"
                    "111111110101010001111101001011000010100111111001110111010011101100001010001111100110011010111111011110111111101000101001"
                    "111111110101010001111110100110010010110011000111100100101100001111101100111010011100010100110000111110111111101000101001"
                    "111111110101010001010001100000110010011101111001100111101101011100001111010010011110010100011000011000111111101000101001"
                    "111111110101010001110100111000110011110100101000000111111011101000101101011111001110011010011100100000111111101000101001"
                },
        /*161*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "+C910332+02032018+KXXXX CXXXX", 0, 9, 120, 1, "",
                    "111111110101010001111101010111110011010110001110000100000100010010001000011011100011011111010101111100111111101000101001"
                    "111111110101010001111010100001000011000111011111010110101110010000001101001110010000011110101001000000111111101000101001"
                    "111111110101010001010100111100000011101001111100100101010011110000001101010011111000010101000011110000111111101000101001"
                    "111111110101010001101011110011111010100111011100000110100100110000001110000100101110011010111100111110111111101000101001"
                    "111111110101010001101011100000100011001001111001100110000100111001001100011011110001011110101110011100111111101000101001"
                    "111111110101010001111101011110110011101001111110110101111101110011001011111011110111011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100111001000001011101101011000111000011101001110111110111111101000101001"
                    "111111110101010001111110100101110011000101110000010110000101110010001110010000110001010101111110111000111111101000101001"
                    "111111110101010001111110100110010010000011100100110110111110001001001100010010111110011111010011101000111111101000101001"
                },
        /*162*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "+C910332+02032018+KXXXX CXXXX", 0, 9, 120, 1, "",
                    "111111110101010001111101010111110011010110001110000100000100010010001000011011100011011111010101111100111111101000101001"
                    "111111110101010001111010100001000011000111011111010110101110010000001101001110010000011110101001000000111111101000101001"
                    "111111110101010001010100111100000011101001111100100101010011110000001101010011111000010101000011110000111111101000101001"
                    "111111110101010001101011110011111010100111011100000110100100110000001110000100101110011010111100111110111111101000101001"
                    "111111110101010001101011100000100011001001111001100110000100111001001100011011110001011110101110011100111111101000101001"
                    "111111110101010001111101011110110011101001111110110101111101110011001011111011110111011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100111001000001011101101011000111000011101001110111110111111101000101001"
                    "111111110101010001111110100101110011000101110000010110000101110010001110010000110001010101111110111000111111101000101001"
                    "111111110101010001111110100110010010000011100100110110111110001001001100010010111110011111010011101000111111101000101001"
                },
        /*163*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "BP2D+1.00+0005+FLE ESC BV+1.00+3.60*BX2D+1.00+0001+Casual shoes & apparel+90044030118100801265*D_2D+1.02+31351440315981+C910332+02032018+KXXXX CXXXX+UNIT 4 HXXXXXXXX BUSINESS PARK++ST  ALBANS+ST  ALBANS++AL2 3TA+0001+000001+001+00000000+00++N+N+N+0000++++++N+++N*DS2D+1.01+0001+0001+90044030118100801265+++++07852389322++E*F_2D+1.00+0005*", 0, 26, 222, 0, "MR #151 NUM -> BYTE Ex. 1; BWIPP different encodation",
                    "111111110101010001101010000110000011001001000011000111010010000111001000011011100011010000010011101110100000100010010001101001000001100011101010111000000100001010000010001110101011100000011010100001100000111111101000101001"
                    "111111110101010001111010110110000011101000011001000111000011001110101001001111100110011101111011111010100010011110000101110000100001101011001100001111010100010001111001001111100110110011011110101000010000111111101000101001"
                    "111111110101010001010100001111000011111101011000010111010001111100101011010000001110011011111010001000111010010011111101000011100001011010001111000110110110111111000110101101011111100010011010110111111000111111101000101001"
                    "111111110101010001101001001100000011101010111000000100001010000010001110101011100000011101001101111000100001101110001101000001010000010011011100011001000101010000001000001100111000011010011010010011000000111111101000101001"
                    "111111110101010001111010111101111011111001011011100111010001100000101100000101110100011001001111000110110000010111001001111100010110111011110101100110000111101000110110001101111101011110011010111000001000111111101000101001"
                    "111111110101010001111010111110110010000111100010100111011101001111001011110110000110010110111101111110100110110000111101001111100001101010000110010111000110000011010111101100011111001001011010111111010000111111101000101001"
                    "111111110101010001101001111011111011001111011100100110100001100011101101100100010000011010001100111000101000011000110001100001010000011011011100110100000101000100001000001001111011110100011010011110111110111111101000101001"
                    "111111110101010001111101001000110010100000111101000101000011110100001111101101000100011001111101011110100010111101000001011111101011000011011111010111100111100001100110101100011101111101011111101001011100111111101000101001"
                    "111111110101010001111101001110100011111010111000010101000100001111001110100111110010010101001111000000110101001111100001010011011111000011110101111101100100001101001110001001101000111000011111010011100010111111101000101001"
                    "111111110101010001010001110001110011101111011010000111011110110010001010001000000100011101111011010000111011110110001001110000100101110010000101110011100110110011000011001000001010001000010100011100011100111111101000101001"
                    "111111110101010001101001111011000011010001110000100110000010001110101100001001110010011000010011100100110000100111001001100001001110010011100001000011010111110111001101001100101111000110011010011100001000111111101000101001"
                    "111111110101010001111110100011001011001010000111110101111001000010001011100000101100011011110100110000110111111000110101000011010011100011011110010110000110001111001011001111110101101000011111010001110010111111101000101001"
                    "111111110101010001101000001001100011010111101111100100110110001000001000001000100100010000010001010000110000101000110001000010000010100010010000010100000111010110111100001000100000100100011010000010011000111111101000101001"
                    "111111110101010001110100011001000010111001101111110111110001110110101110010001101000011101001100001000101001111100001101111001100110100011011111010111100111110101011000001111110101011100011110100010000100111111101000101001"
                    "111111110101010001010000001001111010011100001000110110101011111000001101010111110000011101001011111100110101011111000001110100101111110011010101111100000110101011111000001101010111110000011001010011111000111111101000101001"
                    "111111110101010001001011001100000011101010111000000100001010000010001010110000110000011100001001011100100010011001100001110000100101110010001001100110000111000010010111001000100110011000010010110011000000111111101000101001"
                    "111111110101010001111101000010110010011001111101000111110101011000001111010110001100010111001101111110101110011011111101110001000011010011011000111100010101110011011111101110001000011010011010001111101110111111101000101001"
                    "111111110101010001111001011110001010011000110011110100111101000010001011110010000010011101000101111110101000011001111101011110001100011010000100010111100111010010111111001101010111110000011001011111101000111111101000101001"
                    "111111110101010001101101000001000011110101011110000100001010000010001110101011100000011101001101111000100111101111010001110001010111000010001000011001100101101110110000001101100001100110011011010000010000111111101000101001"
                    "111111110101010001111010000011011011011111010011110101110011111100101101111010011111010111111010110000110111110101111001011100110111111010111001101111110111111010100011101100101110000001011110100000101000111111101000101001"
                    "111111110101010001101101100111110010100110011111000101100111001111101101000100011111011101001111100100100001101001110001111100101110100010011110100000100100001111010000101000011100001011010010010000011110111111101000101001"
                    "111111110101010001001001100001100010000010011101110100000100010010001101001000001100011101010111000000100001010000010001110101011100000010010111101111000100001100011001001000011000110010010010011000011000111111101000101001"
                    "111111110101010001111001011011000010111111010110000101111110101100001011111101011000010111111010110000111101110111001001111101011000111011100100001100010111010011000010001001111100101110011110010100001000111111101000101001"
                    "111111110101010001110010011111001011110010111100010101111011100111001010000100001111010010001101111100100100000101111001100111100010011011011111000100010101110010011000001001001011110000011011001011110000111111101000101001"
                    "111111110101010001101100110110000010110001111001110101011111011111001001001100110000011011000010000010110110001100110001110100100011100010011011000010000111011010000110001100010111011110011011001101100000111111101000101001"
                    "111111110101010001100101111000110010111011011111100110001011100000101110001011000010010011100010111110111001001110011001110001011001000011110101111011110101011111011000001100110011110100011100101111011100111111101000101001"
                },
        /*164*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "BP2D+1.00+0005+FLE ESC BV+1.00+3.60*BX2D+1.00+0001+Casual shoes & apparel+90044030118100801265*D_2D+1.02+31351440315981+C910332+02032018+KXXXX CXXXX+UNIT 4 HXXXXXXXX BUSINESS PARK++ST  ALBANS+ST  ALBANS++AL2 3TA+0001+000001+001+00000000+00++N+N+N+0000++++++N+++N*DS2D+1.01+0001+0001+90044030118100801265+++++07852389322++E*F_2D+1.00+0005*", 0, 26, 222, 0, "MR #151 NUM -> BYTE Ex. 1; BWIPP different encodation",
                    "111111110101010001101010000110000011001001000011000111010010000111001000011011100011010000010011101110100000100010010001101001000001100011101010111000000100001010000010001110101011100000011010100001100000111111101000101001"
                    "111111110101010001111010110110000011101000011001000111000011001110101001001111100110011101111011111010100010011110000101110000100001101011001100001111010100010001111001001111100110110011011110101000010000111111101000101001"
                    "111111110101010001010100001111000011111101011000010111010001111100101011010000001110011011111010001000111010010011111101000011100001011010001111000110110110111111000110101101011111100010011010110111111000111111101000101001"
                    "111111110101010001101001001100000011101010111000000100001010000010001110101011100000011101001101111000100001101110001101000001010000010011011100011001000101010000001000001100111000011010011010010011000000111111101000101001"
                    "111111110101010001111010111101111011111001011011100111010001100000101100000101110100011001001111000110110000010111001001111100010110111011110101100110000111101000110110001101111101011110011010111000001000111111101000101001"
                    "111111110101010001111010111110110010000111100010100111011101001111001011110110000110010110111101111110100110110000111101001111100001101010000110010111000110000011010111101100011111001001011010111111010000111111101000101001"
                    "111111110101010001101001111011111011001111011100100110100001100011101101100100010000011010001100111000101000011000110001100001010000011011011100110100000101000100001000001101000110001110011010011110111110111111101000101001"
                    "111111110101010001111101001000110011110100111001110111010111000110001010011111100111011101001110110000101000111110011001110010111000110011100010000110100111010011000001001111101101000100011111101001011100111111101000101001"
                    "111111110101010001111101001110100011010100111110000110100010001111101001110000100011010100110111110000110100010001111101110101011111100010110001001110000110011111000100101011111011100110011111010011100010111111101000101001"
                    "111111110101010001010001110001110011101111011010000100011000011010001110111101101000011101111011010000100000100010010001000001000100100011000100101100000110000101000110001000011011110111010100011100011100111111101000101001"
                    "111111110101010001101001111011000011000001011101000111001011000000101100001001110010011000010011100100110000100111001001100011011110001010101111000010000111111011101010001011000101111110011010011100001000111111101000101001"
                    "111111110101010001111110100011001010111100101000000100011110110001101111110101100010010011010000001110101111000001101101110111111000101011001110000101110100011110101000001001101101111000011111010001110010111111101000101001"
                    "111111110101010001101000001001100011101011011110000100010000010010001110000100101110011100110000100110100000101001000001010100000010000011010111101111100100110110001000001000001000100100011010000010011000111111101000101001"
                    "111111110101010001110100011001000011100010000110100111110101000001101111000011001101011100111001111010101111101011100001111010111011100010011001111101000111110101011000001010111100001000011110100010000100111111101000101001"
                    "111111110101010001010000001001111011010101111100000110101011111000001110101011111100010011100001000110111010101111110001001110000100011011010101111100000110101011111000001101010111110000011001010011111000111111101000101001"
                    "111111110101010001001011001100000010101100001100000111010101110000001110001101001100011000111011001000100000100010010001100011101100100010000010001001000110001110110010001000001000100100010010110011000000111111101000101001"
                    "111111110101010001111101000010110011111010101100000111110101011000001011100110111111010111001101111110101110011011111101001111010111100011011111010111100101110011011111101001111010111100011010001111101110111111101000101001"
                    "111111110101010001111001011110001010001111101100010110100011111101001000011100001011010001111000110110110111111000110101101011111100010011101010111111000100111000010001101101010111110000011001011111101000111111101000101001"
                    "111111110101010001101101000001000011101001101111000111010101110000001111010101111000010000101100001100111010101110000001010000010100000011101010011100000111101010111100001101001001100000011011010000010000111111101000101001"
                    "111111110101010001111010000011011011010111001000000111101010000100001111110101011100011110100101000000111010000110010001011100110111111010111001101111110111111010100011101100101110000001011110100000101000111111101000101001"
                    "111111110101010001101101100111110010100110011111000101100111001111101101000100011111011101001111100100100001101001110001111100101110100010011110100000100100001111010000101000011100001011010010010000011110111111101000101001"
                    "111111110101010001001001100001100010000010011101110100000100010010001101001000001100011101010111000000100001010000010001110101011100000010010111101111000100001100011001001000011000110010010010011000011000111111101000101001"
                    "111111110101010001111001011011000010111111010110000101111110101100001011111101011000010111111010110000100010000111100101001111101011100011110100000100100111010111000011001010011110000100011110010100001000111111101000101001"
                    "111111110101010001110010011111001010100001001111000101110111011110001001000000101111010000011011011110111001111110100101100111111001101011110101111010000100111011101111001110011111100101011011001011110000111111101000101001"
                    "111111110101010001101100110110000011000111011001000101100011001000001000111001111011010110001110011000110010110011100001011001111001110011101111000110010110111100111010001111101111001011011011001101100000111111101000101001"
                    "111111110101010001100101111000110011110010110011000100010001111101101110001001101000011001011110001100111101110001110101111001110111001011100010011001000111000100011001001100100111101100011100101111011100111111101000101001"
                },
        /*165*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "[)>\03601\0350246290\035840\03501\0355622748502010201\035FDE\035605421261\035280\035\0351/1\0350.30LB\035N\035201 West 103rd St\035Indianapolis\035IN\035Recipient Name\03606\03510ZED006\03511ZSam's Publishing\03512Z1234567890\03515Z118561\03520Z0.00\0340\03531Z1001891751060004629000562274850201\03532Z02\03534Z01\035KShipment PO10001\035\036\004", 0, 26, 222, 0, "MR #151 NUM -> BYTE Ex. 2; BWIPP different encodation",
                    "111111110101010001101010000110000011001001000011000100000100001000101001010000100000010000100011011000100010100000100001000100001110111010010001000000100101011110111100001000011000110010011010100001100000111111101000101001"
                    "111111110101010001111010110110000011111000111011010101011111100111001010000111100100010110100111111000101111110010001101111101011101111011001011100000100111110101110111101011111100100011011110101000010000111111101000101001"
                    "111111110101010001010100001111000010101110001111110111010101111110001001111110011101010101110001111110100001111000101001111110101110011011011100110011110100001001001111001010111000111111011010110111111000111111101000101001"
                    "111111110101010001101001001100000010111000111000010111101010111100001000001000010001010101111011110000101001111000111101101001111011111010100111100111100101011110111100001000011000110010011010010011000000111111101000101001"
                    "111111110101010001111010111101111011100011101111010111101010001000001111101000101100011110101110011100110100001111011001011111100100011011111010111011110111111010010111001111101011101111011010111000001000111111101000101001"
                    "111111110101010001111010111110110010000011110010100101001011110000001001111001100110011110100001111010111011000101111101000100100001111011000111110010010100111110011000101001110111011110011010111111010000111111101000101001"
                    "111111110101010001101001111011111011010111000111100100100000101000001111001111001010010101111011110000111001101110111101111001111001010010101111011110000100001101110001101111010101111000011010011110111110111111101000101001"
                    "111111110101010001111101001000110011000001011101000100001011110000101110100011000001011001100111100010111100000100010101111101010011000011100001101110010110100111110011101100000100111010011111101001011100111111101000101001"
                    "111111110101010001111101001110100010111100011001100100111111001110101010111000111111010011111011101100101110010110000001101100110011111010001000110111110100001011011111001001101100001111011111010011100010111111101000101001"
                    "111111110101010001010001110001110011000100000100110111100111100101001010111101111000010001110111000100111011001100111001111001111001010010101111011110000100110001100000101101000011101111010100011100011100111111101000101001"
                    "111111110101010001101001111011000011001011110000110110010111000001001110110001110010011100000101100010100100011111001101111011001110011010111111001000110110101110010000001111100011101101011010011100001000111111101000101001"
                    "111111110101010001111110100011001011111001001110010100111111001110101010111000111111011111010111000010100000011010011101101000001011111010011111001100010110101000111110001001111110011101011111010001110010111111101000101001"
                    "111111110101010001101000001001100010101111011110000101011110011110001000111011100100010011000011010000110101101110000001100001100011011011000100000101100110000010010110001100001010000011011010000010011000111111101000101001"
                    "111111110101010001110100011001000010010011111101110100110111111001101100101111000110011010000001110010101111110010001101111101011101111011110000010001010111010011000001001111000110001101011110100010000100111111101000101001"
                    "111111110101010001010000001001111011111010111101100101000100000111101111100101111011011100100111110010101100011001111001001111110011101010101110001111110111101011110001001000000110100111011001010011111000111111101000101001"
                    "111111110101010001001011001100000010000010011000110110100100110000001110010110111100011010011000011100111100111100101001010111101111000010100111011100000100011101110010001000011011100110010010110011000000111111101000101001"
                    "111111110101010001111101000010110011111001101100110111110101110111101011111100100011011110101110111000111110101110111101011111100100011011111010111011110111010011101100001001111001011110011010001111101110111111101000101001"
                    "111111110101010001111001011110001010000111100010100111101011110001001110100100111111011110100011110010100111000110011101101001111110010010000010100011110101111100110100001110111101011100011001011111101000111111101000101001"
                    "111111110101010001101101000001000010001110111100110100000100010001001010100000010000010110001100100000100000100001000101010111101111000010000110001100100100000100111011101101000110011100011011010000010000111111101000101001"
                    "111111110101010001111010000011011011110001100011010111101010010000001011111100100011011111010111011110111010011100110001001111001011110011111000111011010101011111000110001011111100100011011110100000101000111111101000101001"
                    "111111110101010001101101100111110010101110001111110110011111000100101101111001000011011100100111110010100001010001111001100101001111100011001110000101110100000111010011001000010110111110010010010000011110111111101000101001"
                    "111111110101010001001001100001100010000010011000110111010101110000001111010101111000010000010000100010101011110111100001101011110111110011110101001111000100001100011001001000011000110010010010011000011000111111101000101001"
                    "111111110101010001111001011011000010111111010110000101111110101100001011111101011000010111111010110000101000011110010001011001111100001010011100101111100111011101111010001101011111000111011110010100001000111111101000101001"
                    "111111110101010001110010011111001011001111101101100110001101001111001110001010111111011000001010111110101100011011110001101110100111000010101000001111000111001111110101001000110011011110011011001011110000111111101000101001"
                    "111111110101010001101100110110000010011000011101100100111101111010001100011011001100011110111101001000111100110100111001101000010011000011100110010110000100100110000110001100001000001011011011001101100000111111101000101001"
                    "111111110101010001100101111000110011101000011000010100110111111011001111100011001010010100111110001100100111011011111101111110010100111011111011000010010111101011011000001110101111101111011100101111011100111111101000101001"
                },
        /*166*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "[)>\03601\0350246290\035840\03501\0355622748502010201\035FDE\035605421261\035280\035\0351/1\0350.30LB\035N\035201 West 103rd St\035Indianapolis\035IN\035Recipient Name\03606\03510ZED006\03511ZSam's Publishing\03512Z1234567890\03515Z118561\03520Z0.00\0340\03531Z1001891751060004629000562274850201\03532Z02\03534Z01\035KShipment PO10001\035\036\004", 0, 25, 222, 0, "MR #151 NUM -> BYTE Ex. 2; BWIPP different encodation",
                    "111111110101010001101010000110000011011011011000000100000100001000101001010000100000010000100011011000100010100000100001000100001110111010010001000000100101011110111100001001111011110100011010100001100000111111101000101001"
                    "111111110101010001111010100000010011110101000000100111000100011000101111100011010100011011111101011100110101111000110001101111100101111011100101111011100111101001001000001100100001110100011110101000010000111111101000101001"
                    "111111110101010001010100001111000011010010111110000101011100011111101000011110001010011111101011100110110111001100111101000010010011110010101110001111110100111000100001101110101011111100010101101111100000111111101000101001"
                    "111111110101010001101001001100000011000111000110100110100110111000001101001100111000011101000011011110110010110001110001101110001110011010011110111101000101000001000100001010000010000010011010010011000000111111101000101001"
                    "111111110101010001110101111011100010011000011111010110111111010111001101011110001100010000110111110010111010110010000001100100111101100010001011110100000111101001100110001100100011110011011010111000001000111111101000101001"
                    "111111110101010001111010111110110011010001101111110110111110010010001001110000100110011010011111100010101111101100000101111010001111001010000111100101000100011100110111001010111000111111011101011111000010111111101000101001"
                    "111111110101010001101001111011111010000110001100100100001101110001101111010101111000010000010100010000110011101110011001010000001010000011000010100011000100000100110001101110101001110000011010011110111110111111101000101001"
                    "111111110101010001111010010001000011100001101110010110100111110011101100000100111010011000100011110110101111110010001101111101011101111010000110101111110111111011000101101100101110001000011111101001011100111111101000101001"
                    "111111110101010001111101001110100010001000110111110100001011011111001001101100001111010111100100000010100111111001110101010111000111111010000011101001100111101100111110101001111110011101010100111001111110111111101000101001"
                    "111111110101010001010001110001110010101111011110000100110001100000101101000011101111010110011000100000110110010000001001100110110001100011000001010011000111100110100111001000101000010000010100011100011100111111101000101001"
                    "111111110101010001010011110100000010011111101000110101011110000100001100110111100010010010001111100110101101000111111001100010001110100010111111010110000110001100111100101010011111000011011010011100001000111111101000101001"
                    "111111110101010001111110100011001011010101111100000111110010011100101001111110011101010101110001111110111101011110100001000000110100111011011110010000110101011011111000001100011111001010010100011101111110111111101000101001"
                    "111111110101010001101000001001100011000100000101100110000010010110001100001010000011011011000011001100100010000111011101110110011001110011100100110111100111100111100101001010111101111000011010000010011000111111101000101001"
                    "111111110101010001111101000110111011110000010001010111010011000001001111000110001101011110101110011100111010011100110001010001111000001011110010100001000101101001111110001011111100100011011110100010000100111111101000101001"
                    "111111110101010001010000001001111010101110001111110111101011110001001000000110100111010000111100010100111010101111110001001101110111110010111110000110010110011111101100101010010111100000010010100111100000111111101000101001"
                    "111111110101010001001011001100000010000010010000010100111001111011001010011001100000011100110010110000101000100001000001000110011010000011001000101100000101011110011110001100000101001100010010110011000000111111101000101001"
                    "111111110101010001111010000101000011101011110001110110100111010000001011111101001100011110101110001110110101111000001101010011110010000011100010001100010111010011000100001111000100001001011010001111101110111111101000101001"
                    "111111110101010001111001011110001010111110011010000111011110101110001100001111010011010000001101011100111111010110100001000101000001111011001111110110010101001011110000001001111100110001011100101111100010111111101000101001"
                    "111111110101010001101101000001000010011101111011000111000011010001101101000100011000010100100000010000100001101110011001100001000010011011101100011001110111101111010001001010111101111000011011010000010000111111101000101001"
                    "111111110101010001110100000110100010111111010110000100100111101000001111100001101010011001011110000110111101100111001101110110001110010011000001011101000100111101001111001111011000001101011110100000101000111111101000101001"
                    "111111110101010001101101100111110010000111100010100101000001011110001001001100111110011001111110110010101011100011111101111101011100001011101010011111100110001111100100101100011111001001010110110000111100111111101000101001"
                    "111111110101010001001001100001100010000110001100100100001100011001001000011000110010010000110001100100100111011100100001011100111000100011100100001011100100111011101000001101101100110000010010011000011000111111101000101001"
                    "111111110101010001110010110100000011001101111100110100100011110000101110111001111010010111111011011100101001111100001101011111000101110011110001010001000110010111000001001111010000100001011110010100001000111111101000101001"
                    "111111110101010001110010011111001010001111010000100100111110001100101100101100111111011001111010001100100111101100011001011111001100010011001110000101110101111101110110001011000011011110010110010111000000111111101000101001"
                    "111111110101010001101100110110000011010110111000000100001100110100001111101010011111010100001000100000111000010100011101011110011110001011110010100011110111011001100111001110001110110011011011001101100000111111101000101001"
                },
        /*167*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "[)>\03601\0350274310\035250\03570\0351111123177100430\035FDE\035630133769\035222\035\0351/1\035160.00KG\035N\03554 Some Paris St\035Paris\035  \035F. Consignee\03606\03510ZEIO05\03511ZThe French Company\03512Z9876543210\03514Z5th Floor - Receiving\03515Z113167\03531Z1010147571640963660600111112317710\03532Z02\035KMISC_REF1\03599ZEI0005\034US\034200\034USD\034Content DESCRIPTION\034\034Y\034NO EEI 30.37 (a)\0340\034\035\036\004", 0, 28, 239, 0, "MR #151 Ex. 3; BWIPP different encodation",
                    "11111111010101000101010000010000001101100110110000010000010000100010100101000010000001000010001101100010001010000010000100010000111011101001000100000010010101111011110000100001100011001001000011011100110010101000001000000111111101000101001"
                    "11111111010101000111101010000001001111101001011000011111101000101110110101110010000001011111100100011011111010111011110101011111100011101111101011101111010111111001000110111110101110111101111001010100000011111010100001100111111101000101001"
                    "11111111010101000110101000011111001001111110011101010101110001111110100001111000101001010110001111100010111110011000010111111010001101001110111101001110010100011000111110100110000101110001100111111011001010101101111100000111111101000101001"
                    "11111111010101000111010010011100001010111101111000010100111100011110110100111101111101010011110011110010101111011110000100001100011001001110001110100100010100011000110000101011111011111001101000111001111011101001001110000111111101000101001"
                    "11111111010101000111010111101110001111101000001011010111111001000110111110101110111101010111111011100011101001100000010100111111010001101101011110001100011100001001110110101001111110011101001011111001100011101011100000110111111101000101001"
                    "11111111010101000111010111110010001000100100001111011000111110010010101111110111000101011010000001110010000100010111100111110101110001001000101100111110010011111100111010101011100011111101001100011001111011101011111000010111111101000101001"
                    "11111111010101000101001111001111001111001111001010010101111011110000111000111010100001100101001100000010000010001010000100000100101000001000101000010000011000001010011000111101110110111001101110011000010010100111100111100111111101000101001"
                    "11111111010101000111101001000100001011101100111111011000001001110100110001000111101101011111100100011011111010111011110111000011011101001111010110011000011001011111011100101111110010001101111101011101111011110100100100000111111101000101001"
                    "11111111010101000111111010011101101100011110010110010011111100111010101011100011111101000011100010110010111111001110010110011111011011001101000101111100010011000010011100101111011101110001001001000001111010100111001111110111111101000101001"
                    "11111111010101000101000111101111001010000010100000011110011110010100110101111011111001000011011100110011100100110111100111100111100101001010111101111000011010111101111100100011101110010001101000001001100010100011110111100111111101000101001"
                    "11111111010101000101001111010000001111001000000101011110101000100000101111110010001101111101011101111011101011100110000100111100101111001101110111110001011111001010011000111000001011000101111010000100001011101001110000110111111101000101001"
                    "11111111010101000101000110011111001100101001111100010100110000111110100111110110100001111010011110100010111011000111000110101101111110001001111000010010010011111100111010101011100011111101011111101110001010100011101111110111111101000101001"
                    "11111111010101000111010000010011101101000110011100011001110001100010101100111100111001110110111001111010010100000100000101000110000110001101011110111110011110011110010100101011110111100001111101011111101011101000001001110111111101000101001"
                    "11111111010101000111110100011011101001111001011110011100001100111010110000110111101001111001011101110010000010111100010101100100111111001110010000011001011000001011101000111100110110100001000111001011111011110100010000010111111101000101001"
                    "11111111010101000100101011110000001001111101100001011010011001111110110110001001111001111011001111101011111001001110010100111111001110101010111000111111010111111011100010101101000001110001000001100100111010010100111100000111111101000101001"
                    "11111111010101000110010110011100001010111100111100011010001100011100111101101000011101111001111001010010101111011110000110100011000111001000111011100100010011110111101000111010010111000001111010001001111011001011001110000111111101000101001"
                    "11111111010101000111101000010100001100001001110001010010011110000100100100001111010001110100011100011010001001111000010111110111101110101001101111110110011001011110000110100111101011110001111010100000100010100011111001100111111101000101001"
                    "11111111010101000111001011111010001100111111011001010101110001111110110001111100100101000111100011011011101000101111110100000110010011101010100111100000010011111100111010101011100011111101100111110001001011100101111100010111111101000101001"
                    "11111111010101000111011010000011001000101101100000010111001111011000111100111101001001110111001100110011111011010111100110100110000111001111001111001010010101111011110000100100011100011101000111011100100011101101000001100111111101000101001"
                    "11111111010101000111010000011010001111010001000010011111000111011010111110101011000001101000011100001010111111001000110111101011101110001101111101011110011111000100010110101111110010001101111010111011100011111010000010110111111101000101001"
                    "11111111010101000110010010001111101000011100001011011010101111100000100111111001110101111101011100010011011111100011010101111011000001101001111110011101011111010111000100110100010111110001001100001001110010110110000111100111111101000101001"
                    "11111111010101000101101110000110001101110001110011011001101100011000100000101000100001000001001110111010100000010100000101000100010000001011001100010000010111000111000010100110111110111101000001000010001010110111000011000111111101000101001"
                    "11111111010101000111001011010000001111010111011100011110101110111000111010011000000101111010111011100010111111010110000111101100011000101110111101111101011110100010000100110000010111010001101001110100000011111001010000110111111101000101001"
                    "11111111010101000111011011111100101001110111011110010110011011110000100000111000101101001111101100100010000111100110110100111111001110101111101011100010010011111001100010110011111101100101111101011100010010110010111000000111111101000101001"
                    "11111111010101000110010001001100001010111101111000011010111101111100111101010011110001000011000110010010000110001100100100001100011001001000011000110010010000110001100100110111011001000001100001000101100011001000100110000111111101000101001"
                    "11111111010101000100101111000100001101100111100001010001011111011000110101111101110001110100011001000011110111001110100110010111000010001101110111110010010011111101101110110111100101111101011001111110110010010111100100000111111101000101001"
                    "11111111010101000101100111001111101110010111110100010110001001110000101110000100011001001110000001011010111100001000100100111000100001101000010011011111010000011101101110110011001011110001111010111100100011110010001111010111111101000101001"
                    "11111111010101000100100011100011101001000110011000011010001000110000111001011011110001000101110001110011001000100110000101100011101100001110001000100111011100100011011110110110001000100001010000001000001010010001110001110111111101000101001"
                },
        /*168*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "[)>\03601\0350274310\035250\03570\0351111123177100430\035FDE\035630133769\035222\035\0351/1\035160.00KG\035N\03554 Some Paris St\035Paris\035  \035F. Consignee\03606\03510ZEIO05\03511ZThe French Company\03512Z9876543210\03514Z5th Floor - Receiving\03515Z113167\03531Z1010147571640963660600111112317710\03532Z02\035KMISC_REF1\03599ZEI0005\034US\034200\034USD\034Content DESCRIPTION\034\034Y\034NO EEI 30.37 (a)\0340\034\035\036\004", 0, 30, 222, 0, "MR #151 Ex. 3; BWIPP different encodation",
                    "111111110101010001010100000100000011011001000100000100000100001000101001010000100000010000100011011000100010100000100001000100001110111010010001000000100101011110111100001001111011110100011010100001100000111111101000101001"
                    "111111110101010001111101011011100011110101000000100101110010001111101100000101110010011011111101011100110101111000110001000001001111001011001111010111110111010000011000101111000101000100011111010100001100111111101000101001"
                    "111111110101010001010100001111000010100101111000000101011100011111101000011110001010010101100011111000101111100110000101111110100011010011101111010011100101000110001111101001100001011100011111101011001000111111101000101001"
                    "111111110101010001110100100111000011000111000110100110100110111000001101001100111000011101000011011110110010110001110001101110001110011010011110111101000111001011001111001000010100100000011010010011000000111111101000101001"
                    "111111110101010001010111100100000011110100100100000100111111010001101101011110001100010000010011110010101111001011110001111100101000110010101111100110000111101001100110001100100011110011011101011100000110111111101000101001"
                    "111111110101010001111010111110110011010001101111110110111110010010001001111100001101011101001111100100100011100101100001000111100001001011011100100011100110110010000111101101001011111000011101011111101100111111101000101001"
                    "111111110101010001010011110011110010100011110111100110110000100100001100110011000011010011011000100000100001100011001001001100001101000010001001111011110110010100110000001100000100101100011010011110111110111111101000101001"
                    "111111110101010001111010010000100011110101100110000110010111110111001110000010110001010011001011111100101111110010001101111101011101111011100001101110100111101011001100001100101111101110011110100100100000111111101000101001"
                    "111111110101010001111101001110100010011111100111010101011100011111101100011110010110010011111100111010101011100011111101000011100010110011000011110010110100111110110100001111010011110100011110100111101000111111101000101001"
                    "111111110101010001010001111011110010011011000100000111001000101110001100110100001000011001010001100000110001110001101001110100110111100010111001110010000111100110100111001001000110000011010100011100011100111111101000101001"
                    "111111110101010001110100111101110011000100011101000101111110101100001100011001111001010010111100001000111110001110110101101000011100001010111111001000110111110101110111101110101110011000011101001110000110111111101000101001"
                    "111111110101010001111110100011001010000001101001110101111000110110001111001001111001010011111011010000110110101111000001100101001111100010100110000111110100111110110100001111010011110100011110100011110100111111101000101001"
                    "111111110101010001110100000100111011000101100011100111010110111100001000110001100001011110011110010100101011110111100001000001001100011011010001100111000110011100011000101011001111001110011010000010011000111111101000101001"
                    "111111110101010001111010001100110011110010100010000110100011110001101111010011101110011010111001000000101111110010001101111101011101111011101011100011000100111100101111001110000110011101011110100010000010111111101000101001"
                    "111111110101010001010000001001111011011111101100100101100110111100001000011100010110010011011100111110100010000010111101100011110010011010001100100011100110111111011010001010000010111100011100101001111110111111101000101001"
                    "111111110101010001100101100111000011010000010011000110000100100001101100110100000100011110011110010100101011110111100001000001001100011011111011010111100110011100011000101010111100111100010010110011000000111111101000101001"
                    "111111110101010001111010000100100011101001110110000101000011111001101011111100100011011111010111011110111010011101100001001111001011110010111111010011000111101011100011101111010010000010010100011111001100111111101000101001"
                    "111111110101010001111001011110001010111110111000110100110100000011101001100011001111011001011001111110100111001000001101000001100101110011000110001011110111011001111110101001111100011010011100101111110110111111101000101001"
                    "111111110101010001110110100000110011010100000110000100000100001000101010010000001000010000110111001100100111011110110001110000110100011011010001000110000101011110111100001000011000110010011011010000010000111111101000101001"
                    "111111110101010001110100000110010011100100111000110110010111110111001110100110000001011111001011001110111110100010011001111000001000101010111111001000110111110101110111101111011011010000011111010000010110111111101000101001"
                    "111111110101010001101101100111110010000001101001110101000000100111101000011110001010011111101011010000111101000111100101000001111001010011010111111000100100110000100111001101111011001110010110110000011110111111101000101001"
                    "111111110101010001011011100001100011011011000011000100101111011110001010001000010000011011100011000100100011001101000001100001101100110011000011010001000100001100011001001010001100110000010010011000011000111111101000101001"
                    "111111110101010001111100101101110011110010000010010111100010001000101110110001110010011000001011101000111100011101110101110100011000001011110100100000010110010111100001101110001000110100011111001010000110111111101000101001"
                    "111111110101010001110010011111001010011000010011100110011111101100101111101011100010011111010111000100101000100111100001111101011100010011000111110010010101110010000001101100011110100110011101100101111100111111101000101001"
                    "111111110101010001100100010011000011010000010011000100000101000100001010001100011000011011100110010000111011001001100001000001000010001010100111101111000100001011011000001100100010000110011011001101100000111111101000101001"
                    "111111110101010001001011110000100011000100111101100111101100001101001111101011101111011010111001000000111111010100111001011111101011000010111111010110000101111110101100001011111101011000010010111100100000111111101000101001"
                    "111111110101010001001000110011111011000111110010010110001111100100101100011111001001011000111110010010100001111000100101101110001011100010100100000111100101111000010010001110101000111111011100100011111010111111101000101001"
                    "111111110101010001001000111000111010110111001100000100110001100000101000010010000001011000110101000000100010000110001101001011111011111011100100101110000110100000010001101100111001100001010110011110011100111111101000101001"
                    "111111110101010001111100100100011010101111100011000110100011100000101001110100111110011111010010011000110110111100001001111110111001010011110010000110110101110011111101001100010011101000011110110110100000111111101000101001"
                    "111111110101010001101100001001111010011001000001110101001100111110001011100010001100011011110100000110111001110101111001000111100100010010101100111110000110011000100111101000011001101111010110000110011110111111101000101001"
                },
        /*169*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "[)>\03601\0350278759\035840\03503\0355659756807730201\035FDE\035604081602\035169\035\0351/1\0355.00LB\035N\0351234\035Austin\035TX\035Test Co\03606\03510ZED007\03511ZTest Co\03512Z8005553333\03515Z119534\03520Z0.00\034134\03531Z1001901752720007875900565975680773\03532Z02\03534Z01\03539ZNOHA\035\03609\035FDX\035z\0358\035-]\021\020<2\177B\036\004", 0, 25, 222, 0, "MR #151 Ex. 4; BWIPP different encodation",
                    "111111110101010001101010000110000011011011011000000100000100001000101001010000100000010000100011011000100010100000100001000100001110111010010001000000100101011110111100001000011000110010011010100001100000111111101000101001"
                    "111111110101010001111010100000010011111000111011010111110100101100001100101111011000010100011111001100101111110010001101111101011101111011001011100000100111110101110111101011111100100011011110101000010000111111101000101001"
                    "111111110101010001010100001111000010101110001111110110101001111100001001111110011101010101110001111110100001111000101001111110101110011011110000101111010111011111100100101000111101100011010101101111100000111111101000101001"
                    "111111110101010001101001001100000010000110011000100100011000001101001000001000010001010101111011110000101001111000111101101001111011111010100111100111100101011110111100001000011000110010011010010011000000111111101000101001"
                    "111111110101010001110101111011100011100011101111010111111010100111001111010100001000011010111000010000111101010010000001011111100100011011111010111011110110101110000100001110110111000100011010111000001000111111101000101001"
                    "111111110101010001111010111110110010000011110010100101001011110000001001111001100110011110100001111010111011000101111101000100100001111011000111110010010101111110111101101000010001011110011101011111000010111111101000101001"
                    "111111110101010001101001111011111011010111000111100100100000101000001111001111001010010101111011110000111001101110111101111001111001010010101111011110000100000100110001101010011100111000011010011110111110111111101000101001"
                    "111111110101010001111010010001000011010001111011000101111110010001101111101011101111011111000111011010110000010011100101001100101111110011001011110001100101111110010001101111101011101111011111101001011100111111101000101001"
                    "111111110101010001111101001110100010000011101001100101111000100001001001111110011101010101110001111110101111000110110001100101000011111011001110000101110100111110110001001011100001100111010100111001111110111111101000101001"
                    "111111110101010001010001110001110011110011110010100110101111011111001000011011100110011100100110111100111100111100101001010111101111000011010111101111100100011101110010001010000111000111010100011100011100111111101000101001"
                    "111111110101010001010011110100000011111000111011010111111010100011101011111100100011011111010111011110111010111001100001001111001011110011011101111100010111010001100000101100110011110001011010011100001000111111101000101001"
                    "111111110101010001111110100011001010011111011000100101110000110011101001111110011101010101110001111110101111110111000101110100010111111010000011001001110110110011001111101010100011110000010100011101111110111111101000101001"
                    "111111110101010001101000001001100011001011011100000101000110000011001010001100000110011110011110010100101011110111100001110100101110000010001110111001000100000100110001101110100100111000011010000010011000111111101000101001"
                    "111111110101010001111101000110111010100011110001000110100011110110001011111100100011011111010111011110110101111100011101001111001011110011111000111011010111110011011001101111101011101111011110100010000100111111101000101001"
                    "111111110101010001010000001001111010011111100111010111110101110001001111010111100100011111001011101000100111111001110101010111000111111010100010001111000100000011010011101000011110001010010010100111100000111111101000101001"
                    "111111110101010001001011001100000011101001011100000110100110011100001101000001000110010001001100110000111001100110011101111100101011111011100100000101110110000100110111001110111101100010010010110011000000111111101000101001"
                    "111111110101010001111010000101000011011011110001000111101000010000101110001011101100011011111101011100111110101110111101011111101011000011110001110111010111010011000001001111000110001101011010001111101110111111101000101001"
                    "111111110101010001111001011110001010101001111000000100111111001110101010111000111111010100010000011110100000011010011101001111100110001011111101001101000100111111001110101010111000111111011100101111100010111111101000101001"
                    "111111110101010001101101000001000010100011110111100100011101110010001001101100100000010110111011000000100000100001000101010010000001000011110111101100110111110101011111001101111011101000011011010000010000111111101000101001"
                    "111111110101010001110100000110100011110010000101000110100111110111001100010111000010011100000101101000111110100010011001100100000011101011010111100011000111000001011001001100100011101000011110100000101000111111101000101001"
                    "111111110101010001101101100111110011111101001101000111010011111001001110100101111110010100000010111100101001100011111001111101011100001011101010011111100110001111100100101100011111001001010110110000111100111111101000101001"
                    "111111110101010001001001100001100010000110001100100100001100011001001000011000110010010000110001100100110000110110110001110011101001000010111000111000010101000111001110001001101100001000010010011000011000111111101000101001"
                    "111111110101010001110010110100000011000100011100100111100100010010001011111101011000011000100111100110101001111101100001111101000100011011001100111100010111000100111011001110001001101000011110010100001000111111101000101001"
                    "111111110101010001110010011111001010111111001110100101111100001100101100111101001100011011111011001100101111001000100001011110010000001011011000010011110111011111101001001111001011110010010110010111000000111111101000101001"
                    "111111110101010001101100110110000010110011100110000110011101100100001110001110100010010001001110001110100100000100001001101000001000110011010000100011000111010000100111001000110110010000011011001101100000111111101000101001"
                },
        /*170*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "[)>\03601\0350278759\035840\03503\0355659756807730201\035FDE\035604081602\035169\035\0351/1\0355.00LB\035N\0351234\035Austin\035TX\035Test Co\03606\03510ZED007\03511ZTest Co\03512Z8005553333\03515Z119534\03520Z0.00\034134\03531Z1001901752720007875900565975680773\03532Z02\03534Z01\03539ZNOHA\035\03609\035FDX\035z\0358\035-]\021\020<2\177B\036\004", 0, 26, 205, 0, "MR #151 Ex. 4; BWIPP different encodation",
                    "1111111101010100011010100001100000111011010110000001000001000010001010010100001000000100001000110110001000101000001000010001000011101110100100010000001001010111101111000011101010001110000111111101000101001"
                    "1111111101010100011110101101100000101111110100110001111010100000010010111001101111110100111011111101001101111110101110011010111100011000110111110010111101110010111101110011110101000010000111111101000101001"
                    "1111111101010100011101010001111110111111010011101101011100110001110010100100111100000101011100011111101000011110001010011111101011100110111100001011110101110111111001001011010110111111000111111101000101001"
                    "1111111101010100011010010011000000111101111000100101000011001100010010001100000110100110001110001101001101001101110000011010011001110000111010000110111101100101100011100011111010010111110111111101000101001"
                    "1111111101010100011110101111011110111100010001000101011111101001100011110100010000100101100001111100101010000011110010010011111101000110110101111000110001100001101111010011010111000001000111111101000101001"
                    "1111111101010100011101011111010000111110100111000101111100100111010010110000100111000110100111111010001001100110111100011010001101111110110111110010001001110100111110100011010111111010000111111101000101001"
                    "1111111101010100011010011110111110101000100001000001000010001001000010110000110001000110000100101100001111011101101110010100100000010000110000010010011001100111101110100010100111101111000111111101000101001"
                    "1111111101010100011111010010001100110000010111001001100010111100110011010011110001100111101110011100101000001101111101011101001110001100111100011000110101110101111000111011111101001011100111111101000101001"
                    "1111111101010100010100110000111110110010100011111001010011011111000011111100100110100111110101110010001001001000001111010111000110111000101110000010011001000001001001111011111010011100010111111101000101001"
                    "1111111101010100010100011100011100111001000101110001010001000010000011110111101101100101110001111011001000101000001000010000100001001000100001100011001001010000111000111011010001110011110111111101000101001"
                    "1111111101010100011010011110110000111110001110110101111110101000111010111111001000110111110101110111101110101110011000010011110010111100110111011111000101110100011000001011010011100001000111111101000101001"
                    "1111111101010100011010001101111110110011100001011101001111101100010010111000011001110100111111001110101010111000111111010111111011100010111010001011111101000001100100111011111010001110010111111101000101001"
                    "1111111101010100011010000010011000111101100100011101111101010011111011001011011100000101000110000011001010001100000110011110011110010100101011110111100001110100101110000010100000100100000111111101000101001"
                    "1111111101010100011101000110010000100111100101111001111000001000101011101011100000110101000111100010001101000111101100010111111001000110111110101110111101101011111000111011110100010000100111111101000101001"
                    "1111111101010100010100000010111100100000011010011101001111100110001010000100010111100101011100011111101001111110011101011111010111000100111101011110010001111100101110100011001010011111000111111101000101001"
                    "1111111101010100010010110011000000111100111100101001010111101111000011010001100011100100011101110010001001111011110100011101001011100000110100110011100001101000001000110011110010110111110111111101000101001"
                    "1111111101010100011111010000101100110110001111000101110001011000001011101000111000110101101111110110001011100111111010011000010011110110110110111100010001111010000100001011010001111101110111111101000101001"
                    "1111111101010100011110010111100100100011010011100001100111111011001010100101111000000100111110011000101100111100100110011101111110101000111010011111010001010010111100000011001011111101000111111101000101001"
                    "1111111101010100011011010000010000100001101110011001100001000010011011101100011001110111101111010001001010010000001000010000110111001100100001101110001101110000110100110011110110100001110111111101000101001"
                    "1111111101010100011110100000110110111010011100001101111010001010000011110100100000010101101111100010001111000001010010011111101010111000111010111100011101101011110000011011110100000101000111111101000101001"
                    "1111111101010100010110110011110000111111010000110101100001100101111011011001001111000101001011110000001000001110100011011000111001001110101110011101111001010001001111000010010010000011110111111101000101001"
                    "1111111101010100010010011000011000101011100011100001110110100001100011000100001101110111011110011000101000011001110011011110101001111000100001100011001001000011000110010010110111000110000111111101000101001"
                    "1111111101010100011110010110110000100011011111101101101011100100000011000001011100100110111101001111101001000011110100011100001011000100110010111100110001101000111001000011110010100001000111111101000101001"
                    "1111111101010100011101101111110100101101100000111101111100010011101011100010011111010101111011110111101011110011000011010101000111100000100000111011011101001001100111110011011001011110000111111101000101001"
                    "1111111101010100011011001101100000100001010001000001111011101011000011101100000100110110010111001111001110111110111001011000100100110000110101000001100001010001110011100011110010001011110111111101000101001"
                    "1111111101010100011001011110001100101101111100001001111010001000010011000011011110100110111011111010001000000101111001010000100111101000111000010110000101100001001110001011100101111011100111111101000101001"
                },
        /*171*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "[)>\03601\0350285040\035840\03501\035D10011060813097\035EMSY\03537\03562\035\0351/1\0353LB\035N\0354440 E ELWOOD ST\035PHOENIX\035AZ\035CXXXXXX RXXX\03606\0353Z01\03511ZONTRAC - CXXXXXX RXXX\03512Z\03514ZSTE 102\03515Z90210\03520Z2000\034U\0341288\03521Z1\03522Z0\03524Z1\0359KRef-12549\035\036\004", 0, 25, 205, 0, "MR #151 Ex. 5; BWIPP different encodation",
                    "1111111101010100011010100001100000100101110011100001000001000010001010010100001000000100001000110110001000101000001000010001000011101110100100010000001001010111101111000011101010001110000111111101000101001"
                    "1111111101010100011110101000000100101111110101100001111100011101101011111101001011100111010001111011101010011111100111010111111001000110111110101110111101100101110000010011110101000010000111111101000101001"
                    "1111111101010100011101010001111110101011100011111101001111110011101010101110001111110111010101111110001001111110011101010101110001111110100011110001101101000011110001010010101101111100000111111101000101001"
                    "1111111101010100011010010011000000111001011101111101100001011001110011000110001001000100010011011000001001100111100111010000010000100010110100110111000001101001000001100011111010010111110111111101000101001"
                    "1111111101010100011101011110111000111101100011010001111011011000100011100100011001000111101001100011001110001100111010011100101111011100111000101110011001000101111100011011010111000001000111111101000101001"
                    "1111111101010100011101011111010000101001011110000001100111110010010011011111101101000100111001001100001100010100011111011110100111100100101001100011111001010111000111111011101011111000010111111101000101001"
                    "1111111101010100011010011110111110110100010001100001010111101111000010000110001100100100001101111011101010000010100000011101011100111110100001101111011101001110001110100010100111101111000111111101000101001"
                    "1111111101010100011110100100010000111110010001001101111011000011001011010011111001110100110010111111001011111100100011011111010111011110111100010100100001001000011110001011111101001011100111111101000101001"
                    "1111111101010100010100110000111110111011100100111101011111011110111010011111100111010101011100011111101111101011100100010011111100111010101011100011111101110100111111011010100111001111110111111101000101001"
                    "1111111101010100010100011100011100111011110110100001110111101101000011101111011001000100010000100001001110111101101000011110011110010100110101111011111001000011011100110011010001110011110111111101000101001"
                    "1111111101010100010100111101000000110100000011100101011111100100011011111010111011110101001111100001101111000110001101011111101010111000101111110010001101111101011101111011010011100001000111111101000101001"
                    "1111111101010100011010001101111110111101011110100001000000110100111010011000010011100110111100010110001010100111100000011001111101101100100011001000111001000011100001011010100011101111110111111101000101001"
                    "1111111101010100011010000010011000111011110110100001110111101101000011101111011010000111011110001100101110111101101000011100111001000010111100111100101001010111101111000010100000100100000111111101000101001"
                    "1111111101010100011111010001101110111100000100010101110100110000010011110011100111010101111110010001101111101011101111011110000010001010101000111101000001111110011100101011110100010000100111111101000101001"
                    "1111111101010100010100000010111100110011100010011101100111110110110011111010111000010101000100111100001001111110011101010101110001111110111101011110001001000000110100111010010100111100000111111101000101001"
                    "1111111101010100010010110011000000110000110010100001111101010111110011010111101111100111100111100101001010111101111000010100111011100000100011101110010001000011011100011011110010110111110111111101000101001"
                    "1111111101010100011110100001010000111110101011000001111101011101111010111111001000110111101011101110001101111101011110010111111001000110111101011101110001111000001000101011010001111101110111111101000101001"
                    "1111111101010100011110010111100100111110100111010001001000010011110010011111100111010101011100011111101101001101111110010000001101001110101111110111000101001111110011101011100101111100010111111101000101001"
                    "1111111101010100011011010000010000101011110111100001110100111011111010001110111001000100001101110011001111001111001010010101111011110000110100111001111001000111011100100011110110100001110111111101000101001"
                    "1111111101010100011101000001101000111100000100010101011111100100011011111010111011110111100100110001101101101111000001010000010111100100110100001110001001000101111010000011110100000101000111111101000101001"
                    "1111111101010100010110110011110000111111010011000101001010111100000011001111110110010101011100011111101111101011100001011101010011111100110001111100100101100011111001001010110110000111100111111101000101001"
                    "1111111101010100010010011000011000110101110011110001111101111101001011110011110100100100010000100001001001011000001100010010001000100000110011011011000001111001000101111010110111000110000111111101000101001"
                    "1111111101010100011100101101000000101111101101111001110100000011010011110100100100000110000101110010001011110000101111011111001100101000101110100111110001000100000111101011110010100001000111111101000101001"
                    "1111111101010100011101101111110100111111000101101001110011110010111011011111101100010101111100110000101011110100000010011111101000011010110110010011110001111110100011001010110010111000000111111101000101001"
                    "1111111101010100011011001101100000111011000010011001101001000011000010000101100000110110100010110000001101110111100111011100011101100110111011101100110001110010111011111011110010001011110111111101000101001"
                },
        /*172*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "[)>\03601\0350285040\035840\03501\035D10011060813097\035EMSY\03537\03562\035\0351/1\0353LB\035N\0354440 E ELWOOD ST\035PHOENIX\035AZ\035CXXXXXX RXXX\03606\0353Z01\03511ZONTRAC - CXXXXXX RXXX\03512Z\03514ZSTE 102\03515Z90210\03520Z2000\034U\0341288\03521Z1\03522Z0\03524Z1\0359KRef-12549\035\036\004", 0, 22, 205, 0, "MR #151 Ex. 5; BWIPP different encodation",
                    "1111111101010100011101010001110000111001011001111001000001000010001010010100001000000100001000110110001000101000001000010001000011101110100100010000001001010111101111000011101010001110000111111101000101001"
                    "1111111101010100011111010100001100101111110100110001111010100000010011110111001110010111100001000100101101111110101110011010111100011000110111110010111101110010111101110011111101010001110111111101000101001"
                    "1111111101010100011101010001111110111111010011101101011100110001110011010010111110000101011100011111101111101001110100010000111100010100110110101111000001101111000010110011010100001111100111111101000101001"
                    "1111111101010100011111010010111110110001100010010001000100110110000010011001111001110110001110001101001101001101110000011010010000011000111100010010111101101100010010000011111010010111110111111101000101001"
                    "1111111101010100011101011100000110111001000110010001111010011000110011100011001110100111001011110111001110001011100110010001011111000110110101111000110001100000100111010011101011100001100111111101000101001"
                    "1111111101010100011101011111010000110111111011010001001110010011000011000101000111110101000000101111001000011000010111011001001111110010110110010111100001110111010001111011101011111001000111111101000101001"
                    "1111111101010100010100111101111000100001100011001001000011011110111010100001100001100100000101000100001100101001100000011010000010001100110011101100001001001101110001100010100111101111000111111101000101001"
                    "1111111101010100011110100100100000101111000101111001100010001111011010111111001000110111110101110111101111000101001000010010000111100010111100100001010001000110111110100011111010010110000111111101000101001"
                    "1111111101010100010100110000111110100111111001110101010111000111111011111010111001000100111111001110101010111000111111011101001111110110101111101110011001011111011100110011111101001110110111111101000101001"
                    "1111111101010100011010001110011110111011110110010001000100001000010011101111011010000110001110001101001110100110111100010111001110010000111100110100111001001000111011100011010001110011110111111101000101001"
                    "1111111101010100011101001110000110110010111000010001111101001101110011000010111100110111010000110100001100101110010000010111001001111100101111110101100001111001000001001011101001110001100111111101000101001"
                    "1111111101010100011010001101111110110111100010110001010100111100000011000111100100110100011001000111001110100111111011010111110111001100101111101110011001000011000010111010100011001111100111111101000101001"
                    "1111111101010100010100000100100000100010000100001001110111101101000010000010000100010101001000000100001100000100100110011000111001100010101001000010000001110000110100011010100000100100000111111101000101001"
                    "1111111101010100011110100010000010101011110000010001011111101011000011111100111001010111100010001000101100000101110100011010111001000000111010011000000101011111100100011011111010001000110111111101000101001"
                    "1111111101010100010100000010111100101011100011111101111010111100010010000001101001110110111111001101001010100111100000011111010111000010100111111001110101010111000111111010010101111000000111111101000101001"
                    "1111111101010100011110010110111110101001110111000001000111011100100010011110111101000111010110111100001110010000100111011000111000110100110100100000110001000100111101111011110010110111110111111101000101001"
                    "1111111101010100010100011111001100111011011100010001000100000111101011101011111011110111010011100110001101011100010000011001110101111110111111000110101101110110111010000010100011111011000111111101000101001"
                    "1111111101010100011110010111100100101001011110000001001111100110100010001111011000110100110000100011101110010111110010011010011111100100111101000011110101001111011100111011100101111101000111111101000101001"
                    "1111111101010100011110110100001110100010001100110001101111001110010010000110001100100101100011100011001000001011000011011111011010111100100010001101100001010011100011100011110110100001110111111101000101001"
                    "1111111101010100011111010000010110111101000100000101101111110101110011111010111011110110101110010000001111110101001110010111111010110000101111110101100001011111101011000010100001111100110111111101000101001"
                    "1111111101010100010110110011110000101110001000001101101000100011111010011111011101100110110110011111001000000110100111011000011101001110110011110010110001010011000011111011001001000111110111111101000101001"
                    "1111111101010100010110111000110000110111000110001001110001110100010011011000110000110111011000100110001000110001100100010100110001100000110011100110100001100000100110111010110111000110000111111101000101001"
                },
        /*173*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "01\01130\011{)>\01194\011GSA/XE 7\0110200\01502\01107072017\0111Z291YX2AT50000027\01111\011P\011\0113\01110.0\011KGS\011\011\011F/D\011415.52\011USD\011\011\011\011US\011EFTA\011U\011\011\011\011\0112\01504\011SH\011PHILIPS HEALTHCARE\011ROERMOND\011\0116045GH   \011NL\011291YX2\011MARIE CURIEWEG 20\011\011\011NL009076840B01\011PHS EMEA TOMS\011310475528727\011\011\011\01504\011ST\011PHILIPS MEDICAL SYSTEMS\011LOUISVILLE\011KY\01140219    \011US\011\0111920 OUTER LOOP  DRIVE\011\011\011\011C/O UPS-SPS. DOCK 157\011\011\011\011\01505\011GSI\011MEDICAL EQUIPMENT\01507\0111Z291YX2AT50000027\01110.0\011\011\011\011\011\011\011\011\011\011\011\01508\0112\011EA\011103.88\011FILTER  603Y0066\011JP\011\011\011\011\011\011451213341491\01508\0112\011EA\011103.88\011FILTER  603Y0066\011JP\011\011\011\011\011\011451213341491\01513\011\011\011\0114509123000\0112\011415.52\011415.52\01599\015", 0, 32, 256, 0, "MR #151 Ex. 6; BWIPP different encodation",
                    "1111111101010100011010100000110000101100011110111001000011011100110011101001000111000101000110001100001110011010110000010111100011110010111010001011100001100001100011011010010001110111000110011010010000001110110110011100011010100000110000111111101000101001"
                    "1111111101010100011110101101100000111110101110111101100110011110010011010001110000100110000010001110101110110011101000011010111110001110111110101000001101111010100100000011101100111010000111100101010000001111110010101110011110101000001000111111101000101001"
                    "1111111101010100010101000001111000111010101111110001011001011100000010100100000111100100000110010011101111110100111011010100100000111100111011111000101101000011100001011010011111001100010100111000010011001111110010111011011010110111111000111111101000101001"
                    "1111111101010100011110100100111100111010101110000001110101011100000010100111101111000100010100010000001110100100011100011000111011000100111000011001001101110001010001110010001010001000000111101011001111101101011011100000011110100100111100111111101000101001"
                    "1111111101010100011110101111011110100000100011110101101000001110010010001110111111010111001000111001101110000110011101010000001001111010101001111100001101111011001110011011110101110001110111100010011011001111010010001000011010111000000100111111101000101001"
                    "1111111101010100011110101111100110110111111000110101011110110000011010001111000010100101110110011100001001100110001111010111100000100010110001111101010001010000010001111010011100010110000110001111101010001000011010011100011010111111010000111111101000101001"
                    "1111111101010100010100111100011110110001011001110001100010110011100011000101000110000110110000100000101010000010000100010000010001010000111011001000110001100010110001110010010011000011000110110000110011001011101110100000010100111100011110111111101000101001"
                    "1111111101010100011111010010001100110011100111110101010011111100111011111001000101100111111001010111001111010110011000011010001111011000110110011110000101001000011110001011100010011000010111100100000100101010011111000011011111010010011000111111101000101001"
                    "1111111101010100010100111011111100101110110011100001011010000001110010100000100011110111011111100101001011001101111000011000111100101100110001111101010001011100100001100010001111000010100111111010011101101010010000011110011111010011100010111111101000101001"
                    "1111111101010100011010000101100000101111011110100001000011011100011011001101001000000111000101001110001101110011000010011001010011000000101000100001000001101110011000010010100000010000010111010000010111001000001010001000011010000101100000111111101000101001"
                    "1111111101010100011010011110110000110101111100011101110010001110011010010001111101100111110110001101101111100011101101011111010100001100111111010100011101111010000010100010100111111001110111100000100010101111100011101101011010011100000100111111101000101001"
                    "1111111101010100010100011000111110111010111110000101011111100111010010110010001110000110001111010011001101110100000111011111101011100110100111000110011101001100100000111010001111000010100101000100011110001110101001111110011111010001110010111111101000101001"
                    "1111111101010100010100000100010000110110111001110001001010000100000011110010001011110101001111011110001100010110011100010001011001100000111101010011110001100110100100000011100110000100110111001111011001001110011001000011010100000100010000111111101000101001"
                    "1111111101010100011101000110010000111001011110011101100101111000011010111011001111110111101100111001101101001110000100011010111110001110111100100010000101111101110110010010011001011111100111110100011011101111100010001011011101000110100000111111101000101001"
                    "1111111101010100011001010111110000110011001001111001011100001011000011011000101111000111000111010111101111110011011101011111001011101000100110010011100001011111000110100011011101000001110101010011110000001110101111100100011001010011111000111111101000101001"
                    "1111111101010100011100101100111100100000101001000001000001010010000011001101001000000111100011010111001110000110010011010001010001000000100100011000011001110101110011111011100011101100110110001101001000001010000011000110011100101100111100111111101000101001"
                    "1111111101010100011111010000101100100111001111110101111011000011001011100010111001100111001110011110101111000100110011011000100001110100100011101111110101110010001110011010010001111101100111010011000000101110001000110001010100011111000110111111101000101001"
                    "1111111101010100011110010111110110111001111101011001110000101111101010000111100101000100011000110111101011110010000010010111101100110000100011110001101101101110001011100011001101000011110101111110111000101111001011110010011001011111101000111111101000101001"
                    "1111111101010100011001001011000000110001011001110001100010110011100011011000010000010110010110001110001110001110100100010111001111101110111001111011001001000101000010000010100011100011100101001110111000001100010101100000011001001011000000111111101000101001"
                    "1111111101010100011110100000110110111101000110001101000100011111011011110001010000010111010001100100001000100011110100010010011110000010111001011010000001101011111001110011110001100011010111101001001000001101011111001110011110100000100100111111101000101001"
                    "1111111101010100010110110001111000111011111000101101000011100001011010011111001100010100111000010011001111110010111011011010101111100000110101011111000001010011000011111010111010000110000111111010110001001010110111110000010010010000011110111111101000101001"
                    "1111111101010100010010011101110000110001011001110001100010110011100011000101100111000110001011001110001100010110011100011011000010000010100100010001000001110100010111000010000110111101110110101110001111001000101000100000010010011101110000111111101000101001"
                    "1111111101010100011110010110110000111110101001100001111000100110011010010111100010000111000011001110101110010111100111011110001000100010100110111110100001100000101110100010100001111010000101001111100001101110001100111001011110010100000100111111101000101001"
                    "1111111101010100011001001111110100110101011111000001100100100111110010011001100011110101100000101110001000111100001010010111011001110000101110110011100001101110100000111010010111011111100110100110111111001010001000011110011011001011110000111111101000101001"
                    "1111111101010100011100100010011100101000011000001101110100000100111011010010001100000110101000011000001100010100011000011001101001000000101000011000011001110000110010011011010111101111100110100001000110001101100110110000011100100010011100111111101000101001"
                    "1111111101010100011001011110001100100100011111011001101000111110111011111001000101100111010001100001001110000010110010011100011101111010111110101001100001011111010000111011111000111011010111110101000110001110100000110100011001011110011000111111101000101001"
                    "1111111101010100011111001000111010110111111001101001000110100000111010111011001110000101110110011100001011101100111000010100000100011110111110101111011001111010111100100010100010000011110111110101111001101001000010001111011110110011111010111111101000101001"
                    "1111111101010100010110011110001110100100000101000001111010000101111011000101100111000100010100001000001110010100011100010110011100011000101001110011100001110101011100000011010110111000000111010001011100001010000110000011010110011110001110111111101000101001"
                    "1111111101010100011111011011001100111101000010000101101000111100110011110110011100110111101011100011101111000100110110011111101001001110111101101101000001111001000110110010111111010110000101111110101100001011111101011000011110010010010000111111101000101001"
                    "1111111101010100010110000100001110110001111100100101001111011000011010000010100111100101111000010100001001111110111001011011110100011000111110101110010001110000101111101011001111101000010100100111011111101001111100001101010010000010011110111111101000101001"
                    "1111111101010100010010000110110000100100001101100001010000001010000011011001000100000101001110111000001000101000100000011001011011100000110101110111100001000001010000010010000010011101110110011110111010001110111101110011010010000110110000111111101000101001"
                    "1111111101010100011011011110001000100100001111001001111000100001010011111100110101100101011111100011101000001011110010011101000011000010101110011111101001011101111110001011100110001110010100110101111110001111110100010111011011011110010000111111101000101001"
                },
        /*174*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "01\01130\011{)>\01194\011GSA/XE 7\0110200\01502\01107072017\0111Z291YX2AT50000027\01111\011P\011\0113\01110.0\011KGS\011\011\011F/D\011415.52\011USD\011\011\011\011US\011EFTA\011U\011\011\011\011\0112\01504\011SH\011PHILIPS HEALTHCARE\011ROERMOND\011\0116045GH   \011NL\011291YX2\011MARIE CURIEWEG 20\011\011\011NL009076840B01\011PHS EMEA TOMS\011310475528727\011\011\011\01504\011ST\011PHILIPS MEDICAL SYSTEMS\011LOUISVILLE\011KY\01140219    \011US\011\0111920 OUTER LOOP  DRIVE\011\011\011\011C/O UPS-SPS. DOCK 157\011\011\011\011\01505\011GSI\011MEDICAL EQUIPMENT\01507\0111Z291YX2AT50000027\01110.0\011\011\011\011\011\011\011\011\011\011\011\01508\0112\011EA\011103.88\011FILTER  603Y0066\011JP\011\011\011\011\011\011451213341491\01508\0112\011EA\011103.88\011FILTER  603Y0066\011JP\011\011\011\011\011\011451213341491\01513\011\011\011\0114509123000\0112\011415.52\011415.52\01599\015", 0, 32, 256, 0, "MR #151 Ex. 6; BWIPP different encodation",
                    "1111111101010100011010100000110000101100011110111001000011011100110011101001000111000101000110001100001110011010011000010000010100001000100011000110000101110100010111000010010001110111000110011010010000001110110110011100011010100000110000111111101000101001"
                    "1111111101010100011110101101100000111110101110111101100110011110010011010001110000100110000010001110101110110011101000011010111110001110111110101000001101111010100100000011101100111010000111100101010000001111110010101110011110101000001000111111101000101001"
                    "1111111101010100010101000001111000111010101111110001011001011100000010100100000111100100000110010011101111110100111011010100100000111100111011111000101101000011100001011010011111001100010100111000010011001111110010111011011010110111111000111111101000101001"
                    "1111111101010100011110100100111100111010101110000001110101011100000010100111101111000100010100010000001110100100011100011000111011000100111000011001001101110001010001110010001010001000000111101011001111101101011011100000011110100100111100111111101000101001"
                    "1111111101010100011110101111011110100000100011110101101000001110010010001110111111010111001000111001101110000110011101010000001001111010101001111100001101111011001110011011110101110001110111100010011011001111010010001000011010111000000100111111101000101001"
                    "1111111101010100011110101111100110110111111000110101011110110000011010001111000010100101110110011100001001100110001111010111100000100010110001111101010001010000010001111010011100010110000110001111101010001000011010011100011010111111010000111111101000101001"
                    "1111111101010100010100111100011110110001011001110001100010110011100011000101000110000110110000100000101010000010000100010000010001010000111011001000110001100010110001110010010011000011000110110000110011001011101110100000010100111100011110111111101000101001"
                    "1111111101010100011111010010001100110011100111110101010011111100111011111001000101100111111001010111001111010110011000011010001111011000110110011110000101001000011110001011100010011000010111100100000100101010011111000011011111010010011000111111101000101001"
                    "1111111101010100010100111011111100101110110011100001011010000001110010100000100011110111011111100101001011001101111000011000111100101100110001111101010001011100100001100010001111000010100111111010011101101010010000011110011111010011100010111111101000101001"
                    "1111111101010100011010000101100000101111011110100001000011011100011011001101001000000111000101001110001101110011000010011001010011000000101000100001000001101110011000010010100000010000010111010000010111001000001010001000011010000101100000111111101000101001"
                    "1111111101010100011010011110110000110101111100011101110010001110011010010001111101100111110110001101101111100011101101011111010100001100111111010100011101111010000010100010100111111001110111100000100010101111100011101101011010011100000100111111101000101001"
                    "1111111101010100010100011000111110111010111110000101011111100111010010110010001110000110001111010011001101110100000111011111101011100110100111000110011101001100100000111010001111000010100101000100011110001110101001111110011111010001110010111111101000101001"
                    "1111111101010100010100000100010000110110111001110001001010000100000011110010001011110101001111011110001100010110011100010001011001100000111101010011110001100110100100000011100110000100110111001111011001001110011001000011010100000100010000111111101000101001"
                    "1111111101010100011101000110010000111001011110011101100101111000011010111011001111110111101100111001101101001110000100011010111110001110111100100010000101111101110110010010011001011111100111110100011011101111100010001011011101000110100000111111101000101001"
                    "1111111101010100011001010111110000110011001001111001011100001011000011011000101111000111000111010111101111110011011101011111001011101000100110010011100001011111000110100011011101000001110101010011110000001110101111100100011001010011111000111111101000101001"
                    "1111111101010100011100101100111100100000101001000001000001010010000011001101001000000111100011010111001110000110010011010001010001000000100100011000011001110101110011111011100011101100110110001101001000001010000011000110011100101100111100111111101000101001"
                    "1111111101010100011111010000101100100111001111110101111011000011001011100010111001100111001110011110101111000100110011011000100001110100100011101111110101110010001110011010010001111101100111010011000000101110001000110001010100011111000110111111101000101001"
                    "1111111101010100011110010111110110111001111101011001110000101111101010000111100101000100011000110111101011110010000001010111101100110000111101000111101001010011101111110011001111101101100111101011110001001011001011100000011001011111101000111111101000101001"
                    "1111111101010100011001001011000000110001011001110001000101100110000011111010100111110110011010010000001110110110011100011011001110011100110001011001110001010000111000111011101100100001100101010000001000001001110001110100011001001011000000111111101000101001"
                    "1111111101010100011110100000110110100110100111111001100101111000011011110110011100110111011000111001001000011001111101011111101010001110111001000110010001001111001011110011110000110011010111110110100010001011111010000111011110100000100100111111101000101001"
                    "1111111101010100010110110001111000100111100111011101110100010111111011010110001111110101111110111101101101010111110000011010101111100000101010011110000001011001011100000011111010111000010100001000101111001011101100111000010010010000011110111111101000101001"
                    "1111111101010100010010011101110000110001011001110001100010110011100011000101100111000110001011001110001000101100110000011010100001100000110001010001100001100110100100000010100001100001100111000011001001101101011110111110010010011101110000111111101000101001"
                    "1111111101010100011110010110110000111010011110011101110010111101110010010001111101100110100011111011101111100100010110011101000110000100111000001011001001110001110111101011111010100110000101111101000011101111100011101101011110010100000100111111101000101001"
                    "1111111101010100011001001111110100110101000111110001011011000011110011011111100110100100011010000011101011101100111000010111011001110000101110110011100001010000010001111011111010111101100111101011110010001010001000001111011011001011110000111111101000101001"
                    "1111111101010100011100100010011100111110101111110101011001110001100011011000010000010100100010001000001110100010111000010000110111101110110101110001111001000101000100000011101010011100000110111001100001001001000100010000011100100010011100111111101000101001"
                    "1111111101010100011001011110001100111000011001110101110010111100111011110001000100010100110111110100001100000101110100010100001111010000101001111100001101110001100111001011111010101100000101000011111011001001000111110110011001011110011000111111101000101001"
                    "1111111101010100011111001000111010101100000101110001000111100001010010111011001110000101110110011100001101110100000111010010111011111100110100110111111001010001000011110011101000011111010100101011110000001110101111100010011110110011111010111111101000101001"
                    "1111111101010100010110011110001110101011111011111001100010110011100011000101100111000110100000101100001010100000100000011010111100111110101000110001100001110101011100000011000101000110000100010100001000001110100101110000010110011110001110111111101000101001"
                    "1111111101010100011111011011001100111100010011011001111010010001000011110100010100000111101000010000101101000111100110011111011001010000111111011011011101011111101011000010111111010110000101111110101100001011111101011000011110010010010000111111101000101001"
                    "1111111101010100010110000100001110110001111100100101100011111001010010011001000001110110001111101000101001100011011110010001111010100000100111100110110001100011010001111010000111001011000100011110001000101110111110101100010010000010011110111111101000101001"
                    "1111111101010100010010000110110000100001001000010001010100000100000011001110110000010111010101110000001110000110100011010110011000100000100100000010010001110011001011000011011000011001100101110011110110001011001111000111010010000110110000111111101000101001"
                    "1111111101010100011011011110001000111011100111101001010001111000001011101101110001000101111101100111101000110100111111011110001101100100101100111110000101111010000001001011100100011101100111100100001101101110000101100010011011011110010000111111101000101001"
                },
        /*175*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABC123456789ABC", 0, 9, 103, 1, "T3 N9 T3 -> T15",
                    "1111111101010100011111010101111100110101000001100001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111010011000001001111010111001110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011010101000011110000111111101000101001"
                    "1111111101010100011010111100111110100100110000110001001000010010000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111111010101110001110100110000001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100111101001000001110110001011111011110101111101100111111101000101001"
                    "1111111101010100011101001110111110110111000110010001100110001001000011010011101111000111111101000101001"
                    "1111111101010100011111101001011100100100111110110001110110000111010010101111110111000111111101000101001"
                    "1111111101010100011010011011111100101111100110010001011110010100000011111010011101000111111101000101001"
                },
        /*176*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABC123456789ABC", 0, 9, 103, 1, "T3 N9 T3 -> T15",
                    "1111111101010100011111010101111100110101000001100001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111010011000001001111010111001110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011010101000011110000111111101000101001"
                    "1111111101010100011010111100111110100100110000110001001000010010000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111111010101110001110100110000001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100111101001000001110110001011111011110101111101100111111101000101001"
                    "1111111101010100011101001110111110110111000110010001100110001001000011010011101111000111111101000101001"
                    "1111111101010100011111101001011100100100111110110001110110000111010010101111110111000111111101000101001"
                    "1111111101010100011010011011111100101111100110010001011110010100000011111010011101000111111101000101001"
                },
        /*177*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABC1234567890ABC", 0, 9, 103, 1, "T3 N10 T3 -> T16",
                    "1111111101010100011111010101111100110101000001100001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111010011000001001111010111001110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011010101000011110000111111101000101001"
                    "1111111101010100011010111100111110100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111110001110110101111010111001110011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111011111101000101010011100111111011110101111101100111111101000101001"
                    "1111111101010100011101001110111110100001011011000001001000001101100011010011101111000111111101000101001"
                    "1111111101010100011111101001011100110100011111011101110000010111011010101111110111000111111101000101001"
                    "1111111101010100011010011011111100110001001111110101000111100001001011111010011101000111111101000101001"
                },
        /*178*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABC1234567890ABC", 0, 9, 103, 1, "T3 N10 T3 -> T16",
                    "1111111101010100011111010101111100110101000001100001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111010011000001001111010111001110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011010101000011110000111111101000101001"
                    "1111111101010100011010111100111110100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111110001110110101111010111001110011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111011111101000101010011100111111011110101111101100111111101000101001"
                    "1111111101010100011101001110111110100001011011000001001000001101100011010011101111000111111101000101001"
                    "1111111101010100011111101001011100110100011111011101110000010111011010101111110111000111111101000101001"
                    "1111111101010100011010011011111100110001001111110101000111100001001011111010011101000111111101000101001"
                },
        /*179*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABC12345678901ABC", 0, 10, 103, 0, "T3 N11 T3; BWIPP -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111010011000000101011111101001100011111010100110000111111101000101001"
                    "1111111101010100011101010111111000111110010111000101111110100011001011010100011111000111111101000101001"
                    "1111111101010100010101111101111100110010000010110001110100001011100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000101111110101100001111110101011100011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000101000011000001101000100111011100011010011101111000111111101000101001"
                    "1111111101010100011110100101000000100110111110100001111100011010100011010111111011110111111101000101001"
                    "1111111101010100011010011011111100100111000101100001100110100111100010100110001111100111111101000101001"
                    "1111111101010100010100011000001100110111011000100001000110001100001011010001100011100111111101000101001"
                },
        /*180*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABC12345678901ABC", 0, 10, 103, 0, "T3 N11 T3; BWIPP -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111010011000001001111010111001110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000110101111100111001111110101011100011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110001110011010001110001110110110011010011101111000111111101000101001"
                    "1111111101010100011110100101000000110001011100010001000111001011111011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110111010111000001011100100000011010100110001111100111111101000101001"
                    "1111111101010100010100011000001100100010000111011101110010010011100011010001100011100111111101000101001"
                },
        /*181*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "AB+12345678901ABC", 0, 10, 103, 0, "T3 N11 T3; BWIPP -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110111110101111001011111101001100011111010100110000111111101000101001"
                    "1111111101010100011101010111111000111110010111000101111110100011001011010100011111000111111101000101001"
                    "1111111101010100010101111101111100110010000010110001110100001011100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000101111110101100001111110101011100011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110001000010001101111010100111100011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111101101101000001110001000011010011010111111011110111111101000101001"
                    "1111111101010100011010011011111100101000010001111001101111000101100010100110001111100111111101000101001"
                    "1111111101010100010100011000001100101100011100001101101100100010000011010001100011100111111101000101001"
                },
        /*182*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "AB+12345678901ABC", 0, 10, 103, 1, "T3 N11 T3",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110111110101111001111010111001110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000110101111100111001111110101011100011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000100011011010000001001001110111000011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111110000110010101111100110001001011010111111011110111111101000101001"
                    "1111111101010100011010011011111100111101011111001101011110001010000010100110001111100111111101000101001"
                    "1111111101010100010100011000001100110011110111000101000100011011000011010001100011100111111101000101001"
                },
        /*183*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABC12345678901+BC", 0, 10, 103, 1, "T3 N11 T3 -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111010011000001001111010111001110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000101011110000100001111000001000101011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000111000011010001101000110011010000011010011101111000111111101000101001"
                    "1111111101010100011110100101000000100110101111110001100000010111010011010111111011110111111101000101001"
                    "1111111101010100011010011011111100111110101110001001011110001010000010100110001111100111111101000101001"
                    "1111111101010100010100011000001100110001011000111001101011000111000011010001100011100111111101000101001"
                },
        /*184*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABC12345678901+BC", 0, 10, 103, 1, "T3 N11 T3 -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111010011000001001111010111001110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000101011110000100001111000001000101011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000111000011010001101000110011010000011010011101111000111111101000101001"
                    "1111111101010100011110100101000000100110101111110001100000010111010011010111111011110111111101000101001"
                    "1111111101010100011010011011111100111110101110001001011110001010000010100110001111100111111101000101001"
                    "1111111101010100010100011000001100110001011000111001101011000111000011010001100011100111111101000101001"
                },
        /*185*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "AB+12345678901+BC", 0, 10, 103, 1, "T3 N11 T3 -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110111110101111001111010111001110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000101011110000100001111000001000101011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110011000010000101010010000001000011010011101111000111111101000101001"
                    "1111111101010100011110100101000000101111101011100001000100001111001011010111111011110111111101000101001"
                    "1111111101010100011010011011111100100001100101110001000000100101111010100110001111100111111101000101001"
                    "1111111101010100010100011000001100111000110001011001001000010000100011010001100011100111111101000101001"
                },
        /*186*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "AB+12345678901+BC", 0, 10, 103, 1, "T3 N11 T3 -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110111110101111001111010111001110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000101011110000100001111000001000101011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110011000010000101010010000001000011010011101111000111111101000101001"
                    "1111111101010100011110100101000000101111101011100001000100001111001011010111111011110111111101000101001"
                    "1111111101010100011010011011111100100001100101110001000000100101111010100110001111100111111101000101001"
                    "1111111101010100010100011000001100111000110001011001001000010000100011010001100011100111111101000101001"
                },
        /*187*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABC123456789012ABC", 0, 10, 103, 1, "T3 N12 T3 -> T18",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111010011000001001111010111001110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111101011100111001111100011101101011010111000100000111111101000101001"
                    "1111111101010100011110101111010000111110101111011001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110100001001100001011000011100110011010011101111000111111101000101001"
                    "1111111101010100011110100101000000110000011011110101000100011111011011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110011101000111001100011110001011010100110001111100111111101000101001"
                    "1111111101010100010100011000001100101000110110000001001001111001111011010001100011100111111101000101001"
                },
        /*188*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABC123456789012ABC", 0, 10, 103, 1, "T3 N12 T3 -> T18",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111010011000001001111010111001110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101000100000111101111100101111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100100110000110001100100011000111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111101011100111001111100011101101011010111000100000111111101000101001"
                    "1111111101010100011110101111010000111110101111011001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110100001001100001011000011100110011010011101111000111111101000101001"
                    "1111111101010100011110100101000000110000011011110101000100011111011011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110011101000111001100011110001011010100110001111100111111101000101001"
                    "1111111101010100010100011000001100101000110110000001001001111001111011010001100011100111111101000101001"
                },
        /*189*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCD123456789ABC", 0, 9, 103, 1, "T4 N9 T3 -> T16",
                    "1111111101010100011111010101111100110101000001100001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011110101000010000110101111110111101111000001000101011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101001100111110001010000010001111010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111101101000011101100100010011000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111110001110110101111010111001110011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110011111010010001110100111110001011110101111101100111111101000101001"
                    "1111111101010100011101001110111110101110011100010001110010100011100011010011101111000111111101000101001"
                    "1111111101010100011111101001011100100010000111100101111001001100011010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100100010111100001001001100011111011111010011101000111111101000101001"
                },
        /*190*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD123456789ABC", 0, 9, 103, 1, "T4 N9 T3 -> T16",
                    "1111111101010100011111010101111100110101000001100001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011110101000010000110101111110111101111000001000101011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101001100111110001010000010001111010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111101101000011101100100010011000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111110001110110101111010111001110011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110011111010010001110100111110001011110101111101100111111101000101001"
                    "1111111101010100011101001110111110101110011100010001110010100011100011010011101111000111111101000101001"
                    "1111111101010100011111101001011100100010000111100101111001001100011010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100100010111100001001001100011111011111010011101000111111101000101001"
                },
        /*191*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCD1234567890ABC", 0, 10, 103, 1, "T4 N10 T3 -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110101111110111101111000001000101011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101001100111110001010000010001111011010100011111000111111101000101001"
                    "1111111101010100010101111101111100111101101000011101100100010011000010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111101011101110001111110101011100011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000101100001101000001100100000010110011010011101111000111111101000101001"
                    "1111111101010100011110100101000000100000100111100101011011111010000011010111111011110111111101000101001"
                    "1111111101010100011010011011111100100001111000100101110111010011110010100110001111100111111101000101001"
                    "1111111101010100010100011000001100110010110111000001011001111011100011010001100011100111111101000101001"
                },
        /*192*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD1234567890ABC", 0, 10, 103, 1, "T4 N10 T3 -> T17",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110101111110111101111000001000101011111010100110000111111101000101001"
                    "1111111101010100011101010111111000101001100111110001010000010001111011010100011111000111111101000101001"
                    "1111111101010100010101111101111100111101101000011101100100010011000010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111101011101110001111110101011100011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000101100001101000001100100000010110011010011101111000111111101000101001"
                    "1111111101010100011110100101000000100000100111100101011011111010000011010111111011110111111101000101001"
                    "1111111101010100011010011011111100100001111000100101110111010011110010100110001111100111111101000101001"
                    "1111111101010100010100011000001100110010110111000001011001111011100011010001100011100111111101000101001"
                },
        /*193*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCD12345678901ABC", 0, 10, 103, 0, "T4 N11 T3; BWIPP -> T18",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110101111110111101011111101001100011111010100110000111111101000101001"
                    "1111111101010100011101010111111000111110010111000101111110100011001011010100011111000111111101000101001"
                    "1111111101010100010101111101111100110010000010110001110100001011100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000101111110101100001111110101011100011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000100011000110010001110110001001100011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111001011101100001111101110011001011010111111011110111111101000101001"
                    "1111111101010100011010011011111100100111001000001101000111011011100010100110001111100111111101000101001"
                    "1111111101010100010100011000001100100011100111001001000100010010000011010001100011100111111101000101001"
                },
        /*194*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD12345678901ABC", 0, 10, 103, 0, "T4 N11 T3; BWIPP -> T18",
                    "1111111101010100011101010011100000110101101110000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110101111110111101011111101001100011111010100110000111111101000101001"
                    "1111111101010100011101010111111000111110010111000101111110100011001011010100011111000111111101000101001"
                    "1111111101010100010101111101111100110010000010110001110100001011100010101111001111000111111101000101001"
                    "1111111101010100011010111000010000101111110101100001111110101011100011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101000100111100001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000100011000110010001110110001001100011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111001011101100001111101110011001011010111111011110111111101000101001"
                    "1111111101010100011010011011111100100111001000001101000111011011100010100110001111100111111101000101001"
                    "1111111101010100010100011000001100100011100111001001000100010010000011010001100011100111111101000101001"
                },
        /*195*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCD123456789012ABC", 0, 7, 120, 0, "T4 N12 T3; BWIPP -> T19",
                    "111111110101010001111101010111110011101011011110000111101010111100001010011100111000011111010101111100111111101000101001"
                    "111111110101010001111101010001100010111111010011000111111010101110001001100011111010011110101001000000111111101000101001"
                    "111111110101010001010100111100000011101001111110110100000101011110001010000100111100011010100011111000111111101000101001"
                    "111111110101010001101011110011111010000110001100100111101010111100001110100011001111011010111100111110111111101000101001"
                    "111111110101010001101011100001000010111111010110000111100100100100001100010111001000011110101110011100111111101000101001"
                    "111111110101010001111101011110110010110100011100000100011110010001001101111000010110011110101111000010111111101000101001"
                    "111111110101010001110100111011111010011100011101000110110001000000101001100111011000011101001110111110111111101000101001"
                },
        /*196*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD123456789012ABC", 0, 7, 120, 1, "T5 N11 T3 -> T19",
                    "111111110101010001111101010111110011101011011110000111101010111100001010011100111000011111010101111100111111101000101001"
                    "111111110101010001111101010001100011110000010001010110101111110111101111101000100110011110101001000000111111101000101001"
                    "111111110101010001010100111100000010110110011110000101100111001111101110101011111100011010100011111000111111101000101001"
                    "111111110101010001101011110011111011010001100111000111101010111100001110100011001111011010111100111110111111101000101001"
                    "111111110101010001101011100001000010111111010110000100111011011111101110110011101000011110101110011100111111101000101001"
                    "111111110101010001111101011110110011011000010011110100011110101000001011111001100010011110101111000010111111101000101001"
                    "111111110101010001110100111011111010100001101100000111001100110011101010001110011100011101001110111110111111101000101001"
                },
        /*197*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCD\177FGH", 0, 9, 103, 1, "BYTE1",
                    "1111111101010100011111010101111100110101000001100001000001000010001011110101011110000111111101000101001"
                    "1111111101010100011110101000010000110100111100011001100100111100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111001100101111101111110010110010010101000011110000111111101000101001"
                    "1111111101010100011010111100111110110011100011000101101000101100000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111101001000100001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011110101111010000101111100111011001001000100111100011110101111101100111111101000101001"
                    "1111111101010100011101001110111110100011000110010001010000001000001011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111000010011001001111000101100011010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100111000001001101100110110011111011111010011101000111111101000101001"
                },
        /*198*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD\177FGH", 0, 8, 103, 0, "BYTE1; BWIPP same as FAST_MODE",
                    "1111111101010100011111010101111100110101000011000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111101010001110110101111110111101011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000000101111001111100101111011011101010001111110111111101000101001"
                    "1111111101010100011010111100111110111011001000110001000011000110010010101111001111000111111101000101001"
                    "1111111101010100011101011100001100101111101000111001011001111110110011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100011100010110001111011111101011011101011111010000111111101000101001"
                    "1111111101010100011101001110111110100010010000010001101000011101111011010011101111000111111101000101001"
                    "1111111101010100011111010010110000101111000100111101001100111110100010101111110111000111111101000101001"
                },
        /*199*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABC+\177FGH", 0, 9, 103, 1, "BYTE1",
                    "1111111101010100011111010101111100110101000001100001000001000010001011110101011110000111111101000101001"
                    "1111111101010100011110101000010000110100111100011001100100111100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000100110010111000001101000001011111010101000011110000111111101000101001"
                    "1111111101010100011010111100111110100010111011100001101000101100000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111101001000100001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110001100101111001010000001011110011110101111101100111111101000101001"
                    "1111111101010100011101001110111110100001001100110001101100010000010011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111010000111001101110010011000001010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100111101100001101001110110011100011111010011101000111111101000101001"
                },
        /*200*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD\177FGH", 0, 8, 103, 0, "BYTE1; BWIPP same as FAST_MODE",
                    "1111111101010100011111010101111100110101000011000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111101010001110110101111110111101011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000000101111001111100101111011011101010001111110111111101000101001"
                    "1111111101010100011010111100111110111011001000110001000011000110010010101111001111000111111101000101001"
                    "1111111101010100011101011100001100101111101000111001011001111110110011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100011100010110001111011111101011011101011111010000111111101000101001"
                    "1111111101010100011101001110111110100010010000010001101000011101111011010011101111000111111101000101001"
                    "1111111101010100011111010010110000101111000100111101001100111110100010101111110111000111111101000101001"
                },
        /*201*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABC+\177+GH", 0, 9, 103, 1, "BYTE1",
                    "1111111101010100011111010101111100110101000001100001000001000010001011110101011110000111111101000101001"
                    "1111111101010100011110101000010000110100111100011001100100111100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000100110010111000001101000001011111010101000011110000111111101000101001"
                    "1111111101010100011010111100111110110010000001011001101000101100000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111101001000100001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110001111101010001010000010011110011110101111101100111111101000101001"
                    "1111111101010100011101001110111110111101000100111101101000110111000011010011101111000111111101000101001"
                    "1111111101010100011111101001011100110110001111001001110110111001000010101111110111000111111101000101001"
                    "1111111101010100011010011011111100110110001011110001000000100101111011111010011101000111111101000101001"
                },
        /*202*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABC+\177+GH", 0, 8, 103, 0, "BYTE1; BWIPP same as FAST_MODE",
                    "1111111101010100011111010101111100110101000011000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111101010001110111010011000001001110011000111010011110101001000000111111101000101001"
                    "1111111101010100011101010111111000100111111001110101010000001011110011101010001111110111111101000101001"
                    "1111111101010100011010111100111110111000010010111001111011010000111010101111001111000111111101000101001"
                    "1111111101010100011101011100001100111100001000010101111011011000010011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110100011011111101101001000011111011101011111010000111111101000101001"
                    "1111111101010100011101001110111110110010000100110001110001000100111011010011101111000111111101000101001"
                    "1111111101010100011111010010110000100101111000001001110001101110001010101111110111000111111101000101001"
                },
        /*203*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCD+\177GH", 0, 8, 103, 0, "BYTE1; BWIPP different encodation (A5 B3)",
                    "1111111101010100011111010101111100110101000011000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111101010001110110101111110111101101111101011110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000100111111001110101010000001011110011101010001111110111111101000101001"
                    "1111111101010100011010111100111110111000111010010001110110010001100010101111001111000111111101000101001"
                    "1111111101010100011101011100001100111101111011110101111000010100001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000101110001000011001100111110100001011101011111010000111111101000101001"
                    "1111111101010100011101001110111110101000010000010001100110100000100011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111100010001101101001001111100110010101111110111000111111101000101001"
                },
        /*204*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD+\177GH", 0, 8, 103, 0, "BYTE1; BWIPP different encodation (A5 B3)",
                    "1111111101010100011111010101111100110101000011000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111101010001110110101111110111101101111101011110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000100111111001110101010000001011110011101010001111110111111101000101001"
                    "1111111101010100011010111100111110111000111010010001110110010001100010101111001111000111111101000101001"
                    "1111111101010100011101011100001100111101111011110101111000010100001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000101110001000011001100111110100001011101011111010000111111101000101001"
                    "1111111101010100011101001110111110101000010000010001100110100000100011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111100010001101101001001111100110010101111110111000111111101000101001"
                },
        /*205*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCD\177+GH", 0, 9, 103, 1, "BYTE1",
                    "1111111101010100011111010101111100110101000001100001000001000010001011110101011110000111111101000101001"
                    "1111111101010100011110101000010000110100111100011001100100111100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111001100101111101111110010110010010101000011110000111111101000101001"
                    "1111111101010100011010111100111110100001000011011001101000101100000010101111001111000111111101000101001"
                    "1111111101010100011010111000001000111101001000100001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100100100001111001001000101111000011110101111101100111111101000101001"
                    "1111111101010100011101001110111110100101111011110001000001000100010011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111010110010000001110001011010000010101111110111000111111101000101001"
                    "1111111101010100011010011011111100111111001001101001001110000100110011111010011101000111111101000101001"
                },
        /*206*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD\177+GH", 0, 8, 103, 0, "BYTE1; BWIPP same as FAST_MODE",
                    "1111111101010100011111010101111100110101000011000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111101010001110110101111110111101011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000101000000101111001101111110001101011101010001111110111111101000101001"
                    "1111111101010100011010111100111110111000111010010001110110010001100010101111001111000111111101000101001"
                    "1111111101010100011101011100001100110011000111101001111101110111011011110101110011100111111101000101001"
                    "1111111101010100011110101111010000110001111000101101001110100011000011101011111010000111111101000101001"
                    "1111111101010100011101001110111110110011001100001101010010000100000011010011101111000111111101000101001"
                    "1111111101010100011111010010110000110101111101110001100010001110010010101111110111000111111101000101001"
                },
        /*207*/ { BARCODE_PDF417, -1, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCD+\177+GH", 0, 8, 103, 0, "BYTE1; BWIPP different encodation (A5 B4)",
                    "1111111101010100011111010101111100110101000011000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111101010001110110101111110111101101111101011110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000100111111001110101010000001011110011101010001111110111111101000101001"
                    "1111111101010100011010111100111110111000010010111001111011010000111010101111001111000111111101000101001"
                    "1111111101010100011101011100001100101001111000001001100100111100110011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100011110100001001111010111100100011101011111010000111111101000101001"
                    "1111111101010100011101001110111110100000101100110001010011001100000011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111101000001101101110101110000011010101111110111000111111101000101001"
                },
        /*208*/ { BARCODE_PDF417, -1, UNICODE_MODE, -1, -1, -1, "ABCD+\177+GH", 0, 8, 103, 0, "BYTE1; BWIPP different encodation (A5 B4)",
                    "1111111101010100011111010101111100110101000011000001111010101111000011110101011110000111111101000101001"
                    "1111111101010100011111101010001110110101111110111101101111101011110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000100111111001110101010000001011110011101010001111110111111101000101001"
                    "1111111101010100011010111100111110111000010010111001111011010000111010101111001111000111111101000101001"
                    "1111111101010100011101011100001100101001111000001001100100111100110011110101110011100111111101000101001"
                    "1111111101010100011110101111010000100011110100001001111010111100100011101011111010000111111101000101001"
                    "1111111101010100011101001110111110100000101100110001010011001100000011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111101000001101101110101110000011010101111110111000111111101000101001"
                },
        /*209*/ { BARCODE_PDF417, 29, UNICODE_MODE | FAST_MODE, -1, -1, -1, "￥3149.79", 0, 10, 103, 1, "",
                    "1111111101010100011101010011100000110101101110000001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111110101110111101101111110101110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000110010111111010001110010111111011011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100001100011001001000001001110111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111010111000110001111011011000010011010111000100000111111101000101001"
                    "1111111101010100011110101111010000111011011111100101100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110100100001100001110001101000110011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111010110000010001000101111110111011010111111011110111111101000101001"
                    "1111111101010100011010011011111100111110101110100001001111101110110010100110001111100111111101000101001"
                    "1111111101010100010100011000001100111001000110111101000010110000011011010001100011100111111101000101001"
                },
        /*210*/ { BARCODE_PDF417, 29, UNICODE_MODE, -1, -1, -1, "￥3149.79", 0, 10, 103, 0, "BWIPP same as FAST_MODE",
                    "1111111101010100011101010011100000110101101110000001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111110101110111101011111100100011011111010100110000111111101000101001"
                    "1111111101010100011101010111111000110010111111010001001111110011101011010100011111000111111101000101001"
                    "1111111101010100010101111101111100100101100000110001000001001110111010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111010111000110001111011011000010011010111000100000111111101000101001"
                    "1111111101010100011110101111010000111011011111100101100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000111001101000110001110110001011000011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111110111011101101011111101100111011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110011100001011101001111000100100010100110001111100111111101000101001"
                    "1111111101010100010100011000001100110100000100001101001011110111100011010001100011100111111101000101001"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;
    int last_fast_num_cwds = 0; /* Keep clang-tidy happy */

    char escaped[1024];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %s, %d, %d, %d, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].eci, testUtilInputModeName(data[i].input_mode),
                    data[i].option_1, data[i].option_2, data[i].option_3,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                testUtilEscape(data[i].data, length, escaped, sizeof(escaped));

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, escaped);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, escaped);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, escaped);

                if (ret == 0 && p_ctx->index == -1) {
                    if (i && (data[i - 1].input_mode & FAST_MODE) && !(data[i].input_mode & FAST_MODE)
                            && strcmp(data[i - 1].data, data[i].data) == 0) {
                        int num_cwds;
                        assert_equal(sscanf(symbol->errtxt, "(%d)", &num_cwds), 1, "i:%d num_cwds sscanf != 1 (%s)\n", i, symbol->errtxt);
                        assert_nonzero(last_fast_num_cwds >= num_cwds, "i:%d last_fast_num_cwds %d < num_cwds %d\n", i, last_fast_num_cwds, num_cwds);
                        if (num_cwds < last_fast_num_cwds && (debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) {
                            printf("i:%d diff %d\n", i, num_cwds - last_fast_num_cwds);
                        }
                    }
                    if (data[i].input_mode & FAST_MODE) {
                        assert_equal(sscanf(symbol->errtxt, "(%d)", &last_fast_num_cwds), 1, "i:%d last_fast sscanf != 1 (%s)\n", i, symbol->errtxt);
                    }
                }

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[2710 * 8 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
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
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 8, 103, 1, "Standard example",
                    "1111111101010100011111010101111100110101000011000001111001111001010011110101011110000111111101000101001"
                    "1111111101010100011111101010001110101000011110010001001111110010110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111010100011111101001111110011101011101010001111110111111101000101001"
                    "1111111101010100011010111100111110110110100010000001000011000110010010101111001111000111111101000101001"
                    "1111111101010100011101011100001100101110100011111001111000001001001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111011111000101101001111100001101011101011111010000111111101000101001"
                    "1111111101010100011101001110111110101000111011100001000111011100100011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111000101110110001101011100001000010101111110111000111111101000101001"
                },
        /*  1*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 8, 103, 1, "Standard example",
                    "1111111101010100011111010101111100110101000011000001111001111001010011110101011110000111111101000101001"
                    "1111111101010100011111101010001110101000011110010001001111110010110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111010100011111101001111110011101011101010001111110111111101000101001"
                    "1111111101010100011010111100111110110110100010000001000011000110010010101111001111000111111101000101001"
                    "1111111101010100011101011100001100101110100011111001111000001001001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111011111000101101001111100001101011101011111010000111111101000101001"
                    "1111111101010100011101001110111110101000111011100001000111011100100011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111000101110110001101011100001000010101111110111000111111101000101001"
                },
        /*  2*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 8, 103, 1, "Standard example auto-ECI",
                    "1111111101010100011111010101111100110101000011000001111001111001010011110101011110000111111101000101001"
                    "1111111101010100011111101010001110101000011110010001001111110010110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111010100011111101001111110011101011101010001111110111111101000101001"
                    "1111111101010100011010111100111110110110100010000001000011000110010010101111001111000111111101000101001"
                    "1111111101010100011101011100001100101110100011111001111000001001001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111011111000101101001111100001101011101011111010000111111101000101001"
                    "1111111101010100011101001110111110101000111011100001000111011100100011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111000101110110001101011100001000010101111110111000111111101000101001"
                },
        /*  3*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 8, 103, 1, "Standard example auto-ECI",
                    "1111111101010100011111010101111100110101000011000001111001111001010011110101011110000111111101000101001"
                    "1111111101010100011111101010001110101000011110010001001111110010110011110101001000000111111101000101001"
                    "1111111101010100011101010111111000111010100011111101001111110011101011101010001111110111111101000101001"
                    "1111111101010100011010111100111110110110100010000001000011000110010010101111001111000111111101000101001"
                    "1111111101010100011101011100001100101110100011111001111000001001001011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111011111000101101001111100001101011101011111010000111111101000101001"
                    "1111111101010100011101001110111110101000111011100001000111011100100011010011101111000111111101000101001"
                    "1111111101010100011111010010110000111000101110110001101011100001000010101111110111000111111101000101001"
                },
        /*  4*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 9, 103, 1, "Standard example inverted",
                    "1111111101010100011111010101111100110101000001100001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111111010100011101011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000110010010111110001000011111011001010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111010100111000001111001111001010010101111001111000111111101000101001"
                    "1111111101010100011010111000001000101000011110010001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111000111111010101100111110100010011110101111101100111111101000101001"
                    "1111111101010100011101001110111110111110111010111001111001100101110011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111100100010100001001111101011100010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100111010011000001100011010011110011111010011101000111111101000101001"
                },
        /*  5*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 9, 103, 1, "Standard example inverted",
                    "1111111101010100011111010101111100110101000001100001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111111010100011101011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000110010010111110001000011111011001010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111010100111000001111001111001010010101111001111000111111101000101001"
                    "1111111101010100011010111000001000101000011110010001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111000111111010101100111110100010011110101111101100111111101000101001"
                    "1111111101010100011101001110111110111110111010111001111001100101110011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111100100010100001001111101011100010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100111010011000001100011010011110011111010011101000111111101000101001"
                },
        /*  6*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 9, 103, 1, "Standard example inverted auto-ECI",
                    "1111111101010100011111010101111100110101000001100001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111111010100011101011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000110010010111110001000011111011001010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111010100111000001111001111001010010101111001111000111111101000101001"
                    "1111111101010100011010111000001000101000011110010001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111000111111010101100111110100010011110101111101100111111101000101001"
                    "1111111101010100011101001110111110111110111010111001111001100101110011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111100100010100001001111101011100010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100111010011000001100011010011110011111010011101000111111101000101001"
                },
        /*  7*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 9, 103, 1, "Standard example inverted auto-ECI",
                    "1111111101010100011111010101111100110101000001100001100011100011001011110101011110000111111101000101001"
                    "1111111101010100011110101000010000111111010100011101011111100100011011110101001000000111111101000101001"
                    "1111111101010100011101010111111000110010010111110001000011111011001010101000011110000111111101000101001"
                    "1111111101010100011010111100111110111010100111000001111001111001010010101111001111000111111101000101001"
                    "1111111101010100011010111000001000101000011110010001011111101011000011110101110011100111111101000101001"
                    "1111111101010100011110101111010000111000111111010101100111110100010011110101111101100111111101000101001"
                    "1111111101010100011101001110111110111110111010111001111001100101110011010011101111000111111101000101001"
                    "1111111101010100011111101001011100111100100010100001001111101011100010101111110111000111111101000101001"
                    "1111111101010100011010011011111100100111010011000001100011010011110011111010011101000111111101000101001"
                },
        /*  8*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("$439.97"), -1, 3 }, { TU("￥3149.79"), -1, 29 }, { TU("Produkt:444,90 €"), -1, 17 } }, 0, 10, 137, 0, "AIM ITS/04-023:2022 Annex A example (shortened); BWIPP different encodation (LL AS P instead of AL P LL)",
                    "11111111010101000111010100111000001101011110011111011000111000110010111010100111000001000001000101000011101010011100000111111101000101001"
                    "11111111010101000111110101000110001111110100010111011110110110000100111110010010110001001111110010110011111010100110000111111101000101001"
                    "11111111010101000110101001111100001010111000111111011001111110110010110010111111010001110010111111011011010100011111000111111101000101001"
                    "11111111010101000101011111011111001000011000110010010000010011101110111110101111110101111011000100111010101111101111100111111101000101001"
                    "11111111010101000110101110000100001111100101000011010011111100101100111101011001100001001111010011110011010111000100000111111101000101001"
                    "11111111010101000111101011110010001101111110110100011101110001011110101111000010000101001110000100110011110101111000010111111101000101001"
                    "11111111010101000101001110011100001000100110000110010100000101000000111000100101110001110101110011111010100111001110000111111101000101001"
                    "11111111010101000111101001010000001011111100100011011110100001001000101111110101100001011111101011000011010111111011110111111101000101001"
                    "11111111010101000101001100111110001100011110001011010110001100111100100001100100111001001111001001000010100110001111100111111101000101001"
                    "11111111010101000101000110000011001110111101110011011101110110011000111100111001001101101011110111110010100011000001100111111101000101001"
                },
        /*  9*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("$439.97"), -1, 3 }, { TU("￥3149.79"), -1, 29 }, { TU("Produkt:444,90 €"), -1, 17 } }, 0, 10, 137, 0, "AIM ITS/04-023:2022 Annex A example (shortened); BWIPP different encodation",
                    "11111111010101000111010100111000001101011110011111011000111000110010111010100111000001000001000101000011101010011100000111111101000101001"
                    "11111111010101000111110101000110001111110100010111011110110110000100111110010010110001001111110010110011111010100110000111111101000101001"
                    "11111111010101000110101001111100001010111000111111010011111100111010110010111111010001001111110011101011010100011111000111111101000101001"
                    "11111111010101000101011111011111001001011000001100011010001100011100111010000010011101100110001000010010101111101111100111111101000101001"
                    "11111111010101000110101110000100001111001001100011010011111100101100111101011001100001100010111000100011010111000100000111111101000101001"
                    "11111111010101000111101011110010001110111101000111010100001011110000111001101011111001000111100001001011110101111000010111111101000101001"
                    "11111111010101000101001110011100001010000010100000011010000010000110110010001100011101000001010000100010100111001110000111111101000101001"
                    "11111111010101000111101001010000001011111100100011011110100001001000101111110101100001011111101011000011010111111011110111111101000101001"
                    "11111111010101000101001100111110001100000101011111011011010001111000110001111001001101100000111101011010100110001111100111111101000101001"
                    "11111111010101000101000110000011001101000010000110010111011100010000111001000100011101111001100010111010100011000001100111111101000101001"
                },
        /* 10*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("$39.97"), -1, 3 }, { TU("￥149.79"), -1, 29 }, { TU("Produkt:44,90 €"), -1, 17 } }, 0, 13, 120, 0, "AIM ITS/04-023:2022 Annex A example (shortened), PS pad issue spanning ECIs; BWIPP different encodation (uses PS for 1st pad instead of ML)",
                    "111111110101010001111010100111100010101111001111000110001110001100101110101001110000011111010101111100111111101000101001"
                    "111111110101010001111101010001100010011110110111110111010011100001101110011011100100011111101010011100111111101000101001"
                    "111111110101010001010100111100000011001000100111110100001111101100101010111000111111011010100011111000111111101000101001"
                    "111111110101010001111101011111101010000010000100010100101100001100001001011000001100011010111100111110111111101000101001"
                    "111111110101010001101011100001000010111111010110000111100000100010101111010001000001011101011100011000111111101000101001"
                    "111111110101010001111101011110110011101111010111000100010100011110001000011111011001011110101111000010111111101000101001"
                    "111111110101010001101001110011110011110101100111110100110011000010001001100011010000011101001110111110111111101000101001"
                    "111111110101010001111010010100000010100111100001000110010011110110001100111101011111010101111110011100111111101000101001"
                    "111111110101010001111110100110010010100000100111100101110010001100001111110101110011010100110001111100111111101000101001"
                    "111111110101010001010001110111000011110011110010100100101100000110001000011000110010010100011000011000111111101000101001"
                    "111111110101010001101001110001000010111111010110000111110001100010101100000101110010011101001110011000111111101000101001"
                    "111111110101010001101000100011111011101101011111000100111110110100001001111000110011010100011011111000111111101000101001"
                    "111111110101010001010000010100000010110011100110000101000111001110001001000110110000011010000111011110111111101000101001"
                },
        /* 11*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("$39.97"), -1, 3 }, { TU("￥149.79"), -1, 29 }, { TU("Produkt:44,90 €"), -1, 17 } }, 0, 13, 120, 0, "AIM ITS/04-023:2022 Annex A example (shortened), PS pad issue spanning ECIs; BWIPP different encodation",
                    "111111110101010001111010100111100010101111001111000110001110001100101110101001110000011111010101111100111111101000101001"
                    "111111110101010001111101010001100010011110110111110111010011100001101110011011100100011111101010011100111111101000101001"
                    "111111110101010001010100111100000011001000100111110100001111101100101010111000111111011010100011111000111111101000101001"
                    "111111110101010001111101011111101011110011110010100100101100001100001111001111001010011010111100111110111111101000101001"
                    "111111110101010001101011100001000011110100001001000111100000100010101111010001000001011101011100011000111111101000101001"
                    "111111110101010001111101011110110011101111010111000100010100011110001000011111011001011110101111000010111111101000101001"
                    "111111110101010001101001110011110011110101100111110100110011000010001001100011010000011101001110111110111111101000101001"
                    "111111110101010001111010010100000010100111100001000110010011110110001100111101011111010101111110011100111111101000101001"
                    "111111110101010001111110100110010010100000100111100101110010001100001111110101110011010100110001111100111111101000101001"
                    "111111110101010001010001110111000011110011110010100100101100000110001000011000110010010100011000011000111111101000101001"
                    "111111110101010001101001110001000010111111010110000111110100100011001111000100011011011101001110011000111111101000101001"
                    "111111110101010001101000100011111010111010001100000100011110010001001111010111110110010100011011111000111111101000101001"
                    "111111110101010001010000010100000011011000010001000110001011100111101011100111001000011010000111011110111111101000101001"
                },
        /* 12*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a -128 GB的存储空间-黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a - 128 GB Speicher - Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 23, 188, 0, "AIM ITS/04-023:2022 Annex A example; BWIPP different encodation",
                    "11111111010101000111010100011100001111100101011111011000111000110010111010100111000001100000100101100010011000110100000110100001000011001110100010001110011010100011000000111111101000101001"
                    "11111111010101000111101010000010001111000001101101010000010111110110111101100001100101111010000010001011010001110000100111000011011101001001011111101110011111101010001110111111101000101001"
                    "11111111010101000110101000111110001110010101111110011001111101101100100101100011111001111110101110011010011111000110010110001111010110001111101001110100010101000001111000111111101000101001"
                    "11111111010101000111110100101111101000001010001000011111011010011110110000010100110001001101110000110011000001010011000111001100001001101110111010010000011110100101111000111111101000101001"
                    "11111111010101000110101110000001001111101010001100011010001110000100100111101000111101100000101110100010101111100110000100100111100000101111101001001100011101011100001100111111101000101001"
                    "11111111010101000111101011110000101000000110010111011110000101111010100100010000111101001011000011111010011000011011110110100000101111101011000001001110011110101111100110111111101000101001"
                    "11111111010101000101001111011110001001000111001110011000111000110010101011110111100001000001000010001011101101100111000111011010001100001110010010011100011010011100011110111111101000101001"
                    "11111111010101000111110100100110001110010111100111010111111010110000111100000110110101110100000011001011110110000110010111101000001000101101000111000010011111010010110000111111101000101001"
                    "11111111010101000101001100011111001100111111011001010010010011110000101101110111110001011011011110000010110010000011100110001111100100101000111101100011010100111011111100111111101000101001"
                    "11111111010101000110100011100111101100001101001000011000010001101110110001010110000001000011011110111010000010100000100100000101000100001000100011011000010100011100111000111111101000101001"
                    "11111111010101000110100111000001001111110100101110011000001011101000110100001111011001101111110101110011001001110100000111111011101001001111000101000001011101001110001100111111101000101001"
                    "11111111010101000101000110111110001111110101100010011001110001001110110011010011110001011100011001110011011000001011110100110011000111101011110001010000010100011000111110111111101000101001"
                    "11111111010101000101000001001000001000101000001000011001100000100100110111100111000101010000010100000011011001100001100110010000010001101000001000100100011101000001011100111111101000101001"
                    "11111111010101000111010001101000001111101100101000010111011111101000111100111001110101111010000100100010111111010110000111100011101110101110101110001100011111010001000110111111101000101001"
                    "11111111010101000101000001101111101011000001001110011101101111110010100001111101100101111110101100010010111111001110100110111111011010001110111000101111011001010111110000111111101000101001"
                    "11111111010101000111100101101111101000010110000011011000010100001100111100110010111001100100100110000010001001110111000101100001101000001100000101001100011100101101111000111111101000101001"
                    "11111111010101000101000111110001101111100010100110010001101001111110111100100010000101110000010001101011000001011100100110000010111010001111001101101000010100011111011000111111101000101001"
                    "11111111010101000111110010111101101111101011110110010010000101111000111011111100101001010010000111100011011110010000110100110000001011101001000100001111011110010111110110111111101000101001"
                    "11111111010101000111101101000011101011011100110000010001000010000010110000110010000101000001010001000010011000011010000101001111011110001110011101000100011101101000011000111111101000101001"
                    "11111111010101000111101000001001001100010011110110011011111001011110111000011011101001110001001101000011001011111011100110011110101111101111010001001000010100001111100110111111101000101001"
                    "11111111010101000110010010011111001100101001111100010110001100111100110001111001001101001111110011101011100101111110110101011100111111001100100101111100010110110001111000111111101000101001"
                    "11111111010101000101101110001100001100111000110100010001100110010000111011110110001001111101110100111010000100100000010111000101101111001011100111010000011101101110011110111111101000101001"
                    "11111111010101000111100101000001001100000110111101011101001110001100111100010111011101101101111000010011111000011010010101101111101000001100100001111011011111001010001100111111101000101001"
                },
        /* 13*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a -128 GB的存储空间-黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a - 128 GB Speicher - Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 23, 188, 0, "AIM ITS/04-023:2022 Annex A example; BWIPP different encodation",
                    "11111111010101000111010100011100001111100101011111011000111000110010111010100111000001100000100101100010011000110100000110100001000011001110100010001110011010100011000000111111101000101001"
                    "11111111010101000111101010000010001111000001101101010000010111110110111101100001100101111010000010001011010001110000100111000011011101001001011111101110011111101010001110111111101000101001"
                    "11111111010101000110101000111110001110010101111110011001111101101100100101100011111001111110101110011010011111000110010110001111010110001111101001110100010101000001111000111111101000101001"
                    "11111111010101000111110100101111101000001010001000011111011010011110110000010100110001001101110000110011000001010011000111001100001001101110111010010000011110100101111000111111101000101001"
                    "11111111010101000110101110000001001111101010001100011010001110000100111000001101110101110000010110001011010111101100000111101010010000001011011111000010011101011100001100111111101000101001"
                    "11111111010101000111101011110000101010110011111000011110111101011110110100110011111101000111100001001010001100010111000101000110011111001011110100100000011110101111100110111111101000101001"
                    "11111111010101000101001111011110001101100100010000011000111000110010101011110111100001000001000010001011100100000101110110101100111000001000100100000100011010011100011110111111101000101001"
                    "11111111010101000111110100100110001000010111100100010011100111111010101000011111011001111000010110110011110111001110010110110011110000101100011011110001011111010010110000111111101000101001"
                    "11111111010101000101001100011111001001001001111000010110111011111000101101101111000001011001000001110011000111110010010100011110110001101001111101110110010100111011111100111111101000101001"
                    "11111111010101000110100011100111101100001000110111011000101011000000100001101111011101000001010000010010000010100010000100010001101100001101001111011111010100011100111000111111101000101001"
                    "11111111010101000110100111000001001100000101110100011010000111101100110111111010111001100100111010000011111101110100100111100010100000101111010110011000011101001110001100111111101000101001"
                    "11111111010101000101000110111110001100111000100111011001101001111000101110001100111001101100000101111010011001100011110101111000101000001001100101110000010100011000111110111111101000101001"
                    "11111111010101000101000001001000001100110000010010011011110011100010101000001010000001101100110000110011001000001000110100000100010010001100100000100011011101000001011100111111101000101001"
                    "11111111010101000111010001101000001011101111110100011110011100111010111101000010010001011111101011000011110001110111010111010111000110001111011011000010011111010001000110111111101000101001"
                    "11111111010101000101000001101111101110110111111001010000111110110010111111010110001001011111100111010011011111101101000111011100010111101011110000100001011001010111110000111111101000101001"
                    "11111111010101000111100101101111101100001010000110011110011001011100110010010011000001000100111011100010110000110100000110000010100110001101110110000100011100101101111000111111101000101001"
                    "11111111010101000101000111110001101000110100111111011110010001000010111000001000110101100000101110010011000001011101000111100110110100001111010111001110010100011111011000111111101000101001"
                    "11111111010101000111110010111101101001000010111100011101111110010100101001000011110001101111001000011010011000000101110100100010000111101111001001111001011110010111110110111111101000101001"
                    "11111111010101000111101101000011101000100001000001011100001100010110110000010100110001011100111101100011101100101100000111101011001111101101111001110001011101101000011000111111101000101001"
                    "11111111010101000111101000001001001110101110110000011110001011001100111101000100001001111000100010100010010000111100010111101000100100001111100100001011010100001111100110111111101000101001"
                    "11111111010101000110010010011111001111110101110011010011111100111010111001011111101101100011111001001011000111110010010111011001011111001000111000010110010110110001111000111111101000101001"
                    "11111111010101000101101110001100001101100010000010010100011110111100100000100010000101111011110010001011101000000101110100010001010000001101011001110000011101101110011110111111101000101001"
                    "11111111010101000111100101000001001100011001111010011010011100100000101011111011000001101100001111010010110111110010000110100011110110001110010011000001011111001010001100111111101000101001"
                },
        /* 14*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("\357"), 1, 0 }, { TU("\357"), 1, 7 }, { TU("\357"), 1, 0 } }, 0, 10, 103, 1, "Standard example + extra seg, data mode",
                    "1111111101010100011101010011100000110101101110000001111001111001010011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111100101110011101001111110010110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000111010100011111101001111110011101011010100011111000111111101000101001"
                    "1111111101010100010101111101111100111011001000110001100011100011001010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111110101001100001011111100100011011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101100110011110001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110101000011000001100100011001110011010011101111000111111101000101001"
                    "1111111101010100011110100101000000100001011110000101101011111101111011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110100111111010001001011101111110010100110001111100111111101000101001"
                    "1111111101010100010100011000001100111100100100111101100011001010000011010001100011100111111101000101001"
                },
        /* 15*/ { BARCODE_PDF417, DATA_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("\357"), 1, 0 }, { TU("\357"), 1, 7 }, { TU("\357"), 1, 0 } }, 0, 10, 103, 1, "Standard example + extra seg, data mode",
                    "1111111101010100011101010011100000110101101110000001111001111001010011110101011110000111111101000101001"
                    "1111111101010100011111010100011000111100101110011101001111110010110011111010100110000111111101000101001"
                    "1111111101010100011101010111111000111010100011111101001111110011101011010100011111000111111101000101001"
                    "1111111101010100010101111101111100111011001000110001100011100011001010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111110101001100001011111100100011011010111000100000111111101000101001"
                    "1111111101010100011110101111010000101100110011110001100011111001001011110101111000010111111101000101001"
                    "1111111101010100010100111001110000110101000011000001100100011001110011010011101111000111111101000101001"
                    "1111111101010100011110100101000000100001011110000101101011111101111011010111111011110111111101000101001"
                    "1111111101010100011010011011111100110100111111010001001011101111110010100110001111100111111101000101001"
                    "1111111101010100010100011000001100111100100100111101100011001010000011010001100011100111111101000101001"
                },
        /* 16*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 0 }, { TU("貫やぐ禁"), -1, 20 } }, ZINT_WARN_USES_ECI, 11, 120, 1, "Auto-ECI",
                    "111111110101010001110101001110000011010111001111000110001110001100101010100000100000011111010101111100111111101000101001"
                    "111111110101010001111110101000111011011111101011100111111001010111001111001011000110011111010100110000111111101000101001"
                    "111111110101010001010100111100000010110011101111100111111001000110101000011111011001011101010001111110111111101000101001"
                    "111111110101010001010111110111110011101011011110000100000100001000101001011000110000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011110100001010000111010000001100101001111110010110011010111000100000111111101000101001"
                    "111111110101010001111101011110110011111101011000010110011111101100101011001000001110011101011111010000111111101000101001"
                    "111111110101010001010011100111000011001100001100110101100011110011101100111001110011011101001110111110111111101000101001"
                    "111111110101010001111101001011000011111010001011000110100011101000001111100101001100011010111111011110111111101000101001"
                    "111111110101010001111110100110010011000111110010010100111010011000001110101111100010010100110000111110111111101000101001"
                    "111111110101010001010001100000110011001000010011000110011000110011001011110011110010010100011000011000111111101000101001"
                    "111111110101010001110100111000110011010001111011000110000110111100101101001111000011011010011100100000111111101000101001"
                },
        /* 17*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 0 }, { TU("貫やぐ禁"), -1, 20 } }, ZINT_WARN_USES_ECI, 11, 120, 1, "Auto-ECI",
                    "111111110101010001110101001110000011010111001111000110001110001100101010100000100000011111010101111100111111101000101001"
                    "111111110101010001111110101000111011011111101011100111111001010111001111001011000110011111010100110000111111101000101001"
                    "111111110101010001010100111100000010110011101111100111111001000110101000011111011001011101010001111110111111101000101001"
                    "111111110101010001010111110111110011101011011110000100000100001000101001011000110000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011110100001010000111010000001100101001111110010110011010111000100000111111101000101001"
                    "111111110101010001111101011110110011111101011000010110011111101100101011001000001110011101011111010000111111101000101001"
                    "111111110101010001010011100111000011001100001100110101100011110011101100111001110011011101001110111110111111101000101001"
                    "111111110101010001111101001011000011111010001011000110100011101000001111100101001100011010111111011110111111101000101001"
                    "111111110101010001111110100110010011000111110010010100111010011000001110101111100010010100110000111110111111101000101001"
                    "111111110101010001010001100000110011001000010011000110011000110011001011110011110010010100011000011000111111101000101001"
                    "111111110101010001110100111000110011010001111011000110000110111100101101001111000011011010011100100000111111101000101001"
                },
        /* 18*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("12345678"), -1, 0 }, { TU("ABCDEF"), -1, 4 }, { TU("\001\001\001\001"), -1, 0 } }, 0, 9, 120, 0, "NUM seg TEX seg BYT seg; BWIPP different encodation",
                    "111111110101010001111101010111110011010110001110000100111101111010001010000001010000011111010101111100111111101000101001"
                    "111111110101010001111010100001000011100010000110100111000101110001101001111110010110011110101001000000111111101000101001"
                    "111111110101010001010100111100000011101010011111100110001111100100101110101011111100010101000011110000111111101000101001"
                    "111111110101010001101011110011111010100111001110000110100000101100001100011100011001011010111100111110111111101000101001"
                    "111111110101010001101011100000100011111010100110000110111111010111001111110101011100011110101110011100111111101000101001"
                    "111111110101010001111101011110110011101010111111000111010101111110001110101011111100011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100110011101100000101011011000010000011101001110111110111111101000101001"
                    "111111110101010001111110100101110011110011011100110110110000011110101000001011111011010101111110111000111111101000101001"
                    "111111110101010001111110100110010010011110010010000100000100010111101110001110101111011111010011101000111111101000101001"
                },
        /* 19*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("12345678"), -1, 0 }, { TU("ABCDEF"), -1, 4 }, { TU("\001\001\001\001"), -1, 0 } }, 0, 9, 120, 0, "NUM seg TEX seg BYT seg; BWIPP different encodation",
                    "111111110101010001111101010111110011010110001110000100111101111010001010000001010000011111010101111100111111101000101001"
                    "111111110101010001111010100001000011100010000110100111000101110001101001111110010110011110101001000000111111101000101001"
                    "111111110101010001010100111100000011101010011111100110001111100100101110101011111100010101000011110000111111101000101001"
                    "111111110101010001101011110011111010100111001110000110100000101100001100011100011001011010111100111110111111101000101001"
                    "111111110101010001101011100000100011111010100110000110111111010111001111110101011100011110101110011100111111101000101001"
                    "111111110101010001111101011110110011101010111111000111010101111110001110101011111100011110101111101100111111101000101001"
                    "111111110101010001110100111011111010000110001100100110011101100000101011011000010000011101001110111110111111101000101001"
                    "111111110101010001111110100101110011110011011100110110110000011110101000001011111011010101111110111000111111101000101001"
                    "111111110101010001111110100110010010011110010010000100000100010111101110001110101111011111010011101000111111101000101001"
                },
        /* 20*/ { BARCODE_PDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 2, 4, "017053" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 13 }, { TU("貫やぐ禁"), -1, 20 } }, 0, 11, 120, 1, "Structured Append",
                    "111111110101010001110101001110000011010111001111000110001110001100101010100000100000011111010101111100111111101000101001"
                    "111111110101010001111110101000111011011111101011100111111001010111001111001011000110011111010100110000111111101000101001"
                    "111111110101010001010100111100000010110011101111100111111001000110101000011111011001011101010001111110111111101000101001"
                    "111111110101010001010111110111110011101011011110000100000100001000101001011000110000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011110100001010000111010000001100101001111110010110011010111000100000111111101000101001"
                    "111111110101010001111101011110110011111101011000010110011111101100101011001000001110011101011111010000111111101000101001"
                    "111111110101010001010011100111000011001100001100110101100011110011101100111001110011011101001110111110111111101000101001"
                    "111111110101010001111101001011000011111010001011000110100011101000001111100101001100011010111111011110111111101000101001"
                    "111111110101010001111110100110010011000111110010010100111010011000001110101111100010010100110000111110111111101000101001"
                    "111111110101010001010001100000110011001000010011000110011000110011001011110011110010010100011000011000111111101000101001"
                    "111111110101010001110100111000110011010001111011000110000110111100101101001111000011011010011100100000111111101000101001"
                },
        /* 21*/ { BARCODE_PDF417, UNICODE_MODE, -1, -1, -1, { 2, 4, "017053" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 13 }, { TU("貫やぐ禁"), -1, 20 } }, 0, 11, 120, 1, "Structured Append",
                    "111111110101010001110101001110000011010111001111000110001110001100101010100000100000011111010101111100111111101000101001"
                    "111111110101010001111110101000111011011111101011100111111001010111001111001011000110011111010100110000111111101000101001"
                    "111111110101010001010100111100000010110011101111100111111001000110101000011111011001011101010001111110111111101000101001"
                    "111111110101010001010111110111110011101011011110000100000100001000101001011000110000011010111100111110111111101000101001"
                    "111111110101010001110101110000110011110100001010000111010000001100101001111110010110011010111000100000111111101000101001"
                    "111111110101010001111101011110110011111101011000010110011111101100101011001000001110011101011111010000111111101000101001"
                    "111111110101010001010011100111000011001100001100110101100011110011101100111001110011011101001110111110111111101000101001"
                    "111111110101010001111101001011000011111010001011000110100011101000001111100101001100011010111111011110111111101000101001"
                    "111111110101010001111110100110010011000111110010010100111010011000001110101111100010010100110000111110111111101000101001"
                    "111111110101010001010001100000110011001000010011000110011000110011001011110011110010010100011000011000111111101000101001"
                    "111111110101010001110100111000110011010001111011000110000110111100101101001111000011011010011100100000111111101000101001"
                },
        /* 22*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("¶¶"), -1, 0 }, { TU("ЖЖ"), -1, 7 }, { TU(""), 0, 0 } }, 0, 8, 82, 1, "Standard example (doubled to avoid Byte Shift)",
                    "1100111010100000100001000101000010110110110100010000001101101000100000011001110101"
                    "1110111010100111111001011001000010010111111010100011101101111110101110011101110101"
                    "1110011010110010010111110001000011010110010010111110001100011111001001011100110101"
                    "1111011010100001100011001001000111010100001011101110001100010000010011011110110101"
                    "1111001010111001000011010001000110010110111001111101001101000011101000011110010101"
                    "1110001010101101101111000001000100010100111100001101101011011100011111011100010101"
                    "1100001010111011100010100001001100010101110001110010001100100111011110011000010101"
                    "1100011010101111100100111001001110010111010001100010001111000101000001011000110101"
                },
        /* 23*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("¶¶"), -1, 0 }, { TU("ЖЖ"), -1, 7 }, { TU(""), 0, 0 } }, 0, 8, 82, 1, "Standard example (doubled to avoid Byte Shift)",
                    "1100111010100000100001000101000010110110110100010000001101101000100000011001110101"
                    "1110111010100111111001011001000010010111111010100011101101111110101110011101110101"
                    "1110011010110010010111110001000011010110010010111110001100011111001001011100110101"
                    "1111011010100001100011001001000111010100001011101110001100010000010011011110110101"
                    "1111001010111001000011010001000110010110111001111101001101000011101000011110010101"
                    "1110001010101101101111000001000100010100111100001101101011011100011111011100010101"
                    "1100001010111011100010100001001100010101110001110010001100100111011110011000010101"
                    "1100011010101111100100111001001110010111010001100010001111000101000001011000110101"
                },
        /* 24*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("¶¶"), -1, 0 }, { TU("ЖЖ"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 8, 82, 1, "Standard example auto-ECI",
                    "1100111010100000100001000101000010110110110100010000001101101000100000011001110101"
                    "1110111010100111111001011001000010010111111010100011101101111110101110011101110101"
                    "1110011010110010010111110001000011010110010010111110001100011111001001011100110101"
                    "1111011010100001100011001001000111010100001011101110001100010000010011011110110101"
                    "1111001010111001000011010001000110010110111001111101001101000011101000011110010101"
                    "1110001010101101101111000001000100010100111100001101101011011100011111011100010101"
                    "1100001010111011100010100001001100010101110001110010001100100111011110011000010101"
                    "1100011010101111100100111001001110010111010001100010001111000101000001011000110101"
                },
        /* 25*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("¶¶"), -1, 0 }, { TU("ЖЖ"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 8, 82, 1, "Standard example auto-ECI",
                    "1100111010100000100001000101000010110110110100010000001101101000100000011001110101"
                    "1110111010100111111001011001000010010111111010100011101101111110101110011101110101"
                    "1110011010110010010111110001000011010110010010111110001100011111001001011100110101"
                    "1111011010100001100011001001000111010100001011101110001100010000010011011110110101"
                    "1111001010111001000011010001000110010110111001111101001101000011101000011110010101"
                    "1110001010101101101111000001000100010100111100001101101011011100011111011100010101"
                    "1100001010111011100010100001001100010101110001110010001100100111011110011000010101"
                    "1100011010101111100100111001001110010111010001100010001111000101000001011000110101"
                },
        /* 26*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("ЖЖ"), -1, 7 }, { TU("¶¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 8, 82, 1, "Standard example inverted",
                    "1100111010110001110001100101000010110111010100011100001000001000010001011001110101"
                    "1110111010101000011110010001000010010101000011110010001001111110010110011101110101"
                    "1110011010110101001111100001000011010110011111101100101100100101111100011100110101"
                    "1111011010110110100010000001000111010100000100111011101010000010100000011110110101"
                    "1111001010110111001111101001000110010110111011111010001011111010011100011110010101"
                    "1110001010101100010001110001000100010110111110010000101000000110101110011100010101"
                    "1100001010110100001000110001001100010111010010000111001110001100010011011000010101"
                    "1100011010111110111011000101001110010111011000111000101110100111000110011000110101"
                },
        /* 27*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("ЖЖ"), -1, 7 }, { TU("¶¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 8, 82, 1, "Standard example inverted",
                    "1100111010110001110001100101000010110111010100011100001000001000010001011001110101"
                    "1110111010101000011110010001000010010101000011110010001001111110010110011101110101"
                    "1110011010110101001111100001000011010110011111101100101100100101111100011100110101"
                    "1111011010110110100010000001000111010100000100111011101010000010100000011110110101"
                    "1111001010110111001111101001000110010110111011111010001011111010011100011110010101"
                    "1110001010101100010001110001000100010110111110010000101000000110101110011100010101"
                    "1100001010110100001000110001001100010111010010000111001110001100010011011000010101"
                    "1100011010111110111011000101001110010111011000111000101110100111000110011000110101"
                },
        /* 28*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("ЖЖ"), -1, 0 }, { TU("¶¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 8, 82, 1, "Standard example inverted auto-ECI",
                    "1100111010110001110001100101000010110111010100011100001000001000010001011001110101"
                    "1110111010101000011110010001000010010101000011110010001001111110010110011101110101"
                    "1110011010110101001111100001000011010110011111101100101100100101111100011100110101"
                    "1111011010110110100010000001000111010100000100111011101010000010100000011110110101"
                    "1111001010110111001111101001000110010110111011111010001011111010011100011110010101"
                    "1110001010101100010001110001000100010110111110010000101000000110101110011100010101"
                    "1100001010110100001000110001001100010111010010000111001110001100010011011000010101"
                    "1100011010111110111011000101001110010111011000111000101110100111000110011000110101"
                },
        /* 29*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("ЖЖ"), -1, 0 }, { TU("¶¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 8, 82, 1, "Standard example inverted auto-ECI",
                    "1100111010110001110001100101000010110111010100011100001000001000010001011001110101"
                    "1110111010101000011110010001000010010101000011110010001001111110010110011101110101"
                    "1110011010110101001111100001000011010110011111101100101100100101111100011100110101"
                    "1111011010110110100010000001000111010100000100111011101010000010100000011110110101"
                    "1111001010110111001111101001000110010110111011111010001011111010011100011110010101"
                    "1110001010101100010001110001000100010110111110010000101000000110101110011100010101"
                    "1100001010110100001000110001001100010111010010000111001110001100010011011000010101"
                    "1100011010111110111011000101001110010111011000111000101110100111000110011000110101"
                },
        /* 30*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, 4, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a 128 GB Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a 128 GB 黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a 128 GB Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 44, 99, 0, "AIM ITS/04-023:2022 Annex A example (shortened); BWIPP different encodation",
                    "110010001011000111000110010111010100111000001011000010100001100011001001100000100101100011010001001"
                    "111010001011000100111010000101001111000010001011100010111101001101100001111000001101101011010001101"
                    "111011001010001111001101100101110001100111001011100110100100100001111001111110010110001011010000101"
                    "110011001011000001001011000110010001101110001011100100110100000100011001000001010001000011011000101"
                    "110111001011010001110000010111010110000001001011101100111100000100010101111110100101110011001000101"
                    "110111101011001111101101100100100101111000001001101100110001111010110001110111111010100011101000101"
                    "110011101011111010101111100110110000100001001000101100110101100111000001101110011000010011101100101"
                    "111011101010101111110011100110011110101111101000101000100010011111011001110100111000011011001100101"
                    "111001101010111101001000000110010001001111101001101000100001111101100101010111000111111011011100101"
                    "111101101010000010000100010111011011001110001011101000111011010001100001110010010011100011011110101"
                    "111100101011100101111001110101111110101100001011001000111100000110110101110100000011001011001110101"
                    "111000101010111000110011100100100100001111001011001100111111001011000101100111111011001011101110101"
                    "110000101011011010000100000101101100010000001011000100111101101000111001110110010110000011100110101"
                    "110001101010111111010110000101111001000111101011000110100001101011111101000110100111111011110110101"
                    "110001001011011101001110000100111110011101101010000110100111110110010001100111110110110011110010101"
                    "111001001011010111100111110100100011001100001010001110111000111010010001101001100011100011100010101"
                    "111101001011011111101011100110110111100010001010001100111110100010110001100011001111001011000010101"
                    "111101011010011001000011100110001111000101101010011100110001000101111101000111001011000011000110101"
                    "111101010010100010000010000111101000101111001010011000111101110100110001001011000011000011000100101"
                    "111001010011110100001001000101111110101100001010111000111100011101110101110101110001100011100100101"
                    "111011010010110000010011100111011011111100101010110000100001111101100101111110101100010011110100101"
                    "111010010011000111011000100110000110110110001010010000100110111000110001000010110000011011110101101"
                    "111010011011000100011110110111110110000010101011010000111010000011001001001000011111011011110101001"
                    "111010111010011011001111000100111110110100001001010000100001010111100001110011110010111011100101001"
                    "111010110011000101011000000100001101111011101001011000100000101000001001000001010001000011101101001"
                    "111010100011110101110011100111111011010110001001011100111000111011110101110101111101111011101001001"
                    "110010100011011110010000110101001100001111101011011100111001111010011101100111001001110011101001101"
                    "110010110010000010001101100110000010010110001011011110111011100110011001011001100001000011101011101"
                    "110010111011001111010111110111101000100100001011001110111010001100100001011010011111100011101011001"
                    "110010011011000111100100110100111111001110101001001110111001011111101101100011111001001011101010001"
                    "110011011010000110001100100100001100011001001001101110100001100011001001000011000110010011001010001"
                    "111011011010111111010110000101111110101100001000101110110001000111001001110001100111010011001011001"
                    "111001011011010010111110000111010100011111101000100110100011110000100101011100100110000011001011101"
                    "110001011011101100100011000111000111011011001000110110110001001110111101000011001100010011001001101"
                    "110001010011010001110000100101110011111100101000010110111101001100001101110100000110100011001101101"
                    "110011010010111001110011110100110111001111101000010010101110000001011001000110110001111011101101101"
                    "110111010010010000001001000101100011110111001000011010111001010011100001111101110100111011100101101"
                    "110110010011111010111011110101111110011011101000111010100010011110100001111000001101101011000101101"
                    "110110011010000111100101000100001111000101001000110010111101000011110101011000001011100011000101001"
                    "110110111010000100011000110110111110111101001000100010110010111011110001000010011001100011001101001"
                    "110110110011111011010100000111100100110011001001100010111110101000001101111011000011001011011101001"
                    "110110100011001110010001110101110100000011001001110010101110110011100001000110010000111011011001001"
                    "110100100011101110100000100110101110011110001001111010111101110100001101010000110001100011011001101"
                    "110101100011111010110011100111111010010111001011111010111100100100000101011111100101100011011011101"
                },
        /* 31*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, 4, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a 128 GB Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a 128 GB 黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a 128 GB Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 44, 99, 0, "AIM ITS/04-023:2022 Annex A example (shortened); BWIPP different encodation",
                    "110010001011000111000110010111010100111000001011000010100001100011001001100000100101100011010001001"
                    "111010001011000100111010000101001111000010001011100010111101001101100001111000001101101011010001101"
                    "111011001010001111001101100101110001100111001011100110100100100001111001111110010110001011010000101"
                    "110011001011000001001011000110010001101110001011100100110100000100011001000001010001000011011000101"
                    "110111001011010001110000010111010110000001001011101100111100000100010101111110100101110011001000101"
                    "110111101011001111101101100100100101111000001001101100110001111010110001110111111010100011101000101"
                    "110011101011111010101111100110110000100001001000101100110101100111000001101110011000010011101100101"
                    "111011101010101111110011100110011110101111101000101000100010011111011001110100111000011011001100101"
                    "111001101010111101001000000110010001001111101001101000100001111101100101010111000111111011011100101"
                    "111101101010000010000100010111001000001011101011101000110101100111000001000100100000100011011110101"
                    "111100101010000101111001000100111001111110101011001000101000011111011001111000010110110011001110101"
                    "111000101011000011010111100101110110000111001011001100100001100001011101001001001111000011101110101"
                    "110000101010110110001000000111101101000111001011000100111011001011000001000011000110010011100110101"
                    "110001101010111100100011110100001101011111101011000110100011010011111101111001000100001011110110101"
                    "110001001010011111001110110100111110110010001010000110110011111011011001111101011110110011110010101"
                    "111001001010010001100110000111000111010010001010001110110100110001110001000001000010001011100010101"
                    "111101001011011011110001000111110100010110001010001100110001100111100101100100011100001011000010101"
                    "111101011011000111100010110110001000101111101010011100100011100101100001101001111110100011000110101"
                    "111101010011110100010111100111101110100110001010011000100101100001100001001011000001100011000100101"
                    "111001010010111111010110000111100011101110101010111000111010111000110001111011011000010011100100101"
                    "111011010011101101111110010100001111101100101010110000111111010110001001011111100111010011110100101"
                    "111010010011000011011011000100110111000110001010010000100001011000001101100001010000110011110101101"
                    "111010011011111011000001010111010000011001001011010000100100001111101101001001111100110011110101001"
                    "111010111010011111011010000100001010111100001001010000111001111001011101101110100111000011100101001"
                    "111010110010000110111101110100000101000001001001011000100000101000100001101011110011111011101101001"
                    "111010100011111101101011000111000111011110101001011100111010111110111101111101110110001011101001001"
                    "110010100010100110000111110111001111010011101011011100110011100100111001000000110010111011101001101"
                    "110010110011000001001011000111011100110011001011011110101100110000100001110001110110011011101011101"
                    "110010111011110100010010000111010001100100001011001110101101001111110001110000010111011011101011001"
                    "110010011010011111100111010111001011111101101001001110110001111100100101100011111001001011101010001"
                    "110011011010000110001100100100001100011001001001101110100001100011001001000011000110010011001010001"
                    "111011011010111111010110000101111110101100001000101110101111000010111101111100100110111011001011001"
                    "111001011011101110010011110110011110001001101000100110110001011111101001011101100011100011001011101"
                    "110001011010001010000001000110001100100000101000110110110100001000011001101100100100000011001001101"
                    "110001010011110110110000010111101000010100001000010110100000010011110101100010111101100011001101101"
                    "110011010011100111110010110110001110100111001000010010110011011001111101101100110011111011101101101"
                    "110111010010000010011011000110100011011100001000011010110110010000100001101110011001000011100101101"
                    "110110010011110110000011010100001101111100101000111010110100001111001101101001111000110011000101101"
                    "110110011011011101100111100111111010000110101000110010100100110001111101100111010011100011000101001"
                    "110110111010111000011101000110010111001111001000100010100001001010000001010000011011000011001101001"
                    "110110110011000100000111010111111011010001101001100010100100111110011001010000011110100011011101001"
                    "110110100011110000101111010100111101100011001001110010110001111101010001100100101111100011011001001"
                    "110100100010011011111011110111001110100001001001111010110111011100110001011110001111010011011001101"
                    "110101100010101111110001110111100001100110101011111010111101000010000101011111000010111011011011101"
                },
        /* 32*/ { BARCODE_MICROPDF417, DATA_MODE | FAST_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("\357\357"), 2, 0 }, { TU("\357\357"), 2, 7 }, { TU("\357\357"), 2, 0 } }, 0, 10, 82, 1, "Standard example (doubled) + extra seg, data mode",
                    "1100010010110011111101100101001111010101100110011110001011001100111100011000100101"
                    "1110010010110001110001100101011111010111010100011100001000001000010001011100100101"
                    "1111010010111100101110011101011110010111100101110011101001111110010110011110100101"
                    "1111010110110101001111100001011110110110011111101100101011001100111100011110101101"
                    "1111010100111011001000110001001110110100001100011001001100111011001000011110101001"
                    "1110010100110001000001110101001110100111111011101010001101110111110100011100101001"
                    "1110110100110100010011111001001100100110010100001111101110011111101010011101101001"
                    "1110100100110110000010000101001100110101001000001000001110100000100111011101001001"
                    "1110100110111101000011001101001000110111101001000100001100100111010000011101001101"
                    "1110101110100000101101111101001000010111010011111000101001110111001111011101011101"
                },
        /* 33*/ { BARCODE_MICROPDF417, DATA_MODE, -1, 3, -1, { 0, 0, "" }, { { TU("\357\357"), 2, 0 }, { TU("\357\357"), 2, 7 }, { TU("\357\357"), 2, 0 } }, 0, 10, 82, 1, "Standard example (doubled) + extra seg, data mode",
                    "1100010010110011111101100101001111010101100110011110001011001100111100011000100101"
                    "1110010010110001110001100101011111010111010100011100001000001000010001011100100101"
                    "1111010010111100101110011101011110010111100101110011101001111110010110011110100101"
                    "1111010110110101001111100001011110110110011111101100101011001100111100011110101101"
                    "1111010100111011001000110001001110110100001100011001001100111011001000011110101001"
                    "1110010100110001000001110101001110100111111011101010001101110111110100011100101001"
                    "1110110100110100010011111001001100100110010100001111101110011111101010011101101001"
                    "1110100100110110000010000101001100110101001000001000001110100000100111011101001001"
                    "1110100110111101000011001101001000110111101001000100001100100111010000011101001101"
                    "1110101110100000101101111101001000010111010011111000101001110111001111011101011101"
                },
        /* 34*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 0 }, { TU("貫やぐ禁"), -1, 20 } }, ZINT_WARN_USES_ECI, 17, 55, 1, "Auto-ECI",
                    "1100110100100001111101100101101010000111110011001101001"
                    "1101110100100000100001000101100100110011100011011101001"
                    "1101100100111100101100011001100101111011000011011001001"
                    "1101100110111111001000110101000011111011001011011001101"
                    "1101101110111010110111100001000001000010001011011011101"
                    "1101101100101000111111011101111010000101000011011011001"
                    "1101101000100100111011111101000011111011001011011010001"
                    "1101001000101011000011000001000001000010001011010010001"
                    "1101011000111001011000001001111011101110100011010110001"
                    "1101011100110011010011110001101111100010001011010111001"
                    "1101011110110100001110111101110100000010111011010111101"
                    "1101001110111110010100110001011111101011000011010011101"
                    "1101001100101000011001111101000000110100111011010011001"
                    "1101000100110100110111000001000010000100100011010001001"
                    "1101000110111001011110011101101101111001000011010001101"
                    "1101000010110000011010111101100110010001111011010000101"
                    "1101100010100001001101100001001001110111000011011000101"
                },
        /* 35*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 0 }, { TU("貫やぐ禁"), -1, 20 } }, ZINT_WARN_USES_ECI, 17, 55, 1, "Auto-ECI",
                    "1100110100100001111101100101101010000111110011001101001"
                    "1101110100100000100001000101100100110011100011011101001"
                    "1101100100111100101100011001100101111011000011011001001"
                    "1101100110111111001000110101000011111011001011011001101"
                    "1101101110111010110111100001000001000010001011011011101"
                    "1101101100101000111111011101111010000101000011011011001"
                    "1101101000100100111011111101000011111011001011011010001"
                    "1101001000101011000011000001000001000010001011010010001"
                    "1101011000111001011000001001111011101110100011010110001"
                    "1101011100110011010011110001101111100010001011010111001"
                    "1101011110110100001110111101110100000010111011010111101"
                    "1101001110111110010100110001011111101011000011010011101"
                    "1101001100101000011001111101000000110100111011010011001"
                    "1101000100110100110111000001000010000100100011010001001"
                    "1101000110111001011110011101101101111001000011010001101"
                    "1101000010110000011010111101100110010001111011010000101"
                    "1101100010100001001101100001001001110111000011011000101"
                },
        /* 36*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("ABCDE"), -1, 0 }, { TU("fghij"), -1, 17 }, { TU(""), 0, 0 } }, 0, 17, 38, 0, "Pad spanning ECI; BWIPP uses LL (better) instead of ML",
                    "11001101001100011111001001011001101001"
                    "11011101001111010101111000011011101001"
                    "11011001001101011111101111011011001001"
                    "11011001101001011000011111011011001101"
                    "11011011101100011100011001011011011101"
                    "11011011001111010110011000011011011001"
                    "11011010001000011100010110011011010001"
                    "11010010001111011010000111011010010001"
                    "11010110001001011110010000011010110001"
                    "11010111001100011111001001011010111001"
                    "11010111101000111011100010011010111101"
                    "11010011101111010001100110011010011101"
                    "11010011001100110000101111011010011001"
                    "11010001001101110011001000011010001001"
                    "11010001101000101111100011011010001101"
                    "11010000101001111010000100011010000101"
                    "11011000101100110100100000011011000101"
                },
        /* 37*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("ABCDE"), -1, 0 }, { TU("fghij"), -1, 17 }, { TU(""), 0, 0 } }, 0, 17, 38, 0, "Pad spanning ECI; BWIPP uses LL (better) instead of ML",
                    "11001101001100011111001001011001101001"
                    "11011101001111010101111000011011101001"
                    "11011001001101011111101111011011001001"
                    "11011001101001011000011111011011001101"
                    "11011011101100011100011001011011011101"
                    "11011011001111010110011000011011011001"
                    "11011010001000011100010110011011010001"
                    "11010010001111011010000111011010010001"
                    "11010110001001011110010000011010110001"
                    "11010111001100011111001001011010111001"
                    "11010111101000111011100010011010111101"
                    "11010011101111010001100110011010011101"
                    "11010011001100110000101111011010011001"
                    "11010001001101110011001000011010001001"
                    "11010001101000101111100011011010001101"
                    "11010000101001111010000100011010000101"
                    "11011000101100110100100000011011000101"
                },
        /* 38*/ { BARCODE_MICROPDF417, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 3, 4, "017053" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 13 }, { TU("貫やぐ禁"), -1, 20 } }, 0, 17, 55, 1, "Structured Append",
                    "1100110100100001111101100101101010000111110011001101001"
                    "1101110100100000100001000101100100110011100011011101001"
                    "1101100100111100101100011001100101111011000011011001001"
                    "1101100110111111001000110101000011111011001011011001101"
                    "1101101110111010110111100001000001000010001011011011101"
                    "1101101100101000111111011101111010000101000011011011001"
                    "1101101000100100111011111101000011111011001011011010001"
                    "1101001000101011000011000001000001000010001011010010001"
                    "1101011000111001011000001001111011101110100011010110001"
                    "1101011100110011010011110001101111100010001011010111001"
                    "1101011110110100001110111101110100000010111011010111101"
                    "1101001110111110010100110001011111101011000011010011101"
                    "1101001100101000011001111101000000110100111011010011001"
                    "1101000100110100110111000001000010000100100011010001001"
                    "1101000110111001011110011101101101111001000011010001101"
                    "1101000010110000011010111101100110010001111011010000101"
                    "1101100010100001001101100001001001110111000011011000101"
                },
        /* 39*/ { BARCODE_MICROPDF417, UNICODE_MODE, -1, -1, -1, { 3, 4, "017053" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 13 }, { TU("貫やぐ禁"), -1, 20 } }, 0, 17, 55, 1, "Structured Append",
                    "1100110100100001111101100101101010000111110011001101001"
                    "1101110100100000100001000101100100110011100011011101001"
                    "1101100100111100101100011001100101111011000011011001001"
                    "1101100110111111001000110101000011111011001011011001101"
                    "1101101110111010110111100001000001000010001011011011101"
                    "1101101100101000111111011101111010000101000011011011001"
                    "1101101000100100111011111101000011111011001011011010001"
                    "1101001000101011000011000001000001000010001011010010001"
                    "1101011000111001011000001001111011101110100011010110001"
                    "1101011100110011010011110001101111100010001011010111001"
                    "1101011110110100001110111101110100000010111011010111101"
                    "1101001110111110010100110001011111101011000011010011101"
                    "1101001100101000011001111101000000110100111011010011001"
                    "1101000100110100110111000001000010000100100011010001001"
                    "1101000110111001011110011101101101111001000011010001101"
                    "1101000010110000011010111101100110010001111011010000101"
                    "1101100010100001001101100001001001110111000011011000101"
                },
        /* 40*/ { BARCODE_PDF417COMP, UNICODE_MODE | FAST_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 8, 69, 1, "Standard example",
                    "111111110101010001111101010111110011010100001100000111100111100101001"
                    "111111110101010001111110101000111010100001111001000100111111001011001"
                    "111111110101010001110101011111100011101010001111110100111111001110101"
                    "111111110101010001101011110011111011011010001000000100001100011001001"
                    "111111110101010001110101110000110010111010001111100111100000100100101"
                    "111111110101010001111010111101000011101111100010110100111110000110101"
                    "111111110101010001110100111011111010100011101110000100011101110010001"
                    "111111110101010001111101001011000011100010111011000110101110000100001"
                },
        /* 41*/ { BARCODE_PDF417COMP, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 8, 69, 1, "Standard example",
                    "111111110101010001111101010111110011010100001100000111100111100101001"
                    "111111110101010001111110101000111010100001111001000100111111001011001"
                    "111111110101010001110101011111100011101010001111110100111111001110101"
                    "111111110101010001101011110011111011011010001000000100001100011001001"
                    "111111110101010001110101110000110010111010001111100111100000100100101"
                    "111111110101010001111010111101000011101111100010110100111110000110101"
                    "111111110101010001110100111011111010100011101110000100011101110010001"
                    "111111110101010001111101001011000011100010111011000110101110000100001"
                },
        /* 42*/ { BARCODE_HIBC_PDF, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("H123ABC"), -1, 0 }, { TU("012345678"), -1, 0 }, { TU("90D"), -1, 20 } }, ZINT_ERROR_INVALID_OPTION, 0, 0, 1, "HIBC",
                    ""
                },
        /* 43*/ { BARCODE_HIBC_MICPDF, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("H123ABC"), -1, 0 }, { TU("012345678"), -1, 0 }, { TU("90D"), -1, 20 } }, ZINT_ERROR_INVALID_OPTION, 0, 0, 1, "HIBC",
                    ""
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;
    int last_fast_num_cwds = 0; /* Keep clang-tidy happy */

    char escaped[1024];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_encode_segs", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/,
                    data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, NULL, 0, debug);
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            char escaped1[4096];
            char escaped2[4096];
            int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
            int length1 = data[i].segs[1].length == -1 ? (int) ustrlen(data[i].segs[1].source) : data[i].segs[1].length;
            int length2 = data[i].segs[2].length == -1 ? (int) ustrlen(data[i].segs[2].source) : data[i].segs[2].length;
            printf("        /*%3d*/ { %s, %s, %d, %d, %d, { %d, %d, \"%s\" }, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                    data[i].option_1, data[i].option_2, data[i].option_3,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            if (data[i].ret < ZINT_ERROR) {
                testUtilModulesPrint(symbol, "                    ", "\n");
            } else {
                printf("                    \"\"\n");
            }
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

                if (data[i].ret == 0 && p_ctx->index == -1) {
                    if (i && (data[i - 1].input_mode & FAST_MODE) && !(data[i].input_mode & FAST_MODE)) {
                        int num_cwds;
                        assert_equal(sscanf(symbol->errtxt, "(%d)", &num_cwds), 1, "i:%d num_cwds sscanf != 1 (%s)\n", i, symbol->errtxt);
                        assert_nonzero(last_fast_num_cwds >= num_cwds, "i:%d last_fast_num_cwds %d < num_cwds %d\n", i, last_fast_num_cwds, num_cwds);
                        if (num_cwds < last_fast_num_cwds && (debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) {
                            printf("i:%d diff %d\n", i, num_cwds - last_fast_num_cwds);
                        }
                    }
                    if (data[i].input_mode & FAST_MODE) {
                        assert_equal(sscanf(symbol->errtxt, "(%d)", &last_fast_num_cwds), 1, "i:%d last_fast sscanf != 1 (%s)\n", i, symbol->errtxt);
                    }
                }

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwippSegs(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].segs, seg_count, NULL, cmp_buf, sizeof(cmp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwippSegs ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, (const char *) data[i].segs[0].source, data[i].segs[0].length, debug)) {
                    if ((data[i].input_mode & 0x07) == DATA_MODE) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) {
                            printf("i:%d multiple segments in DATA_MODE not currently supported for ZXing-C++ testing (%s)\n",
                                    i, testUtilBarcodeName(symbol->symbology));
                        }
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[2710 * 8 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, (const char *) data[i].segs[0].source, data[i].segs[0].length,
                                modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmpSegs(symbol, cmp_msg, cmp_buf, cmp_len, data[i].segs, seg_count,
                                NULL /*primary*/, escaped, &ret_len);
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

/* #181 Nico Gunkel OSS-Fuzz and #300 Andre Maute */
static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int length;
        int ret;
        int bwipp_cmp;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, -1, -1,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, ZINT_ERROR_TOO_LONG, 1, ""
                }, /* Original OSS-Fuzz triggering data */
        /*  1*/ { BARCODE_PDF417, DATA_MODE, -1, -1,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, 0, 0, "BWIPP different encodation"
                }, /* Original OSS-Fuzz triggering data */
        /*  2*/ { BARCODE_PDF417COMP, DATA_MODE | FAST_MODE, -1, -1,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /*  3*/ { BARCODE_PDF417COMP, DATA_MODE, -1, -1,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, 0, 0, "BWIPP different encodation"
                },
        /*  4*/ { BARCODE_MICROPDF417, DATA_MODE | FAST_MODE, -1, -1,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /*  5*/ { BARCODE_MICROPDF417, DATA_MODE, -1, -1,
                    "\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075"
                    "\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\073\060\025\060\075\060\204\060\103\204\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\110\060\160\060\075\060\075\204\060"
                    "\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\073\060\074\060\075"
                    "\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\120\214\060\204\060\074\204\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041"
                    "\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\120\214\060\204\060\075\060\204\060\075\204\060\204\060\204\075\060\075\060"
                    "\204\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060"
                    "\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\075\060\103\204\060\214\060\204\060\075\060"
                    "\200\060\204\075\060\075\060\204\060\000\075\060\226\060\100\204\060\204\060\075\204\060\204\060\204\075\060\075\060\204\060\134\060\075\204\060\040\060\103\204"
                    "\060\372\120\060\124\060\004\060\103\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\000\060\113\060\377\060\235\060\075\060"
                    "\204\060\103\204\060\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\211\060\214\060\204\060\075\060"
                    "\041\060\204\060\204\060\120\060\075\060\204\060\003\060\103\204\060\204\060\377\060\350\060\223\060\000\075\060\226\060\103\204\060\204\060\204\120\060\075\060"
                    "\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120"
                    "\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\031\060\155\060\000\075\060\226\060\100\204\060\204\060"
                    "\204\060\075\204\060\120\214\060\204\060\074\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\000\060\204\060\120\214\060\204\060\074\204"
                    "\060\377\060\075\204\060\075\060\103\204\060\214\060\204\060\075\060\041\060\204\060\204\060\126\060\075\060\204\060\177\060\103\204\060\204\060\377\060\262\060"
                    "\000\075\060\226\060\100\204\060\204\060\204\075\060\073\060\204\060\000\075\060\226\060\100\060\103\204\060\204\060\075\204\060\204\060\204\075\060\110\060\160"
                    "\060\075\060\075\204\060\075\060\103\204\060\204\060\372\120\060\124\060\000\060\204\060\074\204\060\204\060\075\204\060\075\060\103\204\060\214\060\204\060\075"
                    "\060\073\060\074\060\075\060\204\060\103\204\060\075\060\204\060\204\060\075\204\060\075\214\060\204\060\074\204\060\204\060\075\204\060\075\060\103\214\060\214"
                    "\060\204\060\075\060\041\060\000\060\000\060\200\060\204\060\214\060\204\060\075\060\141\060\000\060\204\060\075\214\060\204\060\075\060\204\060\075\204\060\204"
                    "\060\204\041\060\075\060\204\060\075\060\075\204\060\075\060\103\204\060\204\060\003\120\060\075\060\004\060\204\060\074\204\060\204\060\075\204\060\075\060\103"
                    "\204\060\214\060\204\060\075\060\073\060\075\060\204\060\103\204\060\075\060\204\060\204\060\122\060\000\060\075\060\000\076\060\100\000\060\004\060\103\204\060"
                    "\204\060\003\060\204\075\060\120\214\060\204\060\004\060\103\204\060\204\060\003\060\211\074\060\120\060\124\060\351\060\120\060\075\060\351\060\072\375\060\204\060",
                    1001, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /*  6*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, 0, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "1234567890",
                    2710, 0, 0, "BWIPP different encodation"
                }, /* Max numerics with ECC 0 */
        /*  7*/ { BARCODE_PDF417, DATA_MODE, 0, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "1234567890",
                    2710, 0, 0, "BWIPP different encodation"
                }, /* Max numerics with ECC 0 */
        /*  8*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, 0, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "12345678901",
                    2711, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /*  9*/ { BARCODE_PDF417, DATA_MODE, 0, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "12345678901",
                    2711, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 10*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, -1, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
                    2528, 0, 0, "BWIPP different encodation"
                }, /* Max numerics with ECC 5 */
        /* 11*/ { BARCODE_PDF417, DATA_MODE, -1, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678",
                    2528, 0, 0, "BWIPP different encodation"
                }, /* Max numerics with ECC 5 */
        /* 12*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, 0, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCD",
                    1850, 0, 0, "BWIPP different encodation"
                }, /* Max text with ECC 0 */
        /* 13*/ { BARCODE_PDF417, DATA_MODE, 0, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCD",
                    1850, 0, 0, "BWIPP different encodation"
                }, /* Max text with ECC 0 */
        /* 14*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, 0, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG",
                    1853, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 15*/ { BARCODE_PDF417, DATA_MODE, 0, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG",
                    1853, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 16*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, 0, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    1108, 0, 0, "BWIPP different encodation"
                }, /* Max bytes with ECC 0 */
        /* 17*/ { BARCODE_PDF417, DATA_MODE, 0, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    1108, 0, 0, "BWIPP different encodation"
                }, /* Max bytes with ECC 0 */
        /* 18*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, 0, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    1111, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 19*/ { BARCODE_PDF417, DATA_MODE, 0, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    1111, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 20*/ { BARCODE_MICROPDF417, DATA_MODE | FAST_MODE, -1, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456",
                    366, 0, 1, ""
                }, /* Max numerics */
        /* 21*/ { BARCODE_MICROPDF417, DATA_MODE, -1, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456",
                    366, 0, 1, ""
                }, /* Max numerics */
        /* 22*/ { BARCODE_MICROPDF417, DATA_MODE | FAST_MODE, -1, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "1234567890123456789012345678901234567890123456789012345678901234567",
                    367, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 23*/ { BARCODE_MICROPDF417, DATA_MODE, -1, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "1234567890123456789012345678901234567890123456789012345678901234567",
                    367, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 24*/ { BARCODE_MICROPDF417, DATA_MODE | FAST_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOP",
                    250, 0, 1, ""
                }, /* Max text */
        /* 25*/ { BARCODE_MICROPDF417, DATA_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOP",
                    250, 0, 1, ""
                }, /* Max text */
        /* 26*/ { BARCODE_MICROPDF417, DATA_MODE | FAST_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ",
                    251, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 27*/ { BARCODE_MICROPDF417, DATA_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ",
                    251, ZINT_ERROR_TOO_LONG, 1, ""
                },
        /* 28*/ { BARCODE_PDF417COMP, DATA_MODE | FAST_MODE, 0, -1,
                    "\000\000\000\377\377\010\002\000\000\033\005\031\000\000\002\000\000\000\000\101\101\101\101\101\101\101\101\000\000\000\000\000\000\000\374\000\101\101\101\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\323\000\000\000\000\000\010\000\000\000\000\165\000\000\000\000\000\000\000\000\000\000\000\000\056"
                    "\000\000\000\000\000\000\000\000\000\000\100\000\000\101\101\101\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\323\000\000\000\000\000\010\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\323\000\000\000\000\000\010\000\000\000\000\165\000\000\000\000\000\000\000\000\000\000\000\000\056\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\101\101\101\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\323\000\010\000\000\000\010\000\000\000\000\165\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\056\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\377\377\377\377\377\377\377\375\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\040\000\000\000\000\000\000\000\000\000\000\101\101\101\101\101\101\101\101\101\101\101\055\101\101\101\101\101\101\101\101\101"
                    "\101\101\101\060\151\003\000\000\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\137\101\101\101\101\101\101\101\101\000\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\101\101\101\101\101\101\101\101\101\101\000\000\000\000\000\000\000\000\000\070\000\000\000\000\000\000\000"
                    "\000\000\000\377\377\010\002\000\000\033\005\031\000\000\002\000\000\000\000\101\101\101\101\101\101\101\101\000\000\000\000\000\000\000\374\000\101\101\101\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\323\000\000\000\000\000\010\000\000\000\000\165\000\000\000\000\000\000\000\000\000\000\000\000\056"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\377\377\377\377\377\377\377\375\000\000\000\000\000\000\000\000\000\000\000\000\000\040\000\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\377\377\377\377\377\377\377\375\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\040\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\101\101\101\101\101\101\101\101\101\101\101\101\101\055\101\101\101\101\101\101\101\101\101"
                    "\101\101\101\060\151\003\000\000\101\101\101\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\373\377\377\377\377\060"
                    "\060\060\060\060\060\060\060\060\060\060\060\051\060\060\060\060\060\060\377\377\377\377\000\000\000\000\377\161\000\151\151\250\122\141\012\377\377\021\021\021"
                    "\021\021\021\161\021\000\324\324\324\324\324\324\324\324\324\324\324\000\000\000\000\112\000\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324"
                    "\320\324\324\324\324\021\176\012\000\000\000\000\000\000\324\324\324\324\324\324\324\101\101\101\101\101\101\101\352\352\352\352\352\352\352\352\352\352\352\352"
                    "\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\352\000"
                    "\000\000\000\000\101\101\101\101\101\101\101\101\101\101\101\101\101\137\101\101\101\101\101\101\101\101\000\000\000\000\000\000\000\000\000\000\000\000\101\101"
                    "\101\101\041\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\324\324\324\324\324\324\324\324\324\077\324\324\324\324\324\324\324\324\324\324"
                    "\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\324\000\060\060\060\060\060"
                    "\060\060\060\060\060\060\060\060\060\060\060\051\060\060\060\060\060\060\377\377\377\377\000\000\000\000\377\161\000\151\151\250\122\141\012\377\377\021\021\021"
                    "\021\021\021\161\021\000\324\324\324\324\324\324\324\324\324\324\324\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\101\137\101\101\101\101\101"
                    "\101\000\000\000\000\374\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\323\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\323\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000",
                    1048, 0, 0, "BWIPP different encodation"
                }, /* #300 (#1) Andre Maute (`pdf_text_submode_length()` not checking if previous/next BYT) */
        /* 29*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, -1, -1,
                    "\060\060\060\060\060\060\060\060\060\060\060\162\162\162\162\162\162\162\162\162\162\047\122\162\000\000\167\211\206\001\000\047\153\153\153\153\153\067\066\164"
                    "\060\060\060\060\060\060\060\060\060\060\060\162\162\162\162\162\162\162\162\162\162\047\122\162\000\000\167\211\206\001\000\047\153\153\153\153\153\153\153\164"
                    "\164\164\164\164\164\124\164\164\164\164\164\164\164\164\164\164\164\164\164\164\060\060\060\060\060\060\060\060\060\060\060\162\162\162\162\162\162\162\162\162"
                    "\162\047\122\162\162\162\162\162\162\162\167\167\167\162\162\162\162\047\122\162\000\000\167\167\167\001\152\152\152\152\152\152\051\050\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\107\107\107\107\107\107\107\107"
                    "\107\107\162\107\107\107\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\152\152\152\152\152"
                    "\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152"
                    "\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050"
                    "\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051"
                    "\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152"
                    "\152\152\051\050\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152"
                    "\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051"
                    "\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107"
                    "\107\107\107\107\152\051\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152"
                    "\152\152\152\152\152\107\107\107\107\107\152\051\051\051\152\051\050\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\107\107\107\152"
                    "\051\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152"
                    "\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\050\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051"
                    "\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152"
                    "\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\050\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\107\107\107\152\051"
                    "\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051"
                    "\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107"
                    "\107\107\107\107\152\051\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\050\152"
                    "\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152"
                    "\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\050\051\051\051\051\050\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051"
                    "\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152"
                    "\152\152\152\152\152\152\152\152\107\107\107\107\107\152\051\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\000\124\164\164\162\162\162\047\122\162\162"
                    "\162\162\001\100\167\167\001\044\204\167\167",
                    2611, ZINT_ERROR_TOO_LONG, 1, ""
                }, /* #300 (#7) Andre Maute */
        /* 30*/ { BARCODE_PDF417, DATA_MODE, -1, -1,
                    "\060\060\060\060\060\060\060\060\060\060\060\162\162\162\162\162\162\162\162\162\162\047\122\162\000\000\167\211\206\001\000\047\153\153\153\153\153\067\066\164"
                    "\060\060\060\060\060\060\060\060\060\060\060\162\162\162\162\162\162\162\162\162\162\047\122\162\000\000\167\211\206\001\000\047\153\153\153\153\153\153\153\164"
                    "\164\164\164\164\164\124\164\164\164\164\164\164\164\164\164\164\164\164\164\164\060\060\060\060\060\060\060\060\060\060\060\162\162\162\162\162\162\162\162\162"
                    "\162\047\122\162\162\162\162\162\162\162\167\167\167\162\162\162\162\047\122\162\000\000\167\167\167\001\152\152\152\152\152\152\051\050\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\107\107\107\107\107\107\107\107"
                    "\107\107\162\107\107\107\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\152\152\152\152\152"
                    "\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152"
                    "\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050"
                    "\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051"
                    "\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152"
                    "\152\152\051\050\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152"
                    "\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051"
                    "\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107"
                    "\107\107\107\107\152\051\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152"
                    "\152\152\152\152\152\107\107\107\107\107\152\051\051\051\152\051\050\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\107\107\107\152"
                    "\051\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152"
                    "\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\050\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051"
                    "\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152"
                    "\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\050\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\107\107\107\152\051"
                    "\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051"
                    "\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107"
                    "\107\107\107\107\152\051\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\051\051\051\050\152"
                    "\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152"
                    "\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\050\051\051\051\051\050\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051"
                    "\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152"
                    "\152\152\152\152\152\152\152\152\107\107\107\107\107\152\051\051\051\051\107\107\107\107\107\107\107\107\107\107\107\051\051\051\051\051\051\050\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\152\152\152\152\152\152\152\152\152\152\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051"
                    "\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\051\051\050\152\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\152\152\051\051\051\051\051\051\051\051\051"
                    "\051\051\051\051\107\107\051\051\051\051\051\051\051\051\051\051\051\051\050\051\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051"
                    "\051\051\050\152\152\152\152\152\152\152\152\051\050\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\051\000\124\164\164\162\162\162\047\122\162\162"
                    "\162\162\001\100\167\167\001\044\204\167\167",
                    2611, ZINT_ERROR_TOO_LONG, 1, ""
                }, /* #300 (#7) Andre Maute !FAST_MODE */
        /* 31*/ { BARCODE_PDF417, DATA_MODE | FAST_MODE, -1, 242,
                    "\000\000\000\000\000\000\000\000\000\000\000\212\377\000\000\153\153\153\153\153\153\153\060\047\047\043\047\057\153\000\153\153\137\377\153\153\000\134\000\000"
                    "\000\153\153\343\153\153\153\060\047\047\043\047\057\157\153\153\153\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\377\377\377\167"
                    "\001\100\000\377\004\002\000\000\000\000\000\001\000\000\134\077\162\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162"
                    "\077\162\072\176\000\162\362\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\364\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362"
                    "\134\162\162\162\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077"
                    "\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\162\162\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\377\134\134\126\142\134"
                    "\162\362\162\162\162\072\176\000\162\134\134\126\142\134\162\362\162\162\162\072\176\000\162\162\162\162\162\174\174\377\134\362\162\362\162\162\162\072\176\000"
                    "\162\362\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000\000\044\162\162\162\162\174\174\377\134\362\162\162\162\134\134\134\142\162"
                    "\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000"
                    "\162\162\377\377\377\377\377\377\134\134\134\142\134\162\153\153\153\153\153\153\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134"
                    "\134\134\142\134\162\362\162\162\162\072\176\000\162\000\001\000\000\134\077\162\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162"
                    "\162\162\162\077\162\072\176\000\162\362\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\364\072\176\000\162\162\162\162\162\174\174\377\134\134\134"
                    "\162\142\362\134\162\162\162\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162"
                    "\162\162\077\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\162\162\072"
                    "\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\377\134\134\126\142\134\162\362\162\162\162\072\176\000\174\174\377\134\362\162\362\162\162\162"
                    "\072\176\000\162\362\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000\162\362\162\162\162\134\134\134\142\134\162\162\162\162\077\173"
                    "\153\153\153\165\000\000\000\153\151\153\153\153\153\153\153\153\153\153\153\153\153\153\153\176\000\162\362\377\377\377\377\377\377\134\134\134\142\134\162\362"
                    "\162\162\162\072\176\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\362"
                    "\162\162\162\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\377\134\134\134\167\167\167\167\167\001\100\000\002\000\000\000\000\000\000\000"
                    "\000\153\153\067\000\000\000\153\153\300\000\000\000\000\000\000\000\000\000\000\000\000\212\377\000\000\153\153\153\153\153\153\153\060\047\047\043\047\057\153"
                    "\000\153\153\137\377\153\153\000\134\000\000\000\153\153\343\153\153\153\060\047\047\043\047\057\157\153\153\153\162\162\077\162\072\176\000\162\134\142\134\162"
                    "\162\162\162\077\162\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\377\377\377\167\001\100\000\377\004\002\000\000\000\000"
                    "\000\001\000\000\134\077\162\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\362\377\377\377"
                    "\377\377\377\134\134\134\142\134\162\362\162\162\364\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000\215\215\162"
                    "\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377"
                    "\377\377\134\134\134\162\162\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\377\134\134\126\142\134\162\362\162\162\162\072\176\000\162\162"
                    "\162\162\162\174\174\377\134\362\162\362\162\162\162\072\176\000\162\362\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000\000\044\162"
                    "\162\162\162\174\174\377\134\362\162\162\162\134\134\134\142\162\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377"
                    "\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\153\153\153\153\153\153\142\134\162\162"
                    "\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\162\072\176\000\162\000\001\000\000\134\077\162\072\176\000\162"
                    "\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\362\377\377\377\377\377\377\134\134\134\142\134\162\362\162"
                    "\162\364\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162"
                    "\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\162\162\072\176\000\162"
                    "\162\162\162\162\174\174\377\134\134\134\142\134\162\362\162\162\162\072\176\000\162\162\162\162\162\174\174\377\134\362\162\172\162\134\134\134\142\162\072\162"
                    "\162\162\162\174\174\377\134\362\162\362\162\162\162\072\176\000\162\362\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000\000\044\162"
                    "\162\162\162\174\174\377\134\362\162\162\162\134\134\134\142\162\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377"
                    "\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\153\153\153\153\153\153\142\134\162\162"
                    "\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\162\072\176\000\162\000\001\000\000\134\077\162\072\176\000\162"
                    "\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\362\377\377\377\377\377\377\134\134\134\142\134\162\362\162"
                    "\162\364\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162"
                    "\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072"
                    "\176\000\162\162\377\377\377\377\377\377\134\134\134\162\162\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\377\134\134\126\142\134\162\362"
                    "\162\162\162\072\176\000\174\174\377\134\362\162\362\162\162\162\072\176\000\162\362\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000"
                    "\162\362\162\162\162\134\134\134\142\134\162\162\162\162\077\173\153\153\153\165\000\000\000\153\151\153\153\153\153\153\153\153\153\153\153\153\153\153\153\176"
                    "\000\162\362\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\162\072\176\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176"
                    "\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\162\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\377\134\134\134"
                    "\167\167\167\167\167\001\100\000\002\000\000\000\000\000\000\000\000\153\153\067\000\000\000\153\153\300\000\000\000\000\000\000\000\000\000\000\000\000\212\377"
                    "\000\000\153\153\153\153\153\153\153\060\047\047\043\047\057\153\000\153\153\137\377\153\153\000\134\000\000\000\153\153\343\153\153\153\060\047\047\043\047\057"
                    "\157\153\153\153\162\162\077\162\072\176\000\162\134\142\134\162\162\162\162\077\162\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134"
                    "\162\162\377\377\377\167\001\100\000\377\004\002\000\000\000\000\000\001\000\000\134\077\162\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134"
                    "\142\134\162\162\162\162\077\162\072\176\000\162\362\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\364\072\176\000\162\162\162\162\162\174\174\377"
                    "\134\134\134\162\142\362\134\162\162\162\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142"
                    "\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\162\162\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362"
                    "\377\134\134\126\142\134\162\362\162\162\162\072\176\000\162\162\162\162\162\174\174\377\134\362\162\362\162\162\162\072\176\000\162\362\162\162\162\174\174\377"
                    "\134\134\134\162\142\362\134\162\162\162\072\176\000\000\044\162\162\162\162\174\174\377\134\362\162\162\162\134\134\134\142\162\072\176\000\215\215\162\162\162"
                    "\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377"
                    "\134\134\134\142\134\162\153\153\153\153\153\153\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162"
                    "\162\072\176\000\162\000\001\000\000\134\077\162\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000"
                    "\162\362\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\364\072\176\000\162\162\162\162\162\174\174\377\134\134\134\162\142\362\134\162\162\162\072"
                    "\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162"
                    "\162\377\377\377\377\377\377\134\134\134\162\162\072\176\000\162\162\162\162\162\174\174\377\134\134\134\142\134\162\362\162\162\162\072\176\000\162\162\162\162"
                    "\162\174\174\377\134\362\162\172\162\134\134\134\142\162\072\176\000\215\215\162\162\162\174\174\174\171\072\176\000\162\162\162\162\162\162\377\053\377\377\377"
                    "\134\134\134\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\162\072\176\000\162\362\162\162\162"
                    "\174\174\377\134\134\134\162\142\362\134\162\162\162\072\176\000\162\362\162\162\162\134\134\142\134\162\362\162\162\162\072\176\000\044\162\162\162\162\174\174"
                    "\377\134\362\162\162\162\134\134\134\142\162\072\176\000\215\215\162\162\162\174\174\174\174\072\176\000\162\162\162\162\162\162\377\053\377\377\377\134\134\134"
                    "\142\134\162\162\162\162\077\162\072\176\000\162\162\377\377\377\377\377\377\134\134\134\142\134\162\362\162\162\162\072\176\000\162\162\162\162\162\174\174\377"
                    "\134\134\134\162\142\362\134\162\162\162\072\176\000\215\215\162\162\162\174\174\174\174\134\134\134\142\134\000\153\153\153\153\153\153\153\062\047\047\043\047"
                    "\057\262\054\377\134\134\142\153\330\153",
                    2690, ZINT_ERROR_TOO_LONG, 1, ""
                }, /* #300 (#10) Andre Maute */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[16834];
    char cmp_buf[32768];
    char cmp_msg[32768];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_fuzz", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {

            if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, debug)) {
                if (!data[i].bwipp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    char modules_dump[32768];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                }
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                int cmp_len, ret_len;
                char modules_dump[32768];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}


INTERNAL void pdf_numbprocess_test(short *chainemc, int *p_mclength, const unsigned char chaine[], const int start,
                const int length);

#include "../large.h"

/* Max codewords 12 */
static int annex_d_decode_dump(short chainemc[], int mclength, unsigned char *chaine, int length, char *buf1, char *buf2) {
    static const large_uint pow900s[] = {
        { 0x1, 0 }, /*1*/
        { 0x384, 0 }, /*900*/
        { 0xC5C10, 0 }, /*810000*/
        { 0x2B73A840, 0 }, /*729000000*/
        { 0x98C29B8100, 0 }, /*656100000000*/
        { 0x2190C2AB18400, 0 }, /*590490000000000*/
        { 0x7600EC618141000, 0 }, /*531441000000000000*/
        { 0xEDB3F06CA6884000, 0x19 }, /*478296900000000000000*/
        { 0xAC993DF977010000, 0x5B27 }, /*430467210000000000000000*/
        { 0xCABDE1065F840000, 0x140777A }, /*387420489000000000000000000*/
        { 0xC38B1A67CC100000, 0x466A40BB0 }, /*348678440100000000000000000000*/
        { 0x7508D4E968400000, 0xF78D8B9196F }, /*313810596090000000000000000000000*/
    };
    static const large_uint pow10s[] = {
        { 1, 0 }, /*1*/
        { 0xA, 0 }, /*10*/
        { 0x64, 0 }, /*100*/
        { 0x3E8, 0 }, /*1000*/
        { 0x2710, 0 }, /*10000*/
        { 0x186A0, 0 }, /*100000*/
        { 0xF4240, 0 }, /*1000000*/
        { 0x989680, 0 }, /*10000000*/
        { 0x5F5E100, 0 }, /*100000000*/
        { 0x3B9ACA00, 0 }, /*1000000000*/
        { 0x2540BE400, 0 }, /*10000000000*/
        { 0x174876E800, 0 }, /*100000000000*/
        { 0xE8D4A51000, 0 }, /*1000000000000*/
        { 0x9184E72A000, 0 }, /*10000000000000*/
        { 0x5AF3107A4000, 0 }, /*100000000000000*/
        { 0x38D7EA4C68000, 0 }, /*1000000000000000*/
        { 0x2386F26FC10000, 0 }, /*10000000000000000*/
        { 0x16345785D8A0000, 0 }, /*100000000000000000*/
        { 0xDE0B6B3A7640000, 0 }, /*1000000000000000000*/
        { 0x8AC7230489E80000, 0 }, /*10000000000000000000*/
        { 0x6BC75E2D63100000, 0x5 }, /*100000000000000000000*/
        { 0x35C9ADC5DEA00000, 0x36 }, /*1000000000000000000000*/
    };
    int i;
    large_uint t, s, e;

    if (mclength > 12 + 1) {
        return -1;
    }

    large_load_u64(&t, 0);
    for (i = 1; i < mclength; i++) {
        large_load(&s, &pow900s[mclength - i - 1]);
        large_mul_u64(&s, chainemc[i]);
        large_add(&t, &s);
    }
    large_dump(&t, buf1);

    large_load_str_u64(&e, chaine, length);
    large_add(&e, &pow10s[length]); /* Add "1" prefix */
    large_dump(&e, buf2);

    return 0;
}

static void test_numbprocess(const testCtx *const p_ctx) {

    struct item {
        unsigned char *chaine;
        int expected_len;
        int expected[16];
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { TU("1"), 2, { 902, 11, }, },
        /*  1*/ { TU("9"), 2, { 902, 19, }, },
        /*  2*/ { TU("12"), 2, { 902, 112, }, },
        /*  3*/ { TU("99"), 2, { 902, 199, }, },
        /*  4*/ { TU("000"), 3, { 902, 1, 100, }, },
        /*  5*/ { TU("123"), 3, { 902, 1, 223, }, },
        /*  6*/ { TU("999"), 3, { 902, 2, 199, }, },
        /*  7*/ { TU("1234"), 3, { 902, 12, 434, }, },
        /*  8*/ { TU("9999"), 3, { 902, 22, 199, }, },
        /*  9*/ { TU("12345"), 3, { 902, 124, 745, }, },
        /* 10*/ { TU("99999"), 3, { 902, 222, 199, }, },
        /* 11*/ { TU("000000"), 4, { 902, 1, 211, 100, }, },
        /* 12*/ { TU("123456"), 4, { 902, 1, 348, 256, }, },
        /* 13*/ { TU("999999"), 4, { 902, 2, 422, 199, }, },
        /* 14*/ { TU("1234567"), 4, { 902, 13, 782, 767, }, },
        /* 15*/ { TU("9999999"), 4, { 902, 24, 622, 199, }, },
        /* 16*/ { TU("12345678"), 4, { 902, 138, 628, 478, }, },
        /* 17*/ { TU("99999999"), 4, { 902, 246, 822, 199, }, },
        /* 18*/ { TU("000000000"), 5, { 902, 1, 334, 511, 100, }, },
        /* 19*/ { TU("123456789"), 5, { 902, 1, 486, 885, 289, }, },
        /* 20*/ { TU("999999999"), 5, { 902, 2, 669, 122, 199, }, },
        /* 21*/ { TU("1234567890"), 5, { 902, 15, 369, 753, 190, }, },
        /* 22*/ { TU("9999999999"), 5, { 902, 27, 391, 322, 199, }, },
        /* 23*/ { TU("12345678901"), 5, { 902, 154, 98, 332, 101, }, },
        /* 24*/ { TU("99999999999"), 5, { 902, 274, 313, 522, 199, }, },
        /* 25*/ { TU("000000000000"), 6, { 902, 1, 471, 667, 811, 100, }, },
        /* 26*/ { TU("123456789012"), 6, { 902, 1, 641, 83, 621, 112, }, },
        /* 27*/ { TU("999999999999"), 6, { 902, 3, 43, 435, 722, 199, }, },
        /* 28*/ { TU("1234567890123"), 6, { 902, 17, 110, 836, 811, 223, }, },
        /* 29*/ { TU("9999999999999"), 6, { 902, 30, 434, 758, 22, 199, }, },
        /* 30*/ { TU("12345678901234"), 6, { 902, 171, 209, 269, 12, 434, }, },
        /* 31*/ { TU("99999999999999"), 6, { 902, 304, 748, 380, 222, 199, }, },
        /* 32*/ { TU("000000000000000"), 7, { 902, 1, 624, 142, 101, 211, 100, }, },
        /* 33*/ { TU("123456789012345"), 7, { 902, 1, 812, 292, 890, 124, 745, }, },
        /* 34*/ { TU("999999999999999"), 7, { 902, 3, 348, 284, 202, 422, 199, }, },
        /* 35*/ { TU("000213298174000"), 7, { 902, 1, 624, 434, 632, 282, 200, }, }, /* ISO/IEC 15438:2015 Annex D example */
        /* 36*/ { TU("12345678901234567"), 7, { 902, 190, 232, 498, 813, 782, 767, }, },
        /* 37*/ { TU("99999999999999999"), 7, { 902, 338, 631, 522, 446, 822, 199, }, },
        /* 38*/ { TU("000000000000000000"), 8, { 902, 1, 793, 457, 812, 434, 511, 100, }, },
        /* 39*/ { TU("123456789012345678"), 8, { 902, 2, 102, 525, 489, 38, 628, 478, }, },
        /* 40*/ { TU("1234567890123456789"), 8, { 902, 21, 125, 755, 390, 386, 885, 289, }, },
        /* 41*/ { TU("9999999999999999999"), 8, { 902, 37, 570, 158, 49, 591, 322, 199, }, },
        /* 42*/ { TU("12345678901234567890"), 8, { 902, 211, 358, 354, 304, 269, 753, 190, }, },
        /* 43*/ { TU("99999999999999999999"), 8, { 902, 376, 301, 680, 496, 513, 522, 199, }, },
        /* 44*/ { TU("000000000000000000000"), 9, { 902, 2, 81, 608, 702, 682, 767, 811, 100, }, },
        /* 45*/ { TU("123456789012345678901"), 9, { 902, 2, 313, 883, 843, 342, 898, 332, 101, }, },
        /* 46*/ { TU("999999999999999999999"), 9, { 902, 4, 163, 317, 505, 465, 635, 722, 199, }, },
        /* 47*/ { TU("12345678901234567890123456789012345678901"), 15, { 902, 441, 883, 33, 675, 271, 780, 283, 607, 83, 596, 420, 698, 332, 101, }, },
        /* 48*/ { TU("99999999999999999999999999999999999999999"), 15, { 902, 786, 741, 209, 58, 439, 392, 682, 346, 534, 445, 515, 80, 222, 199, }, },
        /* 49*/ { TU("000000000000000000000000000000000000000000"), 16, { 902, 4, 334, 106, 145, 292, 397, 163, 711, 832, 872, 427, 775, 401, 211, 100, }, },
        /* 50*/ { TU("12345678901234567890123456789012345678901234"), 16, { 902, 491, 81, 137, 450, 302, 67, 15, 174, 492, 862, 667, 475, 869, 12, 434, }, },
        /* 51*/ { TU("99999999999999999999999999999999999999999999"), 16, { 902, 874, 223, 532, 264, 888, 236, 358, 185, 93, 795, 72, 289, 146, 822, 199, }, },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length;

    short chainemc[32];
    int mclength;

    testStart("test_numbprocess");

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        length = (int) ustrlen(data[i].chaine);
        mclength = 0;
        pdf_numbprocess_test(chainemc, &mclength, data[i].chaine, 0, length);
        assert_nonzero(mclength < ARRAY_SIZE(chainemc), "i:%d mclength %d >= ARRAY_SIZE(chainemc) %d\n", i, mclength, ARRAY_SIZE(chainemc));
#if 0
        for (j = 0; j < mclength; j++) { printf(" %d", chainemc[j]); } printf("\n");
#endif
        assert_equal(mclength, data[i].expected_len, "i:%d mclength %d != expected_len %d\n", i, mclength, data[i].expected_len);
        for (j = 0; j < mclength; j++) {
            assert_equal(chainemc[j], data[i].expected[j], "i:%d chainemc[%d] %d != %d\n", i, j, chainemc[j], data[i].expected[j]);
        }
        if (length < 20) {
            char buf1[64], buf2[64];
            assert_zero(annex_d_decode_dump(chainemc, mclength, data[i].chaine, length, buf1, buf2), "i:%d annex_d_decode_dump() fail\n", i);
            assert_zero(strcmp(buf1, buf2), "i:%d, strcmp(%s, %s) != 0\n", i, buf1, buf2);
        }
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITER_MILLES   5
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
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, "1234567890", 0, 7, 103, "10 numerics" },
        /*  1*/ { BARCODE_PDF417, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    0, 43, 290, "960 chars, text/numeric" },
        /*  2*/ { BARCODE_PDF417, DATA_MODE, -1, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    0, 51, 358, "960 chars, byte" },
        /*  3*/ { BARCODE_PDF417, -1, -1, -1,
                    "BP2D+1.00+0005+FLE ESC BV+1.00+3.60*BX2D+1.00+0001+Casual shoes & apparel+90044030118100801265*D_2D+1.02+31351440315981+C910332+02032018+KXXXX CXXXX+UNIT 4 HXXX"
                    "XXXXX BUSINESS PARK++ST  ALBANS+ST  ALBANS++AL2 3TA+0001+000001+001+00000000+00++N+N+N+0000++++++N+++N*DS2D+1.01+0001+0001+90044030118100801265+++++07852389322+"
                    "+E*F_2D+1.00+0005*",
                    0, 26, 222, "338 chars, text/numeric/byte" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start;
    clock_t total_create = 0, total_encode = 0, total_buffer = 0, total_buf_inter = 0, total_print = 0;
    clock_t diff_create, diff_encode, diff_buffer, diff_buf_inter, diff_print;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    printf("FAST_MODE\n");
    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = diff_buf_inter = diff_print = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            int input_mode = data[i].input_mode == -1 ? FAST_MODE : (data[i].input_mode | FAST_MODE);
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

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

            symbol->output_options |= OUT_BUFFER_INTERMEDIATE;
            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buf_inter += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer OUT_BUFFER_INTERMEDIATE ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; /* Undo */

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

            ZBarcode_Delete(symbol);
        }

        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_buf_inter), TEST_PERF_TIME(diff_print), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
        total_buf_inter += diff_buf_inter;
        total_print += diff_print;
    }
    if (p_ctx->index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter), TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }

    printf("OPTIMIZED\n");
    total_create = 0, total_encode = 0, total_buffer = 0, total_buf_inter = 0, total_print = 0;
    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = diff_buf_inter = diff_print = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

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

            symbol->output_options |= OUT_BUFFER_INTERMEDIATE;
            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buf_inter += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer OUT_BUFFER_INTERMEDIATE ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; /* Undo */

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

            ZBarcode_Delete(symbol);
        }

        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_buf_inter), TEST_PERF_TIME(diff_print), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
        total_buf_inter += diff_buf_inter;
        total_print += diff_print;
    }
    if (p_ctx->index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter), TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_options", test_options },
        { "test_reader_init", test_reader_init },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_encode_segs", test_encode_segs },
        { "test_fuzz", test_fuzz },
        { "test_numbprocess", test_numbprocess },
        { "test_perf", test_perf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
