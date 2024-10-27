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
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_AUSPOST, "1", 23, 0, 3, 133 },
        /*  1*/ { BARCODE_AUSPOST, "1", 24, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_AUSPOST, "1", 18, 0, 3, 133 },
        /*  3*/ { BARCODE_AUSPOST, "1", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_AUSPOST, "1", 16, 0, 3, 103 },
        /*  5*/ { BARCODE_AUSPOST, "1", 17, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_AUSPOST, "1", 13, 0, 3, 103 },
        /*  7*/ { BARCODE_AUSPOST, "1", 14, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_AUSPOST, "1", 8, 0, 3, 73 },
        /*  9*/ { BARCODE_AUSPOST, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { BARCODE_AUSREPLY, "1", 8, 0, 3, 73 },
        /* 11*/ { BARCODE_AUSREPLY, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 12*/ { BARCODE_AUSROUTE, "1", 8, 0, 3, 73 },
        /* 13*/ { BARCODE_AUSROUTE, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { BARCODE_AUSREDIRECT, "1", 8, 0, 3, 73 },
        /* 15*/ { BARCODE_AUSREDIRECT, "1", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[64];

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

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_AUSPOST, "12345678901234567890123", "" }, /* None */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_hrt", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

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
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_AUSPOST, "12345678", 0, 3, 73, "" },
        /*  1*/ { BARCODE_AUSPOST, "1234567A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 405: Invalid character at position 8 in DPID (first 8 characters) (digits only)" },
        /*  2*/ { BARCODE_AUSPOST, "12345678ABcd#", 0, 3, 103, "" },
        /*  3*/ { BARCODE_AUSPOST, "12345678ABcd!", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 404: Invalid character at position 13 in input (alphanumerics, space and \"#\" only)" },
        /*  4*/ { BARCODE_AUSPOST, "12345678ABcd#", 0, 3, 103, "" },
        /*  5*/ { BARCODE_AUSPOST, "1234567890123456", 0, 3, 103, "" },
        /*  6*/ { BARCODE_AUSPOST, "123456789012345A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 402: Invalid character at position 16 in input (digits only for FCC 59 length 16)" },
        /*  7*/ { BARCODE_AUSPOST, "12345678ABCDefgh #", 0, 3, 133, "" }, /* Length 18 */
        /*  8*/ { BARCODE_AUSPOST, "12345678901234567890123", 0, 3, 133, "" },
        /*  9*/ { BARCODE_AUSPOST, "1234567890123456789012A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 406: Invalid character at position 23 in input (digits only for FCC 62 length 23)" },
        /* 10*/ { BARCODE_AUSPOST, "1234567", ZINT_ERROR_TOO_LONG, -1, -1, "Error 401: Input length 7 wrong (8, 13, 16, 18 or 23 only)" }, /* No leading zeroes added */
        /* 11*/ { BARCODE_AUSREPLY, "12345678", 0, 3, 73, "" },
        /* 12*/ { BARCODE_AUSREPLY, "1234567", 0, 3, 73, "" }, /* Leading zeroes added */
        /* 13*/ { BARCODE_AUSREPLY, "123456789", ZINT_ERROR_TOO_LONG, -1, -1, "Error 403: Input length 9 too long (maximum 8)" },
        /* 14*/ { BARCODE_AUSROUTE, "123456", 0, 3, 73, "" },
        /* 15*/ { BARCODE_AUSROUTE, "12345", 0, 3, 73, "" },
        /* 16*/ { BARCODE_AUSROUTE, "123456789", ZINT_ERROR_TOO_LONG, -1, -1, "Error 403: Input length 9 too long (maximum 8)" },
        /* 17*/ { BARCODE_AUSREDIRECT, "1234", 0, 3, 73, "" },
        /* 18*/ { BARCODE_AUSREDIRECT, "123", 0, 3, 73, "" },
        /* 19*/ { BARCODE_AUSREDIRECT, "0", 0, 3, 73, "" },
        /* 20*/ { BARCODE_AUSREDIRECT, "123456789", ZINT_ERROR_TOO_LONG, -1, -1, "Error 403: Input length 9 too long (maximum 8)" },
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
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                char modules_dump[4096];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Australia Post Customer Barcoding Technical Specifications (Revised 3 Aug 2012) "AusPost Tech Specs"
   https://auspost.com.au/content/dam/auspost_corp/media/documents/customer-barcode-technical-specifications-aug2012.pdf
   Australia Post A Guide To Printing the 4-State Barcode (Revised 16 March 2012) "AusPost Guide"
   https://auspost.com.au/content/dam/auspost_corp/media/documents/a-guide-to-printing-the-4state-barcode-v31-mar2012.pdf
*/
static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_AUSPOST, "96184209", 0, 3, 73, "AusPost Tech Specs Diagram 1; verified manually against TEC-IT",
                    "1000101010100010001010100000101010001010001000001010000010001000001000100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000100010000010101010001010000010101010001000101010001000100010000010000"
                },
        /*  1*/ { BARCODE_AUSPOST, "39549554", 0, 3, 73, "AusPost Guide Figure 3, same; verified manually against TEC-IT",
                    "1000101010101010001010001010001010001000101000001000101010001010000000100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000100010000010001000100000001000100010000000000010001000000000001010000"
                },
        /*  2*/ { BARCODE_AUSPOST, "56439111ABA 9", 0, 3, 103, "AusPost Guide Figure 4, same; verified manually against TEC-IT",
                    "1000100000101000001010101010001010101010101010101010101010101010100000000000001010100010101010000010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000001000100010101000000010001010001000100010101010100010101010100000101000000010001000101010000000000"
                },
        /*  3*/ { BARCODE_AUSPOST, "3221132412345678", 0, 3, 103, "59 Custom 2 N encoding",
                    "1000100000101010100010001010101010101000101010101000101010101000001000100000101000000000001000000000100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000001000100010101010101000100000101010000010001010001000000010101010001010000010001010101000100000000"
                },
        /*  4*/ { BARCODE_AUSPOST, "32211324Ab #2", 0, 3, 103, "59 Custom 2 C encoding",
                    "1000100000101010100010001010101010101000101010101010001010100010100000101000100000000010100000100010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000001000100010101010101000100000101010000010101010001010100010000000100000001000101010000010000000000"
                },
        /*  5*/ { BARCODE_AUSPOST, "32211324123456789012345", 0, 3, 133, "62 Custom 3 N encoding",
                    "1000001010001010100010001010101010101000101010101000101010101000001000100000001010101010100010101010100000100000100010101010100010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010100010101010101000100000101010000010001010001000000010101010001010001010101000101000100000001000001010000010001010100010000"
                },
        /*  6*/ { BARCODE_AUSPOST, "32211324aBCd#F hIz", 0, 3, 133, "62 Custom 3 C encoding",
                    "1000001010001010100010001010101010101000101010000010101010100010000010100010100010100010000010000000000000100010100010101010000000100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010100010101010101000100000101010000010100010100010101010001010000010001010100000100010101000000000101000001010100000000010000"
                },
        /*  7*/ { BARCODE_AUSPOST, "12345678DEGHJKLMNO", 0, 3, 133, "62 Custom 3 C encoding GDSET 1st part",
                    "1000001010001010100010101010100000100010000010101010101010001010001010101010101010100010101010101010100000001010000010000000000010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010101000101000100000001010101000101010001010000010101010100000101000100000101000001000000000001000001010000010001010001010000"
                },
        /*  8*/ { BARCODE_AUSPOST, "23456789PQRSTUVWXY", 0, 3, 133, "62 Custom 3 C encoding GDSET 2nd part",
                    "1000001010001000101010101000001000100000001010001010001010000000101000101000100000101000101000100000001000101000101010101000101010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010101010001000000010101010001010001000101000100000101010101010100010101010001010000010001010101000000010001000001010101000000"
                },
        /*  9*/ { BARCODE_AUSPOST, "34567890Zcefgijklm", 0, 3, 133, "62 Custom 3 C encoding GDSET 3rd part",
                    "1000001010001010101010000010001000000010101000001010001010000010100010100010001010001010000010000000100000101000100000001010001010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010100010000000101010100010100010101010100010000010000000100000000000001000000000001000000010100000101000000010101010100010000"
                },
        /* 10*/ { BARCODE_AUSPOST, "12345678lnopqrstuv", 0, 3, 133, "62 Custom 3 C encoding GDSET 4th part",
                    "1000001010001010100010101010100000100010000010000000100000000000001000001000000000000000100000100000000000001010001010101000000010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010101000101000100000001010101000101000000010000010100010001010000010001010000000100000000000100000100000001010001000100000000"
                },
        /* 11*/ { BARCODE_AUSPOST, "09876543wxy# ", 0, 3, 103, "59 Custom 2 C encoding GDSET 5th part",
                    "1000100000101010001000000010001010001010101000001000001000000010100010100000100010000000000010100010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000001000101010001010101000101000100000001000001000000000001010000010100000001010001000001000100000000"
                },
        /* 12*/ { BARCODE_AUSREPLY, "12345678", 0, 3, 73, "Verified manually against tec-it",
                    "1000101010001010100010101010100000100010000000001000001000000000100010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000000000101000101000100000001010101000101000000000100010101000101000000"
                },
        /* 13*/ { BARCODE_AUSROUTE, "34567890", 0, 3, 73, "Verified manually against tec-it",
                    "1000000000101010101010000010001000000010101000100010101010000000101000100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000101010000010000000101010100010100010101000100010101010001010001000000"
                },
        /* 14*/ { BARCODE_AUSREDIRECT, "98765432", 0, 3, 73, "Verified manually against tec-it",
                    "1000001010000010000000100010100010101010100000101010101000100010100010100"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101"
                    "0000001010100010101010001010001000000010101000000000001010101000001010000"
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

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
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

/* #181 Christian Hartlage OSS-Fuzz */
static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        char *data;
        int length;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /* 0*/ { BARCODE_AUSROUTE, "A\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 1*/ { BARCODE_AUSROUTE, "1\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 2*/ { BARCODE_AUSREPLY, "A\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 3*/ { BARCODE_AUSREPLY, "1\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 4*/ { BARCODE_AUSREDIRECT, "A\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
        /* 5*/ { BARCODE_AUSREDIRECT, "1\000\000\000", 4, ZINT_ERROR_INVALID_DATA },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_fuzz", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

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
        { "test_fuzz", test_fuzz },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
