/*
    libzint - the open source barcode library
    Copyright (C) 2008 - 2020 Robin Stuart <rstuart114@gmail.com>

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

//#define TEST_ENCODE_GENERATE_EXPECTED 1

#include "testcommon.h"

static void test_options(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_1;
        int option_2;
        int ret;
        char* comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "é", -1, -1, 0, "" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_CODABLOCKF;
        symbol->option_1 = data[i].option_1;
        symbol->option_2 = data[i].option_2;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        int option_1;
        int option_2;
        unsigned char* data;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, "AIM", 0, 1, 68, "Same as CODE128",
                    "11010010000101000110001100010001010111011000101110110001100011101011"
               },
        /*  1*/ { UNICODE_MODE, -1, -1, "AAAAAAAAA", 0, 4, 90, "",
                    "110100001001011110111010010000110101000110001010001100010100011000110010001001100011101011"
                    "110100001001011110111011000100100101000110001010001100010100011000100100110001100011101011"
                    "110100001001011110111010110011100101000110001010001100010100011000100010011001100011101011"
                    "110100001001011101111010111011000101111011101001011000010010011110111001001101100011101011"
               },
        /*  2*/ { UNICODE_MODE, 3, -1, "AAAAAAAAA", 0, 3, 101, "Verified manually against tec-it (bwipp differs)",
                    "11010000100101111011101001011000010100011000101000110001010001100010100011000110110011001100011101011"
                    "11010000100101111011101100010010010100011000101000110001010001100010100011000111101000101100011101011"
                    "11010000100101111011101011001110010100011000101110111101100110110011001110010110001001001100011101011"
               },
        /*  3*/ { UNICODE_MODE, -1, -1, "AAAAAAAAAAAAAAA", 0, 6, 90, "",
                    "110100001001011110111010000100110101000110001010001100010100011000100110011101100011101011"
                    "110100001001011110111011000100100101000110001010001100010100011000100100110001100011101011"
                    "110100001001011110111010110011100101000110001010001100010100011000100010011001100011101011"
                    "110100001001011110111010011011100101000110001010001100010100011000100110001001100011101011"
                    "110100001001011110111010011001110101000110001010001100010100011000110010010001100011101011"
                    "110100001001011101111010001110110101111011101000110001010111101000110111101101100011101011"
               },
        /*  4*/ { UNICODE_MODE, -1, 14, "AAAAAAAAAAAAAAA", 0, 2, 156, "",
                    "110100001001011110111010100001100101000110001010001100010100011000101000110001010001100010100011000101000110001010001100010100011000110001000101100011101011"
                    "110100001001011110111011000100100101000110001010001100010100011000101000110001010001100010100011000101110111101110111101011011000110111000101101100011101011"
               },
        /*  5*/ { UNICODE_MODE, 3, -1, "AAAAAAAAAAAAAAA", 0, 3, 123, "Differs from tec-it (and bwipp); TODO: verify",
                    "110100001001011110111010010110000101000110001010001100010100011000101000110001010001100010100011000101111001001100011101011"
                    "110100001001011110111011000100100101000110001010001100010100011000101000110001010001100010100011000110000101001100011101011"
                    "110100001001011110111010110011100101000110001010001100010100011000101110111101110111101011011000110110110001101100011101011"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_CODABLOCKF;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        //symbol->debug = ZINT_DEBUG_PRINT;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, %d, %d, \"%s\", %s, %d, %d, \"%s\",\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
        testUtilModulesDump(symbol, "                    ", "\n");
        printf("               },\n");
        #else
        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            if (ret == 0) {
                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
            }
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// #181 Christian Hartlage OSS-Fuzz
static void test_fuzz(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "\034\034I", 3, 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_CODABLOCKF;
        int length = data[i].length;
        if (length == -1) {
            length = strlen(data[i].data);
        }

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_options();
    test_encode();
    test_fuzz();

    testReport();

    return 0;
}
