/*
    libzint - the open source barcode library
    Copyright (C) 2019-2024 Robin Stuart <rstuart114@gmail.com>

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
/*
 * Intelligent Mail barcode Encoder Test Case Reference Set (csv file)
 * Copyright (C) 2009 U.S. Postal Service
 */

#include "testcommon.h"

#include <time.h>

#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)

#define TEST_CSV_PERF_ITERATIONS    100

#if 0
#define TEST_IMAIL_CSV_MAX 300
#endif

static void test_csv(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    char csvfile[1024];
    FILE *fd;
    char buffer[1024];
    char id[10];
    char tracking_code[50];
    char routing_code[50];
    char data[102];
    char expected_daft[70];
    char return_code[10];
    char actual_daft[70];

    int ret;
    int lc = 0;

    int j;
    clock_t start;
    clock_t total = 0;
    int test_performance = debug & ZINT_DEBUG_TEST_PERFORMANCE; /* -d 256 */
    int perf_iterations = test_performance ? TEST_CSV_PERF_ITERATIONS : 1;

    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_csv", &symbol);

    if (test_performance) {
        printf("test_csv perf iterations: %d\n", perf_iterations);
    }

    assert_nonzero(testUtilDataPath(csvfile, sizeof(csvfile),
        "/backend/tests/data/imail/usps/", "uspsIMbEncoderTestCases.csv"), "testUtilDataPath == 0\n");

    for (j = 0; j < perf_iterations; j++) {
        fd = testUtilOpen(csvfile, "r");
        assert_nonnull(fd, "testUtilOpen(%s) == NULL", csvfile);

        while (fgets(buffer, sizeof(buffer), fd) != NULL) {
            const char *b;

            lc++;

            if (testContinue(p_ctx, lc - 1)) continue;

            #ifdef TEST_IMAIL_CSV_MAX
            if (lc > TEST_IMAIL_CSV_MAX && p_ctx->index == -1) {
                break;
            }
            #endif

            id[0] = tracking_code[0] = routing_code[0] = expected_daft[0] = return_code[0] = '\0';

            b = testUtilReadCSVField(buffer, id, sizeof(id));
            assert_nonnull(b, "lc:%d id b == NULL", lc);
            assert_equal(*b, ',', "lc:%d id *b %c != ','", lc, *b);

            b = testUtilReadCSVField(++b, tracking_code, sizeof(tracking_code));
            assert_nonnull(b, "lc:%d tracking_code b == NULL", lc);
            assert_equal(*b, ',', "lc:%d tracking_code *b %c != ','", lc, *b);

            b = testUtilReadCSVField(++b, routing_code, sizeof(routing_code));
            assert_nonnull(b, "lc:%d routing_code b == NULL", lc);
            assert_equal(*b, ',', "lc:%d routing_code *b %c != ','", lc, *b);

            b = testUtilReadCSVField(++b, expected_daft, sizeof(expected_daft));
            assert_nonnull(b, "lc:%d expected_daft b == NULL", lc);
            assert_equal(*b, ',', "lc:%d expected_daft *b %c != ','", lc, *b);

            b = testUtilReadCSVField(++b, return_code, sizeof(return_code));
            assert_nonnull(b, "lc:%d return_code b == NULL", lc);
            assert_equal(*b, ',', "lc:%d return_code *b %c != ','", lc, *b);

            strcpy(data, tracking_code);
            strcat(data, "-");
            strcat(data, routing_code);

            assert_nonzero((int) strlen(data), "lc:%d strlen(data) == 0", lc);

            symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            symbol->symbology = BARCODE_USPS_IMAIL;
            symbol->debug |= debug;

            if (test_performance) {
                start = clock();
            }
            ret = ZBarcode_Encode(symbol, (unsigned char *) data, (int) strlen(data));
            if (test_performance) {
                total += clock() - start;
            }

            if (strcmp(return_code, "00") == 0) {

                assert_zero(ret, "lc:%d ZBarcode_Encode ret %d != 0\n", lc, ret);

                assert_equal(symbol->rows, 3, "rows %d != 3", symbol->rows);

                ret = testUtilDAFTConvert(symbol, actual_daft, sizeof(actual_daft));
                assert_nonzero(ret, "lc:%d testUtilDAFTConvert == 0", lc);
                assert_zero(strcmp(actual_daft, expected_daft), "lc:%d\n  actual %s\nexpected %s\n", lc, actual_daft, expected_daft);
            } else {
                assert_nonzero(ret, "lc:%d ZBarcode_Encode ret %d == 0\n", lc, ret);
            }

            ZBarcode_Delete(symbol);
        }

        assert_zero(fclose(fd), "fclose != 0\n");
    }

    if (test_performance) {
        printf("test_csv perf total: %8gms\n", TEST_PERF_TIME(total));
    }
    testFinish();
}

static void test_hrt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "53379777234994544928-51135759461", "" }, /* None */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_hrt", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_USPS_IMAIL, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

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
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "53379777234994544928-51135759461", 0, 3, 129, "" },
        /*  1*/ { "123456789012345678901234567890123", ZINT_ERROR_TOO_LONG, -1, -1, "Error 450: Input length 33 too long (maximum 32)" },
        /*  2*/ { "A", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 451: Invalid character at position 1 in input (digits and \"-\" only)" },
        /*  3*/ { "12345678901234567890", 0, 3, 129, "" }, /* Tracker only, no ZIP */
        /*  4*/ { "15345678901234567890", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 454: Barcode Identifier (second character) out of range (0 to 4)" }, /* Tracker 2nd char > 4 */
        /*  5*/ { "1234567890123456789", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 452: Invalid length for tracking code (20 characters required)" }, /* Tracker 20 chars */
        /*  6*/ { "12345678901234567890-1234", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 453: Invalid length for ZIP code (5, 9 or 11 characters required)" }, /* ZIP wrong len */
        /*  7*/ { "12345678901234567890-12345", 0, 3, 129, "" },
        /*  8*/ { "12345678901234567890-123456", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 453: Invalid length for ZIP code (5, 9 or 11 characters required)" }, /* ZIP wrong len */
        /*  9*/ { "12345678901234567890-12345678", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 453: Invalid length for ZIP code (5, 9 or 11 characters required)" }, /* ZIP wrong len */
        /* 10*/ { "12345678901234567890-123456789", 0, 3, 129, "" },
        /* 11*/ { "12345678901234567890-1234567890", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 453: Invalid length for ZIP code (5, 9 or 11 characters required)" }, /* ZIP wrong len */
        /* 12*/ { "12345678901234567890-12345678901", 0, 3, 129, "" },
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

        length = testUtilSetSymbol(symbol, BARCODE_USPS_IMAIL, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
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

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { "01234567094987654321-01234567891", 0, 3, 129, "USPS-B-3200 Rev. H (2015) Figure 5",
                    "101000001010001000001000001010001010001000000000101010000000000000001010100010000000001010100000000000100010101010001000001010001"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "000010001010101000100010000000100000001010001010000000101000100000100010001000101010001010101010000000001010000000101000100000100"
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

        length = testUtilSetSymbol(symbol, BARCODE_USPS_IMAIL, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_csv", test_csv },
        { "test_hrt", test_hrt },
        { "test_input", test_input },
        { "test_encode", test_encode },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
