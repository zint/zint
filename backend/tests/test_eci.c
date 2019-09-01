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
        unsigned char* data;
        int ret_encode;
        float w;
        float h;
        int ret_render;
        int expected_eci;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), in Shift-JIS, in GB-2312/18030
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859), in Shift-JIS, in GB-2312/18030
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859), not in Shift-JIS, not in GB-2312/18030
    // ກ U+0E81 Lao not in any ISO 8859 (or Windows page) or Shift-JIS or GB-2312/18030
    struct item data[] = {
        /* 0*/ { BARCODE_QRCODE, UNICODE_MODE, "Aé", 0, 100, 30, 1, 3 },
        /* 1*/ { BARCODE_QRCODE, UNICODE_MODE, "Aβ", 0, 100, 30, 1, 3 }, // Converts to Shift-JIS
        /* 2*/ { BARCODE_QRCODE, UNICODE_MODE, "Aก", ZINT_WARN_USES_ECI, 100, 30, 1, 13 }, // ECI 13 == ISO 8859-11
        /* 3*/ { BARCODE_QRCODE, UNICODE_MODE, "Aéβ", 0, 100, 30, 1, 3 }, // Converts to Shift-JIS
        /* 4*/ { BARCODE_QRCODE, UNICODE_MODE, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 1, 26 },
        /* 5*/ { BARCODE_QRCODE, UNICODE_MODE, "Aກ", ZINT_WARN_USES_ECI, 100, 30, 1, 26 },
        /* 6*/ { BARCODE_MICROQR, UNICODE_MODE, "Aé", 0, 100, 30, 1, 3 },
        /* 7*/ { BARCODE_MICROQR, UNICODE_MODE, "Aβ", 0, 100, 30, 1, 3 }, // Converts to Shift-JIS
        /* 8*/ { BARCODE_MICROQR, UNICODE_MODE, "Aéβ", 0, 100, 30, 1, 3 }, // Converts to Shift-JIS
        /* 9*/ { BARCODE_MICROQR, UNICODE_MODE, "ก", ZINT_ERROR_INVALID_DATA, 100, 30, -1, -1 },
        /*10*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, "Aé", 0, 100, 30, 1, 3 },
        /*11*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, "Aβ", 0, 100, 30, 1, 3 }, // Converts to GB-2312
        /*12*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, "Aก", ZINT_WARN_USES_ECI, 100, 30, 1, 13 }, // ECI 13 == ISO 8859-11
        /*13*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, "Aéβ", 0, 100, 30, 1, 3 }, // Converts to GB-2312
        /*14*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 1, 26 },
        /*15*/ { BARCODE_HANXIN, UNICODE_MODE, "Aé", 0, 100, 30, 1, 3 },
        /*16*/ { BARCODE_HANXIN, UNICODE_MODE, "Aβ", 0, 100, 30, 1, 3 }, // Converts to GB-18030
        /*17*/ { BARCODE_HANXIN, UNICODE_MODE, "Aก", ZINT_WARN_USES_ECI, 100, 30, 1, 13 }, // ECI 13 == ISO 8859-11
        /*18*/ { BARCODE_HANXIN, UNICODE_MODE, "Aéβ", 0, 100, 30, 1, 3 }, // Converts to GB-18030
        /*19*/ { BARCODE_HANXIN, UNICODE_MODE, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 1, 26 },
        /*20*/ { BARCODE_UPNQR, UNICODE_MODE, "Aé", 0, 100, 30, 1, 4 }, // ECI 4 == iSO 8859-2
        /*21*/ { BARCODE_UPNQR, UNICODE_MODE, "Aβ", ZINT_ERROR_INVALID_DATA, 100, 30, -1, -1 },
        /*22*/ { BARCODE_DATAMATRIX, UNICODE_MODE, "Aé", 0, 100, 30, 1, 3 },
        /*23*/ { BARCODE_DATAMATRIX, UNICODE_MODE, "Aβ", ZINT_WARN_USES_ECI, 100, 30, 1, 9 }, // ECI 9 == ISO 8859-7
        /*24*/ { BARCODE_DATAMATRIX, UNICODE_MODE, "Aก", ZINT_WARN_USES_ECI, 100, 30, 1, 13 }, // ECI 13 == ISO 8859-11
        /*25*/ { BARCODE_DATAMATRIX, UNICODE_MODE, "Aéβ", ZINT_WARN_USES_ECI, 100, 30, 1, 26 },
        /*26*/ { BARCODE_DATAMATRIX, UNICODE_MODE, "Aéβก", ZINT_WARN_USES_ECI, 100, 30, 1, 26 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = data[i].input_mode;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        if (data[i].ret_render != -1) {
            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);

            ret = ZBarcode_Render( symbol, data[i].w, data[i].h );
            assert_equal(ret, data[i].ret_render, "i:%d ZBarcode_Render ret %d != %d\n", i, ret, data[i].ret_render);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_bom();
    test_iso_8859_16();
    test_encode();

    testReport();

    return 0;
}
