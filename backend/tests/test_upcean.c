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

static void test_upce_length(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        int ret;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_UPCE, "12345", 0 },
        /* 1*/ { BARCODE_UPCE_CHK, "12345", ZINT_ERROR_INVALID_CHECK },
        /* 2*/ { BARCODE_UPCE_CHK, "12344", 0 }, // 4 is correct check digit
        /* 3*/ { BARCODE_UPCE, "123456", 0 },
        /* 4*/ { BARCODE_UPCE_CHK, "123456", ZINT_ERROR_INVALID_CHECK },
        /* 5*/ { BARCODE_UPCE_CHK, "123457", 0 }, // 7 is correct check digit
        /* 6*/ { BARCODE_UPCE, "1234567", 0 },
        /* 7*/ { BARCODE_UPCE_CHK, "1234567", ZINT_ERROR_INVALID_CHECK },
        /* 8*/ { BARCODE_UPCE_CHK, "1234565", 0 }, // 5 is correct check digit
        /* 9*/ { BARCODE_UPCE, "12345678", ZINT_ERROR_TOO_LONG },
        /*10*/ { BARCODE_UPCE_CHK, "12345678", ZINT_ERROR_INVALID_CHECK },
        /*11*/ { BARCODE_UPCE_CHK, "12345670", 0 }, // 0 is correct check digit
        /*12*/ { BARCODE_UPCE, "123456789", ZINT_ERROR_TOO_LONG },
        /*13*/ { BARCODE_UPCE_CHK, "123456789", ZINT_ERROR_TOO_LONG },

        /*14*/ { BARCODE_UPCE, "123406", ZINT_ERROR_INVALID_DATA }, // If last digit (emode) 6, 2nd last can't be zero
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_isbn(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int ret_encode;
        float w;
        float h;
        int ret_render;
    };
    struct item data[] = {
        /* 0*/ { "0", 0, 100, 30, 1 }, // Left zero-padded if < 10 chars
        /* 1*/ { "12345678", ZINT_ERROR_INVALID_CHECK, 100, 30, -1 },
        /* 2*/ { "12345679", 0, 100, 30, 1 }, // 9 is correct check digit
        /* 3*/ { "123456789", 0, 100, 30, 1 },
        /* 4*/ { "0123456789", 0, 100, 30, 1 },
        /* 5*/ { "1234567890", ZINT_ERROR_INVALID_CHECK, 100, 30, -1 },
        /* 6*/ { "123456789X", 0, 100, 30, 1 }, // X is correct check digit
        /* 7*/ { "8175257660", 0, 100, 30, 1 }, // 0 is correct check digit
        /* 8*/ { "0590764845", 0, 100, 30, 1 }, // 5 is correct check digit
        /* 9*/ { "0906495741", 0, 100, 30, 1 }, // 1 is correct check digit
        /*10*/ { "0140430016", 0, 100, 30, 1 }, // 6 is correct check digit
        /*11*/ { "0571086187", 0, 100, 30, 1 }, // 7 is correct check digit
        /*12*/ { "0486600882", 0, 100, 30, 1 }, // 2 is correct check digit
        /*13*/ { "12345678901", ZINT_ERROR_TOO_LONG, 100, 30, -1 },
        /*14*/ { "123456789012", ZINT_ERROR_TOO_LONG, 100, 30, -1 },
        /*15*/ { "1234567890123", ZINT_ERROR_INVALID_DATA, 100, 30, -1 },
        /*16*/ { "9784567890120", 0, 100, 30, 1 }, // 0 is correct check digit
        /*17*/ { "9783161484100", 0, 100, 30, 1 }, // 0 is correct check digit
        /*18*/ { "9781846688225", 0, 100, 30, 1 }, // 5 is correct check digit
        /*19*/ { "9781847657954", 0, 100, 30, 1 }, // 4 is correct check digit
        /*20*/ { "9781846688188", 0, 100, 30, 1 }, // 8 is correct check digit
        /*21*/ { "9781847659293", 0, 100, 30, 1 }, // 3 is correct check digit
        /*22*/ { "97845678901201", ZINT_ERROR_TOO_LONG, 100, 30, -1 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_ISBNX;
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d, errtxt %s\n", i, ret, data[i].ret_encode, symbol->errtxt);

        if (data[i].ret_render != -1) {
            ret = ZBarcode_Render( symbol, data[i].w, data[i].h );
            assert_equal(ret, data[i].ret_render, "i:%d ZBarcode_Render ret %d != %d\n", i, ret, data[i].ret_render);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_render_same(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        int ret_encode;
        float w;
        float h;
        int ret_render;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_UPCE, "123456", 0, 100, 30, 1 },
        /* 1*/ { BARCODE_UPCE_CHK, "1234565", 0, 100, 30, 1 }, // 5 is correct check digit
        /* 1*/ { BARCODE_ISBNX, "0195049969", 0, 100, 30, 1 }, // 9 is correct check digit
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_render* renders[4];
        int renders_size = sizeof(renders) / sizeof(struct zint_render*);

        for (int j = 0; j < renders_size; j++) {
            struct zint_symbol* symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            symbol->symbology = data[i].symbology;
            int length = strlen(data[i].data);

            ret = ZBarcode_Encode(symbol, data[i].data, length);
            assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

            ret = ZBarcode_Render(symbol, data[i].w, data[i].h);
            assert_equal(ret, data[i].ret_render, "i:%d ZBarcode_Render ret %d != %d\n", i, ret, data[i].ret_render);

            assert_nonnull(symbol->rendered, "i:%d symbol->rendered NULL\n", i);
            renders[j] = testUtilRenderCpy(symbol->rendered);

            ZBarcode_Delete(symbol);
        }

        for (int j = 1; j < renders_size; j++) {
            ret = testUtilRenderCmp(renders[j - 1], renders[j]);
            assert_zero(ret, "i:%d testUtilRenderCmp ret %d != 0\n", i, ret);
        }

        for (int j = 0; j < renders_size; j++) {
            struct zint_symbol symbol_render;
            symbol_render.rendered = renders[j];
            render_free(&symbol_render);
        }
    }

    testFinish();
}

int main()
{
    test_upce_length();
    test_isbn();
    test_render_same();

    testReport();

    return 0;
}
