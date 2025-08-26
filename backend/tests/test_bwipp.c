/* Test BWIPP against ZXing-C++ (no zint involved) */
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

#include <math.h>
#include "testcommon.h"

#define FLAG_FULL_8BIT  0
#define FLAG_LATIN_1    1
#define FLAG_ASCII      2

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

typedef int (*random_width_func_t)(const struct random_item *, const int);

static void test_bwipp_random(const testCtx *const p_ctx, const struct random_item *rdata,
                random_width_func_t width_func) {
#ifndef _WIN32
    int debug = p_ctx->debug;

    int i, length, ret;
#endif
    struct zint_symbol *symbol = NULL;

#ifndef _WIN32
    char data_buf[4096];
    char bwipp_buf[0x100000]; /* Megabyte */
    char escaped[40960];
    char escaped2[40960];
    char cmp_buf[0x100000];
    char cmp_msg[40960];
    char ret_buf[40960] = {0}; /* Suppress clang -fsanitize=memory false positive */

    const int iterations = p_ctx->arg ? p_ctx->arg : 10000; /* Use "-a N" to set iterations */

    /* Requires to be run with "-d 1024" (see ZINT_DEBUG_TEST_BWIPP_ZXINGCPP in "testcommon.h") */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP_ZXINGCPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_BWIPP_ZXINGCPP) && testUtilHaveZXingCPPDecoder();
#endif

    testStartSymbol(p_ctx->func_name, &symbol);

#ifdef _WIN32
    testSkip("Test not implemented on Windows");
#else
    if (!do_bwipp || !do_zxingcpp) {
        testSkip("Test requires BWIPP and ZXing-C++");
        return;
    }

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    printf("        iterations %d\n", iterations);

    for (i = 0; i < iterations; i++) {
        int bwipp_len, cmp_len, ret_len;

        length = arc4random_uniform(rdata->max_len) + 1;

        arc4random_buf(data_buf, length);

        testUtilSetSymbol(symbol, rdata->symbology, rdata->input_mode, rdata->eci,
                            rdata->option_1, rdata->option_2, rdata->option_3, rdata->output_options,
                            data_buf, length, debug);

        assert_nonzero(testUtilCanBwipp(i, symbol, rdata->option_1, rdata->option_2, rdata->option_3, debug),
                    "i:%d testUtilCanBwipp != 0\n", i);
        assert_nonzero(testUtilCanZXingCPP(i, symbol, data_buf, length, debug), "i:%d testUtilCanZXingCPP != 0\n", i);

        symbol->rows = 0;
        ret = testUtilBwipp(i, symbol, rdata->option_1, rdata->option_2, rdata->option_3, data_buf, length, NULL,
                    bwipp_buf, sizeof(bwipp_buf), NULL);
        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

        bwipp_len = strlen(bwipp_buf);
        assert_nonzero(bwipp_len, "i:%d bwipp_len %d = 0\n", i, bwipp_len);

        symbol->width = width_func ? width_func(rdata, bwipp_len) : bwipp_len;
        assert_nonzero(symbol->width, "i:%d symbol->width zero\n", i);

        ret = testUtilZXingCPP(i, symbol, data_buf, length, bwipp_buf, 899 /*zxingcpp_cmp*/, cmp_buf, sizeof(cmp_buf),
                                &cmp_len);
        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);
        /*fprintf(stderr, "cmp_len %d\n", cmp_len);*/

        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data_buf, length, NULL /*primary*/,
                    ret_buf, &ret_len);
        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                    i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg,
                    testUtilEscape(cmp_buf, cmp_len, escaped, sizeof(escaped)),
                    testUtilEscape(ret_buf, ret_len, escaped2, sizeof(escaped2)));
    }

    ZBarcode_Delete(symbol);

    testFinish();
#endif /* _WIN32 */
}

static int sqrt_width_func(const struct random_item *rdata, const int bwipp_len) {
    const int width = (int) sqrt(bwipp_len);
    const int sq = width * width;
    (void)rdata;
    if (sq != bwipp_len) {
        fprintf(stderr, "sqrt_width_func: width %d, bwipp_len %d, sq %d\n", width, bwipp_len, sq);
    }
    return sq == bwipp_len? width : 0;
}

