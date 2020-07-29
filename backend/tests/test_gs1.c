/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2020 Robin Stuart <rstuart114@gmail.com>

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
        unsigned char *data;
        unsigned char *composite;
        int ret;

        char *comment;
        unsigned char *expected;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_GS1_128, -1, "12345678901234", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /* 1*/ { BARCODE_GS1_128, -1, "[01]12345678901234", "", 0, "Input mode ignored; verified manually against tec-it",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110010001011000111010111101100011101011"
               },
        /* 2*/ { BARCODE_GS1_128, GS1_MODE, "[01]12345678901234", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110010001011000111010111101100011101011"
               },
        /* 3*/ { BARCODE_GS1_128, UNICODE_MODE, "[01]12345678901234", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110010001011000111010111101100011101011"
               },
        /* 4*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901234", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110111010011100010100001011010000110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001000101100011101011110100101111001100011101011"
               },
        /* 5*/ { BARCODE_GS1_128_CC, GS1_MODE, "[01]12345678901234", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110111010011100010100001011010000110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001000101100011101011110100101111001100011101011"
               },
        /* 6*/ { BARCODE_GS1_128_CC, UNICODE_MODE, "[01]12345678901234", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110111010011100010100001011010000110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001000101100011101011110100101111001100011101011"
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
    int data_size = sizeof(data) / sizeof(struct item);

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

        ret = ZBarcode_Encode(symbol, text, length);

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

            if (ret == 0) {
                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d %s testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(symbol->symbology, -1, -1, -1, debug)) {
                    ret = testUtilBwipp(symbol, -1, -1, -1, text, length, symbol->primary, bwipp_buf, sizeof(bwipp_buf));
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), ret);

                    ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(data[i].symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
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
        unsigned char *data;
        unsigned char *composite;

        unsigned char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_GS1_128, "[01]12345678901234[20]12", "", "(01)12345678901234(20)12" },
        /*  1*/ { BARCODE_GS1_128_CC, "[01]12345678901234[20]12", "[21]12345", "(01)12345678901234(20)12" },
        /*  2*/ { BARCODE_EAN14, "1234567890123", "", "(01)12345678901231" },
        /*  3*/ { BARCODE_NVE18, "12345678901234567", "", "(00)123456789012345675" },
        /*  4*/ { BARCODE_DBAR_EXP, "[01]12345678901234[20]12", "", "(01)12345678901234(20)12" },
        /*  5*/ { BARCODE_DBAR_EXP_CC, "[01]12345678901234", "[21]12345", "(01)12345678901234" },
        /*  6*/ { BARCODE_DBAR_EXPSTK, "[01]12345678901234[20]12", "", "" },
        /*  7*/ { BARCODE_DBAR_EXPSTK_CC, "[01]12345678901234[20]12", "[21]12345", "" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

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

        ret = ZBarcode_Encode(symbol, text, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_zero(strcmp(symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

extern int gs1_verify(struct zint_symbol *symbol, const unsigned char source[], const size_t src_len, char reduced[]);

static void test_gs1_verify(int index) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
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
        /*  9*/ { "[00]123456789012345678", 0, "00123456789012345678" },
        /* 10*/ { "[00]12345678901234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 11*/ { "[01]12345678901234", 0, "0112345678901234" },
        /* 12*/ { "[01]123456789012345", ZINT_ERROR_INVALID_DATA, "" },
        /* 13*/ { "[02]12345678901234", 0, "0212345678901234" },
        /* 14*/ { "[02]1234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /* 15*/ { "[03]12345678901234", ZINT_ERROR_INVALID_DATA, "" },
        /* 16*/ { "[04]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 17*/ { "[05]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 18*/ { "[06]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 19*/ { "[07]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 20*/ { "[08]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 21*/ { "[09]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 22*/ { "[10]ABCD123456", 0, "10ABCD123456" },
        /* 23*/ { "[10]123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /* 24*/ { "[100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 25*/ { "[11]990102", 0, "11990102" },
        /* 26*/ { "[11]9901023", ZINT_ERROR_INVALID_DATA, "" },
        /* 27*/ { "[12]000100", 0, "12000100" },
        /* 28*/ { "[12]00010", ZINT_ERROR_INVALID_DATA, "" },
        /* 29*/ { "[13]991301", 0, "13991301" },
        /* 30*/ { "[13]9913011", ZINT_ERROR_INVALID_DATA, "" },
        /* 31*/ { "[14]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 32*/ { "[15]021231", 0, "15021231" },
        /* 33*/ { "[15]02123", ZINT_ERROR_INVALID_DATA, "" },
        /* 34*/ { "[16]000000", 0, "16000000" },
        /* 35*/ { "[16]00000", ZINT_ERROR_INVALID_DATA, "" },
        /* 36*/ { "[17]010200", 0, "17010200" },
        /* 37*/ { "[17]0102000", ZINT_ERROR_INVALID_DATA, "" },
        /* 38*/ { "[18]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 39*/ { "[19]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 40*/ { "[20]12", 0, "2012" },
        /* 41*/ { "[20]1", ZINT_ERROR_INVALID_DATA, "" },
        /* 42*/ { "[200]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 43*/ { "[21]A12345678", 0, "21A12345678" },
        /* 44*/ { "[21]123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /* 45*/ { "[22]abcdefghijklmnopqrst", 0, "22abcdefghijklmnopqrst" },
        /* 46*/ { "[22]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /* 47*/ { "[23]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 48*/ { "[235]1abcdefghijklmnopqrstuvwxyz0", 0, "2351abcdefghijklmnopqrstuvwxyz0" },
        /* 49*/ { "[235]1abcdefghijklmnopqrstuvwxyz01", ZINT_ERROR_INVALID_DATA, "" },
        /* 50*/ { "[24]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 51*/ { "[240]abcdefghijklmnopqrstuvwxyz1234", 0, "240abcdefghijklmnopqrstuvwxyz1234" },
        /* 52*/ { "[240]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 53*/ { "[241]abcdefghijklmnopqrstuvwxyz1234", 0, "241abcdefghijklmnopqrstuvwxyz1234" },
        /* 54*/ { "[241]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 55*/ { "[242]12345", 0, "24212345" },
        /* 56*/ { "[242]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 57*/ { "[243]abcdefghijklmnopqrst", 0, "243abcdefghijklmnopqrst" },
        /* 58*/ { "[243]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /* 59*/ { "[244]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 60*/ { "[249]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 61*/ { "[25]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 62*/ { "[250]abcdefghijklmnopqrstuvwxyz1234", 0, "250abcdefghijklmnopqrstuvwxyz1234" },
        /* 63*/ { "[250]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 64*/ { "[251]abcdefghijklmnopqrstuvwxyz1234", 0, "251abcdefghijklmnopqrstuvwxyz1234" },
        /* 65*/ { "[251]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 66*/ { "[252]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 67*/ { "[253]131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /* 68*/ { "[253]1313131313134", 0, "2531313131313134" },
        /* 69*/ { "[253]131313131313412345678901234567", 0, "253131313131313412345678901234567" },
        /* 70*/ { "[253]1313131313134123456789012345678", ZINT_ERROR_INVALID_DATA, "" },
        /* 71*/ { "[254]abcdefghijklmnopqrst", 0, "254abcdefghijklmnopqrst" },
        /* 72*/ { "[254]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /* 73*/ { "[255]131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /* 74*/ { "[255]1313131313134", 0, "2551313131313134" },
        /* 75*/ { "[255]1313131313134123456789012", 0, "2551313131313134123456789012" },
        /* 76*/ { "[255]13131313131341234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /* 77*/ { "[256]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 78*/ { "[259]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 79*/ { "[26]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 80*/ { "[27]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 81*/ { "[28]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 82*/ { "[29]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 83*/ { "[30]12345678", 0, "3012345678" },
        /* 84*/ { "[30]123456789", ZINT_ERROR_INVALID_DATA, "" },
        /* 85*/ { "[3000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 86*/ { "[31]123456", ZINT_ERROR_INVALID_DATA, "" },
        /* 87*/ { "[3100]123456", 0, "3100123456" },
        /* 88*/ { "[3100]12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 89*/ { "[3101]123456", 0, "3101123456" },
        /* 90*/ { "[3101]12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 91*/ { "[3109]123456", 0, "3109123456" },
        /* 92*/ { "[3110]123456", 0, "3110123456" },
        /* 93*/ { "[3110]12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 94*/ { "[3119]123456", 0, "3119123456" },
        /* 95*/ { "[3120]123456", 0, "3120123456" },
        /* 96*/ { "[3120]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 97*/ { "[3129]123456", 0, "3129123456" },
        /* 98*/ { "[3130]123456", 0, "3130123456" },
        /* 99*/ { "[3130]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*100*/ { "[3139]123456", 0, "3139123456" },
        /*101*/ { "[3140]123456", 0, "3140123456" },
        /*102*/ { "[3140]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*103*/ { "[3149]123456", 0, "3149123456" },
        /*104*/ { "[3150]123456", 0, "3150123456" },
        /*105*/ { "[3150]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*106*/ { "[3159]123456", 0, "3159123456" },
        /*107*/ { "[3160]123456", 0, "3160123456" },
        /*108*/ { "[3160]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*109*/ { "[3169]123456", 0, "3169123456" },
        /*110*/ { "[3170]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*111*/ { "[3180]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*112*/ { "[3190]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*113*/ { "[32]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*114*/ { "[3200]123456", 0, "3200123456" },
        /*115*/ { "[3200]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*116*/ { "[3210]123456", 0, "3210123456" },
        /*117*/ { "[3210]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*118*/ { "[3220]123456", 0, "3220123456" },
        /*119*/ { "[3220]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*120*/ { "[3230]123456", 0, "3230123456" },
        /*121*/ { "[3240]123456", 0, "3240123456" },
        /*122*/ { "[3250]123456", 0, "3250123456" },
        /*123*/ { "[3260]123456", 0, "3260123456" },
        /*124*/ { "[3270]123456", 0, "3270123456" },
        /*125*/ { "[3280]123456", 0, "3280123456" },
        /*126*/ { "[3290]123456", 0, "3290123456" },
        /*127*/ { "[3290]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*128*/ { "[3299]123456", 0, "3299123456" },
        /*129*/ { "[33]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*130*/ { "[3300]123456", 0, "3300123456" },
        /*131*/ { "[3300]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*132*/ { "[3310]123456", 0, "3310123456" },
        /*133*/ { "[3310]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*134*/ { "[3320]123456", 0, "3320123456" },
        /*135*/ { "[3330]123456", 0, "3330123456" },
        /*136*/ { "[3340]123456", 0, "3340123456" },
        /*137*/ { "[3350]123456", 0, "3350123456" },
        /*138*/ { "[3360]123456", 0, "3360123456" },
        /*139*/ { "[3370]123456", 0, "3370123456" },
        /*140*/ { "[3370]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*141*/ { "[3379]123456", 0, "3379123456" },
        /*142*/ { "[3380]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*143*/ { "[3390]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*144*/ { "[34]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*145*/ { "[3400]123456", 0, "3400123456" },
        /*146*/ { "[3400]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*147*/ { "[3410]123456", 0, "3410123456" },
        /*148*/ { "[3410]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*149*/ { "[3420]123456", 0, "3420123456" },
        /*150*/ { "[3430]123456", 0, "3430123456" },
        /*151*/ { "[3440]123456", 0, "3440123456" },
        /*152*/ { "[3450]123456", 0, "3450123456" },
        /*153*/ { "[3460]123456", 0, "3460123456" },
        /*154*/ { "[3470]123456", 0, "3470123456" },
        /*155*/ { "[3480]123456", 0, "3480123456" },
        /*156*/ { "[3490]123456", 0, "3490123456" },
        /*157*/ { "[3490]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*158*/ { "[3499]123456", 0, "3499123456" },
        /*159*/ { "[35]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*160*/ { "[3500]123456", 0, "3500123456" },
        /*161*/ { "[3500]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*162*/ { "[3510]123456", 0, "3510123456", },
        /*163*/ { "[3510]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*164*/ { "[3520]123456", 0, "3520123456", },
        /*165*/ { "[3530]123456", 0, "3530123456", },
        /*166*/ { "[3540]123456", 0, "3540123456", },
        /*167*/ { "[3550]123456", 0, "3550123456", },
        /*168*/ { "[3560]123456", 0, "3560123456", },
        /*169*/ { "[3570]123456", 0, "3570123456", },
        /*170*/ { "[3579]123456", 0, "3579123456" },
        /*171*/ { "[3580]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*172*/ { "[3590]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*173*/ { "[36]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*174*/ { "[3600]123456", 0, "3600123456" },
        /*175*/ { "[3600]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*176*/ { "[3610]123456", 0, "3610123456" },
        /*177*/ { "[3610]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*178*/ { "[3620]123456", 0, "3620123456", },
        /*179*/ { "[3630]123456", 0, "3630123456", },
        /*180*/ { "[3640]123456", 0, "3640123456", },
        /*181*/ { "[3650]123456", 0, "3650123456", },
        /*182*/ { "[3660]123456", 0, "3660123456", },
        /*183*/ { "[3670]123456", 0, "3670123456", },
        /*184*/ { "[3680]123456", 0, "3680123456", },
        /*185*/ { "[3680]123456", 0, "3680123456", },
        /*186*/ { "[3680]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*187*/ { "[3690]123456", 0, "3690123456" },
        /*188*/ { "[3699]123456", 0, "3699123456" },
        /*189*/ { "[37]12345678", 0, "3712345678" },
        /*190*/ { "[37]123456789", ZINT_ERROR_INVALID_DATA, "" },
        /*191*/ { "[3700]12345678", ZINT_ERROR_INVALID_DATA, "" },
        /*192*/ { "[38]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*193*/ { "[3800]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*194*/ { "[39]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*195*/ { "[3900]123456789012345", 0, "3900123456789012345" },
        /*196*/ { "[3900]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*197*/ { "[3909]123456789012345", 0, "3909123456789012345" },
        /*198*/ { "[3910]123123456789012345", 0, "3910123123456789012345" },
        /*199*/ { "[3910]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*200*/ { "[3910]123", ZINT_ERROR_INVALID_DATA, "" },
        /*201*/ { "[3920]123456789012345", 0, "3920123456789012345" },
        /*202*/ { "[3920]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*203*/ { "[3930]123123456789012345", 0, "3930123123456789012345" },
        /*204*/ { "[3930]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*205*/ { "[3930]123", ZINT_ERROR_INVALID_DATA, "" },
        /*206*/ { "[3940]1234", 0, "39401234" },
        /*207*/ { "[3940]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*208*/ { "[3940]123", ZINT_ERROR_INVALID_DATA, "" },
        /*209*/ { "[3949]1234", 0, "39491234" },
        /*210*/ { "[3950]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*211*/ { "[3960]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*212*/ { "[3970]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*213*/ { "[3980]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*214*/ { "[3999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*215*/ { "[40]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*216*/ { "[400]123456789012345678901234567890", 0, "400123456789012345678901234567890" },
        /*217*/ { "[400]1234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*218*/ { "[401]1234abcdefghijklmnopqrstuvwxyz", 0, "4011234abcdefghijklmnopqrstuvwxyz" },
        /*219*/ { "[401]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*220*/ { "[402]13131313131313132", 0, "40213131313131313132" },
        /*221*/ { "[402]1313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*222*/ { "[403]abcdefghijklmnopqrstuvwxyz1234", 0, "403abcdefghijklmnopqrstuvwxyz1234" },
        /*223*/ { "[403]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*224*/ { "[404]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*225*/ { "[409]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*226*/ { "[410]3898765432108", 0, "4103898765432108" },
        /*227*/ { "[410]12345678901234", ZINT_ERROR_INVALID_DATA, "" },
        /*228*/ { "[411]1313131313134", 0, "4111313131313134" },
        /*229*/ { "[411]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*230*/ { "[412]1313131313134", 0, "4121313131313134" },
        /*231*/ { "[412]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*232*/ { "[413]1313131313134", 0, "4131313131313134" },
        /*233*/ { "[413]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*234*/ { "[414]1313131313134", 0, "4141313131313134" },
        /*235*/ { "[414]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*236*/ { "[415]1313131313134", 0, "4151313131313134" },
        /*237*/ { "[415]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*238*/ { "[416]1313131313134", 0, "4161313131313134" },
        /*239*/ { "[416]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*240*/ { "[417]1313131313134", 0, "4171313131313134" },
        /*241*/ { "[417]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*242*/ { "[418]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*243*/ { "[419]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*244*/ { "[420]abcdefghijklmnopqrst", 0, "420abcdefghijklmnopqrst" },
        /*245*/ { "[420]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*246*/ { "[421]123abcdefghi", 0, "421123abcdefghi" },
        /*247*/ { "[421]123abcdefghij", ZINT_ERROR_INVALID_DATA, "" },
        /*248*/ { "[422]123", 0, "422123" },
        /*249*/ { "[422]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*250*/ { "[422]12", ZINT_ERROR_INVALID_DATA, "" },
        /*251*/ { "[423]123123123123123", 0, "423123123123123123" },
        /*252*/ { "[423]1231231231231231", ZINT_ERROR_INVALID_DATA, "" },
        /*253*/ { "[423]12", ZINT_ERROR_INVALID_DATA, "" },
        /*254*/ { "[424]123", 0, "424123" },
        /*255*/ { "[424]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*256*/ { "[424]12", ZINT_ERROR_INVALID_DATA, "" },
        /*257*/ { "[425]123123123123123", 0, "425123123123123123" },
        /*258*/ { "[425]1231231231231231", ZINT_ERROR_INVALID_DATA, "" },
        /*259*/ { "[425]12", ZINT_ERROR_INVALID_DATA, "" },
        /*260*/ { "[426]123", 0, "426123" },
        /*261*/ { "[426]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*262*/ { "[426]12", ZINT_ERROR_INVALID_DATA, "" },
        /*263*/ { "[427]abc", 0, "427abc" },
        /*264*/ { "[427]abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*265*/ { "[428]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*266*/ { "[429]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*267*/ { "[430]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*268*/ { "[499]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*269*/ { "[50]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*270*/ { "[500]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*271*/ { "[5000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*272*/ { "[51]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*273*/ { "[59]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*274*/ { "[60]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*275*/ { "[600]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*276*/ { "[6000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*277*/ { "[61]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*278*/ { "[69]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*279*/ { "[70]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*280*/ { "[700]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*281*/ { "[7000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*282*/ { "[7001]1234567890123", 0, "70011234567890123" },
        /*283*/ { "[7001]123456789012", ZINT_ERROR_INVALID_DATA, "" },
        /*284*/ { "[7002]abcdefghijklmnopqrstuvwxyz1234", 0, "7002abcdefghijklmnopqrstuvwxyz1234" },
        /*285*/ { "[7002]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*286*/ { "[7003]1212121212", 0, "70031212121212" },
        /*287*/ { "[7003]121212121", ZINT_ERROR_INVALID_DATA, "" },
        /*288*/ { "[7004]1234", 0, "70041234" },
        /*289*/ { "[7004]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*290*/ { "[7005]abcdefghijkl", 0, "7005abcdefghijkl" },
        /*291*/ { "[7005]abcdefghijklm", ZINT_ERROR_INVALID_DATA, "" },
        /*292*/ { "[7006]200132", 0, "7006200132" },
        /*293*/ { "[7006]2001320", ZINT_ERROR_INVALID_DATA, "" },
        /*294*/ { "[7007]010101121212", 0, "7007010101121212" },
        /*295*/ { "[7007]01010112121", ZINT_ERROR_INVALID_DATA, "" },
        /*296*/ { "[7007]121212", 0, "7007121212" },
        /*297*/ { "[7007]12121", ZINT_ERROR_INVALID_DATA, "" },
        /*298*/ { "[7007]1212121", ZINT_ERROR_INVALID_DATA, "" },
        /*299*/ { "[7008]abc", 0, "7008abc" },
        /*300*/ { "[7008]abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*301*/ { "[7009]abcdefghij", 0, "7009abcdefghij" },
        /*302*/ { "[7009]abcdefghijk", ZINT_ERROR_INVALID_DATA, "" },
        /*303*/ { "[7010]01", 0, "701001" },
        /*304*/ { "[7010]1", 0, "70101" },
        /*305*/ { "[7010]012", ZINT_ERROR_INVALID_DATA, "" },
        /*306*/ { "[7011]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*307*/ { "[7012]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*308*/ { "[7019]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*309*/ { "[7020]abcdefghijklmnopqrst", 0, "7020abcdefghijklmnopqrst" },
        /*310*/ { "[7020]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*311*/ { "[7021]abcdefghijklmnopqrst", 0, "7021abcdefghijklmnopqrst" },
        /*312*/ { "[7021]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*313*/ { "[7022]abcdefghijklmnopqrst", 0, "7022abcdefghijklmnopqrst" },
        /*314*/ { "[7022]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*315*/ { "[7023]1234abcdefghijklmnopqrstuvwxyz", 0, "70231234abcdefghijklmnopqrstuvwxyz" },
        /*316*/ { "[7023]1234abcdefghijklmnopqrstuvwxyza", ZINT_ERROR_INVALID_DATA, "" },
        /*317*/ { "[7024]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*318*/ { "[7025]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*319*/ { "[7029]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*320*/ { "[7030]123abcdefghijklmnopqrstuvwxyza", 0, "7030123abcdefghijklmnopqrstuvwxyza" },
        /*321*/ { "[7030]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*322*/ { "[7031]123abcdefghijklmnopqrstuvwxyza", 0, "7031123abcdefghijklmnopqrstuvwxyza" },
        /*323*/ { "[7031]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*324*/ { "[7032]123abcdefghijklmnopqrstuvwxyza", 0, "7032123abcdefghijklmnopqrstuvwxyza" },
        /*325*/ { "[7033]123abcdefghijklmnopqrstuvwxyza", 0, "7033123abcdefghijklmnopqrstuvwxyza" },
        /*326*/ { "[7034]123abcdefghijklmnopqrstuvwxyza", 0, "7034123abcdefghijklmnopqrstuvwxyza" },
        /*327*/ { "[7035]123abcdefghijklmnopqrstuvwxyza", 0, "7035123abcdefghijklmnopqrstuvwxyza" },
        /*328*/ { "[7036]123abcdefghijklmnopqrstuvwxyza", 0, "7036123abcdefghijklmnopqrstuvwxyza" },
        /*329*/ { "[7037]123abcdefghijklmnopqrstuvwxyza", 0, "7037123abcdefghijklmnopqrstuvwxyza" },
        /*330*/ { "[7038]123abcdefghijklmnopqrstuvwxyza", 0, "7038123abcdefghijklmnopqrstuvwxyza" },
        /*331*/ { "[7039]123abcdefghijklmnopqrstuvwxyza", 0, "7039123abcdefghijklmnopqrstuvwxyza" },
        /*332*/ { "[7039]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*333*/ { "[7040]1abc", 0, "70401abc" },
        /*334*/ { "[7040]1ab", ZINT_ERROR_INVALID_DATA, "" },
        /*335*/ { "[7040]1abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*336*/ { "[7041]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*337*/ { "[7042]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*338*/ { "[7050]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*339*/ { "[7090]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*340*/ { "[7099]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*341*/ { "[71]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*342*/ { "[710]abcdefghijklmnopqrst", 0, "710abcdefghijklmnopqrst" },
        /*343*/ { "[710]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*344*/ { "[711]abcdefghijklmnopqrst", 0, "711abcdefghijklmnopqrst" },
        /*345*/ { "[711]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*346*/ { "[712]abcdefghijklmnopqrst", 0, "712abcdefghijklmnopqrst" },
        /*347*/ { "[712]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*348*/ { "[713]abcdefghijklmnopqrst", 0, "713abcdefghijklmnopqrst" },
        /*349*/ { "[713]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*350*/ { "[714]abcdefghijklmnopqrst", 0, "714abcdefghijklmnopqrst" },
        /*351*/ { "[714]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*352*/ { "[715]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*353*/ { "[716]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*354*/ { "[719]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*355*/ { "[7100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*356*/ { "[72]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*357*/ { "[7200]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*358*/ { "[7210]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*359*/ { "[7220]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*360*/ { "[7230]EMabcdefghijklmnopqrstuvwxyzab", 0, "7230EMabcdefghijklmnopqrstuvwxyzab" },
        /*361*/ { "[7230]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*362*/ { "[7230]EM", ZINT_ERROR_INVALID_DATA, "" },
        /*363*/ { "[7231]EMabcdefghijklmnopqrstuvwxyzab", 0, "7231EMabcdefghijklmnopqrstuvwxyzab" },
        /*364*/ { "[7232]EMabcdefghijklmnopqrstuvwxyzab", 0, "7232EMabcdefghijklmnopqrstuvwxyzab" },
        /*365*/ { "[7233]EMabcdefghijklmnopqrstuvwxyzab", 0, "7233EMabcdefghijklmnopqrstuvwxyzab" },
        /*366*/ { "[7234]EMabcdefghijklmnopqrstuvwxyzab", 0, "7234EMabcdefghijklmnopqrstuvwxyzab" },
        /*367*/ { "[7235]EMabcdefghijklmnopqrstuvwxyzab", 0, "7235EMabcdefghijklmnopqrstuvwxyzab" },
        /*368*/ { "[7236]EMabcdefghijklmnopqrstuvwxyzab", 0, "7236EMabcdefghijklmnopqrstuvwxyzab" },
        /*369*/ { "[7237]EMabcdefghijklmnopqrstuvwxyzab", 0, "7237EMabcdefghijklmnopqrstuvwxyzab" },
        /*370*/ { "[7238]EMabcdefghijklmnopqrstuvwxyzab", 0, "7238EMabcdefghijklmnopqrstuvwxyzab" },
        /*371*/ { "[7239]EMabcdefghijklmnopqrstuvwxyzab", 0, "7239EMabcdefghijklmnopqrstuvwxyzab" },
        /*372*/ { "[7239]E", ZINT_ERROR_INVALID_DATA, "" },
        /*373*/ { "[7240]abcdefghijklmnopqrst", 0, "7240abcdefghijklmnopqrst" },
        /*374*/ { "[7240]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*375*/ { "[7241]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*376*/ { "[7249]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*377*/ { "[7250]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*378*/ { "[7299]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*379*/ { "[73]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*380*/ { "[7300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*381*/ { "[74]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*382*/ { "[7400]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*383*/ { "[79]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*384*/ { "[7900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*385*/ { "[7999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*386*/ { "[80]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*387*/ { "[800]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*388*/ { "[8000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*389*/ { "[8001]12345678901234", 0, "800112345678901234" },
        /*390*/ { "[8001]1234123456789012345", ZINT_ERROR_INVALID_DATA, "" },
        /*391*/ { "[8002]abcdefghijklmnopqrst", 0, "8002abcdefghijklmnopqrst" },
        /*392*/ { "[8002]abcdefghijklmnopqrstuv", ZINT_ERROR_INVALID_DATA, "" },
        /*393*/ { "[8003]01234567890123abcdefghijklmnop", 0, "800301234567890123abcdefghijklmnop" },
        /*394*/ { "[8004]abcdefghijklmnopqrstuvwxyz1234", 0, "8004abcdefghijklmnopqrstuvwxyz1234" },
        /*395*/ { "[8004]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*396*/ { "[8005]123456", 0, "8005123456" },
        /*397*/ { "[8005]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*398*/ { "[8005]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*399*/ { "[8006]123456789012341212", 0, "8006123456789012341212" },
        /*400*/ { "[8006]12345678901234121", ZINT_ERROR_INVALID_DATA, "" },
        /*401*/ { "[8006]1234567890123412123", ZINT_ERROR_INVALID_DATA, "" },
        /*402*/ { "[8007]abcdefghijklmnopqrstuvwxyz12345678", 0, "8007abcdefghijklmnopqrstuvwxyz12345678" },
        /*403*/ { "[8007]abcdefghijklmnopqrstuvwxyz123456789", ZINT_ERROR_INVALID_DATA, "" },
        /*404*/ { "[8008]123456121212", 0, "8008123456121212" },
        /*405*/ { "[8008]1234561212", 0, "80081234561212" },
        /*406*/ { "[8008]12345612", 0, "800812345612" },
        /*407*/ { "[8008]1234561", ZINT_ERROR_INVALID_DATA, "" },
        /*408*/ { "[8008]123456121", ZINT_ERROR_INVALID_DATA, "" },
        /*409*/ { "[8008]12345612121", ZINT_ERROR_INVALID_DATA, "" },
        /*410*/ { "[8008]1234561212123", ZINT_ERROR_INVALID_DATA, "" },
        /*411*/ { "[8009]12345678901234567890123456789012345678901234567890", 0, "800912345678901234567890123456789012345678901234567890" },
        /*412*/ { "[8009]123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*413*/ { "[8010]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*414*/ { "[8011]123456789012", 0, "8011123456789012" },
        /*415*/ { "[8011]1234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /*416*/ { "[8012]abcdefghijklmnopqrst", 0, "8012abcdefghijklmnopqrst" },
        /*417*/ { "[8012]abcdefghijklmnopqrstuv", ZINT_ERROR_INVALID_DATA, "" },
        /*418*/ { "[8013]1234abcdefghijklmnopqrstuvwxyz", 0, "80131234abcdefghijklmnopqrstuvwxyz" },
        /*419*/ { "[8013]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*420*/ { "[8014]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*421*/ { "[8016]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*422*/ { "[8017]313131313131313139", 0, "8017313131313131313139" },
        /*423*/ { "[8017]31313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*424*/ { "[8017]3131313131313131390", ZINT_ERROR_INVALID_DATA, "" },
        /*425*/ { "[8018]313131313131313139", 0, "8018313131313131313139" },
        /*426*/ { "[8018]31313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*427*/ { "[8018]3131313131313131390", ZINT_ERROR_INVALID_DATA, "" },
        /*428*/ { "[8019]1234567890", 0, "80191234567890" },
        /*429*/ { "[8019]12345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*430*/ { "[8020]abcdefghijklmnopqrstuvwxy", 0, "8020abcdefghijklmnopqrstuvwxy" },
        /*431*/ { "[8020]abcdefghijklmnopqrstuvwxyz", ZINT_ERROR_INVALID_DATA, "" },
        /*432*/ { "[8021]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*433*/ { "[8025]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*434*/ { "[8026]123456789012341212", 0, "8026123456789012341212" },
        /*435*/ { "[8026]1234567890123451212", ZINT_ERROR_INVALID_DATA, "" },
        /*436*/ { "[8026]12345678901234512", ZINT_ERROR_INVALID_DATA, "" },
        /*437*/ { "[8027]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*438*/ { "[8030]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*439*/ { "[8040]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*440*/ { "[8090]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*441*/ { "[8099]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*442*/ { "[81]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*443*/ { "[8100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*444*/ { "[8109]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*445*/ { "[8110]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "81101234567890123456789012345678901234567890123456789012345678901234567890" },
        /*446*/ { "[8110]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*447*/ { "[8111]1234", 0, "81111234" },
        /*448*/ { "[8111]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*449*/ { "[8111]123", ZINT_ERROR_INVALID_DATA, "" },
        /*450*/ { "[8112]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "81121234567890123456789012345678901234567890123456789012345678901234567890" },
        /*451*/ { "[8112]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*452*/ { "[8113]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*453*/ { "[8120]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*454*/ { "[8190]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*455*/ { "[8199]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*456*/ { "[82]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*457*/ { "[8200]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "82001234567890123456789012345678901234567890123456789012345678901234567890" },
        /*458*/ { "[8201]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*459*/ { "[8210]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*460*/ { "[8290]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*461*/ { "[8299]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*462*/ { "[83]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*463*/ { "[8300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*464*/ { "[89]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*465*/ { "[8900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*466*/ { "[90]abcdefghijklmnopqrstuvwxyz1234", 0, "90abcdefghijklmnopqrstuvwxyz1234" },
        /*467*/ { "[9000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*468*/ { "[90]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*469*/ { "[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "91123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*470*/ { "[91]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*471*/ { "[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "92123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*472*/ { "[92]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*473*/ { "[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "93123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*474*/ { "[93]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*475*/ { "[94]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "94123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*476*/ { "[94]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*477*/ { "[98]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "98123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*478*/ { "[98]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*479*/ { "[99]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "99123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*480*/ { "[99]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*481*/ { "[9900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*482*/ { "[9999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*483*/ { "[01]12345678901234[7006]200101", 0, "01123456789012347006200101" },
        /*484*/ { "[3900]1234567890[01]12345678901234", 0, "39001234567890[0112345678901234" },
        /*485*/ { "[253]12345678901234[3901]12345678901234[20]12", 0, "25312345678901234[390112345678901234[2012" },
        /*486*/ { "[253]12345678901234[01]12345678901234[3901]12345678901234[20]12", 0, "25312345678901234[0112345678901234390112345678901234[2012" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char reduced[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = strlen(data[i].data);

        ret = gs1_verify(symbol, data[i].data, length, reduced);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (length %d \"%s\") %s\n", i, ret, data[i].ret, length, data[i].data, symbol->errtxt);

        if (ret == 0) {
            assert_zero(strcmp(reduced, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, reduced, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input_mode(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        unsigned char *composite;
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
    int data_size = sizeof(data) / sizeof(struct item);

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

        ret = ZBarcode_Encode(symbol, text, length);
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
        { "test_gs1_verify", test_gs1_verify, 1, 0, 0 },
        { "test_input_mode", test_input_mode, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
