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
#include <sys/stat.h>

static void test_print(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int border_width;
        int output_options;
        int whitespace_width;
        int whitespace_height;
        int option_1;
        int option_2;
        float scale;
        float dot_size;
        char *fgcolour;
        char *bgcolour;
        int rotate_angle;
        char *data;
        char *expected_file;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 0, "Égjpqy", "code128_egrave_bold.eps" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 90, "Égjpqy", "code128_egrave_bold_rotate_90.eps" },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 180, "Égjpqy", "code128_egrave_bold_rotate_180.eps" },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 270, "Égjpqy", "code128_egrave_bold_rotate_270.eps" },
        /*  4*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, -1, -1, 0, 0, "147AD0", "FC9630", 0, "123", "code39_fg_bg.eps" },
        /*  5*/ { BARCODE_CODE39, -1, -1, CMYK_COLOUR, -1, -1, -1, -1, 0, 0, "147AD0EE", "FC9630", 0, "123", "code39_fgalpha_bg_cmyk.eps" },
        /*  6*/ { BARCODE_CODE39, -1, -1, CMYK_COLOUR, -1, -1, -1, -1, 0, 0, "90,40,0,9", "FC963000", 0, "123", "code39_nobg_cmyk.eps" },
        /*  7*/ { BARCODE_CODE39, -1, -1, SMALL_TEXT, -1, -1, -1, -1, 0, 0, "", "", 0, "123", "code39_small.eps" },
        /*  8*/ { BARCODE_ULTRA, -1, -1, -1, -1, -1, -1, -1, 0, 0, "147AD0", "FC9630", 0, "123", "ultra_fg_bg.eps" },
        /*  9*/ { BARCODE_ULTRA, -1, 1, BARCODE_BOX, 2, -1, -1, -1, 0, 0, "0000FF", "FF0000", 0, "123", "ultra_fg_bg_box.eps" },
        /* 10*/ { BARCODE_ULTRA, -1, 2, BARCODE_BOX | CMYK_COLOUR, 1, 1, -1, -1, 0, 0, "0000FF", "FF0000", 0, "123", "ultra_fg_bg_box_cmyk.eps" },
        /* 11*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "9501101531000", "ean13_ggs_5.2.2.1-1.eps" },
        /* 12*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0, 0, "", "", 0, "9501101531000", "ean13_ggs_5.2.2.1-1_gws.eps" },
        /* 13*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "9771384524017+12", "ean13_2addon_ggs_5.2.2.5.1-2.eps" },
        /* 14*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0, 0, "", "", 0, "9771384524017+12", "ean13_2addon_ggs_5.2.2.5.1-2_gws.eps" },
        /* 15*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "9501234", "ean8_gss_5.2.2.2-1.eps" },
        /* 16*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0, 0, "", "", 0, "9501234", "ean8_gss_5.2.2.2-1_gws.eps" },
        /* 17*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, 3, -1, -1, -1, 0, 0, "", "", 0, "9501234", "ean8_gss_5.2.2.2-1_gws_wsw3.eps" },
        /* 18*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "98765", "ean5.eps" },
        /* 19*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0, 0, "", "", 0, "98765", "ean5_gws.eps" },
        /* 20*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5.eps" },
        /* 21*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0, 0, "", "", 0, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5_gws.eps" },
        /* 22*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, 1, 3, -1, -1, 0, 0, "", "", 0, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5_gws_wsw1h3.eps" },
        /* 23*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 90, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5_rotate_90.eps" },
        /* 24*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0, 0, "", "", 180, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5_gws_rotate_180.eps" },
        /* 25*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, 4, -1, -1, -1, 0, 0, "", "", 270, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5_gws_rotate_270_wsw4.eps" },
        /* 26*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "0123456+12345", "upce_5addon.eps" },
        /* 27*/ { BARCODE_UPCE, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0, 0, "", "", 0, "0123456+12345", "upce_5addon_gws.eps" },
        /* 28*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 270, "0123456+12345", "upce_5addon_rotate_270.eps" },
        /* 29*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT | BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 0, "0123456+12345", "upce_5addon_small_bold.eps" },
        /* 30*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT | BOLD_TEXT | EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0, 0, "", "", 0, "0123456+12345", "upce_5addon_small_bold_gws.eps" },
        /* 31*/ { BARCODE_CODE128, UNICODE_MODE, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "A\\B)ç(D", "code128_escape_latin1.eps" },
        /* 32*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 0, "1501234567890", "dbar_ltd_24724_fig7_bold.eps" },
        /* 33*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0.1, 0, "", "", 0, "12", "dotcode_0.1.eps" },
        /* 34*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0.08, 0, "", "", 0, "12", "dotcode_0.1.eps" },
        /* 35*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "12", "dotcode_1.0.eps" },
        /* 36*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0, 0.1, "", "", 0, "12", "dotcode_1.0_ds0.1.eps" },
        /* 37*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0, 1.1, "", "", 0, "12", "dotcode_1.0_ds1.1.eps" },
        /* 38*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 1.5, 0, "", "", 0, "12", "dotcode_1.5.eps" },
        /* 39*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 1.5, 0.4, "", "", 0, "12", "dotcode_1.5_ds0.4.eps" },
        /* 40*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 1.5, 1.1, "", "", 0, "12", "dotcode_1.5_ds1.1.eps" },
        /* 41*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 1.5, 2.1, "", "", 0, "12", "dotcode_1.5_ds2.1.eps" },
        /* 42*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 2, 0, "", "", 0, "12", "dotcode_2.0.eps" },
        /* 43*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 2, 0.9, "", "", 0, "12", "dotcode_2.0_ds0.9.eps" },
        /* 44*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 2, 1.1, "", "", 0, "12", "dotcode_2.0_ds1.1.eps" },
        /* 45*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3, 0, "", "", 0, "12", "dotcode_3.0.eps" },
        /* 46*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3, 0.4, "", "", 0, "12", "dotcode_3.0_ds0.4.eps" },
        /* 47*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3, 1.1, "", "", 0, "12", "dotcode_3.0_ds1.1.eps" },
        /* 48*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3.5, 0, "", "", 0, "12", "dotcode_3.5.eps" },
        /* 49*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3.5, 0.4, "", "", 0, "12", "dotcode_3.5_ds0.4.eps" },
        /* 50*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3.5, 1.1, "", "", 0, "12", "dotcode_3.5_ds1.1.eps" },
        /* 51*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 5, 0, "", "", 0, "12", "dotcode_5.0.eps" },
        /* 52*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 5, 0.2, "", "", 0, "12", "dotcode_5.0_ds0.2.eps" },
        /* 53*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 5, 1.1, "", "", 0, "12", "dotcode_5.0_ds1.1.eps" },
        /* 54*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 5, 1.7, "", "", 0, "12", "dotcode_5.0_ds1.7.eps" },
        /* 55*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0, 0, "FF0000", "0000FF00", 0, "12", "dotcode_no_bg.eps" },
        /* 56*/ { BARCODE_MAXICODE, -1, -1, CMYK_COLOUR, -1, -1, -1, -1, 0, 0, "", "", 270, "12", "maxicode_rotate_270_cmyk.eps" },
        /* 57*/ { BARCODE_MAXICODE, -1, -1, -1, 3, -1, -1, -1, 0, 0, "", "0000FF00", 180, "12", "maxicode_no_bg_hwsp3_rotate_180.eps" },
        /* 58*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, 2.4, 0, "", "", 90, "12", "maxicode_2.4_rotate_90.eps" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char data_dir[] = "/backend/tests/data/eps";
    const char eps[] = "out.eps";
    const char memfile[] = "mem.eps";
    char expected_file[1024];
    char escaped[1024];
    int escaped_size = 1024;

    int have_ghostscript = testUtilHaveGhostscript();

    testStartSymbol("test_print", &symbol);

    if (p_ctx->generate) {
        char data_dir_path[1024];
        assert_nonzero(testUtilDataPath(data_dir_path, sizeof(data_dir_path), data_dir, NULL), "testUtilDataPath(%s) == 0\n", data_dir);
        if (!testUtilDirExists(data_dir_path)) {
            ret = testUtilMkDir(data_dir_path);
            assert_zero(ret, "testUtilMkDir(%s) ret %d != 0 (%d: %s)\n", data_dir_path, ret, errno, strerror(errno));
        }
    }

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].whitespace_height != -1) {
            symbol->whitespace_height = data[i].whitespace_height;
        }
        if (data[i].scale) {
            symbol->scale = data[i].scale;
        }
        if (data[i].dot_size) {
            symbol->dot_size = data[i].dot_size;
        }
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, eps);
        ret = ZBarcode_Print(symbol, data[i].rotate_angle);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %d, %d, %d, %.5g, %.5g, \"%s\", \"%s\", %d, \"%s\", \"%s\"},\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].border_width,
                    testUtilOutputOptionsName(data[i].output_options), data[i].whitespace_width, data[i].whitespace_height, data[i].option_1, data[i].option_2,
                    data[i].scale, data[i].dot_size, data[i].fgcolour, data[i].bgcolour, data[i].rotate_angle,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            if (have_ghostscript) {
                ret = testUtilVerifyGhostscript(expected_file, debug);
                assert_zero(ret, "i:%d %s ghostscript %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(expected_file), "i:%d testUtilExists(%s) == 0\n", i, expected_file);

            ret = testUtilCmpEpss(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpEpss(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);

            symbol->output_options |= BARCODE_MEMORY_FILE;
            ret = ZBarcode_Print(symbol, data[i].rotate_angle);
            assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0 (%s)\n",
                            i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret, symbol->errtxt);
            assert_nonnull(symbol->memfile, "i:%d %s memfile NULL\n", i, testUtilBarcodeName(data[i].symbology));
            assert_nonzero(symbol->memfile_size, "i:%d %s memfile_size 0\n", i, testUtilBarcodeName(data[i].symbology));

            ret = testUtilWriteFile(memfile, symbol->memfile, symbol->memfile_size, "wb");
            assert_zero(ret, "%d: testUtilWriteFile(%s) fail ret %d != 0\n", i, memfile, ret);

            ret = testUtilCmpEpss(symbol->outfile, memfile);
            assert_zero(ret, "i:%d %s testUtilCmpEpss(%s, %s) %d != 0\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->outfile, memfile, ret);

            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);
                assert_zero(testUtilRemove(memfile), "i:%d testUtilRemove(%s) != 0\n", i, memfile);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

INTERNAL void ps_convert_test(const unsigned char *string, unsigned char *ps_string);

static void test_ps_convert(const testCtx *const p_ctx) {

    struct item {
        char *data;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { "1\\(é)2€3¿", "1\\\\\\(\351\\)23\277" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i;

    unsigned char converted[256];

    testStart("test_ps_convert");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        ps_convert_test((unsigned char *) data[i].data, converted);
        assert_zero(strcmp((char *) converted, data[i].expected), "i:%d ps_convert(%s) %s != %s\n", i, data[i].data, converted, data[i].expected);
    }

    testFinish();
}

INTERNAL int ps_plot(struct zint_symbol *symbol, int rotate_angle);

static void test_outfile(const testCtx *const p_ctx) {
    int ret;
    int skip_readonly_test = 0;
    struct zint_symbol symbol = {0};
    struct zint_vector vector = {0};

    (void)p_ctx;

    testStart("test_outfile");

    symbol.symbology = BARCODE_CODE128;
    symbol.vector = &vector;

    strcpy(symbol.outfile, "test_ps_out.eps");
#ifndef _WIN32
    skip_readonly_test = getuid() == 0; /* Skip if running as root on Unix as can't create read-only file */
