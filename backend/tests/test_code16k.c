/*
    libzint - the open source barcode library
    Copyright (C) 2020-2025 Robin Stuart <rstuart114@gmail.com>

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
        const char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "A", 77, 0, 16, 70, "" }, /* BS EN 12323:2005 4.1 (l) */
        /*  1*/ { "A", 78, ZINT_ERROR_TOO_LONG, -1, -1, "Error 421: Input too long, requires 79 symbol characters (maximum 78)" },
        /*  2*/ { "A", 257, ZINT_ERROR_TOO_LONG, -1, -1, "Error 420: Input length 257 too long (maximum 256)" },
        /*  3*/ { "0", 154, 0, 16, 70, "" }, /* BS EN 12323:2005 4.1 (l) */
        /*  4*/ { "0", 155, ZINT_ERROR_TOO_LONG, -1, -1, "Error 421: Input too long, requires 80 symbol characters (maximum 78)" },
        /*  5*/ { "0", 153, ZINT_ERROR_TOO_LONG, -1, -1, "Error 421: Input too long, requires 79 symbol characters (maximum 78)" }, /* Fails due to odd length */
        /*  6*/ { "0", 161, ZINT_ERROR_TOO_LONG, -1, -1, "Error 421: Input too long, requires 79 symbol characters (maximum 78)" },
        /*  7*/ { "\001", 77, 0, 16, 70, "" },
        /*  8*/ { "\001", 78, ZINT_ERROR_TOO_LONG, -1, -1, "Error 421: Input too long, requires 79 symbol characters (maximum 78)" },
        /*  9*/ { "\377", 38, 0, 16, 70, "" }, /* FNC4 + char for each so half 77 as not using double latch */
        /* 10*/ { "\377", 39, ZINT_ERROR_TOO_LONG, -1, -1, "Error 421: Input too long, requires 79 symbol characters (maximum 78)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data_buf), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int output_options;
        const char *data;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected;
        const char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, READER_INIT, "A", 0, 2, 70, "(10) 1 96 33 103 103 103 103 103 68 35", "ModeB FNC3 A Pad (5)" },
        /*  1*/ { UNICODE_MODE, READER_INIT, "12", 0, 2, 70, "(10) 5 96 12 103 103 103 103 103 99 41", "ModeC1SB FNC3 12 Pad (5)" },
        /*  2*/ { UNICODE_MODE, READER_INIT, "A1234", 0, 2, 70, "(10) 6 96 33 12 34 103 103 103 65 53", "ModeC2SB FNC3 A 12 34 Pad (3)" },
        /*  3*/ { GS1_MODE, READER_INIT, "[90]1", ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 422: Cannot use Reader Initialisation in GS1 mode", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
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
        int output_options;
        int option_1;
        int option_3;
        const char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        int zxingcpp_cmp;
        const char *expected;
        int expected_option_1;
        int expected_option_3;
        const char *comment;
    };
    /*
       NUL U+0000, CodeA-only
       US U+001F (\037, 31), CodeA-only
       a U+0061 (\141, 97), CodeB-only
       b U+0062 (\142, 98), CodeB-only
       APC U+009F (\237, 159), UTF-8 C29F, CodeA-only extended ASCII, not in ISO 8859-1
       ß U+00DF (\337, 223), UTF-8 C39F, CodeA and CodeB extended ASCII
       é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, 0, "\037", -1, 0, 2, 70, 1, 1, "(10) 0 95 103 103 103 103 103 103 22 42", 2, 0, "ModeA US Pad (6)" },
        /*  1*/ { UNICODE_MODE, -1, -1, 0, "A", -1, 0, 2, 70, 1, 1, "(10) 1 33 103 103 103 103 103 103 52 82", 2, 0, "ModeB A Pad (6)" },
        /*  2*/ { UNICODE_MODE, -1, -1, 0, "12", -1, 0, 2, 70, 1, 1, "(10) 2 12 103 103 103 103 103 103 98 27", 2, 0, "ModeC 12 Pad (6)" },
        /*  3*/ { GS1_MODE, -1, -1, 0, "[90]A", -1, 0, 2, 70, 1, 1, "(10) 3 25 16 33 103 103 103 103 83 20", 2, 0, "ModeBFNC1 9 0 A Pad (4)" },
        /*  4*/ { GS1_MODE, -1, -1, 0, "[90]12", -1, 0, 2, 70, 1, 1, "(10) 4 90 12 103 103 103 103 103 79 62", 2, 0, "ModeCFNC1 90 12 Pad (5)" },
        /*  5*/ { GS1_MODE, -1, -1, 0, "[90]12[20]12", -1, 0, 2, 70, 1, 1, "(10) 4 90 12 102 20 12 103 103 9 72", 2, 0, "ModeCFNC1 90 12 FNC1 20 12 Pad (2)" },
        /*  6*/ { GS1_MODE, -1, -1, 0, "[90]123[20]12", -1, 0, 3, 70, 1, 1, "(15) 11 90 12 100 19 99 102 20 12 103 103 103 103 102 11", 3, 0, "ModeCFNC1 90 CodeB 3 CodeC FNC1 20 12 Pad (4)" },
        /*  7*/ { GS1_MODE, -1, -1, 0, "[90]123[91]1A3[20]12", -1, 0, 4, 70, 1, 1, "(20) 18 90 12 100 19 102 25 99 11 100 33 19 99 102 20 12 103 103 0 3", 4, 0, "ModeCFNC1 90 12 CodeB 3 FNC1 9 CodeC 11 CodeB A 3 CodeC FNC1 20 12 Pad (2)" },
        /*  8*/ { GS1_MODE, -1, -1, 0, "[90]123A[91]123", -1, 0, 3, 70, 1, 1, "(15) 11 90 12 100 19 33 102 25 99 11 23 103 103 81 56", 3, 0, "ModeCFNC1 90 12 CodeB 3 A FNC1 9 CodeC 11 23 Pad (2)" },
        /*  9*/ { GS1_MODE | GS1PARENS_MODE, -1, -1, 0, "(90)12", -1, 0, 2, 70, 1, 1, "(10) 4 90 12 103 103 103 103 103 79 62", 2, 0, "ModeCFNC1 90 12 Pad (5)" },
        /* 10*/ { GS1_MODE, -1, -1, 0, "[90](2", -1, 0, 2, 70, 1, 1, "(10) 3 25 16 8 18 103 103 103 90 93", 2, 0, "ModeBFNC1 9 0 ( Pad (3)" },
        /* 11*/ { GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, -1, 0, "(90)\\(2", -1, 0, 2, 70, 1, 1, "(10) 3 25 16 8 18 103 103 103 90 93", 2, 0, "ModeBFNC1 9 0 ( Pad (3)" },
        /* 12*/ { UNICODE_MODE, -1, -1, 0, "a0123456789", -1, 0, 2, 70, 1, 1, "(10) 5 65 1 23 45 67 89 103 27 86", 2, 0, "ModeC1SB a 01 23 45 67 89 Pad" },
        /* 13*/ { UNICODE_MODE, -1, -1, 0, "ab0123456789", -1, 0, 2, 70, 1, 1, "(10) 6 65 66 1 23 45 67 89 19 42", 2, 0, "ModeC2SB a b 01 23 45 67 89" },
        /* 14*/ { UNICODE_MODE, -1, -1, 0, "1234\037a", -1, 0, 2, 70, 0, 1, "(10) 2 12 34 101 95 98 65 103 67 53", 2, 0, "ModeC 12 34 CodeA US 1SB a Pad; BWIPP different encodation, CodeB instead of 1SB" },
        /* 15*/ { UNICODE_MODE, -1, -1, 0, "\000\037ß", 4, 0, 2, 70, 1, 1, "(10) 0 64 95 101 63 103 103 103 75 11", 2, 0, "ModeA NUL US FNC4 ß Pad (3)" },
        /* 16*/ { UNICODE_MODE, -1, -1, 0, "\000\037é", 4, 0, 2, 70, 0, 1, "(10) 0 64 95 101 98 73 103 103 75 6", 2, 0, "ModeA NUL US FNC4 1SB é Pad (2); BWIPP different encodation, CodeB instead of 1SB" },
        /* 17*/ { UNICODE_MODE, -1, -1, 0, "\000\037éa", 5, 0, 2, 70, 0, 1, "(10) 0 64 95 100 100 73 65 103 99 69", 2, 0, "ModeA NUL US CodeB FNC4 é a Pad; BWIPP different encodation, FNC4 before CodeB" },
        /* 18*/ { UNICODE_MODE, -1, -1, 0, "abß", -1, 0, 2, 70, 1, 1, "(10) 1 65 66 100 63 103 103 103 66 56", 2, 0, "ModeB a b FNC4 ß Pad (3)" },
        /* 19*/ { DATA_MODE, -1, -1, 0, "\141\142\237", -1, 0, 2, 70, 0, 899, "(10) 1 65 66 100 98 95 103 103 6 71", 2, 0, "ModeB a b FNC4 1SA APC Pad (2); BWIPP different encodation, CodeA instead of 1SA" },
        /* 20*/ { DATA_MODE, -1, -1, 0, "\141\142\237\037", -1, 0, 2, 70, 0, 899, "(10) 1 65 66 101 101 95 95 103 72 93", 2, 0, "ModeB a b CodeA FNC4 APC US Pad; BWIPP different encodation, FNC4 before CodeA" },
        /* 21*/ { UNICODE_MODE, -1, -1, 0, "ééé", -1, 0, 2, 70, 0, 1, "(10) 1 100 73 100 73 100 73 103 105 106", 2, 0, "ModeB FNC4 é FNC4 é FNC4 é Pad; BWIPP different encodation, uses double FNC4 latch" },
        /* 22*/ { UNICODE_MODE, -1, -1, 0, "aééééb", -1, 0, 3, 70, 1, 1, "(15) 8 65 100 73 100 73 100 73 100 73 66 103 103 39 83", 3, 0, "ModeB a FNC4 é (4) b Pad (2)" },
        /* 23*/ { UNICODE_MODE, -1, -1, 0, "aéééééb", -1, 0, 3, 70, 0, 1, "(15) 8 65 100 73 100 73 100 73 100 73 100 73 66 74 106", 3, 0, "ModeB a FNC4 é (5) b; BWIPP different encodation, uses double FNC4 latch" },
        /* 24*/ { UNICODE_MODE, -1, -1, 0, "aééééébcdeé", -1, 0, 4, 70, 0, 1, "(20) 15 65 100 73 100 73 100 73 100 73 100 73 66 67 68 69 100 73 14 69", 4, 0, "ModeB a FNC4 é (5) b c d e FNC4 é; BWIPP different encodation, uses double FNC4 latch then FNC4 escapes" },
        /* 25*/ { UNICODE_MODE, -1, -1, 0, "123456789012345678901234", -1, 0, 3, 70, 1, 1, "(15) 9 12 34 56 78 90 12 34 56 78 90 12 34 71 42", 3, 0, "3 rows" },
        /* 26*/ { UNICODE_MODE, -1, 2, 0, "123456789012345678901234", -1, 0, 3, 70, 0, 1, "(15) 9 12 34 56 78 90 12 34 56 78 90 12 34 71 42", 3, 0, "Min 2 rows (no change); BWIPP doesn't support min rows < required" },
        /* 27*/ { UNICODE_MODE, -1, 3, 0, "123456789012345678901234", -1, 0, 3, 70, 1, 1, "(15) 9 12 34 56 78 90 12 34 56 78 90 12 34 71 42", 3, 0, "Min 3 rows (no change)" },
        /* 28*/ { UNICODE_MODE, -1, 4, 0, "123456789012345678901234", -1, 0, 4, 70, 1, 1, "(20) 16 12 34 56 78 90 12 34 56 78 90 12 34 103 103 103 103 103 66 96", 4, 0, "Min 4 rows" },
        /* 29*/ { UNICODE_MODE, -1, 5, 0, "123456789012345678901234", -1, 0, 5, 70, 1, 1, "(25) 23 12 34 56 78 90 12 34 56 78 90 12 34 103 103 103 103 103 103 103 103 103 103 68 61", 5, 0, "Min 5 rows" },
        /* 30*/ { UNICODE_MODE, -1, 16, 0, "123456789012345678901234", -1, 0, 16, 70, 1, 1, "(80) 100 12 34 56 78 90 12 34 56 78 90 12 34 103 103 103 103 103 103 103 103 103 103 103", 16, 0, "Min 16 rows" },
        /* 31*/ { UNICODE_MODE, -1, 1, 0, "123456789012345678901234", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, 1, 1, "Error 424: Minimum number of rows '1' out of range (2 to 16)", 1, 0, "" },
        /* 32*/ { UNICODE_MODE, -1, 17, 0, "123456789012345678901234", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, 1, 1, "Error 424: Minimum number of rows '17' out of range (2 to 16)", 17, 0, "" },
        /* 33*/ { UNICODE_MODE, -1, -1, 0, "ÁÁÁÁÁÁ99999999999999Á", -1, 0, 6, 70, 0, 1, "(30) 29 100 33 100 33 100 33 100 33 100 33 100 33 99 99 99 99 99 99 99 99 100 100 33 103", 6, 0, "BWIPP different encodation, uses double FNC4 latch" },
        /* 34*/ { UNICODE_MODE, -1, -1, 0, "ÿ\012àa\0121\012àAà", -1, 0, 4, 70, 0, 1, "(20) 15 100 95 98 74 100 64 65 98 74 17 98 74 100 64 33 100 64 47 35", 4, 0, "BWIPP different encodation, uses CodeA instead of 1SA" },
        /* 35*/ { UNICODE_MODE, -1, -1, 0, "ÿ\012àa\0121\012àAà\012à", -1, 0, 5, 70, 0, 1, "(25) 22 100 95 98 74 100 64 65 98 74 17 98 74 100 64 33 100 64 98 74 100 64 103 89 18", 5, 0, "BWIPP different encodation, uses CodeA instead of 1SA" },
        /* 36*/ { UNICODE_MODE, -1, -1, 0, "y1234\012àa\0121\0127890àAàDà\012à", -1, 0, 7, 70, 0, 1, "(35) 40 89 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33 100 64 36 100", 7, 0, "BWIPP different encodation, uses Sh2B + other differences" },
        /* 37*/ { UNICODE_MODE, -1, -1, 0, "ÿ1234\012àa\0121\0127890àAàDà\012à", -1, 0, 7, 70, 0, 1, "(35) 41 100 95 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33 100 64 36", 7, 0, "BWIPP different encodation" },
        /* 38*/ { UNICODE_MODE, -1, -1, 0, "yÿ1234\012àa\0121\0127890àAàDà\012à", -1, 0, 8, 70, 0, 1, "(40) 43 89 100 95 99 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33 100", 8, 0, "BWIPP different encodation (and fits in 7 rows)" },
        /* 39*/ { UNICODE_MODE, -1, -1, 0, "ÿy1234\012àa\0121\0127890àAàDà\012à", -1, 0, 8, 70, 0, 1, "(40) 43 100 95 89 99 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33 100", 8, 0, "BWIPP different encodation (and fits in 7 rows)" },
        /* 40*/ { UNICODE_MODE, -1, -1, 0, "ÿÿ1234\012àa\0121\0127890àAàDà\012à", -1, 0, 8, 70, 0, 1, "(40) 43 100 95 100 95 99 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33", 8, 0, "BWIPP different encodation (and fits in 7 rows)" },
        /* 41*/ { UNICODE_MODE, -1, -1, 0, "ÿ1234\012àa\0121\01223456\0127890àAàDà\012à", -1, 0, 8, 70, 0, 1, "(40) 48 100 95 12 34 101 74 100 100 64 65 98 74 17 98 74 18 99 34 56 101 74 99 78 90 100", 8, 0, "BWIPP different encodation, uses Sh2B + other differences" },
        /* 42*/ { UNICODE_MODE, -1, -1, 0, "ÿ1234\012à1234a\0121\01223456\0127890àAàDà\012à", -1, 0, 9, 70, 0, 1, "(45) 55 100 95 12 34 101 74 101 98 64 99 12 34 100 65 98 74 17 98 74 18 99 34 56 101 74 99", 9, 0, "BWIPP different encodation, uses Sh2C + other differences" },
        /* 43*/ { UNICODE_MODE, -1, -1, 0, "ÿ1234\012à1234ab\0121\01223456\012\0127890àAàBCDà\012\012à", -1, 0, 10, 70, 0, 1, "(50) 62 100 95 12 34 101 74 101 98 64 99 12 34 100 65 66 98 74 17 98 74 18 99 34 56 101 74", 10, 0, "BWIPP different encodation, uses Sh2C + other differences" },
        /* 44*/ { UNICODE_MODE, -1, -1, 0, "ÿ123456\012à123456abcd\0121\01223456\012\0127890àAàBCDEà\012\012à", -1, 0, 11, 70, 0, 1, "(55) 69 100 95 12 34 56 101 74 101 98 64 99 12 34 56 100 65 66 67 68 98 74 17 98 74 18 99", 11, 0, "BWIPP different encodation, uses Sh3C + other differences" },
        /* 45*/ { UNICODE_MODE, -1, -1, 0, "ÿ12345678\012à12345678abcdef\0121\01223456\012\0127890àAàBCDEFà\012\012à", -1, 0, 12, 70, 0, 1, "(60) 76 100 95 12 34 56 78 101 74 101 98 64 99 12 34 56 78 100 65 66 67 68 69 70 98 74 17", 12, 0, "BWIPP different encodation, uses Sh2C + other differences" },
        /* 46*/ { UNICODE_MODE, COMPLIANT_HEIGHT, -1, 4, "A", -1, 0, 2, 70, 1, 1, "(10) 1 33 103 103 103 103 103 103 52 82", 2, 4, "option_3 separator" },
        /* 47*/ { UNICODE_MODE, COMPLIANT_HEIGHT, -1, 5, "A", -1, 0, 2, 70, 1, 1, "(10) 1 33 103 103 103 103 103 103 52 82", 2, 1, "option_3 invalid 5 -> 1" },
        /* 48*/ { UNICODE_MODE, -1, -1, 5, "A", -1, 0, 2, 70, 1, 1, "(10) 1 33 103 103 103 103 103 103 52 82", 2, 5, "option_3 invalid 5 ignored unless COMPLIANT_HEIGHT" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    /* Only do BWIPP/ZXing-C++ tests if asked, too slow otherwise */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/,
                                    data[i].option_1, -1 /*option_2*/, data[i].option_3, data[i].output_options,
                                    data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, \"%s\", %d, %s, %d, %d, %d, %d, \"%s\", %d, %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
                    data[i].option_1, data[i].option_3,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp,
                    data[i].zxingcpp_cmp, symbol->errtxt, symbol->option_1, symbol->option_3, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, -1, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        char modules_dump[4096];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, data[i].option_1, -1, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[4096];
                    assert_nonzero(data[i].zxingcpp_cmp, "i:%d data[i].zxingcpp_cmp == 0", i);
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, data[i].zxingcpp_cmp,
                                cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n",
                                i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length,
                                NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len,
                                escaped);
                }
            }
            assert_equal(symbol->option_1, data[i].expected_option_1, "i:%d symbol->option_1 %d != %d\n",
                        i, symbol->option_1, data[i].expected_option_1);
            assert_zero(symbol->option_2, "i:%d symbol->option_2 %d != 0\n", i, symbol->option_2);
            assert_equal(symbol->option_3, data[i].expected_option_3, "i:%d symbol->option_3 %d != %d\n",
                        i, symbol->option_3, data[i].expected_option_3);
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
        const char *data;
        int ret;

        int expected_rows;
        int expected_width;
        const char *comment;
        const char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "ab0123456789", 0, 2, 70, "BS EN 12323:2005 Figure 3",
                    "1110010101100110111011010011110110111100100110010011000100100010001101"
                    "1100110101000100111011110100110010010000100110100011010010001110011001"
                },
        /*  1*/ { UNICODE_MODE, -1, "www.wikipedia.de", 0, 4, 70, "https://commons.wikimedia.org/wiki/File:Code_16K_wikipedia.png",
                    "1110010101000110011000011010110000110101100001101011011001100010001101"
                    "1100110100001101011011110010110011110110101111001011010110000110011001"
                    "1101100101001101111011110110010111100101101101001111011001100010010011"
                    "1000010101111011001010011011110010111101101100001011010001001110111101"
               },
        /*  2*/ { UNICODE_MODE, -1, "12345678901234567890123456789012", 0, 4, 70, "",
                    "1110010101100010011010011000110111010011100011101001001111010110001101"
                    "1100110100100001001010011000110111010011100011101001001111010110011001"
                    "1101100100100001001010011000110111010011100011101001001111010110010011"
                    "1000010100100001001010011000110010111101100001011101000111010010111101"
                },
        /*  3*/ { UNICODE_MODE, 5, "12345678901234567890123456789012", 0, 5, 70, "Min 5 rows",
                    "1110010100010010001010011000110111010011100011101001001111010110001101"
                    "1100110100100001001010011000110111010011100011101001001111010110011001"
                    "1101100100100001001010011000110111010011100011101001001111010110010011"
                    "1000010100100001001010011000110010111101100101111011001011110110111101"
                    "1011100100101111011001011110110010111101101000010001011110011010100011"
                },
        /*  4*/ { UNICODE_MODE, 16, "12345678901234567890123456789012", 0, 16, 70, "Min 16 rows",
                    "1110010101000010001010011000110111010011100011101001001111010110001101"
                    "1100110100100001001010011000110111010011100011101001001111010110011001"
                    "1101100100100001001010011000110111010011100011101001001111010110010011"
                    "1000010100100001001010011000110010111101100101111011001011110110111101"
                    "1011100100101111011001011110110010111101100101111011001011110110100011"
                    "1001110100101111011001011110110010111101100101111011001011110110110001"
                    "1010000100101111011001011110110010111101100101111011001011110110101111"
                    "1110100100101111011001011110110010111101100101111011001011110110001011"
                    "1110010100101111011001011110110010111101100101111011001011110110100011"
                    "1100110100101111011001011110110010111101100101111011001011110110110001"
                    "1101100100101111011001011110110010111101100101111011001011110110101111"
                    "1000010100101111011001011110110010111101100101111011001011110110001011"
                    "1011100100101111011001011110110010111101100101111011001011110110001101"
                    "1001110100101111011001011110110010111101100101111011001011110110011001"
                    "1010000100101111011001011110110010111101100101111011001011110110010011"
                    "1110100100101111011001011110110010111101100101110001001110010010111101"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    /* Only do BWIPP/ZXing-C++ tests if asked, too slow otherwise */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/, data[i].option_1, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, -1, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, data[i].option_1, -1, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[4096];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, 1 /*zxingcpp_cmp*/, cmp_buf,
                                sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n",
                                i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length,
                                NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len,
                                escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int output_options;
        const char *data;
        int length;
        int ret;
        int expected_eci;
        const char *expected;
        int expected_length;
        int expected_content_eci;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "é", -1, 0, 0, "", -1, 0 },
        /*  1*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "é", -1, 0, 0, "é", -1, 3 }, /* Now UTF-8, not converted */
        /*  2*/ { DATA_MODE, -1, "\351", -1, 0, 0, "", -1, 0 },
        /*  3*/ { DATA_MODE, BARCODE_CONTENT_SEGS, "\351", -1, 0, 0, "\351", -1, 3 },
        /*  4*/ { GS1_MODE, -1, "[01]04912345123459[15]970331[30]128[10]ABC123", -1, 0, 0, "", -1, 0 },
        /*  5*/ { GS1_MODE, BARCODE_CONTENT_SEGS, "[01]04912345123459[15]970331[30]128[10]ABC123", -1, 0, 0, "01049123451234591597033130128\03510ABC123", -1, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                                    data[i].data, data[i].length, debug);
        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n",
                        i, symbol->eci, data[i].expected_eci);
            if (symbol->output_options & BARCODE_CONTENT_SEGS) {
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
                assert_equal(symbol->content_segs[0].eci, data[i].expected_content_eci,
                            "i:%d content_segs[0].eci %d != expected_content_eci %d\n",
                            i, symbol->content_segs[0].eci, data[i].expected_content_eci);
            } else {
                assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_reader_init", test_reader_init },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_rt", test_rt },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
