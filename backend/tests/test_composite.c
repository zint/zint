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

static void test_eanx_leading_zeroes(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        unsigned char* composite;
        int ret;

        int expected_rows;
        int expected_width;
    };
    // Vi} :s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_EANX_CC, "1", "[21]A12345678", 0, 8, 72 }, // EAN-8
        /* 1*/ { BARCODE_EANX_CC, "12", "[21]A12345678", 0, 8, 72 },
        /* 2*/ { BARCODE_EANX_CC, "123", "[21]A12345678", 0, 8, 72 },
        /* 3*/ { BARCODE_EANX_CC, "1234", "[21]A12345678", 0, 8, 72 },
        /* 4*/ { BARCODE_EANX_CC, "12345", "[21]A12345678", 0, 8, 72 },
        /* 5*/ { BARCODE_EANX_CC, "123456", "[21]A12345678", 0, 8, 72 },
        /* 6*/ { BARCODE_EANX_CC, "1234567", "[21]A12345678", 0, 8, 72 },
        /* 7*/ { BARCODE_EANX_CC, "12345678", "[21]A12345678", 0, 7, 99 }, // EAN-13
        /* 8*/ { BARCODE_EANX_CC, "1+12", "[21]A12345678", 0, 8, 101 }, // EAN-8 + EAN-2
        /* 9*/ { BARCODE_EANX_CC, "12+12", "[21]A12345678", 0, 8, 101 },
        /*10*/ { BARCODE_EANX_CC, "123+12", "[21]A12345678", 0, 8, 101 },
        /*11*/ { BARCODE_EANX_CC, "1234+12", "[21]A12345678", 0, 8, 101 },
        /*12*/ { BARCODE_EANX_CC, "12345+12", "[21]A12345678", 0, 8, 101 },
        /*13*/ { BARCODE_EANX_CC, "123456+12", "[21]A12345678", 0, 8, 101 },
        /*14*/ { BARCODE_EANX_CC, "1234567+12", "[21]A12345678", 0, 8, 101 },
        /*15*/ { BARCODE_EANX_CC, "12345678+12", "[21]A12345678", 0, 7, 128 }, // EAN-13 + EAN-2
        /*16*/ { BARCODE_EANX_CC, "1+123", "[21]A12345678", 0, 8, 128 }, // EAN-8 + EAN-5
        /*17*/ { BARCODE_EANX_CC, "12+123", "[21]A12345678", 0, 8, 128 },
        /*18*/ { BARCODE_EANX_CC, "123+123", "[21]A12345678", 0, 8, 128 },
        /*19*/ { BARCODE_EANX_CC, "1234+123", "[21]A12345678", 0, 8, 128 },
        /*20*/ { BARCODE_EANX_CC, "12345+123", "[21]A12345678", 0, 8, 128 },
        /*21*/ { BARCODE_EANX_CC, "123456+123", "[21]A12345678", 0, 8, 128 },
        /*22*/ { BARCODE_EANX_CC, "1234567+123", "[21]A12345678", 0, 8, 128 },
        /*23*/ { BARCODE_EANX_CC, "12345678+123", "[21]A12345678", 0, 7, 155 }, // EAN-13 + EAN-5
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        int length = strlen(data[i].data);
        assert_zero(length >= 128, "i:%d length %d >= 128\n", i, length);
        strcpy(symbol->primary, data[i].data);

        int composite_length = strlen(data[i].composite);

        ret = ZBarcode_Encode(symbol, data[i].composite, composite_length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_eanx_leading_zeroes();

    testReport();

    return 0;
}
