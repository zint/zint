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
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, "\177", 30, 0, 1, 528 },
        /*  1*/ { BARCODE_TELEPEN, "\177", 31, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_TELEPEN_NUM, "1", 60, 0, 1, 528 },
        /*  3*/ { BARCODE_TELEPEN_NUM, "1", 61, ZINT_ERROR_TOO_LONG, -1, -1 },
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

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
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

    struct item {
        int symbology;
        char *data;
        int length;

        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, "ABC1234.;$", -1, "ABC1234.;$" },
        /*  1*/ { BARCODE_TELEPEN, "abc1234.;$", -1, "abc1234.;$" },
        /*  2*/ { BARCODE_TELEPEN, "ABC1234\001", -1, "ABC1234\001" },
        /*  3*/ { BARCODE_TELEPEN, "ABC\0001234", 8, "ABC 1234" },
        /*  4*/ { BARCODE_TELEPEN_NUM, "1234", -1, "1234" },
        /*  5*/ { BARCODE_TELEPEN_NUM, "123X", -1, "123X" },
        /*  6*/ { BARCODE_TELEPEN_NUM, "123x", -1, "123X" }, // Converts to upper
        /*  7*/ { BARCODE_TELEPEN_NUM, "12345", -1, "012345" }, // Adds leading zero if odd
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_hrt");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

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
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, " !\"#$%&'()*+,-./0123456789:;<", -1, 0, 1, 512 },
        /*  1*/ { BARCODE_TELEPEN, "AZaz\176\001", -1, 0, 1, 144 },
        /*  2*/ { BARCODE_TELEPEN, "\000\177", 2, 0, 1, 80 },
        /*  3*/ { BARCODE_TELEPEN, "é", -1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  4*/ { BARCODE_TELEPEN_NUM, "1234567890", -1, 0, 1, 128 },
        /*  5*/ { BARCODE_TELEPEN_NUM, "123456789A", -1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  6*/ { BARCODE_TELEPEN_NUM, "123456789X", -1, 0, 1, 128 }, // [0-9]X allowed
        /*  7*/ { BARCODE_TELEPEN_NUM, "12345678X9", -1, ZINT_ERROR_INVALID_DATA, -1, -1 }, // X[0-9] not allowed
        /*  8*/ { BARCODE_TELEPEN_NUM, "1X34567X9X", -1, 0, 1, 128 }, // [0-9]X allowed multiple times
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Telepen Barcode Symbology information and History (BSiH) https://telepen.co.uk/wp-content/uploads/2018/10/Barcode-Symbology-information-and-History.pdf
// E2326U: SB Telepen Barcode Fonts Guide Issue 2 (Apr 2009) https://telepen.co.uk/wp-content/uploads/2018/09/SB-Telepen-Barcode-Fonts-V2.pdf
static void test_encode(int index, int generate, int debug) {

    struct item {
        int symbology;
        char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, "1A", -1, 0, 1, 80, "Telepen BSiH Example, same",
                    "10101010101110001011101000100010101110111011100010100010001110101110001010101010"
                },
        /*  1*/ { BARCODE_TELEPEN, "ABC", -1, 0, 1, 96, "Telepen E2326U Example, same",
                    "101010101011100010111011101110001110001110111000101011101110101011101000101000101110001010101010"
                },
        /*  2*/ { BARCODE_TELEPEN, "RST", -1, 0, 1, 96, "Verified manually against tec-it",
                    "101010101011100011100011100010101010111010111000111010111000101010111000111011101110001010101010"
                },
        /*  3*/ { BARCODE_TELEPEN, "?@", -1, 0, 1, 80, "ASCII count 127, check 0; verified manually against tec-it",
                    "10101010101110001010101010101110111011101110101011101110111011101110001010101010"
                },
        /*  4*/ { BARCODE_TELEPEN, "\000", 1, 0, 1, 64, "Verified manually against tec-it",
                    "1010101010111000111011101110111011101110111011101110001010101010"
                },
        /*  5*/ { BARCODE_TELEPEN_NUM, "1234567890", -1, 0, 1, 128, "Verified manually against tec-it",
                    "10101010101110001010101110101110101000101010001010101110101110001011101010001000101110001010101010101011101010101110001010101010"
                },
        /*  6*/ { BARCODE_TELEPEN_NUM, "123456789", -1, 0, 1, 128, "Verified manually against tec-it (012345679)",
                    "10101010101110001110101010111010111000100010001011101110001110001000101010001010111010100010100010111000101110101110001010101010"
                },
        /*  7*/ { BARCODE_TELEPEN_NUM, "123X", -1, 0, 1, 80, "Verified manually against tec-it",
                    "10101010101110001010101110101110111010111000111011101011101110001110001010101010"
                },
        /*  8*/ { BARCODE_TELEPEN_NUM, "1X3X", -1, 0, 1, 80, "Verified manually against tec-it",
                    "10101010101110001110001110001110111010111000111010111010101110001110001010101010"
                },
        /*  9*/ { BARCODE_TELEPEN_NUM, "3637", -1, 0, 1, 80, "Glyph count 127, check 0; verified manually against tec-it",
                    "10101010101110001010101010101110111011101110101011101110111011101110001010101010"
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

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %d, %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < 5) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

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

// #181 Nico Gunkel OSS-Fuzz
static void test_fuzz(int index, int debug) {

    struct item {
        int symbology;
        char *data;
        int length;
        int ret;
    };
    // Note NULs where using DELs code (16 binary characters wide)
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_TELEPEN, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 30, 0 },
        /* 1*/ { BARCODE_TELEPEN, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 31, ZINT_ERROR_TOO_LONG },
        /* 2*/ { BARCODE_TELEPEN_NUM, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 60, ZINT_ERROR_INVALID_DATA },
        /* 3*/ { BARCODE_TELEPEN_NUM, "040404040404040404040404040404040404040404040404040404040404", 60, 0 },
        /* 4*/ { BARCODE_TELEPEN_NUM, "1234567890123456789012345678901234567890123456789012345678901", 61, ZINT_ERROR_TOO_LONG },
        /* 5*/ { BARCODE_TELEPEN_NUM, "00000000000000000000000000000000000000000000000000000000000X", 60, 0 },
        /* 6*/ { BARCODE_TELEPEN_NUM, "999999999999999999999999999999999999999999999999999999999999", 60, 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_fuzz");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->debug |= debug;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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
