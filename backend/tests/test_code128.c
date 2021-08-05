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

/* Note BARCODE_GS1_128, BARCODE_EAN14, BARCODE_NVE18 also tested in test_gs1.c */

#include "testcommon.h"

static void test_large(int index, int debug) {

    struct item {
        int symbology;
        char *pattern;
        int length;
        int ret;
        int expected_width;
    };
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "A", 60, 0, 695 },
        /*  1*/ { BARCODE_CODE128, "A", 61, ZINT_ERROR_TOO_LONG, -1 },
        /*  2*/ { BARCODE_CODE128, "A", 161, ZINT_ERROR_TOO_LONG, -1 },
        /*  3*/ { BARCODE_CODE128, "\351A", 40, 0, 695 },
        /*  4*/ { BARCODE_CODE128, "\351A", 41, ZINT_ERROR_TOO_LONG, -1 }, // 41 chars (+ 20 shifts)
        /*  5*/ { BARCODE_CODE128, "0", 120, 0, 695 },
        /*  6*/ { BARCODE_CODE128, "0", 121, ZINT_ERROR_TOO_LONG, -1 },
        /*  7*/ { BARCODE_CODE128B, "A", 60, 0, 695 },
        /*  8*/ { BARCODE_CODE128B, "A", 61, ZINT_ERROR_TOO_LONG, -1 },
        /*  9*/ { BARCODE_CODE128B, "0", 60, 0, 695 },
        /* 10*/ { BARCODE_CODE128B, "0", 61, ZINT_ERROR_TOO_LONG, -1 },
        /* 11*/ { BARCODE_GS1_128, "[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890123456789012345678901234567890[92]1234567890", -1, 0, 706 }, // 116 nos + 3 FNC1s
        /* 12*/ { BARCODE_GS1_128, "[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890123456789012345678901234567890[92]1234[93]1234", -1, ZINT_ERROR_TOO_LONG, -1 }, // 116 nos + 4 FNC1s
        /* 13*/ { BARCODE_GS1_128, "A", 161, ZINT_ERROR_TOO_LONG, -1 },
        /* 14*/ { BARCODE_EAN14, "1234567890123", -1, 0, 134 },
        /* 15*/ { BARCODE_EAN14, "12345678901234", -1, ZINT_ERROR_TOO_LONG, -1 },
        /* 16*/ { BARCODE_NVE18, "12345678901234567", -1, 0, 156 },
        /* 17*/ { BARCODE_NVE18, "123456789012345678", -1, ZINT_ERROR_TOO_LONG, -1 },
        /* 18*/ { BARCODE_HIBC_128, "1", 110, 0, 684 },
        /* 19*/ { BARCODE_HIBC_128, "1", 111, ZINT_ERROR_TOO_LONG, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[4096];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].length != -1) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
            assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));
        } else {
            strcpy(data_buf, data[i].pattern);
        }

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int hrt_cpy_iso8859_1(struct zint_symbol *symbol, const unsigned char *source, int source_len);

static void test_hrt_cpy_iso8859_1(int index, int debug) {

    struct item {
        char *data;
        int length;
        int ret;
        char *expected;
        char *comment;
    };
    // NBSP U+00A0 (\240, 160), UTF-8 C2A0 (\302\240)
    // ¡ U+00A1 (\241, 161), UTF-8 C2A1 (\302\241)
    // é U+00E9 (\351, 233), UTF-8 C3A9
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "", -1, 0, "", "" },
        /*  1*/ { "abc", -1, 3, "abc", "" },
        /*  2*/ { "\000A\001B\002\036\037C ~\177", 11, 11, " A B   C ~ ", "" },
        /*  3*/ { "~\177\200\201\237\240", -1, 7, "~    \302\240", "" },
        /*  4*/ { "\241\242\243\244\257\260", -1, 12, "¡¢£¤¯°", "" },
        /*  5*/ { "\276\277\300\337\377", -1, 10, "¾¿Àßÿ", "" },
        /*  6*/ { "\351", -1, 2, "é", "" },
        /*  7*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 126, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "63 \241" },
        /*  8*/ { "a\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 127, "a¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "a + 63 \241" },
        /*  9*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241a", -1, 127, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡a", "63 \241 + a" },
        /* 10*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 126, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "64 \241 (truncated)" },
        /* 11*/ { "a\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 127, "a¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "a + 64 \241 (truncated)" },
        /* 12*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241a", -1, 126, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "64 \241 + a (truncated)" },
        /* 13*/ { "\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241\241", -1, 126, "¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡¡", "65 \241 (truncated)" },
        /* 14*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 126, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "63 \351" },
        /* 15*/ { "a\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 127, "aééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "a + 63 \351" },
        /* 16*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351a", -1, 127, "éééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééa", "63 \351 + a" },
        /* 17*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 126, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "64 \351 (truncated)" },
        /* 18*/ { "a\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 127, "aééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "a + 64 \351 (truncated)" },
        /* 19*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351a", -1, 126, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "64 \351 + a (truncated)" },
        /* 20*/ { "\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351\351", -1, 126, "ééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééééé", "65 \351 (truncated)" },
        /* 21*/ { "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 127, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "128 A (truncated)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};

    testStart("test_hrt_cpy_iso8859_1");

    symbol.debug = debug;

    for (i = 0; i < data_size; i++) {
        int j;

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = hrt_cpy_iso8859_1(&symbol, (unsigned char *) data[i].data, length);
        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            for (j = 0; j < ret; j++) {
                fprintf(stderr, "symbol.text[%d] %2X\n", j, symbol.text[j]);
            }
        }
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_equal(ret, (int) ustrlen(symbol.text), "i:%d ret %d != strlen %d\n", i, ret, (int) ustrlen(symbol.text));
        assert_nonzero(testUtilIsValidUTF8(symbol.text, (int) ustrlen(symbol.text)), "i:%d testUtilIsValidUTF8(%s) != 1\n", i, symbol.text);
        assert_zero(strcmp((char *) symbol.text, data[i].expected), "i:%d symbol.text (%s) != expected (%s)\n", i, symbol.text, data[i].expected);
    }

    testFinish();
}

