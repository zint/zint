/*
    libzint - the open source barcode library
    Copyright (C) 2020-2024 Robin Stuart <rstuart114@gmail.com>

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
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
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

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
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
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
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

    testStartSymbol("test_reader_init", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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
        int option_1;
        char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *expected;
        char *comment;
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
        /*  0*/ { UNICODE_MODE, -1, "\037", -1, 0, 2, 70, 1, "(10) 0 95 103 103 103 103 103 103 22 42", "ModeA US Pad (6)" },
        /*  1*/ { UNICODE_MODE, -1, "A", -1, 0, 2, 70, 1, "(10) 1 33 103 103 103 103 103 103 52 82", "ModeB A Pad (6)" },
        /*  2*/ { UNICODE_MODE, -1, "12", -1, 0, 2, 70, 1, "(10) 2 12 103 103 103 103 103 103 98 27", "ModeC 12 Pad (6)" },
        /*  3*/ { GS1_MODE, -1, "[90]A", -1, 0, 2, 70, 1, "(10) 3 25 16 33 103 103 103 103 83 20", "ModeBFNC1 9 0 A Pad (4)" },
        /*  4*/ { GS1_MODE, -1, "[90]12", -1, 0, 2, 70, 1, "(10) 4 90 12 103 103 103 103 103 79 62", "ModeCFNC1 90 12 Pad (5)" },
        /*  5*/ { GS1_MODE, -1, "[90]12[20]12", -1, 0, 2, 70, 1, "(10) 4 90 12 102 20 12 103 103 9 72", "ModeCFNC1 90 12 FNC1 20 12 Pad (2)" },
        /*  6*/ { GS1_MODE, -1, "[90]123[20]12", -1, 0, 3, 70, 1, "(15) 11 90 12 100 19 99 102 20 12 103 103 103 103 102 11", "ModeCFNC1 90 CodeB 3 CodeC FNC1 20 12 Pad (4)" },
        /*  7*/ { GS1_MODE, -1, "[90]123[91]1A3[20]12", -1, 0, 4, 70, 1, "(20) 18 90 12 100 19 102 25 99 11 100 33 19 99 102 20 12 103 103 0 3", "ModeCFNC1 90 12 CodeB 3 FNC1 9 CodeC 11 CodeB A 3 CodeC FNC1 20 12 Pad (2)" },
        /*  8*/ { GS1_MODE, -1, "[90]123A[91]123", -1, 0, 3, 70, 1, "(15) 11 90 12 100 19 33 102 25 99 11 23 103 103 81 56", "ModeCFNC1 90 12 CodeB 3 A FNC1 9 CodeC 11 23 Pad (2)" },
        /*  9*/ { GS1_MODE | GS1PARENS_MODE, -1, "(90)12", -1, 0, 2, 70, 1, "(10) 4 90 12 103 103 103 103 103 79 62", "ModeCFNC1 90 12 Pad (5)" },
        /* 10*/ { UNICODE_MODE, -1, "a0123456789", -1, 0, 2, 70, 1, "(10) 5 65 1 23 45 67 89 103 27 86", "ModeC1SB a 01 23 45 67 89 Pad" },
        /* 11*/ { UNICODE_MODE, -1, "ab0123456789", -1, 0, 2, 70, 1, "(10) 6 65 66 1 23 45 67 89 19 42", "ModeC2SB a b 01 23 45 67 89" },
        /* 12*/ { UNICODE_MODE, -1, "1234\037a", -1, 0, 2, 70, 0, "(10) 2 12 34 101 95 98 65 103 67 53", "ModeC 12 34 CodeA US 1SB a Pad; BWIPP different encodation, CodeB instead of 1SB" },
        /* 13*/ { UNICODE_MODE, -1, "\000\037ß", 4, 0, 2, 70, 1, "(10) 0 64 95 101 63 103 103 103 75 11", "ModeA NUL US FNC4 ß Pad (3)" },
        /* 14*/ { UNICODE_MODE, -1, "\000\037é", 4, 0, 2, 70, 0, "(10) 0 64 95 101 98 73 103 103 75 6", "ModeA NUL US FNC4 1SB é Pad (2); BWIPP different encodation, CodeB instead of 1SB" },
        /* 15*/ { UNICODE_MODE, -1, "\000\037éa", 5, 0, 2, 70, 0, "(10) 0 64 95 100 100 73 65 103 99 69", "ModeA NUL US CodeB FNC4 é a Pad; BWIPP different encodation, FNC4 before CodeB" },
        /* 16*/ { UNICODE_MODE, -1, "abß", -1, 0, 2, 70, 1, "(10) 1 65 66 100 63 103 103 103 66 56", "ModeB a b FNC4 ß Pad (3)" },
        /* 17*/ { DATA_MODE, -1, "\141\142\237", -1, 0, 2, 70, 0, "(10) 1 65 66 100 98 95 103 103 6 71", "ModeB a b FNC4 1SA APC Pad (2); BWIPP different encodation, CodeA instead of 1SA" },
        /* 18*/ { DATA_MODE, -1, "\141\142\237\037", -1, 0, 2, 70, 0, "(10) 1 65 66 101 101 95 95 103 72 93", "ModeB a b CodeA FNC4 APC US Pad; BWIPP different encodation, FNC4 before CodeA" },
        /* 19*/ { UNICODE_MODE, -1, "ééé", -1, 0, 2, 70, 0, "(10) 1 100 73 100 73 100 73 103 105 106", "ModeB FNC4 é FNC4 é FNC4 é Pad; BWIPP different encodation, uses double FNC4 latch" },
        /* 20*/ { UNICODE_MODE, -1, "aééééb", -1, 0, 3, 70, 1, "(15) 8 65 100 73 100 73 100 73 100 73 66 103 103 39 83", "ModeB a FNC4 é (4) b Pad (2)" },
        /* 21*/ { UNICODE_MODE, -1, "aéééééb", -1, 0, 3, 70, 0, "(15) 8 65 100 73 100 73 100 73 100 73 100 73 66 74 106", "ModeB a FNC4 é (5) b; BWIPP different encodation, uses double FNC4 latch" },
        /* 22*/ { UNICODE_MODE, -1, "aééééébcdeé", -1, 0, 4, 70, 0, "(20) 15 65 100 73 100 73 100 73 100 73 100 73 66 67 68 69 100 73 14 69", "ModeB a FNC4 é (5) b c d e FNC4 é; BWIPP different encodation, uses double FNC4 latch then FNC4 escapes" },
        /* 23*/ { UNICODE_MODE, -1, "123456789012345678901234", -1, 0, 3, 70, 1, "(15) 9 12 34 56 78 90 12 34 56 78 90 12 34 71 42", "3 rows" },
        /* 24*/ { UNICODE_MODE, 2, "123456789012345678901234", -1, 0, 3, 70, 0, "(15) 9 12 34 56 78 90 12 34 56 78 90 12 34 71 42", "Min 2 rows (no change); BWIPP doesn't support min rows < required" },
        /* 25*/ { UNICODE_MODE, 3, "123456789012345678901234", -1, 0, 3, 70, 1, "(15) 9 12 34 56 78 90 12 34 56 78 90 12 34 71 42", "Min 3 rows (no change)" },
        /* 26*/ { UNICODE_MODE, 4, "123456789012345678901234", -1, 0, 4, 70, 1, "(20) 16 12 34 56 78 90 12 34 56 78 90 12 34 103 103 103 103 103 66 96", "Min 4 rows" },
        /* 27*/ { UNICODE_MODE, 5, "123456789012345678901234", -1, 0, 5, 70, 1, "(25) 23 12 34 56 78 90 12 34 56 78 90 12 34 103 103 103 103 103 103 103 103 103 103 68 61", "Min 5 rows" },
        /* 28*/ { UNICODE_MODE, 16, "123456789012345678901234", -1, 0, 16, 70, 1, "(80) 100 12 34 56 78 90 12 34 56 78 90 12 34 103 103 103 103 103 103 103 103 103 103 103", "Min 16 rows" },
        /* 29*/ { UNICODE_MODE, 1, "123456789012345678901234", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, 1, "Error 424: Minimum number of rows '1' out of range (2 to 16)", "" },
        /* 30*/ { UNICODE_MODE, 17, "123456789012345678901234", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, 1, "Error 424: Minimum number of rows '17' out of range (2 to 16)", "" },
        /* 31*/ { UNICODE_MODE, -1, "ÁÁÁÁÁÁ99999999999999Á", -1, 0, 6, 70, 0, "(30) 29 100 33 100 33 100 33 100 33 100 33 100 33 99 99 99 99 99 99 99 99 100 100 33 103", "BWIPP different encodation, uses double FNC4 latch" },
        /* 32*/ { UNICODE_MODE, -1, "ÿ\012àa\0121\012àAà", -1, 0, 4, 70, 0, "(20) 15 100 95 98 74 100 64 65 98 74 17 98 74 100 64 33 100 64 47 35", "BWIPP different encodation, uses CodeA instead of 1SA" },
        /* 33*/ { UNICODE_MODE, -1, "ÿ\012àa\0121\012àAà\012à", -1, 0, 5, 70, 0, "(25) 22 100 95 98 74 100 64 65 98 74 17 98 74 100 64 33 100 64 98 74 100 64 103 89 18", "BWIPP different encodation, uses CodeA instead of 1SA" },
        /* 34*/ { UNICODE_MODE, -1, "y1234\012àa\0121\0127890àAàDà\012à", -1, 0, 7, 70, 0, "(35) 40 89 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33 100 64 36 100", "BWIPP different encodation, uses Sh2B + other differences" },
        /* 35*/ { UNICODE_MODE, -1, "ÿ1234\012àa\0121\0127890àAàDà\012à", -1, 0, 7, 70, 0, "(35) 41 100 95 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33 100 64 36", "BWIPP different encodation" },
        /* 36*/ { UNICODE_MODE, -1, "yÿ1234\012àa\0121\0127890àAàDà\012à", -1, 0, 8, 70, 0, "(40) 43 89 100 95 99 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33 100", "BWIPP different encodation (and fits in 7 rows)" },
        /* 37*/ { UNICODE_MODE, -1, "ÿy1234\012àa\0121\0127890àAàDà\012à", -1, 0, 8, 70, 0, "(40) 43 100 95 89 99 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33 100", "BWIPP different encodation (and fits in 7 rows)" },
        /* 38*/ { UNICODE_MODE, -1, "ÿÿ1234\012àa\0121\0127890àAàDà\012à", -1, 0, 8, 70, 0, "(40) 43 100 95 100 95 99 12 34 101 74 100 100 64 65 98 74 17 98 74 99 78 90 100 100 64 33", "BWIPP different encodation (and fits in 7 rows)" },
        /* 39*/ { UNICODE_MODE, -1, "ÿ1234\012àa\0121\01223456\0127890àAàDà\012à", -1, 0, 8, 70, 0, "(40) 48 100 95 12 34 101 74 100 100 64 65 98 74 17 98 74 18 99 34 56 101 74 99 78 90 100", "BWIPP different encodation, uses Sh2B + other differences" },
        /* 40*/ { UNICODE_MODE, -1, "ÿ1234\012à1234a\0121\01223456\0127890àAàDà\012à", -1, 0, 9, 70, 0, "(45) 55 100 95 12 34 101 74 101 98 64 99 12 34 100 65 98 74 17 98 74 18 99 34 56 101 74 99", "BWIPP different encodation, uses Sh2C + other differences" },
        /* 41*/ { UNICODE_MODE, -1, "ÿ1234\012à1234ab\0121\01223456\012\0127890àAàBCDà\012\012à", -1, 0, 10, 70, 0, "(50) 62 100 95 12 34 101 74 101 98 64 99 12 34 100 65 66 98 74 17 98 74 18 99 34 56 101 74", "BWIPP different encodation, uses Sh2C + other differences" },
        /* 42*/ { UNICODE_MODE, -1, "ÿ123456\012à123456abcd\0121\01223456\012\0127890àAàBCDEà\012\012à", -1, 0, 11, 70, 0, "(55) 69 100 95 12 34 56 101 74 101 98 64 99 12 34 56 100 65 66 67 68 98 74 17 98 74 18 99", "BWIPP different encodation, uses Sh3C + other differences" },
        /* 43*/ { UNICODE_MODE, -1, "ÿ12345678\012à12345678abcdef\0121\01223456\012\0127890àAàBCDEFà\012\012à", -1, 0, 12, 70, 0, "(60) 76 100 95 12 34 56 78 101 74 101 98 64 99 12 34 56 78 100 65 66 67 68 69 70 98 74 17", "BWIPP different encodation, uses Sh2C + other differences" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/, data[i].option_1, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %d, %s, %d, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, symbol->errtxt, data[i].comment);
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
        int input_mode;
        int option_1;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
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

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/, data[i].option_1, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_reader_init", test_reader_init },
        { "test_input", test_input },
        { "test_encode", test_encode },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
