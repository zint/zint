/*
    libzint - the open source barcode library
    Copyright (C) 2024 Antoine Merino <antoine.merino.dev@gmail.com>

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
        /*  0*/ { BARCODE_DXFILMEDGE, -1, "79-7", 0, 2, 23, "DX code 1: 79, DX code 2: 7. DX Extract = 1271. DX Full: X1271X (X is any digit)",
                    "1111101010101010101011110101010011110011100101"
                },
        /*  1*/ { BARCODE_DXFILMEDGE, -1, "1271", 0, 2, 23, "",
                    "1111101010101010101011110101010011110011100101"
                },
        /*  2*/ { BARCODE_DXFILMEDGE, -1, "012710", 0, 2, 23, "",
                    "1111101010101010101011110101010011110011100101"
                },
        /*  3*/ { BARCODE_DXFILMEDGE, -1, "112712", 0, 2, 23, "",
                    "1111101010101010101011110101010011110011100101"
                },
        /*  4*/ { BARCODE_DXFILMEDGE, -1, "1-0", 0, 2, 23, "Lower limit: DX part 1 = 1, DX part 2 = 0",
                    "1111101010101010101011110101000000010000010101"
                },
        /*  5*/ { BARCODE_DXFILMEDGE, -1, "000160", 0, 2, 23, "",
                    "1111101010101010101011110101000000010000010101"
                },
        /*  6*/ { BARCODE_DXFILMEDGE, -1, "16", 0, 2, 23, "",
                    "1111101010101010101011110101000000010000010101"
                },
        /*  7*/ { BARCODE_DXFILMEDGE, -1, "920479/63A", 0, 2, 31, "Upper limit: DX part 1 = 127, DX part 2 = 15",
                    "11111010101010101010101010101111010101111111011111111111000101"
                },
        /*  8*/ { BARCODE_DXFILMEDGE, -1, "127-15/00A", 0, 2, 31, "",
                    "11111010101010101010101010101111010101111111011111111111000101"
                },
        /*  9*/ { BARCODE_DXFILMEDGE, -1, "79-7/1", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000010010101"
                },
        /* 10*/ { BARCODE_DXFILMEDGE, -1, "0079-7/001", 0, 2, 31, "Optional behaviour: leading zeros are accepted",
                    "11111010101010101010101010101111010101001111001110000010010101"
                },
        /* 11*/ { BARCODE_DXFILMEDGE, -1, "79-7/1", 0, 2, 31, "Frame number",
                    "11111010101010101010101010101111010101001111001110000010010101"
                },
        /* 12*/ { BARCODE_DXFILMEDGE, -1, "1271/1", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000010010101"
                },
        /* 13*/ { BARCODE_DXFILMEDGE, -1, "912718/1", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000010010101"
                },
        /* 14*/ { BARCODE_DXFILMEDGE, -1, "79-7/1A", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000011000101"
                },
        /* 15*/ { BARCODE_DXFILMEDGE, -1, "1271/1a", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000011000101"
                },
        /* 16*/ { BARCODE_DXFILMEDGE, -1, "212715/1A", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000011000101"
                },
        /* 17*/ { BARCODE_DXFILMEDGE, -1, "79-7/62", 0, 2, 31, "Special frame numbers",
                    "11111010101010101010101010101111010101001111001111111100010101"
                },
        /* 18*/ { BARCODE_DXFILMEDGE, -1, "79-7/S", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111100010101"
                },
        /* 19*/ { BARCODE_DXFILMEDGE, -1, "79-7/x", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111100010101"
                },
        /* 20*/ { BARCODE_DXFILMEDGE, -1, "79-7/62a", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111101000101"
                },
        /* 21*/ { BARCODE_DXFILMEDGE, -1, "79-7/sA", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111101000101"
                },
        /* 22*/ { BARCODE_DXFILMEDGE, -1, "79-7/Xa", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111101000101"
                },
        /* 23*/ { BARCODE_DXFILMEDGE, -1, "79-7/63", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111110000101"
                },
        /* 24*/ { BARCODE_DXFILMEDGE, -1, "79-7/k", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111110000101"
                },
        /* 25*/ { BARCODE_DXFILMEDGE, -1, "79-7/00", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111110000101"
                },
        /* 26*/ { BARCODE_DXFILMEDGE, -1, "79-7/63a", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111111010101"
                },
        /* 27*/ { BARCODE_DXFILMEDGE, -1, "79-7/kA", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111111010101"
                },
        /* 28*/ { BARCODE_DXFILMEDGE, -1, "79-7/00a", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001111111111010101"
                },
        /* 29*/ { BARCODE_DXFILMEDGE, -1, "79-7/0", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000000000101"
                },
        /* 30*/ { BARCODE_DXFILMEDGE, -1, "79-7/F", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000000000101"
                },
        /* 31*/ { BARCODE_DXFILMEDGE, -1, "79-7/0a", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000001010101"
                },
        /* 32*/ { BARCODE_DXFILMEDGE, -1, "79-7/fA", 0, 2, 31, "",
                    "11111010101010101010101010101111010101001111001110000001010101"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[4096];
    char cmp_msg[1024];

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

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_DXFILMEDGE, -1, "79-1/123A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 972: Frame number part length 4 too long (maximum 3)" },
        /*  1*/ { BARCODE_DXFILMEDGE, -1, "79-1/1@A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 973: Frame number \"1@A\" is invalid (expected digits, optionally followed by a single \"A\")" },
        /*  2*/ { BARCODE_DXFILMEDGE, -1, "012312365", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 974: DX information length 9 too long (maximum 6)" },
        /*  3*/ { BARCODE_DXFILMEDGE, -1, "12-", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 977: Wrong format for DX parts 1 and 2 (expected format: XXX-XX, digits)" },
        /*  4*/ { BARCODE_DXFILMEDGE, -1, "01234/00A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 980: DX number \"01234\" is incorrect; expected 4 digits (DX extract) or 6 digits (DX full)" },
        /*  5*/ { BARCODE_DXFILMEDGE, -1, "0123/0AA", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 983: Frame number \"0AA\" is invalid (expected digits, optionally followed by a single \"A\")" },
        /*  6*/ { BARCODE_DXFILMEDGE, -1, "128-0/24", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 978: DX part 1 \"128\" out of range (1 to 127)" },
        /*  7*/ { BARCODE_DXFILMEDGE, -1, "127-16", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 979: DX part 2 \"16\" out of range (0 to 15)" },
        /*  8*/ { BARCODE_DXFILMEDGE, -1, "79-2/A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 983: Frame number \"A\" is invalid (expected digits, optionally followed by a single \"A\")" },
        /*  9*/ { BARCODE_DXFILMEDGE, -1, "79-2/-1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 984: Frame number \"-1\" out of range (0 to 63)" },
        /* 10*/ { BARCODE_DXFILMEDGE, -1, "79-2/64", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 984: Frame number \"64\" out of range (0 to 63)" },
        /* 11*/ { BARCODE_DXFILMEDGE, -1, "79-2-1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 976: The \"-\" is used to separate DX parts 1 and 2, and should be used no more than once" },
        /* 12*/ { BARCODE_DXFILMEDGE, -1, "110-2/2B", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 985: Frame number \"2B\" is invalid (expected digits, optionally followed by a single \"A\")" },
        /* 13*/ { BARCODE_DXFILMEDGE, -1, "099990/123A", ZINT_ERROR_TOO_LONG, -1, -1, "Error 986: Input length 11 too long (maximum 10)" },
        /* 14*/ { BARCODE_DXFILMEDGE, -1, "0123123/1", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 971: DX information length 7 too long (maximum 6)" },
        /* 15*/ { BARCODE_DXFILMEDGE, -1, "120481", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 981: DX extract \"2048\" out of range (16 to 2047)" },
        /* 16*/ { BARCODE_DXFILMEDGE, -1, "100151", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 981: DX extract \"15\" out of range (16 to 2047)" },
        /* 17*/ { BARCODE_DXFILMEDGE, -1, "15", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 981: DX extract \"15\" out of range (16 to 2047)" },
        /* 18*/ { BARCODE_DXFILMEDGE, -1, "12-12A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 975: Invalid character at position 6 in DX info (digits and \"-\" character only)" },
        /* 19*/ { BARCODE_DXFILMEDGE, -1, "012X", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 975: Invalid character at position 4 in DX info (digits and \"-\" character only)" },
        /* 20*/ { BARCODE_DXFILMEDGE, -1, "110-2/", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 982: Frame number indicator \"/\" at position 6, but frame number is empty" },
        /* 21*/ { BARCODE_DXFILMEDGE, -1, "/", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 970: Invalid first character \"/\", DX code should start with a number" },
        /* 22*/ { BARCODE_DXFILMEDGE, -1, "-12", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 970: Invalid first character \"-\", DX code should start with a number" },
        /* 23*/ { BARCODE_DXFILMEDGE, -1, "X1234X", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 970: Invalid first character \"X\", DX code should start with a number" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, -1 /*output_options*/, data[i].data, -1, debug);

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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_input", test_input },
        { "test_encode", test_encode },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
