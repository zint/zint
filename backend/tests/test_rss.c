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

#include "testcommon.h"

//#define TEST_RSS_LINEAR_GENERATE_EXPECTED 1
//#define TEST_RSS_COMPOSITE_GENERATE_EXPECTED 1

static void test_linear(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        int ret_encode;
        float w;
        float h;
        int ret_render;

        int expected_rows;
        int expected_width;
        unsigned char* expected;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_RSS14, "1234567890123", 0, 100, 30, 1, 1, 96, "010111010010000001001110000000010100001011111010110100011001100101111111110001011011000111000101" },
        /* 1*/ { BARCODE_RSS14, "0000004537076", 0, 100, 30, 1, 1, 96, "010101001000000001000100000000010111111100101010111101111101010101111111110111010100100000000101" },
        /* 2*/ { BARCODE_RSS14, "0000004537077", 0, 100, 30, 1, 1, 96, "010101001000000001000111000000010111111101001010101010110000000101111100000111011111111011010101" },
        /* 3*/ { BARCODE_RSS14, "0000004537078", 0, 100, 30, 1, 1, 96, "010101001000000001000111000000010111111101001010101011010000000101111000000011011111111011010101" },
        /* 4*/ { BARCODE_RSS14, "0000000001596", 0, 100, 30, 1, 1, 96, "010101001000000001001111100000010111111100101010111101111101010101111100000111011111111011010101" },
        /* 5*/ { BARCODE_RSS14, "0000000001597", 0, 100, 30, 1, 1, 96, "010101001000000001011111000000010111111100101010101010110000000101111100000111011111110111010101" },
        /* 6*/ { BARCODE_RSS14, "0000000001598", 0, 100, 30, 1, 1, 96, "010101001000000001011111000000010111111100101010101011010000000101111000000011011111110111010101" },
        /* 7*/ { BARCODE_RSS_LTD, "1234567890123", 0, 100, 30, 1, 1, 74, "01001100111100101000100111010110101011001001010010101001010000011100011101" },
        /* 8*/ { BARCODE_RSS_LTD, "0000002013570", 0, 100, 30, 1, 1, 74, "01010101010100000010000001110100101101011001010111111110111111010101010101" },
        /* 9*/ { BARCODE_RSS_LTD, "0000002013571", 0, 100, 30, 1, 1, 74, "01010101010100000011000000110101011010100011010101010101000000100000011101" },
        /*10*/ { BARCODE_RSS_LTD, "0000002013572", 0, 100, 30, 1, 1, 74, "01010101010100000011000000110101010010111001010101010101000000110000001101" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        ret = ZBarcode_Render( symbol, data[i].w, data[i].h );
        assert_equal(ret, data[i].ret_render, "i:%d ZBarcode_Render ret %d != %d\n", i, ret, data[i].ret_render);

        #ifdef TEST_RSS_LINEAR_GENERATE_EXPECTED
        printf("symbology %d, data %s, length %d, rows %d, height %d\n", symbol->symbology, data[i].data, length, symbol->rows, symbol->height);
        testUtilModulesDump(symbol);
        #else

        assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

        int width, row;
        ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
        assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_composite(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        unsigned char* composite;
        int ret_encode;
        float w;
        float h;
        int ret_render;

        int expected_rows;
        int expected_width;
        unsigned char* expected;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_RSS14_CC, "0361234567890", "[11]990102", 0, 100, 30, 1, 5, 100, // TODO: verify expected
                    "1101101110111010001001110001100110010100000010011101001110100110011110011101110010010000110110001010"
                    "1101101100111000010011010001000010111110011010011001001011111001000111011111010001101110110010001010"
                    "1101101000110111110010010001011110100100000010011001101001111000110011011000010111111010111010001010"
                    "0000000000010110001110100000000101001011010111111011001101010000011010000000010100101000110011110000"
                    "0000010011101001110001001111111000010100101000000100110010101111100101111111100011010111001100001101"
                },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        int length = strlen(data[i].data);
        assert_zero(length >= 128, "i:%d length %d >= 128\n", i, length);
        strcpy(symbol->primary, data[i].data);

        int composite_length = strlen(data[i].composite);

        ret = ZBarcode_Encode(symbol, data[i].composite, composite_length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        ret = ZBarcode_Render( symbol, data[i].w, data[i].h );
        assert_equal(ret, data[i].ret_render, "i:%d ZBarcode_Render ret %d != %d\n", i, ret, data[i].ret_render);

        #ifdef TEST_RSS_COMPOSITE_GENERATE_EXPECTED
        printf("symbology %d, data %s, length %d, rows %d, height %d\n", symbol->symbology, data[i].data, length, symbol->rows, symbol->height);
        testUtilModulesDump(symbol);
        #else

        assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

        int width, row;
        ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
        assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_linear();
    test_composite();

    testReport();

    return 0;
}
