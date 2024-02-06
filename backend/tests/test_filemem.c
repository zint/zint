/*
    libzint - the open source barcode library
    Copyright (C) 2023-2024 Robin Stuart <rstuart114@gmail.com>

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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <sys/stat.h>
#include "testcommon.h"
#include "../common.h"
#include "../filemem.h"

static void test_svg(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        char *outfile;
        char *data;
        int length;
        int ret;

        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, BARCODE_MEMORY_FILE, "out.svg", "ABCDEF", -1, 0,
                    "<?xml version=\"1.0\" standalone=\"no\"?>\n"
                    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
                    "<svg width=\"202\" height=\"117\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n"
                    " <desc>Zint Generated Symbol</desc>\n"
                    " <g id=\"barcode\" fill=\"#000000\">\n"
                    "  <rect x=\"0\" y=\"0\" width=\"202\" height=\"117\" fill=\"#FFFFFF\"/>\n"
                    "  <path d=\"M0 0h4v100h-4ZM6 0h2v100h-2ZM12 0h2v100h-2ZM22 0h2v100h-2ZM26 0h2v100h-2ZM34 0h4v100h-4ZM44 0h2v100h-2ZM52 0h2v100h-2ZM56 0h4v100h-4ZM66 0h2v100h-2ZM74 0h2v100h-2ZM82 0h4v100h-4ZM88 0h2v100h-2ZM92 0h4v100h-4ZM102 0h2v100h-2ZM110 0h2v100h-2ZM118 0h4v100h-4ZM124 0h2v100h-2ZM132 0h2v100h-2ZM140 0h4v100h-4ZM150 0h2v100h-2ZM154 0h2v100h-2ZM158 0h4v100h-4ZM168 0h6v100h-6ZM176 0h4v100h-4ZM186 0h6v100h-6ZM194 0h2v100h-2ZM198 0h4v100h-4Z\"/>\n"
                    "  <text x=\"101\" y=\"113.34\" text-anchor=\"middle\" font-family=\"Arimo, Arial, sans-serif\" font-size=\"14\">\n"
                    "   ABCDEF\n"
                    "  </text>\n"
                    " </g>\n"
                    "</svg>\n"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_svg", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, data[i].length, debug);
        strcpy(symbol->outfile, data[i].outfile);

        ret = ZBarcode_Encode_and_Print(symbol, TU(data[i].data), length, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_and_Print(%d) ret %d != %d (%s)\n",
                        i, data[i].symbology, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            const int expected_size = (int) strlen(data[i].expected);

            assert_nonnull(symbol->memfile, "i:%d memfile NULL (%s)\n", i, symbol->errtxt);

            assert_equal(symbol->memfile_size, expected_size, "i:%d memfile_size %d != %d (%s)\n",
                            i, symbol->memfile_size, expected_size, symbol->errtxt);
            ret = memcmp(symbol->memfile, data[i].expected, symbol->memfile_size);
            assert_zero(ret, "i:%d memcmp() %d != 0\n", i, ret);
        } else {
            assert_null(symbol->memfile, "i:%d memfile != NULL (%s)\n", i, symbol->errtxt);
            assert_zero(symbol->memfile_size, "i:%d memfile_size != 0 (%s)\n", i, symbol->errtxt);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#if defined(_WIN32) || (defined(__sun) && defined(__SVR4))
#define ZINT_TEST_NO_FMEMOPEN
#endif

#ifndef ZINT_TEST_NO_FMEMOPEN
extern FILE *fmemopen(void *buf, size_t size, const char *mode);
#endif

static void test_putsf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *prefix;
        int dp;
        float arg;
        const char *locale;
        const char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", 2, 1234.123, "", "1234.12" },
        /*  1*/ { "", 3, 1234.123, "", "1234.123" },
        /*  2*/ { "prefix ", 4, 1234.123, "", "prefix 1234.123" },
        /*  3*/ { "", 2, -1234.126, "", "-1234.13" },
        /*  4*/ { "", 2, 1234.1, "", "1234.1" },
        /*  5*/ { "", 3, 1234.1, "", "1234.1" },
        /*  6*/ { "", 4, 1234.1, "", "1234.1" },
        /*  7*/ { "", 2, 1234.0, "", "1234" },
        /*  8*/ { "", 2, -1234.0, "", "-1234" },
        /*  9*/ { "", 3, 1234.1234, "de_DE.UTF-8", "1234.123" },
        /* 10*/ { "", 4, -1234.1234, "de_DE.UTF-8", "-1234.1234" },
        /* 11*/ { "prefix ", 4, -1234.1234, "de_DE.UTF-8", "prefix -1234.1234" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, j;

    struct zint_symbol symbol_data = {0};
    struct zint_symbol *const symbol = &symbol_data;
    struct filemem fm;
    struct filemem *const fmp = &fm;
#ifndef ZINT_TEST_NO_FMEMOPEN
    FILE *fp;
    char buf[512] = {0}; /* Suppress clang-16/17 run-time exception MemorySanitizer: use-of-uninitialized-value */
#endif

    testStart("test_putsf");

    for (j = 0; j < 2; j++) { /* 1st `memfile`, then file */
#ifdef ZINT_TEST_NO_FMEMOPEN
        if (j == 1) break; /* Skip file test on Windows/Solaris */
#endif
        for (i = 0; i < data_size; i++) {
            const char *locale = NULL;
            int expected_size;

            if (testContinue(p_ctx, i)) continue;

            ZBarcode_Reset(symbol);
            if (j == 1) {
#ifndef ZINT_TEST_NO_FMEMOPEN
                buf[0] = '\0';
                fp = fmemopen(buf, sizeof(buf), "w");
                assert_nonnull(fp, "%d: fmemopen fail (%d, %s)\n", i, errno, strerror(errno));
#endif
            } else {
                symbol->output_options |= BARCODE_MEMORY_FILE;
            }
            assert_nonzero(fm_open(fmp, symbol, "w"), "i:%d: fm_open fail (%d, %s)\n", i, fmp->err, strerror(fmp->err));
            if (j == 1) {
#ifndef ZINT_TEST_NO_FMEMOPEN
                /* Hack in `fmemopen()` fp */
                assert_zero(fclose(fmp->fp), "i:%d fclose(fmp->fp) fail (%d, %s)\n", i, errno, strerror(errno));
                fmp->fp = fp;
#endif
            }

            if (data[i].locale && data[i].locale[0]) {
                locale = setlocale(LC_ALL, data[i].locale);
                if (!locale) { /* May not be available - warn unless quiet mode */
                    if (!(debug & ZINT_DEBUG_TEST_LESS_NOISY)) {
                        printf("i:%d: Warning: locale \"%s\" not available\n", i, data[i].locale);
                    }
                }
            }

            fm_putsf(data[i].prefix, data[i].dp, data[i].arg, fmp);

            assert_nonzero(fm_close(fmp, symbol), "i:%d: fm_close fail (%d, %s)\n", i, fmp->err, strerror(fmp->err));

            if (locale) {
                assert_nonnull(setlocale(LC_ALL, locale), "i:%d: setlocale(%s) restore fail (%d, %s)\n",
                    i, locale, errno, strerror(errno));
            }

            if (j == 1) {
#ifndef ZINT_TEST_NO_FMEMOPEN
                assert_zero(strcmp(buf, data[i].expected), "%d: strcmp(%s, %s) != 0\n", i, buf, data[i].expected);
#endif
            } else {
                expected_size = (int) strlen(data[i].expected);
                assert_equal(symbol->memfile_size, expected_size, "i:%d: memfile_size %d != expected_size %d\n",
                            i, symbol->memfile_size, expected_size);
                assert_nonnull(symbol->memfile, "i:%d memfile NULL\n", i);
                assert_zero(memcmp(symbol->memfile, data[i].expected, expected_size), "i:%d: memcmp(%.*s, %.*s) != 0\n",
                            i, symbol->memfile_size, symbol->memfile, expected_size, data[i].expected);
            }

            ZBarcode_Clear(symbol);
        }
    }

    testFinish();
}

