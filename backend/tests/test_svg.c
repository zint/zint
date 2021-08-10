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
        int show_hrt;
        int option_1;
        int option_2;
        float height;
        char *fgcolour;
        char *bgcolour;
        int rotate_angle;
        char *data;
        char *composite;
        int ret;
        char *expected_file;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "<>\"&'", "", 0, "code128_amperands.svg" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold.svg" },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, 3, BOLD_TEXT | BARCODE_BOX, -1, -1, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold_box3.svg" },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, 2, BOLD_TEXT | BARCODE_BOX, 2, 2, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold_hvwsp2_box2.svg" },
        /*  4*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, 3, 3, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold_hvwsp3.svg" },
        /*  5*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, -1, -1, 3, -1, 0, "", "", 0, "[00]030123456789012340", "[02]13012345678909[37]24[10]1234567ABCDEFG", 0, "gs1_128_cc_fig12.svg" },
        /*  6*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, -1, -1, 3, -1, 0, "", "", 0, "AAAAAAAAA", "", 0, "codablockf_3rows.svg" },
        /*  7*/ { BARCODE_CODABLOCKF, -1, -1, -1, 2, 2, -1, 3, -1, 0, "", "", 0, "AAAAAAAAA", "", 0, "codablockf_hvwsp2.svg" },
        /*  8*/ { BARCODE_CODABLOCKF, -1, 2, BARCODE_BOX, 2, 2, -1, -1, -1, 0, "", "", 0, "AAAAAAAAA", "", 0, "codablockf_hvwsp2_box2.svg" },
        /*  9*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9771384524017+12", "", 0, "ean13_2addon_ggs_5.2.2.5.1-2.svg" },
        /* 10*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9780877799306+54321", "", 0, "ean13_5addon_ggs_5.2.2.5.2-2.svg" },
        /* 11*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, "", "", 0, "123456789012+12", "[91]123456789012345678901", 0, "ean13_cc_2addon_cca_4x4.svg" },
        /* 12*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, "", "", 0, "123456789012+54321", "[91]1234567890", 0, "ean13_cc_5addon_ccb_3x4.svg" },
        /* 13*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, "", "", 0, "123456789012+54321", "[91]1234567890", 0, "ean13_cc_5addon_ccb_3x4_notext.svg" },
        /* 14*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "012345678905+24", "", 0, "upca_2addon_ggs_5.2.6.6-5.svg" },
        /* 15*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "614141234417+12345", "", 0, "upca_5addon.svg" },
        /* 16*/ { BARCODE_UPCA, -1, 3, BARCODE_BIND, -1, -1, -1, -1, -1, 0, "", "", 0, "614141234417+12345", "", 0, "upca_5addon_bind3.svg" },
        /* 17*/ { BARCODE_UPCA, -1, -1, SMALL_TEXT | BOLD_TEXT, -1, -1, -1, -1, -1, 0, "", "", 0, "614141234417+12345", "", 0, "upca_5addon_small_bold.svg" },
        /* 18*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, "", "", 0, "12345678901+12", "[91]123456789", 0, "upca_cc_2addon_cca_3x4.svg" },
        /* 19*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, "", "", 0, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4.svg" },
        /* 20*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, "", "", 0, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4_notext.svg" },
        /* 21*/ { BARCODE_UPCA_CC, -1, 3, BARCODE_BIND, -1, -1, -1, 2, -1, 0, "", "", 0, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4_bind3.svg" },
        /* 22*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12", "", 0, "upce_2addon.svg" },
        /* 23*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "upce_5addon.svg" },
        /* 24*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "upce_5addon_small.svg" },
        /* 25*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, 0, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "upce_5addon_notext.svg" },
        /* 26*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, "", "", 0, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2.svg" },
        /* 27*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, "FF0000EE", "0000FF11", 0, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2_fgbgalpha.svg" },
        /* 28*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, "", "FFFFFF00", 0, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2_nobg.svg" },
        /* 29*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, "", "", 270, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2_rotate_270.svg" },
        /* 30*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, "", "", 0, "1876543+56789", "[91]12345", 0, "upce_cc_5addon_ccb_8x2.svg" },
        /* 31*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, "", "", 0, "1876543+56789", "[91]12345", 0, "upce_cc_5addon_ccb_8x2_notext.svg" },
        /* 32*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12", "", 0, "ean8_2addon.svg" },
        /* 33*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "ean8_5addon.svg" },
        /* 34*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, "", "", 0, "9876543+65", "[91]1234567", 0, "ean8_cc_2addon_cca_4x3.svg" },
        /* 35*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, "", "", 0, "9876543+74083", "[91]123456789012345678", 0, "ean8_cc_5addon_ccb_8x3.svg" },
        /* 36*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12345", "", 0, "ean5.svg" },
        /* 37*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12", "", 0, "ean2.svg" },
        /* 38*/ { BARCODE_CODE39, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, 0, "", "", 0, "123", "", 0, "code39_small.svg" },
        /* 39*/ { BARCODE_POSTNET, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12345", "", 0, "postnet_zip.svg" },
        /* 40*/ { BARCODE_MAXICODE, -1, 2, BARCODE_BOX, -1, -1, -1, -1, -1, 0, "", "", 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_box2.svg" },
        /* 41*/ { BARCODE_MAXICODE, -1, 1, BARCODE_BIND, -1, 1, -1, -1, -1, 0, "", "", 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_vwsp1_bind1.svg" },
        /* 42*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, -1, 0, "121212DD", "EEEEEE22", 90, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_fgbg_rotate_90.svg" },
        /* 43*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, -1, 1, -1, -1, -1, 0, "", "", 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "datamatrix_vwsp1_bind1_dotty.svg" },
        /* 44*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, 1, 1, -1, -1, -1, 0, "", "", 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "datamatrix_hvwsp1_bind1_dotty.svg" },
        /* 45*/ { BARCODE_DBAR_LTD, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12345678909", "", 0, "dbar_ltd.svg" },
        /* 46*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, 5, "", "", 0, "Your Data Here!", "", ZINT_WARN_NONCOMPLIANT, "pdf417_height5.svg" },
        /* 47*/ { BARCODE_USPS_IMAIL, -1, -1, -1, -1, -1, -1, -1, -1, 7.75, "", "", 0, "12345678901234567890", "", 0, "imail_height7.75.svg" },
        /* 48*/ { BARCODE_ULTRA, -1, 3, BARCODE_BOX, 2, 2, -1, -1, -1, 0, "FF0000", "0000FF", 0, "12345678901234567890", "", 0, "ultra_fgbg_hvwsp2_box3.svg" },
        /* 49*/ { BARCODE_TELEPEN, -1, -1, -1, -1, -1, -1, -1, -1, 0.4, "", "", 180, "A", "", 0, "telepen_height0.4_rotate_180.svg" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    const char *data_dir = "/backend/tests/data/svg";
    const char *svg = "out.svg";
    char expected_file[1024];
    char escaped[1024];
    int escaped_size = 1024;
    char *text;

    int have_libreoffice = 0;
    int have_vnu = 0;

    testStart("test_print");

    if (generate) {
        char data_dir_path[1024];

        have_libreoffice = testUtilHaveLibreOffice();
        have_vnu = testUtilHaveVnu();

        assert_nonzero(testUtilDataPath(data_dir_path, sizeof(data_dir_path), data_dir, NULL), "testUtilDataPath(%s) == 0\n", data_dir);
        if (!testUtilDirExists(data_dir_path)) {
            ret = testUtilMkDir(data_dir_path);
            assert_zero(ret, "testUtilMkDir(%s) ret %d != 0 (%d: %s)\n", data_dir_path, ret, errno, strerror(errno));
        }
    }

    for (i = 0; i < data_size; i++) {
        int text_length;

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        if (data[i].height) {
            symbol->height = data[i].height;
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
        text_length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, text_length);
        assert_equal(ret, data[i].ret, "i:%d %s ZBarcode_Encode ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        strcpy(symbol->outfile, svg);
        ret = ZBarcode_Print(symbol, data[i].rotate_angle);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %d, %d, %d, %d, %.8g, \"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].border_width,
                    testUtilOutputOptionsName(data[i].output_options), data[i].whitespace_width, data[i].whitespace_height, data[i].show_hrt,
                    data[i].option_1, data[i].option_2, data[i].height, data[i].fgcolour, data[i].bgcolour, data[i].rotate_angle,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].composite, data[i].expected_file);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            if (have_libreoffice) {
                ret = testUtilVerifyLibreOffice(expected_file, debug);
                assert_zero(ret, "i:%d %s libreoffice %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
            if (have_vnu) {
                ret = testUtilVerifyVnu(expected_file, debug); // Very slow
                assert_zero(ret, "i:%d %s vnu libreoffice %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(expected_file), "i:%d testUtilExists(%s) == 0\n", i, expected_file);

            ret = testUtilCmpSvgs(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpSvgs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

INTERNAL int svg_plot(struct zint_symbol *symbol, int rotate_angle);

static void test_outfile(void) {
    int ret;
    struct zint_symbol symbol = {0};
    struct zint_vector vector = {0};

    testStart("test_outfile");

    symbol.symbology = BARCODE_CODE128;
    symbol.vector = &vector;

    strcpy(symbol.outfile, "nosuch_dir/out.svg");

    ret = svg_plot(&symbol, 0);
    assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "svg_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);

    symbol.output_options |= BARCODE_STDOUT;

    ret = svg_plot(&symbol, 0);
    printf(" - ignore (SVG to stdout)\n"); fflush(stdout);
    assert_zero(ret, "svg_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    symbol.vector = NULL;
    ret = svg_plot(&symbol, 0);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "svg_plot ret %d != ZINT_ERROR_INVALID_DATA (%d) (%s)\n", ret, ZINT_ERROR_INVALID_DATA, symbol.errtxt);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_print", test_print, 1, 1, 1 },
        { "test_outfile", test_outfile, 0, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
