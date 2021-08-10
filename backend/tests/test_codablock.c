/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2021 Robin Stuart <rstuart114@gmail.com>

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
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"

static void test_large(int index, int debug) {

    struct item {
        int option_1;
        int option_2;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -1, -1, "A", 2666, 0, 44, 728 },
        /*  1*/ { -1, -1, "A", 2725, 0, 44, 739 },
        /*  2*/ { -1, -1, "A", 2726, 0, 44, 739 }, // 4.2.1 c.3 says max 2725 but actually 44 * 62 - 2 == 2726 as mentioned later in 4.8.1
        /*  3*/ { -1, -1, "A", 2727, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { -1, -1, "A", ZINT_MAX_DATA_LEN, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  5*/ { -1, -1, "12", 2726 * 2, 0, 44, 739 },
        /*  6*/ { -1, -1, "12", 2726 * 2 + 1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  7*/ { -1, -1, "\351", 2726 / 2, 0, 44, 739 },
        /*  8*/ { -1, -1, "\351", 2726 / 2 + 1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  9*/ { 1, -1, "A", 60, 0, 1, 695 }, // CODE128 60 max
        /* 10*/ { 1, -1, "A", 61, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 11*/ { 2, -1, "A", 122, 0, 2, 739 },
        /* 12*/ { 2, 10, "A", 122, 0, 2, 739 }, // Cols 10 -> 67
        /* 13*/ { 2, 67, "A", 122, 0, 2, 739 },
        /* 14*/ { 2, -1, "A", 123, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 15*/ { 2, -1, "A", 63 * 2, ZINT_ERROR_TOO_LONG, -1, -1 }, // Triggers initial testColumns > 62
        /* 16*/ { 2, -1, "A", 2726 * 2 + 1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 17*/ { 2, 9, "A", 2726 * 2 + 1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 18*/ { 3, -1, "A", 184, 0, 3, 739 },
        /* 19*/ { 3, -1, "A", 185, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 20*/ { 10, -1, "A", 618, 0, 10, 739 },
        /* 21*/ { 10, -1, "A", 619, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 22*/ { 20, -1, "A", 1238, 0, 20, 739 },
        /* 23*/ { 20, -1, "A", 1239, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 24*/ { 30, -1, "A", 1858, 0, 30, 739 },
        /* 25*/ { 30, -1, "A", 1859, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 26*/ { 40, -1, "A", 2478, 0, 40, 739 },
        /* 27*/ { 40, -1, "A", 2479, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 28*/ { 43, -1, "A", 2664, 0, 43, 739 },
        /* 29*/ { 43, -1, "A", 2665, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 30*/ { 44, -1, "A", 2726, 0, 44, 739 },
        /* 31*/ { 44, -1, "A", 2727, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 32*/ { 44, 60, "A", 2726, 0, 44, 739 }, // Cols 60 -> 67
        /* 33*/ { 44, 67, "A", 2726, 0, 44, 739 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[ZINT_MAX_DATA_LEN + 2];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_CODABLOCKF, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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

static void test_options(int index, int debug) {

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 1, -1, "é", 0, 1, 57, "CODE128" },
        /*  1*/ { UNICODE_MODE, -1, -1, "A", 0, 2, 101, "Defaults" },
        /*  2*/ { UNICODE_MODE, 0, -1, "A", 0, 2, 101, "0 rows same as -1" },
        /*  3*/ { UNICODE_MODE, 2, -1, "A", 0, 2, 101, "Rows 2, columns default" },
        /*  4*/ { UNICODE_MODE, 3, -1, "A", 0, 3, 101, "Rows 3" },
        /*  5*/ { UNICODE_MODE, 43, -1, "A", 0, 43, 101, "Rows 43" },
        /*  6*/ { UNICODE_MODE, 44, -1, "A", 0, 44, 101, "Max rows" },
        /*  7*/ { UNICODE_MODE, 45, -1, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "" },
        /*  8*/ { UNICODE_MODE, -1, -1, "abcdefg", 0, 3, 101, "" },
        /*  9*/ { UNICODE_MODE, 2, -1, "abcdefg", 0, 2, 112, "Rows given so columns expanded" },
        /* 10*/ { UNICODE_MODE, 3, -1, "abcdefg", 0, 3, 101, "" },
        /* 11*/ { UNICODE_MODE, -1, 8, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "Min columns 9 (4 data)" },
        /* 12*/ { UNICODE_MODE, -1, 9, "A", 0, 2, 101, "Min columns 9 (4 data)" },
        /* 13*/ { UNICODE_MODE, -1, 10, "A", 0, 2, 112, "Columns 10" },
        /* 14*/ { UNICODE_MODE, -1, 66, "A", 0, 2, 728, "Columns 66" },
        /* 15*/ { UNICODE_MODE, -1, 67, "A", 0, 2, 739, "Max columns 67 (62 data)" },
        /* 16*/ { UNICODE_MODE, -1, 68, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "" },
        /* 17*/ { UNICODE_MODE, 2, 9, "A", 0, 2, 101, "Rows and columns defaults given" },
        /* 18*/ { UNICODE_MODE, 2, 10, "A", 0, 2, 112, "Rows and columns given" },
        /* 19*/ { UNICODE_MODE, 3, 11, "A", 0, 3, 123, "" },
        /* 20*/ { UNICODE_MODE, 43, 66, "A", 0, 43, 728, "" },
        /* 21*/ { UNICODE_MODE, 44, 67, "A", 0, 44, 739, "Max rows, max columns" },
        /* 22*/ { GS1_MODE, -1, -1, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "GS1 not supported" },
        /* 23*/ { GS1_MODE, 1, -1, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, "Check for CODE128" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODABLOCKF, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(int index, int generate, int debug) {

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
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, UNICODE_MODE, READER_INIT, "1234", 0, 2, 101, "67 64 40 60 63 0C 22 2B 6A 67 64 0B 63 64 3A 1C 29 6A", "CodeB FNC3 CodeC 12 34 / CodeB Pads" },
        /*  1*/ { BARCODE_CODABLOCKF, UNICODE_MODE, READER_INIT, "\001\002", 0, 2, 101, "67 62 40 60 41 42 63 32 6A 67 64 0B 63 64 45 42 0F 6A", "FNC3 SOH STX / CodeB Pads" },
        /*  2*/ { BARCODE_HIBC_BLOCKF, UNICODE_MODE, READER_INIT, "123456", 0, 3, 101, "67 64 41 60 0B 11 12 22 6A 67 63 2B 22 38 64 2A 1B 6A 67 64 0C 63 64 2B 2F 52 6A", "CodeB FNC3 + 1 2 / CodeC 34 56 CodeB J" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_reader_init");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
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

static void test_input(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int option_2;
        char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    // NUL U+0000, CodeA-only
    // US U+001F (\037, 31), CodeA-only
    // a U+0061 (\141, 97), CodeB-only
    // DEL U+007F (\177, 127), CodeB-only
    // PAD U+0080 (\200, 128), UTF-8 C280 (\302\200), CodeA-only extended ASCII, not in ISO 8859-1
    // APC U+009F (\237, 159), UTF-8 C29F, CodeA-only extended ASCII, not in ISO 8859-1
    // NBSP U+00A0 (\240, 160), UTF-8 C2A0, CodeA and CodeB extended ASCII
    // ß U+00DF (\337, 223), UTF-8 C39F, CodeA and CodeB extended ASCII
    // à U+00E0 (\340, 224), UTF-8 C3A0, CodeB-only extended ASCII
    // á U+00E1 (\341, 225), UTF-8 C3A1, CodeB-only extended ASCII
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    // ñ U+00F1 (\361, 241), UTF-8 C3B1, CodeB-only extended ASCII
    // ÿ U+00FF (\377, 255), UTF-8 C3BF, CodeB-only extended ASCII
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "A", -1, 0, 2, 101, "67 64 40 21 63 64 63 42 6A 67 64 0B 63 64 2B 40 4F 6A", "Fillings 5" },
        /*  1*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "AAA", -1, 0, 2, 101, "67 64 40 21 21 21 63 55 6A 67 64 0B 63 64 0E 57 48 6A", "Fillings 3" },
        /*  2*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "AAAA", -1, 0, 2, 101, "67 64 40 21 21 21 21 65 6A 67 64 0B 63 64 1A 0E 03 6A", "Fillings 2" },
        /*  3*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "AAAAA", -1, 0, 2, 101, "67 64 40 21 21 21 21 65 6A 67 64 0B 21 63 1D 30 14 6A", "Fillings 1" },
        /*  4*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "AAAAAA", -1, 0, 2, 101, "67 64 40 21 21 21 21 65 6A 67 64 0B 21 21 35 5D 2B 6A", "Fillings 0" },
        /*  5*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "1234", -1, 0, 2, 101, "67 63 00 0C 22 64 63 1A 6A 67 64 0B 63 64 3A 1C 29 6A", "Fillings 4" },
        /*  6*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "12345", -1, 0, 2, 101, "67 63 00 0C 22 64 15 49 6A 67 64 0B 63 64 41 44 07 6A", "Fillings 2 (not counting CodeB at end of 1st line)" },
        /*  7*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "123456", -1, 0, 2, 101, "67 63 00 0C 22 38 64 12 6A 67 64 0B 63 64 2D 50 52 6A", "Fillings 3" },
        /*  8*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "1234567", -1, 0, 2, 101, "67 63 00 0C 22 38 64 12 6A 67 64 0B 17 63 16 02 5B 6A", "Fillings 1 (not counting CodeB at end of 1st line)" },
        /*  9*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "12345678", -1, 0, 2, 101, "67 63 00 0C 22 38 4E 5C 6A 67 64 0B 63 64 08 1C 64 6A", "Fillings 2" },
        /* 10*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "123456789", -1, 0, 2, 101, "67 63 00 0C 22 38 4E 5C 6A 67 64 0B 19 63 25 4C 65 6A", "Fillings 1" },
        /* 11*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "1234567890", -1, 0, 2, 101, "67 63 00 0C 22 38 4E 5C 6A 67 64 0B 19 10 41 38 62 6A", "Fillings 0" },
        /* 12*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "A123ñ", -1, 0, 2, 101, "67 64 40 21 11 12 13 54 6A 67 64 0B 64 51 42 28 50 6A", "K1/K2 example in Annex F" },
        /* 13*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "aß", -1, 0, 2, 101, "67 64 40 41 64 3F 63 54 6A 67 64 0B 63 64 5B 1D 06 6A", "CodeB a FNC4 ß fits 1st line" },
        /* 14*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "\037ß", -1, 0, 2, 101, "67 62 40 5F 65 3F 63 49 6A 67 64 0B 63 64 0F 1D 26 6A", "CodeA US FNC4 ß fits 1st line" },
        /* 15*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "aaß", -1, 0, 2, 101, "67 64 40 41 41 64 3F 10 6A 67 64 0B 63 64 4E 5B 04 6A", "CodeB a a FNC4 ß fits 1st line" },
        /* 16*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "\037\037ß", -1, 0, 2, 101, "67 62 40 5F 5F 65 3F 17 6A 67 64 0B 63 64 34 0F 24 6A", "CodeA US US FNC4 ß fits 1st line" },
        /* 17*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "aaaß", -1, 0, 2, 101, "67 64 40 41 41 41 63 39 6A 67 64 0B 64 3F 4C 4E 50 6A", "CodeB a (3) / CodeB FNC4 ß fully on next line" },
        /* 18*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "\037\037\037ß", -1, 0, 2, 101, "67 62 40 5F 5F 5F 63 03 6A 67 64 0B 64 3F 0E 34 1A 6A", "CodeA US (3) / CodeB FNC4 ß fully on next line" },
        /* 19*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "aà", -1, 0, 2, 101, "67 64 40 41 64 40 63 59 6A 67 64 0B 63 64 5D 1E 16 6A", "CodeB a FNC4 à fits 1st line" },
        /* 20*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "\037à", -1, 0, 2, 101, "67 62 40 5F 65 62 40 26 6A 67 64 0B 63 64 1B 1E 01 6A", "CodeA US FNC4 Shift à fits 1st line" },
        /* 21*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "\037àa", -1, 0, 2, 101, "67 62 40 5F 64 64 40 2C 6A 67 64 0B 41 63 52 4A 16 6A", "CodeA US LatchB FNC4 à fits 1st line / Code B a" },
        /* 22*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "aaà", -1, 0, 2, 101, "67 64 40 41 41 64 40 16 6A 67 64 0B 63 64 51 5D 1F 6A", "CodeB a a FNC4 à fits 1st line" },
        /* 23*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "\037\037à", -1, 0, 2, 101, "67 62 40 5F 5F 63 64 1D 6A 67 64 0B 64 40 37 1B 55 6A", "CodeA US US / Code B FNC4 à fully on next line" },
        /* 24*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "aaaà", -1, 0, 2, 101, "67 64 40 41 41 41 63 39 6A 67 64 0B 64 40 50 51 13 6A", "CodeB a (3) / Code B FNC4 à fully on next line" },
        /* 25*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "\037\037\037à", -1, 0, 2, 101, "67 62 40 5F 5F 5F 63 03 6A 67 64 0B 64 40 1C 37 0F 6A", "CodeA US (3) / CodeB FNC4 à fully on next line" },
        /* 26*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\037\200", -1, 0, 2, 101, "67 62 40 5F 65 40 63 4E 6A 67 64 0B 63 64 5D 0A 05 6A", "CodeA US FNC4 PAD fits 1st line" },
        /* 27*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\141\200", -1, 0, 2, 101, "67 64 40 41 64 62 40 31 6A 67 64 0B 63 64 49 0A 08 6A", "CodeB a FNC4 Shift PAD fits 1st line" },
        /* 28*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\141\200\037", -1, 0, 2, 101, "67 64 40 41 65 65 40 44 6A 67 62 0B 5F 63 10 12 3E 6A", "CodeB a LatchA FNC4 PAD fits 1st line / CodeA US" },
        /* 29*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\037\037\200", -1, 0, 2, 101, "67 62 40 5F 5F 65 40 1D 6A 67 64 0B 63 64 0F 5D 0A 6A", "CodeA US US FNC4 PAD fits 1st line" },
        /* 30*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\141\141\200", -1, 0, 2, 101, "67 64 40 41 41 63 64 1B 6A 67 62 0B 65 40 33 49 21 6A", "CodeB a a / CodeA FNC4 PAD fully on next line" },
        /* 31*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\037\037\037\200", -1, 0, 2, 101, "67 62 40 5F 5F 5F 63 03 6A 67 62 0B 65 40 4A 0F 06 6A", "CodeA US (3) / CodeA FNC4 PAD fully on next line" },
        /* 32*/ { BARCODE_CODABLOCKF, DATA_MODE, -1, "\141\141\141\200", -1, 0, 2, 101, "67 64 40 41 41 41 63 39 6A 67 62 0B 65 40 28 33 34 6A", "CodeB a (3) / CodeA FNC4 PAD fully on next line" },
        /* 33*/ { BARCODE_CODABLOCKF, DATA_MODE, 10, "\200\240\237\340\337\341\377", -1, 0, 4, 112, "(40) 67 62 42 65 40 65 00 63 1E 6A 67 62 0B 65 5F 64 64 40 55 6A 67 64 0C 64 3F 64 41 63", "" },
        /* 34*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "\000a\037\177}12", 7, 0, 3, 101, "67 62 41 40 62 41 5F 3B 6A 67 64 0B 5F 5D 11 12 2D 6A 67 64 0C 63 64 40 05 26 6A", "" },
        /* 35*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "abcdéf", -1, 0, 3, 101, "67 64 41 41 42 43 44 5D 6A 67 64 0B 64 49 46 63 0A 6A 67 64 0C 63 64 4F 26 02 6A", "" },
        /* 36*/ { BARCODE_CODABLOCKF, UNICODE_MODE, -1, "a12é\000", 6, 0, 3, 101, "67 64 41 41 11 12 63 2C 6A 67 64 0B 64 49 62 40 2B 6A 67 64 0C 63 64 33 34 31 6A", "" },
        /* 37*/ { BARCODE_CODABLOCKF, UNICODE_MODE, 11, "1234\001", -1, 0, 2, 123, "67 63 00 0C 22 65 41 63 64 54 6A 67 64 0B 63 64 63 64 3F 20 24 6A", "" },
        /* 38*/ { BARCODE_HIBC_BLOCKF, UNICODE_MODE, -1, "A99912345/$$52001510X3", -1, 0, 6, 101, "(54) 67 64 44 0B 21 19 19 3A 6A 67 63 2B 5B 17 2D 64 24 6A 67 64 0C 0F 04 04 15 16 6A 67", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, \"%s\", %d, %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].option_2,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
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

static void test_encode(int index, int generate, int debug) {

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
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, 1, -1, "AIM", 0, 1, 68, 1, "Same as CODE128 (not supported by BWIPP)",
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
        /* 11*/ { BARCODE_HIBC_BLOCKF, 3, -1, "A123BJC5D6E71", 0, 3, 123, 0, "Verified manually against tec-it; differs from BWIPP (columns=6) which uses Code C for final 71 (same no. of codewords)",
                    "110100001001011110111010010110000110001001001010001100010011100110110011100101100101110010001011000100100001101100011101011"
                    "110100001001011110111011000100100101101110001000100011011011100100101100010001100111010010001101000111001001101100011101011"
                    "110100001001011110111010110011100111011011101001110011011010001000101110111101011100011011001110100100100110001100011101011"
                },
        /* 12*/ { BARCODE_HIBC_BLOCKF, -1, -1, "$$52001510X3G", 0, 4, 101, 1, "tec-it differs as adds unnecessary Code C at end of 1st line",
                    "11010000100101111011101001000011011000100100100100011001001000110011011100100101110011001100011101011"
                    "11010000100101110111101011000111011001001110110011011001101110100010111101110100001100101100011101011"
                    "11010000100101111011101011001110010011101100111000101101100101110011010001000100100011001100011101011"
                    "11010000100101111011101001101110010110001000101110111101101000111011000110110100011010001100011101011"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char bwipp_buf[8192];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    testStart("test_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
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
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// #181 Christian Hartlage OSS-Fuzz
static void test_fuzz(int index, int debug) {

    struct item {
        char *data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "\034\034I", 3, 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_fuzz");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODABLOCKF, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_options", test_options, 1, 0, 1 },
        { "test_reader_init", test_reader_init, 1, 1, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_fuzz", test_fuzz, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
