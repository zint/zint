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

static void test_encode_render(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int ret_encode;
        float w;
        float h;
        int ret_render;
        unsigned char* expected_daft;
    };
    struct item data[] = {
        /* 0*/ { "0100000000000AA000AA0A", 0, 100, 30, 1, "TFATTADAAATAFAFADFTAFATDTTDTTAAFTTFFTTDFTTFFTTAFADFDFAAFTDDFDADDAA" },
        /* 1*/ { "0100000000009JA500AA0A", 0, 100, 30, 1, "TAFTTDADATTFDTFDFDFDTAATADADTTTATTFTDDDDTATDATDFTFFATAFFAFADAFFTDT" },
        /* 2*/ { "1100000000000XY11     ", 0, 100, 30, 1, "TTDTTATTDTAATTDTAATTDTAATTDTTDDAATAADDATAATDDFAFTDDTAADDDTAAFDFAFF" },
        /* 3*/ { "21B2254800659JW5O9QA6Y", 0, 100, 30, 1, "DAATATTTADTAATTFADDDDTTFTFDDDDFFDFDAFTADDTFFTDDATADTTFATTDAFDTFDDA" },
        /* 4*/ { "11000000000000000XY11    ", 0, 100, 30, 1, "TTDTTATDDTTATTDTAATTDTAATDDTTATTDTTDATFTAATDDTAATDDTATATFAADDAATAATDDTAADFTFTA" },
        /* 5*/ { "41038422416563762EF61AH8T", 0, 100, 30, 1, "DTTFATTDDTATTTATFTDFFFTFDFDAFTTTADTTFDTFDDDTDFDDFTFAADTFDTDTDTFAATAFDDTAATTDTT" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char actual_daft[80];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MAILMARK;
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        assert_equal(symbol->rows, 3, "i:%d symbol->rows %d != 3\n", i, symbol->rows);

        ret = testUtilDAFTConvert(symbol, actual_daft, sizeof(actual_daft));
        assert_nonzero(ret, "i:%d testUtilDAFTConvert ret == 0", i);
        assert_zero(strcmp(actual_daft, data[i].expected_daft), "i:%d\n  actual %s\nexpected %s\n", i, actual_daft, data[i].expected_daft);

        ret = ZBarcode_Render( symbol, data[i].w, data[i].h );
        assert_equal(ret, data[i].ret_render, "i:%d ZBarcode_Render ret %d != %d\n", i, ret, data[i].ret_render);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_encode_render();

    testReport();

    return 0;
}
