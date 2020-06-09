/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2020 Robin Stuart <rstuart114@gmail.com>

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
        int symbology;
        unsigned char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, "1", 3116, 0, 144, 144 },
        /*  1*/ { BARCODE_DATAMATRIX, "1", 3117, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_DATAMATRIX, "A", 2335, 0, 144, 144 },
        /*  3*/ { BARCODE_DATAMATRIX, "A", 2336, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_DATAMATRIX, "\200", 1555, 0, 144, 144 },
        /*  5*/ { BARCODE_DATAMATRIX, "\200", 1556, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_HIBC_DM, "1", 110, 0, 32, 32 },
        /*  7*/ { BARCODE_HIBC_DM, "1", 111, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[3118];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Note need ZINT_SANITIZE set for these
static void test_buffer(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int eci;
        int input_mode;
        int output_options;
        int ret;
        char *comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "1", 16383, UNICODE_MODE, READER_INIT, 0, "" },
        /*  1*/ { "000106j 05 Galeria A NaÃ§Ã£o0000000000", 3, UNICODE_MODE, 0, 0, "From Okapi, consecutive use of upper shift; ticket #176" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DATAMATRIX;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;
        symbol->output_options = data[i].output_options;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

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
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, "1234abcd", 0, 14, 14, "",
                    "10101010101010"
                    "11001010001111"
                    "11000101100100"
                    "11001001100001"
                    "11011001110000"
                    "10100101011001"
                    "10101110011000"
                    "10011101100101"
                    "10100001001000"
                    "10101000001111"
                    "11101100000010"
                    "11010010100101"
                    "10011111000100"
                    "11111111111111"
                },
        /*  1*/ { BARCODE_DATAMATRIX, -1, "A1B2C3D4E5F6G7H8I9J0K1L2", 0, 18, 18, "ISO 16022:2006 Figure 1",
                    "101010101010101010"
                    "101000101010001111"
                    "101100000111000010"
                    "100000101110100111"
                    "100100000100011100"
                    "101011111010101111"
                    "100110011111110110"
                    "110011101111111101"
                    "111111111101111100"
                    "101110110100101101"
                    "100010110101001110"
                    "101101111001100001"
                    "110001101010011110"
                    "110110100000100011"
                    "101101001101011010"
                    "100010011001011011"
                    "100011000000100100"
                    "111111111111111111"
                },
        /*  2*/ { BARCODE_DATAMATRIX, -1, "123456", 0, 10, 10, "ISO 16022:2006 Figure O.2",
                    "1010101010"
                    "1100101101"
                    "1100000100"
                    "1100011101"
                    "1100001000"
                    "1000001111"
                    "1110110000"
                    "1111011001"
                    "1001110100"
                    "1111111111"
                },
        /*  3*/ { BARCODE_DATAMATRIX, -1, "30Q324343430794<OQQ", 0, 16, 16, "ISO 16022:2006 Figure R.1",
                    "1010101010101010"
                    "1010101010000001"
                    "1010101011101100"
                    "1010101010110011"
                    "1010101010001100"
                    "1010101010001101"
                    "1010101010000100"
                    "1010101001101001"
                    "1010101010000110"
                    "1000001001011001"
                    "1111111110000100"
                    "1101100110010101"
                    "1111111001100100"
                    "1110010111100101"
                    "1110010010100010"
                    "1111111111111111"
                },
        /*  4*/ { BARCODE_DATAMATRIX, GS1_MODE, "[01]09501101530003[17]150119[10]AB-123", 0, 20, 20, "GS1 General Specfications 20.0 Figure 2.6.14-3",
                    "10101010101010101010"
                    "11001111010100000111"
                    "10001010001001010100"
                    "10110011010100010001"
                    "11101010000001101010"
                    "10000100111011010111"
                    "10011010101101010110"
                    "11010001001110101001"
                    "11101000110100101100"
                    "11001111010111001101"
                    "10001010000001100000"
                    "11010000100010111011"
                    "10110010011000001000"
                    "10011010000011010011"
                    "11111010101110100110"
                    "11010010111011100001"
                    "11010100101100111110"
                    "11000001110010010101"
                    "10011011100101011010"
                    "11111111111111111111"
                },
        /*  5*/ { BARCODE_DATAMATRIX, GS1_MODE, "[01]04012345678901[21]ABCDEFG123456789", 0, 20, 20, "GS1 General Specfications 20.0 Figure 2.6.14-4",
                    "10101010101010101010"
                    "11011000001101000111"
                    "10001001100001110100"
                    "10110110110000010001"
                    "11100010000111110110"
                    "10101010110011101101"
                    "11111000100000100010"
                    "10010010001011110001"
                    "10101010110011010110"
                    "11011110011010001111"
                    "10001010011101010010"
                    "10111111011110110011"
                    "11110100101101011000"
                    "11010111011100100111"
                    "10000000011001100000"
                    "11101111110100001011"
                    "11010001001000101010"
                    "11010011101000100101"
                    "10001000100001111010"
                    "11111111111111111111"
                },
        /*  6*/ { BARCODE_DATAMATRIX, GS1_MODE, "[01]04012345678901[17]170101[10]ABC123", 0, 20, 20, "GS1 General Specfications 20.0 Figure 4.15-1",
                    "10101010101010101010"
                    "11011000010100000111"
                    "10001001100001010100"
                    "10110111001100000001"
                    "11100010000101101110"
                    "10101100110001010101"
                    "11111010101000100110"
                    "10010011001000100011"
                    "10101000110010111010"
                    "11001111001010101111"
                    "10001010000111000010"
                    "10110000010101000011"
                    "11110010000001011000"
                    "11011000000110101111"
                    "11111010111011110110"
                    "11001001001110101111"
                    "11011010000001110010"
                    "11010111010111101101"
                    "10001000000101111010"
                    "11111111111111111111"
                },
        /*  7*/ { BARCODE_DATAMATRIX, GS1_MODE, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", 0, 32, 32, "GS1 General Specfications 20.0 Figure 4.15.1-2 **NOT SMAE** TODO: investigate",
                    "10101010101010101010101010101010"
                    "11001111010000111101100000101001"
                    "10001010011111001011011001000010"
                    "10111011001001111101111101000101"
                    "11100101000010001000011011011110"
                    "10000101001101111010111000100101"
                    "10010001110100101000000010011110"
                    "10010110101101011000101100101111"
                    "11101010000010001100011100110100"
                    "11011100110110111101010000001001"
                    "10001010000101001100000111011010"
                    "10110011010101111000011001111001"
                    "11101100100110101110010100010110"
                    "11100100000001111000101100010101"
                    "10100010111011101100011101011010"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11111011100111111011101011111111"
                    "11010100001100001001011001001010"
                    "10100011010010011101010101001101"
                    "11101011010000101011001110001110"
                    "10010000011001111110010000110111"
                    "11000101110110101010001111101000"
                    "10000010110111111010001011110011"
                    "10110011100010101010001011101000"
                    "10111010000000111100111100110001"
                    "10010000110001101101001110110110"
                    "11101001001110011001101111101001"
                    "10000000111100001100000010010000"
                    "11001101001100011110110001010101"
                    "10110100011001101010010100011000"
                    "11111111111111111111111111111111"
                },
        /*  8*/ { BARCODE_HIBC_DM, -1, "A123BJC5D6E71", 0, 16, 16, "**NOT SAME** ANSI/HIBC 2.6 - 2016 Figure 3 TODO: investigate",
                    "1010101010101010"
                    "1110000011011011"
                    "1100001110001000"
                    "1110101011011111"
                    "1100110100001000"
                    "1011000001001001"
                    "1100010011110100"
                    "1000101001010101"
                    "1010110011110000"
                    "1011000001001111"
                    "1000010001001110"
                    "1001111110001111"
                    "1000110101010010"
                    "1101101110100101"
                    "1100101101000010"
                    "1111111111111111"
                },
        /*  9*/ { BARCODE_HIBC_DM, -1, "A123BJC5D6E71/$$52001510X3", 0, 20, 20, "**NOT SAME** ANSI/HIBC 2.6 - 2016 Section 4.3.3 TODO: investigate",
                    "10101010101010101010"
                    "11100000100101100001"
                    "11000011111010101100"
                    "11101011100011000101"
                    "11001100011011000100"
                    "10110010010000101011"
                    "11000000100101100010"
                    "10000110010100000101"
                    "10111010001100001110"
                    "11111100101000000011"
                    "11110110001001111110"
                    "11100111110010000011"
                    "11000010001110101000"
                    "10110110001001010001"
                    "11100011101111010110"
                    "10000010110000110001"
                    "10000100001100100110"
                    "10111011000001111101"
                    "10110110110000011010"
                    "11111111111111111111"
                },
        /* 10*/ { BARCODE_HIBC_DM, -1, "H123ABC01234567890", 0, 12, 26, "ANSI/HIBC 2.6 - 2016 Figure C2, same",
                    "10101010101010101010101010"
                    "10111011011011110101001101"
                    "10010110000001001100110100"
                    "10010001010100001011110001"
                    "11010101011010110100111100"
                    "10000101110000001110001101"
                    "11011011110011001011100000"
                    "10010001101011100010001001"
                    "10000001101101100110101010"
                    "11001111011110011111010001"
                    "10010010001100110000011010"
                    "11111111111111111111111111"
                },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, \"%s\", %s, %d, %d, \"%s\",\n",
                                i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                                data[i].data, testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
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
        { "test_buffer", test_buffer, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
