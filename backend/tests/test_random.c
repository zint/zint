/* Test zint against ZXing-C++ */
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

#include "testcommon.h"

#define FLAG_FULL_8BIT  0
#define FLAG_LATIN_1    1
#define FLAG_ASCII      2
#define FLAG_NUMERIC    4
#define FLAG_MASK       0xFF
#define FLAG_ZERO_FILL  0x100

struct random_item {
    int data_flag;
    int symbology;
    int input_mode;
    int eci;
    int option_1;
    int option_2;
    int option_3;
    int output_options;
    int max_len;
};

static void test_random(const testCtx *const p_ctx, const struct random_item *rdata) {
#ifndef _WIN32
    int debug = p_ctx->debug;
    int i, length, ret;
#endif
    struct zint_symbol *symbol = NULL;

#ifndef _WIN32
    unsigned char data_buf[4096];
    char modules_dump[40960];
    char escaped[40960];
    char escaped2[40960];
    char cmp_buf[40960];
    char cmp_msg[40960];
    char ret_buf[40960] = {0}; /* Suppress clang -fsanitize=memory false positive */

    const int iterations = p_ctx->arg ? p_ctx->arg : 10000; /* Use "-a N" to set iterations */

    /* Requires to be run with "-d 512" (see ZINT_DEBUG_TEST_ZXINGCPP in "testcommon.h") */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder();
#endif

    testStartSymbol(p_ctx->func_name, &symbol);

#ifdef _WIN32
    testSkip("Test not implemented on Windows");
#else

    if (!do_zxingcpp) {
        testSkip("Test requires ZXing-C++");
        return;
    }

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    printf("        iterations %d\n", iterations);

    for (i = 0; i < iterations; i++) {
        int j;
        int cmp_len, ret_len;

        length = arc4random_uniform(rdata->max_len) + 1;

        arc4random_buf(data_buf, length);

        switch (rdata->data_flag & FLAG_MASK) {
            case FLAG_FULL_8BIT: /* Full 8-bit */
                break;
            case FLAG_LATIN_1: /* ASCII + Latin-1 only */
                for (j = 0; j < length; j++) {
                    if (data_buf[j] >= 0x80) {
                        if (data_buf[j] < 0x90) {
                            data_buf[j] = (unsigned char) ((data_buf[j] - 0x80) + 0x70);
                        } else if (data_buf[j] < 0xA0) {
                            data_buf[j] = (unsigned char) ((data_buf[j] - 0x90) + 0xA0);
                        }
                    }
                }
                break;
            case FLAG_ASCII: /* ASCII only */
                for (j = 0; j < length; j++) {
                    data_buf[j] &= 0x7F;
                }
                break;
            case FLAG_NUMERIC: /* Digits only */
                for (j = 0; j < length; j++) {
                    data_buf[j] = '0' + (data_buf[j] % 10);
                }
                break;
            default:
                assert_nonzero(0, "i:%d invalid data_flag %d\n", i, rdata->data_flag);
                break;
        }

        if (rdata->data_flag & FLAG_ZERO_FILL) {
            const int zeroes = rdata->max_len - length;
            memmove(data_buf + zeroes, data_buf, length);
            memset(data_buf, '0', zeroes);
            length = rdata->max_len;
        }

        (void) testUtilSetSymbol(symbol, rdata->symbology, rdata->input_mode, rdata->eci,
                                rdata->option_1, rdata->option_2, rdata->option_3, rdata->output_options,
                                (const char *) data_buf, length, debug);

        ret = ZBarcode_Encode(symbol, data_buf, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

        assert_nonzero(testUtilCanZXingCPP(i, symbol, (const char *) data_buf, length, debug), "i:%d testUtilCanZXingCPP != 0\n", i);

        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                    "i:%d testUtilModulesDump == -1\n", i);

        ret = testUtilZXingCPP(i, symbol, (const char *) data_buf, length, modules_dump, 899 /*zxingcpp_cmp*/, cmp_buf,
                    sizeof(cmp_buf), &cmp_len);
        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);
        /*fprintf(stderr, "cmp_len %d\n", cmp_len);*/

        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, (const char *) data_buf, length,
                    NULL /*primary*/, ret_buf, &ret_len);
        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                    i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg,
                    testUtilUCharArrayDump(TCU(cmp_buf), cmp_len, escaped, sizeof(escaped)),
                    testUtilUCharArrayDump(TCU(ret_buf), ret_len, escaped2, sizeof(escaped2)));

        ZBarcode_Reset(symbol);
    }

    ZBarcode_Delete(symbol);

    testFinish();
