/*
    libzint - the open source barcode library
    Copyright (C) 2020-2026 Robin Stuart <rstuart114@gmail.com>

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
        float scale;
        const char *data;
        const char *expected_file;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_GRIDMATRIX, -1, -1, -1, -1, -1, -1, "C3C3C3", "", 0.75, "Grid Matrix", "gridmatrix_fg_0.75.pcx" },
        /*  1*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, -1, 20, "FFFFFF", "000000", 0, "1234567890123456789012345678901234567890", "codeblockf_reverse.pcx" },
        /*  2*/ { BARCODE_QRCODE, -1, -1, -1, -1, 2, 1, "", "D2E3F4", 0, "1234567890", "qr_bg.pcx" },
        /*  3*/ { BARCODE_ULTRA, 1, BARCODE_BOX, 1, 1, -1, -1, "FF0000", "0000FF", 0, "ULTRACODE_123456789!", "ultra_fg_bg_hvwsp1_box1.pcx" },
        /*  4*/ { BARCODE_CODE11, -1, -1, -1, -1, -1, -1, "12345678", "FEDCBA98", 0, "123", "code11_fgbgtrans.pcx" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *data_dir = "/backend/tests/data/pcx";
    const char *pcx = "out.pcx";
    char expected_file[4096];
    char escaped[1024];
    int escaped_size = 1024;
    unsigned char filebuf[36864];
    int filebuf_size;

    const char *const have_identify = testUtilHaveIdentify();

    testStartSymbol(p_ctx->func_name, &symbol);

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
        if (data[i].scale != 0) {
            symbol->scale = data[i].scale;
        }
        symbol->debug |= debug;

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, pcx);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file),
                    "i:%d testUtilDataPath == 0\n", i);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %s, %d, %d, %d, %d, \"%s\", \"%s\", %.5g, \"%s\", \"%s\"},\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].border_width,
                    testUtilOutputOptionsName(data[i].output_options),
                    data[i].whitespace_width, data[i].whitespace_height,
                    data[i].option_1, data[i].option_2, data[i].fgcolour, data[i].bgcolour, data[i].scale,
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
                        "i:%d %s memcmp(memfile, filebuf) != 0\n",
                        i, testUtilBarcodeName(data[i].symbology));
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

INTERNAL int zint_pcx_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

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

    strcpy(symbol.outfile, "test_pcx_out.pcx");
#ifndef _WIN32
    skip_readonly_test = getuid() == 0; /* Skip if running as root on Unix as can't create read-only file */
#endif
    if (!skip_readonly_test) {
        /* Excluding OS-dependent `errno` stuff */
        static char expected_errtxt[] = "621: Could not open PCX output file (";

        (void) testUtilRmROFile(symbol.outfile); /* In case lying around from previous fail */
        assert_nonzero(testUtilCreateROFile(symbol.outfile),
                    "zint_pcx_pixel_plot testUtilCreateROFile(%s) fail (%d: %s)\n",
                    symbol.outfile, errno, strerror(errno));

        ret = zint_pcx_pixel_plot(&symbol, data);
        assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "zint_pcx_pixel_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n",
                    ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);
        assert_zero(testUtilRmROFile(symbol.outfile), "zint_pcx_pixel_plot testUtilRmROFile(%s) != 0 (%d: %s)\n",
                    symbol.outfile, errno, strerror(errno));
        assert_zero(strncmp(symbol.errtxt, expected_errtxt, sizeof(expected_errtxt) - 1), "strncmp(%s, %s) != 0\n",
                    symbol.errtxt, expected_errtxt);
    }

    symbol.output_options |= BARCODE_STDOUT;

    ret = zint_pcx_pixel_plot(&symbol, data);
    printf(" - ignore (PCX to stdout)\n"); fflush(stdout);
    assert_zero(ret, "zint_pcx_pixel_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    testFinish();
}

#include "filemem.h"

static void test_fm(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        const char *data;
        int ret;
        int ats[5];
        int at_cnt;
        int id;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, "123", ZINT_ERROR_FILE_ACCESS, { 1, 0, 0, 0, 0 }, 1, FM_FAIL_ID_OPEN },
        /*  1*/ { BARCODE_DATAMATRIX, -1, "123", ZINT_ERROR_FILE_WRITE, { 1, 0, 0, 0, 0 }, 1, FM_FAIL_ID_WRITE },
        /*  3*/ { BARCODE_DATAMATRIX, -1, "123", ZINT_ERROR_FILE_WRITE, { 1, 2, 40, 200, 0 }, 4, FM_FAIL_ID_PUTC },
        /*  5*/ { BARCODE_DATAMATRIX, -1, "123", ZINT_ERROR_FILE_WRITE, { 1, 0, 0, 0, 0 }, 1, FM_FAIL_ID_CLOSE },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        for (j = 0; j < data[i].at_cnt; j++) {

            length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                                        -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                                        data[i].data, -1, debug);
            strcpy(symbol->outfile, "out.pcx");
            ret = ZBarcode_Encode(symbol, ZCUCP(data[i].data), length);
            assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n",
                        i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

            zint_test_fm_set_fail(data[i].id, data[i].ats[j]);
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            assert_equal(ret, data[i].ret, "i:%d j:%d ZBarcode_Print (%d,%d) ret %d != %d (%s)\n",
                            i, j, data[i].id, data[i].ats[j], ret, data[i].ret, symbol->errtxt);
            ZBarcode_Reset(symbol);
        }
        zint_test_fm_set_fail(0, 0);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_print", test_print },
        { "test_outfile", test_outfile },
        { "test_fm", test_fm },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
