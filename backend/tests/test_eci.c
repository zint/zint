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
#include "../eci.h"

static void test_bom(int debug) {

    char data[] = "\xEF\xBB\xBF‹"; // U+FEFF BOM, with U+2039 (only in Windows pages)

    char expected[] =
        "111111100001001111111"
        "100000101110101000001"
        "101110100000101011101"
        "101110100111101011101"
        "101110100110101011101"
        "100000101011001000001"
        "111111101010101111111"
        "000000001100100000000"
        "000011110110101100010"
        "010011011100000100001"
        "111110110001011111000"
        "000110000110001011100"
        "000111110111100001011"
        "000000001011001000111"
        "111111101010111001010"
        "100000101110101101010"
        "101110101110001110101"
        "101110100001100101001"
        "101110100111111111100"
        "100000100010011010111"
        "111111100101101000101";

    int length, ret;
    struct zint_symbol *symbol;

    int width, height;

    testStart("test_bom");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    symbol->symbology = BARCODE_QRCODE;
    symbol->input_mode = UNICODE_MODE;
    symbol->option_1 = 4;
    symbol->option_2 = 1;
    symbol->option_3 = 5 << 8; // Mask 100 (instead of automatic 010)
    symbol->debug |= debug;

    length = (int) strlen(data);

    ret = ZBarcode_Encode(symbol, (unsigned char *) data, length);
    assert_equal(ret, ZINT_WARN_USES_ECI, "ZBarcode_Encode ret %d != ZINT_WARN_USES_ECI\n", ret);
    assert_equal(symbol->eci, 21, "eci %d != 21\n", symbol->eci); // ECI 21 == Windows-1250

    ret = testUtilModulesCmp(symbol, expected, &width, &height);
    assert_equal(ret, 0, "testUtilModulesEqual ret %d != 0, width %d, height %d\n", ret, width, height);

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_iso_8859_16(int debug) {

    char data[] = "Ț"; // U+021A only in ISO 8859-16
    int length, ret;
    struct zint_symbol *symbol;

    testStart("test_iso_8859_16");

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    symbol->symbology = BARCODE_QRCODE;
    symbol->input_mode = UNICODE_MODE;
    symbol->debug |= debug;

    length = (int) strlen(data);

    ret = ZBarcode_Encode(symbol, (unsigned char *) data, length);
    assert_equal(ret, ZINT_WARN_USES_ECI, "ZBarcode_Encode ret %d != ZINT_WARN_USES_ECI\n", ret);
    assert_equal(symbol->eci, 18, "eci %d != 18\n", symbol->eci); // ECI 18 == ISO 8859-16

    ZBarcode_Delete(symbol);

    testFinish();
}

// Only testing standard non-extended barcodes here, ie not QRCODE, MICROQR, GRIDMATRIX, HANXIN or UPNQR
static void test_reduced_charset_input(int index, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int eci;
        char *data;
        int ret;
        int expected_eci;
        char *comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS
    // Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS
    // ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS
    // … U+2026 in Win pages (but not in any ISO 8859)
    // テ U+30C6 katakana, in Shift JIS
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /*  1*/ { BARCODE_C25STANDARD, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /*  2*/ { BARCODE_CODE39, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /*  3*/ { BARCODE_EXCODE39, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII only" },
        /*  4*/ { BARCODE_EANX, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /*  5*/ { BARCODE_CODABAR, UNICODE_MODE, 0, "AéB", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /*  6*/ { BARCODE_CODE128, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*  7*/ { BARCODE_CODE128, UNICODE_MODE, 3, "é", ZINT_ERROR_INVALID_OPTION, -1, "Does not support ECI" },
        /*  8*/ { BARCODE_CODE128, UNICODE_MODE, 0, "β", ZINT_ERROR_INVALID_DATA, -1, "β not in ISO 8859-1" },
        /*  9*/ { BARCODE_DPLEIT, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /* 10*/ { BARCODE_CODE16K, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 11*/ { BARCODE_CODE16K, UNICODE_MODE, 3, "é", ZINT_ERROR_INVALID_OPTION, -1, "Does not support ECI" },
        /* 12*/ { BARCODE_CODE16K, UNICODE_MODE, 0, "β", ZINT_ERROR_INVALID_DATA, -1, "β not in ISO 8859-1" },
        /* 13*/ { BARCODE_CODE49, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII only" },
        /* 14*/ { BARCODE_CODE93, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /* 15*/ { BARCODE_FLAT, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /* 16*/ { BARCODE_DBAR_OMN, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /* 17*/ { BARCODE_DBAR_EXP, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /* 18*/ { BARCODE_LOGMARS, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /* 19*/ { BARCODE_PDF417, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 20*/ { BARCODE_PDF417, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 21*/ { BARCODE_PDF417, UNICODE_MODE, 3, "\302\200", ZINT_ERROR_INVALID_DATA, -1, "U+0080" },
        /* 22*/ { BARCODE_PDF417, UNICODE_MODE, 3, "\302\237", ZINT_ERROR_INVALID_DATA, -1, "U+009F" },
        /* 23*/ { BARCODE_PDF417, UNICODE_MODE, 0, "˘", ZINT_WARN_USES_ECI, 4, "In ISO 8859-2 and ISO 8859-3 only of single-byte pages" },
        /* 24*/ { BARCODE_PDF417, UNICODE_MODE, 4, "˘", 0, 4, "" },
        /* 25*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Ħ", ZINT_WARN_USES_ECI, 5, "In ISO 8859-3 only of single-byte pages" },
        /* 26*/ { BARCODE_PDF417, UNICODE_MODE, 5, "Ħ", 0, 5, "" },
        /* 27*/ { BARCODE_PDF417, UNICODE_MODE, 0, "ĸ", ZINT_WARN_USES_ECI, 6, "In ISO 8859-4 and ISO 8859-6 only of single-byte pages" },
        /* 28*/ { BARCODE_PDF417, UNICODE_MODE, 6, "ĸ", 0, 6, "" },
        /* 29*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Ж", ZINT_WARN_USES_ECI, 7, "In ISO 8859-5 and Win 1251 only of single-byte pages" },
        /* 30*/ { BARCODE_PDF417, UNICODE_MODE, 7, "Ж", 0, 7, "" },
        /* 31*/ { BARCODE_PDF417, UNICODE_MODE, 0, "غ", ZINT_WARN_USES_ECI, 8, "In ISO 8859-6 and Win 1256 only of single-byte pages" },
        /* 32*/ { BARCODE_PDF417, UNICODE_MODE, 8, "غ", 0, 8, "" },
        /* 33*/ { BARCODE_PDF417, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "In ISO 8859-7 only of single-byte pages" },
        /* 34*/ { BARCODE_PDF417, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 35*/ { BARCODE_PDF417, UNICODE_MODE, 0, "‗", ZINT_WARN_USES_ECI, 10, "In ISO 8859-8 only of single-byte pages" },
        /* 36*/ { BARCODE_PDF417, UNICODE_MODE, 10, "‗", 0, 10, "" },
        /* 37*/ { BARCODE_PDF417, UNICODE_MODE, 11, "Ğ", 0, 11, "In ISO 8859-9; Note no characters in ISO 8859-9 that aren't also in earlier ISO pages" },
        /* 38*/ { BARCODE_PDF417, UNICODE_MODE, 12, "Ĩ", 0, 12, "In ISO 8859-10; Note no characters in ISO 8859-10 that aren't also in earlier ISO pages" },
        /* 39*/ { BARCODE_PDF417, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 13, "" },
        /* 40*/ { BARCODE_PDF417, UNICODE_MODE, 13, "ก", 0, 13, "" },
        /* 41*/ { BARCODE_PDF417, UNICODE_MODE, 14, "A", ZINT_ERROR_INVALID_DATA, -1, "Reserved ECI" },
        /* 42*/ { BARCODE_PDF417, UNICODE_MODE, 0, "„", ZINT_WARN_USES_ECI, 15, "" },
        /* 43*/ { BARCODE_PDF417, UNICODE_MODE, 15, "„", 0, 15, "In ISO 8859-13 and ISO 8859-16 and Win 125x pages" },
        /* 44*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Ḃ", ZINT_WARN_USES_ECI, 16, "In ISO 8859-14 only of single-byte pages" },
        /* 45*/ { BARCODE_PDF417, UNICODE_MODE, 16, "Ḃ", 0, 16, "" },
        /* 46*/ { BARCODE_PDF417, UNICODE_MODE, 17, "Ž", 0, 17, "In ISO 8859-15; Note no characters in ISO 8859-15 that aren't also in earlier ISO pages" },
        /* 47*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Ș", ZINT_WARN_USES_ECI, 18, "In ISO 8859-16 only of single-byte pages" },
        /* 48*/ { BARCODE_PDF417, UNICODE_MODE, 18, "Ș", 0, 18, "" },
        /* 49*/ { BARCODE_PDF417, UNICODE_MODE, 0, "テ", ZINT_WARN_USES_ECI, 26, "Not in any single-byte page" },
        /* 50*/ { BARCODE_PDF417, UNICODE_MODE, 19, "A", ZINT_ERROR_INVALID_DATA, -1, "Reserved ECI" },
        /* 51*/ { BARCODE_PDF417, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /* 52*/ { BARCODE_PDF417, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /* 53*/ { BARCODE_PDF417, UNICODE_MODE, 20, "\\\\", 0, 20, "In Shift JIS" },
        /* 54*/ { BARCODE_PDF417, UNICODE_MODE, 0, "…", ZINT_WARN_USES_ECI, 21, "In Win 1250 and other Win pages but not in ISO pages" },
        /* 55*/ { BARCODE_PDF417, UNICODE_MODE, 21, "…", 0, 21, "" },
        /* 56*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Ґ", ZINT_WARN_USES_ECI, 22, "In Win 1251 only of single-byte pages" },
        /* 57*/ { BARCODE_PDF417, UNICODE_MODE, 22, "Ґ", 0, 22, "" },
        /* 58*/ { BARCODE_PDF417, UNICODE_MODE, 0, "˜", ZINT_WARN_USES_ECI, 23, "In Win 1252 only of single-byte pages" },
        /* 59*/ { BARCODE_PDF417, UNICODE_MODE, 23, "˜", 0, 23, "" },
        /* 60*/ { BARCODE_PDF417, UNICODE_MODE, 0, "پ", ZINT_WARN_USES_ECI, 24, "In Win 1256 only of single-byte pages" },
        /* 61*/ { BARCODE_PDF417, UNICODE_MODE, 24, "پ", 0, 24, "" },
        /* 62*/ { BARCODE_PDF417, UNICODE_MODE, 0, "က", ZINT_WARN_USES_ECI, 26, "Not in any single-byte page" },
        /* 63*/ { BARCODE_PDF417, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /* 64*/ { BARCODE_PDF417, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /* 65*/ { BARCODE_PDF417, UNICODE_MODE, 25, "12", 0, 25, "UCS-2BE ASCII" },
        /* 66*/ { BARCODE_PDF417, UNICODE_MODE, 0, "𐀀", ZINT_WARN_USES_ECI, 26, "Not in any single-byte page" },
        /* 67*/ { BARCODE_PDF417, UNICODE_MODE, 25, "𐀀", ZINT_ERROR_INVALID_DATA, -1, "Not in UCS-2BE (in Supplementary Plane)" },
        /* 68*/ { BARCODE_PDF417, UNICODE_MODE, 0, "テ", ZINT_WARN_USES_ECI, 26, "Defaults to UTF-8 if not in any ISO 8859 or Win page" },
        /* 69*/ { BARCODE_PDF417, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 70*/ { BARCODE_PDF417, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /* 71*/ { BARCODE_PDF417, UNICODE_MODE, 27, "@", 0, 27, "ASCII" },
        /* 72*/ { BARCODE_PDF417, UNICODE_MODE, 0, "龘", ZINT_WARN_USES_ECI, 26, "Not in any single-byte page" },
        /* 73*/ { BARCODE_PDF417, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /* 74*/ { BARCODE_PDF417, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /* 75*/ { BARCODE_PDF417, UNICODE_MODE, 0, "齄", ZINT_WARN_USES_ECI, 26, "Not in any single-byte page" },
        /* 76*/ { BARCODE_PDF417, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /* 77*/ { BARCODE_PDF417, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /* 78*/ { BARCODE_PDF417, UNICODE_MODE, 0, "가", ZINT_WARN_USES_ECI, 26, "Not in any single-byte page" },
        /* 79*/ { BARCODE_PDF417, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /* 80*/ { BARCODE_PDF417, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
        /* 81*/ { BARCODE_PDF417, UNICODE_MODE, 31, "A", 0, 31, "Undefined character set ECI - ignored for character set conversion" },
        /* 82*/ { BARCODE_PDF417, UNICODE_MODE, 170, "?", 0, 170, "ASCII invariant" },
        /* 83*/ { BARCODE_PDF417, UNICODE_MODE, 170, "@", ZINT_ERROR_INVALID_DATA, -1, "Not in ASCII invariant" },
        /* 84*/ { BARCODE_PDF417, UNICODE_MODE, 0, "\200", ZINT_ERROR_INVALID_DATA, -1, "Not UTF-8" },
        /* 85*/ { BARCODE_PDF417, DATA_MODE, 899, "\200", 0, 899, "8-bit binary" },
        /* 86*/ { BARCODE_PDF417, UNICODE_MODE, 900, "é", 0, 900, "Non-character set ECIs > 899 ignored for character set conversion" },
        /* 87*/ { BARCODE_PDF417, UNICODE_MODE, 900, "β", 0, 900, "Non-character set ECIs > 899 ignored for character set conversion" },
        /* 88*/ { BARCODE_PDF417COMP, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 89*/ { BARCODE_PDF417COMP, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 90*/ { BARCODE_PDF417COMP, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /* 91*/ { BARCODE_PDF417COMP, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 92*/ { BARCODE_PDF417COMP, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /* 93*/ { BARCODE_PDF417COMP, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /* 94*/ { BARCODE_PDF417COMP, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /* 95*/ { BARCODE_PDF417COMP, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /* 96*/ { BARCODE_PDF417COMP, UNICODE_MODE, 25, "12", 0, 25, "ASCII" },
        /* 97*/ { BARCODE_PDF417COMP, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 98*/ { BARCODE_PDF417COMP, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /* 99*/ { BARCODE_PDF417COMP, UNICODE_MODE, 27, "@", 0, 27, "ASCII" },
        /*100*/ { BARCODE_PDF417COMP, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*101*/ { BARCODE_PDF417COMP, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*102*/ { BARCODE_PDF417COMP, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*103*/ { BARCODE_PDF417COMP, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*104*/ { BARCODE_PDF417COMP, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /*105*/ { BARCODE_PDF417COMP, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
        /*106*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*107*/ { BARCODE_MAXICODE, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /*108*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /*109*/ { BARCODE_MAXICODE, UNICODE_MODE, 9, "β", 0, 9, "" },
        /*110*/ { BARCODE_MAXICODE, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /*111*/ { BARCODE_MAXICODE, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /*112*/ { BARCODE_MAXICODE, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /*113*/ { BARCODE_MAXICODE, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /*114*/ { BARCODE_MAXICODE, UNICODE_MODE, 25, "12", 0, 25, "ASCII" },
        /*115*/ { BARCODE_MAXICODE, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /*116*/ { BARCODE_MAXICODE, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /*117*/ { BARCODE_MAXICODE, UNICODE_MODE, 27, "@", 0, 27, "ASCII" },
        /*118*/ { BARCODE_MAXICODE, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*119*/ { BARCODE_MAXICODE, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*120*/ { BARCODE_MAXICODE, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*121*/ { BARCODE_MAXICODE, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*122*/ { BARCODE_MAXICODE, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /*123*/ { BARCODE_MAXICODE, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
        /*124*/ { BARCODE_CODE128B, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*125*/ { BARCODE_CODE128B, UNICODE_MODE, 3, "é", ZINT_ERROR_INVALID_OPTION, -1, "Does not support ECI" },
        /*126*/ { BARCODE_CODE128B, UNICODE_MODE, 0, "β", ZINT_ERROR_INVALID_DATA, -1, "β not in ISO 8859-1" },
        /*127*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*128*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /*129*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /*130*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 9, "β", 0, 9, "" },
        /*131*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /*132*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /*133*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /*134*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /*135*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 25, "12", 0, 25, "ASCII" },
        /*136*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /*137*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /*138*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*139*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*140*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*141*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*142*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /*143*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
        /*144*/ { BARCODE_CODABLOCKF, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*145*/ { BARCODE_CODABLOCKF, UNICODE_MODE, 3, "é", ZINT_ERROR_INVALID_OPTION, -1, "Does not support ECI" },
        /*146*/ { BARCODE_CODABLOCKF, UNICODE_MODE, 0, "β", ZINT_ERROR_INVALID_DATA, -1, "β not in ISO 8859-1" },
        /*147*/ { BARCODE_NVE18, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /*148*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*149*/ { BARCODE_MICROPDF417, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /*150*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /*151*/ { BARCODE_MICROPDF417, UNICODE_MODE, 9, "β", 0, 9, "" },
        /*152*/ { BARCODE_MICROPDF417, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /*153*/ { BARCODE_MICROPDF417, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /*154*/ { BARCODE_MICROPDF417, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /*155*/ { BARCODE_MICROPDF417, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /*156*/ { BARCODE_MICROPDF417, UNICODE_MODE, 25, "12", 0, 25, "ASCII" },
        /*157*/ { BARCODE_MICROPDF417, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /*158*/ { BARCODE_MICROPDF417, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /*159*/ { BARCODE_MICROPDF417, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*160*/ { BARCODE_MICROPDF417, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*161*/ { BARCODE_MICROPDF417, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*162*/ { BARCODE_MICROPDF417, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*163*/ { BARCODE_MICROPDF417, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /*164*/ { BARCODE_MICROPDF417, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
        /*165*/ { BARCODE_USPS_IMAIL, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers/dash only" },
        /*166*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*167*/ { BARCODE_AZTEC, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /*168*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /*169*/ { BARCODE_AZTEC, UNICODE_MODE, 9, "β", 0, 9, "" },
        /*170*/ { BARCODE_AZTEC, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /*171*/ { BARCODE_AZTEC, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /*172*/ { BARCODE_AZTEC, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /*173*/ { BARCODE_AZTEC, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /*174*/ { BARCODE_AZTEC, UNICODE_MODE, 25, "12", 0, 25, "ASCII" },
        /*175*/ { BARCODE_AZTEC, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /*176*/ { BARCODE_AZTEC, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /*177*/ { BARCODE_AZTEC, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*178*/ { BARCODE_AZTEC, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*179*/ { BARCODE_AZTEC, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*180*/ { BARCODE_AZTEC, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*181*/ { BARCODE_AZTEC, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /*182*/ { BARCODE_AZTEC, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
        /*183*/ { BARCODE_HIBC_128, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "HIBC ASCII subset only" },
        /*184*/ { BARCODE_HIBC_AZTEC, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "HIBC ASCII subset only" },
        /*185*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*186*/ { BARCODE_DOTCODE, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /*187*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /*188*/ { BARCODE_DOTCODE, UNICODE_MODE, 9, "β", 0, 9, "" },
        /*189*/ { BARCODE_DOTCODE, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /*190*/ { BARCODE_DOTCODE, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /*191*/ { BARCODE_DOTCODE, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /*192*/ { BARCODE_DOTCODE, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /*193*/ { BARCODE_DOTCODE, UNICODE_MODE, 25, "12", 0, 25, "ASCII" },
        /*194*/ { BARCODE_DOTCODE, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /*195*/ { BARCODE_DOTCODE, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /*196*/ { BARCODE_DOTCODE, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*197*/ { BARCODE_DOTCODE, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*198*/ { BARCODE_DOTCODE, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*199*/ { BARCODE_DOTCODE, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*200*/ { BARCODE_DOTCODE, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /*201*/ { BARCODE_DOTCODE, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
        /*202*/ { BARCODE_AZRUNE, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers <= 255 only" },
        /*203*/ { BARCODE_CODE32, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /*204*/ { BARCODE_CODEONE, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*205*/ { BARCODE_CODEONE, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /*206*/ { BARCODE_CODEONE, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /*207*/ { BARCODE_CODEONE, UNICODE_MODE, 9, "β", 0, 9, "" },
        /*208*/ { BARCODE_CODEONE, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /*209*/ { BARCODE_CODEONE, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /*210*/ { BARCODE_CODEONE, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /*211*/ { BARCODE_CODEONE, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /*212*/ { BARCODE_CODEONE, UNICODE_MODE, 25, "12", 0, 25, "ASCII" },
        /*213*/ { BARCODE_CODEONE, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /*214*/ { BARCODE_CODEONE, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /*215*/ { BARCODE_CODEONE, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*216*/ { BARCODE_CODEONE, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*217*/ { BARCODE_CODEONE, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*218*/ { BARCODE_CODEONE, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*219*/ { BARCODE_CODEONE, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /*220*/ { BARCODE_CODEONE, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
        /*221*/ { BARCODE_ULTRA, UNICODE_MODE, 0, "é", 0, 0, "" },
        /*222*/ { BARCODE_ULTRA, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /*223*/ { BARCODE_ULTRA, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /*224*/ { BARCODE_ULTRA, UNICODE_MODE, 9, "β", 0, 9, "" },
        /*225*/ { BARCODE_ULTRA, UNICODE_MODE, 20, "テ", 0, 20, "In Shift JIS" },
        /*226*/ { BARCODE_ULTRA, UNICODE_MODE, 20, "テテ", 0, 20, "In Shift JIS" },
        /*227*/ { BARCODE_ULTRA, UNICODE_MODE, 25, "က", 0, 25, "In UCS-2BE" },
        /*228*/ { BARCODE_ULTRA, UNICODE_MODE, 25, "ကက", 0, 25, "In UCS-2BE" },
        /*229*/ { BARCODE_ULTRA, UNICODE_MODE, 25, "12", 0, 25, "ASCII" },
        /*230*/ { BARCODE_ULTRA, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /*231*/ { BARCODE_ULTRA, UNICODE_MODE, 26, "テテ", 0, 26, "" },
        /*232*/ { BARCODE_ULTRA, UNICODE_MODE, 28, "龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*233*/ { BARCODE_ULTRA, UNICODE_MODE, 28, "龘龘", 0, 28, "U+9F98 in Big5 but not in GB2312" },
        /*234*/ { BARCODE_ULTRA, UNICODE_MODE, 29, "齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*235*/ { BARCODE_ULTRA, UNICODE_MODE, 29, "齄齄", 0, 29, "U+9F44 in GB2312 but not in Big5" },
        /*236*/ { BARCODE_ULTRA, UNICODE_MODE, 30, "가", 0, 30, "U+AC00 in EUC-KR" },
        /*237*/ { BARCODE_ULTRA, UNICODE_MODE, 30, "가가", 0, 30, "U+AC00 in EUC-KR" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_reduced_charset_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (data[i].expected_eci != -1) {
            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static int to_utf8(const unsigned int codepoint, unsigned char *buf) {
    int length = 0;

    if (codepoint < 0x80) {
        buf[0] = (unsigned char) codepoint;
        length = 1;
    } else if (codepoint < 0x800) {
        buf[0] = (unsigned char) (0xC0 | (codepoint >> 6));
        buf[1] = (unsigned char) (0x80 | (codepoint & 0x3F));
        length = 2;
    } else if (codepoint < 0x10000) {
        buf[0] = (unsigned char) (0xE0 | (codepoint >> 12));
        buf[1] = (unsigned char) (0x80 | ((codepoint >> 6) & 0x3F));
        buf[2] = (unsigned char) (0x80 | (codepoint & 0x3F));
        length = 3;
    } else {
        buf[0] = (unsigned char) (0xF0 | (codepoint >> 18));
        buf[1] = (unsigned char) (0x80 | ((codepoint >> 12) & 0x3F));
        buf[2] = (unsigned char) (0x80 | ((codepoint >> 6) & 0x3F));
        buf[3] = (unsigned char) (0x80 | (codepoint & 0x3F));
        length = 4;
    }
    buf[length] = '\0';

    return length;
}

// Original eci.h tables

static const unsigned short int iso_8859_1[] = {// Latin alphabet No. 1
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

static const unsigned short int iso_8859_2[] = {// Latin alphabet No. 2
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0104, 0x02d8, 0x0141, 0x00a4, 0x013d, 0x015a, 0x00a7, 0x00a8, 0x0160, 0x015e, 0x0164, 0x0179, 0x00ad, 0x017d, 0x017b,
    0x00b0, 0x0105, 0x02db, 0x0142, 0x00b4, 0x013e, 0x015b, 0x02c7, 0x00b8, 0x0161, 0x015f, 0x0165, 0x017a, 0x02dd, 0x017e, 0x017c,
    0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
    0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7, 0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
    0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
    0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7, 0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9
};

static const unsigned short int iso_8859_3[] = {// Latin alphabet No. 3
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0126, 0x02d8, 0x00a3, 0x00a4, 0x0000, 0x0124, 0x00a7, 0x00a8, 0x0130, 0x015e, 0x011e, 0x0134, 0x00ad, 0x0000, 0x017b,
    0x00b0, 0x0127, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x0125, 0x00b7, 0x00b8, 0x0131, 0x015f, 0x011f, 0x0135, 0x00bd, 0x0000, 0x017c,
    0x00c0, 0x00c1, 0x00c2, 0x0000, 0x00c4, 0x010a, 0x0108, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x0000, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x0120, 0x00d6, 0x00d7, 0x011c, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x016c, 0x015c, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x0000, 0x00e4, 0x010b, 0x0109, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x0000, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x0121, 0x00f6, 0x00f7, 0x011d, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x016d, 0x015d, 0x02d9
};

static const unsigned short int iso_8859_4[] = {// Latin alphabet No. 4
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0104, 0x0138, 0x0156, 0x00a4, 0x0128, 0x013b, 0x00a7, 0x00a8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00ad, 0x017d, 0x00af, // A5 0x012b -> 0x0128
    0x00b0, 0x0105, 0x02db, 0x0157, 0x00b4, 0x0129, 0x013c, 0x02c7, 0x00b8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014a, 0x017e, 0x014b,
    0x0100, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x012e, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x0116, 0x00cd, 0x00ce, 0x012a,
    0x0110, 0x0145, 0x014c, 0x0136, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x0172, 0x00da, 0x00db, 0x00dc, 0x0168, 0x016a, 0x00df,
    0x0101, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x012f, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x0117, 0x00ed, 0x00ee, 0x012b,
    0x0111, 0x0146, 0x014d, 0x0137, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x0173, 0x00fa, 0x00fb, 0x00fc, 0x0169, 0x016b, 0x02d9
};

static const unsigned short int iso_8859_5[] = {// Latin/Cyrillic alphabet
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 0x040a, 0x040b, 0x040c, 0x00ad, 0x040e, 0x040f,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
    0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 0x0459, 0x045a, 0x045b, 0x045c, 0x00a7, 0x045e, 0x045f
};

static const unsigned short int iso_8859_6[] = {// Latin/Arabic alphabet
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0000, 0x0000, 0x0000, 0x00a4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x060c, 0x00ad, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x061b, 0x0000, 0x0000, 0x0000, 0x061f,
    0x0000, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062a, 0x062b, 0x062c, 0x062d, 0x062e, 0x062f,
    0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x063a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649, 0x064a, 0x064b, 0x064c, 0x064d, 0x064e, 0x064f,
    0x0650, 0x0651, 0x0652, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static const unsigned short int iso_8859_7[] = {// Latin/Greek alphabet
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x2018, 0x2019, 0x00a3, 0x20ac, 0x20af, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x037a, 0x00ab, 0x00ac, 0x00ad, 0x0000, 0x2015,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x0384, 0x0385, 0x0386, 0x00b7, 0x0388, 0x0389, 0x038a, 0x00bb, 0x038c, 0x00bd, 0x038e, 0x038f,
    0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
    0x03a0, 0x03a1, 0x0000, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7, 0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
    0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
    0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce, 0x0000
};

static const unsigned short int iso_8859_8[] = {// Latin/Hebrew alphabet
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0000, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00d7, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00f7, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2017,
    0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05d4, 0x05d5, 0x05d6, 0x05d7, 0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05dd, 0x05de, 0x05df,
    0x05e0, 0x05e1, 0x05e2, 0x05e3, 0x05e4, 0x05e5, 0x05e6, 0x05e7, 0x05e8, 0x05e9, 0x05ea, 0x0000, 0x0000, 0x200e, 0x200f, 0x0000
};

static const unsigned short int iso_8859_9[] = {// Latin alphabet No. 5
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x011e, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0130, 0x015e, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x011f, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0131, 0x015f, 0x00ff
};

static const unsigned short int iso_8859_10[] = {// Latin alphabet No. 6
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0104, 0x0112, 0x0122, 0x012a, 0x0128, 0x0136, 0x00a7, 0x013b, 0x0110, 0x0160, 0x0166, 0x017d, 0x00ad, 0x016a, 0x014a, // A5 0x012b -> 0x0128
    0x00b0, 0x0105, 0x0113, 0x0123, 0x012b, 0x0129, 0x0137, 0x00b7, 0x013c, 0x0111, 0x0161, 0x0167, 0x017e, 0x2015, 0x016b, 0x014b,
    0x0100, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x012e, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x0116, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x0145, 0x014c, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x0168, 0x00d8, 0x0172, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x0101, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x012f, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x0117, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x0146, 0x014d, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x0169, 0x00f8, 0x0173, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x0138
};

static const unsigned short int iso_8859_11[] = {// Latin/Thai alphabet
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0e01, 0x0e02, 0x0e03, 0x0e04, 0x0e05, 0x0e06, 0x0e07, 0x0e08, 0x0e09, 0x0e0a, 0x0e0b, 0x0e0c, 0x0e0d, 0x0e0e, 0x0e0f,
    0x0e10, 0x0e11, 0x0e12, 0x0e13, 0x0e14, 0x0e15, 0x0e16, 0x0e17, 0x0e18, 0x0e19, 0x0e1a, 0x0e1b, 0x0e1c, 0x0e1d, 0x0e1e, 0x0e1f,
    0x0e20, 0x0e21, 0x0e22, 0x0e23, 0x0e24, 0x0e25, 0x0e26, 0x0e27, 0x0e28, 0x0e29, 0x0e2a, 0x0e2b, 0x0e2c, 0x0e2d, 0x0e2e, 0x0e2f,
    0x0e30, 0x0e31, 0x0e32, 0x0e33, 0x0e34, 0x0e35, 0x0e36, 0x0e37, 0x0e38, 0x0e39, 0x0e3a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0e3f, // D5 0x0e36 -> 0x0e35
    0x0e40, 0x0e41, 0x0e42, 0x0e43, 0x0e44, 0x0e45, 0x0e46, 0x0e47, 0x0e48, 0x0e49, 0x0e4a, 0x0e4b, 0x0e4c, 0x0e4d, 0x0e4e, 0x0e4f,
    0x0e50, 0x0e51, 0x0e52, 0x0e53, 0x0e54, 0x0e55, 0x0e56, 0x0e57, 0x0e58, 0x0e59, 0x0e5a, 0x0e5b, 0x0000, 0x0000, 0x0000, 0x0000
};

static const unsigned short int iso_8859_13[] = {// Latin alphabet No. 7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x201d, 0x00a2, 0x00a3, 0x00a4, 0x201e, 0x00a6, 0x00a7, 0x00d8, 0x00a9, 0x0156, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00c6,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x201c, 0x00b5, 0x00b6, 0x00b7, 0x00f8, 0x00b9, 0x0157, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00e6,
    0x0104, 0x012e, 0x0100, 0x0106, 0x00c4, 0x00c5, 0x0118, 0x0112, 0x010c, 0x00c9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012a, 0x013b,
    0x0160, 0x0143, 0x0145, 0x00d3, 0x014c, 0x00d5, 0x00d6, 0x00d7, 0x0172, 0x0141, 0x015a, 0x016a, 0x00dc, 0x017b, 0x017d, 0x00df,
    0x0105, 0x012f, 0x0101, 0x0107, 0x00e4, 0x00e5, 0x0119, 0x0113, 0x010d, 0x00e9, 0x017a, 0x0117, 0x0123, 0x0137, 0x012b, 0x013c,
    0x0161, 0x0144, 0x0146, 0x00f3, 0x014d, 0x00f5, 0x00f6, 0x00f7, 0x0173, 0x0142, 0x015b, 0x016b, 0x00fc, 0x017c, 0x017e, 0x2019
};

static const unsigned short int iso_8859_14[] = {// Latin alphabet No. 8 (Celtic)
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x1e02, 0x1e03, 0x00a3, 0x010a, 0x010b, 0x1e0a, 0x00a7, 0x1e80, 0x00a9, 0x1e82, 0x1e0b, 0x1ef2, 0x00ad, 0x00ae, 0x0178,
    0x1e1e, 0x1e1f, 0x0120, 0x0121, 0x1e40, 0x1e41, 0x00b6, 0x1e56, 0x1e81, 0x1e57, 0x1e83, 0x1e60, 0x1ef3, 0x1e84, 0x1e85, 0x1e61,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x0174, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x1e6a, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x0176, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x0175, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x1e6b, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x0177, 0x00ff
};

static const unsigned short int iso_8859_15[] = {// Latin alphabet No. 9
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x20ac, 0x00a5, 0x0160, 0x00a7, 0x0161, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x017d, 0x00b5, 0x00b6, 0x00b7, 0x017e, 0x00b9, 0x00ba, 0x00bb, 0x0152, 0x0153, 0x0178, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

static const unsigned short int iso_8859_16[] = {// Latin alphabet No. 10
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00a0, 0x0104, 0x0105, 0x0141, 0x20ac, 0x201e, 0x0160, 0x00a7, 0x0161, 0x00a9, 0x0218, 0x00ab, 0x0179, 0x00ad, 0x017a, 0x017b,
    0x00b0, 0x00b1, 0x010c, 0x0142, 0x017d, 0x201d, 0x00b6, 0x00b7, 0x017e, 0x010d, 0x0219, 0x00bb, 0x0152, 0x0153, 0x0178, 0x017c,
    0x00c0, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0106, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x0110, 0x0143, 0x00d2, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x015a, 0x0170, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0118, 0x021a, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x0107, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x0111, 0x0144, 0x00f2, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x015b, 0x0171, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0119, 0x021b, 0x00ff
};

static const unsigned short int windows_1250[] = {
    0x20ac, 0x0000, 0x201a, 0x0000, 0x201e, 0x2026, 0x2020, 0x2021, 0x0000, 0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179,
    0x0000, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x0000, 0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a,
    0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b, // A2 0x02db -> 0x02d8
    0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c,
    0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7, 0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
    0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7, 0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
    0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7, 0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
    0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7, 0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9
};

static const unsigned short int windows_1251[] = {
    0x0402, 0x0403, 0x201a, 0x0453, 0x201e, 0x2026, 0x2020, 0x2021, 0x20ac, 0x2030, 0x0409, 0x2039, 0x040a, 0x040c, 0x040b, 0x040f,
    0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x0000, 0x2122, 0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f,
    0x00a0, 0x040e, 0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6, 0x00a7, 0x0401, 0x00a9, 0x0404, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407,
    0x00b0, 0x00b1, 0x0406, 0x0456, 0x0491, 0x00b5, 0x00b6, 0x00b7, 0x0451, 0x2116, 0x0454, 0x00bb, 0x0458, 0x0405, 0x0455, 0x0457,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f
};

static const unsigned short int windows_1252[] = {
    0x20ac, 0x0000, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017d, 0x0000,
    0x0000, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x0000, 0x017e, 0x0178,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

static const unsigned short int windows_1256[] = {
    0x20ac, 0x067e, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0679, 0x2039, 0x0152, 0x0686, 0x0698, 0x0688,
    0x06af, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x06a9, 0x2122, 0x0691, 0x203a, 0x0153, 0x200c, 0x200d, 0x06ba,
    0x00a0, 0x060c, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x06be, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x061b, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x061f,
    0x06c1, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062a, 0x062b, 0x062c, 0x062d, 0x062e, 0x062f,
    0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00d7, 0x0637, 0x0638, 0x0639, 0x063a, 0x0640, 0x0641, 0x0642, 0x0643,
    0x00e0, 0x0644, 0x00e2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x0649, 0x064a, 0x00ee, 0x00ef,
    0x064b, 0x064c, 0x064d, 0x064e, 0x00f4, 0x064f, 0x0650, 0x00f7, 0x0651, 0x00f9, 0x0652, 0x00fb, 0x00fc, 0x200e, 0x200f, 0x06d2
};

static void test_utf8_to_eci_sb(int index) {

    struct item {
        int eci;
        const unsigned short *tab;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 3, iso_8859_1 },
        /*  1*/ { 4, iso_8859_2 },
        /*  2*/ { 5, iso_8859_3 },
        /*  3*/ { 6, iso_8859_4 },
        /*  4*/ { 7, iso_8859_5 },
        /*  5*/ { 8, iso_8859_6 },
        /*  6*/ { 9, iso_8859_7 },
        /*  7*/ { 10, iso_8859_8 },
        /*  8*/ { 11, iso_8859_9 },
        /*  9*/ { 12, iso_8859_10 },
        /* 10*/ { 13, iso_8859_11 },
        /* 11*/ { 15, iso_8859_13 },
        /* 12*/ { 16, iso_8859_14 },
        /* 13*/ { 17, iso_8859_15 },
        /* 14*/ { 18, iso_8859_16 },
        /* 15*/ { 21, windows_1250 },
        /* 16*/ { 22, windows_1251 },
        /* 17*/ { 23, windows_1252 },
        /* 18*/ { 24, windows_1256 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    unsigned char source[5];
    unsigned char dest[2] = {0};

    testStart("test_utf8_to_eci_sb");

    for (i = 0; i < data_size; i++) {
        int j;

        if (index != -1 && i != index) continue;

        for (j = 0; j < 128; j++) {
            int k = j + 128;
            if (data[i].tab[j]) {
                length = to_utf8(data[i].tab[j], source);
                assert_nonzero(length, "i:%d to_utf8 length %d == 0\n", i, length);
                ret = utf8_to_eci(data[i].eci, source, dest, &length);
                assert_zero(ret, "i:%d utf8_to_eci ret %d != 0\n", i, ret);
                assert_equal(*dest, k, "i:%d j:%d eci:%d codepoint:0x%x *dest 0x%X (%d) != 0x%X (%d)\n", i, j, data[i].eci, data[i].tab[j], *dest, *dest, k, k);
            } else {
                length = to_utf8(k, source);
                assert_nonzero(length, "i:%d to_utf8 length %d == 0\n", i, length);
                ret = utf8_to_eci(data[i].eci, source, dest, &length);
                if (ret == 0) { // Should be mapping for this codepoint in another entry
                    int found = 0;
                    int m;
                    for (m = 0; m < 128; m++) {
                        if (data[i].tab[m] == k) {
                            found = 1;
                            break;
                        }
                    }
                    assert_nonzero(found, "i:%d j:%d eci:%d codepoint:0x%x source:%s not found utf8_to_eci ret %d == 0\n", i, j, data[i].eci, k, source, ret);
                } else {
                    assert_equal(ret, ZINT_ERROR_INVALID_DATA, "i:%d j:%d eci:%d codepoint:0x%x source:%s utf8_to_eci ret %d != ZINT_ERROR_INVALID_DATA\n", i, j, data[i].eci, k, source, ret);
                }
            }
        }
    }

    testFinish();
}

static void test_utf8_to_eci_ascii(void) {

    struct item {
        int eci;
        char *data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 27, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 32, 0 },
        /*  1*/ { 27, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177", 96, 0 },
        /*  2*/ { 27, "\302\200", -1, ZINT_ERROR_INVALID_DATA },
        /*  3*/ { 170, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 32, 0 },
        /*  4*/ { 170, " !\"  %&'()*+,-./0123456789:;<=>? ABCDEFGHIJKLMNOPQRSTUVWXYZ    _ abcdefghijklmnopqrstuvwxyz    \177", 96, 0 },
        /*  5*/ { 170, "#", -1, ZINT_ERROR_INVALID_DATA },
        /*  6*/ { 170, "$", -1, ZINT_ERROR_INVALID_DATA },
        /*  7*/ { 170, "@", -1, ZINT_ERROR_INVALID_DATA },
        /*  8*/ { 170, "[", -1, ZINT_ERROR_INVALID_DATA },
        /*  9*/ { 170, "\\", -1, ZINT_ERROR_INVALID_DATA },
        /* 10*/ { 170, "]", -1, ZINT_ERROR_INVALID_DATA },
        /* 11*/ { 170, "^", -1, ZINT_ERROR_INVALID_DATA },
        /* 12*/ { 170, "`", -1, ZINT_ERROR_INVALID_DATA },
        /* 13*/ { 170, "{", -1, ZINT_ERROR_INVALID_DATA },
        /* 14*/ { 170, "|", -1, ZINT_ERROR_INVALID_DATA },
        /* 15*/ { 170, "}", -1, ZINT_ERROR_INVALID_DATA },
        /* 16*/ { 170, "~", -1, ZINT_ERROR_INVALID_DATA },
        /* 17*/ { 170, "\302\200", -1, ZINT_ERROR_INVALID_DATA },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    char dest[128];

    testStart("test_utf8_to_eci_ascii");

    for (i = 0; i < data_size; i++) {
        int out_length;
        length = data[i].length != -1 ? data[i].length : (int) strlen(data[i].data);
        out_length = length;
        ret = utf8_to_eci(data[i].eci, (const unsigned char *) data[i].data, (unsigned char *) dest, &out_length);
        assert_equal(ret, data[i].ret, "i:%d utf8_to_eci ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(length, out_length, "i:%d length %d != %d\n", i, length, out_length);
            assert_zero(memcmp(data[i].data, dest, length), "i:%d memcmp != 0\n", i);
        }
    }
};

static void test_utf8_to_eci_ucs2be(void) {

    struct item {
        int eci;
        char *data;
        int length;
        int ret;
        int expected_length;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 25, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 32, 0, 32 * 2 },
        /*  1*/ { 25, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177", 96, 0, 96 * 2 },
        /*  2*/ { 25, "\302\200\357\277\277", -1, 0, 4 }, // U+0080 U+FFFF
        /*  3*/ { 25, "\357\277\276", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+FFFE (reversed BOM) not allowed
        /*  4*/ { 25, "\355\240\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+D800 surrogate not allowed
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_utf8_to_eci_ucs2be");

    for (i = 0; i < data_size; i++) {
        int out_length, eci_length;
        char dest[1024];

        length = data[i].length != -1 ? data[i].length : (int) strlen(data[i].data);
        out_length = length;
        eci_length = get_eci_length(data[i].eci, (const unsigned char *) data[i].data, length);

        assert_nonzero(eci_length + 1 <= 1024, "i:%d eci_length %d + 1 > 1024\n", i, eci_length);
        ret = utf8_to_eci(data[i].eci, (const unsigned char *) data[i].data, (unsigned char *) dest, &out_length);
        assert_equal(ret, data[i].ret, "i:%d utf8_to_eci ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(out_length, data[i].expected_length, "i:%d length %d != %d\n", i, out_length, data[i].expected_length);
            assert_nonzero(out_length <= eci_length, "i:%d out_length %d > eci_length %d\n", i, out_length, eci_length);
        }
    }
};

static void test_utf8_to_eci_sjis(void) {

    struct item {
        int eci;
        char *data;
        int length;
        int ret;
        int expected_length;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 20, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 32, 0, 32 },
        /*  1*/ { 20, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}\177", 95, 0, 95 + 1 }, // Backslash goes to 2 byte
        /*  2*/ { 20, "~", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for tilde
        /*  3*/ { 20, "\302\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+0080
        /*  4*/ { 20, "\302\241", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+00A1 Inverted exclaimation mark
        /*  5*/ { 20, "\302\245", -1, 0, 1 }, // U+00A5 Yen goes to backslash
        /*  6*/ { 20, "\302\277", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+00BF Inverted question mark
        /*  7*/ { 20, "\303\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+00C0 À
        /*  8*/ { 20, "\303\251", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+00E9 é
        /*  9*/ { 20, "\312\262", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+03B2 β
        /* 10*/ { 20, "\342\272\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+2E80 CJK RADICAL REPEAT
        /* 11*/ { 20, "\343\200\200", -1, 0, 2 }, // U+3000 IDEOGRAPHIC SPACE
        /* 12*/ { 20, "\343\200\204", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+3004 JAPANESE INDUSTRIAL STANDARD SYMBOL
        /* 13*/ { 20, "\343\201\201", -1, 0, 2 }, //U+3041 HIRAGANA LETTER SMALL A
        /* 14*/ { 20, "\357\277\277", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+FFFF
        /* 15*/ { 20, "\357\277\276", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+FFFE (reversed BOM) not allowed
        /* 16*/ { 20, "\355\240\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+D800 surrogate not allowed
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_utf8_to_eci_sjis");

    for (i = 0; i < data_size; i++) {
        int out_length, eci_length;
        char dest[1024];

        length = data[i].length != -1 ? data[i].length : (int) strlen(data[i].data);
        out_length = length;
        eci_length = get_eci_length(data[i].eci, (const unsigned char *) data[i].data, length);

        assert_nonzero(eci_length + 1 <= 1024, "i:%d eci_length %d + 1 > 1024\n", i, eci_length);
        ret = utf8_to_eci(data[i].eci, (const unsigned char *) data[i].data, (unsigned char *) dest, &out_length);
        assert_equal(ret, data[i].ret, "i:%d utf8_to_eci ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(out_length, data[i].expected_length, "i:%d length %d != %d\n", i, out_length, data[i].expected_length);
            assert_nonzero(out_length <= eci_length, "i:%d out_length %d > eci_length %d\n", i, out_length, eci_length);
        }
    }
};

static void test_utf8_to_eci_big5(void) {

    struct item {
        int eci;
        char *data;
        int length;
        int ret;
        int expected_length;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 28, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 32, 0, 32 },
        /*  1*/ { 28, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177", 96, 0, 96 },
        /*  2*/ { 28, "\302\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+0080
        /*  3*/ { 28, "\343\200\200", -1, 0, 2 }, // U+3000 IDEOGRAPHIC SPACE
        /*  4*/ { 28, "\357\277\277", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+FFFF
        /*  5*/ { 28, "\357\277\276", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+FFFE (reversed BOM) not allowed
        /*  6*/ { 28, "\355\240\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+D800 surrogate not allowed
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_utf8_to_eci_big5");

    for (i = 0; i < data_size; i++) {
        int out_length, eci_length;
        char dest[1024];

        length = data[i].length != -1 ? data[i].length : (int) strlen(data[i].data);
        out_length = length;
        eci_length = get_eci_length(data[i].eci, (const unsigned char *) data[i].data, length);

        assert_nonzero(eci_length + 1 <= 1024, "i:%d eci_length %d + 1 > 1024\n", i, eci_length);
        ret = utf8_to_eci(data[i].eci, (const unsigned char *) data[i].data, (unsigned char *) dest, &out_length);
        assert_equal(ret, data[i].ret, "i:%d utf8_to_eci ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(out_length, data[i].expected_length, "i:%d length %d != %d\n", i, out_length, data[i].expected_length);
            assert_nonzero(out_length <= eci_length, "i:%d out_length %d > eci_length %d\n", i, out_length, eci_length);
        }
    }
};

static void test_utf8_to_eci_gb2312(void) {

    struct item {
        int eci;
        char *data;
        int length;
        int ret;
        int expected_length;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 29, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 32, 0, 32 },
        /*  1*/ { 29, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177", 96, 0, 96 },
        /*  2*/ { 29, "\302\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+0080
        /*  3*/ { 29, "\343\200\200", -1, 0, 2 }, // U+3000 IDEOGRAPHIC SPACE
        /*  4*/ { 29, "\357\277\277", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+FFFF
        /*  5*/ { 29, "\357\277\276", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+FFFE (reversed BOM) not allowed
        /*  6*/ { 29, "\355\240\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+D800 surrogate not allowed
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_utf8_to_eci_gb2312");

    for (i = 0; i < data_size; i++) {
        int out_length, eci_length;
        char dest[1024];

        length = data[i].length != -1 ? data[i].length : (int) strlen(data[i].data);
        out_length = length;
        eci_length = get_eci_length(data[i].eci, (const unsigned char *) data[i].data, length);

        assert_nonzero(eci_length + 1 <= 1024, "i:%d eci_length %d + 1 > 1024\n", i, eci_length);
        ret = utf8_to_eci(data[i].eci, (const unsigned char *) data[i].data, (unsigned char *) dest, &out_length);
        assert_equal(ret, data[i].ret, "i:%d utf8_to_eci ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(out_length, data[i].expected_length, "i:%d length %d != %d\n", i, out_length, data[i].expected_length);
            assert_nonzero(out_length <= eci_length, "i:%d out_length %d > eci_length %d\n", i, out_length, eci_length);
        }
    }
};

static void test_utf8_to_eci_euc_kr(void) {

    struct item {
        int eci;
        char *data;
        int length;
        int ret;
        int expected_length;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 30, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 32, 0, 32 },
        /*  1*/ { 30, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177", 96, 0, 96 },
        /*  2*/ { 30, "\302\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+0080
        /*  3*/ { 30, "\343\200\200", -1, 0, 2 }, // U+3000 IDEOGRAPHIC SPACE
        /*  4*/ { 30, "\357\277\277", -1, ZINT_ERROR_INVALID_DATA, -1 }, // No mapping for U+FFFF
        /*  5*/ { 30, "\357\277\276", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+FFFE (reversed BOM) not allowed
        /*  6*/ { 30, "\355\240\200", -1, ZINT_ERROR_INVALID_DATA, -1 }, // U+D800 surrogate not allowed
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_utf8_to_eci_euc_kr");

    for (i = 0; i < data_size; i++) {
        int out_length, eci_length;
        char dest[1024];

        length = data[i].length != -1 ? data[i].length : (int) strlen(data[i].data);
        out_length = length;
        eci_length = get_eci_length(data[i].eci, (const unsigned char *) data[i].data, length);

        assert_nonzero(eci_length + 1 <= 1024, "i:%d eci_length %d + 1 > 1024\n", i, eci_length);
        ret = utf8_to_eci(data[i].eci, (const unsigned char *) data[i].data, (unsigned char *) dest, &out_length);
        assert_equal(ret, data[i].ret, "i:%d utf8_to_eci ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(out_length, data[i].expected_length, "i:%d length %d != %d\n", i, out_length, data[i].expected_length);
            assert_nonzero(out_length <= eci_length, "i:%d out_length %d > eci_length %d\n", i, out_length, eci_length);
        }
    }
};

static void test_get_best_eci(int index) {

    struct item {
        const char *data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "\300\301", -1, 0 },
        /*  1*/ { "ÀÁ", -1, 3 },
        /*  2*/ { "Ђ", -1, 7 },
        /*  3*/ { "Ѐ", -1, 26 }, // Cyrillic U+0400 not in single-byte code pages
        /*  4*/ { "β", -1, 9 },
        /*  5*/ { "˜", -1, 23 },
        /*  6*/ { "βЂ", -1, 26 },
        /*  7*/ { "AB\200", -1, 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    testStart("test_get_best_eci");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = get_best_eci((const unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d get_best_eci ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_bom", test_bom, 0, 0, 1 },
        { "test_iso_8859_16", test_iso_8859_16, 0, 0, 1 },
        { "test_reduced_charset_input", test_reduced_charset_input, 1, 0, 1 },
        { "test_utf8_to_eci_sb", test_utf8_to_eci_sb, 1, 0, 0 },
        { "test_utf8_to_eci_ascii", test_utf8_to_eci_ascii, 0, 0, 0 },
        { "test_utf8_to_eci_ucs2be", test_utf8_to_eci_ucs2be, 0, 0, 0 },
        { "test_utf8_to_eci_sjis", test_utf8_to_eci_sjis, 0, 0, 0 },
        { "test_utf8_to_eci_big5", test_utf8_to_eci_big5, 0, 0, 0 },
        { "test_utf8_to_eci_gb2312", test_utf8_to_eci_gb2312, 0, 0, 0 },
        { "test_utf8_to_eci_euc_kr", test_utf8_to_eci_euc_kr, 0, 0, 0 },
        { "test_get_best_eci", test_get_best_eci, 1, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
