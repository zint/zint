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

//#define TEST_ENCODE_GENERATE_EXPECTED 1

static void test_input(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        unsigned char* data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0 }, // 59 chars
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, ZINT_ERROR_TOO_LONG }, // 60 chars
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, "éAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéAé", -1, 0 }, // 39 chars
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, "éAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéAéA", -1, ZINT_ERROR_TOO_LONG }, // 40 chars (+ 20 shifts)
        /*  4*/ { BARCODE_CODE128, UNICODE_MODE, "\302\200", -1, ZINT_ERROR_INVALID_DATA }, // PAD U+0080, not in ISO 8859-1 although encodable in CODE128
        /*  5*/ { BARCODE_CODE128, DATA_MODE, "\302\200", -1, 0 }, // PAD U+0080, use binary
        /*  6*/ { BARCODE_CODE128, UNICODE_MODE, "\000\037é", 4, 0 },
        /*  7*/ { BARCODE_CODE128B, UNICODE_MODE, "\000\037é", 4, 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = data[i].input_mode;

        int length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        int output_options;
        unsigned char* data;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, -1, "AIM", 0, 1, 68, "ISO/IEC 15417:2007 Figure 1",
                    "11010010000101000110001100010001010111011000101110110001100011101011"
               },
        /*  1*/ { BARCODE_CODE128B, UNICODE_MODE, -1, "AIM", 0, 1, 68, "",
                    "11010010000101000110001100010001010111011000101110110001100011101011"
               },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, 16, "AIM", 0, 1, 79, "READER_INIT",
                    "1101001000010111100010101000110001100010001010111011000111010110001100011101011"
               },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, -1, "1234567890", 0, 1, 90, "",
                    "110100111001011001110010001011000111000101101100001010011011110110100111100101100011101011"
               },
        /*  4*/ { BARCODE_CODE128B, UNICODE_MODE, -1, "1234567890", 0, 1, 145, "",
                    "1101001000010011100110110011100101100101110011001001110110111001001100111010011101101110111010011001110010110010011101100101000110001100011101011"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = data[i].input_mode;
        if (data[i].output_options != -1) {
            symbol->output_options = data[i].output_options;
        }

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, %s, %d, \"%s\", %s, %d, %d, \"%s\",\n",
                i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].output_options, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
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

int main()
{
    test_input();
    test_encode();

    testReport();

    return 0;
}
