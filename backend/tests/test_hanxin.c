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

//#define TEST_INPUT_GENERATE_EXPECTED 1
//#define TEST_ENCODE_GENERATE_EXPECTED 1

static void test_options(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_1;
        int option_2;
        int ret_encode;
        int ret_vector;
        int expected_size;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "12345", -1, -1, 0, 0, 23 }, // Default version 1, ECC auto-set to 4
        /*  1*/ { "12345", 1, -1, 0, 0, 23 },
        /*  2*/ { "12345", -1, 2, 0, 0, 25 },
        /*  3*/ { "12345", -1, 85, 0, 0, 23 }, // Version > max version 85 so ignored
        /*  4*/ { "12345", -1, 84, 0, 0, 189 },
        /*  5*/ { "1234567890123456789012345678901234567890123", 1, 1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { "1234567890123456", 4, 1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  7*/ { "12345678901234567", 4, 2, 0, 0, 25 },
        /*  8*/ { "12345678901234567", 4, -1, 0, 0, 25 }, // Version auto-set to 2
        /*  9*/ { "12345678901234567", -1, -1, 0, 0, 23 }, // Version auto-set to 1, ECC auto-set to 3
        /* 10*/ { "12345678901234567", 5, -1, 0, 0, 23 }, // ECC > max ECC 4 so ignored and auto-settings version 1, ECC 3 used
        /* 11*/ { "1234567890123456789012345678901234567890123", -1, -1, 0, 0, 25 }, // Version auto-set to 2, ECC auto-set to 2
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_HANXIN;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
            assert_equal(symbol->width, data[i].expected_size, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_size);
            assert_equal(symbol->rows, data[i].expected_size, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_size);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        int eci;
        unsigned char* data;
        int length;
        int ret;
        int expected_eci;
        char* expected;
        char* comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 18030 0xA8A6, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek 0xE2 (but not other ISO 8859 or Win page), in GB 18030 0xA6C2, UTF-8 CEB2
    // ÿ U+00FF in ISO 8859-1 0xFF, not in GB 18030, outside first byte and second byte range, UTF-8 C3BF
    // 啊 U+554A GB 18030 Region One 0xB0A1, UTF-8 E5958A
    // 亍 U+4E8D GB 18030 Region Two 0xD8A1, UTF-8 E4BA8D
    // 齄 U+9F44 GB 18030 Region Two 0xF7FE, UTF-8 E9BD84
    // 丂 U+4E02 GB 18030 2-byte Region 0x8140, UTF-8 E4B882
    // PAD U+0080 GB 18030 4-byte Region 0x81308130, UTF-8 C280 (\302\200)
    // � (REPLACEMENT CHARACTER) U+FFFD GB 18030 4-byte Region 0x81308130, UTF-8 EFBFBD (\357\277\275)
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, "é", -1, 0, 0, "30 00 F4 80 00 00 00 00 00", "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, "é", -1, 0, 3, "80 33 00 0F 48 00 00 00 00", "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 29, "é", -1, 0, 29, "81 D4 FC FF FF 00 00 00 00", "ECI-29 H(1)1 (GB 18030) (Region One)" },
        /*  3*/ { UNICODE_MODE, 26, "é", -1, 0, 26, "81 A4 70 2F FF 00 00 00 00", "ECI-26 H(1)1 (UTF-8) (Region One)" },
        /*  4*/ { DATA_MODE, 0, "é", -1, 0, 0, "47 02 FF F0 00 00 00 00 00", "H(1)1 (UTF-8) (Region One)" },
        /*  5*/ { DATA_MODE, 0, "\351", -1, 0, 0, "30 00 F4 80 00 00 00 00 00", "B1 (ISO 8859-1) (0xE9)" },
        /*  6*/ { UNICODE_MODE, 0, "β", -1, 0, 0, "30 01 53 61 00 00 00 00 00", "B2 (GB 18030) (2-byte Region)" },
        /*  7*/ { UNICODE_MODE, 9, "β", -1, 0, 9, "80 93 00 0F 10 00 00 00 00", "ECI-9 B1 (ISO 8859-7)" },
        /*  8*/ { UNICODE_MODE, 29, "β", -1, 0, 29, "81 D3 00 15 36 10 00 00 00", "ECI-29 B2 (GB 18030) (2-byte Region)" },
        /*  9*/ { UNICODE_MODE, 26, "β", -1, 0, 26, "81 A4 B1 5F FF 00 00 00 00", "ECI-26 H(1)1 (UTF-8) (Region One)" },
        /* 10*/ { DATA_MODE, 0, "β", -1, 0, 0, "4B 15 FF F0 00 00 00 00 00", "H(1)1 (UTF-8) (Region One)" },
        /* 11*/ { UNICODE_MODE, 0, "ÿ", -1, 0, 0, "30 00 FF 80 00 00 00 00 00", "B1 (ISO 8859-1)" },
        /* 12*/ { UNICODE_MODE, 0, "ÿÿÿ", -1, 0, 0, "30 01 FF FF FF 80 00 00 00", "B3 (ISO 8859-1)" },
        /* 13*/ { UNICODE_MODE, 0, "\302\200", -1, 0, 0, "70 00 00 00 00 00 00 00 00", "H(f)1 (GB 18030) (4-byte Region)" },
        /* 14*/ { UNICODE_MODE, 0, "\302\200�", -1, 0, 0, "70 00 00 38 26 7E 40 00 00", "H(f)2 (GB 18030) (both 4-byte Region)" },
        /* 15*/ { UNICODE_MODE, 0, "啊亍齄丂\302\200", -1, 0, 0, "64 68 50 3C AC 28 80 00 FF FE E0 00 00 00 00 00 00", "H(d)4 H(f)1 (GB 18030)" },
        /* 16*/ { DATA_MODE, 0, "\177\177", -1, 0, 0, "2F BD F7 F0 00 00 00 00 00", "T2 (ASCII)" },
        /* 17*/ { DATA_MODE, 0, "\177\177\177", -1, 0, 0, "2F BD F7 DF C0 00 00 00 00", "T3 (ASCII)" },
        /* 18*/ { UNICODE_MODE, 0, "123", -1, 0, 0, "11 EF FF 00 00 00 00 00 00", "N3 (ASCII)" },
        /* 19*/ { UNICODE_MODE, 0, "12345", -1, 0, 0, "11 EC 2D FF 80 00 00 00 00", "N5 (ASCII)" },
        /* 20*/ { UNICODE_MODE, 0, "Aa%$Bb9", -1, 0, 0, "22 A4 FA 18 3E 2E 52 7F 00", "T7 (ASCII)" },
        /* 21*/ { UNICODE_MODE, 0, "Summer Palace Ticket for 6 June 2015 13:00;2015年6月6日夜01時00分PM頤和園のチケット;2015년6월6일13시오후여름궁전티켓.2015年6月6号下午13:00的颐和园门票;", -1, 0, 0, "(189) 27 38 C3 0A 35 F9 CF 99 92 F9 26 A3 E7 3E 76 C9 AE A3 7F CC 08 04 0C CD EE 44 06 C4", "T20 B64 N4 H(f)1 T1 H(f)1 T1 H(f)1 T2 H(f)9 B35 (GB 18030)" },
        /* 22*/ { UNICODE_MODE, 0, "\000\014\033 #/059:<@AMZ", 15, 0, 0, "2F 80 31 B7 1F AF E0 05 27 EB 2E CB E2 96 8F F0 00", "T15 (ASCII)" },
        /* 23*/ { UNICODE_MODE, 0, "Z[\\`alz{~\177", -1, 0, 0, "28 FE CF 4E 3E 92 FF 7E E7 CF 7F 00 00", "T10 (ASCII)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_HANXIN;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                ret < 5 ? symbol->eci : -1, symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {

            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int input_mode;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { "1234", UNICODE_MODE, -1, -1, 0, 23, 23, "",
                    "11111110101000101111111"
                    "10000000010001000000001"
                    "10111110101110001111101"
                    "10100000101011000000101"
                    "10101110010011101110101"
                    "10101110101111101110101"
                    "10101110011010001110101"
                    "00000000110001100000000"
                    "00010101111011001010101"
                    "01001010101101000111110"
                    "10101101001010101010100"
                    "11011101010100100010010"
                    "10101010010011101010100"
                    "01010100111110000011110"
                    "10101010001011110101000"
                    "00000000110111100000000"
                    "11111110011001001110101"
                    "00000010110000101110101"
                    "11111010101110001110101"
                    "00001010010110100000101"
                    "11101010001100101111101"
                    "11101010000011000000001"
                    "11101010101010101111111"
               },
        /*  1*/ { "1234567890", UNICODE_MODE, 1, 1, 0, 23, 23, "**NOT SAME** as ISO 20830 Draft K.1 Example 1, uses masking pattern 2 instead of pattern 1; however matches pattern 2 example (excluding Function Info)",
                    "11111110001011101111111"
                    "10000000100000000000001"
                    "10111110011111001111101"
                    "10100000000111000000101"
                    "10101110011110101110101"
                    "10101110011111101110101"
                    "10101110101111101110101"
                    "00000000001100000000000"
                    "00010101011111010010101"
                    "01010001100001001001101"
                    "00100100100100101011100"
                    "11111000010111111111111"
                    "01001001001001001001000"
                    "11110010100100110010000"
                    "10101001011111010101000"
                    "00000000001000000000000"
                    "11111110110000101110101"
                    "00000010111111001110101"
                    "11111010101001001110101"
                    "00001010000011000000101"
                    "11101010010111001111101"
                    "11101010001001100000001"
                    "11101010100100001111111"
               },
        /*  2*/ { "1234567890ABCDEFGabcdefg,Han Xin Code", UNICODE_MODE, 3, 10, 0, 41, 41, "Same as ISO 20830 Draft K.2 Example 2 (happens to use same mask pattern, 2)",
                    "11111110001011000000100000010101101111111"
                    "10000000001011110010000000011010100000001"
                    "10111110111111111010111011101111101111101"
                    "10100000101001001001001001001001100000101"
                    "10101110000100100101010010010111001110101"
                    "10101110010111110111111011101100101110101"
                    "10101110101111001001011101110011001110101"
                    "00000000011001100100100100100100100000000"
                    "00011110111111111111111001101111110010101"
                    "10110011011100110010001001100000001001001"
                    "11001100100100100100100100100100100100100"
                    "11111111001110101101011011110011011110110"
                    "10100001001001000001100001010010001001001"
                    "10100100100100100100100101100100001110011"
                    "10011111100010110000001111111111101000001"
                    "10110000001001001001001001001001001001001"
                    "10101011011110000011001101111001100100100"
                    "11111111000110010101111011111111111111111"
                    "11001001001001000110011001110111100000010"
                    "10111110100100100100100110001110011011000"
                    "11111111111111111111111111111111111111101"
                    "00000000000000000000101001001001001011000"
                    "01110110101001001010101000010110100100100"
                    "11111111101110010010100011111111111111111"
                    "01001001001001001100110001101111011101011"
                    "00101111100100100100100100001101001101110"
                    "11111111111111111110111111111111111001111"
                    "10010110100000101110111010011001001001001"
                    "00101001101100100010100100100100100100100"
                    "11111111111110110000111000110000110000101"
                    "00000001001001001000101011011100000100101"
                    "00100100100100100100100100100100101011111"
                    "10101001100101110100111011100010101111000"
                    "00000000110001001000101001001111000000000"
                    "11111110001011100100100100100100101110101"
                    "00000010111111111110111111111111001110101"
                    "11111010001001000000111111111011001110101"
                    "00001010100101111000101010100100100000101"
                    "11101010111111111100101100001111101111101"
                    "11101010101001001000101001001001000000001"
                    "11101010100100100100111111111111001111111"
               },
        /*  3*/ { "Summer Palace Ticket for 6 June 2015 13:00;2015年6月6日夜01時00分PM頤和園のチケット;2015년6월6일13시오후여름궁전티켓.2015年6月6号下午13:00的颐和园门票;", UNICODE_MODE, 2, 17, 0, 55, 55, "**NOT SAME** as ISO 20830 Draft K.3 Example 3, different encoding modes; if same encoding modes forced, uses masking pattern 1 instead of pattern 2, but matches pattern 1 example (excluding Function Info)",
                    "1111111001111111111011100100110101101010101100101111111"
                    "1000000000000000001100011000011001000010101111100000001"
                    "1011111011110010101110010110100000111010101101101111101"
                    "1010000001010100001101011100001101100100010100000000101"
                    "1010111000011011001111001000010010110010101010001110101"
                    "1010111011010101001101010100001010011001000110001110101"
                    "1010111001101001001001110010001001100100001001001110101"
                    "0000000011100111101101111010001010001100110011000000000"
                    "0010010101010100001100111100101010101111010001101010101"
                    "1111111011101110101000110010100010000101010101010101010"
                    "1010100111011011001101110110100101100011101000111110110"
                    "0011100111010001101001111011100001001111110010000011001"
                    "0011000100100010101011000001101101010000001010011010000"
                    "1100111101010101001101010101010100010100001110110101000"
                    "0000000100111001001010101100101100000001011111001110100"
                    "1101000010010001001101110001010101101100101110001110111"
                    "0101010101011100001110010001111110101010101010101010101"
                    "0001011000101000101011010011111000010010000011110101100"
                    "1001000100000110001111111111111111111000011101001110001"
                    "0010110011110110100000000000000000001101011101001100000"
                    "1111111100000100100000111011111001001111011001011100101"
                    "1101010101010101010101010100111011001110101010100111101"
                    "1101010011001001100110000001001000101000001011111110000"
                    "1000111001010111001010111111101100101101010000111001101"
                    "1110101100000011001001001011001010101010101010101010101"
                    "1101001110000100101100101011001100001001110111011001000"
                    "1001101010000000001000010010101100001011101001110010101"
                    "1101001100101100100011100010110000101101110100110010110"
                    "1000001010101010101010101010111101001010101100011001100"
                    "1101000101010110010101100101011000101001001000001000001"
                    "1010011101101101010001000111011011101011111010101111001"
                    "1101101100100000010010000011011001001101010101010101010"
                    "1010111000110111100101100011101010001010001001101110011"
                    "1100110000001101010011010000001011101011110011010101001"
                    "1011101111011001101010110111101100101100110001101100101"
                    "1000110111010101010101010101010100001101011110111010101"
                    "1111111111111111111110001011001010101111111111111111111"
                    "0000000000000000001110000010010000000000000000000000001"
                    "0100011110001011001010000111010001001101001001010010101"
                    "0101011111000100101000110110001100101111110100110101001"
                    "0100011010101010101010100110110101100111011001011000101"
                    "1000110011110001101000011011111101100010100001110000101"
                    "1111001110101101101000100011011010001010011000010000001"
                    "0001110000001011001100000011111101010101010101010101001"
                    "1010011101001101001101011101000010010100010000001110101"
                    "1101100101011001101001000100010000001101111001000111001"
                    "1010101010110101101010010110010011001111101011010100100"
                    "0000000000000100001001100011100100010101010101100000000"
                    "1111111000101010101010101110111100111011011111001110101"
                    "0000001000101011001010000100010101001110001111101110101"
                    "1111101000100001101101000001100001001011101001001110101"
                    "0000101001000100101010110010010101010110011001000000101"
                    "1110101010111000101011111101011101111100001110101111101"
                    "1110101011010101001101010101010101000000001010000000001"
                    "1110101011010001001111111111111111101000001110001111111"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_HANXIN;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { \"%s\", %s, %d, %d, %s, %d, %d, \"%s\",\n",
                i, data[i].data, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
                symbol->rows, symbol->width, data[i].comment);
        testUtilModulesDump(symbol, "                    ", "\n");
        printf("               },\n");
        #else
        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            if (ret == 0) {
                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
            }
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_options();
    test_input();
    test_encode();

    testReport();

    return 0;
}
