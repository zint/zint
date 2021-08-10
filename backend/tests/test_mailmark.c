/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2021 Robin Stuart <rstuart114@gmail.com>

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

static void test_input(int index, int debug) {

    struct item {
        char *data;
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
        /* 10*/ { "0100000000000 A000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // Remaining chars post code
        /* 11*/ { "0100000000000C1I2JQ3N ", 0, 3, 131, }, // F N F N L L N L S
        /* 12*/ { "010000000000091I2JQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N F N L L N L S bad 1st F
        /* 13*/ { "0100000000000CAI2JQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N F N L L N L S bad 1st N
        /* 14*/ { "0100000000000C1I2IQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N F N L L N L S bad 1st L
        /* 15*/ { "0100000000000C1I2IQ3NA", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N F N L L N L S bad S
        /* 16*/ { "0100000000000KM12JQ3N ", 0, 3, 131, }, // F F N N L L N L S
        /* 17*/ { "0100000000000K 12JQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N L L N L S bad 2nd F (non-numeric otherwise matches last pattern)
        /* 18*/ { "0100000000000KM1AJQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N L L N L S bad 2nd N
        /* 19*/ { "0100000000000KM12JO3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N L L N L S bad 2nd L
        /* 20*/ { "0100000000000KM12JQ3NA", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N L L N L S bad S
        /* 21*/ { "0100000000000OV123JQ4U", 0, 3, 131, }, // F F N N N L L N L
        /* 22*/ { "01000000000009V123JQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N N L L N L bad 1st F
        /* 23*/ { "0100000000000OV12AJQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N N L L N L bad 3rd N
        /* 24*/ { "0100000000000OV123JQ4V", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N N L L N L bad 3rd L
        /* 25*/ { "0100000000000CI1K3JQ4U", 0, 3, 131, }, // F F N F N L L N L
        /* 26*/ { "0100000000000CI1 3JQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N F N L L N L bad 3rd F (non-numeric otherwise matches pattern above)
        /* 27*/ { "0100000000000CIAK3JQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N F N L L N L bad 1st N
        /* 28*/ { "0100000000000CI1K3CQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N F N L L N L bad 1st L
        /* 29*/ { "0100000000000C12JQ3U  ", 0, 3, 131, }, // F N N L L N L S S
        /* 30*/ { "0100000000000912JQ3U  ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N L L N L S S bad F
        /* 31*/ { "0100000000000C1AJQ3U  ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N L L N L S S bad 2nd N
        /* 32*/ { "0100000000000C12JO3U  ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N L L N L S S bad 2nd L
        /* 33*/ { "0100000000000C12JQ3UA ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N L L N L S S bad 1st S
        /* 34*/ { "0100000000000C123JQ4U ", 0, 3, 131, }, // F N N N L L N L S
        /* 35*/ { "01000000000009123JQ4U ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N N L L N L S bad F
        /* 36*/ { "0100000000000C12AJQ4U ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N N L L N L S bad 3rd N
        /* 37*/ { "0100000000000C123JQ4V ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N N L L N L S bad 3rd L
        /* 38*/ { "0100000000000C123JQ4U1", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N N L L N L S bad S
        /* 39*/ { "01000000000000000AA000AA0A", 0, 3, 155, }, // Length 26, Mailmark L (6 digit chain id)
        /* 40*/ { "010A0000000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // 4-9th chars chain id 6 digits
        /* 41*/ { "010A0000000000000 A000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // Post code
        /* 42*/ { "01000000000000000C1I2JQ3N ", 0, 3, 155, }, // F N F N L L N L S
        /* 43*/ { "01000000000000000C1 2JQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N F N L L N L S bad 2nd F
        /* 44*/ { "01000000000000000KM12JQ3N ", 0, 3, 155, }, // F F N N L L N L S
        /* 45*/ { "01000000000000000KM12JQAN ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N L L N L S bad 3rd N
        /* 46*/ { "01000000000000000OV123JQ4U", 0, 3, 155, }, // F F N N N L L N L
        /* 47*/ { "01000000000000000OV123IQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N N N L L N L bad 1st L
        /* 48*/ { "01000000000000000CI1K3JQ4U", 0, 3, 155, }, // F F N F N L L N L
        /* 49*/ { "010000000000000009I1K3JQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F F N F N L L N L bad 1st F
        /* 50*/ { "01000000000000000C12JQ3U  ", 0, 3, 155, }, // F N N L L N L S S
        /* 51*/ { "01000000000000000C12JQ3U A", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N L L N L S S bad 2nd S
        /* 52*/ { "01000000000000000C123JQ4U ", 0, 3, 155, }, // F N N N L L N L S
        /* 53*/ { "01000000000000000C 23JQ4U ", ZINT_ERROR_INVALID_DATA, -1, -1 }, // F N N N L L N L S bad 1st N (non-alpha otherwise matches 2nd pattern)
        /* 54*/ { "41038422416563762XY1", ZINT_ERROR_INVALID_DATA, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Royal Mail Mailmark barcode C encoding and decoding (Sep 2015) RMMBCED
// https://www.royalmail.com/sites/default/files/Mailmark-4-state-barcode-C-encoding-and-decoding-instructions-Sept-2015.pdf
// Royal Mail Mailmark barcode L encoding and decoding (Sep 2015) RMMBLED
// https://www.royalmail.com/sites/default/files/Mailmark-4-state-barcode-L-encoding-and-decoding-instructions-Sept-2015.pdf
static void test_encode_vector(int index, int debug) {

    struct item {
        char *data;
        int ret_encode;
        float w;
        float h;
        int ret_vector;
        char *expected_daft;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "1100000000000XY11     ", 0, 100, 30, 0, "TTDTTATTDTAATTDTAATTDTAATTDTTDDAATAADDATAATDDFAFTDDTAADDDTAAFDFAFF" }, // Example 1 from RMMBCED
        /*  1*/ { "21B2254800659JW5O9QA6Y", 0, 100, 30, 0, "DAATATTTADTAATTFADDDDTTFTFDDDDFFDFDAFTADDTFFTDDATADTTFATTDAFDTFDDA" }, // Example 2 from RMMBCED
        /*  2*/ { "11000000000000000XY11    ", 0, 100, 30, 0, "TTDTTATDDTTATTDTAATTDTAATDDTTATTDTTDATFTAATDDTAATDDTATATFAADDAATAATDDTAADFTFTA" }, // Example 1 from RMMBLED
        /*  3*/ { "41038422416563762EF61AH8T", 0, 100, 30, 0, "DTTFATTDDTATTTATFTDFFFTFDFDAFTTTADTTFDTFDDDTDFDDFTFAADTFDTDTDTFAATAFDDTAATTDTT" }, // Example 2 from RMMBLED
        /*  4*/ { "0100000000000AA000AA0A", 0, 100, 30, 0, "TFATTADAAATAFAFADFTAFATDTTDTTAAFTTFFTTDFTTFFTTAFADFDFAAFTDDFDADDAA" },
        /*  5*/ { "0100000000009JA500AA0A", 0, 100, 30, 0, "TAFTTDADATTFDTFDFDFDTAATADADTTTATTFTDDDDTATDATDFTFFATAFFAFADAFFTDT" },
        /*  6*/ { "1234567890123C1I2JQ3N ", 0, 100, 30, 0, "TTDTDFTFDTAAFFDATAATFDDTATTTFDFTDTTFDFTTTFAAFFTDTADDATDDDTFFATFFAF" }, // F N F N L L N L S
        /*  7*/ { "2345678901234KM12JQ3N ", 0, 100, 30, 0, "TDTFAFADDATATATTAAFAFTFTDADFDTDAAADFFDTTDDTDDFATDDTFDATFDFTFFFDFTA" }, // F F N N L L N L S
        /*  8*/ { "34A6789012345OV123JQ4U", 0, 100, 30, 0, "TDTTATADDATDADADATTAFATDAADAFTTTAAFDATFAFDDFDATFDTADDTFFAAFDDTTAAD" }, // F F N N N L L N L
        /*  9*/ { "44E9999999999CI1K3JQ4U", 0, 100, 30, 0, "DADTFDATFAFAFFAADTDFTTAAFFAFFDFATFTFDDAAFDFDDFAFDTDTDDATTDATTAFDAD" }, // F F N F N L L N L
        /* 10*/ { "0418070504030C12JQ3U  ", 0, 100, 30, 0, "FTAFFDAFTDDFTDTAFAADDDTADFTFAFFFATAAATFTDDDATFATFTTFTFFATADTDAAFTA" }, // F N N L L N L S S
        /* 11*/ { "02D7543219876C123JQ4U ", 0, 100, 30, 0, "AFTFAFATFDDFFDTTDDFAFDFDAFFTFAAFTTDATDFATATFTDFTFTTTFFTTATDATFDFTA" }, // F N N N L L N L S
        /* 12*/ { "01000000000000000AA000AA0A", 0, 100, 30, 0, "FDTTATTDDFFDATFAFDDTFFDDAATTTATTDTTADADDFDFTFAFAADTADDFDFTDTFTAFDDATDAFDFFATDT" },
        /* 13*/ { "01000000000000009JA500AA0A", 0, 100, 30, 0, "FDTDAADTDAAADDDAFADDDFDDAFAADDAADTTAAFFDDTFTTDDTDFDTTDDTTFFDTDTFAADTAFAAFFDFDF" },
        /* 14*/ { "01000000000000000C1I2JQ3N ", 0, 100, 30, 0, "TDTTDFTDDFDFFDTTAAATFFAADAFTTATTDTTAATFATDTTFAAFDFDAFTDADFTDFDFTFFFDADAFDDAATT" }, // F N F N L L N L S
        /* 15*/ { "01777777777777777KM12JQ3N ", 0, 100, 30, 0, "AFFADATDDTATDFFAFAAADDTATFFDADDDADDAFFAFDDTADTTFDFATAFDDAAFFTTAFADTAAFTTDTFFTA" }, // F F N N L L N L S
        /* 16*/ { "33C55555555555555OV123JQ4U", 0, 100, 30, 0, "FDTTFDDTDFFAAAADDFFDTFTFDDTADAAADATTFAFTTFDATTDDTDAFTAFTDDTTADDFFTAFDTDDTDTTAF" }, // F F N N N L L N L
        /* 17*/ { "42944444444444444CI1K3JQ4U", 0, 100, 30, 0, "FTDDTTTTFAFFTATFTTFTDTFTFAADADTAFDADDTFDFDAFDFTTAFATFDFDTTFATFDFDAAAFTFTDFFTAF" }, // F F N F N L L N L
        /* 18*/ { "31833333333333333C12JQ3U  ", 0, 100, 30, 0, "DTTAFFDATATFAADAFDFATFFTFFTTTADTTTDTAAATDDTFFDDFTAADTTDTFFFDAFTFAADFDDAFDFTAFF" }, // F N N L L N L S S
        /* 19*/ { "22799999999999999C123JQ4U ", 0, 100, 30, 0, "DDATTDDATATTTAFDTAADATDDFFTFFDFFDTFAADDFAADFDFFTFFTFFDFDFTATATFDDFTFFFTFFTDDTF" }, // F N N N L L N L S
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char actual_daft[80];

    testStart("test_encode_vector");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

        assert_equal(symbol->rows, 3, "i:%d symbol->rows %d != 3\n", i, symbol->rows);

        ret = testUtilDAFTConvert(symbol, actual_daft, sizeof(actual_daft));
        assert_nonzero(ret, "i:%d testUtilDAFTConvert ret == 0", i);
        assert_zero(strcmp(actual_daft, data[i].expected_daft), "i:%d\n  actual %s\nexpected %s\n", i, actual_daft, data[i].expected_daft);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d (%s)\n", i, ret, data[i].ret_vector, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { "1100000000000XY11     ", 0, 3, 131, "Verified manually against tec-it",
                    "00000000001000000000101000000000101000000000101000000000000000101000101000001000101000000010101000000000101000000000101010001010101"
                    "10101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "00001000000000001000000000001000000000001000000000001000001010000000000010100000000000101010001000101000000010101000000010101000101"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_input", test_input, 1, 0, 1 },
        { "test_encode_vector", test_encode_vector, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
