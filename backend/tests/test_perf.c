/* These are not real tests, just performance indicators */
/*
    libzint - the open source barcode library
    Copyright (C) 2025 Robin Stuart <rstuart114@gmail.com>

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

#include <time.h>
#include "testcommon.h"

#define TEST_PERF_TIME(arg) ((arg) * 1000.0 / CLOCKS_PER_SEC)

struct perf_item {
    int symbology;
    int input_mode;
    int option_1;
    int option_2;
    int option_3;
    const char *data;
    const char *composite_primary;
    int ret;

    int expected_rows;
    int expected_width;
    const char *comment;
};

static void test_perf(const testCtx *const p_ctx, const int default_iterations, const struct perf_item *data,
                const int data_size) {
    int debug = p_ctx->debug;

    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    clock_t start;
    clock_t total_create = 0, total_encode = 0, total_buffer = 0, total_buf_inter = 0, total_print = 0;
    clock_t diff_create, diff_encode, diff_buffer, diff_buf_inter, diff_print;
    int comment_max = 0;

    const int iterations = p_ctx->arg ? p_ctx->arg : default_iterations; /* Use "-a N" to set iterations */

    const char *text;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);
    }

    printf("     Iterations %d\n", iterations);

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = diff_buf_inter = diff_print = 0;

        for (j = 0; j < iterations; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            if (strlen(data[i].composite_primary)) {
                if (z_is_composite(data[i].symbology)) {
                    text = data[i].composite_primary;
                    assert_nonzero((int) strlen(data[i].data) < ARRAY_SIZE(symbol->primary),
                                "i:%d composite length %d >= %d\n",
                                i, (int) strlen(data[i].data), ARRAY_SIZE(symbol->primary));
                    strcpy(symbol->primary, data[i].data);
                } else {
                    text = data[i].data;
                    assert_nonzero((int) strlen(data[i].composite_primary) < ARRAY_SIZE(symbol->primary),
                                "i:%d primary length %d >= %d\n",
                                i, (int) strlen(data[i].composite_primary), ARRAY_SIZE(symbol->primary));
                    strcpy(symbol->primary, data[i].composite_primary);
                }
            } else {
                text = data[i].data;
            }

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/,
                                        data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/,
                                        text, -1, debug);

            start = clock();
            ret = ZBarcode_Encode(symbol, TCU(text), length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                        i, ret, data[i].ret, symbol->errtxt);

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n",
                        i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n",
                        i, symbol->width, data[i].expected_width, data[i].data);

            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

            symbol->output_options |= OUT_BUFFER_INTERMEDIATE;
            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buf_inter += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer OUT_BUFFER_INTERMEDIATE ret %d != 0 (%s)\n",
                        i, ret, symbol->errtxt);
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; /* Undo */

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

            ZBarcode_Delete(symbol);
        }

        printf("     %*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n",
                comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_buf_inter),
                TEST_PERF_TIME(diff_print), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
        total_buf_inter += diff_buf_inter;
        total_print += diff_print;
    }
    if (p_ctx->index == -1) {
        printf("     %*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n",
                comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter),
                TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }

    testFinish();
}

