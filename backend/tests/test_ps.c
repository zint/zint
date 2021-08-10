/*
    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

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
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"
#include <sys/stat.h>

static void test_print(int index, int generate, int debug) {

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
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 0, "Égjpqy", "code128_egrave_bold.eps" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 90, "Égjpqy", "code128_egrave_bold_rotate_90.eps" },
        /*  2*/ { BARCODE_CODE39, -1, -1, -1, -1, -1, -1, -1, 0, 0, "147AD0", "FC9630", 0, "123", "code39_fg_bg.eps" },
        /*  3*/ { BARCODE_CODE39, -1, -1, CMYK_COLOUR, -1, -1, -1, -1, 0, 0, "147AD0EE", "FC9630", 0, "123", "code39_fgalpha_bg_cmyk.eps" },
        /*  4*/ { BARCODE_ULTRA, -1, -1, -1, -1, -1, -1, -1, 0, 0, "147AD0", "FC9630", 0, "123", "ultra_fg_bg.eps" },
        /*  5*/ { BARCODE_ULTRA, -1, 1, BARCODE_BOX, 2, -1, -1, -1, 0, 0, "0000FF", "FF0000", 0, "123", "ultra_fg_bg_box.eps" },
        /*  6*/ { BARCODE_ULTRA, -1, 2, BARCODE_BOX | CMYK_COLOUR, 1, 1, -1, -1, 0, 0, "0000FF", "FF0000", 0, "123", "ultra_fg_bg_box_cmyk.eps" },
        /*  7*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "9771384524017+12", "ean13_2addon_ggs_5.2.2.5.1-2.eps" },
        /*  8*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "012345678905+24", "upca_2addon_ggs_5.2.6.6-5.eps" },
        /*  9*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "0123456+12345", "upce_5addon.eps" },
        /* 10*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT | BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 0, "0123456+12345", "upce_5addon_small_bold.eps" },
        /* 11*/ { BARCODE_CODE128, UNICODE_MODE, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "A\\B)ç(D", "code128_escape_latin1.eps" },
        /* 12*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, -1, -1, -1, -1, 0, 0, "", "", 0, "1501234567890", "dbar_ltd_24724_fig7_bold.eps" },
        /* 13*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", 0, "12", "dotcode_1.0.eps" },
        /* 14*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0, 0.1, "", "", 0, "12", "dotcode_1.0_ds0.1.eps" },
        /* 15*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0, 1.1, "", "", 0, "12", "dotcode_1.0_ds1.1.eps" },
        /* 16*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 1.5, 0, "", "", 0, "12", "dotcode_1.5.eps" },
        /* 17*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 1.5, 0.4, "", "", 0, "12", "dotcode_1.5_ds0.4.eps" },
        /* 18*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 1.5, 1.1, "", "", 0, "12", "dotcode_1.5_ds1.1.eps" },
        /* 19*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 1.5, 2.1, "", "", 0, "12", "dotcode_1.5_ds2.1.eps" },
        /* 20*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 2, 0, "", "", 0, "12", "dotcode_2.0.eps" },
        /* 21*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 2, 0.9, "", "", 0, "12", "dotcode_2.0_ds0.9.eps" },
        /* 22*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 2, 1.1, "", "", 0, "12", "dotcode_2.0_ds1.1.eps" },
        /* 23*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3, 0, "", "", 0, "12", "dotcode_3.0.eps" },
        /* 24*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3, 0.4, "", "", 0, "12", "dotcode_3.0_ds0.4.eps" },
        /* 25*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3, 1.1, "", "", 0, "12", "dotcode_3.0_ds1.1.eps" },
        /* 26*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3.5, 0, "", "", 0, "12", "dotcode_3.5.eps" },
        /* 27*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3.5, 0.4, "", "", 0, "12", "dotcode_3.5_ds0.4.eps" },
        /* 28*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 3.5, 1.1, "", "", 0, "12", "dotcode_3.5_ds1.1.eps" },
        /* 29*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 5, 0, "", "", 0, "12", "dotcode_5.0.eps" },
        /* 30*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 5, 0.2, "", "", 0, "12", "dotcode_5.0_ds0.2.eps" },
        /* 31*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 5, 1.1, "", "", 0, "12", "dotcode_5.0_ds1.1.eps" },
        /* 32*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 5, 1.7, "", "", 0, "12", "dotcode_5.0_ds1.7.eps" },
        /* 33*/ { BARCODE_DOTCODE, -1, -1, -1, -1, -1, -1, -1, 0, 0, "FF0000", "0000FF00", 0, "12", "dotcode_no_bg.eps" },
        /* 34*/ { BARCODE_MAXICODE, -1, -1, CMYK_COLOUR, -1, -1, -1, -1, 0, 0, "", "", 270, "12", "maxicode_rotate_270_cmyk.eps" },
        /* 35*/ { BARCODE_MAXICODE, -1, -1, -1, 3, -1, -1, -1, 0, 0, "", "0000FF00", 180, "12", "maxicode_no_bg_hwsp3_rotate_180.eps" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    const char *data_dir = "/backend/tests/data/eps";
    const char *eps = "out.eps";
    char expected_file[1024];
    char escaped[1024];
    int escaped_size = 1024;

    int have_ghostscript = testUtilHaveGhostscript();

    testStart("test_print");

    if (generate) {
        char data_dir_path[1024];
        assert_nonzero(testUtilDataPath(data_dir_path, sizeof(data_dir_path), data_dir, NULL), "testUtilDataPath(%s) == 0\n", data_dir);
        if (!testUtilDirExists(data_dir_path)) {
            ret = testUtilMkDir(data_dir_path);
            assert_zero(ret, "testUtilMkDir(%s) ret %d != 0 (%d: %s)\n", data_dir_path, ret, errno, strerror(errno));
        }
    }

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

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

        if (generate) {
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
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

void ps_convert(const unsigned char *string, unsigned char *ps_string);

static void test_ps_convert(int index) {

    struct item {
        char *data;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { "1\\(é)2€3¿", "1\\\\\\(\351\\)23\277" },
    };
    int data_size = ARRAY_SIZE(data);
    int i;

    unsigned char converted[256];

    testStart("test_ps_convert");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        ps_convert((unsigned char *) data[i].data, converted);
        assert_zero(strcmp((char *) converted, data[i].expected), "i:%d ps_convert(%s) %s != %s\n", i, data[i].data, converted, data[i].expected);
    }

    testFinish();
}

INTERNAL int ps_plot(struct zint_symbol *symbol, int rotate_angle);

static void test_outfile(void) {
    int ret;
    struct zint_symbol symbol = {0};
    struct zint_vector vector = {0};

    testStart("test_outfile");

    symbol.symbology = BARCODE_CODE128;
    symbol.vector = &vector;

    strcpy(symbol.outfile, "nosuch_dir/out.eps");

    ret = ps_plot(&symbol, 0);
    assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "ps_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);

    symbol.output_options |= BARCODE_STDOUT;

    ret = ps_plot(&symbol, 0);
    printf(" - ignore (EPS to stdout)\n"); fflush(stdout);
    assert_zero(ret, "ps_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    symbol.vector = NULL;
    ret = ps_plot(&symbol, 0);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "ps_plot ret %d != ZINT_ERROR_INVALID_DATA (%d) (%s)\n", ret, ZINT_ERROR_INVALID_DATA, symbol.errtxt);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_print", test_print, 1, 1, 1 },
        { "test_ps_convert", test_ps_convert, 1, 0, 0 },
        { "test_outfile", test_outfile, 0, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
