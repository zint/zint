/*
    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

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
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "A", 77, 0, 16, 70 }, // BS EN 12323:2005 4.1 (l)
        /*  1*/ { "A", 78, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { "0", 154, 0, 16, 70 }, // BS EN 12323:2005 4.1 (l)
        /*  3*/ { "0", 155, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { "0", 161, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[4096];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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

static void test_reader_init(int index, int generate, int debug) {

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
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, READER_INIT, "A", 0, 2, 70, "(10) 1 96 33 103 103 103 103 103 68 35", "ModeB FNC3 A Pad (5)" },
        /*  1*/ { UNICODE_MODE, READER_INIT, "12", 0, 2, 70, "(10) 5 96 12 103 103 103 103 103 99 41", "ModeC1SB FNC3 12 Pad (5)" },
        /*  2*/ { UNICODE_MODE, READER_INIT, "A1234", 0, 2, 70, "(10) 6 96 33 12 34 103 103 103 65 53", "ModeC2SB FNC3 A 12 34 Pad (3)" },
        /*  3*/ { GS1_MODE, READER_INIT, "[90]1", ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 422: Cannot use both GS1 mode and Reader Initialisation", "" },
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

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
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
        int input_mode;
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
    // b U+0062 (\142, 98), CodeB-only
    // APC U+009F (\237, 159), UTF-8 C29F, CodeA-only extended ASCII, not in ISO 8859-1
    // ß U+00DF (\337, 223), UTF-8 C39F, CodeA and CodeB extended ASCII
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "\037", -1, 0, 2, 70, "(10) 0 95 103 103 103 103 103 103 22 42", "ModeA US Pad (6)" },
        /*  1*/ { UNICODE_MODE, "A", -1, 0, 2, 70, "(10) 1 33 103 103 103 103 103 103 52 82", "ModeB A Pad (6)" },
        /*  2*/ { UNICODE_MODE, "12", -1, 0, 2, 70, "(10) 2 12 103 103 103 103 103 103 98 27", "ModeC 12 Pad (6)" },
        /*  3*/ { GS1_MODE, "[90]A", -1, 0, 2, 70, "(10) 3 25 16 33 103 103 103 103 83 20", "ModeBFNC1 9 0 A Pad (4)" },
        /*  4*/ { GS1_MODE, "[90]12", -1, 0, 2, 70, "(10) 4 90 12 103 103 103 103 103 79 62", "ModeCFNC1 90 12 Pad (5)" },
        /*  5*/ { GS1_MODE, "[90]12[20]12", -1, 0, 2, 70, "(10) 4 90 12 102 20 12 103 103 9 72", "ModeCFNC1 90 12 FNC1 20 12 Pad (2)" },
        /*  6*/ { GS1_MODE, "[90]123[20]12", -1, 0, 2, 70, "(15) 4 90 12 100 19 99 102 20 12 103 103 103 103 88 22", "ModeCFNC1 90 CodeB 3 CodeC FNC1 20 12 Pad (4)" },
        /*  7*/ { GS1_MODE, "[90]123[91]1A3[20]12", -1, 0, 4, 70, "(20) 18 90 12 100 19 102 25 99 11 100 33 19 99 102 20 12 103 103 0 3", "ModeCFNC1 90 12 CodeB 3 FNC1 9 CodeC 11 CodeB A 3 CodeC FNC1 20 12 Pad (2)" },
        /*  8*/ { GS1_MODE, "[90]123A[91]123", -1, 0, 3, 70, "(15) 11 90 12 100 19 33 102 25 99 11 23 103 103 81 56", "ModeCFNC1 90 12 CodeB 3 A FNC1 9 CodeC 11 23 Pad (2)" },
        /*  9*/ { GS1_MODE | GS1PARENS_MODE, "(90)12", -1, 0, 2, 70, "(10) 4 90 12 103 103 103 103 103 79 62", "ModeCFNC1 90 12 Pad (5)" },
        /* 10*/ { UNICODE_MODE, "a0123456789", -1, 0, 2, 70, "(10) 5 65 1 23 45 67 89 103 27 86", "ModeC1SB a 01 23 45 67 89 Pad" },
        /* 11*/ { UNICODE_MODE, "ab0123456789", -1, 0, 2, 70, "(10) 6 65 66 1 23 45 67 89 19 42", "ModeC2SB a b 01 23 45 67 89" },
        /* 12*/ { UNICODE_MODE, "1234\037a", -1, 0, 2, 70, "(10) 2 12 34 101 95 98 65 103 67 53", "ModeC 12 34 CodeA US 1SB a Pad" },
        /* 13*/ { UNICODE_MODE, "\000\037ß", 4, 0, 2, 70, "(10) 0 64 95 101 63 103 103 103 75 11", "ModeA NUL US FNC4 ß Pad (3)" },
        /* 14*/ { UNICODE_MODE, "\000\037é", 4, 0, 2, 70, "(10) 0 64 95 101 98 73 103 103 75 6", "ModeA NUL US FNC4 1SB é Pad (2)" },
        /* 15*/ { UNICODE_MODE, "\000\037éa", 5, 0, 2, 70, "(10) 0 64 95 100 100 73 65 103 99 69", "ModeA NUL US CodeB FNC4 é a Pad" },
        /* 16*/ { UNICODE_MODE, "abß", -1, 0, 2, 70, "(10) 1 65 66 100 63 103 103 103 66 56", "ModeB a b FNC4 ß Pad (3)" },
        /* 17*/ { DATA_MODE, "\141\142\237", -1, 0, 2, 70, "(10) 1 65 66 100 98 95 103 103 6 71", "ModeB a b FNC4 1SA APC Pad (2)" },
        /* 18*/ { DATA_MODE, "\141\142\237\037", -1, 0, 2, 70, "(10) 1 65 66 101 101 95 95 103 72 93", "ModeB a b CodeA FNC4 APC US Pad" },
        /* 19*/ { UNICODE_MODE, "ééé", -1, 0, 2, 70, "(10) 1 100 73 100 73 100 73 103 105 106", "ModeB FNC4 é FNC4 é FNC4 é Pad" },
        /* 20*/ { UNICODE_MODE, "aééééb", -1, 0, 3, 70, "(15) 8 65 100 73 100 73 100 73 100 73 66 103 103 39 83", "ModeB a FNC4 é (4) b Pad (2)" },
        /* 21*/ { UNICODE_MODE, "aéééééb", -1, 0, 3, 70, "(15) 8 65 100 73 100 73 100 73 100 73 100 73 66 74 106", "ModeB a FNC4 é (5) b" },
        /* 22*/ { UNICODE_MODE, "aééééébcdeé", -1, 0, 4, 70, "(20) 15 65 100 73 100 73 100 73 100 73 100 73 66 67 68 69 100 73 14 69", "ModeB a FNC4 é (5) b c d e FNC4 é" },
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

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %d, %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
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
        int input_mode;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "ab0123456789", 0, 2, 70, "BS EN 12323:2005 Figure 3",
                    "1110010101100110111011010011110110111100100110010011000100100010001101"
                    "1100110101000100111011110100110010010000100110100011010010001110011001"
                },
        /*  1*/ { UNICODE_MODE, "www.wikipedia.de", 0, 4, 70, "https://commons.wikimedia.org/wiki/File:Code_16K_wikipedia.png",
                    "1110010101000110011000011010110000110101100001101011011001100010001101"
                    "1100110100001101011011110010110011110110101111001011010110000110011001"
                    "1101100101001101111011110110010111100101101101001111011001100010010011"
                    "1000010101111011001010011011110010111101101100001011010001001110111101"
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

        length = testUtilSetSymbol(symbol, BARCODE_CODE16K, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
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

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
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
        { "test_reader_init", test_reader_init, 1, 1, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