static void test_2of5(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_C25INTER, -1, -1, -1, -1, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "", 0, 1, 819, "C25INTER 90" },
        /*  1*/ { BARCODE_C25INTER, -1, -1, -1, -1, "1234567890", "", 0, 1, 99, "C25INTER 10" },
        /*  2*/ { BARCODE_C25STANDARD, -1, -1, -1, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "", 0, 1, 817, "C25STANDARD 80" },
        /*  3*/ { BARCODE_C25STANDARD, -1, -1, -1, -1, "1234567890", "", 0, 1, 117, "C25STANDARD 10" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_aztec(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_AZTEC, -1, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
                    "", 0, 49, 49, "286 chars, 8-bit words, upper" },
        /*  1*/ { BARCODE_AZTEC, -1, -1, -1, -1,
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
                    "", 0, 79, 79, "900 chars, 10-bit words, numeric" },
        /*  2*/ { BARCODE_AZTEC, -1, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377",
                    "", 0, 91, 91, "980 chars, 10-bit words, mixed" },
        /*  3*/ { BARCODE_AZTEC, -1, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz ~~~~~~~~~~~~~~~~~~~~~~~~~ ?????????????????????????? 12345678901234567890123456 \377\377\377\377\377\377",
                    "", 0, 113, 113, "1540 chars, 12-bit words, mixed" },
        /*  4*/ { BARCODE_AZRUNE, -1, -1, -1, -1,
                    "255",
                    "", 0, 11, 11, "3 chars, AZRUNE" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 1 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_code11(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, -1, -1, "1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-1234567890-", "", 0, 1, 966, "CODE11 121" },
        /*  1*/ { BARCODE_CODE11, -1, -1, -1, -1, "1234567890-", "", 0, 1, 116, "CODE11 5" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_code128(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, -1, "123456ABCD123456ABCD123456ABCD123456ABCD123456ABCD123456ABCD", "", 0, 1, 618, "CODE128 60" },
        /*  1*/ { BARCODE_CODE128, -1, -1, -1, -1, "123456ABCD", "", 0, 1, 123, "CODE128 10" },
        /*  2*/ { BARCODE_GS1_128, -1, -1, -1, -1, "[01]09501101530003", "", 0, 1, 134, "GS1_128 (01)" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_composite(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_EAN13_CC, -1, 1, -1, -1, "123456789012",
                    "[91]123456789012345678901234567890123456789012345678901234",
                    0, 11, 99, "58 chars CC-A" },
        /*  1*/ { BARCODE_UPCA_CC, -1, 2, -1, -1, "12345678901",
                    "[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "[94]12345678901234567890123456789012345678901234567890",
                    0, 48, 99, "336 chars CC-B" },
        /*  2*/ { BARCODE_GS1_128_CC, -1, 3, -1, -1, "[01]12345678901231",
                    "[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "[94]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "[95]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
                    "[96]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
                    0, 32, 205, "564 chars CC-C" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 1 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_dmatrix(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    "", 0, 96, 96, "960 chars, text/numeric" },
        /*  1*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    "", 0, 96, 96, "960 chars, text/numeric" },
        /*  2*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    "", 0, 120, 120, "960 chars, byte" },
        /*  3*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    "", 0, 120, 120, "960 chars, byte" },
        /*  4*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, "https://example.com/01/09506000134369", "", 0, 22, 22, "37 chars, text/numeric" },
        /*  5*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, "https://example.com/01/09506000134369", "", 0, 22, 22, "37 chars, text/numeric" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_dotcode(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_DOTCODE, -1, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    "", 0, 124, 185, "960 chars, text/numeric" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 1 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_gridmtx(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, -1, -1, -1,
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738",
                    "", 0, 66, 66, "97 chars, mixed modes" },
        /*  1*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, -1, -1, -1,
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738",
                    "", 0, 162, 162, "970 chars, mixed modes" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 1 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_hanxin(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_HANXIN, UNICODE_MODE, -1, -1, -1,
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。",
                    "", ZINT_WARN_NONCOMPLIANT, 43, 43, "98 chars, Region One and Text" },
        /*  1*/ { BARCODE_HANXIN, UNICODE_MODE, -1, -1, -1,
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。",
                    "", ZINT_WARN_NONCOMPLIANT, 121, 121, "980 chars, Region One and Text" },
        /*  2*/ { BARCODE_HANXIN, UNICODE_MODE, -1, -1, -1,
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。",
                    "", ZINT_WARN_NONCOMPLIANT, 147, 147, "1470 chars, Region One and Text" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 1 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_maxicode(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_MAXICODE, UNICODE_MODE | ESCAPE_MODE, -1, -1, -1,
                    "1Z34567890\\GUPSN\\G102562\\G034\\G\\G1/1\\G\\GY\\G2201 Second St\\GFt Myers\\GFL\\R\\E",
                    "339010000840001", 0, 33, 30, "Mode 2" },
        /*  1*/ { BARCODE_MAXICODE, UNICODE_MODE, 4, -1, -1, "MaxiCode (19 chars)", "", 0, 33, 30, "Mode 4 small" },
        /*  2*/ { BARCODE_MAXICODE, DATA_MODE | ESCAPE_MODE, 4, -1, -1, "ABCDabcdAabcABabcABCabcABCDaABCabABCabcABC\\d233a", "", 0, 33, 30, "Mode 4 medium" },
        /*  3*/ { BARCODE_MAXICODE, DATA_MODE | ESCAPE_MODE, 4, -1, -1,
                    "ABabcdeAabcdABCabcdABabc\\d192\\d192 \\d192\\d224\\d224\\d028\\d224\\d001\\d001\\d001\\d029\\d00112345678a123456789aABCDa\\d192\\d224\\d001\\d192\\d001\\d224\\d030\\d004",
                    "", 0, 33, 30, "Mode 4 latches" },
        /*  4*/ { BARCODE_MAXICODE, UNICODE_MODE, 4, -1, -1,
                    "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "", 0, 33, 30, "Mode 4 txt max" },
        /*  5*/ { BARCODE_MAXICODE, UNICODE_MODE, 4, -1, -1,
                    "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999",
                    "", 0, 33, 30, "Mode 4 num max" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 10 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_pdf417(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, -1, "1234567890", "", 0, 7, 103, "10 numerics" },
        /*  1*/ { BARCODE_PDF417, -1, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    "", 0, 43, 290, "960 chars, text/numeric" },
        /*  2*/ { BARCODE_PDF417, DATA_MODE, -1, -1, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    "", 0, 51, 358, "960 chars, byte" },
        /*  3*/ { BARCODE_PDF417, -1, -1, -1, -1,
                    "BP2D+1.00+0005+FLE ESC BV+1.00+3.60*BX2D+1.00+0001+Casual shoes & apparel+90044030118100801265*D_2D+1.02+31351440315981+C910332+02032018+KXXXX CXXXX+UNIT 4 HXXX"
                    "XXXXX BUSINESS PARK++ST  ALBANS+ST  ALBANS++AL2 3TA+0001+000001+001+00000000+00++N+N+N+0000++++++N+++N*DS2D+1.01+0001+0001+90044030118100801265+++++07852389322+"
                    "+E*F_2D+1.00+0005*",
                    "", 0, 26, 222, "338 chars, text/numeric/byte" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_plessey(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_PLESSEY, -1, -1, -1, -1, "1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1", "", 0, 1, 1107, "PLESSEY 65" },
        /*  1*/ { BARCODE_PLESSEY, -1, -1, -1, -1, "123456ABCD", "", 0, 1, 227, "PLESSEY 10" },
        /*  2*/ { BARCODE_MSI_PLESSEY, -1, -1, -1, -1, "12345678901234567890123456789012345678901234567890123456789012345", "", 0, 1, 787, "MSI_PLESSEY 65" },
        /*  3*/ { BARCODE_MSI_PLESSEY, -1, -1, -1, -1, "1234567890", "", 0, 1, 127, "MSI_PLESSEY 10" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_postal(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_POSTNET, -1, -1, -1, -1, "12345678901", "", 0, 2, 123, "POSTNET 11" },
        /*  1*/ { BARCODE_PLANET, -1, -1, -1, -1, "1234567890123", "", 0, 2, 143, "PLANET 13" },
        /*  2*/ { BARCODE_KOREAPOST, -1, -1, -1, -1, "123456", "", 0, 1, 167, "KOREAPOST 6" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 10 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_qr(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, "12345678901234", "", 0, 21, 21, "14 chars, Numeric mode" },
        /*  1*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, "ABC 123 ABC 123 ABCD", "", 0, 21, 21, "20 chars, Alphanumeric" },
        /*  2*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, "ABCde fG H 123456 IJKlmn, 1234567890 opQ Rst uvwxyz. 1234", "", 0, 29, 29, "57 chars, Alphanumeric" },
        /*  3*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 37, 37, "107 chars, Mixed modes" },
        /*  4*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 53, 53, "214 chars, Mixed modes" },
        /*  5*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 73, 73, "428 chars, Mixed modes" },
        /*  6*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 105, 105, "963 chars, Mixed modes" },
        /*  7*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 133, 133, "1498 chars, Mixed modes" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_qr_fast(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1, "12345678901234", "", 0, 21, 21, "14 chars, Numeric mode" },
        /*  1*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABC 123 ABC 123 ABCD", "", 0, 21, 21, "20 chars, Alphanumeric" },
        /*  2*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1, "ABCde fG H 123456 IJKlmn, 1234567890 opQ Rst uvwxyz. 1234", "", 0, 29, 29, "57 chars, Alphanumeric" },
        /*  3*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 37, 37, "107 chars, Mixed modes" },
        /*  4*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 53, 53, "214 chars, Mixed modes" },
        /*  5*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 73, 73, "428 chars, Mixed modes" },
        /*  6*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 105, 105, "963 chars, Mixed modes" },
        /*  7*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    "", ZINT_WARN_NONCOMPLIANT, 133, 133, "1498 chars, Mixed modes" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_microqr(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 1, -1, "12345", "", 0, 11, 11, "Max 5 numbers, M1" },
        /*  1*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 2, -1, "1234567890", "", 0, 13, 13, "Max 10 numbers, M2-L" },
        /*  2*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 3, -1, "123456789012345", "", 0, 15, 15, "Max 15 numbers, M3-L" },
        /*  3*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 4, -1, "12345678901234567890123456789012345", "", 0, 17, 17, "Max 35 numbers, M4-L" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 5 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

static void test_upcean(const testCtx *const p_ctx) {

    static const struct perf_item data[] = {
        /*  0*/ { BARCODE_EAN13, -1, -1, -1, -1, "123456789012+12345", "", 0, 1, 149, "EAN-13 add-on 5" },
        /*  1*/ { BARCODE_EAN13, -1, -1, -1, -1, "123456789012", "", 0, 1, 95, "EAN-13 no add-on" },
        /*  2*/ { BARCODE_UPCA, -1, -1, -1, -1, "12345678901+12345", "", 0, 1, 151, "UPC-A add-on 5" },
        /*  3*/ { BARCODE_UPCA, -1, -1, -1, -1, "12345678901", "", 0, 1, 95, "UPC-A no add-on" },
        /*  4*/ { BARCODE_EAN8, -1, -1, -1, -1, "1234567+12345", "", ZINT_WARN_NONCOMPLIANT, 1, 121, "EAN-8 add-on 5" },
        /*  5*/ { BARCODE_EAN8, -1, -1, -1, -1, "1234567", "", 0, 1, 67, "EAN-8 no add-on" },
        /*  6*/ { BARCODE_UPCE, -1, -1, -1, -1, "1234567+12", "", 0, 1, 78, "UPC-E add-on 2" },
        /*  7*/ { BARCODE_UPCE, -1, -1, -1, -1, "1234567", "", 0, 1, 51, "UPC-E no add-on" },
        /*  8*/ { BARCODE_EAN_5ADDON, -1, -1, -1, -1, "12345", "", 0, 1, 47, "EAN 5-digit add-on" },
        /*  9*/ { BARCODE_EAN_2ADDON, -1, -1, -1, -1, "12", "", 0, 1, 20, "EAN 2-digit add-on" },
    };
    const int data_size = ARRAY_SIZE(data);
    const int default_iterations = 10 * 1000;

    test_perf(p_ctx, default_iterations, data, data_size);
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_2of5", test_2of5 },
        { "test_aztec", test_aztec },
        { "test_code11", test_code11 },
        { "test_code128", test_code128 },
        { "test_composite", test_composite },
        { "test_dmatrix", test_dmatrix },
        { "test_dotcode", test_dotcode },
        { "test_gridmtx", test_gridmtx },
        { "test_hanxin", test_hanxin },
        { "test_maxicode", test_maxicode },
        { "test_pdf417", test_pdf417 },
        { "test_plessey", test_plessey },
        { "test_postal", test_postal },
        { "test_qr", test_qr },
        { "test_qr_fast", test_qr_fast },
        { "test_microqr", test_microqr },
        { "test_upcean", test_upcean },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
