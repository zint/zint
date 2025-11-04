/*
    libzint - the open source barcode library
    Copyright (C) 2019-2025 Robin Stuart <rstuart114@gmail.com>

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
/* SPDX-License-Identifier: BSD-3-Clause */

#include "testcommon.h"

static void test_4s_hrt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int output_options;
        const char *data;

        const char *expected;
        const char *expected_content;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, "1100000000000XY11", "", "" }, /* None */
        /*  1*/ { BARCODE_CONTENT_SEGS, "1100000000000XY11", "", "1100000000000XY11     " },
        /*  2*/ { -1, "1100000000000XY11 ", "", "" }, /* None */
        /*  3*/ { BARCODE_CONTENT_SEGS, "1100000000000XY11 ", "", "1100000000000XY11     " },
        /*  4*/ { -1, "0100000000000A00AA0A", "", "" }, /* None */
        /*  5*/ { BARCODE_CONTENT_SEGS, "0100000000000A00AA0A", "", "0100000000000A00AA0A  " }, /* None */
        /*  6*/ { -1, "41038422416563762XY11  ", "", "" }, /* None */
        /*  7*/ { BARCODE_CONTENT_SEGS, "41038422416563762XY11  ", "", "41038422416563762XY11     " },
        /*  8*/ { -1, "01000000000000000AA000AA0A", "", "" }, /* None */
        /*  9*/ { BARCODE_CONTENT_SEGS, "01000000000000000AA000AA0A", "", "01000000000000000AA000AA0A" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;
    int expected_length, expected_content_length;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK_4S, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                                    data[i].data, -1, debug);
        expected_length = (int) strlen(data[i].expected);
        expected_content_length = (int) strlen(data[i].expected_content);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d (%s)\n",
                    i, symbol->text_length, expected_length, symbol->text);
        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->text, data[i].expected);
        if (symbol->output_options & BARCODE_CONTENT_SEGS) {
            assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
            assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
            assert_equal(symbol->content_segs[0].length, expected_content_length,
                        "i:%d content_segs[0].length %d != expected_content_length %d\n",
                        i, symbol->content_segs[0].length, expected_content_length);
            assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected_content, expected_content_length),
                        "i:%d memcmp(%.*s, %s, %d) != 0\n",
                        i, symbol->content_segs[0].length, symbol->content_segs[0].source, data[i].expected_content,
                        expected_content_length);
        } else {
            assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_4s_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int ret;
        int expected_rows;
        int expected_width;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "41038422416563762XY11     ", 0, 3, 155 },
        /*  1*/ { "41038422416563762XY11      ", ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { "4103842241656", ZINT_ERROR_TOO_LONG, -1, -1 }, /* Too short (< 14) */
        /*  3*/ { "41038422416563", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  4*/ { "41038422416563762xy11     ", 0, 3, 155 }, /* Case insensitive */
        /*  5*/ { "41038422416563762xy11    .", ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  6*/ { "0100000000000AA000AA0A", 0, 3, 131, }, /* Length 22, Mailmark C (2 digit chain id) */
        /*  7*/ { "5100000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* 1st char format 0-4 only */
        /*  8*/ { "0000000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* 2nd char version id 1-4 only */
        /*  9*/ { "01F0000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* 3rd char class 0-9A-E only */
        /* 10*/ { "0100A00000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* 4-5th chars chain id 2 digits */
        /* 11*/ { "010000000000AAA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* 6-13th chars item id 8 digits */
        /* 12*/ { "0100000000000 A000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* Remaining chars post code */
        /* 13*/ { "0100000000000C1I2JQ3N ", 0, 3, 131, }, /* F N F N L L N L S */
        /* 14*/ { "010000000000091I2JQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N F N L L N L S bad 1st F */
        /* 15*/ { "0100000000000CAI2JQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N F N L L N L S bad 1st N */
        /* 16*/ { "0100000000000C1I2IQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N F N L L N L S bad 1st L */
        /* 17*/ { "0100000000000C1I2IQ3NA", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N F N L L N L S bad S */
        /* 18*/ { "0100000000000KM12JQ3N ", 0, 3, 131, }, /* F F N N L L N L S */
        /* 19*/ { "0100000000000K 12JQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N L L N L S bad 2nd F (non-numeric otherwise matches last pattern) */
        /* 20*/ { "0100000000000KM1AJQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N L L N L S bad 2nd N */
        /* 21*/ { "0100000000000KM12JO3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N L L N L S bad 2nd L */
        /* 22*/ { "0100000000000KM12JQ3NA", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N L L N L S bad S */
        /* 23*/ { "0100000000000OV123JQ4U", 0, 3, 131, }, /* F F N N N L L N L */
        /* 24*/ { "01000000000009V123JQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N N L L N L bad 1st F */
        /* 25*/ { "0100000000000OV12AJQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N N L L N L bad 3rd N */
        /* 26*/ { "0100000000000OV123JQ4V", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N N L L N L bad 3rd L */
        /* 27*/ { "0100000000000CI1K3JQ4U", 0, 3, 131, }, /* F F N F N L L N L */
        /* 28*/ { "0100000000000CI1 3JQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N F N L L N L bad 3rd F (non-numeric otherwise matches pattern above) */
        /* 29*/ { "0100000000000CIAK3JQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N F N L L N L bad 1st N */
        /* 30*/ { "0100000000000CI1K3CQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N F N L L N L bad 1st L */
        /* 31*/ { "0100000000000C12JQ3U  ", 0, 3, 131, }, /* F N N L L N L S S */
        /* 32*/ { "0100000000000912JQ3U  ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N L L N L S S bad F */
        /* 33*/ { "0100000000000C1AJQ3U  ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N L L N L S S bad 2nd N */
        /* 34*/ { "0100000000000C12JO3U  ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N L L N L S S bad 2nd L */
        /* 35*/ { "0100000000000C12JQ3UA ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N L L N L S S bad 1st S */
        /* 36*/ { "0100000000000C123JQ4U ", 0, 3, 131, }, /* F N N N L L N L S */
        /* 37*/ { "01000000000009123JQ4U ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N N L L N L S bad F */
        /* 38*/ { "0100000000000C12AJQ4U ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N N L L N L S bad 3rd N */
        /* 39*/ { "0100000000000C123JQ4V ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N N L L N L S bad 3rd L */
        /* 40*/ { "0100000000000C123JQ4U1", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N N L L N L S bad S */
        /* 41*/ { "01000000000000000AA000AA0A", 0, 3, 155, }, /* Length 26, Mailmark L (6 digit chain id) */
        /* 42*/ { "010A0000000000000AA000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* 4-9th chars chain id 6 digits */
        /* 43*/ { "010A0000000000000 A000AA0A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* Post code */
        /* 44*/ { "01000000000000000C1I2JQ3N ", 0, 3, 155, }, /* F N F N L L N L S */
        /* 45*/ { "01000000000000000C1 2JQ3N ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N F N L L N L S bad 2nd F */
        /* 46*/ { "01000000000000000KM12JQ3N ", 0, 3, 155, }, /* F F N N L L N L S */
        /* 47*/ { "01000000000000000KM12JQAN ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N L L N L S bad 3rd N */
        /* 48*/ { "01000000000000000OV123JQ4U", 0, 3, 155, }, /* F F N N N L L N L */
        /* 49*/ { "01000000000000000OV123IQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N N N L L N L bad 1st L */
        /* 50*/ { "01000000000000000CI1K3JQ4U", 0, 3, 155, }, /* F F N F N L L N L */
        /* 51*/ { "010000000000000009I1K3JQ4U", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F F N F N L L N L bad 1st F */
        /* 52*/ { "01000000000000000C12JQ3U  ", 0, 3, 155, }, /* F N N L L N L S S */
        /* 53*/ { "01000000000000000C12JQ3U A", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N L L N L S S bad 2nd S */
        /* 54*/ { "01000000000000000C123JQ4U ", 0, 3, 155, }, /* F N N N L L N L S */
        /* 55*/ { "01000000000000000C 23JQ4U ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* F N N N L L N L S bad 1st N (non-alpha otherwise matches 2nd pattern) */
        /* 56*/ { "01000000000000000         ", ZINT_ERROR_INVALID_DATA, -1, -1 }, /* All spaces */
        /* 57*/ { "41038422416563762XY1", ZINT_ERROR_INVALID_DATA, -1, -1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK_4S, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_2*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n",
                        i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n",
                        i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Royal Mail Mailmark barcode C encoding and decoding (Sep 2015) RMMBCED
   https://www.royalmail.com/sites/default/files/Mailmark-4-state-barcode-C-encoding-and-decoding-instructions-Sept-2015.pdf
   Royal Mail Mailmark barcode L encoding and decoding (Sep 2015) RMMBLED
   https://www.royalmail.com/sites/default/files/Mailmark-4-state-barcode-L-encoding-and-decoding-instructions-Sept-2015.pdf
*/
static void test_4s_encode_vector(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int ret_encode;
        float w;
        float h;
        int ret_vector;
        const char *expected_daft;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "1100000000000XY11     ", 0, 100, 30, 0, "TTDTTATTDTAATTDTAATTDTAATTDTTDDAATAADDATAATDDFAFTDDTAADDDTAAFDFAFF" }, /* Example 1 from RMMBCED */
        /*  1*/ { "21B2254800659JW5O9QA6Y", 0, 100, 30, 0, "DAATATTTADTAATTFADDDDTTFTFDDDDFFDFDAFTADDTFFTDDATADTTFATTDAFDTFDDA" }, /* Example 2 from RMMBCED */
        /*  2*/ { "11000000000000000XY11    ", 0, 100, 30, 0, "TTDTTATDDTTATTDTAATTDTAATDDTTATTDTTDATFTAATDDTAATDDTATATFAADDAATAATDDTAADFTFTA" }, /* Example 1 from RMMBLED */
        /*  3*/ { "41038422416563762EF61AH8T", 0, 100, 30, 0, "DTTFATTDDTATTTATFTDFFFTFDFDAFTTTADTTFDTFDDDTDFDDFTFAADTFDTDTDTFAATAFDDTAATTDTT" }, /* Example 2 from RMMBLED */
        /*  4*/ { "0100000000000AA000AA0A", 0, 100, 30, 0, "TFATTADAAATAFAFADFTAFATDTTDTTAAFTTFFTTDFTTFFTTAFADFDFAAFTDDFDADDAA" },
        /*  5*/ { "0100000000009JA500AA0A", 0, 100, 30, 0, "TAFTTDADATTFDTFDFDFDTAATADADTTTATTFTDDDDTATDATDFTFFATAFFAFADAFFTDT" },
        /*  6*/ { "1234567890123C1I2JQ3N ", 0, 100, 30, 0, "TTDTDFTFDTAAFFDATAATFDDTATTTFDFTDTTFDFTTTFAAFFTDTADDATDDDTFFATFFAF" }, /* F N F N L L N L S */
        /*  7*/ { "2345678901234KM12JQ3N ", 0, 100, 30, 0, "TDTFAFADDATATATTAAFAFTFTDADFDTDAAADFFDTTDDTDDFATDDTFDATFDFTFFFDFTA" }, /* F F N N L L N L S */
        /*  8*/ { "34A6789012345OV123JQ4U", 0, 100, 30, 0, "TDTTATADDATDADADATTAFATDAADAFTTTAAFDATFAFDDFDATFDTADDTFFAAFDDTTAAD" }, /* F F N N N L L N L */
        /*  9*/ { "44E9999999999CI1K3JQ4U", 0, 100, 30, 0, "DADTFDATFAFAFFAADTDFTTAAFFAFFDFATFTFDDAAFDFDDFAFDTDTDDATTDATTAFDAD" }, /* F F N F N L L N L */
        /* 10*/ { "0418070504030C12JQ3U  ", 0, 100, 30, 0, "FTAFFDAFTDDFTDTAFAADDDTADFTFAFFFATAAATFTDDDATFATFTTFTFFATADTDAAFTA" }, /* F N N L L N L S S */
        /* 11*/ { "02D7543219876C123JQ4U ", 0, 100, 30, 0, "AFTFAFATFDDFFDTTDDFAFDFDAFFTFAAFTTDATDFATATFTDFTFTTTFFTTATDATFDFTA" }, /* F N N N L L N L S */
        /* 12*/ { "01000000000000000AA000AA0A", 0, 100, 30, 0, "FDTTATTDDFFDATFAFDDTFFDDAATTTATTDTTADADDFDFTFAFAADTADDFDFTDTFTAFDDATDAFDFFATDT" },
        /* 13*/ { "01000000000000009JA500AA0A", 0, 100, 30, 0, "FDTDAADTDAAADDDAFADDDFDDAFAADDAADTTAAFFDDTFTTDDTDFDTTDDTTFFDTDTFAADTAFAAFFDFDF" },
        /* 14*/ { "01000000000000000C1I2JQ3N ", 0, 100, 30, 0, "TDTTDFTDDFDFFDTTAAATFFAADAFTTATTDTTAATFATDTTFAAFDFDAFTDADFTDFDFTFFFDADAFDDAATT" }, /* F N F N L L N L S */
        /* 15*/ { "01777777777777777KM12JQ3N ", 0, 100, 30, 0, "AFFADATDDTATDFFAFAAADDTATFFDADDDADDAFFAFDDTADTTFDFATAFDDAAFFTTAFADTAAFTTDTFFTA" }, /* F F N N L L N L S */
        /* 16*/ { "33C55555555555555OV123JQ4U", 0, 100, 30, 0, "FDTTFDDTDFFAAAADDFFDTFTFDDTADAAADATTFAFTTFDATTDDTDAFTAFTDDTTADDFFTAFDTDDTDTTAF" }, /* F F N N N L L N L */
        /* 17*/ { "42944444444444444CI1K3JQ4U", 0, 100, 30, 0, "FTDDTTTTFAFFTATFTTFTDTFTFAADADTAFDADDTFDFDAFDFTTAFATFDFDTTFATFDFDAAAFTFTDFFTAF" }, /* F F N F N L L N L */
        /* 18*/ { "31833333333333333C12JQ3U  ", 0, 100, 30, 0, "DTTAFFDATATFAADAFDFATFFTFFTTTADTTTDTAAATDDTFFDDFTAADTTDTFFFDAFTFAADFDDAFDFTAFF" }, /* F N N L L N L S S */
        /* 19*/ { "22799999999999999C123JQ4U ", 0, 100, 30, 0, "DDATTDDATATTTAFDTAADATDDFFTFFDFFDTFAADDFAADFDFFTFFTFFDFDFTATATFDDFTFFFTFFTDDTF" }, /* F N N N L L N L S */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char actual_daft[80];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK_4S, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret_encode, symbol->errtxt);

        assert_equal(symbol->rows, 3, "i:%d symbol->rows %d != 3\n", i, symbol->rows);

        ret = testUtilDAFTConvert(symbol, actual_daft, sizeof(actual_daft));
        assert_nonzero(ret, "i:%d testUtilDAFTConvert ret == 0", i);
        assert_zero(strcmp(actual_daft, data[i].expected_daft), "i:%d\n  actual %s\nexpected %s\n",
                    i, actual_daft, data[i].expected_daft);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d (%s)\n",
                    i, ret, data[i].ret_vector, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_4s_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;
        int ret;

        int expected_rows;
        int expected_width;
        const char *comment;
        const char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { "1100000000000XY11     ", 0, 3, 131, "Verified manually against TEC-IT",
                    "00000000001000000000101000000000101000000000101000000000000000101000101000001000101000000010101000000000101000000000101010001010101"
                    "10101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "00001000000000001000000000001000000000001000000000001000001010000000000010100000000000101010001000101000000010101000000010101000101"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK_4S, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n",
                            i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n",
                            i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n",
                            i, ret, width, row, data[i].data);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_2d_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_2;
        const char *data;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
        int expected_option_2;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
                    /* UUUUIVCSSSSSSSIIIIIIIIDDDDDDDDDSRRRRRRRSSSSSS */
    static const struct item data[] = {
        /*  0*/ { -1, "012100123412345678AB19XY1A 0", 0, 24, 24, "", 8 },
        /*  1*/ { -1, "012100123412345678ab19xy1a 0", 0, 24, 24, "", 8 }, /* Converts to upper */
        /*  2*/ { -1, "jgb 012100123412345678ab19xy1a 0", 0, 24, 24, "", 8 },
        /*  3*/ { -1, "012100123412345678AB19XY1A 0901234567890123456789012345678901234567890123456789012345678901", ZINT_ERROR_TOO_LONG, -1, -1, "Error 589: Input length 91 too long (maximum 90)", 0 },
        /*  4*/ { -1, "012100123412345678AB19XY1A ", ZINT_ERROR_TOO_LONG, -1, -1, "Error 860: Input length 27 too short (minimum 28)", 0 },
        /*  5*/ { -1, "012100123412345678AB19XY1A 090123456789012345678901234567890123456789012345678901234567", ZINT_ERROR_TOO_LONG, -1, -1, "Error 861: Input length 87 too long (maximum 86)", 0 },
        /*  6*/ { -1, "JGB 012100123412345678AB19XY1A ", ZINT_ERROR_TOO_LONG, -1, -1, "Error 862: Input length 31 too short (minimum 32)", 0 },
        /*  7*/ { 9, "JGB 012100123412345678AB19XY1A 0", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 863: Invalid Version '9' (8, 10 or 30 only)", 9 },
        /*  8*/ { -1, "JGB 012100123412345678AB19XY1A 0             123456", 0, 24, 24, "", 8 },
        /*  9*/ { 8, "JGB 012100123412345678AB19XY1A 0             1234567", ZINT_ERROR_TOO_LONG, -1, -1, "Error 864: Input length 52 too long for Version 8 (maximum 51)", 8 },
        /* 10*/ { -1, "JGB 012100123412345678AB19XY1A 0             1234567890123456789012345", 0, 16, 48, "", 30 },
        /* 11*/ { 30, "JGB 012100123412345678AB19XY1A 0             12345678901234567890123456", ZINT_ERROR_TOO_LONG, -1, -1, "Error 865: Input length 71 too long for Version 30 (maximum 70)", 30 },
        /* 12*/ { -1, "JGB 012100123412345678AB19XY1A 0             123456789012345678901234567890123456789012345", 0, 32, 32, "", 10 },
        /* 13*/ { -1, "JGB 012100123412345678AB19XY1A 0             1234567890123456789012345678901234567890123456", ZINT_ERROR_TOO_LONG, -1, -1, "Error 589: Input length 91 too long (maximum 90)", 0 },
        /* 14*/ { -1, "JGB 012100123412345678AB19XY1A 0             .23456", 0, 24, 24, "", 8 },
        /* 15*/ { -1, "JGB 012100123412345678AB19XY1A 0            . 23456", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 866: Invalid character at position 45 in input (alphanumerics and space only in first 45)", 8 },
        /* 16*/ { -1, "JGB  12100123412345678AB19XY1A 0", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 867: Invalid Information Type ID (cannot be space)", 8 },
        /* 17*/ { -1, " 12100123412345678AB19XY1A 0", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 867: Invalid Information Type ID (cannot be space)", 8 },
        /* 18*/ { -1, "JGB 022100123412345678AB19XY1A 0", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 868: Invalid Version ID (\"1\" only)", 8 },
        /* 19*/ { -1, "JGB 01 100123412345678AB19XY1A 0", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 869: Invalid Class (cannot be space)", 8 },
        /* 20*/ { -1, "JGB 012100123A12345678AB19XY1A 0", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 870: Invalid Supply Chain ID (7 digits only)", 8 },
        /* 21*/ { -1, "JGB 01210012341234567AAB19XY1A 0", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 871: Invalid Item ID (8 digits only)", 8 },
        /* 22*/ { -1, "JGB 012100123412345678AB19VY1A 0", 0, 24, 24, "", 8 }, /* Limited ('V') allowed for 2D (ticket #334, props Milton Neal) */
        /* 23*/ { -1, "JGB 012100123412345678AB19XY11 0", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 872: Invalid Destination Post Code plus DPS", 8 },
        /* 24*/ { -1, "JGB 012100123412345678ABC9XY1A 0", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 872: Invalid Destination Post Code plus DPS", 8 },
        /* 25*/ { -1, "JGB 012100123412345678         0AB181XY     ", 0, 24, 24, "", 8 }, /* DPS all spaces */
        /* 25*/ { -1, "JGB 012100123412345678AB1      0AB181XY     ", 0, 24, 24, "", 8 }, /* DPS 'AAN' + spaces */
        /* 26*/ { -1, "JGB 012100123412345678AB       0AB181XY     ", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 872: Invalid Destination Post Code plus DPS", 8 },
        /* 27*/ { -1, "JGB 012100123412345678A1       0AB181XY     ", 0, 24, 24, "", 8 }, /* DPS 'AN' + spaces */
        /* 28*/ { -1, "JGB 012100123412345678A        0AB181XY     ", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 872: Invalid Destination Post Code plus DPS", 8 },
        /* 29*/ { -1, "JGB 012100123412345678A12      0AB181XY     ", 0, 24, 24, "", 8 }, /* DPS 'ANN' + spaces */
        /* 30*/ { -1, "JGB 012100123412345678A123     0AB181XY     ", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 872: Invalid Destination Post Code plus DPS", 8 },
        /* 31*/ { -1, "JGB 012100123412345678AB12     0AB181XY     ", 0, 24, 24, "", 8 }, /* DPS 'AANN' + spaces */
        /* 32*/ { -1, "JGB 012100123412345678AB123    0AB181XY     ", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 872: Invalid Destination Post Code plus DPS", 8 },
        /* 33*/ { -1, "JGB 012100123412345678A1B      0AB181XY     ", 0, 24, 24, "", 8 }, /* DPS 'ANA' + spaces */
        /* 34*/ { -1, "JGB 012100123412345678A1B1     0AB181XY     ", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 872: Invalid Destination Post Code plus DPS", 8 },
        /* 35*/ { -1, "JGB 012100123412345678AB1A     0AB181XY     ", 0, 24, 24, "", 8 }, /* DPS 'AANA' + spaces */
        /* 36*/ { -1, "JGB 012100123412345678AB1A1    0AB181XY     ", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 872: Invalid Destination Post Code plus DPS", 8 },
        /* 37*/ { -1, "JGB 012100123412345678AB19XY1A 7", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 873: Invalid Service Type (\"0\" to \"6\" only)", 8 },
        /* 38*/ { -1, "JGB 012100123412345678AB19XY1A 0AB18XY", 0, 24, 24, "", 8 },
        /* 39*/ { -1, "JGB 012100123412345678AB190XY1A0AB18XY", 0, 24, 24, "", 8 },
        /* 40*/ { -1, "JGB 012100123412345678AB19XY1A 0AB18XI", 0, 24, 24, "", 8 }, /* Limited ('I') allowed for 2D (ticket #334, props Milton Neal) */
        /* 41*/ { -1, "JGB 012100123412345678AB19XY1A 0A18XY", 0, 24, 24, "", 8 },
        /* 42*/ { -1, "JGB 012100123412345678AB19XY1A 0A18XC", 0, 24, 24, "", 8 }, /* Limited ('C') allowed for 2D (ticket #334, props Milton Neal) */
        /* 43*/ { -1, "JGB 012100123412345678AB19XY1A 0AB181XY", 0, 24, 24, "", 8 },
        /* 44*/ { -1, "JGB 012100123412345678AB19XY1A 0AB181VY", 0, 24, 24, "", 8 }, /* Limited ('C') allowed for 2D (ticket #334, props Milton Neal) */
        /* 45*/ { -1, "JGB 012100123412345678AB19XY1A 0ABC81XY", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 874: Invalid Return to Sender Post Code", 8 },
        /* 46*/ { -1, "JGB 012100123412345678AB19XY1A 01B181VY", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 874: Invalid Return to Sender Post Code", 8 },
        /* 47*/ { -1, "JGB 012100123412345678AB19XY1A 0AB1811Y", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 874: Invalid Return to Sender Post Code", 8 },
        /* 48*/ { -1, "JGB 012100123412345678AB19XY1A 0A1AB1XY", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 874: Invalid Return to Sender Post Code", 8 },
        /* 49*/ { -1, "JGB 012100123412345678AB19XY1A 0AB181XY     ", 0, 24, 24, "", 8 },
        /* 50*/ { -1, "JGB 012100123412345678AB19XY1A 0AB181XYA    ", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 875: Invalid Reserved field (must be spaces only)", 8 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK_2D, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n",
                        i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n",
                        i, symbol->width, data[i].expected_width);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d  strcmp(%s, %s) != 0\n",
                    i, symbol->errtxt, data[i].expected_errtxt);
        assert_equal(symbol->option_2, data[i].expected_option_2, "i:%d symbol->option_2 %d != %d\n",
                    i, symbol->option_2, data[i].expected_option_2);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_2d_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_2;
        const char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        const char *comment;
        const char *expected;
    };
    /* Mailmark Mailing Requirements for Letters and Large Letters 14th Nov 2019 (MMRLLL)
       https://www.royalmailtechnical.com/rmt_docs/User_Guides_2020/Mailmark_Letters_and_Large_Letters_20200723.pdf */
    /* Mailmark Barcode Definition Document 15th Feb 2021 (MBDD)
       https://www.royalmailtechnical.com/rmt_docs/User_Guides_2021/Mailmark_Barcode_definition_document_20210215.pdf */
    static const struct item data[] = {
        /*  0*/ { 8, "JGB 01Z999999900000001EC1A1AA1A0SN35TQ       ", 0, 24, 24, 1, "MMRLLL Section 2.4 Code Type Format 7 figure **NOT SAME**, figure switches from C40 to ASC to C40 to ASC contrary to spec",
                    "101010101010101010101010"
                    "110000010100011001100001"
                    "101001011100001011111010"
                    "101010000101100001100001"
                    "101111001100010111100010"
                    "101100100110000100001111"
                    "111010010000101000000100"
                    "110011011000001101111111"
                    "110010010011111011110110"
                    "101011110001001101110101"
                    "101001110010111010110110"
                    "111001111011110011011101"
                    "101000111000000010011010"
                    "111101110011110011010011"
                    "111100101110011011101010"
                    "111110101100111010011011"
                    "100100011000000001011100"
                    "110010100001110110011101"
                    "110110110111111011010000"
                    "100101010001111111010001"
                    "100110100000000011011110"
                    "111011110111100011100101"
                    "100111100000101110001010"
                    "111111111111111111111111"
                },
        /*  1*/ { 10, "JGB 01Z999999900000001EC1A1AA1A0SN35TQ       ", 0, 32, 32, 1, "MMRLLL Section 2.4 Code Type Format 9 figure **NOT SAME**, figure switches as above",
                    "10101010101010101010101010101010"
                    "11000001010001111001100100101111"
                    "10100101110000101011110000000000"
                    "10101000010110011001100111000101"
                    "10111100110001001111101111011110"
                    "10110010011000011100110000100101"
                    "11101001000010101001101010011110"
                    "11001101100000111100101100101111"
                    "11001001001111101110011100110100"
                    "10101111000100111001110000001111"
                    "10100111001001001101001011000110"
                    "11100111101111111011011000100101"
                    "10100011100000101001101010001010"
                    "11110111000101011111000110010001"
                    "11110010100011101101111000110100"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11111010100000011011010000101011"
                    "10010000111100001001001100111010"
                    "11001011110010011011011101101011"
                    "11011111000101101001110001011100"
                    "10010101101110111000011000011001"
                    "10011001100100101100011101011100"
                    "11101010110101111001010110000001"
                    "10011011101011001000101001101000"
                    "10011101010100111110010101001011"
                    "11100011101000101010001110000000"
                    "10011001100101111101001111110001"
                    "11100101110010101100001100100100"
                    "10000111111010111001100111010101"
                    "11001101100011101011101000001000"
                    "11111111111111111111111111111111"
                },
        /*  2*/ { 30, "JGB 01Z999999900000001EC1A1AA1A0SN35TQ       ", 0, 16, 48, 1, "MMRLLL Section 2.4 Code Type Format 29 figure **NOT SAME**, figure switches as above",
                    "101010101010101010101010101010101010101010101010"
                    "110000010100011001101111111101100101100100011101"
                    "101001011100001011100010100100011001100010101100"
                    "101010000101100111000101100111101011001110100011"
                    "101111001100010010000110101000011010011110000110"
                    "101100100110011010111011100100001111010110011111"
                    "111010010000110011100110111001111011111101000100"
                    "110011011001001011111001110110000100111000101111"
                    "110010010011011011010000100011000011000101110100"
                    "101011110010010000101011110001011011000101110111"
                    "101001110000100010011100100101010111111011010110"
                    "111001101001001101000001101011100011010010111001"
                    "101000101100000110011100101011010001101010010110"
                    "111101100001100101011001110001000000010111101101"
                    "101101100011101110101010100001111111001010100010"
                    "111111111111111111111111111111111111111111111111"
                },
        /*  3*/ { 30, "JGB 012100123412345678AB19XY1A 0             REFERENCE 12300AB", 0, 16, 48, 1, "MBDD Table 4 example",
                    "101010101010101010101010101010101010101010101010"
                    "110000010100010110001111111011100000001001111111"
                    "101001011100011001110010100100010100110011111100"
                    "101010000001001001000101100101000011100101110101"
                    "100011010001101010000110110110011101100110110110"
                    "101000011000000011111101110000010100101111011111"
                    "111100000000011111100000111000100011011111100110"
                    "101001101000010000000111110111110111001011110111"
                    "101101010010101010001000100011000101001111101100"
                    "111101101010111000101111110101011100111101000111"
                    "100111110100011000101100100001011000110111100100"
                    "111110100100110100110001111110101000010010001101"
                    "100111001001101101011100111010111100110110101110"
                    "100101101110100011011001111010111000111010100011"
                    "101111100001111111101010100010111101100101001100"
                    "111111111111111111111111111111111111111111111111"
                },
        /*  4*/ { 10, "JGB 012100123412345678AB19XY1A 0             www.xyz.com", 0, 32, 32, 1, "MBDD Table 5 (& BWIPP) example",
                    "10101010101010101010101010101010"
                    "11000001010001011111111110001111"
                    "10100101110001101001111111000000"
                    "10101000000100111001100001000101"
                    "10001101000110101111001011011110"
                    "10100001100000011100001000100101"
                    "11110000000000101001000010011110"
                    "10100110100000111110101100101111"
                    "10110101001111101111011100110100"
                    "11110110100100111101110000001111"
                    "10011111001010001111000111001000"
                    "11111010001101111011010000000101"
                    "11011101110101001001000110101010"
                    "11010001000101011111100001000101"
                    "10111010110111101101010101000000"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11000011011000011010010111011001"
                    "11111111101100101110011110110010"
                    "10010011110010111010110010110001"
                    "10100111000101001001001000011000"
                    "10111011101111111101000001101001"
                    "10101101100000101000111111011000"
                    "10000010111001111001111110111101"
                    "10001011111100101011101101110100"
                    "10011101110010111101110101101011"
                    "11100010111001101010010000101100"
                    "10011000111001011010011101001101"
                    "11101100101110101100110111001000"
                    "10001011110011111001100001110101"
                    "11000011100001101111110101011100"
                    "11111111111111111111111111111111"
                },
        /*  5*/ { 10, "JGB 010100000700009001B707RH1A 0SN35XX       ABCDEFGHIJ1234567890ABCDEFGHIJ1234567890A", 0, 32, 32, 1, "TEC-IT example, same",
                    "10101010101010101010101010101010"
                    "11000001010001111000100110110001"
                    "10100101110000101010110010100000"
                    "10101000010110011000011000100101"
                    "10001100111000101111101000001100"
                    "10011100011000011100000110100101"
                    "11001111001000101001000011011110"
                    "10100101011000111101111100101111"
                    "10101111010111101011101000110100"
                    "11001001100100011101000000010111"
                    "11001000001010001001111111010110"
                    "11010000001100111100101111011101"
                    "11001000010011101100011100100010"
                    "11010111001011011100001110100111"
                    "10111101001010001100100000100000"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11011101001111111001100001001001"
                    "11110001010111001111011010111010"
                    "11001000011110111111000101111001"
                    "11010100000101101111101010001000"
                    "10010001000010011000000110011001"
                    "10011010001001101001100011100100"
                    "11010101100001011011100101000101"
                    "11001001100110101101010011000100"
                    "10101101001010111001100010011101"
                    "11111010111000101001011000000000"
                    "10101101110110111010110011100001"
                    "10100011100100101000101111010110"
                    "10100001111001011010010001100011"
                    "10111101100000001010110001010110"
                    "11111111111111111111111111111111"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[8192];
    char cmp_buf[32768];
    char cmp_msg[1024];

    /* Only do BWIPP/ZXing-C++ tests if asked, too slow otherwise */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript();
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder();

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK_2D, -1 /*input_mode*/, -1 /*eci*/,
                                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %d, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n",
                            i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n",
                            i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n",
                            i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) {
                            printf("i:%d %s not BWIPP compatible (%s)\n",
                                    i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                        }
                    } else {
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf,
                                    sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n",
                                    i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf,
                                       data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[144 * 144 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1,
                                "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, 1 /*zxingcpp_cmp*/, cmp_buf,
                                sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n",
                                i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length,
                                NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len,
                                escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_2d_rt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int output_options;
        const char *data;
        int length;
        int ret;
        int expected_eci;
        const char *expected;
        int expected_length;
        int expected_content_eci;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "jgb 012100123412345678ab19xy1a 0", -1, 0, 0, "", -1, 0 },
        /*  1*/ { UNICODE_MODE, BARCODE_CONTENT_SEGS, "jgb 012100123412345678ab19xy1a 0", -1, 0, 0, "JGB 012100123412345678AB19XY1A 0             ", -1, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MAILMARK_2D, data[i].input_mode, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                                    data[i].data, data[i].length, debug);
        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n",
                        i, symbol->eci, data[i].expected_eci);
            if (symbol->output_options & BARCODE_CONTENT_SEGS) {
                assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
                assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
                assert_equal(symbol->content_segs[0].length, expected_length,
                            "i:%d content_segs[0].length %d != expected_length %d\n",
                            i, symbol->content_segs[0].length, expected_length);
                assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected, expected_length),
                            "i:%d content_segs[0].source memcmp(%s, %s, %d) != 0\n", i,
                            testUtilEscape((const char *) symbol->content_segs[0].source, symbol->content_segs[0].length,
                                            escaped, sizeof(escaped)),
                            testUtilEscape(data[i].expected, expected_length, escaped2, sizeof(escaped2)),
                            expected_length);
                assert_equal(symbol->content_segs[0].eci, data[i].expected_content_eci,
                            "i:%d content_segs[0].eci %d != expected_content_eci %d\n",
                            i, symbol->content_segs[0].eci, data[i].expected_content_eci);
            } else {
                assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_4s_hrt", test_4s_hrt },
        { "test_4s_input", test_4s_input },
        { "test_4s_encode_vector", test_4s_encode_vector },
        { "test_4s_encode", test_4s_encode },
        { "test_2d_input", test_2d_input },
        { "test_2d_encode", test_2d_encode },
        { "test_2d_rt", test_2d_rt },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
