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
        float dpmm;
        char *fgcolour;
        char *bgcolour;
        int rotate_angle;
        char *data;
        char *expected_file;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "Égjpqy", "code128_egrave_bold.emf", "" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0.0f, 100.0f / 25.4f, "", "", 0, "Égjpqy", "code128_egrave_bold_100dpi.emf", "" },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0.0f, 150.0f / 25.4f, "", "", 0, "Égjpqy", "code128_egrave_bold_150dpi.emf", "" },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0.0f, 300.0f / 25.4f, "", "", 0, "Égjpqy", "code128_egrave_bold_300dpi.emf", "" },
        /*  4*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0.0f, 400.0f / 25.4f, "", "", 0, "Égjpqy", "code128_egrave_bold_400dpi.emf", "" },
        /*  5*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0.0f, 1200.0f / 25.4f, "", "", 0, "Égjpqy", "code128_egrave_bold_1200dpi.emf", "" },
        /*  6*/ { BARCODE_TELEPEN, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "147AD0", "FC9630", 0, "123", "telenum_fg_bg.emf", "" },
        /*  7*/ { BARCODE_TELEPEN, -1, -1, -1, -1, -1, -1, -1, 0.0f, 150.0f / 25.4f, "147AD0", "FC9630", 0, "123", "telenum_fg_bg_150dpi.emf", "" },
        /*  8*/ { BARCODE_ULTRA, -1, -1, -1, 5, -1, -1, -1, 0.0f, 0, "147AD0", "FC9630", 0, "123", "ultracode_fg_bg.emf", "" },
        /*  9*/ { BARCODE_ULTRA, -1, 2, BARCODE_BOX, 2, 2, -1, -1, 0.0f, 0, "FF0000", "0000FF", 0, "123", "ultracode_fg_bg_box2.emf", "" },
        /* 10*/ { BARCODE_ULTRA, -1, 2, BARCODE_BOX, 2, 2, -1, -1, 0.0f, 600.0f / 25.4f, "FF0000", "0000FF", 0, "123", "ultracode_fg_bg_box2_600dpi.emf", "" },
        /* 11*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "9501101531000", "ean13_ggs_5.2.2.1-1.emf", "" },
        /* 12*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "9501101531000", "ean13_ggs_5.2.2.1-1_gws.emf", "" },
        /* 13*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "9780877799306+54321", "ean13_5addon_ggs_5.2.2.5.2-2.emf", "" },
        /* 14*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "9780877799306+54321", "ean13_5addon_ggs_5.2.2.5.2-2_gws.emf", "" },
        /* 15*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "210987654321+54321", "ean13_5addon_#185.emf", "#185 Byte count, font data, HeaderExtension1/2" },
        /* 16*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "9501234", "ean8_gss_5.2.2.2-1.emf", "" },
        /* 17*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "9501234", "ean8_gss_5.2.2.2-1_gws.emf", "" },
        /* 18*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, 3, -1, -1, -1, 0.0f, 0, "", "", 0, "9501234", "ean8_gss_5.2.2.2-1_gws_wsw3.emf", "" },
        /* 19*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 2.5f, 0, "", "", 0, "9501234", "ean8_gss_5.2.2.2-1_gws_sc2_5.emf", "" },
        /* 20*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5.emf", "" },
        /* 21*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5_gws.emf", "" },
        /* 22*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, 1, 2, -1, -1, 0.0f, 0, "", "", 0, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5_gws_wsw1h2.emf", "" },
        /* 23*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "0123456+12", "upce_2addon.emf", "" },
        /* 24*/ { BARCODE_UPCE, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "0123456+12", "upce_2addon_gws.emf", "" },
        /* 25*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, 0.0f, 150.f / 25.4f, "", "", 0, "0123456+12", "upce_2addon_150dpi.emf", "" },
        /* 26*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT | BOLD_TEXT, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "0123456+12", "upce_2addon_small_bold.emf", "" },
        /* 27*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT | BOLD_TEXT | EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "0123456+12", "upce_2addon_small_bold_gws.emf", "" },
        /* 28*/ { BARCODE_ITF14, -1, -1, BOLD_TEXT, -1, -1, -1, -1, 0.0f, 0, "", "", 0, "123", "itf14_bold.emf", "" },
        /* 29*/ { BARCODE_ITF14, -1, -1, BOLD_TEXT, -1, -1, -1, -1, 0.0f, 600.f / 25.4f, "", "", 0, "123", "itf14_bold_600dpi.emf", "" },
        /* 30*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 90, "123", "code39_rotate_90.emf", "" },
        /* 31*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, -1, -1, 0.0f, 300.f / 25.4f, "", "", 90, "123", "code39_rotate_90_300dpi.emf", "" },
        /* 32*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 180, "123", "code39_rotate_180.emf", "" },
        /* 33*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "", 270, "123", "code39_rotate_270.emf", "" },
        /* 34*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "E0E0E0", "700070", 0, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "maxicode_#185.emf", "#185 Maxicode scaling" },
        /* 35*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, 0.0f, 150.f / 25.4f, "E0E0E0", "700070", 0, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "maxicode_#185_150dpi.emf", "#185 Maxicode scaling" },
        /* 36*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, 0.0f, 600.f / 25.4f, "E0E0E0", "700070", 0, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "maxicode_#185_600dpi.emf", "#185 Maxicode scaling" },
        /* 37*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, 0.0f, 0, "", "FFFFFF00", 90, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "maxicode_rotate_90_nobg.emf", "" },
        /* 38*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, 0.0f, 300.0f, "", "FFFFFF00", 90, "THIS IS A 93 CHARACTER CODE SET A MESSAGE THAT FILLS A MODE 4, UNAPPENDED, MAXICODE SYMBOL...", "maxicode_rotate_90_nobg_300dpi.emf", "" },
        /* 39*/ { BARCODE_UPU_S10, -1, -1, CMYK_COLOUR, -1, -1, -1, -1, 0.0f, 0, "71,0,40,44", "FFFFFF00", 0, "QA47312482PS", "upu_s10_cmyk_nobg.emf", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *data_dir = "/backend/tests/data/emf";
    const char *emf = "out.emf";
    char expected_file[1024];
    char escaped[1024];
    int escaped_size = 1024;
    unsigned char filebuf[32768];
    int filebuf_size;

    int have_libreoffice = 0;
    if (p_ctx->generate) {
        have_libreoffice = testUtilHaveLibreOffice();
    }

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
        if (data[i].dpmm) {
            symbol->dpmm = data[i].dpmm;
            symbol->scale = ZBarcode_Scale_From_XdimDp(symbol->symbology, ZBarcode_Default_Xdim(symbol->symbology), symbol->dpmm, "EMF");
        }
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, emf);
        ret = ZBarcode_Print(symbol, data[i].rotate_angle);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %d, %d, %d, %g, \"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].border_width,
                    testUtilOutputOptionsName(data[i].output_options), data[i].whitespace_width, data[i].whitespace_height,
                    data[i].option_1, data[i].option_2, data[i].dpmm, data[i].fgcolour, data[i].bgcolour, data[i].rotate_angle,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].expected_file, data[i].comment);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            if (have_libreoffice) {
                /* Note this will fail (on Ubuntu anyway) if LibreOffice Base/Calc/Impress/Writer running (i.e. anything but LibreOffice Draw)
                   Doesn't seem to be a way to force Draw invocation through the command line */
                ret = testUtilVerifyLibreOffice(expected_file, debug);
                assert_zero(ret, "i:%d %s libreoffice %s ret %d != 0 - check that LibreOffice is not running!\n",
                            i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(expected_file), "i:%d testUtilExists(%s) == 0\n", i, expected_file);

            ret = testUtilCmpBins(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);

            ret = testUtilReadFile(symbol->outfile, filebuf, sizeof(filebuf), &filebuf_size); /* For BARCODE_MEMORY_FILE */
            assert_zero(ret, "i:%d %s testUtilReadFile(%s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

            if (!(debug & ZINT_DEBUG_TEST_KEEP_OUTFILE)) {
                assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);
            }

            symbol->output_options |= BARCODE_MEMORY_FILE;
            ret = ZBarcode_Print(symbol, data[i].rotate_angle);
            assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0 (%s)\n",
                            i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret, symbol->errtxt);
            assert_nonnull(symbol->memfile, "i:%d %s memfile NULL\n", i, testUtilBarcodeName(data[i].symbology));
            assert_equal(symbol->memfile_size, filebuf_size, "i:%d %s memfile_size %d != %d\n",
                            i, testUtilBarcodeName(data[i].symbology), symbol->memfile_size, filebuf_size);
            assert_zero(memcmp(symbol->memfile, filebuf, symbol->memfile_size), "i:%d %s memcmp(memfile, filebuf) != 0\n",
                            i, testUtilBarcodeName(data[i].symbology));
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

