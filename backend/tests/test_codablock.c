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
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /*
       é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, "A", 2666, 0, 44, 728, "" },
        /*  1*/ { -1, -1, "A", 2725, 0, 44, 739, "" },
        /*  2*/ { -1, -1, "A", 2726, 0, 44, 739, "" }, /* 4.2.1 c.3 says max 2725 but actually 44 * 62 - 2 == 2726 as mentioned later in 4.8.1 */
        /*  3*/ { -1, -1, "A", 2727, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /*  4*/ { -1, -1, "A", ZINT_MAX_DATA_LEN, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /*  5*/ { -1, -1, "12", 2726 * 2, 0, 44, 739, "" },
        /*  6*/ { -1, -1, "12", 2726 * 2 + 1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /*  7*/ { -1, -1, "\351", 2726 / 2, 0, 44, 739, "" },
        /*  8*/ { -1, -1, "\351", 2726 / 2 + 1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /*  9*/ { -1, -1, "\001", 2726, 0, 44, 739, "" },
        /* 10*/ { -1, -1, "\001", 2727, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 11*/ { 1, -1, "A", 101, 0, 1, 1146, "" }, /* CODE128 102 max (including start code) */
        /* 12*/ { 1, -1, "A", 102, ZINT_ERROR_TOO_LONG, -1, -1, "Error 341: Input too long, requires 103 symbol characters (maximum 102)" },
        /* 13*/ { 2, -1, "A", 122, 0, 2, 739, "" },
        /* 14*/ { 2, 10, "A", 122, 0, 2, 739, "" }, /* Cols 10 -> 67 */
        /* 15*/ { 2, 67, "A", 122, 0, 2, 739, "" },
        /* 16*/ { 2, -1, "A", 123, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 17*/ { 2, -1, "A", 63 * 2, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" }, /* Triggers initial testColumns > 62 */
        /* 18*/ { 2, -1, "A", 2726 * 2 + 1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 19*/ { 2, 9, "A", 2726 * 2 + 1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 20*/ { 3, -1, "A", 184, 0, 3, 739, "" },
        /* 21*/ { 3, -1, "A", 185, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 22*/ { 10, -1, "A", 618, 0, 10, 739, "" },
        /* 23*/ { 10, -1, "A", 619, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 24*/ { 20, -1, "A", 1238, 0, 20, 739, "" },
        /* 25*/ { 20, -1, "A", 1239, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 26*/ { 30, -1, "A", 1858, 0, 30, 739, "" },
        /* 27*/ { 30, -1, "A", 1859, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 28*/ { 40, -1, "A", 2478, 0, 40, 739, "" },
        /* 29*/ { 40, -1, "A", 2479, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 30*/ { 43, -1, "A", 2664, 0, 43, 739, "" },
        /* 31*/ { 43, -1, "A", 2665, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 32*/ { 44, -1, "A", 2726, 0, 44, 739, "" },
        /* 33*/ { 44, -1, "A", 2727, ZINT_ERROR_TOO_LONG, -1, -1, "Error 413: Input too long, requires too many symbol characters (maximum 2726)" },
        /* 34*/ { 44, 60, "A", 2726, 0, 44, 739, "" }, /* Cols 60 -> 67 */
        /* 35*/ { 44, 67, "A", 2726, 0, 44, 739, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[ZINT_MAX_DATA_LEN + 2];

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_CODABLOCKF, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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

static void test_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 1, -1, "é", 0, 1, 57, "", "CODE128" },
        /*  1*/ { UNICODE_MODE, -1, -1, "A", 0, 2, 101, "", "Defaults" },
        /*  2*/ { UNICODE_MODE, 0, -1, "A", 0, 2, 101, "", "0 rows same as -1" },
        /*  3*/ { UNICODE_MODE, 2, -1, "A", 0, 2, 101, "", "Rows 2, columns default" },
        /*  4*/ { UNICODE_MODE, 3, -1, "A", 0, 3, 101, "", "Rows 3" },
        /*  5*/ { UNICODE_MODE, 43, -1, "A", 0, 43, 101, "", "Rows 43" },
        /*  6*/ { UNICODE_MODE, 44, -1, "A", 0, 44, 101, "", "Max rows" },
        /*  7*/ { UNICODE_MODE, 45, -1, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 410: Number of rows '45' out of range (0 to 44)", "" },
        /*  8*/ { UNICODE_MODE, -1, -1, "abcdefg", 0, 3, 101, "", "" },
        /*  9*/ { UNICODE_MODE, 2, -1, "abcdefg", 0, 2, 112, "", "Rows given so columns expanded" },
        /* 10*/ { UNICODE_MODE, 3, -1, "abcdefg", 0, 3, 101, "", "" },
        /* 11*/ { UNICODE_MODE, -1, 8, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 411: Number of columns '8' out of range (9 to 67)", "Min columns 9 (4 data)" },
        /* 12*/ { UNICODE_MODE, -1, 9, "A", 0, 2, 101, "", "Min columns 9 (4 data)" },
        /* 13*/ { UNICODE_MODE, -1, 10, "A", 0, 2, 112, "", "Columns 10" },
        /* 14*/ { UNICODE_MODE, -1, 66, "A", 0, 2, 728, "", "Columns 66" },
        /* 15*/ { UNICODE_MODE, -1, 67, "A", 0, 2, 739, "", "Max columns 67 (62 data)" },
        /* 16*/ { UNICODE_MODE, -1, 68, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 411: Number of columns '68' out of range (9 to 67)", "" },
        /* 17*/ { UNICODE_MODE, 2, 9, "A", 0, 2, 101, "", "Rows and columns defaults given" },
        /* 18*/ { UNICODE_MODE, 2, 10, "A", 0, 2, 112, "", "Rows and columns given" },
        /* 19*/ { UNICODE_MODE, 3, 11, "A", 0, 3, 123, "", "" },
        /* 20*/ { UNICODE_MODE, 43, 66, "A", 0, 43, 728, "", "" },
        /* 21*/ { UNICODE_MODE, 44, 67, "A", 0, 44, 739, "", "Max rows, max columns" },
        /* 22*/ { GS1_MODE, -1, -1, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 220: Selected symbology does not support GS1 mode", "" },
        /* 23*/ { GS1_MODE, 1, -1, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 220: Selected symbology does not support GS1 mode", "Check for CODE128" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_options", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODABLOCKF, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
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
        /*  0*/ { BARCODE_CODABLOCKF, UNICODE_MODE, READER_INIT, "1234", 0, 2, 101, "67 64 40 60 63 0C 22 2B 6A 67 64 0B 63 64 3A 1C 29 6A", "CodeB FNC3 CodeC 12 34 / CodeB Pads" },
        /*  1*/ { BARCODE_CODABLOCKF, UNICODE_MODE, READER_INIT, "\001\002", 0, 2, 101, "67 62 40 60 41 42 63 32 6A 67 64 0B 63 64 45 42 0F 6A", "FNC3 SOH STX / CodeB Pads" },
        /*  2*/ { BARCODE_HIBC_BLOCKF, UNICODE_MODE, READER_INIT, "123456", 0, 3, 101, "67 64 41 60 0B 11 12 22 6A 67 63 2B 22 38 64 2A 1B 6A 67 64 0C 63 64 2B 2F 52 6A", "CodeB FNC3 + 1 2 / CodeC 34 56 CodeB J" },
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
                assert_zero(strcmp((char *) symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
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
        int option_1;
        int option_2;
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
       DEL U+007F (\177, 127), CodeB-only
       PAD U+0080 (\200, 128), UTF-8 C280 (\302\200), CodeA-only extended ASCII, not in ISO 8859-1
       APC U+009F (\237, 159), UTF-8 C29F, CodeA-only extended ASCII, not in ISO 8859-1
       NBSP U+00A0 (\240, 160), UTF-8 C2A0, CodeA and CodeB extended ASCII
       ß U+00DF (\337, 223), UTF-8 C39F, CodeA and CodeB extended ASCII
       à U+00E0 (\340, 224), UTF-8 C3A0, CodeB-only extended ASCII
       á U+00E1 (\341, 225), UTF-8 C3A1, CodeB-only extended ASCII
       é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
       ñ U+00F1 (\361, 241), UTF-8 C3B1, CodeB-only extended ASCII
       ÿ U+00FF (\377, 255), UTF-8 C3BF, CodeB-only extended ASCII
    */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "A", -1, 0, 2, 101, 1, "67 64 40 21 63 64 63 42 6A 67 64 0B 63 64 2B 40 4F 6A", "Fillings 5" },
        /*  1*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "AAA", -1, 0, 2, 101, 1, "67 64 40 21 21 21 63 55 6A 67 64 0B 63 64 0E 57 48 6A", "Fillings 3" },
        /*  2*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "AAAA", -1, 0, 2, 101, 1, "67 64 40 21 21 21 21 65 6A 67 64 0B 63 64 1A 0E 03 6A", "Fillings 2" },
        /*  3*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "AAAAA", -1, 0, 2, 101, 1, "67 64 40 21 21 21 21 65 6A 67 64 0B 21 63 1D 30 14 6A", "Fillings 1" },
        /*  4*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "AAAAAA", -1, 0, 2, 101, 1, "67 64 40 21 21 21 21 65 6A 67 64 0B 21 21 35 5D 2B 6A", "Fillings 0" },
        /*  5*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "1234", -1, 0, 2, 101, 1, "67 63 00 0C 22 64 63 1A 6A 67 64 0B 63 64 3A 1C 29 6A", "Fillings 4" },
        /*  6*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "12345", -1, 0, 2, 101, 1, "67 63 00 0C 22 64 15 49 6A 67 64 0B 63 64 41 44 07 6A", "Fillings 2 (not counting CodeB at end of 1st line)" },
        /*  7*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "123456", -1, 0, 2, 101, 1, "67 63 00 0C 22 38 64 12 6A 67 64 0B 63 64 2D 50 52 6A", "Fillings 3" },
        /*  8*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "1234567", -1, 0, 2, 101, 1, "67 63 00 0C 22 38 64 12 6A 67 64 0B 17 63 16 02 5B 6A", "Fillings 1 (not counting CodeB at end of 1st line)" },
        /*  9*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "12345678", -1, 0, 2, 101, 1, "67 63 00 0C 22 38 4E 5C 6A 67 64 0B 63 64 08 1C 64 6A", "Fillings 2" },
        /* 10*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "123456789", -1, 0, 2, 101, 1, "67 63 00 0C 22 38 4E 5C 6A 67 64 0B 19 63 25 4C 65 6A", "Fillings 1" },
        /* 11*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "1234567890", -1, 0, 2, 101, 0, "67 63 00 0C 22 38 4E 5C 6A 67 64 0B 19 10 41 38 62 6A", "Fillings 0; BWIPP different encodation (CodeB 9 0 instead of CodeC 90" },
        /* 12*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "A123ñ", -1, 0, 2, 101, 1, "67 64 40 21 11 12 13 54 6A 67 64 0B 64 51 42 28 50 6A", "K1/K2 example in Annex F" },
        /* 13*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "aß", -1, 0, 2, 101, 1, "67 64 40 41 64 3F 63 54 6A 67 64 0B 63 64 5B 1D 06 6A", "CodeB a FNC4 ß fits 1st line" },
        /* 14*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "\037ß", -1, 0, 2, 101, 1, "67 62 40 5F 65 3F 63 49 6A 67 64 0B 63 64 0F 1D 26 6A", "CodeA US FNC4 ß fits 1st line" },
        /* 15*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "aaß", -1, 0, 2, 101, 1, "67 64 40 41 41 64 3F 10 6A 67 64 0B 63 64 4E 5B 04 6A", "CodeB a a FNC4 ß fits 1st line" },
        /* 16*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "\037\037ß", -1, 0, 2, 101, 1, "67 62 40 5F 5F 65 3F 17 6A 67 64 0B 63 64 34 0F 24 6A", "CodeA US US FNC4 ß fits 1st line" },
        /* 17*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "aaaß", -1, 0, 2, 101, 1, "67 64 40 41 41 41 63 39 6A 67 64 0B 64 3F 4C 4E 50 6A", "CodeB a (3) / CodeB FNC4 ß fully on next line" },
        /* 18*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "\037\037\037ß", -1, 0, 2, 101, 1, "67 62 40 5F 5F 5F 63 03 6A 67 64 0B 64 3F 0E 34 1A 6A", "CodeA US (3) / CodeB FNC4 ß fully on next line" },
        /* 19*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "aà", -1, 0, 2, 101, 1, "67 64 40 41 64 40 63 59 6A 67 64 0B 63 64 5D 1E 16 6A", "CodeB a FNC4 à fits 1st line" },
        /* 20*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "\037à", -1, 0, 2, 101, 0, "67 62 40 5F 65 62 40 26 6A 67 64 0B 63 64 1B 1E 01 6A", "CodeA US FNC4 Shift à fits 1st line; BWIPP different encodation (CodeB instead of Shift)" },
        /* 21*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "\037àa", -1, 0, 2, 101, 0, "67 62 40 5F 64 64 40 2C 6A 67 64 0B 41 63 52 4A 16 6A", "CodeA US LatchB FNC4 à fits 1st line / Code B a; BWIPP diffent encodation (as above)" },
        /* 22*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "aaà", -1, 0, 2, 101, 1, "67 64 40 41 41 64 40 16 6A 67 64 0B 63 64 51 5D 1F 6A", "CodeB a a FNC4 à fits 1st line" },
        /* 23*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "\037\037à", -1, 0, 2, 101, 1, "67 62 40 5F 5F 63 64 1D 6A 67 64 0B 64 40 37 1B 55 6A", "CodeA US US / Code B FNC4 à fully on next line" },
        /* 24*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "aaaà", -1, 0, 2, 101, 1, "67 64 40 41 41 41 63 39 6A 67 64 0B 64 40 50 51 13 6A", "CodeB a (3) / Code B FNC4 à fully on next line" },
        /* 25*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "\037\037\037à", -1, 0, 2, 101, 1, "67 62 40 5F 5F 5F 63 03 6A 67 64 0B 64 40 1C 37 0F 6A", "CodeA US (3) / CodeB FNC4 à fully on next line" },
        /* 26*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, -1, "\037\200", -1, 0, 2, 101, 1, "67 62 40 5F 65 40 63 4E 6A 67 64 0B 63 64 5D 0A 05 6A", "CodeA US FNC4 PAD fits 1st line" },
        /* 27*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, -1, "\141\200", -1, 0, 2, 101, 0, "67 64 40 41 64 62 40 31 6A 67 64 0B 63 64 49 0A 08 6A", "CodeB a FNC4 Shift PAD fits 1st line; BWIPP different encodation (CodeA instead of Shift)" },
        /* 28*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, -1, "\141\200\037", -1, 0, 2, 101, 0, "67 64 40 41 65 65 40 44 6A 67 62 0B 5F 63 10 12 3E 6A", "CodeB a LatchA FNC4 PAD fits 1st line / CodeA US; BWIPP diffent encodation (as above)" },
        /* 29*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, -1, "\037\037\200", -1, 0, 2, 101, 1, "67 62 40 5F 5F 65 40 1D 6A 67 64 0B 63 64 0F 5D 0A 6A", "CodeA US US FNC4 PAD fits 1st line" },
        /* 30*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, -1, "\141\141\200", -1, 0, 2, 101, 1, "67 64 40 41 41 63 64 1B 6A 67 62 0B 65 40 33 49 21 6A", "CodeB a a / CodeA FNC4 PAD fully on next line" },
        /* 31*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, -1, "\037\037\037\200", -1, 0, 2, 101, 1, "67 62 40 5F 5F 5F 63 03 6A 67 62 0B 65 40 4A 0F 06 6A", "CodeA US (3) / CodeA FNC4 PAD fully on next line" },
        /* 32*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, -1, "\141\141\141\200", -1, 0, 2, 101, 1, "67 64 40 41 41 41 63 39 6A 67 62 0B 65 40 28 33 34 6A", "CodeB a (3) / CodeA FNC4 PAD fully on next line" },
        /* 33*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, 10, "\200\240\237\340\337\341\377", -1, 0, 4, 112, 0, "(40) 67 62 42 65 40 65 00 63 1E 6A 67 62 0B 65 5F 64 64 40 55 6A 67 64 0C 64 3F 64 41 63", "BWIPP different encodation (CodeB before FNC4 instead of after)" },
        /* 34*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "\000a\037\177}12", 7, 0, 3, 101, 1, "67 62 41 40 62 41 5F 3B 6A 67 64 0B 5F 5D 11 12 2D 6A 67 64 0C 63 64 40 05 26 6A", "" },
        /* 35*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "abcdéf", -1, 0, 3, 101, 1, "67 64 41 41 42 43 44 5D 6A 67 64 0B 64 49 46 63 0A 6A 67 64 0C 63 64 4F 26 02 6A", "" },
        /* 36*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "a12é\000", 6, 0, 3, 101, 0, "67 64 41 41 11 12 63 2C 6A 67 64 0B 64 49 62 40 2B 6A 67 64 0C 63 64 33 34 31 6A", "BWIPP different encodation (CodeA instead of Shift)" },
        /* 37*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, 11, "1234\001", -1, 0, 2, 123, 1, "67 63 00 0C 22 65 41 63 64 54 6A 67 64 0B 63 64 63 64 3F 20 24 6A", "" },
        /* 38*/ { BARCODE_CODABLOCKF, UNICODE_MODE, 3, -1, "ÁÁÁÁÁÁ99999999999999ÁÁÁÁÁÁÁ99999999999999Á", -1, 0, 3, 244, 1, "(66) 67 64 41 64 21 64 21 64 21 64 21 64 21 64 21 63 63 63 63 63 56 6A 67 63 2B 63 63 63", "Latching to extended ASCII not used by `codablockf()` (see test_code128 Okapi)" },
        /* 39*/ { BARCODE_CODABLOCKF, DATA_MODE, 2, -1, "@g(\302\302\302\302\3025555\302\302\302\302\302\302\302\302", -1, 0, 2, 255, 1, "(46) 67 64 40 20 47 08 64 22 64 22 64 22 64 22 64 22 63 37 37 64 63 4B 6A 67 64 0B 64 22", "Must allow for FNC4 when testing if enough room when switching from CodeC" },
        /* 40*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "ÿ\012àa\0121\012àAà", -1, 0, 5, 101, 0, "(45) 67 64 43 64 5F 65 4A 09 6A 67 64 0B 64 40 41 63 34 6A 67 62 0C 4A 11 4A 63 25 6A 67", "BWIPP different encoding (Shift instead of CodeA)" },
        /* 41*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "ÿy1234\012àa\0121\0127890àAàDà\012à", -1, 0, 7, 112, 0, "(70) 67 64 45 64 5F 59 11 12 2E 6A 67 62 0B 13 14 4A 63 64 43 6A 67 64 0C 64 40 41 65 4A", "BWIPP different encoding (Shift instead of CodeA)" },
        /* 42*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "ÿ12345678\012à12345678abcdef\0121\01223456\012\0127890àAàBCDEFà\012\012à", -1, 0, 8, 134, 0, "(96) 67 64 46 64 5F 63 0C 22 38 4E 5E 6A 67 62 0B 4A 64 64 40 63 0C 22 2B 6A 67 63 2C 38", "BWIPP different encoding (CodeB before FNC4 instead of after)" },
        /* 43*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "123456789012345ABCDEFGHI\012123456ÿ12345678\012à12345678abcdef\012\012123456\012\0127890àABCDEFà\012\012ààGHIJKàLMNOPQabc\012defà1234567ghijk\012\012à901234\012\012\012\012567890àààààààABCDEFGààà\012\012\012HIJK\012\012\012\012à\012à\012à\01212345à\012à\012à67890ààÄ9012ÄÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2B3C4a5b6c7d8e9\0120\0121\0122\0123Ä4Ä5Ä6A7a8A9a0\012Ä12345678ÄÄÄÄÄÄÄÄÄÄÄ2ÄÄÄÄÄÄÄÄ4ÄÄÄÄÄÄAÄÄÄÄÄÄaÄÄÄÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécé123456789012345ABCDEFGHI\012123456ÿ12345678\012à12345678abcdef\012\012123456\012\0127890àABCDEFà\012\012ààGHIJKàLMNOPQabc\012defà1234567ghijk\012\012à901234\012\012\012\012567890àààààààABCDEFGààà\012\012\012HIJK\012\012\012\012à\012à\012à\01212345à\012à\012à67890ààÄ9012ÄÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2B3C4a5b6c7d8e9\0120\0121\0122\0123Ä4Ä5Ä6A7a8A9a0\012Ä12345678ÄÄÄÄÄÄÄÄÄÄÄ2ÄÄÄÄÄÄÄÄ4ÄÄÄÄÄÄAÄÄÄÄÄÄaÄÄÄÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécé12345123456789012345ABCDEFGHI\012123456ÿ12345678\012à12345678abcdef\012\012123456\012\0127890àABCDEFà\012\012ààGHIJKàLMNOPQabc\012defà1234567ghijk\012\012à901234\012\012\012\012567890àààààààABCDEFGààà\012\012\012HIJK\012\012\012\012à\012à\012à\01212345à\012à\012à67890ààÄ9012ÄÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2B3C4a5b6c7d8e9\0120\0121\0122\0123Ä4Ä5Ä6A7a8A9a0\012Ä12345678ÄÄÄÄÄÄÄÄÄÄÄ2ÄÄÄÄÄÄÄÄ4ÄÄÄÄÄÄAÄÄÄÄÄÄaÄÄÄÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécé123456789012345ABCDEFGHI\012123456ÿ12345678\012à12345678abcdef\012\012123456\012\0127890àABCDEFà\012\012ààGHIJKàLMNOPQabc\012defà1234567ghijk\012\012à901234\012\012\012\012567890àààààààABCDEFGààà\012\012\012HIJK\012\012\012\012à\012à\012à\01212345à\012à\012à67890ààÄ9012ÄÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2" "B3C4a5b6c7d8e9\0120\0121\0122\0123Ä4Ä5Ä6A7a8A9a0\012Ä12345678ÄÄÄÄÄÄÄÄÄÄÄ2ÄÄÄÄÄÄÄÄ4ÄÄÄÄÄÄAÄÄÄÄÄÄaÄÄÄÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécé6789012345ABCDEFGHI\012123456ÿ12345678\012à12345678abcdef\012\012123456\012\0127890àABCDEFà\012\012ààGHIJKàLMNOPQabc\012defà1234567ghijk\012\012à901234\012\012\012\012567890àààààààABCDEFGààà\012\012\012HIJK\012\012\012\012à\012à\012à\01212345à\012à\012à67890ààÄ9012ÄÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2B3C4a5b6c7d8e9\0120\0121\0122\0123Ä4Ä5Ä6A7a8A9a0\012Ä12345678ÄÄÄÄÄÄÄÄÄÄÄ2ÄÄÄÄÄÄÄÄ4ÄÄÄÄÄÄAÄÄÄÄÄÄaÄÄÄÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécé123456789012345ABCDEFGHI\012123456ÿ12345678\012à12345678abcdef\012\012123456\012\0127890àABCDEFà\012\012ààGHIJKàLMNOPQabc\012defà1234567ghijk\012\012à901234\012\012\012\012567890àààààààABCDEFGààà\012\012\012HIJK\012\012\012\012à\012à\012à\012123456à\012à\012à\0123Ä4Ä5Ä6AÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2B3C4a5b6c7d8e9\0120\0121\0122\0123Ä4Ä5Ä6A7a8A9a0\012Ä12345678ÄÄÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécéÄÄÄÄÄÄ2ÄÄÄÄÄÄÄÄ4ÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécéÄÄÄAÄÄÄÄÄÄaÄÄÄÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécé", -1, 0, 44, 739, 0, "(2948) 67 63 2A 0C 22 38 4E 5A 0C 22 65 15 21 22 23 24 25 26 27 28 29 4A 63 0C 22 38 64", "BWIPP gs command too long" },
        /* 44*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, -1, "ÿ12345678\012à12345678abcdef\012\012123456\012\0127890àABCDEFà\012\012ààGHIJKàLMNOPQabc\012defà1234567ghijk\012\012à901234\012\012\012\012567890ààààABCDEFGààà\012\012\012HIJK\012\012\012\012à\012à\012à\01212345à\012à\012à67890ààÄ9012ÄÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2B3C4a5b6c7d8e9\0120\0121\0122\0123Ä4Ä5Ä6A7a8A9a0\012Ä12345678ÄÄÄÄÄÄ2ÄÄÄÄÄÄ4ÄÄÄÄÄÄaÄÄÄÄé1é2é34é56Ä78é9éAéBéCéééééaébécé123456789012345ABCDEFGHI\012123456ÿ12345678\012à12345678abcdef\012123456\012\0127890àABCDEFà\012\012ààGHIJKàLMNOPQabc\012defà1234567ghijk\012\012à901234\0122567890àààààABCDEFGààà\012\012\012HIJK\012\012\0122à\012à\012à\01212345à\012à\012à67890ààÄ9012ÄÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2B3C4a5b6c7d8e9\0120\0121\0122\0123Ä4Ä5Ä6A7a8A9a0\012Ä12345678ÄÄ2Ä4ÄaÄé1é2é34é56Ä78é9éAéBéCééaébécé123456789012345ABCDEF\012123456ÿ123456\012à12345678abcdef\012\0121234\012\0127890àABCDà\012\012ààGHIJKàLMabc\012defà1234567ghijk\012\012à901234\012\012\012\012567890ààABCDEFGààà\012\012\012HIJK\012\012\012\012à\012à\012à\01212345à\012à\012à67890ààÄ9012ÄÄ56789Ä12Ä3\0124\0125\0126A\012a\012A\012A\012a\012a\012BCD1A2", -1, 0, 33, 387, 0, "(1155) 67 64 5F 64 5F 63 0C 22 38 4E 65 4A 64 64 40 63 0C 22 38 4E 64 41 42 43 44 45 46", "BWIPP different encodation" },
        /* 45*/ { BARCODE_HIBC_BLOCKF, UNICODE_MODE, -1, -1, "A99912345/$$52001510X3", -1, 0, 6, 101, 1, "(54) 67 64 44 0B 21 19 19 3A 6A 67 63 2B 5B 17 2D 64 24 6A 67 64 0C 0F 04 04 15 16 6A 67", "" },
        /* 46*/ { BARCODE_HIBC_BLOCKF, UNICODE_MODE, -1, -1, "A99912345/$$520:1510X3", -1, ZINT_ERROR_INVALID_DATA, -1, -1, 1, "Error 203: Invalid character at position 16 in input (alphanumerics, space and \"-.$/+%\" only)", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[16834];
    char cmp_buf[32768];
    char cmp_msg[32768];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, \"%s\", %d, %s, %d, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp((char *) symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

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
        int option_1;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, 1, -1, "AIM", 0, 1, 68, 1, "Same as CODE128 (not supported by BWIPP or ZXing-C++)",
                    "11010010000101000110001100010001010111011000101110110001100011101011"
                },
        /*  1*/ { BARCODE_CODABLOCKF, -1, -1, "AAAAAAA", 0, 3, 101, 1, "Defaults to rows 3, columns 9 (4 data); verified manually against tec-it",
                    "11010000100101111011101001011000010100011000101000110001010001100010100011000110110011001100011101011"
                    "11010000100101111011101100010010010100011000101000110001010001100010111011110100100111101100011101011"
                    "11010000100101111011101011001110010111011110101111011101100001010011011101110111100101001100011101011"
                },
        /*  2*/ { BARCODE_CODABLOCKF, -1, -1, "AAAAAAAAAA", 0, 3, 101, 1, "Defaults to rows 3, columns 9 (4 data); verified manually against tec-it",
                    "11010000100101111011101001011000010100011000101000110001010001100010100011000110110011001100011101011"
                    "11010000100101111011101100010010010100011000101000110001010001100010100011000111101000101100011101011"
                    "11010000100101111011101011001110010100011000101000110001110110010010010110000111000101101100011101011"
                },
        /*  3*/ { BARCODE_CODABLOCKF, -1, -1, "AAAAAAAAAAA", 0, 4, 101, 1, "Defaults to rows 4, columns 9 (4 data); verified manually against tec-it",
                    "11010000100101111011101001000011010100011000101000110001010001100010100011000110011001101100011101011"
                    "11010000100101111011101100010010010100011000101000110001010001100010100011000111101000101100011101011"
                    "11010000100101111011101011001110010100011000101000110001010001100010111011110100111101001100011101011"
                    "11010000100101111011101001101110010111011110101111011101110101100011101100100110010111001100011101011"
                },
        /*  4*/ { BARCODE_CODABLOCKF, -1, -1, "AAAAAAAAAAAAAA", 0, 4, 101, 1, "Defaults to rows 4, columns 9 (4 data); verified manually against tec-it",
                    "11010000100101111011101001000011010100011000101000110001010001100010100011000110011001101100011101011"
                    "11010000100101111011101100010010010100011000101000110001010001100010100011000111101000101100011101011"
                    "11010000100101111011101011001110010100011000101000110001010001100010100011000101111011101100011101011"
                    "11010000100101111011101001101110010100011000101000110001011110100010111011000100110000101100011101011"
                },
        /*  5*/ { BARCODE_CODABLOCKF, -1, -1, "AAAAAAAAAAAAAAA", 0, 5, 101, 1, "Defaults to rows 5, columns 9 (4 data); verified manually against tec-it",
                    "11010000100101111011101000010110010100011000101000110001010001100010100011000100100011001100011101011"
                    "11010000100101111011101100010010010100011000101000110001010001100010100011000111101000101100011101011"
                    "11010000100101111011101011001110010100011000101000110001010001100010100011000101111011101100011101011"
                    "11010000100101111011101001101110010100011000101000110001010001100010111011110111101001001100011101011"
                    "11010000100101111011101001100111010111011110101111011101000110001010111101000110001010001100011101011"
                },
        /*  6*/ { BARCODE_CODABLOCKF, -1, 14, "AAAAAAAAAAAAAAA", 0, 2, 156, 1, "Rows 2, columns 14 (9 data); verified manually against tec-it",
                    "110100001001011110111010100001100101000110001010001100010100011000101000110001010001100010100011000101000110001010001100010100011000110001000101100011101011"
                    "110100001001011110111011000100100101000110001010001100010100011000101000110001010001100010100011000101110111101110111101011011000110111000101101100011101011"
                },
        /*  7*/ { BARCODE_CODABLOCKF, -1, -1, "AAAAAAAAAAAAAAAA", 0, 5, 101, 1, "Defaults to rows 5, columns 9 (4 data); verified manually against tec-it",
                    "11010000100101111011101000010110010100011000101000110001010001100010100011000100100011001100011101011"
                    "11010000100101111011101100010010010100011000101000110001010001100010100011000111101000101100011101011"
                    "11010000100101111011101011001110010100011000101000110001010001100010100011000101111011101100011101011"
                    "11010000100101111011101001101110010100011000101000110001010001100010100011000111101011101100011101011"
                    "11010000100101111011101001100111010111011110101111011101011100011010001100010100011101101100011101011"
                },
        /*  8*/ { BARCODE_CODABLOCKF, -1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAA", 0, 6, 112, 1, "Defaults to rows 6, columns 10 (5 data); verified manually against tec-it",
                    "1101000010010111101110100001001101010001100010100011000101000110001010001100010100011000110110001101100011101011"
                    "1101000010010111101110110001001001010001100010100011000101000110001010001100010100011000110010011101100011101011"
                    "1101000010010111101110101100111001010001100010100011000101000110001010001100010100011000110011101001100011101011"
                    "1101000010010111101110100110111001010001100010100011000101000110001010001100010100011000111010011001100011101011"
                    "1101000010010111101110100110011101010001100010100011000101000110001010001100010100011000111001001101100011101011"
                    "1101000010010111101110101110011001011101111010111101110101110111101110100011010100001100110001010001100011101011"
                },
        /*  9*/ { BARCODE_CODABLOCKF, 4, -1, "CODABLOCK F 34567890123456789010040digit", 0, 4, 145, 1, "AIM ISS-X-24 Figure 1",
                    "1101000010010111101110100100001101000100011010001110110101100010001010001100010001011000100011011101000111011010001000110110110011001100011101011"
                    "1101000010010111101110110001001001011000111011011001100100011000101101100110010111011110100010110001110001011011000010100101100111001100011101011"
                    "1101000010010111011110100011011101101111011010110011100100010110001110001011011000010100110111101101100100010010010001100100011000101100011101011"
                    "1101000010010111101110100110111001001110110010000100110100001101001001101000010000110100100111101001101110111010111000110110010000101100011101011"
                },
        /* 10*/ { BARCODE_CODABLOCKF, 3, -1, "CODABLOCK F Symbology", 0, 3, 145, 1, "AIM ISS-X-24 Figure on front page",
                    "1101000010010111101110100101100001000100011010001110110101100010001010001100010001011000100011011101000111011010001000110111010111101100011101011"
                    "1101000010010111101110110001001001011000111011011001100100011000101101100110011011101000110110111101111011101010010000110100100111101100011101011"
                    "1101000010010111101110101100111001000111101011001010000100011110101001101000011011011110101110111101000011001011011101110101001111001100011101011"
                },
        /* 11*/ { BARCODE_CODABLOCKF, -1, -1, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 0, 11, 156, 1, "Visible ASCII",
                    "110100001001011110111010000110100110110011001100110110011001100110100100110001001000110010001001100100110010001001100010010001100100101100011101100011101011"
                    "110100001001011110111011000100100110010010001100100010011000100100101100111001001101110010011001110101110011001001110110010011100110100001100101100011101011"
                    "110100001001011101111010001101110111011011101011101100010000101100110110111101011110111011100100110111011001001110011010011100110010100001001101100011101011"
                    "110100001001011110111010011011100110110110001101100011011000110110101000110001000101100010001000110101100010001000110100010001100010111100010101100011101011"
                    "110100001001011110111010011001110110100010001100010100011000100010101101110001011000111010001101110101110110001011100011010001110110100110111001100011101011"
                    "110100001001011110111010111001100111011101101101000111011000101110110111010001101110001011011101110111010110001110100011011100010110100001011001100011101011"
                    "110100001001011110111011100100110111011010001110110001011100011010111011110101100100001011110001010101001100001010000110010010110000100011000101100011101011"
                    "110100001001011110111011101100100100100001101000010110010000100110101100100001011000010010011010000100110000101000011010010000110010101011110001100011101011"
                    "110100001001011110111011100110100110000100101100101000011110111010110000101001000111101010100111100100101111001001001111010111100100101100011101100011101011"
                    "110100001001011110111011100110010100111101001001111001011110100100111100101001111001001011011011110110111101101111011011010101111000111101010001100011101011"
                    "110100001001011110111011011011000101000111101000101111010111011110101111011101011101111010111101110101110111101011100011011101101110101001100001100011101011"
                },
        /* 12*/ { BARCODE_HIBC_BLOCKF, 3, -1, "A123BJC5D6E71", 0, 3, 123, 0, "Verified manually against tec-it; differs from BWIPP (columns=6) which uses Code C for final 71 (same no. of codewords)",
                    "110100001001011110111010010110000110001001001010001100010011100110110011100101100101110010001011000100100001101100011101011"
                    "110100001001011110111011000100100101101110001000100011011011100100101100010001100111010010001101000111001001101100011101011"
                    "110100001001011110111010110011100111011011101001110011011010001000101110111101011100011011001110100100100110001100011101011"
                },
        /* 13*/ { BARCODE_HIBC_BLOCKF, -1, -1, "$$52001510X3G", 0, 4, 101, 1, "tec-it differs as adds unnecessary Code C at end of 1st line",
                    "11010000100101111011101001000011011000100100100100011001001000110011011100100101110011001100011101011"
                    "11010000100101110111101011000111011001001110110011011001101110100010111101110100001100101100011101011"
                    "11010000100101111011101011001110010011101100111000101101100101110011010001000100100011001100011101011"
                    "11010000100101111011101001101110010110001000101110111101101000111011000110110100011010001100011101011"
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

        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].option_1, data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
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
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
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

/* #181 Christian Hartlage OSS-Fuzz and #300 Andre Maute */
static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_1;
        int option_2;
        char *data;
        int length;
        int ret;
        int bwipp_cmp;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { -1, -1, "\034\034I", 3, 0, 1, "" },
        /*  1*/ { 6, -2147483647 - 1 /*Suppress MSVC warning C4146*/,
                    "\134\000\377\153\143\163\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061"
                    "\071\065\062\000\000\000\000\061\061\061\061\061\061\366\366\366\366\366\366\366\366\366\366\007\366\366\366\366\366\366\366\061\061\061\061\061\061\061\061\061"
                    "\061\061\061\061\061\061\061\323\323\323\323\000\200\135\000\362\000\000\000\000\000\050\000\000\000\000\162\162\162\162\034\153\143\163\061\061\061\061\061\061"
                    "\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\312\061\061\061\061\061\061\061"
                    "\061\366\366\366\366\366\366\366\366\366\366\007\366\366\366\366\366\366\366\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\061\323\323\323\323\000"
                    "\200\135\000\362\362\362\362\000\167\027\000\000\000\000\000\000\000\000\000\167\167\167\167\000\000\000\154\000\043\000\000\000\000\000\000\000\000\153",
                    238, 0, 0, "BWIPP different (better) encodation"
                }, /* #300 (#9) Andre Maute */
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

        length = testUtilSetSymbol(symbol, BARCODE_CODABLOCKF, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_options", test_options },
        { "test_reader_init", test_reader_init },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_fuzz", test_fuzz },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
