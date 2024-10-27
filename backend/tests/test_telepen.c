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
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, "\177", 69, 0, 1, 1152, "" },
        /*  1*/ { BARCODE_TELEPEN, "\177", 70, ZINT_ERROR_TOO_LONG, -1, -1, "Error 390: Input length 70 too long (maximum 69)" },
        /*  2*/ { BARCODE_TELEPEN_NUM, "1", 136, 0, 1, 1136, "" },
        /*  3*/ { BARCODE_TELEPEN_NUM, "1", 137, ZINT_ERROR_TOO_LONG, -1, -1, "Error 392: Input length 137 too long (maximum 136)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[256];

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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
        char *data;
        int length;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, "ABC1234.;$", -1, "ABC1234.;$" },
        /*  1*/ { BARCODE_TELEPEN, "abc1234.;$", -1, "abc1234.;$" },
        /*  2*/ { BARCODE_TELEPEN, "ABC1234\001", -1, "ABC1234\001" },
        /*  3*/ { BARCODE_TELEPEN, "ABC\0001234", 8, "ABC 1234" },
        /*  4*/ { BARCODE_TELEPEN_NUM, "1234", -1, "1234" },
        /*  5*/ { BARCODE_TELEPEN_NUM, "123X", -1, "123X" },
        /*  6*/ { BARCODE_TELEPEN_NUM, "123x", -1, "123X" }, /* Converts to upper */
        /*  7*/ { BARCODE_TELEPEN_NUM, "12345", -1, "012345" }, /* Adds leading zero if odd */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_hrt", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

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
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, " !\"#$%&'()*+,-./0123456789:;<", -1, 0, 1, 512, "" },
        /*  1*/ { BARCODE_TELEPEN, "AZaz\176\001", -1, 0, 1, 144, "" },
        /*  2*/ { BARCODE_TELEPEN, "\000\177", 2, 0, 1, 80, "" },
        /*  3*/ { BARCODE_TELEPEN, "é", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 391: Invalid character at position 1 in input, extended ASCII not allowed" },
        /*  4*/ { BARCODE_TELEPEN_NUM, "1234567890", -1, 0, 1, 128, "" },
        /*  5*/ { BARCODE_TELEPEN_NUM, "123456789A", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 393: Invalid character at position 10 in input (digits and \"X\" only)" },
        /*  6*/ { BARCODE_TELEPEN_NUM, "123456789X", -1, 0, 1, 128, "" }, /* [0-9]X allowed */
        /*  7*/ { BARCODE_TELEPEN_NUM, "12345678X9", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 394: Invalid odd position 9 of \"X\" in Telepen data" }, /* X[0-9] not allowed */
        /*  8*/ { BARCODE_TELEPEN_NUM, "1X34567X9X", -1, 0, 1, 128, "" }, /* [0-9]X allowed multiple times */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

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

/* Telepen Barcode Symbology information and History (BSiH)
     https://telepen.co.uk/wp-content/uploads/2018/10/Barcode-Symbology-information-and-History.pdf */
/* E2326U: SB Telepen Barcode Fonts Guide Issue 2 (Apr 2009)
     https://telepen.co.uk/wp-content/uploads/2018/09/SB-Telepen-Barcode-Fonts-V2.pdf */
static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, "1A", -1, 0, 1, 80, "Telepen BSiH Example, same",
                    "10101010101110001011101000100010101110111011100010100010001110101110001010101010"
                },
        /*  1*/ { BARCODE_TELEPEN, "ABC", -1, 0, 1, 96, "Telepen E2326U Example, same",
                    "101010101011100010111011101110001110001110111000101011101110101011101000101000101110001010101010"
                },
        /*  2*/ { BARCODE_TELEPEN, "RST", -1, 0, 1, 96, "Verified manually against tec-it",
                    "101010101011100011100011100010101010111010111000111010111000101010111000111011101110001010101010"
                },
        /*  3*/ { BARCODE_TELEPEN, "?@", -1, 0, 1, 80, "ASCII count 127, check 0; verified manually against tec-it",
                    "10101010101110001010101010101110111011101110101011101110111011101110001010101010"
                },
        /*  4*/ { BARCODE_TELEPEN, "\000", 1, 0, 1, 64, "Verified manually against tec-it",
                    "1010101010111000111011101110111011101110111011101110001010101010"
                },
        /*  5*/ { BARCODE_TELEPEN_NUM, "1234567890", -1, 0, 1, 128, "Verified manually against tec-it",
                    "10101010101110001010101110101110101000101010001010101110101110001011101010001000101110001010101010101011101010101110001010101010"
                },
        /*  6*/ { BARCODE_TELEPEN_NUM, "123456789", -1, 0, 1, 128, "Verified manually against tec-it (012345679)",
                    "10101010101110001110101010111010111000100010001011101110001110001000101010001010111010100010100010111000101110101110001010101010"
                },
        /*  7*/ { BARCODE_TELEPEN_NUM, "123X", -1, 0, 1, 80, "Verified manually against tec-it",
                    "10101010101110001010101110101110111010111000111011101011101110001110001010101010"
                },
        /*  8*/ { BARCODE_TELEPEN_NUM, "1X3X", -1, 0, 1, 80, "Verified manually against tec-it",
                    "10101010101110001110001110001110111010111000111010111010101110001110001010101010"
                },
        /*  9*/ { BARCODE_TELEPEN_NUM, "3637", -1, 0, 1, 80, "Glyph count 127, check 0; verified manually against tec-it",
                    "10101010101110001010101010101110111011101110101011101110111011101110001010101010"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char bwipp_buf[8192];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol("test_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, \"%s\", %d, %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
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

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf), NULL);
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

/* #181 Nico Gunkel OSS-Fuzz */
static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        char *data;
        int length;
        int ret;
    };
    /* Note NULs where using DELs code (16 binary characters wide) */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_TELEPEN, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 69, 0 },
        /*  1*/ { BARCODE_TELEPEN, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 70, ZINT_ERROR_TOO_LONG },
        /*  2*/ { BARCODE_TELEPEN_NUM, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 70, ZINT_ERROR_INVALID_DATA },
        /*  3*/ { BARCODE_TELEPEN_NUM, "0404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404040404", 136, 0 },
        /*  4*/ { BARCODE_TELEPEN_NUM, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567", 137, ZINT_ERROR_TOO_LONG },
        /*  5*/ { BARCODE_TELEPEN_NUM, "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000X", 136, 0 },
        /*  6*/ { BARCODE_TELEPEN_NUM, "9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999", 136, 0 },
        /*  7*/ { BARCODE_TELEPEN_NUM, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 4, 0 }, /* Length given, strlen > 137, so pseudo not NUL-terminated */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_fuzz", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->debug |= debug;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static const char TeleTable[128][16] = {
    { "31313131"       }, { "1131313111"     }, { "33313111"     }, { "1111313131"     },
    { "3111313111"     }, { "11333131"       }, { "13133131"     }, { "111111313111"   },
    { "31333111"       }, { "1131113131"     }, { "33113131"     }, { "1111333111"     },
    { "3111113131"     }, { "1113133111"     }, { "1311133111"   }, { "111111113131"   },
    { "3131113111"     }, { "11313331"       }, { "333331"       }, { "111131113111"   },
    { "31113331"       }, { "1133113111"     }, { "1313113111"   }, { "1111113331"     },
    { "31131331"       }, { "113111113111"   }, { "3311113111"   }, { "1111131331"     },
    { "311111113111"   }, { "1113111331"     }, { "1311111331"   }, { "11111111113111" },
    { "31313311"       }, { "1131311131"     }, { "33311131"     }, { "1111313311"     },
    { "3111311131"     }, { "11333311"       }, { "13133311"     }, { "111111311131"   },
    { "31331131"       }, { "1131113311"     }, { "33113311"     }, { "1111331131"     },
    { "3111113311"     }, { "1113131131"     }, { "1311131131"   }, { "111111113311"   },
    { "3131111131"     }, { "1131131311"     }, { "33131311"     }, { "111131111131"   },
    { "3111131311"     }, { "1133111131"     }, { "1313111131"   }, { "111111131311"   },
    { "3113111311"     }, { "113111111131"   }, { "3311111131"   }, { "111113111311"   },
    { "311111111131"   }, { "111311111311"   }, { "131111111311" }, { "11111111111131" },
    { "3131311111"     }, { "11313133"       }, { "333133"       }, { "111131311111"   },
    { "31113133"       }, { "1133311111"     }, { "1313311111"   }, { "1111113133"     },
    { "313333"         }, { "113111311111"   }, { "3311311111"   }, { "11113333"       },
    { "311111311111"   }, { "11131333"       }, { "13111333"     }, { "11111111311111" },
    { "31311133"       }, { "1131331111"     }, { "33331111"     }, { "1111311133"     },
    { "3111331111"     }, { "11331133"       }, { "13131133"     }, { "111111331111"   },
    { "3113131111"     }, { "1131111133"     }, { "33111133"     }, { "111113131111"   },
    { "3111111133"     }, { "111311131111"   }, { "131111131111" }, { "111111111133"   },
    { "31311313"       }, { "113131111111"   }, { "3331111111"   }, { "1111311313"     },
    { "311131111111"   }, { "11331313"       }, { "13131313"     }, { "11111131111111" },
    { "3133111111"     }, { "1131111313"     }, { "33111313"     }, { "111133111111"   },
    { "3111111313"     }, { "111313111111"   }, { "131113111111" }, { "111111111313"   },
    { "313111111111"   }, { "1131131113"     }, { "33131113"     }, { "11113111111111" },
    { "3111131113"     }, { "113311111111"   }, { "131311111111" }, { "111111131113"   },
    { "3113111113"     }, { "11311111111111" }, { "331111111111" }, { "111113111113"   },
    { "31111111111111" }, { "111311111113"   }, { "131111111113" },
    {'1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
};

/* Dummy to generate lengths table */
static void test_generate_lens(const testCtx *const p_ctx) {

    int i;

    if (!p_ctx->generate) {
        return;
    }

    printf("static const char TeleLens[128] = {");
    for (i = 0; i < 127; i++) {
        if ((i % 16) == 0) {
            printf("\n    %2d,", (int) strlen(TeleTable[i]));
        } else {
            printf(" %2d,", (int) strlen(TeleTable[i]));
        }
    }
    printf(" 16\n};\n");
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_hrt", test_hrt },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_fuzz", test_fuzz },
        { "test_generate_lens", test_generate_lens },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
