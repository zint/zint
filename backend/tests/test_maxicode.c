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

#include "testcommon.h"

static void test_large(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int option_1;
        unsigned char *pattern;
        int length;
        char* primary;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -1, "1", 132, "", 0, 33, 30 }, // 138 according to ISO/IEC 16023:2000 TODO: investigate (see also test_fuzz)
        /*  1*/ { -1, "1", 133, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { -1, "A", 93, "", 0, 33, 30 },
        /*  3*/ { -1, "A", 94, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { -1, "\001", 91, "", 0, 33, 30 },
        /*  5*/ { -1, "\001", 92, "", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { -1, "\200", 91, "", 0, 33, 30 },
        /*  7*/ { -1, "\200", 92, "", ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[256];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        int length = testUtilSetSymbol(symbol, BARCODE_MAXICODE, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);
        strcpy(symbol->primary, data[i].primary);

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

static void test_encode(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        int option_1;
        unsigned char *data;
        char* primary;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { -1, -1, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "", 0, 33, 30, "ISO/IEC 16023:2000 Figure 2",
                    "011111010000001000001000100111"
                    "000100000001000000001010000000"
                    "001011001100100110110010010010"
                    "100000010001100010010000000000"
                    "001011000000101000001010110011"
                    "111010001000001011001000111100"
                    "100000000110000010010000000000"
                    "000010100010010010001001111100"
                    "111011100000001000000110000000"
                    "000000011011000000010100011000"
                    "101111000001010110001100000011"
                    "001110001010000000111010001110"
                    "000111100000000000100001011000"
                    "100010000000000000000111001000"
                    "100000001000000000011000001000"
                    "000010111000000000000010000010"
                    "111000001000000000001000001101"
                    "011000000000000000001000100100"
                    "000000101100000000001001010001"
                    "101010001000000000100111001100"
                    "001000011000000000011100001010"
                    "000000000000000000110000100000"
                    "101011001010100001000101010001"
                    "100011110010101001101010001010"
                    "011010000000000101011010011111"
                    "000001110011111111111100010100"
                    "001110100111000101011000011100"
                    "110111011100100001101001010110"
                    "000001011011101010010111001100"
                    "111000110111100010001111011110"
                    "101111010111111000010110111001"
                    "001001101111101101101010011100"
                    "001011000000111101100100001000"
                },
        /*  1*/ { -1, 4, "MaxiCode (19 chars)", "", 0, 33, 30, "ISO/IEC 16023:2000 Figure H1 **NOT SAME** TODO: investigate",
                    "001101011111011100000010101111"
                    "101100010001001100010000001100"
                    "101100001010001111001001111101"
                    "010101010101010101010101010100"
                    "000000000000000000000000000111"
                    "101010101010101010101010101000"
                    "010101010101010101010101010111"
                    "000000000000000000000000000010"
                    "101010101010101010101010101000"
                    "010101011111111100000001010100"
                    "000000000011110110001000000000"
                    "101010101110000000111010101000"
                    "010101100010000000001101010101"
                    "000000101000000000001000000000"
                    "101010000000000000011010101000"
                    "010101010000000000001101010100"
                    "000000001000000000001000000011"
                    "101010110000000000001010101010"
                    "010101101100000000010101010111"
                    "000000100000000000000000000000"
                    "101010010110000000000110101011"
                    "010101010110000000001001010100"
                    "000000000110001011000000000010"
                    "101010100110111001010010101000"
                    "010101010101010101010000101111"
                    "000000000000000000001100100000"
                    "101010101010101010100101000001"
                    "000011000111010110101100010000"
                    "111001111110111110011000111111"
                    "000001110010000010110001100100"
                    "000111000000001111011000010010"
                    "010110010110001110100000010100"
                    "010011110011000001010111100111"
                },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_MAXICODE, data[i].input_mode, -1 /*eci*/, data[i].option_1, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);
        strcpy(symbol->primary, data[i].primary);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].primary,
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

static void test_best_supported_set(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int ret;
        float w;
        float h;
        int ret_vector;

        int expected_rows;
        int expected_width;
        char *comment;
        unsigned char *expected;
    };
    struct item data[] = {
        /* 0*/ { "am.//ab,\034TA# z\015!", 0, 100, 100, 0, 33, 30, "TODO: Better data and verify expected",
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

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MAXICODE;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%2d*/ { \"%s\", %d, %.0f, %.0f, %d, %d, %d, \"%s\",\n",
                    i, testUtilEscape(data[i].data, length, escaped_data, sizeof(escaped_data)), ret,
                    data[i].w, data[i].h, data[i].ret_vector, symbol->rows, symbol->width, data[i].comment);
            testUtilModulesDump(symbol, "                    ",  "\n");
            printf("                },\n");
        } else {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            int width, row;
            ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
            assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// #181 Nico Gunkel OSS-Fuzz
static void test_fuzz(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { "\223\223\223\223\223\200\000\060\060\020\122\104\060\343\000\000\040\104\104\104\104\177\377\040\000\324\336\000\000\000\000\104\060\060\060\060\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\104\060\104\104\000\000\000\040\104\104\104\104\177\377\377\377\324\336\000\000\000\000\104\377\104\001\104\104\104\104\104\104\233\233\060\060\060\060\060\060\060\060\060\325\074", 107, ZINT_ERROR_TOO_LONG }, // Original OSS-Fuzz triggering data
        /* 1*/ { "AaAaAaAaAaAaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA123456789", -1, ZINT_ERROR_TOO_LONG }, // Add 6 lowercase a's so 6 SHIFTS inserted so 6 + 138 (max input len) = 144 and numbers come at end of buffer
        /* 2*/ { "AaAaAaAaAaAaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA123456789A", -1, ZINT_ERROR_TOO_LONG },
        /* 3*/ { "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", -1, ZINT_ERROR_TOO_LONG }, // First 6 chars ignored for number compaction so max numeric digits appears to be 135 not 138 (for mode 4 anyway) TODO: investigate further
        /* 4*/ { "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345", -1, 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MAXICODE;
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
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_best_supported_set", test_best_supported_set, 1, 1, 1 },
        { "test_fuzz", test_fuzz, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
