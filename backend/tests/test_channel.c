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

static void test_encode(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_2;
        int ret_encode;
        float w;
        float h;
        int ret_vector;
    };
    // Vi} :s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { "0", 0, 0, 100, 30, 0 },
        /* 1*/ { "1", 1, 0, 100, 30, 0 },
        /* 2*/ { "26", 2, 0, 100, 30, 0 },
        /* 3*/ { "026", 3, 0, 100, 30, 0 },
        /* 4*/ { "0026", 3, 0, 100, 30, 0 },
        /* 5*/ { "1234", 3, ZINT_ERROR_INVALID_DATA, 100, 30, -1 },
        /* 6*/ { "1234", 4, ZINT_ERROR_INVALID_DATA, 100, 30, -1 },
        /* 7*/ { "292", 4, 0, 100, 30, 0 },
        /* 8*/ { "1234", 5, 0, 100, 30, 0 },
        /* 9*/ { "1234567", 0, 0, 100, 30, 0 },
        /*10*/ { "576688", 7, 0, 100, 30, 0 },
        /*11*/ { "576689", 7, ZINT_ERROR_INVALID_DATA, 100, 30, -1 },
        /*12*/ { "1234567", 0, 0, 100, 30, 0 },
        /*13*/ { "1234567", 8, 0, 100, 30, 0 },
        /*14*/ { "7742863", 8, ZINT_ERROR_INVALID_DATA, 100, 30, -1 },
        /*15*/ { "0000000", 2, 0, 100, 30, 0 },
        /*16*/ { "12345678", 8, ZINT_ERROR_TOO_LONG, 100, 30, -1 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_CHANNEL;
        symbol->option_2 = data[i].option_2;
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_encode();

    testReport();

    return 0;
}
