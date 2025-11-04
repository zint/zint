/*
    libzint - the open source barcode library
    Copyright (C) 2022-2025 Robin Stuart <rstuart114@gmail.com>

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

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int output_options;
        const char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected;
        const char *expected_content;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "123456", -1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 792: Input length 6 too short (minimum 7)", "", "" },
        /*  1*/ { UNICODE_MODE, -1, "1234567890123456789", -1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 790: Input length 19 too long (maximum 18)", "", "" },
        /*  2*/ { UNICODE_MODE, -1, "1234567", -1, 0, 1, 102, "1U234567", "", "" },
        /*  3*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "1234567", -1, 0, 1, 102, "1U234567", "1U234567", "No difference" },
        /*  4*/ { UNICODE_MODE, -1, "1234567 ", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 791: Invalid character at position 8 in input (alphanumerics only, excluding \"O\")", "", "" },
        /*  5*/ { UNICODE_MODE, -1, "ABCDEFGHIJKLMNOPQR", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 791: Invalid character at position 15 in input (alphanumerics only, excluding \"O\")", "", "" },
        /*  6*/ { UNICODE_MODE, -1, "ABCDEFGHIJKLMNPQRS", -1, 0, 1, 234, "AQBCDEFGHIJKLMNPQRS", "", "" },
        /*  7*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "ABCDEFGHIJKLMNPQRS", -1, 0, 1, 234, "AQBCDEFGHIJKLMNPQRS", "AQBCDEFGHIJKLMNPQRS", "No difference" },
        /*  8*/ { UNICODE_MODE, -1, "STUVWXYZ1234567890", -1, 0, 1, 234, "SCTUVWXYZ1234567890", "", "" },
        /*  9*/ { UNICODE_MODE, -1, "abcdefghijklmnpqrs", -1, 0, 1, 234, "AQBCDEFGHIJKLMNPQRS", "", "" },
        /* 10*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "abcdefghijklmnpqrs", -1, 0, 1, 234, "AQBCDEFGHIJKLMNPQRS", "AQBCDEFGHIJKLMNPQRS", "No difference" },
        /* 11*/ { UNICODE_MODE, -1, "123456789012345678", -1, 0, 1, 234, "1223456789012345678", "", "" },
        /* 12*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "123456789012345678", -1, 0, 1, 234, "1223456789012345678", "1223456789012345678", "No difference" },
        /* 13*/ { UNICODE_MODE, -1, "MMMMMMMMMMMMMMMMMM", -1, 0, 1, 234, "MTMMMMMMMMMMMMMMMMM", "", "" },
        /* 14*/ { UNICODE_MODE, -1, "00000000", -1, 0, 1, 114, "000000000", "", "" },
        /* 15*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "00000000", -1, 0, 1, 114, "000000000", "000000000", "" },
        /* 16*/ { UNICODE_MODE, -1, "000000000", -1, 0, 1, 126, "0000000000", "", "" },
        /* 17*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "000000000", -1, 0, 1, 126, "0000000000", "0000000000", "" },
        /* 18*/ { UNICODE_MODE, -1, "000S0S000S", -1, 0, 1, 138, "0S00S0S000S", "", "" },
        /* 19*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "000S0S000S", -1, 0, 1, 138, "0S00S0S000S", "0S00S0S000S", "" },
        /* 20*/ { UNICODE_MODE, -1, "ZYXWVUTSRQP", -1, 0, 1, 150, "Z0YXWVUTSRQP", "", "" },
        /* 21*/ { UNICODE_MODE, -1, "0R9GLVHA8EZ4", -1, 0, 1, 162, "0DR9GLVHA8EZ4", "", "" },
        /* 22*/ { UNICODE_MODE, -1, "NTS1J2Q6C7DYK", -1, 0, 1, 174, "NRTS1J2Q6C7DYK", "", "" },
        /* 23*/ { UNICODE_MODE, -1, "TS1J2Q6C7DYKBU", -1, 0, 1, 186, "TWS1J2Q6C7DYKBU", "", "" },
        /* 24*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "TS1J2Q6C7DYKBU", -1, 0, 1, 186, "TWS1J2Q6C7DYKBU", "TWS1J2Q6C7DYKBU", "" },
        /* 25*/ { UNICODE_MODE, -1, "IX3FWP5M0R9GLVH", -1, 0, 1, 198, "IBX3FWP5M0R9GLVH", "", "" },
        /* 26*/ { UNICODE_MODE, -1, "R9GLVHA8EZ4NTS1J", -1, 0, 1, 210, "RY9GLVHA8EZ4NTS1J", "", "" },
        /* 27*/ { UNICODE_MODE, -1, "M5PWF3XIUBKYD7C6Q", -1, 0, 1, 222, "M35PWF3XIUBKYD7C6Q", "", "" },
        /* 28*/ { UNICODE_MODE, -1, "2J1STN4ZE8AHVLG90R", -1, 0, 1, 234, "2PJ1STN4ZE8AHVLG90R", "", "" },
        /* 29*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "2J1STN4ZE8AHVLG90R", -1, 0, 1, 234, "2PJ1STN4ZE8AHVLG90R", "2PJ1STN4ZE8AHVLG90R", "" },
        /* 30*/ { UNICODE_MODE, -1, "JJJJJJJJJJJJJJJJJJ", -1, 0, 1, 234, "J9JJJJJJJJJJJJJJJJJ", "", "" },
        /* 31*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "JJJJJJJJJJJJJJJJJJ", -1, 0, 1, 234, "J9JJJJJJJJJJJJJJJJJ", "J9JJJJJJJJJJJJJJJJJ", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;
    int expected_length, expected_content_length;

    char bwipp_buf[8192];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_BC412, data[i].input_mode, -1 /*eci*/,
                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                    data[i].data, data[i].length, debug);
        expected_length = (int) strlen(data[i].expected);
        expected_content_length = (int) strlen(data[i].expected_content);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n",
                        i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n",
                        i, symbol->width, data[i].expected_width, data[i].data);
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
                            i, symbol->content_segs[0].length, symbol->content_segs[0].source,
                            data[i].expected_content, expected_content_length);
            } else {
                assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
            }

            if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                char modules_dump[8192 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf),
                                    NULL);
                assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n",
                            i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, modules_dump);
                assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, modules_dump);
            }
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                        i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        const char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        const char *comment;
        const char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, "AQ45670", 0, 1, 102, 1, "SEMI T1-95 Figure 2, same",
                    "100101000100100100010101000100010101000101010000010101001010000101001001000101001000100101010100000101"
                },
        /*  1*/ { UNICODE_MODE, "A6BC1234", 0, 1, 114, 1, "https://www.grapecity.co.jp, same",
                    "100101000100100100100010100101001001000101000100010101000010100101010010000101010001000101010000100101010000010101"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];
    char bwipp_buf[8192];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_BC412, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
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
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf), NULL);
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
