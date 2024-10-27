/*
    libzint - the open source barcode library
    Copyright (C) 2020-2024 Robin Stuart <rstuart114@gmail.com>

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
        int symbology;
        int option_2;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "13", 140, 0, 1, 1151, "" }, /* 8 (Start) + 140*8 + 2*8 (Checks) + 7 (Stop) == 1151 */
        /*  1*/ { BARCODE_CODE11, -1, "13", 141, ZINT_ERROR_TOO_LONG, -1, -1, "Error 320: Input length 141 too long (maximum 140)" },
        /*  2*/ { BARCODE_CODE39, -1, "1", 86, 0, 1, 1143, "" }, /* 13 (Start) + 86*13 + 12 (Stop) == 1143 */
        /*  3*/ { BARCODE_CODE39, -1, "1", 87, ZINT_ERROR_TOO_LONG, -1, -1, "Error 323: Input length 87 too long (maximum 86)" },
        /*  4*/ { BARCODE_EXCODE39, -1, "1", 86, 0, 1, 1143, "" },
        /*  5*/ { BARCODE_EXCODE39, -1, "1", 87, ZINT_ERROR_TOO_LONG, -1, -1, "Error 328: Input length 87 too long (maximum 86)" },
        /*  6*/ { BARCODE_EXCODE39, -1, "a", 43, 0, 1, 1143, "" }, /* Takes 2 encoding chars per char */
        /*  7*/ { BARCODE_EXCODE39, -1, "a", 44, ZINT_ERROR_TOO_LONG, -1, -1, "Error 317: Input too long, requires 88 symbol characters (maximum 86)" },
        /*  8*/ { BARCODE_EXCODE39, -1, "a", 86, ZINT_ERROR_TOO_LONG, -1, -1, "Error 317: Input too long, requires 172 symbol characters (maximum 86)" },
        /*  9*/ { BARCODE_LOGMARS, -1, "1", 30, 0, 1, 511, "" }, /* 16 (Start) + 30*16 + 15 (Stop) == 511 */
        /* 10*/ { BARCODE_LOGMARS, -1, "1", 31, ZINT_ERROR_TOO_LONG, -1, -1, "Error 322: Input length 31 too long (maximum 30)" },
        /* 11*/ { BARCODE_CODE93, -1, "1", 123, 0, 1, 1144, "" }, /* 9 (Start) + 123*9 + 2*9 (Checks) + 10 (Stop) == 1144 */
        /* 12*/ { BARCODE_CODE93, -1, "1", 124, ZINT_ERROR_TOO_LONG, -1, -1, "Error 330: Input length 124 too long (maximum 123)" },
        /* 13*/ { BARCODE_CODE93, -1, "a", 61, 0, 1, 1135, "" }, /* Takes 2 encoding chars per char */
        /* 14*/ { BARCODE_CODE93, -1, "a", 62, ZINT_ERROR_TOO_LONG, -1, -1, "Error 332: Input too long, requires 124 symbol characters (maximum 123)" },
        /* 15*/ { BARCODE_CODE93, -1, "a", 124, ZINT_ERROR_TOO_LONG, -1, -1, "Error 330: Input length 124 too long (maximum 123)" },
        /* 16*/ { BARCODE_CODE93, -1, "a1", 82, 0, 1, 1144, "" }, /* Takes 1.5 encoding chars (1.5*82 == 123) */
        /* 17*/ { BARCODE_CODE93, -1, "a1", 83, ZINT_ERROR_TOO_LONG, -1, -1, "Error 332: Input too long, requires 125 symbol characters (maximum 123)" },
        /* 18*/ { BARCODE_PZN, -1, "1", 7, 0, 1, 142, "" }, /* Takes 8 with correct check digit */
        /* 19*/ { BARCODE_PZN, -1, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1, "Error 325: Input length 9 too long (maximum 8)" },
        /* 20*/ { BARCODE_PZN, 1, "1", 6, 0, 1, 129, "" }, /* PZN7 takes 7 with correct check digit */
        /* 21*/ { BARCODE_PZN, 1, "1", 8, ZINT_ERROR_TOO_LONG, -1, -1, "Error 325: Input length 8 too long (maximum 7)" },
        /* 22*/ { BARCODE_VIN, -1, "1", 17, 0, 1, 246, "" },
        /* 23*/ { BARCODE_VIN, -1, "1", 18, ZINT_ERROR_TOO_LONG, -1, -1, "Error 336: Input length 18 wrong (17 only)" },
        /* 24*/ { BARCODE_VIN, -1, "1", 16, ZINT_ERROR_TOO_LONG, -1, -1, "Error 336: Input length 16 wrong (17 only)" },
        /* 25*/ { BARCODE_VIN, 1, "1", 17, 0, 1, 259, "" },
        /* 26*/ { BARCODE_HIBC_39, -1, "1", 68, 0, 1, 1151, "" }, /* 70 - 2 ('+' and check digit) */
        /* 27*/ { BARCODE_HIBC_39, -1, "1", 69, ZINT_ERROR_TOO_LONG, -1, -1, "Error 319: Input length 69 too long (maximum 68)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[4096];

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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

static void test_hrt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_2;
        char *data;
        int length;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "123-45", -1, "123-4552" }, /* 2 checksums */
        /*  1*/ { BARCODE_CODE11, 1, "123-45", -1, "123-455" }, /* 1 check digit */
        /*  2*/ { BARCODE_CODE11, 2, "123-45", -1, "123-45" }, /* No checksums */
        /*  3*/ { BARCODE_CODE11, -1, "123456789012", -1, "123456789012-8" }, /* First check digit 10 (A) goes to hyphen */
        /*  4*/ { BARCODE_CODE39, -1, "ABC1234", -1, "*ABC1234*" },
        /*  5*/ { BARCODE_CODE39, 1, "ABC1234", -1, "*ABC12340*" }, /* With visible check digit */
        /*  6*/ { BARCODE_CODE39, -1, "abc1234", -1, "*ABC1234*" }, /* Converts to upper */
        /*  7*/ { BARCODE_CODE39, 1, "abc1234", -1, "*ABC12340*" }, /* Converts to upper */
        /*  8*/ { BARCODE_CODE39, -1, "123456789", -1, "*123456789*" },
        /*  9*/ { BARCODE_CODE39, 1, "123456789", -1, "*1234567892*" }, /* With visible check digit */
        /* 10*/ { BARCODE_CODE39, 2, "123456789", -1, "*123456789*" }, /* With hidden check digit */
        /* 11*/ { BARCODE_EXCODE39, -1, "ABC1234", -1, "ABC1234" },
        /* 12*/ { BARCODE_EXCODE39, 1, "ABC1234", -1, "ABC12340" }, /* With visible check digit */
        /* 13*/ { BARCODE_EXCODE39, -1, "abc1234", -1, "abc1234" },
        /* 14*/ { BARCODE_EXCODE39, 1, "abc1234", -1, "abc1234." }, /* With visible check digit (previously was hidden) */
        /* 15*/ { BARCODE_EXCODE39, 2, "abc1234", -1, "abc1234" }, /* With hidden check digit */
        /* 16*/ { BARCODE_EXCODE39, -1, "a%\000\001$\177z\033\037!+/\\@A~", 16, "a%  $ z  !+/\\@A~" }, /* NUL, ctrls and DEL replaced with spaces */
        /* 17*/ { BARCODE_EXCODE39, 1, "a%\000\001$\177z\033\037!+/\\@A~", 16, "a%  $ z  !+/\\@A~L" }, /* With visible check digit */
        /* 18*/ { BARCODE_EXCODE39, 2, "a%\000\001$\177z\033\037!+/\\@A~", 16, "a%  $ z  !+/\\@A~" }, /* With hidden check digit */
        /* 19*/ { BARCODE_LOGMARS, -1, "ABC1234", -1, "ABC1234" },
        /* 20*/ { BARCODE_LOGMARS, -1, "abc1234", -1, "ABC1234" }, /* Converts to upper */
        /* 21*/ { BARCODE_LOGMARS, 1, "abc1234", -1, "ABC12340" }, /* With check digit */
        /* 22*/ { BARCODE_LOGMARS, 1, "12345/ABCDE", -1, "12345/ABCDET" }, /* With visible check digit */
        /* 23*/ { BARCODE_LOGMARS, 2, "12345/ABCDE", -1, "12345/ABCDE" }, /* With hidden check digit */
        /* 24*/ { BARCODE_CODE93, -1, "ABC1234", -1, "ABC1234" }, /* No longer shows 2 check chars added (same as BWIPP and TEC-IT) */
        /* 25*/ { BARCODE_CODE93, 1, "ABC1234", -1, "ABC1234S5" }, /* Unless requested */
        /* 26*/ { BARCODE_CODE93, -1, "abc1234", -1, "abc1234" },
        /* 27*/ { BARCODE_CODE93, 1, "abc1234", -1, "abc1234ZG" },
        /* 28*/ { BARCODE_CODE93, -1, "A\001a\000b\177d\037e", 9, "A a b d e" }, /* NUL, ctrls and DEL replaced with spaces */
        /* 29*/ { BARCODE_PZN, -1, "12345", -1, "PZN - 00123458" }, /* Pads with zeroes if length < 7 */
        /* 30*/ { BARCODE_PZN, -1, "123456", -1, "PZN - 01234562" },
        /* 31*/ { BARCODE_PZN, -1, "1234567", -1, "PZN - 12345678" },
        /* 32*/ { BARCODE_PZN, -1, "12345678", -1, "PZN - 12345678" },
        /* 33*/ { BARCODE_PZN, 1, "1234", -1, "PZN - 0012345" }, /* PZN7, pads with zeroes if length < 6 */
        /* 34*/ { BARCODE_PZN, 1, "12345", -1, "PZN - 0123458" },
        /* 35*/ { BARCODE_PZN, 1, "123456", -1, "PZN - 1234562" },
        /* 36*/ { BARCODE_PZN, 1, "1234562", -1, "PZN - 1234562" },
        /* 37*/ { BARCODE_VIN, -1, "1FTCR10UXTPA78180", -1, "1FTCR10UXTPA78180" },
        /* 38*/ { BARCODE_VIN, 1, "2FTPX28L0XCA15511", -1, "2FTPX28L0XCA15511" }, /* Include Import char - no change */
        /* 39*/ { BARCODE_HIBC_39, -1, "ABC1234", -1, "*+ABC1234+*" },
        /* 40*/ { BARCODE_HIBC_39, -1, "abc1234", -1, "*+ABC1234+*" }, /* Converts to upper */
        /* 41*/ { BARCODE_HIBC_39, -1, "123456789", -1, "*+1234567890*" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_hrt", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_2;
        char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, "-", -1, 0, 1, 37, "" },
        /*  1*/ { BARCODE_CODE11, -1, -1, "0123456789-", -1, 0, 1, 115, "" },
        /*  2*/ { BARCODE_CODE11, -1, -1, "A", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 321: Invalid character at position 1 in input (digits and \"-\" only)" },
        /*  3*/ { BARCODE_CODE11, -1, -1, "12+", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 321: Invalid character at position 3 in input (digits and \"-\" only)" },
        /*  4*/ { BARCODE_CODE11, -1, -1, "1.2", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 321: Invalid character at position 2 in input (digits and \"-\" only)" },
        /*  5*/ { BARCODE_CODE11, -1, -1, "12!", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 321: Invalid character at position 3 in input (digits and \"-\" only)" },
        /*  6*/ { BARCODE_CODE11, -1, -1, " ", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 321: Invalid character at position 1 in input (digits and \"-\" only)" },
        /*  7*/ { BARCODE_CODE11, ESCAPE_MODE, -1, "\\d048 ", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 321: Invalid character at position 2 in input (digits and \"-\" only)" }, /* Note position doesn't account for escape sequences */
        /*  8*/ { BARCODE_CODE11, -1, 3, "1", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 339: Invalid check digit version '3' (1 or 2 only)" },
        /*  9*/ { BARCODE_CODE39, -1, -1, "a", -1, 0, 1, 38, "" }, /* Converts to upper */
        /* 10*/ { BARCODE_CODE39, -1, -1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%", -1, 0, 1, 584, "" },
        /* 11*/ { BARCODE_CODE39, -1, -1, "AB!", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 3 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 12*/ { BARCODE_CODE39, -1, -1, "A\"B", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 2 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 13*/ { BARCODE_CODE39, -1, -1, "#AB", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 14*/ { BARCODE_CODE39, -1, -1, "&", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 15*/ { BARCODE_CODE39, -1, -1, "'", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 16*/ { BARCODE_CODE39, -1, -1, "(", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 17*/ { BARCODE_CODE39, -1, -1, ")", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 18*/ { BARCODE_CODE39, -1, -1, "*", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 19*/ { BARCODE_CODE39, -1, -1, ",", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 20*/ { BARCODE_CODE39, -1, -1, ":", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 21*/ { BARCODE_CODE39, -1, -1, "@", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 22*/ { BARCODE_CODE39, -1, -1, "[", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 23*/ { BARCODE_CODE39, -1, -1, "`", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 24*/ { BARCODE_CODE39, -1, -1, "{", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 25*/ { BARCODE_CODE39, -1, -1, "\000", 1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 26*/ { BARCODE_CODE39, -1, -1, "\300", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 27*/ { BARCODE_CODE39, -1, 0, "1", -1, 0, 1, 38, "" },
        /* 28*/ { BARCODE_CODE39, -1, 1, "1", -1, 0, 1, 51, "" }, /* Check digit */
        /* 29*/ { BARCODE_CODE39, -1, 2, "1", -1, 0, 1, 51, "" }, /* Hidden check digit */
        /* 30*/ { BARCODE_CODE39, -1, 3, "1", -1, 0, 1, 38, "" }, /* option_2 > 2 ignored */
        /* 31*/ { BARCODE_EXCODE39, -1, -1, "A", -1, 0, 1, 38, "" },
        /* 32*/ { BARCODE_EXCODE39, -1, 3, "A", -1, 0, 1, 38, "" }, /* option_2 > 2 ignored */
        /* 33*/ { BARCODE_EXCODE39, -1, -1, "a", -1, 0, 1, 51, "" },
        /* 34*/ { BARCODE_EXCODE39, -1, -1, ",", -1, 0, 1, 51, "" },
        /* 35*/ { BARCODE_EXCODE39, -1, -1, "\000", 1, 0, 1, 51, "" },
        /* 36*/ { BARCODE_EXCODE39, -1, -1, "\300", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 329: Invalid character at position 1 in input, extended ASCII not allowed" },
        /* 37*/ { BARCODE_EXCODE39, -1, -1, "ABCDé", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 329: Invalid character at position 5 in input, extended ASCII not allowed" },
        /* 38*/ { BARCODE_LOGMARS, -1, -1, "A", -1, 0, 1, 47, "" },
        /* 39*/ { BARCODE_LOGMARS, -1, -1, "a", -1, 0, 1, 47, "" },
        /* 40*/ { BARCODE_LOGMARS, -1, -1, ",", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 41*/ { BARCODE_LOGMARS, -1, -1, "\000", 1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 42*/ { BARCODE_LOGMARS, -1, -1, "\300", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 324: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 43*/ { BARCODE_LOGMARS, -1, 3, "A", -1, 0, 1, 47, "" }, /* option_2 > 2 ignored */
        /* 44*/ { BARCODE_CODE93, -1, -1, "A", -1, 0, 1, 46, "" },
        /* 45*/ { BARCODE_CODE93, -1, -1, "a", -1, 0, 1, 55, "" },
        /* 46*/ { BARCODE_CODE93, -1, -1, ",", -1, 0, 1, 55, "" },
        /* 47*/ { BARCODE_CODE93, -1, -1, "\000", 1, 0, 1, 55, "" },
        /* 48*/ { BARCODE_CODE93, -1, -1, "12\3004", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 331: Invalid character at position 3 in input, extended ASCII not allowed" },
        /* 49*/ { BARCODE_CODE93, -1, -1, "é", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 331: Invalid character at position 1 in input, extended ASCII not allowed" },
        /* 50*/ { BARCODE_PZN, -1, -1, "1", -1, 0, 1, 142, "" },
        /* 51*/ { BARCODE_PZN, -1, -1, "A", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 326: Invalid character at position 1 in input (digits only)" },
        /* 52*/ { BARCODE_PZN, -1, -1, "1000006", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 327: Invalid PZN, check digit is '10'" }, /* Check digit == 10 so can't be used */
        /* 53*/ { BARCODE_PZN, -1, -1, "00000011", -1, ZINT_ERROR_INVALID_CHECK, -1, -1, "Error 890: Invalid check digit '1', expecting '7'" },
        /* 54*/ { BARCODE_PZN, -1, 1, "100009", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 327: Invalid PZN, check digit is '10'" }, /* Check digit == 10 so can't be used */
        /* 55*/ { BARCODE_PZN, -1, 1, "0000011", -1, ZINT_ERROR_INVALID_CHECK, -1, -1, "Error 890: Invalid check digit '1', expecting '7'" },
        /* 56*/ { BARCODE_VIN, -1, -1, "5GZCZ43D13S812715", -1, 0, 1, 246, "" },
        /* 57*/ { BARCODE_VIN, -1, -1, "5GZCZ43D23S812715", -1, ZINT_ERROR_INVALID_CHECK, -1, -1, "Error 338: Invalid check digit '2' (position 9), expecting '1'" }, /* North American with invalid check character */
        /* 58*/ { BARCODE_VIN, -1, -1, "WP0ZZZ99ZTS392124", -1, 0, 1, 246, "" }, /* Not North American so no check */
        /* 59*/ { BARCODE_VIN, -1, -1, "WP0ZZZ99ZTS392I24", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 337: Invalid character at position 15 in input (alphanumerics only, excluding \"IOQ\")" }, /* I not allowed */
        /* 60*/ { BARCODE_VIN, -1, -1, "WPOZZZ99ZTS392124", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 337: Invalid character at position 3 in input (alphanumerics only, excluding \"IOQ\")" }, /* O not allowed */
        /* 61*/ { BARCODE_VIN, -1, -1, "WPQZZZ99ZTS392124", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 337: Invalid character at position 3 in input (alphanumerics only, excluding \"IOQ\")" }, /* Q not allowed */
        /* 62*/ { BARCODE_HIBC_39, -1, -1, "a", -1, 0, 1, 79, "" }, /* Converts to upper */
        /* 63*/ { BARCODE_HIBC_39, -1, -1, ",", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 203: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 64*/ { BARCODE_HIBC_39, -1, -1, "\000", 1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 203: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 65*/ { BARCODE_HIBC_39, -1, -1, "\300", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 203: Invalid character at position 1 in input (alphanumerics, space and \"-.$/+%\" only)" },
        /* 66*/ { BARCODE_HIBC_39, -1, 1, "a", -1, 0, 1, 79, "" }, /* option_2 ignored */
        /* 67*/ { BARCODE_HIBC_39, -1, 2, "a", -1, 0, 1, 79, "" }, /* option_2 ignored */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_2;
        char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "123-45", -1, 0, 1, 78, "2 check digits (52); verified manually against TEC-IT",
                    "101100101101011010010110110010101011010101101101101101011011010100101101011001"
                },
        /*  1*/ { BARCODE_CODE11, -1, "93", -1, 0, 1, 44, "2 check digits (--); verified manually against TEC-IT",
                    "10110010110101011001010101101010110101011001"
                },
        /*  2*/ { BARCODE_CODE11, 1, "123-455", -1, 0, 1, 78, "1 check digit (2); verified manually against TEC-IT",
                    "101100101101011010010110110010101011010101101101101101011011010100101101011001"
                },
        /*  3*/ { BARCODE_CODE11, 2, "123-4552", -1, 0, 1, 78, "0 check digits; verified manually against TEC-IT",
                    "101100101101011010010110110010101011010101101101101101011011010100101101011001"
                },
        /*  4*/ { BARCODE_CODE11, 1, "123-45", -1, 0, 1, 70, "1 check digit; verified manually against TEC-IT",
                    "1011001011010110100101101100101010110101011011011011010110110101011001"
                },
        /*  5*/ { BARCODE_CODE11, 2, "123-45", -1, 0, 1, 62, "0 check digits; verified manually against TEC-IT",
                    "10110010110101101001011011001010101101010110110110110101011001"
                },
        /*  6*/ { BARCODE_CODE39, -1, "1A", -1, 0, 1, 51, "ISO/IEC 16388:2007 Figure 1",
                    "100101101101011010010101101101010010110100101101101"
                },
        /*  7*/ { BARCODE_CODE39, 1, "1A", -1, 0, 1, 64, "With check digit (B)",
                    "1001011011010110100101011011010100101101011010010110100101101101"
                },
        /*  8*/ { BARCODE_CODE39, 1, "Z1", -1, 0, 1, 64, "Check digit '-'",
                    "1001011011010100110110101011010010101101001010110110100101101101"
                },
        /*  9*/ { BARCODE_CODE39, 1, "Z2", -1, 0, 1, 64, "Check digit '.'",
                    "1001011011010100110110101010110010101101100101011010100101101101"
                },
        /* 10*/ { BARCODE_CODE39, 1, "Z3", -1, 0, 1, 64, "Check digit space, displayed as underscore",
                    "1001011011010100110110101011011001010101001101011010100101101101"
                },
        /* 11*/ { BARCODE_CODE39, 1, "Z4", -1, 0, 1, 64, "Check digit '$'",
                    "1001011011010100110110101010100110101101001001001010100101101101"
                },
        /* 12*/ { BARCODE_CODE39, 1, "Z5", -1, 0, 1, 64, "Check digit '/'",
                    "1001011011010100110110101011010011010101001001010010100101101101"
                },
        /* 13*/ { BARCODE_CODE39, 1, "Z6", -1, 0, 1, 64, "Check digit '+'",
                    "1001011011010100110110101010110011010101001010010010100101101101"
                },
        /* 14*/ { BARCODE_CODE39, 1, "Z7", -1, 0, 1, 64, "Check digit '%'",
                    "1001011011010100110110101010100101101101010010010010100101101101"
                },
        /* 15*/ { BARCODE_CODE39, -1, "+A/E%U$A/D%T+Z", -1, 0, 1, 207, "Same as BARCODE_EXCODE39 'a%\000\001$\177z' below",
                    "100101101101010010100100101101010010110100100101001011010110010101010010010010110010101011010010010010101101010010110100100101001010101100101101010010010010101011011001010010100100101001101101010100101101101"
                },
        /* 16*/ { BARCODE_CODE39, -1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%", -1, 0, 1, 584, "Full CODE39 set",
                    "10010110110101010011011010110100101011010110010101101101100101010101001101011011010011010101011001101010101001011011011010010110101011001011010110101001011010110100101101101101001010101011001011011010110010101011011001010101010011011011010100110101011010011010101011001101011010101001101011010100110110110101001010101101001101101011010010101101101001010101011001101101010110010101101011001010101101100101100101010110100110101011011001101010101001011010110110010110101010011011010101001010110110110010101101010011010110101001001001010100100101001010010100100101010010010010100101101101"
                },
        /* 17*/ { BARCODE_EXCODE39, -1, "1A", -1, 0, 1, 51, "ISO/IEC 16388:2007 Figure 1",
                    "100101101101011010010101101101010010110100101101101"
                },
        /* 18*/ { BARCODE_EXCODE39, 1, "1A", -1, 0, 1, 64, "With check digit",
                    "1001011011010110100101011011010100101101011010010110100101101101"
                },
        /* 19*/ { BARCODE_EXCODE39, 1, "Z4", -1, 0, 1, 64, "Check digit $",
                    "1001011011010100110110101010100110101101001001001010100101101101"
                },
        /* 20*/ { BARCODE_EXCODE39, -1, "a%\000\001$\177z", 7, 0, 1, 207, "Verified manually against TEC-IT",
                    "100101101101010010100100101101010010110100100101001011010110010101010010010010110010101011010010010010101101010010110100100101001010101100101101010010010010101011011001010010100100101001101101010100101101101"
                },
        /* 21*/ { BARCODE_EXCODE39, -1, "\033\037!+/\\@A~", -1, 0, 1, 246, "Verified manually against TEC-IT",
                    "100101101101010100100100101101010010110101001001001011010110010101001001010010110101001011010010010100101101010100110100100101001011010110100101010010010010101101010011010100100100101001101010110110101001011010100100100101011010110010100101101101"
                },
        /* 22*/ { BARCODE_EXCODE39, -1, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]", -1, 0, 1, 1130, "Visible ASCII 1st 85 symbol chars",
                    "10010110110101001101011010100100101001011010100101101001001010010101101001011010010010100101101101001010100100101001010101100101101001001010010110101100101010010010100101011011001010100100101001010101001101101001001010010110101001101010010010100101011010011010100100101001010101100110101001001010010110101010011010010010100101011010100110100101011011011001010110101001001010010110101101001010100110110101101001010110101100101011011011001010101010011010110110100110101010110011010101010010110110110100101101010110010110101001001010010100110110101010100100100101011011001010101001001001010101001101101010010010010110101001101010100100100101011010011010101001001001010101100110101010010010010100110101011011010100101101011010010110110110100101010101100101101101011001010101101100101010101001101101101010011010101101001101010101100110101101010100110101101010011011011010100101010110100110110101101001010110110100101010101100110110101011001010110101100101010110110010110010101011010011010101101100110101010100101101011011001011010101001101101010101001001001011010101001101010010010010101101010011010100100100101101101010010100101101101"
                },
        /* 23*/ { BARCODE_EXCODE39, -1, "^_`abcdefghijklmnopqrstuvwxyz{|}~", -1, 0, 1, 883, "Visible ASCII last part",
                    "1001011011010101001001001010101101001101010010010010110101101001010100100100101100110101010100101001001011010100101101001010010010101101001011010010100100101101101001010100101001001010101100101101001010010010110101100101010010100100101011011001010100101001001010101001101101001010010010110101001101010010100100101011010011010100101001001010101100110101001010010010110101010011010010100100101011010100110100101001001011011010100101001010010010101011010011010010100100101101011010010100101001001010110110100101001010010010101010110011010010100100101101010110010100101001001010110101100101001010010010101011011001010010100100101100101010110100101001001010011010101101001010010010110011010101010010100100101001011010110100101001001011001011010101001010010010100110110101010100100100101011011010010101001001001010101011001101010010010010110101011001010100100100101011010110010100101101101"
                },
        /* 24*/ { BARCODE_LOGMARS, -1, "1A", -1, 0, 1, 63, "Verified manually against TEC-IT",
                    "100010111011101011101000101011101110101000101110100010111011101"
                },
        /* 25*/ { BARCODE_LOGMARS, 1, "1A", -1, 0, 1, 79, "With check digit; verified manually against TEC-IT",
                    "1000101110111010111010001010111011101010001011101011101000101110100010111011101"
                },
        /* 26*/ { BARCODE_LOGMARS, -1, "ABC", -1, 0, 1, 79, "MIL-STD-1189 Rev. B Figure 1",
                    "1000101110111010111010100010111010111010001011101110111010001010100010111011101"
                },
        /* 27*/ { BARCODE_LOGMARS, -1, "SAMPLE 1", -1, 0, 1, 159, "MIL-STD-1189 Rev. B Figure 2 top",
                    "100010111011101010111010111000101110101000101110111011101010001010111011101000101011101010001110111010111000101010001110101110101110100010101110100010111011101"
                },
        /* 28*/ { BARCODE_LOGMARS, 1, "12345/ABCDE", -1, 0, 1, 223, "MIL-STD-1189 Rev. B Section 6.2.1 check character example; verified manually against TEC-IT",
                    "1000101110111010111010001010111010111000101011101110111000101010101000111010111011101000111010101000100010100010111010100010111010111010001011101110111010001010101011100010111011101011100010101010111011100010100010111011101"
                },
        /* 29*/ { BARCODE_CODE93, -1, "C93", -1, 0, 1, 64, "ANSI/AIM BC5-1995 Figure 1; verified manually against TEC-IT",
                    "1010111101101000101000010101010000101101010001110110101010111101"
                },
        /* 30*/ { BARCODE_CODE93, -1, "CODE\01593", -1, 0, 1, 109, "ANSI/AIM BC5-1995 Figure B1; verified manually against TEC-IT",
                    "1010111101101000101001011001100101001100100101001001101010011001000010101010000101100101001000101101010111101"
                },
        /* 31*/ { BARCODE_CODE93, -1, "1A", -1, 0, 1, 55, "Verified manually against TEC-IT",
                    "1010111101010010001101010001101000101001110101010111101"
                },
        /* 32*/ { BARCODE_CODE93, -1, "TEST93", -1, 0, 1, 91, "Verified manually against TEC-IT",
                    "1010111101101001101100100101101011001101001101000010101010000101011101101001000101010111101"
                },
        /* 33*/ { BARCODE_CODE93, -1, "\000a\177", 3, 0, 1, 91, "Verified manually against TEC-IT",
                    "1010111101110110101100101101001100101101010001110110101101001101011011101010010001010111101"
                },
        /* 34*/ { BARCODE_CODE93, -1, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghij", -1, 0, 1, 1000, "Visible ASCII 1st 107 symbol chars",
                    "1010111101110100101110101101101010001110101101101001001110101101101000101110010101101011101110101101100010101110101101011010001110101101011001001110101101011000101110101101001101001011101101110101101010110001001011101110101001011011101000101001010010001010001001010000101001010001001001001001000101010100001000100101000010101110101101001110101110110101100010101110110101011010001110110101011001001110110101011000101110110101001101001110110101100110101101010001101001001101000101100101001100100101100010101011010001011001001011000101001101001000110101010110001010011001010001101001011001000101101101101001101100101101011001101001101100101101100110101011011001011001101001101101001110101110110101000110101110110101010110001110110101010011001110110101010001101110110101001011001110110101011011001001100101101010001001100101101001001001100101101000101001100101100101001001100101100100101001100101100010101001100101011010001001100101011001001001100101011000101001100101001101001010001101101001101010111101"
                },
        /* 35*/ { BARCODE_CODE93, -1, "klmnopqrstuvwxyz{|}~", -1, 0, 1, 397, "Visible ASCII last part",
                    "1010111101001100101000110101001100101010110001001100101010011001001100101010001101001100101001011001001100101000101101001100101101101001001100101101100101001100101101011001001100101101001101001100101100101101001100101100110101001100101011011001001100101011001101001100101001101101001100101001110101110110101000101101110110101101101001110110101101100101110110101101011001101001001101100101010111101"
                },
        /* 36*/ { BARCODE_PZN, -1, "1234567", -1, 0, 1, 142, "Example from IFA Info Code 39 EN V2.1; verified manually against TEC-IT",
                    "1001011011010100101011011011010010101101011001010110110110010101010100110101101101001101010101100110101010100101101101101001011010100101101101"
                },
        /* 37*/ { BARCODE_PZN, -1, "2758089", -1, 0, 1, 142, "Example from IFA Info Check Digit Calculations EN 15 July 2019; verified manually against TEC-IT",
                    "1001011011010100101011011010110010101101010010110110110100110101011010010110101010011011010110100101101010110010110101011001011010100101101101"
                },
        /* 38*/ { BARCODE_PZN, 1, "123456", -1, 0, 1, 129, "Example from BWIPP; verified manually against TEC-IT",
                    "100101101101010010101101101101001010110101100101011011011001010101010011010110110100110101010110011010101011001010110100101101101"
                },
        /* 39*/ { BARCODE_VIN, -1, "1FTCR10UXTPA78180", -1, 0, 1, 246, "https://www.vinquery.com/img/vinbarcode/vinbarcode4.jpg",
                    "100101101101011010010101101011011001010101011011001011011010010101101010110010110100101011010100110110101100101010110100101101011010101101100101011011010010110101001011010100101101101101001011010110100101011011010010110101010011011010100101101101"
                },
        /* 40*/ { BARCODE_VIN, 1, "2FTPX28L0XCA15511", -1, 0, 1, 259, "With Import 'I' prefix; https://www.vinquery.com/img/vinbarcode/vinbarcode1.jpg",
                    "1001011011010101101001101010110010101101011011001010101011011001010110110100101001011010110101100101011011010010110101011010100110101001101101010010110101101101101001010110101001011011010010101101101001101010110100110101011010010101101101001010110100101101101"
                },
        /* 41*/ { BARCODE_HIBC_39, -1, "A123BJC5D6E71", -1, 0, 1, 271, "ANSI/HIBC 2.6 - 2016 Figure 2, same",
                    "1000101110111010100010100010001011101010001011101110100010101110101110001010111011101110001010101011101000101110101011100011101011101110100010101110100011101010101011100010111010111000111010101110101110001010101000101110111011101000101011101010100011101110100010111011101"
                },
        /* 42*/ { BARCODE_HIBC_39, -1, "$$52001510X3G", -1, 0, 1, 271, "ANSI/HIBC 2.6 - 2016 Figure 6, same",
                    "1000101110111010100010100010001010001000100010101000100010001010111010001110101010111000101011101010001110111010101000111011101011101000101011101110100011101010111010001010111010100011101110101000101110101110111011100010101010101000111011101010111000101110100010111011101"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %d, %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
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
                    ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[8192 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITER_MILLES   5
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)
#define TEST_PERF_TIME(arg) ((arg) * 1000.0 / CLOCKS_PER_SEC)

/* Not a real test, just performance indicator */
static void test_perf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE39, -1, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+", 0, 1, 1130, "CODE39 85" },
        /*  1*/ { BARCODE_CODE39, -1, "123456ABCD", 0, 1, 155, "CODE39 10" },
        /*  2*/ { BARCODE_CODE93, -1,
                    "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037 !\"#$%&'()*+,-./0123456789ABCDEFGHIJ",
                    0, 1, 1000, "CODE93 107 symbol chars" },
        /*  3*/ { BARCODE_CODE93, -1, "123456ABCD", 0, 1, 127, "CODE93 10" },
        /*  4*/ { BARCODE_CODE11, -1, "1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-", 0, 1, 966, "CODE11 121" },
        /*  5*/ { BARCODE_CODE11, -1, "1234567890-", 0, 1, 116, "CODE11 5" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start;
    clock_t total_create = 0, total_encode = 0, total_buffer = 0, total_buf_inter = 0, total_print = 0;
    clock_t diff_create, diff_encode, diff_buffer, diff_buf_inter, diff_print;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = diff_buf_inter = diff_print = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, DATA_MODE, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

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

            symbol->output_options |= OUT_BUFFER_INTERMEDIATE;
            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buf_inter += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer OUT_BUFFER_INTERMEDIATE ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; /* Undo */

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

            ZBarcode_Delete(symbol);
        }

        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_buf_inter), TEST_PERF_TIME(diff_print), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
        total_buf_inter += diff_buf_inter;
        total_print += diff_print;
    }
    if (p_ctx->index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter), TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_hrt", test_hrt },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_perf", test_perf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
