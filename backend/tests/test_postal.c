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

// USPS Publication 25 (July 2003) Designing Letter and Reply Mail https://web.archive.org/web/20050118015758/http://www.siemons.com/forms/pdf/designing_letter_reply_mail.pdf
// USPS DMM Domestic Mail Manual https://pe.usps.com/DMM300
// USPS Publication 197 (Sept 2004) Confirm User Guide https://web.archive.org/web/20060505214851/https://mailtracking.usps.com/mtr/resources/documents/Guide.pdf

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
        /*  0*/ { BARCODE_FLAT, "1", 90, 0, 1, 810 },
        /*  1*/ { BARCODE_FLAT, "1", 91, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_POSTNET, "1", 11, 0, 2, 185 },
        /*  3*/ { BARCODE_POSTNET, "1", 12, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_FIM, "D", 1, 0, 1, 17 },
        /*  5*/ { BARCODE_FIM, "D", 2, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_RM4SCC, "1", 50, 0, 3, 411 },
        /*  7*/ { BARCODE_RM4SCC, "1", 51, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_JAPANPOST, "1", 20, 0, 3, 133 },
        /*  9*/ { BARCODE_JAPANPOST, "1", 21, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { BARCODE_KOREAPOST, "1", 6, 0, 1, 162 },
        /* 11*/ { BARCODE_KOREAPOST, "1", 7, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 12*/ { BARCODE_PLANET, "1", 13, 0, 2, 215 },
        /* 13*/ { BARCODE_PLANET, "1", 14, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { BARCODE_KIX, "1", 18, 0, 3, 143 },
        /* 15*/ { BARCODE_KIX, "1", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 16*/ { BARCODE_DAFT, "D", 50, 0, 3, 99 },
        /* 17*/ { BARCODE_DAFT, "D", 51, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[4096];

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

static void test_koreapost(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int ret_encode;
        int ret_vector;

        int expected_height;
        int expected_rows;
        int expected_width;
    };
    struct item data[] = {
        /* 0*/ { "123456", 0, 0, 50, 1, 167 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_KOREAPOST;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        if (data[i].ret_vector != -1) {
            assert_equal(symbol->height, data[i].expected_height, "i:%d symbol->height %d != %d\n", i, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_japanpost(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int ret_encode;
        int ret_vector;

        int expected_height;
        int expected_rows;
        int expected_width;
        char *comment;
    };
    struct item data[] = {
        /* 0*/ { "123", 0, 0, 8, 3, 133, "Check 3" },
        /* 1*/ { "123456-AB", 0, 0, 8, 3, 133, "Check 10" },
        /* 2*/ { "123456", 0, 0, 8, 3, 133, "Check 11" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_JAPANPOST;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        if (data[i].ret_vector != -1) {
            assert_equal(symbol->height, data[i].expected_height, "i:%d symbol->height %d != %d\n", i, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

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
        /*  0*/ { BARCODE_FLAT, "1234567890", 0, 1, 90 },
        /*  1*/ { BARCODE_FLAT, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  2*/ { BARCODE_POSTNET, "12345", 0, 2, 95 },
        /*  3*/ { BARCODE_POSTNET, "123457689", 0, 2, 155 },
        /*  4*/ { BARCODE_POSTNET, "12345768901", 0, 2, 185 },
        /*  5*/ { BARCODE_POSTNET, "1234", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_POSTNET, "123456", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  7*/ { BARCODE_POSTNET, "123456789012", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_POSTNET, "1234A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  9*/ { BARCODE_FIM, "a", 0, 1, 17 },
        /* 10*/ { BARCODE_FIM, "b", 0, 1, 17 },
        /* 11*/ { BARCODE_FIM, "c", 0, 1, 17 },
        /* 12*/ { BARCODE_FIM, "d", 0, 1, 17 },
        /* 13*/ { BARCODE_FIM, "ad", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { BARCODE_FIM, "e", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 15*/ { BARCODE_RM4SCC, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 3, 299 },
        /* 16*/ { BARCODE_RM4SCC, "a", 0, 3, 19 }, // Converts to upper
        /* 17*/ { BARCODE_RM4SCC, ",", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 18*/ { BARCODE_JAPANPOST, "1234567890-ABCDEFGH", 0, 3, 133 },
        /* 19*/ { BARCODE_JAPANPOST, "a", 0, 3, 133 }, // Converts to upper
        /* 20*/ { BARCODE_JAPANPOST, ",", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 21*/ { BARCODE_KOREAPOST, "123456", 0, 1, 167 },
        /* 22*/ { BARCODE_KOREAPOST, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 23*/ { BARCODE_PLANET, "12345678901", 0, 2, 185 },
        /* 24*/ { BARCODE_PLANET, "1234567890123", 0, 2, 215 },
        /* 25*/ { BARCODE_PLANET, "1234567890", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 26*/ { BARCODE_PLANET, "123456789012", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 27*/ { BARCODE_PLANET, "12345678901234", ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 28*/ { BARCODE_PLANET, "1234567890A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 29*/ { BARCODE_KIX, "0123456789ABCDEFGH", 0, 3, 143 },
        /* 30*/ { BARCODE_KIX, "a", 0, 3, 7 }, // Converts to upper
        /* 31*/ { BARCODE_KIX, ",", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 32*/ { BARCODE_DAFT, "DAFT", 0, 3, 7 },
        /* 33*/ { BARCODE_DAFT, "a", 0, 3, 1 }, // Converts to upper
        /* 34*/ { BARCODE_DAFT, "B", ZINT_ERROR_INVALID_DATA, -1, -1 },
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
        /*  0*/ { BARCODE_FLAT, "1304056", 0, 1, 63, "Verified manually against bwipp and tec-it",
                    "100000000001000000000000000000100000000000000000010000000001000"
                },
        /*  1*/ { BARCODE_POSTNET, "12345678901", 0, 2, 185, "USPS Publication 25 (2003) Exhibit 4-1",
                    "10000000000010010000000010000010000000010010000000010000000010000010000010000000010010000000010000000000010010000000010000010000010000000010010000000000000000000010010000010000000010010"
                    "10010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010"
                },
        /*  2*/ { BARCODE_POSTNET, "555551237", 0, 2, 155, "Verified manually against bwipp and tec-it",
                    "10000010000010000000010000010000000010000010000000010000010000000010000010000000000000010010000000010000010000000010010000010000000000010000000010000010010"
                    "10010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010"
                },
        /*  3*/ { BARCODE_FIM, "C", 0, 1, 17, "USPS DMM Exhibit 8.2.0 FIM C",
                    "10100010001000101"
                },
        /*  4*/ { BARCODE_RM4SCC, "BX11LT1A", 0, 3, 75, "Verified manually against bwipp and tec-it",
                    "100010001010100000000010100000101010000010100010000000101000100010100000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001010000010000010001000100010001010000010101000000010001010001000000010101"
                },
        /*  5*/ { BARCODE_RM4SCC, "W1J0TR01", 0, 3, 75, "Verified manually against bwipp and tec-it",
                    "101010000000001010100000100000101010001000100010000000101000001010101000001"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "000010100000100010001000100000101010100000100000100000101000100010001010001"
                },
        /*  6*/ { BARCODE_JAPANPOST, "12345671-2-3", 0, 3, 133, "Verified manually against bwipp and tec-it",
                    "1000101000100010001010101000100010001010101000101000001000100010001000001010000010000010000010000010000010000010000010000010100010001"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "1010101000101000101000100010100010100010001010101000001000101000001000101000001000001000001000001000001000001000001000001000100010101"
                },
        /*  7*/ { BARCODE_KOREAPOST, "010230", 0, 1, 167, "Verified manually against tec-it",
                    "10001000100000000000100010000000000010001000100000001000000010001000100010001000100000000000100000000001000100010001000100010001000000000001000000010001000000010001000"
                },
        /*  8*/ { BARCODE_PLANET, "4012345235636", 0, 2, 215, "USPS Publication 197 (2004) Exhibit 5; verified manually against bwipp and tec-it",
                    "10010000010010000000000010010010010010010000000010010000010000010010000000010010000010010000010000010000010010010000010000010010000000010010000010000010010000000010010010010000000010010000000010010010000000010010010"
                    "10010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010"
                },
        /*  9*/ { BARCODE_PLANET, "40123452356", 0, 2, 185, "Verified manually against bwipp and tec-it",
                    "10010000010010000000000010010010010010010000000010010000010000010010000000010010000010010000010000010000010010010000010000010010000000010010000010000010010000000010010010000010000010010"
                    "10010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010010"
                },
        /* 10*/ { BARCODE_KIX, "2500GG30250", 0, 3, 87, "PostNL Handleiding KIX code Section 2.1 Example 1",
                    "000010100000101000001010000010100010100000101000000010100000101000001010000010100000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001010001010000000001010000010101000100010001000100000100000101000101000101000000000101"
                },
        /* 11*/ { BARCODE_KIX, "2130VA80430", 0, 3, 87, "PostNL Handleiding KIX code Section 2.1 Example 2",
                    "000010100000101000001010000010101010000000100010001000100000101000001010000010100000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001010000010001010000010000010100010001010001000001010000000101010001000100000100000101"
                },
        /* 12*/ { BARCODE_KIX, "1231GF156X2", 0, 3, 87, "PostNL Handleiding KIX code Section 2.1 Example 3",
                    "000010100000101000001010000010100010100000101000000010100000101000100010101000000000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001000100010100010000010001000101000100010000010001000101010000000001010100000100010100"
                },
        /* 13*/ { BARCODE_KIX, "1231FZ13Xhs", 0, 3, 87, "PostNL Handleiding KIX code Section 2.1 Example 4",
                    "000010100000101000001010000010100010100010100000000010100000101010100000001010001000100"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001000100010100010000010001000101000001010100000001000101000001010000010101000001000100"
                },
        /* 14*/ { BARCODE_DAFT, "DAFTTFADFATDTATFT", 0, 3, 33, "Verified manually against bwipp and tec-it",
                    "001010000010100010100000001000100"
                    "101010101010101010101010101010101"
                    "100010000010001010000010000000100"
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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_koreapost", test_koreapost, 1, 0, 1 },
        { "test_japanpost", test_japanpost, 1, 0, 1 },
        { "test_input", test_input, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