INTERNAL int emf_plot(struct zint_symbol *symbol, int rotate_angle);

static void test_outfile(const testCtx *const p_ctx) {
    int ret;
    int skip_readonly_test = 0;
    struct zint_symbol symbol = {0};
    struct zint_vector vector = {0};

    (void)p_ctx;

    testStart("test_outfile");

    symbol.symbology = BARCODE_CODE128;
    symbol.vector = &vector;

    strcpy(symbol.outfile, "test_emf_out.emf");
#ifndef _WIN32
    skip_readonly_test = getuid() == 0; /* Skip if running as root on Unix as can't create read-only file */
#endif
    if (!skip_readonly_test) {
        static char expected_errtxt[] = "640: Could not open EMF output file ("; /* Excluding OS-dependent `errno` stuff */

        (void) testUtilRmROFile(symbol.outfile); /* In case lying around from previous fail */
        assert_nonzero(testUtilCreateROFile(symbol.outfile), "emf_plot testUtilCreateROFile(%s) fail (%d: %s)\n", symbol.outfile, errno, strerror(errno));

        ret = emf_plot(&symbol, 0);
        assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "emf_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);
        assert_zero(testUtilRmROFile(symbol.outfile), "emf_plot testUtilRmROFile(%s) != 0 (%d: %s)\n", symbol.outfile, errno, strerror(errno));
        assert_zero(strncmp(symbol.errtxt, expected_errtxt, sizeof(expected_errtxt) - 1), "strncmp(%s, %s) != 0\n", symbol.errtxt, expected_errtxt);
    }

    symbol.output_options |= BARCODE_STDOUT;

    printf("<<<Begin ignore (EMF to stdout)\n"); fflush(stdout);
    ret = emf_plot(&symbol, 0);
    printf("\n<<<End ignore (EMF to stdout)\n"); fflush(stdout);
    assert_zero(ret, "emf_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    symbol.vector = NULL;
    ret = emf_plot(&symbol, 0);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "emf_plot ret %d != ZINT_ERROR_INVALID_DATA (%d) (%s)\n", ret, ZINT_ERROR_INVALID_DATA, symbol.errtxt);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_print", test_print },
        { "test_outfile", test_outfile },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
