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
        unsigned char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_AUSPOST, "1", 23, 0, 3, 133 },
        /*  1*/ { BARCODE_AUSPOST, "1", 24, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_AUSPOST, "1", 18, 0, 3, 133 },
        /*  3*/ { BARCODE_AUSPOST, "1", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_AUSPOST, "1", 16, 0, 3, 103 },
        /*  5*/ { BARCODE_AUSPOST, "1", 17, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_AUSPOST, "1", 13, 0, 3, 103 },
        /*  7*/ { BARCODE_AUSPOST, "1", 14, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_AUSPOST, "1", 8, 0, 3, 73 },
        /*  9*/ { BARCODE_AUSPOST, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { BARCODE_AUSREPLY, "1", 8, 0, 3, 73 },
        /* 11*/ { BARCODE_AUSREPLY, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 12*/ { BARCODE_AUSROUTE, "1", 8, 0, 3, 73 },
        /* 13*/ { BARCODE_AUSROUTE, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { BARCODE_AUSREDIRECT, "1", 8, 0, 3, 73 },
        /* 15*/ { BARCODE_AUSREDIRECT, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[64];

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

static void test_hrt(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;

        unsigned char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_AUSPOST, "12345678901234567890123", "" }, // None
    };
    int data_size = ARRAY_SIZE(data);

    char *text;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

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
        unsigned char *data;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_AUSPOST, "12345678", 0, 3, 73 },
        /*  1*/ { BARCODE_AUSPOST, "1234567A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  2*/ { BARCODE_AUSPOST, "12345678ABcd#", 0, 3, 103 },
        /*  3*/ { BARCODE_AUSPOST, "12345678ABcd!", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  4*/ { BARCODE_AUSPOST, "12345678ABcd#", 0, 3, 103 },
        /*  5*/ { BARCODE_AUSPOST, "1234567890123456", 0, 3, 103 },
        /*  6*/ { BARCODE_AUSPOST, "123456789012345A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  7*/ { BARCODE_AUSPOST, "12345678ABCDefgh #", 0, 3, 133 }, // Length 18
        /*  8*/ { BARCODE_AUSPOST, "12345678901234567890123", 0, 3, 133 },
        /*  9*/ { BARCODE_AUSPOST, "1234567890123456789012A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 10*/ { BARCODE_AUSREPLY, "12345678", 0, 3, 73 },
        /* 11*/ { BARCODE_AUSREPLY, "1234567", 0, 3, 73 }, // Leading zeroes added
        /* 12*/ { BARCODE_AUSROUTE, "123456", 0, 3, 73 },
        /* 13*/ { BARCODE_AUSROUTE, "12345", 0, 3, 73 },
        /* 14*/ { BARCODE_AUSREDIRECT, "1234", 0, 3, 73 },
        /* 15*/ { BARCODE_AUSREDIRECT, "123", 0, 3, 73 },
        /* 16*/ { BARCODE_AUSREDIRECT, "0", 0, 3, 73 },
        /* 17*/ { BARCODE_AUSPOST, "1234567", ZINT_ERROR_TOO_LONG, -1, -1 }, // No leading zeroes added
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

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

// Australia Post Customer Barcoding Technical Specifications (Revised Aug 2012)
// https://auspost.com.au/content/dam/auspost_corp/media/documents/customer-barcode-technical-specifications-aug2012.pdf
static void test_encode(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_AUSPOST, "119618420932573854", 0, 3, 133, "Australia Post Customer Barcoding Tech Specs Diagram 1; verified manually against tec-it",
                    "1000001010001010101000100010101000001010100000000000000000100000101000101000001000100000001000101000101000101000101000001000100010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010101000100000101010100010100000101010101000101000101000100000000000101000101000100000000000001000000010100010001010101000000"
                },
        /*  1*/ { BARCODE_AUSREPLY, "12345678", 0, 3, 73, "Verified manually against tec-it",
                    "1000101010001010100010101010100000100010000000001000001000000000100010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000000000101000101000100000001010101000101000000000100010101000101000000"
                },
        /*  2*/ { BARCODE_AUSROUTE, "34567890", 0, 3, 73, "Verified manually against tec-it",
                    "1000000000101010101010000010001000000010101000100010101010000000101000100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010000010000000101010100010100010101000100010101010001010001000000"
                },
        /*  3*/ { BARCODE_AUSREDIRECT, "98765432", 0, 3, 73, "Verified manually against tec-it",
                    "1000001010000010000000100010100010101010100000101010101000100010100010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000001010100010101010001010001000000010101000000000001010101000001010000"
                },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
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

// #181 Christian Hartlage OSS-Fuzz
static void test_fuzz(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_AUSROUTE, "A\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 1*/ { BARCODE_AUSROUTE, "1\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 2*/ { BARCODE_AUSREPLY, "A\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 3*/ { BARCODE_AUSREPLY, "1\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 4*/ { BARCODE_AUSREDIRECT, "A\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 5*/ { BARCODE_AUSREDIRECT, "1\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->debug |= debug;

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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_hrt", test_hrt, 1, 0, 1 },
        { "test_input", test_input, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_fuzz", test_fuzz, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
