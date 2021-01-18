/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2021 Robin Stuart <rstuart114@gmail.com>

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

/*
 * Check that EAN128 and RSS_EXP based symbologies reduce GS1 data
 */
static void test_gs1_reduce(int index, int generate, int debug) {

    testStart("");

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    int ret;
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
        /* 4*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110010011100100010100001000101001110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001101100011011101011110111010110001100011101011"
               },
        /* 5*/ { BARCODE_GS1_128_CC, GS1_MODE, "[01]12345678901231", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110010011100100010100001000101001110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001101100011011101011110111010110001100011101011"
               },
        /* 6*/ { BARCODE_GS1_128_CC, UNICODE_MODE, "[01]12345678901231", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110010011100100010100001000101001110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001101100011011101011110111010110001100011101011"
               },
        /* 7*/ { BARCODE_EAN14, -1, "1234567890123", "", 0, "Input mode ignored; verified manually against tec-it",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /* 8*/ { BARCODE_EAN14, GS1_MODE, "1234567890123", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /* 9*/ { BARCODE_EAN14, UNICODE_MODE, "1234567890123", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /*10*/ { BARCODE_NVE18, -1, "12345678901234567", "", 0, "Input mode ignored; verified manually against tec-it",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*11*/ { BARCODE_NVE18, GS1_MODE, "12345678901234567", "", 0, "Input mode ignored",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*12*/ { BARCODE_NVE18, UNICODE_MODE, "12345678901234567", "", 0, "Input mode ignored",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*13*/ { BARCODE_DBAR_EXP, -1, "2012", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*14*/ { BARCODE_DBAR_EXP, -1, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*15*/ { BARCODE_DBAR_EXP, GS1_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*16*/ { BARCODE_DBAR_EXP, UNICODE_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*17*/ { BARCODE_DBAR_EXP_CC, -1, "[20]12", "[21]1234", 0, "Input mode ignored; verified manually against tec-it",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*18*/ { BARCODE_DBAR_EXP_CC, GS1_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*19*/ { BARCODE_DBAR_EXP_CC, UNICODE_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*20*/ { BARCODE_DBAR_EXPSTK, -1, "12", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*21*/ { BARCODE_DBAR_EXPSTK, -1, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*22*/ { BARCODE_DBAR_EXPSTK, GS1_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*23*/ { BARCODE_DBAR_EXPSTK, UNICODE_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*24*/ { BARCODE_DBAR_EXPSTK_CC, -1, "12", "[21]1234", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*25*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[20]12", "[21]1234", 0, "Input mode ignored; verified manually against tec-it (same as BARCODE_DBAR_EXP_CC above)",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*26*/ { BARCODE_DBAR_EXPSTK_CC, GS1_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*27*/ { BARCODE_DBAR_EXPSTK_CC, UNICODE_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001110100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
    };
    int data_size = ARRAY_SIZE(data);

    char *text;

    char bwipp_buf[8196];
    char bwipp_msg[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].input_mode != -1) {
            symbol->input_mode = data[i].input_mode;
        }
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int length = strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);

        if (generate) {
            if (data[i].ret == 0) {
                printf("        /*%2d*/ { %s, %s, \"%s\", \"%s\", %d, \"%s\",\n",
                        i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].data, data[i].composite, data[i].ret, data[i].comment);
                testUtilModulesDump(symbol, "                    ", "\n");
                printf("               },\n");
            } else {
                printf("        /*%2d*/ { %s, %s, \"%s\", \"%s\", %s, \"%s\", \"\" },\n",
                        i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].data, data[i].composite, testUtilErrorName(data[i].ret), data[i].comment);
            }
        } else {
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d %s\n", i, ret, data[i].ret, symbol->errtxt);

            if (ret < 5) {
                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d %s testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, -1, -1, text, length, symbol->primary, bwipp_buf, sizeof(bwipp_buf));
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

static void test_hrt(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        char *data;
        char *composite;

        int ret;
        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_GS1_128, "[01]12345678901234[20]12", "", ZINT_WARN_NONCOMPLIANT, "(01)12345678901234(20)12" },
        /*  1*/ { BARCODE_GS1_128, "[01]12345678901231[20]12", "", 0, "(01)12345678901231(20)12" },
        /*  2*/ { BARCODE_GS1_128_CC, "[01]12345678901234[20]12", "[21]12345", ZINT_WARN_NONCOMPLIANT, "(01)12345678901234(20)12" },
        /*  3*/ { BARCODE_GS1_128_CC, "[01]12345678901231[20]12", "[21]12345", 0, "(01)12345678901231(20)12" },
        /*  4*/ { BARCODE_EAN14, "1234567890123", "", 0, "(01)12345678901231" },
        /*  5*/ { BARCODE_NVE18, "12345678901234567", "", 0, "(00)123456789012345675" },
        /*  6*/ { BARCODE_DBAR_EXP, "[01]12345678901234[20]12", "", ZINT_WARN_NONCOMPLIANT, "(01)12345678901234(20)12" },
        /*  7*/ { BARCODE_DBAR_EXP, "[01]12345678901231[20]12", "", 0, "(01)12345678901231(20)12" },
        /*  8*/ { BARCODE_DBAR_EXP_CC, "[01]12345678901234", "[21]12345", ZINT_WARN_NONCOMPLIANT, "(01)12345678901234" },
        /*  9*/ { BARCODE_DBAR_EXP_CC, "[01]12345678901231", "[21]12345", 0, "(01)12345678901231" },
        /* 10*/ { BARCODE_DBAR_EXPSTK, "[01]12345678901234[20]12", "", ZINT_WARN_NONCOMPLIANT, "" },
        /* 11*/ { BARCODE_DBAR_EXPSTK, "[01]12345678901231[20]12", "", 0, "" },
        /* 12*/ { BARCODE_DBAR_EXPSTK_CC, "[01]12345678901234[20]12", "[21]12345", ZINT_WARN_NONCOMPLIANT, "" },
        /* 13*/ { BARCODE_DBAR_EXPSTK_CC, "[01]12345678901231[20]12", "[21]12345", 0, "" },
    };
    int data_size = ARRAY_SIZE(data);

    char *text;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int length = strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, data[i].ret, ret, symbol->errtxt);

        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include "../gs1.h"

static void test_gs1_verify(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        char *data;
        int ret;
        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "", ZINT_ERROR_INVALID_DATA, "" },
        /*  1*/ { "[", ZINT_ERROR_INVALID_DATA, "" },
        /*  2*/ { "[]12", ZINT_ERROR_INVALID_DATA, "" },
        /*  3*/ { "[12345]12", ZINT_ERROR_INVALID_DATA, "" },
        /*  4*/ { "[9999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*  5*/ { "[[01]]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*  6*/ { "[90]\n", ZINT_ERROR_INVALID_DATA, "" },
        /*  7*/ { "[90]\x7F", ZINT_ERROR_INVALID_DATA, "" },
        /*  8*/ { "[90]\x80", ZINT_ERROR_INVALID_DATA, "" },
        /*  9*/ { "[00]123456789012345678", ZINT_WARN_NONCOMPLIANT, "00123456789012345678" },
        /* 10*/ { "[00]123456789012345675", 0, "00123456789012345675" },
        /* 11*/ { "[00]12345678901234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 12*/ { "[01]12345678901234", ZINT_WARN_NONCOMPLIANT, "0112345678901234" },
        /* 13*/ { "[01]12345678901231", 0, "0112345678901231" },
        /* 14*/ { "[01]123456789012345", ZINT_ERROR_INVALID_DATA, "" },
        /* 15*/ { "[02]12345678901234", ZINT_WARN_NONCOMPLIANT, "0212345678901234" },
        /* 16*/ { "[02]12345678901231", 0, "0212345678901231" },
        /* 17*/ { "[02]1234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /* 18*/ { "[03]12345678901234", ZINT_ERROR_INVALID_DATA, "" },
        /* 19*/ { "[04]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 20*/ { "[05]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 21*/ { "[06]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 22*/ { "[07]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 23*/ { "[08]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 24*/ { "[09]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 25*/ { "[10]ABCD123456", 0, "10ABCD123456" },
        /* 26*/ { "[10]123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /* 27*/ { "[100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 28*/ { "[1000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 29*/ { "[11]990102", 0, "11990102" },
        /* 30*/ { "[11]9901023", ZINT_ERROR_INVALID_DATA, "" },
        /* 31*/ { "[110]990102", ZINT_ERROR_INVALID_DATA, "" },
        /* 32*/ { "[1100]990102", ZINT_ERROR_INVALID_DATA, "" },
        /* 33*/ { "[12]000100", 0, "12000100" },
        /* 34*/ { "[12]00010", ZINT_ERROR_INVALID_DATA, "" },
        /* 35*/ { "[120]000100", ZINT_ERROR_INVALID_DATA, "" },
        /* 36*/ { "[1200]000100", ZINT_ERROR_INVALID_DATA, "" },
        /* 37*/ { "[13]991301", ZINT_WARN_NONCOMPLIANT, "13991301" },
        /* 38*/ { "[13]991201", 0, "13991201" },
        /* 39*/ { "[13]9913011", ZINT_ERROR_INVALID_DATA, "" },
        /* 40*/ { "[130]991301", ZINT_ERROR_INVALID_DATA, "" },
        /* 41*/ { "[1300]991301", ZINT_ERROR_INVALID_DATA, "" },
        /* 42*/ { "[14]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 43*/ { "[140]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 44*/ { "[1400]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 45*/ { "[15]021231", 0, "15021231" },
        /* 46*/ { "[15]02123", ZINT_ERROR_INVALID_DATA, "" },
        /* 47*/ { "[150]021231", ZINT_ERROR_INVALID_DATA, "" },
        /* 48*/ { "[1500]021231", ZINT_ERROR_INVALID_DATA, "" },
        /* 49*/ { "[16]000000", ZINT_WARN_NONCOMPLIANT, "16000000" },
        /* 50*/ { "[16]000100", 0, "16000100" },
        /* 51*/ { "[16]00000", ZINT_ERROR_INVALID_DATA, "" },
        /* 52*/ { "[160]000000", ZINT_ERROR_INVALID_DATA, "" },
        /* 53*/ { "[1600]000000", ZINT_ERROR_INVALID_DATA, "" },
        /* 54*/ { "[17]010200", 0, "17010200" },
        /* 55*/ { "[17]0102000", ZINT_ERROR_INVALID_DATA, "" },
        /* 56*/ { "[170]010200", ZINT_ERROR_INVALID_DATA, "" },
        /* 57*/ { "[1700]010200", ZINT_ERROR_INVALID_DATA, "" },
        /* 58*/ { "[18]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 59*/ { "[180]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 60*/ { "[1800]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 61*/ { "[19]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 62*/ { "[190]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 63*/ { "[1900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 64*/ { "[20]12", 0, "2012" },
        /* 65*/ { "[20]1", ZINT_ERROR_INVALID_DATA, "" },
        /* 66*/ { "[200]12", ZINT_ERROR_INVALID_DATA, "" },
        /* 67*/ { "[2000]12", ZINT_ERROR_INVALID_DATA, "" },
        /* 68*/ { "[21]A12345678", 0, "21A12345678" },
        /* 69*/ { "[21]123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /* 70*/ { "[210]A12345678", ZINT_ERROR_INVALID_DATA, "" },
        /* 71*/ { "[2100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 72*/ { "[22]abcdefghijklmnopqrst", 0, "22abcdefghijklmnopqrst" },
        /* 73*/ { "[22]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /* 74*/ { "[220]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 75*/ { "[2200]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 76*/ { "[23]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 77*/ { "[230]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 78*/ { "[2300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 79*/ { "[235]1abcdefghijklmnopqrstuvwxyz0", 0, "2351abcdefghijklmnopqrstuvwxyz0" },
        /* 80*/ { "[235]1abcdefghijklmnopqrstuvwxyz01", ZINT_ERROR_INVALID_DATA, "" },
        /* 81*/ { "[24]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 82*/ { "[240]abcdefghijklmnopqrstuvwxyz1234", 0, "240abcdefghijklmnopqrstuvwxyz1234" },
        /* 83*/ { "[240]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 84*/ { "[2400]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 85*/ { "[241]abcdefghijklmnopqrstuvwxyz1234", 0, "241abcdefghijklmnopqrstuvwxyz1234" },
        /* 86*/ { "[241]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 87*/ { "[2410]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 88*/ { "[242]12345", 0, "24212345" },
        /* 89*/ { "[242]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 90*/ { "[2420]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 91*/ { "[243]abcdefghijklmnopqrst", 0, "243abcdefghijklmnopqrst" },
        /* 92*/ { "[243]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /* 93*/ { "[2430]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 94*/ { "[244]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 95*/ { "[2440]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 96*/ { "[249]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 97*/ { "[2490]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 98*/ { "[25]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 99*/ { "[250]abcdefghijklmnopqrstuvwxyz1234", 0, "250abcdefghijklmnopqrstuvwxyz1234" },
        /*100*/ { "[250]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*101*/ { "[2500]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*102*/ { "[251]abcdefghijklmnopqrstuvwxyz1234", 0, "251abcdefghijklmnopqrstuvwxyz1234" },
        /*103*/ { "[251]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*104*/ { "[2510]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*105*/ { "[252]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*106*/ { "[2520]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*107*/ { "[253]131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*108*/ { "[253]1313131313134", ZINT_WARN_NONCOMPLIANT, "2531313131313134" },
        /*109*/ { "[253]1313131313130", 0, "2531313131313130" },
        /*110*/ { "[253]131313131313412345678901234567", ZINT_WARN_NONCOMPLIANT, "253131313131313412345678901234567" },
        /*111*/ { "[253]131313131313012345678901234567", 0, "253131313131313012345678901234567" },
        /*112*/ { "[253]1313131313134123456789012345678", ZINT_ERROR_INVALID_DATA, "" },
        /*113*/ { "[2530]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*114*/ { "[254]abcdefghijklmnopqrst", 0, "254abcdefghijklmnopqrst" },
        /*115*/ { "[254]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*116*/ { "[2540]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*117*/ { "[255]131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*118*/ { "[255]1313131313134", ZINT_WARN_NONCOMPLIANT, "2551313131313134" },
        /*119*/ { "[255]1313131313130", 0, "2551313131313130" },
        /*120*/ { "[255]1313131313134123456789012", ZINT_WARN_NONCOMPLIANT, "2551313131313134123456789012" },
        /*121*/ { "[255]1313131313130123456789012", 0, "2551313131313130123456789012" },
        /*122*/ { "[255]13131313131341234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /*123*/ { "[2550]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*124*/ { "[256]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*125*/ { "[2560]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*126*/ { "[259]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*127*/ { "[2590]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*128*/ { "[26]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*129*/ { "[260]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*130*/ { "[2600]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*131*/ { "[27]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*132*/ { "[270]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*133*/ { "[2700]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*134*/ { "[28]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*135*/ { "[280]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*136*/ { "[2800]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*137*/ { "[29]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*138*/ { "[290]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*139*/ { "[2900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*140*/ { "[30]12345678", 0, "3012345678" },
        /*141*/ { "[30]123456789", ZINT_ERROR_INVALID_DATA, "" },
        /*142*/ { "[300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*143*/ { "[3000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*144*/ { "[31]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*145*/ { "[310]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*146*/ { "[3100]123456", 0, "3100123456" },
        /*147*/ { "[3100]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*148*/ { "[3101]123456", 0, "3101123456" },
        /*149*/ { "[3101]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*150*/ { "[3105]123456", 0, "3105123456" },
        /*151*/ { "[3105]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*152*/ { "[3106]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*153*/ { "[3109]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*154*/ { "[3110]123456", 0, "3110123456" },
        /*155*/ { "[3110]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*156*/ { "[3115]123456", 0, "3115123456" },
        /*157*/ { "[3115]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*158*/ { "[3116]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*159*/ { "[3119]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*160*/ { "[3120]123456", 0, "3120123456" },
        /*161*/ { "[3120]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*162*/ { "[3125]123456", 0, "3125123456" },
        /*163*/ { "[3125]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*164*/ { "[3126]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*165*/ { "[3129]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*166*/ { "[3130]123456", 0, "3130123456" },
        /*167*/ { "[3130]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*168*/ { "[3135]123456", 0, "3135123456" },
        /*169*/ { "[3135]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*170*/ { "[3136]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*171*/ { "[3139]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*172*/ { "[3140]123456", 0, "3140123456" },
        /*173*/ { "[3140]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*174*/ { "[3145]123456", 0, "3145123456" },
        /*175*/ { "[3145]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*176*/ { "[3146]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*177*/ { "[3149]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*178*/ { "[3150]123456", 0, "3150123456" },
        /*179*/ { "[3150]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*180*/ { "[3155]123456", 0, "3155123456" },
        /*181*/ { "[3155]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*182*/ { "[3156]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*183*/ { "[3159]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*184*/ { "[3160]123456", 0, "3160123456" },
        /*185*/ { "[3160]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*186*/ { "[3165]123456", 0, "3165123456" },
        /*187*/ { "[3165]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*188*/ { "[3166]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*189*/ { "[3169]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*190*/ { "[3170]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*191*/ { "[3179]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*192*/ { "[3180]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*193*/ { "[3189]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*194*/ { "[3190]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*195*/ { "[3199]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*196*/ { "[32]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*197*/ { "[320]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*198*/ { "[3200]123456", 0, "3200123456" },
        /*199*/ { "[3200]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*200*/ { "[3205]123456", 0, "3205123456" },
        /*201*/ { "[3205]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*202*/ { "[3206]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*203*/ { "[3209]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*204*/ { "[3210]123456", 0, "3210123456" },
        /*205*/ { "[3210]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*206*/ { "[3215]123456", 0, "3215123456" },
        /*207*/ { "[3215]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*208*/ { "[3216]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*209*/ { "[3219]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*210*/ { "[3220]123456", 0, "3220123456" },
        /*211*/ { "[3220]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*212*/ { "[3225]123456", 0, "3225123456" },
        /*213*/ { "[3225]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*214*/ { "[3229]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*215*/ { "[3230]123456", 0, "3230123456" },
        /*216*/ { "[3230]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*217*/ { "[3235]123456", 0, "3235123456" },
        /*218*/ { "[3235]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*219*/ { "[3239]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*220*/ { "[3240]123456", 0, "3240123456" },
        /*221*/ { "[3240]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*222*/ { "[3245]123456", 0, "3245123456" },
        /*223*/ { "[3245]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*224*/ { "[3249]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*225*/ { "[3250]123456", 0, "3250123456" },
        /*226*/ { "[3250]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*227*/ { "[3255]123456", 0, "3255123456" },
        /*228*/ { "[3255]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*229*/ { "[3259]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*230*/ { "[3260]123456", 0, "3260123456" },
        /*231*/ { "[3260]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*232*/ { "[3265]123456", 0, "3265123456" },
        /*233*/ { "[3265]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*234*/ { "[3269]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*235*/ { "[3270]123456", 0, "3270123456" },
        /*236*/ { "[3270]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*237*/ { "[3275]123456", 0, "3275123456" },
        /*238*/ { "[3275]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*239*/ { "[3279]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*240*/ { "[3280]123456", 0, "3280123456" },
        /*241*/ { "[3280]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*242*/ { "[3285]123456", 0, "3285123456" },
        /*243*/ { "[3285]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*244*/ { "[3289]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*245*/ { "[3290]123456", 0, "3290123456" },
        /*246*/ { "[3290]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*247*/ { "[3295]123456", 0, "3295123456" },
        /*248*/ { "[3295]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*249*/ { "[3296]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*250*/ { "[3299]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*251*/ { "[33]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*252*/ { "[330]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*253*/ { "[3300]123456", 0, "3300123456" },
        /*254*/ { "[3300]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*255*/ { "[3305]123456", 0, "3305123456" },
        /*256*/ { "[3305]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*257*/ { "[3306]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*258*/ { "[3309]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*259*/ { "[3310]123456", 0, "3310123456" },
        /*260*/ { "[3310]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*261*/ { "[3319]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*262*/ { "[3320]123456", 0, "3320123456" },
        /*263*/ { "[3320]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*264*/ { "[3329]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*265*/ { "[3330]123456", 0, "3330123456" },
        /*266*/ { "[3330]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*267*/ { "[3339]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*268*/ { "[3340]123456", 0, "3340123456" },
        /*269*/ { "[3340]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*270*/ { "[3349]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*271*/ { "[3350]123456", 0, "3350123456" },
        /*272*/ { "[3350]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*273*/ { "[3359]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*274*/ { "[3360]123456", 0, "3360123456" },
        /*275*/ { "[3360]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*276*/ { "[3369]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*277*/ { "[3370]123456", 0, "3370123456" },
        /*278*/ { "[3370]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*279*/ { "[3375]123456", 0, "3375123456" },
        /*280*/ { "[3375]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*281*/ { "[3376]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*282*/ { "[3379]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*283*/ { "[3380]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*284*/ { "[3390]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*285*/ { "[3399]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*286*/ { "[34]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*287*/ { "[340]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*288*/ { "[3400]123456", 0, "3400123456" },
        /*289*/ { "[3400]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*290*/ { "[3405]123456", 0, "3405123456" },
        /*291*/ { "[3405]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*292*/ { "[3406]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*293*/ { "[3409]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*294*/ { "[3410]123456", 0, "3410123456" },
        /*295*/ { "[3410]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*296*/ { "[3419]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*297*/ { "[3420]123456", 0, "3420123456" },
        /*298*/ { "[3420]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*299*/ { "[3429]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*300*/ { "[3430]123456", 0, "3430123456" },
        /*301*/ { "[3430]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*302*/ { "[3439]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*303*/ { "[3440]123456", 0, "3440123456" },
        /*304*/ { "[3440]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*305*/ { "[3449]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*306*/ { "[3450]123456", 0, "3450123456" },
        /*307*/ { "[3450]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*308*/ { "[3459]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*309*/ { "[3460]123456", 0, "3460123456" },
        /*310*/ { "[3460]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*311*/ { "[3469]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*312*/ { "[3470]123456", 0, "3470123456" },
        /*313*/ { "[3470]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*314*/ { "[3479]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*315*/ { "[3480]123456", 0, "3480123456" },
        /*316*/ { "[3480]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*317*/ { "[3489]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*318*/ { "[3490]123456", 0, "3490123456" },
        /*319*/ { "[3490]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*320*/ { "[3495]123456", 0, "3495123456" },
        /*321*/ { "[3495]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*322*/ { "[3496]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*323*/ { "[3499]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*324*/ { "[35]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*325*/ { "[350]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*326*/ { "[3500]123456", 0, "3500123456" },
        /*327*/ { "[3500]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*328*/ { "[3505]123456", 0, "3505123456" },
        /*329*/ { "[3505]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*330*/ { "[3506]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*331*/ { "[3509]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*332*/ { "[3510]123456", 0, "3510123456", },
        /*333*/ { "[3510]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*334*/ { "[3519]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*335*/ { "[3520]123456", 0, "3520123456", },
        /*336*/ { "[3520]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*337*/ { "[3529]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*338*/ { "[3530]123456", 0, "3530123456", },
        /*339*/ { "[3530]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*340*/ { "[3539]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*341*/ { "[3540]123456", 0, "3540123456", },
        /*342*/ { "[3540]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*343*/ { "[3549]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*344*/ { "[3550]123456", 0, "3550123456", },
        /*345*/ { "[3550]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*346*/ { "[3559]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*347*/ { "[3560]123456", 0, "3560123456", },
        /*348*/ { "[3560]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*349*/ { "[3569]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*350*/ { "[3570]123456", 0, "3570123456", },
        /*351*/ { "[3570]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*352*/ { "[3575]123456", 0, "3575123456" },
        /*353*/ { "[3376]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*354*/ { "[3579]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*355*/ { "[3580]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*356*/ { "[3590]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*357*/ { "[3599]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*358*/ { "[36]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*359*/ { "[360]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*360*/ { "[3600]123456", 0, "3600123456" },
        /*361*/ { "[3600]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*362*/ { "[3605]123456", 0, "3605123456" },
        /*363*/ { "[3605]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*364*/ { "[3606]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*365*/ { "[3609]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*366*/ { "[3610]123456", 0, "3610123456" },
        /*367*/ { "[3610]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*368*/ { "[3619]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*369*/ { "[3620]123456", 0, "3620123456", },
        /*370*/ { "[3620]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*371*/ { "[3629]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*372*/ { "[3630]123456", 0, "3630123456", },
        /*373*/ { "[3630]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*374*/ { "[3639]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*375*/ { "[3640]123456", 0, "3640123456", },
        /*376*/ { "[3640]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*377*/ { "[3649]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*378*/ { "[3650]123456", 0, "3650123456", },
        /*379*/ { "[3650]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*380*/ { "[3659]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*381*/ { "[3660]123456", 0, "3660123456", },
        /*382*/ { "[3660]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*383*/ { "[3669]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*384*/ { "[3670]123456", 0, "3670123456", },
        /*385*/ { "[3670]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*386*/ { "[3679]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*387*/ { "[3680]123456", 0, "3680123456", },
        /*388*/ { "[3680]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*389*/ { "[3689]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*390*/ { "[3690]123456", 0, "3690123456" },
        /*391*/ { "[3690]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*392*/ { "[3695]123456", 0, "3695123456" },
        /*393*/ { "[3695]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*394*/ { "[3696]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*395*/ { "[3699]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*396*/ { "[37]12345678", 0, "3712345678" },
        /*397*/ { "[37]123456789", ZINT_ERROR_INVALID_DATA, "" },
        /*398*/ { "[370]12345678", ZINT_ERROR_INVALID_DATA, "" },
        /*399*/ { "[3700]12345678", ZINT_ERROR_INVALID_DATA, "" },
        /*400*/ { "[38]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*401*/ { "[380]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*402*/ { "[3800]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*403*/ { "[39]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*404*/ { "[390]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*405*/ { "[3900]123456789012345", 0, "3900123456789012345" },
        /*406*/ { "[3900]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*407*/ { "[3900]12345678901234", 0, "390012345678901234" },
        /*408*/ { "[3901]123456789012345", 0, "3901123456789012345" },
        /*409*/ { "[3901]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*410*/ { "[3905]123456789012345", 0, "3905123456789012345" },
        /*411*/ { "[3909]123456789012345", 0, "3909123456789012345" },
        /*412*/ { "[3909]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*413*/ { "[3910]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3910123123456789012345" },
        /*414*/ { "[3910]997123456789012345", 0, "3910997123456789012345" },
        /*415*/ { "[3910]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*416*/ { "[3910]123", ZINT_ERROR_INVALID_DATA, "" },
        /*417*/ { "[3915]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3915123123456789012345" },
        /*418*/ { "[3915]997123456789012345", 0, "3915997123456789012345" },
        /*419*/ { "[3915]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*420*/ { "[3919]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3919123123456789012345" },
        /*421*/ { "[3919]997123456789012345", 0, "3919997123456789012345" },
        /*422*/ { "[3919]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*423*/ { "[3920]123456789012345", 0, "3920123456789012345" },
        /*424*/ { "[3920]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*425*/ { "[3925]123456789012345", 0, "3925123456789012345" },
        /*426*/ { "[3925]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*427*/ { "[3929]123456789012345", 0, "3929123456789012345" },
        /*428*/ { "[3929]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*429*/ { "[3930]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3930123123456789012345" },
        /*430*/ { "[3930]997123456789012345", 0, "3930997123456789012345" },
        /*431*/ { "[3930]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*432*/ { "[3930]123", ZINT_ERROR_INVALID_DATA, "" },
        /*433*/ { "[3935]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3935123123456789012345" },
        /*434*/ { "[3935]997123456789012345", 0, "3935997123456789012345" },
        /*435*/ { "[3935]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*436*/ { "[3939]123123456789012345", ZINT_WARN_NONCOMPLIANT, "3939123123456789012345" },
        /*437*/ { "[3939]997123456789012345", 0, "3939997123456789012345" },
        /*438*/ { "[3939]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*439*/ { "[3940]1234", 0, "39401234" },
        /*440*/ { "[3940]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*441*/ { "[3940]123", ZINT_ERROR_INVALID_DATA, "" },
        /*442*/ { "[3941]1234", 0, "39411234" },
        /*443*/ { "[3941]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*444*/ { "[3941]123", ZINT_ERROR_INVALID_DATA, "" },
        /*445*/ { "[3942]1234", 0, "39421234" },
        /*446*/ { "[3942]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*447*/ { "[3943]1234", 0, "39431234" },
        /*448*/ { "[3943]123", ZINT_ERROR_INVALID_DATA, "" },
        /*449*/ { "[3944]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*450*/ { "[3945]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*451*/ { "[3949]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*452*/ { "[3950]123456", 0, "3950123456" },
        /*453*/ { "[3950]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*454*/ { "[3950]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*455*/ { "[3951]123456", 0, "3951123456" },
        /*456*/ { "[3951]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*457*/ { "[3952]123456", 0, "3952123456" },
        /*458*/ { "[3952]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*459*/ { "[3953]123456", 0, "3953123456" },
        /*460*/ { "[3953]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*461*/ { "[3954]123456", 0, "3954123456" },
        /*462*/ { "[3954]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*463*/ { "[3955]123456", 0, "3955123456" },
        /*464*/ { "[3955]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*465*/ { "[3956]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*466*/ { "[3959]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*467*/ { "[3960]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*468*/ { "[3970]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*469*/ { "[3980]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*470*/ { "[3999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*471*/ { "[40]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*472*/ { "[400]123456789012345678901234567890", 0, "400123456789012345678901234567890" },
        /*473*/ { "[400]1234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*474*/ { "[4000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*475*/ { "[401]1234abcdefghijklmnopqrstuvwxyz", 0, "4011234abcdefghijklmnopqrstuvwxyz" },
        /*476*/ { "[401]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*477*/ { "[4010]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*478*/ { "[402]13131313131313132", ZINT_WARN_NONCOMPLIANT, "40213131313131313132" },
        /*479*/ { "[402]13131313131313130", 0, "40213131313131313130" },
        /*480*/ { "[402]1313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*481*/ { "[4020]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*482*/ { "[403]abcdefghijklmnopqrstuvwxyz1234", 0, "403abcdefghijklmnopqrstuvwxyz1234" },
        /*483*/ { "[403]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*484*/ { "[4030]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*485*/ { "[404]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*486*/ { "[4040]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*487*/ { "[409]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*488*/ { "[4090]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*489*/ { "[41]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*490*/ { "[410]3898765432108", 0, "4103898765432108" },
        /*491*/ { "[410]12345678901234", ZINT_ERROR_INVALID_DATA, "" },
        /*492*/ { "[4100]12345678901234", ZINT_ERROR_INVALID_DATA, "" },
        /*493*/ { "[411]1313131313134", ZINT_WARN_NONCOMPLIANT, "4111313131313134" },
        /*494*/ { "[411]1313131313130", 0, "4111313131313130" },
        /*495*/ { "[411]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*496*/ { "[4110]1313131313134", ZINT_ERROR_INVALID_DATA, "" },
        /*497*/ { "[412]1313131313130", 0, "4121313131313130" },
        /*498*/ { "[412]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*499*/ { "[4120]1313131313134", ZINT_ERROR_INVALID_DATA, "" },
        /*500*/ { "[413]1313131313130", 0, "4131313131313130" },
        /*501*/ { "[413]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*502*/ { "[4130]1313131313134", ZINT_ERROR_INVALID_DATA, "" },
        /*503*/ { "[414]1313131313130", 0, "4141313131313130" },
        /*504*/ { "[414]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*505*/ { "[4140]1313131313134", ZINT_ERROR_INVALID_DATA, "" },
        /*506*/ { "[415]1313131313130", 0, "4151313131313130" },
        /*507*/ { "[415]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*508*/ { "[4150]1313131313134", ZINT_ERROR_INVALID_DATA, "" },
        /*509*/ { "[416]1313131313130", 0, "4161313131313130" },
        /*510*/ { "[416]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*511*/ { "[4160]1313131313134", ZINT_ERROR_INVALID_DATA, "" },
        /*512*/ { "[417]1313131313130", 0, "4171313131313130" },
        /*513*/ { "[417]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*514*/ { "[4170]1313131313134", ZINT_ERROR_INVALID_DATA, "" },
        /*515*/ { "[418]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*516*/ { "[4180]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*517*/ { "[419]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*518*/ { "[4190]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*519*/ { "[42]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*520*/ { "[420]abcdefghijklmnopqrst", 0, "420abcdefghijklmnopqrst" },
        /*521*/ { "[420]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*522*/ { "[4200]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*523*/ { "[421]123abcdefghi", ZINT_WARN_NONCOMPLIANT, "421123abcdefghi" },
        /*524*/ { "[421]434abcdefghi", 0, "421434abcdefghi" },
        /*525*/ { "[421]123abcdefghij", ZINT_ERROR_INVALID_DATA, "" },
        /*526*/ { "[421]1231", ZINT_WARN_NONCOMPLIANT, "4211231" },
        /*527*/ { "[421]4341", 0, "4214341" },
        /*528*/ { "[421]123", ZINT_ERROR_INVALID_DATA, "" },
        /*529*/ { "[4210]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*530*/ { "[422]123", ZINT_WARN_NONCOMPLIANT, "422123" },
        /*531*/ { "[422]004", 0, "422004" },
        /*532*/ { "[422]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*533*/ { "[422]12", ZINT_ERROR_INVALID_DATA, "" },
        /*534*/ { "[4220]123", ZINT_ERROR_INVALID_DATA, "" },
        /*535*/ { "[423]123123123123123", ZINT_WARN_NONCOMPLIANT, "423123123123123123" },
        /*536*/ { "[423]470004012887123", ZINT_WARN_NONCOMPLIANT, "423470004012887123" },
        /*537*/ { "[423]470004012887438", 0, "423470004012887438" },
        /*538*/ { "[423]1231231231231231", ZINT_ERROR_INVALID_DATA, "4231231231231231231" },
        /*539*/ { "[423]12312312312312", ZINT_WARN_NONCOMPLIANT, "42312312312312312" },
        /*540*/ { "[423]1231231231231", ZINT_WARN_NONCOMPLIANT, "4231231231231231" },
        /*541*/ { "[423]12312312312", ZINT_WARN_NONCOMPLIANT, "42312312312312" },
        /*542*/ { "[423]1231231231", ZINT_WARN_NONCOMPLIANT, "4231231231231" },
        /*543*/ { "[423]123", ZINT_WARN_NONCOMPLIANT, "423123" },
        /*544*/ { "[423]004", 0, "423004" },
        /*545*/ { "[423]12", ZINT_ERROR_INVALID_DATA, "" },
        /*546*/ { "[4230]123123123123123", ZINT_ERROR_INVALID_DATA, "" },
        /*547*/ { "[424]123", ZINT_WARN_NONCOMPLIANT, "424123" },
        /*548*/ { "[424]004", 0, "424004" },
        /*549*/ { "[424]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*550*/ { "[424]12", ZINT_ERROR_INVALID_DATA, "" },
        /*551*/ { "[4240]123", ZINT_ERROR_INVALID_DATA, "" },
        /*552*/ { "[425]123123123123123", ZINT_WARN_NONCOMPLIANT, "425123123123123123" },
        /*553*/ { "[425]010500276634894", 0, "425010500276634894" },
        /*554*/ { "[425]010500276123894", ZINT_WARN_NONCOMPLIANT, "425010500276123894" },
        /*555*/ { "[425]1231231231231231", ZINT_ERROR_INVALID_DATA, "" },
        /*556*/ { "[425]12", ZINT_ERROR_INVALID_DATA, "" },
        /*557*/ { "[4250]123123123123123", ZINT_ERROR_INVALID_DATA, "" },
        /*558*/ { "[426]123", ZINT_WARN_NONCOMPLIANT, "426123" },
        /*559*/ { "[426]426", 0, "426426" },
        /*560*/ { "[426]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*561*/ { "[426]12", ZINT_ERROR_INVALID_DATA, "" },
        /*562*/ { "[4260]123", ZINT_ERROR_INVALID_DATA, "" },
        /*563*/ { "[427]abc", 0, "427abc" },
        /*564*/ { "[427]abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*565*/ { "[4270]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*566*/ { "[428]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*567*/ { "[4280]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*568*/ { "[429]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*569*/ { "[4290]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*570*/ { "[43]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*571*/ { "[430]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*572*/ { "[4300]1", 0, "43001" },
        /*573*/ { "[4300]12345678901234567890123456789012345", 0, "430012345678901234567890123456789012345" },
        /*574*/ { "[4300]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*575*/ { "[4301]1", 0, "43011" },
        /*576*/ { "[4301]12345678901234567890123456789012345", 0, "430112345678901234567890123456789012345" },
        /*577*/ { "[4301]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*578*/ { "[4302]1", 0, "43021" },
        /*579*/ { "[4302]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43021234567890123456789012345678901234567890123456789012345678901234567890" },
        /*580*/ { "[4302]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*581*/ { "[4303]1", 0, "43031" },
        /*582*/ { "[4303]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43031234567890123456789012345678901234567890123456789012345678901234567890" },
        /*583*/ { "[4303]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*584*/ { "[4304]1", 0, "43041" },
        /*585*/ { "[4304]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43041234567890123456789012345678901234567890123456789012345678901234567890" },
        /*586*/ { "[4304]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*587*/ { "[4305]1", 0, "43051" },
        /*588*/ { "[4305]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43051234567890123456789012345678901234567890123456789012345678901234567890" },
        /*589*/ { "[4305]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*590*/ { "[4306]1", 0, "43061" },
        /*591*/ { "[4306]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43061234567890123456789012345678901234567890123456789012345678901234567890" },
        /*592*/ { "[4306]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*593*/ { "[4307]FR", 0, "4307FR" },
        /*594*/ { "[4307]F", ZINT_ERROR_INVALID_DATA, "" },
        /*595*/ { "[4307]FRR", ZINT_ERROR_INVALID_DATA, "" },
        /*596*/ { "[4308]1", 0, "43081" },
        /*597*/ { "[4308]123456789012345678901234567890", 0, "4308123456789012345678901234567890" },
        /*598*/ { "[4308]1234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*599*/ { "[4309]1", ZINT_ERROR_INVALID_DATA, "" },
        /*600*/ { "[431]1", ZINT_ERROR_INVALID_DATA, "" },
        /*601*/ { "[4310]1", 0, "43101" },
        /*602*/ { "[4310]12345678901234567890123456789012345", 0, "431012345678901234567890123456789012345" },
        /*603*/ { "[4310]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*604*/ { "[4311]1", 0, "43111" },
        /*605*/ { "[4311]12345678901234567890123456789012345", 0, "431112345678901234567890123456789012345" },
        /*606*/ { "[4311]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*607*/ { "[4312]1", 0, "43121" },
        /*608*/ { "[4312]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43121234567890123456789012345678901234567890123456789012345678901234567890" },
        /*609*/ { "[4312]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*610*/ { "[4313]1", 0, "43131" },
        /*611*/ { "[4313]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43131234567890123456789012345678901234567890123456789012345678901234567890" },
        /*612*/ { "[4313]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*613*/ { "[4314]1", 0, "43141" },
        /*614*/ { "[4314]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43141234567890123456789012345678901234567890123456789012345678901234567890" },
        /*615*/ { "[4314]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*616*/ { "[4315]1", 0, "43151" },
        /*617*/ { "[4315]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43151234567890123456789012345678901234567890123456789012345678901234567890" },
        /*618*/ { "[4315]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*619*/ { "[4316]1", 0, "43161" },
        /*620*/ { "[4316]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "43161234567890123456789012345678901234567890123456789012345678901234567890" },
        /*621*/ { "[4316]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*622*/ { "[4317]FR", 0, "4317FR" },
        /*623*/ { "[4317]F", ZINT_ERROR_INVALID_DATA, "" },
        /*624*/ { "[4317]FRF", ZINT_ERROR_INVALID_DATA, "" },
        /*625*/ { "[4318]1", 0, "43181" },
        /*626*/ { "[4318]12345678901234567890", 0, "431812345678901234567890" },
        /*627*/ { "[4318]123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*628*/ { "[4319]1", 0, "43191" },
        /*629*/ { "[4319]123456789012345678901234567890", 0, "4319123456789012345678901234567890" },
        /*630*/ { "[4319]1234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*631*/ { "[432]1", ZINT_ERROR_INVALID_DATA, "" },
        /*632*/ { "[4320]1", 0, "43201" },
        /*633*/ { "[4320]12345678901234567890123456789012345", 0, "432012345678901234567890123456789012345" },
        /*634*/ { "[4320]123456789012345678901234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*635*/ { "[4321]1", 0, "43211" },
        /*636*/ { "[4321]10", ZINT_ERROR_INVALID_DATA, "" },
        /*637*/ { "[4322]1", 0, "43221" },
        /*638*/ { "[4322]10", ZINT_ERROR_INVALID_DATA, "" },
        /*639*/ { "[4323]1", 0, "43231" },
        /*640*/ { "[4323]10", ZINT_ERROR_INVALID_DATA, "" },
        /*641*/ { "[4324]1212120000", 0, "43241212120000" },
        /*642*/ { "[4324]121212000", ZINT_ERROR_INVALID_DATA, "" },
        /*643*/ { "[4324]12121200000", ZINT_ERROR_INVALID_DATA, "" },
        /*644*/ { "[4325]1212120000", 0, "43251212120000" },
        /*645*/ { "[4325]121212000", ZINT_ERROR_INVALID_DATA, "" },
        /*646*/ { "[4325]12121200000", ZINT_ERROR_INVALID_DATA, "" },
        /*647*/ { "[4326]121212", 0, "4326121212" },
        /*648*/ { "[4326]12121", ZINT_ERROR_INVALID_DATA, "" },
        /*649*/ { "[4326]1212120", ZINT_ERROR_INVALID_DATA, "" },
        /*650*/ { "[4327]121212", ZINT_ERROR_INVALID_DATA, "" },
        /*651*/ { "[4328]121212", ZINT_ERROR_INVALID_DATA, "" },
        /*652*/ { "[4329]121212", ZINT_ERROR_INVALID_DATA, "" },
        /*653*/ { "[433]121212", ZINT_ERROR_INVALID_DATA, "" },
        /*654*/ { "[4330]121212", ZINT_ERROR_INVALID_DATA, "" },
        /*655*/ { "[44]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*656*/ { "[440]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*657*/ { "[4400]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*658*/ { "[49]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*659*/ { "[490]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*660*/ { "[4900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*661*/ { "[499]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*662*/ { "[4990]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*663*/ { "[50]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*664*/ { "[500]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*665*/ { "[5000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*666*/ { "[51]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*667*/ { "[510]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*668*/ { "[5100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*669*/ { "[59]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*670*/ { "[590]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*671*/ { "[5900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*672*/ { "[60]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*673*/ { "[600]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*674*/ { "[6000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*675*/ { "[61]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*676*/ { "[610]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*677*/ { "[6100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*678*/ { "[69]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*679*/ { "[690]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*680*/ { "[6900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*681*/ { "[70]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*682*/ { "[700]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*683*/ { "[7000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*684*/ { "[7001]1234567890123", 0, "70011234567890123" },
        /*685*/ { "[7001]123456789012", ZINT_ERROR_INVALID_DATA, "" },
        /*686*/ { "[7002]abcdefghijklmnopqrstuvwxyz1234", 0, "7002abcdefghijklmnopqrstuvwxyz1234" },
        /*687*/ { "[7002]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*688*/ { "[7003]1212121212", 0, "70031212121212" },
        /*689*/ { "[7003]121212121", ZINT_ERROR_INVALID_DATA, "" },
        /*690*/ { "[7004]1234", 0, "70041234" },
        /*691*/ { "[7004]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*692*/ { "[7005]abcdefghijkl", 0, "7005abcdefghijkl" },
        /*693*/ { "[7005]abcdefghijklm", ZINT_ERROR_INVALID_DATA, "" },
        /*694*/ { "[7006]200132", ZINT_WARN_NONCOMPLIANT, "7006200132" },
        /*695*/ { "[7006]200100", ZINT_WARN_NONCOMPLIANT, "7006200100" },
        /*696*/ { "[7006]200120", 0, "7006200120" },
        /*697*/ { "[7006]2001320", ZINT_ERROR_INVALID_DATA, "" },
        /*698*/ { "[7007]010101121212", 0, "7007010101121212" },
        /*699*/ { "[7007]01010112121", ZINT_ERROR_INVALID_DATA, "" },
        /*700*/ { "[7007]A1010112121", ZINT_ERROR_INVALID_DATA, "" },
        /*701*/ { "[7007]121212", 0, "7007121212" },
        /*702*/ { "[7007]12121", ZINT_ERROR_INVALID_DATA, "" },
        /*703*/ { "[7007]1212121", ZINT_ERROR_INVALID_DATA, "" },
        /*704*/ { "[7008]abc", 0, "7008abc" },
        /*705*/ { "[7008]abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*706*/ { "[7009]abcdefghij", 0, "7009abcdefghij" },
        /*707*/ { "[7009]abcdefghijk", ZINT_ERROR_INVALID_DATA, "" },
        /*708*/ { "[7010]01", 0, "701001" },
        /*709*/ { "[7010]1", 0, "70101" },
        /*710*/ { "[7010]012", ZINT_ERROR_INVALID_DATA, "" },
        /*711*/ { "[7011]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*712*/ { "[7012]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*713*/ { "[7019]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*714*/ { "[7020]abcdefghijklmnopqrst", 0, "7020abcdefghijklmnopqrst" },
        /*715*/ { "[7020]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*716*/ { "[7021]abcdefghijklmnopqrst", 0, "7021abcdefghijklmnopqrst" },
        /*717*/ { "[7021]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*718*/ { "[7022]abcdefghijklmnopqrst", 0, "7022abcdefghijklmnopqrst" },
        /*719*/ { "[7022]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*720*/ { "[7023]1234abcdefghijklmnopqrstuvwxyz", 0, "70231234abcdefghijklmnopqrstuvwxyz" },
        /*721*/ { "[7023]1234abcdefghijklmnopqrstuvwxyza", ZINT_ERROR_INVALID_DATA, "" },
        /*722*/ { "[7024]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*723*/ { "[7025]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*724*/ { "[7029]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*725*/ { "[7030]123abcdefghijklmnopqrstuvwxyza", ZINT_WARN_NONCOMPLIANT, "7030123abcdefghijklmnopqrstuvwxyza" },
        /*726*/ { "[7030]004abcdefghijklmnopqrstuvwxyza", 0, "7030004abcdefghijklmnopqrstuvwxyza" },
        /*727*/ { "[7030]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*728*/ { "[7031]123abcdefghijklmnopqrstuvwxyza", ZINT_WARN_NONCOMPLIANT, "7031123abcdefghijklmnopqrstuvwxyza" },
        /*729*/ { "[7031]004abcdefghijklmnopqrstuvwxyza", 0, "7031004abcdefghijklmnopqrstuvwxyza" },
        /*730*/ { "[7031]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*731*/ { "[7032]004abcdefghijklmnopqrstuvwxyza", 0, "7032004abcdefghijklmnopqrstuvwxyza" },
        /*732*/ { "[7032]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*733*/ { "[7033]004abcdefghijklmnopqrstuvwxyza", 0, "7033004abcdefghijklmnopqrstuvwxyza" },
        /*734*/ { "[7033]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*735*/ { "[7034]004abcdefghijklmnopqrstuvwxyza", 0, "7034004abcdefghijklmnopqrstuvwxyza" },
        /*736*/ { "[7034]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*737*/ { "[7035]004abcdefghijklmnopqrstuvwxyza", 0, "7035004abcdefghijklmnopqrstuvwxyza" },
        /*738*/ { "[7035]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*739*/ { "[7036]004abcdefghijklmnopqrstuvwxyza", 0, "7036004abcdefghijklmnopqrstuvwxyza" },
        /*740*/ { "[7036]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*741*/ { "[7037]004abcdefghijklmnopqrstuvwxyza", 0, "7037004abcdefghijklmnopqrstuvwxyza" },
        /*742*/ { "[7037]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*743*/ { "[7038]004abcdefghijklmnopqrstuvwxyza", 0, "7038004abcdefghijklmnopqrstuvwxyza" },
        /*744*/ { "[7038]004abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*745*/ { "[7039]004abcdefghijklmnopqrstuvwxyza", 0, "7039004abcdefghijklmnopqrstuvwxyza" },
        /*746*/ { "[7039]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*747*/ { "[7040]1abc", 0, "70401abc" },
        /*748*/ { "[7040]1ab", ZINT_ERROR_INVALID_DATA, "" },
        /*749*/ { "[7040]1abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*750*/ { "[7041]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*751*/ { "[7042]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*752*/ { "[7050]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*753*/ { "[7090]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*754*/ { "[7099]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*755*/ { "[71]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*756*/ { "[710]abcdefghijklmnopqrst", 0, "710abcdefghijklmnopqrst" },
        /*757*/ { "[710]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*758*/ { "[7100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*759*/ { "[711]abcdefghijklmnopqrst", 0, "711abcdefghijklmnopqrst" },
        /*760*/ { "[711]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*761*/ { "[712]abcdefghijklmnopqrst", 0, "712abcdefghijklmnopqrst" },
        /*762*/ { "[712]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*763*/ { "[713]abcdefghijklmnopqrst", 0, "713abcdefghijklmnopqrst" },
        /*764*/ { "[713]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*765*/ { "[714]abcdefghijklmnopqrst", 0, "714abcdefghijklmnopqrst" },
        /*766*/ { "[714]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*767*/ { "[715]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*768*/ { "[716]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*769*/ { "[719]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*770*/ { "[72]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*771*/ { "[720]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*772*/ { "[7200]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*773*/ { "[721]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*774*/ { "[7210]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*775*/ { "[7220]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*776*/ { "[7230]EMabcdefghijklmnopqrstuvwxyzab", 0, "7230EMabcdefghijklmnopqrstuvwxyzab" },
        /*777*/ { "[7230]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*778*/ { "[7230]EM", ZINT_ERROR_INVALID_DATA, "" },
        /*779*/ { "[7231]EMabcdefghijklmnopqrstuvwxyzab", 0, "7231EMabcdefghijklmnopqrstuvwxyzab" },
        /*780*/ { "[7231]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*781*/ { "[7232]EMabcdefghijklmnopqrstuvwxyzab", 0, "7232EMabcdefghijklmnopqrstuvwxyzab" },
        /*782*/ { "[7232]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*783*/ { "[7233]EMabcdefghijklmnopqrstuvwxyzab", 0, "7233EMabcdefghijklmnopqrstuvwxyzab" },
        /*784*/ { "[7233]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*785*/ { "[7234]EMabcdefghijklmnopqrstuvwxyzab", 0, "7234EMabcdefghijklmnopqrstuvwxyzab" },
        /*786*/ { "[7234]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*787*/ { "[7235]EMabcdefghijklmnopqrstuvwxyzab", 0, "7235EMabcdefghijklmnopqrstuvwxyzab" },
        /*788*/ { "[7235]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*789*/ { "[7236]EMabcdefghijklmnopqrstuvwxyzab", 0, "7236EMabcdefghijklmnopqrstuvwxyzab" },
        /*790*/ { "[7236]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*791*/ { "[7237]EMabcdefghijklmnopqrstuvwxyzab", 0, "7237EMabcdefghijklmnopqrstuvwxyzab" },
        /*792*/ { "[7237]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*793*/ { "[7238]EMabcdefghijklmnopqrstuvwxyzab", 0, "7238EMabcdefghijklmnopqrstuvwxyzab" },
        /*794*/ { "[7238]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*795*/ { "[7239]EMabcdefghijklmnopqrstuvwxyzab", 0, "7239EMabcdefghijklmnopqrstuvwxyzab" },
        /*796*/ { "[7239]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*797*/ { "[7239]E", ZINT_ERROR_INVALID_DATA, "" },
        /*798*/ { "[7240]abcdefghijklmnopqrst", 0, "7240abcdefghijklmnopqrst" },
        /*799*/ { "[7240]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*800*/ { "[7241]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*801*/ { "[7249]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*802*/ { "[7250]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*803*/ { "[7299]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*804*/ { "[73]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*805*/ { "[7300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*806*/ { "[74]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*807*/ { "[7400]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*808*/ { "[79]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*809*/ { "[7900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*810*/ { "[7999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*811*/ { "[80]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*812*/ { "[800]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*813*/ { "[8000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*814*/ { "[8001]12345678901234", ZINT_WARN_NONCOMPLIANT, "800112345678901234" },
        /*815*/ { "[8001]12345678901204", 0, "800112345678901204" },
        /*816*/ { "[8001]1234123456789012345", ZINT_ERROR_INVALID_DATA, "" },
        /*817*/ { "[8002]abcdefghijklmnopqrst", 0, "8002abcdefghijklmnopqrst" },
        /*818*/ { "[8002]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*819*/ { "[8003]01234567890123abcdefghijklmnop", ZINT_WARN_NONCOMPLIANT, "800301234567890123abcdefghijklmnop" },
        /*820*/ { "[8003]01234567890128abcdefghijklmnop", 0, "800301234567890128abcdefghijklmnop" },
        /*821*/ { "[8003]01234567890128abcdefghijklmnopq", ZINT_ERROR_INVALID_DATA, "" },
        /*822*/ { "[8004]abcdefghijklmnopqrstuvwxyz1234", ZINT_WARN_NONCOMPLIANT, "8004abcdefghijklmnopqrstuvwxyz1234" },
        /*823*/ { "[8004]12cdefghijklmnopqrstuvwxyz1234", 0, "800412cdefghijklmnopqrstuvwxyz1234" },
        /*824*/ { "[8004]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*825*/ { "[8005]123456", 0, "8005123456" },
        /*826*/ { "[8005]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*827*/ { "[8005]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*828*/ { "[8006]123456789012341212", ZINT_WARN_NONCOMPLIANT, "8006123456789012341212" },
        /*829*/ { "[8006]123456789012311212", 0, "8006123456789012311212" },
        /*830*/ { "[8006]12345678901234121", ZINT_ERROR_INVALID_DATA, "" },
        /*831*/ { "[8006]1234567890123412123", ZINT_ERROR_INVALID_DATA, "" },
        /*832*/ { "[8007]abcdefghijklmnopqrstuvwxyz12345678", ZINT_WARN_NONCOMPLIANT, "8007abcdefghijklmnopqrstuvwxyz12345678" },
        /*833*/ { "[8007]AD95EFGHIJKLMNOPQRSTUVWXYZ12345678", 0, "8007AD95EFGHIJKLMNOPQRSTUVWXYZ12345678" },
        /*834*/ { "[8007]AD95EFGHIJKLMNOPQRSTUVWXYZ123456789", ZINT_ERROR_INVALID_DATA, "" },
        /*835*/ { "[8008]123456121212", ZINT_WARN_NONCOMPLIANT, "8008123456121212" },
        /*836*/ { "[8008]121256121212", ZINT_WARN_NONCOMPLIANT, "8008121256121212" },
        /*837*/ { "[8008]121231121212", 0, "8008121231121212" },
        /*838*/ { "[8008]1234561212", ZINT_WARN_NONCOMPLIANT, "80081234561212" },
        /*839*/ { "[8008]1212311212", 0, "80081212311212" },
        /*840*/ { "[8008]12345612", ZINT_WARN_NONCOMPLIANT, "800812345612" },
        /*841*/ { "[8008]12010112", 0, "800812010112" },
        /*842*/ { "[8008]1234561", ZINT_ERROR_INVALID_DATA, "" },
        /*843*/ { "[8008]123456121", ZINT_ERROR_INVALID_DATA, "" },
        /*844*/ { "[8008]12345612121", ZINT_ERROR_INVALID_DATA, "" },
        /*845*/ { "[8008]1234561212123", ZINT_ERROR_INVALID_DATA, "" },
        /*846*/ { "[8009]12345678901234567890123456789012345678901234567890", 0, "800912345678901234567890123456789012345678901234567890" },
        /*847*/ { "[8009]123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*848*/ { "[8010]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*849*/ { "[8011]123456789012", 0, "8011123456789012" },
        /*850*/ { "[8011]1234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /*851*/ { "[8012]abcdefghijklmnopqrst", 0, "8012abcdefghijklmnopqrst" },
        /*852*/ { "[8012]abcdefghijklmnopqrstuv", ZINT_ERROR_INVALID_DATA, "" },
        /*853*/ { "[8013]1234abcdefghijklmnopqrstuvwxyz", 0, "80131234abcdefghijklmnopqrstuvwxyz" },
        /*854*/ { "[8013]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*855*/ { "[8014]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*856*/ { "[8016]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*857*/ { "[8017]313131313131313139", ZINT_WARN_NONCOMPLIANT, "8017313131313131313139" },
        /*858*/ { "[8017]313131313131313131", 0, "8017313131313131313131" },
        /*859*/ { "[8017]31313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*860*/ { "[8017]3131313131313131390", ZINT_ERROR_INVALID_DATA, "" },
        /*861*/ { "[8018]313131313131313139", ZINT_WARN_NONCOMPLIANT, "8018313131313131313139" },
        /*862*/ { "[8018]313131313131313131", 0, "8018313131313131313131" },
        /*863*/ { "[8018]31313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*864*/ { "[8018]3131313131313131390", ZINT_ERROR_INVALID_DATA, "" },
        /*865*/ { "[8019]1234567890", 0, "80191234567890" },
        /*866*/ { "[8019]12345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*867*/ { "[8020]abcdefghijklmnopqrstuvwxy", 0, "8020abcdefghijklmnopqrstuvwxy" },
        /*868*/ { "[8020]abcdefghijklmnopqrstuvwxyz", ZINT_ERROR_INVALID_DATA, "" },
        /*869*/ { "[8021]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*870*/ { "[8025]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*871*/ { "[8026]123456789012341212", ZINT_WARN_NONCOMPLIANT, "8026123456789012341212" },
        /*872*/ { "[8026]123456789012311212", 0, "8026123456789012311212" },
        /*873*/ { "[8026]1234567890123451212", ZINT_ERROR_INVALID_DATA, "" },
        /*874*/ { "[8026]12345678901234512", ZINT_ERROR_INVALID_DATA, "" },
        /*875*/ { "[8027]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*876*/ { "[8030]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*877*/ { "[8040]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*878*/ { "[8050]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*879*/ { "[8060]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*880*/ { "[8070]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*881*/ { "[8080]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*882*/ { "[8090]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*883*/ { "[8099]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*884*/ { "[81]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*885*/ { "[8100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*886*/ { "[8109]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*887*/ { "[8110]5123456789011234565123455123450123105123450123512345678901320123190000", 0, "81105123456789011234565123455123450123105123450123512345678901320123190000" },
        /*888*/ { "[8110]51234567890112345651234551234501231051234501235123456789013201231900001", ZINT_ERROR_INVALID_DATA, "" },
        /*889*/ { "[8111]1234", 0, "81111234" },
        /*890*/ { "[8111]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*891*/ { "[8111]123", ZINT_ERROR_INVALID_DATA, "" },
        /*892*/ { "[8112]1234567890123456789012345678901234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, "81121234567890123456789012345678901234567890123456789012345678901234567890" },
        /*893*/ { "[8112]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*894*/ { "[8112]061234567890121234569123456789012345", 0, "8112061234567890121234569123456789012345" },
        /*895*/ { "[8113]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*896*/ { "[8120]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*897*/ { "[8130]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*898*/ { "[8140]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*899*/ { "[8150]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*900*/ { "[8190]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*901*/ { "[8199]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*902*/ { "[82]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*903*/ { "[8200]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "82001234567890123456789012345678901234567890123456789012345678901234567890" },
        /*904*/ { "[8201]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*905*/ { "[8210]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*906*/ { "[8220]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*907*/ { "[8230]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*908*/ { "[8240]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*909*/ { "[8250]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*910*/ { "[8290]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*911*/ { "[8299]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*912*/ { "[83]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*913*/ { "[830]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*914*/ { "[8300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*915*/ { "[84]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*916*/ { "[840]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*917*/ { "[8400]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*918*/ { "[85]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*919*/ { "[850]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*920*/ { "[8500]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*921*/ { "[89]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*922*/ { "[890]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*923*/ { "[8900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*924*/ { "[90]abcdefghijklmnopqrstuvwxyz1234", 0, "90abcdefghijklmnopqrstuvwxyz1234" },
        /*925*/ { "[90]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*926*/ { "[900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*927*/ { "[9000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*928*/ { "[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "91123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*929*/ { "[91]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*930*/ { "[910]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*931*/ { "[9100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*932*/ { "[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "92123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*933*/ { "[92]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*934*/ { "[920]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*935*/ { "[9200]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*936*/ { "[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "93123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*937*/ { "[93]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*938*/ { "[930]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*939*/ { "[9300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*940*/ { "[94]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "94123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*941*/ { "[94]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*942*/ { "[940]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*943*/ { "[9400]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*944*/ { "[95]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "95123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*945*/ { "[95]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*946*/ { "[950]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*947*/ { "[9500]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*948*/ { "[96]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "96123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*949*/ { "[96]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*950*/ { "[960]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*951*/ { "[9600]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*952*/ { "[97]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "97123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*953*/ { "[97]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*954*/ { "[970]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*955*/ { "[9700]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*956*/ { "[98]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "98123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*957*/ { "[98]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*958*/ { "[980]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*959*/ { "[9800]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*960*/ { "[99]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "99123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*961*/ { "[99]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*962*/ { "[990]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*963*/ { "[9900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*964*/ { "[9999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*965*/ { "[01]12345678901234[7006]200101", ZINT_WARN_NONCOMPLIANT, "01123456789012347006200101" },
        /*966*/ { "[01]12345678901231[7006]200101", 0, "01123456789012317006200101" },
        /*967*/ { "[3900]1234567890[01]12345678901234", ZINT_WARN_NONCOMPLIANT, "39001234567890[0112345678901234" },
        /*968*/ { "[3900]1234567890[01]12345678901231", 0, "39001234567890[0112345678901231" },
        /*969*/ { "[253]12345678901234[3901]12345678901234[20]12", ZINT_WARN_NONCOMPLIANT, "25312345678901234[390112345678901234[2012" },
        /*970*/ { "[253]12345678901284[3901]12345678901234[20]12", 0, "25312345678901284[390112345678901234[2012" },
        /*971*/ { "[253]12345678901234[01]12345678901234[3901]12345678901234[20]12", ZINT_WARN_NONCOMPLIANT, "25312345678901234[0112345678901234390112345678901234[2012" },
        /*972*/ { "[253]12345678901284[01]12345678901231[3901]12345678901234[20]12", 0, "25312345678901284[0112345678901231390112345678901234[2012" },
    };
    int data_size = ARRAY_SIZE(data);

    char reduced[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = strlen(data[i].data);

        ret = gs1_verify(symbol, (unsigned char *) data[i].data, length, (unsigned char *) reduced);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (length %d \"%s\") %s\n", i, ret, data[i].ret, length, data[i].data, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_zero(strcmp(reduced, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, reduced, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_gs1_lint(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        char *data;
        int ret;
        char *expected;
        char *expected_errtxt;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "[00]123456789012345675", 0, "00123456789012345675", "" }, // numeric
        /*  1*/ { "[00]12345678901234567.", ZINT_WARN_NONCOMPLIANT, "0012345678901234567.", "261: AI (00) position 18: Non-numeric character '.'" }, // numeric
        /*  2*/ { "[00]123456789012345678", ZINT_WARN_NONCOMPLIANT, "00123456789012345678", "261: AI (00) position 18: Bad checksum '8', expected '5'" }, // csum
        /*  3*/ { "[91]!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", 0, "91!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", "" }, // cset82
        /*  4*/ { "[91] ", ZINT_WARN_NONCOMPLIANT, "91 ", "261: AI (91) position 1: Invalid CSET 82 character ' '" }, // cset82
        /*  5*/ { "[91]#", ZINT_WARN_NONCOMPLIANT, "91#", "261: AI (91) position 1: Invalid CSET 82 character '#'" }, // cset82
        /*  6*/ { "[91]a^", ZINT_WARN_NONCOMPLIANT, "91a^", "261: AI (91) position 2: Invalid CSET 82 character '^'" }, // cset82
        /*  7*/ { "[91]!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxy{", ZINT_WARN_NONCOMPLIANT, "91!\"%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxy{", "261: AI (91) position 82: Invalid CSET 82 character '{'" }, // cset82
        /*  8*/ { "[8010]01#-/23456789ABCDEFGHIJKLMNOPQ", 0, "801001#-/23456789ABCDEFGHIJKLMNOPQ", "" }, // cset39
        /*  9*/ { "[8010]6789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, "80106789ABCDEFGHIJKLMNOPQRSTUVWXYZ", "" }, // cset39
        /* 10*/ { "[8010]01!", ZINT_WARN_NONCOMPLIANT, "801001!", "261: AI (8010) position 3: Invalid CSET 39 character '!'" }, // cset39
        /* 11*/ { "[8010]01a", ZINT_WARN_NONCOMPLIANT, "801001a", "261: AI (8010) position 3: Invalid CSET 39 character 'a'" }, // cset39
        /* 12*/ { "[8010]6789ABCDEFGHIJKLMNOPQRSTUVWXY}", ZINT_WARN_NONCOMPLIANT, "80106789ABCDEFGHIJKLMNOPQRSTUVWXY}", "261: AI (8010) position 30: Invalid CSET 39 character '}'" }, // cset39
        /* 13*/ { "[8010]#-/0123456789ABCDEFGHIJKLMNOPQ", ZINT_WARN_NONCOMPLIANT, "8010#-/0123456789ABCDEFGHIJKLMNOPQ", "261: AI (8010) position 1: Non-numeric company prefix '#'" }, // key
        /* 14*/ { "[8010]0#-/123456789ABCDEFGHIJKLMNOPQ", ZINT_WARN_NONCOMPLIANT, "80100#-/123456789ABCDEFGHIJKLMNOPQ", "261: AI (8010) position 2: Non-numeric company prefix '#'" }, // key
        /* 15*/ { "[11]120100", 0, "11120100", "" }, // yymmd0
        /* 16*/ { "[11]120131", 0, "11120131", "" }, // yymmd0
        /* 17*/ { "[11]120132", ZINT_WARN_NONCOMPLIANT, "11120132", "261: AI (11) position 5: Invalid day '32'" }, // yymmd0
        /* 18*/ { "[11]120229", 0, "11120229", "" }, // yymmd0
        /* 19*/ { "[11]110229", ZINT_WARN_NONCOMPLIANT, "11110229", "261: AI (11) position 5: Invalid day '29'" }, // yymmd0
        /* 20*/ { "[11]000229", 0, "11000229", "" }, // yymmd0 NOTE: will be false in 2050
        /* 21*/ { "[11]480229", 0, "11480229", "" }, // yymmd0
        /* 22*/ { "[11]500229", ZINT_WARN_NONCOMPLIANT, "11500229", "261: AI (11) position 5: Invalid day '29'" }, // yymmd0
        /* 23*/ { "[11]980229", ZINT_WARN_NONCOMPLIANT, "11980229", "261: AI (11) position 5: Invalid day '29'" }, // yymmd0
        /* 24*/ { "[11]110228", 0, "11110228", "" }, // yymmd0
        /* 25*/ { "[11]120230", ZINT_WARN_NONCOMPLIANT, "11120230", "261: AI (11) position 5: Invalid day '30'" }, // yymmd0
        /* 26*/ { "[11]120331", 0, "11120331", "" }, // yymmd0
        /* 27*/ { "[11]120332", ZINT_WARN_NONCOMPLIANT, "11120332", "261: AI (11) position 5: Invalid day '32'" }, // yymmd0
        /* 28*/ { "[11]120430", 0, "11120430", "" }, // yymmd0
        /* 29*/ { "[11]120431", ZINT_WARN_NONCOMPLIANT, "11120431", "261: AI (11) position 5: Invalid day '31'" }, // yymmd0
        /* 30*/ { "[11]120531", 0, "11120531", "" }, // yymmd0
        /* 31*/ { "[11]120532", ZINT_WARN_NONCOMPLIANT, "11120532", "261: AI (11) position 5: Invalid day '32'" }, // yymmd0
        /* 32*/ { "[11]120630", 0, "11120630", "" }, // yymmd0
        /* 33*/ { "[11]120631", ZINT_WARN_NONCOMPLIANT, "11120631", "261: AI (11) position 5: Invalid day '31'" }, // yymmd0
        /* 34*/ { "[11]120731", 0, "11120731", "" }, // yymmd0
        /* 35*/ { "[11]120732", ZINT_WARN_NONCOMPLIANT, "11120732", "261: AI (11) position 5: Invalid day '32'" }, // yymmd0
        /* 36*/ { "[11]120831", 0, "11120831", "" }, // yymmd0
        /* 37*/ { "[11]120832", ZINT_WARN_NONCOMPLIANT, "11120832", "261: AI (11) position 5: Invalid day '32'" }, // yymmd0
        /* 38*/ { "[11]120930", 0, "11120930", "" }, // yymmd0
        /* 39*/ { "[11]120931", ZINT_WARN_NONCOMPLIANT, "11120931", "261: AI (11) position 5: Invalid day '31'" }, // yymmd0
        /* 40*/ { "[11]121031", 0, "11121031", "" }, // yymmd0
        /* 41*/ { "[11]121032", ZINT_WARN_NONCOMPLIANT, "11121032", "261: AI (11) position 5: Invalid day '32'" }, // yymmd0
        /* 42*/ { "[11]121130", 0, "11121130", "" }, // yymmd0
        /* 43*/ { "[11]121131", ZINT_WARN_NONCOMPLIANT, "11121131", "261: AI (11) position 5: Invalid day '31'" }, // yymmd0
        /* 44*/ { "[11]121200", 0, "11121200", "" }, // yymmd0
        /* 45*/ { "[11]121231", 0, "11121231", "" }, // yymmd0
        /* 46*/ { "[11]121232", ZINT_WARN_NONCOMPLIANT, "11121232", "261: AI (11) position 5: Invalid day '32'" }, // yymmd0
        /* 47*/ { "[11]120031", ZINT_WARN_NONCOMPLIANT, "11120031", "261: AI (11) position 3: Invalid month '00'" }, // yymmd0
        /* 48*/ { "[11]121331", ZINT_WARN_NONCOMPLIANT, "11121331", "261: AI (11) position 3: Invalid month '13'" }, // yymmd0
        /* 49*/ { "[4326]121231", 0, "4326121231", "" }, // yymmdd
        /* 50*/ { "[4326]121200", ZINT_WARN_NONCOMPLIANT, "4326121200", "261: AI (4326) position 5: Invalid day '00'" }, // yymmdd
        /* 51*/ { "[4326]120031", ZINT_WARN_NONCOMPLIANT, "4326120031", "261: AI (4326) position 3: Invalid month '00'" }, // yymmdd
        /* 52*/ { "[4326]129931", ZINT_WARN_NONCOMPLIANT, "4326129931", "261: AI (4326) position 3: Invalid month '99'" }, // yymmdd
        /* 53*/ { "[4326]121299", ZINT_WARN_NONCOMPLIANT, "4326121299", "261: AI (4326) position 5: Invalid day '99'" }, // yymmdd
        /* 54*/ { "[4326]120230", ZINT_WARN_NONCOMPLIANT, "4326120230", "261: AI (4326) position 5: Invalid day '30'" }, // yymmdd
        /* 55*/ { "[4326]110229", ZINT_WARN_NONCOMPLIANT, "4326110229", "261: AI (4326) position 5: Invalid day '29'" }, // yymmdd
        /* 56*/ { "[4326]000229", 0, "4326000229", "" }, // yymmdd NOTE: will be false in 2050
        /* 57*/ { "[4326]940229", ZINT_WARN_NONCOMPLIANT, "4326940229", "261: AI (4326) position 5: Invalid day '29'" }, // yymmdd
        /* 58*/ { "[4324]1212310000", 0, "43241212310000", "" }, // hhmm
        /* 59*/ { "[4324]1212312359", 0, "43241212312359", "" }, // hhmm
        /* 60*/ { "[4324]1212312400", ZINT_WARN_NONCOMPLIANT, "43241212312400", "261: AI (4324) position 7: Invalid hour of day '24'" }, // hhmm
        /* 61*/ { "[4324]1212312360", ZINT_WARN_NONCOMPLIANT, "43241212312360", "261: AI (4324) position 9: Invalid minutes in the hour '60'" }, // hhmm
        /* 62*/ { "[8008]121231000000", 0, "8008121231000000", "" }, // hhoptmmss
        /* 63*/ { "[8008]1212310000", 0, "80081212310000", "" }, // hhoptmmss
        /* 64*/ { "[8008]12123100", 0, "800812123100", "" }, // hhoptmmss
        /* 65*/ { "[8008]12123123", 0, "800812123123", "" }, // hhoptmmss
        /* 66*/ { "[8008]12123124", ZINT_WARN_NONCOMPLIANT, "800812123124", "261: AI (8008) position 7: Invalid hour of day '24'" }, // hhoptmmss
        /* 67*/ { "[8008]1212312359", 0, "80081212312359", "" }, // hhoptmmss
        /* 68*/ { "[8008]1212312360", ZINT_WARN_NONCOMPLIANT, "80081212312360", "261: AI (8008) position 9: Invalid minutes in the hour '60'" }, // hhoptmmss
        /* 69*/ { "[8008]121231235959", 0, "8008121231235959", "" }, // hhoptmmss
        /* 70*/ { "[8008]121231235960", ZINT_WARN_NONCOMPLIANT, "8008121231235960", "261: AI (8008) position 11: Invalid seconds in the minute '60'" }, // hhoptmmss
        /* 71*/ { "[422]004", 0, "422004", "" }, // iso3166
        /* 72*/ { "[422]894", 0, "422894", "" }, // iso3166
        /* 73*/ { "[422]00", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (422)" }, // iso3166
        /* 74*/ { "[422]0A", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (422)" }, // iso3166
        /* 75*/ { "[422]003", ZINT_WARN_NONCOMPLIANT, "422003", "261: AI (422) position 1: Unknown country code '003'" }, // iso3166
        /* 76*/ { "[422]895", ZINT_WARN_NONCOMPLIANT, "422895", "261: AI (422) position 1: Unknown country code '895'" }, // iso3166
        /* 77*/ { "[422]999", ZINT_WARN_NONCOMPLIANT, "422999", "261: AI (422) position 1: Unknown country code '999'" }, // iso3166
        /* 78*/ { "[423]004", 0, "423004", "" }, // iso3166list
        /* 79*/ { "[423]004894", 0, "423004894", "" }, // iso3166list
        /* 80*/ { "[423]004894004", 0, "423004894004", "" }, // iso3166list
        /* 81*/ { "[423]004894004894", 0, "423004894004894", "" }, // iso3166list
        /* 82*/ { "[423]004894004894004", 0, "423004894004894004", "" }, // iso3166list
        /* 83*/ { "[423]004894004894004894", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 84*/ { "[423]123894004894004894", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 85*/ { "[423]A04894004894004894", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 86*/ { "[423]00489400489400489", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 87*/ { "[423]0048940048940048", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 88*/ { "[423]00489400489400", ZINT_WARN_NONCOMPLIANT, "42300489400489400", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 89*/ { "[423]0048940048940", ZINT_WARN_NONCOMPLIANT, "4230048940048940", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 90*/ { "[423]00489400489", ZINT_WARN_NONCOMPLIANT, "42300489400489", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 91*/ { "[423]0048940048", ZINT_WARN_NONCOMPLIANT, "4230048940048", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 92*/ { "[423]00489400", ZINT_WARN_NONCOMPLIANT, "42300489400", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 93*/ { "[423]0048940", ZINT_WARN_NONCOMPLIANT, "4230048940", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 94*/ { "[423]00489", ZINT_WARN_NONCOMPLIANT, "42300489", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 95*/ { "[423]0048", ZINT_WARN_NONCOMPLIANT, "4230048", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 96*/ { "[423]00", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 97*/ { "[423]0", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (423)" }, // iso3166list
        /* 98*/ { "[423]004894004894003", ZINT_WARN_NONCOMPLIANT, "423004894004894003", "261: AI (423) position 13: Unknown country code '003'" }, // iso3166list
        /* 99*/ { "[423]004894004895004", ZINT_WARN_NONCOMPLIANT, "423004894004895004", "261: AI (423) position 10: Unknown country code '895'" }, // iso3166list
        /*100*/ { "[423]004894004999004", ZINT_WARN_NONCOMPLIANT, "423004894004999004", "261: AI (423) position 10: Unknown country code '999'" }, // iso3166list
        /*101*/ { "[423]004894005894004", ZINT_WARN_NONCOMPLIANT, "423004894005894004", "261: AI (423) position 7: Unknown country code '005'" }, // iso3166list
        /*102*/ { "[423]004893004894004", ZINT_WARN_NONCOMPLIANT, "423004893004894004", "261: AI (423) position 4: Unknown country code '893'" }, // iso3166list
        /*103*/ { "[423]004999004894004", ZINT_WARN_NONCOMPLIANT, "423004999004894004", "261: AI (423) position 4: Unknown country code '999'" }, // iso3166list
        /*104*/ { "[423]003894004894004", ZINT_WARN_NONCOMPLIANT, "423003894004894004", "261: AI (423) position 1: Unknown country code '003'" }, // iso3166list
        /*105*/ { "[423]004894004433", ZINT_WARN_NONCOMPLIANT, "423004894004433", "261: AI (423) position 10: Unknown country code '433'" }, // iso3166list
        /*106*/ { "[423]004894435894", ZINT_WARN_NONCOMPLIANT, "423004894435894", "261: AI (423) position 7: Unknown country code '435'" }, // iso3166list
        /*107*/ { "[423]004433004894", ZINT_WARN_NONCOMPLIANT, "423004433004894", "261: AI (423) position 4: Unknown country code '433'" }, // iso3166list
        /*108*/ { "[423]432894004894", ZINT_WARN_NONCOMPLIANT, "423432894004894", "261: AI (423) position 1: Unknown country code '432'" }, // iso3166list
        /*109*/ { "[423]004894003", ZINT_WARN_NONCOMPLIANT, "423004894003", "261: AI (423) position 7: Unknown country code '003'" }, // iso3166list
        /*110*/ { "[423]004895004", ZINT_WARN_NONCOMPLIANT, "423004895004", "261: AI (423) position 4: Unknown country code '895'" }, // iso3166list
        /*111*/ { "[423]004999004", ZINT_WARN_NONCOMPLIANT, "423004999004", "261: AI (423) position 4: Unknown country code '999'" }, // iso3166list
        /*112*/ { "[423]003894004", ZINT_WARN_NONCOMPLIANT, "423003894004", "261: AI (423) position 1: Unknown country code '003'" }, // iso3166list
        /*113*/ { "[423]004999", ZINT_WARN_NONCOMPLIANT, "423004999", "261: AI (423) position 4: Unknown country code '999'" }, // iso3166list
        /*114*/ { "[423]000894", ZINT_WARN_NONCOMPLIANT, "423000894", "261: AI (423) position 1: Unknown country code '000'" }, // iso3166list
        /*115*/ { "[423]003", ZINT_WARN_NONCOMPLIANT, "423003", "261: AI (423) position 1: Unknown country code '003'" }, // iso3166list
        /*116*/ { "[7030]999A", 0, "7030999A", "" }, // iso3166999
        /*117*/ { "[7030]894A", 0, "7030894A", "" }, // iso3166999
        /*118*/ { "[7030]004A", 0, "7030004A", "" }, // iso3166999
        /*119*/ { "[7030]004", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7030)" }, // iso3166999
        /*120*/ { "[7030]04", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (7030)" }, // iso3166999
        /*121*/ { "[7030]001A", ZINT_WARN_NONCOMPLIANT, "7030001A", "261: AI (7030) position 1: Unknown country code '001'" }, // iso3166999
        /*122*/ { "[7030]998A", ZINT_WARN_NONCOMPLIANT, "7030998A", "261: AI (7030) position 1: Unknown country code '998'" }, // iso3166999
        /*123*/ { "[3910]0081", 0, "39100081", "" }, // iso4217
        /*124*/ { "[3910]9991", 0, "39109991", "" }, // iso4217
        /*125*/ { "[3910]9971", 0, "39109971", "" }, // iso4217
        /*126*/ { "[3910]01", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3910)" }, // iso4217
        /*127*/ { "[3910]001", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (3910)" }, // iso4217
        /*128*/ { "[3910]9981", ZINT_WARN_NONCOMPLIANT, "39109981", "261: AI (3910) position 1: Unknown currency code '998'" }, // iso4217
        /*129*/ { "[3910]0041", ZINT_WARN_NONCOMPLIANT, "39100041", "261: AI (3910) position 1: Unknown currency code '004'" }, // iso4217
        /*130*/ { "[3910]8941", ZINT_WARN_NONCOMPLIANT, "39108941", "261: AI (3910) position 1: Unknown currency code '894'" }, // iso4217
        /*131*/ { "[4300]%12", 0, "4300%12", "" }, // pcenc
        /*132*/ { "[4300]%1", ZINT_WARN_NONCOMPLIANT, "4300%1", "261: AI (4300) position 1: Invalid % escape" }, // pcenc
        /*133*/ { "[4300]%", ZINT_WARN_NONCOMPLIANT, "4300%", "261: AI (4300) position 1: Invalid % escape" }, // pcenc
        /*134*/ { "[4300]12%1212", 0, "430012%1212", "" }, // pcenc
        /*135*/ { "[4300]12%1G12", ZINT_WARN_NONCOMPLIANT, "430012%1G12", "261: AI (4300) position 5: Invalid characters for percent encoding" }, // pcenc
        /*136*/ { "[4321]1", 0, "43211", "" }, // yesno
        /*137*/ { "[4321]0", 0, "43210", "" }, // yesno
        /*138*/ { "[4321]2", ZINT_WARN_NONCOMPLIANT, "43212", "261: AI (4321) position 1: Neither 0 nor 1 for yes or no" }, // yesno
        /*139*/ { "[4321]9", ZINT_WARN_NONCOMPLIANT, "43219", "261: AI (4321) position 1: Neither 0 nor 1 for yes or no" }, // yesno
        /*140*/ { "[7040]1234", 0, "70401234", "" }, // importeridx
        /*141*/ { "[7040]123A", 0, "7040123A", "" }, // importeridx
        /*142*/ { "[7040]123Z", 0, "7040123Z", "" }, // importeridx
        /*143*/ { "[7040]123a", 0, "7040123a", "" }, // importeridx
        /*144*/ { "[7040]123z", 0, "7040123z", "" }, // importeridx
        /*145*/ { "[7040]123-", 0, "7040123-", "" }, // importeridx
        /*146*/ { "[7040]123_", 0, "7040123_", "" }, // importeridx
        /*147*/ { "[7040]123!", ZINT_WARN_NONCOMPLIANT, "7040123!", "261: AI (7040) position 4: Invalid importer index '!'" }, // importeridx
        /*148*/ { "[7040]123/", ZINT_WARN_NONCOMPLIANT, "7040123/", "261: AI (7040) position 4: Invalid importer index '/'" }, // importeridx
        /*149*/ { "[7040]123:", ZINT_WARN_NONCOMPLIANT, "7040123:", "261: AI (7040) position 4: Invalid importer index ':'" }, // importeridx
        /*150*/ { "[7040]123?", ZINT_WARN_NONCOMPLIANT, "7040123?", "261: AI (7040) position 4: Invalid importer index '?'" }, // importeridx
        /*151*/ { "[8001]12341234512311", 0, "800112341234512311", "" }, // nonzero
        /*152*/ { "[8001]00010000100100", 0, "800100010000100100", "" }, // nonzero
        /*153*/ { "[8001]00001234512311", ZINT_WARN_NONCOMPLIANT, "800100001234512311", "261: AI (8001) position 1: Zero not permitted" }, // nonzero
        /*154*/ { "[8001]12340000012311", ZINT_WARN_NONCOMPLIANT, "800112340000012311", "261: AI (8001) position 5: Zero not permitted" }, // nonzero
        /*155*/ { "[8001]00010000100011", ZINT_WARN_NONCOMPLIANT, "800100010000100011", "261: AI (8001) position 10: Zero not permitted" }, // nonzero
        /*156*/ { "[8001]00010000100101", 0, "800100010000100101", "" }, // winding
        /*157*/ { "[8001]00010000100111", 0, "800100010000100111", "" }, // winding
        /*158*/ { "[8001]00010000100191", 0, "800100010000100191", "" }, // winding
        /*159*/ { "[8001]00010000100121", ZINT_WARN_NONCOMPLIANT, "800100010000100121", "261: AI (8001) position 13: Invalid winding direction '2'" }, // winding
        /*160*/ { "[8001]00010000100131", ZINT_WARN_NONCOMPLIANT, "800100010000100131", "261: AI (8001) position 13: Invalid winding direction '3'" }, // winding
        /*161*/ { "[8001]00010000100171", ZINT_WARN_NONCOMPLIANT, "800100010000100171", "261: AI (8001) position 13: Invalid winding direction '7'" }, // winding
        /*162*/ { "[8001]00010000100181", ZINT_WARN_NONCOMPLIANT, "800100010000100181", "261: AI (8001) position 13: Invalid winding direction '8'" }, // winding
        /*163*/ { "[8003]01234567890128", 0, "800301234567890128", "" }, // zero
        /*164*/ { "[8003]11234567890128", ZINT_WARN_NONCOMPLIANT, "800311234567890128", "261: AI (8003) position 1: Zero is required" }, // zero
        /*165*/ { "[8003]91234567890128", ZINT_WARN_NONCOMPLIANT, "800391234567890128", "261: AI (8003) position 1: Zero is required" }, // zero
        /*166*/ { "[8006]123456789012310101", 0, "8006123456789012310101", "" }, // pieceoftotal
        /*167*/ { "[8006]123456789012310199", 0, "8006123456789012310199", "" }, // pieceoftotal
        /*168*/ { "[8006]123456789012319999", 0, "8006123456789012319999", "" }, // pieceoftotal
        /*169*/ { "[8006]123456789012310001", ZINT_WARN_NONCOMPLIANT, "8006123456789012310001", "261: AI (8006) position 15: Piece number cannot be zero" }, // pieceoftotal
        /*170*/ { "[8006]123456789012310100", ZINT_WARN_NONCOMPLIANT, "8006123456789012310100", "261: AI (8006) position 15: Total number cannot be zero" }, // pieceoftotal
        /*171*/ { "[8006]123456789012310201", ZINT_WARN_NONCOMPLIANT, "8006123456789012310201", "261: AI (8006) position 15: Piece number '02' exceeds total '01'" }, // pieceoftotal
        /*172*/ { "[8006]123456789012319998", ZINT_WARN_NONCOMPLIANT, "8006123456789012319998", "261: AI (8006) position 15: Piece number '99' exceeds total '98'" }, // pieceoftotal
        /*173*/ { "[8007]GB82WEST12345698765432", 0, "8007GB82WEST12345698765432", "" }, // iban
        /*174*/ { "[8007]GB83WEST12345698765432", ZINT_WARN_NONCOMPLIANT, "8007GB83WEST12345698765432", "261: AI (8007) position 3: Bad IBAN checksum '83', expected '82'" }, // iban
        /*175*/ { "[8007]BE71096123456769", 0, "8007BE71096123456769", "" }, // iban
        /*176*/ { "[8007]BE71096123456760", ZINT_WARN_NONCOMPLIANT, "8007BE71096123456760", "261: AI (8007) position 3: Bad IBAN checksum '71', expected '23'" }, // iban
        /*177*/ { "[8007]BE01096123456760", ZINT_WARN_NONCOMPLIANT, "8007BE01096123456760", "261: AI (8007) position 3: Bad IBAN checksum '01', expected '23'" }, // iban
        /*178*/ { "[8007]BE00096123456760", ZINT_WARN_NONCOMPLIANT, "8007BE00096123456760", "261: AI (8007) position 3: Bad IBAN checksum '00', expected '23'" }, // iban
        /*179*/ { "[8007]LC14BOSL123456789012345678901234", 0, "8007LC14BOSL123456789012345678901234", "" }, // iban
        /*180*/ { "[8007]LC14BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007LC14BOSL123456789012345678901230", "261: AI (8007) position 3: Bad IBAN checksum '14', expected '25'" }, // iban
        /*181*/ { "[8007]A114BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007A114BOSL123456789012345678901230", "261: AI (8007) position 1: Non-alphabetic IBAN country code 'A1'" }, // iban
        /*182*/ { "[8007]1A14BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "80071A14BOSL123456789012345678901230", "261: AI (8007) position 1: Non-alphabetic IBAN country code '1A'" }, // iban
        /*183*/ { "[8007]AA14BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007AA14BOSL123456789012345678901230", "261: AI (8007) position 1: Invalid IBAN country code 'AA'" }, // iban
        /*184*/ { "[8007]ZZ14BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007ZZ14BOSL123456789012345678901230", "261: AI (8007) position 1: Invalid IBAN country code 'ZZ'" }, // iban
        /*185*/ { "[8007]ZW33BOSL123456789012345678901230", 0, "8007ZW33BOSL123456789012345678901230", "" }, // iban
        /*186*/ { "[8007]ZWA3BOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007ZWA3BOSL123456789012345678901230", "261: AI (8007) position 3: Non-numeric IBAN checksum 'A3'" }, // iban
        /*187*/ { "[8007]ZW3ABOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007ZW3ABOSL123456789012345678901230", "261: AI (8007) position 3: Non-numeric IBAN checksum '3A'" }, // iban
        /*188*/ { "[8007]ZW33bOSL123456789012345678901230", ZINT_WARN_NONCOMPLIANT, "8007ZW33bOSL123456789012345678901230", "261: AI (8007) position 5: Invalid IBAN character 'b'" }, // iban
        /*189*/ { "[8007]FR7630006000011234567890189", 0, "8007FR7630006000011234567890189", "" }, // iban
        /*190*/ { "[8007]DE91100000000123456789", 0, "8007DE91100000000123456789", "" }, // iban
        /*191*/ { "[8007]GR9608100010000001234567890", 0, "8007GR9608100010000001234567890", "" }, // iban
        /*192*/ { "[8007]MU43BOMM0101123456789101000MUR", 0, "8007MU43BOMM0101123456789101000MUR", "" }, // iban
        /*193*/ { "[8007]PL10105000997603123456789123", 0, "8007PL10105000997603123456789123", "" }, // iban
        /*194*/ { "[8007]RO09BCYP0000001234567890", 0, "8007RO09BCYP0000001234567890", "" }, // iban
        /*195*/ { "[8007]SA4420000001234567891234", 0, "8007SA4420000001234567891234", "" }, // iban
        /*196*/ { "[8007]ES7921000813610123456789", 0, "8007ES7921000813610123456789", "" }, // iban
        /*197*/ { "[8007]CH5604835012345678009", 0, "8007CH5604835012345678009", "" }, // iban
        /*198*/ { "[8007]GB98MIDL07009312345678", 0, "8007GB98MIDL07009312345678", "" }, // iban
        /*199*/ { "[8011]1", 0, "80111", "" }, // nozeroprefix
        /*200*/ { "[8011]11", 0, "801111", "" }, // nozeroprefix
        /*201*/ { "[8011]0", 0, "80110", "" }, // nozeroprefix
        /*202*/ { "[8011]01", ZINT_WARN_NONCOMPLIANT, "801101", "261: AI (8011) position 1: Zero prefix is not permitted" }, // nozeroprefix
        /*203*/ { "[8110]106141416543213150110120", 0, "8110106141416543213150110120", "" }, // couponcode (first part of NACAG Appendix C: Example 1 - see test_rss.c test_examples)
        /*204*/ { "[8110]012345612345610104123", 0, "8110012345612345610104123", "" }, // couponcode
        /*205*/ { "[8110]01234561234561010412", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412", "259: Invalid data length for AI (8110)" }, // couponcode
        /*206*/ { "[8110]12345678901234567890", ZINT_WARN_NONCOMPLIANT, "811012345678901234567890", "259: Invalid data length for AI (8110)" }, // couponcode
        /*207*/ { "[8110]712345612345610104123", ZINT_WARN_NONCOMPLIANT, "8110712345612345610104123", "261: AI (8110) position 1: Invalid Primary GS1 Co. Prefix VLI '7'" }, // couponcode
        /*208*/ { "[8110]A12345612345610104123", ZINT_WARN_NONCOMPLIANT, "8110A12345612345610104123", "261: AI (8110) position 1: Non-numeric Primary GS1 Co. Prefix VLI 'A'" }, // couponcode
        /*209*/ { "[8110]012345A12345610104123", ZINT_WARN_NONCOMPLIANT, "8110012345A12345610104123", "261: AI (8110) position 7: Non-numeric Primary GS1 Co. Prefix 'A'" }, // couponcode
        /*210*/ { "[8110]012345612345A10104123", ZINT_WARN_NONCOMPLIANT, "8110012345612345A10104123", "261: AI (8110) position 8: Non-numeric Offer Code" }, // couponcode
        /*211*/ { "[8110]012345612345600104123", ZINT_WARN_NONCOMPLIANT, "8110012345612345600104123", "261: AI (8110) position 14: Invalid Save Value VLI '0'" }, // couponcode
        /*212*/ { "[8110]012345612345660104123", ZINT_WARN_NONCOMPLIANT, "8110012345612345660104123", "261: AI (8110) position 14: Invalid Save Value VLI '6'" }, // couponcode
        /*213*/ { "[8110]01234561234561A104123", ZINT_WARN_NONCOMPLIANT, "811001234561234561A104123", "261: AI (8110) position 15: Non-numeric Save Value 'A'" }, // couponcode
        /*214*/ { "[8110]012345612345610004123", ZINT_WARN_NONCOMPLIANT, "8110012345612345610004123", "261: AI (8110) position 16: Invalid Primary Purch. Req. VLI '0'" }, // couponcode
        /*215*/ { "[8110]012345612345610604123", ZINT_WARN_NONCOMPLIANT, "8110012345612345610604123", "261: AI (8110) position 16: Invalid Primary Purch. Req. VLI '6'" }, // couponcode
        /*216*/ { "[8110]0123456123456101A4123", ZINT_WARN_NONCOMPLIANT, "81100123456123456101A4123", "261: AI (8110) position 17: Non-numeric Primary Purch. Req. 'A'" }, // couponcode
        /*217*/ { "[8110]012345612345621251234", ZINT_WARN_NONCOMPLIANT, "8110012345612345621251234", "261: AI (8110) position 18: Primary Purch. Req. incomplete" }, // couponcode
        /*218*/ { "[8110]012345612345610106123", ZINT_WARN_NONCOMPLIANT, "8110012345612345610106123", "261: AI (8110) position 18: Invalid Primary Purch. Req. Code '6'" }, // couponcode
        /*219*/ { "[8110]012345612345610212412", ZINT_WARN_NONCOMPLIANT, "8110012345612345610212412", "261: AI (8110) position 20: Primary Purch. Family Code incomplete" }, // couponcode
        /*220*/ { "[8110]0123456123456103123412", ZINT_WARN_NONCOMPLIANT, "81100123456123456103123412", "261: AI (8110) position 21: Primary Purch. Family Code incomplete" }, // couponcode
        /*221*/ { "[8110]0123456123456103123412A", ZINT_WARN_NONCOMPLIANT, "81100123456123456103123412A", "261: AI (8110) position 21: Non-numeric Primary Purch. Family Code" }, // couponcode
        /*222*/ { "[8110]01234561234561031234123", 0, "811001234561234561031234123", "" }, // couponcode
        /*223*/ { "[8110]612345678901212345651", ZINT_WARN_NONCOMPLIANT, "8110612345678901212345651", "261: AI (8110) position 21: Save Value incomplete" }, // couponcode
        /*224*/ { "[8110]6123456789012123456512345", ZINT_WARN_NONCOMPLIANT, "81106123456789012123456512345", "261: AI (8110) position 26: Primary Purch. Req. VLI missing" }, // couponcode
        /*225*/ { "[8110]61234567890121234565123455123454123", 0, "811061234567890121234565123455123454123", "" }, // couponcode
        /*226*/ { "[8110]61234567890121234565123455123454123A", ZINT_WARN_NONCOMPLIANT, "811061234567890121234565123455123454123A", "261: AI (8110) position 36: Non-numeric Data Field 'A'" }, // couponcode
        /*227*/ { "[8110]612345678901212345651234551234541237", ZINT_WARN_NONCOMPLIANT, "8110612345678901212345651234551234541237", "261: AI (8110) position 36: Invalid Data Field '7'" }, // couponcode
        /*228*/ { "[8110]612345678901212345651234551234541238", ZINT_WARN_NONCOMPLIANT, "8110612345678901212345651234551234541238", "261: AI (8110) position 36: Invalid Data Field '8'" }, // couponcode
        /*229*/ { "[8110]0123456123456101041231", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041231", "261: AI (8110) position 23: Add. Purch. Rules Code incomplete" }, // couponcode
        /*230*/ { "[8110]0123456123456101041231A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041231A", "261: AI (8110) position 23: Non-numeric Add. Purch. Rules Code" }, // couponcode
        /*231*/ { "[8110]01234561234561010412314", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412314", "261: AI (8110) position 23: Invalid Add. Purch. Rules Code '4'" }, // couponcode
        /*232*/ { "[8110]01234561234561010412313", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313", "261: AI (8110) position 24: 2nd Purch. Req. VLI missing" }, // couponcode
        /*233*/ { "[8110]01234561234561010412313A", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313A", "261: AI (8110) position 24: Non-numeric 2nd Purch. Req. VLI 'A'" }, // couponcode
        /*234*/ { "[8110]012345612345610104123130", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123130", "261: AI (8110) position 24: Invalid 2nd Purch. Req. VLI '0'" }, // couponcode
        /*235*/ { "[8110]012345612345610104123131", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123131", "261: AI (8110) position 25: 2nd Purch. Req. incomplete" }, // couponcode
        /*236*/ { "[8110]012345612345610104123131A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123131A", "261: AI (8110) position 25: Non-numeric 2nd Purch. Req. 'A'" }, // couponcode
        /*237*/ { "[8110]0123456123456101041231310", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041231310", "261: AI (8110) position 26: 2nd Purch. Req. Code incomplete" }, // couponcode
        /*238*/ { "[8110]0123456123456101041231310A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041231310A", "261: AI (8110) position 26: Non-numeric 2nd Purch. Req. Code" }, // couponcode
        /*239*/ { "[8110]01234561234561010412313108", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313108", "261: AI (8110) position 26: Invalid 2nd Purch. Req. Code '8'" }, // couponcode
        /*240*/ { "[8110]01234561234561010412313100", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313100", "261: AI (8110) position 27: 2nd Purch. Family Code incomplete" }, // couponcode
        /*241*/ { "[8110]01234561234561010412313100123", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313100123", "261: AI (8110) position 30: 2nd Purch. GS1 Co. Prefix VLI missing" }, // couponcode
        /*242*/ { "[8110]01234561234561010412313100123A", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313100123A", "261: AI (8110) position 30: Non-numeric 2nd Purch. GS1 Co. Prefix VLI 'A'" }, // couponcode
        /*243*/ { "[8110]012345612345610104123131001239", 0, "8110012345612345610104123131001239", "" }, // couponcode
        /*244*/ { "[8110]01234561234561010412313100123012345", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412313100123012345", "261: AI (8110) position 31: 2nd Purch. GS1 Co. Prefix incomplete" }, // couponcode
        /*245*/ { "[8110]0123456123456101041231310012311234567", 0, "81100123456123456101041231310012311234567", "" }, // couponcode
        /*246*/ { "[8110]0123456123456101041232", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232", "261: AI (8110) position 23: 3rd Purch. Req. VLI missing" }, // couponcode
        /*247*/ { "[8110]0123456123456101041232A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232A", "261: AI (8110) position 23: Non-numeric 3rd Purch. Req. VLI 'A'" }, // couponcode
        /*248*/ { "[8110]01234561234561010412326", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412326", "261: AI (8110) position 23: Invalid 3rd Purch. Req. VLI '6'" }, // couponcode
        /*249*/ { "[8110]01234561234561010412321", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412321", "261: AI (8110) position 24: 3rd Purch. Req. incomplete" }, // couponcode
        /*250*/ { "[8110]012345612345610104123210", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123210", "261: AI (8110) position 25: 3rd Purch. Req. Code incomplete" }, // couponcode
        /*251*/ { "[8110]0123456123456101041232105", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232105", "261: AI (8110) position 25: Invalid 3rd Purch. Req. Code '5'" }, // couponcode
        /*252*/ { "[8110]0123456123456101041232104", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104", "261: AI (8110) position 26: 3rd Purch. Family Code incomplete" }, // couponcode
        /*253*/ { "[8110]012345612345610104123210412A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123210412A", "261: AI (8110) position 26: Non-numeric 3rd Purch. Family Code" }, // couponcode
        /*254*/ { "[8110]0123456123456101041232104123", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104123", "261: AI (8110) position 29: 3rd Purch. GS1 Co. Prefix VLI missing" }, // couponcode
        /*255*/ { "[8110]01234561234561010412321041230", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412321041230", "261: AI (8110) position 30: 3rd Purch. GS1 Co. Prefix incomplete" }, // couponcode
        /*256*/ { "[8110]0123456123456101041232104123A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104123A", "261: AI (8110) position 29: Non-numeric 3rd Purch. GS1 Co. Prefix VLI 'A'" }, // couponcode
        /*257*/ { "[8110]0123456123456101041232104123012345", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104123012345", "261: AI (8110) position 30: 3rd Purch. GS1 Co. Prefix incomplete" }, // couponcode
        /*258*/ { "[8110]0123456123456101041232104123012345A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041232104123012345A", "261: AI (8110) position 35: Non-numeric 3rd Purch. GS1 Co. Prefix 'A'" }, // couponcode
        /*259*/ { "[8110]01234561234561010412321041230123456", 0, "811001234561234561010412321041230123456", "" }, // couponcode
        /*260*/ { "[8110]0123456123456101041233", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041233", "261: AI (8110) position 23: Expiration Date incomplete" }, // couponcode
        /*261*/ { "[8110]01234561234561010412332012", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412332012", "261: AI (8110) position 23: Expiration Date incomplete" }, // couponcode
        /*262*/ { "[8110]012345612345610104123320123A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123320123A", "261: AI (8110) position 23: Non-numeric Expiration Date" }, // couponcode
        /*263*/ { "[8110]0123456123456101041233201232", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041233201232", "261: AI (8110) position 27: Invalid day '32'" }, // couponcode
        /*264*/ { "[8110]0123456123456101041233200031", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041233200031", "261: AI (8110) position 25: Invalid month '00'" }, // couponcode
        /*265*/ { "[8110]0123456123456101041233201231", 0, "81100123456123456101041233201231", "" }, // couponcode
        /*266*/ { "[8110]0123456123456101041234", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041234", "261: AI (8110) position 23: Start Date incomplete" }, // couponcode
        /*267*/ { "[8110]01234561234561010412342012", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412342012", "261: AI (8110) position 23: Start Date incomplete" }, // couponcode
        /*268*/ { "[8110]012345612345610104123420123A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123420123A", "261: AI (8110) position 23: Non-numeric Start Date" }, // couponcode
        /*269*/ { "[8110]0123456123456101041234200230", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041234200230", "261: AI (8110) position 27: Invalid day '30'" }, // couponcode
        /*270*/ { "[8110]0123456123456101041234201329", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041234201329", "261: AI (8110) position 25: Invalid month '13'" }, // couponcode
        /*271*/ { "[8110]0123456123456101041234200229", 0, "81100123456123456101041234200229", "" }, // couponcode
        /*272*/ { "[8110]0123456123456101041235", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235", "261: AI (8110) position 23: Serial Number VLI missing" }, // couponcode
        /*273*/ { "[8110]0123456123456101041235A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235A", "261: AI (8110) position 23: Non-numeric Serial Number VLI 'A'" }, // couponcode
        /*274*/ { "[8110]01234561234561010412350", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412350", "261: AI (8110) position 24: Serial Number incomplete" }, // couponcode
        /*275*/ { "[8110]0123456123456101041235012345", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235012345", "261: AI (8110) position 24: Serial Number incomplete" }, // couponcode
        /*276*/ { "[8110]0123456123456101041235912345678901234", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235912345678901234", "261: AI (8110) position 24: Serial Number incomplete" }, // couponcode
        /*277*/ { "[8110]0123456123456101041235912345678901234A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041235912345678901234A", "261: AI (8110) position 38: Non-numeric Serial Number 'A'" }, // couponcode
        /*278*/ { "[8110]01234561234561010412359123456789012345", 0, "811001234561234561010412359123456789012345", "" }, // couponcode
        /*279*/ { "[8110]0123456123456101041236", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041236", "261: AI (8110) position 23: Retailer ID VLI missing" }, // couponcode
        /*280*/ { "[8110]0123456123456101041236A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041236A", "261: AI (8110) position 23: Non-numeric Retailer ID VLI 'A'" }, // couponcode
        /*281*/ { "[8110]01234561234561010412360", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412360", "261: AI (8110) position 23: Invalid Retailer ID VLI '0'" }, // couponcode
        /*282*/ { "[8110]01234561234561010412368", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412368", "261: AI (8110) position 23: Invalid Retailer ID VLI '8'" }, // couponcode
        /*283*/ { "[8110]01234561234561010412361", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412361", "261: AI (8110) position 24: Retailer ID incomplete" }, // couponcode
        /*284*/ { "[8110]01234561234561010412361123456", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412361123456", "261: AI (8110) position 24: Retailer ID incomplete" }, // couponcode
        /*285*/ { "[8110]01234561234561010412361123456A", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412361123456A", "261: AI (8110) position 30: Non-numeric Retailer ID 'A'" }, // couponcode
        /*286*/ { "[8110]012345612345610104123671234567890123", 0, "8110012345612345610104123671234567890123", "" }, // couponcode
        /*287*/ { "[8110]0123456123456101041239", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041239", "261: AI (8110) position 23: Save Value Code incomplete" }, // couponcode
        /*288*/ { "[8110]0123456123456101041239A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041239A", "261: AI (8110) position 23: Non-numeric Save Value Code" }, // couponcode
        /*289*/ { "[8110]01234561234561010412393", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412393", "261: AI (8110) position 23: Invalid Save Value Code '3'" }, // couponcode
        /*290*/ { "[8110]01234561234561010412394", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412394", "261: AI (8110) position 23: Invalid Save Value Code '4'" }, // couponcode
        /*291*/ { "[8110]01234561234561010412397", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412397", "261: AI (8110) position 23: Invalid Save Value Code '7'" }, // couponcode
        /*292*/ { "[8110]01234561234561010412390", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412390", "261: AI (8110) position 24: Save Value Applies To incomplete" }, // couponcode
        /*293*/ { "[8110]01234561234561010412390A", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412390A", "261: AI (8110) position 24: Non-numeric Save Value Applies To" }, // couponcode
        /*294*/ { "[8110]012345612345610104123903", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123903", "261: AI (8110) position 24: Invalid Save Value Applies To '3'" }, // couponcode
        /*295*/ { "[8110]012345612345610104123902", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123902", "261: AI (8110) position 25: Store Coupon Flag incomplete" }, // couponcode
        /*296*/ { "[8110]012345612345610104123902A", ZINT_WARN_NONCOMPLIANT, "8110012345612345610104123902A", "261: AI (8110) position 25: Non-numeric Store Coupon Flag" }, // couponcode
        /*297*/ { "[8110]0123456123456101041239029", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041239029", "261: AI (8110) position 26: Don't Multiply Flag incomplete" }, // couponcode
        /*298*/ { "[8110]0123456123456101041239029A", ZINT_WARN_NONCOMPLIANT, "81100123456123456101041239029A", "261: AI (8110) position 26: Non-numeric Don't Multiply Flag" }, // couponcode
        /*299*/ { "[8110]01234561234561010412390292", ZINT_WARN_NONCOMPLIANT, "811001234561234561010412390292", "261: AI (8110) position 26: Invalid Don't Multiply Flag '2'" }, // couponcode
        /*300*/ { "[8110]01234561234561010412390291", 0, "811001234561234561010412390291", "" }, // couponcode
        /*301*/ { "[8110]177777776666663100120444101105551888888821109991222222232012314200601522345678961345678990000", ZINT_ERROR_INVALID_DATA, "", "259: Invalid data length for AI (8110)" }, // couponcode (example from GS1 AI (8112) Coupon Data Specifications Appendix A: AI (8110) vs AI (8112))
        /*302*/ { "[8110]177777776666663100120444101105551888888821109991222222232012314200601", 0, "8110177777776666663100120444101105551888888821109991222222232012314200601", "" }, // couponcode
        /*303*/ { "[8112]017777777666666223456789", 0, "8112017777777666666223456789", "" }, // couponposoffer (example from GS1 AI (8112) Coupon Data Specifications Appendix A: AI (8110) vs AI (8112))
        /*304*/ { "[8112]001234561234560123456", 0, "8112001234561234560123456", "" }, // couponposoffer
        /*305*/ { "[8112]00123456123456012345", ZINT_WARN_NONCOMPLIANT, "811200123456123456012345", "259: Invalid data length for AI (8112)" }, // couponposoffer
        /*306*/ { "[8112]0012345612345601234561", ZINT_WARN_NONCOMPLIANT, "81120012345612345601234561", "261: AI (8112) position 22: Reserved trailing characters" }, // couponposoffer
        /*307*/ { "[8112]061234567890121234569123456789012345", 0, "8112061234567890121234569123456789012345", "" }, // couponposoffer
        /*308*/ { "[8112]0612345678901212345691234567890123456", ZINT_WARN_NONCOMPLIANT, "81120612345678901212345691234567890123456", "259: Invalid data length for AI (8112)" }, // couponposoffer
        /*309*/ { "[8112]06123456789012123456912345678901234A", ZINT_WARN_NONCOMPLIANT, "811206123456789012123456912345678901234A", "261: AI (8112) position 36: Non-numeric Serial Number 'A'" }, // couponposoffer
        /*310*/ { "[8112]06123456789012123456912345678901234", ZINT_WARN_NONCOMPLIANT, "811206123456789012123456912345678901234", "261: AI (8112) position 22: Serial Number incomplete" }, // couponposoffer
        /*311*/ { "[8112]06123456789012123456812345678901234", 0, "811206123456789012123456812345678901234", "" }, // couponposoffer
        /*312*/ { "[8112]0612345678901212345681234567890123", ZINT_WARN_NONCOMPLIANT, "81120612345678901212345681234567890123", "261: AI (8112) position 22: Serial Number incomplete" }, // couponposoffer
        /*313*/ { "[8112]0612345678901212345A0123456", ZINT_WARN_NONCOMPLIANT, "81120612345678901212345A0123456", "261: AI (8112) position 15: Non-numeric Offer Code" }, // couponposoffer
        /*314*/ { "[8112]0612345678901A1234560123456", ZINT_WARN_NONCOMPLIANT, "81120612345678901A1234560123456", "261: AI (8112) position 14: Non-numeric Coupon Funder ID 'A'" }, // couponposoffer
        /*315*/ { "[8112]071234567890121234560123456", ZINT_WARN_NONCOMPLIANT, "8112071234567890121234560123456", "261: AI (8112) position 2: Invalid Coupon Funder ID VLI '7'" }, // couponposoffer
        /*316*/ { "[8112]0A1234567890121234560123456", ZINT_WARN_NONCOMPLIANT, "81120A1234567890121234560123456", "261: AI (8112) position 2: Non-numeric Coupon Funder ID VLI 'A'" }, // couponposoffer
        /*317*/ { "[8112]261234567890121234560123456", ZINT_WARN_NONCOMPLIANT, "8112261234567890121234560123456", "261: AI (8112) position 1: Coupon Format must be 0 or 1" }, // couponposoffer
        /*318*/ { "[8112]A61234567890121234560123456", ZINT_WARN_NONCOMPLIANT, "8112A61234567890121234560123456", "261: AI (8112) position 1: Non-numeric Coupon Format" }, // couponposoffer
    };
    int data_size = ARRAY_SIZE(data);

    char reduced[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = strlen(data[i].data);

        ret = gs1_verify(symbol, (unsigned char *) data[i].data, length, (unsigned char *) reduced);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (length %d \"%s\") %s\n", i, ret, data[i].ret, length, data[i].data, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_zero(strcmp(reduced, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, reduced, data[i].expected);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input_mode(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        char *data;
        char *composite;
        int input_mode;
        int output_options;
        int ret;
        int compare_previous;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_AZTEC, "[01]12345678901234", "", GS1_MODE, -1 , 0, 0 },
        /*  1*/ { BARCODE_AZTEC, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /*  2*/ { BARCODE_AZTEC, "[01]12345678901234", "", GS1_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /*  3*/ { BARCODE_AZTEC, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /*  4*/ { BARCODE_AZTEC, "1234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*  5*/ { BARCODE_AZTEC, "1234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /*  6*/ { BARCODE_CODABLOCKF, "[01]12345678901234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 },
        /*  7*/ { BARCODE_CODABLOCKF, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 },
        /*  8*/ { BARCODE_CODABLOCKF, "1234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 },
        /*  9*/ { BARCODE_CODABLOCKF, "1234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 10*/ { BARCODE_CODEONE, "[01]12345678901234", "", GS1_MODE, -1, 0, 0 },
        /* 11*/ { BARCODE_CODEONE, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 12*/ { BARCODE_CODEONE, "1234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 13*/ { BARCODE_CODEONE, "1234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 14*/ { BARCODE_CODE16K, "[01]12345678901234", "", GS1_MODE, -1, 0, 0 },
        /* 15*/ { BARCODE_CODE16K, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 16*/ { BARCODE_CODE16K, "1234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 17*/ { BARCODE_CODE16K, "1234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 18*/ { BARCODE_CODE49, "[01]12345678901234", "", GS1_MODE, -1, 0, 0 },
        /* 19*/ { BARCODE_CODE49, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 20*/ { BARCODE_CODE49, "1234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 21*/ { BARCODE_CODE49, "1234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 22*/ { BARCODE_DATAMATRIX, "[01]12345678901234", "", GS1_MODE, -1, 0, 0 },
        /* 23*/ { BARCODE_DATAMATRIX, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 24*/ { BARCODE_DATAMATRIX, "[01]12345678901234", "", GS1_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 25*/ { BARCODE_DATAMATRIX, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, READER_INIT, ZINT_ERROR_INVALID_OPTION, 0 },
        /* 26*/ { BARCODE_DATAMATRIX, "1234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 27*/ { BARCODE_DATAMATRIX, "1234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 28*/ { BARCODE_DOTCODE, "[01]12345678901234", "", GS1_MODE, -1, 0, 0 },
        /* 29*/ { BARCODE_DOTCODE, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 30*/ { BARCODE_DOTCODE, "1234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 31*/ { BARCODE_DOTCODE, "1234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 32*/ { BARCODE_QRCODE, "[01]12345678901234", "", GS1_MODE, -1, 0, 0 },
        /* 33*/ { BARCODE_QRCODE, "[01]12345678901234", "", GS1_MODE | ESCAPE_MODE, -1, 0, 1 },
        /* 34*/ { BARCODE_QRCODE, "1234", "", GS1_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
        /* 35*/ { BARCODE_QRCODE, "1234", "", GS1_MODE | ESCAPE_MODE, -1, ZINT_ERROR_INVALID_DATA, 0 },
    };
    int data_size = ARRAY_SIZE(data);

    char *text;
    struct zint_symbol previous_symbol;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = data[i].input_mode;
        if (data[i].output_options != -1) {
            symbol->output_options = data[i].output_options;
        }
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int length = strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d %s\n", i, ret, data[i].ret, symbol->errtxt);
        if (index == -1 && data[i].compare_previous) {
            ret = testUtilSymbolCmp(symbol, &previous_symbol);
            assert_zero(ret, "i:%d testUtilSymbolCmp ret %d != 0\n", i, ret);
        }
        memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_gs1_reduce", test_gs1_reduce, 1, 1, 1 },
        { "test_hrt", test_hrt, 1, 0, 1 },
        { "test_gs1_verify", test_gs1_verify, 1, 0, 1 },
        { "test_gs1_lint", test_gs1_lint, 1, 0, 1 },
        { "test_input_mode", test_input_mode, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
