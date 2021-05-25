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

    testStart("");

    int have_libreoffice = 0;
    int have_vnu = 0;
    if (generate) {
        have_libreoffice = testUtilHaveLibreOffice();
        have_vnu = testUtilHaveVnu();
    }

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
        char *data;
        char *composite;
        char *expected_file;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, -1, -1, -1, -1, -1, "<>\"&'", "", "../data/svg/code128_amperands.svg" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, -1, "Égjpqy", "", "../data/svg/code128_egrave_bold.svg" },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, 3, BOLD_TEXT | BARCODE_BOX, -1, -1, -1, -1, -1, "Égjpqy", "", "../data/svg/code128_egrave_bold_box3.svg" },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, 2, BOLD_TEXT | BARCODE_BOX, 2, 2, -1, -1, -1, "Égjpqy", "", "../data/svg/code128_egrave_bold_hvwsp2_box2.svg" },
        /*  4*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, 3, 3, -1, -1, -1, "Égjpqy", "", "../data/svg/code128_egrave_bold_hvwsp3.svg" },
        /*  5*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, -1, -1, 3, -1, "[00]030123456789012340", "[02]13012345678909[37]24[10]1234567ABCDEFG", "../data/svg/gs1_128_cc_fig12.svg" },
        /*  6*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, -1, -1, 3, -1, "AAAAAAAAA", "", "../data/svg/codablockf_3rows.svg" },
        /*  7*/ { BARCODE_CODABLOCKF, -1, -1, -1, 2, 2, -1, 3, -1, "AAAAAAAAA", "", "../data/svg/codablockf_hvwsp2.svg" },
        /*  8*/ { BARCODE_CODABLOCKF, -1, 2, BARCODE_BOX, 2, 2, -1, -1, -1, "AAAAAAAAA", "", "../data/svg/codablockf_hvwsp2_box2.svg" },
        /*  9*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, "9771384524017+12", "", "../data/svg/ean13_2addon_ggs_5.2.2.5.1-2.svg" },
        /* 10*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, "9780877799306+54321", "", "../data/svg/ean13_5addon_ggs_5.2.2.5.2-2.svg" },
        /* 11*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, "123456789012+12", "[91]123456789012345678901", "../data/svg/ean13_cc_2addon_cca_4x4.svg" },
        /* 12*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, "123456789012+54321", "[91]1234567890", "../data/svg/ean13_cc_5addon_ccb_3x4.svg" },
        /* 13*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, 0, 2, -1, "123456789012+54321", "[91]1234567890", "../data/svg/ean13_cc_5addon_ccb_3x4_notext.svg" },
        /* 14*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, "012345678905+24", "", "../data/svg/upca_2addon_ggs_5.2.6.6-5.svg" },
        /* 15*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, "614141234417+12345", "", "../data/svg/upca_5addon.svg" },
        /* 16*/ { BARCODE_UPCA, -1, 3, BARCODE_BIND, -1, -1, -1, -1, -1, "614141234417+12345", "", "../data/svg/upca_5addon_bind3.svg" },
        /* 17*/ { BARCODE_UPCA, -1, -1, SMALL_TEXT | BOLD_TEXT, -1, -1, -1, -1, -1, "614141234417+12345", "", "../data/svg/upca_5addon_small_bold.svg" },
        /* 18*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 1, -1, "12345678901+12", "[91]123456789", "../data/svg/upca_cc_2addon_cca_3x4.svg" },
        /* 19*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 2, -1, "12345678901+12121", "[91]1234567890123", "../data/svg/upca_cc_5addon_ccb_4x4.svg" },
        /* 20*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, 0, 2, -1, "12345678901+12121", "[91]1234567890123", "../data/svg/upca_cc_5addon_ccb_4x4_notext.svg" },
        /* 21*/ { BARCODE_UPCA_CC, -1, 3, BARCODE_BIND, -1, -1, -1, 2, -1, "12345678901+12121", "[91]1234567890123", "../data/svg/upca_cc_5addon_ccb_4x4_bind3.svg" },
        /* 22*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, "1234567+12", "", "../data/svg/upce_2addon.svg" },
        /* 23*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, "1234567+12345", "", "../data/svg/upce_5addon.svg" },
        /* 24*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, "1234567+12345", "", "../data/svg/upce_5addon_small.svg" },
        /* 25*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, 0, -1, -1, "1234567+12345", "", "../data/svg/upce_5addon_notext.svg" },
        /* 26*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, "0654321+89", "[91]1", "../data/svg/upce_cc_2addon_cca_5x2.svg" },
        /* 27*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 2, -1, "1876543+56789", "[91]12345", "../data/svg/upce_cc_5addon_ccb_8x2.svg" },
        /* 28*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, 0, 2, -1, "1876543+56789", "[91]12345", "../data/svg/upce_cc_5addon_ccb_8x2_notext.svg" },
        /* 29*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, "1234567+12", "", "../data/svg/ean8_2addon.svg" },
        /* 30*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, "1234567+12345", "", "../data/svg/ean8_5addon.svg" },
        /* 31*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, "9876543+65", "[91]1234567", "../data/svg/ean8_cc_2addon_cca_4x3.svg" },
        /* 32*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, "9876543+74083", "[91]123456789012345678", "../data/svg/ean8_cc_5addon_ccb_8x3.svg" },
        /* 33*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, "12345", "", "../data/svg/ean5.svg" },
        /* 34*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, "12", "", "../data/svg/ean2.svg" },
        /* 35*/ { BARCODE_CODE39, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, "123", "", "../data/svg/code39_small.svg" },
        /* 36*/ { BARCODE_POSTNET, -1, -1, -1, -1, -1, -1, -1, -1, "12345", "", "../data/svg/postnet_zip.svg" },
        /* 37*/ { BARCODE_MAXICODE, -1, 2, BARCODE_BOX, -1, -1, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "../data/svg/maxicode_box2.svg" },
        /* 38*/ { BARCODE_MAXICODE, -1, 1, BARCODE_BIND, -1, 1, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "../data/svg/maxicode_vwsp1_bind1.svg" },
        /* 39*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, -1, 1, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "../data/svg/datamatrix_vwsp1_bind1_dotty.svg" },
        /* 40*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, 1, 1, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", "../data/svg/datamatrix_hvwsp1_bind1_dotty.svg" },
    };
    int data_size = ARRAY_SIZE(data);

    char *data_dir = "../data/svg";
    char *svg = "out.svg";
    char escaped[1024];
    int escaped_size = 1024;
    char *text;

    if (generate) {
        if (!testUtilExists(data_dir)) {
            ret = mkdir(data_dir, 0755);
            assert_zero(ret, "mkdir(%s) ret %d != 0\n", data_dir, ret);
        }
    }

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
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
        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int text_length = strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, text_length);
        assert_zero(ret, "i:%d %s ZBarcode_Encode ret %d != 0 %s\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        strcpy(symbol->outfile, svg);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %d, %d, %d, %d, \"%s\", \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].border_width, testUtilOutputOptionsName(data[i].output_options),
                    data[i].whitespace_width, data[i].whitespace_height, data[i].show_hrt, data[i].option_1, data[i].option_2,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].composite, data[i].expected_file);
            ret = rename(symbol->outfile, data[i].expected_file);
            assert_zero(ret, "i:%d rename(%s, %s) ret %d != 0\n", i, symbol->outfile, data[i].expected_file, ret);
            if (have_libreoffice) {
                ret = testUtilVerifyLibreOffice(data[i].expected_file, debug);
                assert_zero(ret, "i:%d %s libreoffice %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_file, ret);
            }
            if (have_vnu) {
                ret = testUtilVerifyVnu(data[i].expected_file, debug); // Very slow
                assert_zero(ret, "i:%d %s vnu libreoffice %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(data[i].expected_file), "i:%d testUtilExists(%s) == 0\n", i, data[i].expected_file);

            ret = testUtilCmpSvgs(symbol->outfile, data[i].expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpSvgs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, data[i].expected_file, ret);
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_print", test_print, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
