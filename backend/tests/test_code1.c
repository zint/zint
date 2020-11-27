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

#include "testcommon.h"

static void test_large(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int option_2;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -1, "1", 2955, 0, 148, 134 },
        /*  1*/ { -1, "1", 2956, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { -1, "A", 2217, 0, 148, 134 },
        /*  3*/ { -1, "A", 2218, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { -1, "\001", 1480, 0, 148, 134 },
        /*  5*/ { -1, "\001", 1481, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { 1, "1", 12, 0, 16, 18 },
        /*  7*/ { 1, "1", 13, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { 2, "1", 33, 0, 22, 22 },
        /*  9*/ { 2, "1", 34, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { 3, "1", 75, 0, 28, 32 },
        /* 11*/ { 3, "1", 76, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 12*/ { 4, "1", 177, 0, 40, 42 },
        /* 13*/ { 4, "1", 178, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { 5, "1", 360, 0, 52, 54 },
        /* 15*/ { 5, "1", 361, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 16*/ { 6, "1", 732, 0, 70, 76 },
        /* 17*/ { 6, "1", 733, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 18*/ { 7, "1", 1452, 0, 104, 98 },
        /* 19*/ { 7, "1", 1453, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 20*/ { 8, "1", 2955, 0, 148, 134 },
        /* 21*/ { 8, "1", 2956, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 22*/ { 9, "1", 18, 0, 8, 31 },
        /* 23*/ { 9, "1", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 24*/ { 10, "1", 69, 0, 16, 49 },
        /* 25*/ { 10, "1", 70, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 26*/ { 10, "A", 54, 0, 16, 49 },
        /* 27*/ { 10, "A", 55, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 28*/ { 10, "\001", 38, 0, 16, 49 },
        /* 29*/ { 10, "\001", 39, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[4096];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        int length = testUtilSetSymbol(symbol, BARCODE_CODEONE, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        int option_2;
        char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -1, -1, "123456789012ABCDEFGHI", -1, 0, 22, 22 },
        /*  1*/ { -1, -1, "123456789012ABCDEFGHIJ", -1, 0, 22, 22 },
        /*  2*/ { -1, -1, "1", -1, 0, 16, 18 },
        /*  3*/ { -1, 0, "1", -1, 0, 16, 18 },
        /*  4*/ { -1, 1, "1", -1, 0, 16, 18 },
        /*  5*/ { -1, 11, "1", -1, ZINT_ERROR_INVALID_OPTION, -1, -1 },
        /*  6*/ { -1, 9, "123456789012345678", -1, 0, 8, 31 },
        /*  7*/ { -1, 9, "12345678901234567A", -1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  8*/ { -1, 10, "123456789012345678901234567890123456789012345678901234567890123456789", -1, 0, 16, 49 },
        /*  9*/ { -1, 10, "1234567890123456789012345678901234567890123456789012345678901234567890123456", -1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { -1, 10, "1234567890123456789012345678901234567890123456789012345678901234567890123456789", -1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 11*/ { -1, 10, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0, 16, 49 },
        /* 12*/ { -1, 10, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 13*/ { -1, 10, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 38, 0, 16, 49 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_CODEONE, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    testStart("");

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    int ret;
    struct item {
        int option_2;
        char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { -1, "123456789012", -1, 0, 16, 18, 1, "",
                    "100011101010111101"
                    "111010010010100000"
                    "110110100010001000"
                    "110010111000010001"
                    "100010100011010100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010100100101"
                    "011001001110101101"
                    "011010010010101111"
                    "010111110100100111"
                    "100010001101111100"
                },
        /*  1*/ { -1, "Code One", -1, 0, 16, 18, 1, "BWIPP example",
                    "010011011101100110"
                    "010010000001010110"
                    "001010010101100110"
                    "000110000011110110"
                    "100010100000111001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011110101101"
                    "101101000111101011"
                    "010100001110101100"
                    "100001100111100100"
                    "100000111000111000"
                },
        /*  2*/ { 3, "1234567890ABCDEF", -1, 0, 28, 32, 0, "https://fr.wikipedia.org/wiki/Liste_des_symbologies, same; BWIPP **NOT SAME**, has unlatch to ASCII at end, no doc so don't know if necessary",
                    "10001110101011110111011110110101"
                    "11101001001010000011000110101001"
                    "11101001100010100010001000101000"
                    "10011011010100000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "00001000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000000000"
                    "01111111111111111111111111111110"
                    "01000000000000000000000000000010"
                    "01111111111111111111111111111110"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010000101011110001101100110"
                    "00011000010010101010111011100100"
                    "11101011011100101001000110101100"
                    "01111000000000010001000111101111"
                    "00001010100010111100100100101100"
                    "10001000101110100001010011100110"
                    "00001011001001010100010001101111"
                    "00101101111001111011011001111010"
                },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];
    char bwipp_buf[8192];
    char bwipp_msg[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_CODEONE, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %d, \"%s\", %d, %s, %d, %d, %d, \"%s\",\n",
                    i, data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesDump(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < 5) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
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

// #181 Nico Gunkel OSS-Fuzz
static void test_fuzz(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        char *data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { "3333P33B\035333V3333333333333\0363", -1, 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_CODEONE;
        symbol->debug |= debug;

        int length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_input", test_input, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_fuzz", test_fuzz, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
