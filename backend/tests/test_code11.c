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

/* Was in "test_code.c" */

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
        /*  0*/ { BARCODE_CODE11, -1, "13", 140, 0, 1, 1151, "" }, /* 8 (Start) + 140*8 + 2*8 (Checks) + 7 (Stop) == 1151 */
        /*  1*/ { BARCODE_CODE11, -1, "13", 141, ZINT_ERROR_TOO_LONG, -1, -1, "Error 320: Input length 141 too long (maximum 140)" },
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
        int length;

        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, "123-45", -1, "123-4552" }, /* 2 checksums */
        /*  1*/ { BARCODE_CODE11, -1, BARCODE_CONTENT_SEGS, "123-45", -1, "123-4552" }, /* No difference */
        /*  2*/ { BARCODE_CODE11, 1, -1, "123-45", -1, "123-455" }, /* 1 check digit */
        /*  3*/ { BARCODE_CODE11, 1, BARCODE_CONTENT_SEGS, "123-45", -1, "123-455" }, /* No difference */
        /*  4*/ { BARCODE_CODE11, 2, -1, "123-45", -1, "123-45" }, /* No checksums */
        /*  5*/ { BARCODE_CODE11, 2, BARCODE_CONTENT_SEGS, "123-45", -1, "123-45" }, /* No difference */
        /*  6*/ { BARCODE_CODE11, -1, -1, "123456789012", -1, "123456789012-8" }, /* First check digit 10 (A) goes to hyphen */
        /*  7*/ { BARCODE_CODE11, -1, BARCODE_CONTENT_SEGS, "123456789012", -1, "123456789012-8" }, /* No difference */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;
    int expected_length;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, data[i].output_options,
                    data[i].data, data[i].length, debug);
        expected_length = (int) strlen(data[i].expected);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d\n",
                    i, symbol->text_length, expected_length);
        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->text, data[i].expected);
        if (symbol->output_options & BARCODE_CONTENT_SEGS) {
            assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
            assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
            assert_equal(symbol->content_segs[0].length, expected_length,
                        "i:%d content_segs[0].length %d != expected_length %d\n",
                        i, symbol->content_segs[0].length, expected_length);
            assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected, expected_length),
                        "i:%d memcmp(%.*s, %s, %d) != 0\n",
                        i, expected_length, symbol->content_segs[0].source, data[i].expected, expected_length);
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
        int input_mode;
        int option_2;
        const char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
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
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
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
        const char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        const char *comment;
        const char *expected;
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

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
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
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, 1 /*zxingcpp_cmp*/, cmp_buf,
                                sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

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
