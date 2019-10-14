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

static void test_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        int eci;
        unsigned char* data;
        int ret_encode;
        float w;
        float h;
        int ret_vector;
        int expected_eci;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), in Shift-JIS, in GB-2312/18030
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859), in Shift-JIS, in GB-2312/18030
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859), not in Shift-JIS, not in GB-2312/18030
    // ກ U+0E81 Lao not in any ISO 8859 (or Windows page) or Shift-JIS or GB-2312/18030
    // Vi} :s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_QRCODE, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /*  1*/ { BARCODE_QRCODE, UNICODE_MODE, 0, "Aβ", 0, 100, 30, 0, 0 }, // Converts to Shift-JIS
        /*  2*/ { BARCODE_QRCODE, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /*  3*/ { BARCODE_QRCODE, UNICODE_MODE, 0, "Aéβ", 0, 100, 30, 0, 0 }, // Converts to Shift-JIS
        /*  4*/ { BARCODE_QRCODE, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /*  5*/ { BARCODE_QRCODE, UNICODE_MODE, 0, "Aກ", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /*  6*/ { BARCODE_QRCODE, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /*  7*/ { BARCODE_MICROQR, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /*  8*/ { BARCODE_MICROQR, UNICODE_MODE, 0, "Aβ", 0, 100, 30, 0, 0 }, // Converts to Shift-JIS
        /*  9*/ { BARCODE_MICROQR, UNICODE_MODE, 0, "Aéβ", 0, 100, 30, 0, 0 }, // Converts to Shift-JIS
        /* 10*/ { BARCODE_MICROQR, UNICODE_MODE, 0, "ก", ZINT_ERROR_INVALID_DATA, 100, 30, -1, -1 },
        /* 11*/ { BARCODE_MICROQR, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 12*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 13*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, 0, "Aβ", 0, 100, 30, 0, 0 }, // Converts to GB-2312
        /* 14*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 15*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, 0, "Aéβ", 0, 100, 30, 0, 0 }, // Converts to GB-2312
        /* 16*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 17*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 18*/ { BARCODE_HANXIN, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 19*/ { BARCODE_HANXIN, UNICODE_MODE, 0, "Aβ", 0, 100, 30, 0, 0 }, // Converts to GB-18030
        /* 20*/ { BARCODE_HANXIN, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 21*/ { BARCODE_HANXIN, UNICODE_MODE, 0, "Aéβ", 0, 100, 30, 0, 0 }, // Converts to GB-18030
        /* 22*/ { BARCODE_HANXIN, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 23*/ { BARCODE_HANXIN, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 24*/ { BARCODE_UPNQR, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 4 }, // ECI 4 == iSO 8859-2
        /* 25*/ { BARCODE_UPNQR, UNICODE_MODE, 0, "Aβ", ZINT_ERROR_INVALID_DATA, 100, 30, -1, -1 },
        /* 26*/ { BARCODE_UPNQR, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 4 },
        /* 27*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 28*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "Aβ", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 29*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 30*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 31*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 32*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "Aéβ", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 33*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 34*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 35*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 36*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "Aβ", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 37*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 38*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 39*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 40*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "Aéβ", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 41*/ { BARCODE_AZTEC, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 42*/ { BARCODE_AZTEC, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 43*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 44*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "Aβ", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 45*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 46*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 47*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 48*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "Aéβ", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 49*/ { BARCODE_MAXICODE, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 50*/ { BARCODE_MAXICODE, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 51*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 52*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "Aβ", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 53*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 54*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 55*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 56*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "Aéβ", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 57*/ { BARCODE_MICROPDF417, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 58*/ { BARCODE_MICROPDF417, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 59*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 60*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Aβ", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 61*/ { BARCODE_PDF417, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 62*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 63*/ { BARCODE_PDF417, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 64*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Aéβ", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 65*/ { BARCODE_PDF417, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 66*/ { BARCODE_PDF417, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 67*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 68*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "Aβ", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 69*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 70*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 71*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 72*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "Aéβ", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 73*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 74*/ { BARCODE_PDF417TRUNC, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 75*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 76*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "Aβ", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 77*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "β", ZINT_WARN_USES_ECI, 100, 30, 0, 9 }, // ECI 9 == ISO 8859-7
        /* 78*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "Aก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 79*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 100, 30, 0, 13 }, // ECI 13 == ISO 8859-11
        /* 80*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "Aéβ", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 81*/ { BARCODE_DOTCODE, UNICODE_MODE, 0, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 0, 26 },
        /* 82*/ { BARCODE_DOTCODE, UNICODE_MODE, 3, "Aé", 0, 100, 30, 0, 3 },
        /* 83*/ { BARCODE_CODE11, UNICODE_MODE, 0, "Aé", ZINT_ERROR_INVALID_DATA, 100, 30, -1, -1 },
        /* 84*/ { BARCODE_CODE39, UNICODE_MODE, 0, "Aé", ZINT_ERROR_INVALID_DATA, 100, 30, -1, -1 },
        /* 85*/ { BARCODE_EXCODE39, UNICODE_MODE, 0, "Aé", ZINT_ERROR_INVALID_DATA, 100, 30, -1, -1 },
        /* 86*/ { BARCODE_CODE128, UNICODE_MODE, 0, "Aé", 0, 100, 30, 0, 0 },
        /* 87*/ { BARCODE_CODE128, UNICODE_MODE, 0, "Aβ", ZINT_ERROR_INVALID_DATA, 100, 30, -1, -1 },
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
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        if (data[i].ret_vector != -1) {
            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_bom();
    test_iso_8859_16();
    //test_encode(); // Disable for now due to ECI change

    testReport();

    return 0;
}
