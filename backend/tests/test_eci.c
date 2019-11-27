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

static void test_bom(void)
{
    testStart("");

    struct zint_symbol* symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    symbol->symbology = BARCODE_QRCODE;
    symbol->input_mode = UNICODE_MODE;
    symbol->option_1 = 4;
    symbol->option_2 = 1;

    char data[] = "\xEF\xBB\xBF‹"; // U+FEFF BOM, with U+2039 (only in Windows pages)
    int length = strlen(data);

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

    int ret;
    ret = ZBarcode_Encode(symbol, data, length);
    assert_equal(ret, ZINT_WARN_USES_ECI, "ZBarcode_Encode ret %d != ZINT_WARN_USES_ECI\n", ret);
    assert_equal(symbol->eci, 21, "eci %d != 21\n", symbol->eci); // ECI 21 == Windows-1250

    int width, height;
    ret = testUtilModulesCmp(symbol, expected, &width, &height);
    assert_equal(ret, 0, "testUtilModulesEqual ret %d != 0, width %d, height %d\n", ret, width, height);

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_iso_8859_16(void)
{
    testStart("");

    struct zint_symbol* symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    symbol->symbology = BARCODE_QRCODE;
    symbol->input_mode = UNICODE_MODE;

    char data[] = "Ț"; // U+021A only in ISO 8859-16
    int length = strlen(data);

    int ret;
    ret = ZBarcode_Encode(symbol, data, length);
    assert_equal(ret, ZINT_WARN_USES_ECI, "ZBarcode_Encode ret %d != ZINT_WARN_USES_ECI\n", ret);
    assert_equal(symbol->eci, 18, "eci %d != 18\n", symbol->eci); // ECI 18 == ISO 8859-16

    ZBarcode_Delete(symbol);

    testFinish();
}

// Only testing standard non-extended barcodes here, ie not QRCODE, MICROQR, GRIDMATRIX, HANXIN or UPNQR
static void test_reduced_charset_input(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        int eci;
        unsigned char* data;
        int ret;
        int expected_eci;
        char* comment;
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
        /*  1*/ { BARCODE_C25MATRIX, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /*  2*/ { BARCODE_CODE39, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /*  3*/ { BARCODE_EXCODE39, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII only" },
        /*  4*/ { BARCODE_EANX, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /*  5*/ { BARCODE_CODABAR, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
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
        /* 16*/ { BARCODE_RSS14, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /* 17*/ { BARCODE_RSS_EXP, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /* 18*/ { BARCODE_LOGMARS, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "ASCII subset only" },
        /* 19*/ { BARCODE_PDF417, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 20*/ { BARCODE_PDF417, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 21*/ { BARCODE_PDF417, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /* 22*/ { BARCODE_PDF417, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 23*/ { BARCODE_PDF417, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 13, "" },
        /* 24*/ { BARCODE_PDF417, UNICODE_MODE, 13, "ก", 0, 13, "" },
        /* 25*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Ж", ZINT_WARN_USES_ECI, 7, "" },
        /* 26*/ { BARCODE_PDF417, UNICODE_MODE, 7, "Ж", 0, 7, "" },
        /* 27*/ { BARCODE_PDF417, UNICODE_MODE, 0, "…", ZINT_WARN_USES_ECI, 21, "" },
        /* 28*/ { BARCODE_PDF417, UNICODE_MODE, 21, "…", 0, 21, "" },
        /* 29*/ { BARCODE_PDF417, UNICODE_MODE, 0, "テ", ZINT_WARN_USES_ECI, 26, "Defaults to UTF-8 if not in any ISO 8859 or Win page" },
        /* 30*/ { BARCODE_PDF417, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 31*/ { BARCODE_PDF417, UNICODE_MODE, 20, "テ", ZINT_ERROR_INVALID_DATA, -1, "テ in ECI 20 but that conversion not currently supported in Zint for non-extended barcodes" },
        /* 32*/ { BARCODE_PDF417, UNICODE_MODE, 900, "é", 0, 900, "ECI > 899 ignored for character set conversion" },
        /* 33*/ { BARCODE_PDF417, UNICODE_MODE, 900, "β", ZINT_ERROR_INVALID_DATA, 900, "But ECI > 899 suppresses auto-ECI `get_best_eci()`" },
        /* 34*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 35*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 36*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /* 37*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 38*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 39*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 40*/ { BARCODE_MAXICODE, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 41*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /* 42*/ { BARCODE_MAXICODE, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 43*/ { BARCODE_MAXICODE, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 44*/ { BARCODE_CODE128B, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 45*/ { BARCODE_CODE128B, UNICODE_MODE, 3, "é", ZINT_ERROR_INVALID_OPTION, -1, "Does not support ECI" },
        /* 46*/ { BARCODE_CODE128B, UNICODE_MODE, 0, "β", ZINT_ERROR_INVALID_DATA, -1, "β not in ISO 8859-1" },
        /* 47*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 48*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 49*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /* 50*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 51*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 52*/ { BARCODE_CODABLOCKF, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 53*/ { BARCODE_CODABLOCKF, UNICODE_MODE, 3, "é", ZINT_ERROR_INVALID_OPTION, -1, "Does not support ECI" },
        /* 54*/ { BARCODE_CODABLOCKF, UNICODE_MODE, 0, "β", ZINT_ERROR_INVALID_DATA, -1, "β not in ISO 8859-1" },
        /* 55*/ { BARCODE_NVE18, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /* 56*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 57*/ { BARCODE_MICROPDF417, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 58*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /* 59*/ { BARCODE_MICROPDF417, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 60*/ { BARCODE_MICROPDF417, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 61*/ { BARCODE_ONECODE, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers/dash only" },
        /* 62*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 63*/ { BARCODE_AZTEC, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 64*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /* 65*/ { BARCODE_AZTEC, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 66*/ { BARCODE_AZTEC, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 67*/ { BARCODE_HIBC_128, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "HIBC ASCII subset only" },
        /* 68*/ { BARCODE_HIBC_AZTEC, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "HIBC ASCII subset only" },
        /* 69*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 70*/ { BARCODE_DOTCODE, UNICODE_MODE, 3, "é", 0, 3, "Supports ECI" },
        /* 71*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 9, "" },
        /* 72*/ { BARCODE_DOTCODE, UNICODE_MODE, 9, "β", 0, 9, "" },
        /* 73*/ { BARCODE_DOTCODE, UNICODE_MODE, 26, "テ", 0, 26, "" },
        /* 74*/ { BARCODE_AZRUNE, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers <= 255 only" },
        /* 75*/ { BARCODE_CODE32, UNICODE_MODE, 0, "é", ZINT_ERROR_INVALID_DATA, -1, "Numbers only" },
        /* 76*/ { BARCODE_CODEONE, UNICODE_MODE, 0, "é", 0, 0, "" },
        /* 77*/ { BARCODE_CODEONE, UNICODE_MODE, 3, "é", ZINT_ERROR_INVALID_OPTION, -1, "Does not support ECI" },
        /* 78*/ { BARCODE_CODEONE, UNICODE_MODE, 0, "β", ZINT_ERROR_INVALID_DATA, -1, "β not in ISO 8859-1" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (data[i].expected_eci != -1) {
            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_bom();
    test_iso_8859_16();
    test_reduced_charset_input();

    testReport();

    return 0;
}
