/*
    libzint - the open source barcode library
    Copyright (C) 2019-2024 Robin Stuart <rstuart114@gmail.com>

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

/*
 * Check that GS1_128-based and DBAR_EXP-based symbologies reduce GS1 data
 */
static void test_gs1_reduce(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        char *data;
        char *composite;
        int ret;

        char *comment;
        char *expected;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_GS1_128, -1, "12345678901234", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /* 1*/ { BARCODE_GS1_128, -1, "[01]12345678901231", "", 0, "Input mode ignored; verified manually against tec-it",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /* 2*/ { BARCODE_GS1_128, GS1_MODE, "[01]12345678901231", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /* 3*/ { BARCODE_GS1_128, UNICODE_MODE, "[01]12345678901231", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /* 4*/ { BARCODE_GS1_128, GS1PARENS_MODE, "(01)12345678901231", "", 0, "Input mode ignored (parentheses instead of square brackets)",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /* 5*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110010011100100010100001000101001110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001101100011011101011110111010110001100011101011"
               },
        /* 6*/ { BARCODE_GS1_128_CC, GS1_MODE, "[01]12345678901231", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110010011100100010100001000101001110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001101100011011101011110111010110001100011101011"
               },
        /* 7*/ { BARCODE_GS1_128_CC, UNICODE_MODE, "[01]12345678901231", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110010011100100010100001000101001110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001101100011011101011110111010110001100011101011"
               },
        /* 8*/ { BARCODE_GS1_128_CC, GS1PARENS_MODE, "(01)12345678901231", "(21)1234", 0, "Input mode ignored (parentheses instead of square brackets)",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110010011100100010100001000101001110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001101100011011101011110111010110001100011101011"
               },
        /* 9*/ { BARCODE_EAN14, -1, "1234567890123", "", 0, "Input mode ignored; verified manually against tec-it",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /*10*/ { BARCODE_EAN14, GS1_MODE, "1234567890123", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /*11*/ { BARCODE_EAN14, UNICODE_MODE, "1234567890123", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /*12*/ { BARCODE_EAN14, GS1PARENS_MODE, "1234567890123", "", 0, "Input mode ignored (parentheses instead of square brackets)",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /*13*/ { BARCODE_NVE18, -1, "12345678901234567", "", 0, "Input mode ignored; verified manually against tec-it",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*14*/ { BARCODE_NVE18, GS1_MODE, "12345678901234567", "", 0, "Input mode ignored",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*15*/ { BARCODE_NVE18, UNICODE_MODE, "12345678901234567", "", 0, "Input mode ignored",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*16*/ { BARCODE_NVE18, GS1PARENS_MODE, "12345678901234567", "", 0, "Input mode ignored (parentheses instead of square brackets)",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*17*/ { BARCODE_DBAR_EXP, -1, "2012", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*18*/ { BARCODE_DBAR_EXP, -1, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*19*/ { BARCODE_DBAR_EXP, GS1PARENS_MODE, "(20)12", "", 0, "Input mode ignored (parentheses instead of square brackets)",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*20*/ { BARCODE_DBAR_EXP, GS1_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*21*/ { BARCODE_DBAR_EXP, GS1_MODE | GS1PARENS_MODE, "(20)12", "", 0, "Input mode ignored (parentheses instead of square brackets)",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*22*/ { BARCODE_DBAR_EXP, UNICODE_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*23*/ { BARCODE_DBAR_EXP_CC, -1, "[20]12", "[21]1234", 0, "Input mode ignored; verified manually against tec-it",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*24*/ { BARCODE_DBAR_EXP_CC, GS1_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*25*/ { BARCODE_DBAR_EXP_CC, UNICODE_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*26*/ { BARCODE_DBAR_EXPSTK, -1, "12", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*27*/ { BARCODE_DBAR_EXPSTK, -1, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*28*/ { BARCODE_DBAR_EXPSTK, GS1_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*29*/ { BARCODE_DBAR_EXPSTK, UNICODE_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*30*/ { BARCODE_DBAR_EXPSTK_CC, -1, "12", "[21]1234", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*31*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[20]12", "[21]1234", 0, "Input mode ignored; verified manually against tec-it (same as BARCODE_DBAR_EXP_CC above)",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*32*/ { BARCODE_DBAR_EXPSTK_CC, GS1PARENS_MODE, "(20)12", "(21)1234", 0, "Input mode ignored (parentheses instead of square brackets)",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*33*/ { BARCODE_DBAR_EXPSTK_CC, GS1_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*34*/ { BARCODE_DBAR_EXPSTK_CC, UNICODE_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;

    char bwipp_buf[8196];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStart("test_gs1_reduce");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].composite[0]) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, text, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);

        if (p_ctx->generate) {
            if (data[i].ret == 0) {
                printf("        /*%2d*/ { %s, %s, \"%s\", \"%s\", %d, \"%s\",\n",
                        i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].data, data[i].composite, data[i].ret, data[i].comment);
                testUtilModulesPrint(symbol, "                    ", "\n");
                printf("               },\n");
            } else {
                printf("        /*%2d*/ { %s, %s, \"%s\", \"%s\", %s, \"%s\", \"\" },\n",
                        i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].data, data[i].composite, testUtilErrorName(data[i].ret), data[i].comment);
            }
        } else {
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            if (ret < ZINT_ERROR) {
                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d %s testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, -1, -1, text, length, symbol->primary, bwipp_buf, sizeof(bwipp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/*
 * Check GS1_128-based and DBAR_EXP-based symbologies HRT
 * See test_hrt() in test_rss.c and test_composite.c for DBAR other than DBAR_EXP-based
 */
static void test_hrt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        char *data;
        char *composite;

        int ret;
        char *expected;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_GS1_128, -1, "[01]12345678901234[20]12", "", ZINT_WARN_NONCOMPLIANT, "(01)12345678901234(20)12" }, /* Incorrect check digit */
        /*  1*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]12345678901234[20]12", "", 0, "(01)12345678901234(20)12" },
        /*  2*/ { BARCODE_GS1_128, -1, "[01]12345678901231[20]12", "", 0, "(01)12345678901231(20)12" },
        /*  3*/ { BARCODE_GS1_128, -1, "[01]12345678901231[10]12[20]AB", "", ZINT_WARN_NONCOMPLIANT, "(01)12345678901231(10)12(20)AB" }, /* AI (20) should be 2 nos. */
        /*  4*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]12345678901231[10]10[20]AB", "", 0, "(01)12345678901231(10)10(20)AB" },
        /*  5*/ { BARCODE_GS1_128, -1, "[01]12345678901231[10]AB[20]12", "", 0, "(01)12345678901231(10)AB(20)12" },
        /*  6*/ { BARCODE_GS1_128, -1, "[91]ABCDEF]GH", "", ZINT_WARN_NONCOMPLIANT, "(91)ABCDEF]GH" }, /* Invalid CSET 82 character */
        /*  7*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[91]ABCDEF]GH", "", 0, "(91)ABCDEF]GH" },
        /*  8*/ { BARCODE_GS1_128, -1, "[91]ABCDEF)GH", "", 0, "(91)ABCDEF)GH" },
        /*  9*/ { BARCODE_GS1_128, -1, "[91]ABCDEF(GH", "", 0, "(91)ABCDEF(GH" },
        /* 10*/ { BARCODE_GS1_128, -1, "[91]ABCDE(20)12", "", 0, "(91)ABCDE(20)12" },
        /* 11*/ { BARCODE_GS1_128, GS1PARENS_MODE, "(91)ABCDEF]GH", "", ZINT_WARN_NONCOMPLIANT, "(91)ABCDEF]GH" }, /* Invalid CSET 82 character */
        /* 12*/ { BARCODE_GS1_128, GS1PARENS_MODE | GS1NOCHECK_MODE, "(91)ABCDEF]GH", "", 0, "(91)ABCDEF]GH" },
        /* 13*/ { BARCODE_GS1_128, GS1PARENS_MODE, "(91)ABCDEF)GH", "", 0, "(91)ABCDEF)GH" },
        /* 14*/ { BARCODE_GS1_128, GS1PARENS_MODE, "(91)ABCDE[FGH", "", ZINT_WARN_NONCOMPLIANT, "(91)ABCDE[FGH" }, /* Invalid CSET 82 character */
        /* 15*/ { BARCODE_GS1_128, GS1PARENS_MODE | GS1NOCHECK_MODE, "(91)ABCDE[FGH", "", 0, "(91)ABCDE[FGH" },
        /* 16*/ { BARCODE_GS1_128, GS1PARENS_MODE, "(91)ABCDE[92]GH", "", ZINT_WARN_NONCOMPLIANT, "(91)ABCDE[92]GH" }, /* Invalid CSET 82 character */
        /* 17*/ { BARCODE_GS1_128, GS1PARENS_MODE | GS1NOCHECK_MODE, "(91)ABCDE[92]GH", "", 0, "(91)ABCDE[92]GH" },
        /* 18*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901234[20]12", "[21]12345", ZINT_WARN_NONCOMPLIANT, "(01)12345678901234(20)12" }, /* Incorrect check digit */
        /* 19*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]12345678901234[20]12", "[21]12345", 0, "(01)12345678901234(20)12" },
        /* 20*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231[20]12", "[21]12345", 0, "(01)12345678901231(20)12" },
        /* 21*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231[10]12[20]AB", "[21]12345", ZINT_WARN_NONCOMPLIANT, "(01)12345678901231(10)12(20)AB" }, /* AI (20) should be 2 nos. */
        /* 22*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]12345678901231[10]12[20]AB", "[21]12345", 0, "(01)12345678901231(10)12(20)AB" },
        /* 23*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231[10]AB[20]12", "[21]12345", 0, "(01)12345678901231(10)AB(20)12" },
        /* 24*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231[10]AB[20]12", "[30]1234567A", ZINT_WARN_NONCOMPLIANT, "(01)12345678901231(10)AB(20)12" },
        /* 25*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]12345678901231[10]AB[20]12", "[30]1234567A", 0, "(01)12345678901231(10)AB(20)12" },
        /* 26*/ { BARCODE_EAN14, -1, "1234567890123", "", 0, "(01)12345678901231" },
        /* 27*/ { BARCODE_EAN14, -1, "1234", "", 0, "(01)00000000012348" },
        /* 28*/ { BARCODE_EAN14, -1, "12345", "", 0, "(01)00000000123457" },
        /* 29*/ { BARCODE_EAN14, -1, "12340", "", 0, "(01)00000000123402" },
        /* 30*/ { BARCODE_NVE18, -1, "12345678901234567", "", 0, "(00)123456789012345675" },
        /* 31*/ { BARCODE_NVE18, -1, "1234", "", 0, "(00)000000000000012348" },
        /* 32*/ { BARCODE_NVE18, -1, "12345", "", 0, "(00)000000000000123457" },
        /* 33*/ { BARCODE_NVE18, -1, "12340", "", 0, "(00)000000000000123402" },
        /* 34*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901234[20]12", "", ZINT_WARN_NONCOMPLIANT, "(01)12345678901234(20)12" }, /* Incorrect check digit */
        /* 35*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[01]12345678901234[20]12", "", 0, "(01)12345678901234(20)12" },
        /* 36*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[20]12", "", 0, "(01)12345678901231(20)12" },
        /* 37*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[10]12[20]AB", "", ZINT_WARN_NONCOMPLIANT, "(01)12345678901231(10)12(20)AB" }, /* AI (20) should be 2 nos. */
        /* 38*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[01]12345678901231[10]12[20]AB", "", 0, "(01)12345678901231(10)12(20)AB" },
        /* 39*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[10]AB[20]12", "", 0, "(01)12345678901231(10)AB(20)12" },
        /* 40*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[10]AB[20]12[90]ABC(2012", "", 0, "(01)12345678901231(10)AB(20)12(90)ABC(2012" },
        /* 41*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[10]AB[20]12[90]ABC20)12", "", 0, "(01)12345678901231(10)AB(20)12(90)ABC20)12" },
        /* 42*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[10]AB[20]12[90]ABC(20)12", "", 0, "(01)12345678901231(10)AB(20)12(90)ABC(20)12" },
        /* 43*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901234", "[21]12345", ZINT_WARN_NONCOMPLIANT, "(01)12345678901234" },
        /* 44*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]12345678901234", "[21]12345", 0, "(01)12345678901234" },
        /* 45*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231", "[21]12345", 0, "(01)12345678901231" },
        /* 46*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231[20]12[21]12345", "[21]12345", 0, "(01)12345678901231(20)12(21)12345" },
        /* 47*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901234[20]12", "", ZINT_WARN_NONCOMPLIANT, "" },
        /* 48*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[01]12345678901234[20]12", "", 0, "" },
        /* 49*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901231[20]12", "", 0, "" },
        /* 50*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[20]12[90]ABC(20)12", "", 0, "(01)12345678901231(20)12(90)ABC(20)12" },
        /* 51*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901234[20]12", "[21]12345", ZINT_WARN_NONCOMPLIANT, "" },
        /* 52*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]12345678901234[20]12", "[21]12345", 0, "" },
        /* 53*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231[20]12", "[21]12345", 0, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;

    testStart("test_hrt");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].composite[0]) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, text, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, data[i].ret, ret, symbol->errtxt);

        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include "../gs1.h"

static void test_gs1_verify(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int ret;
        char *expected;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "", ZINT_ERROR_INVALID_DATA, "", "252: Data does not start with an AI" },
        /*  1*/ { "[", ZINT_ERROR_INVALID_DATA, "", "253: Malformed AI in input data (brackets don't match)" },
        /*  2*/ { "[]12", ZINT_ERROR_INVALID_DATA, "", "256: Invalid AI in input data (AI too short)" },
        /*  3*/ { "[1]12", ZINT_ERROR_INVALID_DATA, "", "256: Invalid AI in input data (AI too short)" },
        /*  4*/ { "[12345]12", ZINT_ERROR_INVALID_DATA, "", "255: Invalid AI in input data (AI too long)" },
        /*  5*/ { "[9999]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9999)" },
        /*  6*/ { "[[01]]1234", ZINT_ERROR_INVALID_DATA, "", "254: Found nested brackets in input data" },
        /*  7*/ { "[1A]12", ZINT_ERROR_INVALID_DATA, "", "257: Invalid AI in input data (non-numeric characters in AI)" },
        /*  8*/ { "[10]", ZINT_ERROR_INVALID_DATA, "", "258: Empty data field in input data" },
        /*  9*/ { "[90]\012", ZINT_ERROR_INVALID_DATA, "", "251: Control characters are not supported by GS1" },
        /* 10*/ { "[90]\177", ZINT_ERROR_INVALID_DATA, "", "263: DEL characters are not supported by GS1" },
        /* 11*/ { "[90]\200", ZINT_ERROR_INVALID_DATA, "", "250: Extended ASCII characters are not supported by GS1" },
        /* 12*/ { "[00]123456789012345678", ZINT_WARN_NONCOMPLIANT, "00123456789012345678", "261: AI (00) position 18: Bad checksum '8', expected '5'" },
        /* 13*/ { "[00]123456789012345675", 0, "00123456789012345675", "" },
        /* 14*/ { "[00]12345678901234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (00)" },
        /* 15*/ { "[00]1234567890123456789", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (00)" },
        /* 16*/ { "[00]376104250021234569", 0, "00376104250021234569", "" },
        /* 17*/ { "[00]376104250021234568", ZINT_WARN_NONCOMPLIANT, "00376104250021234568", "261: AI (00) position 18: Bad checksum '8', expected '9'" },
        /* 18*/ { "[01]12345678901234", ZINT_WARN_NONCOMPLIANT, "0112345678901234", "261: AI (01) position 14: Bad checksum '4', expected '1'" },
        /* 19*/ { "[01]12345678901231", 0, "0112345678901231", "" },
        /* 20*/ { "[01]123456789012345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (01)" },
        /* 21*/ { "[02]12345678901234", ZINT_WARN_NONCOMPLIANT, "0212345678901234", "261: AI (02) position 14: Bad checksum '4', expected '1'" },
        /* 22*/ { "[02]12345678901231", 0, "0212345678901231", "" },
        /* 23*/ { "[02]1234567890123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (02)" },
        /* 24*/ { "[03]12345678901234", ZINT_WARN_NONCOMPLIANT, "0312345678901234", "261: AI (03) position 14: Bad checksum '4', expected '1'" },
        /* 25*/ { "[03]12345678901231", 0, "0312345678901231", "" },
        /* 26*/ { "[03]1234567890123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (03)" },
        /* 27*/ { "[04]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (04)" },
        /* 28*/ { "[05]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (05)" },
        /* 29*/ { "[06]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (06)" },
        /* 30*/ { "[07]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (07)" },
        /* 31*/ { "[08]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (08)" },
        /* 32*/ { "[09]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (09)" },
        /* 33*/ { "[10]ABCD123456", 0, "10ABCD123456", "" },
        /* 34*/ { "[10]123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (10)" },
        /* 35*/ { "[100]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (100)" },
        /* 36*/ { "[1000]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1000)" },
        /* 37*/ { "[11]990102", 0, "11990102", "" },
        /* 38*/ { "[11]9901023", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (11)" },
        /* 39*/ { "[110]990102", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (110)" },
        /* 40*/ { "[1100]990102", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1100)" },
        /* 41*/ { "[12]000100", 0, "12000100", "" },
        /* 42*/ { "[12]00010", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (12)" },
        /* 43*/ { "[120]000100", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (120)" },
        /* 44*/ { "[1200]000100", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1200)" },
        /* 45*/ { "[13]991301", ZINT_WARN_NONCOMPLIANT, "13991301", "261: AI (13) position 3: Invalid month '13'" },
        /* 46*/ { "[13]991201", 0, "13991201", "" },
        /* 47*/ { "[13]9913011", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (13)" },
        /* 48*/ { "[130]991301", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (130)" },
        /* 49*/ { "[1300]991301", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1300)" },
        /* 50*/ { "[14]991201", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (14)" },
        /* 51*/ { "[140]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (140)" },
        /* 52*/ { "[1400]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1400)" },
        /* 53*/ { "[15]021231", 0, "15021231", "" },
        /* 54*/ { "[15]02123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (15)" },
        /* 55*/ { "[150]021231", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (150)" },
        /* 56*/ { "[1500]021231", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1500)" },
        /* 57*/ { "[16]000000", ZINT_WARN_NONCOMPLIANT, "16000000", "261: AI (16) position 3: Invalid month '00'" },
        /* 58*/ { "[16]000100", 0, "16000100", "" },
        /* 59*/ { "[16]00000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (16)" },
        /* 60*/ { "[160]000000", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (160)" },
        /* 61*/ { "[1600]000000", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1600)" },
        /* 62*/ { "[17]010200", 0, "17010200", "" },
        /* 63*/ { "[17]0102000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (17)" },
        /* 64*/ { "[170]010200", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (170)" },
        /* 65*/ { "[1700]010200", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1700)" },
        /* 66*/ { "[18]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (18)" },
        /* 67*/ { "[180]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (180)" },
        /* 68*/ { "[1800]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1800)" },
        /* 69*/ { "[19]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (19)" },
        /* 70*/ { "[190]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (190)" },
        /* 71*/ { "[1900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (1900)" },
        /* 72*/ { "[20]12", 0, "2012", "" },
        /* 73*/ { "[20]1", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (20)" },
        /* 74*/ { "[200]12", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (200)" },
        /* 75*/ { "[2000]12", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2000)" },
        /* 76*/ { "[21]A12345678", 0, "21A12345678", "" },
        /* 77*/ { "[21]123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (21)" },
        /* 78*/ { "[210]A12345678", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (210)" },
        /* 79*/ { "[2100]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2100)" },
        /* 80*/ { "[22]abcdefghijklmnopqrst", 0, "22abcdefghijklmnopqrst", "" },
        /* 81*/ { "[22]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (22)" },
        /* 82*/ { "[220]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (220)" },
        /* 83*/ { "[2200]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2200)" },
        /* 84*/ { "[23]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (23)" },
        /* 85*/ { "[230]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (230)" },
        /* 86*/ { "[2300]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2300)" },
        /* 87*/ { "[235]1abcdefghijklmnopqrstuvwxyz0", 0, "2351abcdefghijklmnopqrstuvwxyz0", "" },
        /* 88*/ { "[235]1abcdefghijklmnopqrstuvwxyz01", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (235)" },
        /* 89*/ { "[24]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (24)" },
        /* 90*/ { "[240]abcdefghijklmnopqrstuvwxyz1234", 0, "240abcdefghijklmnopqrstuvwxyz1234", "" },
        /* 91*/ { "[240]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (240)" },
        /* 92*/ { "[2400]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2400)" },
        /* 93*/ { "[241]abcdefghijklmnopqrstuvwxyz1234", 0, "241abcdefghijklmnopqrstuvwxyz1234", "" },
        /* 94*/ { "[241]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (241)" },
        /* 95*/ { "[2410]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2410)" },
        /* 96*/ { "[242]12345", 0, "24212345", "" },
        /* 97*/ { "[242]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (242)" },
        /* 98*/ { "[2420]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2420)" },
        /* 99*/ { "[243]abcdefghijklmnopqrst", 0, "243abcdefghijklmnopqrst", "" },
        /*100*/ { "[243]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (243)" },
        /*101*/ { "[2430]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2430)" },
        /*102*/ { "[244]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (244)" },
        /*103*/ { "[2440]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2440)" },
        /*104*/ { "[249]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (249)" },
        /*105*/ { "[2490]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2490)" },
        /*106*/ { "[25]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (25)" },
        /*107*/ { "[250]abcdefghijklmnopqrstuvwxyz1234", 0, "250abcdefghijklmnopqrstuvwxyz1234", "" },
        /*108*/ { "[250]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (250)" },
        /*109*/ { "[2500]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2500)" },
        /*110*/ { "[251]abcdefghijklmnopqrstuvwxyz1234", 0, "251abcdefghijklmnopqrstuvwxyz1234", "" },
        /*111*/ { "[251]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (251)" },
        /*112*/ { "[2510]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2510)" },
        /*113*/ { "[252]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (252)" },
        /*114*/ { "[2520]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2520)" },
        /*115*/ { "[253]131313131313", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (253)" },
        /*116*/ { "[253]1313131313134", ZINT_WARN_NONCOMPLIANT, "2531313131313134", "261: AI (253) position 13: Bad checksum '4', expected '0'" },
        /*117*/ { "[253]1313131313130", 0, "2531313131313130", "" },
        /*118*/ { "[253]131313131313412345678901234567", ZINT_WARN_NONCOMPLIANT, "253131313131313412345678901234567", "261: AI (253) position 13: Bad checksum '4', expected '0'" },
        /*119*/ { "[253]131313131313012345678901234567", 0, "253131313131313012345678901234567", "" },
        /*120*/ { "[253]1313131313134123456789012345678", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (253)" },
        /*121*/ { "[2530]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2530)" },
        /*122*/ { "[254]abcdefghijklmnopqrst", 0, "254abcdefghijklmnopqrst", "" },
        /*123*/ { "[254]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (254)" },
        /*124*/ { "[2540]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2540)" },
        /*125*/ { "[255]131313131313", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (255)" },
        /*126*/ { "[255]1313131313134", ZINT_WARN_NONCOMPLIANT, "2551313131313134", "261: AI (255) position 13: Bad checksum '4', expected '0'" },
        /*127*/ { "[255]1313131313130", 0, "2551313131313130", "" },
        /*128*/ { "[255]1313131313134123456789012", ZINT_WARN_NONCOMPLIANT, "2551313131313134123456789012", "261: AI (255) position 13: Bad checksum '4', expected '0'" },
        /*129*/ { "[255]1313131313130123456789012", 0, "2551313131313130123456789012", "" },
        /*130*/ { "[255]13131313131341234567890123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (255)" },
        /*131*/ { "[2550]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2550)" },
        /*132*/ { "[256]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (256)" },
        /*133*/ { "[2560]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2560)" },
        /*134*/ { "[259]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (259)" },
        /*135*/ { "[2590]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2590)" },
        /*136*/ { "[26]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (26)" },
        /*137*/ { "[260]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (260)" },
        /*138*/ { "[2600]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2600)" },
        /*139*/ { "[27]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (27)" },
        /*140*/ { "[270]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (270)" },
        /*141*/ { "[2700]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2700)" },
        /*142*/ { "[28]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (28)" },
        /*143*/ { "[280]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (280)" },
        /*144*/ { "[2800]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2800)" },
        /*145*/ { "[29]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (29)" },
        /*146*/ { "[290]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (290)" },
        /*147*/ { "[2900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (2900)" },
        /*148*/ { "[30]12345678", 0, "3012345678", "" },
        /*149*/ { "[30]123456789", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (30)" },
        /*150*/ { "[300]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (300)" },
        /*151*/ { "[3000]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3000)" },
        /*152*/ { "[31]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (31)" },
        /*153*/ { "[310]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (310)" },
        /*154*/ { "[3100]123456", 0, "3100123456", "" },
        /*155*/ { "[3100]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3100)" },
        /*156*/ { "[3101]123456", 0, "3101123456", "" },
        /*157*/ { "[3101]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3101)" },
        /*158*/ { "[3105]123456", 0, "3105123456", "" },
        /*159*/ { "[3105]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3105)" },
        /*160*/ { "[3106]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3106)" },
        /*161*/ { "[3109]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3109)" },
        /*162*/ { "[3110]123456", 0, "3110123456", "" },
        /*163*/ { "[3110]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3110)" },
        /*164*/ { "[3115]123456", 0, "3115123456", "" },
        /*165*/ { "[3115]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3115)" },
        /*166*/ { "[3116]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3116)" },
        /*167*/ { "[3119]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3119)" },
        /*168*/ { "[3120]123456", 0, "3120123456", "" },
        /*169*/ { "[3120]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3120)" },
        /*170*/ { "[3125]123456", 0, "3125123456", "" },
        /*171*/ { "[3125]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3125)" },
        /*172*/ { "[3126]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3126)" },
        /*173*/ { "[3129]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3129)" },
        /*174*/ { "[3130]123456", 0, "3130123456", "" },
        /*175*/ { "[3130]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3130)" },
        /*176*/ { "[3135]123456", 0, "3135123456", "" },
        /*177*/ { "[3135]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3135)" },
        /*178*/ { "[3136]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3136)" },
        /*179*/ { "[3139]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3139)" },
        /*180*/ { "[3140]123456", 0, "3140123456", "" },
        /*181*/ { "[3140]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3140)" },
        /*182*/ { "[3145]123456", 0, "3145123456", "" },
        /*183*/ { "[3145]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3145)" },
        /*184*/ { "[3146]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3146)" },
        /*185*/ { "[3149]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3149)" },
        /*186*/ { "[3150]123456", 0, "3150123456", "" },
        /*187*/ { "[3150]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3150)" },
        /*188*/ { "[3155]123456", 0, "3155123456", "" },
        /*189*/ { "[3155]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3155)" },
        /*190*/ { "[3156]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3156)" },
        /*191*/ { "[3159]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3159)" },
        /*192*/ { "[3160]123456", 0, "3160123456", "" },
        /*193*/ { "[3160]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3160)" },
        /*194*/ { "[3165]123456", 0, "3165123456", "" },
        /*195*/ { "[3165]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3165)" },
        /*196*/ { "[3166]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3166)" },
        /*197*/ { "[3169]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3169)" },
        /*198*/ { "[3170]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3170)" },
        /*199*/ { "[3179]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3179)" },
        /*200*/ { "[3180]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3180)" },
        /*201*/ { "[3189]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3189)" },
        /*202*/ { "[3190]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3190)" },
        /*203*/ { "[3199]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3199)" },
        /*204*/ { "[32]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (32)" },
        /*205*/ { "[320]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (320)" },
        /*206*/ { "[3200]123456", 0, "3200123456", "" },
        /*207*/ { "[3200]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3200)" },
        /*208*/ { "[3205]123456", 0, "3205123456", "" },
        /*209*/ { "[3205]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3205)" },
        /*210*/ { "[3206]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3206)" },
        /*211*/ { "[3209]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3209)" },
        /*212*/ { "[3210]123456", 0, "3210123456", "" },
        /*213*/ { "[3210]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3210)" },
        /*214*/ { "[3215]123456", 0, "3215123456", "" },
        /*215*/ { "[3215]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3215)" },
        /*216*/ { "[3216]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3216)" },
        /*217*/ { "[3219]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3219)" },
        /*218*/ { "[3220]123456", 0, "3220123456", "" },
        /*219*/ { "[3220]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3220)" },
        /*220*/ { "[3225]123456", 0, "3225123456", "" },
        /*221*/ { "[3225]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3225)" },
        /*222*/ { "[3229]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3229)" },
        /*223*/ { "[3230]123456", 0, "3230123456", "" },
        /*224*/ { "[3230]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3230)" },
        /*225*/ { "[3235]123456", 0, "3235123456", "" },
        /*226*/ { "[3235]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3235)" },
        /*227*/ { "[3239]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3239)" },
        /*228*/ { "[3240]123456", 0, "3240123456", "" },
        /*229*/ { "[3240]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3240)" },
        /*230*/ { "[3245]123456", 0, "3245123456", "" },
        /*231*/ { "[3245]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3245)" },
        /*232*/ { "[3249]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3249)" },
        /*233*/ { "[3250]123456", 0, "3250123456", "" },
        /*234*/ { "[3250]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3250)" },
        /*235*/ { "[3255]123456", 0, "3255123456", "" },
        /*236*/ { "[3255]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3255)" },
        /*237*/ { "[3259]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3259)" },
        /*238*/ { "[3260]123456", 0, "3260123456", "" },
        /*239*/ { "[3260]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3260)" },
        /*240*/ { "[3265]123456", 0, "3265123456", "" },
        /*241*/ { "[3265]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3265)" },
        /*242*/ { "[3269]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3269)" },
        /*243*/ { "[3270]123456", 0, "3270123456", "" },
        /*244*/ { "[3270]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3270)" },
        /*245*/ { "[3275]123456", 0, "3275123456", "" },
        /*246*/ { "[3275]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3275)" },
        /*247*/ { "[3279]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3279)" },
        /*248*/ { "[3280]123456", 0, "3280123456", "" },
        /*249*/ { "[3280]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3280)" },
        /*250*/ { "[3285]123456", 0, "3285123456", "" },
        /*251*/ { "[3285]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3285)" },
        /*252*/ { "[3289]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3289)" },
        /*253*/ { "[3290]123456", 0, "3290123456", "" },
        /*254*/ { "[3290]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3290)" },
        /*255*/ { "[3295]123456", 0, "3295123456", "" },
        /*256*/ { "[3295]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3295)" },
        /*257*/ { "[3296]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3296)" },
        /*258*/ { "[3299]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3299)" },
        /*259*/ { "[33]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (33)" },
        /*260*/ { "[330]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (330)" },
        /*261*/ { "[3300]123456", 0, "3300123456", "" },
        /*262*/ { "[3300]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3300)" },
        /*263*/ { "[3305]123456", 0, "3305123456", "" },
        /*264*/ { "[3305]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3305)" },
        /*265*/ { "[3306]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3306)" },
        /*266*/ { "[3309]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3309)" },
        /*267*/ { "[3310]123456", 0, "3310123456", "" },
        /*268*/ { "[3310]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3310)" },
        /*269*/ { "[3319]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3319)" },
        /*270*/ { "[3320]123456", 0, "3320123456", "" },
        /*271*/ { "[3320]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3320)" },
        /*272*/ { "[3329]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3329)" },
        /*273*/ { "[3330]123456", 0, "3330123456", "" },
        /*274*/ { "[3330]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3330)" },
        /*275*/ { "[3339]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3339)" },
        /*276*/ { "[3340]123456", 0, "3340123456", "" },
        /*277*/ { "[3340]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3340)" },
        /*278*/ { "[3349]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3349)" },
        /*279*/ { "[3350]123456", 0, "3350123456", "" },
        /*280*/ { "[3350]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3350)" },
        /*281*/ { "[3359]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3359)" },
        /*282*/ { "[3360]123456", 0, "3360123456", "" },
        /*283*/ { "[3360]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3360)" },
        /*284*/ { "[3369]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3369)" },
        /*285*/ { "[3370]123456", 0, "3370123456", "" },
        /*286*/ { "[3370]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3370)" },
        /*287*/ { "[3375]123456", 0, "3375123456", "" },
        /*288*/ { "[3375]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3375)" },
        /*289*/ { "[3376]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3376)" },
        /*290*/ { "[3379]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3379)" },
        /*291*/ { "[3380]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3380)" },
        /*292*/ { "[3390]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3390)" },
        /*293*/ { "[3399]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3399)" },
        /*294*/ { "[34]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (34)" },
        /*295*/ { "[340]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (340)" },
        /*296*/ { "[3400]123456", 0, "3400123456", "" },
        /*297*/ { "[3400]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3400)" },
        /*298*/ { "[3405]123456", 0, "3405123456", "" },
        /*299*/ { "[3405]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3405)" },
        /*300*/ { "[3406]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3406)" },
        /*301*/ { "[3409]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3409)" },
        /*302*/ { "[3410]123456", 0, "3410123456", "" },
        /*303*/ { "[3410]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3410)" },
        /*304*/ { "[3419]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3419)" },
        /*305*/ { "[3420]123456", 0, "3420123456", "" },
        /*306*/ { "[3420]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3420)" },
        /*307*/ { "[3429]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3429)" },
        /*308*/ { "[3430]123456", 0, "3430123456", "" },
        /*309*/ { "[3430]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3430)" },
        /*310*/ { "[3439]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3439)" },
        /*311*/ { "[3440]123456", 0, "3440123456", "" },
        /*312*/ { "[3440]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3440)" },
        /*313*/ { "[3449]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3449)" },
        /*314*/ { "[3450]123456", 0, "3450123456", "" },
        /*315*/ { "[3450]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3450)" },
        /*316*/ { "[3459]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3459)" },
        /*317*/ { "[3460]123456", 0, "3460123456", "" },
        /*318*/ { "[3460]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3460)" },
        /*319*/ { "[3469]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3469)" },
        /*320*/ { "[3470]123456", 0, "3470123456", "" },
        /*321*/ { "[3470]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3470)" },
        /*322*/ { "[3479]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3479)" },
        /*323*/ { "[3480]123456", 0, "3480123456", "" },
        /*324*/ { "[3480]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3480)" },
        /*325*/ { "[3489]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3489)" },
        /*326*/ { "[3490]123456", 0, "3490123456", "" },
        /*327*/ { "[3490]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3490)" },
        /*328*/ { "[3495]123456", 0, "3495123456", "" },
        /*329*/ { "[3495]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3495)" },
        /*330*/ { "[3496]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3496)" },
        /*331*/ { "[3499]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3499)" },
        /*332*/ { "[35]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (35)" },
        /*333*/ { "[350]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (350)" },
        /*334*/ { "[3500]123456", 0, "3500123456", "" },
        /*335*/ { "[3500]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3500)" },
        /*336*/ { "[3505]123456", 0, "3505123456", "" },
        /*337*/ { "[3505]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3505)" },
        /*338*/ { "[3506]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3506)" },
        /*339*/ { "[3509]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3509)" },
        /*340*/ { "[3510]123456", 0, "3510123456", "" },
        /*341*/ { "[3510]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3510)" },
        /*342*/ { "[3519]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3519)" },
        /*343*/ { "[3520]123456", 0, "3520123456", "" },
        /*344*/ { "[3520]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3520)" },
        /*345*/ { "[3529]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3529)" },
        /*346*/ { "[3530]123456", 0, "3530123456", "" },
        /*347*/ { "[3530]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3530)" },
        /*348*/ { "[3539]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3539)" },
        /*349*/ { "[3540]123456", 0, "3540123456", "" },
        /*350*/ { "[3540]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3540)" },
        /*351*/ { "[3549]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3549)" },
        /*352*/ { "[3550]123456", 0, "3550123456", "" },
        /*353*/ { "[3550]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3550)" },
        /*354*/ { "[3559]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3559)" },
        /*355*/ { "[3560]123456", 0, "3560123456", "" },
        /*356*/ { "[3560]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3560)" },
        /*357*/ { "[3569]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3569)" },
        /*358*/ { "[3570]123456", 0, "3570123456", "" },
        /*359*/ { "[3570]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3570)" },
        /*360*/ { "[3575]123456", 0, "3575123456", "" },
        /*361*/ { "[3376]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3376)" },
        /*362*/ { "[3579]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3579)" },
        /*363*/ { "[3580]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3580)" },
        /*364*/ { "[3590]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3590)" },
        /*365*/ { "[3599]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3599)" },
        /*366*/ { "[36]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (36)" },
        /*367*/ { "[360]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (360)" },
        /*368*/ { "[3600]123456", 0, "3600123456", "" },
        /*369*/ { "[3600]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3600)" },
        /*370*/ { "[3605]123456", 0, "3605123456", "" },
        /*371*/ { "[3605]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3605)" },
        /*372*/ { "[3606]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3606)" },
        /*373*/ { "[3609]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3609)" },
        /*374*/ { "[3610]123456", 0, "3610123456", "" },
        /*375*/ { "[3610]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3610)" },
        /*376*/ { "[3619]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3619)" },
        /*377*/ { "[3620]123456", 0, "3620123456", "" },
        /*378*/ { "[3620]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3620)" },
        /*379*/ { "[3629]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3629)" },
        /*380*/ { "[3630]123456", 0, "3630123456", "" },
        /*381*/ { "[3630]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3630)" },
        /*382*/ { "[3639]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3639)" },
        /*383*/ { "[3640]123456", 0, "3640123456", "" },
        /*384*/ { "[3640]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3640)" },
        /*385*/ { "[3649]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3649)" },
        /*386*/ { "[3650]123456", 0, "3650123456", "" },
        /*387*/ { "[3650]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3650)" },
        /*388*/ { "[3659]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3659)" },
        /*389*/ { "[3660]123456", 0, "3660123456", "" },
        /*390*/ { "[3660]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3660)" },
        /*391*/ { "[3669]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3669)" },
        /*392*/ { "[3670]123456", 0, "3670123456", "" },
        /*393*/ { "[3670]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3670)" },
        /*394*/ { "[3679]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3679)" },
        /*395*/ { "[3680]123456", 0, "3680123456", "" },
        /*396*/ { "[3680]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3680)" },
        /*397*/ { "[3689]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3689)" },
        /*398*/ { "[3690]123456", 0, "3690123456", "" },
        /*399*/ { "[3690]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3690)" },
        /*400*/ { "[3695]123456", 0, "3695123456", "" },
        /*401*/ { "[3695]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3695)" },
        /*402*/ { "[3696]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3696)" },
        /*403*/ { "[3699]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3699)" },
        /*404*/ { "[37]12345678", 0, "3712345678", "" },
        /*405*/ { "[37]123456789", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (37)" },
        /*406*/ { "[370]12345678", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (370)" },
        /*407*/ { "[3700]12345678", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3700)" },
        /*408*/ { "[38]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (38)" },
        /*409*/ { "[380]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (380)" },
        /*410*/ { "[3800]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3800)" },
        /*411*/ { "[39]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (39)" },
        /*412*/ { "[390]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (390)" },
        /*413*/ { "[3900]123456789012345", 0, "3900123456789012345", "" },
        /*414*/ { "[3900]1234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3900)" },
        /*415*/ { "[3900]12345678901234", 0, "390012345678901234", "" },
        /*416*/ { "[3901]123456789012345", 0, "3901123456789012345", "" },
        /*417*/ { "[3901]1234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3901)" },
        /*418*/ { "[3905]123456789012345", 0, "3905123456789012345", "" },
        /*419*/ { "[3909]123456789012345", 0, "3909123456789012345", "" },
        /*420*/ { "[3909]1234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3909)" },
        /*421*/ { "[3910]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3910123123456789012345", "261: AI (3910) position 1: Unknown currency code '123'" },
        /*422*/ { "[3910]997123456789012345", 0, "3910997123456789012345", "" },
        /*423*/ { "[3910]1231234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3910)" },
        /*424*/ { "[3910]123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3910)" },
        /*425*/ { "[3915]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3915123123456789012345", "261: AI (3915) position 1: Unknown currency code '123'" },
        /*426*/ { "[3915]997123456789012345", 0, "3915997123456789012345", "" },
        /*427*/ { "[3915]1231234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3915)" },
        /*428*/ { "[3919]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3919123123456789012345", "261: AI (3919) position 1: Unknown currency code '123'" },
        /*429*/ { "[3919]997123456789012345", 0, "3919997123456789012345", "" },
        /*430*/ { "[3919]1231234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3919)" },
        /*431*/ { "[3920]123456789012345", 0, "3920123456789012345", "" },
        /*432*/ { "[3920]1234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3920)" },
        /*433*/ { "[3925]123456789012345", 0, "3925123456789012345", "" },
        /*434*/ { "[3925]1234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3925)" },
        /*435*/ { "[3929]123456789012345", 0, "3929123456789012345", "" },
        /*436*/ { "[3929]1234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3929)" },
        /*437*/ { "[3930]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3930123123456789012345", "261: AI (3930) position 1: Unknown currency code '123'" },
        /*438*/ { "[3930]997123456789012345", 0, "3930997123456789012345", "" },
        /*439*/ { "[3930]1231234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3930)" },
        /*440*/ { "[3930]123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3930)" },
        /*441*/ { "[3935]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3935123123456789012345", "261: AI (3935) position 1: Unknown currency code '123'" },
        /*442*/ { "[3935]997123456789012345", 0, "3935997123456789012345", "" },
        /*443*/ { "[3935]1231234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3935)" },
        /*444*/ { "[3939]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3939123123456789012345", "261: AI (3939) position 1: Unknown currency code '123'" },
        /*445*/ { "[3939]997123456789012345", 0, "3939997123456789012345", "" },
        /*446*/ { "[3939]1231234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3939)" },
        /*447*/ { "[3940]1234", 0, "39401234", "" },
        /*448*/ { "[3940]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3940)" },
        /*449*/ { "[3940]123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3940)" },
        /*450*/ { "[3941]1234", 0, "39411234", "" },
        /*451*/ { "[3941]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3941)" },
        /*452*/ { "[3941]123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3941)" },
        /*453*/ { "[3942]1234", 0, "39421234", "" },
        /*454*/ { "[3942]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3942)" },
        /*455*/ { "[3943]1234", 0, "39431234", "" },
        /*456*/ { "[3943]123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3943)" },
        /*457*/ { "[3944]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3944)" },
        /*458*/ { "[3945]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3945)" },
        /*459*/ { "[3949]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3949)" },
        /*460*/ { "[3950]123456", 0, "3950123456", "" },
        /*461*/ { "[3950]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3950)" },
        /*462*/ { "[3950]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3950)" },
        /*463*/ { "[3951]123456", 0, "3951123456", "" },
        /*464*/ { "[3951]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3951)" },
        /*465*/ { "[3952]123456", 0, "3952123456", "" },
        /*466*/ { "[3952]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3952)" },
        /*467*/ { "[3953]123456", 0, "3953123456", "" },
        /*468*/ { "[3953]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3953)" },
        /*469*/ { "[3954]123456", 0, "3954123456", "" },
        /*470*/ { "[3954]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3954)" },
        /*471*/ { "[3955]123456", 0, "3955123456", "" },
        /*472*/ { "[3955]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3955)" },
        /*473*/ { "[3956]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3956)" },
        /*474*/ { "[3959]123456", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3959)" },
        /*475*/ { "[3960]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3960)" },
        /*476*/ { "[3970]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3970)" },
        /*477*/ { "[3980]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3980)" },
        /*478*/ { "[3999]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (3999)" },
        /*479*/ { "[40]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (40)" },
        /*480*/ { "[400]123456789012345678901234567890", 0, "400123456789012345678901234567890", "" },
        /*481*/ { "[400]1234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (400)" },
        /*482*/ { "[4000]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4000)" },
        /*483*/ { "[401]1234abcdefghijklmnopqrstuvwxyz", 0, "4011234abcdefghijklmnopqrstuvwxyz", "" },
        /*484*/ { "[401]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (401)" },
        /*485*/ { "[4010]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4010)" },
        /*486*/ { "[402]13131313131313132", ZINT_WARN_NONCOMPLIANT, "40213131313131313132", "261: AI (402) position 17: Bad checksum '2', expected '0'" },
        /*487*/ { "[402]13131313131313130", 0, "40213131313131313130", "" },
        /*488*/ { "[402]1313131313131313", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (402)" },
        /*489*/ { "[4020]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4020)" },
        /*490*/ { "[403]abcdefghijklmnopqrstuvwxyz1234", 0, "403abcdefghijklmnopqrstuvwxyz1234", "" },
        /*491*/ { "[403]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (403)" },
        /*492*/ { "[4030]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4030)" },
        /*493*/ { "[404]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (404)" },
        /*494*/ { "[4040]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4040)" },
        /*495*/ { "[409]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (409)" },
        /*496*/ { "[4090]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4090)" },
        /*497*/ { "[41]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (41)" },
        /*498*/ { "[410]3898765432108", 0, "4103898765432108", "" },
        /*499*/ { "[410]12345678901234", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (410)" },
        /*500*/ { "[4100]12345678901234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4100)" },
        /*501*/ { "[411]1313131313134", ZINT_WARN_NONCOMPLIANT, "4111313131313134", "261: AI (411) position 13: Bad checksum '4', expected '0'" },
        /*502*/ { "[411]1313131313130", 0, "4111313131313130", "" },
        /*503*/ { "[411]13131313131", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (411)" },
        /*504*/ { "[4110]1313131313134", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4110)" },
        /*505*/ { "[412]1313131313130", 0, "4121313131313130", "" },
        /*506*/ { "[412]13131313131", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (412)" },
        /*507*/ { "[4120]1313131313134", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4120)" },
        /*508*/ { "[413]1313131313130", 0, "4131313131313130", "" },
        /*509*/ { "[413]13131313131", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (413)" },
        /*510*/ { "[4130]1313131313134", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4130)" },
        /*511*/ { "[414]1313131313130", 0, "4141313131313130", "" },
        /*512*/ { "[414]13131313131", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (414)" },
        /*513*/ { "[4140]1313131313134", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4140)" },
        /*514*/ { "[415]1313131313130", 0, "4151313131313130", "" },
        /*515*/ { "[415]13131313131", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (415)" },
        /*516*/ { "[4150]1313131313134", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4150)" },
        /*517*/ { "[416]1313131313130", 0, "4161313131313130", "" },
        /*518*/ { "[416]13131313131", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (416)" },
        /*519*/ { "[4160]1313131313134", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4160)" },
        /*520*/ { "[417]1313131313130", 0, "4171313131313130", "" },
        /*521*/ { "[417]13131313131", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (417)" },
        /*522*/ { "[4170]1313131313134", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4170)" },
        /*523*/ { "[418]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (418)" },
        /*524*/ { "[4180]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4180)" },
        /*525*/ { "[419]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (419)" },
        /*526*/ { "[4190]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4190)" },
        /*527*/ { "[42]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (42)" },
        /*528*/ { "[420]abcdefghijklmnopqrst", 0, "420abcdefghijklmnopqrst", "" },
        /*529*/ { "[420]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (420)" },
        /*530*/ { "[4200]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4200)" },
        /*531*/ { "[421]123abcdefghi", ZINT_WARN_NONCOMPLIANT, "421123abcdefghi", "261: AI (421) position 1: Unknown country code '123'" },
        /*532*/ { "[421]434abcdefghi", 0, "421434abcdefghi", "" },
        /*533*/ { "[421]123abcdefghij", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (421)" },
        /*534*/ { "[421]1231", ZINT_WARN_NONCOMPLIANT, "4211231", "261: AI (421) position 1: Unknown country code '123'" },
        /*535*/ { "[421]4341", 0, "4214341", "" },
        /*536*/ { "[421]123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (421)" },
        /*537*/ { "[4210]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4210)" },
        /*538*/ { "[422]123", ZINT_WARN_NONCOMPLIANT, "422123", "261: AI (422) position 1: Unknown country code '123'" },
        /*539*/ { "[422]004", 0, "422004", "" },
        /*540*/ { "[422]1234", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (422)" },
        /*541*/ { "[422]12", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (422)" },
        /*542*/ { "[4220]123", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4220)" },
        /*543*/ { "[423]123123123123123", ZINT_WARN_NONCOMPLIANT, "423123123123123123", "261: AI (423) position 1: Unknown country code '123'" },
        /*544*/ { "[423]470004012887123", ZINT_WARN_NONCOMPLIANT, "423470004012887123", "261: AI (423) position 13: Unknown country code '123'" },
        /*545*/ { "[423]470004012887438", 0, "423470004012887438", "" },
        /*546*/ { "[423]1231231231231231", ZINT_ERROR_INVALID_DATA, "4231231231231231231", "259: Invalid data length for AI (423)" },
        /*547*/ { "[423]12312312312312", ZINT_WARN_NONCOMPLIANT, "42312312312312312", "259: Invalid data length for AI (423)" },
        /*548*/ { "[423]1231231231231", ZINT_WARN_NONCOMPLIANT, "4231231231231231", "259: Invalid data length for AI (423)" },
        /*549*/ { "[423]12312312312", ZINT_WARN_NONCOMPLIANT, "42312312312312", "259: Invalid data length for AI (423)" },
        /*550*/ { "[423]1231231231", ZINT_WARN_NONCOMPLIANT, "4231231231231", "259: Invalid data length for AI (423)" },
        /*551*/ { "[423]123", ZINT_WARN_NONCOMPLIANT, "423123", "261: AI (423) position 1: Unknown country code '123'" },
        /*552*/ { "[423]004", 0, "423004", "" },
        /*553*/ { "[423]12", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" },
        /*554*/ { "[4230]123123123123123", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4230)" },
        /*555*/ { "[424]123", ZINT_WARN_NONCOMPLIANT, "424123", "261: AI (424) position 1: Unknown country code '123'" },
        /*556*/ { "[424]004", 0, "424004", "" },
        /*557*/ { "[424]1234", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (424)" },
        /*558*/ { "[424]12", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (424)" },
        /*559*/ { "[4240]123", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4240)" },
        /*560*/ { "[425]123123123123123", ZINT_WARN_NONCOMPLIANT, "425123123123123123", "261: AI (425) position 1: Unknown country code '123'" },
        /*561*/ { "[425]010500276634894", 0, "425010500276634894", "" },
        /*562*/ { "[425]010500276123894", ZINT_WARN_NONCOMPLIANT, "425010500276123894", "261: AI (425) position 10: Unknown country code '123'" },
        /*563*/ { "[425]1231231231231231", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (425)" },
        /*564*/ { "[425]12", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (425)" },
        /*565*/ { "[4250]123123123123123", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4250)" },
        /*566*/ { "[426]123", ZINT_WARN_NONCOMPLIANT, "426123", "261: AI (426) position 1: Unknown country code '123'" },
        /*567*/ { "[426]426", 0, "426426", "" },
        /*568*/ { "[426]1234", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (426)" },
        /*569*/ { "[426]12", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (426)" },
        /*570*/ { "[4260]123", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4260)" },
        /*571*/ { "[427]abc", 0, "427abc", "" },
        /*572*/ { "[427]abcd", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (427)" },
        /*573*/ { "[4270]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4270)" },
        /*574*/ { "[428]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (428)" },
        /*575*/ { "[4280]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4280)" },
        /*576*/ { "[429]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (429)" },
        /*577*/ { "[4290]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4290)" },
        /*578*/ { "[43]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (43)" },
        /*579*/ { "[430]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (430)" },
        /*580*/ { "[4300]1", 0, "43001", "" },
        /*581*/ { "[4300]12345678901234567890123456789012345", 0, "430012345678901234567890123456789012345", "" },
        /*582*/ { "[4300]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4300)" },
        /*583*/ { "[4301]1", 0, "43011", "" },
        /*584*/ { "[4301]12345678901234567890123456789012345", 0, "430112345678901234567890123456789012345", "" },
        /*585*/ { "[4301]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4301)" },
        /*586*/ { "[4302]1", 0, "43021", "" },
        /*587*/ { "[4302]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43021234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*588*/ { "[4302]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4302)" },
        /*589*/ { "[4303]1", 0, "43031", "" },
        /*590*/ { "[4303]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43031234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*591*/ { "[4303]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4303)" },
        /*592*/ { "[4304]1", 0, "43041", "" },
        /*593*/ { "[4304]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43041234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*594*/ { "[4304]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4304)" },
        /*595*/ { "[4305]1", 0, "43051", "" },
        /*596*/ { "[4305]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43051234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*597*/ { "[4305]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4305)" },
        /*598*/ { "[4306]1", 0, "43061", "" },
        /*599*/ { "[4306]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43061234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*600*/ { "[4306]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4306)" },
        /*601*/ { "[4307]FR", 0, "4307FR", "" },
        /*602*/ { "[4307]F", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4307)" },
        /*603*/ { "[4307]FRR", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4307)" },
        /*604*/ { "[4308]1", 0, "43081", "" },
        /*605*/ { "[4308]123456789012345678901234567890", 0, "4308123456789012345678901234567890", "" },
        /*606*/ { "[4308]1234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4308)" },
        /*607*/ { "[4309]12345678901234567890", 0, "430912345678901234567890", "" },
        /*608*/ { "[4309]1234567890123456789", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4309)" },
        /*609*/ { "[4309]1234567890123456789A", ZINT_WARN_NONCOMPLIANT, "43091234567890123456789A", "261: AI (4309) position 20: Non-numeric character 'A'" },
        /*610*/ { "[431]1", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (431)" },
        /*611*/ { "[4310]1", 0, "43101", "" },
        /*612*/ { "[4310]12345678901234567890123456789012345", 0, "431012345678901234567890123456789012345", "" },
        /*613*/ { "[4310]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4310)" },
        /*614*/ { "[4311]1", 0, "43111", "" },
        /*615*/ { "[4311]12345678901234567890123456789012345", 0, "431112345678901234567890123456789012345", "" },
        /*616*/ { "[4311]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4311)" },
        /*617*/ { "[4312]1", 0, "43121", "" },
        /*618*/ { "[4312]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43121234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*619*/ { "[4312]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4312)" },
        /*620*/ { "[4313]1", 0, "43131", "" },
        /*621*/ { "[4313]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43131234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*622*/ { "[4313]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4313)" },
        /*623*/ { "[4314]1", 0, "43141", "" },
        /*624*/ { "[4314]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43141234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*625*/ { "[4314]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4314)" },
        /*626*/ { "[4315]1", 0, "43151", "" },
        /*627*/ { "[4315]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43151234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*628*/ { "[4315]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4315)" },
        /*629*/ { "[4316]1", 0, "43161", "" },
        /*630*/ { "[4316]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43161234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*631*/ { "[4316]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4316)" },
        /*632*/ { "[4317]FR", 0, "4317FR", "" },
        /*633*/ { "[4317]F", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4317)" },
        /*634*/ { "[4317]FRF", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4317)" },
        /*635*/ { "[4318]1", 0, "43181", "" },
        /*636*/ { "[4318]12345678901234567890", 0, "431812345678901234567890", "" },
        /*637*/ { "[4318]123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4318)" },
        /*638*/ { "[4319]1", 0, "43191", "" },
        /*639*/ { "[4319]123456789012345678901234567890", 0, "4319123456789012345678901234567890", "" },
        /*640*/ { "[4319]1234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4319)" },
        /*641*/ { "[432]1", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (432)" },
        /*642*/ { "[4320]1", 0, "43201", "" },
        /*643*/ { "[4320]12345678901234567890123456789012345", 0, "432012345678901234567890123456789012345", "" },
        /*644*/ { "[4320]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4320)" },
        /*645*/ { "[4321]1", 0, "43211", "" },
        /*646*/ { "[4321]10", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4321)" },
        /*647*/ { "[4322]1", 0, "43221", "" },
        /*648*/ { "[4322]10", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4322)" },
        /*649*/ { "[4323]1", 0, "43231", "" },
        /*650*/ { "[4323]10", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4323)" },
        /*651*/ { "[4324]1212120000", 0, "43241212120000", "" },
        /*652*/ { "[4324]121212000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4324)" },
        /*653*/ { "[4324]12121200000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4324)" },
        /*654*/ { "[4325]1212120000", 0, "43251212120000", "" },
        /*655*/ { "[4325]121212000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4325)" },
        /*656*/ { "[4325]12121200000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4325)" },
        /*657*/ { "[4326]121212", 0, "4326121212", "" },
        /*658*/ { "[4326]12121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4326)" },
        /*659*/ { "[4326]1212120", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4326)" },
        /*660*/ { "[4327]121212", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4327)" },
        /*661*/ { "[4328]121212", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4328)" },
        /*662*/ { "[4329]121212", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4329)" },
        /*663*/ { "[433]121212", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (433)" },
        /*664*/ { "[4330]121212", 0, "4330121212", "" },
        /*665*/ { "[4331]121212-", 0, "4331121212-", "" },
        /*666*/ { "[4332]121212", 0, "4332121212", "" },
        /*667*/ { "[4333]121212-", 0, "4333121212-", "" },
        /*668*/ { "[4334]121212", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4334)" },
        /*669*/ { "[44]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (44)" },
        /*670*/ { "[440]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (440)" },
        /*671*/ { "[4400]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4400)" },
        /*672*/ { "[49]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (49)" },
        /*673*/ { "[490]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (490)" },
        /*674*/ { "[4900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4900)" },
        /*675*/ { "[499]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (499)" },
        /*676*/ { "[4990]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (4990)" },
        /*677*/ { "[50]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (50)" },
        /*678*/ { "[500]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (500)" },
        /*679*/ { "[5000]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (5000)" },
        /*680*/ { "[51]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (51)" },
        /*681*/ { "[510]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (510)" },
        /*682*/ { "[5100]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (5100)" },
        /*683*/ { "[59]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (59)" },
        /*684*/ { "[590]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (590)" },
        /*685*/ { "[5900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (5900)" },
        /*686*/ { "[60]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (60)" },
        /*687*/ { "[600]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (600)" },
        /*688*/ { "[6000]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (6000)" },
        /*689*/ { "[61]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (61)" },
        /*690*/ { "[610]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (610)" },
        /*691*/ { "[6100]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (6100)" },
        /*692*/ { "[69]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (69)" },
        /*693*/ { "[690]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (690)" },
        /*694*/ { "[6900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (6900)" },
        /*695*/ { "[70]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (70)" },
        /*696*/ { "[700]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (700)" },
        /*697*/ { "[7000]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7000)" },
        /*698*/ { "[7001]1234567890123", 0, "70011234567890123", "" },
        /*699*/ { "[7001]123456789012", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7001)" },
        /*700*/ { "[7002]abcdefghijklmnopqrstuvwxyz1234", 0, "7002abcdefghijklmnopqrstuvwxyz1234", "" },
        /*701*/ { "[7002]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7002)" },
        /*702*/ { "[7003]1212121212", 0, "70031212121212", "" },
        /*703*/ { "[7003]121212121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7003)" },
        /*704*/ { "[7004]1234", 0, "70041234", "" },
        /*705*/ { "[7004]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7004)" },
        /*706*/ { "[7005]abcdefghijkl", 0, "7005abcdefghijkl", "" },
        /*707*/ { "[7005]abcdefghijklm", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7005)" },
        /*708*/ { "[7006]200132", ZINT_WARN_NONCOMPLIANT, "7006200132", "261: AI (7006) position 5: Invalid day '32'" },
        /*709*/ { "[7006]200100", ZINT_WARN_NONCOMPLIANT, "7006200100", "261: AI (7006) position 5: Invalid day '00'" },
        /*710*/ { "[7006]200120", 0, "7006200120", "" },
        /*711*/ { "[7006]2001320", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7006)" },
        /*712*/ { "[7007]010101121212", 0, "7007010101121212", "" },
        /*713*/ { "[7007]01010112121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7007)" },
        /*714*/ { "[7007]A1010112121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7007)" },
        /*715*/ { "[7007]121212", 0, "7007121212", "" },
        /*716*/ { "[7007]12121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7007)" },
        /*717*/ { "[7007]1212121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7007)" },
        /*718*/ { "[7008]abc", 0, "7008abc", "" },
        /*719*/ { "[7008]abcd", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7008)" },
        /*720*/ { "[7009]abcdefghij", 0, "7009abcdefghij", "" },
        /*721*/ { "[7009]abcdefghijk", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7009)" },
        /*722*/ { "[7010]01", 0, "701001", "" },
        /*723*/ { "[7010]1", 0, "70101", "" },
        /*724*/ { "[7010]012", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7010)" },
        /*725*/ { "[7011]121212", 0, "7011121212", "" },
        /*726*/ { "[7011]1212121212", 0, "70111212121212", "" },
        /*727*/ { "[7011]12121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7011)" },
        /*728*/ { "[7011]121212121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7011)" },
        /*729*/ { "[7012]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7012)" },
        /*730*/ { "[7019]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7019)" },
        /*731*/ { "[7020]abcdefghijklmnopqrst", 0, "7020abcdefghijklmnopqrst", "" },
        /*732*/ { "[7020]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7020)" },
        /*733*/ { "[7021]abcdefghijklmnopqrst", 0, "7021abcdefghijklmnopqrst", "" },
        /*734*/ { "[7021]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7021)" },
        /*735*/ { "[7022]abcdefghijklmnopqrst", 0, "7022abcdefghijklmnopqrst", "" },
        /*736*/ { "[7022]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7022)" },
        /*737*/ { "[7023]1234abcdefghijklmnopqrstuvwxyz", 0, "70231234abcdefghijklmnopqrstuvwxyz", "" },
        /*738*/ { "[7023]1234abcdefghijklmnopqrstuvwxyza", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7023)" },
        /*739*/ { "[7024]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7024)" },
        /*740*/ { "[7025]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7025)" },
        /*741*/ { "[7029]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7029)" },
        /*742*/ { "[7030]123abcdefghijklmnopqrstuvwxyza", ZINT_WARN_NONCOMPLIANT, "7030123abcdefghijklmnopqrstuvwxyza", "261: AI (7030) position 1: Unknown country code '123'" },
        /*743*/ { "[7030]004abcdefghijklmnopqrstuvwxyza", 0, "7030004abcdefghijklmnopqrstuvwxyza", "" },
        /*744*/ { "[7030]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7030)" },
        /*745*/ { "[7031]123abcdefghijklmnopqrstuvwxyza", ZINT_WARN_NONCOMPLIANT, "7031123abcdefghijklmnopqrstuvwxyza", "261: AI (7031) position 1: Unknown country code '123'" },
        /*746*/ { "[7031]004abcdefghijklmnopqrstuvwxyza", 0, "7031004abcdefghijklmnopqrstuvwxyza", "" },
        /*747*/ { "[7031]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7031)" },
        /*748*/ { "[7032]004abcdefghijklmnopqrstuvwxyza", 0, "7032004abcdefghijklmnopqrstuvwxyza", "" },
        /*749*/ { "[7032]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7032)" },
        /*750*/ { "[7033]004abcdefghijklmnopqrstuvwxyza", 0, "7033004abcdefghijklmnopqrstuvwxyza", "" },
        /*751*/ { "[7033]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7033)" },
        /*752*/ { "[7034]004abcdefghijklmnopqrstuvwxyza", 0, "7034004abcdefghijklmnopqrstuvwxyza", "" },
        /*753*/ { "[7034]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7034)" },
        /*754*/ { "[7035]004abcdefghijklmnopqrstuvwxyza", 0, "7035004abcdefghijklmnopqrstuvwxyza", "" },
        /*755*/ { "[7035]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7035)" },
        /*756*/ { "[7036]004abcdefghijklmnopqrstuvwxyza", 0, "7036004abcdefghijklmnopqrstuvwxyza", "" },
        /*757*/ { "[7036]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7036)" },
        /*758*/ { "[7037]004abcdefghijklmnopqrstuvwxyza", 0, "7037004abcdefghijklmnopqrstuvwxyza", "" },
        /*759*/ { "[7037]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7037)" },
        /*760*/ { "[7038]004abcdefghijklmnopqrstuvwxyza", 0, "7038004abcdefghijklmnopqrstuvwxyza", "" },
        /*761*/ { "[7038]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7038)" },
        /*762*/ { "[7039]004abcdefghijklmnopqrstuvwxyza", 0, "7039004abcdefghijklmnopqrstuvwxyza", "" },
        /*763*/ { "[7039]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7039)" },
        /*764*/ { "[7040]1abc", 0, "70401abc", "" },
        /*765*/ { "[7040]1ab", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7040)" },
        /*766*/ { "[7040]1abcd", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7040)" },
        /*767*/ { "[7041]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7041)" },
        /*768*/ { "[7042]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7042)" },
        /*769*/ { "[7050]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7050)" },
        /*770*/ { "[7090]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7090)" },
        /*771*/ { "[7099]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7099)" },
        /*772*/ { "[71]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (71)" },
        /*773*/ { "[710]abcdefghijklmnopqrst", 0, "710abcdefghijklmnopqrst", "" },
        /*774*/ { "[710]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (710)" },
        /*775*/ { "[7100]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7100)" },
        /*776*/ { "[711]abcdefghijklmnopqrst", 0, "711abcdefghijklmnopqrst", "" },
        /*777*/ { "[711]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (711)" },
        /*778*/ { "[712]abcdefghijklmnopqrst", 0, "712abcdefghijklmnopqrst", "" },
        /*779*/ { "[712]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (712)" },
        /*780*/ { "[713]abcdefghijklmnopqrst", 0, "713abcdefghijklmnopqrst", "" },
        /*781*/ { "[713]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (713)" },
        /*782*/ { "[714]abcdefghijklmnopqrst", 0, "714abcdefghijklmnopqrst", "" },
        /*783*/ { "[714]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (714)" },
        /*784*/ { "[715]abcdefghijklmnopqrst", 0, "715abcdefghijklmnopqrst", "" },
        /*785*/ { "[715]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (715)" },
        /*786*/ { "[716]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (716)" },
        /*787*/ { "[719]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (719)" },
        /*788*/ { "[72]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (72)" },
        /*789*/ { "[720]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (720)" },
        /*790*/ { "[7200]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7200)" },
        /*791*/ { "[721]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (721)" },
        /*792*/ { "[7210]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7210)" },
        /*793*/ { "[7220]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7220)" },
        /*794*/ { "[7230]EMabcdefghijklmnopqrstuvwxyzab", 0, "7230EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*795*/ { "[7230]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7230)" },
        /*796*/ { "[7230]EM", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7230)" },
        /*797*/ { "[7231]EMabcdefghijklmnopqrstuvwxyzab", 0, "7231EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*798*/ { "[7231]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7231)" },
        /*799*/ { "[7232]EMabcdefghijklmnopqrstuvwxyzab", 0, "7232EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*800*/ { "[7232]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7232)" },
        /*801*/ { "[7233]EMabcdefghijklmnopqrstuvwxyzab", 0, "7233EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*802*/ { "[7233]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7233)" },
        /*803*/ { "[7234]EMabcdefghijklmnopqrstuvwxyzab", 0, "7234EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*804*/ { "[7234]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7234)" },
        /*805*/ { "[7235]EMabcdefghijklmnopqrstuvwxyzab", 0, "7235EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*806*/ { "[7235]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7235)" },
        /*807*/ { "[7236]EMabcdefghijklmnopqrstuvwxyzab", 0, "7236EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*808*/ { "[7236]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7236)" },
        /*809*/ { "[7237]EMabcdefghijklmnopqrstuvwxyzab", 0, "7237EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*810*/ { "[7237]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7237)" },
        /*811*/ { "[7238]EMabcdefghijklmnopqrstuvwxyzab", 0, "7238EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*812*/ { "[7238]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7238)" },
        /*813*/ { "[7239]EMabcdefghijklmnopqrstuvwxyzab", 0, "7239EMabcdefghijklmnopqrstuvwxyzab", "" },
        /*814*/ { "[7239]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7239)" },
        /*815*/ { "[7239]E", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7239)" },
        /*816*/ { "[7240]abcdefghijklmnopqrst", 0, "7240abcdefghijklmnopqrst", "" },
        /*817*/ { "[7240]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7240)" },
        /*818*/ { "[7241]99", 0, "724199", "" },
        /*819*/ { "[7241]100", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7241)" },
        /*820*/ { "[7242]abcdefghijklmnopqrstuvwxy", 0, "7242abcdefghijklmnopqrstuvwxy", "" },
        /*821*/ { "[7242]abcdefghijklmnopqrstuvwxyz", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7242)" },
        /*822*/ { "[7243]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7243)" },
        /*823*/ { "[7249]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7249)" },
        /*824*/ { "[7250]12341201", 0, "725012341201", "" },
        /*825*/ { "[7250]123412012", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7250)" },
        /*826*/ { "[7251]123412011359", 0, "7251123412011359", "" },
        /*827*/ { "[7251]1234120113591", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7251)" },
        /*828*/ { "[7252]2", 0, "72522", "" },
        /*829*/ { "[7252]20", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7252)" },
        /*830*/ { "[7253]abcdefghijklmnopqrstuvwxyzabcdefghijklmn", 0, "7253abcdefghijklmnopqrstuvwxyzabcdefghijklmn", "" },
        /*831*/ { "[7253]abcdefghijklmnopqrstuvwxyzabcdefghijklmno", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7253)" },
        /*832*/ { "[7254]abcdefghijklmnopqrstuvwxyzabcdefghijklmn", 0, "7254abcdefghijklmnopqrstuvwxyzabcdefghijklmn", "" },
        /*833*/ { "[7254]abcdefghijklmnopqrstuvwxyzabcdefghijklmno", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7254)" },
        /*834*/ { "[7255]abcdefghij", 0, "7255abcdefghij", "" },
        /*835*/ { "[7255]abcdefghijk", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7255)" },
        /*836*/ { "[7256]abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl", 0, "7256abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl", "" },
        /*837*/ { "[7256]abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklm", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7256)" },
        /*838*/ { "[7257]abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqr", 0, "7257abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqr", "" },
        /*839*/ { "[7257]abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7257)" },
        /*840*/ { "[7258]1/1", 0, "72581/1", "" },
        /*841*/ { "[7258]1/01", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7258)" },
        /*842*/ { "[7259]abcdefghijklmnopqrstuvwxyzabcdefghijklmn", 0, "7259abcdefghijklmnopqrstuvwxyzabcdefghijklmn", "" },
        /*843*/ { "[7259]abcdefghijklmnopqrstuvwxyzabcdefghijklmno", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7259)" },
        /*844*/ { "[7260]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7260)" },
        /*845*/ { "[7299]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7299)" },
        /*846*/ { "[73]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (73)" },
        /*847*/ { "[7300]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7300)" },
        /*848*/ { "[74]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (74)" },
        /*849*/ { "[7400]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7400)" },
        /*850*/ { "[79]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (79)" },
        /*851*/ { "[7900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7900)" },
        /*852*/ { "[7999]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (7999)" },
        /*853*/ { "[80]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (80)" },
        /*854*/ { "[800]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (800)" },
        /*855*/ { "[8000]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8000)" },
        /*856*/ { "[8001]12345678901234", ZINT_WARN_NONCOMPLIANT, "800112345678901234", "261: AI (8001) position 13: Invalid winding direction '3'" },
        /*857*/ { "[8001]12345678901204", 0, "800112345678901204", "" },
        /*858*/ { "[8001]1234123456789012345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8001)" },
        /*859*/ { "[8002]abcdefghijklmnopqrst", 0, "8002abcdefghijklmnopqrst", "" },
        /*860*/ { "[8002]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8002)" },
        /*861*/ { "[8003]01234567890123abcdefghijklmnop", ZINT_WARN_NONCOMPLIANT, "800301234567890123abcdefghijklmnop", "261: AI (8003) position 14: Bad checksum '3', expected '8'" },
        /*862*/ { "[8003]01234567890128abcdefghijklmnop", 0, "800301234567890128abcdefghijklmnop", "" },
        /*863*/ { "[8003]01234567890128abcdefghijklmnopq", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8003)" },
        /*864*/ { "[8004]abcdefghijklmnopqrstuvwxyz1234", ZINT_WARN_NONCOMPLIANT, "8004abcdefghijklmnopqrstuvwxyz1234", "261: AI (8004) position 1: Non-numeric company prefix 'a'" },
        /*865*/ { "[8004]12cdefghijklmnopqrstuvwxyz1234", 0, "800412cdefghijklmnopqrstuvwxyz1234", "" },
        /*866*/ { "[8004]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8004)" },
        /*867*/ { "[8005]123456", 0, "8005123456", "" },
        /*868*/ { "[8005]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8005)" },
        /*869*/ { "[8005]1234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8005)" },
        /*870*/ { "[8006]123456789012341212", ZINT_WARN_NONCOMPLIANT, "8006123456789012341212", "261: AI (8006) position 14: Bad checksum '4', expected '1'" },
        /*871*/ { "[8006]123456789012311212", 0, "8006123456789012311212", "" },
        /*872*/ { "[8006]12345678901234121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8006)" },
        /*873*/ { "[8006]1234567890123412123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8006)" },
        /*874*/ { "[8007]abcdefghijklmnopqrstuvwxyz12345678", ZINT_WARN_NONCOMPLIANT, "8007abcdefghijklmnopqrstuvwxyz12345678", "261: AI (8007) position 1: Non-alphabetic IBAN country code 'ab'" },
        /*875*/ { "[8007]AD95EFGHIJKLMNOPQRSTUVWXYZ12345678", 0, "8007AD95EFGHIJKLMNOPQRSTUVWXYZ12345678", "" },
        /*876*/ { "[8007]AD95EFGHIJKLMNOPQRSTUVWXYZ123456789", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8007)" },
        /*877*/ { "[8008]123456121212", ZINT_WARN_NONCOMPLIANT, "8008123456121212", "261: AI (8008) position 3: Invalid month '34'" },
        /*878*/ { "[8008]121256121212", ZINT_WARN_NONCOMPLIANT, "8008121256121212", "261: AI (8008) position 5: Invalid day '56'" },
        /*879*/ { "[8008]121231121212", 0, "8008121231121212", "" },
        /*880*/ { "[8008]1234561212", ZINT_WARN_NONCOMPLIANT, "80081234561212", "261: AI (8008) position 3: Invalid month '34'" },
        /*881*/ { "[8008]1212311212", 0, "80081212311212", "" },
        /*882*/ { "[8008]12345612", ZINT_WARN_NONCOMPLIANT, "800812345612", "261: AI (8008) position 3: Invalid month '34'" },
        /*883*/ { "[8008]12010112", 0, "800812010112", "" },
        /*884*/ { "[8008]1234561", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8008)" },
        /*885*/ { "[8008]123456121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8008)" },
        /*886*/ { "[8008]12345612121", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8008)" },
        /*887*/ { "[8008]1234561212123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8008)" },
        /*888*/ { "[8009]12345678901234567890123456789012345678901234567890", 0, "800912345678901234567890123456789012345678901234567890", "" },
        /*889*/ { "[8009]123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8009)" },
        /*890*/ { "[8010]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8010)" },
        /*891*/ { "[8011]123456789012", 0, "8011123456789012", "" },
        /*892*/ { "[8011]1234567890123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8011)" },
        /*893*/ { "[8012]abcdefghijklmnopqrst", 0, "8012abcdefghijklmnopqrst", "" },
        /*894*/ { "[8012]abcdefghijklmnopqrstuv", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8012)" },
        /*895*/ { "[8013]1234abcdefghijklmnopqrsQP", 0, "80131234abcdefghijklmnopqrsQP", "" },
        /*896*/ { "[8013]1234abcdefghijklmnopqrsQPv", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8013)" },
        /*897*/ { "[8014]1234abcdefghijklmnopqrsQP", 0, "80141234abcdefghijklmnopqrsQP", "" },
        /*898*/ { "[8014]1234abcdefghijklmnopqrsQPv", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8014)" },
        /*899*/ { "[8015]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8015)" },
        /*900*/ { "[8016]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8016)" },
        /*901*/ { "[8017]313131313131313139", ZINT_WARN_NONCOMPLIANT, "8017313131313131313139", "261: AI (8017) position 18: Bad checksum '9', expected '1'" },
        /*902*/ { "[8017]313131313131313131", 0, "8017313131313131313131", "" },
        /*903*/ { "[8017]31313131313131313", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8017)" },
        /*904*/ { "[8017]3131313131313131390", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8017)" },
        /*905*/ { "[8018]313131313131313139", ZINT_WARN_NONCOMPLIANT, "8018313131313131313139", "261: AI (8018) position 18: Bad checksum '9', expected '1'" },
        /*906*/ { "[8018]313131313131313131", 0, "8018313131313131313131", "" },
        /*907*/ { "[8018]31313131313131313", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8018)" },
        /*908*/ { "[8018]3131313131313131390", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8018)" },
        /*909*/ { "[8019]1234567890", 0, "80191234567890", "" },
        /*910*/ { "[8019]12345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8019)" },
        /*911*/ { "[8020]abcdefghijklmnopqrstuvwxy", 0, "8020abcdefghijklmnopqrstuvwxy", "" },
        /*912*/ { "[8020]abcdefghijklmnopqrstuvwxyz", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8020)" },
        /*913*/ { "[8021]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8021)" },
        /*914*/ { "[8025]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8025)" },
        /*915*/ { "[8026]123456789012341212", ZINT_WARN_NONCOMPLIANT, "8026123456789012341212", "261: AI (8026) position 14: Bad checksum '4', expected '1'" },
        /*916*/ { "[8026]123456789012311212", 0, "8026123456789012311212", "" },
        /*917*/ { "[8026]1234567890123451212", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8026)" },
        /*918*/ { "[8026]12345678901234512", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8026)" },
        /*919*/ { "[8027]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8027)" },
        /*920*/ { "[8030]-1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, "8030-1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ", "" },
        /*921*/ { "[8030]-1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ1", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8030)" },
        /*922*/ { "[8031]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8031)" },
        /*923*/ { "[8040]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8040)" },
        /*924*/ { "[8050]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8050)" },
        /*925*/ { "[8060]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8060)" },
        /*926*/ { "[8070]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8070)" },
        /*927*/ { "[8080]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8080)" },
        /*928*/ { "[8090]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8090)" },
        /*929*/ { "[8099]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8099)" },
        /*930*/ { "[81]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (81)" },
        /*931*/ { "[8100]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8100)" },
        /*932*/ { "[8109]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8109)" },
        /*933*/ { "[8110]5123456789011234565123455123450123105123450123512345678901320123190000", 0, "81105123456789011234565123455123450123105123450123512345678901320123190000", "" },
        /*934*/ { "[8110]51234567890112345651234551234501231051234501235123456789013201231900001", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8110)" },
        /*935*/ { "[8111]1234", 0, "81111234", "" },
        /*936*/ { "[8111]12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8111)" },
        /*937*/ { "[8111]123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8111)" },
        /*938*/ { "[8112]1234567890123456789012345678901234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, "81121234567890123456789012345678901234567890123456789012345678901234567890", "259: Invalid data length for AI (8112)" },
        /*939*/ { "[8112]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8112)" },
        /*940*/ { "[8112]061234567890121234569123456789012345", 0, "8112061234567890121234569123456789012345", "" },
        /*941*/ { "[8113]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8113)" },
        /*942*/ { "[8120]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8120)" },
        /*943*/ { "[8130]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8130)" },
        /*944*/ { "[8140]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8140)" },
        /*945*/ { "[8150]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8150)" },
        /*946*/ { "[8190]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8190)" },
        /*947*/ { "[8199]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8199)" },
        /*948*/ { "[82]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (82)" },
        /*949*/ { "[8200]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "82001234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*950*/ { "[8201]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8201)" },
        /*951*/ { "[8210]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8210)" },
        /*952*/ { "[8220]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8220)" },
        /*953*/ { "[8230]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8230)" },
        /*954*/ { "[8240]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8240)" },
        /*955*/ { "[8250]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8250)" },
        /*956*/ { "[8290]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8290)" },
        /*957*/ { "[8299]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8299)" },
        /*958*/ { "[83]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (83)" },
        /*959*/ { "[830]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (830)" },
        /*960*/ { "[8300]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8300)" },
        /*961*/ { "[84]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (84)" },
        /*962*/ { "[840]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (840)" },
        /*963*/ { "[8400]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8400)" },
        /*964*/ { "[85]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (85)" },
        /*965*/ { "[850]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (850)" },
        /*966*/ { "[8500]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8500)" },
        /*967*/ { "[89]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (89)" },
        /*968*/ { "[890]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (890)" },
        /*969*/ { "[8900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (8900)" },
        /*970*/ { "[90]abcdefghijklmnopqrstuvwxyz1234", 0, "90abcdefghijklmnopqrstuvwxyz1234", "" },
        /*971*/ { "[90]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (90)" },
        /*972*/ { "[900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (900)" },
        /*973*/ { "[9000]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9000)" },
        /*974*/ { "[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "91123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*975*/ { "[91]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (91)" },
        /*976*/ { "[910]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (910)" },
        /*977*/ { "[9100]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9100)" },
        /*978*/ { "[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "92123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*979*/ { "[92]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (92)" },
        /*980*/ { "[920]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (920)" },
        /*981*/ { "[9200]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9200)" },
        /*982*/ { "[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "93123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*983*/ { "[93]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (93)" },
        /*984*/ { "[930]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (930)" },
        /*985*/ { "[9300]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9300)" },
        /*986*/ { "[94]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "94123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*987*/ { "[94]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (94)" },
        /*988*/ { "[940]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (940)" },
        /*989*/ { "[9400]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9400)" },
        /*990*/ { "[95]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "95123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*991*/ { "[95]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (95)" },
        /*992*/ { "[950]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (950)" },
        /*993*/ { "[9500]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9500)" },
        /*994*/ { "[96]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "96123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*995*/ { "[96]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (96)" },
        /*996*/ { "[960]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (960)" },
        /*997*/ { "[9600]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9600)" },
        /*998*/ { "[97]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "97123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*999*/ { "[97]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (97)" },
        /*1000*/ { "[970]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (970)" },
        /*1001*/ { "[9700]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9700)" },
        /*1002*/ { "[98]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "98123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*1003*/ { "[98]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (98)" },
        /*1004*/ { "[980]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (980)" },
        /*1005*/ { "[9800]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9800)" },
        /*1006*/ { "[99]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "99123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "" },
        /*1007*/ { "[99]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (99)" },
        /*1008*/ { "[990]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (990)" },
        /*1009*/ { "[9900]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9900)" },
        /*1010*/ { "[9999]1234", ZINT_ERROR_INVALID_DATA, "", "260: Invalid AI (9999)" },
        /*1011*/ { "[01]12345678901234[7006]200101", ZINT_WARN_NONCOMPLIANT, "01123456789012347006200101", "261: AI (01) position 14: Bad checksum '4', expected '1'" },
        /*1012*/ { "[01]12345678901231[7006]200101", 0, "01123456789012317006200101", "" },
        /*1013*/ { "[3900]1234567890[01]12345678901234", ZINT_WARN_NONCOMPLIANT, "39001234567890\0350112345678901234", "261: AI (01) position 14: Bad checksum '4', expected '1'" },
        /*1014*/ { "[3900]1234567890[01]12345678901231", 0, "39001234567890\0350112345678901231", "" },
        /*1015*/ { "[253]12345678901234[3901]12345678901234[20]12", ZINT_WARN_NONCOMPLIANT, "25312345678901234\035390112345678901234\0352012", "261: AI (253) position 13: Bad checksum '3', expected '8'" },
        /*1016*/ { "[253]12345678901284[3901]12345678901234[20]12", 0, "25312345678901284\035390112345678901234\0352012", "" },
        /*1017*/ { "[253]12345678901234[01]12345678901234[3901]12345678901234[20]12", ZINT_WARN_NONCOMPLIANT, "25312345678901234\0350112345678901234390112345678901234\0352012", "261: AI (01) position 14: Bad checksum '4', expected '1'" },
        /*1018*/ { "[253]12345678901284[01]12345678901231[3901]12345678901234[20]12", 0, "25312345678901284\0350112345678901231390112345678901234\0352012", "" },
        /*1019*/ { "[01]12345678901231[0A]12345678901231[20]12", ZINT_ERROR_INVALID_DATA, "", "257: Invalid AI in input data (non-numeric characters in AI)" },
        /*1020*/ { "[01]12345678901231[0]12345678901231[20]12", ZINT_ERROR_INVALID_DATA, "", "256: Invalid AI in input data (AI too short)" },
        /*1021*/ { "[01]12345678901231[]12345678901231[20]12", ZINT_ERROR_INVALID_DATA, "", "256: Invalid AI in input data (AI too short)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char reduced[1024];
    char escaped[1024];

    testStart("test_gs1_verify");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = (int) strlen(data[i].data);

        ret = gs1_verify(symbol, (unsigned char *) data[i].data, length, (unsigned char *) reduced);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, \"%s\", \"%s\" },\n",
                        i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(ret),
                        data[i].expected, symbol->errtxt);
        } else {
            assert_equal(ret, data[i].ret, "i:%d ret %d != %d (length %d \"%s\") (%s)\n",
                        i, ret, data[i].ret, length, data[i].data, symbol->errtxt);

            if (ret < ZINT_ERROR) {
                assert_zero(strcmp(reduced, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                            i, reduced, data[i].expected);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n",
                        i, symbol->errtxt, data[i].expected_errtxt);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_gs1_lint(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int ret;
        char *expected;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { "[00]123456789012345675", 0, "00123456789012345675", "" }, /* numeric */
        /*  1*/ { "[00]12345678901234567.", ZINT_WARN_NONCOMPLIANT, "0012345678901234567.", "261: AI (00) position 18: Non-numeric character '.'" }, /* numeric */
        /*  2*/ { "[00]123456789012345678", ZINT_WARN_NONCOMPLIANT, "00123456789012345678", "261: AI (00) position 18: Bad checksum '8', expected '5'" }, /* csum */
        /*  3*/ { "[00]1234567890123456759", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (00)" }, /* length */
        /*  4*/ { "[00]12345678901234567", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (00)" }, /* length */
        /*  5*/ { "[00]123456789012345675A", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (00)" }, /* length */
        /*  6*/ { "[91]!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", 0, "91!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", "" }, /* cset82 */
        /*  7*/ { "[91] ", ZINT_WARN_NONCOMPLIANT, "91 ", "261: AI (91) position 1: Invalid CSET 82 character ' '" }, /* cset82 */
        /*  8*/ { "[91]#", ZINT_WARN_NONCOMPLIANT, "91#", "261: AI (91) position 1: Invalid CSET 82 character '#'" }, /* cset82 */
        /*  9*/ { "[91]a^", ZINT_WARN_NONCOMPLIANT, "91a^", "261: AI (91) position 2: Invalid CSET 82 character '^'" }, /* cset82 */
        /* 10*/ { "[91]!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxy{", ZINT_WARN_NONCOMPLIANT, "91!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxy{", "261: AI (91) position 82: Invalid CSET 82 character '{'" }, /* cset82 */
        /* 11*/ { "[8010]01#-/23456789ABCDEFGHIJKLMNOPQ", 0, "801001#-/23456789ABCDEFGHIJKLMNOPQ", "" }, /* cset39 */
        /* 12*/ { "[8010]6789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, "80106789ABCDEFGHIJKLMNOPQRSTUVWXYZ", "" }, /* cset39 */
        /* 13*/ { "[8010]01!", ZINT_WARN_NONCOMPLIANT, "801001!", "261: AI (8010) position 3: Invalid CSET 39 character '!'" }, /* cset39 */
        /* 14*/ { "[8010]01a", ZINT_WARN_NONCOMPLIANT, "801001a", "261: AI (8010) position 3: Invalid CSET 39 character 'a'" }, /* cset39 */
        /* 15*/ { "[8010]6789ABCDEFGHIJKLMNOPQRSTUVWXY}", ZINT_WARN_NONCOMPLIANT, "80106789ABCDEFGHIJKLMNOPQRSTUVWXY}", "261: AI (8010) position 30: Invalid CSET 39 character '}'" }, /* cset39 */
        /* 16*/ { "[8030]-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", 0, "8030-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", "" }, /* cset64 */
        /* 17*/ { "[8030]+0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", ZINT_WARN_NONCOMPLIANT, "8030+0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", "261: AI (8030) position 1: Invalid CSET 64 character '+'" }, /* cset64 */
        /* 18*/ { "[8030]-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^abcdefghijklmnopqrstuvwxyz", ZINT_WARN_NONCOMPLIANT, "8030-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^abcdefghijklmnopqrstuvwxyz", "261: AI (8030) position 38: Invalid CSET 64 character '^'" }, /* cset64 */
        /* 19*/ { "[8030]-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz==", 0, "8030-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz==", "" }, /* cset64 */
        /* 20*/ { "[8030]-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz=", ZINT_WARN_NONCOMPLIANT, "8030-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz=", "261: AI (8030) position 65: Invalid CSET 64 character '='" }, /* cset64 */
        /* 21*/ { "[8030]-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz1=", 0, "8030-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz1=", "" }, /* cset64 */
        /* 22*/ { "[8030]-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ=abcdefghijklmnopqrstuvwxyz", ZINT_WARN_NONCOMPLIANT, "8030-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ=abcdefghijklmnopqrstuvwxyz", "261: AI (8030) position 38: Invalid CSET 64 character '='" }, /* cset64 */
        /* 23*/ { "[8010]#-/0123456789ABCDEFGHIJKLMNOPQ", ZINT_WARN_NONCOMPLIANT, "8010#-/0123456789ABCDEFGHIJKLMNOPQ", "261: AI (8010) position 1: Non-numeric company prefix '#'" }, /* key */
        /* 24*/ { "[8010]0#-/123456789ABCDEFGHIJKLMNOPQ", ZINT_WARN_NONCOMPLIANT, "80100#-/123456789ABCDEFGHIJKLMNOPQ", "261: AI (8010) position 2: Non-numeric company prefix '#'" }, /* key */
        /* 25*/ { "[401]0", ZINT_WARN_NONCOMPLIANT, "4010", "259: Invalid data length for AI (401)" }, /* key */
        /* 26*/ { "[8013]1987654Ad4X4bL5ttr2310c2K", 0, "80131987654Ad4X4bL5ttr2310c2K", "" }, /* csumalpha */
        /* 27*/ { "[8013]12345678901234567890123NT", 0, "801312345678901234567890123NT", "" }, /* csumalpha */
        /* 28*/ { "[8013]12345_ABCDEFGHIJKLMCP", 0, "801312345_ABCDEFGHIJKLMCP", "" }, /* csumalpha */
        /* 29*/ { "[8013]12345_NOPQRSTUVWXYZDN", 0, "801312345_NOPQRSTUVWXYZDN", "" }, /* csumalpha */
        /* 30*/ { "[8013]12345_abcdefghijklmN3", 0, "801312345_abcdefghijklmN3", "" }, /* csumalpha */
        /* 31*/ { "[8013]12345_nopqrstuvwxyzP2", 0, "801312345_nopqrstuvwxyzP2", "" }, /* csumalpha */
        /* 32*/ { "[8013]12345_!\"%&'()*+,-./LC", 0, "801312345_!\"%&'()*+,-./LC", "" }, /* csumalpha */
        /* 33*/ { "[8013]12345_0123456789:;<=>?62", 0, "801312345_0123456789:;<=>?62", "" }, /* csumalpha */
        /* 34*/ { "[8013]7907665Bm8v2AB", 0, "80137907665Bm8v2AB", "" }, /* csumalpha */
        /* 35*/ { "[8013]97850l6KZm0yCD", 0, "801397850l6KZm0yCD", "" }, /* csumalpha */
        /* 36*/ { "[8013]225803106GSpEF", 0, "8013225803106GSpEF", "" }, /* csumalpha */
        /* 37*/ { "[8013]149512464PM+GH", 0, "8013149512464PM+GH", "" }, /* csumalpha */
        /* 38*/ { "[8013]62577B8fRG7HJK", 0, "801362577B8fRG7HJK", "" }, /* csumalpha */
        /* 39*/ { "[8013]515942070CYxLM", 0, "8013515942070CYxLM", "" }, /* csumalpha */
        /* 40*/ { "[8013]390800494sP6NP", 0, "8013390800494sP6NP", "" }, /* csumalpha */
        /* 41*/ { "[8013]386830132uO+QR", 0, "8013386830132uO+QR", "" }, /* csumalpha */
        /* 42*/ { "[8013]53395376X1:nST", 0, "801353395376X1:nST", "" }, /* csumalpha */
        /* 43*/ { "[8013]957813138Sb6UV", 0, "8013957813138Sb6UV", "" }, /* csumalpha */
        /* 44*/ { "[8013]530790no0qOgWX", 0, "8013530790no0qOgWX", "" }, /* csumalpha */
        /* 45*/ { "[8013]62185314IvwmYZ", 0, "801362185314IvwmYZ", "" }, /* csumalpha */
        /* 46*/ { "[8013]23956qk1&dB!23", 0, "801323956qk1&dB!23", "" }, /* csumalpha */
        /* 47*/ { "[8013]794394895ic045", 0, "8013794394895ic045", "" }, /* csumalpha */
        /* 48*/ { "[8013]57453Uq3qA<H67", 0, "801357453Uq3qA<H67", "" }, /* csumalpha */
        /* 49*/ { "[8013]62185314IvwmYZ", 0, "801362185314IvwmYZ", "" }, /* csumalpha */
        /* 50*/ { "[8013]0881063PhHvY89", 0, "80130881063PhHvY89", "" }, /* csumalpha */
        /* 51*/ { "[8013]00000!HV", 0, "801300000!HV", "" }, /* csumalpha */
        /* 52*/ { "[8013]99999zzzzzzzzzzzzzzzzzzT2", 0, "801399999zzzzzzzzzzzzzzzzzzT2", "" }, /* csumalpha */
        /* 53*/ { "[8013]1987654Ad4X4bL5ttr2310cXK", ZINT_WARN_NONCOMPLIANT, "80131987654Ad4X4bL5ttr2310cXK", "261: AI (8013) position 24: Bad checksum 'X', expected '2'" }, /* csumalpha */
        /* 54*/ { "[8013]1987654Ad4X4bL5ttr2310c2X", ZINT_WARN_NONCOMPLIANT, "80131987654Ad4X4bL5ttr2310c2X", "261: AI (8013) position 25: Bad checksum 'X', expected 'K'" }, /* csumalpha */
        /* 55*/ { "[8013]198765£Ad4X4bL5ttr2310c2K", ZINT_ERROR_INVALID_DATA, "", "250: Extended ASCII characters are not supported by GS1" }, /* csumalpha */
        /* 56*/ { "[8013]1987654Ad4X4bL5ttr2310£2K", ZINT_ERROR_INVALID_DATA, "", "250: Extended ASCII characters are not supported by GS1" }, /* csumalpha */
        /* 57*/ { "[8013]1987654Ad4X4bL5ttr2310cxK", ZINT_WARN_NONCOMPLIANT, "80131987654Ad4X4bL5ttr2310cxK", "261: AI (8013) position 24: Bad checksum 'x', expected '2'" }, /* csumalpha */
        /* 58*/ { "[8013]1987654Ad4X4bL5ttr2310c2x", ZINT_WARN_NONCOMPLIANT, "80131987654Ad4X4bL5ttr2310c2x", "261: AI (8013) position 25: Bad checksum 'x', expected 'K'" }, /* csumalpha */
        /* 59*/ { "[8013]12345678901234567890123NTX", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8013)" }, /* csumalpha */
        /* 60*/ { "[8013]1", ZINT_WARN_NONCOMPLIANT, "80131", "259: Invalid data length for AI (8013)" }, /* csumalpha */
        /* 61*/ { "[8013]12", ZINT_WARN_NONCOMPLIANT, "801312", "261: AI (8013) position 1: Bad checksum '1', expected '2'" }, /* csumalpha */
        /* 62*/ { "[8013]22", 0, "801322", "" }, /* csumalpha */
        /* 63*/ { "[8013]123", ZINT_WARN_NONCOMPLIANT, "8013123", "261: AI (8013) position 3: Bad checksum '3', expected 'W'" }, /* csumalpha */
        /* 64*/ { "[8013]12W", 0, "801312W", "" }, /* csumalpha */
        /* 65*/ { "[8013]00000!HW", ZINT_WARN_NONCOMPLIANT, "801300000!HW", "261: AI (8013) position 8: Bad checksum 'W', expected 'V'" }, /* csumalpha */
        /* 66*/ { "[8013]7907665Bm8v2BB", ZINT_WARN_NONCOMPLIANT, "80137907665Bm8v2BB", "261: AI (8013) position 13: Bad checksum 'B', expected 'A'" }, /* csumalpha */
        /* 67*/ { "[8013]99zzzzzzzzzzzzzzzzzzzzzZ7", 0, "801399zzzzzzzzzzzzzzzzzzzzzZ7", "" }, /* csumalpha */
        /* 68*/ { "[11]120100", 0, "11120100", "" }, /* yymmd0 */
        /* 69*/ { "[11]120131", 0, "11120131", "" }, /* yymmd0 */
        /* 70*/ { "[11]120132", ZINT_WARN_NONCOMPLIANT, "11120132", "261: AI (11) position 5: Invalid day '32'" }, /* yymmd0 */
        /* 71*/ { "[11]120229", 0, "11120229", "" }, /* yymmd0 */
        /* 72*/ { "[11]110229", ZINT_WARN_NONCOMPLIANT, "11110229", "261: AI (11) position 5: Invalid day '29'" }, /* yymmd0 */
        /* 73*/ { "[11]000229", 0, "11000229", "" }, /* yymmd0 NOTE: will be false in 2050 */
        /* 74*/ { "[11]480229", 0, "11480229", "" }, /* yymmd0 */
        /* 75*/ { "[11]500229", ZINT_WARN_NONCOMPLIANT, "11500229", "261: AI (11) position 5: Invalid day '29'" }, /* yymmd0 */
        /* 76*/ { "[11]980229", ZINT_WARN_NONCOMPLIANT, "11980229", "261: AI (11) position 5: Invalid day '29'" }, /* yymmd0 */
        /* 77*/ { "[11]110228", 0, "11110228", "" }, /* yymmd0 */
        /* 78*/ { "[11]120230", ZINT_WARN_NONCOMPLIANT, "11120230", "261: AI (11) position 5: Invalid day '30'" }, /* yymmd0 */
        /* 79*/ { "[11]120331", 0, "11120331", "" }, /* yymmd0 */
        /* 80*/ { "[11]120332", ZINT_WARN_NONCOMPLIANT, "11120332", "261: AI (11) position 5: Invalid day '32'" }, /* yymmd0 */
        /* 81*/ { "[11]120430", 0, "11120430", "" }, /* yymmd0 */
        /* 82*/ { "[11]120431", ZINT_WARN_NONCOMPLIANT, "11120431", "261: AI (11) position 5: Invalid day '31'" }, /* yymmd0 */
        /* 83*/ { "[11]120531", 0, "11120531", "" }, /* yymmd0 */
        /* 84*/ { "[11]120532", ZINT_WARN_NONCOMPLIANT, "11120532", "261: AI (11) position 5: Invalid day '32'" }, /* yymmd0 */
        /* 85*/ { "[11]120630", 0, "11120630", "" }, /* yymmd0 */
        /* 86*/ { "[11]120631", ZINT_WARN_NONCOMPLIANT, "11120631", "261: AI (11) position 5: Invalid day '31'" }, /* yymmd0 */
        /* 87*/ { "[11]120731", 0, "11120731", "" }, /* yymmd0 */
        /* 88*/ { "[11]120732", ZINT_WARN_NONCOMPLIANT, "11120732", "261: AI (11) position 5: Invalid day '32'" }, /* yymmd0 */
        /* 89*/ { "[11]120831", 0, "11120831", "" }, /* yymmd0 */
        /* 90*/ { "[11]120832", ZINT_WARN_NONCOMPLIANT, "11120832", "261: AI (11) position 5: Invalid day '32'" }, /* yymmd0 */
        /* 91*/ { "[11]120930", 0, "11120930", "" }, /* yymmd0 */
        /* 92*/ { "[11]120931", ZINT_WARN_NONCOMPLIANT, "11120931", "261: AI (11) position 5: Invalid day '31'" }, /* yymmd0 */
        /* 93*/ { "[11]121031", 0, "11121031", "" }, /* yymmd0 */
        /* 94*/ { "[11]121032", ZINT_WARN_NONCOMPLIANT, "11121032", "261: AI (11) position 5: Invalid day '32'" }, /* yymmd0 */
        /* 95*/ { "[11]121130", 0, "11121130", "" }, /* yymmd0 */
        /* 96*/ { "[11]121131", ZINT_WARN_NONCOMPLIANT, "11121131", "261: AI (11) position 5: Invalid day '31'" }, /* yymmd0 */
        /* 97*/ { "[11]121200", 0, "11121200", "" }, /* yymmd0 */
        /* 98*/ { "[11]121231", 0, "11121231", "" }, /* yymmd0 */
        /* 99*/ { "[11]121232", ZINT_WARN_NONCOMPLIANT, "11121232", "261: AI (11) position 5: Invalid day '32'" }, /* yymmd0 */
        /*100*/ { "[11]120031", ZINT_WARN_NONCOMPLIANT, "11120031", "261: AI (11) position 3: Invalid month '00'" }, /* yymmd0 */
        /*101*/ { "[11]121331", ZINT_WARN_NONCOMPLIANT, "11121331", "261: AI (11) position 3: Invalid month '13'" }, /* yymmd0 */
        /*102*/ { "[4326]121231", 0, "4326121231", "" }, /* yymmdd */
        /*103*/ { "[4326]121200", ZINT_WARN_NONCOMPLIANT, "4326121200", "261: AI (4326) position 5: Invalid day '00'" }, /* yymmdd */
        /*104*/ { "[4326]120031", ZINT_WARN_NONCOMPLIANT, "4326120031", "261: AI (4326) position 3: Invalid month '00'" }, /* yymmdd */
        /*105*/ { "[4326]129931", ZINT_WARN_NONCOMPLIANT, "4326129931", "261: AI (4326) position 3: Invalid month '99'" }, /* yymmdd */
        /*106*/ { "[4326]121299", ZINT_WARN_NONCOMPLIANT, "4326121299", "261: AI (4326) position 5: Invalid day '99'" }, /* yymmdd */
        /*107*/ { "[4326]120230", ZINT_WARN_NONCOMPLIANT, "4326120230", "261: AI (4326) position 5: Invalid day '30'" }, /* yymmdd */
        /*108*/ { "[4326]110229", ZINT_WARN_NONCOMPLIANT, "4326110229", "261: AI (4326) position 5: Invalid day '29'" }, /* yymmdd */
        /*109*/ { "[4326]000229", 0, "4326000229", "" }, /* yymmdd NOTE: will be false in 2050 */
        /*110*/ { "[4326]940229", ZINT_WARN_NONCOMPLIANT, "4326940229", "261: AI (4326) position 5: Invalid day '29'" }, /* yymmdd */
        /*111*/ { "[7250]20241231", 0, "725020241231", "" }, /* yyyymmdd */
        /*112*/ { "[7250]99991231", 0, "725099991231", "" }, /* yyyymmdd */
        /*113*/ { "[7250]00001231", 0, "725000001231", "" }, /* yyyymmdd */
        /*114*/ { "[7250]20241200", ZINT_WARN_NONCOMPLIANT, "725020241200", "261: AI (7250) position 7: Invalid day '00'" }, /* yyyymmdd */
        /*115*/ { "[7250]20240001", ZINT_WARN_NONCOMPLIANT, "725020240001", "261: AI (7250) position 5: Invalid month '00'" }, /* yyyymmdd */
        /*116*/ { "[7250]20241301", ZINT_WARN_NONCOMPLIANT, "725020241301", "261: AI (7250) position 5: Invalid month '13'" }, /* yyyymmdd */
        /*117*/ { "[7250]20240230", ZINT_WARN_NONCOMPLIANT, "725020240230", "261: AI (7250) position 7: Invalid day '30'" }, /* yyyymmdd */
        /*118*/ { "[7250]20240229", 0, "725020240229", "" }, /* yyyymmdd */
        /*119*/ { "[7250]20230229", ZINT_WARN_NONCOMPLIANT, "725020230229", "261: AI (7250) position 7: Invalid day '29'" }, /* yyyymmdd */
        /*120*/ { "[7250]20000229", 0, "725020000229", "" }, /* yyyymmdd */
        /*121*/ { "[7250]30000229", ZINT_WARN_NONCOMPLIANT, "725030000229", "261: AI (7250) position 7: Invalid day '29'" }, /* yyyymmdd */
        /*122*/ { "[7250]20240930", 0, "725020240930", "" }, /* yyyymmdd */
        /*123*/ { "[7250]20240931", ZINT_WARN_NONCOMPLIANT, "725020240931", "261: AI (7250) position 7: Invalid day '31'" }, /* yyyymmdd */
        /*124*/ { "[4324]1212310000", 0, "43241212310000", "" }, /* hh,mm */
        /*125*/ { "[4324]1212312359", 0, "43241212312359", "" }, /* hh,mm */
        /*126*/ { "[4324]1212312400", ZINT_WARN_NONCOMPLIANT, "43241212312400", "261: AI (4324) position 7: Invalid hour of day '24'" }, /* hh,mm */
        /*127*/ { "[4324]1212312360", ZINT_WARN_NONCOMPLIANT, "43241212312360", "261: AI (4324) position 9: Invalid minutes in the hour '60'" }, /* hh,mm */
        /*128*/ { "[8008]121231000000", 0, "8008121231000000", "" }, /* hh,mm,ss */
        /*129*/ { "[8008]1212310000", 0, "80081212310000", "" }, /* hh,mm[,ss] */
        /*130*/ { "[8008]12123100", 0, "800812123100", "" }, /* hh[,mm,ss] */
        /*131*/ { "[8008]12123123", 0, "800812123123", "" }, /* hh[,mm,ss] */
        /*132*/ { "[8008]12123124", ZINT_WARN_NONCOMPLIANT, "800812123124", "261: AI (8008) position 7: Invalid hour of day '24'" }, /* hh[,mm,ss] */
        /*133*/ { "[8008]1212312359", 0, "80081212312359", "" }, /* hh,mm[,ss] */
        /*134*/ { "[8008]1212312360", ZINT_WARN_NONCOMPLIANT, "80081212312360", "261: AI (8008) position 9: Invalid minutes in the hour '60'" }, /* hh,mm[,ss] */
        /*135*/ { "[8008]121231235959", 0, "8008121231235959", "" }, /* hh,mm,ss */
        /*136*/ { "[8008]121231235960", ZINT_WARN_NONCOMPLIANT, "8008121231235960", "261: AI (8008) position 11: Invalid seconds in the minute '60'" }, /* hh,mm,ss */
        /*137*/ { "[422]004", 0, "422004", "" }, /* iso3166 */
        /*138*/ { "[422]894", 0, "422894", "" }, /* iso3166 */
        /*139*/ { "[422]00", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (422)" }, /* iso3166 */
        /*140*/ { "[422]0A", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (422)" }, /* iso3166 */
        /*141*/ { "[422]003", ZINT_WARN_NONCOMPLIANT, "422003", "261: AI (422) position 1: Unknown country code '003'" }, /* iso3166 */
        /*142*/ { "[422]895", ZINT_WARN_NONCOMPLIANT, "422895", "261: AI (422) position 1: Unknown country code '895'" }, /* iso3166 */
        /*143*/ { "[422]999", ZINT_WARN_NONCOMPLIANT, "422999", "261: AI (422) position 1: Unknown country code '999'" }, /* iso3166 */
        /*144*/ { "[423]004", 0, "423004", "" }, /* iso3166list */
        /*145*/ { "[423]004894", 0, "423004894", "" }, /* iso3166list */
        /*146*/ { "[423]004894004", 0, "423004894004", "" }, /* iso3166list */
        /*147*/ { "[423]004894004894", 0, "423004894004894", "" }, /* iso3166list */
        /*148*/ { "[423]004894004894004", 0, "423004894004894004", "" }, /* iso3166list */
        /*149*/ { "[423]004894004894004894", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*150*/ { "[423]123894004894004894", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*151*/ { "[423]A04894004894004894", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*152*/ { "[423]00489400489400489", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*153*/ { "[423]0048940048940048", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*154*/ { "[423]00489400489400", ZINT_WARN_NONCOMPLIANT, "42300489400489400", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*155*/ { "[423]0048940048940", ZINT_WARN_NONCOMPLIANT, "4230048940048940", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*156*/ { "[423]00489400489", ZINT_WARN_NONCOMPLIANT, "42300489400489", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*157*/ { "[423]0048940048", ZINT_WARN_NONCOMPLIANT, "4230048940048", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*158*/ { "[423]00489400", ZINT_WARN_NONCOMPLIANT, "42300489400", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*159*/ { "[423]0048940", ZINT_WARN_NONCOMPLIANT, "4230048940", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*160*/ { "[423]00489", ZINT_WARN_NONCOMPLIANT, "42300489", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*161*/ { "[423]0048", ZINT_WARN_NONCOMPLIANT, "4230048", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*162*/ { "[423]00", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*163*/ { "[423]0", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, /* iso3166list */
        /*164*/ { "[423]004894004894003", ZINT_WARN_NONCOMPLIANT, "423004894004894003", "261: AI (423) position 13: Unknown country code '003'" }, /* iso3166list */
        /*165*/ { "[423]004894004895004", ZINT_WARN_NONCOMPLIANT, "423004894004895004", "261: AI (423) position 10: Unknown country code '895'" }, /* iso3166list */
        /*166*/ { "[423]004894004999004", ZINT_WARN_NONCOMPLIANT, "423004894004999004", "261: AI (423) position 10: Unknown country code '999'" }, /* iso3166list */
        /*167*/ { "[423]004894005894004", ZINT_WARN_NONCOMPLIANT, "423004894005894004", "261: AI (423) position 7: Unknown country code '005'" }, /* iso3166list */
        /*168*/ { "[423]004893004894004", ZINT_WARN_NONCOMPLIANT, "423004893004894004", "261: AI (423) position 4: Unknown country code '893'" }, /* iso3166list */
        /*169*/ { "[423]004999004894004", ZINT_WARN_NONCOMPLIANT, "423004999004894004", "261: AI (423) position 4: Unknown country code '999'" }, /* iso3166list */
        /*170*/ { "[423]003894004894004", ZINT_WARN_NONCOMPLIANT, "423003894004894004", "261: AI (423) position 1: Unknown country code '003'" }, /* iso3166list */
        /*171*/ { "[423]004894004433", ZINT_WARN_NONCOMPLIANT, "423004894004433", "261: AI (423) position 10: Unknown country code '433'" }, /* iso3166list */
        /*172*/ { "[423]004894435894", ZINT_WARN_NONCOMPLIANT, "423004894435894", "261: AI (423) position 7: Unknown country code '435'" }, /* iso3166list */
        /*173*/ { "[423]004433004894", ZINT_WARN_NONCOMPLIANT, "423004433004894", "261: AI (423) position 4: Unknown country code '433'" }, /* iso3166list */
        /*174*/ { "[423]432894004894", ZINT_WARN_NONCOMPLIANT, "423432894004894", "261: AI (423) position 1: Unknown country code '432'" }, /* iso3166list */
        /*175*/ { "[423]004894003", ZINT_WARN_NONCOMPLIANT, "423004894003", "261: AI (423) position 7: Unknown country code '003'" }, /* iso3166list */
        /*176*/ { "[423]004895004", ZINT_WARN_NONCOMPLIANT, "423004895004", "261: AI (423) position 4: Unknown country code '895'" }, /* iso3166list */
        /*177*/ { "[423]004999004", ZINT_WARN_NONCOMPLIANT, "423004999004", "261: AI (423) position 4: Unknown country code '999'" }, /* iso3166list */
        /*178*/ { "[423]003894004", ZINT_WARN_NONCOMPLIANT, "423003894004", "261: AI (423) position 1: Unknown country code '003'" }, /* iso3166list */
        /*179*/ { "[423]004999", ZINT_WARN_NONCOMPLIANT, "423004999", "261: AI (423) position 4: Unknown country code '999'" }, /* iso3166list */
        /*180*/ { "[423]000894", ZINT_WARN_NONCOMPLIANT, "423000894", "261: AI (423) position 1: Unknown country code '000'" }, /* iso3166list */
        /*181*/ { "[423]003", ZINT_WARN_NONCOMPLIANT, "423003", "261: AI (423) position 1: Unknown country code '003'" }, /* iso3166list */
        /*182*/ { "[7030]999A", 0, "7030999A", "" }, /* iso3166999 */
        /*183*/ { "[7030]894A", 0, "7030894A", "" }, /* iso3166999 */
        /*184*/ { "[7030]004A", 0, "7030004A", "" }, /* iso3166999 */
        /*185*/ { "[7030]004", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7030)" }, /* iso3166999 */
        /*186*/ { "[7030]04", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7030)" }, /* iso3166999 */
        /*187*/ { "[7030]001A", ZINT_WARN_NONCOMPLIANT, "7030001A", "261: AI (7030) position 1: Unknown country code '001'" }, /* iso3166999 */
        /*188*/ { "[7030]998A", ZINT_WARN_NONCOMPLIANT, "7030998A", "261: AI (7030) position 1: Unknown country code '998'" }, /* iso3166999 */
        /*189*/ { "[7030]998A", ZINT_WARN_NONCOMPLIANT, "7030998A", "261: AI (7030) position 1: Unknown country code '998'" }, /* iso3166999 */
        /*190*/ { "[4307]AD", 0, "4307AD", "" }, /* iso3166alpha2 */
        /*191*/ { "[4307]AC", ZINT_WARN_NONCOMPLIANT, "4307AC", "261: AI (4307) position 1: Unknown country code 'AC'" }, /* iso3166alpha2 */
        /*192*/ { "[3910]0081", 0, "39100081", "" }, /* iso4217 */
        /*193*/ { "[3910]9991", 0, "39109991", "" }, /* iso4217 */
        /*194*/ { "[3910]9971", 0, "39109971", "" }, /* iso4217 */
        /*195*/ { "[3910]9251", 0, "39109251", "" }, /* iso4217 */
        /*196*/ { "[3910]01", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3910)" }, /* iso4217 */
        /*197*/ { "[3910]001", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3910)" }, /* iso4217 */
        /*198*/ { "[3910]9981", ZINT_WARN_NONCOMPLIANT, "39109981", "261: AI (3910) position 1: Unknown currency code '998'" }, /* iso4217 */
        /*199*/ { "[3910]0041", ZINT_WARN_NONCOMPLIANT, "39100041", "261: AI (3910) position 1: Unknown currency code '004'" }, /* iso4217 */
        /*200*/ { "[3910]8941", ZINT_WARN_NONCOMPLIANT, "39108941", "261: AI (3910) position 1: Unknown currency code '894'" }, /* iso4217 */
        /*201*/ { "[4300]%12", 0, "4300%12", "" }, /* pcenc */
        /*202*/ { "[4300]%1", ZINT_WARN_NONCOMPLIANT, "4300%1", "261: AI (4300) position 1: Invalid % escape" }, /* pcenc */
        /*203*/ { "[4300]%", ZINT_WARN_NONCOMPLIANT, "4300%", "261: AI (4300) position 1: Invalid % escape" }, /* pcenc */
        /*204*/ { "[4300]12%1212", 0, "430012%1212", "" }, /* pcenc */
        /*205*/ { "[4300]12%1G12", ZINT_WARN_NONCOMPLIANT, "430012%1G12", "261: AI (4300) position 5: Invalid character for percent encoding" }, /* pcenc */
        /*206*/ { "[4308]ABCDEFGHIJKLMNOPQRSTUVWXYZ%+12", 0, "4308ABCDEFGHIJKLMNOPQRSTUVWXYZ%+12", "" }, /* no pcenc */
        /*207*/ { "[4308]ABCDEFGHIJKLMNOPQRSTUVWXYZ%+123", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4308)" }, /* no pcenc */
        /*208*/ { "[4309]02790858483015297971", 0, "430902790858483015297971", "" }, /* latlong */
        /*209*/ { "[4309]18000000013015297971", ZINT_WARN_NONCOMPLIANT, "430918000000013015297971", "261: AI (4309) position 10: Invalid latitude" }, /* latlong */
        /*210*/ { "[4309]02790858413600000001", ZINT_WARN_NONCOMPLIANT, "430902790858413600000001", "261: AI (4309) position 20: Invalid longitude" }, /* latlong */
        /*211*/ { "[4321]1", 0, "43211", "" }, /* yesno */
        /*212*/ { "[4321]0", 0, "43210", "" }, /* yesno */
        /*213*/ { "[4321]2", ZINT_WARN_NONCOMPLIANT, "43212", "261: AI (4321) position 1: Neither 0 nor 1 for yes or no" }, /* yesno */
        /*214*/ { "[4321]9", ZINT_WARN_NONCOMPLIANT, "43219", "261: AI (4321) position 1: Neither 0 nor 1 for yes or no" }, /* yesno */
        /*215*/ { "[7040]1234", 0, "70401234", "" }, /* importeridx */
        /*216*/ { "[7040]123A", 0, "7040123A", "" }, /* importeridx */
        /*217*/ { "[7040]123Z", 0, "7040123Z", "" }, /* importeridx */
        /*218*/ { "[7040]123a", 0, "7040123a", "" }, /* importeridx */
        /*219*/ { "[7040]123z", 0, "7040123z", "" }, /* importeridx */
        /*220*/ { "[7040]123-", 0, "7040123-", "" }, /* importeridx */
        /*221*/ { "[7040]123_", 0, "7040123_", "" }, /* importeridx */
        /*222*/ { "[7040]123!", ZINT_WARN_NONCOMPLIANT, "7040123!", "261: AI (7040) position 4: Invalid importer index '!'" }, /* importeridx */
        /*223*/ { "[7040]123/", ZINT_WARN_NONCOMPLIANT, "7040123/", "261: AI (7040) position 4: Invalid importer index '/'" }, /* importeridx */
        /*224*/ { "[7040]123:", ZINT_WARN_NONCOMPLIANT, "7040123:", "261: AI (7040) position 4: Invalid importer index ':'" }, /* importeridx */
        /*225*/ { "[7040]123?", ZINT_WARN_NONCOMPLIANT, "7040123?", "261: AI (7040) position 4: Invalid importer index '?'" }, /* importeridx */
        /*226*/ { "[7241]01", 0, "724101", "" }, /* mediatype */
        /*227*/ { "[7241]10", 0, "724110", "" }, /* mediatype */
        /*228*/ { "[7241]80", 0, "724180", "" }, /* mediatype */
        /*229*/ { "[7241]99", 0, "724199", "" }, /* mediatype */
        /*230*/ { "[7241]00", ZINT_WARN_NONCOMPLIANT, "724100", "261: AI (7241) position 3: Invalid AIDC media type" }, /* mediatype */
        /*231*/ { "[7241]11", ZINT_WARN_NONCOMPLIANT, "724111", "261: AI (7241) position 3: Invalid AIDC media type" }, /* mediatype */
        /*232*/ { "[7241]79", ZINT_WARN_NONCOMPLIANT, "724179", "261: AI (7241) position 3: Invalid AIDC media type" }, /* mediatype */
        /*233*/ { "[8001]12341234512311", 0, "800112341234512311", "" }, /* nonzero */
        /*234*/ { "[8001]00010000100100", 0, "800100010000100100", "" }, /* nonzero */
        /*235*/ { "[8001]00001234512311", ZINT_WARN_NONCOMPLIANT, "800100001234512311", "261: AI (8001) position 1: Zero not permitted" }, /* nonzero */
        /*236*/ { "[8001]12340000012311", ZINT_WARN_NONCOMPLIANT, "800112340000012311", "261: AI (8001) position 5: Zero not permitted" }, /* nonzero */
        /*237*/ { "[8001]00010000100011", ZINT_WARN_NONCOMPLIANT, "800100010000100011", "261: AI (8001) position 10: Zero not permitted" }, /* nonzero */
        /*238*/ { "[8001]00010000100101", 0, "800100010000100101", "" }, /* winding */
        /*239*/ { "[8001]00010000100111", 0, "800100010000100111", "" }, /* winding */
        /*240*/ { "[8001]00010000100191", 0, "800100010000100191", "" }, /* winding */
        /*241*/ { "[8001]00010000100121", ZINT_WARN_NONCOMPLIANT, "800100010000100121", "261: AI (8001) position 13: Invalid winding direction '2'" }, /* winding */
        /*242*/ { "[8001]00010000100131", ZINT_WARN_NONCOMPLIANT, "800100010000100131", "261: AI (8001) position 13: Invalid winding direction '3'" }, /* winding */
        /*243*/ { "[8001]00010000100171", ZINT_WARN_NONCOMPLIANT, "800100010000100171", "261: AI (8001) position 13: Invalid winding direction '7'" }, /* winding */
        /*244*/ { "[8001]00010000100181", ZINT_WARN_NONCOMPLIANT, "800100010000100181", "261: AI (8001) position 13: Invalid winding direction '8'" }, /* winding */
        /*245*/ { "[8003]01234567890128", 0, "800301234567890128", "" }, /* zero */
        /*246*/ { "[8003]11234567890128", ZINT_WARN_NONCOMPLIANT, "800311234567890128", "261: AI (8003) position 1: Zero is required" }, /* zero */
        /*247*/ { "[8003]91234567890128", ZINT_WARN_NONCOMPLIANT, "800391234567890128", "261: AI (8003) position 1: Zero is required" }, /* zero */
        /*248*/ { "[8006]123456789012310101", 0, "8006123456789012310101", "" }, /* pieceoftotal */
        /*249*/ { "[8006]123456789012310199", 0, "8006123456789012310199", "" }, /* pieceoftotal */
        /*250*/ { "[8006]123456789012319999", 0, "8006123456789012319999", "" }, /* pieceoftotal */
        /*251*/ { "[8006]123456789012310001", ZINT_WARN_NONCOMPLIANT, "8006123456789012310001", "261: AI (8006) position 15: Piece number cannot be zero" }, /* pieceoftotal */
        /*252*/ { "[8006]123456789012310100", ZINT_WARN_NONCOMPLIANT, "8006123456789012310100", "261: AI (8006) position 15: Total number cannot be zero" }, /* pieceoftotal */
        /*253*/ { "[8006]123456789012310201", ZINT_WARN_NONCOMPLIANT, "8006123456789012310201", "261: AI (8006) position 15: Piece number '02' exceeds total '01'" }, /* pieceoftotal */
        /*254*/ { "[8006]123456789012319998", ZINT_WARN_NONCOMPLIANT, "8006123456789012319998", "261: AI (8006) position 15: Piece number '99' exceeds total '98'" }, /* pieceoftotal */
        /*255*/ { "[8007]GB82WEST12345698765432", 0, "8007GB82WEST12345698765432", "" }, /* iban */
        /*256*/ { "[8007]GB83WEST12345698765432", ZINT_WARN_NONCOMPLIANT, "8007GB83WEST12345698765432", "261: AI (8007) position 3: Bad IBAN checksum '83', expected '82'" }, /* iban */
        /*257*/ { "[8007]BE71096123456769", 0, "8007BE71096123456769", "" }, /* iban */
        /*258*/ { "[8007]BE71096123456760", ZINT_WARN_NONCOMPLIANT, "8007BE71096123456760", "261: AI (8007) position 3: Bad IBAN checksum '71', expected '23'" }, /* iban */
        /*259*/ { "[8007]BE01096123456760", ZINT_WARN_NONCOMPLIANT, "8007BE01096123456760", "261: AI (8007) position 3: Bad IBAN checksum '01', expected '23'" }, /* iban */
        /*260*/ { "[8007]BE00096123456760", ZINT_WARN_NONCOMPLIANT, "8007BE00096123456760", "261: AI (8007) position 3: Bad IBAN checksum '00', expected '23'" }, /* iban */
        /*261*/ { "[8007]LC14BOSL123456789012345678901234", 0, "8007LC14BOSL123456789012345678901234", "" }, /* iban */
        /*262*/ { "[8007]LC14BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007LC14BOSL123456789012345678901230", "261: AI (8007) position 3: Bad IBAN checksum '14', expected '25'" }, /* iban */
        /*263*/ { "[8007]A114BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007A114BOSL123456789012345678901230", "261: AI (8007) position 1: Non-alphabetic IBAN country code 'A1'" }, /* iban */
        /*264*/ { "[8007]1A14BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "80071A14BOSL123456789012345678901230", "261: AI (8007) position 1: Non-alphabetic IBAN country code '1A'" }, /* iban */
        /*265*/ { "[8007]AA14BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007AA14BOSL123456789012345678901230", "261: AI (8007) position 1: Invalid IBAN country code 'AA'" }, /* iban */
        /*266*/ { "[8007]ZZ14BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007ZZ14BOSL123456789012345678901230", "261: AI (8007) position 1: Invalid IBAN country code 'ZZ'" }, /* iban */
        /*267*/ { "[8007]ZW33BOSL123456789012345678901230", 0, "8007ZW33BOSL123456789012345678901230", "" }, /* iban */
        /*268*/ { "[8007]ZWA3BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007ZWA3BOSL123456789012345678901230", "261: AI (8007) position 3: Non-numeric IBAN checksum 'A3'" }, /* iban */
        /*269*/ { "[8007]ZW3ABOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007ZW3ABOSL123456789012345678901230", "261: AI (8007) position 3: Non-numeric IBAN checksum '3A'" }, /* iban */
        /*270*/ { "[8007]ZW33bOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007ZW33bOSL123456789012345678901230", "261: AI (8007) position 5: Invalid IBAN character 'b'" }, /* iban */
        /*271*/ { "[8007]GB98", ZINT_WARN_NONCOMPLIANT, "8007GB98", "259: Invalid data length for AI (8007)" }, /* iban */
        /*272*/ { "[8007]FR7630006000011234567890189", 0, "8007FR7630006000011234567890189", "" }, /* iban */
        /*273*/ { "[8007]DE91100000000123456789", 0, "8007DE91100000000123456789", "" }, /* iban */
        /*274*/ { "[8007]GR9608100010000001234567890", 0, "8007GR9608100010000001234567890", "" }, /* iban */
        /*275*/ { "[8007]MU43BOMM0101123456789101000MUR", 0, "8007MU43BOMM0101123456789101000MUR", "" }, /* iban */
        /*276*/ { "[8007]PL10105000997603123456789123", 0, "8007PL10105000997603123456789123", "" }, /* iban */
        /*277*/ { "[8007]RO09BCYP0000001234567890", 0, "8007RO09BCYP0000001234567890", "" }, /* iban */
        /*278*/ { "[8007]SA4420000001234567891234", 0, "8007SA4420000001234567891234", "" }, /* iban */
        /*279*/ { "[8007]ES7921000813610123456789", 0, "8007ES7921000813610123456789", "" }, /* iban */
        /*280*/ { "[8007]CH5604835012345678009", 0, "8007CH5604835012345678009", "" }, /* iban */
        /*281*/ { "[8007]GB98MIDL07009312345678", 0, "8007GB98MIDL07009312345678", "" }, /* iban */
        /*282*/ { "[8011]1", 0, "80111", "" }, /* nozeroprefix */
        /*283*/ { "[8011]11", 0, "801111", "" }, /* nozeroprefix */
        /*284*/ { "[8011]0", 0, "80110", "" }, /* nozeroprefix */
        /*285*/ { "[8011]01", ZINT_WARN_NONCOMPLIANT, "801101", "261: AI (8011) position 1: Zero prefix is not permitted" }, /* nozeroprefix */
        /*286*/ { "[8110]106141416543213150110120", 0, "8110106141416543213150110120", "" }, /* couponcode (first part of NACAG Appendix C: Example 1 - see test_rss.c test_examples) */
        /*287*/ { "[8110]012345612345610104123", 0, "8110012345612345610104123", "" }, /* couponcode */
        /*288*/ { "[8110]01234561234561010412", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412", "259: Invalid data length for AI (8110)" }, /* couponcode */
        /*289*/ { "[8110]12345678901234567890", ZINT_WARN_NONCOMPLIANT, "811012345678901234567890", "259: Invalid data length for AI (8110)" }, /* couponcode */
        /*290*/ { "[8110]712345612345610104123", ZINT_WARN_NONCOMPLIANT, "8110712345612345610104123", "261: AI (8110) position 1: Invalid Primary GS1 Co. Prefix VLI '7'" }, /* couponcode */
        /*291*/ { "[8110]A12345612345610104123", ZINT_WARN_NONCOMPLIANT, "8110A12345612345610104123", "261: AI (8110) position 1: Non-numeric Primary GS1 Co. Prefix VLI 'A'" }, /* couponcode */
        /*292*/ { "[8110]012345A12345610104123", ZINT_WARN_NONCOMPLIANT, "8110012345A12345610104123", "261: AI (8110) position 7: Non-numeric Primary GS1 Co. Prefix 'A'" }, /* couponcode */
        /*293*/ { "[8110]012345612345A10104123", ZINT_WARN_NONCOMPLIANT, "8110012345612345A10104123", "261: AI (8110) position 8: Non-numeric Offer Code" }, /* couponcode */
        /*294*/ { "[8110]012345612345600104123", ZINT_WARN_NONCOMPLIANT, "8110012345612345600104123", "261: AI (8110) position 14: Invalid Save Value VLI '0'" }, /* couponcode */
        /*295*/ { "[8110]012345612345660104123", ZINT_WARN_NONCOMPLIANT, "8110012345612345660104123", "261: AI (8110) position 14: Invalid Save Value VLI '6'" }, /* couponcode */
        /*296*/ { "[8110]01234561234561A104123", ZINT_WARN_NONCOMPLIANT, "811001234561234561A104123", "261: AI (8110) position 15: Non-numeric Save Value 'A'" }, /* couponcode */
        /*297*/ { "[8110]012345612345610004123", ZINT_WARN_NONCOMPLIANT, "8110012345612345610004123", "261: AI (8110) position 16: Invalid Primary Purch. Req. VLI '0'" }, /* couponcode */
        /*298*/ { "[8110]012345612345610604123", ZINT_WARN_NONCOMPLIANT, "8110012345612345610604123", "261: AI (8110) position 16: Invalid Primary Purch. Req. VLI '6'" }, /* couponcode */
        /*299*/ { "[8110]0123456123456101A4123", ZINT_WARN_NONCOMPLIANT, "81100123456123456101A4123", "261: AI (8110) position 17: Non-numeric Primary Purch. Req. 'A'" }, /* couponcode */
        /*300*/ { "[8110]012345612345621251234", ZINT_WARN_NONCOMPLIANT, "8110012345612345621251234", "261: AI (8110) position 18: Primary Purch. Req. incomplete" }, /* couponcode */
        /*301*/ { "[8110]01234561234561010A123", ZINT_WARN_NONCOMPLIANT, "811001234561234561010A123", "261: AI (8110) position 18: Non-numeric Primary Purch. Req. Code" }, /* couponcode */
        /*302*/ { "[8110]012345612345610106123", ZINT_WARN_NONCOMPLIANT, "8110012345612345610106123", "261: AI (8110) position 18: Invalid Primary Purch. Req. Code '6'" }, /* couponcode */
        /*303*/ { "[8110]012345612345610212412", ZINT_WARN_NONCOMPLIANT, "8110012345612345610212412", "261: AI (8110) position 20: Primary Purch. Family Code incomplete" }, /* couponcode */
        /*304*/ { "[8110]0123456123456103123412", ZINT_WARN_NONCOMPLIANT, "81100123456123456103123412", "261: AI (8110) position 21: Primary Purch. Family Code incomplete" }, /* couponcode */
        /*305*/ { "[8110]0123456123456103123412A", ZINT_WARN_NONCOMPLIANT, "81100123456123456103123412A", "261: AI (8110) position 21: Non-numeric Primary Purch. Family Code" }, /* couponcode */
        /*306*/ { "[8110]01234561234561031234123", 0, "811001234561234561031234123", "" }, /* couponcode */
        /*307*/ { "[8110]612345678901212345651", ZINT_WARN_NONCOMPLIANT, "8110612345678901212345651", "261: AI (8110) position 21: Save Value incomplete" }, /* couponcode */
        /*308*/ { "[8110]6123456789012123456512345", ZINT_WARN_NONCOMPLIANT, "81106123456789012123456512345", "261: AI (8110) position 26: Primary Purch. Req. VLI missing" }, /* couponcode */
        /*309*/ { "[8110]61234567890121234565123455123454123", 0, "811061234567890121234565123455123454123", "" }, /* couponcode */
        /*310*/ { "[8110]61234567890121234565123455123454123A", ZINT_WARN_NONCOMPLIANT, "811061234567890121234565123455123454123A", "261: AI (8110) position 36: Non-numeric Data Field 'A'" }, /* couponcode */
        /*311*/ { "[8110]612345678901212345651234551234541237", ZINT_WARN_NONCOMPLIANT, "8110612345678901212345651234551234541237", "261: AI (8110) position 36: Invalid Data Field '7'" }, /* couponcode */
        /*312*/ { "[8110]612345678901212345651234551234541238", ZINT_WARN_NONCOMPLIANT, "8110612345678901212345651234551234541238", "261: AI (8110) position 36: Invalid Data Field '8'" }, /* couponcode */
        /*313*/ { "[8110]0123456123456101041231", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041231", "261: AI (8110) position 23: Add. Purch. Rules Code incomplete" }, /* couponcode */
        /*314*/ { "[8110]0123456123456101041231A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041231A", "261: AI (8110) position 23: Non-numeric Add. Purch. Rules Code" }, /* couponcode */
        /*315*/ { "[8110]01234561234561010412314", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412314", "261: AI (8110) position 23: Invalid Add. Purch. Rules Code '4'" }, /* couponcode */
        /*316*/ { "[8110]01234561234561010412313", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313", "261: AI (8110) position 24: 2nd Purch. Req. VLI missing" }, /* couponcode */
        /*317*/ { "[8110]01234561234561010412313A", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313A", "261: AI (8110) position 24: Non-numeric 2nd Purch. Req. VLI 'A'" }, /* couponcode */
        /*318*/ { "[8110]012345612345610104123130", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123130", "261: AI (8110) position 24: Invalid 2nd Purch. Req. VLI '0'" }, /* couponcode */
        /*319*/ { "[8110]012345612345610104123131", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123131", "261: AI (8110) position 25: 2nd Purch. Req. incomplete" }, /* couponcode */
        /*320*/ { "[8110]012345612345610104123131A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123131A", "261: AI (8110) position 25: Non-numeric 2nd Purch. Req. 'A'" }, /* couponcode */
        /*321*/ { "[8110]0123456123456101041231310", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041231310", "261: AI (8110) position 26: 2nd Purch. Req. Code incomplete" }, /* couponcode */
        /*322*/ { "[8110]0123456123456101041231310A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041231310A", "261: AI (8110) position 26: Non-numeric 2nd Purch. Req. Code" }, /* couponcode */
        /*323*/ { "[8110]01234561234561010412313108", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313108", "261: AI (8110) position 26: Invalid 2nd Purch. Req. Code '8'" }, /* couponcode */
        /*324*/ { "[8110]01234561234561010412313100", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313100", "261: AI (8110) position 27: 2nd Purch. Family Code incomplete" }, /* couponcode */
        /*325*/ { "[8110]01234561234561010412313100123", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313100123", "261: AI (8110) position 30: 2nd Purch. GS1 Co. Prefix VLI missing" }, /* couponcode */
        /*326*/ { "[8110]01234561234561010412313100123A", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313100123A", "261: AI (8110) position 30: Non-numeric 2nd Purch. GS1 Co. Prefix VLI 'A'" }, /* couponcode */
        /*327*/ { "[8110]012345612345610104123131001239", 0, "8110012345612345610104123131001239", "" }, /* couponcode */
        /*328*/ { "[8110]01234561234561010412313100123012345", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313100123012345", "261: AI (8110) position 31: 2nd Purch. GS1 Co. Prefix incomplete" }, /* couponcode */
        /*329*/ { "[8110]0123456123456101041231310012311234567", 0, "81100123456123456101041231310012311234567", "" }, /* couponcode */
        /*330*/ { "[8110]0123456123456101041232", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232", "261: AI (8110) position 23: 3rd Purch. Req. VLI missing" }, /* couponcode */
        /*331*/ { "[8110]0123456123456101041232A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232A", "261: AI (8110) position 23: Non-numeric 3rd Purch. Req. VLI 'A'" }, /* couponcode */
        /*332*/ { "[8110]01234561234561010412326", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412326", "261: AI (8110) position 23: Invalid 3rd Purch. Req. VLI '6'" }, /* couponcode */
        /*333*/ { "[8110]01234561234561010412321", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412321", "261: AI (8110) position 24: 3rd Purch. Req. incomplete" }, /* couponcode */
        /*334*/ { "[8110]012345612345610104123210", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123210", "261: AI (8110) position 25: 3rd Purch. Req. Code incomplete" }, /* couponcode */
        /*335*/ { "[8110]0123456123456101041232105", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232105", "261: AI (8110) position 25: Invalid 3rd Purch. Req. Code '5'" }, /* couponcode */
        /*336*/ { "[8110]0123456123456101041232104", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104", "261: AI (8110) position 26: 3rd Purch. Family Code incomplete" }, /* couponcode */
        /*337*/ { "[8110]012345612345610104123210412A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123210412A", "261: AI (8110) position 26: Non-numeric 3rd Purch. Family Code" }, /* couponcode */
        /*338*/ { "[8110]0123456123456101041232104123", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104123", "261: AI (8110) position 29: 3rd Purch. GS1 Co. Prefix VLI missing" }, /* couponcode */
        /*339*/ { "[8110]01234561234561010412321041230", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412321041230", "261: AI (8110) position 30: 3rd Purch. GS1 Co. Prefix incomplete" }, /* couponcode */
        /*340*/ { "[8110]0123456123456101041232104123A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104123A", "261: AI (8110) position 29: Non-numeric 3rd Purch. GS1 Co. Prefix VLI 'A'" }, /* couponcode */
        /*341*/ { "[8110]0123456123456101041232104123012345", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104123012345", "261: AI (8110) position 30: 3rd Purch. GS1 Co. Prefix incomplete" }, /* couponcode */
        /*342*/ { "[8110]0123456123456101041232104123012345A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104123012345A", "261: AI (8110) position 35: Non-numeric 3rd Purch. GS1 Co. Prefix 'A'" }, /* couponcode */
        /*343*/ { "[8110]01234561234561010412321041230123456", 0, "811001234561234561010412321041230123456", "" }, /* couponcode */
        /*344*/ { "[8110]0123456123456101041233", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041233", "261: AI (8110) position 23: Expiration Date incomplete" }, /* couponcode */
        /*345*/ { "[8110]01234561234561010412332012", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412332012", "261: AI (8110) position 23: Expiration Date incomplete" }, /* couponcode */
        /*346*/ { "[8110]012345612345610104123320123A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123320123A", "261: AI (8110) position 23: Non-numeric Expiration Date" }, /* couponcode */
        /*347*/ { "[8110]0123456123456101041233201232", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041233201232", "261: AI (8110) position 27: Invalid day '32'" }, /* couponcode */
        /*348*/ { "[8110]0123456123456101041233200031", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041233200031", "261: AI (8110) position 25: Invalid month '00'" }, /* couponcode */
        /*349*/ { "[8110]0123456123456101041233201231", 0, "81100123456123456101041233201231", "" }, /* couponcode */
        /*350*/ { "[8110]0123456123456101041234", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041234", "261: AI (8110) position 23: Start Date incomplete" }, /* couponcode */
        /*351*/ { "[8110]01234561234561010412342012", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412342012", "261: AI (8110) position 23: Start Date incomplete" }, /* couponcode */
        /*352*/ { "[8110]012345612345610104123420123A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123420123A", "261: AI (8110) position 23: Non-numeric Start Date" }, /* couponcode */
        /*353*/ { "[8110]0123456123456101041234200230", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041234200230", "261: AI (8110) position 27: Invalid day '30'" }, /* couponcode */
        /*354*/ { "[8110]0123456123456101041234201329", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041234201329", "261: AI (8110) position 25: Invalid month '13'" }, /* couponcode */
        /*355*/ { "[8110]0123456123456101041234200229", 0, "81100123456123456101041234200229", "" }, /* couponcode */
        /*356*/ { "[8110]0123456123456101041235", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235", "261: AI (8110) position 23: Serial Number VLI missing" }, /* couponcode */
        /*357*/ { "[8110]0123456123456101041235A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235A", "261: AI (8110) position 23: Non-numeric Serial Number VLI 'A'" }, /* couponcode */
        /*358*/ { "[8110]01234561234561010412350", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412350", "261: AI (8110) position 24: Serial Number incomplete" }, /* couponcode */
        /*359*/ { "[8110]0123456123456101041235012345", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235012345", "261: AI (8110) position 24: Serial Number incomplete" }, /* couponcode */
        /*360*/ { "[8110]0123456123456101041235912345678901234", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235912345678901234", "261: AI (8110) position 24: Serial Number incomplete" }, /* couponcode */
        /*361*/ { "[8110]0123456123456101041235912345678901234A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235912345678901234A", "261: AI (8110) position 38: Non-numeric Serial Number 'A'" }, /* couponcode */
        /*362*/ { "[8110]01234561234561010412359123456789012345", 0, "811001234561234561010412359123456789012345", "" }, /* couponcode */
        /*363*/ { "[8110]0123456123456101041236", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041236", "261: AI (8110) position 23: Retailer ID VLI missing" }, /* couponcode */
        /*364*/ { "[8110]0123456123456101041236A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041236A", "261: AI (8110) position 23: Non-numeric Retailer ID VLI 'A'" }, /* couponcode */
        /*365*/ { "[8110]01234561234561010412360", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412360", "261: AI (8110) position 23: Invalid Retailer ID VLI '0'" }, /* couponcode */
        /*366*/ { "[8110]01234561234561010412368", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412368", "261: AI (8110) position 23: Invalid Retailer ID VLI '8'" }, /* couponcode */
        /*367*/ { "[8110]01234561234561010412361", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412361", "261: AI (8110) position 24: Retailer ID incomplete" }, /* couponcode */
        /*368*/ { "[8110]01234561234561010412361123456", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412361123456", "261: AI (8110) position 24: Retailer ID incomplete" }, /* couponcode */
        /*369*/ { "[8110]01234561234561010412361123456A", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412361123456A", "261: AI (8110) position 30: Non-numeric Retailer ID 'A'" }, /* couponcode */
        /*370*/ { "[8110]012345612345610104123671234567890123", 0, "8110012345612345610104123671234567890123", "" }, /* couponcode */
        /*371*/ { "[8110]0123456123456101041239", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041239", "261: AI (8110) position 23: Save Value Code incomplete" }, /* couponcode */
        /*372*/ { "[8110]0123456123456101041239A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041239A", "261: AI (8110) position 23: Non-numeric Save Value Code" }, /* couponcode */
        /*373*/ { "[8110]01234561234561010412393", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412393", "261: AI (8110) position 23: Invalid Save Value Code '3'" }, /* couponcode */
        /*374*/ { "[8110]01234561234561010412394", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412394", "261: AI (8110) position 23: Invalid Save Value Code '4'" }, /* couponcode */
        /*375*/ { "[8110]01234561234561010412397", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412397", "261: AI (8110) position 23: Invalid Save Value Code '7'" }, /* couponcode */
        /*376*/ { "[8110]01234561234561010412390", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412390", "261: AI (8110) position 24: Save Value Applies To incomplete" }, /* couponcode */
        /*377*/ { "[8110]01234561234561010412390A", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412390A", "261: AI (8110) position 24: Non-numeric Save Value Applies To" }, /* couponcode */
        /*378*/ { "[8110]012345612345610104123903", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123903", "261: AI (8110) position 24: Invalid Save Value Applies To '3'" }, /* couponcode */
        /*379*/ { "[8110]012345612345610104123902", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123902", "261: AI (8110) position 25: Store Coupon Flag incomplete" }, /* couponcode */
        /*380*/ { "[8110]012345612345610104123902A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123902A", "261: AI (8110) position 25: Non-numeric Store Coupon Flag" }, /* couponcode */
        /*381*/ { "[8110]0123456123456101041239029", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041239029", "261: AI (8110) position 26: Don't Multiply Flag incomplete" }, /* couponcode */
        /*382*/ { "[8110]0123456123456101041239029A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041239029A", "261: AI (8110) position 26: Non-numeric Don't Multiply Flag" }, /* couponcode */
        /*383*/ { "[8110]01234561234561010412390292", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412390292", "261: AI (8110) position 26: Invalid Don't Multiply Flag '2'" }, /* couponcode */
        /*384*/ { "[8110]01234561234561010412390291", 0, "811001234561234561010412390291", "" }, /* couponcode */
        /*385*/ { "[8110]177777776666663100120444101105551888888821109991222222232012314200601522345678961345678990000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8110)" }, /* couponcode (example from GS1 AI (8112) Coupon Data Specifications Appendix A: AI (8110) vs AI (8112)) */
        /*386*/ { "[8110]177777776666663100120444101105551888888821109991222222232012314200601", 0, "8110177777776666663100120444101105551888888821109991222222232012314200601", "" }, /* couponcode */
        /*387*/ { "[8112]017777777666666223456789", 0, "8112017777777666666223456789", "" }, /* couponposoffer (example from GS1 AI (8112) Coupon Data Specifications Appendix A: AI (8110) vs AI (8112)) */
        /*388*/ { "[8112]001234561234560123456", 0, "8112001234561234560123456", "" }, /* couponposoffer */
        /*389*/ { "[8112]00123456123456012345", ZINT_WARN_NONCOMPLIANT, "811200123456123456012345", "259: Invalid data length for AI (8112)" }, /* couponposoffer */
        /*390*/ { "[8112]0012345612345601234561", ZINT_WARN_NONCOMPLIANT, "81120012345612345601234561", "261: AI (8112) position 22: Reserved trailing characters" }, /* couponposoffer */
        /*391*/ { "[8112]061234567890121234569123456789012345", 0, "8112061234567890121234569123456789012345", "" }, /* couponposoffer */
        /*392*/ { "[8112]0612345678901212345691234567890123456", ZINT_WARN_NONCOMPLIANT, "81120612345678901212345691234567890123456", "259: Invalid data length for AI (8112)" }, /* couponposoffer */
        /*393*/ { "[8112]06123456789012123456912345678901234A", ZINT_WARN_NONCOMPLIANT, "811206123456789012123456912345678901234A", "261: AI (8112) position 36: Non-numeric Serial Number 'A'" }, /* couponposoffer */
        /*394*/ { "[8112]06123456789012123456912345678901234", ZINT_WARN_NONCOMPLIANT, "811206123456789012123456912345678901234", "261: AI (8112) position 22: Serial Number incomplete" }, /* couponposoffer */
        /*395*/ { "[8112]06123456789012123456812345678901234", 0, "811206123456789012123456812345678901234", "" }, /* couponposoffer */
        /*396*/ { "[8112]0612345678901212345681234567890123", ZINT_WARN_NONCOMPLIANT, "81120612345678901212345681234567890123", "261: AI (8112) position 22: Serial Number incomplete" }, /* couponposoffer */
        /*397*/ { "[8112]0612345678901212345A0123456", ZINT_WARN_NONCOMPLIANT, "81120612345678901212345A0123456", "261: AI (8112) position 15: Non-numeric Offer Code" }, /* couponposoffer */
        /*398*/ { "[8112]0612345678901A1234560123456", ZINT_WARN_NONCOMPLIANT, "81120612345678901A1234560123456", "261: AI (8112) position 14: Non-numeric Coupon Funder ID 'A'" }, /* couponposoffer */
        /*399*/ { "[8112]071234567890121234560123456", ZINT_WARN_NONCOMPLIANT, "8112071234567890121234560123456", "261: AI (8112) position 2: Invalid Coupon Funder ID VLI '7'" }, /* couponposoffer */
        /*400*/ { "[8112]0A1234567890121234560123456", ZINT_WARN_NONCOMPLIANT, "81120A1234567890121234560123456", "261: AI (8112) position 2: Non-numeric Coupon Funder ID VLI 'A'" }, /* couponposoffer */
        /*401*/ { "[8112]261234567890121234560123456", ZINT_WARN_NONCOMPLIANT, "8112261234567890121234560123456", "261: AI (8112) position 1: Coupon Format must be 0 or 1" }, /* couponposoffer */
        /*402*/ { "[8112]A61234567890121234560123456", ZINT_WARN_NONCOMPLIANT, "8112A61234567890121234560123456", "261: AI (8112) position 1: Non-numeric Coupon Format" }, /* couponposoffer */
        /*403*/ { "[4330]023020", 0, "4330023020", "" }, /* hyphen */
        /*404*/ { "[4330]023020-", 0, "4330023020-", "" }, /* hyphen */
        /*405*/ { "[4330]023020+", ZINT_WARN_NONCOMPLIANT, "4330023020+", "261: AI (4330) position 7: Invalid temperature indicator (hyphen only)" }, /* hyphen */
        /*406*/ { "[4330]02302", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4330)" }, /* hyphen */
        /*407*/ { "[4330]02302000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (4330)" }, /* hyphen */
        /*408*/ { "[7252]0", 0, "72520", "" }, /* iso5218 */
        /*409*/ { "[7252]1", 0, "72521", "" }, /* iso5218 */
        /*410*/ { "[7252]2", 0, "72522", "" }, /* iso5218 */
        /*411*/ { "[7252]9", 0, "72529", "" }, /* iso5218 */
        /*412*/ { "[7252]3", ZINT_WARN_NONCOMPLIANT, "72523", "261: AI (7252) position 1: Invalid biological sex code (0, 1, 2 or 9 only)" }, /* iso5218 */
        /*413*/ { "[7252]5", ZINT_WARN_NONCOMPLIANT, "72525", "261: AI (7252) position 1: Invalid biological sex code (0, 1, 2 or 9 only)" }, /* iso5218 */
        /*414*/ { "[7252]8", ZINT_WARN_NONCOMPLIANT, "72528", "261: AI (7252) position 1: Invalid biological sex code (0, 1, 2 or 9 only)" }, /* iso5218 */
        /*415*/ { "[7258]1/2", 0, "72581/2", "" }, /* posinseqslash */
        /*416*/ { "[7258]2/2", 0, "72582/2", "" }, /* posinseqslash */
        /*417*/ { "[7258]1/9", 0, "72581/9", "" }, /* posinseqslash */
        /*418*/ { "[7258]8/9", 0, "72588/9", "" }, /* posinseqslash */
        /*419*/ { "[7258]9/9", 0, "72589/9", "" }, /* posinseqslash */
        /*420*/ { "[7258]0/2", ZINT_WARN_NONCOMPLIANT, "72580/2", "261: AI (7258) position 1: Sequence position cannot be zero" }, /* posinseqslash */
        /*421*/ { "[7258]1/0", ZINT_WARN_NONCOMPLIANT, "72581/0", "261: AI (7258) position 3: Sequence total cannot be zero" }, /* posinseqslash */
        /*422*/ { "[7258]2/1", ZINT_WARN_NONCOMPLIANT, "72582/1", "261: AI (7258) position 1: Sequence position greater than total" }, /* posinseqslash */
        /*423*/ { "[7258]111", ZINT_WARN_NONCOMPLIANT, "7258111", "261: AI (7258) position 1: No sequence separator ('/')" }, /* posinseqslash */
        /*424*/ { "[7258]1//", ZINT_WARN_NONCOMPLIANT, "72581//", "261: AI (7258) position 3: Single sequence separator ('/') only" }, /* posinseqslash */
        /*425*/ { "[7258]/11", ZINT_WARN_NONCOMPLIANT, "7258/11", "261: AI (7258) position 1: Sequence separator '/' cannot start or end" }, /* posinseqslash */
        /*426*/ { "[7258]11/", ZINT_WARN_NONCOMPLIANT, "725811/", "261: AI (7258) position 3: Sequence separator '/' cannot start or end" }, /* posinseqslash */
        /*427*/ { "[7258]1+1", ZINT_WARN_NONCOMPLIANT, "72581+1", "261: AI (7258) position 2: Invalid character '+' in sequence" }, /* posinseqslash */
        /*428*/ { "[7258]1/A", ZINT_WARN_NONCOMPLIANT, "72581/A", "261: AI (7258) position 3: Invalid character 'A' in sequence" }, /* posinseqslash */
        /*429*/ { "[8014]990000A35", 0, "8014990000A35", "" }, /* hasnondigit */
        /*430*/ { "[8014]990000025", ZINT_WARN_NONCOMPLIANT, "8014990000025", "261: AI (8014) position 1: A non-digit character is required" }, /* hasnondigit */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char reduced[1024];

    testStart("test_gs1_lint");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = (int) strlen(data[i].data);

        ret = gs1_verify(symbol, (unsigned char *) data[i].data, length, (unsigned char *) reduced);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (length %d \"%s\") (%s)\n", i, ret, data[i].ret, length, data[i].data, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_zero(strcmp(reduced, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, reduced, data[i].expected);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/*
 * Check GS1_MODE for non-forced GS1 compliant symbologies (see gs1_compliant() in library.c)
 */
static void test_input_mode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        char *data;
        int input_mode;
        int output_options;
        int ret;
        int compare_previous;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_AZTEC, "[01]12345678901231", GS1_MODE, -1 , 0, 0 },
        /*  1*/ { BARCODE_AZTEC, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /*  2*/ { BARCODE_AZTEC, "(01)12345678901231", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, 0, 1 },
        /*  3*/ { BARCODE_AZTEC, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /*  4*/ { BARCODE_AZTEC, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /*  5*/ { BARCODE_AZTEC, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*  6*/ { BARCODE_AZTEC, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /*  7*/ { BARCODE_AZTEC, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*  8*/ { BARCODE_AZTEC, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /*  9*/ { BARCODE_AZTEC, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 10*/ { BARCODE_AZTEC, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 11*/ { BARCODE_AZTEC, "[01]1234567890123", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 12*/ { BARCODE_AZTEC, "[01]1234567890123", GS1_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 13*/ { BARCODE_AZTEC, "[01]12345678901231", GS1_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 14*/ { BARCODE_AZTEC, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 15*/ { BARCODE_AZTEC, "1234", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 16*/ { BARCODE_AZTEC, "1234", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 17*/ { BARCODE_AZTEC, "1234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 }, /* Must still begin with AI */
        /* 18*/ { BARCODE_CODABLOCKF, "[01]12345678901231", GS1_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 }, /* Codablock-F does not support GS1 */
        /* 19*/ { BARCODE_CODABLOCKF, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 20*/ { BARCODE_CODABLOCKF, "[01]12345678901231", GS1_MODE | ESCAPE_MODE | GS1NOCHECK_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 21*/ { BARCODE_CODABLOCKF, "1234", GS1_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 22*/ { BARCODE_CODABLOCKF, "1234", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 23*/ { BARCODE_CODEONE, "[01]12345678901231", GS1_MODE, -1, 0, 0 },
        /* 24*/ { BARCODE_CODEONE, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 25*/ { BARCODE_CODEONE, "(01)12345678901231", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, 0, 1 },
        /* 26*/ { BARCODE_CODEONE, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 27*/ { BARCODE_CODEONE, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 28*/ { BARCODE_CODEONE, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 29*/ { BARCODE_CODEONE, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 30*/ { BARCODE_CODEONE, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 31*/ { BARCODE_CODEONE, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 32*/ { BARCODE_CODEONE, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 33*/ { BARCODE_CODEONE, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 34*/ { BARCODE_CODEONE, "[01]1234567890123", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 35*/ { BARCODE_CODEONE, "[01]1234567890123", GS1_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 36*/ { BARCODE_CODEONE, "1234", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 37*/ { BARCODE_CODEONE, "1234", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 38*/ { BARCODE_CODE16K, "[01]12345678901231", GS1_MODE, -1, 0, 0 },
        /* 39*/ { BARCODE_CODE16K, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 40*/ { BARCODE_CODE16K, "(01)12345678901231", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, 0, 1 },
        /* 41*/ { BARCODE_CODE16K, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 42*/ { BARCODE_CODE16K, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 43*/ { BARCODE_CODE16K, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 44*/ { BARCODE_CODE16K, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 45*/ { BARCODE_CODE16K, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 46*/ { BARCODE_CODE16K, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 47*/ { BARCODE_CODE16K, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 48*/ { BARCODE_CODE16K, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 49*/ { BARCODE_CODE16K, "[01]1234567890123", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 50*/ { BARCODE_CODE16K, "[01]1234567890123", GS1_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 51*/ { BARCODE_CODE16K, "[01]12345678901231", GS1_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 52*/ { BARCODE_CODE16K, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 53*/ { BARCODE_CODE16K, "1234", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 54*/ { BARCODE_CODE16K, "1234", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 55*/ { BARCODE_CODE49, "[01]12345678901231", GS1_MODE, -1, 0, 0 },
        /* 56*/ { BARCODE_CODE49, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 57*/ { BARCODE_CODE49, "(01)12345678901231", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, 0, 1 },
        /* 58*/ { BARCODE_CODE49, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 59*/ { BARCODE_CODE49, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 60*/ { BARCODE_CODE49, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 61*/ { BARCODE_CODE49, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 62*/ { BARCODE_CODE49, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 63*/ { BARCODE_CODE49, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 64*/ { BARCODE_CODE49, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 65*/ { BARCODE_CODE49, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 66*/ { BARCODE_CODE49, "[01]1234567890123", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 67*/ { BARCODE_CODE49, "[01]1234567890123", GS1_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 68*/ { BARCODE_CODE49, "1234", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 69*/ { BARCODE_CODE49, "1234", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 70*/ { BARCODE_DATAMATRIX, "[01]12345678901231", GS1_MODE, -1, 0, 0 },
        /* 71*/ { BARCODE_DATAMATRIX, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 72*/ { BARCODE_DATAMATRIX, "(01)12345678901231", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, 0, 1 },
        /* 73*/ { BARCODE_DATAMATRIX, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 74*/ { BARCODE_DATAMATRIX, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 75*/ { BARCODE_DATAMATRIX, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 76*/ { BARCODE_DATAMATRIX, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 77*/ { BARCODE_DATAMATRIX, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 78*/ { BARCODE_DATAMATRIX, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 79*/ { BARCODE_DATAMATRIX, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 80*/ { BARCODE_DATAMATRIX, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 81*/ { BARCODE_DATAMATRIX, "[01]1234567890123", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 82*/ { BARCODE_DATAMATRIX, "[01]1234567890123", GS1_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 83*/ { BARCODE_DATAMATRIX, "[01]12345678901231", GS1_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 84*/ { BARCODE_DATAMATRIX, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 85*/ { BARCODE_DATAMATRIX, "1234", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 86*/ { BARCODE_DATAMATRIX, "1234", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 87*/ { BARCODE_DOTCODE, "[01]12345678901231", GS1_MODE, -1, 0, 0 },
        /* 88*/ { BARCODE_DOTCODE, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 89*/ { BARCODE_DOTCODE, "(01)12345678901231", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, 0, 1 },
        /* 90*/ { BARCODE_DOTCODE, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 91*/ { BARCODE_DOTCODE, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 92*/ { BARCODE_DOTCODE, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 93*/ { BARCODE_DOTCODE, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 94*/ { BARCODE_DOTCODE, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 95*/ { BARCODE_DOTCODE, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /* 96*/ { BARCODE_DOTCODE, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /* 97*/ { BARCODE_DOTCODE, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /* 98*/ { BARCODE_DOTCODE, "[01]1234567890123", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 99*/ { BARCODE_DOTCODE, "[01]1234567890123", GS1_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /*100*/ { BARCODE_DOTCODE, "[01]12345678901231", GS1_MODE, READER_INIT, 0, 0 }, /* Reader Init permissible with default GS1 mode */
        /*101*/ { BARCODE_DOTCODE, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, READER_INIT, 0, 1 },
        /*102*/ { BARCODE_DOTCODE, "1234", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*103*/ { BARCODE_DOTCODE, "1234", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*104*/ { BARCODE_QRCODE, "[01]12345678901231", GS1_MODE, -1, 0, 0 },
        /*105*/ { BARCODE_QRCODE, "[01]12345678901231", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /*106*/ { BARCODE_QRCODE, "(01)12345678901231", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, 0, 1 },
        /*107*/ { BARCODE_QRCODE, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /*108*/ { BARCODE_QRCODE, "(01)12345678901234", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /*109*/ { BARCODE_QRCODE, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*110*/ { BARCODE_QRCODE, "(01)123456789012345", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /*111*/ { BARCODE_QRCODE, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*112*/ { BARCODE_QRCODE, "(01)12345678901234A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /*113*/ { BARCODE_QRCODE, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, ZINT_WARN_NONCOMPLIANT, 0 },
        /*114*/ { BARCODE_QRCODE, "(01)1234567890123A", GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE, -1, 0, 1 },
        /*115*/ { BARCODE_QRCODE, "[01]1234567890123", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*116*/ { BARCODE_QRCODE, "[01]1234567890123", GS1_MODE | GS1NOCHECK_MODE, -1, 0, 0 },
        /*117*/ { BARCODE_QRCODE, "1234", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*118*/ { BARCODE_QRCODE, "1234", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_symbol previous_symbol;

    testStart("test_input_mode");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        if (p_ctx->index == -1 && data[i].compare_previous) {
            ret = testUtilSymbolCmp(symbol, &previous_symbol);
            assert_zero(ret, "i:%d testUtilSymbolCmp ret %d != 0\n", i, ret);
        }
        memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/*
 * Check GS1NOCHECK_MODE for GS1_128-based and DBAR_EXP-based symbologies
 */
static void test_gs1nocheck_mode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        char *data;
        char *composite;
        int ret;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_GS1_128, -1, "[01]12345678901231", "", 0, "" },
        /*  1*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]12345678901231", "", 0, "" },
        /*  2*/ { BARCODE_GS1_128, -1, "[01]12345678901234", "", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (01) position 14: Bad checksum '4', expected '1'" },
        /*  3*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]12345678901234", "", 0, "" },
        /*  4*/ { BARCODE_GS1_128, -1, "[01]123456789012345", "", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (01)" },
        /*  5*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]123456789012345", "", 0, "" },
        /*  6*/ { BARCODE_GS1_128, -1, "[01]1234567890123", "", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (01)" },
        /*  7*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]1234567890123", "", 0, "" },
        /*  8*/ { BARCODE_GS1_128, -1, "[01]12345678901231[20]1", "", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (20)" },
        /*  9*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]12345678901231[20]1", "", 0, "" },
        /* 10*/ { BARCODE_GS1_128, -1, "[03]123", "", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (03)" },
        /* 11*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[03]123", "", 0, "" },
        /* 12*/ { BARCODE_GS1_128, -1, "[04]1234[05]12345[06]123456", "", ZINT_ERROR_INVALID_DATA, "Error 260: Invalid AI (04)" },
        /* 13*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[04]1234[05]12345[06]123456", "", 0, "" },
        /* 14*/ { BARCODE_GS1_128, -1, "[01]1234567890123A", "", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (01) position 14: Non-numeric character 'A'" },
        /* 15*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]1234567890123A", "", 0, "" },
        /* 16*/ { BARCODE_GS1_128, -1, "[01]1234567890123.", "", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (01) position 14: Non-numeric character '.'" },
        /* 17*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]1234567890123.", "", 0, "" },
        /* 18*/ { BARCODE_GS1_128, -1, "[01]1234567890123\177", "", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1" },
        /* 19*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]1234567890123\177", "", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1" }, /* Nonprintable ASCII still checked */
        /* 20*/ { BARCODE_GS1_128, -1, "[01]1234567890123\200", "", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1" },
        /* 21*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]1234567890123\200", "", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1" }, /* Extended ASCII still checked */
        /* 22*/ { BARCODE_GS1_128, -1, "0112345678901231", "", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI" },
        /* 23*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "0112345678901231", "", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI" }, /* Format still checked */
        /* 24*/ { BARCODE_GS1_128, -1, "[01]", "", ZINT_ERROR_INVALID_DATA, "Error 258: Empty data field in input data" },
        /* 25*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]", "", 0, "" }, /* Zero-length data not checked */
        /* 26*/ { BARCODE_GS1_128, -1, "[01][20]12", "", ZINT_ERROR_INVALID_DATA, "Error 258: Empty data field in input data" },
        /* 27*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01][20]12", "", 0, "" }, /* Zero-length data not checked */
        /* 28*/ { BARCODE_GS1_128, -1, "[0]123", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 29*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[0]123", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 1 AI with no data still checked */
        /* 30*/ { BARCODE_GS1_128, -1, "[0]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 31*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[0]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 1 AI with no data still checked */
        /* 32*/ { BARCODE_GS1_128, -1, "[]12", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 33*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[]12", "", 0, "" }, /* Length 0 AI with data not checked */
        /* 34*/ { BARCODE_GS1_128, -1, "[]12[01]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 35*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[]12[01]", "", 0, "" }, /* Length 0 AI with data not checked, non-short AI with zero-length data not checked */
        /* 36*/ { BARCODE_GS1_128, -1, "[01][]12", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 37*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01][]12", "", 0, "" }, /* Length 0 AI with data not checked, non-short AI with zero-length data not checked */
        /* 38*/ { BARCODE_GS1_128, -1, "[1]1[]12", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 39*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[1]1[]12", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 1 AI with data still checked */
        /* 40*/ { BARCODE_GS1_128, -1, "[]12[1]1", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 41*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[]12[1]1", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 1 AI with data still checked */
        /* 42*/ { BARCODE_GS1_128, -1, "[]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 43*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 0 AI with no data still checked */
        /* 44*/ { BARCODE_GS1_128, -1, "[01]12345678901231[]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 45*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]12345678901231[]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 0 AI with no data still checked */
        /* 46*/ { BARCODE_GS1_128, -1, "[01]12345678901231[][20]12", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 47*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]12345678901231[][20]12", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 0 AI with no data still checked */
        /* 48*/ { BARCODE_GS1_128, -1, "[01]12345678901231[]1[20]12", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /* 49*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[01]12345678901231[]1[20]12", "", 0, "" }, /* Length 0 AI with data not checked */
        /* 50*/ { BARCODE_GS1_128, -1, "[1234567890]123", "", ZINT_ERROR_INVALID_DATA, "Error 255: Invalid AI in input data (AI too long)" },
        /* 51*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[1234567890]123", "", ZINT_ERROR_INVALID_DATA, "Error 255: Invalid AI in input data (AI too long)" }, /* Too long still checked */
        /* 52*/ { BARCODE_GS1_128, -1, "[12345]123", "", ZINT_ERROR_INVALID_DATA, "Error 255: Invalid AI in input data (AI too long)" },
        /* 53*/ { BARCODE_GS1_128, GS1NOCHECK_MODE, "[12345]123", "", ZINT_ERROR_INVALID_DATA, "Error 255: Invalid AI in input data (AI too long)" }, /* Too long still checked */
        /* 54*/ { BARCODE_GS1_128, GS1PARENS_MODE, "(91)AB[", "", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (91) position 3: Invalid CSET 82 character '['" },
        /* 55*/ { BARCODE_GS1_128, GS1PARENS_MODE | GS1NOCHECK_MODE, "(91)AB[", "", 0, "" },
        /* 56*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]12", 0, "" },
        /* 57*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]12345678901231", "[20]12", 0, "" },
        /* 58*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901234", "[20]12", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (01) position 14: Bad checksum '4', expected '1' in linear component" },
        /* 59*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]12345678901234", "[20]12", 0, "" },
        /* 60*/ { BARCODE_GS1_128_CC, -1, "[01]123456789012345", "[20]12", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (01) in linear component" },
        /* 61*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]123456789012345", "[20]12", 0, "" },
        /* 62*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]123", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (20) in 2D component" },
        /* 63*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]12345678901231", "[20]123", 0, "" },
        /* 64*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]1A", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (20) position 2: Non-numeric character 'A' in 2D component" },
        /* 65*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]12345678901231", "[20]1A", 0, "" },
        /* 66*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[20]1\177", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1 in 2D component" },
        /* 67*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]1\177", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1 in 2D component" }, /* Nonprintable ASCII still checked */
        /* 68*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121\200", "[20]12", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1 in linear component" },
        /* 69*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121\200", "[20]12", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1 in linear component" },
        /* 70*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[20]1\200", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1 in 2D component" },
        /* 71*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]1\200", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1 in 2D component" }, /* Extended ASCII still checked */
        /* 72*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "2012", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI in 2D component" },
        /* 73*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "2012", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI in 2D component" }, /* Format still checked */
        /* 74*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[20]", ZINT_ERROR_INVALID_DATA, "Error 258: Empty data field in input data in 2D component" },
        /* 75*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]", 0, "" }, /* Zero-length data not checked */
        /* 76*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[2]12", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /* 77*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[2]12", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" }, /* Length 1 AI still checked */
        /* 78*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[]12", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /* 79*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[]12", 0, "" }, /* Length 0 AI with data not checked */
        /* 80*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[1]2[]1", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /* 81*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[1]2[]1", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" }, /* Length 1 AI still checked */
        /* 82*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[]", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /* 83*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[]", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" }, /* Length 0 AI with no data still checked */
        /* 84*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[][20]12", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /* 85*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[][20]12", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" }, /* Length 0 AI with no data still checked */
        /* 86*/ { BARCODE_GS1_128_CC, -1, "[01]1234567890121", "[20]12[]", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /* 87*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]12[]", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" }, /* Length 0 AI with no data still checked */
        /* 88*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[90]12]34", ZINT_ERROR_INVALID_DATA, "Error 441: Invalid character in 2D component input data" },
        /* 89*/ { BARCODE_GS1_128_CC, GS1NOCHECK_MODE, "[01]12345678901231", "[90]12]34", ZINT_ERROR_INVALID_DATA, "Error 441: Invalid character in 2D component input data" }, /* Non-CSET 82 always checked for composite data */
        /* 90*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231", "", 0, "" },
        /* 91*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[01]12345678901231", "", 0, "" },
        /* 92*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[10]123[11]1234", "", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (11)" },
        /* 93*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]1234", "", 0, "" },
        /* 94*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[10]123[11]1234A", "", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (11)" },
        /* 95*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]1234A", "", 0, "" },
        /* 96*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231[10]123[11]12345A", "", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (11) position 6: Non-numeric character 'A'" },
        /* 97*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]12345A", "", 0, "" },
        /* 98*/ { BARCODE_DBAR_EXP, -1, "[01]1234567890121\177", "", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1" },
        /* 99*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[01]1234567890121\177", "", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1" }, /* Nonprintable ASCII still checked */
        /*100*/ { BARCODE_DBAR_EXP, -1, "[01]1234567890121\200", "", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1" },
        /*101*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[01]1234567890121\200", "", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1" }, /* Extended ASCII still checked */
        /*102*/ { BARCODE_DBAR_EXP, -1, "011234567890121", "", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI" },
        /*103*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "011234567890121", "", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI" }, /* Format still checked */
        /*104*/ { BARCODE_DBAR_EXP, -1, "[10]", "", ZINT_ERROR_INVALID_DATA, "Error 258: Empty data field in input data" },
        /*105*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[10]", "", 0, "" }, /* Zero-length data not checked */
        /*106*/ { BARCODE_DBAR_EXP, -1, "[2]1", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /*107*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[2]1", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 1 AI still checked */
        /*108*/ { BARCODE_DBAR_EXP, -1, "[]1", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /*109*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[]1", "", 0, "" }, /* Length 0 AI with data not checked */
        /*110*/ { BARCODE_DBAR_EXP, -1, "[]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /*111*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 0 AI with no data still checked */
        /*112*/ { BARCODE_DBAR_EXP, -1, "[20]12[]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" },
        /*113*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[20]12[]", "", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short)" }, /* Length 0 AI with no data still checked */
        /*114*/ { BARCODE_DBAR_EXP, -1, "[90]12]34", "", ZINT_ERROR_INVALID_DATA, "Error 386: Invalid character in General Field data" },
        /*115*/ { BARCODE_DBAR_EXP, GS1NOCHECK_MODE, "[90]12]34", "", ZINT_ERROR_INVALID_DATA, "Error 386: Invalid character in General Field data" }, /* Non-CSET 82 always checked for DBAR_EXP */
        /*116*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345", 0, "" },
        /*117*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]123456789012[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345", 0, "" },
        /*118*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231[10]123[11]1234", "[21]ABC123[22]12345", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (11) in linear component" },
        /*119*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]1234", "[21]ABC123[22]12345", 0, "" },
        /*120*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231[10]123[11]123456", "[21]ABC123[22]12345", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (11) position 3: Invalid month '34' in linear component" },
        /*121*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]123456", "[21]ABC123[22]12345", 0, "" },
        /*122*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345[30]123456789", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (30) in 2D component" },
        /*123*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]123456789012[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345[30]123456789", 0, "" },
        /*124*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345[30]1234567A", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (30) position 8: Non-numeric character 'A' in 2D component" },
        /*125*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]123456789012[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345[30]1234567A", 0, "" },
        /*126*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "[20]1\177", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1 in 2D component" },
        /*127*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]1\177", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1 in 2D component" }, /* Nonprintable ASCII still checked */
        /*128*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "[20]1\200", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1 in 2D component" },
        /*129*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]1\200", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1 in 2D component" }, /* Extended ASCII still checked */
        /*130*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "2012", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI in 2D component" },
        /*131*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "2012", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI in 2D component" }, /* Format still checked */
        /*132*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "[10]", ZINT_ERROR_INVALID_DATA, "Error 258: Empty data field in input data in 2D component" },
        /*133*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[10]", 0, "" }, /* Zero-length data not checked */
        /*134*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "[2]1", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /*135*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[2]1", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" }, /* Length 1 AI still checked */
        /*136*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "[]12", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /*137*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[]12", 0, "" }, /* Length 0 AI with data not checked */
        /*138*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "[]", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /*139*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[]", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" }, /* Length 0 AI with no data still checked */
        /*140*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "[20]12[][10]123", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" },
        /*141*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]12[][10]123", ZINT_ERROR_INVALID_DATA, "Error 256: Invalid AI in input data (AI too short) in 2D component" }, /* Length 0 AI with no data still checked */
        /*142*/ { BARCODE_DBAR_EXP_CC, -1, "[01]1234567890121", "[90]12]34", ZINT_ERROR_INVALID_DATA, "Error 441: Invalid character in 2D component input data" },
        /*143*/ { BARCODE_DBAR_EXP_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[90]12]34", ZINT_ERROR_INVALID_DATA, "Error 441: Invalid character in 2D component input data" }, /* Non-CSET 82 always checked for composite */
        /*144*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901231", "", 0, "" },
        /*145*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[01]12345678901231", "", 0, "" },
        /*146*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901231[10]123[11]1234", "", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (11)" },
        /*147*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]1234", "", 0, "" },
        /*148*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901231[10]123[11]1234A", "", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (11)" },
        /*149*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]1234A", "", 0, "" },
        /*150*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901231[10]123[11]12345A", "", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (11) position 6: Non-numeric character 'A'" },
        /*151*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]12345A", "", 0, "" },
        /*152*/ { BARCODE_DBAR_EXPSTK, -1, "[01]1234567890121\177", "", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1" },
        /*153*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[01]1234567890121\177", "", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1" }, /* Nonprintable ASCII still checked */
        /*154*/ { BARCODE_DBAR_EXPSTK, -1, "[01]1234567890121\200", "", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1" },
        /*155*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[01]1234567890121\200", "", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1" }, /* Extended ASCII still checked */
        /*156*/ { BARCODE_DBAR_EXPSTK, -1, "011234567890121", "", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI" },
        /*157*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "011234567890121", "", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI" }, /* Format still checked */
        /*158*/ { BARCODE_DBAR_EXPSTK, -1, "[01]", "", ZINT_ERROR_INVALID_DATA, "Error 258: Empty data field in input data" },
        /*159*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[01]", "", 0, "" }, /* Zero-length data not checked */
        /*160*/ { BARCODE_DBAR_EXPSTK, -1, "[90]12]34", "", ZINT_ERROR_INVALID_DATA, "Error 386: Invalid character in General Field data" },
        /*161*/ { BARCODE_DBAR_EXPSTK, GS1NOCHECK_MODE, "[90]12]34", "", ZINT_ERROR_INVALID_DATA, "Error 386: Invalid character in General Field data" }, /* Non-CSET 82 always checked for DBAR_EXPSTK */
        /*162*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345", 0, "" },
        /*163*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]123456789012[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345", 0, "" },
        /*164*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231[10]123[11]1234", "[21]ABC123[22]12345", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (11) in linear component" },
        /*165*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]1234", "[21]ABC123[22]12345", 0, "" },
        /*166*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231[10]123[11]123456", "[21]ABC123[22]12345", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (11) position 3: Invalid month '34' in linear component" },
        /*167*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]12345678901231[10]123[11]123456", "[21]ABC123[22]12345", 0, "" },
        /*168*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345[30]123456789", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (30) in 2D component" },
        /*169*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]123456789012[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345[30]123456789", 0, "" },
        /*170*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345[30]1234567A", ZINT_WARN_NONCOMPLIANT, "Warning 261: AI (30) position 8: Non-numeric character 'A' in 2D component" },
        /*171*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]123456789012[01]12345678901231[10]123[11]121212", "[21]ABC123[22]12345[30]1234567A", 0, "" },
        /*172*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]1234567890121", "[20]1\177", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1 in 2D component" },
        /*173*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]1\177", ZINT_ERROR_INVALID_DATA, "Error 263: DEL characters are not supported by GS1 in 2D component" }, /* Nonprintable ASCII still checked */
        /*174*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]1234567890121", "[20]1\200", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1 in 2D component" },
        /*175*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[20]1\200", ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1 in 2D component" }, /* Extended ASCII still checked */
        /*176*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]1234567890121", "2012", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI in 2D component" },
        /*177*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]1234567890121", "2012", ZINT_ERROR_INVALID_DATA, "Error 252: Data does not start with an AI in 2D component" }, /* Format still checked */
        /*178*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]1234567890121", "[235]", ZINT_ERROR_INVALID_DATA, "Error 258: Empty data field in input data in 2D component" },
        /*179*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[235]", 0, "" }, /* Zero-length data not checked */
        /*180*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]1234567890121", "[90]12]34", ZINT_ERROR_INVALID_DATA, "Error 441: Invalid character in 2D component input data" },
        /*181*/ { BARCODE_DBAR_EXPSTK_CC, GS1NOCHECK_MODE, "[01]1234567890121", "[90]12]34", ZINT_ERROR_INVALID_DATA, "Error 441: Invalid character in 2D component input data" }, /* Non-CSET 82 always checked for composite */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;

    testStart("test_gs1nocheck_mode");

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].composite[0]) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, text, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_gs1_reduce", test_gs1_reduce },
        { "test_hrt", test_hrt },
        { "test_gs1_verify", test_gs1_verify },
        { "test_gs1_lint", test_gs1_lint },
        { "test_input_mode", test_input_mode },
        { "test_gs1nocheck_mode", test_gs1nocheck_mode },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
