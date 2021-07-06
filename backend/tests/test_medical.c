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

    struct item {
        int symbology;
        int option_2;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODABAR, -1, "A1234567890123456789012345678901234567890123456789012345678B", 60, 0, 1, 602 },
        /*  1*/ { BARCODE_CODABAR, -1, "A12345678901234567890123456789012345678901234567890123456789B", 61, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_CODABAR, 1, "A1234567890123456789012345678901234567890123456789012345678B", 60, 0, 1, 612 },
        /*  3*/ { BARCODE_CODABAR, 1, "A12345678901234567890123456789012345678901234567890123456789B", 61, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_PHARMA, -1, "131070", 6, 0, 1, 78 },
        /*  5*/ { BARCODE_PHARMA, -1, "1", 7, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_PHARMA_TWO, -1, "64570080", 8, 0, 2, 31 },
        /*  7*/ { BARCODE_PHARMA_TWO, -1, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_CODE32, -1, "1", 8, 0, 1, 103 },
        /*  9*/ { BARCODE_CODE32, -1, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[64];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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

    struct item {
        int symbology;
        int option_2;
        char *data;

        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODABAR, -1, "A1234B", "A1234B" },
        /*  1*/ { BARCODE_CODABAR, -1, "a1234c", "A1234C" }, // Converts to upper
        /*  2*/ { BARCODE_CODABAR, 1, "A1234B", "A1234B" }, // Check not included
        /*  3*/ { BARCODE_PHARMA, -1, "123456", "" }, // None
        /*  4*/ { BARCODE_PHARMA_TWO, -1, "123456", "" }, // None
        /*  5*/ { BARCODE_CODE32, -1, "123456", "A001234564" },
        /*  6*/ { BARCODE_CODE32, -1, "12345678", "A123456788" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_hrt");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int debug) {

    struct item {
        int symbology;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODABAR, "A1234B", 0, 1, 62, "" },
        /*  1*/ { BARCODE_CODABAR, "1234B", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 358: Does not begin with \"A\", \"B\", \"C\" or \"D\"" },
        /*  2*/ { BARCODE_CODABAR, "A1234", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 359: Does not end with \"A\", \"B\", \"C\" or \"D\"" },
        /*  3*/ { BARCODE_CODABAR, "A1234E", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 359: Does not end with \"A\", \"B\", \"C\" or \"D\"" },
        /*  4*/ { BARCODE_CODABAR, "C123.D", 0, 1, 63, "" },
        /*  5*/ { BARCODE_CODABAR, "C123,D", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 357: Invalid character in data (\"0123456789-$:/.+ABCD\" only)" },
        /*  6*/ { BARCODE_CODABAR, "D:C", 0, 1, 33, "" },
        /*  7*/ { BARCODE_CODABAR, "DCC", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 363: Cannot contain \"A\", \"B\", \"C\" or \"D\"" },
        /*  8*/ { BARCODE_CODABAR, "AB", ZINT_ERROR_TOO_LONG, -1, -1, "Error 362: Input too short (3 character minimum)" },
        /*  9*/ { BARCODE_PHARMA, "131070", 0, 1, 78, "" },
        /* 10*/ { BARCODE_PHARMA, "131071", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Data out of range (3 to 131070)" },
        /* 11*/ { BARCODE_PHARMA, "3", 0, 1, 4, "" },
        /* 12*/ { BARCODE_PHARMA, "2", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Data out of range (3 to 131070)" },
        /* 13*/ { BARCODE_PHARMA, "1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Data out of range (3 to 131070)" },
        /* 14*/ { BARCODE_PHARMA, "12A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 351: Invalid character in data (digits only)" },
        /* 15*/ { BARCODE_PHARMA_TWO, "64570080", 0, 2, 31, "" },
        /* 16*/ { BARCODE_PHARMA_TWO, "64570081", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Data out of range (4 to 64570080)" },
        /* 17*/ { BARCODE_PHARMA_TWO, "4", 0, 2, 3, "" },
        /* 18*/ { BARCODE_PHARMA_TWO, "3", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Data out of range (4 to 64570080)" },
        /* 19*/ { BARCODE_PHARMA_TWO, "2", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Data out of range (4 to 64570080)" },
        /* 20*/ { BARCODE_PHARMA_TWO, "1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Data out of range (4 to 64570080)" },
        /* 21*/ { BARCODE_PHARMA_TWO, "123A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 355: Invalid character in data (digits only)" },
        /* 22*/ { BARCODE_CODE32, "12345678", 0, 1, 103, "" },
        /* 23*/ { BARCODE_CODE32, "9", 0, 1, 103, "" },
        /* 24*/ { BARCODE_CODE32, "0", 0, 1, 103, "" },
        /* 25*/ { BARCODE_CODE32, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 361: Invalid character in data (digits only)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d symbol->errtxt %s != %s\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        int symbology;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODABAR, -1, "A37859B", 0, 1, 72, "BS EN 798:1995 Figure 1",
                    "101100100101100101010100101101010011010101101010010110100101010010010110"
                },
        /*  1*/ { BARCODE_CODABAR, -1, "A0123456789-$:/.+D", 0, 1, 186, "Verified manually against tec-it",
                    "101100100101010100110101011001010100101101100101010101101001011010100101001010110100101101010011010101101001010101001101010110010101101011011011011010110110110110101011011011010100110010"
                },
        /*  2*/ { BARCODE_CODABAR, 1, "A1B", 0, 1, 43, "Verified manually against tec-it",
                    "1011001001010101100101101101101010010010110"
                },
        /*  3*/ { BARCODE_CODABAR, 1, "A+B", 0, 1, 43, "Verified manually against tec-it",
                    "1011001001010110110110101010011010010010110"
                },
        /*  4*/ { BARCODE_CODABAR, 1, "B0123456789-$:/.+B", 0, 1, 196, "Verified manually against tec-it",
                    "1001001011010101001101010110010101001011011001010101011010010110101001010010101101001011010100110101011010010101010011010101100101011010110110110110101101101101101010110110110100101011010010010110"
                },
        /*  5*/ { BARCODE_PHARMA, -1, "131070", 0, 1, 78, "",
                    "111001110011100111001110011100111001110011100111001110011100111001110011100111"
                },
        /*  6*/ { BARCODE_PHARMA, -1, "123456", 0, 1, 58, "",
                    "1110011100111001001001001110010010011100100100100100100111"
                },
        /*  7*/ { BARCODE_PHARMA_TWO, -1, "64570080", 0, 2, 31, "Verified manually against tec-it",
                    "1010101010101010101010101010101"
                    "1010101010101010101010101010101"
                },
        /*  8*/ { BARCODE_PHARMA_TWO, -1, "29876543", 0, 2, 31, "Verified manually against tec-it",
                    "0010100010001010001010001000101"
                    "1000101010100000100000101010000"
                },
        /*  9*/ { BARCODE_CODE32, -1, "34567890", 0, 1, 103, "Verified manually against tec-it",
                    "1001011011010101101001011010110010110101011011010010101100101101011010010101101010101100110100101101101"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char bwipp_buf[8192];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    testStart("test_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
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