#endif
    if (!skip_readonly_test) {
        static char expected_errtxt[] = "645: Could not open EPS output file ("; /* Excluding OS-dependent `errno` stuff */

        (void) testUtilRmROFile(symbol.outfile); /* In case lying around from previous fail */
        assert_nonzero(testUtilCreateROFile(symbol.outfile), "ps_plot testUtilCreateROFile(%s) fail (%d: %s)\n", symbol.outfile, errno, strerror(errno));

        ret = ps_plot(&symbol, 0);
        assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "ps_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);
        assert_zero(testUtilRmROFile(symbol.outfile), "ps_plot testUtilRmROFile(%s) != 0 (%d: %s)\n", symbol.outfile, errno, strerror(errno));
        assert_zero(strncmp(symbol.errtxt, expected_errtxt, sizeof(expected_errtxt) - 1), "strncmp(%s, %s) != 0\n", symbol.errtxt, expected_errtxt);
    }

    symbol.output_options |= BARCODE_STDOUT;

    printf(">>>Begin ignore (EPS to stdout)\n"); fflush(stdout);
    ret = ps_plot(&symbol, 0);
    printf("<<<End ignore (EPS to stdout)\n"); fflush(stdout);
    assert_zero(ret, "ps_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    symbol.vector = NULL;
    ret = ps_plot(&symbol, 0);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ps_plot ret %d != ZINT_ERROR_INVALID_DATA (%d) (%s)\n", ret, ZINT_ERROR_INVALID_DATA, symbol.errtxt);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_print", test_print },
        { "test_ps_convert", test_ps_convert },
        { "test_outfile", test_outfile },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
