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
        /*  0*/ { "A", 49, 0, 8, 70 }, // ANSI/AIM BC6-2000 Table 1
        /*  1*/ { "A", 50, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { "0", 81, 0, 8, 70 }, // ANSI/AIM BC6-2000 Table 1
        /*  3*/ { "0", 82, ZINT_ERROR_TOO_LONG, -1, -1 },
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

        length = testUtilSetSymbol(symbol, BARCODE_CODE49, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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
    // NUL U+0000, S1 SP (39)
    // US U+001F (\037, 31), S1 5
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "é", -1, ZINT_ERROR_INVALID_DATA, 0, 0, "Error 431: Invalid character in input data", "ASCII only" },
        /*  1*/ { UNICODE_MODE, "EXAMPLE 2", -1, 0, 2, 70, "(16) 14 33 10 22 25 21 14 41 38 2 35 14 18 13 0 22", "2.3.7 Symbol Example" },
        /*  2*/ { UNICODE_MODE, "12345", -1, 0, 2, 70, "(16) 5 17 9 48 48 48 48 27 48 48 13 23 0 13 2 0", "2.3 Example 1: Numeric Encodation (Start 2, Numeric)" },
        /*  3*/ { UNICODE_MODE, "123456", -1, 0, 2, 70, "(16) 5 17 9 6 48 48 48 34 48 48 36 9 23 41 2 11", "2.3 Example 1: Numeric Encodation" },
        /*  4*/ { UNICODE_MODE, "12345678", -1, 0, 2, 70, "(16) 5 17 9 14 6 48 48 0 48 48 25 42 2 17 2 37", "2.3 Example 1: Numeric Encodation" },
        /*  5*/ { UNICODE_MODE, "123456789", -1, 0, 2, 70, "(16) 5 17 9 46 16 37 48 31 48 48 7 26 9 39 2 32", "2.3 Example 1: Numeric Encodation" },
        /*  6*/ { UNICODE_MODE, "1234567", -1, 0, 2, 70, "(16) 43 45 2 11 39 48 48 40 48 48 33 36 38 6 2 15", "2.3 Example 1: Numeric Encodation" },
        /*  7*/ { UNICODE_MODE, "\037", -1, 0, 2, 70, "(16) 5 48 48 48 48 48 48 48 48 48 4 33 13 15 4 18", "US (Start 4, Alphanumeric S1)" },
        /*  8*/ { UNICODE_MODE, "\000\037", 2, 0, 2, 70, "(16) 38 43 5 48 48 48 48 33 48 48 45 7 38 43 4 37", "NUL S1 US (Start 4, Alphanumeric S1)" },
        /*  9*/ { UNICODE_MODE, "a\000", 2, 0, 2, 70, "(16) 10 43 38 48 48 48 48 38 48 48 32 33 14 15 5 48", "a S1 NUL (Start 5, Alphanumeric S2)" },
        /* 10*/ { UNICODE_MODE, "ab", -1, 0, 2, 70, "(16) 10 44 11 48 48 48 48 12 48 48 27 39 42 0 5 13", "a S2 b (Start 5, Alphanumeric S2)" },
        /* 11*/ { UNICODE_MODE, "\000A\000a\000", 5, 0, 2, 70, "(16) 38 10 43 38 44 10 43 30 38 48 25 23 38 32 4 12", "NUL A S1 NUL S2 a S1 (C18 30) NUL (Start 4, Alphanumeric S1)" },
        /* 12*/ { UNICODE_MODE, "1234\037aA12345A", -1, 0, 3, 70, "(24) 1 2 3 4 43 5 44 4 10 10 48 5 17 9 48 0 10 48 19 2 13 32 7 33", "1 2 3 4 S1 US S2 (C18 4) a A NS 12345 NS (C28 0) A (Start 0, Alpha)" },
        /* 13*/ { GS1_MODE, "[90]12345[91]AB12345", -1, 0, 4, 70, "(32) 45 48 47 15 4 7 9 28 48 45 9 1 10 11 48 25 5 17 9 48 48 48 48 27 48 48 37 39 26 8 14", "FNC1 NS 9012345 (C18 28) NS FNC1 9 1 A B NS (C28 25) 12345 Pad (4) (C38 27) (Start 0, Alpha)" },
        /* 14*/ { GS1_MODE | GS1PARENS_MODE, "(90)12345(91)AB12345", -1, 0, 4, 70, "(32) 45 48 47 15 4 7 9 28 48 45 9 1 10 11 48 25 5 17 9 48 48 48 48 27 48 48 37 39 26 8 14", "FNC1 NS 9012345 (C18 28) NS FNC1 9 1 A B NS (C28 25) 12345 Pad (4) (C38 27) (Start 0, Alpha)" },
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

        length = testUtilSetSymbol(symbol, BARCODE_CODE49, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

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
        /*  0*/ { UNICODE_MODE, "MULTIPLE ROWS IN CODE 49", 0, 5, 70, "ANSI/AIM BC6-2000 Figure 1",
                    "1011111011001011101011100110000110111101011011111010111101000100001111"
                    "1010100001000010001001111000101110100110001111010010001011100011001111"
                    "1011001100000101101101110111000010110010110000111011101011110001101111"
                    "1010011001100100001111010010001100101011101111110011010001001111101111"
                    "1011001111001011101000000101001110111110111010001011010001101111101111"
                },
        /*  1*/ { UNICODE_MODE, "EXAMPLE 2", 0, 2, 70, "ANSI/AIM BC6-2000 Figure 3",
                    "1011000111011100101111001001000110110011110010100010001111000100101111"
                    "1011000100110010001100010110010000100001101001111010000001001011101111"
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

        length = testUtilSetSymbol(symbol, BARCODE_CODE49, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

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
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
