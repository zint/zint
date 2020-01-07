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

//#define TEST_ENCODE_GENERATE_EXPECTED 1

// Note need ZINT_SANITIZE set for these
static void test_buffer(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int eci;
        int input_mode;
        int output_options;
        int ret;
        char* comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "1", 16383, UNICODE_MODE, READER_INIT, 0, "" },
        /*  1*/ { "000106j 05 Galeria A NaÃ§Ã£o0000000000", 3, UNICODE_MODE, 0, 0, "From Okapi, consecutive use of upper shift; ticket #176" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DATAMATRIX;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;
        symbol->output_options = data[i].output_options;

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
        unsigned char* data;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { "1234abcd", 0, 14, 14, "",
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
        /*  1*/ { "A1B2C3D4E5F6G7H8I9J0K1L2", 0, 18, 18, "ISO 16022:2006 Figure 1",
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
        /*  2*/ { "30Q324343430794<OQQ", 0, 16, 16, "ISO 16022:2006 Figure R.1",
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
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DATAMATRIX;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { \"%s\", %s, %d, %d, \"%s\",\n", i, data[i].data, testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
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
    test_buffer();
    test_encode();

    testReport();

    return 0;
}