static void test_printf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    int ret;
    int j;
    struct zint_symbol symbol_data = {0};
    struct zint_symbol *const symbol = &symbol_data;
    struct filemem fm;
    struct filemem *const fmp = &fm;
    const char outfile[] = "test_printf.tst";
    unsigned char filebuf[32768];
    int filebuf_size;

    const char fmt1[] = "\n%s%04d\n\032\nwow\n\r\n%.2s\n"; /* '\032' SUB (^Z) */
    const char expected1[] = "\ngosh0123\n\032\nwow\n\r\nge\n";
#ifdef _WIN32
    /* On Windows, non-binary (i.e. text) files, LF -> LF+CR (note, actual files only, not memfiles) */
    const char expected1_text_file[] = "\r\ngosh0123\r\n\032\r\nwow\r\n\r\r\nge\r\n";
#endif
    const char *expected;
    int expected_size;

    (void)debug;

    testStart("test_printf");

    for (j = 0; j < 2; j++) { /* 1st memfile, then file */
        ZBarcode_Reset(symbol);

        /* Binary */
        expected = expected1;
        if (j == 1) {
            strcpy(symbol->outfile, outfile);
        } else {
            symbol->output_options |= BARCODE_MEMORY_FILE;
        }
        ret = fm_open(fmp, symbol, "wb");
        assert_equal(ret, 1, "fm_open ret %d != 1\n", ret);

        ret = fm_printf(fmp, fmt1, "gosh", 123, "gee");
        assert_equal(ret, 1, "fm_printf ret %d != 1\n", ret);

        ret = fm_close(fmp, symbol);
        assert_equal(ret, 1, "fm_close ret %d != 1\n", ret);

        expected_size = (int) strlen(expected);

        if (j == 1) {
            ret = testUtilReadFile(symbol->outfile, filebuf, sizeof(filebuf), &filebuf_size);
            assert_zero(ret, "testUtilReadFile(%s) %d != 0\n", symbol->outfile, ret);
            assert_equal((int) filebuf_size, expected_size, "filebuf_size %d != %d\n", filebuf_size, expected_size);
            assert_zero(memcmp(filebuf, expected, filebuf_size), "memcmp(%.*s, %s) != 0\n",
                        filebuf_size, filebuf, expected);
            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                assert_zero(testUtilRemove(symbol->outfile), "testUtilRemove(%s) != 0\n", symbol->outfile);
            }
        } else {
            assert_nonnull(symbol->memfile, "memfile NULL (%d: %s)\n", fmp->err, strerror(fmp->err));
            assert_equal(symbol->memfile_size, expected_size, "mempos %d != %d\n",
                        symbol->memfile_size, expected_size);
            assert_zero(memcmp(symbol->memfile, expected, symbol->memfile_size), "memcmp(%.*s, %s) != 0\n",
                        symbol->memfile_size, symbol->memfile, expected);
        }

        /* Non-binary */
        expected = expected1;
        if (j == 1) {
            strcpy(symbol->outfile, outfile);
#ifdef _WIN32
            expected = expected1_text_file;
#endif
        } else {
            symbol->output_options |= BARCODE_MEMORY_FILE;
        }
        ret = fm_open(fmp, symbol, "w");
        assert_equal(ret, 1, "fm_open ret %d != 1\n", ret);

        ret = fm_printf(fmp, fmt1, "gosh", 123, "gee");
        assert_equal(ret, 1, "fm_printf ret %d != 1\n", ret);

        ret = fm_close(fmp, symbol);
        assert_equal(ret, 1, "fm_close ret %d != 1\n", ret);

        expected_size = (int) strlen(expected);

        if (j == 1) {
            ret = testUtilReadFile(symbol->outfile, filebuf, sizeof(filebuf), &filebuf_size);
            assert_zero(ret, "testUtilReadFile(%s) %d != 0\n", symbol->outfile, ret);
            assert_equal((int) filebuf_size, expected_size, "filebuf_size %d != %d\n", filebuf_size, expected_size);
            assert_zero(memcmp(filebuf, expected, filebuf_size), "memcmp(%.*s, %s) != 0\n",
                        filebuf_size, filebuf, expected);
            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                assert_zero(testUtilRemove(symbol->outfile), "testUtilRemove(%s) != 0\n", symbol->outfile);
            }
        } else {
            assert_nonnull(symbol->memfile, "mem NULL (%d: %s)\n", fmp->err, strerror(fmp->err));
            assert_equal(symbol->memfile_size, expected_size, "mempos %d != %d\n",
                        symbol->memfile_size, expected_size);
            assert_zero(memcmp(symbol->memfile, expected, symbol->memfile_size), "memcmp(%.*s, %s) != 0\n",
                        symbol->memfile_size, symbol->memfile, expected);
        }

        ZBarcode_Clear(symbol);
    }

    testFinish();
}

