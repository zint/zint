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
        int symbology;
        int option_2;
        unsigned char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_MSI_PLESSEY, -1, "9", 55, 0, 1, 667 },
        /*  1*/ { BARCODE_MSI_PLESSEY, -1, "9", 56, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_MSI_PLESSEY, 1, "9", 18, 0, 1, 235 }, // 1 mod-10 check digit
        /*  3*/ { BARCODE_MSI_PLESSEY, 1, "9", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_MSI_PLESSEY, 2, "9", 18, 0, 1, 247 }, // 2 mod-10 check digits
        /*  5*/ { BARCODE_MSI_PLESSEY, 2, "9", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_MSI_PLESSEY, 3, "9", 55, 0, 1, 679 }, // 1 mod-11 check digit
        /*  7*/ { BARCODE_MSI_PLESSEY, 3, "9", 56, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_MSI_PLESSEY, 4, "9", 18, 0, 1, 247 }, // 1 mod-11 and 1 mod-10 check digit
        /*  9*/ { BARCODE_MSI_PLESSEY, 4, "9", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { BARCODE_PLESSEY, -1, "A", 65, 0, 1, 1107 },
        /* 11*/ { BARCODE_PLESSEY, -1, "A", 66, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[4096];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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

static void test_hrt(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int option_2;
        unsigned char *data;

        unsigned char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_MSI_PLESSEY, -1, "1234567", "1234567" },
        /*  1*/ { BARCODE_MSI_PLESSEY, 0, "1234567", "1234567" },
        /*  2*/ { BARCODE_MSI_PLESSEY, 1, "1234567", "12345674" },
        /*  3*/ { BARCODE_MSI_PLESSEY, 2, "1234567", "123456741" },
        /*  4*/ { BARCODE_MSI_PLESSEY, 3, "1234567", "12345674" },
        /*  5*/ { BARCODE_MSI_PLESSEY, 4, "1234567", "123456741" },
        /*  6*/ { BARCODE_MSI_PLESSEY, 1, "123456", "1234566" },
        /*  7*/ { BARCODE_MSI_PLESSEY, 2, "123456", "12345666" },
        /*  8*/ { BARCODE_MSI_PLESSEY, 3, "123456", "1234560" },
        /*  9*/ { BARCODE_MSI_PLESSEY, 4, "123456", "12345609" },
        /* 10*/ { BARCODE_MSI_PLESSEY, 3, "2211", "221110" }, // Mod-11 check digit '10'
        /* 11*/ { BARCODE_MSI_PLESSEY, 4, "2211", "2211100" },
        /* 12*/ { BARCODE_PLESSEY, -1, "0123456789ABCDEF", "0123456789ABCDEF" },
    };
    int data_size = ARRAY_SIZE(data);

    char *text;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_zero(strcmp(symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int option_2;
        unsigned char *data;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_MSI_PLESSEY, -1, "1", 0, 1, 19 },
        /*  1*/ { BARCODE_MSI_PLESSEY, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  2*/ { BARCODE_PLESSEY, -1, "A", 0, 1, 83 },
        /*  3*/ { BARCODE_PLESSEY, -1, "G", ZINT_ERROR_INVALID_DATA, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
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

    int ret;
    struct item {
        int symbology;
        int option_2;
        unsigned char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_MSI_PLESSEY, -1, "1234567890", 0, 1, 127, "Verified manually against bwipp and tec-it",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001"
                },
        /*  1*/ { BARCODE_MSI_PLESSEY, 1, "1234567890", 0, 1, 139, "Verified manually against bwipp and tec-it",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001101101001"
                },
        /*  2*/ { BARCODE_MSI_PLESSEY, 2, "1234567890", 0, 1, 151, "Verified manually against bwipp and tec-it",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001101101001001001101001"
                },
        /*  3*/ { BARCODE_MSI_PLESSEY, 3, "1234567890", 0, 1, 139, "Verified manually against bwipp",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001101101001"
                },
        /*  4*/ { BARCODE_MSI_PLESSEY, 4, "1234567890", 0, 1, 151, "Verified manually against bwipp",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001101101001001001101001"
                },
        /*  5*/ { BARCODE_MSI_PLESSEY, 3, "2211", 0, 1, 79, "Produces mod-11 '10' check digit; verified manually against bwipp (badmod11)",
                    "1101001001101001001001101001001001001101001001001101001001001101001001001001001"
                },
        /*  6*/ { BARCODE_MSI_PLESSEY, 4, "2211", 0, 1, 91, "Verified manually against bwipp (badmod11)",
                    "1101001001101001001001101001001001001101001001001101001001001101001001001001001001001001001"
                },
        /*  7*/ { BARCODE_PLESSEY, -1, "0123456789ABCDEF", 0, 1, 323, "Verified manually against bwipp",
                    "11101110100011101000100010001000111010001000100010001110100010001110111010001000100010001110100011101000111010001000111011101000111011101110100010001000100011101110100010001110100011101000111011101110100011101000100011101110111010001110111010001110111011101110111011101110111010001000111010001000100010001110001000101110111"
                },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
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
        { "test_hrt", test_hrt, 1, 0, 1 },
        { "test_input", test_input, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
