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

//#define TEST_QR_INPUT_GENERATE_EXPECTED 1
//#define TEST_MICROQR_INPUT_GENERATE_EXPECTED 1
//#define TEST_UPNQR_INPUT_GENERATE_EXPECTED 1
//#define TEST_QR_ENCODE_GENERATE_EXPECTED 1
//#define TEST_MICROQR_ENCODE_GENERATE_EXPECTED 1
//#define TEST_MICROQR_ENCODE_GENERATE_EXPECTED 1
//#define TEST_UPNQR_ENCODE_GENERATE_EXPECTED 1

static void test_microqr_options(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_1;
        int option_2;
        int ret_encode;
        float w;
        float h;
        int ret_vector;
        int expected_size;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { "12345", 0, 0, 0, 100, 100, 0, 11 },
        /* 1*/ { "12345", 1, 0, 0, 100, 100, 0, 11 },
        /* 2*/ { "12345", 2, 0, 0, 100, 100, 0, 13 },
        /* 3*/ { "12345", 3, 0, 0, 100, 100, 0, 17 },
        /* 4*/ { "12345", 4, 0, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /* 5*/ { "12345", 0, 1, 0, 100, 100, 0, 11 },
        /* 6*/ { "12345", 0, 2, 0, 100, 100, 0, 13 },
        /* 7*/ { "12345", 0, 3, 0, 100, 100, 0, 15 },
        /* 8*/ { "12345", 0, 4, 0, 100, 100, 0, 17 },
        /* 9*/ { "12345", 0, 5, 0, 100, 100, 0, 11 }, // Size > 4 ignored
        /*10*/ { "12345", 1, 5, 0, 100, 100, 0, 11 }, // Ignored also if ECC given
        /*11*/ { "12345", 1, 1, 0, 100, 100, 0, 11 },
        /*12*/ { "12345", 1, 2, 0, 100, 100, 0, 13 },
        /*13*/ { "12345", 1, 3, 0, 100, 100, 0, 15 },
        /*14*/ { "12345", 1, 4, 0, 100, 100, 0, 17 },
        /*15*/ { "12345", 2, 1, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*16*/ { "12345", 2, 2, 0, 100, 100, -1, 13 },
        /*17*/ { "12345", 2, 3, 0, 100, 100, -1, 15 },
        /*18*/ { "12345", 2, 4, 0, 100, 100, -1, 17 },
        /*19*/ { "12345", 3, 1, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*20*/ { "12345", 3, 2, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*21*/ { "12345", 3, 3, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*22*/ { "12345", 3, 4, 0, 100, 100, -1, 17 },
        /*23*/ { "12345", 4, 4, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*24*/ { "12345", 5, 0, 0, 100, 100, 0, 11 }, // ECC > 4 ignored
        /*25*/ { "12345", 5, 1, 0, 100, 100, 0, 11 }, // Ignored also if size given
        /*26*/ { "123456", 1, 0, 0, 100, 100, 0, 13 },
        /*27*/ { "123456", 1, 1, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*28*/ { "123456", 1, 2, 0, 100, 100, 0, 13 },
        /*29*/ { "ABCDEF", 1, 0, 0, 100, 100, 0, 13 },
        /*30*/ { "ABCDEF", 1, 2, 0, 100, 100, 0, 13 },
        /*31*/ { "ABCDEF", 2, 2, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*32*/ { "ABCDE", 2, 0, 0, 100, 100, 0, 13 },
        /*33*/ { "ABCDEABCDEABCD", 1, 0, 0, 100, 100, 0, 15 }, // 14 alphanumerics
        /*34*/ { "ABCDEABCDEABCD", 1, 3, 0, 100, 100, 0, 15 },
        /*35*/ { "ABCDEABCDEABCD", 2, 3, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*36*/ { "ABCDEABCDEA", 2, 3, 0, 100, 100, 0, 15 }, // 11 alphanumerics
        /*37*/ { "ABCDEFGHIJABCDEFGHIJA", 1, 0, 0, 100, 100, 0, 17 }, // 21 alphanumerics
        /*38*/ { "ABCDEFGHIJABCDEFGHIJA", 1, 4, 0, 100, 100, 0, 17 },
        /*39*/ { "ABCDEFGHIJABCDEFGHIJA", 2, 4, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*40*/ { "ABCDEFGHIJABCDEFGH", 2, 4, 0, 100, 100, 0, 17 }, // 18 alphanumerics
        /*41*/ { "ABCDEFGHIJABCDEFGH", 3, 4, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*42*/ { "ABCDEFGHIJABC", 3, 4, 0, 100, 100, 0, 17 }, // 13 alphanumerics
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
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

static void test_qr_input(void)
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
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS, UTF-8 E0B881
    // Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447, UTF-8 D096
    // ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS, UTF-8 E0BA81
    // ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5
    // ･ U+FF65 half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xA5 (\245), UTF-8 EFBDA5
    // ¿ U+00BF in ISO 8859-1 0xBF (\277), not in Shift JIS; 0xBF same codepoint as single-byte half-width katakana ｿ (U+FF7F) in Shift JIS (below), UTF-8 C2BF
    // ｿ U+FF7F half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xBF (\277), UTF-8 EFBDBF
    // ‾ U+203E overline, not in ISO/Win, in Shift JIS single-byte 0x7E (\176) (tilde), UTF-8 E280BE
    // 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9
    // 茗 U+8317 kanji, in Shift JIS 0xE4AA (\344\252), UTF-8 E88C97
    // テ U+30C6 katakana, in Shift JIS 0x8365 (\203\145), UTF-8 E38386
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, "é", 0, 0, "40 1E 90 EC 11 EC 11 EC 11", "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, "é", 0, 3, "70 34 01 E9 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 20, "é", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "é not in Shift JIS" },
        /*  3*/ { UNICODE_MODE, 26, "é", 0, 26, "71 A4 02 C3 A9 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /*  4*/ { DATA_MODE, 0, "é", 0, 0, "40 2C 3A 90 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /*  5*/ { DATA_MODE, 0, "\351", 0, 0, "40 1E 90 EC 11 EC 11 EC 11", "B1 (ISO 8859-1)" },
        /*  6*/ { UNICODE_MODE, 0, "β", 0, 0, "80 11 00 00 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /*  7*/ { UNICODE_MODE, 9, "β", 0, 9, "70 94 01 E2 00 EC 11 EC 11", "ECI-9 B1 (ISO 8859-7)" },
        /*  8*/ { UNICODE_MODE, 20, "β", 0, 20, "71 48 01 10 00 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /*  9*/ { UNICODE_MODE, 26, "β", 0, 26, "71 A4 02 CE B2 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 10*/ { DATA_MODE, 0, "β", 0, 0, "40 2C EB 20 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 11*/ { UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 13, "Warning 70 D4 01 A1 00 EC 11 EC 11", "ECI-13 B1 (ISO 8859-11)" },
        /* 12*/ { UNICODE_MODE, 13, "ก", 0, 13, "70 D4 01 A1 00 EC 11 EC 11", "ECI-13 B1 (ISO 8859-11)" },
        /* 13*/ { UNICODE_MODE, 20, "ก", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "ก not in Shift JIS" },
        /* 14*/ { UNICODE_MODE, 26, "ก", 0, 26, "71 A4 03 E0 B8 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 15*/ { DATA_MODE, 0, "ก", 0, 0, "40 3E 0B 88 10 EC 11 EC 11", "B3 (UTF-8)" },
        /* 16*/ { UNICODE_MODE, 0, "Ж", 0, 0, "80 11 23 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 17*/ { UNICODE_MODE, 7, "Ж", 0, 7, "70 74 01 B6 00 EC 11 EC 11", "ECI-7 B1 (ISO 8859-5)" },
        /* 18*/ { UNICODE_MODE, 20, "Ж", 0, 20, "71 48 01 12 38 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 19*/ { UNICODE_MODE, 26, "Ж", 0, 26, "71 A4 02 D0 96 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 20*/ { DATA_MODE, 0, "Ж", 0, 0, "40 2D 09 60 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 21*/ { UNICODE_MODE, 0, "ກ", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E0 BA 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 22*/ { UNICODE_MODE, 20, "ກ", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "ກ not in Shift JIS" },
        /* 23*/ { UNICODE_MODE, 26, "ກ", 0, 26, "71 A4 03 E0 BA 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 24*/ { DATA_MODE, 0, "ກ", 0, 0, "40 3E 0B A8 10 EC 11 EC 11", "B3 (UTF-8)" },
        /* 25*/ { UNICODE_MODE, 0, "\\", 0, 0, "40 15 C0 EC 11 EC 11 EC 11", "B1 (ASCII)" },
        /* 26*/ { UNICODE_MODE, 20, "\\", 0, 20, "71 48 01 00 F8 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 27*/ { UNICODE_MODE, 20, "[", 0, 20, "71 44 01 5B 00 EC 11 EC 11", "B1 (ASCII)" },
        /* 28*/ { UNICODE_MODE, 20, "\177", 0, 20, "71 44 01 7F 00 EC 11 EC 11", "ECI-20 B1 (ASCII)" },
        /* 29*/ { UNICODE_MODE, 0, "¥", 0, 0, "40 1A 50 EC 11 EC 11 EC 11", "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous)" },
        /* 30*/ { UNICODE_MODE, 3, "¥", 0, 3, "70 34 01 A5 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /* 31*/ { UNICODE_MODE, 20, "¥", 0, 20, "71 44 01 5C 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) (to single-byte backslash codepoint 5C, so byte mode)" },
        /* 32*/ { UNICODE_MODE, 26, "¥", 0, 26, "71 A4 02 C2 A5 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 33*/ { DATA_MODE, 0, "¥", 0, 0, "40 2C 2A 50 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 34*/ { UNICODE_MODE, 0, "･", 0, 0, "40 1A 50 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /* 35*/ { UNICODE_MODE, 3, "･", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 36*/ { UNICODE_MODE, 20, "･", 0, 20, "71 44 01 A5 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) single-byte codepoint A5" },
        /* 37*/ { UNICODE_MODE, 26, "･", 0, 26, "71 A4 03 EF BD A5 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 38*/ { DATA_MODE, 0, "･", 0, 0, "40 3E FB DA 50 EC 11 EC 11", "B3 (UTF-8)" },
        /* 39*/ { UNICODE_MODE, 0, "¿", 0, 0, "40 1B F0 EC 11 EC 11 EC 11", "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous)" },
        /* 40*/ { UNICODE_MODE, 3, "¿", 0, 3, "70 34 01 BF 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /* 41*/ { UNICODE_MODE, 20, "¿", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "¿ not in Shift JIS" },
        /* 42*/ { UNICODE_MODE, 26, "¿", 0, 26, "71 A4 02 C2 BF 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 43*/ { DATA_MODE, 0, "¿", 0, 0, "40 2C 2B F0 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 44*/ { UNICODE_MODE, 0, "ｿ", 0, 0, "40 1B F0 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint BF (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 45*/ { UNICODE_MODE, 3, "ｿ", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 46*/ { UNICODE_MODE, 20, "ｿ", 0, 20, "71 44 01 BF 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) single-byte codepoint BF" },
        /* 47*/ { UNICODE_MODE, 26, "ｿ", 0, 26, "71 A4 03 EF BD BF 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 48*/ { DATA_MODE, 0, "ｿ", 0, 0, "40 3E FB DB F0 EC 11 EC 11", "B3 (UTF-8)" },
        /* 49*/ { UNICODE_MODE, 0, "~", 0, 0, "40 17 E0 EC 11 EC 11 EC 11", "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 50*/ { UNICODE_MODE, 3, "~", 0, 3, "70 34 01 7E 00 EC 11 EC 11", "ECI-3 B1 (ASCII)" },
        /* 51*/ { UNICODE_MODE, 20, "~", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "tilde not in Shift JIS (codepoint used for overline)" },
        /* 52*/ { UNICODE_MODE, 0, "‾", 0, 0, "40 17 E0 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint 7E (same bytes as ~ ASCII above, so ambiguous)" },
        /* 53*/ { UNICODE_MODE, 3, "‾", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 54*/ { UNICODE_MODE, 20, "‾", 0, 20, "71 44 01 7E 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) (to single-byte tilde codepoint 7E, so byte mode)" },
        /* 55*/ { UNICODE_MODE, 26, "‾", 0, 26, "71 A4 03 E2 80 BE 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 56*/ { DATA_MODE, 0, "‾", 0, 0, "40 3E 28 0B E0 EC 11 EC 11", "B3 (UTF-8)" },
        /* 57*/ { UNICODE_MODE, 0, "点", 0, 0, "80 16 CF 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 58*/ { UNICODE_MODE, 3, "点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 59*/ { UNICODE_MODE, 20, "点", 0, 20, "71 48 01 6C F8 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 60*/ { UNICODE_MODE, 26, "点", 0, 26, "71 A4 03 E7 82 B9 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 61*/ { DATA_MODE, 0, "点", 0, 0, "40 3E 78 2B 90 EC 11 EC 11", "B3 (UTF-8)" },
        /* 62*/ { DATA_MODE, 0, "\223\137", 0, 0, "80 16 CF 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 63*/ { UNICODE_MODE, 0, "¥･点", 0, 0, "40 45 CA 59 35 F0 EC 11 EC", "B4 (Shift JIS) (optimized to byte mode only)" },
        /* 64*/ { UNICODE_MODE, 3, "¥･点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 65*/ { UNICODE_MODE, 20, "¥･点", 0, 20, "71 44 04 5C A5 93 5F 00 EC", "ECI-20 B4 (Shift JIS)" },
        /* 66*/ { UNICODE_MODE, 26, "¥･点", 0, 26, "71 A4 08 C2 A5 EF BD A5 E7 82 B9 00 EC", "ECI-26 B8 (UTF-8)" },
        /* 67*/ { DATA_MODE, 0, "\134\245\223\137", 0, 0, "40 45 CA 59 35 F0 EC 11 EC", "B8 (Shift JIS)" },
        /* 68*/ { DATA_MODE, 0, "¥･点", 0, 0, "40 8C 2A 5E FB DA 5E 78 2B 90 EC 11 EC", "B8 (UTF-8)" },
        /* 69*/ { UNICODE_MODE, 0, "点茗", 0, 0, "80 26 CF EA A8 00 EC 11 EC", "K2 (Shift JIS)" },
        /* 70*/ { UNICODE_MODE, 0, "点茗テ", 0, 0, "80 36 CF EA A8 34 A0 EC 11", "K3 (Shift JIS)" },
        /* 71*/ { UNICODE_MODE, 0, "点茗テ点", 0, 0, "80 46 CF EA A8 34 AD 9F 00", "K4 (Shift JIS)" },
        /* 72*/ { UNICODE_MODE, 0, "点茗テ点茗", 0, 0, "80 56 CF EA A8 34 AD 9F D5 50 00 EC 11", "K5 (Shift JIS)" },
        /* 73*/ { UNICODE_MODE, 0, "点茗テ点茗テ", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 40 EC", "K6 (Shift JIS)" },
        /* 74*/ { UNICODE_MODE, 0, "点茗テ点茗テｿ", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 50 06 FC 00 EC", "K6 B1 (Shift JIS)" },
        /* 75*/ { DATA_MODE, 0, "\223\137\344\252\203\145\223\137\344\252\203\145\277", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 50 06 FC 00 EC", "K6 B1 (Shift JIS)" },
        /* 76*/ { DATA_MODE, 0, "点茗テ点茗テｿ", 0, 0, "41 5E 78 2B 9E 88 C9 7E 38 38 6E 78 2B 9E 88 C9 7E 38 38 6E FB DB F0 EC 11 EC 11 EC", "B21 (UTF-8)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_QRCODE;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_QR_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilEscape(data[i].data, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
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

static void test_microqr_input(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int ret;
        char* expected;
        char* comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS, UTF-8 E0B881
    // Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447 (\204\107), UTF-8 D096
    // ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS, UTF-8 E0BA81
    // ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5
    // ･ U+FF65 half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xA5 (\245), UTF-8 EFBDA5
    // ¿ U+00BF in ISO 8859-1 0xBF (\277), not in Shift JIS; 0xBF same codepoint as single-byte half-width katakana ｿ (U+FF7F) in Shift JIS (below), UTF-8 C2BF
    // ｿ U+FF7F half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xBF (\277), UTF-8 EFBDBF
    // ‾ U+203E overline, not in ISO/Win, in Shift JIS single-byte 0x7E (\176) (tilde), UTF-8 E280BE
    // 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9
    // 茗 U+8317 kanji, in Shift JIS 0xE4AA (\344\252), UTF-8 E88C97
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "é", 0, "87 A4 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1)" },
        /*  1*/ { DATA_MODE, "é", 0, "8B 0E A4 00 EC 11 EC 11 00", "B2 (UTF-8)" },
        /*  2*/ { UNICODE_MODE, "β", 0, "C8 80 00 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /*  3*/ { UNICODE_MODE, "ก", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input data", "ก not in Shift JIS" },
        /*  4*/ { UNICODE_MODE, "Ж", 0, "C8 91 C0 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /*  5*/ { UNICODE_MODE, "ກ", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input data", "ກ not in Shift JIS" },
        /*  6*/ { UNICODE_MODE, "\\", 0, "85 70 00 EC 11 EC 11 EC 00", "B1 (ASCII)" },
        /*  7*/ { UNICODE_MODE, "¥", 0, "86 94 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous)" },
        /*  8*/ { UNICODE_MODE, "･", 0, "86 94 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /*  9*/ { UNICODE_MODE, "¿", 0, "86 FC 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous)" },
        /* 10*/ { UNICODE_MODE, "ｿ", 0, "86 FC 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 11*/ { UNICODE_MODE, "~", 0, "85 F8 00 EC 11 EC 11 EC 00", "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 12*/ { UNICODE_MODE, "‾", 0, "85 F8 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) (same bytes as ~ ASCII above, so ambiguous)" },
        /* 13*/ { UNICODE_MODE, "点", 0, "CB 67 C0 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 14*/ { DATA_MODE, "\223\137", 0, "CB 67 C0 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 15*/ { DATA_MODE, "点", 0, "CF 30 A1 B9 00 EC 11 EC 00", "K1 B1 (UTF-8) (1st 2 UTF-8 bytes E782 encoded in Kanji mode) (non-optimal)" },
        /* 16*/ { UNICODE_MODE, "茗", 0, "CE AA 80 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 17*/ { DATA_MODE, "\344\252", 0, "CE AA 80 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 18*/ { DATA_MODE, "茗", 0, "CF 63 21 97 00 EC 11 EC 00", "K1 B1 (UTF-8) (1st 2 UTF-8 bytes E88C encoded in Kanji mode) (non-optimal)" },
        /* 19*/ { UNICODE_MODE, "¥点", 0, "85 73 2D 9F 00 EC 11 EC 00", "B1 K1 (Shift JIS) (non-optimal)" },
        /* 20*/ { DATA_MODE, "\134\223\137", 0, "85 73 2D 9F 00 EC 11 EC 00", "B1 K1 (Shift JIS) (non-optimal)" },
        /* 21*/ { DATA_MODE, "¥点", 0, "8B 0A 97 3C C2 86 E4 00 00", "B2 K1 B1 (UTF-8) (last 2 UTF-8 bytes 82B9 encoded in Kanji mode) (non-optimal)" },
        /* 22*/ { UNICODE_MODE, "点茗", 0, "D3 67 F5 54 00 EC 11 EC 00", "K2 (Shift JIS)" },
        /* 23*/ { DATA_MODE, "\223\137\344\252", 0, "D3 67 F5 54 00 EC 11 EC 00", "K2 (Shift JIS)" },
        /* 24*/ { DATA_MODE, "点茗", 0, "CF 30 A1 B9 CF 63 21 97 00", "K1 B1 K1 B1 (UTF-8) (non-optimal)" },
        /* 25*/ { UNICODE_MODE, "点茗･", 0, "D3 67 F5 55 0D 28 00 EC 00", "K2 B1 (Shift JIS) (non-optimal)" },
        /* 26*/ { DATA_MODE, "\223\137\344\252\245", 0, "D3 67 F5 55 0D 28 00 EC 00", "K2 B1 (Shift JIS) (non-optimal)" },
        /* 27*/ { DATA_MODE, "点茗･", 0, "63 CC 24 1B 96 5D 8C 89 7A 15 ED 28 00 EC", "K1 B1 K2 B2 (UTF-8) (non-optimal)" },
        /* 28*/ { UNICODE_MODE, "¥点茗･", 0, "85 73 4D 9F D5 54 34 A0 00", "B1 K2 B1 (Shift JIS) (non-optimal)" },
        /* 29*/ { DATA_MODE, "\134\223\137\344\252\245", 0, "85 73 4D 9F D5 54 34 A0 00", "B1 K2 B1 (Shift JIS) (non-optimal)" },
        /* 30*/ { DATA_MODE, "¥点茗･", 0, "42 C2 A5 63 CC 24 1B 96 5D 8C 89 7A 15 ED 28 00", "B2 K1 B1 K2 B2 (UTF-8) (non-optimal)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
        symbol->input_mode = data[i].input_mode;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_MICROQR_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_upnqr_input(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int ret;
        char* expected;
        char* comment;
    };
    // Ą U+0104 in ISO 8859-2 0xA1, in other ISO 8859 and Win 1250, UTF-8 C484
    // Ŕ U+0154 in ISO 8859-2 0xC0, in Win 1250 but not other ISO 8859 or Win page, UTF-8 C594
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "ĄŔ", 0, "70 44 00 02 A1 C0 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11", "ECI-4 B2 (ISO 8859-2)" },
        /*  1*/ { UNICODE_MODE, "é", 0, "70 44 00 01 E9 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC", "ECI-4 B1 (ISO 8859-2)" },
        /*  2*/ { UNICODE_MODE, "β", ZINT_ERROR_INVALID_DATA, "Error 572: Invalid characters in input data", "β not in ISO 8859-2" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_UPNQR;
        symbol->input_mode = data[i].input_mode;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);
        assert_equal(symbol->eci, 4, "i:%d ZBarcode_Encode symbol->eci %d != 4\n", i, symbol->eci);

        #ifdef TEST_UPNQR_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_encode(void)
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
        /*  0*/ { "12345678901234567890123456789012345678901", UNICODE_MODE, -1, -1, 0, 21, 21, "Max capacity Version 1 41 numbers",
                    "111111100011001111111"
                    "100000100001001000001"
                    "101110101110001011101"
                    "101110100001001011101"
                    "101110100011001011101"
                    "100000100100001000001"
                    "111111101010101111111"
                    "000000001100100000000"
                    "111011111000011000100"
                    "111000000110111000001"
                    "001001111011110001000"
                    "001110011011011111001"
                    "001100101000001000011"
                    "000000001101100001010"
                    "111111101011011001011"
                    "100000101100010001011"
                    "101110101101010111100"
                    "101110100000111000010"
                    "101110101001100011101"
                    "100000101010111011000"
                    "111111101000100010101"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_QRCODE;
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

        #ifdef TEST_QR_ENCODE_GENERATE_EXPECTED
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

static void test_microqr_encode(void)
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
        /*  0*/ { "12345", UNICODE_MODE, -1, -1, 0, 11, 11, "Max capacity M1 5 numbers",
                    "11111110101"
                    "10000010110"
                    "10111010100"
                    "10111010000"
                    "10111010111"
                    "10000010011"
                    "11111110100"
                    "00000000011"
                    "11001110011"
                    "01010001100"
                    "11110000011"
               },
        /*  1*/ { "点茗テ点茗テ点茗テ", UNICODE_MODE, -1, -1, 0, 17, 17, "Max capacity M4 9 Kanji",
                    "11111110101010101"
                    "10000010111110010"
                    "10111010000011101"
                    "10111010110011010"
                    "10111010001011001"
                    "10000010110110110"
                    "11111110101100100"
                    "00000000010011011"
                    "10010111011110100"
                    "00001010100100110"
                    "11101010001000010"
                    "00010111101000010"
                    "11000100010110000"
                    "01001111010011101"
                    "10000100101100011"
                    "01011000000010111"
                    "11001111011101001"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
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

        #ifdef TEST_MICROQR_ENCODE_GENERATE_EXPECTED
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

static void test_upnqr_encode(void)
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
        /*  0*/ { "Ą˘Ł¤ĽŚ§¨ŠŞŤŹŽŻ°ą˛ł´ľśˇ¸šşťź˝žżŔÁÂĂÄĹĆÇČÉĘËĚÍÎĎĐŃŇÓÔŐÖ×ŘŮÚŰÜÝŢßŕáâăäĺćçčéęëěíîďđńňóôőö÷řůúűüýţ˙", UNICODE_MODE, -1, -1, 0, 77, 77, "ISO 8859-2",
                    "11111110000111101001000110101100101001111110111011001111111000110000001111111"
                    "10000010001011100100110111111011110100001011110000100001001110011010101000001"
                    "10111010110101111111101101111101001010110101111011011110100001100100101011101"
                    "10111010100110111001110101101000011101101000110011100111001111101000101011101"
                    "10111010100110010010011011111101001001111101101111101001101111001011101011101"
                    "10000010101011011010010010001010010101101010111000100011001110100010001000001"
                    "11111110101010101010101010101010101010101010101010101010101010101010101111111"
                    "00000000110101001100101010001100001011110010011000111100111000000000100000000"
                    "10111110010111011110010011111001111100101111111111100001010110011010001111100"
                    "10000001110110110001011001111100101000111010111001001101111100111000111100000"
                    "11111110110010111100110011101011110100001111110110101111010110000010001011010"
                    "00001100011100001010101111010101001111110100011101010110101000110000010100000"
                    "11010011100111011010010001110000010010101001110101111101001110000010101001010"
                    "10000100100110010010011000011101010111111100111011010111100100100001011100000"
                    "11101110110010111100110111110010001010001011110000110001011110000011101011010"
                    "00001100011110101011101111011101101010010010011011011010110000111001110100000"
                    "11011111111100011010010101110001111101001111110011000011010110000010001001010"
                    "10010000100101010010011100001100101000011010111000101001111100101000111100010"
                    "11110110111100011100110011100011110101001111110110101011110110000010001111001"
                    "00111100010001101011101111000010101111100010001101110100011000111100110010011"
                    "11000111111100011010010111101111011010110111100110111100110110000111111111001"
                    "10000000111001110010011100010010001111100010101011110110011100101010111010000"
                    "11111110100100111100110001101111110010010111110011111000010110000001110010010"
                    "00111000000011101011111101001011010100010010000101011010111000111101011100000"
                    "11111111110110011010001011111110000001001111101111101011010110000001111110010"
                    "11101000100001111010011010001011010000011010101000111001111100101111100011100"
                    "10001010111100101100110110101110000101001111111010100011110100000101101010010"
                    "01011000101001110011101110001011001000000011011000111000011011111100100010101"
                    "01101111101010001010011011111110011011010110011111111010110001000100111111001"
                    "01100101111101101010010101101011001010000011000101100000011011001101101001110"
                    "01010110111100110000110110010110010101010110010101001010110010000010010101111"
                    "11110000101111101011011001101011010110001010000000110000011001111111001010010"
                    "01000010001010001010001111001110000011000111110110011011110110100100110110101"
                    "10110101111001101010100101101011010110011010100110010000111100101110101101110"
                    "01111110011100110001111110010110000011001111101000111011010111001000001100100"
                    "01111101101001101001101001001011010110011001011110010000001101110011100101110"
                    "00011110001000001001011110001110000011001100001101111010100101111010100000000"
                    "00110001111011111101110101101011010110001101010111110001000101110001110010010"
                    "00111111111101100001110110010110000011011010010001011011101100110010000101001"
                    "11111100101101010011000001101111010110000101100110110000000001011101110110011"
                    "10011110101001010101000111101100000011010000010100011010111001100100100011001"
                    "00110000111011111100100100001011010110000110100110010000011001001111101001110"
                    "00111111111100101111110110110110000011010111101000111010101100100000011000101"
                    "00111100101101001100100001001100010111100010111110010100011110000111101111111"
                    "00011110101000110101001110001101100010010110101100111110110110001100110010001"
                    "00110001011010010000110111100000110110000011110110010000010110101111110111110"
                    "11111111111100101001111011111011100010010110101111111110110100001000111110101"
                    "00111000101101001011000110001111010111100101001000110110000011110110100011111"
                    "00011010110001110111001110101000000010110000001010111000101001111101101010001"
                    "00111000100011110100110010001001010110100101011000110100000011011111100011110"
                    "11111111100101101100111011111000000011010010001111110110111000101000111110101"
                    "00111101101101001000000100110000001010000011011000111000010011001111100011111"
                    "00011111100000110000101100100101110101010110000110000010110001110101010110001"
                    "00111100100011110000110010000100100000000011001100101000011011000110100011110"
                    "11000010000101001110011100001101110101010110000110001010110000101001010110101"
                    "00101100101110101001000110101000101111100101011000110100001011010111100011111"
                    "00011110001010110000101011101111111010110000000110011100101001111101110110001"
                    "00011100001000010000110010100000101111100101001100110110000011010110000011100"
                    "11000011000101101110011100010011100010010010000100010000011000111000110110110"
                    "00000101111010001001000010001001010010010101011010111010100011010110000011100"
                    "00011110011001110000101100100101000101001000000000000011001001111110010110010"
                    "00111100000101110000110101000000010000011101001010101001100011010010100011010"
                    "11100110111101101110001011001111100101001010000010000011111000111101010111001"
                    "00010001110110000001001101010000001001100101011000111100000011010111100011111"
                    "10010010000001111000111010111101111000110000000110001100101001111001010110101"
                    "10010000000111110000111011001000101001100101000101111110000011010001100011110"
                    "01001110100011111110011101000111110100010010001101001000011010111010110110010"
                    "00001001111000001001000001001000010111110100010010100110100001010111100010101"
                    "01111010010011110000101011111101100010101001101111111101001111011101111110101"
                    "00000000110101101000111110001000110011111100101000100111100101010101100011101"
                    "11111110001101111010001010101111100100001011101010100001011101011010101011100"
                    "10000010111110000001101010001000001011111101001000111110000011010001100010010"
                    "10111010110101110101011111111101111100100000011111100101101000111011111111101"
                    "10111010110001100110111110110000101001111101010001101110100010110000010010010"
                    "10111010101011111111010111011111110100001010011011000001111000110011100101010"
                    "10000010011100010001111111000000001011100000001000111111100001011101111000001"
                    "11111110110101111101111001011101111100101101111101100101000101100100011101000"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_UPNQR;
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

        #ifdef TEST_UPNQR_ENCODE_GENERATE_EXPECTED
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
    test_microqr_options();
    test_qr_input();
    test_microqr_input();
    test_upnqr_input();
    test_qr_encode();
    test_microqr_encode();
    test_upnqr_encode();

    testReport();

    return 0;
}
