/*
    libzint - the open source barcode library
    Copyright (C) 2008-2019 Robin Stuart <rstuart114@gmail.com>

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

//#define TEST_GS1_REDUCE_GENERATE_EXPECTED

/*
 * Check that EAN128 and RSS_EXP based symbologies reduce GS1 data
 */
static void test_gs1_reduce(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        unsigned char* data;
        unsigned char* composite;
        int ret;

        char* comment;
        unsigned char* expected;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_EAN128, -1, "12345678901234", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /* 1*/ { BARCODE_EAN128, -1, "[01]12345678901234", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110010001011000111010111101100011101011"
               },
        /* 2*/ { BARCODE_EAN128, GS1_MODE, "[01]12345678901234", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110010001011000111010111101100011101011"
               },
        /* 3*/ { BARCODE_EAN128, UNICODE_MODE, "[01]12345678901234", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110010001011000111010111101100011101011"
               },
        /* 4*/ { BARCODE_EAN128_CC, -1, "[01]12345678901234", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110111010011100010100001011010000110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001000101100011101011110100101111001100011101011"
               },
        /* 5*/ { BARCODE_EAN128_CC, GS1_MODE, "[01]12345678901234", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110111010011100010100001011010000110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001000101100011101011110100101111001100011101011"
               },
        /* 6*/ { BARCODE_EAN128_CC, UNICODE_MODE, "[01]12345678901234", "[21]1234", 0, "Input mode ignored",
                    "0000000000000000000001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010000000000000000000000000"
                    "0000000000000000000001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010000000000000000000000000"
                    "0000000000000000000001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010000000000000000000000000"
                    "0010110001100001010001001100100110100110001101110100111000111010010011110101100100001001010011000110111010011100010100001011010000110011100010100"
                    "1101001110011110101110110011011001011001110010001011000111000101101100001010011011110110101100111001000101100011101011110100101111001100011101011"
               },
        /* 7*/ { BARCODE_EAN14, -1, "1234567890123", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /* 8*/ { BARCODE_EAN14, GS1_MODE, "1234567890123", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /* 9*/ { BARCODE_EAN14, UNICODE_MODE, "1234567890123", "", 0, "Input mode ignored",
                    "11010011100111101011101100110110010110011100100010110001110001011011000010100110111101101011001110011011000110100001100101100011101011"
               },
        /*10*/ { BARCODE_NVE18, -1, "12345678901234567", "", 0, "Input mode ignored",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*11*/ { BARCODE_NVE18, GS1_MODE, "12345678901234567", "", 0, "Input mode ignored",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*12*/ { BARCODE_NVE18, UNICODE_MODE, "12345678901234567", "", 0, "Input mode ignored",
                    "110100111001111010111011011001100101100111001000101100011100010110110000101001101111011010110011100100010110001110001011011000010010101101110001100011101011"
               },
        /*13*/ { BARCODE_RSS_EXP, -1, "2012", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*14*/ { BARCODE_RSS_EXP, -1, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*15*/ { BARCODE_RSS_EXP, GS1_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*16*/ { BARCODE_RSS_EXP, UNICODE_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*17*/ { BARCODE_RSS_EXP_CC, -1, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001100100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*18*/ { BARCODE_RSS_EXP_CC, GS1_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001100100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*19*/ { BARCODE_RSS_EXP_CC, UNICODE_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001100100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*20*/ { BARCODE_RSS_EXPSTACK, -1, "12", "", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*21*/ { BARCODE_RSS_EXPSTACK, -1, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*22*/ { BARCODE_RSS_EXPSTACK, GS1_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*23*/ { BARCODE_RSS_EXPSTACK, UNICODE_MODE, "[20]12", "", 0, "Input mode ignored",
                    "010010000010000101101111111100001010000010000110010101111100101110001011110000000010101111100001011101"
               },
        /*24*/ { BARCODE_RSS_EXPSTACK_CC, -1, "12", "[21]1234", ZINT_ERROR_INVALID_DATA, "GS1 data required", "" },
        /*25*/ { BARCODE_RSS_EXPSTACK_CC, -1, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001100100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*26*/ { BARCODE_RSS_EXPSTACK_CC, GS1_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001100100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
        /*27*/ { BARCODE_RSS_EXPSTACK_CC, UNICODE_MODE, "[20]12", "[21]1234", 0, "Input mode ignored",
                    "001101101110110100001000001101001100111011000010011101001100001010001100010010011011000000110110001010"
                    "001101101100111110100010011001101011100100000010011001001001111001011110011101011001000000110010001010"
                    "001101101000101111100110000101001111010000001010011001101011101110011110010011110110000110111010001010"
                    "000001111111010110010000000010100100111001100001011010000011010001100100001010101001010000011110100000"
                    "010010000000101001101111111100001011000110011110100101111100101110001011110000000010101111100001011101"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char* text;

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].input_mode != -1) {
            symbol->input_mode = data[i].input_mode;
        }

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int length = strlen(text);

        ret = ZBarcode_Encode(symbol, text, length);

        #ifdef TEST_GS1_REDUCE_GENERATE_EXPECTED
        if (data[i].ret == 0) {
            printf("        /*%2d*/ { %s, %s, \"%s\", \"%s\", %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].data, data[i].composite, data[i].ret, data[i].comment);
            testUtilModulesDump(symbol, "                    ", "\n");
            printf("               },\n");
        } else {
            printf("        /*%2d*/ { %s, %s, \"%s\", \"%s\", %s, \"%s\", \"\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].data, data[i].composite, testUtilErrorName(data[i].ret), data[i].comment);
        }
        #else
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d %s\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret == 0) {
            int width, row;
            ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
            assert_zero(ret, "i:%d %s testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, width, row, data[i].data);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_hrt(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        unsigned char* composite;

        unsigned char* expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_EAN128, "[01]12345678901234[20]12", "", "(01)12345678901234(20)12" },
        /* 1*/ { BARCODE_EAN128_CC, "[01]12345678901234[20]12", "[21]12345", "(01)12345678901234(20)12" },
        /* 2*/ { BARCODE_RSS_EXP, "[01]12345678901234[20]12", "", "(01)12345678901234(20)12" },
        /* 3*/ { BARCODE_RSS_EXP_CC, "[01]12345678901234", "[21]12345", "(01)12345678901234" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char* text;

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;

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

static void test_gs1_verify(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int ret;
        char* expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "", ZINT_ERROR_INVALID_DATA, "" },
        /*  1*/ { "[", ZINT_ERROR_INVALID_DATA, "" },
        /*  2*/ { "[]12", ZINT_ERROR_INVALID_DATA, "" },
        /*  3*/ { "[12345]12", ZINT_ERROR_INVALID_DATA, "" },
        /*  4*/ { "[9999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*  5*/ { "[[01]]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*  6*/ { "[20]1\n", ZINT_ERROR_INVALID_DATA, "" },
        /*  7*/ { "[20]1\x80", ZINT_ERROR_INVALID_DATA, "" },
        /*  8*/ { "[00]123456789012345678", 0, "00123456789012345678" },
        /*  9*/ { "[00]12345678901234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 10*/ { "[01]12345678901234", 0, "0112345678901234" },
        /* 11*/ { "[01]123456789012345", ZINT_ERROR_INVALID_DATA, "" },
        /* 12*/ { "[02]12345678901234", 0, "0212345678901234" },
        /* 13*/ { "[02]1234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /* 14*/ { "[03]12345678901234", ZINT_ERROR_INVALID_DATA, "" },
        /* 15*/ { "[04]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 16*/ { "[05]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 17*/ { "[06]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 18*/ { "[07]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 19*/ { "[08]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 20*/ { "[09]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 21*/ { "[10]ABCD123456", 0, "10ABCD123456" },
        /* 22*/ { "[10]123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /* 23*/ { "[100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 24*/ { "[11]990102", 0, "11990102" },
        /* 25*/ { "[11]9901023", ZINT_ERROR_INVALID_DATA, "" },
        /* 26*/ { "[12]000100", 0, "12000100" },
        /* 27*/ { "[12]00010", ZINT_ERROR_INVALID_DATA, "" },
        /* 28*/ { "[13]991301", 0, "13991301" },
        /* 29*/ { "[13]9913011", ZINT_ERROR_INVALID_DATA, "" },
        /* 30*/ { "[14]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 31*/ { "[15]021231", 0, "15021231" },
        /* 32*/ { "[15]02123", ZINT_ERROR_INVALID_DATA, "" },
        /* 33*/ { "[16]000000", 0, "16000000" },
        /* 34*/ { "[16]00000", ZINT_ERROR_INVALID_DATA, "" },
        /* 35*/ { "[17]010200", 0, "17010200" },
        /* 36*/ { "[17]0102000", ZINT_ERROR_INVALID_DATA, "" },
        /* 37*/ { "[18]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 38*/ { "[19]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 39*/ { "[20]12", 0, "2012" },
        /* 40*/ { "[20]1", ZINT_ERROR_INVALID_DATA, "" },
        /* 41*/ { "[200]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 42*/ { "[21]A12345678", 0, "21A12345678" },
        /* 43*/ { "[21]123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /* 44*/ { "[22]abcdefghijklmnopqrst", 0, "22abcdefghijklmnopqrst" },
        /* 45*/ { "[22]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /* 46*/ { "[23]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 47*/ { "[235]1abcdefghijklmnopqrstuvwxyz0", 0, "2351abcdefghijklmnopqrstuvwxyz0" },
        /* 48*/ { "[235]1abcdefghijklmnopqrstuvwxyz01", ZINT_ERROR_INVALID_DATA, "" },
        /* 49*/ { "[24]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 50*/ { "[240]abcdefghijklmnopqrstuvwxyz1234", 0, "240abcdefghijklmnopqrstuvwxyz1234" },
        /* 51*/ { "[240]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 52*/ { "[241]abcdefghijklmnopqrstuvwxyz1234", 0, "241abcdefghijklmnopqrstuvwxyz1234" },
        /* 53*/ { "[241]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 54*/ { "[242]12345", 0, "24212345" },
        /* 55*/ { "[242]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 56*/ { "[243]abcdefghijklmnopqrst", 0, "243abcdefghijklmnopqrst" },
        /* 57*/ { "[243]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /* 58*/ { "[244]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 59*/ { "[249]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 60*/ { "[25]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 61*/ { "[250]abcdefghijklmnopqrstuvwxyz1234", 0, "250abcdefghijklmnopqrstuvwxyz1234" },
        /* 62*/ { "[250]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 63*/ { "[251]abcdefghijklmnopqrstuvwxyz1234", 0, "251abcdefghijklmnopqrstuvwxyz1234" },
        /* 64*/ { "[251]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 65*/ { "[252]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 66*/ { "[253]131313131313412345678901234567", 0, "253131313131313412345678901234567" },
        /* 67*/ { "[253]13131313131341234567890123345678", ZINT_ERROR_INVALID_DATA, "" },
        /* 68*/ { "[254]abcdefghijklmnopqrst", 0, "254abcdefghijklmnopqrst" },
        /* 69*/ { "[254]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /* 70*/ { "[255]1313131313134123456789012", 0, "2551313131313134123456789012" },
        /* 71*/ { "[255]13131313131341234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /* 72*/ { "[256]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 73*/ { "[259]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 74*/ { "[26]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 75*/ { "[27]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 76*/ { "[28]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 77*/ { "[29]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 78*/ { "[30]12345678", 0, "3012345678" },
        /* 79*/ { "[30]123456789", ZINT_ERROR_INVALID_DATA, "" },
        /* 80*/ { "[3000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /* 81*/ { "[31]123456", ZINT_ERROR_INVALID_DATA, "" },
        /* 82*/ { "[3100]123456", 0, "3100123456" },
        /* 83*/ { "[3100]12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 84*/ { "[3101]123456", 0, "3101123456" },
        /* 85*/ { "[3101]12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 86*/ { "[3109]123456", 0, "3109123456" },
        /* 87*/ { "[3110]123456", 0, "3110123456" },
        /* 88*/ { "[3110]12345", ZINT_ERROR_INVALID_DATA, "" },
        /* 89*/ { "[3119]123456", 0, "3119123456" },
        /* 90*/ { "[3120]123456", 0, "3120123456" },
        /* 91*/ { "[3120]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 92*/ { "[3129]123456", 0, "3129123456" },
        /* 93*/ { "[3130]123456", 0, "3130123456" },
        /* 94*/ { "[3130]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 95*/ { "[3139]123456", 0, "3139123456" },
        /* 96*/ { "[3140]123456", 0, "3140123456" },
        /* 97*/ { "[3140]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /* 98*/ { "[3149]123456", 0, "3149123456" },
        /* 99*/ { "[3150]123456", 0, "3150123456" },
        /*100*/ { "[3150]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*101*/ { "[3159]123456", 0, "3159123456" },
        /*102*/ { "[3160]123456", 0, "3160123456" },
        /*103*/ { "[3160]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*104*/ { "[3169]123456", 0, "3169123456" },
        /*105*/ { "[3170]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*106*/ { "[3180]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*107*/ { "[3190]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*108*/ { "[32]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*109*/ { "[3200]123456", 0, "3200123456" },
        /*110*/ { "[3200]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*111*/ { "[3210]123456", 0, "3210123456" },
        /*112*/ { "[3210]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*113*/ { "[3220]123456", 0, "3220123456" },
        /*114*/ { "[3220]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*115*/ { "[3230]123456", 0, "3230123456" },
        /*116*/ { "[3240]123456", 0, "3240123456" },
        /*117*/ { "[3250]123456", 0, "3250123456" },
        /*118*/ { "[3260]123456", 0, "3260123456" },
        /*119*/ { "[3270]123456", 0, "3270123456" },
        /*120*/ { "[3280]123456", 0, "3280123456" },
        /*121*/ { "[3290]123456", 0, "3290123456" },
        /*122*/ { "[3290]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*123*/ { "[3299]123456", 0, "3299123456" },
        /*124*/ { "[33]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*125*/ { "[3300]123456", 0, "3300123456" },
        /*126*/ { "[3300]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*127*/ { "[3310]123456", 0, "3310123456" },
        /*128*/ { "[3310]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*129*/ { "[3320]123456", 0, "3320123456" },
        /*130*/ { "[3330]123456", 0, "3330123456" },
        /*131*/ { "[3340]123456", 0, "3340123456" },
        /*132*/ { "[3350]123456", 0, "3350123456" },
        /*133*/ { "[3360]123456", 0, "3360123456" },
        /*134*/ { "[3370]123456", 0, "3370123456" },
        /*135*/ { "[3370]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*136*/ { "[3379]123456", 0, "3379123456" },
        /*137*/ { "[3380]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*138*/ { "[3390]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*139*/ { "[34]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*140*/ { "[3400]123456", 0, "3400123456" },
        /*141*/ { "[3400]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*142*/ { "[3410]123456", 0, "3410123456" },
        /*143*/ { "[3410]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*144*/ { "[3420]123456", 0, "3420123456" },
        /*145*/ { "[3430]123456", 0, "3430123456" },
        /*146*/ { "[3440]123456", 0, "3440123456" },
        /*147*/ { "[3450]123456", 0, "3450123456" },
        /*148*/ { "[3460]123456", 0, "3460123456" },
        /*149*/ { "[3470]123456", 0, "3470123456" },
        /*150*/ { "[3480]123456", 0, "3480123456" },
        /*151*/ { "[3490]123456", 0, "3490123456" },
        /*152*/ { "[3490]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*153*/ { "[3499]123456", 0, "3499123456" },
        /*154*/ { "[35]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*155*/ { "[3500]123456", 0, "3500123456" },
        /*156*/ { "[3500]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*157*/ { "[3510]123456", 0, "3510123456", },
        /*158*/ { "[3510]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*159*/ { "[3520]123456", 0, "3520123456", },
        /*160*/ { "[3530]123456", 0, "3530123456", },
        /*161*/ { "[3540]123456", 0, "3540123456", },
        /*162*/ { "[3550]123456", 0, "3550123456", },
        /*163*/ { "[3560]123456", 0, "3560123456", },
        /*164*/ { "[3570]123456", 0, "3570123456", },
        /*165*/ { "[3579]123456", 0, "3579123456" },
        /*166*/ { "[3580]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*167*/ { "[3590]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*168*/ { "[36]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*169*/ { "[3600]123456", 0, "3600123456" },
        /*170*/ { "[3600]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*171*/ { "[3610]123456", 0, "3610123456" },
        /*172*/ { "[3610]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*173*/ { "[3620]123456", 0, "3620123456", },
        /*174*/ { "[3630]123456", 0, "3630123456", },
        /*175*/ { "[3640]123456", 0, "3640123456", },
        /*176*/ { "[3650]123456", 0, "3650123456", },
        /*177*/ { "[3660]123456", 0, "3660123456", },
        /*178*/ { "[3670]123456", 0, "3670123456", },
        /*179*/ { "[3680]123456", 0, "3680123456", },
        /*180*/ { "[3680]123456", 0, "3680123456", },
        /*181*/ { "[3680]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*182*/ { "[3690]123456", 0, "3690123456" },
        /*183*/ { "[3699]123456", 0, "3699123456" },
        /*184*/ { "[37]12345678", 0, "3712345678" },
        /*185*/ { "[37]123456789", ZINT_ERROR_INVALID_DATA, "" },
        /*186*/ { "[3700]12345678", ZINT_ERROR_INVALID_DATA, "" },
        /*187*/ { "[38]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*188*/ { "[3800]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*189*/ { "[39]123456", ZINT_ERROR_INVALID_DATA, "" },
        /*190*/ { "[3900]123456789012345", 0, "3900123456789012345" },
        /*191*/ { "[3900]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*192*/ { "[3909]123456789012345", 0, "3909123456789012345" },
        /*193*/ { "[3910]123123456789012345", 0, "3910123123456789012345" },
        /*194*/ { "[3910]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*195*/ { "[3910]123", ZINT_ERROR_INVALID_DATA, "" },
        /*196*/ { "[3920]123456789012345", 0, "3920123456789012345" },
        /*197*/ { "[3920]1234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*198*/ { "[3930]123123456789012345", 0, "3930123123456789012345" },
        /*199*/ { "[3930]1231234567890123456", ZINT_ERROR_INVALID_DATA, "" },
        /*200*/ { "[3930]123", ZINT_ERROR_INVALID_DATA, "" },
        /*201*/ { "[3940]1234", 0, "39401234" },
        /*202*/ { "[3940]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*203*/ { "[3940]123", ZINT_ERROR_INVALID_DATA, "" },
        /*204*/ { "[3949]1234", 0, "39491234" },
        /*205*/ { "[3950]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*206*/ { "[3960]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*207*/ { "[3970]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*208*/ { "[3980]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*209*/ { "[3999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*210*/ { "[40]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*211*/ { "[400]123456789012345678901234567890", 0, "400123456789012345678901234567890" },
        /*212*/ { "[400]1234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*213*/ { "[401]1234abcdefghijklmnopqrstuvwxyz", 0, "4011234abcdefghijklmnopqrstuvwxyz" },
        /*214*/ { "[401]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*215*/ { "[402]13131313131313132", 0, "40213131313131313132" },
        /*216*/ { "[402]1313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*217*/ { "[403]abcdefghijklmnopqrstuvwxyz1234", 0, "403abcdefghijklmnopqrstuvwxyz1234" },
        /*218*/ { "[403]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*219*/ { "[404]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*220*/ { "[409]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*221*/ { "[410]3898765432108", 0, "4103898765432108" },
        /*222*/ { "[410]12345678901234", ZINT_ERROR_INVALID_DATA, "" },
        /*223*/ { "[411]1313131313134", 0, "4111313131313134" },
        /*224*/ { "[411]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*225*/ { "[412]1313131313134", 0, "4121313131313134" },
        /*226*/ { "[412]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*227*/ { "[413]1313131313134", 0, "4131313131313134" },
        /*228*/ { "[413]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*229*/ { "[414]1313131313134", 0, "4141313131313134" },
        /*230*/ { "[414]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*231*/ { "[415]1313131313134", 0, "4151313131313134" },
        /*232*/ { "[415]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*233*/ { "[416]1313131313134", 0, "4161313131313134" },
        /*234*/ { "[416]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*235*/ { "[417]1313131313134", 0, "4171313131313134" },
        /*236*/ { "[417]13131313131", ZINT_ERROR_INVALID_DATA, "" },
        /*237*/ { "[418]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*238*/ { "[419]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*239*/ { "[420]abcdefghijklmnopqrst", 0, "420abcdefghijklmnopqrst" },
        /*240*/ { "[420]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*241*/ { "[421]123abcdefghi", 0, "421123abcdefghi" },
        /*242*/ { "[421]123abcdefghij", ZINT_ERROR_INVALID_DATA, "" },
        /*243*/ { "[422]123", 0, "422123" },
        /*244*/ { "[422]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*245*/ { "[422]12", ZINT_ERROR_INVALID_DATA, "" },
        /*246*/ { "[423]123123123123123", 0, "423123123123123123" },
        /*247*/ { "[423]1231231231231231", ZINT_ERROR_INVALID_DATA, "" },
        /*248*/ { "[423]12", ZINT_ERROR_INVALID_DATA, "" },
        /*249*/ { "[424]123", 0, "424123" },
        /*250*/ { "[424]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*251*/ { "[424]12", ZINT_ERROR_INVALID_DATA, "" },
        /*252*/ { "[425]123123123123123", 0, "425123123123123123" },
        /*253*/ { "[425]1231231231231231", ZINT_ERROR_INVALID_DATA, "" },
        /*254*/ { "[425]12", ZINT_ERROR_INVALID_DATA, "" },
        /*255*/ { "[426]123", 0, "426123" },
        /*256*/ { "[426]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*257*/ { "[426]12", ZINT_ERROR_INVALID_DATA, "" },
        /*258*/ { "[427]abc", 0, "427abc" },
        /*259*/ { "[427]abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*260*/ { "[428]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*261*/ { "[429]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*262*/ { "[430]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*263*/ { "[499]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*264*/ { "[50]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*265*/ { "[500]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*266*/ { "[5000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*267*/ { "[51]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*268*/ { "[59]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*269*/ { "[60]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*270*/ { "[600]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*271*/ { "[6000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*272*/ { "[61]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*273*/ { "[69]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*274*/ { "[70]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*275*/ { "[700]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*276*/ { "[7000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*277*/ { "[7001]1234567890123", 0, "70011234567890123" },
        /*278*/ { "[7001]123456789012", ZINT_ERROR_INVALID_DATA, "" },
        /*279*/ { "[7002]abcdefghijklmnopqrstuvwxyz1234", 0, "7002abcdefghijklmnopqrstuvwxyz1234" },
        /*280*/ { "[7002]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*281*/ { "[7003]1212121212", 0, "70031212121212" },
        /*282*/ { "[7003]121212121", ZINT_ERROR_INVALID_DATA, "" },
        /*283*/ { "[7004]1234", 0, "70041234" },
        /*284*/ { "[7004]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*285*/ { "[7005]abcdefghijkl", 0, "7005abcdefghijkl" },
        /*286*/ { "[7005]abcdefghijklm", ZINT_ERROR_INVALID_DATA, "" },
        /*287*/ { "[7006]200132", 0, "7006200132" },
        /*288*/ { "[7006]2001320", ZINT_ERROR_INVALID_DATA, "" },
        /*289*/ { "[7007]010101121212", 0, "7007010101121212" },
        /*290*/ { "[7007]01010112121", ZINT_ERROR_INVALID_DATA, "" },
        /*291*/ { "[7007]121212", 0, "7007121212" },
        /*292*/ { "[7007]12121", ZINT_ERROR_INVALID_DATA, "" },
        /*293*/ { "[7007]1212121", ZINT_ERROR_INVALID_DATA, "" },
        /*294*/ { "[7008]abc", 0, "7008abc" },
        /*295*/ { "[7008]abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*296*/ { "[7009]abcdefghij", 0, "7009abcdefghij" },
        /*297*/ { "[7009]abcdefghijk", ZINT_ERROR_INVALID_DATA, "" },
        /*298*/ { "[7010]01", 0, "701001" },
        /*299*/ { "[7010]1", 0, "70101" },
        /*300*/ { "[7010]012", ZINT_ERROR_INVALID_DATA, "" },
        /*301*/ { "[7011]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*302*/ { "[7012]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*303*/ { "[7019]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*304*/ { "[7020]abcdefghijklmnopqrst", 0, "7020abcdefghijklmnopqrst" },
        /*305*/ { "[7020]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*306*/ { "[7021]abcdefghijklmnopqrst", 0, "7021abcdefghijklmnopqrst" },
        /*307*/ { "[7021]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*308*/ { "[7022]abcdefghijklmnopqrst", 0, "7022abcdefghijklmnopqrst" },
        /*309*/ { "[7022]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*310*/ { "[7023]1234abcdefghijklmnopqrstuvwxyz", 0, "70231234abcdefghijklmnopqrstuvwxyz" },
        /*311*/ { "[7023]1234abcdefghijklmnopqrstuvwxyza", ZINT_ERROR_INVALID_DATA, "" },
        /*312*/ { "[7024]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*313*/ { "[7025]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*314*/ { "[7029]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*315*/ { "[7030]123abcdefghijklmnopqrstuvwxyza", 0, "7030123abcdefghijklmnopqrstuvwxyza" },
        /*316*/ { "[7030]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*317*/ { "[7031]123abcdefghijklmnopqrstuvwxyza", 0, "7031123abcdefghijklmnopqrstuvwxyza" },
        /*318*/ { "[7031]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*319*/ { "[7032]123abcdefghijklmnopqrstuvwxyza", 0, "7032123abcdefghijklmnopqrstuvwxyza" },
        /*320*/ { "[7033]123abcdefghijklmnopqrstuvwxyza", 0, "7033123abcdefghijklmnopqrstuvwxyza" },
        /*321*/ { "[7034]123abcdefghijklmnopqrstuvwxyza", 0, "7034123abcdefghijklmnopqrstuvwxyza" },
        /*322*/ { "[7035]123abcdefghijklmnopqrstuvwxyza", 0, "7035123abcdefghijklmnopqrstuvwxyza" },
        /*323*/ { "[7036]123abcdefghijklmnopqrstuvwxyza", 0, "7036123abcdefghijklmnopqrstuvwxyza" },
        /*324*/ { "[7037]123abcdefghijklmnopqrstuvwxyza", 0, "7037123abcdefghijklmnopqrstuvwxyza" },
        /*325*/ { "[7038]123abcdefghijklmnopqrstuvwxyza", 0, "7038123abcdefghijklmnopqrstuvwxyza" },
        /*326*/ { "[7039]123abcdefghijklmnopqrstuvwxyza", 0, "7039123abcdefghijklmnopqrstuvwxyza" },
        /*327*/ { "[7039]123abcdefghijklmnopqrstuvwxyzab", ZINT_ERROR_INVALID_DATA, "" },
        /*328*/ { "[7040]1abc", 0, "70401abc" },
        /*329*/ { "[7040]1ab", ZINT_ERROR_INVALID_DATA, "" },
        /*330*/ { "[7040]1abcd", ZINT_ERROR_INVALID_DATA, "" },
        /*331*/ { "[7041]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*332*/ { "[7042]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*333*/ { "[7050]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*334*/ { "[7090]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*335*/ { "[7099]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*336*/ { "[71]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*337*/ { "[710]abcdefghijklmnopqrst", 0, "710abcdefghijklmnopqrst" },
        /*338*/ { "[710]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*339*/ { "[711]abcdefghijklmnopqrst", 0, "711abcdefghijklmnopqrst" },
        /*340*/ { "[711]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*341*/ { "[712]abcdefghijklmnopqrst", 0, "712abcdefghijklmnopqrst" },
        /*342*/ { "[712]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*343*/ { "[713]abcdefghijklmnopqrst", 0, "713abcdefghijklmnopqrst" },
        /*344*/ { "[713]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*345*/ { "[714]abcdefghijklmnopqrst", 0, "714abcdefghijklmnopqrst" },
        /*346*/ { "[714]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*347*/ { "[715]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*348*/ { "[716]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*349*/ { "[719]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*350*/ { "[7100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*351*/ { "[72]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*352*/ { "[7200]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*353*/ { "[7210]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*354*/ { "[7220]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*355*/ { "[7230]EMabcdefghijklmnopqrstuvwxyzab", 0, "7230EMabcdefghijklmnopqrstuvwxyzab" },
        /*356*/ { "[7230]EMabcdefghijklmnopqrstuvwxyzabc", ZINT_ERROR_INVALID_DATA, "" },
        /*357*/ { "[7230]EM", ZINT_ERROR_INVALID_DATA, "" },
        /*358*/ { "[7231]EMabcdefghijklmnopqrstuvwxyzab", 0, "7231EMabcdefghijklmnopqrstuvwxyzab" },
        /*359*/ { "[7232]EMabcdefghijklmnopqrstuvwxyzab", 0, "7232EMabcdefghijklmnopqrstuvwxyzab" },
        /*360*/ { "[7233]EMabcdefghijklmnopqrstuvwxyzab", 0, "7233EMabcdefghijklmnopqrstuvwxyzab" },
        /*361*/ { "[7234]EMabcdefghijklmnopqrstuvwxyzab", 0, "7234EMabcdefghijklmnopqrstuvwxyzab" },
        /*362*/ { "[7235]EMabcdefghijklmnopqrstuvwxyzab", 0, "7235EMabcdefghijklmnopqrstuvwxyzab" },
        /*363*/ { "[7236]EMabcdefghijklmnopqrstuvwxyzab", 0, "7236EMabcdefghijklmnopqrstuvwxyzab" },
        /*364*/ { "[7237]EMabcdefghijklmnopqrstuvwxyzab", 0, "7237EMabcdefghijklmnopqrstuvwxyzab" },
        /*365*/ { "[7238]EMabcdefghijklmnopqrstuvwxyzab", 0, "7238EMabcdefghijklmnopqrstuvwxyzab" },
        /*366*/ { "[7239]EMabcdefghijklmnopqrstuvwxyzab", 0, "7239EMabcdefghijklmnopqrstuvwxyzab" },
        /*367*/ { "[7239]E", ZINT_ERROR_INVALID_DATA, "" },
        /*368*/ { "[7240]abcdefghijklmnopqrst", 0, "7240abcdefghijklmnopqrst" },
        /*369*/ { "[7240]abcdefghijklmnopqrstu", ZINT_ERROR_INVALID_DATA, "" },
        /*370*/ { "[7241]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*371*/ { "[7249]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*372*/ { "[7250]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*373*/ { "[7299]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*374*/ { "[73]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*375*/ { "[7300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*376*/ { "[74]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*377*/ { "[7400]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*378*/ { "[79]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*379*/ { "[7900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*380*/ { "[7999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*381*/ { "[80]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*382*/ { "[800]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*383*/ { "[8000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*384*/ { "[8001]12345678901234", 0, "800112345678901234" },
        /*385*/ { "[8001]1234123456789012345", ZINT_ERROR_INVALID_DATA, "" },
        /*386*/ { "[8002]abcdefghijklmnopqrst", 0, "8002abcdefghijklmnopqrst" },
        /*387*/ { "[8002]abcdefghijklmnopqrstuv", ZINT_ERROR_INVALID_DATA, "" },
        /*388*/ { "[8003]01234567890123abcdefghijklmnop", 0, "800301234567890123abcdefghijklmnop" },
        /*389*/ { "[8004]abcdefghijklmnopqrstuvwxyz1234", 0, "8004abcdefghijklmnopqrstuvwxyz1234" },
        /*390*/ { "[8004]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*391*/ { "[8005]123456", 0, "8005123456" },
        /*392*/ { "[8005]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*393*/ { "[8005]1234567", ZINT_ERROR_INVALID_DATA, "" },
        /*394*/ { "[8006]123456789012341212", 0, "8006123456789012341212" },
        /*395*/ { "[8006]12345678901234121", ZINT_ERROR_INVALID_DATA, "" },
        /*396*/ { "[8006]1234567890123412123", ZINT_ERROR_INVALID_DATA, "" },
        /*397*/ { "[8007]abcdefghijklmnopqrstuvwxyz12345678", 0, "8007abcdefghijklmnopqrstuvwxyz12345678" },
        /*398*/ { "[8007]abcdefghijklmnopqrstuvwxyz123456789", ZINT_ERROR_INVALID_DATA, "" },
        /*399*/ { "[8008]123456121212", 0, "8008123456121212" },
        /*400*/ { "[8008]1234561212", 0, "80081234561212" },
        /*401*/ { "[8008]12345612", 0, "800812345612" },
        /*402*/ { "[8008]1234561", ZINT_ERROR_INVALID_DATA, "" },
        /*403*/ { "[8008]123456121", ZINT_ERROR_INVALID_DATA, "" },
        /*404*/ { "[8008]12345612121", ZINT_ERROR_INVALID_DATA, "" },
        /*405*/ { "[8008]1234561212123", ZINT_ERROR_INVALID_DATA, "" },
        /*406*/ { "[8009]12345678901234567890123456789012345678901234567890", 0, "800912345678901234567890123456789012345678901234567890" },
        /*407*/ { "[8009]123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*408*/ { "[8010]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*409*/ { "[8011]123456789012", 0, "8011123456789012" },
        /*410*/ { "[8011]1234567890123", ZINT_ERROR_INVALID_DATA, "" },
        /*411*/ { "[8012]abcdefghijklmnopqrst", 0, "8012abcdefghijklmnopqrst" },
        /*412*/ { "[8012]abcdefghijklmnopqrstuv", ZINT_ERROR_INVALID_DATA, "" },
        /*413*/ { "[8013]1234abcdefghijklmnopqrstuvwxyz", 0, "80131234abcdefghijklmnopqrstuvwxyz" },
        /*414*/ { "[8013]1234abcdefghijklmnopqrstuvwxyz1", ZINT_ERROR_INVALID_DATA, "" },
        /*415*/ { "[8014]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*416*/ { "[8016]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*417*/ { "[8017]313131313131313139", 0, "8017313131313131313139" },
        /*418*/ { "[8017]31313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*419*/ { "[8017]3131313131313131390", ZINT_ERROR_INVALID_DATA, "" },
        /*420*/ { "[8018]313131313131313139", 0, "8018313131313131313139" },
        /*421*/ { "[8018]31313131313131313", ZINT_ERROR_INVALID_DATA, "" },
        /*422*/ { "[8018]3131313131313131390", ZINT_ERROR_INVALID_DATA, "" },
        /*423*/ { "[8019]1234567890", 0, "80191234567890" },
        /*424*/ { "[8019]12345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*425*/ { "[8020]abcdefghijklmnopqrstuvwxy", 0, "8020abcdefghijklmnopqrstuvwxy" },
        /*426*/ { "[8020]abcdefghijklmnopqrstuvwxyz", ZINT_ERROR_INVALID_DATA, "" },
        /*427*/ { "[8021]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*428*/ { "[8025]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*429*/ { "[8026]123456789012341212", 0, "8026123456789012341212" },
        /*430*/ { "[8026]1234567890123451212", ZINT_ERROR_INVALID_DATA, "" },
        /*431*/ { "[8026]12345678901234512", ZINT_ERROR_INVALID_DATA, "" },
        /*432*/ { "[8027]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*433*/ { "[8030]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*434*/ { "[8040]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*435*/ { "[8090]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*436*/ { "[8099]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*437*/ { "[81]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*438*/ { "[8100]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*439*/ { "[8109]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*440*/ { "[8110]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "81101234567890123456789012345678901234567890123456789012345678901234567890" },
        /*441*/ { "[8110]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*442*/ { "[8111]1234", 0, "81111234" },
        /*443*/ { "[8111]12345", ZINT_ERROR_INVALID_DATA, "" },
        /*444*/ { "[8111]123", ZINT_ERROR_INVALID_DATA, "" },
        /*445*/ { "[8112]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "81121234567890123456789012345678901234567890123456789012345678901234567890" },
        /*446*/ { "[8112]12345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*447*/ { "[8113]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*448*/ { "[8120]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*449*/ { "[8190]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*450*/ { "[8199]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*451*/ { "[82]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*452*/ { "[8200]1234567890123456789012345678901234567890123456789012345678901234567890", 0, "82001234567890123456789012345678901234567890123456789012345678901234567890" },
        /*453*/ { "[8201]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*454*/ { "[8210]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*455*/ { "[8290]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*456*/ { "[8299]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*457*/ { "[83]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*458*/ { "[8300]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*459*/ { "[89]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*460*/ { "[8900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*461*/ { "[90]abcdefghijklmnopqrstuvwxyz1234", 0, "90abcdefghijklmnopqrstuvwxyz1234" },
        /*462*/ { "[9000]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*463*/ { "[90]abcdefghijklmnopqrstuvwxyz12345", ZINT_ERROR_INVALID_DATA, "" },
        /*464*/ { "[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "91123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*465*/ { "[91]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*466*/ { "[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "92123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*467*/ { "[92]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*468*/ { "[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "93123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*469*/ { "[93]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*470*/ { "[94]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "94123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*471*/ { "[94]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*472*/ { "[98]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "98123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*473*/ { "[98]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*474*/ { "[99]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, "99123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" },
        /*475*/ { "[99]1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_INVALID_DATA, "" },
        /*476*/ { "[9900]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*477*/ { "[9999]1234", ZINT_ERROR_INVALID_DATA, "" },
        /*478*/ { "[01]12345678901234[7006]200101", 0, "01123456789012347006200101" },
        /*479*/ { "[3900]1234567890[01]12345678901234", 0, "39001234567890[0112345678901234" },
        /*480*/ { "[253]12345678901234[3901]12345678901234[20]12", 0, "25312345678901234[390112345678901234[2012" },
        /*481*/ { "[253]12345678901234[01]12345678901234[3901]12345678901234[20]12", 0, "25312345678901234[0112345678901234390112345678901234[2012" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char reduced[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
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

static void test_input_mode(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        unsigned char* composite;
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

    char* text;
    struct zint_symbol previous_symbol;

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = data[i].input_mode;
        if (data[i].output_options != -1) {
            symbol->output_options = data[i].output_options;
        }

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        int length = strlen(text);

        ret = ZBarcode_Encode(symbol, text, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d %s\n", i, ret, data[i].ret, symbol->errtxt);
        if (data[i].compare_previous) {
            ret = testUtilSymbolCmp(symbol, &previous_symbol);
            assert_zero(ret, "i:%d testUtilSymbolCmp ret %d != 0\n", i, ret);
        }
        memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_gs1_reduce();
    test_hrt();
    test_gs1_verify();
    test_input_mode();

    testReport();

    return 0;
}