static void test_seek(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    int ret;
    int j;
    struct zint_symbol symbol_data = {0};
    struct zint_symbol *const symbol = &symbol_data;
    struct filemem fm;
    struct filemem *const fmp = &fm;
    const char outfile[] = "test_seek.tst";

    (void)debug;

    testStart("test_seek");

    for (j = 0; j < 2; j++) { /* 1st memfile, then file */
        ZBarcode_Reset(symbol);

        if (j == 1) {
            strcpy(symbol->outfile, outfile);
        } else {
            symbol->output_options |= BARCODE_MEMORY_FILE;
        }
        ret = fm_open(fmp, symbol, "wb");
        assert_equal(ret, 1, "j:%d fm_open ret %d != 1\n", j, ret);

        ret = fm_puts("1234567890", fmp);
        assert_equal(ret, 1, "j:%d fm_puts ret %d != 1\n", j, ret);
        if (j != 1) {
            assert_nonnull(fmp->mem, "mem NULL (%d: %s)\n", fmp->err, strerror(fmp->err));
            assert_equal(fmp->mempos, 10, "mempos %d != 10\n", (int) fmp->mempos);
            assert_zero(memcmp(fmp->mem, "1234567890", fmp->mempos), "memcmp fail\n");
        }

        ret = fm_seek(fmp, -10, SEEK_CUR);
        assert_equal(ret, 1, "j:%d fm_seek ret %d != 1 (%d: %s)\n", j, ret, fmp->err, strerror(fmp->err));
        ret = fm_error(fmp);
        assert_zero(ret, "j:%d fm_error ret %d != 0\n", j, ret);
        ret = (int) fm_tell(fmp);
        assert_zero(ret, "j:%d fm_tell ret %d != 0\n", j, ret);

        ret = fm_seek(fmp, 0, SEEK_END);
        assert_equal(ret, 1, "j:%d fm_seek ret %d != 1\n", j, ret);
        ret = fm_error(fmp);
        assert_zero(ret, "j:%d fm_error ret %d != 0\n", j, ret);
        ret = (int) fm_tell(fmp);
        assert_equal(ret, 10, "j:%d fm_tell ret %d != 10\n", j, ret);

        ret = fm_seek(fmp, -1, SEEK_SET);
        assert_zero(ret, "j:%d fm_seek ret %d != 1\n", j, ret);
        assert_equal(fmp->err, EINVAL, "j:%d fmp->err %d (%s) != EINVAL\n", j, fmp->err, strerror(fmp->err));

        ret = fm_close(fmp, symbol);
        assert_zero(ret, "j:%d fm_close ret %d != 0\n", j, ret);
        assert_equal(fmp->err, EINVAL, "j:%d fmp->err %d (%s) != EINVAL\n", j, fmp->err, strerror(fmp->err));

        if (j == 1) {
            assert_zero(testUtilRemove(symbol->outfile), "testUtilRemove(%s) != 0\n", symbol->outfile);
        }

        ret = fm_open(fmp, symbol, "wb");
        assert_equal(ret, 1, "j:%d fm_open ret %d != 1\n", j, ret);

        ret = fm_seek(fmp, LONG_MAX, SEEK_CUR);
        if (j == 1) { /* May work on some file systems */
            if (ret == 0) {
                assert_equal(fmp->err, EINVAL, "j:%d fmp->err %d (%s) != EINVAL\n", j, fmp->err, strerror(fmp->err));
            }
        } else {
            assert_zero(ret, "j:%d fm_seek ret %d != 0\n", j, ret);
            assert_equal(fmp->err, EINVAL, "j:%d fmp->err %d (%s) != EINVAL\n", j, fmp->err, strerror(fmp->err));
        }

        ret = fm_close(fmp, symbol);
        if (j == 1) { /* See above */
            if (ret == 0) {
                assert_equal(fmp->err, EINVAL, "j:%d fmp->err %d (%s) != EINVAL\n", j, fmp->err, strerror(fmp->err));
            }
        } else {
            assert_zero(ret, "j:%d fm_close ret %d != 0\n", j, ret);
            assert_equal(fmp->err, EINVAL, "j:%d fmp->err %d (%s) != EINVAL\n", j, fmp->err, strerror(fmp->err));
        }

        if (j == 1) {
            assert_zero(testUtilRemove(symbol->outfile), "testUtilRemove(%s) != 0\n", symbol->outfile);
        }

        ZBarcode_Clear(symbol);
    }

    testFinish();
}

