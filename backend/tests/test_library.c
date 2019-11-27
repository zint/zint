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

static void test_checks(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        int length;
        int input_mode;
        int eci;
        float dot_size;
        int ret;

        char* expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_CODE128, "1234", -1, -1, 3, -1, ZINT_ERROR_INVALID_OPTION, "Error 217: Symbology does not support ECI switching" },
        /* 1*/ { BARCODE_CODE128, "1234", -1, -1, 0, -1, 0, "" },
        /* 2*/ { BARCODE_QRCODE, "1234", -1, -1, 3, -1, 0, "" },
        /* 3*/ { BARCODE_QRCODE, "1234", -1, -1, 999999 + 1, -1, ZINT_ERROR_INVALID_OPTION, "Error 218: Invalid ECI mode" },
        /* 4*/ { BARCODE_CODE128, "1234", -1, -1, -1, 20.1, ZINT_ERROR_INVALID_OPTION, "Error 221: Invalid dot size" },
        /* 5*/ { BARCODE_CODE128, "1234", -1, GS1_MODE, -1, -1, ZINT_ERROR_INVALID_OPTION, "Error 220: Selected symbology does not support GS1 mode" },
        /* 6*/ { BARCODE_EAN128, "[21]12\0004", 8, GS1_MODE, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 262: NUL characters not permitted in GS1 mode" },
        /* 7*/ { BARCODE_EAN128, "[21]12é4", -1, GS1_MODE, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 250: Extended ASCII characters are not supported by GS1" },
        /* 8*/ { BARCODE_EAN128, "[21]12\0074", -1, GS1_MODE, -1, -1, ZINT_ERROR_INVALID_DATA, "Error 251: Control characters are not supported by GS1" },
        /* 9*/ { BARCODE_EAN128, "[21]1234", -1, GS1_MODE, -1, -1, 0, "" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char* text;
    char* primary;
    char escaped_primary[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].input_mode != -1) {
            symbol->input_mode = data[i].input_mode;
        }
        if (data[i].eci != -1) {
            symbol->eci = data[i].eci;
        }
        if (data[i].dot_size != -1) {
            symbol->dot_size = data[i].dot_size;
        }
        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret, symbol->errtxt);

        ret = strcmp(symbol->errtxt, data[i].expected);
        assert_zero(ret, "i:%d (%d) strcmp(%s, %s) %d != 0\n", i, data[i].symbology, symbol->errtxt, data[i].expected, ret);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input_mode(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int input_mode;
        int ret;

        int expected_input_mode;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "1234", DATA_MODE, 0, DATA_MODE },
        /*  1*/ { "1234", DATA_MODE | ESCAPE_MODE, 0, DATA_MODE | ESCAPE_MODE },
        /*  2*/ { "1234", UNICODE_MODE, 0, UNICODE_MODE },
        /*  3*/ { "1234", UNICODE_MODE | ESCAPE_MODE, 0, UNICODE_MODE | ESCAPE_MODE },
        /*  4*/ { "[01]12345678901234", GS1_MODE, 0, GS1_MODE },
        /*  5*/ { "[01]12345678901234", GS1_MODE | ESCAPE_MODE, 0, GS1_MODE | ESCAPE_MODE },
        /*  6*/ { "1234", 4 | ESCAPE_MODE, 0, DATA_MODE }, // Unknown mode reset to bare DATA_MODE
        /*  7*/ { "1234", -1, 0, DATA_MODE },
        /*  8*/ { "1234", DATA_MODE | 0x10, 0, DATA_MODE | 0x10 }, // Unknown flags kept (but ignored)
        /*  9*/ { "1234", UNICODE_MODE | 0x10, 0, UNICODE_MODE | 0x10 },
        /* 10*/ { "[01]12345678901234", GS1_MODE | 0x20, 0, GS1_MODE | 0x20 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_CODE49; // Supports GS1
        symbol->input_mode = data[i].input_mode;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->input_mode, data[i].expected_input_mode, "i:%d symbol->input_mode %d != %d\n", i, symbol->input_mode, data[i].expected_input_mode);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_checks();
    test_input_mode();

    testReport();

    return 0;
}
