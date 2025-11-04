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
        /*  0*/ { BARCODE_PHARMA, -1, "131070", 6, 0, 1, 78, "", },
        /*  1*/ { BARCODE_PHARMA, -1, "1", 7, ZINT_ERROR_TOO_LONG, -1, -1, "Error 350: Input length 7 too long (maximum 6)" },
        /*  2*/ { BARCODE_PHARMA_TWO, -1, "64570080", 8, 0, 2, 31, "", },
        /*  3*/ { BARCODE_PHARMA_TWO, -1, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1, "Error 354: Input length 9 too long (maximum 8)" },
        /*  4*/ { BARCODE_CODE32, -1, "1", 8, 0, 1, 103, "", },
        /*  5*/ { BARCODE_CODE32, -1, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1, "Error 360: Input length 9 too long (maximum 8)" },
        /*  6*/ { BARCODE_PZN, -1, "1", 7, 0, 1, 142, "" }, /* Takes 8 with correct check digit */
        /*  7*/ { BARCODE_PZN, -1, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1, "Error 325: Input length 9 too long (maximum 8)" },
        /*  8*/ { BARCODE_PZN, 1, "1", 6, 0, 1, 129, "" }, /* PZN7 takes 7 with correct check digit */
        /*  9*/ { BARCODE_PZN, 1, "1", 8, ZINT_ERROR_TOO_LONG, -1, -1, "Error 325: Input length 8 too long (maximum 7)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[128];

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
        /*  0*/ { BARCODE_PHARMA, -1, -1, "123456", "", "" }, /* None */
        /*  1*/ { BARCODE_PHARMA, -1, BARCODE_CONTENT_SEGS, "123456", "", "123456" },
        /*  2*/ { BARCODE_PHARMA_TWO, -1, -1, "123456", "", "" }, /* None */
        /*  3*/ { BARCODE_PHARMA_TWO, -1, BARCODE_CONTENT_SEGS, "123456", "", "123456" },
        /*  4*/ { BARCODE_CODE32, -1, -1, "123456", "A001234564", "" },
        /*  5*/ { BARCODE_CODE32, -1, BARCODE_CONTENT_SEGS, "123456", "A001234564", "015PN4" }, /* Actual encoded CODE39 value */
        /*  6*/ { BARCODE_CODE32, -1, -1, "12345678", "A123456788", "" },
        /*  7*/ { BARCODE_CODE32, -1, BARCODE_CONTENT_SEGS, "12345678", "A123456788", "3PRM8N" },
        /*  8*/ { BARCODE_CODE32, 1, -1, "12345678", "A123456788", "" }, /* Ignore option_2 re check digits */
        /*  9*/ { BARCODE_CODE32, 1, BARCODE_CONTENT_SEGS, "12345678", "A123456788", "3PRM8N" },
        /* 10*/ { BARCODE_CODE32, 2, -1, "12345678", "A123456788", "" }, /* Ignore option_2 re check digits */
        /* 11*/ { BARCODE_CODE32, 2, BARCODE_CONTENT_SEGS, "12345678", "A123456788", "3PRM8N" },
        /* 12*/ { BARCODE_PZN, -1, -1, "12345", "PZN - 00123458", "" }, /* Pads with zeroes if length < 7 */
        /* 13*/ { BARCODE_PZN, -1, BARCODE_CONTENT_SEGS, "12345", "PZN - 00123458", "-00123458" }, /* Actual encoded CODE39 value */
        /* 14*/ { BARCODE_PZN, -1, -1, "123456", "PZN - 01234562", "" },
        /* 15*/ { BARCODE_PZN, -1, BARCODE_CONTENT_SEGS, "123456", "PZN - 01234562", "-01234562" },
        /* 16*/ { BARCODE_PZN, -1, -1, "1234567", "PZN - 12345678", "" },
        /* 17*/ { BARCODE_PZN, -1, BARCODE_CONTENT_SEGS, "1234567", "PZN - 12345678", "-12345678" },
        /* 18*/ { BARCODE_PZN, -1, -1, "12345678", "PZN - 12345678", "" },
        /* 19*/ { BARCODE_PZN, -1, BARCODE_CONTENT_SEGS, "12345678", "PZN - 12345678", "-12345678" },
        /* 20*/ { BARCODE_PZN, 1, -1, "1234", "PZN - 0012345", "" }, /* PZN7, pads with zeroes if length < 6 */
        /* 21*/ { BARCODE_PZN, 1, BARCODE_CONTENT_SEGS, "1234", "PZN - 0012345", "-0012345" },
        /* 22*/ { BARCODE_PZN, 1, -1, "12345", "PZN - 0123458", "" },
        /* 23*/ { BARCODE_PZN, 1, BARCODE_CONTENT_SEGS, "12345", "PZN - 0123458", "-0123458" },
        /* 24*/ { BARCODE_PZN, 1, -1, "123456", "PZN - 1234562", "" },
        /* 25*/ { BARCODE_PZN, 1, BARCODE_CONTENT_SEGS, "123456", "PZN - 1234562", "-1234562" },
        /* 26*/ { BARCODE_PZN, 1, -1, "1234562", "PZN - 1234562", "" },
        /* 27*/ { BARCODE_PZN, 1, BARCODE_CONTENT_SEGS, "1234562", "PZN - 1234562", "-1234562" },
        /* 28*/ { BARCODE_PZN, 2, -1, "12345", "PZN - 00123458", "" }, /* Ignore option_2 re check digits */
        /* 29*/ { BARCODE_PZN, 2, BARCODE_CONTENT_SEGS, "12345", "PZN - 00123458", "-00123458" },
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
                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, data[i].output_options,
                    data[i].data, -1, debug);
        expected_length = (int) strlen(data[i].expected);
        expected_content_length = (int) strlen(data[i].expected_content);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d (%s)\n",
                    i, symbol->text_length, expected_length, symbol->text);
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
        int bwipp_cmp;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_PHARMA, -1, "131070", 0, 1, 78, "", 1, "" },
        /*  1*/ { BARCODE_PHARMA, -1, "1310700", ZINT_ERROR_TOO_LONG, -1, -1, "Error 350: Input length 7 too long (maximum 6)", 1, "" },
        /*  2*/ { BARCODE_PHARMA, -1, "131071", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Input value '131071' out of range (3 to 131070)", 1, "" },
        /*  3*/ { BARCODE_PHARMA, -1, "3", 0, 1, 4, "", 1, "" },
        /*  4*/ { BARCODE_PHARMA, -1, "2", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Input value '2' out of range (3 to 131070)", 1, "" },
        /*  5*/ { BARCODE_PHARMA, -1, "1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Input value '1' out of range (3 to 131070)", 1, "" },
        /*  6*/ { BARCODE_PHARMA, -1, "12A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 351: Invalid character at position 3 in input (digits only)", 1, "" },
        /*  7*/ { BARCODE_PHARMA_TWO, -1, "64570080", 0, 2, 31, "", 1, "" },
        /*  8*/ { BARCODE_PHARMA_TWO, -1, "64570081", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Input value '64570081' out of range (4 to 64570080)", 1, "" },
        /*  9*/ { BARCODE_PHARMA_TWO, -1, "064570080", ZINT_ERROR_TOO_LONG, -1, -1, "Error 354: Input length 9 too long (maximum 8)", 1, "" },
        /* 10*/ { BARCODE_PHARMA_TWO, -1, "4", 0, 2, 3, "", 1, "" },
        /* 11*/ { BARCODE_PHARMA_TWO, -1, "3", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Input value '3' out of range (4 to 64570080)", 1, "" },
        /* 12*/ { BARCODE_PHARMA_TWO, -1, "2", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Input value '2' out of range (4 to 64570080)", 1, "" },
        /* 13*/ { BARCODE_PHARMA_TWO, -1, "1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Input value '1' out of range (4 to 64570080)", 1, "" },
        /* 14*/ { BARCODE_PHARMA_TWO, -1, "123A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 355: Invalid character at position 4 in input (digits only)", 1, "" },
        /* 15*/ { BARCODE_CODE32, -1, "12345678", 0, 1, 103, "", 1, "" },
        /* 16*/ { BARCODE_CODE32, -1, "9", 0, 1, 103, "", 0, "BWIPP requires length 8 or 9" },
        /* 17*/ { BARCODE_CODE32, -1, "0", 0, 1, 103, "", 0, "BWIPP requires length 8 or 9" },
        /* 18*/ { BARCODE_CODE32, -1, "123456789", ZINT_ERROR_TOO_LONG, -1, -1, "Error 360: Input length 9 too long (maximum 8)", 1, "" },
        /* 19*/ { BARCODE_CODE32, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 361: Invalid character at position 1 in input (digits only)", 1, "" },
        /* 20*/ { BARCODE_CODE32, -1, "99999999", 0, 1, 103, "", 1, "" },
        /* 21*/ { BARCODE_PZN, -1, "1", 0, 1, 142, "", 0, "BWIPP requires 7 or 8 digits" },
        /* 22*/ { BARCODE_PZN, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 326: Invalid character at position 1 in input (digits only)", 1, "" },
        /* 23*/ { BARCODE_PZN, -1, "1000006", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 327: Invalid PZN, check digit is '10'", 1, "" }, /* Check digit == 10 so can't be used */
        /* 24*/ { BARCODE_PZN, -1, "00000011", ZINT_ERROR_INVALID_CHECK, -1, -1, "Error 890: Invalid check digit '1', expecting '7'", 1, "" },
        /* 25*/ { BARCODE_PZN, 1, "100009", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 327: Invalid PZN, check digit is '10'", 1, "" }, /* Check digit == 10 so can't be used */
        /* 26*/ { BARCODE_PZN, 1, "0000011", ZINT_ERROR_INVALID_CHECK, -1, -1, "Error 890: Invalid check digit '1', expecting '7'", 1, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d symbol->errtxt %s != %s\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                if (!data[i].bwipp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    char modules_dump[4096];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
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
        /*  0*/ { BARCODE_PHARMA, -1, "131070", 0, 1, 78, "",
                    "111001110011100111001110011100111001110011100111001110011100111001110011100111"
                },
        /*  1*/ { BARCODE_PHARMA, -1, "123456", 0, 1, 58, "",
                    "1110011100111001001001001110010010011100100100100100100111"
                },
        /*  2*/ { BARCODE_PHARMA_TWO, -1, "64570080", 0, 2, 31, "Verified manually against TEC-IT",
                    "1010101010101010101010101010101"
                    "1010101010101010101010101010101"
                },
        /*  3*/ { BARCODE_PHARMA_TWO, -1, "29876543", 0, 2, 31, "Verified manually against TEC-IT",
                    "0010100010001010001010001000101"
                    "1000101010100000100000101010000"
                },
        /*  4*/ { BARCODE_CODE32, -1, "34567890", 0, 1, 103, "Verified manually against TEC-IT",
                    "1001011011010101101001011010110010110101011011010010101100101101011010010101101010101100110100101101101"
                },
        /*  5*/ { BARCODE_CODE32, 1, "34567890", 0, 1, 103, "Make sure option_2 doesn't add extra check digit",
                    "1001011011010101101001011010110010110101011011010010101100101101011010010101101010101100110100101101101"
                },
        /*  6*/ { BARCODE_CODE32, 2, "34567890", 0, 1, 103, "Make sure option_2 doesn't add extra check digit",
                    "1001011011010101101001011010110010110101011011010010101100101101011010010101101010101100110100101101101"
                },
        /*  7*/ { BARCODE_PZN, -1, "1234567", 0, 1, 142, "Example from IFA Info Code 39 EN V2.1; verified manually against TEC-IT",
                    "1001011011010100101011011011010010101101011001010110110110010101010100110101101101001101010101100110101010100101101101101001011010100101101101"
                },
        /*  8*/ { BARCODE_PZN, -1, "2758089", 0, 1, 142, "Example from IFA Info Check Digit Calculations EN 15 July 2019; verified manually against TEC-IT",
                    "1001011011010100101011011010110010101101010010110110110100110101011010010110101010011011010110100101101010110010110101011001011010100101101101"
                },
        /*  9*/ { BARCODE_PZN, 1, "123456", 0, 1, 129, "Example from BWIPP; verified manually against TEC-IT",
                    "100101101101010010101101101101001010110101100101011011011001010101010011010110110100110101010110011010101011001010110100101101101"
                },
        /* 10*/ { BARCODE_PZN, 2, "1234567", 0, 1, 142, "Make sure option_2 ignored for check digit",
                    "1001011011010100101011011011010010101101011001010110110110010101010100110101101101001101010101100110101010100101101101101001011010100101101101"
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
                    ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[8192 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, 1 /*zxingcpp_cmp*/, cmp_buf,
                                sizeof(cmp_buf), &cmp_len);
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
