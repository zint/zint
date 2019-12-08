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

#include <stdio.h>
#include <string.h>
#include <zint.h>
#include "testcommon.h"

//#define TEST_GENERATE_EXPECTED 1

static void test_best_supported_set(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        int ret;
        float w;
        float h;
        int ret_vector;

        int expected_rows;
        int expected_width;
        char* comment;
        unsigned char* expected;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_MAXICODE, "am.//ab,\034TA# z\015!", 0, 100, 100, 0, 33, 30, "TODO: Better data and verify expected",
                    "111010000101111000111101010111"
                    "111110000000010100111000000000"
                    "110000101100110100111010101011"
                    "010101010101010101010101010100"
                    "000000000000000000000000000000"
                    "101010101010101010101010101000"
                    "010101010101010101010101010110"
                    "000000000000000000000000000000"
                    "101010101010101010101010101011"
                    "010101010111001100000101010110"
                    "000000001011000010000000000010"
                    "101010101100000000100110101010"
                    "010101001100000000101101010101"
                    "000000100000000000010000000010"
                    "101010110000000000010010101010"
                    "010101011000000000000101010110"
                    "000000001000000000001000000010"
                    "101010001000000000001010101000"
                    "010101010000000000001101010101"
                    "000000001100000000000000000010"
                    "101010110010000000010110101010"
                    "010101010100000001111001010100"
                    "000000001110110111111100000011"
                    "101010100110111101011010101010"
                    "010101010101010101010011101000"
                    "000000000000000000001101100000"
                    "101010101010101010100000100110"
                    "101001001101110001001011010000"
                    "100100110110001010011000011100"
                    "011011000001011011100100100110"
                    "111001100000101101000111001000"
                    "111100000110000011011101001110"
                    "010100101001110111101010110010"
                },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped_data[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_GENERATE_EXPECTED
        printf("        /*%2d*/ { %s, \"%s\", %d, %.0f, %.0f, %d, %d, %d, \"%s\",\n",
                i, testUtilBarcodeName(data[i].symbology), testUtilEscape(data[i].data, length, escaped_data, sizeof(escaped_data)), ret,
                data[i].w, data[i].h, data[i].ret_vector, symbol->rows, symbol->width, data[i].comment);
        testUtilModulesDump(symbol, "                    ",  "\n");
        printf("                },\n");
        #else

        assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

        int width, row;
        ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
        assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_best_supported_set();

    testReport();

    return 0;
}
