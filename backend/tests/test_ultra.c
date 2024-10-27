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
        int option_1;
        int option_3;
        struct zint_structapp structapp;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, { 0, 0, "" }, "1", 252, 0, 31, 66, "" }, /* Default EC2 */
        /*  1*/ { -1, -1, { 0, 0, "" }, "1", 253, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /*  2*/ { -1, -1, { 0, 0, "" }, "1", ZINT_MAX_DATA_LEN, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 17403 codewords (maximum 255)" },
        /*  3*/ { -1, -1, { 1, 2, "" }, "1", 251, 0, 31, 66, "" }, /* Structured Append no File Number 1 codeword overhead */
        /*  4*/ { -1, -1, { 1, 2, "" }, "1", 252, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /*  5*/ { -1, -1, { 1, 2, "1" }, "1", 249, 0, 31, 66, "" }, /* Structured Append with File Number 3 codewords overhead */
        /*  6*/ { -1, -1, { 1, 2, "1" }, "1", 250, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /*  7*/ { -1, -1, { 0, 0, "" }, "A", 252, 0, 31, 66, "" },
        /*  8*/ { -1, -1, { 0, 0, "" }, "A", 253, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /*  9*/ { -1, -1, { 0, 0, "" }, "\200", 252, 0, 31, 66, "" },
        /* 10*/ { -1, -1, { 0, 0, "" }, "\200", 253, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /* 11*/ { -1, -1, { 0, 0, "" }, "\001", 252, 0, 31, 66, "" },
        /* 12*/ { -1, -1, { 0, 0, "" }, "\001", 253, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /* 13*/ { -1, ULTRA_COMPRESSION, { 0, 0, "" }, "1", 504, 0, 31, 66, "" },
        /* 14*/ { -1, ULTRA_COMPRESSION, { 0, 0, "" }, "1", 505, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /* 15*/ { -1, ULTRA_COMPRESSION, { 0, 0, "" }, "A", 375, 0, 31, 66, "" },
        /* 16*/ { -1, ULTRA_COMPRESSION, { 0, 0, "" }, "A", 376, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /* 17*/ { -1, ULTRA_COMPRESSION, { 0, 0, "" }, "\200", 252, 0, 31, 66, "" },
        /* 18*/ { -1, ULTRA_COMPRESSION, { 0, 0, "" }, "\200", 253, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /* 19*/ { -1, ULTRA_COMPRESSION, { 0, 0, "" }, "\001", 252, 0, 31, 66, "" },
        /* 20*/ { -1, ULTRA_COMPRESSION, { 0, 0, "" }, "\001", 253, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /* 21*/ { 1, -1, { 0, 0, "" }, "1", 276, 0, 31, 66, "" },
        /* 22*/ { 1, -1, { 0, 0, "" }, "1", 277, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC0, requires 280 codewords (maximum 279)" },
        /* 23*/ { 1, -1, { 1, 2, "" }, "1", 275, 0, 31, 66, "" },
        /* 24*/ { 1, -1, { 1, 2, "" }, "1", 276, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC0, requires 280 codewords (maximum 279)" },
        /* 25*/ { 1, -1, { 1, 2, "1" }, "1", 273, 0, 31, 66, "" },
        /* 26*/ { 1, -1, { 1, 2, "1" }, "1", 274, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC0, requires 280 codewords (maximum 279)" },
        /* 27*/ { 2, -1, { 0, 0, "" }, "1", 263, 0, 31, 66, "" },
        /* 28*/ { 2, -1, { 0, 0, "" }, "1", 264, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC1, requires 267 codewords (maximum 266)" },
        /* 29*/ { 3, -1, { 0, 0, "" }, "1", 252, 0, 31, 66, "" },
        /* 30*/ { 3, -1, { 0, 0, "" }, "1", 253, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)" },
        /* 31*/ { 4, -1, { 0, 0, "" }, "1", 234, 0, 31, 66, "" },
        /* 32*/ { 4, -1, { 0, 0, "" }, "1", 235, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC3, requires 238 codewords (maximum 237)" },
        /* 33*/ { 5, -1, { 0, 0, "" }, "1", 220, 0, 31, 66, "" },
        /* 34*/ { 5, -1, { 0, 0, "" }, "1", 221, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC4, requires 224 codewords (maximum 223)" },
        /* 35*/ { 6, -1, { 0, 0, "" }, "1", 202, 0, 31, 66, "" },
        /* 36*/ { 6, -1, { 0, 0, "" }, "1", 203, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC5, requires 206 codewords (maximum 205)" },
        /* 37*/ { 6, -1, { 0, 0, "" }, "A", ZINT_MAX_DATA_LEN / 2, ZINT_ERROR_TOO_LONG, -1, -1, "Error 591: Input too long for ECC level EC5, requires 8703 codewords (maximum 205)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[ZINT_MAX_DATA_LEN + 1];

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_ULTRA, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data_buf, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

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

static void test_reader_init(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int output_options;
        int option_3;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, READER_INIT, 0, "A", 0, 13, 14, "(3) 257 269 65", "8-bit FNC3 A" },
        /*  1*/ { UNICODE_MODE, READER_INIT, ULTRA_COMPRESSION, "A", 0, 13, 14, "(3) 272 271 65", "ASCII FNC3 A Note: draft spec inconsistent and FNC3 may be 272 in ASCII mode (and FNC1 271)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];

    testStartSymbol("test_reader_init", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_ULTRA, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, data[i].option_3, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
                    testUtilOption3Name(BARCODE_ULTRA, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
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
        int option_1;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "A", 0, "(2) 257 65", "Default (Revision 1)" },
        /*  1*/ { UNICODE_MODE, 0, -1, 1, -1, { 0, 0, "" }, "A", 0, "(2) 257 65", "Revision 1" },
        /*  2*/ { UNICODE_MODE, 0, -1, 2, -1, { 0, 0, "" }, "A", 0, "(2) 257 65", "Revision 2 (no codeword changes)" },
        /*  3*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "A", 0, "(2) 272 65", "" },
        /*  4*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "12", 0, "(3) 257 49 50", "" },
        /*  5*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "12", 0, "(2) 272 140", "" },
        /*  6*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "123", 0, "(4) 257 49 50 51", "" },
        /*  7*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "123", 0, "(3) 272 140 51", "" },
        /*  8*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "ABC", 0, "(4) 257 65 66 67", "" },
        /*  9*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "ABC", 0, "(4) 272 65 66 67", "" },
        /* 10*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "ULTRACODE_123456789!", 0, "(17) 272 85 76 84 82 65 67 79 68 69 95 140 162 184 206 57 33", "" },
        /* 11*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(253) 257 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "252 chars EC2" },
        /* 12*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", ZINT_ERROR_TOO_LONG, "Error 591: Input too long for ECC level EC2, requires 256 codewords (maximum 255)", "253 chars EC2" },
        /* 13*/ { UNICODE_MODE, 0, 1, -1, -1, { 0, 0, "" }, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(277) 257 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "276 chars EC0" },
        /* 14*/ { UNICODE_MODE, 0, 1, -1, -1, { 0, 0, "" }, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", ZINT_ERROR_TOO_LONG, "Error 591: Input too long for ECC level EC0, requires 280 codewords (maximum 279)", "277 chars EC0" },
        /* 15*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "é", 0, "(2) 257 233", "" },
        /* 16*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "β", ZINT_WARN_USES_ECI, "Warning (2) 263 226", "" },
        /* 17*/ { UNICODE_MODE, 9, -1, -1, -1, { 0, 0, "" }, "β", 0, "(2) 263 226", "" },
        /* 18*/ { UNICODE_MODE, 9, -1, -1, -1, { 0, 0, "" }, "βAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(253) 263 226 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "249 chars EC2" },
        /* 19*/ { UNICODE_MODE, 9, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "A", 0, "(2) 263 65", "Note previously ECI ignored and not outputted if ULTRA_COMPRESSION and all ASCII" },
        /* 20*/ { UNICODE_MODE, 15, -1, -1, -1, { 0, 0, "" }, "Ŗ", 0, "(2) 268 170", "" },
        /* 21*/ { DATA_MODE, 898, -1, -1, -1, { 0, 0, "" }, "\001\002\003\004\377", 0, "(7) 278 130 1 2 3 4 255", "" },
        /* 22*/ { DATA_MODE, 899, -1, -1, -1, { 0, 0, "" }, "\001\002\003\004\377", 0, "(6) 280 1 2 3 4 255", "" },
        /* 23*/ { DATA_MODE, 900, -1, -1, -1, { 0, 0, "" }, "\001\002\003\004\377", 0, "(9) 257 274 137 128 1 2 3 4 255", "" },
        /* 24*/ { DATA_MODE, 9999, -1, -1, -1, { 0, 0, "" }, "\001\002\003\004\377", 0, "(9) 257 274 227 227 1 2 3 4 255", "" },
        /* 25*/ { DATA_MODE, 10000, -1, -1, -1, { 0, 0, "" }, "\001\002\003\004\377", 0, "(10) 257 275 129 128 128 1 2 3 4 255", "" },
        /* 26*/ { DATA_MODE, 811799, -1, -1, -1, { 0, 0, "" }, "\001\002\003\004\377", 0, "(10) 257 275 209 145 227 1 2 3 4 255", "" },
        /* 27*/ { DATA_MODE, 811800, -1, -1, -1, { 0, 0, "" }, "\001\002\003\004\377", ZINT_ERROR_INVALID_OPTION, "Error 590: ECI code '811800' out of range (0 to 811799)", "" },
        /* 28*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "123,456,789/12,/3,4,/5//", 0, "(15) 272 140 231 173 234 206 257 140 44 262 242 44 264 47 47", "Mode: a (24)" },
        /* 29*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS", 0, "(32) 257 256 46 151 78 210 205 208 258 5 148 28 72 2 167 52 127 193 83 75 211 267 76 65 32", "Mode: cccccc88cccccccccc8888aaa8cccccc (32)" },
        /* 30*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS", 0, "(33) 257 72 69 73 77 65 83 205 208 65 32 75 69 78 78 65 82 65 72 193 83 75 211 76 65 32 205", "" },
        /* 31*/ { UNICODE_MODE, 10, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "אולטרה-קוד1234", 0, "(14) 264 224 229 236 232 248 228 45 247 229 227 267 140 162", "Mode: 8888888888aaaa (14); Figure G.3" },
        /* 32*/ { UNICODE_MODE, 10, -1, -1, -1, { 0, 0, "" }, "אולטרה-קוד1234", 0, "(15) 264 224 229 236 232 248 228 45 247 229 227 49 50 51 52", "" },
        /* 33*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "https://aimglobal.org/jcrv3tX", 0, "(16) 282 266 1 74 41 19 6 168 270 212 59 106 144 56 265 70", "Mode: c (21); Figure G.4a" },
        /* 34*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "https://aimglobal.org/jcrv3tX", 0, "(22) 282 97 105 109 103 108 111 98 97 108 46 111 114 103 47 106 99 114 118 51 116 88", "" },
        /* 35*/ { GS1_MODE, 0, -1, -1, -1, { 0, 0, "" }, "[01]03453120000011[17]121125[10]ABCD1234", 0, "(20) 273 129 131 173 159 148 128 128 139 145 140 139 153 138 65 66 67 68 140 162", "Mode: a (34); Figure G.6 uses C43 for 6 of last 7 chars (same codeword count)" },
        /* 36*/ { GS1_MODE, 0, -1, -1, -1, { 0, 0, "" }, "[17]120508[10]ABCD1234[410]9501101020917", 0, "(21) 273 145 140 133 136 138 65 66 67 68 140 162 272 169 137 178 139 129 130 137 145", "Mode: a (35)" },
        /* 37*/ { GS1_MODE, 0, -1, -1, -1, { 0, 0, "" }, "[17]120508[10]ABCDEFGHI[410]9501101020917", 0, "(24) 273 145 140 133 136 138 65 66 67 68 69 70 71 72 73 272 169 137 178 139 129 130 137 145", "Mode: a (36)" },
        /* 38*/ { GS1_MODE | GS1PARENS_MODE, 0, -1, -1, -1, { 0, 0, "" }, "(17)120508(10)ABCDEFGHI(410)9501101020917", 0, "(24) 273 145 140 133 136 138 65 66 67 68 69 70 71 72 73 272 169 137 178 139 129 130 137 145", "Mode: a (36)" },
        /* 39*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "ftp://", 0, "(4) 272 278 269 165", "Mode: c (6)" },
        /* 40*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, ".cgi", 0, "(4) 272 278 274 131", "Mode: c (4)" },
        /* 41*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "ftp://a.cgi", 0, "(6) 272 280 269 123 274 131", "Mode: c (11)" },
        /* 42*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "e: file:f.shtml !", 0, "(12) 272 280 30 94 236 235 72 233 39 52 267 250", "Mode: c (17)" },
        /* 43*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "Aaatel:", 0, "(6) 272 280 262 76 6 89", "Mode: c (7)" },
        /* 44*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "Aatel:a", 0, "(6) 272 280 262 76 271 161", "Mode: c (7)" },
        /* 45*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "Atel:aAa", 0, "(8) 272 275 6 89 275 148 0 42", "Mode: c (8)" },
        /* 46*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "tel:AAaa", 0, "(8) 272 275 271 161 6 28 262 118", "Mode: c (8)" },
        /* 47*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "AAaatel:aA", 0, "(10) 272 276 0 42 0 41 118 46 6 156", "Mode: c (10)" },
        /* 48*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "émailto:étel:éfile:éhttp://éhttps://éftp://", 0, "(18) 257 233 276 282 233 277 282 233 278 282 233 279 282 233 280 282 233 281", "Mode: 8ccccccc8cccc8ccccc8ccccccc8cccccccc8cccccc (43)" },
        /* 49*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "éhttp://www.url.com", 0, "(9) 257 233 279 269 186 113 81 45 252", "Mode: 8cccccccccccccccccc (19)" },
        /* 50*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "éhttps://www.url.com", 0, "(9) 257 233 280 269 186 113 81 45 252", "Mode: 8ccccccccccccccccccc (20)" },
        /* 51*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "http://url.com", 0, "(8) 281 117 114 108 46 99 111 109", "Mode: 8888888 (7)" },
        /* 52*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "https://url.com", 0, "(8) 282 117 114 108 46 99 111 109", "Mode: 8888888 (7)" },
        /* 53*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "http://url.com", 0, "(6) 281 262 133 216 269 251", "Mode: ccccccc (7)" },
        /* 54*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "https://url.com", 0, "(6) 282 262 133 216 269 251", "Mode: ccccccc (7)" },
        /* 55*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "{", 0, "(2) 272 123", "Mode: a (1)" },
        /* 56*/ { UNICODE_MODE, 0, -1, -1, -1, { 2, 3, "" }, "A", 0, "(2) 257 65", "" },
        /* 57*/ { UNICODE_MODE, 0, -1, -1, -1, { 1, 1, "" }, "A", ZINT_ERROR_INVALID_OPTION, "Error 596: Structured Append count '1' out of range (2 to 8)", "" },
        /* 58*/ { UNICODE_MODE, 0, -1, -1, -1, { 1, 9, "" }, "A", ZINT_ERROR_INVALID_OPTION, "Error 596: Structured Append count '9' out of range (2 to 8)", "" },
        /* 59*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 3, "" }, "A", ZINT_ERROR_INVALID_OPTION, "Error 597: Structured Append index '0' out of range (1 to count 3)", "" },
        /* 60*/ { UNICODE_MODE, 0, -1, -1, -1, { 4, 3, "" }, "A", ZINT_ERROR_INVALID_OPTION, "Error 597: Structured Append index '4' out of range (1 to count 3)", "" },
        /* 61*/ { UNICODE_MODE, 0, -1, -1, -1, { 8, 8, "0" }, "A", 0, "(2) 257 65", "" },
        /* 62*/ { UNICODE_MODE, 0, -1, -1, -1, { 8, 8, "80088" }, "A", 0, "(2) 257 65", "" },
        /* 63*/ { UNICODE_MODE, 0, -1, -1, -1, { 8, 8, "123456" }, "A", ZINT_ERROR_INVALID_OPTION, "Error 593: Structured Append ID length 6 too long (5 digit maximum)", "" },
        /* 64*/ { UNICODE_MODE, 0, -1, -1, -1, { 8, 8, "A" }, "A", ZINT_ERROR_INVALID_OPTION, "Error 594: Invalid Structured Append ID (digits only)", "" },
        /* 65*/ { UNICODE_MODE, 0, -1, -1, -1, { 8, 8, "80089" }, "A", ZINT_ERROR_INVALID_OPTION, "Error 595: Structured Append ID value '80089' out of range (1 to 80088)", "" },
        /* 66*/ { UNICODE_MODE, 0, -1, 3, -1, { 0, 0, "" }, "A", ZINT_ERROR_INVALID_OPTION, "Error 592: Revision '3' out of range (1 or 2 only)", "" },
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

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_ULTRA, data[i].input_mode, data[i].eci,
                                    data[i].option_1, data[i].option_2, data[i].option_3,
                                    -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, %s, { %d, %d, \"%s\" }, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, data[i].option_2,
                    testUtilOption3Name(BARCODE_ULTRA, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    /* Based on AIMD/TSC15032-43 (v 0.99c), with values updated from BWIPP update 2021-07-14
       https://github.com/bwipp/postscriptbarcode/commit/4255810845fa8d45c6192dd30aee1fdad1aaf0cc
    */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "ULTRACODE_123456789!", 0, 13, 22, 1, "AIMD/TSC15032-43 Figure G.1 **NOT SAME** different compression",
                    "7777777777777777777777"
                    "7857865353533131551857"
                    "7767853515611616136717"
                    "7837836661565555363857"
                    "7717855333616336135717"
                    "7837836515535515366857"
                    "7787878787878787878787"
                    "7867816561133113551817"
                    "7737835155311665165737"
                    "7867866561155551653857"
                    "7737833315616663515717"
                    "7817851653331136333857"
                    "7777777777777777777777"
                },
        /*  1*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "ULTRACODE_123456789!", 0, 13, 24, 1, "AIMD/TSC15032-43 Figure G.1 **NOT SAME** no compression",
                    "777777777777777777777777"
                    "785786533153313111181157"
                    "776783361661161666676617"
                    "783786115156555511383357"
                    "776785556561633656175517"
                    "783781311653551535581657"
                    "778787878787878787878787"
                    "786781656113311311181117"
                    "775783333531166566676537"
                    "781786651315555113383357"
                    "776785515161666351175517"
                    "781786166533113663683357"
                    "777777777777777777777777"
                },
        /*  2*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS", 0, 19, 23, 1, "AIMD/TSC15032-43 Figure G.2 **NOT SAME** different compression",
                    "77777777777777777777777"
                    "78878663151561555158557"
                    "77878315565635366667617"
                    "78878666656561115538357"
                    "77578535365656556367117"
                    "78378153656135163558357"
                    "77178787878787878787877"
                    "78678156315513136168357"
                    "77378533531631615537117"
                    "78878361155313351368657"
                    "77678515613665166537117"
                    "78178651131551335158357"
                    "77678787878787878787877"
                    "78378535163551333638617"
                    "77178111531613611567137"
                    "78878566665531335618357"
                    "77878151331365561537137"
                    "78878333656153153368617"
                    "77777777777777777777777"
                },
        /*  3*/ { DATA_MODE, 0, -1, -1, -1, { 0, 0, "" }, "\110\105\111\115\101\123\315\320\101\040\113\105\116\116\101\122\101\110\301\123\113\323\114\101\040\315\123\114\101\116\104\123", 0, 19, 23, 1, "AIMD/TSC15032-43 Figure G.2 **NOT SAME** no compression",
                    "77777777777777777777777"
                    "78878633151153313358137"
                    "77878315666661161167617"
                    "78878663155553555538557"
                    "77578531366336136167367"
                    "78378155555515653358537"
                    "77178787878787878787877"
                    "78678135513311133138357"
                    "77378513331166611617117"
                    "78878351153555533558557"
                    "77678613615636356367117"
                    "78178156336355515538657"
                    "77678787878787878787877"
                    "78378615133513355138117"
                    "77178136511651166517637"
                    "78878365635335515358557"
                    "77878613551651656517637"
                    "78878361115516163138317"
                    "77777777777777777777777"
                },
        /*  4*/ { UNICODE_MODE, 10, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "אולטרה-קוד1234", 0, 13, 19, 1, "AIMD/TSC15032-43 Figure G.3 Same except DCC correct whereas DCC in Figure G.3 is incorrent",
                    "7777777777777777777"
                    "7857865565566616657"
                    "7737853333613351517"
                    "7867815155551565167"
                    "7757853333633356657"
                    "7837866561515535537"
                    "7787878787878787877"
                    "7867813561166666517"
                    "7737831653311131137"
                    "7817865336156555357"
                    "7767816565663636117"
                    "7817851316355311357"
                    "7777777777777777777"
                },
        /*  5*/ { DATA_MODE, 0, -1, -1, -1, { 0, 0, "" }, "\340\345\354\350\370\344\055\367\345\343\061\062\063\064", 0, 13, 20, 1, "AIMD/TSC15032-43 Figure G.3 **NOT SAME** no compression",
                    "77777777777777777777"
                    "78578611115666161157"
                    "77678333656133516617"
                    "78178655165515651157"
                    "77578516516333565617"
                    "78378163335155353557"
                    "77878787878787878787"
                    "78678153311666661117"
                    "77378315553111316637"
                    "78578631611565551357"
                    "77378166136636365117"
                    "78178613653553116357"
                    "77777777777777777777"
                },
        /*  6*/ { UNICODE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "https://aimglobal.org/jcrv3tX", 0, 13, 20, 1, "AIMD/TSC15032-43 Figure G.4a **NOT SAME** different compression; also DCC incorrect in figure",
                    "77777777777777777777"
                    "78578655115631563137"
                    "77678563356513315617"
                    "78178611665136133337"
                    "77578565116663516517"
                    "78378311355315331357"
                    "77878787878787878787"
                    "78678113111111615617"
                    "77378331553353561537"
                    "78578655311165333157"
                    "77378311136331165617"
                    "78178163363613633157"
                    "77777777777777777777"
                },
        /*  7*/ { GS1_MODE, 0, -1, -1, -1, { 0, 0, "" }, "[01]03453120000011[17]121125[10]ABCD1234", 0, 13, 23, 1, "AIMD/TSC15032-43 Figure G.6 **NOT SAME** different compression and ECC; also DCC incorrect in figure",
                    "77777777777777777777777"
                    "78578616535355353318157"
                    "77678553116631616667617"
                    "78378331365353335558567"
                    "77578563516616556637657"
                    "78378656335135665368337"
                    "77878787878787878787877"
                    "78678161311353355118517"
                    "77578313153616611667137"
                    "78178635531563535558357"
                    "77378516316135616367117"
                    "78178335533356531518357"
                    "77777777777777777777777"
                },
        /*  8*/ { UNICODE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "A", 0, 13, 13, 1, "",
                    "7777777777777"
                    "7857863335517"
                    "7717835163667"
                    "7867861551557"
                    "7757856115637"
                    "7837813636557"
                    "7787878787877"
                    "7867811361117"
                    "7717833133337"
                    "7857811355157"
                    "7737865163617"
                    "7817833536357"
                    "7777777777777"
                },
        /*  9*/ { UNICODE_MODE, 0, 2, -1, -1, { 0, 0, "" }, "1234567890123456789012", 0, 13, 24, 1, "Length 22 == 25 MCC (C) with EC1 so 6 ECC by Table 12",
                    "777777777777777777777777"
                    "785786663111111111181117"
                    "776783555536666666676667"
                    "783786113311333113383117"
                    "776785365155115351175357"
                    "783781136666363663683667"
                    "778787878787878787878787"
                    "786781313511111111181117"
                    "775785135666666666676637"
                    "781781666511333113383157"
                    "776783531656155561575517"
                    "781786155535516355186337"
                    "777777777777777777777777"
                },
        /* 10*/ { UNICODE_MODE, 0, 2, -1, -1, { 0, 0, "" }, "12345678901234567890123", 0, 13, 25, 1, "Length 23 == 26 MCC (C) with EC1 so 7 ECC by Table 12",
                    "7777777777777777777777777"
                    "7857863655511111111811117"
                    "7767831563666666666766667"
                    "7857863315511333113833117"
                    "7717855133656155561755567"
                    "7837811366535516355816357"
                    "7787878787878787878787877"
                    "7867813331111111111811117"
                    "7757851515366666666766637"
                    "7837816363113331133831157"
                    "7757851535651153511753517"
                    "7817835653363636636836657"
                    "7777777777777777777777777"
                },
        /* 11*/ { UNICODE_MODE, 0, 1, -1, -1, { 0, 0, "" }, "1", 0, 13, 11, 1, "Figure 3a min 2-row, EC0",
                    "77777777777"
                    "78578661517"
                    "77178355667"
                    "78378666517"
                    "77678551657"
                    "78378135537"
                    "77878787877"
                    "78678151117"
                    "77178333337"
                    "78378115117"
                    "77578631357"
                    "78178365567"
                    "77777777777"
                },
        /* 12*/ { UNICODE_MODE, 0, 6, -1, -1, { 0, 0, "" }, "1234567890123456789012", 0, 13, 28, 0, "Figure 3a max 2-row, EC5 **NOT SAME** extra col due to BWIPP update 2021-07-14; BWIPP chooses 3 rows instead",
                    "7777777777777777777777777777"
                    "7857863331131511111811111157"
                    "7717835613316666666766666617"
                    "7837866555153511333811333157"
                    "7757853333361656155756155517"
                    "7867816166656535516835516357"
                    "7787878787878787878787878787"
                    "7837816551551111111811111117"
                    "7717855165135366666766666637"
                    "7837813613616513331813331157"
                    "7767836165151351153751153567"
                    "7817863633563563636863636637"
                    "7777777777777777777777777777"
                },
        /* 13*/ { UNICODE_MODE, 0, 1, -1, -1, { 0, 0, "" }, "12345678901234567890123456789012345", 0, 19, 22, 1, "Figure 3b min 3-row, EC0 **NOT SAME** Zint min not same as real min as chooses lower rows first (would need row option)",
                    "7777777777777777777777"
                    "7887866511111111111817"
                    "7787833666666666666767"
                    "7887861513313311331837"
                    "7757855651551536155717"
                    "7837811565333165666857"
                    "7717878787878787878787"
                    "7857811111111111111857"
                    "7737855366666666666717"
                    "7887863113313313311857"
                    "7767816361551551536717"
                    "7817831556665333165857"
                    "7757878787878787878787"
                    "7867811111111111111817"
                    "7717855666666666666737"
                    "7887816133113313313817"
                    "7787863515361551551757"
                    "7887831331656665333867"
                    "7777777777777777777777"
                },
        /* 14*/ { UNICODE_MODE, 0, 6, -1, -1, { 0, 0, "" }, "1234567890123456789012345678901234567890123456789012345", 0, 19, 38, 0, "Figure 3b max 3-row, EC5 **NOT SAME** extra col due to BWIPP update 2021-07-14; BWIPP chooses 4 rows instead",
                    "77777777777777777777777777777777777777"
                    "78878611311563611118111111111111111817"
                    "77878366156351555667666666666666666767"
                    "78878633333136131138313313311331331837"
                    "77578551555353555617551551536155155717"
                    "78178136611516613568665333165666533837"
                    "77678787878787878787878787878787878787"
                    "78178111165636331118111111111111111817"
                    "77678663533553616667666666666666666767"
                    "78878336161116361338113313313311331837"
                    "77378611355661535157361551551536155717"
                    "78178166536313613318656665333165666857"
                    "77578787878787878787878787878787878787"
                    "78678313563533551118111111111111111817"
                    "77178535616651666667666666666666666757"
                    "78878153331316151338133113313313311837"
                    "77878611116665665157515361551551536717"
                    "78878166553313356538331656665333165837"
                    "77777777777777777777777777777777777777"
                },
        /* 15*/ { UNICODE_MODE, 0, 1, -1, -1, { 0, 0, "" }, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012", 0, 25, 30, 1, "Figure 3c min 4-row, EC0 **NOT SAME** Zint min not same as real min as chooses lower rows first (would need row option)",
                    "777777777777777777777777777777"
                    "788786511111111111181111111117"
                    "778783166666666666676666666667"
                    "788786513313133131383131331317"
                    "778785351565515655175655156557"
                    "788781135651356513586513565137"
                    "778787878787878787878787878787"
                    "785783311111111111181111111117"
                    "771781166666666666676666666667"
                    "786786613133131331381331313317"
                    "775781351315513155173155131557"
                    "783783566633666336686336663367"
                    "778787878787878787878787878787"
                    "786783511111111111181111111157"
                    "771786666666666666676666666617"
                    "785785513133131331381331313357"
                    "773781665515655156575156551517"
                    "781785551356513565183565135657"
                    "778787878787878787878787878787"
                    "788781111111111111181111111117"
                    "778785366666666666676666666637"
                    "788781133131331313381313313117"
                    "778786315513155131575131551357"
                    "788785533666336663386663366667"
                    "777777777777777777777777777777"
                },
        /* 16*/ { UNICODE_MODE, 0, 6, -1, -1, { 0, 0, "" }, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456", 0, 25, 50, 0, "Figure 3c max 4-row **NOT SAME** extra col due to BWIPP update 2021-07-14; BWIPP chooses 5 rows instead",
                    "77777777777777777777777777777777777777777777777777"
                    "78878631533313135518111111111111111811111111111117"
                    "77878315116161313667666666666666666766666666666667"
                    "78878656365333166518313313133131331831331313313137"
                    "77878561133666533667551565515655156755156551565517"
                    "78878156661531351558135651356513565813565135651357"
                    "77878787878787878787878787878787878787878787878787"
                    "78578313331353336118111111111111111811111111111117"
                    "77378661116566653567666666666666666766666666666667"
                    "78578136563115335638313133131331313831313313133137"
                    "77378311656551166317551315513155131755131551315517"
                    "78678653535336613538366633666336663836663366633667"
                    "77878787878787878787878787878787878787878787878787"
                    "78378336656556111118111111111111111811111111111157"
                    "77578153161313353667666666666666666766666666666617"
                    "78178566535655535138313133131331313831313313133157"
                    "77378635316136611517565515655156551756551565515617"
                    "78178551133613153358651356513565135865135651356557"
                    "77878787878787878787878787878787878787878787878787"
                    "78878655635551355118111111111111111811111111111117"
                    "77878313113333563667666666666666666766666666666657"
                    "78878161551515631138133131331313313813313133131367"
                    "77878316165363313517315513155131551731551315513157"
                    "78878633351651561668633666336663366863366633666337"
                    "77777777777777777777777777777777777777777777777777"
                },
        /* 17*/ { UNICODE_MODE, 0, 1, -1, -1, { 0, 0, "" }, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 0, 31, 42, 1, "Figure 3d min 5-row, EC0 **NOT SAME** Zint min not same as real min as chooses lower rows first (would need row option)",
                    "777777777777777777777777777777777777777777"
                    "788786511111111111181111111111111118111117"
                    "778783366666666666676666666666666667666667"
                    "788786113131313131381313131313131318313137"
                    "778785365656565656576565656565656567565657"
                    "788781553535353535385353535353535358353537"
                    "778787878787878787878787878787878787878787"
                    "788785511111111111181111111111111118111117"
                    "778781666666666666676666666666666667666667"
                    "788786533333333333383333333333333338333337"
                    "775785615151515151571515151515151517515157"
                    "783783536363636363683636363636363638636367"
                    "776787878787878787878787878787878787878787"
                    "781781111111111111181111111111111118111157"
                    "773785366666666666676666666666666667666617"
                    "788783131313131313183131313131313138131357"
                    "776786313131313131371313131313131317313117"
                    "783785556565656565685656565656565658656557"
                    "771787878787878787878787878787878787878787"
                    "785781111111111111181111111111111118111157"
                    "771783666666666666676666666666666667666617"
                    "788785131313131313183131313131313138131357"
                    "778781515151515151571515151515151517515117"
                    "788786363636363636386363636363636368363657"
                    "778787878787878787878787878787878787878787"
                    "788781111111111111181111111111111118111117"
                    "778786666666666666676666666666666667666637"
                    "788783131313131313183131313131313138131317"
                    "778785555555555555575555555555555557555557"
                    "788783616161616161681616161616161618616167"
                    "777777777777777777777777777777777777777777"
                },
        /* 18*/ { UNICODE_MODE, 0, 6, -1, -1, { 0, 0, "" }, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012", 0, 31, 66, 1, "Figure 3d max 5-row, EC5 **NOT SAME** Max columns due to 282 limit is 60 not 61 as shown",
                    "777777777777777777777777777777777777777777777777777777777777777777"
                    "788786563656553165385551111111111118111111111111111811111111111117"
                    "778783136511335313673366666666666667666666666666666766666666666667"
                    "788786315633661531381153333333333338333333333333333833333333333337"
                    "778785666366116365673661515151515157151515151515151751515151515157"
                    "788781333151633111586553636363636368363636363636363863636363636367"
                    "778787878787878787878787878787878787878787878787878787878787878787"
                    "788785511663631513386131111111111118111111111111111811111111111117"
                    "778786665351353656571616666666666667666666666666666766666666666667"
                    "788783551116166363183533131313131318313131313131313813131313131317"
                    "775786313333613536571361313131313137131313131313131731313131313137"
                    "786785666615135615686615656565656568565656565656565865656565656567"
                    "775787878787878787878787878787878787878787878787878787878787878787"
                    "783785665636551563586511111111111118111111111111111811111111111117"
                    "776781516115365616671166666666666667666666666666666766666666666667"
                    "788785631666611363586313131313131318313131313131313813131313131317"
                    "773786116553355536673151515151515157151515151515151751515151515157"
                    "786783553131613115381536363636363638636363636363636836363636363637"
                    "775787878787878787878787878787878787878787878787878787878787878787"
                    "783786611351531316585111111111111118111111111111111811111111111117"
                    "771783536663313553371666666666666667666666666666666766666666666667"
                    "788785665115635111683313131313131318313131313131313813131313131317"
                    "778783133333563566376155555555555557555555555555555755555555555557"
                    "788785356111611131583661616161616168161616161616161861616161616167"
                    "778787878787878787878787878787878787878787878787878787878787878787"
                    "788785665536116356681611111111111118111111111111111811111111111137"
                    "778786511361333635576566666666666667666666666666666766666666666617"
                    "788783666116561361181613131313131318313131313131313813131313131357"
                    "778785533633353533375565656565656567565656565656565765656565656517"
                    "788786316551515665186353535353535358353535353535353853535353535357"
                    "777777777777777777777777777777777777777777777777777777777777777777"
                },
        /* 19*/ { UNICODE_MODE | ESCAPE_MODE, 0, -1, -1, -1, { 0, 0, "" }, "[)>\\R06\\G17V12345\\G1P234TYU\\GS6789\\R\\E", 0, 13, 27, 0, "06 Macro; not supported by BWIPP",
                    "777777777777777777777777777"
                    "785786311655611111181311157"
                    "771783153516566666676156617"
                    "783786565165331131183633357"
                    "771785613316555615571311517"
                    "786781336155113553683636357"
                    "778787878787878787878787877"
                    "783781136511131113183331117"
                    "771785651653616651671116637"
                    "783781163535161335185653357"
                    "775786355661515113676165537"
                    "781783531133356335585331617"
                    "777777777777777777777777777"
                },
        /* 20*/ { UNICODE_MODE | ESCAPE_MODE, 0, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, "[)>\\R06\\G17V12345\\G1P234TYU\\GS6789\\R\\E", 0, 13, 23, 0, "06 Macro; not supported by BWIPP",
                    "77777777777777777777777"
                    "78578613335635131318557"
                    "77678536566511516157617"
                    "78378311615366353638157"
                    "77578533166515131317617"
                    "78378356655653353638357"
                    "77878787878787878787877"
                    "78678151311551153338617"
                    "77578333653116611117137"
                    "78178611511333155658357"
                    "77378555366511536167517"
                    "78178116153635315338657"
                    "77777777777777777777777"
                },
        /* 21*/ { UNICODE_MODE, 0, -1, -1, -1, { 1, 2, "" }, "A", 0, 13, 14, 1, "Structured Append without File Number",
                    "77777777777777"
                    "78578633165557"
                    "77378351336117"
                    "78178666115357"
                    "77578533636617"
                    "78378156565557"
                    "77878787878787"
                    "78678153513117"
                    "77178365631637"
                    "78578133353557"
                    "77678651566317"
                    "78178535111557"
                    "77777777777777"
                },
        /* 22*/ { UNICODE_MODE, 0, -1, -1, -1, { 8, 8, "283" }, "A", 0, 13, 15, 1, "Structured Append with File Number",
                    "777777777777777"
                    "785786353356157"
                    "773783115665317"
                    "781786661553157"
                    "776785535165317"
                    "783781613656557"
                    "778787878787877"
                    "786781563131117"
                    "771783651313637"
                    "786785336531557"
                    "773781115663317"
                    "781785653516557"
                    "777777777777777"
                },
        /* 23*/ { UNICODE_MODE, 0, -1, 2, -1, { 0, 0, "" }, "ULTRACODE_123456789!", 0, 13, 24, 1, "Revision 2",
                    "777777777777777777777777"
                    "781786533153313111181157"
                    "776783361661161666676617"
                    "781786115156555511383357"
                    "775785556561633656175517"
                    "786781311653551535581657"
                    "778787878787878787878787"
                    "783781656113311311181117"
                    "776783333531166566676537"
                    "783786651315555113383357"
                    "776785515161666351175517"
                    "785786166533113663683357"
                    "777777777777777777777777"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol("test_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_ULTRA, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, %s, { %d, %d, \"%s\" }, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, data[i].option_2,
                    testUtilOption3Name(BARCODE_ULTRA, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
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
        int option_1;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    /* Based on AIMD/TSC15032-43 (v 0.99c), with values updated from BWIPP update 2021-07-14
       https://github.com/bwipp/postscriptbarcode/commit/4255810845fa8d45c6192dd30aee1fdad1aaf0cc
    */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 13, 15, 0, "Standard example; BWIPP no ECI support for Ultracode",
                    "777777777777777"
                    "785786131155157"
                    "773783613563337"
                    "781786355656167"
                    "776785561363337"
                    "783781355651517"
                    "778787878787877"
                    "786781665116117"
                    "771783136335337"
                    "786785653613557"
                    "773781536165117"
                    "781786115333557"
                    "777777777777777"
                },
        /*  1*/ { UNICODE_MODE, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 13, 15, 0, "Standard example auto-ECI; BWIPP no ECI support for Ultracode",
                    "777777777777777"
                    "785786131155157"
                    "773783613563337"
                    "781786355656167"
                    "776785561363337"
                    "783781355651517"
                    "778787878787877"
                    "786781665116117"
                    "771783136335337"
                    "786785653613557"
                    "773781536165117"
                    "781786115333557"
                    "777777777777777"
                },
        /*  2*/ { UNICODE_MODE, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 13, 15, 0, "Standard example inverted; BWIPP no ECI support for Ultracode",
                    "777777777777777"
                    "785786351355157"
                    "773785113663337"
                    "781781365356167"
                    "776783136163337"
                    "783786555651517"
                    "778787878787877"
                    "786781356116117"
                    "771783663335337"
                    "786785531613157"
                    "773781353165517"
                    "781786565333657"
                    "777777777777777"
                },
        /*  3*/ { UNICODE_MODE, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 13, 15, 0, "Standard example inverted auto-ECI; BWIPP no ECI support for Ultracode",
                    "777777777777777"
                    "785786351355157"
                    "773785113663337"
                    "781781365356167"
                    "776783136163337"
                    "783786555651517"
                    "778787878787877"
                    "786781356116117"
                    "771783663335337"
                    "786785531613157"
                    "773781353165517"
                    "781786565333657"
                    "777777777777777"
                },
        /*  4*/ { UNICODE_MODE, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, { { TU("product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a -128 GB的存储空间-黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a - 128 GB Speicher - Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 31, 51, 0, "AIM ITS/04-023:2022 Annex A example; BWIPP no ECI support for Ultracode",
                    "777777777777777777777777777777777777777777777777777"
                    "788786553615151313686615635315656568556155565633167"
                    "778783131336563655375536556151531317365661116315357"
                    "788786366163615166181363163533665138151555551131667"
                    "778785631636351311375511615351516667316663165656357"
                    "788781563115163655581635531513353338163331636363137"
                    "778787878787878787878787878787878787878787878787877"
                    "788785515563115631383565335311551158666513631316557"
                    "778786651356351315575153153566315667555156366135637"
                    "788781363513515166181666316333666318161633113513317"
                    "775783635365361331576153163515155637655155661351537"
                    "781785353136156566383335656633633318536616353136617"
                    "773787878787878787878787878787878787878787878787877"
                    "781781331535616631386565316331555568565666316651557"
                    "776786666356535565175656551655663617153515131366317"
                    "788783355511663311383331636113116138635136555113557"
                    "773785516665311566576556353555655567351513361555317"
                    "781783633336533155181333115131513638536365113131157"
                    "773787878787878787878787878787878787878787878787877"
                    "785786636561663636181316366313566618513136611663557"
                    "771781563655535363573133555551311167331655355551317"
                    "788783155566111151385315661133633538516533131135557"
                    "778786516633355666676136356651116667635356516556317"
                    "788785133151663353583553135516551118156615665661657"
                    "778787878787878787878787878787878787878787878787877"
                    "788781353515665533386611351311655558113366366156117"
                    "778783616631513151171356535556166617531115155365537"
                    "788785555563336315386533363665613368666653363516367"
                    "778781366656651133171316155153135557515516515663557"
                    "788786635535536351685531611336513668663633636555117"
                    "777777777777777777777777777777777777777777777777777"
                },
        /*  5*/ { DATA_MODE, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, { { TU("\266"), 1, 0 }, { TU("\266"), 1, 7 }, { TU("\266"), 1, 0 } }, 0, 13, 17, 0, "Standard example + extra seg, data mode; BWIPP no ECI support for Ultracode",
                    "77777777777777777"
                    "78578616315515157"
                    "77378361566333337"
                    "78578633115616167"
                    "77678566656333337"
                    "78378131365151517"
                    "77878787878787877"
                    "78678136111616117"
                    "77378361633535337"
                    "78178513161353157"
                    "77378656536515517"
                    "78178135353353657"
                    "77777777777777777"
                },
        /*  6*/ { UNICODE_MODE, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, { { TU("¿é"), -1, 0 }, { TU("กขฯ"), -1, 0 }, { TU("φχψω"), -1, 0 } }, ZINT_WARN_USES_ECI, 13, 20, 0, "Auto-ECI; BWIPP no ECI support for Ultracode",
                    "77777777777777777777"
                    "78578651555561561667"
                    "77678333166653153337"
                    "78178655335135635557"
                    "77578563166556553337"
                    "78378131655133331167"
                    "77878787878787878787"
                    "78678166111615516617"
                    "77378351653131633337"
                    "78578663311616515557"
                    "77378315636535131317"
                    "78178131363151656557"
                    "77777777777777777777"
                },
        /*  7*/ { UNICODE_MODE, -1, -1, ULTRA_COMPRESSION, { 2, 3, "4" }, { { TU("¿é"), -1, 3 }, { TU("กขฯ"), -1, 13 }, { TU("φχψω"), -1, 9 } }, 0, 13, 23, 0, "Structured Append; BWIPP no ECI support for Ultracode",
                    "77777777777777777777777"
                    "78578613615616155168657"
                    "77678335366131316337317"
                    "78378666635516165158557"
                    "77578515156665351317317"
                    "78378163515131516568557"
                    "77878787878787878787877"
                    "78678151513156156168617"
                    "77578366351365315337337"
                    "78178633513113563558557"
                    "77378516666355655337317"
                    "78178133151513333118657"
                    "77777777777777777777777"
                },
        /*  8*/ { UNICODE_MODE, -1, -1, ULTRA_COMPRESSION, { 0, 0, "" }, { { TU("çèéêëì"), -1, 0 }, { TU("òóô"), -1, 899 }, { TU("òóô"), -1, 10000 } }, 0, 13, 27, 0, "ECIs >= 899; BWIPP no ECI support for Ultracode",
                    "777777777777777777777777777"
                    "785786353555666665585335557"
                    "771783161616113513373663337"
                    "783786335335661355686335667"
                    "771785511666353666171656117"
                    "786781655535111113385163357"
                    "778787878787878787878787877"
                    "783781151511666355586355517"
                    "771785616353113666675666637"
                    "783781363635661511183311157"
                    "775786615561353366676566637"
                    "781785551653535633383633317"
                    "777777777777777777777777777"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol("test_encode_segs", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, BARCODE_ULTRA, data[i].input_mode, -1 /*eci*/,
                    data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, NULL, 0, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            char escaped1[4096];
            char escaped2[4096];
            int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
            int length1 = data[i].segs[1].length == -1 ? (int) ustrlen(data[i].segs[1].source) : data[i].segs[1].length;
            int length2 = data[i].segs[2].length == -1 ? (int) ustrlen(data[i].segs[2].source) : data[i].segs[2].length;
            printf("        /*%3d*/ { %s, %d, %d, %s, { %d, %d, \"%s\" }, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    testUtilOption3Name(BARCODE_ULTRA, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwippSegs(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].segs, seg_count, NULL, bwipp_buf, sizeof(bwipp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwippSegs ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                    }
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
        { "test_reader_init", test_reader_init },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_encode_segs", test_encode_segs },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
