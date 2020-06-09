/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2020 Robin Stuart <rstuart114@gmail.com>

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

static void test_input(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "41038422416563762XY11     ", 0, 3, 155 },
        /*  1*/ { "41038422416563762XY11      ", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { "41038422416563762xy11     ", 0, 3, 155 }, // Case insensitive
        /*  3*/ { "41038422416563762xy11    .", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  4*/ { "0100000000000AA000AA0A", 0, 3, 131, }, // Length 22, Mailmark C (2 digit chain id)
        /*  5*/ { "5100000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // 1st char format 0-4 only
        /*  6*/ { "0000000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // 2nd char version id 1-4 only
        /*  7*/ { "01F0000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // 3rd char class 0-9A-E only
        /*  8*/ { "0100A00000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // 4-5th chars chain id 2 digits
        /*  9*/ { "010000000000AAA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // 6-13th chars item id 8 digits
        /* 10*/ { "010000000000 AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // Remaining chars post code, TODO: test various types
        /* 11*/ { "01000000000000000AA000AA0A", 0, 3, 155, }, // Length 26, Mailmark L (6 digit chain id)
        /* 12*/ { "010A0000000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // 4-9th chars chain id 6 digits
        /* 13*/ { "010A000000000000AAA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // Post code
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_MAILMARK, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Royal Mail Mailmark barcode L encoding and decoding (Sep 2015) RMMBLED
// https://www.royalmail.com/sites/default/files/Mailmark-4-state-barcode-L-encoding-and-decoding-instructions-Sept-2015.pdf
static void test_encode_vector(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int ret_encode;
        float w;
        float h;
        int ret_vector;
        unsigned char *expected_daft;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { "0100000000000AA000AA0A", 0, 100, 30, 0, "TFATTADAAATAFAFADFTAFATDTTDTTAAFTTFFTTDFTTFFTTAFADFDFAAFTDDFDADDAA" },
        /* 1*/ { "0100000000009JA500AA0A", 0, 100, 30, 0, "TAFTTDADATTFDTFDFDFDTAATADADTTTATTFTDDDDTATDATDFTFFATAFFAFADAFFTDT" },
        /* 2*/ { "1100000000000XY11     ", 0, 100, 30, 0, "TTDTTATTDTAATTDTAATTDTAATTDTTDDAATAADDATAATDDFAFTDDTAADDDTAAFDFAFF" },
        /* 3*/ { "21B2254800659JW5O9QA6Y", 0, 100, 30, 0, "DAATATTTADTAATTFADDDDTTFTFDDDDFFDFDAFTADDTFFTDDATADTTFATTDAFDTFDDA" },
        /* 4*/ { "11000000000000000XY11    ", 0, 100, 30, 0, "TTDTTATDDTTATTDTAATTDTAATDDTTATTDTTDATFTAATDDTAATDDTATATFAADDAATAATDDTAADFTFTA" }, // Example from RMMBLED
        /* 5*/ { "41038422416563762EF61AH8T", 0, 100, 30, 0, "DTTFATTDDTATTTATFTDFFFTFDFDAFTTTADTTFDTFDDDTDFDDFTFAADTFDTDTDTFAATAFDDTAATTDTT" }, // Example from RMMBLED
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char actual_daft[80];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MAILMARK;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        assert_equal(symbol->rows, 3, "i:%d symbol->rows %d != 3\n", i, symbol->rows);

        ret = testUtilDAFTConvert(symbol, actual_daft, sizeof(actual_daft));
        assert_nonzero(ret, "i:%d testUtilDAFTConvert ret == 0", i);
        assert_zero(strcmp(actual_daft, data[i].expected_daft), "i:%d\n  actual %s\nexpected %s\n", i, actual_daft, data[i].expected_daft);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_input", test_input, 1, 0, 1 },
        { "test_encode_vector", test_encode_vector, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
