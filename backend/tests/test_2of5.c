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
        /*  0*/ { BARCODE_C25STANDARD, -1, "1", 112, 0, 1, 1137, "" },
        /*  1*/ { BARCODE_C25STANDARD, -1, "1", 113, ZINT_ERROR_TOO_LONG, -1, -1, "Error 301: Input length 113 too long (maximum 112)" },
        /*  2*/ { BARCODE_C25STANDARD, 1, "1", 112, 0, 1, 1147, "" },
        /*  3*/ { BARCODE_C25STANDARD, 1, "1", 113, ZINT_ERROR_TOO_LONG, -1, -1, "Error 301: Input length 113 too long (maximum 112)" },
        /*  4*/ { BARCODE_C25INTER, -1, "1", 125, 0, 1, 1143, "" },
        /*  5*/ { BARCODE_C25INTER, -1, "1", 126, ZINT_ERROR_TOO_LONG, -1, -1, "Error 309: Input length 126 too long (maximum 125)" },
        /*  6*/ { BARCODE_C25INTER, 1, "1", 125, 0, 1, 1143, "" },
        /*  7*/ { BARCODE_C25INTER, 1, "1", 126, ZINT_ERROR_TOO_LONG, -1, -1, "Error 309: Input length 126 too long (maximum 125)" },
        /*  8*/ { BARCODE_C25IATA, -1, "1", 80, 0, 1, 1129, "" },
        /*  9*/ { BARCODE_C25IATA, -1, "1", 81, ZINT_ERROR_TOO_LONG, -1, -1, "Error 305: Input length 81 too long (maximum 80)" },
        /* 10*/ { BARCODE_C25IATA, 1, "1", 80, 0, 1, 1143, "" },
        /* 11*/ { BARCODE_C25IATA, 1, "1", 81, ZINT_ERROR_TOO_LONG, -1, -1, "Error 305: Input length 81 too long (maximum 80)" },
        /* 12*/ { BARCODE_C25LOGIC, -1, "1", 113, 0, 1, 1139, "" },
        /* 13*/ { BARCODE_C25LOGIC, -1, "1", 114, ZINT_ERROR_TOO_LONG, -1, -1, "Error 307: Input length 114 too long (maximum 113)" },
        /* 14*/ { BARCODE_C25LOGIC, 1, "1", 113, 0, 1, 1149, "" },
        /* 15*/ { BARCODE_C25LOGIC, 1, "1", 114, ZINT_ERROR_TOO_LONG, -1, -1, "Error 307: Input length 114 too long (maximum 113)" },
        /* 16*/ { BARCODE_C25IND, -1, "1", 79, 0, 1, 1125, "" },
        /* 17*/ { BARCODE_C25IND, -1, "1", 80, ZINT_ERROR_TOO_LONG, -1, -1, "Error 303: Input length 80 too long (maximum 79)" },
        /* 18*/ { BARCODE_C25IND, 1, "1", 79, 0, 1, 1139, "" },
        /* 19*/ { BARCODE_C25IND, 1, "1", 80, ZINT_ERROR_TOO_LONG, -1, -1, "Error 303: Input length 80 too long (maximum 79)" },
        /* 20*/ { BARCODE_DPLEIT, -1, "1", 13, 0, 1, 135, "" },
        /* 21*/ { BARCODE_DPLEIT, -1, "1", 14, ZINT_ERROR_TOO_LONG, -1, -1, "Error 313: Input length 14 too long (maximum 13)" },
        /* 22*/ { BARCODE_DPIDENT, -1, "1", 11, 0, 1, 117, "" },
        /* 23*/ { BARCODE_DPIDENT, -1, "1", 12, ZINT_ERROR_TOO_LONG, -1, -1, "Error 315: Input length 12 too long (maximum 11)" },
        /* 24*/ { BARCODE_ITF14, -1, "1", 13, 0, 1, 135, "" },
        /* 25*/ { BARCODE_ITF14, -1, "1", 14, ZINT_ERROR_TOO_LONG, -1, -1, "Error 311: Input length 14 too long (maximum 13)" },
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
        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_C25STANDARD, -1, "123456789", "123456789" },
        /*  1*/ { BARCODE_C25STANDARD, 1, "123456789", "1234567895" },
        /*  2*/ { BARCODE_C25STANDARD, 2, "123456789", "123456789" }, /* Suppresses printing of check digit */
        /*  3*/ { BARCODE_C25INTER, -1, "123456789", "0123456789" }, /* Adds leading zero if odd */
        /*  4*/ { BARCODE_C25INTER, 1, "123456789", "1234567895" }, /* Unless check digit added when it becomes even */
        /*  5*/ { BARCODE_C25INTER, 2, "123456789", "123456789" },
        /*  6*/ { BARCODE_C25INTER, -1, "1234567890", "1234567890" }, /* No leading zero if even */
        /*  7*/ { BARCODE_C25INTER, 1, "1234567890", "012345678905" }, /* Unless check digit added when it becomes odd */
        /*  8*/ { BARCODE_C25INTER, 2, "1234567890", "01234567890" },
        /*  9*/ { BARCODE_C25IATA, -1, "123456789", "123456789" },
        /* 10*/ { BARCODE_C25IATA, 1, "123456789", "1234567895" },
        /* 11*/ { BARCODE_C25IATA, 2, "123456789", "123456789" },
        /* 12*/ { BARCODE_C25LOGIC, -1, "123456789", "123456789" },
        /* 13*/ { BARCODE_C25LOGIC, 1, "123456789", "1234567895" },
        /* 14*/ { BARCODE_C25LOGIC, 2, "123456789", "123456789" },
        /* 15*/ { BARCODE_C25IND, -1, "123456789", "123456789" },
        /* 16*/ { BARCODE_C25IND, 1, "123456789", "1234567895" },
        /* 17*/ { BARCODE_C25IND, 2, "123456789", "123456789" },
        /* 18*/ { BARCODE_DPLEIT, -1, "123456789", "00001.234.567.890" }, /* Leading zeroes added to make 13 + appended checksum */
        /* 19*/ { BARCODE_DPLEIT, -1, "1234567890123", "12345.678.901.236" },
        /* 20*/ { BARCODE_DPIDENT, -1, "123456789", "00.12 3.456.789 0" }, /* Leading zeroes added to make 11 + appended checksum */
        /* 21*/ { BARCODE_DPIDENT, -1, "12345678901", "12.34 5.678.901 6" },
        /* 22*/ { BARCODE_ITF14, -1, "123456789", "00001234567895" }, /* Leading zeroes added to make 13 + appended checksum */
        /* 23*/ { BARCODE_ITF14, -1, "1234567890123", "12345678901231" },
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
        int input_mode;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_C25STANDARD, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 302: Invalid character at position 1 in input (digits only)" },
        /*  1*/ { BARCODE_C25STANDARD, -1, "1234A6", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 302: Invalid character at position 5 in input (digits only)" },
        /*  2*/ { BARCODE_C25STANDARD, ESCAPE_MODE, "\\d049234A6", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 302: Invalid character at position 5 in input (digits only)" }, /* TODO (maybe): position doesn't account for escapes */
        /*  3*/ { BARCODE_C25INTER, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 310: Invalid character at position 1 in input (digits only)" },
        /*  4*/ { BARCODE_C25IATA, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 306: Invalid character at position 1 in input (digits only)" },
        /*  5*/ { BARCODE_C25LOGIC, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 308: Invalid character at position 1 in input (digits only)" },
        /*  6*/ { BARCODE_C25IND, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 304: Invalid character at position 1 in input (digits only)" },
        /*  7*/ { BARCODE_DPLEIT, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 314: Invalid character at position 1 in input (digits only)" },
        /*  8*/ { BARCODE_DPIDENT, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 316: Invalid character at position 1 in input (digits only)" },
        /*  9*/ { BARCODE_DPIDENT, -1, "1234567890A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 316: Invalid character at position 11 in input (digits only)" },
        /* 10*/ { BARCODE_ITF14, -1, "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 312: Invalid character at position 1 in input (digits only)" },
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
    /* BARCODE_ITF14 examples verified manually against GS1 General Specifications 21.0.1 */
    static const struct item data[] = {
        /*  0*/ { BARCODE_C25STANDARD, -1, "87654321", 0, 1, 97, "Standard Code 2 of 5; note zint uses 4X start/end wides while BWIPP uses 3X",
                    "1111010101110100010101000111010001110101110111010101110111011100010101000101110111010111011110101"
                },
        /*  1*/ { BARCODE_C25STANDARD, 1, "87654321", 0, 1, 107, "With check digit",
                    "11110101011101000101010001110100011101011101110101011101110111000101010001011101110101110100010111011110101"
                },
        /*  2*/ { BARCODE_C25STANDARD, -1, "1234567890", 0, 1, 117, "",
                    "111101010111010111010001011101110001010101110111011101110101000111010101000111011101000101000100010101110001011110101"
                },
        /*  3*/ { BARCODE_C25INTER, -1, "87654321", 0, 1, 81, "Interleaved Code 2 of 5, even; verified manually against tec-it",
                    "101011101010111000100010001110111000101010001000111010111010001110101011100011101"
                },
        /*  4*/ { BARCODE_C25INTER, 1, "87654321", 0, 1, 99, "With check digit",
                    "101010001011101110001010100010001110111011101011100010100011101110001010100011101000101011100011101"
                },
        /*  5*/ { BARCODE_C25INTER, -1, "7654321", 0, 1, 81, "Interleaved Code 2 of 5, odd",
                    "101010101110111000100010001110111000101010001000111010111010001110101011100011101"
                },
        /*  6*/ { BARCODE_C25INTER, 1, "7654321", 0, 1, 81, "With check digit",
                    "101010100010001110111011101011100010100011101110001010100011101010001000111011101"
                },
        /*  7*/ { BARCODE_C25INTER, -1, "602003", 0, 1, 63, "DX cartridge barcode https://en.wikipedia.org/wiki/Interleaved_2_of_5 example",
                    "101010111011100010001010111010001000111010001000111011101011101"
                },
        /*  8*/ { BARCODE_C25IATA, -1, "87654321", 0, 1, 121, "IATA Code 2 of 5; verified manually against tec-it",
                    "1010111010101110101010101110111010111011101010111010111010101010111010111011101110101010101110101011101110101010111011101"
                },
        /*  9*/ { BARCODE_C25IATA, 1, "87654321", 0, 1, 135, "With check digit",
                    "101011101010111010101010111011101011101110101011101011101010101011101011101110111010101010111010101110111010101011101011101010111011101"
                },
        /* 10*/ { BARCODE_C25LOGIC, -1, "87654321", 0, 1, 89, "Code 2 of 5 Data Logic; verified manually against tec-it",
                    "10101110100010101000111010001110101110111010101110111011100010101000101110111010111011101"
                },
        /* 11*/ { BARCODE_C25LOGIC, 1, "87654321", 0, 1, 99, "With check digit",
                    "101011101000101010001110100011101011101110101011101110111000101010001011101110101110100010111011101"
                },
        /* 12*/ { BARCODE_C25IND, -1, "87654321", 0, 1, 131, "Industrial Code 2 of 5; verified manually against tec-it",
                    "11101110101110101011101010101011101110101110111010101110101110101010101110101110111011101010101011101010111011101010101110111010111"
                },
        /* 13*/ { BARCODE_C25IND, 1, "87654321", 0, 1, 145, "With check digit",
                    "1110111010111010101110101010101110111010111011101010111010111010101010111010111011101110101010101110101011101110101010111010111010101110111010111"
                },
        /* 14*/ { BARCODE_C25IND, -1, "1234567890", 0, 1, 159, "",
                    "111011101011101010101110101110101011101110111010101010101110101110111010111010101011101110101010101011101110111010101110101011101011101010101110111010111010111"
                },
        /* 15*/ { BARCODE_DPLEIT, -1, "0000087654321", 0, 1, 135, "Deutsche Post Leitcode; verified manually against tec-it",
                    "101010101110001110001010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000100010111011101"
                },
        /* 16*/ { BARCODE_DPLEIT, 1, "0000087654321", 0, 1, 135, "Check digit option ignored",
                    "101010101110001110001010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000100010111011101"
                },
        /* 17*/ { BARCODE_DPLEIT, -1, "2045703000360", 0, 1, 135, "Deutsche Post DIALOGPOST SCHWER brochure 3.1 example",
                    "101010111010001000111010001011100010111010101000111000111011101110100010001010101110001110001011101110001000101010001011100011101011101"
                },
        /* 18*/ { BARCODE_DPLEIT, -1, "5082300702800", 0, 1, 135, "Deutsche Post Leitcode https://de.wikipedia.org/wiki/Leitcode example",
                    "101011101011100010001011101000101110100011101110100010001010101110111000100010100011101110100011101010001110001010001011100011101011101"
                },
        /* 19*/ { BARCODE_DPIDENT, -1, "00087654321", 0, 1, 117, "Deutsche Post Identcode; verified manually against tec-it (HRT differently formatted)",
                    "101010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000100010111011101"
                },
        /* 20*/ { BARCODE_DPIDENT, -1, "00087654321", 0, 1, 117, "Check digit option ignored",
                    "101010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000100010111011101"
                },
        /* 21*/ { BARCODE_DPIDENT, -1, "80420000001", 0, 1, 117, "Deutsche Post DIALOGPOST SCHWER brochure 3.1 example",
                    "101011101010001110001010100011101011100010101110001110001010101110001110001010101110001110001011101010001000111011101"
                },
        /* 22*/ { BARCODE_DPIDENT, -1, "39601313414", 0, 1, 117, "Deutsche Post Identcode https://de.wikipedia.org/wiki/Leitcode example",
                    "101011101110001010001010111011100010001011100010001010111011100010001010111010001011101011100010101110001000111011101"
                },
        /* 23*/ { BARCODE_ITF14, -1, "0000087654321", 0, 1, 135, "ITF-14; verified manually against tec-it",
                    "101010101110001110001010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000101011100011101"
                },
        /* 24*/ { BARCODE_ITF14, 1, "0000087654321", 0, 1, 135, "Check digit option ignored",
                    "101010101110001110001010101110001110001010001011101110001010100010001110111011101011100010100011101110001010100011101000101011100011101"
                },
        /* 25*/ { BARCODE_ITF14, -1, "0950110153000", 0, 1, 135, "GS1 General Specifications Figure 5.1-2",
                    "101010100011101110001011101011100010001011100010101011100010001011101110100011100010001110101010101110001110001010001000111011101011101"
                },
        /* 26*/ { BARCODE_ITF14, -1, "1540014128876", 0, 1, 135, "GS1 General Specifications Figure 5.3.2.4-1",
                    "101011100010100010111010101110001000111010001011101110100010001011101011100010001110101000111011101010111000100010001110001110101011101"
                },
        /* 27*/ { BARCODE_ITF14, -1, "0950110153001", 0, 1, 135, "GS1 General Specifications Figure 5.3.6-1",
                    "101010100011101110001011101011100010001011100010101011100010001011101110100011100010001110101010101110001110001011101010001000111011101"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char cmp_buf[4096];
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
        /*  0*/ { BARCODE_C25INTER, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, 1, 819, "C25INTER 90" },
        /*  1*/ { BARCODE_C25INTER, -1, "1234567890", 0, 1, 99, "C25INTER 10" },
        /*  2*/ { BARCODE_C25STANDARD, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, 1, 817, "C25STANDARD 80" },
        /*  3*/ { BARCODE_C25STANDARD, -1, "1234567890", 0, 1, 117, "C25STANDARD 10" },
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
