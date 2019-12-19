/*
    libzint - the open source barcode library
    Copyright (C) 2008-2019 Robin Stuart <rstuart114@gmail.com>

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

//#define TEST_PDF417_ENCODE_GENERATE_EXPECTED 1

static void test_pdf417_options(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_1;
        int option_2;
        int option_3;
        int ret_encode;
        int ret_vector;

        int expected_option_1;
        int expected_option_2;
        int expected_rows;
        int expected_width;
        int compare_previous;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "12345", -1, -1, -1, 0, 0, 2, 2, 6, 103, -1 }, // ECC auto-set to 2, cols auto-set to 2
        /*  1*/ { "12345", -1, -1, 928, 0, 0, 2, 2, 6, 103, 0 }, // Option 3 ignored
        /*  2*/ { "12345", -1, -1, 300, 0, 0, 2, 2, 6, 103, 0 }, // Option 3 ignored
        /*  3*/ { "12345", 3, -1, -1, 0, 0, 3, 3, 7, 120, -1 }, // ECC 3, cols auto-set to 3
        /*  4*/ { "12345", 3, 2, -1, 0, 0, 3, 2, 10, 103, -1 }, // ECC 3, cols 2
        /*  5*/ { "12345", 8, 2, -1, ZINT_ERROR_TOO_LONG, -1, 8, 3, 0, 0, -1 }, // ECC 8, cols 2, fails
        /*  6*/ { "12345", 7, 2, -1, 0, 0, 7, 3, 87, 120, -1 }, // ECC 7, cols 2 auto-upped to 3
        /*  7*/ { "12345", -1, 10, -1, 0, 0, 2, 10, 3, 239, -1 }, // ECC auto-set to 2, cols 10
    };
    int data_size = sizeof(data) / sizeof(struct item);

    struct zint_symbol previous_symbol;

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_PDF417;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        if (data[i].option_3 != -1) {
            symbol->option_3 = data[i].option_3;
        }
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

		assert_equal(symbol->option_1, data[i].expected_option_1, "i:%d symbol->option_1 %d != %d (%d)\n", i, symbol->option_1, data[i].expected_option_1, data[i].option_1);
		assert_equal(symbol->option_2, data[i].expected_option_2, "i:%d symbol->option_2 %d != %d (%d)\n", i, symbol->option_2, data[i].expected_option_2, data[i].option_2);
		if (data[i].option_3 != -1) {
			assert_equal(symbol->option_3, data[i].option_3, "i:%d symbol->option_3 %d != %d\n", i, symbol->option_3, data[i].option_3); // Unchanged
		} else {
			assert_zero(symbol->option_3, "i:%d symbol->option_3 %d != 0\n", i, symbol->option_3);
		}

		assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
		assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

        if (data[i].compare_previous != -1) {
            ret = testUtilSymbolCmp(symbol, &previous_symbol);
            assert_equal(!ret, !data[i].compare_previous, "i:%d testUtilSymbolCmp !ret %d != %d\n", i, ret, data[i].compare_previous);
        }
        memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_pdf417_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "PDF417 Symbology Standard", 2, 2, 0, 13, 103, "ISO 15438:2015 Figure 1 **NOT SAME** TODO: investigate",
                    "1111111101010100011110101001111000101011000110000001000011000110010011110101011110000111111101000101001"
                    "1111111101010100011111010100011000110110000011110101101000011100010011111101010011100111111101000101001"
                    "1111111101010100011101010111111000111010000111110101011001101111000011010100011111000111111101000101001"
                    "1111111101010100011111010111111010100000100010100001000001001000001010101111001111000111111101000101001"
                    "1111111101010100011010111000010000111001000110010001111001000001010011101011100011000111111101000101001"
                    "1111111101010100011110101111010000101110001000110001001111000010100011110101111000010111111101000101001"
                    "1111111101010100011010011100111100110000010010011001011100011101000011010011101111000111111101000101001"
                    "1111111101010100011110100101000000111111011000101101111010110011000010101111110011100111111101000101001"
                    "1111111101010100011010011011111100111110100001110101100011111001001010100110001111100111111101000101001"
                    "1111111101010100010100011101110000110110111100111101101100100100000011010001100011100111111101000101001"
                    "1111111101010100011010011100010000111000000101101001101101111100110011101001110011000111111101000101001"
                    "1111111101010100010100010001111000101010000111100001011111100011101010100011011111000111111101000101001"
                    "1111111101010100010100000101000000110000101011000001001000110000110010100001100000110111111101000101001"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_PDF417;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_PDF417_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %d, %d, %s, %d, %d, \"%s\",\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].data, data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
                symbol->rows, symbol->width, data[i].comment);
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

int main()
{
    test_pdf417_options();
    test_pdf417_encode();

    testReport();

    return 0;
}