static void test_hrt(int index, int debug) {

    struct item {
        int symbology;
        int input_mode;
        char *data;
        int length;

        char *expected;
    };
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, "1234567890", -1, "1234567890" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, "\000ABC\000DEF\000", 9, " ABC DEF " },
        /*  2*/ { BARCODE_CODE128B, UNICODE_MODE, "12345\00067890", 11, "12345 67890" },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, "12345\01167890\037\177", -1, "12345 67890  " },
        /*  4*/ { BARCODE_CODE128, UNICODE_MODE, "abcdé", -1, "abcdé" },
        /*  5*/ { BARCODE_CODE128, DATA_MODE, "abcd\351", -1, "abcdé" },
        /*  6*/ { BARCODE_CODE128B, UNICODE_MODE, "abcdé", -1, "abcdé" },
        /*  7*/ { BARCODE_CODE128B, DATA_MODE, "abcd\351", -1, "abcdé" },
        /*  8*/ { BARCODE_HIBC_128, UNICODE_MODE, "1234567890", -1, "*+12345678900*" },
        /*  9*/ { BARCODE_HIBC_128, UNICODE_MODE, "a99912345", -1, "*+A999123457*" }, // Converts to upper
        // BARCODE_GS1_128, BARCODE_EAN14, BARCODE_NVE18 hrt tested in test_gs1.c
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_hrt");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int output_options;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, READER_INIT, "A", 0, 1, 57, "(5) 104 96 33 60 106", "StartA FNC3 A" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, READER_INIT, "12", 0, 1, 68, "(6) 104 96 99 12 22 106", "StartB FNC3 CodeC 12" },
        /*  2*/ { BARCODE_CODE128B, UNICODE_MODE, READER_INIT, "\0371234", 0, 1, 101, "(9) 103 96 95 17 18 19 20 6 106", "StartA FNC3 US 1 2 3 4" },
        /*  3*/ { BARCODE_GS1_128, GS1_MODE, READER_INIT, "[90]12", 0, 1, 68, "(6) 105 102 90 12 11 106", "StartC FNC1 90 12 (Reader Initialise not supported by GS1 barcodes (use CODE128))" },
        /*  4*/ { BARCODE_EAN14, GS1_MODE, READER_INIT, "12", 0, 1, 134, "(12) 105 102 1 0 0 0 0 0 1 23 12 106", "StartC FNC1 01 00 (5) 01 23 (Reader Initialise not supported by GS1 barcodes (use CODE128))" },
        /*  5*/ { BARCODE_NVE18, GS1_MODE, READER_INIT, "12", 0, 1, 156, "(14) 105 102 0 0 0 0 0 0 0 0 1 23 58 106", "StartC FNC1 00 (8) 01 23 (Reader Initialise not supported by GS1 barcodes (use CODE128))" },
        /*  6*/ { BARCODE_HIBC_128, UNICODE_MODE, READER_INIT, "A", 0, 1, 79, "(7) 104 96 11 33 24 5 106", "StartA FNC3 + A 8 (check) (Not sensible, use CODE128)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_reader_init");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int generate, int debug) {

    struct item {
        int input_mode;
        char *data;
        int length;
        int ret;
        int expected_width;
        char *expected;
        char *comment;
    };
    // NUL U+0000, CodeA-only
    // US U+001F (\037, 31), CodeA-only
    // a U+0061 (\141, 97), CodeB-only
    // b U+0062 (\142, 98), CodeB-only
    // PAD U+0080 (\200, 128), UTF-8 C280 (\302\200), CodeA-only extended ASCII, not in ISO 8859-1
    // APC U+009F (\237, 159), UTF-8 C29F, CodeA-only extended ASCII, not in ISO 8859-1
    // ß U+00DF (\337, 223), UTF-8 C39F, CodeA and CodeB extended ASCII
    // é U+00E9 (\351, 233), UTF-8 C3A9, CodeB-only extended ASCII
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "\302\200", -1, ZINT_ERROR_INVALID_DATA, 0, "Error 204: Invalid character in input data (ISO/IEC 8859-1 only)", "PAD not in ISO 8859-1" },
        /*  1*/ { DATA_MODE, "\200", -1, 0, 57, "(5) 103 101 64 23 106", "PAD ok using binary" },
        /*  2*/ { UNICODE_MODE, "AIM1234", -1, 0, 101, "(9) 104 33 41 45 99 12 34 87 106", "Example from Annex A.1, check char value 87" },
        /*  3*/ { GS1_MODE, "[90]12", -1, ZINT_ERROR_INVALID_OPTION, 0, "Error 220: Selected symbology does not support GS1 mode", "" },
        /*  4*/ { UNICODE_MODE, "1", -1, 0, 46, "(4) 104 17 18 106", "StartB 1" },
        /*  5*/ { UNICODE_MODE, "12", -1, 0, 46, "(4) 105 12 14 106", "StartC 12" },
        /*  6*/ { UNICODE_MODE, "123", -1, 0, 68, "(6) 104 17 18 19 8 106", "StartB 1 2 3" },
        /*  7*/ { UNICODE_MODE, "1234", -1, 0, 57, "(5) 105 12 34 82 106", "StartC 12 34" },
        /*  8*/ { UNICODE_MODE, "12345", -1, 0, 79, "(7) 105 12 34 100 21 54 106", "StartC 12 34 CodeB 5" },
        /*  9*/ { UNICODE_MODE, "\037", -1, 0, 46, "(4) 103 95 95 106", "StartA US" },
        /* 10*/ { UNICODE_MODE, "1\037", -1, 0, 57, "(5) 103 17 95 1 106", "StartA 1 US" },
        /* 11*/ { UNICODE_MODE, "12\037", -1, 0, 68, "(6) 103 17 18 95 29 106", "StartA 1 2 US" },
        /* 12*/ { UNICODE_MODE, "a\037a", -1, 0, 79, "(7) 104 65 98 95 65 86 106", "StartB a Shift US a" },
        /* 13*/ { UNICODE_MODE, "1234\037a", -1, 0, 101, "(9) 105 12 34 101 95 98 65 100 106", "StartC 12 34 CodeA US Shift a" },
        /* 14*/ { UNICODE_MODE, "\037AAa\037", -1, 0, 101, "(9) 103 95 33 33 98 65 95 2 106", "StartA US A A Shift a US" },
        /* 15*/ { UNICODE_MODE, "\037AAaa\037", -1, 0, 123, "(11) 103 95 33 33 100 65 65 98 95 40 106", "StartA US A A CodeB a a Shift US" },
        /* 16*/ { UNICODE_MODE, "AAAa12345aAA", -1, 0, 167, "(15) 104 33 33 33 65 17 99 23 45 100 65 33 33 54 106", "StartB A (3) a 1 CodeC 23 45 CodeB a A A" },
        /* 17*/ { UNICODE_MODE, "a\037Aa\037\037a\037aa\037a", -1, 0, 222, "(20) 104 65 98 95 33 65 101 95 95 98 65 95 100 65 65 98 95 65 96 106", "StartB a Shift US A a CodeA US US Shift a US CodeB a a Shift US a" },
        /* 18*/ { UNICODE_MODE, "\000\037ß", 4, 0, 79, "(7) 103 64 95 101 63 88 106", "StartA NUL US FNC4 ß" },
        /* 19*/ { UNICODE_MODE, "\000\037é", 4, 0, 90, "(8) 103 64 95 101 98 73 78 106", "StartA NUL US FNC4 Shift é" },
        /* 20*/ { UNICODE_MODE, "\000\037éa", 5, 0, 101, "(9) 103 64 95 100 100 73 65 61 106", "StartA NUL US LatchB FNC4 é a" },
        /* 21*/ { UNICODE_MODE, "abß", -1, 0, 79, "(7) 104 65 66 100 63 29 106", "StartB a b FNC4 ß" },
        /* 22*/ { DATA_MODE, "\141\142\237", -1, 0, 90, "(8) 104 65 66 100 98 95 26 106", "StartB a b FNC4 Shift APC" },
        /* 23*/ { DATA_MODE, "\141\142\237\037", -1, 0, 101, "(9) 104 65 66 101 101 95 95 96 106", "StartB a b LatchA FNC4 APC US" },
        /* 24*/ { UNICODE_MODE, "ééé", -1, 0, 90, "(8) 104 100 100 73 73 73 44 106", "StartB LatchFNC4 é é é" },
        /* 25*/ { UNICODE_MODE, "aééééb", -1, 0, 145, "(13) 104 65 100 73 100 73 100 73 100 73 66 49 106", "StartB a FNC4 é (4) b" },
        /* 26*/ { UNICODE_MODE, "aéééééb", -1, 0, 145, "(13) 104 65 100 100 73 73 73 73 73 100 66 93 106", "StartB a Latch é (5) Shift b" },
        /* 27*/ { UNICODE_MODE, "aééééébc", -1, 0, 167, "(15) 104 65 100 100 73 73 73 73 73 100 66 100 67 40 106", "StartB a Latch é (5) Shift b Shift c" },
        /* 28*/ { UNICODE_MODE, "aééééébcd", -1, 0, 178, "(16) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 66 106", "StartB a Latch é (5) Unlatch b c d" },
        /* 29*/ { UNICODE_MODE, "aééééébcde", -1, 0, 189, "(17) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 69 2 106", "StartB a Latch é (5) Unlatch b c d e" },
        /* 30*/ { UNICODE_MODE, "aééééébcdeé", -1, 0, 211, "(19) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 69 100 73 95 106", "StartB a Latch é (5) Unlatch b c d e FNC4 é" },
        /* 31*/ { UNICODE_MODE, "aééééébcdeéé", -1, 0, 233, "(21) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 69 100 73 100 73 19 106", "StartB a Latch é (5) Unlatch b c d e FNC4 é (2)" },
        /* 32*/ { UNICODE_MODE, "aééééébcdeééé", -1, 0, 244, "(22) 104 65 100 100 73 73 73 73 73 100 66 100 67 100 68 100 69 73 73 73 83 106", "StartB a Latch é (5) Shift b Shift c Shift d Shift e é (3)" },
        /* 33*/ { UNICODE_MODE, "aééééébcdefééé", -1, 0, 255, "(23) 104 65 100 100 73 73 73 73 73 100 100 66 67 68 69 70 100 100 73 73 73 67 106", "StartB a Latch é (5) Unlatch b c d e f Latch é (3)" },
        /* 34*/ { DATA_MODE, "\200\200\200\200\200\101\060\060\060\060\101\200", -1, 0, 222, "(20) 103 101 101 64 64 64 64 64 101 101 33 99 0 0 101 33 101 64 73 106", "StartA Latch PAD (4) Unlatch A CodeC 00 00 CodeA A FNC4 PAD" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_CODE128, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %d, %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_ean128_input(int index, int generate, int debug) {

    struct item {
        int input_mode;
        char *data;
        int ret;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { GS1_MODE, "[90]1[90]1", 0, 123, "(11) 105 102 90 100 17 102 25 99 1 56 106", "StartC FNC1 90 CodeB 1 FNC1 9" },
        /*  1*/ { GS1_MODE | GS1PARENS_MODE, "(90)1(90)1", 0, 123, "(11) 105 102 90 100 17 102 25 99 1 56 106", "StartC FNC1 90 CodeB 1 FNC1 9" },
        /*  2*/ { GS1_MODE, "[90]1[90]12", 0, 123, "(11) 105 102 90 100 17 99 102 90 12 13 106", "StartC FNC1 90 CodeB 1 CodeC FNC1 90 12" },
        /*  3*/ { GS1_MODE, "[90]1[90]123", 0, 134, "(12) 105 102 90 100 17 102 25 99 1 23 57 106", "StartC FNC1 90 CodeB 1 FNC1 9 CodeC 01 23" },
        /*  4*/ { GS1_MODE, "[90]12[90]1", 0, 123, "(11) 105 102 90 12 102 100 25 99 1 19 106", "StartC FNC1 90 12 FNC1 CodeB 9 CodeC 01" },
        /*  5*/ { GS1_MODE, "[90]12[90]12", 0, 101, "(9) 105 102 90 12 102 90 12 14 106", "StartC FNC1 90 12 FNC1 90 12" },
        /*  6*/ { GS1_MODE, "[90]12[90]123", 0, 134, "(12) 105 102 90 12 102 100 25 99 1 23 20 106", "StartC FNC1 90 12 FNC1 CodeB 9 CodeC 01 23" },
        /*  7*/ { GS1_MODE, "[90]123[90]1", 0, 134, "(12) 105 102 90 12 100 19 102 25 99 1 34 106", "StartC FNC1 90 12 CodeB 3 FNC1 9 CodeC 01" },
        /*  8*/ { GS1_MODE, "[90]123[90]1234", 0, 145, "(13) 105 102 90 12 100 19 99 102 90 12 34 98 106", "StartC FNC1 90 12 CodeB 3 CodeC FNC1 90 12 34" },
        /*  9*/ { GS1_MODE, "[90]1[90]1[90]1", 0, 178, "(16) 105 102 90 100 17 102 25 99 1 102 100 25 99 1 51 106", "StartC FNC1 90 CodeB 1 FNC1 9 CodeC 01 FNC1 CodeB 9 CodeC 01" },
        /* 10*/ { GS1_MODE, "[90]1[90]12[90]1", 0, 178, "(16) 105 102 90 100 17 99 102 90 12 102 100 25 99 1 8 106", "StartC FNC1 90 CodeB 1 CodeC FNC1 90 12 FNC1 CodeB 9 CodeC 01" },
        /* 11*/ { GS1_MODE, "[90]1[90]123[90]1", 0, 189, "(17) 105 102 90 100 17 102 25 99 1 23 102 100 25 99 1 70 106", "StartC FNC1 90 CodeB 1 FNC1 9 CodeC 01 23 FNC1 CodeB 9 CodeC 01" },
        /* 12*/ { GS1_MODE, "[90]12[90]123[90]1", 0, 189, "(17) 105 102 90 12 102 100 25 99 1 23 102 100 25 99 1 33 106", "StartC FNC1 90 12 FNC1 CodeB 9 CodeC 01 23 FNC1 CodeB 9 CodeC 01" },
        /* 13*/ { GS1_MODE, "[90]12[90]123[90]12", 0, 167, "(15) 105 102 90 12 102 100 25 99 1 23 102 90 12 11 106", "StartC FNC1 90 12 FNC1 CodeB 9 CodeC 01 23 FNC1 90 12" },
        /* 14*/ { GS1_MODE, "[90]123[90]1[90]1", 0, 189, "(17) 105 102 90 12 100 19 102 25 99 1 102 100 25 99 1 47 106", "StartC FNC1 90 12 CodeB 3 FNC1 9 CodeC 01 FNC1 CodeB 9 CodeC 01" },
        /* 15*/ { GS1_MODE, "[90]123[90]12[90]1", 0, 189, "(17) 105 102 90 12 100 19 99 102 90 12 102 100 25 99 1 80 106", "StartC FNC1 90 12 CodeB 3 CodeC FNC1 90 12 FNC1 CodeB 9 CodeC 01" },
        /* 16*/ { GS1_MODE, "[90]123[90]123[90]12", 0, 178, "(16) 105 102 90 12 100 19 102 25 99 1 23 102 90 12 47 106", "StartC FNC1 90 12 CodeB 3 FNC1 9 CodeC 01 23 FNC1 90 12" },
        /* 17*/ { GS1_MODE, "[90]123[90]1234[90]1", 0, 200, "(18) 105 102 90 12 100 19 99 102 90 12 34 102 100 25 99 1 26 106", "StartC FNC1 90 12 CodeB 3 CodeC FNC1 90 12 34 FNC1 CodeB 9 CodeC 01" },
        /* 18*/ { GS1_MODE, "[90]123[90]1234[90]123", 0, 211, "(19) 105 102 90 12 100 19 99 102 90 12 34 102 100 25 99 1 23 85 106", "StartC FNC1 90 12 CodeB 3 CodeC FNC1 90 12 34 FNC1 CodeB 9 CodeC 01 23" },
        /* 19*/ { GS1_MODE, "[90]12345[90]1234[90]1", 0, 211, "(19) 105 102 90 12 34 100 21 99 102 90 12 34 102 100 25 99 1 30 106", "StartC FNC1 90 12 34 CodeB 5 CodeC FNC1 90 12 34 FNC1 CodeB 9 CodeC 01" },
        /* 20*/ { GS1_MODE, "[90]1A[90]1", 0, 134, "(12) 104 102 25 16 17 33 102 25 99 1 65 106", "StartB FNC1 9 0 1 A FNC1 9 CodeC 01" },
        /* 21*/ { GS1_MODE, "[90]12A[90]123", 0, 145, "(13) 105 102 90 12 100 33 102 25 99 1 23 25 106", "StartC FNC1 90 12 CodeB A FNC1 9 CodeC 01 23" },
        /* 22*/ { GS1_MODE, "[90]123[90]A234[90]123", 0, 244, "(22) 105 102 90 12 100 19 99 102 90 100 33 18 99 34 102 100 25 99 1 23 37 106", "StartC FNC1 90 12 CodeB 3 CodeC FNC1 90 CodeB A 2 CodeC 34 FNC1 CodeB 9 CodeC 01 23" },
        /* 23*/ { GS1_MODE, "[90]12345A12345A", 0, 178, "(16) 105 102 90 12 34 100 21 33 17 99 23 45 100 33 59 106", "StartC FNC1 90 12 34 CodeB 5 A 1 CodeC 23 45 CodeB A [" },
        /* 24*/ { GS1_MODE, "[01]12345678901231[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890123456789012345678901234", 0, 684, "(62) 105 102 1 12 34 56 78 90 12 31 90 12 34 56 78 90 12 34 56 78 90 12 34 56 78 90 102 91", "Max length" },
        /* 25*/ { GS1_MODE, "[01]12345678901231[90]123456789012345678901234567890[91]12345678901234567890123456789012345678901234567890123456789012345", ZINT_ERROR_TOO_LONG, 0, "Error 344: Input too long (60 symbol character maximum)", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_ean128_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_GS1_128, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_hibc_input(int index, int generate, int debug) {

    struct item {
        char *data;
        int ret;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { ",", ZINT_ERROR_INVALID_DATA, -1, "Error 203: Invalid character in data (alphanumerics, space and \"-.$/+%\" only)", "" },
        /*  1*/ { "A99912345/$$52001510X3", 0, 255, "(23) 104 11 33 99 99 91 23 45 100 15 4 4 99 52 0 15 10 100 56 19 19 53 106", "Check digit 3" },
        /*  2*/ { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%", 0, 497, "(45) 104 11 99 1 23 45 67 89 100 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51", "Check digit +" },
        /*  3*/ { "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", 0, 695, "(63) 104 11 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5", "Check digit Q" },
        /*  4*/ { "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", ZINT_ERROR_TOO_LONG, -1, "Error 341: Input too long (60 symbol character maximum)", "" },
        /*  5*/ { "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0, 684, "(62) 104 11 99 10 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0", "Check digit %" },
        /*  6*/ { "09AZ-.19AZ-.29AZ-.39AZ-.49AZ-.59AZ-.69AZ-.79AZ-.89AZ-.99AZ", 0, 695, "(63) 104 11 16 25 33 58 13 14 17 25 33 58 13 14 18 25 33 58 13 14 19 25 33 58 13 14 20 25", "Check digit -" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_hibc_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_HIBC_128, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_ean14_input(int index, int generate, int debug) {

    struct item {
        char *data;
        int ret;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { "12345678901234", ZINT_ERROR_TOO_LONG, -1, "Error 347: Input too long (13 character maximum)", "" },
        /*  1*/ { "A", ZINT_ERROR_INVALID_DATA, -1, "Error 348: Invalid character in data (digits only)", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_ean14_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_EAN14, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_dpd_input(int index, int generate, int debug) {

    struct item {
        char *data;
        int ret;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { "123456789012345678901234567", ZINT_ERROR_TOO_LONG, -1, "Error 349: DPD input wrong length (28 characters required)", "" },
        /*  1*/ { "12345678901234567890123456789", ZINT_ERROR_TOO_LONG, -1, "Error 349: DPD input wrong length (28 characters required)", "" },
        /*  2*/ { "123456789012345678901234567,", ZINT_ERROR_INVALID_DATA, -1, "Error 300: Invalid character in DPD data (alphanumerics only)", "Alphanumerics only in body" },
        /*  3*/ { ",234567890123456789012345678", 0, 211, "(19) 104 12 18 99 34 56 78 90 12 34 56 78 90 12 34 56 78 64 106", "Non-alphanumeric DPD Identifier (Barcode ID) allowed" },
        /*  4*/ { "\037234567890123456789012345678", ZINT_ERROR_INVALID_DATA, -1, "Error 301: Invalid DPD identifier (first character), ASCII values 32 to 127 only", "Control char <US> as DPD Identifier" },
        /*  5*/ { "é234567890123456789012345678", ZINT_ERROR_INVALID_DATA, -1, "Error 301: Invalid DPD identifier (first character), ASCII values 32 to 127 only", "Extended ASCII as DPD Identifier" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_dpd_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_DPD, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    // BARCODE_GS1_128 examples verified manually against GS1 General Specifications 21.0.1 (GGS)
    // BARCODE_DPD examples Specification DPD and primetime Parcel Despatch (DPDAPPD) Version 4.0.2
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, "AIM", 0, 1, 68, 1, "ISO/IEC 15417:2007 Figure 1",
                    "11010010000101000110001100010001010111011000101110110001100011101011"
                },
        /*  1*/ { BARCODE_CODE128B, UNICODE_MODE, "AIM", 0, 1, 68, 1, "128B same",
                    "11010010000101000110001100010001010111011000101110110001100011101011"
                },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, "1234567890", 0, 1, 90, 1, "",
                    "110100111001011001110010001011000111000101101100001010011011110110100111100101100011101011"
                },
        /*  3*/ { BARCODE_CODE128B, UNICODE_MODE, "1234567890", 0, 1, 145, 1, "",
                    "1101001000010011100110110011100101100101110011001001110110111001001100111010011101101110111010011001110010110010011101100101000110001100011101011"
                },
        /*  4*/ { BARCODE_CODE128, DATA_MODE, "\101\102\103\104\105\106\200\200\200\200\200", 0, 1, 178, 1, "",
                    "1101000010010100011000100010110001000100011010110001000100011010001000110001011101011110111010111101010000110010100001100101000011001010000110010100001100110110110001100011101011"
                },
        /*  5*/ { BARCODE_GS1_128, GS1_MODE, "[8018]950110153123456781", 0, 1, 167, 1, "GGS Figure 2.5.2-1",
                    "11010011100111101011101010011110011001110010101111010001100110110011001000100101110011001101100011011101101110101110110001000010110010010111100101111001001100011101011"
                },
        /*  6*/ { BARCODE_GS1_128, GS1_MODE, "[415]5412345678908[3911]710125", 0, 1, 189, 1, "GGS Figure 2.6.6-1 top",
                    "110100111001111010111011000100010111010001101100010001011101101110101110110001000010110011011011110100011001001101000100011000100100100110100001100110110011100101100100001001101100011101011"
                },
        /*  7*/ { BARCODE_GS1_128, GS1_MODE, "[12]010425[8020]ABC123", 0, 1, 189, 1, "GGS Figure 2.6.6-1 bottom",
                    "110100111001111010111010110011100110011011001001000110011100101100101001111001100100111010111101110101000110001000101100010001000110100111001101100111001011001011100110010111001100011101011"
                },
        /*  8*/ { BARCODE_GS1_128, GS1_MODE, "[253]950110153005812345678901", 0, 1, 211, 1, "GGS Figure 2.6.9-1",
                    "1101001110011110101110111001011001101000100011000101110110001001001100110110011011101110110110011001110110001010110011100100010110001110001011011000010100110111101101011110111010011100110101110110001100011101011"
                },
        /*  9*/ { BARCODE_GS1_128, GS1_MODE, "[253]950110153006567890543210987", 0, 1, 211, 1, "GGS Figure 2.6.9-2",
                    "1101001110011110101110111001011001101000100011000101110110001001001100110110011011101110110110011001001011000010000101100110110111101000100110010110001110110111001001100100100011110010100101110011001100011101011"
                },
        /* 10*/ { BARCODE_GS1_128, GS1_MODE, "[253]95011015300657654321", 0, 1, 189, 1, "GGS Figure 2.6.9-3",
                    "110100111001111010111011100101100110100010001100010111011000100100110011011001101110111011011001100100101100001100101000011101011000110001101101011110111010011100110111001001101100011101011"
                },
        /* 11*/ { BARCODE_GS1_128, GS1_MODE, "[253]9501101530065123456", 0, 1, 167, 1, "GGS Figure 2.6.9-4",
                    "11010011100111101011101110010110011010001000110001011101100010010011001101100110111011101101100110010010110000101100111001000101100011100010110100011110101100011101011"
                },
        /* 12*/ { BARCODE_GS1_128, GS1_MODE, "[01]10857674002017[10]1152KMB", 0, 1, 211, 1, "GGS Figure 4.15.1-1",
                    "1101001110011110101110110011011001100100010010011110010110010100001000011001011011001100110010011101001110011011001000100110001001001101110001010111101110101100011101011101100010001011000100111001101100011101011"
                },
        /* 13*/ { BARCODE_GS1_128, GS1_MODE, "[01]09501101530003", 0, 1, 134, 1, "GGS Figure 5.1-3",
                    "11010011100111101011101100110110011001001000110001011101100010010011001101100110111011101101100110010010011000100110100001100011101011"
                },
        /* 14*/ { BARCODE_GS1_128, GS1_MODE, "[00]395123451234567895", 0, 1, 156, 1, "GGS Figure 5.4.2-1",
                    "110100111001111010111011011001100110100010001101110100011101101110101110110001011001110010001011000111000101101100001010010111101000101111000101100011101011"
                },
        /* 15*/ { BARCODE_GS1_128, GS1_MODE, "[00]006141411234567890", 0, 1, 156, 1, "GGS Figure 6.6.5-1. (and Figures 6.6.5-3 bottom, 6.6.5-4 bottom)",
                    "110100111001111010111011011001100110110011001100100001011000100010110001000101011001110010001011000111000101101100001010011011110110110110110001100011101011"
                },
        /* 16*/ { BARCODE_GS1_128, GS1_MODE, "[403]402621[401]950110153B01001", 0, 1, 266, 0, "GGS Figure 6.6.5-2 top **NOT SAME**, different encodation for zint, BWIPP & standard, same codeword count",
                    "11010011100111101011101100010100010001011000110011001101111000101010111101110100111001101011101111011110101110110001010001100101110011000101110110001001001100110110011011101110101111011101000101100010011101100101110111101100100010011001101100101001111001100011101011"
                },
        /* 17*/ { BARCODE_GS1_128, GS1_MODE, "[00]395011015300000011", 0, 1, 156, 1, "GGS Figure 6.6.5-2 bottom",
                    "110100111001111010111011011001100110100010001100010111011000100100110011011001101110111011011001100110110011001101100110011000100100100011101101100011101011"
                },
        /* 18*/ { BARCODE_GS1_128, GS1_MODE, "[420]45458", 0, 1, 90, 1, "GGS Figure 6.6.5-3 top",
                    "110100111001111010111010110111000100100011001110101100011101100010111100100101100011101011"
                },
        /* 19*/ { BARCODE_GS1_128, GS1_MODE, "[02]00614141000418[15]210228[10]451214[37]20", 0, 1, 255, 1, "GGS Figure 6.6.5-4 top",
                    "110100111001111010111011001100110110110011001100100001011000100010110001000101101100110010010001100110011100101011100110011011100100110011001101110011010011001000100101110110001011001110010011001110111101011101000110100011001001110100011110101100011101011"
                },
        /* 20*/ { BARCODE_GS1_128, GS1_MODE, "[420]87109", 0, 1, 90, 1, "GGS Figure 6.6.5-5 top",
                    "110100111001111010111010110111000100011001001001101000011001001000111001001101100011101011"
                },
        /* 21*/ { BARCODE_GS1_128, GS1_MODE, "[90]1528", 0, 1, 79, 1, "GGS Figure 6.6.5-5 middle",
                    "1101001110011110101110110111101101011100110011100110100111001100101100011101011"
                },
        /* 22*/ { BARCODE_GS1_128, GS1_MODE, "[00]000521775138957172", 0, 1, 156, 1, "GGS Figure 6.6.5-5 bottom",
                    "110100111001111010111011011001100110110011001000100110011011100100111101110101101110100010001100010101111010001001101000010011000010110011011001100011101011"
                },
        /* 23*/ { BARCODE_GS1_128, GS1_MODE, "[00]395011010013000129", 0, 1, 156, 1, "GGS Figure 6.6.5-6",
                    "110100111001111010111011011001100110100010001100010111011000100100110011011001101100110010011011100110110011001100110110011100110010111101101101100011101011"
                },
        /* 24*/ { BARCODE_GS1_128, GS1_MODE, "[00]395011010013000129", 0, 1, 156, 1, "GGS Figure 6.6.5-6",
                    "110100111001111010111011011001100110100010001100010111011000100100110011011001101100110010011011100110110011001100110110011100110010111101101101100011101011"
                },
        /* 25*/ { BARCODE_GS1_128, GS1_MODE, "[401]931234518430GR", 0, 1, 167, 1, "GGS Figure 6.6.5-7 top",
                    "11010011100111101011101100010100011001011100110110001101110110111010111011000110011100101011000111010111101110100111011001101000100011000101110100110111001100011101011"
                },
        /* 26*/ { BARCODE_GS1_128, GS1_MODE, "[00]093123450000000012", 0, 1, 156, 1, "GGS Figure 6.6.5-7 bottom",
                    "110100111001111010111011011001100110010010001101100011011101101110101110110001101100110011011001100110110011001101100110010110011100110111010001100011101011"
                },
        /* 27*/ { BARCODE_GS1_128, GS1_MODE, "[01]95012345678903", 0, 1, 134, 1, "GGS Figure 7.8.5.1-1 1st",
                    "11010011100111101011101100110110010111101000110011011001110110111010111011000100001011001101101111010010011000110110001101100011101011"
                },
        /* 28*/ { BARCODE_GS1_128, GS1_MODE, "[3102]000400", 0, 1, 101, 1, "GGS Figure 7.8.5.1-1 2nd",
                    "11010011100111101011101101100011011001100110110110011001001000110011011001100110110111101100011101011"
                },
        /* 29*/ { BARCODE_GS1_128, GS1_MODE, "[01]95012345678903[3102]000400", 0, 1, 189, 1, "GGS Figure 7.8.5.1-2",
                    "110100111001111010111011001101100101111010001100110110011101101110101110110001000010110011011011110100100110001101100011011001100110110110011001001000110011011001100100100110001100011101011"
                },
        /* 30*/ { BARCODE_GS1_128, GS1_MODE, "[8005]000365", 0, 1, 101, 1, "GGS Figure 7.8.5.2-1 1st",
                    "11010011100111101011101010011110010001001100110110011001001001100010010110000100100001101100011101011"
                },
        /* 31*/ { BARCODE_GS1_128, GS1_MODE, "[10]123456", 0, 1, 90, 1, "GGS Figure 7.8.5.2-1 2nd",
                    "110100111001111010111011001000100101100111001000101100011100010110110010000101100011101011"
                },
        /* 32*/ { BARCODE_GS1_128, GS1_MODE, "[8005]000365[10]123456", 0, 1, 156, 1, "GGS Figure 7.8.5.2-2",
                    "110100111001111010111010100111100100010011001101100110010010011000100101100001111010111011001000100101100111001000101100011100010110101100001001100011101011"
                },
        /* 33*/ { BARCODE_GS1_128, GS1_MODE, "[403]27653113+99000900090010", 0, 1, 222, 1, "DHL Leitcode https://www.dhl.de/de/geschaeftskunden/paket/information/geschaeftskunden/abrechnung/leitcodierung.html",
                    "110100111001111010111011000101000110001101101100101000011011101110110001001001011110111011001011100110001001001011101111010111011110110110011001100100100011011001100110010010001101100110011001000100110001000101100011101011"
                },
        /* 34*/ { BARCODE_GS1_128, GS1_MODE, "[00]340433935039756615", 0, 1, 156, 1, "DHL Identcode https://www.dhl.de/de/geschaeftskunden/paket/information/geschaeftskunden/abrechnung/leitcodierung.html",
                    "110100111001111010111011011001100100010110001001000110010100011000101000111101100010111011010001000110000100101001000011010111001100100111001101100011101011"
                },
        /* 35*/ { BARCODE_GS1_128, GS1_MODE, "[90]ABCDEfGHI", 0, 1, 167, 0, "Shift A; BWIPP different encodation, same codeword count",
                    "11010010000111101011101110010110010011101100101000110001000101100010001000110101100010001000110100010110000100110100010001100010100011000100010110010011101100011101011"
                },
        /* 36*/ { BARCODE_GS1_128, GS1_MODE, "[01]12345678901231[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890123456789012345678901234", 0, 1, 684, 1, "Max length",
                    "110100111001111010111011001101100101100111001000101100011100010110110000101001101111011010110011100110110001101101111011010110011100100010110001110001011011000010100110111101101011001110010001011000111000101101100001010011011110110101100111001000101100011100010110110000101001101111011011110101110111101101101011001110010001011000111000101101100001010011011110110101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010100110111101101011001110010001011000111000101101100001010011011110110101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010100110111101101011001110010001011000100011110101100011101011"
                },
        /* 37*/ { BARCODE_EAN14, GS1_MODE, "4070071967072", 0, 1, 134, 1, "Verified manually against tec-it",
                    "11010011100111101011101100110110011000101000101100001001001100010011001011100100001011001001100010011001001110110111001001100011101011"
                },
        /* 38*/ { BARCODE_NVE18, GS1_MODE, "40700000071967072", 0, 1, 156, 1, "Verified manually against tec-it",
                    "110100111001111010111011011001100110001010001011000010011011001100110110011001001100010011001011100100001011001001100010011001001110110111011101100011101011"
                },
        /* 39*/ { BARCODE_HIBC_128, UNICODE_MODE, "83278F8G9H0J2G", 0, 1, 211, 1, "ANSI/HIBC 2.6 - 2016 Section 4.1, not same, uses different encoding (eg begins StartA instead of StartB)",
                    "1101001000011000100100111010011001011101111011000110110110000101001011110111010001100010111010011001101000100011100101100110001010001001110110010110111000110011100101101000100010001001100111101010001100011101011"
                },
        /* 40*/ { BARCODE_HIBC_128, UNICODE_MODE, "A123BJC5D6E71", 0, 1, 200, 1, "ANSI/HIBC 2.6 - 2016 Figure 1, same",
                    "11010010000110001001001010001100010011100110110011100101100101110010001011000101101110001000100011011011100100101100010001100111010010001101000111011011101001110011011010001000110001101101100011101011"
                },
        /* 41*/ { BARCODE_HIBC_128, UNICODE_MODE, "$$52001510X3G", 0, 1, 178, 1, "ANSI/HIBC 2.6 - 2016 Figure 5, same",
                    "1101001000011000100100100100011001001000110010111011110110111000101101100110010111001100110010001001011110111011100010110110010111001101000100010110001000100011110101100011101011"
                },
        /* 42*/ { BARCODE_DPD, UNICODE_MODE, "%000393206219912345678101040", 0, 1, 211, 1, "DPDAPPD 4.0.2 - Illustrations 2, 7, 8, same; NOTE: correct HRT given by Illustration 7 only",
                    "1101001000010001001100100111011001011101111011011001100110100010001100011011010011001000110111001001011101111010110011100100010110001110001011011000010100110010001001100100010011000101000101011110001100011101011"
                },
        /* 43*/ { BARCODE_DPD, UNICODE_MODE, "%007110601782532948375101276", 0, 1, 211, 1, "DPDAPPD 4.0.2 - Illustration 6 **NOT SAME** HRT incorrect, also uses CodeA and inefficient encoding; verified against tec-it",
                    "1101001000010001001100100111011001011101111010011000100110001001001001100100011001101100110000101001110010110011000110110100010111101011110010011000010010110010001001011001110011001010000100010111101100011101011"
                },
        /* 44*/ { BARCODE_DPD, UNICODE_MODE, "0123456789012345678901234567", 0, 1, 189, 1, "DPDAPPD 4.0.2 - Illustration 9, same (allowing for literal HRT)",
                    "110100111001100110110011101101110101110110001000010110011011011110110011011001110110111010111011000100001011001101101111011001101100111011011101011101100010000101100101011110001100011101011"
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

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                    }
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
        { "test_hrt_cpy_iso8859_1", test_hrt_cpy_iso8859_1, 1, 0, 1 },
        { "test_hrt", test_hrt, 1, 0, 1 },
        { "test_reader_init", test_reader_init, 1, 1, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_ean128_input", test_ean128_input, 1, 1, 1 },
        { "test_hibc_input", test_hibc_input, 1, 1, 1 },
        { "test_ean14_input", test_ean14_input, 1, 1, 1 },
        { "test_dpd_input", test_dpd_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
