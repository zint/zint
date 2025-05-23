/*
    libzint - the open source barcode library
    Copyright (C) 2020-2025 Robin Stuart <rstuart114@gmail.com>

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
#include <sys/stat.h>

INTERNAL int bmp_pixel_plot(struct zint_symbol *symbol, const unsigned char *pixelbuf);

static void test_pixel_plot(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int width;
        int height;
        const char *pattern;
        int repeat;
        int ret;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 1, 1, "1", 0, 0 },
        /*  1*/ { 2, 1, "11", 0, 0 },
        /*  2*/ { 2, 2, "10", 1, 0 },
        /*  3*/ { 3, 1, "101", 0, 0 },
        /*  4*/ { 3, 2, "101010", 0, 0 },
        /*  5*/ { 3, 3, "101010101", 0, 0 },
        /*  6*/ { 4, 1, "1001", 0, 0 },
        /*  7*/ { 4, 3, "1001", 1, 0 },
        /*  8*/ { 5, 1, "10101", 0, 0 },
        /*  9*/ { 8, 2, "CBMWKRYGGYRKWMBC", 0, 0 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;
    struct zint_symbol *symbol;

    const char *bmp = "out.bmp";

    char data_buf[8 * 2 + 1];

    const char *const have_identify = testUtilHaveIdentify();

    testStart(p_ctx->func_name);

    for (i = 0; i < data_size; i++) {
        int size;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, bmp);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->debug |= debug;

        size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d bmp_pixel_plot size %d < sizeof(data_buf) %d\n",
                    i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d bmp_pixel_plot size %d != strlen(data_buf) %d\n",
                    i, size, (int) strlen(data_buf));

        if (*data_buf > '9') {
            symbol->symbology = BARCODE_ULTRA;
        }

        symbol->bitmap = (unsigned char *) data_buf;

        ret = bmp_pixel_plot(symbol, TCU(data_buf));
        assert_equal(ret, data[i].ret, "i:%d bmp_pixel_plot ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            if (have_identify) {
                ret = testUtilVerifyIdentify(have_identify, symbol->outfile, debug);
                assert_zero(ret, "i:%d identify %s ret %d != 0\n", i, symbol->outfile, ret);
            }
            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);
            }
        } else {
            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                (void) testUtilRemove(symbol->outfile);
            }
        }

        symbol->bitmap = NULL;

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_print(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int border_width;
        int output_options;
        int whitespace_width;
        int whitespace_height;
        int option_1;
        int option_2;
        const char *fgcolour;
        const char *bgcolour;
        const char *data;
        const char *expected_file;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, 5, -1, -1, -1, "147AD0", "FC9630", "123", "pdf417_fg_bg.bmp" },
        /*  1*/ { BARCODE_ULTRA, -1, -1, 5, -1, -1, -1, "147AD0", "FC9630", "123", "ultracode_fg_bg.bmp" },
        /*  2*/ { BARCODE_ULTRA, 1, BARCODE_BOX, 1, 1, -1, -1, "147AD0", "FC9630", "123", "ultracode_fg_bg_hvwsp1_box1.bmp" },
        /*  3*/ { BARCODE_PDF417COMP, -1, -1, 2, 2, -1, -1, "", "", "123", "pdf417comp_hvwsp2.bmp" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    const char *data_dir = "/backend/tests/data/bmp";
    const char *bmp = "out.bmp";
    char expected_file[4096];
    char escaped[1024];
    int escaped_size = 1024;
    unsigned char filebuf[32768] = {0}; /* Suppress clang -fsanitize=memory false positive */
    int filebuf_size;

    const char *const have_identify = testUtilHaveIdentify();

    testStart(p_ctx->func_name);

    if (p_ctx->generate) {
        char data_dir_path[1024];
        assert_nonzero(testUtilDataPath(data_dir_path, sizeof(data_dir_path), data_dir, NULL),
                    "testUtilDataPath(%s) == 0\n", data_dir);
        if (!testUtilDirExists(data_dir_path)) {
            ret = testUtilMkDir(data_dir_path);
            assert_zero(ret, "testUtilMkDir(%s) ret %d != 0 (%d: %s)\n", data_dir_path, ret, errno, strerror(errno));
        }
    }

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                    data[i].option_1, data[i].option_2, -1 /*option_3*/, data[i].output_options,
                                    data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].whitespace_height != -1) {
            symbol->whitespace_height = data[i].whitespace_height;
        }
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, bmp);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file),
                    "i:%d testUtilDataPath == 0\n", i);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %s, %d, %d, %d, %d, \"%s\", \"%s\", \"%s\", \"%s\"},\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].border_width,
                    testUtilOutputOptionsName(data[i].output_options),
                    data[i].whitespace_width, data[i].whitespace_height,
                    data[i].option_1, data[i].option_2, data[i].fgcolour, data[i].bgcolour,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0 (%d: %s)\n",
                        i, symbol->outfile, expected_file, ret, errno, strerror(errno));
            if (have_identify) {
                ret = testUtilVerifyIdentify(have_identify, expected_file, debug);
                assert_zero(ret, "i:%d %s identify %s ret %d != 0\n",
                            i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(expected_file), "i:%d testUtilExists(%s) == 0\n", i, expected_file);

            ret = testUtilCmpBins(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);

            /* For BARCODE_MEMORY_FILE */
            ret = testUtilReadFile(symbol->outfile, filebuf, sizeof(filebuf), &filebuf_size);
            assert_zero(ret, "i:%d %s testUtilReadFile(%s) %d != 0\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);
            }

            symbol->output_options |= BARCODE_MEMORY_FILE;
            ret = ZBarcode_Print(symbol, 0);
            assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0 (%s)\n",
                            i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret, symbol->errtxt);
            assert_nonnull(symbol->memfile, "i:%d %s memfile NULL\n", i, testUtilBarcodeName(data[i].symbology));
            assert_equal(symbol->memfile_size, filebuf_size, "i:%d %s memfile_size %d != %d\n",
                            i, testUtilBarcodeName(data[i].symbology), symbol->memfile_size, filebuf_size);
            assert_zero(memcmp(symbol->memfile, filebuf, symbol->memfile_size),
                            "i:%d %s memcmp(memfile, filebuf) != 0\n", i, testUtilBarcodeName(data[i].symbology));
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_outfile(const testCtx *const p_ctx) {
    int ret;
    int skip_readonly_test = 0;
    struct zint_symbol symbol = {0};
    unsigned char data[] = { "1" };

    (void)p_ctx;

    testStart(p_ctx->func_name);

    symbol.symbology = BARCODE_CODE128;
    symbol.bitmap = data;
    symbol.bitmap_width = symbol.bitmap_height = 1;

    strcpy(symbol.outfile, "test_bmp_out.bmp");
#ifndef _WIN32
    skip_readonly_test = getuid() == 0; /* Skip if running as root on Unix as can't create read-only file */
#endif
    if (!skip_readonly_test) {
        /* Excluding OS-dependent `errno` stuff */
        static char expected_errtxt[] = "601: Could not open BMP output file (";

        (void) testUtilRmROFile(symbol.outfile); /* In case lying around from previous fail */
        assert_nonzero(testUtilCreateROFile(symbol.outfile),
                    "bmp_pixel_plot testUtilCreateROFile(%s) fail (%d: %s)\n",
                    symbol.outfile, errno, strerror(errno));

        ret = bmp_pixel_plot(&symbol, data);
        assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "bmp_pixel_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n",
                    ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);
        assert_zero(testUtilRmROFile(symbol.outfile), "bmp_pixel_plot testUtilRmROFile(%s) != 0 (%d: %s)\n",
                    symbol.outfile, errno, strerror(errno));
        assert_zero(strncmp(symbol.errtxt, expected_errtxt, sizeof(expected_errtxt) - 1), "strncmp(%s, %s) != 0\n",
                    symbol.errtxt, expected_errtxt);
    }

    symbol.output_options |= BARCODE_STDOUT;

    ret = bmp_pixel_plot(&symbol, data);
    printf(" - ignore (BMP to stdout)\n"); fflush(stdout);
    assert_zero(ret, "bmp_pixel_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_pixel_plot", test_pixel_plot },
        { "test_print", test_print },
        { "test_outfile", test_outfile },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
