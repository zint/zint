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
/* vim: set ts=4 sw=4 et norl : */

#include "testcommon.h"

static void test_large(int index, int debug) {

    struct item {
        int option_1;
        int option_2;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -1, -1, "1", 7827, 0, 189, 189 },
        /*  1*/ { -1, -1, "1", 7828, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { -1, -1, "A", 4349, 0, 189, 189 }, // TODO: should be 4350 according to spec, investigate
        /*  3*/ { -1, -1, "A", 4350, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { -1, -1, "\200", 3260, 0, 189, 189 }, // TODO: should be 3261 according to spec, investigate
        /*  5*/ { -1, -1, "\200", 3261, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { -1, 1, "1", 42, 0, 23, 23 },
        /*  7*/ { -1, 1, "1", 43, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { -1, 1, "A", 25, 0, 23, 23 },
        /*  9*/ { -1, 1, "A", 26, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { -1, 1, "\200", 17, 0, 23, 23 },
        /* 11*/ { -1, 1, "\200", 18, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 12*/ { 2, 1, "A", 19, 0, 23, 23 },
        /* 13*/ { 2, 1, "A", 20, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { 3, 1, "A", 14, 0, 23, 23 },
        /* 15*/ { 3, 1, "A", 15, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 16*/ { 4, 1, "A", 9, 0, 23, 23 },
        /* 17*/ { 4, 1, "A", 10, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[7829];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_HANXIN, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_options(int index, int debug) {

    struct item {
        int option_1;
        int option_2;
        char *data;
        int ret_encode;
        int ret_vector;
        int expected_size;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -1, -1, "12345", 0, 0, 23 }, // Default version 1, ECC auto-set to 4
        /*  1*/ { 1, -1, "12345", 0, 0, 23 },
        /*  2*/ { -1, 2, "12345", 0, 0, 25 },
        /*  3*/ { -1, 85, "12345", 0, 0, 23 }, // Version > max version 85 so ignored
        /*  4*/ { -1, 84, "12345", 0, 0, 189 },
        /*  5*/ { 1, 1, "1234567890123456789012345678901234567890123", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { 4, 1, "1234567890123456", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  7*/ { 4, 2, "12345678901234567", 0, 0, 25 },
        /*  8*/ { 4, -1, "12345678901234567", 0, 0, 25 }, // Version auto-set to 2
        /*  9*/ { -1, -1, "12345678901234567", 0, 0, 23 }, // Version auto-set to 1, ECC auto-set to 3
        /* 10*/ { 5, -1, "12345678901234567", 0, 0, 23 }, // ECC > max ECC 4 so ignored and auto-settings version 1, ECC 3 used
        /* 11*/ { -1, -1, "1234567890123456789012345678901234567890123", 0, 0, 25 }, // Version auto-set to 2, ECC auto-set to 2
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_HANXIN, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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

static void test_input(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int eci;
        int option_3;
        char *data;
        int length;
        int ret;
        int expected_eci;
        char *expected;
        char *comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 18030 0xA8A6, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek 0xE2 (but not other ISO 8859 or Win page), in GB 18030 0xA6C2, UTF-8 CEB2
    // ÿ U+00FF in ISO 8859-1 0xFF, not in GB 18030, outside first byte and second byte range, UTF-8 C3BF
    // PAD U+0080 GB 18030 4-byte Region 0x81308130, UTF-8 C280 (\302\200)
    // 啊 U+554A GB 18030 Region One 0xB0A1, UTF-8 E5958A
    // 亍 U+4E8D GB 18030 Region Two 0xD8A1, UTF-8 E4BA8D
    // 齄 U+9F44 GB 18030 Region Two 0xF7FE, UTF-8 E9BD84
    // 丂 U+4E02 GB 18030 2-byte Region 0x8140, UTF-8 E4B882
    // � (REPLACEMENT CHARACTER) U+FFFD GB 18030 4-byte Region 0x81308130, UTF-8 EFBFBD (\357\277\275)
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, "é", -1, 0, 0, "30 00 F4 80 00 00 00 00 00", "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, -1, "é", -1, 0, 3, "80 33 00 0F 48 00 00 00 00", "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 29, -1, "é", -1, 0, 29, "81 D4 FC FF FF 00 00 00 00", "ECI-29 H(1)1 (GB 18030) (Region One)" },
        /*  3*/ { UNICODE_MODE, 26, -1, "é", -1, 0, 26, "81 A3 00 16 1D 48 00 00 00", "ECI-26 B2 (UTF-8)" },
        /*  4*/ { UNICODE_MODE, 26, ZINT_FULL_MULTIBYTE, "é", -1, 0, 26, "81 A4 70 2F FF 00 00 00 00", "ECI-26 H(1)1 (Region One) (UTF-8) (full multibyte)" },
        /*  5*/ { DATA_MODE, 0, -1, "é", -1, 0, 0, "30 01 61 D4 80 00 00 00 00", "B2 (UTF-8)" },
        /*  6*/ { DATA_MODE, 0, ZINT_FULL_MULTIBYTE, "é", -1, 0, 0, "47 02 FF F0 00 00 00 00 00", "H(1)1 (UTF-8) (Region One) (full multibyte)" },
        /*  7*/ { DATA_MODE, 0, -1, "\351", -1, 0, 0, "30 00 F4 80 00 00 00 00 00", "B1 (ISO 8859-1) (0xE9)" },
        /*  8*/ { UNICODE_MODE, 0, -1, "β", -1, 0, 0, "30 01 53 61 00 00 00 00 00", "B2 (GB 18030) (2-byte Region)" },
        /*  9*/ { UNICODE_MODE, 9, -1, "β", -1, 0, 9, "80 93 00 0F 10 00 00 00 00", "ECI-9 B1 (ISO 8859-7)" },
        /* 10*/ { UNICODE_MODE, 29, -1, "β", -1, 0, 29, "81 D3 00 15 36 10 00 00 00", "ECI-29 B2 (GB 18030) (2-byte Region)" },
        /* 11*/ { UNICODE_MODE, 26, -1, "β", -1, 0, 26, "81 A3 00 16 75 90 00 00 00", "ECI-26 B2 (UTF-8)" },
        /* 12*/ { UNICODE_MODE, 26, ZINT_FULL_MULTIBYTE, "β", -1, 0, 26, "81 A4 B1 5F FF 00 00 00 00", "ECI-26 B2 (UTF-8) (full multibyte)" },
        /* 13*/ { DATA_MODE, 0, -1, "β", -1, 0, 0, "30 01 67 59 00 00 00 00 00", "B2 (UTF-8)" },
        /* 14*/ { DATA_MODE, 0, ZINT_FULL_MULTIBYTE, "β", -1, 0, 0, "4B 15 FF F0 00 00 00 00 00", "H(1)1 (UTF-8) (Region One) (full multibyte)" },
        /* 15*/ { UNICODE_MODE, 0, -1, "ÿ", -1, 0, 0, "30 00 FF 80 00 00 00 00 00", "B1 (ISO 8859-1)" },
        /* 16*/ { UNICODE_MODE, 0, -1, "ÿÿÿ", -1, 0, 0, "30 01 FF FF FF 80 00 00 00", "B3 (ISO 8859-1)" },
        /* 17*/ { UNICODE_MODE, 0, -1, "\302\200", -1, 0, 0, "70 00 00 00 00 00 00 00 00", "H(f)1 (GB 18030) (4-byte Region) (not DATA_MODE so GB 18030 mapping)" },
        /* 18*/ { UNICODE_MODE, 0, ZINT_FULL_MULTIBYTE, "\302\200", -1, 0, 0, "70 00 00 00 00 00 00 00 00", "H(f)1 (GB 18030) (4-byte Region)" },
        /* 19*/ { DATA_MODE, 0, 0, "\302\200", -1, 0, 0, "30 01 61 40 00 00 00 00 00", "B2 (UTF-8)" },
        /* 20*/ { DATA_MODE, 0, ZINT_FULL_MULTIBYTE, "\302\200", -1, 0, 0, "30 01 61 40 00 00 00 00 00", "B2 (UTF-8) (full multibyte)" },
        /* 21*/ { UNICODE_MODE, 0, -1, "\302\200�", -1, 0, 0, "70 00 00 38 26 7E 40 00 00", "H(f)2 (GB 18030) (both 4-byte Region) (not DATA_MODE so GB 18030 mapping)" },
        /* 22*/ { UNICODE_MODE, 0, ZINT_FULL_MULTIBYTE, "\302\200�", -1, 0, 0, "70 00 00 38 26 7E 40 00 00", "H(f)2 (GB 18030) (both 4-byte Region)" },
        /* 23*/ { UNICODE_MODE, 0, -1, "啊亍齄丂\302\200", -1, 0, 0, "64 68 50 3C AC 28 80 00 FF FE E0 00 00 00 00 00 00", "H(d)4 H(f)1 (GB 18030)" },
        /* 24*/ { DATA_MODE, 0, -1, "\177\177", -1, 0, 0, "2F BD F7 F0 00 00 00 00 00", "T2 (ASCII)" },
        /* 25*/ { DATA_MODE, 0, -1, "\177\177\177", -1, 0, 0, "2F BD F7 DF C0 00 00 00 00", "T3 (ASCII)" },
        /* 26*/ { UNICODE_MODE, 0, -1, "123", -1, 0, 0, "11 EF FF 00 00 00 00 00 00", "N3 (ASCII)" },
        /* 27*/ { UNICODE_MODE, 0, -1, "12345", -1, 0, 0, "11 EC 2D FF 80 00 00 00 00", "N5 (ASCII)" },
        /* 28*/ { UNICODE_MODE, 0, -1, "Aa%$Bb9", -1, 0, 0, "22 A4 FA 18 3E 2E 52 7F 00", "T7 (ASCII)" },
        /* 29*/ { UNICODE_MODE, 0, -1, "Summer Palace Ticket for 6 June 2015 13:00;2015年6月6日夜01時00分PM頤和園のチケット;2015년6월6일13시오후여름궁전티켓.2015年6月6号下午13:00的颐和园门票;", -1, 0, 0, "(189) 27 38 C3 0A 35 F9 CF 99 92 F9 26 A3 E7 3E 76 C9 AE A3 7F CC 08 04 0C CD EE 44 06 C4", "T20 B64 N4 H(f)1 T1 H(f)1 T1 H(f)1 T2 H(f)9 B35 (GB 18030)" },
        /* 30*/ { DATA_MODE, 0, -1, "Summer Palace Ticket for 6 June 2015 13:00;2015年6月6日夜01時00分PM頤和園のチケット;2015년6월6일13시오후여름궁전티켓.2015年6月6号下午13:00的颐和园门票;", -1, 0, 0, "(209) 27 38 C3 0A 35 F9 CF 99 92 F9 26 A3 E7 3E 76 C9 AE A3 7F CC 15 04 0C CD EE 44 06 C4", "T20 B117 (UTF-8)" },
        /* 31*/ { UNICODE_MODE, 0, -1, "\000\014\033 #/059:<@AMZ", 15, 0, 0, "2F 80 31 B7 1F AF E0 05 27 EB 2E CB E2 96 8F F0 00", "T15 (ASCII)" },
        /* 32*/ { UNICODE_MODE, 0, -1, "Z[\\`alz{~\177", -1, 0, 0, "28 FE CF 4E 3E 92 FF 7E E7 CF 7F 00 00", "T10 (ASCII)" },
        /* 33*/ { DATA_MODE, 26, ZINT_FULL_MULTIBYTE, "\202\061\203\063", -1, 0, 26, "81 A7 01 B1 D8 00 00 00 00", "ECI-26 H(f)1 (GB 18030) (Invalid UTF-8, forces GB 2312/18030 utf8tosb() difference) NOTE: 2021-01-10 now UTF-8 is checked and mode -> DATA_MODE this test no longer shows difference" },
        /* 34*/ { UNICODE_MODE, 128, 0, "A", -1, 0, 128, "88 08 02 2B F0 00 00 00 00", "ECI > 127" },
        /* 35*/ { UNICODE_MODE, 16364, 0, "A", -1, 0, 16364, "8B FE C2 2B F0 00 00 00 00", "ECI > 16363" },
        /* 36*/ { UNICODE_MODE, 0, -1, "啊啊啊亍", -1, 0, 0, "40 00 00 00 00 FF E0 00 FF F0 00 00 00", "Region 1 (FFE terminator) -> Region 2 (no indicator)" },
        /* 37*/ { UNICODE_MODE, 0, -1, "亍亍亍啊", -1, 0, 0, "50 00 00 00 00 FF E0 00 FF F0 00 00 00", "Region 2 (FFE terminator) -> Region 1 (no indicator)" },
        /* 38*/ { UNICODE_MODE, 0, -1, "啊啊啊啊亍亍啊", -1, 0, 0, "40 00 00 00 00 00 0F FE 00 00 00 FF E0 00 FF F0 00", "Region 1 (FFE) -> Region 2 (FFE) -> Region 1" },
        /* 39*/ { UNICODE_MODE, 0, -1, "亍亍亍亍啊啊亍", -1, 0, 0, "50 00 00 00 00 00 0F FE 00 00 00 FF E0 00 FF F0 00", "Region 2 (FFE) -> Region 1 (FFE) -> Region 2" },
        /* 40*/ { DATA_MODE, 0, ZINT_FULL_MULTIBYTE | (2 << 8), "é", -1, 0, 0, "47 02 FF F0 00 00 00 00 00", "H(1)1 (UTF-8) (Region One) (full multibyte with mask)" },
        /* 41*/ { UNICODE_MODE, 0, -1, "˘", -1, 0, 0, "70 01 16 80 00 00 00 00 00", "H(f)1 (GB 18030)" },
        /* 42*/ { UNICODE_MODE, 4, -1, "˘", -1, 0, 4, "80 43 00 0D 10 00 00 00 00", "ECI-4 B1 (ISO 8859-2)" },
        /* 43*/ { UNICODE_MODE, 0, -1, "Ħ", -1, 0, 0, "70 00 47 80 00 00 00 00 00", "H(f)1 (GB 18030)" },
        /* 44*/ { UNICODE_MODE, 5, -1, "Ħ", -1, 0, 5, "80 53 00 0D 08 00 00 00 00", "ECI-5 B1 (ISO 8859-3)" },
        /* 45*/ { UNICODE_MODE, 0, -1, "ĸ", -1, 0, 0, "70 00 50 00 00 00 00 00 00", "H(f)1 (GB 18030)" },
        /* 46*/ { UNICODE_MODE, 6, -1, "ĸ", -1, 0, 6, "80 63 00 0D 10 00 00 00 00", "ECI-6 B1 (ISO 8859-4)" },
        /* 47*/ { UNICODE_MODE, 0, -1, "Ж", -1, 0, 0, "30 01 53 D4 00 00 00 00 00", "B2 (GB 18030)" },
        /* 48*/ { UNICODE_MODE, 7, -1, "Ж", -1, 0, 7, "80 73 00 0D B0 00 00 00 00", "ECI-7 B1 (ISO 8859-5)" },
        /* 49*/ { UNICODE_MODE, 0, -1, "Ș", -1, 0, 0, "70 00 B9 80 00 00 00 00 00", "H(f)1 (GB 18030)" },
        /* 50*/ { UNICODE_MODE, 18, -1, "Ș", -1, 0, 18, "81 23 00 0D 50 00 00 00 00", "ECI-18 B1 (ISO 8859-16)" },
        /* 51*/ { UNICODE_MODE, 0, -1, "テ", -1, 0, 0, "30 01 52 E3 00 00 00 00 00", "B2 (GB 18030)" },
        /* 52*/ { UNICODE_MODE, 20, -1, "テ", -1, 0, 20, "81 43 00 14 1B 28 00 00 00", "ECI-20 B2 (SHIFT JIS)" },
        /* 53*/ { UNICODE_MODE, 20, -1, "テテ", -1, 0, 20, "81 43 00 24 1B 2C 1B 28 00", "ECI-20 B4 (SHIFT JIS)" },
        /* 54*/ { UNICODE_MODE, 20, -1, "\\\\", -1, 0, 20, "81 43 00 24 0A FC 0A F8 00", "ECI-20 B4 (SHIFT JIS)" },
        /* 55*/ { UNICODE_MODE, 0, -1, "…", -1, 0, 0, "4E BC FF F0 00 00 00 00 00", "H(1)1 (GB 18030)" },
        /* 56*/ { UNICODE_MODE, 21, -1, "…", -1, 0, 21, "81 53 00 0C 28 00 00 00 00", "ECI-21 B1 (Win 1250)" },
        /* 57*/ { UNICODE_MODE, 0, -1, "Ґ", -1, 0, 0, "70 01 B9 00 00 00 00 00 00", "H(f)1 (GB 18030)" },
        /* 58*/ { UNICODE_MODE, 22, -1, "Ґ", -1, 0, 22, "81 63 00 0D 28 00 00 00 00", "ECI-22 B1 (Win 1251)" },
        /* 59*/ { UNICODE_MODE, 0, -1, "˜", -1, 0, 0, "70 01 18 00 00 00 00 00 00", "H(f)1 (GB 18030)" },
        /* 60*/ { UNICODE_MODE, 23, -1, "˜", -1, 0, 23, "81 73 00 0C C0 00 00 00 00", "ECI-23 B1 (Win 1252)" },
        /* 61*/ { UNICODE_MODE, 24, -1, "پ", -1, 0, 24, "81 83 00 0C 08 00 00 00 00", "ECI-24 B1 (Win 1256)" },
        /* 62*/ { UNICODE_MODE, 0, -1, "က", -1, 0, 0, "70 07 71 00 00 00 00 00 00", "H(f)1 (GB 18030)" },
        /* 63*/ { UNICODE_MODE, 25, -1, "က", -1, 0, 25, "81 92 F9 00 3F 00 00 00 00", "ECI-25 T2 (UCS-2BE)" },
        /* 64*/ { UNICODE_MODE, 25, -1, "ကက", -1, 0, 25, "81 92 F9 00 10 03 F0 00 00", "ECI-25 T4 (UCS-2BE)" },
        /* 65*/ { UNICODE_MODE, 25, -1, "12", -1, 0, 25, "81 93 00 20 01 88 01 90 00", "ECI-25 B4 (UCS-2BE ASCII)" },
        /* 66*/ { UNICODE_MODE, 27, -1, "@", -1, 0, 27, "81 B2 FB 2F C0 00 00 00 00", "ECI-27 T1 (ASCII)" },
        /* 67*/ { UNICODE_MODE, 0, -1, "龘", -1, 0, 0, "30 01 7E C9 80 00 00 00 00", "B2 (GB 18030)" },
        /* 68*/ { UNICODE_MODE, 28, -1, "龘", -1, 0, 28, "81 C3 00 17 CE A8 00 00 00", "ECI-28 B2 (Big5)" },
        /* 69*/ { UNICODE_MODE, 28, -1, "龘龘", -1, 0, 28, "81 C3 00 27 CE AF CE A8 00", "ECI-28 B4 (Big5)" },
        /* 70*/ { UNICODE_MODE, 0, -1, "齄", -1, 0, 0, "5B BF FF F0 00 00 00 00 00", "H(2)1 (GB 18030)" },
        /* 71*/ { UNICODE_MODE, 29, -1, "齄", -1, 0, 29, "81 D5 BB FF FF 00 00 00 00", "ECI-29 H(2)1 (GB 2312)" },
        /* 72*/ { UNICODE_MODE, 29, -1, "齄齄", -1, 0, 29, "81 D5 BB FB BF FF F0 00 00", "ECI-29 H(2)2 (GB 2312)" },
        /* 73*/ { UNICODE_MODE, 0, -1, "가", -1, 0, 0, "70 2B 5E 80 00 00 00 00 00", "H(f)1 (GB 18030)" },
        /* 74*/ { UNICODE_MODE, 30, -1, "가", -1, 0, 30, "81 E3 00 15 85 08 00 00 00", "ECI-30 T2 (EUC-KR)" },
        /* 75*/ { UNICODE_MODE, 30, -1, "가가", -1, 0, 30, "81 E3 00 25 85 0D 85 08 00", "ECI-30 B4 (EUC-KR)" },
        /* 76*/ { UNICODE_MODE, 170, -1, "?", -1, 0, 170, "88 0A A2 FB 1F C0 00 00 00", "ECI-170 L1 (ASCII invariant)" },
        /* 77*/ { DATA_MODE, 899, -1, "\200", -1, 0, 899, "88 38 33 00 0C 00 00 00 00", "ECI-899 B1 (8-bit binary)" },
        /* 78*/ { UNICODE_MODE, 900, -1, "é", -1, 0, 900, "88 38 43 00 16 1D 48 00 00", "ECI-900 B2 (no conversion)" },
        /* 79*/ { UNICODE_MODE, 16384, -1, "é", -1, 0, 16384, "8C 04 00 03 00 16 1D 48 00", "ECI-16384 B2 (no conversion)" },
        /* 80*/ { UNICODE_MODE, 3, -1, "β", -1, ZINT_ERROR_INVALID_DATA, 3, "Error 545: Invalid character in input data for ECI 3", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_HANXIN, data[i].input_mode, data[i].eci, -1 /*option_1*/, -1, data[i].option_3, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %s, \"%s\", %d, %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), ret < ZINT_ERROR ? symbol->eci : -1, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, -1, "1234", 0, 23, 23, "Mode nnnn, mask 10",
                    "11111110101011001111111"
                    "10000000000000100000001"
                    "10111110111011101111101"
                    "10100000010111100000101"
                    "10101110011101101110101"
                    "10101110000101001110101"
                    "10101110111001101110101"
                    "00000000100000000000000"
                    "00010101101110000010101"
                    "01010110110001011011101"
                    "00100011000100100100101"
                    "01110111111110001000111"
                    "01001001110000001001000"
                    "00100101001111110010000"
                    "10101000011110110101000"
                    "00000000001011100000000"
                    "11111110110111101110101"
                    "00000010011010001110101"
                    "11111010101101001110101"
                    "00001010100111000000101"
                    "11101010111001101111101"
                    "11101010111111000000001"
                    "11101010000100101111111"
                },
        /*  1*/ { UNICODE_MODE, 1, 1, -1, "1234567890", 0, 23, 23, "ISO 20830 Draft K.1 Figure K8 (& K5) same (mask 01), except no alternating filler in structural info in figure",
                    "11111110001000001111111"
                    "10000000110001100000001"
                    "10111110001010101111101"
                    "10100000111011100000101"
                    "10101110010000101110101"
                    "10101110110101001110101"
                    "10101110001100001110101"
                    "00000000011101100000000"
                    "00010101001010011010101"
                    "01001101111101010101110"
                    "10101010101010100101101"
                    "01010010111101010101010"
                    "10101010101010101010100"
                    "10000011010101000011110"
                    "10101011001010010101000"
                    "00000000110100000000000"
                    "11111110011110001110101"
                    "00000010010101101110101"
                    "11111010101010001110101"
                    "00001010110010100000101"
                    "11101010100010001111101"
                    "11101010110101100000001"
                    "11101010001010001111111"
                },
        /*  2*/ { UNICODE_MODE, 1, 1, 1 << 8, "1234567890", 0, 23, 23, "ISO 20830 Draft K.1 Figure K4, with explicit mask pattern 00, same except no alternating filler in figure",
                    "11111110100010101111111"
                    "10000000000100100000001"
                    "10111110000000101111101"
                    "10100000001110000000101"
                    "10101110111010101110101"
                    "10101110000000001110101"
                    "10101110000110001110101"
                    "00000000001000100000000"
                    "00010101000000100010101"
                    "00011000101000000000100"
                    "00000000000000001111000"
                    "00000111101000000000000"
                    "00000000000000000000001"
                    "11010110000000010110100"
                    "10101000100000010101000"
                    "00000000100001000000000"
                    "11111110010100001110101"
                    "00000010000000001110101"
                    "11111010100000101110101"
                    "00001010000111000000101"
                    "11101010101000001111101"
                    "11101010100000000000001"
                    "11101010100000101111111"
                },
        /*  3*/ { UNICODE_MODE, 1, 1, 3 << 8, "1234567890", 0, 23, 23, "ISO 20830 Draft K.1 Figure K6, with explicit mask pattern 10, same except no alternating filler in figure",
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
        /*  4*/ { UNICODE_MODE, 1, 1, 4 << 8, "1234567890", 0, 23, 23, "ISO 20830 Draft K.1 Figure K7, with explicit mask pattern 11, same except no alternating filler in figure",
                    "11111110101111001111111"
                    "10000000000011000000001"
                    "10111110000000001111101"
                    "10100000110110100000101"
                    "10101110100010101110101"
                    "10101110100111101110101"
                    "10101110111111101110101"
                    "00000000001111000000000"
                    "00010101000111101010101"
                    "00000010110000111000011"
                    "10011010011111001000011"
                    "11000010000111000111000"
                    "01000101101110111000110"
                    "00010001101101010001100"
                    "10101010110010010101000"
                    "00000000010011000000000"
                    "11111110100110101110101"
                    "00000010101101101110101"
                    "11111010101101101110101"
                    "00001010101010100000101"
                    "11101010011010001111101"
                    "11101010011010000000001"
                    "11101010011010101111111"
                },
        /*  5*/ { UNICODE_MODE, 3, 10, -1, "1234567890ABCDEFGabcdefg,Han Xin Code", 0, 41, 41, "ISO 20830 Draft K.2 Figure K16 (& K14) (happens to use same mask pattern 10), same except for alternating filler",
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
        /*  6*/ { UNICODE_MODE, 3, 10, 1 << 8, "1234567890ABCDEFGabcdefg,Han Xin Code", 0, 41, 41, "ISO 20830 Draft K.2 Figure K12 explicit mask pattern 00, same except for alternating filler",
                    "11111110100010001000101001011100101111111"
                    "10000000101111010110000100111110000000001"
                    "10111110100000000101000100010000001111101"
                    "10100000100000000000000000000000100000101"
                    "10101110100000000001110110110011001110101"
                    "10101110001000001000000100010011001110101"
                    "10101110000110000000010100111010101110101"
                    "00000000011101000000000000000000000000000"
                    "00011110100000000000000110010000000010101"
                    "11111010010101111011000000101001000000000"
                    "11101000000000000000000000000000000000000"
                    "10000000110001010010100100001100100001001"
                    "11101000000000001000101000011011000000000"
                    "10000000000000000000000001000000101010111"
                    "11100000011101001111110000000000010111110"
                    "11111001000000000000000000000000000000000"
                    "10001111111010100111101001011101000000000"
                    "10000000111001101010000100000000000000000"
                    "10000000000000001111010000111110101001011"
                    "10011010000000000000000010101010111111100"
                    "11111111111111111111100000000000000000001"
                    "00000000000000000000100000000000000010000"
                    "01010010001101101110101100110010000000000"
                    "00000000010001101100111100000000000000000"
                    "00000000000000000100111000100110010100010"
                    "00001011000000000000100000101001101001010"
                    "00000000000000000000100000000000000110000"
                    "11011111101001100110110011010000000000000"
                    "00001101001000000110100000000000000000000"
                    "00000000000001001110100111001111001111010"
                    "01001000000000000000100010010101001101100"
                    "00000000000000000000100000000000001111011"
                    "10101000011010001010100100011101101111000"
                    "00000000011000000000100000000110000000000"
                    "11111110101111000000100000000000001110101"
                    "00000010000000000000100000000000001110101"
                    "11111010000000001000110110110010101110101"
                    "00001010100001011100101110000000100000101"
                    "11101010000000000010110011110000101111101"
                    "11101010000000000000100000000000100000001"
                    "11101010100000000000111111111111101111111"
                },
        /*  7*/ { UNICODE_MODE, 3, 10, 2 << 8, "1234567890ABCDEFGabcdefg,Han Xin Code", 0, 41, 41, "ISO 20830 Draft K.2 Figure K13 explicit mask pattern 01, same except for alternating filler",
                    "11111110001000100010100011110110001111111"
                    "10000000011010000010000001101011000000001"
                    "10111110101010101111101110111010001111101"
                    "10100000010101010101010101010101000000101"
                    "10101110001010101011011100011001001110101"
                    "10101110111101011101010001000110001110101"
                    "10101110001100101010111110010000101110101"
                    "00000000001000010101010101010101000000000"
                    "00011110101010101010101100111010111010101"
                    "10101111000000101110010101111100010101010"
                    "11000010101010101010101010101010101010101"
                    "11010101100100000111110001011001110100011"
                    "11000010101010100010000010110001101010101"
                    "11010101010101010101010100010101111111101"
                    "11001010110111100101011010101010111101011"
                    "10101100010101010101010101010101010101010"
                    "10100101010000001101000011110111101010101"
                    "11010101101100111111010001010101010101010"
                    "10101010101010100101111010010100000011110"
                    "11001111010101010101010111111111101010100"
                    "11111111111111111111101010101010101010101"
                    "00000000000000000000110101010101010111000"
                    "11111000100111000100100110011000101010101"
                    "01010101000100111000101001010101010101010"
                    "10101010101010101110110010001100111110111"
                    "01011110010101010100110101111100111100000"
                    "10101010101010101010101010101010101100101"
                    "10001010111100110010100110000101010101010"
                    "10100111100010101100101010101010101010101"
                    "01010101010100011010110010011010011010000"
                    "11100010101010101010101000111111100111001"
                    "01010101010101010100110101010101011010001"
                    "10101011110000100000101110110111101111000"
                    "00000000001101010100110101010011000000000"
                    "11111110100101101010101010101010001110101"
                    "00000010010101010100110101010101101110101"
                    "11111010001010100010111100011000001110101"
                    "00001010010100001000111011010101000000101"
                    "11101010001010101000111001011010101111101"
                    "11101010010101010100110101010101000000001"
                    "11101010001010101010111111111111001111111"
                },
        /*  8*/ { UNICODE_MODE, 3, 10, 4 << 8, "1234567890ABCDEFGabcdefg,Han Xin Code", 0, 41, 41, "ISO 20830 Draft K.2 Figure K15 explicit mask pattern 11, same except for alternating filler",
                    "11111110101111100100100100110001001111111"
                    "10000000101000010000000011111001100000001"
                    "10111110100000000101000100010000101111101"
                    "10100000011000111000111000111000000000101"
                    "10101110111000000111001101110011001110101"
                    "10101110101111001111000011010100101110101"
                    "10101110111111000111101100011101001110101"
                    "00000000011010000111000111000111100000000"
                    "00011110100111000000111110010111001010101"
                    "11100000001101000011111000010001111000111"
                    "11110010011111000111011000111000000111000"
                    "11000101011110010101100011001011100110001"
                    "10101101101110110000010001011100000111111"
                    "11000111101101000111000110000111101101111"
                    "11110000001111101000110111000000101111001"
                    "11101001010010011000111000111000111000111"
                    "10011111101000111000101110011010011000111"
                    "11001111110100000101000011000111000111000"
                    "11001111001101100001101000000110010000011"
                    "11011101001101101101000101101101111000100"
                    "11111111111111111111100111000111000111001"
                    "00000000000000000000111000111000111010100"
                    "11001000110111111100110011110101000111000"
                    "11000101010100000000110011000111000111000"
                    "01001101000101101000110110011110101100101"
                    "11000110000111101100101101101110101110010"
                    "10010010011000010010110010100111000001000"
                    "11001111110001110100100001001000111000111"
                    "10011101010000010100110010011111000111000"
                    "11000111100110000010101010100000001000010"
                    "00001111100111001100101111111011110101011"
                    "11000111100111001100101101101101001000011"
                    "10101010001010111000110110001111101111000"
                    "00000000101000111010110010010100000000000"
                    "11111110011111111010110010010010101110101"
                    "00000010101111000100101101101101101110101"
                    "11111010001111001100111011011111101110101"
                    "00001010001110011010100011101101000000101"
                    "11101010110010111010100001100010101111101"
                    "11101010110010111000110010010010100000001"
                    "11101010010010111000111111111111101111111"
                },
        /*  9*/ { UNICODE_MODE, 2, 17, -1, "Summer Palace Ticket for 6 June 2015 13:00;2015年6月6日夜01時00分PM頤和園のチケット;2015년6월6일13시오후여름궁전티켓.2015年6月6号下午13:00的颐和园门票;", 0, 55, 55, "**NOT SAME** as ISO 20830 Draft K.3 Figure K23, different encoding modes; if same encoding modes forced, uses mask pattern 01 instead of pattern 10, but matches pattern 01 example Figure K20 (excluding alternating filler) (forced encoding mode: ttttttttttttttttttttttttttttttttttttttttttttttt1t1t11ttdtt1ttddddddddtttttfftfftffttffffffffffffffffffttttt1t1t111ttttt111111t)",
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
        /* 10*/ { UNICODE_MODE, -1, -1, -1, "汉信码标准", 0, 23, 23, "ISO 20830 Draft Figure 4, **NOT SAME**, uses mask 11 instead of 10 (note figure includes alternating filler)",
                    "11111110000101001111111"
                    "10000000001000100000001"
                    "10111110110001001111101"
                    "10100000100100000000101"
                    "10101110110011001110101"
                    "10101110111101101110101"
                    "10101110101100101110101"
                    "00000000011001100000000"
                    "00010101110110000010101"
                    "01010010101111101111001"
                    "11110010001011001101110"
                    "01010110010010010011001"
                    "11110011100000000111000"
                    "11100110010101000010101"
                    "10101000010010110101000"
                    "00000000110011000000000"
                    "11111110111011101110101"
                    "00000010101101101110101"
                    "11111010001100101110101"
                    "00001010011111100000101"
                    "11101010011010101111101"
                    "11101010111010000000001"
                    "11101010011010001111111"
                },
        /* 11*/ { UNICODE_MODE, -1, -1, 3 << 8, "汉信码标准", 0, 23, 23, "ISO 20830 Draft Figure 4, explict mask 10, same",
                    "11111110100001101111111"
                    "10000000101011100000001"
                    "10111110101110001111101"
                    "10100000010101100000101"
                    "10101110001111001110101"
                    "10101110000101101110101"
                    "10101110111100101110101"
                    "00000000011010100000000"
                    "00010101101110111010101"
                    "00000001111110011110111"
                    "01001100110000101110001"
                    "01101100000010101011110"
                    "11111111000111110110110"
                    "00000101011100100001001"
                    "10101011111111110101000"
                    "00000000101000000000000"
                    "11111110101101101110101"
                    "00000010111111001110101"
                    "11111010001000001110101"
                    "00001010110110000000101"
                    "11101010010111101111101"
                    "11101010101001100000001"
                    "11101010100100101111111"
                },
        /* 12*/ { UNICODE_MODE | ESCAPE_MODE, -1, 4, -1, "汉信码标准\015\012中国物品编码中心", 0, 29, 29, "ISO 20830 Draft Figure 5, **NOT SAME** uses mask 11 instead of 01",
                    "11111110000100101110101111111"
                    "10000000101110001001000000001"
                    "10111110111111111110001111101"
                    "10100000100100110100100000101"
                    "10101110001110001100101110101"
                    "10101110101111110111101110101"
                    "10101110111001111001101110101"
                    "00000000001000101101000000000"
                    "00011000111000000111100010101"
                    "10111010011011110000101001011"
                    "11100001011111000101000011011"
                    "10100000100100111100011010000"
                    "11001111001110010000001111100"
                    "11100000010111100111111000100"
                    "11111111111111101010110110101"
                    "00000000000000110101011111100"
                    "01111101011000101100111000101"
                    "01110101111110101110000100110"
                    "00000010111010111000111000110"
                    "00111111000100100010010110111"
                    "10101000100000111111100011000"
                    "00000000011110100101000000000"
                    "11111110100010100000101110101"
                    "00000010111100101101101110101"
                    "11111010101110110010001110101"
                    "00001010100000110011100000101"
                    "11101010011000100111101111101"
                    "11101010010110100010100000001"
                    "11101010111000111111001111111"
                },
        /* 13*/ { UNICODE_MODE | ESCAPE_MODE, -1, 4, 2 << 8, "汉信码标准\015\012中国物品编码中心", 0, 29, 29, "ISO 20830 Draft Figure 5, explicit mask 01, same",
                    "11111110100010101001101111111"
                    "10000000011100001011100000001"
                    "10111110110101010100101111101"
                    "10100000101001011001100000101"
                    "10101110111100100000101110101"
                    "10101110111101100101001110101"
                    "10101110001010010100001110101"
                    "00000000011010111111100000000"
                    "00011000110101101101010010101"
                    "11110101010110011101000100110"
                    "11010001101010101000110001001"
                    "10110000011110101110001000010"
                    "10100000001010000010011100001"
                    "11110010101111110101101010100"
                    "11111111111111100111011011001"
                    "00000000000000111000110010000"
                    "01000111100000111001010101000"
                    "11101111100110110100010110100"
                    "11100111011100111100101010100"
                    "10101101011100111010000100101"
                    "10101001011000100111100011000"
                    "00000000110000100010000000000"
                    "11111110010010111000001110101"
                    "00000010001100110101101110101"
                    "11111010100000110101101110101"
                    "00001010110010101011100000101"
                    "11101010101010111111101111101"
                    "11101010111010100101000000001"
                    "11101010101010111111101111111"
                },
        /* 14*/ { UNICODE_MODE | ESCAPE_MODE, -1, 24, -1, "汉信码标准\015\012中国物品编码中心\015\012北京网路畅想科技发展有限公司\015\012张成海、赵楠、黄燕滨、罗秋科、王毅、张铎、王越\015\012施煜、边峥、修兴强\015\012汉信码标准\015\012中国物品编码中心\015\012北京网路畅想科技发展有限公司", 0, 69, 69, "ISO 20830 Draft Figure 6 **NOT SAME** different encodation, Region 1 FFE terminators not used in figure",
                    "111111100000101101011111111111111111111001001110010100100010001111111"
                    "100000001100110100000000000000000010111100110000010001110101000000001"
                    "101111100111100011100101000110011011011110011110101101001000101111101"
                    "101000000010000111011010000110111010111100100011111100110000000000101"
                    "101011100000010010000110101000000011011100101011011111000111101110101"
                    "101011101101000100011000101010101011000000111001000110111100101110101"
                    "101011100010010010100011101010101010101010101010101010101010101110101"
                    "000000001110010000011101101011000011001011100111111010101010000000000"
                    "001011001010010010110100001010101011100110101000001010011011011010101"
                    "110101110101111001111100010010001010101010111011111111010100011010101"
                    "101101010101001001011100011011101010100011111000010100101101111011010"
                    "100101101010101100010100110001110010111010001001010101010101010101010"
                    "101010101000010100100100001011100011111110000101111011010000010011110"
                    "111110111100111000111001011101101011111001011111000111101100011101101"
                    "110010010110011111011010111110110010110010100010100110110101001010101"
                    "100111110010001001001110110110110010000011110011010011011101110001110"
                    "110000111000010110101010101010101010101010101010101011101100011110100"
                    "111111111111111111100101110001010011111111111111111111010100110100001"
                    "000000000000000001011011110011000000000000000000000110101001101000100"
                    "101010000010010001000000101101000011011110101100110111010101010110110"
                    "111000101001110101011100101110011100011100001001100101000011010110110"
                    "011001010011101101111111101011000111010001100010000111111110011110011"
                    "011010101010101001101010101010101010010011001110110101111011100000101"
                    "001111110001111101100000101001100010011110010011110110100000110010000"
                    "011000111100011101000000100010001011111101001110010111001100101001110"
                    "010011110011010101111111110000010010110000111100110111000100111001100"
                    "001010001110010101010000110111001011111001000010100110101010101010101"
                    "010101010000001101000101000000000110111100101101000100101011111000110"
                    "010100111011001001000010110111110010110101010100110100100000110011110"
                    "000110111011111001100011001111011111100010110110000101101111111010000"
                    "011010100001110001000111001000110110100111011010000101000101010010111"
                    "001111101110101101110101010101010101010101011010100101111100111001111"
                    "110001001111100101100010010001100011001111010010110110101101100011011"
                    "000000000010011001001101010111001111101010011001110111100101110111100"
                    "100010010011001001111111111111111111001000001100000111111111111111111"
                    "001000110111000000000000000000000011011111000001010000000000000000001"
                    "110110010101010011111001111011110011101011000100100110001101110000101"
                    "110101010101010101010101100011111011001011101100111000100010110010001"
                    "101111101000111110110100000000110011010100001101101010000000100100101"
                    "110010101010100000110111010011100010000001100010101010101001001100101"
                    "110100011010011000111011011010001010000001101001110000110001100001001"
                    "110001011100001010100111101010100011101001010101010101010101010100101"
                    "110101011110111110001010111011000011010101100010001111110111101001101"
                    "101000001011000111011000011101101010101011110100100110110000001011101"
                    "100000010000100111110110101100010011001010101100001111011010010000001"
                    "111011110101110011110111111110111010101001110101101001100000001100101"
                    "100111001001110010101010101010101010101011010011110101001001000011101"
                    "101010001010111111100110000011010010011110101111001010010110110110101"
                    "100101110111011101100110100001011011110010110001001101000111100001101"
                    "100100101100100101111001011110101011010110010010101000001010110011101"
                    "110010011100000000100100000110100010001010101011010100110010101010100"
                    "111111111111111111010101010101011011111111111111111110100101000000001"
                    "000000000000000001010000101001010000000000000000000101001000011110100"
                    "100000111111111001111001000110000111111100101111010101111001000000010"
                    "111000010111010101100110111110100001011110111111110101101011111010011"
                    "101100001011101001000000110010011011101100010010000111000101011011010"
                    "000100001101010001011010011111111010101010101010100110101011000011111"
                    "001001100001111001101010110100010111101000100001000100100100000000111"
                    "010100001000010101100010001011111010100100100111010111110110001101101"
                    "100000111101011001000100010101101000111111000001010111101110100001001"
                    "101010110100111101010001011011001110111111010011000110000101100110100"
                    "000000000010111101100011010101010101010101010101010101010000100000000"
                    "111111101101011001111010100000101000010101101111010100100100001110101"
                    "000000101101001001101001011100101111001111000000110101010100101110101"
                    "111110101101011001010101110100100010110111111100110111110101001110101"
                    "000010100111111001000111110110011100010110001010110110001110000000101"
                    "111010101001001001100110011111011101010101011011100101100011001111101"
                    "111010100100111101001001010101011000001101010101010101010101100000001"
                    "111010100010101001111111111111111010101010101010100111111111001111111"
                },
        /* 15*/ { UNICODE_MODE | ESCAPE_MODE, -1, 40, -1, "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\015\012", 0, 101, 101, "ISO 20830 Draft Figure 7 **NOT SAME** different encodation, Binary mode not used by figure",
                    "11111110111111111111100010101000011101101011111111111111111110101100101110110110101100011000101111111"
                    "10000000100000000000100101011001001001000000000000000000000011000001111101100010001100010100100000001"
                    "10111110110100010100111000101101010111011101101110011000011010100101110001010110001001010010101111101"
                    "10100000001101100000101100010000001100000111000000110110000010101010000000010011001110101110000000101"
                    "10101110000010000100110111111001010010101001000110010010000011110001011010010101110001001010001110101"
                    "10101110110111101000100011100000001010010101101010001101001010001111001000100001101111110110001110101"
                    "10101110000101010100110000111111110101101010110101000111100010110010010001000101111001110011101110101"
                    "00000000111000000000110111110101101001111101010101101011010011000100101100101111000110010101100000000"
                    "00111100101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "01010101010101010100110101010101010101101000101110100000110011000101101010000010101111010100000100001"
                    "10001101000100110010100010000000110000011011010011011001011011000011100100000110110001111011001100000"
                    "00111110111011110000101011100010001000000000110110111110110010101010110110011011010111100010111110100"
                    "10110100100100001000101010100010011011001001110000100101100010010010111001100000000101111110001101010"
                    "00101001001110010010111000011010010010000111100111111010101010100110010011000000000010111010100101010"
                    "01000001111100001100110101000100110000011010001111010010000011001110001111100101001010101000111101010"
                    "10001100100111010100110101010101010101010101010101010101010011010101010101010101010101010101010101010"
                    "10101010101010101010101010101010101010101000000010001100100011001101000100110100110111010100110110000"
                    "11000011000101101010110101100010101110001010010000010001010010000110001101100010010011000000011100010"
                    "10001110110000110000110011011101001110011000000100100011111011100110101011001001011010011010001111010"
                    "00010000101111000010100111011000110001001101101101001011000010011110100111010010011010000011101111100"
                    "10101110000101100110111111111111111111111101001101100111110011111111111111111111100111100011010100101"
                    "00101010011010110110000000000000000000001010111110101100100000000000000000000000101000110011111110100"
                    "10101110110111011101011010011110011001101001011101000111010000010010000000101010101010110011100011001"
                    "11000010111101010101010101010101010101001101010101010101010101010101010101010100110101010101010101010"
                    "10101010101010101010101010101010101001001101111111101110011000011100011101100010101110000001011110011"
                    "10110101101101100001000101101010000111001000111110001001111011011011001001110110110111011110110000110"
                    "10011000101110001110101100001001010101101111011011001001101110101100101001101100111010001101111001100"
                    "11111101001010110010101100010011011100101011110000000100011110000000110100011110101101001110100011000"
                    "11000101101000000100001011011011100010101001001110111000111011011001010110111100110111010110100011011"
                    "10001000111011101111000011010111000100101110010011111101001000011111101010000010110010010101010001000"
                    "10101011000010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "11010101010101010101010101010101010101001101011000111100000110000000001111101100101011001110110010100"
                    "10101011101111001011011110101101110111001101100110110100001111101110101101111110110001110111100110100"
                    "11010100100011011110011110001001110001101110101101100101011101001100010001001100111001010000001100000"
                    "10000101111110100100111001000011001010101011011110100011110101111001001000011100110000101111100000000"
                    "10101100011100000011110110100100110010001000110111001111001101010010101000010000101010011000001010100"
                    "11000010110000110100101001000111101100001110000101011011101100100000010111100000111000110001000010101"
                    "11001010101000001011010101010101010101001101010101010101010101010101010101010100110101010101010101010"
                    "10101010101010101010101010101010101010101010101010101011011000100111010010001000111010000001110110111"
                    "11101000101111001100000110110001111101001111000100010101000110111110011110100100111011010011101111010"
                    "11111111111111111111111011001111100000001111111111111111111110100100001011010010111111111111111111111"
                    "00000000000000000000100011011101011101000000000000000000000010000001111111100010000000000000000000001"
                    "10000010111011101010101101110110011000111010110000111111110010000111100111000010110001111100001001101"
                    "01100101110100000100111010010111100100001100010000000110111011000110110001001111001111010110011011001"
                    "00111011010111010010111011010010011101101001100101010111110010110100111011010101011111101100110011101"
                    "11001100100111001000100110101011010101011111011001101111010011010101010101010101010101010101010101001"
                    "10101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010011000101"
                    "01001011101100110110100100100111010011110001010000101110001010011001110011010010111000111000111110001"
                    "10101110100110001110101010011100000000001101101100001101110010010100110010110001010100110100000000101"
                    "11011111111111010100111101011101001000101011010000110001111010000110011011010010001011001000111001101"
                    "11110000100101011000111001010111111101010000000001101011011010000011010101100111101010001100001011101"
                    "00100000000011000100100011001100100000110011001011011010011011101111001111011001100110001000110000001"
                    "01110111000011111000111101001000000111101000011010101011101011111111010110101010101010101010101010101"
                    "01010101010101010100110101010101010101010101010101010101010011010101010101010101010101010101010101001"
                    "10101010010001010100100111010110010010011100010010001010001010101110100011011101001011001110011110001"
                    "01111000100110000100111111111110000110111101001110011100111010110110110011001011100101101010101010001"
                    "10110101000000110000100111000010111011001010100110111101010011001100010110001011011000000100000101001"
                    "11010010110011010000110011100110100111010001010111000001100010111100011100010100101111100111001010101"
                    "00001101111011101010101011011011111110110010100110000001110011010111111110110100110001011101110011001"
                    "00111101010000110000110010100010100010011001001001101100101010101101010100100101010000011101010101000"
                    "10101010101010101010111111111111111111111010101010101010101011111111111111111111101010101010101010101"
                    "00010101010101010100000000000000000000001101010101010101010000000000000000000000110101010101010101000"
                    "10101010101010011001101111100110100010001110110001101001011001110010101001111010101110110101110001010"
                    "10101010011001000000011000110011000100101100110110110000010000001110000110011000111000010110100110000"
                    "10000001000100111010001011110110101001101001001010010110010011101100101000011100110001001110000111101"
                    "11110011000000111100111010101100001010001011010001011011011000010011000110111110110101011010101100111"
                    "10110011011001100100111000100000010111101010000111101110001010110100101101000000111011001010110111010"
                    "11010010001011100100110011101010010110001111101000110100100111010101010011001010101010011101010101010"
                    "10101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "11010101010101010101000100110001011010101101111100001010101001100011100011100110101101100100011001111"
                    "11100010010111111100110110111100101111101100001001100111111101110100000001111110110110011111011111100"
                    "11111101011011001011001100011111110101001010110101100100000000110110010000111100110110000100001101101"
                    "10111101111101010000110000000011101010001010110000111010010001110101101010100000111011000110000001010"
                    "10010010100110000101100001100110001101101000101101001010111000001001110000110010110100000011010100100"
                    "11101011110011011110010100011101110111001101110010001100101000101110101010101010101000101100000110101"
                    "11010101010101010101010101010101010101001101010101010101010101010101010101010100110101010101010101010"
                    "10101010101010101010101010101110000111101011001101000011001110000111011011100110110010101001110110010"
                    "11001110100110110000010111000001100110001111110001100010100110010000011001110100110011010000010001000"
                    "11111000010110000000100001110011110101001000011111000100100110011101111011000000100111101001001010111"
                    "11010001101111101100110100000101110101101010110011101011100100100001111010000010110100011110110010001"
                    "11111111111111111111100111100010101011001111111111111111111110111011101101001010111111111111111111111"
                    "00000000000000000000101011111010011010100000000000000000000011000100111100000110000000000000000000001"
                    "01011000001111110000100000011001011101000111001010001010010010110100011010011101111110010100100110101"
                    "10101101010101010010111101011001010101010101010101010101010011010101010101010101010101010101010101001"
                    "10101010101010101010101010101010101010101010101010101010101010101001001000001110111010111100000011101"
                    "10001001000111000110101101100100001101111110100001000110100010001000100000110011100100000110000100001"
                    "00110100100101011010101011011010001111001011011100011011000011001110101100010010111100110010001100101"
                    "00111110011110110110100100010001011100001001010001111011110010100010111010000011011101000111110000101"
                    "00101110111011010100110001101011010011001010010100011101000011001000100011010110000111010011010011101"
                    "00000011000110101010101010101011000000100100001101100001101010111110001111010001001010101001010101101"
                    "00111001011010110000101010101010101010100011101010101010101010101010101010101010101010101010101010101"
                    "01010101010101010100110101010101010101010101010101010101010011010101010101010101000111111011110011001"
                    "10101010100000010010100110100110011011111010001111111101111011010111011111110100111010101000100111100"
                    "00000000110000000000110101001111001001011111000010100110001010111011110011000101000111001110100000000"
                    "11111110110001100000111101111110000101000111011100100000101010000000101111110101100100111001001110101"
                    "00000010010110100000100110100110110000110110001111010110010011111001000101101100000100010110101110101"
                    "11111010011011110010111110110000100000100011011101001001010011101011101010010010000110011011001110101"
                    "00001010010011110110100011000110111000001010001000101000111011001010101011001010011100100110000000101"
                    "11101010110101101110111110000011000100000000010101111110111010011111011110101010101010101010101111101"
                    "11101010110101010100110101010101010101000001010101010101010011010101010101010100000101010101100000001"
                    "11101010101010101010111111111111111111101010101010101010101011111111111111111110101010101010101111111"
                },
        /* 16*/ { UNICODE_MODE | ESCAPE_MODE, -1, 62, -1, "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\015\012本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\015\012本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法RS、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术   122", 0, 145, 145, "ISO 20830 Draft Figure 8 **NOT SAME** different encodation, Binary mode not used by figure",
                    "1111111000001010101011111111111111111000011101101011001011111111111111111010111011011001101011111111111111111100111110101001101010010010101111111"
                    "1000000001000001100000000000000000001111011000100111000000000000000000001101101011101010000000000000000000001111010010101000100011101101000000001"
                    "1011111001100110000110100101110001001101100010010100100110110000011000101000000110000011100000011011000001001101000000001001100111010111101111101"
                    "1010000000000100001010111111001010001101010010001100100100001110001011001100101011100010010101011110100000101110000000101001010110101000000000101"
                    "1010111001101001000111100100010000101101111110110001010101010000111111101101011010101101010001111000110010001100010001011110011100111100101110101"
                    "1010111010000001011111010110100111101101010101101011110110101000101001101110001000100010010111010101011001101100001110010000001010000011001110101"
                    "1010111000011001100010111101000001101010011111100001001000001101111111101100101000000000001110110111100100001011111010000101101111100110101110101"
                    "0000000010111000010011011110110111101000000101111010001010111101010001101110111100110110011110111110011101101001110000101101101010110001100000000"
                    "0101001011111011001010111001100111101011011111100001011100100110011100101111110010100000110000111100101111101011111100000110010010100010100010101"
                    "1101101100001100000110111000011010101101111101010000101101000000111100001110110110101010101000111010001010101010110011110100111010101101010010001"
                    "1010110100111000011010100110000000101001101001100000101010000110000000001001111001010001010010011011111000001101010000111110011011101010100110011"
                    "1100001100110110101100111010011000101000011001101000011100011010011001101100010000010101000000001000011110001101010101010101010101010101010101010"
                    "1010101010101010101011010001011101001000011010001011010100000101011110101010000010000110001101000100110010001010000000110000011011010011011001011"
                    "1011110001101111100100111000010011001011111110000010001000011110100011101011101111111110010010000010011010101010010011001001010000111010000010110"
                    "1011010010010000100010101000100110101100100111000010010110000100101110001110000000010111111000110101000101001010011100100111000011010010010000111"
                    "1011000000101010101100110110011111101111101000101011010101101111100000101111001010101110110011111001011100001010110111101100011100000110101101011"
                    "1101001111110111001101000011011101101110110101011101101000011110011001001101011010111110011001000111110000001100000001110111111001100101111101111"
                    "1100110010100100001010111100111110001010000100000010111001111100000011101101101010101000000111000101011100101010011111101111011011010011000110100"
                    "1111111111111111111001101110111101101111111111111111111100111001001010101111111111111111111111011101000011001111111111111111111101011000101001101"
                    "0000000000000000001111100110000010100000000000000000001110111011111110100000000000000000001101010101101100000000000000000000001000110010001111100"
                    "0100110001110100001101110110100101100000011011011010001110000111001100011100100011010110001001100011010010100001111100000110001110101010010011001"
                    "1011001001100111101100111010110110011101001011000000001010010111110101110111110011000000001101010101000100101111011111000110101111010000110011100"
                    "0110110101100100001000011101111000000010011000001000001011100100001001101010110111110111001010110111101110001111011101111000101010101001101111101"
                    "0010000101010101001101010101010101010101010101010101001101010101010101111111011100110110101100101110110010110010001010110010001111100111100111010"
                    "0101001010011101001100011101011011111011101011111001101100100111011011001111111000111010101110001001111001110110010001011000101100111111011011100"
                    "0000011001010100101101101001011001011100001011011110101000011110110001001110011101100100101001011010011111000010110001011011001010111110001000000"
                    "1000111101001100001101100100110000011100001110010101101001010110010100100101000001010011001111011100110000000100101110110111001111010110100111100"
                    "1100110010111010001011101000001001000000010100110100001110111010100011111100100010011001001110100010001010011000100000111001101110111010100010110"
                    "1011000101011101001101001000110100000110110011110101001111101011001100100010111011011011101101100111010101000011001111110110101101011001101100001"
                    "1010011001100110101001111001101010010111100101000100101000110100101101000000000010000101101110110011111011101000110000101111101110010101010011100"
                    "0100110111001100101010110101001101011101000101110100101111001001000101010001110101011101001110110101010100010111110111010011101100001100100001001"
                    "1000100000100110101010011011100101001101110101110000101110010011001001010001101000111011001011100000100001110101000110011010101110011101101111000"
                    "1111100110010001101100101011111110011110010100001010001010001101100001100100011101100100001011110010100110011111111001011010101010111010001001000"
                    "0001001111001101001100010101011010011011100000010101001101010101010101010101010101010101001101010101010101001010111111110111001011000011100011101"
                    "1000101110000001001111100111001010010010011110111010001101011110001111000001110110000100101001001101100010001000100001001111001010011000101110001"
                    "0011010110000100101010101111101101100100110111010110001101001101101101000110111100110000001001011010100110101001110110010001101010000111111101110"
                    "1000011111110010101111111111111111111110000100101100001111111111111111111101110011110001001111111111111111111110100000010000101111111111111111111"
                    "0001101101110001000000000000000000001100100111011100000000000000000000001111011011001010100000000000000000001101111010111010100000000000000000001"
                    "1010111001000001000111011101111000001110101110001001110010011111101001001000111111010100000110010011100101001000111000010101111110000111110111101"
                    "1111011100100010010110000101010110101000111100111110001010011100101000101000011010001101110010100111001001001101111110000011001111101101001010101"
                    "1100101000100101100011101010101001101101101010111111100100110111101000001001100110000101101110110010001110101110110000001100101010000100011001101"
                    "1000001110010100010101100101001001001111100000110000001111011101111001101001011100011000001001110001101011101101011110111110100010010011001101101"
                    "1011000101011111001111111010010010001100010101010000101101000101001111101010111011100000111111000001100011001010000010010000111101000101010000001"
                    "1011011001100111101110101110100100001011011110010110110011101101111110001111001100001101001101111011110101001101010010011000100010011011101011101"
                    "1001010111001001001011010101001001101011011001101010101101100101001011101100101011001001010110100101011010101010101010101010101010101010101010101"
                    "1101010101010101010101001110000111101100111111111000001001101001100010001110101101010111011110010110111101001110111011101011001101101000011111001"
                    "1000101001000000011100010000110010101110101001000110111100111100010011101000111101011011001010111010011000101000100110110010100000011000001111001"
                    "1011111101001001110010000110010101001110111101000111101011110010010000101110100001011111000000001010011100001111111000010010110110011011111001001"
                    "1110011110011010100101010000100001001100110000010101000111101001111001001110101101110000100111001111010100101000100110111111010000111100111001101"
                    "1010001001100011111100111100111111101110011100100010111001000000011011001010100011000100111100000110011101001011100100110011010010101111110100001"
                    "1111110101111110110101100010110111001101011110110011011001011000010101001010001001011001100010111010101110001011011010110000000001011111101011101"
                    "1100101001101001110101110000001110101001001110110011011100000000111111001110011111110001110000000101110000001010010001001010111001100110111101001"
                    "1010111100011101010100100001011001001101011111010011010000001101001001101111011101101011010101011101101000001001110011011010111010111010101111101"
                    "1111110110001100000000110001011111101110101010100011001000010001110011001111101101101011000111100110001100001101000000000000110110011100101101000"
                    "1111111111111111111000000111110101001111111111111111111000100001000101101111111111111111111011001100110000001111111111111111111100011111110001001"
                    "0000000000000000001000101101000111000000000000000000001010010111001111000000000000000000001111001011111101000000000000000000001001000101001100000"
                    "0101001101110011001011101010101010101010101010101010101010101010101010101010101010110110001010011101001000100110100000011101101011100101110100001"
                    "1001111100100111001000010100011101110101011100100000101100001011010010010110001000010111001110011111000000010010000101101001101110010001010001011"
                    "1111100000000111001011111010001000101010010001001100101110001010011110000000011111000011001001111010011100000111101100100110101000101111101001011"
                    "0100001101111001101101111111001000011100100111011000001110000101001100100111000100101000101011000100101101100010010110011010101010000011001011000"
                    "1001010101000000101001100110000011001101100011011110001101010111100101101010001101110001001111011011110100010011011011100001001110100000111001010"
                    "0001001110001001001011010000101101100011010011011000001010111000010110110111100001111011101011111011111010110110111101001101101111111000111000000"
                    "0100001000111110101000011111101100000011110110001110101010010010110001011101000101001101001000111011011000000111100111100101101101111100111110101"
                    "0001110100011111001001100011010111010100000001101010101100010111000110000010101010001100101111111110010100011010100101001001101101011000000001011"
                    "0100011111111100101110111010101101110100100011100110101110100110100111010101101111011111101101101110111111010011100111010111101110011010011101010"
                    "1111010010111010101111010110011000010001101111001100101001111111111010111110100100111011001011010001110101100000100010011000101001000001101111100"
                    "0011001010001000101000110000111010011101011011010001101010010110101010101010101010101010101010101010101010101010101010101011001011101011010001001"
                    "1001001101101100001101100001110101111010001110110011001001100101101000111000111000001101001100010110011100010101100011111111101100100100111100100"
                    "0111010110011010001111010101100101111111100010000000001000101010001010001011011101010010101111001110000111100110010010110111001100110111000110000"
                    "0011110110101001101001101010000000101011111111100101001010011111001010100110000101011100101111010001101111111110011101111001101001101111100110011"
                    "0100100101100001001000110000100110011001110111001111101100010001111000001100010110111111001000101111001110011010001110010010001100001000010100101"
                    "0011100111010100101101101110101001101010000111101010001101101101010110101010111010011101101101101110000110100101111100111001101011100000111000011"
                    "1001001110110011001111111111111111111011111111001011101111111111111111111000100110111011101111111111111111111111011010100010101111111111111111111"
                    "0001110110010100000000000000000000001100111011110000000000000000000000001010010000000100100000000000000000001001001001011110100000000000000000001"
                    "1010110101110010111001101011001100101011111100000111001101111101000000101000100011010110001001011100001001001011001011000010111110000101010101001"
                    "1110011010111000000100110011011001101001001011100110100010011011000001101110100110001110110100110001111000101100001100000010111100000001100111101"
                    "1100000100011110111100111101100100101101000000110010101001101110001101101011001101101100000110111001000101001101010010001111111100110111110000001"
                    "1101011100000110011101101010010000001101111110010111001110011101010011101101010110001001111011010101011011001001011011110101010101010101010101001"
                    "1010101010101010101010101010101010101001000101010001110101100100100111001001001000101000101011101000110111001100101100111001111000111100010011001"
                    "1111010000000000111100100001011000101100011000100100100110011010001101001010101010101101001010111111001111101000111101000100110101011001000010101"
                    "1101001100010110001011011000000100001001010110100101100110100010011100101101001110100010101110000011000111101000111000101001011111001110010101001"
                    "1111001000010001010101001001000000001100110101100111111000101010000000001100101100111010001000110010000101001111100111011010111010111011001101101"
                    "1100110110010101011110110101011100101111101010011111001001110000100011101100100100101101110001111101110110101011010110000111001110110000101000001"
                    "1101000110111010110010001100000000101110110010011001100100100110011110001101101011011111010101000010000101001110000111010100100101010110111010101"
                    "1001000110000110010100001001000101101000101000011000001110011111011011001101011111101100110000111101001000001100100100111011010111111010010000101"
                    "1010000100010101110011011111010001101100100101101011101010111101000011101100101000011010100100000110110110101010001100110101001110000100001010101"
                    "1110010111110111110010111010001100101101100000100010110100000111011010001111110000000101100010110101001010101110110001110101001110101000110000001"
                    "1110010111100111000011011111101000001110010101111001000111110010011010001101011110101100100110110001111100101111110011000110000010111100001010001"
                    "1110101010000111010001001110101010101010101010101010101010101010101010101010101010101011001100100000110010101110110010011100101101001100011010101"
                    "1100111101011101101011100010101010101011001101111111001110011001110110001110010010011111011111100011110011001011001111010010110011110000001000100"
                    "1111111111111111111110001011101000001111111111111111111100101011001101101111111111111111111010110100110110001111111111111111111010011010111100001"
                    "0000000000000000001010001001110000100000000000000000001111010001100111100000000000000000001110000110001010100000000000000000001001111010111001000"
                    "1110100100100111101011001110010000001010100101010011001000110011011001100100111000100000001101111010000111101110001010110100101011010000011011001"
                    "0101101110100101101011101000110110011000101011010011001000101110010110110001001110010011101100001110100101000010011001001011101101010001011011101"
                    "1001000111100100101100110100010011001110001101010010001110111101100001010010111100111000001111101100110000111000001000100110101101110111101101001"
                    "0100000010110101001001100101100111110111010101100001101110100111001010001100100101111101001100110001000010011001000000101111001000010101011010000"
                    "1001110100101101001000100101100100011111100101100000101000110100011111110011010011001100101101010001110101110100000110001000001011101100011110111"
                    "1110100101110011001011110011010101001001000011101010001111110001011001011010110011110101001001111100111010101100010111001011101010111011101101001"
                    "0110000111000110001110010111001111000001000101010101101000011001110001001011000001111101001000001011111000111001000100110111001001000010011100010"
                    "1101001101110010101111010000000010100010111001000000101111010011100111111100010101010101001101010101010101010101010101010101001101010101011101100"
                    "1110100101001000001011110101010110011100011100011001001010011011100110000110001001011111101100110110111100101111110000100110001111111101110100000"
                    "0011111110110011101110111111000000010100100110100110001111000000010101101001010011011111101110010011011110000101001111011110001100100111101111101"
                    "0100001100000000101110101000101100001110100100011101001110101010000110110001100000010101101011010110011110100111100110011100101001110100101101010"
                    "0011111011000111101001100101111110010101101111010111101001101111001010001110111011101011101001000110010100010111010110000100101010101100100011011"
                    "1011001101010111001101111100110110011010011101011000101000011110010000000110101101001101001100000001101100110011111001000111001010011100010011100"
                    "1010011100001111001110101100100111001110110101100101001011010110110101100011011001010111001011001101111110011001101100100000001110100110101010101"
                    "1011011110101011001010000000110000101000101011000011101001000100110001100101010001010001101111010001001001001100000001101110101011001010110010000"
                    "0011010000011011001100001010000010010011110000101111101110101000101010100001001010011111101101000111101000101000111110000111101100010010110011101"
                    "1010110100100001001111111111111111111011000000100101101111111111111111111010010010111110001111111111111111111111110011110100101111111111111111111"
                    "0000001101100100100000000000000000001101001110010110100000000000000000001101110110101011100000000000000000001011011010010110000000000000000000001"
                    "1101110011101110011010111000100010001001010011000111011001100010011011101110100111110100011011110101001111101010100110100101101110101010101010101"
                    "1101010101010101010101010101010101001101010101011100001111011001101000001110011100001110110111001110010101001011101100100110001011001001111101001"
                    "1110000011001100111110001100010100101100100000110011101010011010000010001010000000111101001111111011110001101000010101111100000111011011001100001"
                    "1011110110000000111101001001010111001101110010000010011001011111010001001100101001100010100011011011110000101011111001011100001001101110001111001"
                    "1110101010011000100010010110100101101111010011010110001001111000001101001110000011111100000000011001011101001001110010100010100100110100011010001"
                    "1000100000011010110110010010101100001111111000011100000000110100110001101000001100110000101001100001001111101010011110101100110101110111010111101"
                    "1011001001000001110110110010000101001011010100010111111010100110011011101100011101001000011111001000011100101111111110101001110110011001011010101"
                    "1110000000010011000100001111100011101011001100010010101001111000001011101001011100110011001100110011110100101001111111100010000101100111010000101"
                    "1011101011001100011001010101101000101000100100110101110010111110000110001011001011101101000010010111010100001011111101110111111111011001010011001"
                    "1111110101011010000000001001011011001001001110010100110100001101001110101001001100111101100010100111001010101110010111010111010001010111001010101"
                    "1110001010111011100101111001000000101101100010000111010001010111001001001110110001000000011111000001110010001110101001010101111110110101101101001"
                    "1010101011001101010000000011100101001101010101010101010101010101010101001101010101010101010101010110110111101100010001010000111111000011101101101"
                    "1011100011011011001000011011111101001000100011010000010001000001100111001010000011000010000011010010010101101010101101101000111100101101110001101"
                    "1100111011000101001110110100001100101101110011011000001100001001001101101101110100011110110101110000100001101011101000101111100100010111000001101"
                    "1001001011101110110101010001101011001100110010100101000111010001001000101000110101100001110100110100111000001001100011010101010101010110000001001"
                    "1101111001001111001010000011100001001111011010101011010101001100011010001101001111100010010110111001101111101010000010101001011000000101111110000"
                    "1111111111111111111001101100110010001111111111111111111010100101101011101111111111111111111111111111111101001111111111111111111100110111011001101"
                    "0000000000000000001000011110001001000000000000000000001011100001010000000000000000000000001011010111000011000000000000000000001010001000101100100"
                    "0100110100100001101000001100000000101001010010101011101010101100111110110010111101110110001001110111000010110001110001101001001110001100111001110"
                    "1001101111010110101000001110001111110011011100110110101001101111000000100000011010101001101101110100110100001110101001101110101011110100010010101"
                    "1010000101000110101100001010011010010011010000011010101001010001011011011101110010101011001011101011010110111110100110110110001010011101100011000"
                    "1110001110110011101000001010111101011010110000001110001001011110001111011010011110010011001110100100101011001100001011000111001111110101011110001"
                    "0111101101100111101110001001010011011000100010101010101010010000000101101000010011110010101010101010101010101010101010101010101010101010101010101"
                    "1100000010000110001100000010010011010011001101111101001001111111101111101011101111111010001111010101000100000000010101001111001010010111110000101"
                    "0011000101110111101001100010100011100111010001100001101101111110000101000111011100100000101010000000101111110101100100111001101011010000001101001"
                    "1011000011011000101111010110010111100100010110110000001100010110110111100111110110000100001001000110111010010010101101011101001100100100001100110"
                    "1010100010001100001010011100111001000111110101110111001101110000110101010100110101100011001110010101001000000011111001110111101111110110101001010"
                    "0000000011101001001011000110010011001001010010001111001111011010010111111111101000010110101100110111001101010010110110100010101011011111100000000"
                    "1111111011110010001010101011010010000001001111100100001101001110111001100111001001110011101100110011111001011101100010001110101111101101001110101"
                    "0000001001111011001110111010001101101101000001011010101111100100100110101001100011100100001110010110100111111000010111111011101000110000101110101"
                    "1111101011101001001011011101011100010001101000001101101111010010100000010001100100011010101111110011100001001100011001111100001010010110001110101"
                    "0000101001100010101000100110111000101101000100111011001111000000001100010111101010001110001001101000011111011101100010011001001000010110000000101"
                    "1110101010101100001101110100101101001110000011011111001100110010111110010110000100111001001001010000010110110011110100000110101100111011001111101"
                    "1110101000010110101111100111011100000010111011110111001101000101010111000001011011001010101011010000000110000011001001000011001101010101000000001"
                    "1110101010101010101111111111111111101010101010101010101111111111111111101010101010101010101111111111111111101010101010101010101111111111001111111"
                },
        /* 17*/ { UNICODE_MODE | ESCAPE_MODE, -1, 84, -1, "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：84\015\012本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：84\015\012本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方" "法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：84\015\012", 0, 189, 189, "ISO 20830 Draft Figure 9 **NOT SAME** different encodation, Binary mode not used by figure",
                    "111111100111111111100010101000011101011111111111111111000111101110100101111111111111111111100101100001010111111111111111101100011011101001011111111111111110100111001010010101111010101111111"
                    "100000001000000000111000010110101000000000000000000011110111100101100000000000000000001000111110110011000000000000000000110001110111000100000000000000000010001110000000010001110110100000001"
                    "101111100010010100101110011110110011111011110000000011110101010100001011100010110000001010100001111001101000000101101000110110001100001101001011001001110011110001101000100110111010001111101"
                    "101000000001110000100110001000100001111011010110111010101110011110101100001111011010101110110010001001111100001111000010111110110101010110101110110110101010101100011110000111110011100000101"
                    "101011100111011110100011110111000100111101000010111011011100111101000011001100001101001111100111110000100001010011010010111100000000110111001111001100111010110101001110101111110001001110101"
                    "101011101001111100111001000001111101110001001000100011100100001111101010001010000001101010010011111010010000001011100000100100000000001111010101000101110010111010001100011100000111101110101"
                    "101011100001011000111100011101111011110101110110000010111000010111101010000000010011101101111010110100001101111000100000111100100110010001010111011011010011011101010100100100011110101110101"
                    "000000001111101000111111110010010110101000010011001011001110111111111101100000101011001101101110001001001111111110001110110000101010100110111100001001001010110001001110010100100000100000000"
                    "011010001111110110110110001010011000001010100000111010011000101111000000101111011010001111000111010011110010100110010000100111110001010001011110001001111010100001110010000001000011001010101"
                    "001111101110101100100011110101010111001111110101100011010010001011001011000111101011001011100101001111011001000000111010110000100011111010110001001101001010110001101100001100110100011011011"
                    "000110010011010000110100001010001101100010101010110010011011010111101001110101110100001000000110010000110101000001010100101101000000010001110100000111110011000100010010101011000011100111000"
                    "110111100011011000101100101100111001111111000101101011111011100111010011000001100010001100111011011001000100001101000000110101100011011101100101100011001011001110101010110100000110101110001"
                    "001111100101010000110001110010111010010010001110011010011111101001000101101000101110001110100010000011111111101101000010101010101010011101111111110001101011001101101000011011110011100001011"
                    "001100111010101110110100100000111011011000101110101010111000001010101010101001110110001110000011100110000010001110010010111100101100101010110011111011110011011111010110100010111011000111110"
                    "101010111111011110111001101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "010101010101010100110101010101101000101110100011010011100100101000100111110101111011001000101111000001101011010010011100100001111001011001001011110101001011111001100010101111011100110100111"
                    "001001011001100010110000001000010000111010101101001011010011101001101010111001011100001001111111101110000010001011110100101000010101010100000110111010100011011101011011001001010110011001000"
                    "101011010011001100111111111111111111010110011110100011111111111111111110001001010101101111111111111111111001010110011010111111111111111111110101100001110011111111111111111110110000100011101"
                    "000001100101100100000000000000000001010111010000100000000000000000000101101000110111000000000000000000011100111011011010000000000000000001011101001011011000000000000000000101001111001001000"
                    "100001010110110010111101001011000001100011110010101000100001100101100100001101100001101000110111101100011101100111110011011010001110000001000011111101011101100000100101000110100010011010000"
                    "111010011001101111100111001110001001010110011001100001110001011110110100010110110010110100101110000111010010011111100010011010111011101001010000111011011010110000010010100111001010011101000"
                    "101101111101101011000000111011001001000010111110101111111011111111010111001101001000101001110001010010011001000110101010000110001111100101100010011111001011011111101010110101001001110111001"
                    "101000001001010111010111001100110001100011001000101010110110100000010110001010010100110011010100001100011011010110001010001111000100110101100101010110000010111010101111100100010111100000011"
                    "100011111110010010001110110000100101000110001011010110100100001001010100111000101100010111111110100011010111001011011101000100010100111101101010000100100111001000010110000110110001111110000"
                    "101000110110110110110011110000010001101101100111110101111111000011010101101100111101101111110000110111011101011101100101011010001011011101100101100101101101000101101100000101101001010000110"
                    "100000011011100101111001011000000001010000011101001011101010101110000110011101011111111100010001111000011000001010111101011010001111001001010110101000101101101001100110010101000110100010001"
                    "101001111001000110111001010000101101000101011001110001111001111101010100011000101100000011101101001000010101011110110100101100100110101101101100110110100000100001010110010110110001011010110"
                    "110001000100010111000100000010011101100000101000110100100000110011100110011001100001011000111100100001010111100101010110000010011010000001100000111011100111001000011101110100100000011001101"
                    "110100110111110010010101010110110001000000100101011011010101001010010101101110011010101000110001110111011010101010101010101010101010101001101010101010101010101010101010100110101010101010101"
                    "110101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101011111110111000011010101001111110110100110101011110000010100100010101100110"
                    "101100111101010101001100011101000101110111011111000100110000010011110100100011011000101000000100010110010100110001000101101111101001101101000100101000001111111100000110100111001111001100010"
                    "101011001100000001011011011011101001100110001111011001110011111111000110110001011100101100010010110101010000011000101101100000011101001001111010010101110111011111011001010101000101001010110"
                    "110101000100011011111001111010010101011101001101011010000100010100100111110011111011010010111111000100010110011101010010011000110000001101011110100010101111010101110011100111011001010110011"
                    "100101010111000001100100011001010101000100101101010100000110100011110101100101000011011000001100111001011100110110101111110001110011111101011101100000100011100010111001000100100001000001110"
                    "111111111111111111100001001010100101111111111111111111110101101011010111111111111111111010100100001101011111111111111111100011101110011101111111111111111110111100101101000111111111111111111"
                    "000000000000000000111100111001001000000000000000000011100001011100000000000000000000001100000010100101000000000000000000110100110010011000000000000000000010011111111110100000000000000000001"
                    "111001110110011110101111101110101010111101010100111010110100101101010000111011110111001110000001101100100001100111100110100110011100010010010111110100111010100011010010100011110011011100001"
                    "111100101000001000100110011011000011110011110001011011011111010000111001011100110000101010111100000010001011111111100100101110101001000110100110011011111011011010010111100101011100101101001"
                    "111011110011101010100110100010000110111100110100111011101001000001111111000001110011001110000011011010000111101111110000101110010000010100100100110110000010101000011011100101000011000101101"
                    "000101010110110110110010111100111101100101110010010011101100011101110010101000001001001101110110110111110110001101010100110111001000101010100110001011100010100010111001101101110000101101101"
                    "001011110001011000100000100100001101000000010010111011101000001100011010001100100000001111101101010111110000001101111110110101100110110010000001000011110010001000100000010100000010101100001"
                    "010010010111111000110011100010101010111110001001100010101111010010000000011011101110101001011010010010110111100101001100111111101001011101010110010001011011001010000000111100111100010111101"
                    "111100001110100000100101001111100101101111001011011011111111001111010001010001010010101000011101010111110101010100010000111011000011010100110000110011010010100010001001011110100001001001101"
                    "111011001101010100111000000010101100010101010101010011010101010101010101010101010101001101010101010101010101010101010100110101010101010101010101010101010011010101010101010101010101010101001"
                    "101010101010101010101010101010101010101010100101011011111110110000100111101000000111001011010100000100101011100100010110101001100101101011111100101100010011000011010110011111101110010110101"
                    "110000101111101100111111100110011110010111001011000010010101101000100011000011011111101110000111010111010011010100101100111011000010111111001001010111001011011000011011101010001100100101001"
                    "111101011100101000100101001100101010111000001010111011100000010000011101001011110000001010101000010100111000100010001000100110010110100010011101011100110011101010101010001111011011110011001"
                    "101110011001110000100111000001101001101110111011101010101000010010110110010110011001101101101110100110000100110000001000111001101011110110011100110001001010101011011111110000000110100010101"
                    "110111000100100000101100111010010011101111111101010010111110111100011000110100111001001110101110010011001000111011110100110101110011001010110000111100000011000000110100011101100010111011001"
                    "111010101100111000101101011001011100110101100111011010011101110011010000110111011011001100110001110000011001110000011110100100010110110010011111001011001010100011001101100111000111111010101"
                    "001111000011011100101001011101110101000011010101111011001101000001011011010010001111101011111001111010011100101110111010101001110110011001101000011001110011101101011000010001100111010100100"
                    "100011111000010110111111111111111111010011111111100011111111111111111100011100011101101111111111111111110101010111111000111111111111111111011001000011011011111111111111111111110100111010101"
                    "001101111110000110000000000000000001101011111110010000000000000000000101000101111111000000000000000000010100111011110110000000000000000001110110101001111000000000000000000101111001111101000"
                    "110110000011011000001011111101100001010011001000100111101111000100010111111001110110110111010010011001010011000000001001101001000100011101000101010011000011001010000101000111010001010001100"
                    "101001110001010100000010011010110001101011010100100010001000110101000101010101001011111100010100011111010100011011001110011110010011111001101101101101000111111000110100110110001001111010111"
                    "101110011101011100110011001000110001111110100100100001110110000001010111111011101000100001101111111001010100111011101100010110010100101001100011010010000000001011111000110100000010110111101"
                    "111101001110111101101010010010100001101100111101011001100101101111010100101000000110001110111011111011010011011010100110110100100110011001100100110100011000101000100100110111011101001001001"
                    "101000101011100001101101100111011101001110101110111011011010001100110101101100101010111011101000011001010101010111101101101000010101010101010101010101010101010101010101010101010101010101010"
                    "101010101010101010101010101010101001101010101010101010101010101010100110101010101010101010101010101010011010101010101010101010101010100101110000111110001010010001001101010110001001010011001"
                    "100000111011010001001010100011010001101101110001011010111001111100000100001101000101001010110010010000010110101100011101001001100111011001101001100100110001100000000101010110100000011101001"
                    "101010011011101011101000001110001001110011011001100111001000101011010101010101000110001000101100011101011111100111011001110111001000111101100110100110110001000111110010100101110010100101011"
                    "101010111010001011101010101010001001011000110001111011001110101010110110010001100001010011101110000000010101001111110011011010000000001001100111011000110001100110110110110110010111101110100"
                    "110111110011001000000000001101001001110011001101110011001000111100100110100001101110100100011011110011010100000100010001110011111111001101001110010100100010011101111011000110100110100110011"
                    "100011010100100100011111011101010101000110010010111011111110010110000110011110011100100011111010010010011010000011010101110110011111100001110100000110011000011011011010110101000100101011100"
                    "111111100001101110001010100101010001001001110011010101010101101000000101001100110100111111010111001100010011111101000100000010010110111101111001100000111111010001110000100110110011111001110"
                    "111011001101000110001001001101101101111111100001100101100010010111010100011100101001101101011010010001010100110011100101110111010111010101101110101101010110000001011111000111101101101101011"
                    "101101111110011010010000011100000101001011100101011001100110000001000110101011011011011011111111100110011100000111000100000111011010011001110110111000101011000011110100010110001010011000010"
                    "110111001110000110010110000011111101111011000000001101111010100011000111011001100011011101011001110111010110101010100011000100000111101001000010000000010100011001110111010110010111110101110"
                    "111111111111111111110001010100110101111111111111111111111101001001000111111111111111111010001110010101011111111111111111100100011001111101111111111111111111111110010100110111111111111111111"
                    "000000000000000000100110000111010000000000000000000010101101101100100000000000000000001010011000011110000000000000000000111000111010100100000000000000000010001101111010100000000000000000001"
                    "010001001101110110110010010100100000000111011010110010101011000001110111100110000000101001011111110110000000101101000100101111011111000111100010000101000011101001011000101111110011000001101"
                    "101101000001010100110101101110011101001011101101110011001101010111011001010010011100001100110010001110111000010001011010101110101011001110110100101100111011001101001110010100100001110000001"
                    "000110010101001010110111101110101010101110011000100010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "010101010101010100110101010101010101010101010101010011010101010010011111111100100011001010010011100010011111100010111110101101100100011000110001110111001010000010011011010001111100100100001"
                    "001100111101001010101001110011010011101101011111001010110101001111010101010010000001101001100001100100001101001000100110101100100111101010111110100011111010110101110110000010111001100000101"
                    "100011001111010010111010111000010001100000000001110010001100110000001110001110110010001110100000010111111000111100010000101110101100111010100111001110100010101110000011110010000011101011101"
                    "010101010011100100100100011010011001111101000100111010011100101101111010100111101100001001001110011101011111111000010000111110100101010110111110111011110011101111010010010011000111011110001"
                    "001010000001011000110101110100011001001101011000111010110111000111011001000110101011101101010011001011001101110011010000111011011100111110111001000011010010111111001001010100000110100001001"
                    "110110001100101110110000101011110111010110011101010010010110101111101101110110010001101001001110011000111001110001011110101100110010000100110101001110101010011100101101001011100101001111101"
                    "001101010111101010111101011111110010000100011001011010111101010011110110110110001010101111111011111111110111100010100010110011101101101111111001101001110010110110110110100000011001001000001"
                    "110000001010000110110110111100010100110110110111010011001010100000011010011011100000101111011110110111000101101101110000100110001000001101111101100111011010110010101110111100001001010001101"
                    "000101110111010000100100001011100100110111011100110010010100111010000111101001101111001110101011000110001011010001001010110101001111001111101010011010001011101001110110100001000100110011101"
                    "000011011101101010100100100000110101101100000010110011110000010010001001110100110010101000001011011110011111100101011010111110101111010111011000000111011011101101001010111011100100010110101"
                    "111000011010010110111001010110011011111110110011011011011111000111011101111111101101101001011101100000000000001011000100101000100100000011000010110010110010111100101111110011010111100100101"
                    "000100110101111110101000110000011101000101110111000010100001110011111111010011001010001000001011111001101110011011100110100111010100000111010011100111010011111110000101001110101101111011000"
                    "100011000011010010111111111111111111011100110111111011111111111111111100110101110001001111111111111111111000100101000010111111111111111111010010111001000011111111111111111100001110011011101"
                    "001001110011011100000000000000000001000101000011101000000000000000000110101011011011000000000000000000011101000110101010000000000000000001101010101010101000000000000000000110101010101010100"
                    "110101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101011001110010100111101101100100101"
                    "101100101101010010101101111100010001001111001100010100001110101010110111100100111000110011001101000001010000001011011001000110111001110101001111000111110111101011001110110111100000010000010"
                    "111110100101011011110100010001010001011100000011011010011101101010010110110010111110101000011110110000011111110111000010000011000011110001011101100000001001110001001000110100100111001000111"
                    "110001100000011010010100111000001101100011010110011010101111001000000100101110100110000000010001011100011100010001101001100100111001010001101110101011111010000011010100110110000100001111010"
                    "101101101101010100111110110101111001000110011000000110000000111110000110101110010001110110000011001101011001001011000111000010100110011101110011100110000111111101101110100100000010001001011"
                    "110010010111100101111111110010100101110111000001100110100110001101010101000101110011011000010010100001011101001011110111101000010011101001111001000110000010111110011111010101000000001000101"
                    "100010110110100101101000100110000001011000011011001010001111100101000100110011101011000101110000001101010010111100001110100011101101111001110010101111101000101000110011010101010011010000010"
                    "101010001000001110101111011100000001011110000001011101101110100001100100101110010100101101011011000010010110010010011010010010101000110001001000101010001110010011000111010110110111100101010"
                    "111000111010010111101011001111100101001110011110111111001100110010110100001100110111111111001100010010010111010101110110110011011011011001101100100111111001001111111010110100001101111001110"
                    "101010001110010100010100010101101001111101111000101000010101101101100110110000010101110010010111001111011101010111011100001100010111101001110101010101111001110110000110010110010111111011111"
                    "101111111000111100000111001101101001100101100111011111100011011010110110110000010011010010011110100101011100001001100100111111000000001101010000101001001001110000000110010111101100101001011"
                    "111000100101010110000011000010111101111011010000111000111001001010010110101001101100011000100010101111010001010101100000100010110101000101100001100101001011010011101010000110000010010101110"
                    "110110000000111010010100111111100001000010100000011001101010011110000110011111011010101010100011010001011011001001110011111011111101011101011000010000111010011111110001000111111100101001101"
                    "101010111101100010101010101110111001001100101010101010101010101010100110101010101010101010101010101010011010101010101010101010101010101001101010101010101010101010101010100110101010101010101"
                    "110101010101010101010101010101010101010101010101010110111001000110110110001001001110101011001110001011011010000011011110100100111001100101100010001010111000000110111100100111111011100101110"
                    "111111111111111111100100001001111001111111111111111110110111111001100111111111111111111110011100111011011111111111111111111001011111100101111111111111111111111010100110110111111111111111111"
                    "000000000000000000100101001101010000000000000000000011100100010100110000000000000000001001000010110110000000000000000000110110000011101100000000000000000010001100001010110000000000000000001"
                    "010011001011111100111010010110100111001000000011110011010100011011011001010000010010101100000000101100110110010000001110100101110011100000100001001110010011100101010010101110110011111011001"
                    "111101010101101010111000110010000100101010101001000011001110101110101111001011110001001011111000100110111011001101111000101001001100011111110110011000100010001110011011110111011001110101101"
                    "101011100110101100100101101110100100111001101001100010011111110000110011011100110111001001010101101111001101011101100000100011010100000000000010101100011011110010110110000100101100100100001"
                    "101000110101100100100110100100111110101011100111001010101011011001100111101111101011101000001001100110110001001010011000111110111101001001000111100000000011000101011100111010110000000101101"
                    "101010000110100100101111110101010110110000110011101011001010101101101000001001000101101001010100110111001011010100100100110000000000011011011111010111110011110000100000100111101100101001001"
                    "111111110101011110111100010101110010100110101010001011001000101110000010110111011001101111001111100100110010110110001100101100010000000001100010000010101010010010011001110000110100001011001"
                    "010110101011011000110101101000001001101001110011101011010011100010111001001000000111101011100010101000011100110001001000101100010101010100001101010000011010101101100111111110010100001010101"
                    "010011101100011000101010100111000101110011001010111010001011011011001010101011000100101010100111111111101010011011101110111011000010011101100011101101100011001100100010010111110010110100101"
                    "101100100011001010110001111100111000010011000111010011101001001000001100110100110110001110011011010000011110000111111010110101001111011110000101101000000011100000001110010101001000100100101"
                    "100101101000011010110011000111101100101100011011100011100011000000111001011100110111101010110011101010100010101001010000101100110110111110010010110010011010100011100111010010101100010111101"
                    "110010101000010100101110011101001000000100000010010010001100100100000000010011101100001100011100010100101011000100010100110001111111011010100010101010100010111000111000101010101010101010101"
                    "010101010101010100110101010101010101010101010101010011010101010101010101010101010101001101010101010101010101010101010100110101010101010101010101010101010011010101010101010101001100111000001"
                    "110010001010101100110001110110001101011011101111110011111000111111000100010011011100101001011000011010101010000001110110110011010000010010011000000110110011000110001110001011100010010011101"
                    "000000000001011110110010110000000010010011010011011011111110111011011001001111011111001111010110001000011011101010110010101111001010101110000101011011000011011110110000010010101111010111101"
                    "001110110010011100101101010101000001110011101001010011000001100101111010101111000110101011010010010100010111011101001010101101001011101100000100000110001011001100010010000000100101000000000"
                    "101001010101000000111111111111111111110111000000111011111111111111111110001011101010001111111111111111110001101101001110111111111111111111010101001100110011111111111111111111110010111011001"
                    "000100001001001110000000000000000001010011100010001000000000000000000101101111000011100000000000000000011001001001110100000000000000000001000111011011000000000000000000000101100100010111100"
                    "110001111000000010011011010001011101001001001111110011001100000001100111110010100001011111100011001101010111011011100001110110111100111001001001001010011111011000111101010111111001111100111"
                    "100110010110001010111110001101000001111111001101110010110000111111110111010111001000111010001011100110010001111110101111000100110001010101010110000011000011000011010001010101100101010010001"
                    "111100000110000110100111011110001001110100111000111011101000001110110100101100001000111011010000101000010001000111101010011110001011100001110011010100110111110011011011010111101000000100001"
                    "101110101000000011111011101101110101000010001001000101010010001001100110100110001011011000000001011000010000001001011010111010000111011101001000100101101101111011000001000110001101101100010"
                    "100011011100010100010111111010111001111010100111000010010101111101100111110001101111001000000101011001010100110001001010110100101010101001001101110001100011100000011011100101110000000110111"
                    "100110111101011101110101101010011001001111000100000111001000010100100111010001001110111111000100110110010100001100011010011010101110011101111001010010110011010000011010110110001001010001101"
                    "100011011111011101000000101001010001101101101110110100111110101010010110111001111101000100010100100101011010000110100111011101100000011101001111100010000101001011110000010111110000101111111"
                    "101100001011111111111111011110010101000001101101101001100101101111100101111000100101010010000001010111010101100101101100100100100011011101101001011010100110100111001100010100011010110010111"
                    "100001001101001011001001110101110001101001010101000101000001110001010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "101010101010101010101010101010101001101010101010101010101010101010100110111011001110101101111010011111011000011101111001000111010100111101100001000001110110010010000111010101101010011101011"
                    "101111010011010010000111101101100001100110111110000010001010001101110111000011101001101101011100000001011010101100100110100110110110011101010100011001101101101101010001000100111011110010011"
                    "100110110110110101101010011101101001001011111000110010110111001111000111000011010111011001001001110011011100111110111000100000001110110101100100101001010110000101100111110101000001001001100"
                    "101110111000000100001101001100001101100111001010101001010111000001010101010010000100000111110000110111011110011011001011011000010001010001100110001111110001001010111000110101001011110100111"
                    "101110011100111101100111011000101001010101011100111011010110110110110100110011000101100111111101011011011101011100111011110011010000101101111100001110011101101111110101010111010101111011100"
                    "110111011000010100100010101100010001111001010010011110111111100110000100111001000000111100111001010010010001011100111010011111001001110001111000001011001111010011110100000100011000110010101"
                    "111111111111111111101101110011001101111111111111111111100100010101010111111111111111111001001100110011011111111111111111101010111101010101111111111111111110101111100000000111111111111111111"
                    "000000000000000000110100101011001100000000000000000010010000111000110000000000000000001010111100101111000000000000000000101011010010011100000000000000000011011100101100100000000000000000001"
                    "011001010000110100111010100010100010111100010011001010001110110100001100110001011010001000001010100111111010000001100100101110001101000000111110100011111010111111000110101011011001010111001"
                    "111111110100100000111110010000110000001011111100100010011111010100101000101111000101101111010101001011110101111100111110101100010011111000001001100101101011001101110110101101011110100110001"
                    "110011111011000100101010000111010110101110100000010010011110000110111010011111000011001001000100001010001001011001001000110101010111110111101100110111000010011101101011110010111101011000001"
                    "100011011000011010110111111111100000111001000001101010101111001010111101001011111001001011101101010001010001010110111000101101110101110100010011001011010010111111101101100100110101100001101"
                    "101010100010100110111010110001001110011100011100000010111001000110010011001011111100101110100011110111001001101011010100100110011011001001010001011110000010101000101101111101111010100110101"
                    "011010011100111000110100011110001100101000011110100011010111101100111101101001011010001100110010001110101001000010010110100010101110011111001010111011001011110101010101111011101011100101001"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "010001111001111010101111111110110110010110011110000011000001010001101001000001000101001010001010011100001110100011001000101101101101001010001001101010011011001100110111000111011100010110101"
                    "110111101000000110111011010100000011000110110111100011101100101001001011000111001111001010001011010101001010100011011100100001110110111011110101001111010010101011001101011111111101011000101"
                    "101111111100000110111101010011001000010111001000001011111010001011111011111100001100101110100000001010101011010110000110111001001001001000001110010100011010110101010010101001000000111001001"
                    "000000111010010100111110000011011011111000011111010011011011011100111111010011111000001011110001100101010110000011010010111100011100101101011000010000010010001110111010111110011110110111001"
                    "111011010111101110101111010111100110101100101100000011100000010000110110000011100101001011111011000011100100000110111010110010110001010110010010111001010011101010010000111000101110010001101"
                    "101100010010011110111101111010011001011010001111011011000000001101110111111001110001001101101111101110001100111010110100110111011000011110001000011001001011110110111011000100100000101110101"
                    "111000001010001000101010001111011001010000000101000011100011111011011100101011011110101100100010101111101000011000100100111000100111100110000000000011110010000110111010011000010110110000101"
                    "001000011000001100101110100010101100000001000111010011101010001011000100111001101010101101000101110111001110101010110100100100011000011010010000110011010011111011000010111000100100100101100"
                    "101100010100000000111111111111111111111010001101110011111111111111111101111101001111101111111111111111110101011100010110111111111111111111101000100000100011111111111111111100101110101101101"
                    "000001010101000110000000000000000001100100010010000000000000000000000101110010111111000000000000000000010011000101010100000000000000000001111011011000010000000000000000000100101100001011000"
                    "101101111001000111000000001100010101010100110111101010011000000010100111011011010010101000100101001001010101111100001001000111101100100101100100110011011001111101111001110101111111111011011"
                    "101000011100011010000001001011010001100010100010111001010001010100010100101010111111001001100100101010010110110110111100011001010001011001011110011001101010110001000010010111010011011010001"
                    "100001101000100111001000000101000001100111110110101110001100111101000111000010001111101100111110001100010101011010011010110010000101111001110111111111111001001011101000010111010011100110110"
                    "101011101110001000100101101011000101011001111010001110000001011010100110000010101000100110101010101010011010101010101010101010101010101001101010101010101010101010101010100110101010101010101"
                    "110101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010110110111011100011010111110101100111001111101110010001100101011100111100111100100010101000100110"
                    "110010010101111101010000111010101001001101001011110001110011001100010110011100010110000101001100010110011100111101110101000110100101011001100101101100111010000101010101010100110110001100110"
                    "100010010010010111011011100100100001100010011001111111100111101000110111000000110101110101111001000010010111101010110101111000001101110101100000111010100101001011001000100110101001101111001"
                    "100100011001011101110111110001000001110011010111000101100110011010000100101001010101011001001001110010011101010011001111000000000110001101011011000011000001001001000111100110110111010101110"
                    "111000100101101001011110010110111101001111111000001011101010111110110101010001101001011000000110000100011000101110011000110110000011111101001001101010101011110110100101000101101111100110010"
                    "100011101100111001101001110100011001101111001111110011100111011110110100000001111010111010100000011001011100110010001110001011011010011101010101100111100011001100010111010101011001101011010"
                    "110010110010011111001010000001110001100010100010010001010010001000100101101001011101000010000110101011010100010011101100111110100011111101010111001100000000110011101001000111001010000011111"
                    "101111010010111011101011110000011001100010000010100101100111000010000101000000101011110100010010110101010100100110101001100100111011010101111111111000111100001001001011110100100101101100101"
                    "111100011100001101101111110100011001010011110001011110111001011001100101111111101000001001010000010110010110101001110010001001110101111101110110011010011011010001000011010100101000101100000"
                    "111011010001111000101101000110010101101111001010010011101111110101000111100101000110000100010011111111011001110010101000001000011000111001110011111000111111000111111001010111011010011100011"
                    "100111000001100000110010000000110101111000101001100010101111001111010100110011011110010111010101110110011110111110100011111111011111001001111110000100000111010000011000100110010110100001101"
                    "111111111111111111101001100101100001111111111111111111101000011111010111111111111111111010101011110001011111111111111111110111000100111101111111111111111111101100111010100111111111111111111"
                    "000000000000000000110100100110110100000000000000000011010100001001010000000000000000001101011011000111000000000000000000101000001010000100000000000000000010101100100101010000000000000000001"
                    "111100001011101000101100111111110010000000011110110011010001101011110111010111110110101100101001101100101000000110010100101111110100101001000100101001010010000111100111001010111011101101001"
                    "110000101110010100110101010101010101010101010101010011010101010101010101010101010101001101010101010101010101010101010100110101010101010101010101010101010011010101010101010101010101010101001"
                    "101010101010101010101110000000010001110100101101111011000011010000111001000001101010001100001110010100110101100110101110100001110110110001111001000110111011010110110011000101011001111110001"
                    "111110000010100010110101110110101101011010010000101010101100011011100011001101100101001111001100100001000010101111111010110001001111100111011010010001110011001001000100001100111010001111001"
                    "111111001011011000111011101011100111000011000111101011111111111001111110011110010110101001110010011000111001000001100000101111100110010101010110001011000011010101000101111000001010010110101"
                    "001010101100100110110001000001101000110011011000100010101110110111101111001010101011101011010010111000000110010100000110100000001001001101011001100010011010101011000010001101100100000000001"
                    "001110001001110000100110110001000110010001000111110010110011000001011100111101100110001001010011001110111100011111010110111011011010010100100000001001001010000101100110010010000011101001001"
                    "101000110011001110110100111000000111011001011011011011010111000001011101000011111000101010101100011010111110010011110010110001010010111010100111110111111010111100110000100101111001000001101"
                    "101010100101110000100000101111000100100011010101001011010010001100001000110000111011101111101010111000111011101110010010110110110001010010011001111001101010001011100000110011011110100010110"
                    "000000001100001110111001010000001011001011000010000011100111101000000110011001111110101010000001100010111110000001000100111101110011100000010110010100100010101011111011110101000000100000000"
                    "111111101111100110101011101111100101011100001000111011011010010110001011010100000101101111000110100111001111111011011010110111000000110010100001010101010010110100101100010100000011001110101"
                    "000000101010100010110000100011101010110011110001100010010111100010001010111010100000101001010011111100101110101000101110110101000101000011101101110111000011100100111100110010100111101110101"
                    "111110100001101010101001011111100101000001111101011010001111001001010110100100100100101000100010111011110110001100110010101101001101001110100100011011000011001110101001010111111001001110101"
                    "000010101101100110111111000100110100010111110000111011010010011101000001111111101111001010001100100100000011011110100000100010111101011100010101000000100010011100111100000010000010000000101"
                    "111010100010101100100001000111011001111101000101100010010010110010011100110011010010101111011001001011011110011110101010101110111100011111011000001001101011010100101111100010111011001111101"
                    "111010101011011100110101010010111000001100010010111011010101010101010001010101010101001101010101010101000101010101010100110101010101010100010101010101010011010101010101010001010101100000001"
                    "111010101010101010111111111111111101001010101010101011111111111111110100101010101010101111111111111111010010101010101010111111111111111101001010101010101011111111111111110100101010001111111"
                },
        /* 18*/ { UNICODE_MODE | ESCAPE_MODE, -1, -1, 4 << 8, "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。它的主要技术特色是：1． 具有高度的汉字表示能力和汉字压缩效率。2． 信息容量大。 3． 编码范围广，可以将照片、指纹、掌纹、签字、声音、文字等凡可数字化的信息进行编码。4． 支持加密技术。5．抗污损和畸变能力强。6．修正错误能力强。7 ．可供用户选择的纠错能力。8．容易制作且成本低。9．汉信码支持84个版本，可以由用户自主进行选择，最小码仅有指甲大小。10． 外形美观。", 0, 67, 67, "Previous draft Figure 1 **NOT SAME** different encodation, Binary mode not used by figure",
                    "1111111000010101011111111111111110001011011110010101111111001111111"
                    "1000000010000100000000000000000010100011101001010001101110000000001"
                    "1011111001100100110100001101111011111100101010000111111111101111101"
                    "1010000011010001001000011101000011111110111001011111100101000000101"
                    "1010111000011101100100100001110011111101011101101011011111101110101"
                    "1010111011110000000100000101110011100011100011100011110100101110101"
                    "1010111011000110000011001011001011110110001010100101101111001110101"
                    "0000000000010011100110010011011011000111101101001111111101000000000"
                    "0010101100110010001100111111010011001110010110000110100100111010101"
                    "1100010010001111111101001101001011111010010100110011001010010001100"
                    "1110001011001110000010010100101010110010010010111010001000000101000"
                    "1000010011000111111001000011111011101100011010010111110011110110111"
                    "1011101100011011001010101101100011111000011110101001011101101100100"
                    "1010101100001100101111110100010010110010011100000101100110011010100"
                    "1011100110100101010000001101011011001111001001011101101001010000010"
                    "1000110001000110010011101001111010011100010101100010001100001100000"
                    "1110111000011000010011111001000010011001001000111000000010000011100"
                    "1111111111111111011111011011011011111111111111111110101001010000101"
                    "0000000000000001101010110001000000000000000000000101110111011010000"
                    "1011000000111001101001011111000001001110001001110101011111101010000"
                    "1101111001111001101011110110000111101111111001110100111100001000111"
                    "0000010100000001111111001100011100100000001100100100111010110101000"
                    "0000000001011001010101010111101100010011010110110111100001001001001"
                    "1111100011110001010010111011111001010010000100100110111001111001110"
                    "0111000001000001101111001001110101001011111111000111000100111101001"
                    "0110100011111001110101110000010111101100110011110100101111011100011"
                    "1100110110011101000101011100101101010011100101010100100010100001100"
                    "1110000110010001001110011101110000011010100011110101011000011110111"
                    "0110111001111001000110110111100110111011101011100111111110110111010"
                    "0110001110100001000001011100011110101111101000000101000110011100000"
                    "0011100000010001010111100100001001111100101000110101001000101100001"
                    "0011001100010101010010000111000011110111000111100100001110000111110"
                    "0000101111100101010011001110010001011000000011100111010010111000010"
                    "0011101110110001011110110110000111011001000111100110101000000000100"
                    "1001100100000001111111111111111110111001100101110111111111111111111"
                    "0001010111111100000000000000000010011111111111110000000000000000001"
                    "1001000011000011000000100100011011100110100001010101100010010001101"
                    "1111010101001110010000110000000010110110001100111011001001000101001"
                    "1110010110101110101100101001001011000100000101010100000110111011001"
                    "1010100110000000010001111110110010111101101100000110110011110001001"
                    "1000111010101011001100101101010011100010110100111011110001100000001"
                    "1001000011100101100000001010111010001110110110100110100100000100101"
                    "1000111011000001101101100011001011101111110100010100101011110011001"
                    "1011111101100010000011100110111011111110110011100010011011101110001"
                    "1011000000001111011100100110011011010010111110000100100011100010001"
                    "1101100010000101111010111000100011110100101010110111011010110001001"
                    "1001000011001110100101100010101010111100101100011100010100111010101"
                    "1000000011010101101010100011111010011000011111111010100001101011001"
                    "1101000110100100011111001110111011001000000100100000111110010110001"
                    "1111110100001111111101100110111011010110010100000001101011010010101"
                    "1000001100001001001110011110111010001100000011011100101100100011000"
                    "1111111111111111111011011001111011111111111111111111100111000010001"
                    "0000000000000001011000100101010000000000000000000110011010110100100"
                    "0001110001101101011100101111100111111000010010000101000001011100110"
                    "0111010100110001000010100001101000001001010110000111111101100011100"
                    "0011011001010101001111011111001101101100101101010100101111001111110"
                    "0001101100100101011100001110111010011110010010100100100001000101001"
                    "1001011100010001101010010110011000110110111100000101110110000010000"
                    "1010101111000101011011011010010110000010111000110100011001011010100"
                    "0000000001101101011110101100001101100101100001110110110111000000000"
                    "1111111000111101000010111100100101111011111010010110011101101110101"
                    "0000001010111001111001001100110011010101111100110100010010101110101"
                    "1111101011100101001111011101111011001000100010100111101010001110101"
                    "0000101000011101111001100110101001011110101110010110100010100000101"
                    "1110101010010101000111011101001011100010000001100100010100001111101"
                    "1110101001101001000100110010101000000101000110010110110110100000001"
                    "1110101000010001111111111111111010010111110110110111111111001111111"
                },
        /* 19*/ { DATA_MODE, -1, -1, ZINT_FULL_MULTIBYTE, "é", 0, 23, 23, "Mask automatic (01)",
                    "11111110100010101111111"
                    "10000000001010000000001"
                    "10111110111010001111101"
                    "10100000100101000000101"
                    "10101110001111101110101"
                    "10101110110100101110101"
                    "10101110011101001110101"
                    "00000000101010100000000"
                    "00010101101101001010101"
                    "00001011010010000111010"
                    "10100010010010101010010"
                    "10110101010101100010110"
                    "10101000010110101010100"
                    "10011111000110000101000"
                    "10101010000001110101000"
                    "00000000101001100000000"
                    "11111110001000001110101"
                    "00000010110000101110101"
                    "11111010110001001110101"
                    "00001010010000100000101"
                    "11101010010100101111101"
                    "11101010001001000000001"
                    "11101010101010101111111"
                },
        /* 20*/ { DATA_MODE, -1, -1, ZINT_FULL_MULTIBYTE | (1 << 8), "é", 0, 23, 23, "Mask 00",
                    "11111110001000001111111"
                    "10000000111111000000001"
                    "10111110110000001111101"
                    "10100000010000100000101"
                    "10101110100101101110101"
                    "10101110000001101110101"
                    "10101110010111001110101"
                    "00000000111111100000000"
                    "00010101100111110010101"
                    "01011110000111010010000"
                    "00001000111000000000111"
                    "11100000000000110111100"
                    "00000010111100000000001"
                    "11001010010011010000010"
                    "10101001101011110101000"
                    "00000000111100100000000"
                    "11111110000010001110101"
                    "00000010100101001110101"
                    "11111010111011101110101"
                    "00001010100101000000101"
                    "11101010011110101111101"
                    "11101010011100100000001"
                    "11101010000000001111111"
                },
        /* 21*/ { DATA_MODE, -1, -1, ZINT_FULL_MULTIBYTE | (4 << 8), "é", 0, 23, 23, "Mask 11",
                    "11111110000101101111111"
                    "10000000111000100000001"
                    "10111110110000101111101"
                    "10100000101000000000101"
                    "10101110111101101110101"
                    "10101110100110001110101"
                    "10101110101110101110101"
                    "00000000111000000000000"
                    "00010101100000111010101"
                    "01000100011111101010111"
                    "10010010100111000111100"
                    "00100101101111110000100"
                    "01000111010010111000110"
                    "00001101111110010111010"
                    "10101011111001110101000"
                    "00000000001110100000000"
                    "11111110110000101110101"
                    "00000010001000101110101"
                    "11111010110110101110101"
                    "00001010001000100000101"
                    "11101010101100101111101"
                    "11101010100110100000001"
                    "11101010111010001111111"
                },
        /* 22*/ { DATA_MODE, -1, -1, ZINT_FULL_MULTIBYTE | (5 << 8), "é", 0, 23, 23, "Mask > 11 ignored",
                    "11111110100010101111111"
                    "10000000001010000000001"
                    "10111110111010001111101"
                    "10100000100101000000101"
                    "10101110001111101110101"
                    "10101110110100101110101"
                    "10101110011101001110101"
                    "00000000101010100000000"
                    "00010101101101001010101"
                    "00001011010010000111010"
                    "10100010010010101010010"
                    "10110101010101100010110"
                    "10101000010110101010100"
                    "10011111000110000101000"
                    "10101010000001110101000"
                    "00000000101001100000000"
                    "11111110001000001110101"
                    "00000010110000101110101"
                    "11111010110001001110101"
                    "00001010010000100000101"
                    "11101010010100101111101"
                    "11101010001001000000001"
                    "11101010101010101111111"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[8192];

    testStart("test_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_HANXIN, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITERATIONS    1000

// Not a real test, just performance indicator
static void test_perf(int index, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_HANXIN, UNICODE_MODE, -1, -1,
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。",
                    0, 43, 43, "98 chars, Region 1 and Text" },
        /*  1*/ { BARCODE_HANXIN, UNICODE_MODE, -1, -1,
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。",
                    0, 121, 121, "980 chars, Region 1 and Text" },
        /*  2*/ { BARCODE_HANXIN, UNICODE_MODE, -1, -1,
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。"
                    "汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。汉信码(Chinese-Sensible Code)是一种能够有效表示汉字、图像等信息的二维条码。",
                    0, 147, 147, "1470 chars, Region 1 and Text" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    clock_t start, total_encode = 0, total_buffer = 0, diff_encode, diff_buffer;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) {
        int j;

        if (index != -1 && i != index) continue;

        diff_encode = diff_buffer = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            struct zint_symbol *symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

            start = clock();
            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

            ZBarcode_Delete(symbol);
        }

        printf("%s: diff_encode %gms, diff_buffer %gms\n", data[i].comment, diff_encode * 1000.0 / CLOCKS_PER_SEC, diff_buffer * 1000.0 / CLOCKS_PER_SEC);

        total_encode += diff_encode;
        total_buffer += diff_buffer;
    }
    if (index != -1) {
        printf("totals: encode %gms, buffer %gms\n", total_encode * 1000.0 / CLOCKS_PER_SEC, total_buffer * 1000.0 / CLOCKS_PER_SEC);
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_options", test_options, 1, 0, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_perf", test_perf, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