static void test_aztec(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_AZTEC, DATA_MODE, 899, -1, -1, -1, -1, 1600
    };

    test_bwipp_random(p_ctx, &rdata, sqrt_width_func);
}

static int codablockf_width_func(const struct random_item *rdata, const int bwipp_len) {
    const int row_bits = rdata->option_2 * 11 + 2;
    const int mod = bwipp_len % row_bits;
    if (mod) {
        fprintf(stderr, "codablockf_width_func: row_bits %d, bwipp_len %d, mod %d\n", row_bits, bwipp_len, mod);
    }
    return mod == 0 ? row_bits : 0;
}

static void test_codablockf(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_LATIN_1, BARCODE_CODABLOCKF, DATA_MODE, 0, -1, 30 + 5, -1, -1, 500
    };

    test_bwipp_random(p_ctx, &rdata, codablockf_width_func);
}

static void test_code128(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_LATIN_1, BARCODE_CODE128, DATA_MODE, 0, -1, -1, -1, -1, 80
    };

    test_bwipp_random(p_ctx, &rdata, NULL /*width_func*/);
}

static void test_datamatrix(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_DATAMATRIX, DATA_MODE, 0, -1, 21, DM_SQUARE, -1, 800
    };

    test_bwipp_random(p_ctx, &rdata, sqrt_width_func);
}

/* TODO: explore why "zxingcppdecoder" fails */
#if 0
static int dotcode_width_func(const struct random_item *rdata, const int bwipp_len) {
    const int row_bits = rdata->option_2 >= 1 ? bwipp_len / rdata->option_2 : 0;
    const int mod = row_bits ? bwipp_len % row_bits : -1;
    if (mod) {
        fprintf(stderr, "dotcode_width_func: row_bits %d, bwipp_len %d, mod %d\n", row_bits, bwipp_len, mod);
    }
    return mod == 0 ? row_bits : 0;
}

static void test_dotcode(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_DOTCODE, DATA_MODE, 0, -1, 50, -1, -1, 200
    };

    test_bwipp_random(p_ctx, &rdata, dotcode_width_func);
}
#endif

static int micropdf417_width_func(const struct random_item *rdata, const int bwipp_len) {
    static const short widths[4] = { 38, 55, 82, 99 };
    const int row_bits = rdata->option_2 >= 1 && rdata->option_2 <= 4 ? widths[rdata->option_2 - 1] : 0;
    const int mod = row_bits ? bwipp_len % row_bits : -1;
    if (mod) {
        fprintf(stderr, "micropdf417_width_func: row_bits %d, bwipp_len %d, mod %d\n", row_bits, bwipp_len, mod);
    }
    return mod == 0 ? row_bits : 0;
}

static void test_micropdf417(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_MICROPDF417, DATA_MODE, 0, -1, 4, -1, -1, 120
    };

    test_bwipp_random(p_ctx, &rdata, micropdf417_width_func);
}

static int pdf417_width_func(const struct random_item *rdata, const int bwipp_len) {
    const int row_bits = (rdata->option_2 + 4) * 17 + 1;
    const int mod = bwipp_len % row_bits;
    if (mod) {
        fprintf(stderr, "pdf417_width_func: row_bits %d, bwipp_len %d, mod %d\n", row_bits, bwipp_len, mod);
    }
    return mod == 0 ? row_bits : 0;
}

static void test_pdf417(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_PDF417, DATA_MODE, 0, -1, 20, -1, -1, 800
    };

    test_bwipp_random(p_ctx, &rdata, pdf417_width_func);
}

static void test_qr(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_QRCODE, DATA_MODE, 0, 1, 21, -1, -1, 800
    };

    test_bwipp_random(p_ctx, &rdata, sqrt_width_func);
}

