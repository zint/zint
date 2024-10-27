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
        int show_hrt;
        int option_1;
        int option_2;
        int option_3;
        float height;
        char *fgcolour;
        char *bgcolour;
        int rotate_angle;
        char *data;
        char *composite;
        int ret;
        char *expected_file;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "<>\"&'", "", 0, "code128_amperands.svg", "" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold.svg", "" },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT | EMBED_VECTOR_FONT, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold_embed.svg", "" },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, 3, BOLD_TEXT | BARCODE_BOX, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold_box3.svg", "" },
        /*  4*/ { BARCODE_CODE128, UNICODE_MODE, 2, BOLD_TEXT | BARCODE_BOX, 2, 2, -1, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold_hvwsp2_box2.svg", "" },
        /*  5*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, 3, 3, -1, -1, -1, -1, 0, "", "", 0, "Égjpqy", "", 0, "code128_egrave_bold_hvwsp3.svg", "" },
        /*  6*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, -1, -1, 3, -1, -1, 0, "", "", 0, "[00]030123456789012340", "[02]13012345678909[37]24[10]1234567ABCDEFG", 0, "gs1_128_cc_fig12.svg", "" },
        /*  7*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, -1, -1, 3, -1, -1, 0, "", "", 0, "AAAAAAAAA", "", 0, "codablockf_3rows.svg", "" },
        /*  8*/ { BARCODE_CODABLOCKF, -1, -1, -1, 2, 2, -1, 3, -1, -1, 0, "", "", 0, "AAAAAAAAA", "", 0, "codablockf_hvwsp2.svg", "" },
        /*  9*/ { BARCODE_CODABLOCKF, -1, 2, BARCODE_BOX, 2, 2, -1, -1, -1, -1, 0, "", "", 0, "AAAAAAAAA", "", 0, "codablockf_hvwsp2_box2.svg", "" },
        /* 10*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9501101531000", "", 0, "ean13_ggs_5.2.2.1-1.svg", "" },
        /* 11*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9501101531000", "", 0, "ean13_ggs_5.2.2.1-1_gws.svg", "" },
        /* 12*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE | EMBED_VECTOR_FONT, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9501101531000", "", 0, "ean13_ggs_5.2.2.1-1_gws_embed.svg", "" },
        /* 13*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9771384524017+12", "", 0, "ean13_2addon_ggs_5.2.2.5.1-2.svg", "" },
        /* 14*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9771384524017+12", "", 0, "ean13_2addon_ggs_5.2.2.5.1-2_gws.svg", "" },
        /* 15*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9780877799306+54321", "", 0, "ean13_5addon_ggs_5.2.2.5.2-2.svg", "" },
        /* 16*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9780877799306+54321", "", 0, "ean13_5addon_ggs_5.2.2.5.2-2_gws.svg", "" },
        /* 17*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "123456789012", "[91]12345678901234567890123456789", 0, "ean13_cc_cca_5x4.svg", "" },
        /* 18*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "123456789012", "[91]12345678901234567890123456789", 0, "ean13_cc_cca_5x4_gws.svg", "" },
        /* 19*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "123456789012+12", "[91]123456789012345678901", 0, "ean13_cc_2addon_cca_4x4.svg", "" },
        /* 20*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "123456789012+12", "[91]123456789012345678901", 0, "ean13_cc_2addon_cca_4x4_gws.svg", "" },
        /* 21*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "123456789012+54321", "[91]1234567890", 0, "ean13_cc_5addon_ccb_3x4.svg", "" },
        /* 22*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "123456789012+54321", "[91]1234567890", 0, "ean13_cc_5addon_ccb_3x4_gws.svg", "" },
        /* 23*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, 0, 2, -1, -1, 0, "", "", 0, "123456789012+54321", "[91]1234567890", 0, "ean13_cc_5addon_ccb_3x4_notext.svg", "" },
        /* 24*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "012345678905+24", "", 0, "upca_2addon_ggs_5.2.6.6-5.svg", "" },
        /* 25*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "012345678905+24", "", 0, "upca_2addon_ggs_5.2.6.6-5_gws.svg", "" },
        /* 26*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "614141234417+12345", "", 0, "upca_5addon.svg", "" },
        /* 27*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "614141234417+12345", "", 0, "upca_5addon_gws.svg", "" },
        /* 28*/ { BARCODE_UPCA, -1, 3, BARCODE_BIND, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "614141234417+12345", "", 0, "upca_5addon_bind3.svg", "" },
        /* 29*/ { BARCODE_UPCA, -1, -1, SMALL_TEXT | BOLD_TEXT, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "614141234417+12345", "", 0, "upca_5addon_small_bold.svg", "Note BOLD_TEXT ignored for UPC/EAN" },
        /* 30*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "12345678901+12", "[91]123456789", 0, "upca_cc_2addon_cca_3x4.svg", "" },
        /* 31*/ { BARCODE_UPCA_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "12345678901+12", "[91]123456789", 0, "upca_cc_2addon_cca_3x4_gws.svg", "" },
        /* 32*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4.svg", "" },
        /* 33*/ { BARCODE_UPCA_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4_gws.svg", "" },
        /* 34*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, 0, 2, -1, -1, 0, "", "", 0, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4_notext.svg", "" },
        /* 35*/ { BARCODE_UPCA_CC, -1, 3, BARCODE_BIND, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4_bind3.svg", "" },
        /* 36*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12", "", 0, "upce_2addon.svg", "" },
        /* 37*/ { BARCODE_UPCE, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12", "", 0, "upce_2addon_gws.svg", "" },
        /* 38*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "upce_5addon.svg", "" },
        /* 39*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "upce_5addon_small.svg", "" },
        /* 40*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT | EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "upce_5addon_small_gws.svg", "" },
        /* 41*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, 0, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "upce_5addon_notext.svg", "" },
        /* 42*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2.svg", "" },
        /* 43*/ { BARCODE_UPCE_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2_gws.svg", "" },
        /* 44*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, "FF0000EE", "0000FF11", 0, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2_fgbgalpha.svg", "" },
        /* 45*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, "", "FFFFFF00", 0, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2_nobg.svg", "" },
        /* 46*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, "", "", 270, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2_rotate_270.svg", "" },
        /* 47*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "1876543+56789", "[91]12345", 0, "upce_cc_5addon_ccb_8x2.svg", "" },
        /* 48*/ { BARCODE_UPCE_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "1876543+56789", "[91]12345", 0, "upce_cc_5addon_ccb_8x2_gws.svg", "" },
        /* 49*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, 0, 2, -1, -1, 0, "", "", 0, "1876543+56789", "[91]12345", 0, "upce_cc_5addon_ccb_8x2_notext.svg", "" },
        /* 50*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9501234", "", 0, "ean8_gss_5.2.2.2-1.svg", "" },
        /* 51*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "9501234", "", 0, "ean8_gss_5.2.2.2-1_gws.svg", "" },
        /* 52*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12", "", 0, "ean8_2addon.svg", "" },
        /* 53*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12", "", 0, "ean8_2addon_gws.svg", "" },
        /* 54*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "ean8_5addon.svg", "" },
        /* 55*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "1234567+12345", "", 0, "ean8_5addon_gws.svg", "" },
        /* 56*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "9876543+65", "[91]1234567", 0, "ean8_cc_2addon_cca_4x3.svg", "" },
        /* 57*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, -1, 0, "", "", 0, "9876543+65", "[91]1234567", 0, "ean8_cc_2addon_cca_4x3_gws.svg", "" },
        /* 58*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "9876543+74083", "[91]123456789012345678", 0, "ean8_cc_5addon_ccb_8x3.svg", "" },
        /* 59*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 2, -1, -1, 0, "", "", 0, "9876543+74083", "[91]123456789012345678", 0, "ean8_cc_5addon_ccb_8x3_gws.svg", "" },
        /* 60*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12345", "", 0, "ean5.svg", "" },
        /* 61*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12345", "", 0, "ean5_gws.svg", "" },
        /* 62*/ { BARCODE_EANX, -1, 2, BARCODE_BIND, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12345", "", 0, "ean5_bind2.svg", "" },
        /* 63*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12", "", 0, "ean2.svg", "" },
        /* 64*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12", "", 0, "ean2_gws.svg", "" },
        /* 65*/ { BARCODE_EANX, -1, 1, BARCODE_BOX, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12", "", 0, "ean2_box1.svg", "" },
        /* 66*/ { BARCODE_CODE39, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "123", "", 0, "code39_small.svg", "" },
        /* 67*/ { BARCODE_POSTNET, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12345", "", 0, "postnet_zip.svg", "" },
        /* 68*/ { BARCODE_MAXICODE, -1, 2, BARCODE_BOX, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_box2.svg", "" },
        /* 69*/ { BARCODE_MAXICODE, -1, 1, BARCODE_BIND, -1, 1, -1, -1, -1, -1, 0, "", "", 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_vwsp1_bind1.svg", "" },
        /* 70*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "121212DD", "EEEEEE22", 90, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_fgbg_rotate_90.svg", "" },
        /* 71*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, -1, 1, -1, -1, -1, -1, 0, "", "", 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "datamatrix_vwsp1_bind1_dotty.svg", "" },
        /* 72*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, 1, 1, -1, -1, -1, -1, 0, "", "", 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "datamatrix_hvwsp1_bind1_dotty.svg", "" },
        /* 73*/ { BARCODE_DBAR_LTD, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "12345678909", "", 0, "dbar_ltd.svg", "" },
        /* 74*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, -1, 5, "", "", 0, "Your Data Here!", "", ZINT_WARN_NONCOMPLIANT, "pdf417_height5.svg", "" },
        /* 75*/ { BARCODE_USPS_IMAIL, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7.75, "", "", 0, "12345678901234567890", "", 0, "imail_height7.75.svg", "" },
        /* 76*/ { BARCODE_ULTRA, -1, 3, BARCODE_BOX, 2, 2, -1, -1, -1, -1, 0, "FF0000", "0000FF", 0, "12345678901234567890", "", 0, "ultra_fgbg_hvwsp2_box3.svg", "" },
        /* 77*/ { BARCODE_TELEPEN, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0.4, "", "", 180, "A", "", 0, "telepen_height0.4_rotate_180.svg", "" },
        /* 78*/ { BARCODE_CODE49, -1, -1, COMPLIANT_HEIGHT, -1, -1, -1, -1, -1, -1, 0, "FF11157F", "", 0, "A", "", 0, "code49_comph_fgalpha.svg", "" },
        /* 79*/ { BARCODE_CODABLOCKF, -1, -1, COMPLIANT_HEIGHT, -1, -1, -1, -1, -1, 2, 0, "00000033", "FFFFFF66", 0, "1234567890123456789012345678901234", "", 0, "codablockf_comph_sep2_fgbgalpha.svg", "" },
        /* 80*/ { BARCODE_DPD, -1, -1, BARCODE_QUIET_ZONES | COMPLIANT_HEIGHT, -1, -1, -1, -1, -1, -1, 0, "", "", 0, "008182709980000020028101276", "", 0, "dpd_compliant.svg", "" },
        /* 81*/ { BARCODE_CHANNEL, -1, -1, CMYK_COLOUR | COMPLIANT_HEIGHT, -1, -1, -1, -1, -1, -1, 0, "100,85,0,20", "FFFFFF00", 0, "123", "", 0, "channel_cmyk_nobg.svg", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char data_dir[] = "/backend/tests/data/svg";
    const char svg[] = "out.svg";
    const char memfile[] = "mem.eps";
    char expected_file[1024];
    char escaped[1024];
    int escaped_size = 1024;
    char *text;

    int have_libreoffice = 0;
    int have_vnu = 0;

    testStartSymbol("test_print", &symbol);

    if (p_ctx->generate) {
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

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, data[i].output_options, data[i].data, -1, debug);
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

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %d, %d, %d, %d, %d, %.8g, \"%s\", \"%s\", %d, \"%s\", \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].border_width,
                    testUtilOutputOptionsName(data[i].output_options), data[i].whitespace_width, data[i].whitespace_height, data[i].show_hrt,
                    data[i].option_1, data[i].option_2, data[i].option_3, data[i].height, data[i].fgcolour, data[i].bgcolour, data[i].rotate_angle,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].composite,
                    testUtilErrorName(data[i].ret), data[i].expected_file, data[i].comment);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            if (have_libreoffice) {
                ret = testUtilVerifyLibreOffice(expected_file, debug);
                assert_zero(ret, "i:%d %s libreoffice %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
            if (have_vnu) {
                ret = testUtilVerifyVnu(expected_file, debug); /* Very slow */
                assert_zero(ret, "i:%d %s vnu libreoffice %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(expected_file), "i:%d testUtilExists(%s) == 0\n", i, expected_file);

            ret = testUtilCmpSvgs(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpSvgs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);

            symbol->output_options |= BARCODE_MEMORY_FILE;
            ret = ZBarcode_Print(symbol, data[i].rotate_angle);
            assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0 (%s)\n",
                            i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret, symbol->errtxt);
            assert_nonnull(symbol->memfile, "i:%d %s memfile NULL\n", i, testUtilBarcodeName(data[i].symbology));
            assert_nonzero(symbol->memfile_size, "i:%d %s memfile_size 0\n", i, testUtilBarcodeName(data[i].symbology));

            ret = testUtilWriteFile(memfile, symbol->memfile, symbol->memfile_size, "wb");
            assert_zero(ret, "%d: testUtilWriteFile(%s) fail ret %d != 0\n", i, memfile, ret);

            ret = testUtilCmpSvgs(symbol->outfile, memfile);
            assert_zero(ret, "i:%d %s testUtilCmpSvgs(%s, %s) %d != 0\n",
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

INTERNAL int svg_plot(struct zint_symbol *symbol, int rotate_angle);

static void test_outfile(const testCtx *const p_ctx) {
    int ret;
    int skip_readonly_test = 0;
    struct zint_symbol symbol = {0};
    struct zint_vector vector = {0};

    (void)p_ctx;

    testStart("test_outfile");

    symbol.symbology = BARCODE_CODE128;
    symbol.vector = &vector;

    strcpy(symbol.outfile, "test_svg_out.svg");
#ifndef _WIN32
    skip_readonly_test = getuid() == 0; /* Skip if running as root on Unix as can't create read-only file */
#endif
    if (!skip_readonly_test) {
        static char expected_errtxt[] = "680: Could not open SVG output file ("; /* Excluding OS-dependent `errno` stuff */

        (void) testUtilRmROFile(symbol.outfile); /* In case lying around from previous fail */
        assert_nonzero(testUtilCreateROFile(symbol.outfile), "svg_plot testUtilCreateROFile(%s) fail (%d: %s)\n", symbol.outfile, errno, strerror(errno));

        ret = svg_plot(&symbol, 0);
        assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "svg_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);
        assert_zero(testUtilRmROFile(symbol.outfile), "svg_plot testUtilRmROFile(%s) != 0 (%d: %s)\n", symbol.outfile, errno, strerror(errno));
        assert_zero(strncmp(symbol.errtxt, expected_errtxt, sizeof(expected_errtxt) - 1), "strncmp(%s, %s) != 0\n", symbol.errtxt, expected_errtxt);
    }

    symbol.output_options |= BARCODE_STDOUT;

    printf(">>>Begin ignore (SVG to stdout)\n"); fflush(stdout);
    ret = svg_plot(&symbol, 0);
    printf("<<<End ignore (SVG to stdout)\n"); fflush(stdout);
    assert_zero(ret, "svg_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    symbol.vector = NULL;
    ret = svg_plot(&symbol, 0);
    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "svg_plot ret %d != ZINT_ERROR_INVALID_DATA (%d) (%s)\n", ret, ZINT_ERROR_INVALID_DATA, symbol.errtxt);

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
