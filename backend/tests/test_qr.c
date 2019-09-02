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
        int ret_render;
        int expected_size;
    };
    struct item data[] = {
        /* 0*/ { "12345", 0, 0, 0, 100, 100, 1, 11 },
        /* 1*/ { "12345", 1, 0, 0, 100, 100, 1, 11 },
        /* 2*/ { "12345", 2, 0, 0, 100, 100, 1, 13 },
        /* 3*/ { "12345", 3, 0, 0, 100, 100, 1, 17 },
        /* 4*/ { "12345", 4, 0, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /* 5*/ { "12345", 0, 1, 0, 100, 100, 1, 11 },
        /* 6*/ { "12345", 0, 2, 0, 100, 100, 1, 13 },
        /* 7*/ { "12345", 0, 3, 0, 100, 100, 1, 15 },
        /* 8*/ { "12345", 0, 4, 0, 100, 100, 1, 17 },
        /* 9*/ { "12345", 0, 5, 0, 100, 100, 1, 11 }, // Size > 4 ignored
        /*10*/ { "12345", 1, 5, 0, 100, 100, 1, 11 }, // Ignored also if ECC given
        /*11*/ { "12345", 1, 1, 0, 100, 100, 1, 11 },
        /*12*/ { "12345", 1, 2, 0, 100, 100, 1, 13 },
        /*13*/ { "12345", 1, 3, 0, 100, 100, 1, 15 },
        /*14*/ { "12345", 1, 4, 0, 100, 100, 1, 17 },
        /*15*/ { "12345", 2, 1, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*16*/ { "12345", 2, 2, 0, 100, 100, -1, 13 },
        /*17*/ { "12345", 2, 3, 0, 100, 100, -1, 15 },
        /*18*/ { "12345", 2, 4, 0, 100, 100, -1, 17 },
        /*19*/ { "12345", 3, 1, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*20*/ { "12345", 3, 2, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*21*/ { "12345", 3, 3, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*22*/ { "12345", 3, 4, 0, 100, 100, -1, 17 },
        /*23*/ { "12345", 4, 4, ZINT_ERROR_INVALID_OPTION, 100, 100, -1, 0 },
        /*24*/ { "12345", 5, 0, 0, 100, 100, 1, 11 }, // ECC > 4 ignored
        /*25*/ { "12345", 5, 1, 0, 100, 100, 1, 11 }, // Ignored also if size given

        /*26*/ { "123456", 1, 0, 0, 100, 100, 1, 13 },
        /*27*/ { "123456", 1, 1, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*28*/ { "123456", 1, 2, 0, 100, 100, 1, 13 },

        /*29*/ { "ABCDEF", 1, 0, 0, 100, 100, 1, 13 },
        /*30*/ { "ABCDEF", 1, 2, 0, 100, 100, 1, 13 },
        /*31*/ { "ABCDEF", 2, 2, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*32*/ { "ABCDE", 2, 0, 0, 100, 100, 1, 13 },

        /*33*/ { "ABCDEABCDEABCD", 1, 0, 0, 100, 100, 1, 15 }, // 14 alphanumerics
        /*34*/ { "ABCDEABCDEABCD", 1, 3, 0, 100, 100, 1, 15 },
        /*35*/ { "ABCDEABCDEABCD", 2, 3, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*36*/ { "ABCDEABCDEA", 2, 3, 0, 100, 100, 1, 15 }, // 11 alphanumerics

        /*37*/ { "ABCDEFGHIJABCDEFGHIJA", 1, 0, 0, 100, 100, 1, 17 }, // 21 alphanumerics
        /*38*/ { "ABCDEFGHIJABCDEFGHIJA", 1, 4, 0, 100, 100, 1, 17 },
        /*39*/ { "ABCDEFGHIJABCDEFGHIJA", 2, 4, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*40*/ { "ABCDEFGHIJABCDEFGH", 2, 4, 0, 100, 100, 1, 17 }, // 18 alphanumerics
        /*41*/ { "ABCDEFGHIJABCDEFGH", 3, 4, ZINT_ERROR_TOO_LONG, 100, 100, -1, 0 },
        /*42*/ { "ABCDEFGHIJABC", 3, 4, 0, 100, 100, 1, 17 }, // 13 alphanumerics
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
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        if (data[i].ret_render != -1) {
            ret = ZBarcode_Render( symbol, data[i].w, data[i].h );
            assert_equal(ret, data[i].ret_render, "i:%d ZBarcode_Render ret %d != %d\n", i, ret, data[i].ret_render);
            assert_equal(symbol->width, data[i].expected_size, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_size);
            assert_equal(symbol->rows, data[i].expected_size, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_size);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_microqr_options();

    testReport();

    return 0;
}