static int rmqr_width_func(const struct random_item *rdata, const int bwipp_len) {
    static const short vers[32] = {
            43, 59, 77, 99, 139,
            43, 59, 77, 99, 139,
        27, 43, 59, 77, 99, 139,
        27, 43, 59, 77, 99, 139,
            43, 59, 77, 99, 139,
            43, 59, 77, 99, 139,
    };
    const int row_bits = rdata->option_2 >= 1 && rdata->option_2 <= ARRAY_SIZE(vers) ? vers[rdata->option_2 - 1] : 0;
    if (row_bits == 0) {
        fprintf(stderr, "rmqr_width_func: row_bits %d, bwipp_len %d, option_2 %d\n", row_bits, bwipp_len,
                    rdata->option_2);
    }
    return row_bits;
}

static void test_rmqr(const testCtx *const p_ctx) {
    struct random_item rdata = {
        FLAG_FULL_8BIT, BARCODE_RMQR, DATA_MODE, 0, 2, 32, -1, -1, 140
    };

    test_bwipp_random(p_ctx, &rdata, rmqr_width_func);
}

static int cnv_hex_data(const char *hex, char *buf, const int buf_size) {
    const char *h = hex;
    const char *const he = hex + strlen(hex);
    char *str_end;
    int i;

    for (i = 0; i < buf_size && h < he; i++) {
        buf[i] = (char) strtol(h, &str_end, 16);
        if (str_end == h) {
            return -1;
        }
        h = str_end + 1;
    }
    if (i < buf_size) buf[i] = '\0';
    return i == buf_size ? -1 : i;
}

