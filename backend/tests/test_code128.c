/*
    libzint - the open source barcode library
    Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>

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

/* Note BARCODE_EAN128, BARCODE_EAN14, BARCODE_NVE18 also tested in test_gs1.c */

#include "testcommon.h"

static void test_large(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *pattern;
        int length;
        int ret;
        int expected_width;
    };
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "A", 60, 0, 695 },
        /*  1*/ { BARCODE_CODE128, "A", 61, ZINT_ERROR_TOO_LONG, -1 },
        /*  2*/ { BARCODE_CODE128, "\351A", 40, 0, 695 },
        /*  3*/ { BARCODE_CODE128, "\351A", 41, ZINT_ERROR_TOO_LONG, -1 }, // 41 chars (+ 20 shifts)
        /*  4*/ { BARCODE_CODE128, "0", 120, 0, 695 },
        /*  5*/ { BARCODE_CODE128, "0", 121, ZINT_ERROR_TOO_LONG, -1 },
        /*  6*/ { BARCODE_CODE128B, "A", 60, 0, 695 },
        /*  7*/ { BARCODE_CODE128B, "A", 61, ZINT_ERROR_TOO_LONG, -1 },
        /*  8*/ { BARCODE_CODE128B, "0", 60, 0, 695 },
        /*  9*/ { BARCODE_CODE128B, "0", 61, ZINT_ERROR_TOO_LONG, -1 },
        /* 10*/ { BARCODE_EAN128, "[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890123456789012345678901234567890[92]1234567890", -1, 0, 706 }, // 116 nos + 3 FNC1s
        /* 11*/ { BARCODE_EAN128, "[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890123456789012345678901234567890[92]1234[93]1234", -1, ZINT_ERROR_TOO_LONG, -1 }, // 116 nos + 4 FNC1s
        /* 12*/ { BARCODE_EAN14, "1234567890123", -1, 0, 134 },
        /* 13*/ { BARCODE_EAN14, "12345678901234", -1, ZINT_ERROR_TOO_LONG, -1 },
        /* 14*/ { BARCODE_NVE18, "12345678901234567", -1, 0, 156 },
        /* 15*/ { BARCODE_NVE18, "123456789012345678", -1, ZINT_ERROR_TOO_LONG, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[4096];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].length != -1) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
            assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));
        } else {
            strcpy(data_buf, data[i].pattern);
        }

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_hrt(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        unsigned char *data;
        int length;

        unsigned char *expected;
    };
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, "1234567890", -1, "1234567890" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, "\000ABC\000DEF\000", 9, " ABC DEF " },
        /*  2*/ { BARCODE_CODE128B, UNICODE_MODE, "12345\00067890", 11, "12345 67890" },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, "12345\01167890\037\177", -1, "12345\01167890\037\177" },
        /*  4*/ { BARCODE_CODE128, UNICODE_MODE, "abcdé", -1, "abcdé" },
        /*  5*/ { BARCODE_CODE128, DATA_MODE, "abcd\351", -1, "abcd\351" },
        /*  6*/ { BARCODE_CODE128B, UNICODE_MODE, "abcdé", -1, "abcdé" },
        /*  7*/ { BARCODE_CODE128B, DATA_MODE, "abcd\351", -1, "abcd\351" },
        // BARCODE_EAN128, BARCODE_EAN14, BARCODE_NVE18 hrt tested in test_gs1.c
    };
    int data_size = ARRAY_SIZE(data);

    char *text;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_zero(strcmp(symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        int output_options;
        unsigned char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, 16, "A", 0, 1, 57, "(5) 104 96 33 60 106", "StartA FNC3 A" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, 16, "12", 0, 1, 68, "(6) 104 96 99 12 22 106", "StartB FNC3 CodeC 12" },
        /*  2*/ { BARCODE_CODE128B, UNICODE_MODE, 16, "\0371234", 0, 1, 101, "(9) 103 96 95 17 18 19 20 6 106", "StartA FNC3 US 1 2 3 4" },
        /*  3*/ { BARCODE_EAN128, GS1_MODE, 16, "[90]12", 0, 1, 68, "", "Reader Initialise not supported by GS1 barcodes (use CODE128)" },
        /*  4*/ { BARCODE_EAN14, GS1_MODE, 16, "12", 0, 1, 134, "", "Reader Initialise not supported by GS1 barcodes (use CODE128)" },
        /*  5*/ { BARCODE_NVE18, GS1_MODE, 16, "12", 0, 1, 156, "", "Reader Initialise not supported by GS1 barcodes (use CODE128)" },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].output_options,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < 5) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
                assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char *data;
        int length;
        int ret;
        int expected_width;
        char *expected;
        char *comment;
    };
    // NUL U+0000, CodeA-only
    // US U+001F (\037, 31), CodeA-only
    // a U+0061 (\141, 97), CodeB-only
    // b U+0062 (\142, 98), CodeB-only
    // PAD U+0080 (\200, 128), UTF-8 C280 (\302\200), CodeA-only extended ASCII, not in ISO 8859-1
    // APC U+009F (\237, 159), UTF-8 C29F, CodeA-only extended ASCII, not in ISO 8859-1
    // ß U+00DF (\337, 223), UTF-8 C39F, CodeA and CodeB extended ASCII
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "\302\200", -1, ZINT_ERROR_INVALID_DATA, 0, "Error 204: Invalid characters in input data", "PAD not in ISO 8859-1" },
        /*  1*/ { DATA_MODE, "\200", -1, 0, 57, "(5) 103 101 64 23 106", "PAD ok using binary" },
        /*  2*/ { UNICODE_MODE, "AIM1234", -1, 0, 101, "(9) 104 33 41 45 99 12 34 87 106", "Example from Annex A.1, check char value 87" },
        /*  3*/ { GS1_MODE, "[90]12", -1, ZINT_ERROR_INVALID_OPTION, 0, "Error 220: Selected symbology does not support GS1 mode", "" },
        /*  4*/ { UNICODE_MODE, "1", -1, 0, 46, "(4) 104 17 18 106", "StartB 1" },
        /*  5*/ { UNICODE_MODE, "12", -1, 0, 46, "(4) 105 12 14 106", "StartC 12" },
        /*  6*/ { UNICODE_MODE, "123", -1, 0, 68, "(6) 104 17 18 19 8 106", "StartB 1 2 3" },
        /*  7*/ { UNICODE_MODE, "1234", -1, 0, 57, "(5) 105 12 34 82 106", "StartC 12 34" },
        /*  8*/ { UNICODE_MODE, "12345", -1, 0, 79, "(7) 105 12 34 100 21 54 106", "StartC 12 34 CodeB 5" },
        /*  9*/ { UNICODE_MODE, "\037", -1, 0, 46, "(4) 103 95 95 106", "StartA US" },
        /* 10*/ { UNICODE_MODE, "1\037", -1, 0, 57, "(5) 103 17 95 1 106", "StartA 1 US" },
        /* 11*/ { UNICODE_MODE, "12\037", -1, 0, 68, "(6) 103 17 18 95 29 106", "StartA 1 2 US" },
        /* 12*/ { UNICODE_MODE, "a\037a", -1, 0, 79, "(7) 104 65 98 95 65 86 106", "StartB a Shift US a" },
        /* 13*/ { UNICODE_MODE, "1234\037a", -1, 0, 101, "(9) 105 12 34 101 95 98 65 100 106", "StartC 12 34 CodeA US Shift a" },
        /* 14*/ { UNICODE_MODE, "\037AAa\037", -1, 0, 101, "(9) 103 95 33 33 98 65 95 2 106", "StartA US A A Shift a US" },
        /* 15*/ { UNICODE_MODE, "\037AAaa\037", -1, 0, 123, "(11) 103 95 33 33 100 65 65 98 95 40 106", "StartA US A A CodeB a a Shift US" },
        /* 16*/ { UNICODE_MODE, "AAAa12345aAA", -1, 0, 167, "(15) 104 33 33 33 65 17 99 23 45 100 65 33 33 54 106", "StartB A (3) a 1 CodeC 23 45 CodeB a A A" },
        /* 17*/ { UNICODE_MODE, "a\037Aa\037\037a\037aa\037a", -1, 0, 222, "(20) 104 65 98 95 33 65 101 95 95 98 65 95 100 65 65 98 95 65 96 106", "StartB a Shift US A a CodeA US US Shift a US CodeB a a Shift US a" },
        /* 18*/ { UNICODE_MODE, "\000\037ß", 4, 0, 79, "(7) 103 64 95 101 63 88 106", "StartA NUL US FNC4 ß" },
        /* 19*/ { UNICODE_MODE, "\000\037é", 4, 0, 90, "(8) 103 64 95 101 98 73 78 106", "StartA NUL US FNC4 Shift é" },
        /* 20*/ { UNICODE_MODE, "\000\037éa", 5, 0, 101, "(9) 103 64 95 100 100 73 65 61 106", "StartA NUL US LatchB FNC4 é a" },
        /* 21*/ { UNICODE_MODE, "abß", -1, 0, 79, "(7) 104 65 66 100 63 29 106", "StartB a b FNC4 ß" },
        /* 22*/ { DATA_MODE, "\141\142\237", -1, 0, 90, "(8) 104 65 66 100 98 95 26 106", "StartB a b FNC4 Shift APC" },
        /* 23*/ { DATA_MODE, "\141\142\237\037", -1, 0, 101, "(9) 104 65 66 101 101 95 95 96 106", "StartB a b LatchA FNC4 APC US" },
        /* 24*/ { UNICODE_MODE, "ééé", -1, 0, 90, "(8) 104 100 100 73 73 73 44 106", "StartB LatchFNC4 é é é" },
        /* 25*/ { UNICODE_MODE, "aééééb", -1, 0, 145, "(13) 104 65 100 73 100 73 100 73 100 73 66 49 106", "StartB a FNC4 é (4) b" },
        /* 26*/ { UNICODE_MODE, "aéééééb", -1, 0, 145, "(13) 104 65 100 100 73 73 73 73 73 100 66 93 106", "StartB a Latch é (5) Shift b" },
        /* 27*/ { UNICODE_MODE, "aééééébc", -1, 0, 167, "(15) 104 65 100 100 73 73 73 73 73 100 66 100 67 40 106", "StartB a Latch é (5) Shift b Shift c" },
        /* 28*/ { UNICODE_MODE, "aééééébcd", -1, 0, 178, "(16) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 66 106", "StartB a Latch é (5) Unlatch b c d" },
        /* 29*/ { UNICODE_MODE, "aééééébcde", -1, 0, 189, "(17) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 69 2 106", "StartB a Latch é (5) Unlatch b c d e" },
        /* 30*/ { UNICODE_MODE, "aééééébcdeé", -1, 0, 211, "(19) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 69 100 73 95 106", "StartB a Latch é (5) Unlatch b c d e FNC4 é" },
        /* 31*/ { UNICODE_MODE, "aééééébcdeéé", -1, 0, 233, "(21) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 69 100 73 100 73 19 106", "StartB a Latch é (5) Unlatch b c d e FNC4 é (2)" },
        /* 32*/ { UNICODE_MODE, "aééééébcdeééé", -1, 0, 244, "(22) 104 65 100 100 73 73 73 73 73 100 66 100 67 100 68 100 69 73 73 73 83 106", "StartB a Latch é (5) Shift b Shift c Shift d Shift e é (3)" },
        /* 33*/ { UNICODE_MODE, "aééééébcdefééé", -1, 0, 255, "(23) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 69 70 100 100 73 73 73 67 106", "StartB a Latch é (5) Unlatch b c d e f Latch é (3)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = testUtilSetSymbol(symbol, BARCODE_CODE128, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %d, %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < 5) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
                assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        unsigned char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    // BARCODE_EAN128 examples verified manually against GS1 General Specifications 20.0
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, "AIM", 0, 1, 68, "ISO/IEC 15417:2007 Figure 1",
                    "11010010000101000110001100010001010111011000101110110001100011101011"
                },
        /*  1*/ { BARCODE_CODE128B, UNICODE_MODE, "AIM", 0, 1, 68, "128B same",
                    "11010010000101000110001100010001010111011000101110110001100011101011"
                },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, "1234567890", 0, 1, 90, "",
                    "110100111001011001110010001011000111000101101100001010011011110110100111100101100011101011"
                },
        /*  3*/ { BARCODE_CODE128B, UNICODE_MODE, "1234567890", 0, 1, 145, "",
                    "1101001000010011100110110011100101100101110011001001110110111001001100111010011101101110111010011001110010110010011101100101000110001100011101011"
                },
        /*  4*/ { BARCODE_EAN128, GS1_MODE, "[8018]950110153123456781", 0, 1, 167, "GS1 General Specifications Figure 2.5.2-1",
                    "11010011100111101011101010011110011001110010101111010001100110110011001000100101110011001101100011011101101110101110110001000010110010010111100101111001001100011101011"
                },
        /*  5*/ { BARCODE_EAN128, GS1_MODE, "[415]5412345678908[3911]710125", 0, 1, 189, "GS1 General Specifications Figure 2.6.6-1 top",
                    "110100111001111010111011000100010111010001101100010001011101101110101110110001000010110011011011110100011001001101000100011000100100100110100001100110110011100101100100001001101100011101011"
                },
        /*  6*/ { BARCODE_EAN128, GS1_MODE, "[12]010425[8020]ABC123", 0, 1, 189, "GS1 General Specifications Figure 2.6.6-1 bottom",
                    "110100111001111010111010110011100110011011001001000110011100101100101001111001100100111010111101110101000110001000101100010001000110100111001101100111001011001011100110010111001100011101011"
                },
        /*  7*/ { BARCODE_EAN128, GS1_MODE, "[253]950110153005812345678901", 0, 1, 211, "GS1 General Specifications Figure 2.6.9-1",
                    "1101001110011110101110111001011001101000100011000101110110001001001100110110011011101110110110011001110110001010110011100100010110001110001011011000010100110111101101011110111010011100110101110110001100011101011"
                },
        /*  8*/ { BARCODE_EAN128, GS1_MODE, "[253]950110153006567890543210987", 0, 1, 211, "GS1 General Specifications Figure 2.6.9-2",
                    "1101001110011110101110111001011001101000100011000101110110001001001100110110011011101110110110011001001011000010000101100110110111101000100110010110001110110111001001100100100011110010100101110011001100011101011"
                },
        /*  9*/ { BARCODE_EAN128, GS1_MODE, "[253]95011015300657654321", 0, 1, 189, "GS1 General Specifications Figure 2.6.9-3",
                    "110100111001111010111011100101100110100010001100010111011000100100110011011001101110111011011001100100101100001100101000011101011000110001101101011110111010011100110111001001101100011101011"
                },
        /* 10*/ { BARCODE_EAN128, GS1_MODE, "[253]9501101530065123456", 0, 1, 167, "GS1 General Specifications Figure 2.6.9-4",
                    "11010011100111101011101110010110011010001000110001011101100010010011001101100110111011101101100110010010110000101100111001000101100011100010110100011110101100011101011"
                },
        /* 11*/ { BARCODE_EAN128, GS1_MODE, "[01]10857674002017[10]1152KMB", 0, 1, 211, "GS1 General Specifications Figure 4.15.1-1",
                    "1101001110011110101110110011011001100100010010011110010110010100001000011001011011001100110010011101001110011011001000100110001001001101110001010111101110101100011101011101100010001011000100111001101100011101011"
                },
        /* 12*/ { BARCODE_EAN128, GS1_MODE, "[01]09501101530003", 0, 1, 134, "GS1 General Specifications Figure 5.1-3",
                    "11010011100111101011101100110110011001001000110001011101100010010011001101100110111011101101100110010010011000100110100001100011101011"
                },
        /* 13*/ { BARCODE_EAN128, GS1_MODE, "[00]395123451234567895", 0, 1, 156, "GS1 General Specifications Figure 5.4.2-1",
                    "110100111001111010111011011001100110100010001101110100011101101110101110110001011001110010001011000111000101101100001010010111101000101111000101100011101011"
                },
        /* 14*/ { BARCODE_EAN128, GS1_MODE, "[00]395011010013000129", 0, 1, 156, "GS1 General Specifications Figure 6.6.5-6",
                    "110100111001111010111011011001100110100010001100010111011000100100110011011001101100110010011011100110110011001100110110011100110010111101101101100011101011"
                },
        /* 15*/ { BARCODE_EAN128, GS1_MODE, "[401]931234518430GR", 0, 1, 167, "GS1 General Specifications Figure 6.6.5-7 top",
                    "11010011100111101011101100010100011001011100110110001101110110111010111011000110011100101011000111010111101110100111011001101000100011000101110100110111001100011101011"
                },
        /* 16*/ { BARCODE_EAN128, GS1_MODE, "[00]093123450000000012", 0, 1, 156, "GS1 General Specifications Figure 6.6.5-7 bottom",
                    "110100111001111010111011011001100110010010001101100011011101101110101110110001101100110011011001100110110011001101100110010110011100110111010001100011101011"
                },
        /* 17*/ { BARCODE_EAN128, GS1_MODE, "[01]95012345678903", 0, 1, 134, "GS1 General Specifications Figure 7.8.5.1-1 1st",
                    "11010011100111101011101100110110010111101000110011011001110110111010111011000100001011001101101111010010011000110110001101100011101011"
                },
        /* 18*/ { BARCODE_EAN128, GS1_MODE, "[3102]000400", 0, 1, 101, "GS1 General Specifications Figure 7.8.5.1-1 2nd",
                    "11010011100111101011101101100011011001100110110110011001001000110011011001100110110111101100011101011"
                },
        /* 19*/ { BARCODE_EAN128, GS1_MODE, "[01]95012345678903[3102]000400", 0, 1, 189, "GS1 General Specifications Figure 7.8.5.1-2",
                    "110100111001111010111011001101100101111010001100110110011101101110101110110001000010110011011011110100100110001101100011011001100110110110011001001000110011011001100100100110001100011101011"
                },
        /* 20*/ { BARCODE_EAN128, GS1_MODE, "[8005]000365", 0, 1, 101, "GS1 General Specifications Figure 7.8.5.2-1 1st",
                    "11010011100111101011101010011110010001001100110110011001001001100010010110000100100001101100011101011"
                },
        /* 21*/ { BARCODE_EAN128, GS1_MODE, "[10]123456", 0, 1, 90, "GS1 General Specifications Figure 7.8.5.2-1 2nd",
                    "110100111001111010111011001000100101100111001000101100011100010110110010000101100011101011"
                },
        /* 22*/ { BARCODE_EAN128, GS1_MODE, "[8005]000365[10]123456", 0, 1, 156, "GS1 General Specifications Figure 7.8.5.2-2",
                    "110100111001111010111010100111100100010011001101100110010010011000100101100001111010111011001000100101100111001000101100011100010110101100001001100011101011"
                },
        /* 23*/ { BARCODE_EAN14, GS1_MODE, "4070071967072", 0, 1, 134, "Verified manually against bwipp and tec-it",
                    "11010011100111101011101100110110011000101000101100001001001100010011001011100100001011001001100010011001001110110111001001100011101011"
                },
        /* 24*/ { BARCODE_NVE18, GS1_MODE, "40700000071967072", 0, 1, 156, "Verified manually against bwipp (sscc18) and tec-it",
                    "110100111001111010111011011001100110001010001011000010011011001100110110011001001100010011001011100100001011001001100010011001001110110111011101100011101011"
                },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesDump(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < 5) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                if (ret == 0) {
                    int width, row;
                    ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                    assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_hrt", test_hrt, 1, 0, 1 },
        { "test_reader_init", test_reader_init, 1, 1, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