static void test_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    int ret;
    struct zint_symbol *symbol = NULL;
    char data[] = "1";
    int expected_size = 354879;

    (void)debug;

    testStart("test_large");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    symbol->symbology = BARCODE_HANXIN;
    symbol->output_options |= BARCODE_MEMORY_FILE;
    strcpy(symbol->outfile, "out.gif"); /* Use GIF in case ZINT_NO_PNG */
    symbol->option_2 = 84;
    symbol->scale = 10.0f; /* Could go up to 86.5 (pixel buffer 0x3FB913B1, file size 8868579) but very very slow */

    ret = ZBarcode_Encode_and_Print(symbol, TU(data), -1, 0);
    assert_zero(ret, "ZBarcode_Encode_and_Print ret %d != 0 (%s)\n", ret, symbol->errtxt);
    assert_nonnull(symbol->memfile, "memfile NULL (%s)\n", symbol->errtxt);
    assert_equal(symbol->memfile_size, expected_size, "memfile_size %d != expected %d\n",
                    symbol->memfile_size, expected_size);

    symbol->scale = 87.0f; /* Too large (pixel buffer > 1GB) */
    ret = ZBarcode_Print(symbol, 0);
    assert_equal(ret, ZINT_ERROR_MEMORY, "ZBarcode_Print ret %d != ZINT_ERROR_MEMORY (%s)\n", ret, symbol->errtxt);

    ZBarcode_Delete(symbol);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_svg", test_svg },
        { "test_putsf", test_putsf },
        { "test_printf", test_printf },
        { "test_seek", test_seek },
        { "test_large", test_large },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */

