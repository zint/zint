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
#include <errno.h>
#include <zlib.h> /* For ZLIBNG_VERSION define (if any) */
#include <sys/stat.h>

INTERNAL int png_pixel_plot(struct zint_symbol *symbol, unsigned char *pixelbuf);

static void test_pixel_plot(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int width;
        int height;
        char *pattern;
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
        /*  6*/ { 8, 2, "CBMWKRYGGYRKWMBC", 0, 0 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, ret;
    struct zint_symbol *symbol = NULL;

    char *png = "out.png";

    char data_buf[8 * 2 + 1];

    const char *const have_identify = testUtilHaveIdentify();

    testStart("test_pixel_plot");

    for (i = 0; i < data_size; i++) {
        int size;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        strcpy(symbol->outfile, png);

        symbol->bitmap_width = data[i].width;
        symbol->bitmap_height = data[i].height;
        symbol->debug |= debug;

        size = data[i].width * data[i].height;
        assert_nonzero(size < (int) sizeof(data_buf), "i:%d png_pixel_plot size %d < sizeof(data_buf) %d\n", i, size, (int) sizeof(data_buf));

        if (data[i].repeat) {
            testUtilStrCpyRepeat(data_buf, data[i].pattern, size);
        } else {
            strcpy(data_buf, data[i].pattern);
        }
        assert_equal(size, (int) strlen(data_buf), "i:%d png_pixel_plot size %d != strlen(data_buf) %d\n", i, size, (int) strlen(data_buf));

        symbol->bitmap = (unsigned char *) data_buf;

        ret = png_pixel_plot(symbol, (unsigned char *) data_buf);
        assert_equal(ret, data[i].ret, "i:%d png_pixel_plot ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

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
        int input_mode;
        int border_width;
        int output_options;
        int whitespace_width;
        int whitespace_height;
        int show_hrt;
        int option_1;
        int option_2;
        float height;
        float scale;
        struct zint_structapp structapp;
        char *fgcolour;
        char *bgcolour;
        float text_gap;
        char *data;
        char *composite;
        int ret;
        char *expected_file;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, UNICODE_MODE, -1, -1, -1, -1, -1, -1, -1, 10.0, 0, { 0, 0, "" }, "", "", 1, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", "", 0, "code128_latin1_1.png", "" },
        /*  1*/ { BARCODE_CODE128, UNICODE_MODE, -1, -1, -1, -1, -1, -1, -1, 10.0, 0, { 0, 0, "" }, "", "", 1, "¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ", "", 0, "code128_latin1_2.png", "" },
        /*  2*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, -1, 10.0, 0, { 0, 0, "" }, "", "", 1, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", "", 0, "code128_latin1_1_bold.png", "" },
        /*  3*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, -1, 10.0, 0, { 0, 0, "" }, "", "", 1, "¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ", "", 0, "code128_latin1_2_bold.png", "" },
        /*  4*/ { BARCODE_CODE128, UNICODE_MODE, -1, SMALL_TEXT, -1, -1, -1, -1, -1, 10.0, 0, { 0, 0, "" }, "", "", 1, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", "", 0, "code128_latin1_1_small.png", "" },
        /*  5*/ { BARCODE_CODE128, UNICODE_MODE, -1, SMALL_TEXT, -1, -1, -1, -1, -1, 10.0, 0, { 0, 0, "" }, "", "", 1, "¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ", "", 0, "code128_latin1_2_small.png", "" },
        /*  6*/ { BARCODE_CODE128, UNICODE_MODE, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "Égjpqy", "", 0, "code128_egrave.png", "" },
        /*  7*/ { BARCODE_CODE128, UNICODE_MODE, -1, BOLD_TEXT, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "Égjpqy", "", 0, "code128_egrave_bold.png", "" },
        /*  8*/ { BARCODE_CODE128, UNICODE_MODE, 3, BOLD_TEXT | BARCODE_BOX, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "Égjpqy", "", 0, "code128_egrave_bold_box3.png", "" },
        /*  9*/ { BARCODE_CODE128, UNICODE_MODE, 2, BOLD_TEXT | BARCODE_BOX, 2, 2, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "Égjpqy", "", 0, "code128_egrave_bold_hvwsp2_box2.png", "" },
        /* 10*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, -1, -1, 3, -1, 0, 0, { 0, 0, "" }, "", "", 1, "[00]030123456789012340", "[02]13012345678909[37]24[10]1234567ABCDEFG", 0, "gs1_128_cc_fig12.png", "" },
        /* 11*/ { BARCODE_CODABLOCKF, -1, 3, -1, -1, -1, -1, 3, -1, 0, 0, { 0, 0, "" }, "", "", 1, "AAAAAAAAA", "", 0, "codablockf_3rows.png", "" },
        /* 12*/ { BARCODE_CODABLOCKF, -1, -1, -1, 2, 2, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "AAAAAAAAA", "", 0, "codablockf_hvwsp2.png", "" },
        /* 13*/ { BARCODE_CODABLOCKF, -1, 2, BARCODE_BOX, 2, 2, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "AAAAAAAAA", "", 0, "codablockf_hvwsp2_box2.png", "" },
        /* 14*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9771384524017", "", 0, "ean13_ggs_5.2.2.1-1.png", "" },
        /* 15*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9771384524017", "", 0, "ean13_ggs_5.2.2.1-1_gws.png", "" },
        /* 16*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9771384524017+12", "", 0, "ean13_2addon_ggs_5.2.2.5.1-2.png", "" },
        /* 17*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9771384524017+12", "", 0, "ean13_2addon_ggs_5.2.2.5.1-2_gws.png", "" },
        /* 18*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9780877799306+54321", "", 0, "ean13_5addon_ggs_5.2.2.5.2-2.png", "" },
        /* 19*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9780877799306+54321", "", 0, "ean13_5addon_ggs_5.2.2.5.2-2_gws.png", "" },
        /* 20*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "123456789012", "[91]12345678901234567890123456789", 0, "ean13_cc_cca_5x4.png", "" },
        /* 21*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "123456789012", "[91]12345678901234567890123456789", 0, "ean13_cc_cca_5x4_gws.png", "" },
        /* 22*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "123456789012+12", "[91]123456789012345678901", 0, "ean13_cc_2addon_cca_4x4.png", "" },
        /* 23*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "123456789012+12", "[91]123456789012345678901", 0, "ean13_cc_2addon_cca_4x4_gws.png", "" },
        /* 24*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "123456789012+54321", "[91]1234567890", 0, "ean13_cc_5addon_ccb_3x4.png", "" },
        /* 25*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "123456789012+54321", "[91]1234567890", 0, "ean13_cc_5addon_ccb_3x4_gws.png", "" },
        /* 26*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "123456789012+54321", "[91]1234567890", 0, "ean13_cc_5addon_ccb_3x4_notext.png", "" },
        /* 27*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "012345678905+24", "", 0, "upca_2addon_ggs_5.2.6.6-5.png", "" },
        /* 28*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "012345678905+24", "", 0, "upca_2addon_ggs_5.2.6.6-5_gws.png", "" },
        /* 29*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "614141234417+12345", "", 0, "upca_5addon.png", "" },
        /* 30*/ { BARCODE_UPCA, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "614141234417+12345", "", 0, "upca_5addon_gws.png", "" },
        /* 31*/ { BARCODE_UPCA, -1, -1, -1, -1, -1, 0, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "614141234417+12345", "", 0, "upca_5addon_notext.png", "" },
        /* 32*/ { BARCODE_UPCA, -1, 3, BARCODE_BIND, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "614141234417+12345", "", 0, "upca_5addon_bind3.png", "" },
        /* 33*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345678901+12", "[91]123456789", 0, "upca_cc_2addon_cca_3x4.png", "" },
        /* 34*/ { BARCODE_UPCA_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345678901+12", "[91]123456789", 0, "upca_cc_2addon_cca_3x4_gws.png", "" },
        /* 35*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4.png", "" },
        /* 36*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4_notext.png", "" },
        /* 37*/ { BARCODE_UPCA_CC, -1, 3, BARCODE_BIND, -1, -1, -1, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345678901+12121", "[91]1234567890123", 0, "upca_cc_5addon_ccb_4x4_bind3.png", "" },
        /* 38*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12", "", 0, "upce_2addon.png", "" },
        /* 39*/ { BARCODE_UPCE, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12", "", 0, "upce_2addon_gws.png", "" },
        /* 40*/ { BARCODE_UPCE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12345", "", 0, "upce_5addon.png", "" },
        /* 41*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12345", "", 0, "upce_5addon_small.png", "" },
        /* 42*/ { BARCODE_UPCE, -1, -1, SMALL_TEXT | EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12345", "", 0, "upce_5addon_small_gws.png", "" },
        /* 43*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2.png", "" },
        /* 44*/ { BARCODE_UPCE_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "0654321+89", "[91]1", 0, "upce_cc_2addon_cca_5x2_gws.png", "" },
        /* 45*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1876543+56789", "[91]12345", 0, "upce_cc_5addon_ccb_8x2.png", "" },
        /* 46*/ { BARCODE_UPCE_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1876543+56789", "[91]12345", 0, "upce_cc_5addon_ccb_8x2_gws.png", "" },
        /* 47*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, -1, 0, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1876543+56789", "[91]12345", 0, "upce_cc_5addon_ccb_8x2_notext.png", "" },
        /* 48*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567", "", 0, "ean8_gss_5.2.2.2-1.png", "" },
        /* 49*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567", "", 0, "ean8_gss_5.2.2.2-1_gws.png", "" },
        /* 50*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12", "", 0, "ean8_2addon.png", "" },
        /* 51*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12", "", 0, "ean8_2addon_gws.png", "" },
        /* 52*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12345", "", 0, "ean8_5addon.png", "" },
        /* 53*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "1234567+12345", "", 0, "ean8_5addon_gws.png", "" },
        /* 54*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9876543+65", "[91]1234567", 0, "ean8_cc_2addon_cca_4x3.png", "" },
        /* 55*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9876543+65", "[91]1234567", 0, "ean8_cc_2addon_cca_4x3_gws.png", "" },
        /* 56*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9876543+74083", "[91]123456789012345678", 0, "ean8_cc_5addon_ccb_8x3.png", "" },
        /* 57*/ { BARCODE_EANX_CC, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, 2, -1, 0, 0, { 0, 0, "" }, "", "", 1, "9876543+74083", "[91]123456789012345678", 0, "ean8_cc_5addon_ccb_8x3_gws.png", "" },
        /* 58*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345", "", 0, "ean5.png", "" },
        /* 59*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345", "", 0, "ean5_gws.png", "" },
        /* 60*/ { BARCODE_EANX, -1, 2, BARCODE_BIND, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345", "", 0, "ean5_bind2.png", "" },
        /* 61*/ { BARCODE_EANX, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12", "", 0, "ean2.png", "" },
        /* 62*/ { BARCODE_EANX, -1, -1, EANUPC_GUARD_WHITESPACE, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12", "", 0, "ean2_gws.png", "" },
        /* 63*/ { BARCODE_EANX, -1, 1, BARCODE_BOX, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12", "", 0, "ean2_box1.png", "" },
        /* 64*/ { BARCODE_CODE39, -1, -1, SMALL_TEXT, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "123", "", 0, "code39_small.png", "" },
        /* 65*/ { BARCODE_POSTNET, -1, -1, -1, -1, -1, -1, -1, -1, 0, 3.5, { 0, 0, "" }, "", "", 1, "12345", "", 0, "postnet_zip.png", "300 dpi, using 1/43in X, 300 / 43 / 2 = ~3.5 scale" },
        /* 66*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "CFCECDCC", 1, "12345", "", 0, "pdf417_bgalpha.png", "" },
        /* 67*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "30313233", "", 1, "12345", "", 0, "pdf417_fgalpha.png", "" },
        /* 68*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "20212244", "CFCECDCC", 1, "12345", "", 0, "pdf417_bgfgalpha.png", "" },
        /* 69*/ { BARCODE_ULTRA, -1, -1, -1, 2, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "0000007F", "FF000033", 1, "12345", "", 0, "ultra_bgfgalpha.png", "" },
        /* 70*/ { BARCODE_ULTRA, -1, -1, -1, 2, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "FF000033", 1, "12345", "", 0, "ultra_bgalpha.png", "" },
        /* 71*/ { BARCODE_ULTRA, -1, -1, -1, 2, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "0000007F", "FF0000", 1, "12345", "", 0, "ultra_fgalpha.png", "" },
        /* 72*/ { BARCODE_ULTRA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "0000007F", "", 1, "12345", "", 0, "ultra_fgalpha_nobg.png", "" },
        /* 73*/ { BARCODE_ULTRA, -1, 1, BARCODE_BOX, 1, 1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345", "", 0, "ultra_hvwsp1_box1.png", "" },
        /* 74*/ { BARCODE_ULTRA, -1, 1, BARCODE_BOX, 1, 1, -1, -1, -1, 0, 0, { 0, 0, "" }, "00FF007F", "BABDB6", 1, "12345", "", 0, "ultra_fgalpha_hvwsp1_box1.png", "" },
        /* 75*/ { BARCODE_ULTRA, -1, 1, BARCODE_BIND_TOP, 1, 1, -1, -1, -1, 0, 0, { 0, 0, "" }, "00FF007F", "BABDB6", 1, "12345", "", 0, "ultra_fgalpha_hvwsp1_bindtop1.png", "" },
        /* 76*/ { BARCODE_ULTRA, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0.5, { 0, 0, "" }, "", "", 1, "1", "", 0, "ultra_odd.png", "" },
        /* 77*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0.5, { 0, 0, "" }, "", "", 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_0.5.png", "6 dpmm, 150 dpi" },
        /* 78*/ { BARCODE_MAXICODE, -1, 1, BARCODE_BOX, 3, -1, -1, -1, -1, 0, 0.7, { 0, 0, "" }, "", "", 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_0.7_wsp3_box1.png", "8 dpmm, 200 dpi" },
        /* 79*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1.4, { 0, 0, "" }, "1111117F", "EEEEEEEE", 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_1.4_bgfgalpha.png", "16 dpmm, 400 dpi" },
        /* 80*/ { BARCODE_MAXICODE, -1, -1, -1, -1, -1, -1, -1, -1, 0, 2.1, { 0, 0, "" }, "", "", 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_2.1.png", "24 dpmm, 600 dpi" },
        /* 81*/ { BARCODE_MAXICODE, -1, 2, BARCODE_BOX, 1, 1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_hvwsp1_box2.png", "" },
        /* 82*/ { BARCODE_MAXICODE, -1, 1, BARCODE_BIND, -1, 1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, "maxicode_vwsp1_bind1.png", "" },
        /* 83*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, -1, -1, -1, -1, -1, 0, 2.0, { 0, 0, "" }, "", "", 1, "1234", "", 0, "datamatrix_2.0_bind1_dotty.png", "" },
        /* 84*/ { BARCODE_DATAMATRIX, -1, 1, BARCODE_BIND | BARCODE_DOTTY_MODE, 1, 1, -1, -1, -1, 0, 2.0, { 0, 0, "" }, "", "", 1, "1234", "", 0, "datamatrix_2.0_hvwsp1_bind1_dotty.png", "" },
        /* 85*/ { BARCODE_DBAR_LTD, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "12345678909", "", 0, "dbar_ltd.png", "" },
        /* 86*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, -1, -1, 5.0, 0, { 0, 0, "" }, "", "", 1, "Your Data Here!", "", ZINT_WARN_NONCOMPLIANT, "pdf417_height5.png", "" },
        /* 87*/ { BARCODE_USPS_IMAIL, -1, -1, -1, -1, -1, -1, -1, -1, 7.75, 0, { 0, 0, "" }, "", "", 1, "12345678901234567890", "", 0, "imail_height7.75.png", "" },
        /* 88*/ { BARCODE_AZTEC, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, { 4, 7, "Z1.txt" }, "", "", 1, "3456", "", 0, "aztec_z1_seq4of7.png", "" },
        /* 89*/ { BARCODE_PDF417, -1, -1, BARCODE_NO_QUIET_ZONES, -1, -1, -1, 5, 8, 16, 1.5, { 0, 0, "" }, "", "", 1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", "", ZINT_WARN_NONCOMPLIANT, "pdf417_#204.png", "Ticket #204 Blank line in PDF417" },
        /* 90*/ { BARCODE_DPD, -1, -1, BARCODE_QUIET_ZONES | COMPLIANT_HEIGHT, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "", "", 1, "008182709980000020028101276", "", 0, "dpd_compliant.png", "Now with bind top 3X default" },
        /* 91*/ { BARCODE_CHANNEL, -1, -1, CMYK_COLOUR | COMPLIANT_HEIGHT, -1, -1, -1, -1, -1, 0, 0, { 0, 0, "" }, "100,85,0,20", "FFFFFF00", 1, "123", "", 0, "channel_cmyk_nobg.png", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *data_dir = "/backend/tests/data/png";
    const char *png = "out.png";
    char expected_file[1024];
    char escaped[1024];
    int escaped_size = 1024;
    unsigned char filebuf[32768];
    int filebuf_size;
    char *text;

    const char *const have_identify = testUtilHaveIdentify();

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
        int text_length;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
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
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }
        symbol->text_gap = data[i].text_gap;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        text_length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, text_length);
        assert_equal(ret, data[i].ret, "i:%d %s ZBarcode_Encode ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        strcpy(symbol->outfile, png);
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret, symbol->errtxt);

        assert_nonzero(testUtilDataPath(expected_file, sizeof(expected_file), data_dir, data[i].expected_file), "i:%d testUtilDataPath == 0\n", i);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %d, %d, %d, %d, %.5g, %.5g, { %d, %d, \"%s\" }, \"%s\", \"%s\", %.5g, \"%s\", \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].border_width, testUtilOutputOptionsName(data[i].output_options),
                    data[i].whitespace_width, data[i].whitespace_height, data[i].show_hrt, data[i].option_1, data[i].option_2, data[i].height, data[i].scale,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    data[i].fgcolour, data[i].bgcolour, data[i].text_gap,
                    testUtilEscape(data[i].data, length, escaped, escaped_size), data[i].composite, testUtilErrorName(data[i].ret), data[i].expected_file, data[i].comment);
            ret = testUtilRename(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d testUtilRename(%s, %s) ret %d != 0\n", i, symbol->outfile, expected_file, ret);
            if (have_identify) {
                ret = testUtilVerifyIdentify(have_identify, expected_file, debug);
                assert_zero(ret, "i:%d %s identify %s ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), expected_file, ret);
            }
        } else {
            assert_nonzero(testUtilExists(symbol->outfile), "i:%d testUtilExists(%s) == 0\n", i, symbol->outfile);
            assert_nonzero(testUtilExists(expected_file), "i:%d testUtilExists(%s) == 0\n", i, expected_file);

            ret = testUtilCmpPngs(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpPngs(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            #ifndef ZLIBNG_VERSION /* zlib-ng (used by e.g. Fedora 40) may produce non-binary compat output */
            ret = testUtilCmpBins(symbol->outfile, expected_file);
            assert_zero(ret, "i:%d %s testUtilCmpBins(%s, %s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, expected_file, ret);
            #endif

            ret = testUtilReadFile(symbol->outfile, filebuf, sizeof(filebuf), &filebuf_size); /* For BARCODE_MEMORY_FILE */
            assert_zero(ret, "i:%d %s testUtilReadFile(%s) %d != 0\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret);

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
            assert_zero(memcmp(symbol->memfile, filebuf, symbol->memfile_size), "i:%d %s memcmp(memfile, filebuf) != 0\n",
                            i, testUtilBarcodeName(data[i].symbology));
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

    testStart("test_outfile");

    symbol.symbology = BARCODE_CODE128;
    symbol.bitmap = data;
    symbol.bitmap_width = symbol.bitmap_height = 1;

    strcpy(symbol.outfile, "test_png_out.png");
#ifndef _WIN32
    skip_readonly_test = getuid() == 0; /* Skip if running as root on Unix as can't create read-only file */
#endif
    if (!skip_readonly_test) {
        static char expected_errtxt[] = "632: Could not open PNG output file ("; /* Excluding OS-dependent `errno` stuff */

        (void) testUtilRmROFile(symbol.outfile); /* In case lying around from previous fail */
        assert_nonzero(testUtilCreateROFile(symbol.outfile), "png_pixel_plot testUtilCreateROFile(%s) fail (%d: %s)\n", symbol.outfile, errno, strerror(errno));

        ret = png_pixel_plot(&symbol, data);
        assert_equal(ret, ZINT_ERROR_FILE_ACCESS, "png_pixel_plot ret %d != ZINT_ERROR_FILE_ACCESS (%d) (%s)\n", ret, ZINT_ERROR_FILE_ACCESS, symbol.errtxt);
        assert_zero(testUtilRmROFile(symbol.outfile), "png_pixel_plot testUtilRmROFile(%s) != 0 (%d: %s)\n", symbol.outfile, errno, strerror(errno));
        assert_zero(strncmp(symbol.errtxt, expected_errtxt, sizeof(expected_errtxt) - 1), "strncmp(%s, %s) != 0\n", symbol.errtxt, expected_errtxt);
    }

    symbol.output_options |= BARCODE_STDOUT;

    printf(">>>Begin ignore (PNG to stdout)\n"); fflush(stdout);
    ret = png_pixel_plot(&symbol, data);
    printf("\n<<<End ignore (PNG to stdout)\n"); fflush(stdout);
    assert_zero(ret, "png_pixel_plot ret %d != 0 (%s)\n", ret, symbol.errtxt);

    testFinish();
}

#include <png.h>
#include <setjmp.h>

struct wpng_error_type {
    struct zint_symbol *symbol;
    jmp_buf jmpbuf;
};

INTERNAL void wpng_error_handler_test(png_structp png_ptr, png_const_charp msg);

static void test_wpng_error_handler(const testCtx *const p_ctx) {
    int ret;
    char filename[] = "out.png";
    FILE *fp;
    struct wpng_error_type wpng_error;
    struct zint_symbol symbol = {0};
    png_structp png_ptr;
    png_infop info_ptr;

    (void)p_ctx;

    testStart("test_wpng_error_handler");

    wpng_error.symbol = &symbol;

    /* Create empty file */
    (void) testUtilRemove(filename); /* In case junk hanging around */
    fp = testUtilOpen(filename, "w+");
    assert_nonnull(fp, "testUtilOpen(%s) failed\n", filename);
    ret = fclose(fp);
    assert_zero(ret, "fclose(%s) %d != 0\n", filename, ret);

    /* Re-open for read, which will cause libpng to error */
    fp = testUtilOpen(filename, "r");
    assert_nonnull(fp, "testUtilOpen(%s) for read failed\n", filename);

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, &wpng_error, wpng_error_handler_test, NULL);
    assert_nonnull(png_ptr, "png_create_write_struct failed\n");

    info_ptr = png_create_info_struct(png_ptr);
    assert_nonnull(info_ptr, "png_create_info_struct failed\n");

    if (setjmp(wpng_error.jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        ret = fclose(fp);
        assert_zero(ret, "fclose(%s) %d != 0\n", filename, ret);
        assert_nonnull(strstr(symbol.errtxt, "635: libpng error:"), "strstr(%s) NULL\n", symbol.errtxt);
        assert_zero(testUtilRemove(filename), "testUtilRemove(%s) != 0 (%d: %s)\n", filename, errno, strerror(errno));
    } else {
        png_init_io(png_ptr, fp);

        /* This should fail and jmp to setjmp */
        png_write_info(png_ptr, info_ptr);
        assert_zero(1, "libpng error setjmp failed\n");
    }

    testFinish();
}

/* Check compliant height printable for max CODABLOCKF with 44 rows * ((62 cols) * 0.55 + 3)) = 1632.4 */
static void test_large_compliant_height(const testCtx *const p_ctx) {
    int ret;
    struct zint_symbol *symbol = NULL;
    const char pattern[] = { "1" };
    const int codablockf_max = 2726;
    char data_buf[2726 + 1];

    (void)p_ctx;

    testStartSymbol("test_large_compliant_height", &symbol);

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    symbol->symbology = BARCODE_CODABLOCKF;
    symbol->output_options |= COMPLIANT_HEIGHT;
    testUtilStrCpyRepeat(data_buf, pattern, codablockf_max);
    assert_equal(codablockf_max, (int) strlen(data_buf), "length %d != strlen(data_buf) %d\n", codablockf_max, (int) strlen(data_buf));

    ret = ZBarcode_Encode_and_Print(symbol, (const unsigned char *) data_buf, codablockf_max, 0);
    assert_zero(ret, "ZBarcode_Encode_and_Print ret %d != 0 (%s)\n", ret, symbol->errtxt);
    assert_zero(testUtilRemove(symbol->outfile), "testUtilRemove(%s) != 0\n", symbol->outfile);

    ZBarcode_Delete(symbol);

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_pixel_plot", test_pixel_plot },
        { "test_print", test_print },
        { "test_outfile", test_outfile },
        { "test_wpng_error_handler", test_wpng_error_handler },
        { "test_large_compliant_height", test_large_compliant_height },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
