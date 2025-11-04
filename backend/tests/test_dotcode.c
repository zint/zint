/*
    libzint - the open source barcode library
    Copyright (C) 2019-2025 Robin Stuart <rstuart114@gmail.com>

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
        char datum;
        int length;
        int ret;
        const char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 200, '0', 2940, 0, "" }, /* 2940 largest Code Set C data that fits in 200x199 HxW */
        /*  1*/ { 200, '0', 2941, ZINT_ERROR_INVALID_OPTION, "Error 735: Resulting symbol height '201' is too large (maximum 200)" },
        /*  2*/ { 200, '9', 200, 0, "" }, /* Changes a number of mask scores re pre-Rev. 4 version, but best score still the same (7) */
        /*  3*/ { 200, '0', 2974, ZINT_ERROR_INVALID_OPTION, "Error 735: Resulting symbol height '203' is too large (maximum 200)" }, /* Width > 200 also */
        /*  4*/ { 200, 'A', 1470, 0, "" },
        /*  5*/ { 200, 'A', 1471, ZINT_ERROR_INVALID_OPTION, "Error 735: Resulting symbol height '201' is too large (maximum 200)" },
        /*  6*/ { 200, '\240', 1225, 0, "" },
        /*  7*/ { 200, '\240', 1226, ZINT_ERROR_INVALID_OPTION, "Error 735: Resulting symbol height '201' is too large (maximum 200)" },
        /*  8*/ { 200, '0', 1, 0, "" }, /* Padding codewords 35 - probably max */
        /*  9*/ { 200, '0', 2, 0, "" }, /* Padding codewords 35 */
        /* 10*/ { 30, '\001', 71, 0, "" }, /* Codeword length 72, ECC length 39, for ND + 1 == 112 */
        /* 11*/ { -1, '0', 1968, 0, "" },
        /* 12*/ { -1, '0', 1969, ZINT_ERROR_INVALID_OPTION, "Error 528: Resulting symbol width '201' is too large (maximum 200)" },
        /* 13*/ { -1, 'A', 984, 0, "" },
        /* 14*/ { -1, 'A', 985, ZINT_ERROR_INVALID_OPTION, "Error 528: Resulting symbol width '201' is too large (maximum 200)" },
        /* 15*/ { -1, '\240', 820, 0, "" },
        /* 16*/ { -1, '\240', 821, ZINT_ERROR_INVALID_OPTION, "Error 528: Resulting symbol width '201' is too large (maximum 200)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        memset(data_buf, data[i].datum, data[i].length);

        length = testUtilSetSymbol(symbol, BARCODE_DOTCODE, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data_buf), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int output_options;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        const char *data;
        int ret;

        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
        int expected_option_2;
        int expected_option_3;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, -1, -1, -1, { 0, 0, "" }, "1", 0, 9, 14, "", 14, 1 << 8 },
        /*  1*/ { -1, -1, -1, -1, -1, { 0, 0, "" }, "1234567890", 0, 12, 19, "", 19, 2 << 8 },
        /*  2*/ { -1, -1, -1, 19, -1, { 0, 0, "" }, "1234567890", 0, 12, 19, "", 19, 2 << 8 },
        /*  3*/ { -1, -1, -1, 12, -1, { 0, 0, "" }, "1234567890", 0, 19, 12, "", 12, 2 << 8 },
        /*  4*/ { -1, -1, -1, 5, -1, { 0, 0, "" }, "1234567890", 0, 44, 5, "", 5, 6 << 8 },
        /*  5*/ { -1, -1, -1, 4, -1, { 0, 0, "" }, "1234567890", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 527: Number of columns '4' is out of range (5 to 200)", 4, 0 }, /* Cols < 5 */
        /*  6*/ { -1, -1, -1, 200, -1, { 0, 0, "" }, "1234567890", 0, 5, 200, "", 200, 7 << 8 }, /* Note used to fail - now sets  height to at least 5 */
        /*  7*/ { -1, -1, -1, 200, -1, { 0, 0, "" }, "1234567890123456789012345678901234567890", 0, 5, 200, "", 200, 7 << 8 }, /* Cols 200 max */
        /*  8*/ { -1, -1, -1, 200, -1, { 0, 0, "" }, "12345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, 7, 200, "", 200, 4 << 8 },
        /*  9*/ { -1, -1, -1, 201, -1, { 0, 0, "" }, "12345678901234567890123456789012345678901234567890123456789012345678901234567890", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 527: Number of columns '201' is out of range (5 to 200)", 201, 0 },
        /* 10*/ { -1, -1, -1, -1, 10 << 8, { 0, 0, "" }, "1", 0, 9, 14, "", 14, 1 << 8 }, /* Mask > 8 + 1 ignored */
        /* 11*/ { -1, -1, -1, -1, 8 << 8, { 0, 0, "" }, "1", 0, 9, 14, "", 14, 8 << 8 },
        /* 12*/ { -1, -1, -1, 19, -1, { 0, 0, "" }, "ABCDE", 0, 12, 19, "", 19, 3 << 8 },
        /* 13*/ { -1, -1, -1, 19, -1, { 35, 35, "" }, "ABCDE", 0, 16, 19, "", 19, 2 << 8 },
        /* 14*/ { -1, -1, -1, 19, -1, { 1, 1, "" }, "ABCDE", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 730: Structured Append count '1' out of range (2 to 35)", 19, 0 },
        /* 15*/ { -1, -1, -1, 19, -1, { 1, 36, "" }, "ABCDE", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 730: Structured Append count '36' out of range (2 to 35)", 19, 0 },
        /* 16*/ { -1, -1, -1, 19, -1, { 3, 2, "" }, "ABCDE", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 731: Structured Append index '3' out of range (1 to count 2)", 19, 0 },
        /* 17*/ { -1, -1, -1, 19, -1, { 1, 2, "1" }, "ABCDE", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 732: Structured Append ID not available for DotCode", 19, 0 },
        /* 18*/ { GS1_MODE, 3, -1, -1, -1, { 0, 0, "" }, "[20]01", ZINT_WARN_NONCOMPLIANT, 11, 16, "Warning 733: Using ECI in GS1 mode not supported by GS1 standards", 16, 2 << 8 },
        /* 19*/ { GS1_MODE, -1, -1, -1, -1, { 1, 2, "" }, "[20]01", ZINT_WARN_NONCOMPLIANT, 12, 19, "Warning 734: Using Structured Append in GS1 mode not supported by GS1 standards", 19, 4 << 8 },
        /* 20*/ { GS1_MODE, 3, -1, -1, -1, { 1, 2, "" }, "[20]01", ZINT_WARN_NONCOMPLIANT, 14, 21, "Warning 733: Using ECI in GS1 mode not supported by GS1 standards", 21, 3 << 8 }, /* ECI trumps Structured Append */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char option_3_buf[64];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_DOTCODE, data[i].input_mode, data[i].eci, -1 /*option_1*/, data[i].option_2, data[i].option_3, data[i].output_options, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, symbol->errtxt);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, symbol->errtxt);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);
        assert_equal(symbol->option_2, data[i].expected_option_2, "i:%d symbol->option_2 %d != %d\n",
                    i, symbol->option_2, data[i].expected_option_2);
        strcpy(option_3_buf, testUtilOption3Name(BARCODE_DOTCODE, symbol->option_3)); /* Copy static buffer */
        assert_equal(symbol->option_3, data[i].expected_option_3, "i:%d symbol->option_3 0x%04X (%s) != 0x%04X (%s)\n",
                    i, symbol->option_3, option_3_buf,
                    data[i].expected_option_3, testUtilOption3Name(BARCODE_DOTCODE, data[i].expected_option_3));

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        const char *data;
        int length;
        int ret;
        const char *expected;
        int bwipp_cmp;
        int zxingcpp_cmp;
        const char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "A", -1, 0, "66 21 6A", 1, 1, "" },
        /*  1*/ { UNICODE_MODE, 3, -1, -1, { 0, 0, "" }, "A", -1, 0, "6C 03 66 21", 1, 1, "" },
        /*  2*/ { UNICODE_MODE, 40, 18, -1, { 0, 0, "" }, "A", -1, 0, "6C 28 00 00 66 21", 1, 1, "" },
        /*  3*/ { UNICODE_MODE, 113, 18, -1, { 0, 0, "" }, "A", -1, 0, "6C 28 00 49 66 21", 1, 1, "" },
        /*  4*/ { UNICODE_MODE, 899, 18, -1, { 0, 0, "" }, "A", -1, 0, "6C 28 07 44 66 21", 1, 1, "" },
        /*  5*/ { UNICODE_MODE, 12769, 18, 8 << 8, { 0, 0, "" }, "A", -1, 0, "6C 28 70 49 66 21", 1, 1, "" },
        /*  6*/ { UNICODE_MODE, 811799, 18, -1, { 0, 0, "" }, "A", -1, 0, "6C 67 40 50 66 21", 1, 1, "" },
        /*  7*/ { UNICODE_MODE, 811800, -1, -1, { 0, 0, "" }, "A", -1, ZINT_ERROR_INVALID_OPTION, "Error 525: ECI code '811800' out of range (0 to 811799)", 1, 1, "" },
        /*  8*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "\000", 1, 0, "65 40 6A", 1, 1, "LatchA (0x65) NUL PAD" },
        /*  9*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "\010", -1, 0, "65 48 6A", 1, 1, "LatchA (0x65) BS PAD" },
        /* 10*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "\011", -1, 0, "65 49 6A", 1, 1, "Lead special; LatchA (0x65) HT PAD" },
        /* 11*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "\034", -1, 0, "65 5C 6A", 1, 1, "Lead special; LatchA (0x65) FS PAD" },
        /* 12*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "\035", -1, 0, "65 5D 6A", 1, 1, "Lead special; LatchA (0x65) GS PAD" },
        /* 13*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "\036", -1, 0, "65 5E 6A", 1, 1, "Lead special; LatchA (0x65) RS PAD" },
        /* 14*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "\037", -1, 0, "65 5F 6A", 1, 1, "LatchA (0x65) US PAD" },
        /* 15*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "\177", -1, 0, "66 5F 6A", 1, 1, "ShiftB (0x66) DEL PAD" },
        /* 16*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "[)>\03605\035A\036\004", -1, 0, "6A 61 21", 1, 1, "[)>RS 05 GS A RS EOT; LatchB (0x6A) Macro97 (0x61) A" },
        /* 17*/ { UNICODE_MODE, -1, 17, -1, { 0, 0, "" }, "[)>\03606\035\011\034\035\036\036\004", -1, 0, "6A 62 61 62 63 64 6A", 1, 1, "[)>RS 06 GS HT FS GS RS RS EOT; LatchB (0x6A) Macro98 (0x62) HT FS GS RS PAD" },
        /* 18*/ { UNICODE_MODE, -1, 17, -1, { 0, 0, "" }, "[)>\03612\03512345\036\004", -1, 0, "6A 63 11 67 17 2D 6A", 1, 1, "[)>RS 12 GS A RS EOT; LatchB (0x6A) Macro99 (0x63) 1 2xShiftC (0x67) 23 45 PAD" },
        /* 19*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "[)>\03601Blah\004", -1, 0, "6A 64 10 11 22 4C 41 48 6A", 1, 1, "[)>RS 01 Blah EOT; LatchB (0x6A) Macro100 (0x64) 0 1 B l a h PAD" },
        /* 20*/ { UNICODE_MODE, -1, 22, -1, { 0, 0, "" }, "[)>\03605\035A\004", -1, 0, "65 3B 09 1E 5E 10 15 5D 21 44", 1, 1, "NOTE: no longer using Macro for malformed 05/06/12" },
        /* 21*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "[)>\03606A\004", -1, 0, "65 3B 09 1E 5E 10 16 21 44", 1, 1, "NOTE: no longer using Macro for malformed 05/06/12" },
        /* 22*/ { UNICODE_MODE, -1, 13, -1, { 0, 0, "" }, "[)>\036991\036\004", -1, 0, "6A 64 19 19 11 64", 1, 1, "[)>RS 99 1 RS EOT; LatchB (0x6A) Macro100 (0x64) 9 9 1 RS" },
        /* 23*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "1712345610", -1, 0, "6B 64 0C 22 38", 1, 1, "FNC1 (0x6B) 17..10 12 34 56" },
        /* 24*/ { GS1_MODE, -1, -1, -1, { 0, 0, "" }, "[17]123456[10]123", -1, ZINT_WARN_NONCOMPLIANT, "64 0C 22 38 0C 66 13", 0, 1, "17..10 12 34 56 12 ShiftB (0x66) 3; BWIPP does not allow bad month" },
        /* 25*/ { GS1_MODE, -1, -1, -1, { 0, 0, "" }, "[90]ABC[90]abc[90]123", -1, 0, "5A 6A 21 22 23 6B 19 10 41 42 43 6B 19 67 01 17 6A", 1, 1, "90 LatchB (0x6A) A B C FNC1 (0x6B) 9 0 a b c FNC1 (0x6B) 9 2xShitfC (0x67) 01 23 PAD" },
        /* 26*/ { GS1_MODE | GS1PARENS_MODE, -1, -1, -1, { 0, 0, "" }, "(90)ABC(90)abc(90)123", -1, 0, "5A 6A 21 22 23 6B 19 10 41 42 43 6B 19 67 01 17 6A", 1, 1, "90 LatchB (0x6A) A B C FNC1 (0x6B) 9 0 a b c FNC1 (0x6B) 9 2xShitfC (0x67) 01 23 PAD" },
        /* 27*/ { GS1_MODE, -1, -1, -1, { 0, 0, "" }, "[90]AB)", -1, 0, "5A 68 21 22 09", 1, 1, "" },
        /* 28*/ { GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, -1, -1, { 0, 0, "" }, "(90)AB\\)", -1, 0, "5A 68 21 22 09", 1, 1, "" },
        /* 29*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "99aA[{00\000", 9, 0, "6B 63 6A 41 21 3B 5B 10 10 65 40", 1, 1, "FNC1 (0x6B) 99 LatchB (0x6A) a A [ { 0 0 ShiftA (0x65) NUL" },
        /* 30*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\015\012", -1, 0, "66 60", 0, 1, "ShiftB (0x66) CR/LF; BWIPP different encodation" },
        /* 31*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "A\015\012", -1, 0, "67 21 60", 0, 1, "2xShiftB (0x67) A CR/LF; BWIPP different encodation" },
        /* 32*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\015\015\012", -1, 0, "65 4D 4D 4A", 1, 1, "LatchA (0x65) CR CR LF" },
        /* 33*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "ABCDE12345678", -1, 0, "6A 21 22 23 24 25 69 0C 22 38 4E", 1, 1, "LatchB (0x6A) A B C D 4xShiftC 12 34 56 78" },
        /* 34*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\000ABCD1234567890", 15, 0, "65 40 21 22 23 24 6A 0C 22 38 4E 5A 6A", 1, 1, "LatchA (0x65) NULL A B C D LatchC (0x6A) 12 34 56 78 90 PAD" },
        /* 35*/ { DATA_MODE, -1, -1, 2 << 8, { 0, 0, "" }, "\141\142\143\144\145\200\201\202\203\204\377", -1, 0, "6A 41 42 43 44 45 70 31 5A 35 21 5A 5F 02 31", 1, 899, "LatchB (0x6A) a b c d e BinaryLatch (0x70) 0x80 0x81 0x82 0x83 0x84 0xFF" },
        /* 36*/ { DATA_MODE, -1, -1, -1, { 0, 0, "" }, "\200\061\062\240\063\064\201\202\065\066", -1, 0, "6E 40 0C 6F 00 22 70 03 10 42 6E 15 16", 1, 899, "UpperShiftA (0x6E) NUL 12 UpperShiftB (0x6F) SP 34 BinaryLatch (0x70) 0x81 0x82 TermB (0x6E) 5 6" },
        /* 37*/ { DATA_MODE, -1, -1, -1, { 0, 0, "" }, "\200\201\202\203\061\062\063\064", -1, 0, "70 13 56 0A 59 2C 67 0C 22", 1, 899, "BinaryLatch (0x70) 0x80 0x81 0x82 0x83 Intr2xShiftC (0x67) 12 3" },
        /* 38*/ { DATA_MODE, -1, -1, -1, { 0, 0, "" }, "\001\200\201\202\203\204\200\201\202\203\204", -1, 0, "65 41 70 31 5A 35 21 5A 5F 31 5A 35 21 5A 5F", 1, 899, "LatchA (0x65) SOH BinaryLatch (0x70) 0x80 0x81 0x82 0x83 0x80 0x81 0x82 0x83" },
        /* 39*/ { UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, "\001abc\011\015\012\036", -1, 0, "65 41 65 41 42 43 61 60 64", 1, 1, "LatchA (0x65) SOH 6xShiftB (0x65) a b c HT CR/LF RS" },
        /* 40*/ { UNICODE_MODE, -1, -1, -1, { 35, 35, "" }, "ABCDE", -1, 0, "6A 21 22 23 24 25 3A 3A 6C", 1, 1, "LatchB (0x6A) A B C D E Z Z FNC2" },
        /* 41*/ { UNICODE_MODE, -1, -1, -1, { 9, 10, "" }, "1234567890", -1, 0, "6B 0C 22 38 4E 5A 65 19 21 6C", 1, 1, "FNC1 (0x6B) 12 34 56 78 90 LatchA (0x65) 9 A FNC2" },
        /* 42*/ { UNICODE_MODE, -1, -1, -1, { 2, 3, "" }, "\001\002\003\004", -1, 0, "65 41 42 43 44 6A 12 13 6C", 1, 1, "LatchA (0x65) <SOH> <STX> <ETX> <EOT> PAD 2 3 FNC2" },
        /* 43*/ { DATA_MODE, -1, -1, -1, { 1, 34, "" }, "\200\201\202\203", -1, 0, "70 13 56 0A 59 2C 6D 11 39 6C", 1, 899, "BinaryLatch (0x70) (...) TermA (0x6D) 1 Y FNC2" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[32768];
    char cmp_msg[1024];

    /* Only do BWIPP/ZXing-C++ tests if asked, too slow otherwise */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_DOTCODE, data[i].input_mode, data[i].eci,
                    -1 /*option_1*/, data[i].option_2, data[i].option_3, -1 /*output_options*/,
                    data[i].data, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, { %d, %d, \"%s\" }, \"%s\", %d, %s, \"%s\", %d, %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_2, data[i].option_3,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    data[i].length, testUtilErrorName(data[i].ret), symbol->errtxt, data[i].bwipp_cmp,
                    data[i].zxingcpp_cmp, data[i].comment);
        } else {
            assert_zero(strcmp((char *) symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        char modules_dump[200 * 200 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                    i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[200 * 200 + 1];
                    assert_nonzero(data[i].zxingcpp_cmp, "i:%d data[i].zxingcpp_cmp == 0", i);
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, data[i].zxingcpp_cmp,
                                cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n",
                                i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length,
                                NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len,
                                escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        const char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        int zxingcpp_cmp;
        const char *comment;
        const char *expected;
    };
    /* ISS DotCode, Rev 4.0, DRAFT 0.15, TSC Pre-PR #5, MAY 28, 2019 */
    static const struct item data[] = {
        /*  0*/ { GS1_MODE, 64, -1, { 0, 0, "" }, "[01]00012345678905[17]201231[10]ABC123456", -1, 0, 9, 64, 1, 1, "ISS DotCode Rev 4.0 Figure 1 (left), same",
                    "1010000000101000101010000010000010001010100010101000101000001010"
                    "0100010001010001010001000001010100010100010001000100010101000001"
                    "1010001010000000101010100010001010000010101000000010100010100000"
                    "0000000101000101010001010100000001000100010001000100010100000001"
                    "1000001000000010100010101000001010100000101010100000100010001010"
                    "0101000101000001000001010001010100000001000100010101000101010001"
                    "1000000010100010001000001000100010101000100010000010101000100000"
                    "0001010100010001010100010001010000010001010000000101010001010101"
                    "1000100010001000100010100010001010001000101000101000100010000010"
                },
        /*  1*/ { GS1_MODE, -1, -1, { 0, 0, "" }, "[01]00012345678905[17]201231[10]ABC123456", -1, 0, 20, 29, 1, 1, "ISS DotCode Rev 4.0 Figure 1 (right) (and Figure 10), same",
                    "10101000101010100010101000101"
                    "00010100010100010100000001010"
                    "00001010100010000000101010000"
                    "01000001010101010101010101000"
                    "10101010100000000010100010101"
                    "01000100000000000101010101010"
                    "00100010001010101000100000101"
                    "00000101010000000001010000010"
                    "10001010100010101010001010100"
                    "01010000000000010000000100010"
                    "00100000000010100000100000000"
                    "00010000000101000001000001000"
                    "10101000101000001010001010001"
                    "01010001010001010101000000010"
                    "00001000001010001010100000101"
                    "01000100010100000000010100010"
                    "10100010000010101000101010001"
                    "00010101000001010100010100010"
                    "10000010101000100000001000001"
                    "01000100010101010000000101010"
                },
        /*  2*/ { GS1_MODE, -1, 1 << 8, { 0, 0, "" }, "[17]070620[10]ABC123456", -1, 0, 16, 23, 1, 1, "ISS DotCode Rev 4.0 Figure 5 (and Figure 6 top-left) when Mask = 0, same",
                    "10101000100010000000001"
                    "01000101010001010000000"
                    "00100010001000101000100"
                    "01010001010000000101010"
                    "00001000100010100010101"
                    "00000101010101010000010"
                    "00100010101000000010001"
                    "00010100000101000100010"
                    "00001000001000001010101"
                    "01010101010001000001010"
                    "10100000100010001000101"
                    "01000100000100010101000"
                    "10000010000010100010001"
                    "00010000010100010101010"
                    "10101000001000101010001"
                    "01000001010101010000010"
                },
        /*  3*/ { GS1_MODE, -1, 2 << 8, { 0, 0, "" }, "[17]070620[10]ABC123456", -1, 0, 16, 23, 1, 1, "ISS DotCode Rev 4.0 Figure 6 top-right Mask = 1, same",
                    "10000000001010001000101"
                    "01010101000100000101000"
                    "00100010000000100000001"
                    "01010001000001000001000"
                    "10101010100000001010101"
                    "00000100010100000100010"
                    "00000000001010101010001"
                    "00010001010001000001000"
                    "00101010101000001010001"
                    "01000100000001010000000"
                    "10101000101000101000001"
                    "00010101000100010101010"
                    "10001000001010100000101"
                    "01010001010001000001010"
                    "10000010101010100010101"
                    "01000101000101010101010"
                },
        /*  4*/ { GS1_MODE, -1, 3 << 8, { 0, 0, "" }, "[17]070620[10]ABC123456", -1, 0, 16, 23, 1, 1, "ISS DotCode Rev 4.0 Figure 6 bottom-left Mask = 2, same",
                    "10100000101010100010001"
                    "01000101000100000000010"
                    "10101010001010000010000"
                    "01010100010000010101010"
                    "00001000101000001000101"
                    "00000000000001010000010"
                    "00100010101010101000001"
                    "00010101010100010000000"
                    "00001000100010101010001"
                    "01000000010101010101000"
                    "10100010100000101000101"
                    "00000000000101000001010"
                    "10000010000010100010101"
                    "01010100010100010001010"
                    "10101010000000001010001"
                    "01010101000001000101010"
                },
        /*  5*/ { GS1_MODE, -1, 4 << 8, { 0, 0, "" }, "[17]070620[10]ABC123456", -1, 0, 16, 23, 1, 1, "ISS DotCode Rev 4.0 Figure 6 bottom-right Mask = 3, same",
                    "10000000100000001010101"
                    "01010001010100010001000"
                    "10001000001010101010100"
                    "01010101000101010000010"
                    "10101010001000000010101"
                    "00000100000100010101000"
                    "00001000101010101000101"
                    "00000001010000000101010"
                    "00100010000000000000001"
                    "01010100010101010101010"
                    "10000000101010100010001"
                    "01010101000001010000010"
                    "10101010100000001000001"
                    "01000001010001000001010"
                    "10001000001010001000001"
                    "01010100000101000100010"
                },
        /*  6*/ { GS1_MODE, -1, -1, { 0, 0, "" }, "[17]070620[10]ABC123456", -1, 0, 16, 23, 1, 1, "ISS DotCode Rev 4.0 Figure 6 top-right, auto Mask = 1, same",
                    "10000000001010001000101"
                    "01010101000100000101000"
                    "00100010000000100000001"
                    "01010001000001000001000"
                    "10101010100000001010101"
                    "00000100010100000100010"
                    "00000000001010101010001"
                    "00010001010001000001000"
                    "00101010101000001010001"
                    "01000100000001010000000"
                    "10101000101000101000001"
                    "00010101000100010101010"
                    "10001000001010100000101"
                    "01010001010001000001010"
                    "10000010101010100010101"
                    "01000101000101010101010"
                },
        /*  7*/ { UNICODE_MODE, -1, 1 << 8, { 0, 0, "" }, "2741", -1, 0, 10, 13, 0, 0, "ISS DotCode Rev 4.0 Figure 7A top-left Mask = 0, same; BWIPP automatically primes mask; ZXing-C++ can't handle zero row",
                    "1010101010100"
                    "0000010001010"
                    "0000101000101"
                    "0101000000000"
                    "0000101010100"
                    "0100010101000"
                    "1000001000001"
                    "0101000101010"
                    "1000100010001"
                    "0000000000000"
                },
        /*  8*/ { UNICODE_MODE, -1, 2 << 8, { 0, 0, "" }, "2741", -1, 0, 10, 13, 0, 1, "ISS DotCode Rev 4.0 Figure 7A top-right Mask = 1, same; BWIPP automatically primes mask",
                    "1010001000101"
                    "0000000100010"
                    "0000100000001"
                    "0101010001000"
                    "1000101000000"
                    "0101010101010"
                    "1000101000101"
                    "0100010101010"
                    "0000000010001"
                    "0001000001000"
                },
        /*  9*/ { UNICODE_MODE, -1, 3 << 8, { 0, 0, "" }, "2741", -1, 0, 10, 13, 0, 1, "ISS DotCode Rev 4.0 Figure 7A bottom-left Mask = 2, same; BWIPP automatically primes mask",
                    "1010001010100"
                    "0001000000000"
                    "1000100010101"
                    "0100000101000"
                    "0000101000100"
                    "0100010000010"
                    "1000101010001"
                    "0101010001000"
                    "1000100010101"
                    "0001000100000"
                },
        /* 10*/ { UNICODE_MODE, -1, 4 << 8, { 0, 0, "" }, "2741", -1, 0, 10, 13, 0, 1, "ISS DotCode Rev 4.0 Figure 7A bottom-right Mask = 3, same; BWIPP automatically primes mask",
                    "1010001000100"
                    "0001000001010"
                    "1000001000000"
                    "0101000100010"
                    "1000101010100"
                    "0101010000010"
                    "1000100000000"
                    "0100000101000"
                    "1000001010001"
                    "0101010101010"
                },
        /* 11*/ { UNICODE_MODE, -1, 5 << 8, { 0, 0, "" }, "2741", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Figure 7B top-left Mask = 0' (4), same",
                    "1010101010101"
                    "0000010001010"
                    "0000101000101"
                    "0101000000000"
                    "0000101010100"
                    "0100010101000"
                    "1000001000001"
                    "0101000101010"
                    "1000100010001"
                    "0100000000010"
                },
        /* 12*/ { UNICODE_MODE, -1, 6 << 8, { 0, 0, "" }, "2741", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Figure 7B top-right Mask = 1' (5), same",
                    "1010001000101"
                    "0000000100010"
                    "0000100000001"
                    "0101010001000"
                    "1000101000000"
                    "0101010101010"
                    "1000101000101"
                    "0100010101010"
                    "1000000010001"
                    "0101000001010"
                },
        /* 13*/ { UNICODE_MODE, -1, 7 << 8, { 0, 0, "" }, "2741", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Figure 7B bottom-left Mask = 2' (6), same",
                    "1010001010101"
                    "0001000000000"
                    "1000100010101"
                    "0100000101000"
                    "0000101000100"
                    "0100010000010"
                    "1000101010001"
                    "0101010001000"
                    "1000100010101"
                    "0101000100010"
                },
        /* 14*/ { UNICODE_MODE, -1, 8 << 8, { 0, 0, "" }, "2741", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Figure 7B bottom-right Mask = 3' (7), same",
                    "1010001000101"
                    "0001000001010"
                    "1000001000000"
                    "0101000100010"
                    "1000101010100"
                    "0101010000010"
                    "1000100000000"
                    "0100000101000"
                    "1000001010001"
                    "0101010101010"
                },
        /* 15*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "2741", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Figure 7B bottom-left auto Mask = 2' (6), same",
                    "1010001010101"
                    "0001000000000"
                    "1000100010101"
                    "0100000101000"
                    "0000101000100"
                    "0100010000010"
                    "1000101010001"
                    "0101010001000"
                    "1000100010101"
                    "0101000100010"
                },
        /* 16*/ { GS1_MODE, 40, -1, { 0, 0, "" }, "[01]00012345678905", -1, 0, 7, 40, 1, 1, "ISS DotCode Rev 4.0 Figure 8 top-left 7x40, Mask = 1, same",
                    "1010101010001000100010100010101000001000"
                    "0000010101000100010100010000010001000001"
                    "1010001000001000001000101010001000101000"
                    "0001010101000000010100010001000001010001"
                    "1010100010001010000010001010000000101010"
                    "0001010001010001000100000001010100010001"
                    "1000100010001000100010100010001010001000"
                },
        /* 17*/ { GS1_MODE, 18, -1, { 0, 0, "" }, "[01]00012345678905", -1, 0, 17, 18, 1, 1, "ISS DotCode Rev 4.0 Figure 8 top-right 17x18 **NOT SAME** no matter what mask; but same as BWIPP and verified manually against tec-it",
                    "101000001000101010"
                    "010100000101010001"
                    "000000101000001010"
                    "000100010101000101"
                    "001010000000100010"
                    "010100000100010101"
                    "100010001000001010"
                    "010001000100010100"
                    "001000001010000010"
                    "010100000001010001"
                    "000000101010001010"
                    "000101000001000101"
                    "100010001010100010"
                    "000100010000000101"
                    "100010001010001010"
                    "010001010001000101"
                    "100010001000100010"
                },
        /* 18*/ { GS1_MODE, 35, -1, { 0, 0, "" }, "[01]00012345678905", -1, 0, 8, 35, 1, 1, "ISS DotCode Rev 4.0 Figure 8 bottom-left 8x35, Mask = 3, same",
                    "10100010000000000010100000100010101"
                    "00010101010001000000010100010100000"
                    "10001000101010101010001010000010101"
                    "01010001000100000101000100010101010"
                    "10101000100000101000100010001000001"
                    "00010100010000010001010001010000000"
                    "10000010101010101010000010000010001"
                    "01000001000101000100010100010001000"
                },
        /* 19*/ { GS1_MODE, 17, -1, { 0, 0, "" }, "[01]00012345678905", -1, 0, 18, 17, 1, 1, "ISS DotCode Rev 4.0 Figure 8 bottom-right 18x17 **NOT SAME** no matter what mask; same as BWIPP; verified manually against tec-it",
                    "10101000001000001"
                    "01000001010100010"
                    "00000000100010001"
                    "00010101000101010"
                    "10101000001010000"
                    "01000100010000000"
                    "00000010000000100"
                    "01010000000001000"
                    "10101010101000101"
                    "00000000010101010"
                    "00101010100000000"
                    "01000101000001010"
                    "10001000000010001"
                    "00000001010100010"
                    "00100010001000101"
                    "01010100010101000"
                    "10101010101010101"
                    "01010101000101010"
                },
        /* 20*/ { UNICODE_MODE, 35, -1, { 0, 0, "" }, "Dots can be Square!", -1, 0, 18, 35, 1, 1, "ISS DotCode Rev 4.0 Figure 11 **NOT SAME**; same as BWIPP; verified manually against tec-it",
                    "10000010101000000000000000101010101"
                    "01010101000101000100010100000001000"
                    "00001000000010101000101010101010000"
                    "01000001000100000001010001000000000"
                    "00100010101010101000000010100000101"
                    "00000001010001010101010100010101000"
                    "10101000100010000010101010001010001"
                    "00010001010100010101000101000101010"
                    "00101010001000001010101000100000101"
                    "00010100010001010000000001010001010"
                    "00101000100010100000100000000000101"
                    "01010100010001010100010100000100000"
                    "10000010000000000010001000101010100"
                    "00010000000100010001000000010001010"
                    "10001000001010101010001010101000001"
                    "01000101010101000100000100010101000"
                    "10101000101000001000100010101000101"
                    "01000001000001000101010001000000010"
                },
        /* 21*/ { GS1_MODE, -1, 1 << 8, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 0, 0, "ISS DotCode Rev 4.0 Table G.1 Mask 0, same; BWIPP automatically primes mask; ZXing-C++ can't handle zero col",
                    "0000001010000"
                    "0001010000010"
                    "0000000010001"
                    "0100010101000"
                    "0010101000101"
                    "0100010101010"
                    "0010000010000"
                    "0101010000010"
                    "0010000000101"
                    "0101000101010"
                },
        /* 22*/ { GS1_MODE, -1, 2 << 8, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 0, 1, "ISS DotCode Rev 4.0 Table G.1 Mask 1, same; BWIPP automatically primes mask",
                    "0000100000001"
                    "0001010000000"
                    "0000000000001"
                    "0101010000010"
                    "1010101010101"
                    "0100000101010"
                    "0010000010100"
                    "0100010101000"
                    "0010101000101"
                    "0100010101000"
                },
        /* 23*/ { GS1_MODE, -1, 3 << 8, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 0, 1, "ISS DotCode Rev 4.0 Table G.1 Mask 2, same; BWIPP automatically primes mask",
                    "0000100010100"
                    "0000000000000"
                    "1000101010101"
                    "0100010101010"
                    "0010101000101"
                    "0101010101010"
                    "0010100000000"
                    "0101010100000"
                    "0000000010001"
                    "0100000001010"
                },
        /* 24*/ { GS1_MODE, -1, 4 << 8, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 0, 0, "ISS DotCode Rev 4.0 Table G.1 Mask 3, same; BWIPP automatically primes mask; ZXing-C++ can't handle zero col",
                    "0000000000000"
                    "0001010001000"
                    "1000001010000"
                    "0101010100010"
                    "1010101000101"
                    "0101010101010"
                    "0010001000101"
                    "0101010101010"
                    "1000000010000"
                    "0100000000010"
                },
        /* 25*/ { GS1_MODE, -1, 5 << 8, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Table G.1 Mask 0' (4), same",
                    "1000001010001"
                    "0001010000010"
                    "0000000010001"
                    "0100010101000"
                    "0010101000101"
                    "0100010101010"
                    "0010000010000"
                    "0101010000010"
                    "1010000000101"
                    "0101000101010"
                },
        /* 26*/ { GS1_MODE, -1, 6 << 8, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Table G.1 Mask 1' (5), same",
                    "1000100000001"
                    "0001010000000"
                    "0000000000001"
                    "0101010000010"
                    "1010101010101"
                    "0100000101010"
                    "0010000010100"
                    "0100010101000"
                    "1010101000101"
                    "0100010101010"
                },
        /* 27*/ { GS1_MODE, -1, 7 << 8, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Table G.1 Mask 2' (6), same",
                    "1000100010101"
                    "0000000000000"
                    "1000101010101"
                    "0100010101010"
                    "0010101000101"
                    "0101010101010"
                    "0010100000000"
                    "0101010100000"
                    "1000000010001"
                    "0100000001010"
                },
        /* 28*/ { GS1_MODE, -1, 8 << 8, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Table G.1 Mask 3' (7), same",
                    "1000000000001"
                    "0001010001000"
                    "1000001010000"
                    "0101010100010"
                    "1010101000101"
                    "0101010101010"
                    "0010001000101"
                    "0101010101010"
                    "1000000010001"
                    "0100000000010"
                },
        /* 29*/ { GS1_MODE, -1, -1, { 0, 0, "" }, "[99]8766", -1, 0, 10, 13, 1, 1, "ISS DotCode Rev 4.0 Table G.1 auto Mask 0' (4); all mask scores match Table G.1",
                    "1000001010001"
                    "0001010000010"
                    "0000000010001"
                    "0100010101000"
                    "0010101000101"
                    "0100010101010"
                    "0010000010000"
                    "0101010000010"
                    "1010000000101"
                    "0101000101010"
                },
        /* 30*/ { UNICODE_MODE, 6, -1, { 0, 0, "" }, "A", -1, 0, 19, 6, 1, 1, "ISS DotCode Rev 4.0 5.2.1.4 2) Table 4, 1 padding dot available; verified manually against tec-it",
                    "101000"
                    "000101"
                    "101010"
                    "000001"
                    "100010"
                    "000100"
                    "001010"
                    "000101"
                    "101000"
                    "010000"
                    "100010"
                    "010000"
                    "000010"
                    "000101"
                    "101010"
                    "000001"
                    "101000"
                    "010000"
                    "101000"
                },
        /* 31*/ { UNICODE_MODE, 94, -1, { 0, 0, "" }, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRS", -1, 0, 37, 94, 1, 1, "Interleaved R-S; verified manually against tec-it",
                    "1000001010000000100010000010101010101000001000100000001010101000001000001000101010001000101010"
                    "0101010000000101000001010001010001010100010001000001000000010101010000000101010100010001010100"
                    "0010101000100010000010101010000000101010000010101000001000100010100000100010100010001000101000"
                    "0000000100010101000001010000010000010101010100010100000100000101000100010001000001010001010001"
                    "0010100000100010101000001000101010000010001000001010100000101000101010000010001000101010100010"
                    "0100000101010001010001000101000001000100000101010001000101000100010100000100010100010001010100"
                    "1010000000100010100000101010000010101000001000001010001010000010001010100000101010100000100000"
                    "0001010100000101000001000000010001010100000101000100010101000001000101000100010000010101010001"
                    "1010001010001010000000100000101010100000100010101000000010100010001010100010000010100010001010"
                    "0001000100010100010001010000000101010100010000010100000100000101010100010000010100010001000001"
                    "1000001010001010100010000010001000100010100010100010000010001000101010100000101000001000101010"
                    "0101000001000101000100000001010001010101010001000001000000010001010100010101010000010100000001"
                    "0000100000001010100010101000000010001010001010000000101010001000101010001000101000100000100010"
                    "0000000100000100000101010101000000010101010001000100010001000101010001000100010000010100010101"
                    "0010101000001010001000101000000000101010001010000010101010000010001000100000001010101010001010"
                    "0100010001010100010000010100010001010100010000010001000100000100010101000101000100000101010001"
                    "1000001000100010101010101000000010001000001010000010101010000010101010000000100010001000101000"
                    "0000010000010101010000010101000000010101000001000101000100010001000101010001000100010000010100"
                    "1000001000101010000010001000001000101010100000100000001010001010101000000010001010101010001000"
                    "0100010001010001000100010000010100000100010100010100010001000001010100010101000100000101000101"
                    "1010000000101000100000001000101010100010000000101010101010000010101000000010100010001000100010"
                    "0100000101010000010100010001010100010000010101000100000100000101000001010100010000000101010001"
                    "1000001000001010001010001000101000001000101010000010101000100010100000101000100000001000101010"
                    "0101010100010001010101000000010101010000010100000100010100010001000100000100010001010100010100"
                    "1000100000101000000010100010101000001010100010100000100000101010000010101000100000001010000000"
                    "0001000001000101000101010000010001000100000101010001000100010100000101000101000100000101010001"
                    "0000000010001000101000100000100000101010101000000010101010001000100010001000101010001000100010"
                    "0101010100010000000100010101010001010100000001010101000001000001000001010100000101010001010101"
                    "1000100010101000100000101010001000101010000010001010001010001010000010100010100010000000100000"
                    "0000010101010000010101000000010101000001000101000100010001000101010001000100010000010100010000"
                    "1000101010000010001000001000101010100000100000001010001010101000000010001010101010001000000010"
                    "0100000101000001010001010000000100010100010100010001000001010100010101000100000101000101010000"
                    "0000001010001000100010100010000010101000100010001000101000001000100000101000100010100010001010"
                    "0101010000010100010000010100000101010100000101000001000000010101000101000101010000000101010101"
                    "1000001010001010001000101000001000101010000010101000100010100000101000100000001000101010100000"
                    "0001000101010100000001010101000001010000010001010001000100010000010001000101010001010001000001"
                    "0010001000001010101000000010101000101000001000001010100000101010001000000010100000001010101000"
                },
        /* 32*/ { GS1_MODE, 50, -1, { 0, 0, "" }, "[17]070620[10]ABC123456", -1, 0, 7, 50, 1, 1, "GS1 Gen Spec Figure 5.1-8.",
                    "10000010101000100010101010001000000010100000100000"
                    "01000101000101010100000100010000010001000001010101"
                    "00001010001000101000101000100010001010100000000010"
                    "01000001000100000101010101010001000001000101000001"
                    "10001000001010100010001010100000100010100010000010"
                    "00010001010000000100010101000100010001010001000101"
                    "10001000001010101000001000100010100010100000101010"
                },
        /* 33*/ { UNICODE_MODE, 200, -1, { 0, 0, "" }, "123456789012345678901234567890123456789012345678901234567890", -1, 0, 5, 200, 1, 1, "Max cols",
                    "10101000100010101010000010101000000010001000100000101010100010100000101000100010000000101000101010001010100000100000101010100000001000101000001010100010001010000010001010001010100000100010101000000010"
                    "00010101010000000101000100010001000101000101000100010001000001010001000001010100000001000101010000000101010100010101010000010001000101010001000001000001010000010100010001010101000001000001010100000001"
                    "10100010000000100010101000101010100000001010001000100000101000101000001000101010001000000010101010100010101000000010100010001000001010100000101000100000101010100010000000001000001010101000101010100000"
                    "00010001010001010000000101000100010001010000010000010100010100000100010101010001000101000000010100010001010100010000010100000101000100010100000101010000000101000001010100010100010001000101000001010001"
                    "10100010001010101000000010001000001010001010001000001010100010000000101010001010000010101010000000101000100010100010100000100010100010001010100000001010101000001010000000001000101000101010000010101010"
                },
        /* 34*/ { UNICODE_MODE, 19, -1, { 0, 0, "" }, "4", -1, 0, 6, 19, 1, 1, "Mask 1 selected",
                    "1010100000101000101"
                    "0100000000010001010"
                    "0010101010000000000"
                    "0000010100000100000"
                    "1000100010001010001"
                    "0001010101010101010"
                },
        /* 35*/ { UNICODE_MODE, 19, 3 << 8, { 0, 0, "" }, "4", -1, 0, 6, 19, 0, 1, "Mask 2 specified, unlit right edge mask; BWIPP automatically primes mask",
                    "1010101010000000100"
                    "0000000101000100010"
                    "1010100000001010100"
                    "0000010101010000000"
                    "1000001010100010100"
                    "0101000100010001010"
                },
        /* 36*/ { UNICODE_MODE, 19, 7 << 8, { 0, 0, "" }, "4", -1, 0, 6, 19, 1, 1, "Mask 2' specified",
                    "1010101010000000101"
                    "0000000101000100010"
                    "1010100000001010100"
                    "0000010101010000000"
                    "1000001010100010101"
                    "0101000100010001010"
                },
        /* 37*/ { GS1_MODE, -1, -1, { 0, 0, "" }, "[10]12[20]12", -1, 0, 12, 17, 1, 1, "Code Set C with FNC1",
                    "10100000001000001"
                    "00010100010101010"
                    "10001000100000001"
                    "01010100000101000"
                    "10000010000010001"
                    "01010001010100010"
                    "00001010100000001"
                    "00000101010001010"
                    "10101000001010001"
                    "01010101000101010"
                    "10000010100000101"
                    "01000100010101010"
                },
        /* 38*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "1234\011\034\035\036", -1, 0, 14, 21, 1, 1, "Code Set B HT FS GS RS",
                    "100010001000001010101"
                    "010001000001000001000"
                    "100010100010101010001"
                    "010100000000010100010"
                    "101000100010001000000"
                    "010000010000000001000"
                    "100000101010100000000"
                    "000101010101010000000"
                    "001000001010101010101"
                    "000000000100010001010"
                    "101010000000000000101"
                    "000101010101000101010"
                    "001010101000101010001"
                    "010101010001000101000"
                },
        /* 39*/ { UNICODE_MODE, 17, -1, { 0, 0, "" }, "abcd\015\012", -1, 0, 14, 17, 1, 1, "Code Set B CRLF",
                    "00001000001000101"
                    "01000101010001000"
                    "10100000100010101"
                    "01000001010000010"
                    "00100010101000101"
                    "01010000010101000"
                    "10101010000010001"
                    "01000001000100010"
                    "00001010101010101"
                    "00000100010100010"
                    "00101010000000000"
                    "00010100010000000"
                    "10100000001010000"
                    "01010001000101000"
                },
        /* 40*/ { DATA_MODE, -1, -1, { 0, 0, "" }, "\101\102\103\104\105\106\107\200\101\102\240\101", -1, 0, 18, 27, 1, 899, "Code Set B Upper Shift A Upper Shift B",
                    "101010100000101000101000001"
                    "010100010101000100010101000"
                    "000010001010100000101010101"
                    "010100010100000001010101000"
                    "001010000010001010101000101"
                    "000001000001010101000000010"
                    "101000101010100000001000001"
                    "000101000001010101010100010"
                    "001010101000101010100010101"
                    "000101010001000100010001010"
                    "000000001000100000000000001"
                    "010000000000010100000100010"
                    "101010101010101010101010101"
                    "000101010001000100010101010"
                    "100000000000100000000010101"
                    "000000010100010000000100010"
                    "100010101000000010001000001"
                    "010001000100000101000001010"
                },
        /* 41*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "ABCDEF\001G1234H", -1, 0, 16, 25, 1, 1, "Code Set A 4x Shift C",
                    "0010101010100000100000101"
                    "0000000100010101000001000"
                    "1000100000101000100000101"
                    "0101000001010100000101010"
                    "0000000000101010001010000"
                    "0100000000010101010100000"
                    "0010101010000000101010101"
                    "0100010101000000010100000"
                    "1010001000100010001000101"
                    "0101010001010000000001010"
                    "0000001010100010001010101"
                    "0001000000000001010001000"
                    "0010100000101010100000000"
                    "0101010100000100010100010"
                    "1010101010001000101000000"
                    "0000010100010101000101010"
                },
        /* 42*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "ABCDEF\001ab\011\034\035\036\001A", -1, 0, 19, 28, 1, 1, "Code Set A 6x Shift B HT FS GS RS",
                    "1000001010100010101010101010"
                    "0101000000010100010101010001"
                    "0010001010100000101000001010"
                    "0001000101000001010001010000"
                    "1000001010100000101010001000"
                    "0100010101010001000000010001"
                    "0000001010100010100010000010"
                    "0000010100000001010100010101"
                    "0000001010001010101000001010"
                    "0100010101010101000001010000"
                    "0010000000101000101000000010"
                    "0101000101000101000101000001"
                    "1000100000001010101000001000"
                    "0101000001010000010001010100"
                    "1010000000100010001010100010"
                    "0101010100000001000100010001"
                    "1010000010100000100000101000"
                    "0001010000010100000100010101"
                    "1010100010000000101010101010"
                },
        /* 43*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "ABCDEF\001abcdefgA", -1, 0, 19, 28, 1, 1, "Code Set A Latch B",
                    "1010001010100010101010101010"
                    "0100010101010000000100010001"
                    "1010000010100000100010101000"
                    "0001010001000001010100000001"
                    "0010001010100000000010101010"
                    "0101000000010100010101000001"
                    "0000101010100000001000001010"
                    "0000010101010000000100010101"
                    "0000101010100000101000001010"
                    "0001010100010001010000000101"
                    "1010000010100010100000001010"
                    "0101000101000000010001010001"
                    "1000100000001010101000001000"
                    "0101000001010000010100010001"
                    "1010000000100010001010100010"
                    "0101010100000001000100010001"
                    "1010000010100000100000101000"
                    "0001010000010100000100010101"
                    "1010100010000000101010101010"
                },
        /* 44*/ { DATA_MODE, -1, -1, { 0, 0, "" }, "\200\200\200\200\061\062\063\064\065\066\067\070\071\060\061\062\063\064\065\066\200", -1, 0, 20, 29, 1, 899, "Binary Latch C",
                    "10101010000010100010101010001"
                    "01010001000101010001000000010"
                    "00001010101000101010001000001"
                    "00010100000001000101010100000"
                    "00100000101000100000000000101"
                    "00000001010101010100000101000"
                    "10101000001010000010101000100"
                    "01010100010000000101000000010"
                    "10101000101000001010100010100"
                    "00010101000101010001000101010"
                    "10000010101000100010101010001"
                    "01000001010100000001010001010"
                    "00001010101010101000000010001"
                    "00010100000000010000010101010"
                    "00101000100010001000101000001"
                    "00000100000101010101000000000"
                    "10000000000000001000000010101"
                    "01010001010001010000010101000"
                    "10000010100010000010001000101"
                    "01000100000100010001010101010"
                },
        /* 45*/ { UNICODE_MODE, -1, -1, { 11, 24, "" }, "ABCDEFG", -1, 0, 16, 23, 1, 1, "Structured Append",
                    "10101000001000101000001"
                    "01010101000001010001010"
                    "10101010100010000000001"
                    "01000001010000010100010"
                    "10000000101010100010101"
                    "01010100010101010101000"
                    "10000010101010101000101"
                    "00010100010100000000010"
                    "00101000100000001010001"
                    "00010100000001000100000"
                    "00100010000000001010101"
                    "01010101010001010101000"
                    "10000000001010000000001"
                    "00000000010100010001010"
                    "10001010101000100010001"
                    "01000001000100010101010"
                },
        /* 46*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "1234", -1, 0, 10, 13, 1, 1, "",
                    "0010100000001"
                    "0000000001010"
                    "1000000010101"
                    "0101010101000"
                    "1000101000000"
                    "0100010100010"
                    "1000000010100"
                    "0101010000010"
                    "1000101000001"
                    "0101010101000"
                },
        /* 47*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "[)>\03605\035101\036\004", -1, 0, 12, 17, 1, 1, "Macro 05",
                    "10000010001000101"
                    "00000001000000010"
                    "10001010100010001"
                    "01010000010000000"
                    "10001010000010001"
                    "01000000010000010"
                    "10100010101000001"
                    "01010100000001010"
                    "00001000101010101"
                    "01010001000101000"
                    "10101010101010101"
                    "01010001010101010"
                },
        /* 48*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "[)>\03607Text\036\004", -1, 0, 14, 21, 1, 1, "Macro 07 (free form text)",
                    "100010001000001000101"
                    "010001000000010101010"
                    "000010101000100000101"
                    "010100000000010000000"
                    "001000100000000000100"
                    "010101010001010101010"
                    "101010101010100010100"
                    "010000010101000000000"
                    "001010001000001010001"
                    "000000000101010101010"
                    "001000000010001000001"
                    "000101000101000100000"
                    "100010101010100010101"
                    "000001010100010100000"
                },
        /* 49*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "[)>\03605\035Študentska št.\0352198390\036\004", -1, ZINT_WARN_USES_ECI, 23, 34, 1, 1, "Macro 05 with ECI",
                    "1010101000100000101000001010000010"
                    "0101000001000101010100000101000001"
                    "1000101000101000100010100010001000"
                    "0000010001010001000100010001010000"
                    "0000101000100010000010001010101000"
                    "0001010001000100010000010101010100"
                    "0000100010100000101010001000001010"
                    "0100010100010100010000010101000001"
                    "1010101000100010000010000010001010"
                    "0101000100000100010001010000010001"
                    "0010101000100010001000001010101000"
                    "0100010100010001000101010000010000"
                    "0000101010100010000010100010100010"
                    "0101000000000001010001010001000001"
                    "0010100010000000101010100010100010"
                    "0101000100010101000100000100010100"
                    "0010100010000010101000101000001000"
                    "0001000100000101000100010000010101"
                    "1000100010100000100010001010100010"
                    "0101010001000001000101000101000100"
                    "0010000010001010100010001000001000"
                    "0100000001000100010001010100010101"
                    "1010001000001010101000000010101000"
                },
        /* 50*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, "[)>\03607Τεχτ\036\004", -1, ZINT_WARN_USES_ECI, 17, 26, 1, 1, "Macro 07 with ECI",
                    "10001010001000100000101000"
                    "00010101000101000000010101"
                    "00100010100000100010001000"
                    "00010101000101010101000100"
                    "10101000100010001010000000"
                    "00000100010100010100000001"
                    "00001010101000100000001010"
                    "01010000010001010100010100"
                    "00000010100010101010000010"
                    "00010101000101000101000001"
                    "00101000101000100010001000"
                    "00010101000100000001010100"
                    "10100010000010001010001000"
                    "00010100010100010000010001"
                    "10001000101000101010001000"
                    "01000101010100000000000101"
                    "10100010000010101010000010"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    /* Only do BWIPP/ZXing-C++ tests if asked, too slow otherwise */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_DOTCODE, data[i].input_mode, -1 /*eci*/, -1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %s, { %d, %d, \"%s\" }, \"%s\", %d, %s, %d, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_2,
                    testUtilOption3Name(BARCODE_DOTCODE, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].zxingcpp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    if (!data[i].zxingcpp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[16384];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                    "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, data[i].zxingcpp_cmp,
                                    cmp_buf, sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n",
                                    i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length,
                                    NULL /*primary*/, escaped, &ret_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                    i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf,
                                    ret_len, escaped);
                    }
                }
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
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        int zxingcpp_cmp;
        const char *comment;
        const char *expected;
    };
    /* ISS DotCode, Rev 4.0, DRAFT 0.15, TSC Pre-PR #5, MAY 28, 2019 */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 18, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 13, 18, 1, 1, "ISS DotCode Rev 4.0 13.5 example **NOT SAME** different encodation",
                    "100000001010101010"
                    "000100000100010101"
                    "001010000010101000"
                    "010101010001010001"
                    "100010100010000000"
                    "010100010100000101"
                    "001000101000000010"
                    "000001010101000100"
                    "100010100010000010"
                    "000001000101000001"
                    "101000101010100010"
                    "010100000000010001"
                    "100000101010000010"
                },
        /*  1*/ { UNICODE_MODE, 18, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 13, 18, 1, 1, "ISS DotCode Rev 4.0 13.5 example auto-ECI",
                    "100000001010101010"
                    "000100000100010101"
                    "001010000010101000"
                    "010101010001010001"
                    "100010100010000000"
                    "010100010100000101"
                    "001000101000000010"
                    "000001010101000100"
                    "100010100010000010"
                    "000001000101000001"
                    "101000101010100010"
                    "010100000000010001"
                    "100000101010000010"
                },
        /*  2*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 14, 21, 1, 1, "ISS DotCode Rev 4.0 13.5 example inverted",
                    "100010001010101010000"
                    "000100000101000001010"
                    "101000101000001000101"
                    "000001010100000101000"
                    "001010101010100010001"
                    "010001000001010101000"
                    "101000101000101010100"
                    "010101010001010001000"
                    "100010001000000000100"
                    "010100000001000100010"
                    "001000100010000000101"
                    "000000000100010100010"
                    "100010001000100000001"
                    "010101000100010001000"
                },
        /*  3*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 14, 21, 1, 1, "ISS DotCode Rev 4.0 13.5 example inverted auto-ECI",
                    "100010001010101010000"
                    "000100000101000001010"
                    "101000101000001000101"
                    "000001010100000101000"
                    "001010101010100010001"
                    "010001000001010101000"
                    "101000101000101010100"
                    "010101010001010001000"
                    "100010001000000000100"
                    "010100000001000100010"
                    "001000100010000000101"
                    "000000000100010100010"
                    "100010001000100000001"
                    "010101000100010001000"
                },
        /*  4*/ { UNICODE_MODE, 95, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a -128 GB的存储空间-黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a - 128 GB Speicher - Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 64, 95, 0, 1, "AIM ITS/04-023:2022 Annex A example; BWIPP different encodation",
                    "10101000100000001010000000101000101010100010001010000010001000101000101000001000001000100010101"
                    "00000101010001010000010101000000010100000001010101000101010100010101000101000100000100000100000"
                    "10100010100010001000100000001010100000001000101000101010001010001010000000101000001000101000101"
                    "01010001000100000000000001010000010100010101010100010001010001010000010000010000010100010000010"
                    "10000010101000100000001010000010000000000000000000100010100010100000101010001000000010001000000"
                    "00010100000101000001010100000101010101010000010001010100000100010101010001010101000101000000010"
                    "10001010001010101010000010001010000010101000001000101000001000101000100010101010001000000010100"
                    "00010100010100010101000000010001010101000101000101010001010101000000000101010001010100000101000"
                    "10100000000000000000101000100010101010001000101010001010101010000000000010000000100010101010001"
                    "01000100010001010001000000000000000001000100000100000001010100010100010001010001010101010100010"
                    "00101000101010101000000010101000001010001010100010101000000000101010101000101010101010100000101"
                    "00000101010101010100010001010101000101010000010001010001000001010101000100000100000001010001010"
                    "00101010000010101000101000100000101000100010001000101000101010000010100010100000001000001010101"
                    "01000000010001000101010100000100000000010101000100000100010000010000010100000101000100000001010"
                    "00001010101000101000100000001000001010100010000010101010100000101010100010000000100010001000001"
                    "01010101010101010100000000010100010000010101000101010100000101010000000000010000010100010101000"
                    "10000000001010000010101000100010000010100000100000100010100010000000101010101010100010000000100"
                    "00010000000000010001010101000101010101000000010100000100010001000101010101010001000001010101010"
                    "10100000100000101010101010100000100010101010100010000010001010100010100010000010100000001010001"
                    "01000001010000000101010001010001000001010100010000000100000000000001000100000100000101000101010"
                    "10000010101010100010001010101010000000000010101000101010000000101000001000101010101010100000100"
                    "00010101000100000100000101010000010100010001000001010100000100010101010001010101010001010001010"
                    "00001010101000000010000010001010001000101000100000000000001010000000001000001010000000100010100"
                    "01000001000101000101010000000101010101010101000001000001010101000101010100010001010000010000000"
                    "10101010000010001000000000000000101010001010001010001000100000100010101010101000101010101010000"
                    "01010100000101010000010101010001000101000100010101000100010001010001010101000000010101010101000"
                    "00100010101000100010100010101000101000001000001010001010001010001010100000100010000010100010001"
                    "00000000010100010001000000000100010000010000010101000001000001000101000101010100010100010100000"
                    "10100000101010001000001000101010100000100000101000100010100000101000001000100000101000001010000"
                    "00010101000101000101010101000101000001000001000000010001010100000100000100000101010000000101010"
                    "00100000000010100010101010101010101000100010100010100010001010100010001000101010101000100010000"
                    "01000100010000010000000101000101010100000100010100010101000101010001010001000100010100000000010"
                    "10001000001000001010101010000000001010101000101010000000100010001010000010001000101010100000101"
                    "01010101000001000100000001010000010100010100000101000000010100000000000000010101000001010001000"
                    "00001010100000000000001000100010100010101010000000001000100000001010100010000010000000001010001"
                    "01010101000101000001010100010000000001010100000100010101000101010101010001010000010101000100000"
                    "00001010001000101000000000001010101010000010001000101010001000101000101010001000000010100010101"
                    "00010001010000010100000000000101000001010001000101010101010001010101010101000101010100000101010"
                    "10101000100010000000101010100010000000001010100010101010001010000010101000101010001010100010101"
                    "01000100010000010101000101010000010101000101010001010001000001000000000000010101010101010001010"
                    "10000010001000100010100010101000001010100010001010000000101000101010000010000010100010001000100"
                    "01010000010101010101010100000001010100010000010000000000010100010100000100000000000001010000010"
                    "00100000000010101010001000001010001010001000101010101010101010001000100010101000001000001000100"
                    "00000100000101010001010101010100000001010101010000000100010100010001010001010101000001000101000"
                    "00001010001010001010000000100000100000101000101000101000100010101010101000000010101000101000101"
                    "01010101010001010100000000010001000001010001010101010101010100000001000000010101000101010100000"
                    "10100010101000101010101010100010100000000010100000000010001000100010001010001000101010001010001"
                    "01010001010000000001010101010101010100000101010001000100000001010000010100000000000101000001010"
                    "00001010100010101000101010001000101000101010001010101010001000100000001010100000100000101010100"
                    "00000001000101000101010001010001000001000001010101010101000000000101010101010000010000010000010"
                    "10100000000010101000001000000000000010001010100010001000001010000000000000001000000010101000000"
                    "00000100000100000100000000000100010100000100010000000000010000000101010001000001010101010101000"
                    "00101000101000000010000000001000101010100000000010100010100010001010101010101010001010000000100"
                    "01000101010000010000010100000001010100010001000001010101000101010100000101000101010100010100000"
                    "10101010101010000010100010101010000000000010001000100000001000100000100000100010100010101000101"
                    "01010000010001010001010100000101010001010001000101010001000101010000000101010001000000000101010"
                    "10000000000010101000100010100010101010100000001000101000100010000000100000101010000010100010001"
                    "00010101010100000001010101010000000101000100000100010001010001000100010000000101010100000001010"
                    "10101010001010101010001000101010000010101010100010001000100000001000101000000000101000001000001"
                    "01010100000101010100000000000100010101000101010100000000010101010001010000000100000001000100010"
                    "10000010100000100000101010001000100000000000100000000010100010001010001010100010100010000010100"
                    "00010000010101000101010101010101010000010100010000010001010000010100000101000100010000010000000"
                    "10100000100000001010101010101010001010001010000010100010001010101010000010001010000000100010001"
                    "01000001000100000101000101010100000000010001000100000101000100000100010101010101000101010100010"
                },
        /*  5*/ { DATA_MODE, -1, -1, { 0, 0, "" }, { { TU("\266"), 1, 0 }, { TU("\266"), 1, 7 }, { TU("\266"), 1, 0 } }, 0, 15, 22, 1, 0, "Standard example + extra seg, data mode",
                    "1000101010000000001000"
                    "0100000101000101000001"
                    "1000001000100010101000"
                    "0000010001000001010101"
                    "1000100000100010101010"
                    "0001010000010000010001"
                    "0010101000101010100010"
                    "0000010101000101000000"
                    "1000100010001010100000"
                    "0101000101000100010100"
                    "0010100000100000001010"
                    "0001000101000001010100"
                    "1010000010100010001010"
                    "0000010000010000010101"
                    "1010001010100000101010"
                },
        /*  6*/ { UNICODE_MODE, 38, -1, { 0, 0, "" }, { { TU("1234567890β"), -1, 9 }, { TU("1234567890点"), -1, 20 }, { TU("1234567890"), -1, 0 } }, 0, 23, 38, 0, 1, "FNC2 ECI & BIN_LATCH ECI; BWIPP different encodation",
                    "10000000001010100000101010100000100010"
                    "00000001010001000001010001010101000100"
                    "10001010100000101000001010001010101000"
                    "01000101010001010001000100000000010101"
                    "00001000100010001010100000101000100000"
                    "01000101010000010101010001000000010101"
                    "10001000101000101010000000100010000010"
                    "01000100010001010001010000010100010001"
                    "10101000000010001010000010101010000000"
                    "01010100010000010100010000010000010101"
                    "00000010100010001010000010000010100010"
                    "01010000010001010001000101000001010101"
                    "10000000000010001010101000101000000010"
                    "00010001010001010100010000010101000101"
                    "10000010101010101000000010001000100010"
                    "00000101000101010100010000010100010000"
                    "00001010101000000010100010001010000010"
                    "01010001000100000100010001010000010101"
                    "00001010001010001000000000101010100010"
                    "01010101010000000100010101010000010100"
                    "00101010100010000010100010000010000010"
                    "01010000010100010001010000010000010101"
                    "10000010100000001010100000100010001010"
                },
        /*  7*/ { UNICODE_MODE, 29, -1, { 0, 0, "" }, { { TU("çèéêëì"), -1, 0 }, { TU("òóô"), -1, 899 }, { TU(""), 0, 0 } }, 0, 18, 29, 1, 1, "BIN_LATCH ECI > 0xFF",
                    "10100000000000100010101000101"
                    "01010100010101010101000100000"
                    "10101000101010000000001010100"
                    "01010001000100010101010000010"
                    "10001010101010101010000000101"
                    "00010000010000000000010001010"
                    "10001000100000001000100000000"
                    "00000001010001000001010101000"
                    "10001010100010100010101010101"
                    "01010101000100000101000101000"
                    "10000010001010100000000000001"
                    "00000000010101000000010001000"
                    "00101010100000101010100010100"
                    "01010100000000010100000100010"
                    "00001000101010001000101010101"
                    "01000101010101010001000100010"
                    "10001000000010100000001000101"
                    "01000100010000010101010100010"
                },
        /*  8*/ { UNICODE_MODE, 29, -1, { 0, 0, "" }, { { TU("çèéêëì"), -1, 0 }, { TU("òóô"), -1, 65536 }, { TU(""), 0, 0 } }, 0, 22, 29, 1, 0, "BIN_LATCH ECI > 0xFFFF; ZXing-C++ test can't handle UTF-8 binary",
                    "10101000100000101000001010001"
                    "00010101000000000100010100000"
                    "10100010001010000010101010100"
                    "00010100010101000100010001010"
                    "00001000001010101010101010101"
                    "00010000000001000100010100010"
                    "10001000001010000010000010001"
                    "00000001010101010000000101010"
                    "10101010101000001010100010101"
                    "01000100000100010001010001000"
                    "10000000000000101010100000001"
                    "00010101000101000001000001000"
                    "00101010101000100000001000101"
                    "00010001010001000101000100010"
                    "00000000100010100010000000001"
                    "01000001000100000000010101010"
                    "10100010100010001010001010101"
                    "01000100010001010100010100000"
                    "10101010001000100010100000101"
                    "01000001000000010001000001000"
                    "10001010101000101010000010001"
                    "01010101010101010001010000010"
                },
        /*  9*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("[)>\03605\035"), -1, 0 }, { TU("A\036\004"), -1, 0 }, { TU(""), 0, 0 } }, 0, 10, 13, 1, 1, "Macro 05",
                    "1010001010101"
                    "0001000001010"
                    "1010100010001"
                    "0000000001000"
                    "1000101000100"
                    "0101010101000"
                    "1000001000001"
                    "0100010000010"
                    "0000100000100"
                    "0001010101010"
                },
        /* 10*/ { UNICODE_MODE, -1, -1, { 0, 0, "" }, { { TU("[)>\03605\035"), -1, 0 }, { TU("Študentska št."), -1, 4 }, { TU("\0352198390\036\004"), -1, 0 } }, 0, 24, 35, 1, 1, "Macro 05 with ECI",
                    "10101000000010101000000000101010001"
                    "00010100000001000000000101010001010"
                    "00000010001000000010100010001000101"
                    "00000000010100010101000001010001010"
                    "00000010101010000000101010100010000"
                    "00010000000101000101010001000100000"
                    "10001000000000001010100000001000101"
                    "01000001010101010100000101010101010"
                    "00100010000010100010001010101000000"
                    "00010101010000000001010100010101000"
                    "00101000100010001000001000100010101"
                    "00010000010101010000010001010100000"
                    "10100010101010101010000010000000001"
                    "01010100000100010100000100000000010"
                    "10001000100010100010101000001010101"
                    "01010101010000000101000001000100010"
                    "10001010101000001010101000001000100"
                    "01000101010101000101010101010101000"
                    "10101010101000100010100000000000000"
                    "01000101010000000000010000000001000"
                    "00100010001000101000100010101000100"
                    "01000000000101010101010100000101010"
                    "10001010000010100000001010000010001"
                    "01010001000001010001010001010101010"
                },
        /* 11*/ { UNICODE_MODE, -1, -1, { 35, 35, "" }, { { TU("Τεχτ"), -1, 9 }, { TU("กขฯ"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 20, 29, 0, 1, "Structured Append with Terminate Latch A replaced",
                    "10101000001010000000001000101"
                    "01010101010101000000000100000"
                    "00101010001010100010101010001"
                    "01000100010000000101010100010"
                    "10000010100010001000100000000"
                    "00010000010101010100000101010"
                    "10100010101010101010001000001"
                    "00010101000000010101010001000"
                    "10000000100000101000101000101"
                    "01010000010001010001000100010"
                    "00101000000000100010001010101"
                    "01000001000101010001000001010"
                    "00001010000000000000100010101"
                    "00000101010101000100010001000"
                    "00101000001010001000001010101"
                    "01010101010100000100010001000"
                    "10000000101000100010100010001"
                    "00000000000000010001010101000"
                    "10101010001010101010100000001"
                    "01000101000001010100000100010"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, BARCODE_DOTCODE, data[i].input_mode, -1 /*eci*/,
                    -1 /* option_1*/, data[i].option_2, data[i].option_3, -1 /*output_options*/, NULL, 0, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            char escaped1[4096];
            char escaped2[4096];
            int length = data[i].segs[0].length == -1 ? (int) z_ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
            int length1 = data[i].segs[1].length == -1 ? (int) z_ustrlen(data[i].segs[1].source) : data[i].segs[1].length;
            int length2 = data[i].segs[2].length == -1 ? (int) z_ustrlen(data[i].segs[2].source) : data[i].segs[2].length;
            printf("        /*%3d*/ { %s, %d, %s, { %d, %d, \"%s\" }, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, %s, %d, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_2,
                    testUtilOption3Name(BARCODE_DOTCODE, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].zxingcpp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwippSegs(i, symbol, -1, data[i].option_2, data[i].option_3, data[i].segs, seg_count, NULL, cmp_buf, sizeof(cmp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwippSegs ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, (const char *) data[i].segs[0].source, data[i].segs[0].length, debug)) {
                    if (!data[i].zxingcpp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else if (data[i].input_mode == DATA_MODE) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) {
                            printf("i:%d %s multiple segments in DATA_MODE not currently supported for ZXing-C++ testing\n",
                                    i, testUtilBarcodeName(symbol->symbology));
                        }
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[16384];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                    "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, (const char *) data[i].segs[0].source,
                                    data[i].segs[0].length, modules_dump, data[i].zxingcpp_cmp, cmp_buf,
                                    sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n",
                                    i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmpSegs(symbol, cmp_msg, cmp_buf, cmp_len, data[i].segs, seg_count,
                                    NULL /*primary*/, escaped, &ret_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPPCmpSegs %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                    i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf,
                                    ret_len, escaped);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int output_options;
        const char *data;
        int length;
        int ret;
        int expected_eci;
        const char *expected;
        int expected_length;
        int expected_content_eci;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, "é", -1, 0, 0, "", -1, 0 },
        /*  1*/ { UNICODE_MODE, -1, BARCODE_CONTENT_SEGS, "é", -1, 0, 0, "é", -1, 3 }, /* Now UTF-8, not converted */
        /*  2*/ { UNICODE_MODE, -1, -1, "ก", -1, ZINT_WARN_USES_ECI, 13, "", -1, 0 },
        /*  3*/ { UNICODE_MODE, -1, BARCODE_CONTENT_SEGS, "ก", -1, ZINT_WARN_USES_ECI, 13, "ก", -1, 13 },
        /*  4*/ { UNICODE_MODE | ESCAPE_MODE, -1, -1, "[)>\\R05\\GA\\R\\E", -1, 0, 0, "", -1, 0 },
        /*  5*/ { UNICODE_MODE | ESCAPE_MODE, -1, BARCODE_CONTENT_SEGS, "[)>\\R05\\GA\\R\\E", -1, 0, 0, "[)>\03605\035A\036\004", -1, 3 },
        /*  6*/ { DATA_MODE, -1, -1, "\351", -1, 0, 0, "", -1, 0 },
        /*  7*/ { DATA_MODE, -1, BARCODE_CONTENT_SEGS, "\351", -1, 0, 0, "\351", -1, 3 },
        /*  8*/ { UNICODE_MODE, 26, -1, "é", -1, 0, 26, "", -1, 0 },
        /*  9*/ { UNICODE_MODE, 26, BARCODE_CONTENT_SEGS, "é", -1, 0, 26, "é", -1, 26 },
        /* 10*/ { UNICODE_MODE, 899, -1, "é", -1, 0, 899, "", -1, 0 },
        /* 11*/ { UNICODE_MODE, 899, BARCODE_CONTENT_SEGS, "é", -1, 0, 899, "é", -1, 899 },
        /* 12*/ { GS1_MODE, -1, -1, "[01]04912345123459[15]970331[30]128[10]ABC123", -1, 0, 0, "", -1, 0 },
        /* 13*/ { GS1_MODE, -1, BARCODE_CONTENT_SEGS, "[01]04912345123459[15]970331[30]128[10]ABC123", -1, 0, 0, "01049123451234591597033130128\03510ABC123", -1, 3 },
        /* 14*/ { GS1_MODE, 20, BARCODE_CONTENT_SEGS, "[01]04912345123459[15]970331[30]128[10]ABC123", -1, ZINT_WARN_NONCOMPLIANT, 20, "01049123451234591597033130128\03510ABC123", -1, 3 }, /* Note: content seg ECI remains at default 3 */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_DOTCODE, data[i].input_mode, data[i].eci,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                                    data[i].data, data[i].length, debug);
        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n",
                        i, symbol->eci, data[i].expected_eci);
            if (symbol->output_options & BARCODE_CONTENT_SEGS) {
                assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
                assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
                assert_equal(symbol->content_segs[0].length, expected_length,
                            "i:%d content_segs[0].length %d != expected_length %d\n",
                            i, symbol->content_segs[0].length, expected_length);
                assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected, expected_length),
                            "i:%d content_segs[0].source memcmp(%s, %s, %d) != 0\n", i,
                            testUtilEscape((const char *) symbol->content_segs[0].source, symbol->content_segs[0].length,
                                            escaped, sizeof(escaped)),
                            testUtilEscape(data[i].expected, expected_length, escaped2, sizeof(escaped2)),
                            expected_length);
                assert_equal(symbol->content_segs[0].eci, data[i].expected_content_eci,
                            "i:%d content_segs[0].eci %d != expected_content_eci %d\n",
                            i, symbol->content_segs[0].eci, data[i].expected_content_eci);
            } else {
                assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rt_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int output_options;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        struct zint_seg expected_content_segs[3];
        int expected_content_seg_count;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, {0} }, 0, 12, 19, {{0}}, 0 },
        /*  1*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 12, 19, { { TU("¶"), 2, 3 }, { TU("Ж"), 2, 7 }, {0} }, 2 }, /* Now UTF-8, not converted */
        /*  2*/ { UNICODE_MODE, -1, { { TU("éé"), -1, 0 }, { TU("กขฯ"), -1, 0 }, { TU("βββ"), -1, 0 } }, ZINT_WARN_USES_ECI, 19, 28, {{0}}, 0 },
        /*  3*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, { { TU("éé"), -1, 0 }, { TU("กขฯ"), -1, 0 }, { TU("βββ"), -1, 0 } }, ZINT_WARN_USES_ECI, 19, 28, { { TU("éé"), 4, 3 }, { TU("กขฯ"), 9, 13 }, { TU("βββ"), 6, 9 } }, 3 },
        /*  4*/ { DATA_MODE, -1, { { TU("¶"), -1, 26 }, { TU("Ж"), -1, 0 }, { TU("\223\137"), -1, 20 } }, 0, 19, 28, {{0}}, 0 },
        /*  5*/ { DATA_MODE, BARCODE_CONTENT_SEGS, { { TU("¶"), -1, 26 }, { TU("Ж"), -1, 0 }, { TU("\223\137"), -1, 20 } }, 0, 19, 28, { { TU("¶"), 2, 26 }, { TU("\320\226"), 2, 3 }, { TU("\223\137"), 2, 20 } }, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, BARCODE_DOTCODE, data[i].input_mode, -1 /*eci*/,
                            -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                            NULL, 0, debug);
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (width %d)\n",
                    i, symbol->rows, data[i].expected_rows, symbol->width);
        assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n",
                    i, symbol->width, data[i].expected_width);

        assert_equal(symbol->content_seg_count, data[i].expected_content_seg_count, "i:%d symbol->content_seg_count %d != %d\n",
                    i, symbol->content_seg_count, data[i].expected_content_seg_count);
        if (symbol->output_options & BARCODE_CONTENT_SEGS) {
            assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
            for (j = 0; j < symbol->content_seg_count; j++) {
                assert_nonnull(symbol->content_segs[j].source, "i:%d content_segs[%d].source NULL\n", i, j);

                expected_length = data[i].expected_content_segs[j].length;

                assert_equal(symbol->content_segs[j].length, expected_length,
                            "i:%d content_segs[%d].length %d != expected_length %d\n",
                            i, j, symbol->content_segs[j].length, expected_length);
                assert_zero(memcmp(symbol->content_segs[j].source, data[i].expected_content_segs[j].source, expected_length),
                            "i:%d content_segs[%d].source memcmp(%s, %s, %d) != 0\n", i, j,
                            testUtilEscape((const char *) symbol->content_segs[j].source, expected_length, escaped,
                                            sizeof(escaped)),
                            testUtilEscape((const char *) data[i].expected_content_segs[j].source, expected_length,
                                            escaped2, sizeof(escaped2)),
                            expected_length);
                assert_equal(symbol->content_segs[j].eci, data[i].expected_content_segs[j].eci,
                            "i:%d content_segs[%d].eci %d != expected_content_segs.eci %d\n",
                            i, j, symbol->content_segs[j].eci, data[i].expected_content_segs[j].eci);
            }
        } else {
            assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* #181 Christian Hartlage / Nico Gunkel OSS-Fuzz */
static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        const char *data;
        int length;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { DATA_MODE, "(\207'", -1, 0 }, /* 0x28,0x87,0x27 Note: should but doesn't trigger sanitize error if no length check, for some reason; UPDATE: use up-to-date gcc (9)! */
        /*  1*/ { DATA_MODE,
                    "\133\061\106\133\061\106\070\161\116\133\116\116\067\040\116\016\000\116\125\111\125\125\316\125\125\116\116\116\116\117\116\125"
                    "\111\125\103\316\125\125\116\116\116\116\117\000\000\116\136\116\116\001\116\316\076\116\116\057\136\116\116\134\000\000\116\116"
                    "\116\230\116\116\116\116\125\125\125\257\257\257\000\001\116\130\212\212\212\212\212\212\212\377\377\210\212\212\177\000\212\212"
                    "\212\212\212\212\175\212\212\212\212\212\212\116\117\001\116\116\112\116\116\116\116\176\136\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\005\377\377\005\125\125\125\325\001\116\116\116\266\116\020\000\200\000\116\116\177\000\000\377"
                    "\377\257\257\257\125\112\117\116\001\000\000\044\241\001\116\116\116\136\116\116\116\056\116\125\111\125\125\316\125\125\116\116"
                    "\116\116\057\000\000\116\136\116\116\001\116\116\076\342\116\057\136\116\116\134\000\000\116\116\116\241\116\116\116\116\125\125"
                    "\125\257\257\257\000\001\116\130\212\212\212\212\212\212\212\212\172\212\071\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\110\071\071\051\071\065\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330"
                    "\330\330\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\065\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\072\071\071\277\071\071\077\071\071\071\071\071\071\071\071\154\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\011\071\071\071\071\071\071\071\071\071\071\071\071\071\071\105\105\105\105\105\105\105\105\105\105\105\105\105\071"
                    "\071\071\071\071\071", /* Original OSS-Fuzz triggering data for index out of bounds (encoding of HT/FS/GS/RS when shifting to code set B) */
                    421, 0 },
        /*  2*/ { DATA_MODE, "\233:", -1, 0 }, /* Original OSS-Fuzz triggering data for codeword_array buffer overflow, L777 */
        /*  3*/ { DATA_MODE, "\241\034", -1, 0 }, /* As above L793 */
        /*  4*/ { DATA_MODE, "\270\036", -1, 0 }, /* As above L799 */
        /*  5*/ { DATA_MODE, "\237\032", -1, 0 }, /* As above L904 */
        /*  6*/ { DATA_MODE, "\237", -1, 0 }, /* As above L1090 */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_DOTCODE, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#define GF  113

/* Dummy to generate pre-calculated coefficients for GF(113) of generator polys of degree 3 to 39 */
static void test_generate(const testCtx *const p_ctx) {

    /* roots (antilogs): root[0] = 1; for (i = 1; i < GF - 1; i++) root[i] = (PM * root[i - 1]) % GF; */
    static const int root[GF - 1] = {
          1,   3,   9,  27,  81,  17,  51,  40,   7,  21,
         63,  76,   2,   6,  18,  54,  49,  34, 102,  80,
         14,  42,  13,  39,   4,  12,  36, 108,  98,  68,
         91,  47,  28,  84,  26,  78,   8,  24,  72, 103,
         83,  23,  69,  94,  56,  55,  52,  43,  16,  48,
         31,  93,  53,  46,  25,  75, 112, 110, 104,  86,
         32,  96,  62,  73, 106,  92,  50,  37, 111, 107,
         95,  59,  64,  79,  11,  33,  99,  71, 100,  74,
        109, 101,  77,   5,  15,  45,  22,  66,  85,  29,
         87,  35, 105,  89,  41,  10,  30,  90,  44,  19,
         57,  58,  61,  70,  97,  65,  82,  20,  60,  67,
         88,  38
    };
    int i, j, nc, cind, ci;

    /* Degree nc has nc + 1 terms */
    char coefs[820 - 5] = {0}; /* 40*(41 + 1)/2 == 820 less 2 + 3 (degrees 1 and 2) */
    int cinds[39 - 2] = {0};

    if (!p_ctx->generate) {
        return;
    }

    printf("    static const char coefs[820 - 5] = { /* 40*(41 + 1)/2 == 820 less 2 + 3 (degrees 1 and 2) */\n");
    for (nc = 3, cind = 0, ci = 0; nc <= 39; cind += nc + 1, ci++, nc++) {
        cinds[ci] = cind;

        coefs[cind] = 1;
        for (i = 1; i <= nc; i++) {
            for (j = nc; j >= 1; j--) {
                coefs[cind + j] = (GF + coefs[cind + j] - (root[i] * coefs[cind + j - 1]) % GF) % GF;
            }
        }
        printf("       ");
        for (i = 0; i <= nc; i++) {
            if (i == 22) printf("\n            ");
            printf(" %3d,", coefs[cinds[ci] + i]);
        }
        printf("\n");
    }
    printf("    };\n");

    printf("    static const short cinds[39 - 2] = { /* Indexes into above coefs[] array */\n       ");
    for (i = 0; i < ARRAY_SIZE(cinds); i++) {
        if (i == 22) printf("\n       ");
        printf(" %3d,", cinds[i]);
    }
    printf("\n    };\n");
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_options", test_options },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_encode_segs", test_encode_segs },
        { "test_rt", test_rt },
        { "test_rt_segs", test_rt_segs },
        { "test_fuzz", test_fuzz },
        { "test_generate", test_generate },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
