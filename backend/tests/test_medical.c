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
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODABAR, -1, "A+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++B", 103, 0, 1, 1133 },
        /*  1*/ { BARCODE_CODABAR, -1, "A++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++B", 104, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_CODABAR, 1, "A+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++B", 103, 0, 1, 1143 },
        /*  3*/ { BARCODE_CODABAR, 1, "A++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++B", 104, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_PHARMA, -1, "131070", 6, 0, 1, 78 },
        /*  5*/ { BARCODE_PHARMA, -1, "1", 7, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_PHARMA_TWO, -1, "64570080", 8, 0, 2, 31 },
        /*  7*/ { BARCODE_PHARMA_TWO, -1, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_CODE32, -1, "1", 8, 0, 1, 103 },
        /*  9*/ { BARCODE_CODE32, -1, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[128];

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

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODABAR, -1, "A1234B", "A1234B" },
        /*  1*/ { BARCODE_CODABAR, -1, "a1234c", "A1234C" }, /* Converts to upper */
        /*  2*/ { BARCODE_CODABAR, 1, "A1234B", "A1234B" }, /* Check not included */
        /*  3*/ { BARCODE_CODABAR, 2, "A1234B", "A12345B" }, /* Check included */
        /*  4*/ { BARCODE_CODABAR, 1, "A123456A", "A123456A" }, /* Check not included */
        /*  5*/ { BARCODE_CODABAR, 2, "A123456A", "A123456$A" }, /* Check included */
        /*  6*/ { BARCODE_PHARMA, -1, "123456", "" }, /* None */
        /*  7*/ { BARCODE_PHARMA_TWO, -1, "123456", "" }, /* None */
        /*  8*/ { BARCODE_CODE32, -1, "123456", "A001234564" },
        /*  9*/ { BARCODE_CODE32, -1, "12345678", "A123456788" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_hrt", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

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

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
        int bwipp_cmp;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODABAR, "A1234B", 0, 1, 62, "", 1, "" },
        /*  1*/ { BARCODE_CODABAR, "1234B", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 358: Does not begin with \"A\", \"B\", \"C\" or \"D\"", 1, "" },
        /*  2*/ { BARCODE_CODABAR, "A1234", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 359: Does not end with \"A\", \"B\", \"C\" or \"D\"", 1, "" },
        /*  3*/ { BARCODE_CODABAR, "A1234E", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 359: Does not end with \"A\", \"B\", \"C\" or \"D\"", 1, "" },
        /*  4*/ { BARCODE_CODABAR, "C123.D", 0, 1, 63, "", 1, "" },
        /*  5*/ { BARCODE_CODABAR, "C123,D", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 357: Invalid character at position 5 in input (\"0123456789-$:/.+ABCD\" only)", 1, "" },
        /*  6*/ { BARCODE_CODABAR, "D:C", 0, 1, 33, "", 1, "" },
        /*  7*/ { BARCODE_CODABAR, "DCC", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 363: Invalid character at position 1 in input (cannot contain \"A\", \"B\", \"C\" or \"D\")", 1, "" },
        /*  8*/ { BARCODE_CODABAR, "A234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123B", ZINT_ERROR_TOO_LONG, -1, -1, "Error 356: Input length 104 too long (maximum 103)", 1, "" },
        /*  9*/ { BARCODE_CODABAR, "AB", ZINT_ERROR_TOO_LONG, -1, -1, "Error 362: Input length 2 too short (minimum 3)", 1, "" },
        /* 10*/ { BARCODE_PHARMA, "131070", 0, 1, 78, "", 1, "" },
        /* 11*/ { BARCODE_PHARMA, "1310700", ZINT_ERROR_TOO_LONG, -1, -1, "Error 350: Input length 7 too long (maximum 6)", 1, "" },
        /* 12*/ { BARCODE_PHARMA, "131071", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Input value '131071' out of range (3 to 131070)", 1, "" },
        /* 13*/ { BARCODE_PHARMA, "3", 0, 1, 4, "", 1, "" },
        /* 14*/ { BARCODE_PHARMA, "2", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Input value '2' out of range (3 to 131070)", 1, "" },
        /* 15*/ { BARCODE_PHARMA, "1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 352: Input value '1' out of range (3 to 131070)", 1, "" },
        /* 16*/ { BARCODE_PHARMA, "12A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 351: Invalid character at position 3 in input (digits only)", 1, "" },
        /* 17*/ { BARCODE_PHARMA_TWO, "64570080", 0, 2, 31, "", 1, "" },
        /* 18*/ { BARCODE_PHARMA_TWO, "64570081", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Input value '64570081' out of range (4 to 64570080)", 1, "" },
        /* 19*/ { BARCODE_PHARMA_TWO, "064570080", ZINT_ERROR_TOO_LONG, -1, -1, "Error 354: Input length 9 too long (maximum 8)", 1, "" },
        /* 20*/ { BARCODE_PHARMA_TWO, "4", 0, 2, 3, "", 1, "" },
        /* 21*/ { BARCODE_PHARMA_TWO, "3", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Input value '3' out of range (4 to 64570080)", 1, "" },
        /* 22*/ { BARCODE_PHARMA_TWO, "2", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Input value '2' out of range (4 to 64570080)", 1, "" },
        /* 23*/ { BARCODE_PHARMA_TWO, "1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 353: Input value '1' out of range (4 to 64570080)", 1, "" },
        /* 24*/ { BARCODE_PHARMA_TWO, "123A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 355: Invalid character at position 4 in input (digits only)", 1, "" },
        /* 25*/ { BARCODE_CODE32, "12345678", 0, 1, 103, "", 1, "" },
        /* 26*/ { BARCODE_CODE32, "9", 0, 1, 103, "", 0, "BWIPP requires length 8 or 9" },
        /* 27*/ { BARCODE_CODE32, "0", 0, 1, 103, "", 0, "BWIPP requires length 8 or 9" },
        /* 28*/ { BARCODE_CODE32, "123456789", ZINT_ERROR_TOO_LONG, -1, -1, "Error 360: Input length 9 too long (maximum 8)", 1, "" },
        /* 29*/ { BARCODE_CODE32, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 361: Invalid character at position 1 in input (digits only)", 1, "" },
        /* 30*/ { BARCODE_CODE32, "99999999", 0, 1, 103, "", 1, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char cmp_buf[8192];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
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

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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
