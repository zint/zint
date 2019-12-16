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
        /*  0*/ { "12345", 0, 0, 0, 0, 18 },
        /*  1*/ { "12345", 0, 1, 0, 0, 18 },
        /*  2*/ { "12345", 0, 2, 0, 0, 30 },
        /*  3*/ { "12345", 0, 14, 0, 0, 18 }, // Version > max version 13 so ignored
        /*  4*/ { "12345", 0, 13, 0, 0, 162 },
        /*  5*/ { "1234567890123456789", 0, 1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { "1234567890123456789", 0, 2, 0, 0, 30 },
        /*  7*/ { "123456789012345678", 0, 0, 0, 0, 30 }, // Version auto-set to 2
        /*  8*/ { "123456789012345678", 0, 1, 0, 0, 18 },
        /*  9*/ { "123456789012345678", 5, 1, 0, 0, 18 }, // Version specified so overrides ECC level which gets reduced to 4
        /* 10*/ { "123456789012345678", 5, 0, 0, 0, 30 }, // Version not specified so increased to allow for ECC level
        /* 11*/ { "123456789012345678", 6, 0, 0, 0, 30 }, // ECC > max ECC 5 so ignored and auto-settings version 2, ECC 4 used
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_GRIDMATRIX;
        symbol->option_1 = data[i].option_1;
        symbol->option_2 = data[i].option_2;
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
        int ret;
        int expected_eci;
        char* expected;
        char* comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 2312 0xA8A6, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in GB 2312 0xA6C2, UTF-8 CEB2
    // ÿ U+00FF in ISO 8859-1 0xFF, not in GB 2312, outside first byte and second byte range, UTF-8 C3BF
    // ㈩ U+3229 in GB 2312 0x226E
    // 一 U+4E00 in GB 2312 0x523B
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, "é", 0, 0, "30 01 69 00", "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, "é", 0, 3, "60 01 58 00 74 40", "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 29, "é", 0, 29, "60 0E 44 2A 37 7C 00", "ECI-29 H1 (GB 2312)" },
        /*  3*/ { UNICODE_MODE, 26, "é", 0, 26, "60 0D 05 28 4F 7C 00", "ECI-26 H1 (UTF-8)" },
        /*  4*/ { DATA_MODE, 0, "é", 0, 0, "0A 51 1F 78 00", "H1 (UTF-8)" },
        /*  5*/ { DATA_MODE, 0, "\351", 0, 0, "30 01 69 00", "B1 (ISO 8859-1) (0xE9)" },
        /*  6*/ { UNICODE_MODE, 0, "β", 0, 0, "08 40 2F 78 00", "H1 (GB 2312)" },
        /*  7*/ { UNICODE_MODE, 9, "β", 0, 9, "60 04 58 00 71 00", "ECI-9 B1 (ISO 8859-7)" },
        /*  8*/ { UNICODE_MODE, 29, "β", 0, 29, "60 0E 44 20 17 7C 00", "ECI-29 H1 (GB 2312)" },
        /*  9*/ { UNICODE_MODE, 26, "β", 0, 26, "60 0D 05 6B 17 7C 00", "ECI-26 H1 (UTF-8)" },
        /* 10*/ { DATA_MODE, 0, "β", 0, 0, "0B 56 2F 78 00", "H1 (UTF-8)" },
        /* 11*/ { UNICODE_MODE, 0, "ÿ", 0, 0, "30 01 7F 00", "B1 (ISO 8859-1)" },
        /* 12*/ { UNICODE_MODE, 0, "ÿÿÿ", 0, 0, "30 05 7F 7F 7F 60", "B3 (ISO 8859-1)" },
        /* 13*/ { UNICODE_MODE, 0, "㈩一", 0, 0, "08 15 68 0E 7F 70 00", "H2 (GB 2312)" },
        /* 14*/ { UNICODE_MODE, 29, "㈩一", 0, 29, "60 0E 44 0A 74 07 3F 78 00", "ECI-29 H2 (GB 2312)" },
        /* 15*/ { DATA_MODE, 0, "\177\177", 0, 0, "30 02 7F 3F 40", "B2 (ASCII)" },
        /* 16*/ { DATA_MODE, 0, "\177\177\177", 0, 0, "30 04 7F 3F 5F 60", "B3 (ASCII)" },
        /* 17*/ { UNICODE_MODE, 0, "123", 0, 0, "10 1E 7F 68", "N3 (ASCII)" },
        /* 18*/ { UNICODE_MODE, 0, " 123", 0, 0, "11 7A 03 6F 7D 00", "N4 (ASCII)" },
        /* 19*/ { UNICODE_MODE, 0, "1+23", 0, 0, "11 7B 03 6F 7D 00", "N4 (ASCII)" },
        /* 20*/ { UNICODE_MODE, 0, "12.3", 0, 0, "11 7C 63 6F 7D 00", "N4 (ASCII)" },
        /* 21*/ { UNICODE_MODE, 0, "123,", 0, 0, "10 1E 7F 73 76 5E 60", "N3 L1 (ASCII)" },
        /* 22*/ { UNICODE_MODE, 0, "123,4", 0, 0, "14 1E 7F 51 48 3F 50", "N5 (ASCII)" },
        /* 23*/ { UNICODE_MODE, 0, "\015\012123", 0, 0, "11 7D 63 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 24*/ { UNICODE_MODE, 0, "1\015\01223", 0, 0, "11 7E 03 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 25*/ { UNICODE_MODE, 0, "12\015\0123", 0, 0, "11 7E 23 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 26*/ { UNICODE_MODE, 0, "123\015\012", 0, 0, "10 1E 7F 7C 01 06 42 40", "N3 B2 (ASCII) (EOL)" },
        /* 27*/ { UNICODE_MODE, 0, "123\015\0124", 0, 0, "14 1E 7F 5D 48 3F 50", "N5 (ASCII) (EOL)" },
        /* 28*/ { UNICODE_MODE, 0, "2.2.0", 0, 0, "15 7C 46 73 78 40 07 7A", "N5 (ASCII)" },
        /* 29*/ { UNICODE_MODE, 0, "2.2.0.5", 0, 0, "30 0C 32 17 0C 45 63 01 38 6A 00", "B7 (ASCII)" },
        /* 30*/ { UNICODE_MODE, 0, "2.2.0.56", 0, 0, "13 7C 46 73 78 40 07 71 46 0F 74", "N8 (ASCII)" },
        /* 31*/ { UNICODE_MODE, 0, "1 1234ABCD12.2abcd-12", 0, 0, "13 7A 23 41 2A 3F 68 01 08 3E 4F 66 1E 5F 70 00 44 1F 2F 6E 0F 0F 74", "N6 U4 N4 L4 N3 (ASCII)" },
        /* 32*/ { UNICODE_MODE, 0, "1 123ABCDE12.2abcd-12", 0, 0, "28 1F 40 42 06 28 59 43 27 01 05 7D 56 42 49 16 34 7F 6D 30 08 2F 60", "M21 (ASCII)" },
        /* 33*/ { UNICODE_MODE, 0, "国外通信教材 Matlab6.5", 0, 0, "09 63 27 20 4E 24 1F 05 21 58 22 13 7E 1E 4C 78 09 56 00 3D 3F 4A 45 3F 50", "H6 U2 L5 N3 (GB 2312) (Same as D.2 example)" },
        /* 34*/ { UNICODE_MODE, 0, "AAT", 0, 0, "20 00 4F 30", "U3 (ASCII)" },
        /* 35*/ { UNICODE_MODE, 0, "aat", 0, 0, "18 00 4F 30", "L3 (ASCII)" },
        /* 36*/ { UNICODE_MODE, 0, "AAT2556", 0, 0, "20 00 4F 58 7F 65 47 7A", "U3 N4 (ASCII) (note same bit count as M7)" },
        /* 37*/ { UNICODE_MODE, 0, "AAT2556 ", 0, 0, "29 22 4E 42 0A 14 37 6F 60", "M8 (ASCII)" },
        /* 38*/ { UNICODE_MODE, 0, "AAT2556 电", 0, 0, "29 22 4E 42 0A 14 37 6F 62 2C 1F 7E 00", "M8 H1 (GB 2312)" },
        /* 39*/ { UNICODE_MODE, 0, " 200", 0, 0, "11 7A 06 23 7D 00", "N4 (ASCII)" },
        /* 40*/ { UNICODE_MODE, 0, " 200mA至", 0, 0, "2F 60 40 00 60 2B 78 63 41 7F 40", "M6 H1 (GB 2312)" },
        /* 41*/ { UNICODE_MODE, 0, "2A tel:86 019 82512738", 0, 0, "28 22 5F 4F 29 48 5F 6D 7E 6F 55 57 1F 28 63 0F 5A 11 64 0F 74", "M2 L5(with control) N15 (ASCII)" },
        /* 42*/ { UNICODE_MODE, 0, "至2A tel:86 019 82512738", 0, 0, "30 07 56 60 4C 48 13 6A 32 17 7B 3F 5B 75 35 67 6A 18 63 76 44 39 03 7D 00", "B4 L5(with control) N15 (GB 2312)" },
        /* 43*/ { UNICODE_MODE, 0, "AAT2556 电池充电器＋降压转换器 200mA至2A tel:86 019 82512738", 0, 0, "(62) 29 22 22 1C 4E 41 42 7E 0A 40 14 00 37 7E 6F 00 62 7E 2C 00 1C 7E 4B 00 41 7E 18 00", "M8 H11 M6 B4 L5(with control) N15 (GB 2312) (*NOT SAME* as D3 example, M8 H11 M6 H1 M3 L4(with control) N15, which uses a few more bits)" },
        /* 44*/ { UNICODE_MODE, 0, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::", 0, 0, "(588) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B512 (ASCII)" },
        /* 45*/ { UNICODE_MODE, 0, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\177", 0, 0, "(591) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B513 (ASCII)" },
        /* 46*/ { UNICODE_MODE, 0, ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::至", 0, 0, "(591) 37 68 68 68 68 68 74 7C 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B511 H1 (GB 2312)" },
        /* 47*/ { UNICODE_MODE, 0, ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::至:", 0, 0, "(592) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B513 (GB 2312)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_GRIDMATRIX;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

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
        /*  0*/ { "1234", UNICODE_MODE, -1, -1, 0, 18, 18, "",
                    "111111000000111111"
                    "101111001100101001"
                    "100101000010100001"
                    "111011011100101111"
                    "101011011000111011"
                    "111111000000111111"
                    "000000111111000000"
                    "001100100001001110"
                    "001110111111011110"
                    "001010100011000000"
                    "010100101001000000"
                    "000000111111000000"
                    "111111000000111111"
                    "101001001100101001"
                    "111001000000101111"
                    "111001000100110001"
                    "111111000000100001"
                    "111111000000111111"
               },
        /*  1*/ { "Grid Matrix", UNICODE_MODE, 5, -1, 0, 30, 30, "",
                    "111111000000111111000000111111"
                    "110111010110110111010110110011"
                    "100011011110111111011110111111"
                    "110001000000100001000000100001"
                    "101101000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "010000101111001000101101010000"
                    "000100111111000000110011010110"
                    "001110100001011000111111001000"
                    "001010100001001010111011000010"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "110001001110100101001000110001"
                    "111111011110111111000010111111"
                    "110111000000100101000010100101"
                    "100111000000100011011100100011"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "010100101111001110101101010110"
                    "011100111111000000110001001100"
                    "001010100001000110110111011110"
                    "000010100001011100110001000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "110101010000110101010010110101"
                    "110111000000110011001010101111"
                    "111101010010110101010010111011"
                    "101111010010100001010010110111"
                    "111111000000111111000000111111"
               },
        /*  2*/ { "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738", UNICODE_MODE, 3, 3, 0, 42, 42, "",
                    "111111000000111111000000111111000000111111"
                    "101101001100101111001010101011001100101101"
                    "110001011010110101010000100011000000100001"
                    "110001001110111111010000101111010010101111"
                    "101101010100100111011000110111011000111101"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001010100111000000100011000110100101001110"
                    "011010101101001100101011000110110001011110"
                    "000010111011001110100101001110110101000000"
                    "000110111111011010100011011100100001000000"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101101000110111111011100111001000010101111"
                    "110111011100111011000010110101010100111111"
                    "100011010000111101001000100001011110100001"
                    "100111010110110111011100110101000110100001"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001010100001011110110011011110100111001110"
                    "010010111001000000100011001110111111011110"
                    "010100111111011000100101010110111111000000"
                    "001010101011000100110011001110111011000000"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101001000000111001011100111011000010101101"
                    "111011001000111001001010101101011110111101"
                    "110011010100101001010010101101001110100001"
                    "100001001000100011011000100101000100100001"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001110100011000110100101000000100001001110"
                    "010000100111001010111101000010100001010100"
                    "010100100111010000101011000000100001000100"
                    "001110100001000110111111001100101001000100"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101011001110101001001100101011001010101111"
                    "100011011010100001011100101001010000110101"
                    "111101000110110001000100100111010110110011"
                    "100001001000110011011110110001000100100101"
                    "111111000000111111000000111111000000111111"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_GRIDMATRIX;
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
