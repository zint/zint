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

INTERNAL int png_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

static void test_pixel_plot(int index, int debug) {

    testStart("");

    if (!testUtilHaveIdentify()) {
        testSkip("ImageMagick identify not available");
        return;
    }

    int ret;
    struct item {
        int width;
        int height;
        char *pattern;
        int repeat;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 1, 1, "1", 0 },
        /*  1*/ { 2, 1, "11", 0 },
        /*  2*/ { 2, 2, "10", 1 },
        /*  3*/ { 3, 1, "101", 0 },
        /*  4*/ { 3, 2, "101010", 0 },
        /*  5*/ { 3, 3, "101010101", 0 },
        /*  6*/ { 8, 2, "CBMWKRYGGYRKWMBC", 0 },
    };
    int data_size = ARRAY_SIZE(data);

    char *png = "out.png";

    char data_buf[8 * 2 + 1];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, png);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->debug |= debug;

        int size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d png_pixel_plot size %d < sizeof(data_buf) %d\n", i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d png_pixel_plot size %d != strlen(data_buf) %d\n", i, size, (int) strlen(data_buf));

        symbol->bitmap = (unsigned char *) data_buf;

        ret = png_pixel_plot(symbol, (unsigned char *) data_buf);
        assert_zero(ret, "i:%d png_pixel_plot ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

        ret = testUtilVerifyIdentify(symbol->outfile, debug);
        assert_zero(ret, "i:%d identify %s ret %d != 0\n", i, symbol->outfile, ret);

        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

        symbol->bitmap = NULL;

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_print(int index, int generate, int debug) {

    testStart("");

    int have_identify = testUtilHaveIdentify();

    int ret;
    struct item {
        int symbology;
        int input_mode;
        int border_width;
        int output_options;
        int whitespace_width;
        int whitespace_height;
        int show_hrt;
        int option_1;
        int option_2;
        int height;
        float scale;
        char *fgcolour;
        char *bgcolour;
        char *data;
        char *composite;
        char *expected_file;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, -1, 0, 0, "", "", "Égjpqy", "", "code128_egrave_bold.png", "" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, 3, BOLD_TEXT | BARCODE_BOX, -1, -1, -1, -1, -1, 0, 0, "", "", "Égjpqy", "", "code128_egrave_bold_box3.png", "" },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, 2, BOLD_TEXT | BARCODE_BOX, 2, 2, -1, -1, -1, 0, 0, "", "", "Égjpqy", "", "code128_egrave_bold_hvwsp2_box2.png", "" },
        /*  3*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, -1, -1, 3, -1, 0, 0, "", "", "[00]030123456789012340", "[02]13012345678909[37]24[10]1234567ABCDEFG", "gs1_128_cc_fig12.png", "" },
        /*  4*/ { BARCODE_CODABLOCKF, -1, 3, -1, -1, -1, -1, 3, -1, 0, 0, "", "", "AAAAAAAAA", "", "codablockf_3rows.png", "" },
        /*  5*/ { BARCODE_CODABLOCKF, -1, -1, -1, 2, 2, -1, -1, -1, 0, 0, "", "", "AAAAAAAAA", "", "codablockf_hvwsp2.png", "" },
        /*  6*/ { BARCODE_CODABLOCKF, -1, 2, BARCODE_BOX, 2, 2, -1, -1, -1, 0, 0, "", "", "AAAAAAAAA", "", "codablockf_hvwsp2_box2.png", "" },
        /*  7*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "9771384524017+12", "", "ean13_2addon_ggs_5.2.2.5.1-2.png", "" },
        /*  8*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "9780877799306+54321", "", "ean13_5addon_ggs_5.2.2.5.2-2.png", "" },
        /*  9*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, 0, "", "", "123456789012+12", "[91]123456789012345678901", "ean13_cc_2addon_cca_4x4.png", "" },
        /* 10*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, 0, "", "", "123456789012+54321", "[91]1234567890", "ean13_cc_5addon_ccb_3x4.png", "" },
        /* 11*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, 0, "", "", "123456789012+54321", "[91]1234567890", "ean13_cc_5addon_ccb_3x4_notext.png", "" },
        /* 12*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "012345678905+24", "", "upca_2addon_ggs_5.2.6.6-5.png", "" },
        /* 13*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "614141234417+12345", "", "upca_5addon.png", "" },
        /* 14*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, 0, -1, -1, 0, 0, "", "", "614141234417+12345", "", "upca_5addon_notext.png", "" },
        /* 15*/ { BARCODE_UPCA, -1, 3, BARCODE_BIND, -1, -1, -1, -1, -1, 0, 0, "", "", "614141234417+12345", "", "upca_5addon_bind3.png", "" },
        /* 16*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, 0, "", "", "12345678901+12", "[91]123456789", "upca_cc_2addon_cca_3x4.png", "" },
        /* 17*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, 0, "", "", "12345678901+12121", "[91]1234567890123", "upca_cc_5addon_ccb_4x4.png", "" },
        /* 18*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, 0, "", "", "12345678901+12121", "[91]1234567890123", "upca_cc_5addon_ccb_4x4_notext.png", "" },
        /* 19*/ { BARCODE_UPCA_CC, -1, 3, BARCODE_BIND, -1, -1, -1, 2, -1, 0, 0, "", "", "12345678901+12121", "[91]1234567890123", "upca_cc_5addon_ccb_4x4_bind3.png", "" },
        /* 20*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "1234567+12", "", "upce_2addon.png", "" },
        /* 21*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "1234567+12345", "", "upce_5addon.png", "" },
        /* 22*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, 0, 0, "", "", "1234567+12345", "", "upce_5addon_small.png", "" },
        /* 23*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, 0, "", "", "0654321+89", "[91]1", "upce_cc_2addon_cca_5x2.png", "" },
        /* 24*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, 0, "", "", "1876543+56789", "[91]12345", "upce_cc_5addon_ccb_8x2.png", "" },
        /* 25*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, 0, "", "", "1876543+56789", "[91]12345", "upce_cc_5addon_ccb_8x2_notext.png", "" },
        /* 26*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "1234567+12", "", "ean8_2addon.png", "" },
        /* 27*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "1234567+12345", "", "ean8_5addon.png", "" },
        /* 28*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "9876543+65", "[91]1234567", "ean8_cc_2addon_cca_4x3.png", "" },
        /* 29*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "9876543+74083", "[91]123456789012345678", "ean8_cc_5addon_ccb_8x3.png", "" },
        /* 30*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "12345", "", "ean5.png", "" },
        /* 31*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "", "12", "", "ean2.png", "" },
        /* 32*/ { BARCODE_CODE39, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, 0, 0, "", "", "123", "", "code39_small.png", "" },
        /* 33*/ { BARCODE_POSTNET, -1, -1, -1, -1, -1, -1, -1, -1, 0, 3.5, "", "", "12345", "", "postnet_zip.png", "300 dpi, using 1/43in X, 300 / 43 / 2 = ~3.5 scale" },
        /* 34*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "", "CFCECDCC", "12345", "", "pdf417_bgalpha.png", "" },
        /* 35*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "30313233", "", "12345", "", "pdf417_fgalpha.png", "" },
        /* 36*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "20212244", "CFCECDCC", "12345", "", "pdf417_bgfgalpha.png", "" },
        /* 37*/ { BARCODE_ULTRA, -1, -1, -1, 2, -1, -1, -1, -1, 0, 0, "0000007F", "FF000033", "12345", "", "ultra_bgfgalpha.png", "" },
        /* 38*/ { BARCODE_ULTRA, -1, -1, -1, 2, -1, -1, -1, -1, 0, 0, "", "FF000033", "12345", "", "ultra_bgalpha.png", "" },
        /* 39*/ { BARCODE_ULTRA, -1, -1, -1, 2, -1, -1, -1, -1, 0, 0, "0000007F", "FF0000", "12345", "", "ultra_fgalpha.png", "" },
        /* 40*/ { BARCODE_ULTRA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, "0000007F", "", "12345", "", "ultra_fgalpha_nobg.png", "" },
        /* 41*/ { BARCODE_ULTRA, -1, 1, BARCODE_BOX, 1, 1, -1, -1, -1, 0, 0, "00FF007F", "BABDB6", "12345", "", "ultra_fgalpha_hvwsp1_box1.png", "" },
        /* 42*/ { BARCODE_ULTRA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0.5f, "", "", "1", "", "ultra_odd.png", "" },
        /* 43*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0.5f, "", "", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "maxicode_0.5.png", "6 dpmm, 150 dpi" },
        /* 44*/ { BARCODE_MAXICODE, -1, 1, BARCODE_BOX, 3, -1, -1, -1, -1, 0, 0.7f, "", "", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "maxicode_0.7_wsp3_box1.png", "8 dpmm, 200 dpi" },
        /* 45*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1.4f, "1111117F", "EEEEEEEE", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "maxicode_1.4_bgfgalpha.png", "16 dpmm, 400 dpi" },
        /* 46*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 2.1f, "", "", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "maxicode_2.1.png", "24 dpmm, 600 dpi" },
        /* 47*/ { BARCODE_MAXICODE, -1, 2, BARCODE_BOX, 1, 1, -1, -1, -1, 0, 0, "", "", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "maxicode_hvwsp1_box2.png", "" },
        /* 48*/ { BARCODE_MAXICODE, -1, 1, BARCODE_BIND, -1, 1, -1, -1, -1, 0, 0, "", "", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "maxicode_vwsp1_bind1.png", "" },
        /* 49*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, -1, -1, -1, -1, -1, 0, 2.0f, "", "", "1234", "", "datamatrix_2.0_bind1_dotty.png", "" },
        /* 50*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, 1, 1, -1, -1, -1, 0, 2.0f, "", "", "1234", "", "datamatrix_2.0_hvwsp1_bind1_dotty.png", "" },
    };
    int data_size = ARRAY_SIZE(data);

    const char *data_dir = "/backend/tests/data/png";
    const char *png = "out.png";
    char expected_file[1024];
    char escaped[1024];
    int escaped_size = 1024;
    char *text;

    if (generate) {
        if (!testUtilExists(data_dir)) {
            ret = testutil_mkdir(data_dir, 0755);
            assert_zero(ret, "testutil_mkdir(%s) ret %d != 0 (%d: %s)\n", data_dir, ret, errno, strerror(errno));
        }
    }

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        if (data[i].height) {
            symbol->height = data[i].height;
        }
        if (data[i].scale) {
            symbol->scale = data[i].scale;
        }
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
        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int text_length = strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, text_length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, png);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %d, %d, %d, %d, %d, %.5g, \"%s\",\"%s\",  \"%s\", \"%s\", \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].border_width, testUtilOutputOptionsName(data[i].output_options),
                    data[i].whitespace_width, data[i].whitespace_height, data[i].show_hrt, data[i].option_1, data[i].option_2, data[i].height, data[i].scale, data[i].fgcolour, data[i].bgcolour,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].composite, data[i].expected_file, data[i].comment);
            ret = rename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d rename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            if (have_identify) {
                ret = testUtilVerifyIdentify(expected_file, debug);
                assert_zero(ret, "i:%d %s identify %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(expected_file), "i:%d testUtilExists(%s) == 0\n", i, expected_file);

            ret = testUtilCmpPngs(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpPngs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            ret = testUtilCmpBins(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_pixel_plot", test_pixel_plot, 1, 0, 1 },
        { "test_print", test_print, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