static void test_aztec_bwipjs_354(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "08 77 00 81 A2 2E 20 47 C3 92 B8 F1 AD" },
        /*  1*/ { "86 A0 B3 0D 0A 41 86 66 8E FD 43" },
        /*  2*/ { "AE 45 C1 84 90 0D 0A 59 8C 4D DB 33 44" },
        /*  3*/ { "38 15 AC B3 FA 3A 20 56 0A 94 66 A7 D7" },
        /*  4*/ { "7D 82 BC 46 6A 0D 0A 4A CA 89 B3 D4 66" },
        /*  5*/ { "BD 9C 7F 2C 20 F2 99 D0 66 49 7F 3E F2 44 CD 10 EC 1B 38 3E 27 07 2C 58 5E 42 A3 B4 D3 D9 98 61 F1 B6 E6 0F 82 3A FE FB 05 09 A2 22 8C E6 56 33 49 66 00 3B 5D 0B 5F 81 51 72 BA 6A B0 8C D2 68 2D FA 71 F6 C1 1A FC 1A 85 18 30 89 0F BC FD 44 CB D5 B3 84 5D FE 25 4E 70 86 12 D9 65 D7 6F 3D CD 97 27 07 31 46 F3 85 9A 0A 87 D4 02 B8 98 56 44 56 17 00 7F 21 81 C8 5F 37 81 C0 CC 98 87 2C 2D C5 8D 53 7D 46 8D 6E 8C 20 8A 9B 79 4E 33 C6 55 07 A2 05 C8 82 D0 EF CF 86 F1 79 00 0A 6F 4E 18 52 D8 64 51 0F 0A C5 9A CD 6B BD C0 A0 AB 24 F5 1D 2F E3 4D CF B7 92 7F 9C 24 48 6C D5 CD 21 AD 3C FE 65 5B 19 51 F5 8C 72 01 5D 51 CC 9C 50 7C 95 12 D5 32 E4 17 6F 39 EB 91 37 36 C2 D5 5B 30 DE F6 F1 FF 83 8F 80 75 24 E4 2A EF AA AD C5 A3 5B F0 B3 94 73 9C C9 00 D0 52 DF 1A 8A 2D DE E7 F4 45 FA FE EA 50 4D 12 DF 51 D5 5E D1 97 74 95 48 D6 C5 CB 7D E0 23 91 67 4B F3 DD 07 19 61 05 CC 91 52 D8 33 BB B8 B6 87 16 1C C5 28 72 6C 31 A8 04 7E 90 E8 23 94 4F C6 37 E7 5F 38 01 81 42 08 44 45 56 44 56 11 04 21" },
        /*  6*/ { "CF 03 28 05 68 3A 20 4A DF 7B 82 61 24" },
        /*  7*/ { "AD F2 EF 17 99 3A 20 53 94 9F C3 1D 87" },
        /*  8*/ { "63 F4 F5 FE 28 0D 0A 4C 05 02 DA DA 3F" },
        /*  9*/ { "C5 8C 5F D5 DF 2C 20 56 F3 25 91 57 A0" },
        /* 10*/ { "CC A4 68 67 8D 3A 20 51 38 90 71 50 D8" },
        /* 11*/ { "A9 CE F9 ED 9E 2E 20 53 42 BC 8E 12 57" },
        /* 12*/ { "2D CD 5A 50 8A 0D 0A 46 A4 84 6B B0 6F" },
        /* 13*/ { "C1 30 C4 53 C2 2E 20 4A DF 14 48 94 B1" },
        /* 14*/ { "06 CF 01 7A 73 3A 20 59 76 E0 B2 A4 52" },
        /* 15*/ { "63 D3 1A A2 86 2C 20 1F 2E 20 F4 F9 BB 72 D6 40 15 47 29 4B 7F 18 E2 E6 FF C7 B5 70 8B 0C 66 4D EF 68 9A 05 56 44 56 17 00 7F 21 81 C8 5F 37 81 C0 CC 98 87 2C 2D C5 8D 53 7D 46 8D 6E 8C 20 8A 9B 79 4E 33 C6 55 07 A2 05 C8 82 D0 EF CF 86 F1 79 00 0A 6F 4E 18 52 D8 64 51 0F 0A C5 9A CD 6B BD C0 A0 AB 24 F5 1D 2F E3 4D CF B7 92 7F 9C 24 48 6C D5 CD 21 AD 3C FE 65 5B 19 51 F5 8C 72 01 5D 51 CC 9C 50 7C 95 12 D5 32 E4 17 6F 39 EB 91 37 36 C2 D5 5B 30 DE F6 F1 FF 83 8F 80 75 24 E4 2A EF AA AD C5 A3 5B F0 B3 94 73 9C C9 00 D0 52 DF 1A 8A 2D DE E7 F4 45 FA FE EA 50 4D 12 DF 51 D5 5E D1 97 74 95 48 D6 C5 CB 7D E0 23 91 67 4B F3 DD 07 19 61 05 CC 91 52 D8 33 BB B8 B6 87 16 1C C5 28 72 6C 31 A8 04 7E 90 E8 23 94 4F C6 37 E7 5F 38 01 81 42 08 44 45 56 44 56 11 04 21" },
        /* 16*/ { "D4 E0 65 86 30 3A 20 49 0B F7 A3 2B 84" },
        /* 17*/ { "14 76 23 3A 20 44 7E 4B E9 D5 2C" },
        /* 18*/ { "D1 45 B1 02 EF 0D 0A 34 C6 33 27 FF B5 80 3F EB 3D 06 CD 34 17 FA 73 7F 23 31 83 F9 03 F8 47 60 03 BD 4B 13 58 15 E4 B8 A7 BD CA 0E 1C 6D BC D0 92 18 73 4B A9 EB 50 07 73 2A 59 F0 0F 8B 27 A3 1D 0E 7B CB FB 9C B3 10 38 03 0D 2D C4 5D B0 3F 61 FE FF BA 3B 45 8F 11 83 A6 74 2B 72 BB D3 A0 B8 10 1F D8 6A 42 BC 3F EA 57 9A 05 56 44 56 17 00 7F 21 81 C8 5F 37 81 C0 CC 98 87 2C 2D C5 8D 53 7D 46 8D 6E 8C 20 8A 9B 79 4E 33 C6 55 07 A2 05 C8 82 D0 EF CF 86 F1 79 00 0A 6F 4E 18 52 D8 64 51 0F 0A C5 9A CD 6B BD C0 A0 AB 24 F5 1D 2F E3 4D CF B7 92 7F 9C 24 48 6C D5 CD 21 AD 3C FE 65 5B 19 51 F5 8C 72 01 5D 51 CC 9C 50 7C 95 12 D5 32 E4 17 6F 39 EB 91 37 36 C2 D5 5B 30 DE F6 F1 FF 83 8F 80 75 24 E4 2A EF AA AD C5 A3 5B F0 B3 94 73 9C C9 00 D0 52 DF 1A 8A 2D DE E7 F4 45 FA FE EA 50 4D 12 DF 51 D5 5E D1 97 74 95 48 D6 C5 CB 7D E0 23 91 67 4B F3 DD 07 19 61 05 CC 91 52 D8 33 BB B8 B6 87 16 1C C5 28 72 6C 31 A8 04 7E 90 E8 23 94 4F C6 37 E7 5F 38 01 81 42 08 44 45 56 44 56 11 04 21" },
        /* 19*/ { "E8 F4 51 3F F4 3A 20 0D 84 DA 14 44 E8" },
        /* 20*/ { "2B F2 AB 75 3D 2C 20 3B 50 49 03 2B CD" },
        /* 21*/ { "D8 31 62 57 6E 0D 0A 49 04 10 D2 6A E8" },
        /* 22*/ { "81 80 2E 66 91 2C 20 59 35 4D C5 79 89" },
        /* 23*/ { "37 5D 72 EB DD 2C 20 DB 6C 8C B8 34 46 FE 45 86 14 92 33 B0 AC 87 59 29 3B AF EE 45 ED 99 3A EA FB F5 A5 B4 14 61 5A 09 CB C9 29 F5 83 E7 9B C2 0B F6 E6 5B D6 4A 7F 37 EE 76 8E 03 FB 04 2D E2 20 5D 5D C4 79 F6 64 03 C9 D4 1E C1 3C CA 36 C3 DA 5D 79 90 2F 88 BD 8E FE 60 AA DF 77 28 5E 73 59 69 21 44 71 01 F2 AA 99 3A 9A 05 56 44 56 17 00 7F 21 81 C8 5F 37 81 C0 48 CA D1 84 F0 E7 DC 2E 97 1E 24 29 57 8B 24 01 45 78 F3 05 68 57 89 72 14 BB 01 D7 FA 34 D6 A6 DE 23 52 B9 1E 70 AA 27 C0 3C 84 56 05 24 8B 97 0A A6 27 D9 47 35 07 60 F5 92 B4 B9 C2 9C 8A 40 F7 FE 53 7C 60 8E 89 77 BB B6 07 DF 3F FC DA 11 56 13 51 52 A7 68 57 D7 60 45 22 C5 06 7B D6 1F 70 D6 A5 4B 7F F2 C9 2E B4 54 C4 5E 06 8D 6E 76 A2 C3 2F 22 8A 04 45 E4 C1 5D 82 3C CC 88 22 16 68 77 DB 3F E0 61 01 1C B3 83 4B 6B 54 44 24 03 AF DE 4F E4 10 38 86 E4 D3 BE 18 E9 8E AC AC 9F 38 DD F5 6A 4F AB AF E5 87 47 9B 9A A6 0F 66 B2 A1 B2 EE 71 7A AB EE 48 16 DE 51 B0 53 EB 3C 06 5F 38 01 81 42 08 44 45 56 44 56 11 04 21" },
        /* 24*/ { "C7 C6 DA A1 0D 0A 20 03 CF 8F 2E 8E" },
        /* 25*/ { "D1 A5 72 92 8D 0D 0A 4B 86 38 C8 92 A2" },
        /* 26*/ { "4D 62 77 F8 E6 3A 20 47 AB 2C 1A 17 74" },
        /* 27*/ { "A0 6D 63 A3 92 2C 20 58 F5 4B F4 4B 37" },
        /* 28*/ { "1D 14 1A D3 98 2C 20 20 70 7D D0 E9 23" },
        /* 29*/ { "7A 60 61 0F EE 2E 20 53 B0 82 59 52 2E" },
        /* 30*/ { "27 CB FE 95 25 2C 20 46 20 8A AB B1 4E" },
        /* 31*/ { "D5 CD 8C 8E B2 2C 20 52 13 16 8B C9 69" },
        /* 32*/ { "22 BA 6C 8B A6 0D 0A B7 22 F8 FC F8 40 7C 47 26 F6 0E F1 1D 9A 51 B9 A8 E0 09 C8 B6 A9 27 C9 A9 EB F1 8F D8 82 CA 6B BA 27 B6 72 44 12 D9 EC C5 CF 6B BD 94 1F F9 9C C7 4C 1B 1C 77 40 8A 21 74 22 4C DC E6 38 D1 9D 41 9E 95 48 87 35 85 D1 32 D4 9E F6 20 D2 38 5E B5 77 E9 07 DF F4 84 44 8B DB 9D 09 80 78 9E 2C DA 7B 1B 9A 05 56 44 56 17 00 7F 21 81 C8 5F 37 81 C0 CC 98 87 2C 2D C5 8D 53 7D 46 8D 6E 8C 20 8A 9B 79 4E 33 C6 55 07 A2 05 C8 82 D0 EF CF 86 F1 79 00 0A 6F 4E 18 52 D8 64 51 0F 0A C5 9A CD 6B BD C0 A0 AB 24 F5 1D 2F E3 4D CF B7 92 7F 9C 24 48 6C D5 CD 21 AD 3C FE 65 5B 19 51 F5 8C 72 01 5D 51 CC 9C 50 7C 95 12 D5 32 E4 17 6F 39 EB 91 37 36 C2 D5 5B 30 DE F6 F1 FF 83 8F 80 75 24 E4 2A EF AA AD C5 A3 5B F0 B3 94 73 9C C9 00 D0 52 DF 1A 8A 2D DE E7 F4 45 FA FE EA 50 4D 12 DF 51 D5 5E D1 97 74 95 48 D6 C5 CB 7D E0 23 91 67 4B F3 DD 07 19 61 05 CC 91 52 D8 33 BB B8 B6 87 16 1C C5 28 72 6C 31 A8 04 7E 90 E8 23 94 4F C6 37 E7 5F 38 01 81 42 08 44 45 56 44 56 11 04 21" },
        /* 33*/ { "3A 3B A3 B0 3D 2E 20 4C 12 7A 39 06 CB" },
        /* 34*/ { "4E C6 3E B8 DD 3A 20 6E 72 67 2B BE 56 92 DB" },
        /* 35*/ { "C5 48 67 EA FB 3A 20 07 1F 01 F3 EF 82 85 F0 18 1C 86 E4 E7 70 AD 9A 2A 68 73 4A 82 8A 09 33 16 9C CC 83 61 2D 36 E0 EF 57 96 DB 64 08 90 1B 69 23 77 E9 97 9D 4B 2A 2B F8 09 D8 85 86 BC 80 F7 D0 5E B9 8E 5E D3 33 4E C1 13 58 0D 6A 68 77 CE 6C B0 49 C4 9F 58 86 D3 95 F7 CE A6 DD F0 6E 9A 05 56 44 56 17 00 7F 21 81 C8 5F 37 81 C0 CC 98 87 2C 2D C5 8D 53 7D 46 8D 6E 8C 20 8A 9B 79 4E 33 C6 55 07 A2 05 C8 82 D0 EF CF 86 F1 79 00 0A 6F 4E 18 52 D8 64 51 0F 0A C5 9A CD 6B BD C0 A0 AB 24 F5 1D 2F E3 4D CF B7 92 7F 9C 24 48 6C D5 CD 21 AD 3C FE 65 5B 19 51 F5 8C 72 01 5D 51 CC 9C 50 7C 95 12 D5 32 E4 17 6F 39 EB 91 37 36 C2 D5 5B 30 DE F6 F1 FF 83 8F 80 75 24 E4 2A EF AA AD C5 A3 5B F0 B3 94 73 9C C9 00 D0 52 DF 1A 8A 2D DE E7 F4 45 FA FE EA 50 4D 12 DF 51 D5 5E D1 97 74 95 48 D6 C5 CB 7D E0 23 91 67 4B F3 DD 07 19 61 05 CC 91 52 D8 33 BB B8 B6 87 16 1C C5 28 72 6C 31 A8 04 7E 90 E8 23 94 4F C6 37 E7 5F 38 01 81 42 08 44 45 56 44 56 11 04 21" },
        /* 36*/ { "24 47 8C DF B9 2C 20 4A 72 4D 7A 46 BF" },
        /* 37*/ { "D4 2A 1D CE A4 2E 20 44 04 BD 19 7D A4" },
        /* 38*/ { "1F AB 7C 69 B4 3A 20 4B FE 23 7D 5B CD" },
        /* 39*/ { "2B 2F 40 94 6D 2E 20 55 69 DE 3D 15 74" },
        /* 40*/ { "17 CC A0 F7 7D 2C 20 5A 2B 13 72 7F 7B" },
        /* 41*/ { "E6 48 F3 56 A9 3A 20 58 F5 77 45 B8 71" },
        /* 42*/ { "0D 0A 4C FA D8 67 95 EA" },
        /* 43*/ { "52 38 A1 5F F9 2E 20 41 A0 6E 06 6D CD" },
        /* 44*/ { "D6 FD C4 7D 2D 0D 0A 52 A0 CA 01 A0 A8" },
        /* 45*/ { "DB 89 AD 70 F3 2E 20 4A 53 5E 00 DB E1" },
        /* 46*/ { "8C A1 D2 FB F2 3A 20 71 7A 35 97 55 31 3C" },
        /* 47*/ { "81 80 25 A8 BE 2C 20 49 EE C0 B9 81 7F" },
        /* 48*/ { "2E 20 20 27 37 7E AA E4" },
        /* 49*/ { "76 60 3D D7 32 2C 20 48 39 D9 86 CA 93" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    int symbology = BARCODE_AZTEC;
    int input_mode = DATA_MODE;
    int eci = 899;
    int option_1 = 1;
    int option_2 = -1;
    int option_3 = -1;
    int output_options = -1;

    char data_buf[4096];
    char bwipp_buf[32768];
    char escaped[8192];
    char escaped2[8192];
    char cmp_buf[8192];
    char cmp_msg[8192];
    char ret_buf[8192] = {0}; /* Suppress clang -fsanitize=memory false positive */

    /* Requires to be run with "-d 1024" (see ZINT_DEBUG_TEST_BWIPP_ZXINGCPP in "testcommon.h") */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP_ZXINGCPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_BWIPP_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    if (!do_bwipp || !do_zxingcpp) {
        testSkip("Test requires BWIPP and ZXing-C++");
        return;
    }

    for (i = 0; i < data_size; i++) {
        int bwipp_len, cmp_len, ret_len;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = cnv_hex_data(data[i].data, data_buf, ARRAY_SIZE(data_buf));
        assert_nonzero(length, "i:%d cnv_hex_data length zero\n", i);

        #if 0
        z_debug_print_escape(TCU(data_buf), length, NULL);
        printf("\n");
        #endif

        testUtilSetSymbol(symbol, symbology, input_mode, eci, option_1, option_2, option_3,
                            output_options, data_buf, length, debug);

        assert_nonzero(testUtilCanBwipp(i, symbol, option_1, option_2, option_3, debug),
                    "i:%d testUtilCanBwipp != 0\n", i);
        assert_nonzero(testUtilCanZXingCPP(i, symbol, data_buf, length, debug), "i:%d testUtilCanZXingCPP != 0\n", i);

        symbol->rows = 0;
        ret = testUtilBwipp(i, symbol, option_1, option_2, option_3, data_buf, length, NULL, bwipp_buf,
                    sizeof(bwipp_buf), NULL);
        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

        bwipp_len = strlen(bwipp_buf);
        assert_nonzero(bwipp_len, "i:%d bwipp_len %d = 0\n", i, bwipp_len);

        symbol->width = sqrt_width_func(NULL /*rdata*/, bwipp_len);
        assert_equal(symbol->width * symbol->width, bwipp_len,
                    "i:%d symbol->width^2 %d != bwipp_len %d (symbol->width %d)\n",
                    i, symbol->width * symbol->width, bwipp_len, symbol->width);

        ret = testUtilZXingCPP(i, symbol, data_buf, length, bwipp_buf, 1 /*zxingcpp_cmp*/, cmp_buf, sizeof(cmp_buf),
                    &cmp_len);
        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);
        /*fprintf(stderr, "cmp_len %d\n", cmp_len);*/

        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data_buf, length, NULL /*primary*/,
                    ret_buf, &ret_len);
        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                    i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg,
                    testUtilEscape(cmp_buf, cmp_len, escaped, sizeof(escaped)),
                    testUtilEscape(ret_buf, ret_len, escaped2, sizeof(escaped2)));

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_codablockf_fnc4_digit(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "EE 9F 56 C8 B6 37 36 37" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct random_item s_rdata = { 0, BARCODE_CODABLOCKF, DATA_MODE, 0, -1, 8 + 5, -1, -1, 0 };
    struct random_item *rdata = &s_rdata;

    char data_buf[4096];
    char bwipp_buf[32768];
    char escaped[8192];
    char escaped2[8192];
    char cmp_buf[8192];
    char cmp_msg[8192];
    char ret_buf[8192] = {0}; /* Suppress clang -fsanitize=memory false positive */

    /* Requires to be run with "-d 1024" (see ZINT_DEBUG_TEST_BWIPP_ZXINGCPP in "testcommon.h") */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP_ZXINGCPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_BWIPP_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    if (!do_bwipp || !do_zxingcpp) {
        testSkip("Test requires BWIPP and ZXing-C++");
        return;
    }

    for (i = 0; i < data_size; i++) {
        int bwipp_len, cmp_len, ret_len;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = cnv_hex_data(data[i].data, data_buf, ARRAY_SIZE(data_buf));
        assert_nonzero(length, "i:%d cnv_hex_data length zero\n", i);

        #if 0
        z_debug_print_escape(TCU(data_buf), length, NULL);
        printf("\n");
        #endif

        testUtilSetSymbol(symbol, rdata->symbology, rdata->input_mode, rdata->eci,
                            rdata->option_1, rdata->option_2, rdata->option_3, rdata->output_options,
                            data_buf, length, debug);

        assert_nonzero(testUtilCanBwipp(i, symbol, rdata->option_1, rdata->option_2, rdata->option_3, debug),
                    "i:%d testUtilCanBwipp != 0\n", i);
        assert_nonzero(testUtilCanZXingCPP(i, symbol, data_buf, length, debug), "i:%d testUtilCanZXingCPP != 0\n", i);

        symbol->rows = 0;
        ret = testUtilBwipp(i, symbol, rdata->option_1, rdata->option_2, rdata->option_3, data_buf, length, NULL,
                    bwipp_buf, sizeof(bwipp_buf), NULL);
        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

        bwipp_len = strlen(bwipp_buf);
        assert_nonzero(bwipp_len, "i:%d bwipp_len %d = 0\n", i, bwipp_len);

        symbol->width = codablockf_width_func(rdata, bwipp_len);
        assert_nonzero(symbol->width, "i:%d symbol->width == 0\n", i);

        ret = testUtilZXingCPP(i, symbol, data_buf, length, bwipp_buf, 899 /*zxingcpp_cmp*/, cmp_buf, sizeof(cmp_buf),
                    &cmp_len);
        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);
        /*fprintf(stderr, "cmp_len %d\n", cmp_len);*/

        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data_buf, length, NULL /*primary*/,
                    ret_buf, &ret_len);
        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                    i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg,
                    testUtilEscape(cmp_buf, cmp_len, escaped, sizeof(escaped)),
                    testUtilEscape(ret_buf, ret_len, escaped2, sizeof(escaped2)));

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_aztec", test_aztec },
        { "test_codablockf", test_codablockf },
        { "test_code128", test_code128 },
        { "test_datamatrix", test_datamatrix },
        #if 0
        { "test_dotcode", test_dotcode },
        #endif
        { "test_micropdf417", test_micropdf417 },
        { "test_pdf417", test_pdf417 },
        { "test_qr", test_qr },
        { "test_rmqr", test_rmqr },
        { "test_aztec_bwipjs_354", test_aztec_bwipjs_354 },
        { "test_codablockf_fnc4_digit", test_codablockf_fnc4_digit },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
