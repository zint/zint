/*
    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

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
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_C25STANDARD, "1", 80, 0, 1, 817 },
        /*  1*/ { BARCODE_C25STANDARD, "1", 81, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_C25INTER, "1", 90, 0, 1, 819 },
        /*  3*/ { BARCODE_C25INTER, "1", 91, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_C25IATA, "1", 45, 0, 1, 639 },
        /*  5*/ { BARCODE_C25IATA, "1", 46, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_C25LOGIC, "1", 80, 0, 1, 809 },
        /*  7*/ { BARCODE_C25LOGIC, "1", 81, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_C25IND, "1", 45, 0, 1, 649 },
        /*  9*/ { BARCODE_C25IND, "1", 46, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { BARCODE_DPLEIT, "1", 13, 0, 1, 135 },
        /* 11*/ { BARCODE_DPLEIT, "1", 14, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 12*/ { BARCODE_DPIDENT, "1", 11, 0, 1, 117 },
        /* 13*/ { BARCODE_DPIDENT, "1", 12, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { BARCODE_ITF14, "1", 13, 0, 1, 135 },
        /* 15*/ { BARCODE_ITF14, "1", 14, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[4096];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
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
        char *data;
        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_C25STANDARD, "123456789", "123456789" },
        /*  1*/ { BARCODE_C25INTER, "123456789", "0123456789" }, // Adds leading zero if odd
        /*  2*/ { BARCODE_C25IATA, "123456789", "123456789" },
        /*  3*/ { BARCODE_C25LOGIC, "123456789", "123456789" },
        /*  4*/ { BARCODE_C25IND, "123456789", "123456789" },
        /*  5*/ { BARCODE_DPLEIT, "123456789", "00001234567890" }, // Leading zeroes added to make 13 + appended checksum
        /*  6*/ { BARCODE_DPLEIT, "1234567890123", "12345678901236" },
        /*  7*/ { BARCODE_DPIDENT, "123456789", "001234567890" }, // Leading zeroes added to make 11 + appended checksum
        /*  8*/ { BARCODE_DPIDENT, "12345678901", "123456789016" },
        /*  9*/ { BARCODE_ITF14, "123456789", "00001234567895" }, // Leading zeroes added to make 13 + appended checksum
        /* 10*/ { BARCODE_ITF14, "1234567890123", "12345678901231" },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_C25STANDARD, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  1*/ { BARCODE_C25INTER, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  2*/ { BARCODE_C25IATA, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  3*/ { BARCODE_C25LOGIC, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  4*/ { BARCODE_C25IND, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  5*/ { BARCODE_DPLEIT, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  6*/ { BARCODE_DPIDENT, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  7*/ { BARCODE_ITF14, "A", ZINT_ERROR_INVALID_DATA, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    testStart("");

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    int ret;
    struct item {
        int symbology;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    // BARCODE_ITF14 examples verified manually against GS1 General Specifications 21.0.1
    struct item data[] = {
        /*  0*/ { BARCODE_C25STANDARD, "87654321", 0, 1, 97, "Standard Code 2 of 5; note zint uses 4X start/end wides while BWIPP uses 3X",
                    "1111010101110100010101000111010001110101110111010101110111011100010101000101110111010111011110101"
                },
        /*  1*/ { BARCODE_C25INTER, "87654321", 0, 1, 81, "Interleaved Code 2 of 5; verified manually against tec-it",
                    "101011101010111000100010001110111000101010001000111010111010001110101011100011101"
                },
        /*  2*/ { BARCODE_C25INTER, "602003", 0, 1, 63, "DX cartridge barcode https://en.wikipedia.org/wiki/Interleaved_2_of_5 example",
                    "101010111011100010001010111010001000111010001000111011101011101"
                },
        /*  3*/ { BARCODE_C25IATA, "87654321", 0, 1, 121, "IATA Code 2 of 5; verified manually against tec-it",
                    "1010111010101110101010101110111010111011101010111010111010101010111010111011101110101010101110101011101110101010111011101"
                },
        /*  4*/ { BARCODE_C25LOGIC, "87654321", 0, 1, 89, "Code 2 of 5 Data Logic; verified manually against tec-it",
                    "10101110100010101000111010001110101110111010101110111011100010101000101110111010111011101"
                },
        /*  5*/ { BARCODE_C25IND, "87654321", 0, 1, 131, "Industrial Code 2 of 5; verified manually against tec-it",
                    "11101110101110101011101010101011101110101110111010101110101110101010101110101110111011101010101011101010111011101010101110111010111"
                },
        /*  6*/ { BARCODE_DPLEIT, "0000087654321", 0, 1, 135, "Deutsche Post Leitcode; verified manually against tec-it",
                    "101010101110001110001010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000100010111011101"
                },
        /*  7*/ { BARCODE_DPLEIT, "5082300702800", 0, 1, 135, "Deutsche Post Leitcode https://de.wikipedia.org/wiki/Leitcode example",
                    "101011101011100010001011101000101110100011101110100010001010101110111000100010100011101110100011101010001110001010001011100011101011101"
                },
        /*  8*/ { BARCODE_DPIDENT, "00087654321", 0, 1, 117, "Deutsche Post Identcode; verified manually against tec-it",
                    "101010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000100010111011101"
                },
        /*  9*/ { BARCODE_DPIDENT, "39601313414", 0, 1, 117, "Deutsche Post Identcode https://de.wikipedia.org/wiki/Leitcode example",
                    "101011101110001010001010111011100010001011100010001010111011100010001010111010001011101011100010101110001000111011101"
                },
        /* 10*/ { BARCODE_ITF14, "0000087654321", 0, 1, 135, "ITF-14; verified manually against tec-it",
                    "101010101110001110001010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000101011100011101"
                },
        /* 11*/ { BARCODE_ITF14, "0950110153000", 0, 1, 135, "GS1 General Specifications Figure 5.1-2",
                    "101010100011101110001011101011100010001011100010101011100010001011101110100011100010001110101010101110001110001010001000111011101011101"
                },
        /* 12*/ { BARCODE_ITF14, "1540014128876", 0, 1, 135, "GS1 General Specifications Figure 5.3.2.4-1",
                    "101011100010100010111010101110001000111010001011101110100010001011101011100010001110101000111011101010111000100010001110001110101011101"
                },
        /* 13*/ { BARCODE_ITF14, "0950110153001", 0, 1, 135, "GS1 General Specifications Figure 5.3.6-1",
                    "101010100011101110001011101011100010001011100010101011100010001011101110100011100010001110101010101110001110001011101010001000111011101"
                },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];
    char bwipp_buf[4096];
    char bwipp_msg[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
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