#endif
}

static void test_aztec(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_AZTEC, DATA_MODE, 899, 1, 0, 0, -1, 1600
    };

    test_random(p_ctx, &rdata);
}

static void test_codablockf(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_LATIN_1, BARCODE_CODABLOCKF, DATA_MODE, 0, -1, 0, 0, -1, 1300
    };

    test_random(p_ctx, &rdata);
}

static void test_codablockf_ascii(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_ASCII, BARCODE_CODABLOCKF, DATA_MODE, 0, -1, 0, 0, -1, 2200
    };

    test_random(p_ctx, &rdata);
}

static void test_code128(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_LATIN_1, BARCODE_CODE128, DATA_MODE, 0, -1, 0, 0, -1, 61
    };

    test_random(p_ctx, &rdata);
}

static void test_code128_ascii(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_ASCII, BARCODE_CODE128, DATA_MODE, 0, -1, 0, 0, -1, 82
    };

    test_random(p_ctx, &rdata);
}

static void test_datamatrix(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_DATAMATRIX, DATA_MODE, 899, 1, 0, 0, -1, 1550
    };

    test_random(p_ctx, &rdata);
}

static void test_datamatrix_fast(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, 899, 1, 0, 0, -1, 1550
    };

    test_random(p_ctx, &rdata);
}

static void test_dbar_omn(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_NUMERIC | FLAG_ZERO_FILL, BARCODE_DBAR_OMN, DATA_MODE, 0, -1, 0, 0, -1, 13
    };

    test_random(p_ctx, &rdata);
}

static void test_dotcode(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_DOTCODE, DATA_MODE, 899, 1, 0, 0, -1, 620
    };

    test_random(p_ctx, &rdata);
}

static void test_hanxin(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_HANXIN, DATA_MODE, 899, 1, 0, 0, -1, 3261
    };

    test_random(p_ctx, &rdata);
}

static void test_maxicode(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_MAXICODE, DATA_MODE, 899, 4, 0, 0, -1, 49
    };

    test_random(p_ctx, &rdata);
}

static void test_micropdf417(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_MICROPDF417, DATA_MODE, 899, 1, 0, 0, -1, 140
    };

    test_random(p_ctx, &rdata);
}

static void test_pdf417(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_PDF417, DATA_MODE, 899, 1, 0, 0, -1, 1100
    };

    test_random(p_ctx, &rdata);
}

static void test_pdf417_fast(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_PDF417, DATA_MODE | FAST_MODE, 899, 1, 0, 0, -1, 1000
    };

    test_random(p_ctx, &rdata);
}

static void test_qr(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_QRCODE, DATA_MODE, 899, 1, 0, 0, -1, 2920
    };

    test_random(p_ctx, &rdata);
}

static void test_rmqr(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_RMQR, DATA_MODE, 899, 2, 0, 0, -1, 140
    };

    test_random(p_ctx, &rdata);
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_aztec", test_aztec },
        { "test_codablockf", test_codablockf },
        { "test_codablockf_ascii", test_codablockf_ascii },
        { "test_code128", test_code128 },
        { "test_code128_ascii", test_code128_ascii },
        { "test_datamatrix", test_datamatrix },
        { "test_datamatrix_fast", test_datamatrix_fast },
        { "test_dbar_omn", test_dbar_omn },
        { "test_dotcode", test_dotcode },
        { "test_hanxin", test_hanxin },
        { "test_maxicode", test_maxicode },
        { "test_micropdf417", test_micropdf417 },
        { "test_pdf417", test_pdf417 },
        { "test_pdf417_fast", test_pdf417_fast },
        { "test_qr", test_qr },
        { "test_rmqr", test_rmqr },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
