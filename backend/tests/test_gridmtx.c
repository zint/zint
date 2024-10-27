/*
    libzint - the open source barcode library
    Copyright (C) 2019-2024 Robin Stuart <rstuart114@gmail.com>

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
/* SPDX-License-Identifier: BSD-3-Clause */

#include "testcommon.h"

static void test_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_2;
        int option_3;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, "1", 2751, 0, 162, 162, "" },
        /*  1*/ { -1, -1, "1", 2752, ZINT_ERROR_TOO_LONG, -1, -1, "Error 531: Input too long, requires too many codewords (maximum 1313)" },
        /*  2*/ { -1, -1, "1", 2755, ZINT_ERROR_TOO_LONG, -1, -1, "Error 531: Input too long, requires too many codewords (maximum 1313)" }, /* Triggers buffer > 9191 */
        /*  3*/ { -1, -1, "A", 1836, 0, 162, 162, "" },
        /*  4*/ { -1, -1, "A", 1837, ZINT_ERROR_TOO_LONG, -1, -1, "Error 531: Input too long, requires too many codewords (maximum 1313)" },
        /*  5*/ { -1, -1, "A1", 1529, 0, 162, 162, "" },
        /*  6*/ { -1, -1, "A1", 1530, ZINT_ERROR_TOO_LONG, -1, -1, "Error 531: Input too long, requires too many codewords (maximum 1313)" },
        /*  7*/ { -1, -1, "\200", 1143, 0, 162, 162, "" },
        /*  8*/ { -1, -1, "\200", 1144, ZINT_ERROR_TOO_LONG, -1, -1, "Error 531: Input too long, requires too many codewords (maximum 1313)" },
        /*  9*/ { -1, ZINT_FULL_MULTIBYTE, "\241", 1410, 0, 162, 162, "" },
        /* 10*/ { -1, ZINT_FULL_MULTIBYTE, "\241", 1412, ZINT_ERROR_TOO_LONG, -1, -1, "Error 531: Input too long, requires too many codewords (maximum 1313)" },
        /* 11*/ { 1, -1, "1", 18, 0, 18, 18, "" },
        /* 12*/ { 1, -1, "1", 19, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 1, requires 13 codewords (maximum 11)" },
        /* 13*/ { 1, -1, "A", 13, 0, 18, 18, "" },
        /* 14*/ { 1, -1, "A", 14, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 1, requires 12 codewords (maximum 11)" },
        /* 15*/ { 1, -1, "\200", 7, 0, 18, 18, "" },
        /* 16*/ { 1, -1, "\200", 8, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 1, requires 12 codewords (maximum 11)" },
        /* 17*/ { 1, ZINT_FULL_MULTIBYTE, "\241", 8, 0, 18, 18, "" },
        /* 18*/ { 1, ZINT_FULL_MULTIBYTE, "\241", 10, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 1, requires 12 codewords (maximum 11)" },
        /* 19*/ { 2, ZINT_FULL_MULTIBYTE, "\241", 40, 0, 30, 30, "" },
        /* 20*/ { 2, ZINT_FULL_MULTIBYTE, "\241", 41, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 2, requires 42 codewords (maximum 40)" },
        /* 21*/ { 3, -1, "A", 108, 0, 42, 42, "" },
        /* 22*/ { 3, -1, "A", 109, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 3, requires 80 codewords (maximum 79)" },
        /* 23*/ { 4, -1, "A", 202, 0, 54, 54, "" },
        /* 24*/ { 4, -1, "A", 203, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 4, requires 147 codewords (maximum 146)" },
        /* 25*/ { 5, -1, "1", 453, 0, 66, 66, "" },
        /* 26*/ { 5, -1, "1", 454, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 5, requires 220 codewords (maximum 218)" },
        /* 27*/ { 6, -1, "1", 633, 0, 78, 78, "" },
        /* 28*/ { 6, -1, "1", 634, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 6, requires 306 codewords (maximum 305)" },
        /* 29*/ { 7, -1, "\200", 352, 0, 90, 90, "" },
        /* 30*/ { 7, -1, "\200", 353, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 7, requires 406 codewords (maximum 405)" },
        /* 31*/ { 8, -1, "A", 727, 0, 102, 102, "" },
        /* 32*/ { 8, -1, "A", 728, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 8, requires 522 codewords (maximum 521)" },
        /* 33*/ { 9, -1, "A", 908, 0, 114, 114, "" },
        /* 34*/ { 9, -1, "A", 909, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 9, requires 651 codewords (maximum 650)" },
        /* 35*/ { 10, -1, "1", 1662, 0, 126, 126, "" },
        /* 36*/ { 10, -1, "1", 1663, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 10, requires 796 codewords (maximum 794)" },
        /* 37*/ { 11, -1, "1", 1995, 0, 138, 138, "" },
        /* 38*/ { 11, -1, "1", 1996, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 11, requires 954 codewords (maximum 953)" },
        /* 39*/ { 11, -1, "1", 2748, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 11, requires 1311 codewords (maximum 953)" },
        /* 40*/ { 12, -1, "1", 2355, 0, 150, 150, "" },
        /* 41*/ { 12, -1, "1", 2356, ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 12, requires 1126 codewords (maximum 1125)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[2800 + 1];

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, data[i].option_3, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_1;
        int option_2;
        struct zint_structapp structapp;
        char *data;
        int ret_encode;
        int ret_vector;
        int expected_size;
        const char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 0, 0, { 0, 0, "" }, "12345", 0, 0, 18, "" },
        /*  1*/ { 0, 1, { 0, 0, "" }, "12345", 0, 0, 18, "" },
        /*  2*/ { 0, 2, { 0, 0, "" }, "12345", 0, 0, 30, "" },
        /*  3*/ { 0, 14, { 0, 0, "" }, "12345", 0, 0, 18, "" }, /* Version > max version 13 so ignored */
        /*  4*/ { 0, 13, { 0, 0, "" }, "12345", 0, 0, 162, "" },
        /*  5*/ { 0, 1, { 0, 0, "" }, "1234567890123456789", ZINT_ERROR_TOO_LONG, -1, -1, "Error 534: Input too long for Version 1, requires 13 codewords (maximum 11)" },
        /*  6*/ { 0, 2, { 0, 0, "" }, "1234567890123456789", 0, 0, 30, "" },
        /*  7*/ { 0, 0, { 0, 0, "" }, "123456789012345678", 0, 0, 30, "" }, /* Version auto-set to 2 */
        /*  8*/ { 0, 1, { 0, 0, "" }, "123456789012345678", 0, 0, 18, "" },
        /*  9*/ { 5, 1, { 0, 0, "" }, "123456789012345678", 0, 0, 18, "" }, /* Version specified so overrides ECC level which gets reduced to 4 */
        /* 10*/ { 5, 0, { 0, 0, "" }, "123456789012345678", 0, 0, 30, "" }, /* Version not specified so increased to allow for ECC level */
        /* 11*/ { 6, 0, { 0, 0, "" }, "123456789012345678", 0, 0, 30, "" }, /* ECC > max ECC 5 so ignored and auto-settings version 2, ECC 4 used */
        /* 12*/ { 1, 0, { 0, 0, "" }, "123456789012345678", 0, 0, 30, "" }, /* ECC < min ECC 2, ECC 2 used */
        /* 13*/ { 4, 1, { 0, 0, "" }, "123456789012345678", 0, 0, 18, "" },
        /* 14*/ { 0, 0, { 1, 2, "" }, "12345", 0, 0, 18, "" },
        /* 15*/ { 0, 0, { 1, 1, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 536: Structured Append count '1' out of range (2 to 16)" },
        /* 16*/ { 0, 0, { 1, 17, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 536: Structured Append count '17' out of range (2 to 16)" },
        /* 17*/ { 0, 0, { 0, 2, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 537: Structured Append index '0' out of range (1 to count 2)" },
        /* 18*/ { 0, 0, { 3, 2, "" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 537: Structured Append index '3' out of range (1 to count 2)" },
        /* 19*/ { 0, 0, { 1, 2, "255" }, "12345", 0, 0, 18, "" },
        /* 20*/ { 0, 0, { 1, 2, "1234" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 538: Structured Append ID length 4 too long (3 digit maximum)" },
        /* 21*/ { 0, 0, { 1, 2, "A" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 539: Invalid Structured Append ID (digits only)" },
        /* 22*/ { 0, 0, { 1, 2, "256" }, "12345", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 530: Structured Append ID value '256' out of range (0 to 255)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_options", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);
        if (ret < ZINT_ERROR) {
            assert_equal(symbol->width, data[i].expected_size, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_size);
            assert_equal(symbol->rows, data[i].expected_size, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_size);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d symbol->errtxt %s != %s\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int output_options;
        int option_3;
        struct zint_structapp structapp;
        char *data;
        int ret;
        int expected_eci;
        char *expected;
        char *comment;
    };
    /*
       é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 2312 0xA8A6, UTF-8 C3A9
       β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in GB 2312 0xA6C2, UTF-8 CEB2
       ÿ U+00FF in ISO 8859-1 0xFF, not in GB 2312, outside first byte and second byte range, UTF-8 C3BF
       ㈩ U+3229 in GB 2312 0x226E
       一 U+4E00 in GB 2312 0x523B
    */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "é", 0, 0, "08 54 6F 78 00", "H1 (GB 2312) Note: Grid Matrix default is GB 2312, not ISO 8859-1" },
        /*  1*/ { UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "é", 0, 3, "60 01 58 00 74 40", "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 29, -1, -1, { 0, 0, "" }, "é", 0, 29, "60 0E 44 2A 37 7C 00", "ECI-29 H1 (GB 2312)" },
        /*  3*/ { UNICODE_MODE, 26, -1, -1, { 0, 0, "" }, "é", 0, 26, "60 0D 18 01 61 6A 20", "ECI-26 B2 (UTF-8)" },
        /*  4*/ { UNICODE_MODE, 26, -1, ZINT_FULL_MULTIBYTE, { 0, 0, "" }, "é", 0, 26, "60 0D 05 28 4F 7C 00", "ECI-26 H1 (UTF-8) (full multibyte)" },
        /*  5*/ { DATA_MODE, 0, -1, -1, { 0, 0, "" }, "é", 0, 0, "30 03 43 54 40", "B2 (UTF-8)" },
        /*  6*/ { DATA_MODE, 0, -1, ZINT_FULL_MULTIBYTE, { 0, 0, "" }, "é", 0, 0, "0A 51 1F 78 00", "H1 (UTF-8) (full multibyte)" },
        /*  7*/ { DATA_MODE, 0, -1, -1, { 0, 0, "" }, "\351", 0, 0, "30 01 69 00", "B1 (ISO 8859-1) (0xE9)" },
        /*  8*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "β", 0, 0, "08 40 2F 78 00", "H1 (GB 2312)" },
        /*  9*/ { UNICODE_MODE, 9, -1, -1, { 0, 0, "" }, "β", 0, 9, "60 04 58 00 71 00", "ECI-9 B1 (ISO 8859-7)" },
        /* 10*/ { UNICODE_MODE, 29, -1, -1, { 0, 0, "" }, "β", 0, 29, "60 0E 44 20 17 7C 00", "ECI-29 H1 (GB 2312)" },
        /* 11*/ { UNICODE_MODE, 26, -1, -1, { 0, 0, "" }, "β", 0, 26, "60 0D 18 01 67 2C 40", "ECI-26 H1 (UTF-8)" },
        /* 12*/ { UNICODE_MODE, 26, -1, ZINT_FULL_MULTIBYTE, { 0, 0, "" }, "β", 0, 26, "60 0D 05 6B 17 7C 00", "ECI-26 H1 (UTF-8) (full multibyte)" },
        /* 13*/ { DATA_MODE, 0, -1, -1, { 0, 0, "" }, "β", 0, 0, "30 03 4E 59 00", "B2 (UTF-8)" },
        /* 14*/ { DATA_MODE, 0, -1, ZINT_FULL_MULTIBYTE, { 0, 0, "" }, "β", 0, 0, "0B 56 2F 78 00", "H1 (UTF-8) (full multibyte)" },
        /* 15*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "ÿ", ZINT_WARN_USES_ECI, 3, "Warning 60 01 58 00 7F 40", "ECI-3 B1 (ISO 8859-1)" },
        /* 16*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "ÿÿÿ", ZINT_WARN_USES_ECI, 3, "Warning 60 01 58 02 7F 7F 7F 70", "ECI-3 B3 (ISO 8859-1)" },
        /* 17*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "㈩一", 0, 0, "08 15 68 0E 7F 70 00", "H2 (GB 2312)" },
        /* 18*/ { UNICODE_MODE, 29, -1, -1, { 0, 0, "" }, "㈩一", 0, 29, "60 0E 44 0A 74 07 3F 78 00", "ECI-29 H2 (GB 2312)" },
        /* 19*/ { DATA_MODE, 0, -1, -1, { 0, 0, "" }, "\177\177", 0, 0, "30 02 7F 3F 40", "B2 (ASCII)" },
        /* 20*/ { DATA_MODE, 0, -1, -1, { 0, 0, "" }, "\177\177\177", 0, 0, "30 04 7F 3F 5F 60", "B3 (ASCII)" },
        /* 21*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "123", 0, 0, "10 1E 7F 68", "N3 (ASCII)" },
        /* 22*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, " 123", 0, 0, "11 7A 03 6F 7D 00", "N4 (ASCII)" },
        /* 23*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "1+23", 0, 0, "11 7B 03 6F 7D 00", "N4 (ASCII)" },
        /* 24*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "12.3", 0, 0, "11 7C 63 6F 7D 00", "N4 (ASCII)" },
        /* 25*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "123,", 0, 0, "10 1E 7F 73 76 5E 60", "N3 L1 (ASCII)" },
        /* 26*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "123,4", 0, 0, "14 1E 7F 51 48 3F 50", "N5 (ASCII)" },
        /* 27*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "\015\012123", 0, 0, "11 7D 63 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 28*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "1\015\01223", 0, 0, "11 7E 03 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 29*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "12\015\0123", 0, 0, "11 7E 23 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 30*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "123\015\012", 0, 0, "10 1E 7F 7C 01 06 42 40", "N3 B2 (ASCII) (EOL)" },
        /* 31*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "123\015\0124", 0, 0, "14 1E 7F 5D 48 3F 50", "N5 (ASCII) (EOL)" },
        /* 32*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "2.2.0", 0, 0, "15 7C 46 73 78 40 07 7A", "N5 (ASCII)" },
        /* 33*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "2.2.0.5", 0, 0, "30 0C 32 17 0C 45 63 01 38 6A 00", "B7 (ASCII)" },
        /* 34*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "2.2.0.56", 0, 0, "13 7C 46 73 78 40 07 71 46 0F 74", "N8 (ASCII)" },
        /* 35*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "20.12.13.\015\012", 0, 0, "11 7C 66 27 79 0D 2F 7F 00 45 60 68 28 00", "N8 B3 (ASCII)" },
        /* 36*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "ABCDE\011F", 0, 0, "20 01 08 32 3E 49 17 30", "U7 (ASCII)" },
        /* 37*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "1 1234ABCD12.2abcd-12", 0, 0, "13 7A 23 41 2A 3F 68 01 08 3E 4F 66 1E 5F 70 00 44 1F 2F 6E 0F 0F 74", "N6 U4 N4 L4 N3 (ASCII)" },
        /* 38*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "1 123ABCDE12.2abcd-12", 0, 0, "28 1F 40 42 06 28 59 43 27 01 05 7D 56 42 49 16 34 7F 6D 30 08 2F 60", "M21 (ASCII)" },
        /* 39*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "国外通信教材 Matlab6.5", 0, 0, "09 63 27 20 4E 24 1F 05 21 58 22 13 7E 1E 4C 78 09 56 00 3D 3F 4A 45 3F 50", "H6 U2 L5 N3 (GB 2312) (Same as D.2 example)" },
        /* 40*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "AAT", 0, 0, "20 00 4F 30", "U3 (ASCII)" },
        /* 41*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "aat", 0, 0, "18 00 4F 30", "L3 (ASCII)" },
        /* 42*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "AAT2556", 0, 0, "20 00 4F 58 7F 65 47 7A", "U3 N4 (ASCII) (note same bit count as M7)" },
        /* 43*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "AAT2556 ", 0, 0, "29 22 4E 42 0A 14 37 6F 60", "M8 (ASCII)" },
        /* 44*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "AAT2556 电", 0, 0, "29 22 4E 42 0A 14 37 6F 62 2C 1F 7E 00", "M8 H1 (GB 2312)" },
        /* 45*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, " 200", 0, 0, "11 7A 06 23 7D 00", "N4 (ASCII)" },
        /* 46*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, " 200mA至", 0, 0, "2F 60 40 00 60 2B 78 63 41 7F 40", "M6 H1 (GB 2312)" },
        /* 47*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "2A tel:86 019 82512738", 0, 0, "28 22 5F 4F 29 48 5F 6D 7E 6F 55 57 1F 28 63 0F 5A 11 64 0F 74", "M2 L5(with control) N15 (ASCII)" },
        /* 48*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "至2A tel:86 019 82512738", 0, 0, "30 07 56 60 4C 48 13 6A 32 17 7B 3F 5B 75 35 67 6A 18 63 76 44 39 03 7D 00", "B4 L5(with control) N15 (GB 2312)" },
        /* 49*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "AAT2556 电池充电器＋降压转换器 200mA至2A tel:86 019 82512738", 0, 0, "(62) 29 22 22 1C 4E 41 42 7E 0A 40 14 00 37 7E 6F 00 62 7E 2C 00 1C 7E 4B 00 41 7E 18 00", "M8 H11 M6 B4 L5(with control) N15 (GB 2312) (*NOT SAME* as D3 example Figure D.1, M8 H11 M6 H1 M3 L4(with control) N15, which uses a few more bits)" },
        /* 50*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::", 0, 0, "(588) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B512 (ASCII)" },
        /* 51*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\177", 0, 0, "(591) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B513 (ASCII)" },
        /* 52*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::至", 0, 0, "(591) 37 68 68 68 68 68 74 7C 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B511 H1 (GB 2312)" },
        /* 53*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::至:", 0, 0, "(592) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B513 (GB 2312)" },
        /* 54*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "电电123456", 0, 0, "09 30 72 61 7F 70 41 76 72 1F 68", "H2 (GB 2312) N6" },
        /* 55*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "电电abcdef", 0, 0, "09 30 72 61 7F 71 00 08 43 10 5D 40", "H2 (GB 2312) L6" },
        /* 56*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "电电电电电\011\011\011", 0, 0, "09 30 72 61 65 43 4B 07 16 0F 7F 14 02 04 42 21 10", "H5 (GB 2312) B3" },
        /* 57*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "1234567电电", 0, 0, "14 1E 6E 22 5E 3F 59 30 72 61 7F 70 00", "N7 H2 (GB 2312)" },
        /* 58*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "12345678mA 2", 0, 0, "12 1E 6E 23 06 3F 76 02 5F 02 7E 00", "N8 M4" },
        /* 59*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "ABCDEFG电电", 0, 0, "20 01 08 32 0A 37 05 43 4B 07 7F 40", "U7 H2 (GB 2312)" },
        /* 60*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "ABCDEFGHIJ8mA 2", 0, 0, "20 01 08 32 0A 31 68 27 70 46 02 5F 02 7E 00", "U10 M5" },
        /* 61*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "ABCDEFGHIJ\011\011\011\011", 0, 0, "20 01 08 32 0A 31 68 27 78 03 04 42 21 10 48 00", "U10 B4" },
        /* 62*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "8mA B123456789", 0, 0, "29 0C 05 3E 17 7C 40 7B 39 0C 2B 7E 40", "M5 N9" },
        /* 63*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "8mA aABCDEFGH", 0, 0, "29 0C 05 3E 49 7D 00 04 21 48 29 47 6C", "M5 U8" },
        /* 64*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "\011\011\011\011123456", 0, 0, "30 06 09 04 42 21 12 03 6D 64 3F 50", "B4 N6" },
        /* 65*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "\011\011\011\011ABCDEF", 0, 0, "30 06 09 04 42 21 14 00 11 06 21 3B", "B4 U6" },
        /* 66*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "\011\011\011\0118mA 2", 0, 0, "30 06 09 04 42 21 15 11 40 57 60 5F 40", "B4 M5" },
        /* 67*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "电电电电电\015\012", 0, 0, "09 30 72 61 65 43 4B 07 16 0F 73 03 7E 00", "H7 (GB 2312)" },
        /* 68*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "电电电电电12", 0, 0, "09 30 72 61 65 43 4B 07 16 0F 7B 37 7E 00", "H7 (GB 2312)" },
        /* 69*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "1234567.8\015\012123456", 0, 0, "10 1E 6E 23 79 30 67 77 0F 37 11 7E 40", "N17" },
        /* 70*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "˘", ZINT_WARN_USES_ECI, 4, "Warning 60 02 18 00 51 00", "ECI-4 B1 (ISO 8859-2)" },
        /* 71*/ { UNICODE_MODE, 4, -1, -1, { 0, 0, "" }, "˘", 0, 4, "60 02 18 00 51 00", "ECI-4 B1 (ISO 8859-2)" },
        /* 72*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "Ħ", ZINT_WARN_USES_ECI, 5, "Warning 60 02 58 00 50 40", "ECI-5 B1 (ISO 8859-3)" },
        /* 73*/ { UNICODE_MODE, 5, -1, -1, { 0, 0, "" }, "Ħ", 0, 5, "60 02 58 00 50 40", "ECI-5 B1 (ISO 8859-3)" },
        /* 74*/ { UNICODE_MODE, 6, -1, -1, { 0, 0, "" }, "ĸ", 0, 6, "60 03 18 00 51 00", "ECI-6 B1 (ISO 8859-4)" },
        /* 75*/ { UNICODE_MODE, 7, -1, -1, { 0, 0, "" }, "Ж", 0, 7, "60 03 58 00 5B 00", "ECI-7 B1 (ISO 8859-5)" },
        /* 76*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "Ș", ZINT_WARN_USES_ECI, 18, "Warning 60 09 18 00 55 00", "ECI-18 B1 (ISO 8859-16)" },
        /* 77*/ { UNICODE_MODE, 18, -1, -1, { 0, 0, "" }, "Ș", 0, 18, "60 09 18 00 55 00", "ECI-18 B1 (ISO 8859-16)" },
        /* 78*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "テ", 0, 0, "08 34 6F 78 00", "H1 (GB 2312)" },
        /* 79*/ { UNICODE_MODE, 20, -1, -1, { 0, 0, "" }, "テ", 0, 20, "60 0A 18 01 41 59 20", "ECI-20 B2 (SHIFT JIS)" },
        /* 80*/ { UNICODE_MODE, 20, -1, -1, { 0, 0, "" }, "テテ", 0, 20, "60 0A 18 03 41 59 30 36 28 00", "ECI-20 B4 (SHIFT JIS)" },
        /* 81*/ { UNICODE_MODE, 20, -1, -1, { 0, 0, "" }, "\\\\", 0, 20, "60 0A 18 03 40 57 70 15 78 00", "ECI-20 B4 (SHIFT JIS)" },
        /* 82*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "…", 0, 0, "08 01 5F 78 00", "H1 (GB 2312)" },
        /* 83*/ { UNICODE_MODE, 21, -1, -1, { 0, 0, "" }, "…", 0, 21, "60 0A 58 00 42 40", "ECI-21 B1 (Win 1250)" },
        /* 84*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "Ґ", ZINT_WARN_USES_ECI, 22, "Warning 60 0B 18 00 52 40", "ECI-22 B1 (Win 1251)" },
        /* 85*/ { UNICODE_MODE, 22, -1, -1, { 0, 0, "" }, "Ґ", 0, 22, "60 0B 18 00 52 40", "ECI-22 B1 (Win 1251)" },
        /* 86*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "˜", ZINT_WARN_USES_ECI, 23, "Warning 60 0B 58 00 4C 00", "ECI-23 B1 (Win 1252)" },
        /* 87*/ { UNICODE_MODE, 23, -1, -1, { 0, 0, "" }, "˜", 0, 23, "60 0B 58 00 4C 00", "ECI-23 B1 (Win 1252)" },
        /* 88*/ { UNICODE_MODE, 24, -1, -1, { 0, 0, "" }, "پ", 0, 24, "60 0C 18 00 40 40", "ECI-24 B1 (Win 1256)" },
        /* 89*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "က", ZINT_WARN_USES_ECI, 26, "Warning 60 0D 18 02 70 60 10 00", "ECI-26 B3 (UTF-8)" },
        /* 90*/ { UNICODE_MODE, 25, -1, -1, { 0, 0, "" }, "က", 0, 25, "60 0C 58 01 08 00 00", "ECI-25 B2 (UCS-2BE)" },
        /* 91*/ { UNICODE_MODE, 25, -1, -1, { 0, 0, "" }, "ကက", 0, 25, "60 0C 58 03 08 00 02 00 00 00", "ECI-25 B4 (UCS-2BE)" },
        /* 92*/ { UNICODE_MODE, 25, -1, -1, { 0, 0, "" }, "12", 0, 25, "60 0C 58 03 00 0C 20 03 10 00", "ECI-25 B4 (UCS-2BE ASCII)" },
        /* 93*/ { UNICODE_MODE, 27, -1, -1, { 0, 0, "" }, "@", 0, 27, "60 0D 4F 77 2E 60", "ECI-27 L1 (ASCII)" },
        /* 94*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "龘", ZINT_WARN_USES_ECI, 26, "Warning 60 0D 18 02 74 6F 53 00", "ECI-26 B3 (UTF-8)" },
        /* 95*/ { UNICODE_MODE, 28, -1, -1, { 0, 0, "" }, "龘", 0, 28, "60 0E 18 01 7C 75 20", "ECI-28 B2 (Big5)" },
        /* 96*/ { UNICODE_MODE, 28, -1, -1, { 0, 0, "" }, "龘龘", 0, 28, "60 0E 18 03 7C 75 3F 1D 28 00", "ECI-28 B4 (Big5)" },
        /* 97*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "齄", 0, 0, "0F 4B 6F 78 00", "H1 (GB 2312)" },
        /* 98*/ { UNICODE_MODE, 29, -1, -1, { 0, 0, "" }, "齄", 0, 29, "60 0E 47 65 77 7C 00", "ECI-29 H1 (GB 2312)" },
        /* 99*/ { UNICODE_MODE, 29, -1, -1, { 0, 0, "" }, "齄齄", 0, 29, "60 0E 47 65 77 4B 6F 78 00", "ECI-29 H2 (GB 2312)" },
        /*100*/ { UNICODE_MODE, 0, -1, -1, { 0, 0, "" }, "가", ZINT_WARN_USES_ECI, 26, "Warning 60 0D 18 02 75 2C 10 00", "ECI-26 B3 (UTF-8)" },
        /*101*/ { UNICODE_MODE, 30, -1, -1, { 0, 0, "" }, "가", 0, 30, "60 0F 18 01 58 28 20", "ECI-30 B2 (EUC-KR)" },
        /*102*/ { UNICODE_MODE, 30, -1, -1, { 0, 0, "" }, "가가", 0, 30, "60 0F 18 03 58 28 36 0A 08 00", "ECI-30 B4 (EUC-KR)" },
        /*103*/ { UNICODE_MODE, 170, -1, -1, { 0, 0, "" }, "?", 0, 170, "60 55 0F 77 26 60", "ECI-170 L1 (ASCII invariant)" },
        /*104*/ { DATA_MODE, 899, -1, -1, { 0, 0, "" }, "\200", 0, 899, "63 41 58 00 40 00", "ECI-899 B1 (8-bit binary)" },
        /*105*/ { UNICODE_MODE, 900, -1, -1, { 0, 0, "" }, "é", 0, 900, "63 42 18 01 61 6A 20", "ECI-900 B2 (no conversion)" },
        /*106*/ { UNICODE_MODE, 1024, -1, -1, { 0, 0, "" }, "é", 0, 1024, "64 08 00 30 03 43 54 40", "ECI-1024 B2 (no conversion)" },
        /*107*/ { UNICODE_MODE, 32768, -1, -1, { 0, 0, "" }, "é", 0, 32768, "66 08 00 01 40 0E 0E 52 00", "ECI-32768 B2 (no conversion)" },
        /*108*/ { UNICODE_MODE, 811800, -1, -1, { 0, 0, "" }, "é", ZINT_ERROR_INVALID_OPTION, 811800, "Error 533: ECI code '811800' out of range (0 to 811799)", "" },
        /*109*/ { UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "β", ZINT_ERROR_INVALID_DATA, 3, "Error 535: Invalid character in input for ECI '3'", "" },
        /*110*/ { UNICODE_MODE, 0, READER_INIT, -1, { 0, 0, "" }, "12", 0, 0, "51 11 71 7E 40", "" },
        /*111*/ { UNICODE_MODE, 0, -1, -1, { 1, 16, "" }, "12", 0, 0, "48 03 60 24 3C 3F 50", "FNC2 ID0 Cnt15 Ind0 N2" },
        /*112*/ { UNICODE_MODE, 0, READER_INIT, -1, { 1, 16, "" }, "12", 0, 0, "54 40 1E 02 23 63 7D 00", "FNC3 FNC2 ID0 Cnt15 Ind0 N2" },
        /*113*/ { UNICODE_MODE, 0, -1, -1, { 2, 16, "" }, "12", 0, 0, "48 03 62 24 3C 3F 50", "FNC2 ID0 Cnt15 Ind1 N2" },
        /*114*/ { UNICODE_MODE, 0, READER_INIT, -1, { 2, 16, "" }, "12", 0, 0, "48 03 62 24 3C 3F 50", "FNC2 ID0 Cnt15 Ind1 N2 (FNC3 omitted)" },
        /*115*/ { UNICODE_MODE, 0, -1, -1, { 3, 3, "255" }, "12", 0, 0, "4F 7C 44 24 3C 3F 50", "FNC2 ID256 Cnt2 Ind2 N2" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, data[i].input_mode, data[i].eci, -1 /*option_1*/, -1, data[i].option_3, data[i].output_options, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %s, %s, { %d, %d, \"%s\" }, \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilOutputOptionsName(data[i].output_options),
                    testUtilOption3Name(BARCODE_GRIDMATRIX, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), ret < ZINT_ERROR ? symbol->eci : -1, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
            }
            assert_zero(strcmp((char *) symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        int input_mode;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { "1234", UNICODE_MODE, -1, -1, 0, 18, 18, "",
                    "111111000000111111"
                    "101111001100101001"
                    "100101000010100001"
                    "111011011100101111"
                    "101011011000111011"
                    "111111000000111111"
                    "000000111111000000"
                    "001100100001001110"
                    "001110111111011110"
                    "001010100011000000"
                    "010100101001000000"
                    "000000111111000000"
                    "111111000000111111"
                    "101001001100101001"
                    "111001000000101111"
                    "111001000100110001"
                    "111111000000100001"
                    "111111000000111111"
               },
        /*  1*/ { "Grid Matrix", UNICODE_MODE, 5, -1, 0, 30, 30, "AIMD014 Figure 1 **NOT SAME** different encodation, uses Upper and Lower whereas figure uses Mixed and Lower",
                    "111111000000111111000000111111"
                    "110111010110110111010110110011"
                    "100011011110111111011110111111"
                    "110001000000100001000000100001"
                    "101101000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "010000101111001000101101010000"
                    "000100111111000000110011010110"
                    "001110100001011000111111001000"
                    "001010100001001010111011000010"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "110001001110100101001000110001"
                    "111111011110111111000010111111"
                    "110111000000100101000010100101"
                    "100111000000100011011100100011"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "010100101111001110101101010110"
                    "011100111111000000110001001100"
                    "001010100001000110110111011110"
                    "000010100001011100110001000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "110101010000110101010010110101"
                    "110111000000110011001010101111"
                    "111101010010110101010010111011"
                    "101111010010100001010010110111"
                    "111111000000111111000000111111"
               },
        /*  2*/ { "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738", UNICODE_MODE, 3, 3, 0, 42, 42, "AIMD014 Figure D.1 **NOT SAME** different encodation, see test_input dataset",
                    "111111000000111111000000111111000000111111"
                    "101101001100101111001010101011001100101101"
                    "110001011010110101010000100011000000100001"
                    "110001001110111111010000101111010010101111"
                    "101101010100100111011000110111011000111101"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001010100111000000100011000110100101001110"
                    "011010101101001100101011000110110001011110"
                    "000010111011001110100101001110110101000000"
                    "000110111111011010100011011100100001000000"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101101000110111111011100111001000010101111"
                    "110111011100111011000010110101010100111111"
                    "100011010000111101001000100001011110100001"
                    "100111010110110111011100110101000110100001"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001010100001011110110011011110100111001110"
                    "010010111001000000100011001110111111011110"
                    "010100111111011000100101010110111111000000"
                    "001010101011000100110011001110111011000000"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101001000000111001011100111011000010101101"
                    "111011001000111001001010101101011110111101"
                    "110011010100101001010010101101001110100001"
                    "100001001000100011011000100101000100100001"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001110100011000110100101000000100001001110"
                    "010000100111001010111101000010100001010100"
                    "010100100111010000101011000000100001000100"
                    "001110100001000110111111001100101001000100"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101011001110101001001100101011001010101111"
                    "100011011010100001011100101001010000110101"
                    "111101000110110001000100100111010110110011"
                    "100001001000110011011110110001000100100101"
                    "111111000000111111000000111111000000111111"
               },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, %d, %s, %d, %d, \"%s\",\n",
                    i, data[i].data, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("               },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        struct zint_structapp structapp;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    /*
       ¶ not in GB 2312 (in ISO/IEC 8869-1)
       Ж in GB 2312 (and ISO/IEC 8859-5)
    */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 30, 30, "Standard example (adds ECI 3 for ¶)",
                    "111111000000111111000000111111"
                    "111111011110111111011110111111"
                    "100001011110111111011110111111"
                    "100101000000100001000000100001"
                    "100011000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011110110111010000110001011110"
                    "000110111111000000100011011110"
                    "000100100001001010111011000000"
                    "001110100001010000110111000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111101010110101001010000111111"
                    "100001011110100001001110111111"
                    "100011000000111101000000100001"
                    "111111000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011000110111010000110001011110"
                    "011100111111000000100011001010"
                    "000010100001001100101101000010"
                    "011100100001011000100001001110"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111001011110111011011100111011"
                    "100001010100111101001110110001"
                    "110011001110101111010110110001"
                    "100011001100110011010100100101"
                    "111111000000111111000000111111"
               },
        /*  1*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 30, 30, "Standard example auto-ECI (adds ECI-3 from ¶, ECI-29 for Ж)",
                    "111111000000111111000000111111"
                    "111111011110111111011110111111"
                    "100001011110111111011110111111"
                    "100101000000100001000000100001"
                    "100011000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011110110111010000110001011110"
                    "000110111111000000100011011110"
                    "000100100001001010111011000000"
                    "001110100001010000110111000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111101010110101001010000111111"
                    "100001011110100001001110111111"
                    "100011000000111101000000100001"
                    "111111000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011000110111010000110001011110"
                    "011100111111000000100011001010"
                    "000010100001001100101101000010"
                    "011100100001011000100001001110"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111001011110111011011100111011"
                    "100001010100111101001110110001"
                    "110011001110101111010110110001"
                    "100011001100110011010100100101"
                    "111111000000111111000000111111"
               },
        /*  2*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 30, 30, "Standard example inverted",
                    "111111000000111111000000111111"
                    "111001011110111111011110111111"
                    "101111011110111111011110111111"
                    "111101000000100001000000100001"
                    "101011000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011010110111010000110001011110"
                    "000000111111000000100011011110"
                    "010000100001001010111011000000"
                    "011000100001010000110111000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111101010110101001010000111111"
                    "101011011110100011000110111111"
                    "111001000000111101000000100001"
                    "101011000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011110110111010000110001011000"
                    "010000111111000000100011011000"
                    "001110100001001100101101001100"
                    "001100100001011000100001000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111111011110111101011000111111"
                    "101111011010110001001110111101"
                    "110011001000101101001010110111"
                    "111101011000101001011010101001"
                    "111111000000111111000000111111"
               },
        /*  3*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 30, 30, "Standard example inverted auto-ECI",
                    "111111000000111111000000111111"
                    "111001011110111111011110111111"
                    "101111011110111111011110111111"
                    "111101000000100001000000100001"
                    "101011000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011010110111010000110001011110"
                    "000000111111000000100011011110"
                    "010000100001001010111011000000"
                    "011000100001010000110111000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111101010110101001010000111111"
                    "101011011110100011000110111111"
                    "111001000000111101000000100001"
                    "101011000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011110110111010000110001011000"
                    "010000111111000000100011011000"
                    "001110100001001100101101001100"
                    "001100100001011000100001000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111111011110111101011000111111"
                    "101111011010110001001110111101"
                    "110011001000101101001010110111"
                    "111101011000101001011010101001"
                    "111111000000111111000000111111"
               },
        /*  4*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a -128 GB的存储空间-黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a - 128 GB Speicher - Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 78, 78, "AIM ITS/04-023:2022 Annex A example",
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "100001000100100011000110100011000010100001000100100001000010100111000000100111"
                    "111111001110101001010010111011001010100111010100101011011100101011001010101011"
                    "110111000010100101001000100111010100101011001110111111011100101111000110111011"
                    "111001011000100011010100101011010110101111010100100011011110110101001110111111"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "000010111111011110111001011110111111011010111111011010111111011110111111000010"
                    "011000110001000000110001011110101011010110111111011100100011011110111011001000"
                    "000100101111011010110001000000110011010000101111011110110001000010101111001100"
                    "011100110011000000100001010010111111000000110011011100100001011110101011010110"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "100001011010110001010110110111010010110111010000110101010110110001011010100011"
                    "111101001010100001001100110111001010111111000110101111011100101001010110100111"
                    "100011011010100111001110110011001010101101010000110111001110110101000000110101"
                    "110001010110111111000110100111001000111101001000110011011010100111000000110101"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "000100111011010110101001001000101011001100101111001010101111010100111111000100"
                    "010010101011011000101001011010101111011110101001000010111101000110111111010110"
                    "011100100001000000111011011100111001010010100011001000111011011110101101000100"
                    "010010100001000000111111010100111111011010111101010000111111000110100001010000"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "100011011010110111001000100011000000100101000110100111001110110011011000100101"
                    "101001010010100011001000100101011010101111011010111111011000111011001000101111"
                    "100011011010101001000100101111000000111111011010111111000000100001001110101001"
                    "101101011000100011001110100111000100110111000010101101000100110001011110110011"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "000000111111010100101101000100111101011000111101000110101001010100111111000010"
                    "011100111111001000100001000000101001000000111101011110111101001010100011001010"
                    "000000100111001100101011011110111101010110111001010100101101010100110001011100"
                    "000100111101000000110001010000110001000110110111011000100101011010100001010000"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "100111011010110111001100100011011000110111011000100111001000110001011110100011"
                    "110101011000111011001100100011011100110001010010111101000000111001011110100101"
                    "100111010000110001001110111011010000111101011000100011010100110011001010111101"
                    "111101000000100101011000100011001000100001011010100111001000100001001010111101"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "000010111111010010101101000100111001011110111111000110101101010000111101000110"
                    "001110110101000110100111011110101101011110111001010010101111001100101111010100"
                    "000100100101000010110011011010111011010100100001011100111001001110101111010100"
                    "001100101001001010101001011110111001011100110111000100110001001100100101010100"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "100101011110110001001000100111000010100001000000100001001110110111011100100011"
                    "101001011110101001001000101001001010111101010000101101011100111111011110100111"
                    "101011000100110111001110111101000000111011010010100001011110111001010000111001"
                    "110011010110111111000000110001011110110011010110101001011110110111000000110101"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "000100111111010000101111001010101111001000101111001100101011010110111111000000"
                    "011100100111000000100101000100100001011110101101011010111011011010111111011010"
                    "010010110001011100100101010000101001001100110011001010101011000100101011011110"
                    "011010100001000000100011010100111011001110110001001110111111001110111001000010"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "100001011000110001010110110011010100110101010100110101010010110101011100100101"
                    "111011010100100011011110111111010100101111011000110001011100111011010010100011"
                    "100001011110110101011010100001011010100001011110101111000110100101011100111011"
                    "100111011100111001010100110011000000111011011100111011010000110111010110110111"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "000100111111011110111011011110111001011110111111011110111101011110111001000110"
                    "010100111111000000100001011110101101011010111111000000111101011110100001011010"
                    "000000101001010000110111000100110001010010101011000000110111001110100001010110"
                    "001100110111000000101111011110100001001110111111000000110101010000100001010010"
                    "000000111111000000111111000000111111000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
                    "100111000010100111000110100001000000100011000000100111000110100001000100100111"
                    "101101010110100111000010111111001000100011001010101011011110110011011100100101"
                    "101001001000110101000110111011010000100111010100110101000100101111001100111011"
                    "110011010110101111001110110111010000100111010010110111000100111111000010100001"
                    "111111000000111111000000111111000000111111000000111111000000111111000000111111"
               },
        /*  5*/ { DATA_MODE, -1, -1, { 0, 0, "" }, { { TU("\266"), 1, 0 }, { TU("\266"), 1, 7 }, { TU("\266"), 1, 0 } }, 0, 30, 30, "Standard example + extra seg, data mode (ECIs 0, 7, 29)",
                    "111111000000111111000000111111"
                    "111111011110111111011110111111"
                    "101111011110111111011110111111"
                    "101111000000100001000000100001"
                    "101011000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011100110111010000110001011110"
                    "010010111111000110111101011110"
                    "011100100001000110110001000000"
                    "010110100001001100100001000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111101010000101001010000111111"
                    "100101000000100001000100111111"
                    "100001001100110111011000100001"
                    "110101000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011000110001010100110001011000"
                    "001110110111010110111001011010"
                    "000110100001000000101011000010"
                    "010100100001000110110001010100"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111011011000111101011100111011"
                    "100001001100100101010110101101"
                    "101111000000110001001000111001"
                    "100111010100111001001110101011"
                    "111111000000111111000000111111"
               },
        /*  6*/ { UNICODE_MODE, -1, -1, { 1, 16, "" }, { { TU("齄齄"), -1, 29 }, { TU("Τεχτ"), -1, 0 }, { TU("Text"), -1, 0 } }, 0, 30, 30, "Structured Append (ECIs 29, 0, 0)",
                    "111111000000111111000000111111"
                    "111001011010111011011000111111"
                    "111011011110111101000000111111"
                    "101111010010100001001110100001"
                    "110111000100110011010110100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011100110111010110110101011110"
                    "010000110001000100101011011110"
                    "010010101111011000111111000000"
                    "011000111111001110101111000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111001010000101001010110111111"
                    "110101000100101111011000111111"
                    "111001001100101101001100100001"
                    "110001000010100001011110100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "011100110101010010110001011010"
                    "011010100011000000101111011100"
                    "001100100101001000110001000010"
                    "001100110001011000100101011100"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "111001011000111011011100111101"
                    "111011011010111111001110111101"
                    "101111001100111001001000101011"
                    "101101010000100101000000110001"
                    "111111000000111111000000111111"
               },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[8192];

    testStartSymbol("test_encode_segs", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, data[i].input_mode, -1 /*eci*/,
                            data[i].option_1, data[i].option_2, -1, -1 /*output_options*/,
                            NULL, 0, debug);
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            char escaped1[8192];
            char escaped2[8192];
            int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
            int length1 = data[i].segs[1].length == -1 ? (int) ustrlen(data[i].segs[1].source) : data[i].segs[1].length;
            int length2 = data[i].segs[2].length == -1 ? (int) ustrlen(data[i].segs[2].source) : data[i].segs[2].length;
            printf("        /*%3d*/ { %s, %d, %d, { %d, %d, \"%s\" }, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("               },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITERATIONS    1000

/* Not a real test, just performance indicator */
static void test_perf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, -1, -1,
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738",
                    0, 66, 66, "97 chars, mixed modes" },
        /*  1*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, -1, -1,
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738",
                    0, 162, 162, "970 chars, mixed modes" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start, total_encode = 0, total_buffer = 0, diff_encode, diff_buffer;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_encode = diff_buffer = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

            start = clock();
            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

            ZBarcode_Delete(symbol);
        }

        printf("%s: diff_encode %gms, diff_buffer %gms\n", data[i].comment, diff_encode * 1000.0 / CLOCKS_PER_SEC, diff_buffer * 1000.0 / CLOCKS_PER_SEC);

        total_encode += diff_encode;
        total_buffer += diff_buffer;
    }
    if (p_ctx->index != -1) {
        printf("totals: encode %gms, buffer %gms\n", total_encode * 1000.0 / CLOCKS_PER_SEC, total_buffer * 1000.0 / CLOCKS_PER_SEC);
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_options", test_options },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_encode_segs", test_encode_segs },
        { "test_perf", test_perf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et norl : */
