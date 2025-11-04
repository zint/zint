/*
    libzint - the open source barcode library
    Copyright (C) 2020-2025 Robin Stuart <rstuart114@gmail.com>

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
        const char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_MSI_PLESSEY, -1, "9", 92, 0, 1, 1111, "" },
        /*  1*/ { BARCODE_MSI_PLESSEY, -1, "9", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /*  2*/ { BARCODE_MSI_PLESSEY, 1, "9", 92, 0, 1, 1123, "" }, /* 1 mod-10 check digit */
        /*  3*/ { BARCODE_MSI_PLESSEY, 1, "9", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /*  4*/ { BARCODE_MSI_PLESSEY, 2, "9", 92, 0, 1, 1135, "" }, /* 2 mod-10 check digits */
        /*  5*/ { BARCODE_MSI_PLESSEY, 2, "9", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /*  6*/ { BARCODE_MSI_PLESSEY, 3, "9", 92, 0, 1, 1123, "" }, /* 1 mod-11 check digit */
        /*  7*/ { BARCODE_MSI_PLESSEY, 3, "9", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /*  8*/ { BARCODE_MSI_PLESSEY, 3, "4", 92, 0, 1, 1135, "" }, /* 1 mod-11 double check digit "10" */
        /*  9*/ { BARCODE_MSI_PLESSEY, 3, "4", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /* 10*/ { BARCODE_MSI_PLESSEY, 4, "9", 92, 0, 1, 1135, "" }, /* 1 mod-11 and 1 mod-10 check digit */
        /* 11*/ { BARCODE_MSI_PLESSEY, 4, "9", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /* 12*/ { BARCODE_MSI_PLESSEY, 4, "4", 92, 0, 1, 1147, "" }, /* 1 mod-11 double check digit "10" and 1 mod-10 check digit */
        /* 13*/ { BARCODE_MSI_PLESSEY, 4, "4", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /* 14*/ { BARCODE_MSI_PLESSEY, 5, "9", 92, 0, 1, 1123, "" }, /* 1 NCR mod-11 check digit */
        /* 15*/ { BARCODE_MSI_PLESSEY, 5, "9", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /* 16*/ { BARCODE_MSI_PLESSEY, 6, "9", 92, 0, 1, 1135, "" }, /* 1 NCR mod-11 and 1 mod-10 check digit */
        /* 17*/ { BARCODE_MSI_PLESSEY, 6, "9", 93, ZINT_ERROR_TOO_LONG, -1, -1, "Error 372: Input length 93 too long (maximum 92)" },
        /* 18*/ { BARCODE_PLESSEY, -1, "A", 67, 0, 1, 1139, "" },
        /* 19*/ { BARCODE_PLESSEY, -1, "A", 68, ZINT_ERROR_TOO_LONG, -1, -1, "Error 370: Input length 68 too long (maximum 67)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[4096] = {0}; /* Suppress clang -fsanitize=memory false positive */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data_buf), length);
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
        int output_options;
        const char *data;

        const char *expected;
        const char *expected_content;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_MSI_PLESSEY, -1, -1, "1234567", "1234567", "" },
        /*  1*/ { BARCODE_MSI_PLESSEY, -1, BARCODE_CONTENT_SEGS, "1234567", "1234567", "1234567" },
        /*  2*/ { BARCODE_MSI_PLESSEY, 0, -1, "1234567", "1234567", "" },
        /*  3*/ { BARCODE_MSI_PLESSEY, 0, BARCODE_CONTENT_SEGS, "1234567", "1234567", "1234567" },
        /*  4*/ { BARCODE_MSI_PLESSEY, 1, -1, "1234567", "12345674", "" },
        /*  5*/ { BARCODE_MSI_PLESSEY, 1, BARCODE_CONTENT_SEGS, "1234567", "12345674", "12345674" },
        /*  6*/ { BARCODE_MSI_PLESSEY, 1 + 10, -1, "1234567", "1234567", "" },
        /*  7*/ { BARCODE_MSI_PLESSEY, 1 + 10, BARCODE_CONTENT_SEGS, "1234567", "1234567", "12345674" },
        /*  8*/ { BARCODE_MSI_PLESSEY, 1, -1, "9999999999", "99999999990", "" },
        /*  9*/ { BARCODE_MSI_PLESSEY, 1, BARCODE_CONTENT_SEGS, "9999999999", "99999999990", "99999999990" },
        /* 10*/ { BARCODE_MSI_PLESSEY, 2, -1, "1234567", "123456741", "" },
        /* 11*/ { BARCODE_MSI_PLESSEY, 2, BARCODE_CONTENT_SEGS, "1234567", "123456741", "123456741" },
        /* 12*/ { BARCODE_MSI_PLESSEY, 2 + 10, -1, "1234567", "1234567", "" },
        /* 13*/ { BARCODE_MSI_PLESSEY, 2 + 10, BARCODE_CONTENT_SEGS, "1234567", "1234567", "123456741" },
        /* 14*/ { BARCODE_MSI_PLESSEY, 2, -1, "9999999999", "999999999900", "" },
        /* 15*/ { BARCODE_MSI_PLESSEY, 2, BARCODE_CONTENT_SEGS, "9999999999", "999999999900", "999999999900" },
        /* 16*/ { BARCODE_MSI_PLESSEY, 3, -1, "1234567", "12345674", "" },
        /* 17*/ { BARCODE_MSI_PLESSEY, 3, BARCODE_CONTENT_SEGS, "1234567", "12345674", "12345674" },
        /* 18*/ { BARCODE_MSI_PLESSEY, 3 + 10, -1, "1234567", "1234567", "" },
        /* 19*/ { BARCODE_MSI_PLESSEY, 3 + 10, BARCODE_CONTENT_SEGS, "1234567", "1234567", "12345674" },
        /* 20*/ { BARCODE_MSI_PLESSEY, 3, -1, "9999999999", "99999999995", "" },
        /* 21*/ { BARCODE_MSI_PLESSEY, 3, BARCODE_CONTENT_SEGS, "9999999999", "99999999995", "99999999995" },
        /* 22*/ { BARCODE_MSI_PLESSEY, 4, -1, "1234567", "123456741", "" },
        /* 23*/ { BARCODE_MSI_PLESSEY, 4, BARCODE_CONTENT_SEGS, "1234567", "123456741", "123456741" },
        /* 24*/ { BARCODE_MSI_PLESSEY, 4 + 10, -1, "1234567", "1234567", "" },
        /* 25*/ { BARCODE_MSI_PLESSEY, 4 + 10, BARCODE_CONTENT_SEGS, "1234567", "1234567", "123456741" },
        /* 26*/ { BARCODE_MSI_PLESSEY, 4, -1, "9999999999", "999999999959", "" },
        /* 27*/ { BARCODE_MSI_PLESSEY, 4, BARCODE_CONTENT_SEGS, "9999999999", "999999999959", "999999999959" },
        /* 28*/ { BARCODE_MSI_PLESSEY, 5, -1, "1234567", "12345679", "" },
        /* 29*/ { BARCODE_MSI_PLESSEY, 5, BARCODE_CONTENT_SEGS, "1234567", "12345679", "12345679" },
        /* 30*/ { BARCODE_MSI_PLESSEY, 5 + 10, -1, "1234567", "1234567", "" },
        /* 31*/ { BARCODE_MSI_PLESSEY, 5 + 10, BARCODE_CONTENT_SEGS, "1234567", "1234567", "12345679" },
        /* 32*/ { BARCODE_MSI_PLESSEY, 5, -1, "9999999999", "999999999910", "" },
        /* 33*/ { BARCODE_MSI_PLESSEY, 5, BARCODE_CONTENT_SEGS, "9999999999", "999999999910", "999999999910" },
        /* 34*/ { BARCODE_MSI_PLESSEY, 6, -1, "1234567", "123456790", "" },
        /* 35*/ { BARCODE_MSI_PLESSEY, 6, BARCODE_CONTENT_SEGS, "1234567", "123456790", "123456790" },
        /* 36*/ { BARCODE_MSI_PLESSEY, 6 + 10, -1, "1234567", "1234567", "" },
        /* 37*/ { BARCODE_MSI_PLESSEY, 6 + 10, BARCODE_CONTENT_SEGS, "1234567", "1234567", "123456790" },
        /* 38*/ { BARCODE_MSI_PLESSEY, 6, -1, "9999999999", "9999999999109", "" },
        /* 39*/ { BARCODE_MSI_PLESSEY, 6, BARCODE_CONTENT_SEGS, "9999999999", "9999999999109", "9999999999109" },
        /* 40*/ { BARCODE_MSI_PLESSEY, 1, -1, "123456", "1234566", "" },
        /* 41*/ { BARCODE_MSI_PLESSEY, 1, BARCODE_CONTENT_SEGS, "123456", "1234566", "1234566" },
        /* 42*/ { BARCODE_MSI_PLESSEY, 2, -1, "123456", "12345666", "" },
        /* 43*/ { BARCODE_MSI_PLESSEY, 2, BARCODE_CONTENT_SEGS, "123456", "12345666", "12345666" },
        /* 44*/ { BARCODE_MSI_PLESSEY, 3, -1, "123456", "1234560", "" },
        /* 45*/ { BARCODE_MSI_PLESSEY, 3, BARCODE_CONTENT_SEGS, "123456", "1234560", "1234560" },
        /* 46*/ { BARCODE_MSI_PLESSEY, 4, -1, "123456", "12345609", "" },
        /* 47*/ { BARCODE_MSI_PLESSEY, 4, BARCODE_CONTENT_SEGS, "123456", "12345609", "12345609" },
        /* 48*/ { BARCODE_MSI_PLESSEY, 3, -1, "2211", "221110", "" }, /* Mod-11 check digit '10' */
        /* 49*/ { BARCODE_MSI_PLESSEY, 3, BARCODE_CONTENT_SEGS, "2211", "221110", "221110" },
        /* 50*/ { BARCODE_MSI_PLESSEY, 3 + 10, -1, "2211", "2211", "" }, /* Mod-11 check digit '10' */
        /* 51*/ { BARCODE_MSI_PLESSEY, 3 + 10, BARCODE_CONTENT_SEGS, "2211", "2211", "221110" },
        /* 52*/ { BARCODE_MSI_PLESSEY, 4, -1, "2211", "2211100", "" },
        /* 53*/ { BARCODE_MSI_PLESSEY, 4, BARCODE_CONTENT_SEGS, "2211", "2211100", "2211100" },
        /* 54*/ { BARCODE_MSI_PLESSEY, 4 + 10, -1, "2211", "2211", "" },
        /* 55*/ { BARCODE_MSI_PLESSEY, 4 + 10, BARCODE_CONTENT_SEGS, "2211", "2211", "2211100" },
        /* 56*/ { BARCODE_PLESSEY, -1, -1, "0123456789ABCDEF", "0123456789ABCDEF", "" },
        /* 57*/ { BARCODE_PLESSEY, -1, BARCODE_CONTENT_SEGS, "0123456789ABCDEF", "0123456789ABCDEF", "0123456789ABCDEF90" },
        /* 58*/ { BARCODE_PLESSEY, 1, -1, "0123456789ABCDEF", "0123456789ABCDEF90", "" },
        /* 59*/ { BARCODE_PLESSEY, 1, BARCODE_CONTENT_SEGS, "0123456789ABCDEF", "0123456789ABCDEF90", "0123456789ABCDEF90" },
        /* 60*/ { BARCODE_PLESSEY, -1, -1, "1", "1", "" },
        /* 61*/ { BARCODE_PLESSEY, -1, BARCODE_CONTENT_SEGS, "1", "1", "173" },
        /* 62*/ { BARCODE_PLESSEY, 1, -1, "1", "173", "" },
        /* 63*/ { BARCODE_PLESSEY, 1, BARCODE_CONTENT_SEGS, "1", "173", "173" },
        /* 64*/ { BARCODE_PLESSEY, -1, -1, "7", "7", "" },
        /* 65*/ { BARCODE_PLESSEY, -1, BARCODE_CONTENT_SEGS, "7", "7", "758" },
        /* 66*/ { BARCODE_PLESSEY, 1, -1, "7", "758", "" },
        /* 67*/ { BARCODE_PLESSEY, 1, BARCODE_CONTENT_SEGS, "7", "758", "758" },
        /* 68*/ { BARCODE_PLESSEY, -1, -1, "75", "75", "" },
        /* 69*/ { BARCODE_PLESSEY, -1, BARCODE_CONTENT_SEGS, "75", "75", "7580" },
        /* 70*/ { BARCODE_PLESSEY, 1, -1, "75", "7580", "" },
        /* 71*/ { BARCODE_PLESSEY, 1, BARCODE_CONTENT_SEGS, "75", "7580", "7580" },
        /* 72*/ { BARCODE_PLESSEY, -1, -1, "993", "993", "" },
        /* 73*/ { BARCODE_PLESSEY, -1, BARCODE_CONTENT_SEGS, "993", "993", "993AA" },
        /* 74*/ { BARCODE_PLESSEY, 1, -1, "993", "993AA", "" },
        /* 75*/ { BARCODE_PLESSEY, 1, BARCODE_CONTENT_SEGS, "993", "993AA", "993AA" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;
    int expected_length, expected_content_length;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                    -1 /*option_1*/, data[i].option_2, -1, data[i].output_options,
                    data[i].data, -1, debug);
        expected_length = (int) strlen(data[i].expected);
        expected_content_length = (int) strlen(data[i].expected_content);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d\n",
                    i, symbol->text_length, expected_length);
        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->text, data[i].expected);
        if (symbol->output_options & BARCODE_CONTENT_SEGS) {
            assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
            assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
            assert_equal(symbol->content_segs[0].length, expected_content_length,
                        "i:%d content_segs[0].length %d != expected_content_length %d\n",
                        i, symbol->content_segs[0].length, expected_content_length);
            assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected_content, expected_content_length),
                        "i:%d memcmp(%.*s, %s, %d) != 0\n",
                        i, symbol->content_segs[0].length, symbol->content_segs[0].source, data[i].expected_content,
                        expected_content_length);
        } else {
            assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_2;
        const char *data;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_MSI_PLESSEY, -1, "1", 0, 1, 19, "" },
        /*  1*/ { BARCODE_MSI_PLESSEY, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 377: Invalid character at position 1 in input (digits only)" },
        /*  2*/ { BARCODE_MSI_PLESSEY, -2, "1", 0, 1, 19, "" }, /* < 0 ignored */
        /*  3*/ { BARCODE_MSI_PLESSEY, 7, "1", 0, 1, 19, "" }, /* > 6 ignored */
        /*  4*/ { BARCODE_PLESSEY, -1, "A", 0, 1, 83, "" },
        /*  5*/ { BARCODE_PLESSEY, -1, "G", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 371: Invalid character at position 1 in input (digits and \"ABCDEF\" only)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);

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
        const char *data;
        int ret;

        int expected_rows;
        int expected_width;
        const char *comment;
        const char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_MSI_PLESSEY, -1, "1234567890", 0, 1, 127, "Verified manually against tec-it",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001"
                },
        /*  1*/ { BARCODE_MSI_PLESSEY, 1, "1234567890", 0, 1, 139, "Verified manually against tec-it",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001101101001"
                },
        /*  2*/ { BARCODE_MSI_PLESSEY, 2, "1234567890", 0, 1, 151, "Verified manually against tec-it",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001101101001001001101001"
                },
        /*  3*/ { BARCODE_MSI_PLESSEY, 3, "1234567890", 0, 1, 139, "",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001101101001"
                },
        /*  4*/ { BARCODE_MSI_PLESSEY, 4, "1234567890", 0, 1, 151, "",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001101101001001001101001"
                },
        /*  5*/ { BARCODE_MSI_PLESSEY, 5, "1234567890", 0, 1, 139, "",
                    "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001001001001"
                },
        /*  6*/ { BARCODE_MSI_PLESSEY, 6, "1234567890", 0, 1, 151, "",
                   "1101001001001101001001101001001001101101001101001001001101001101001101101001001101101101101001001001101001001101001001001001001001001001001101101101001"
                },
        /*  7*/ { BARCODE_MSI_PLESSEY, 3, "2211", 0, 1, 79, "Produces mod-11 '10' check digit; BWIPP (badmod11)",
                    "1101001001101001001001101001001001001101001001001101001001001101001001001001001"
                },
        /*  8*/ { BARCODE_MSI_PLESSEY, 4, "2211", 0, 1, 91, "BWIPP (badmod11)",
                    "1101001001101001001001101001001001001101001001001101001001001101001001001001001001001001001"
                },
        /*  9*/ { BARCODE_PLESSEY, -1, "0123456789ABCDEF", 0, 1, 323, "",
                    "11101110100011101000100010001000111010001000100010001110100010001110111010001000100010001110100011101000111010001000111011101000111011101110100010001000100011101110100010001110100011101000111011101110100011101000100011101110111010001110111010001110111011101110111011101110111010001000111010001000100010001110001000101110111"
                },
        /* 10*/ { BARCODE_MSI_PLESSEY, 4, "999999999999999999", 0, 1, 247, "Max value (previously); #209 check buffer not overrun",
                    "1101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001101101001001001001001001101001"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char bwipp_buf[4096];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
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
                    ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf), NULL);
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

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_hrt", test_hrt },
        { "test_input", test_input },
        { "test_encode", test_encode },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
