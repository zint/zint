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

static int is_row_column_black(struct zint_symbol *symbol, int row, int column) {
    int i;
    if (symbol->output_options & OUT_BUFFER_INTERMEDIATE) {
        i = row * symbol->bitmap_width + column;
        return symbol->bitmap[i] == '1'; /* Black */
    }
    i = (row * symbol->bitmap_width + column) * 3;
    return symbol->bitmap[i] == 0 && symbol->bitmap[i + 1] == 0 && symbol->bitmap[i + 2] == 0; /* Black */
}

static void test_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        const char *fgcolour;
        const char *bgcolour;
        int rotate_angle;
        const char *data;
        int ret;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123456", "7890AB", 0, "A", 0, 1, 46, 92, 116 },
        /*  1*/ { BARCODE_CODE128, "123456", "7890ab", 90, "A", 0, 1, 46, 116, 92 },
        /*  2*/ { BARCODE_CODE128, NULL, NULL, 180, "A", 0, 1, 46, 92, 116 },
        /*  3*/ { BARCODE_CODE128, NULL, NULL, 270, "A", 0, 1, 46, 116, 92 },
        /*  4*/ { BARCODE_CODE128, NULL, NULL, 181, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  5*/ { BARCODE_CODE128, "12345", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  6*/ { BARCODE_CODE128, NULL, "1234567", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  7*/ { BARCODE_CODE128, "12345 ", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  8*/ { BARCODE_CODE128, NULL, "EEFFGG", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE128, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        if (data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) data[i].data, length, data[i].rotate_angle);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d symbol->bitmap_width %d != %d\n", i, symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d symbol->bitmap_height %d != %d\n", i, symbol->bitmap_height, data[i].expected_bitmap_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_buffer(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        const char *data;
        const char *composite;
        int ret;
        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "1234567890", "", 0, 50, 1, 108, 216, 116 },
        /*  1*/ { BARCODE_CODE11, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 108, 216, 116 },
        /*  2*/ { BARCODE_C25STANDARD, -1, "1234567890", "", 0, 50, 1, 117, 234, 116 },
        /*  3*/ { BARCODE_C25STANDARD, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 117, 234, 116 },
        /*  4*/ { BARCODE_C25INTER, -1, "1234567890", "", 0, 50, 1, 99, 198, 116 },
        /*  5*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 99, 198, 116 },
        /*  6*/ { BARCODE_C25IATA, -1, "1234567890", "", 0, 50, 1, 149, 298, 116 },
        /*  7*/ { BARCODE_C25IATA, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 149, 298, 116 },
        /*  8*/ { BARCODE_C25LOGIC, -1, "1234567890", "", 0, 50, 1, 109, 218, 116 },
        /*  9*/ { BARCODE_C25LOGIC, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 109, 218, 116 },
        /* 10*/ { BARCODE_C25IND, -1, "1234567890", "", 0, 50, 1, 159, 318, 116 },
        /* 11*/ { BARCODE_C25IND, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 159, 318, 116 },
        /* 12*/ { BARCODE_CODE39, -1, "1234567890", "", 0, 50, 1, 155, 310, 116 },
        /* 13*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 155, 310, 116 },
        /* 14*/ { BARCODE_EXCODE39, -1, "1234567890", "", 0, 50, 1, 155, 310, 116 },
        /* 15*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 155, 310, 116 },
        /* 16*/ { BARCODE_EAN13, -1, "123456789012", "", 0, 50, 1, 95, 226, 116 },
        /* 17*/ { BARCODE_EANX, -1, "123456789012", "", 0, 50, 1, 95, 226, 116 },
        /* 18*/ { BARCODE_EAN13, COMPLIANT_HEIGHT, "123456789012", "", 0, 69, 1, 95, 226, 154 },
        /* 19*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "123456789012", "", 0, 69, 1, 95, 226, 154 },
        /* 20*/ { BARCODE_EANX_CHK, -1, "1234567890128", "", 0, 50, 1, 95, 226, 116 },
        /* 21*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "1234567890128", "", 0, 69, 1, 95, 226, 154 },
        /* 22*/ { BARCODE_EAN13, -1, "123456789012+12", "", 0, 50, 1, 122, 276, 116 },
        /* 23*/ { BARCODE_EANX, -1, "123456789012+12", "", 0, 50, 1, 122, 276, 116 },
        /* 24*/ { BARCODE_EAN13, COMPLIANT_HEIGHT, "123456789012+12", "", 0, 69, 1, 122, 276, 154 },
        /* 25*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "123456789012+12", "", 0, 69, 1, 122, 276, 154 },
        /* 26*/ { BARCODE_EANX_CHK, -1, "1234567890128+12", "", 0, 50, 1, 122, 276, 116 },
        /* 27*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "1234567890128+12", "", 0, 69, 1, 122, 276, 154 },
        /* 28*/ { BARCODE_EAN13, -1, "123456789012+12345", "", 0, 50, 1, 149, 330, 116 },
        /* 29*/ { BARCODE_EANX, -1, "123456789012+12345", "", 0, 50, 1, 149, 330, 116 },
        /* 30*/ { BARCODE_EAN13, COMPLIANT_HEIGHT, "123456789012+12345", "", 0, 69, 1, 149, 330, 154 },
        /* 31*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "123456789012+12345", "", 0, 69, 1, 149, 330, 154 },
        /* 32*/ { BARCODE_EANX_CHK, -1, "1234567890128+12345", "", 0, 50, 1, 149, 330, 116 },
        /* 33*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "1234567890128+12345", "", 0, 69, 1, 149, 330, 154 },
        /* 34*/ { BARCODE_EAN8, -1, "1234567", "", 0, 50, 1, 67, 162, 116 },
        /* 35*/ { BARCODE_EANX, -1, "1234567", "", 0, 50, 1, 67, 162, 116 },
        /* 36*/ { BARCODE_EAN8, COMPLIANT_HEIGHT, "1234567", "", 0, 55, 1, 67, 162, 126 },
        /* 37*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "1234567", "", 0, 55, 1, 67, 162, 126 },
        /* 38*/ { BARCODE_EANX_CHK, -1, "12345670", "", 0, 50, 1, 67, 162, 116 },
        /* 39*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "12345670", "", 0, 55, 1, 67, 162, 126 },
        /* 40*/ { BARCODE_EAN8, -1, "1234567+12", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116 },
        /* 41*/ { BARCODE_EANX, -1, "1234567+12", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116 },
        /* 42*/ { BARCODE_EAN8, COMPLIANT_HEIGHT, "1234567+12", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 94, 212, 126 },
        /* 43*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "1234567+12", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 94, 212, 126 },
        /* 44*/ { BARCODE_EANX_CHK, -1, "12345670+12", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116 },
        /* 45*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "12345670+12", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 94, 212, 126 },
        /* 46*/ { BARCODE_EAN8, -1, "1234567+12345", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116 },
        /* 47*/ { BARCODE_EANX, -1, "1234567+12345", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116 },
        /* 48*/ { BARCODE_EAN8, COMPLIANT_HEIGHT, "1234567+12345", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 121, 266, 126 },
        /* 49*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "1234567+12345", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 121, 266, 126 },
        /* 50*/ { BARCODE_EANX_CHK, -1, "12345670+12345", "", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116 },
        /* 51*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "12345670+12345", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 121, 266, 126 },
        /* 52*/ { BARCODE_EAN_5ADDON, -1, "1234", "", 0, 50, 1, 47, 104, 116 },
        /* 53*/ { BARCODE_EANX, -1, "1234", "", 0, 50, 1, 47, 104, 116 },
        /* 54*/ { BARCODE_EAN_5ADDON, COMPLIANT_HEIGHT, "1234", "", 0, 66.5, 1, 47, 104, 149 },
        /* 55*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "1234", "", 0, 66.5, 1, 47, 104, 149 },
        /* 56*/ { BARCODE_EANX_CHK, -1, "1234", "", 0, 50, 1, 47, 104, 116 },
        /* 57*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "1234", "", 0, 66.5, 1, 47, 104, 149 },
        /* 58*/ { BARCODE_EAN_2ADDON, -1, "12", "", 0, 50, 1, 20, 50, 116 },
        /* 59*/ { BARCODE_EANX, -1, "12", "", 0, 50, 1, 20, 50, 116 },
        /* 60*/ { BARCODE_EAN_2ADDON, COMPLIANT_HEIGHT, "12", "", 0, 66.5, 1, 20, 50, 149 },
        /* 61*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "12", "", 0, 66.5, 1, 20, 50, 149 },
        /* 62*/ { BARCODE_EANX_CHK, -1, "12", "", 0, 50, 1, 20, 50, 116 },
        /* 63*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "12", "", 0, 66.5, 1, 20, 50, 149 },
        /* 64*/ { BARCODE_GS1_128, -1, "[01]12345678901231", "", 0, 50, 1, 134, 268, 116 },
        /* 65*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, "[01]12345678901231", "", 0, 64, 1, 134, 268, 144 },
        /* 66*/ { BARCODE_CODABAR, -1, "A00000000B", "", 0, 50, 1, 102, 204, 116 },
        /* 67*/ { BARCODE_CODABAR, COMPLIANT_HEIGHT, "A00000000B", "", 0, 50, 1, 102, 204, 116 },
        /* 68*/ { BARCODE_CODE128, -1, "1234567890", "", 0, 50, 1, 90, 180, 116 },
        /* 69*/ { BARCODE_CODE128, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 90, 180, 116 },
        /* 70*/ { BARCODE_DPLEIT, -1, "1234567890123", "", 0, 72, 1, 135, 270, 160 },
        /* 71*/ { BARCODE_DPLEIT, COMPLIANT_HEIGHT, "1234567890123", "", 0, 72, 1, 135, 270, 160 },
        /* 72*/ { BARCODE_DPIDENT, -1, "12345678901", "", 0, 72, 1, 117, 234, 160 },
        /* 73*/ { BARCODE_DPIDENT, COMPLIANT_HEIGHT, "12345678901", "", 0, 72, 1, 117, 234, 160 },
        /* 74*/ { BARCODE_CODE16K, -1, "1234567890", "", 0, 20, 2, 70, 162, 44 },
        /* 75*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, "1234567890", "", 0, 21, 2, 70, 162, 46 },
        /* 76*/ { BARCODE_CODE49, -1, "1234567890", "", 0, 20, 2, 70, 162, 44 },
        /* 77*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, "1234567890", "", 0, 21, 2, 70, 162, 46 },
        /* 78*/ { BARCODE_CODE93, -1, "1234567890", "", 0, 50, 1, 127, 254, 116 },
        /* 79*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, "1234567890", "", 0, 40, 1, 127, 254, 96 },
        /* 80*/ { BARCODE_FLAT, -1, "1234567890", "", 0, 50, 1, 90, 180, 100 },
        /* 81*/ { BARCODE_FLAT, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 90, 180, 100 },
        /* 82*/ { BARCODE_DBAR_OMN, -1, "1234567890123", "", 0, 50, 1, 96, 192, 116 },
        /* 83*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, "1234567890123", "", 0, 33, 1, 96, 192, 82 },
        /* 84*/ { BARCODE_DBAR_LTD, -1, "1234567890123", "", 0, 50, 1, 79, 158, 116 },
        /* 85*/ { BARCODE_DBAR_LTD, COMPLIANT_HEIGHT, "1234567890123", "", 0, 10, 1, 79, 158, 36 },
        /* 86*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231", "", 0, 34, 1, 134, 268, 84 },
        /* 87*/ { BARCODE_DBAR_EXP, COMPLIANT_HEIGHT, "[01]12345678901231", "", 0, 34, 1, 134, 268, 84 },
        /* 88*/ { BARCODE_TELEPEN, -1, "1234567890", "", 0, 50, 1, 208, 416, 116 },
        /* 89*/ { BARCODE_TELEPEN, COMPLIANT_HEIGHT, "1234567890", "", 0, 32, 1, 208, 416, 80 },
        /* 90*/ { BARCODE_UPCA, -1, "12345678901", "", 0, 50, 1, 95, 226, 116 },
        /* 91*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, "12345678901", "", 0, 69, 1, 95, 226, 154 },
        /* 92*/ { BARCODE_UPCA_CHK, -1, "123456789012", "", 0, 50, 1, 95, 226, 116 },
        /* 93*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, "123456789012", "", 0, 69, 1, 95, 226, 154 },
        /* 94*/ { BARCODE_UPCA, -1, "12345678901+12", "", 0, 50, 1, 124, 276, 116 },
        /* 95*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, "12345678901+12", "", 0, 69, 1, 124, 276, 154 },
        /* 96*/ { BARCODE_UPCA_CHK, -1, "123456789012+12", "", 0, 50, 1, 124, 276, 116 },
        /* 97*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, "123456789012+12", "", 0, 69, 1, 124, 276, 154 },
        /* 98*/ { BARCODE_UPCA, -1, "12345678901+12345", "", 0, 50, 1, 151, 330, 116 },
        /* 99*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, "12345678901+12345", "", 0, 69, 1, 151, 330, 154 },
        /*100*/ { BARCODE_UPCA_CHK, -1, "123456789012+12345", "", 0, 50, 1, 151, 330, 116 },
        /*101*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, "123456789012+12345", "", 0, 69, 1, 151, 330, 154 },
        /*102*/ { BARCODE_UPCE, -1, "1234567", "", 0, 50, 1, 51, 134, 116 },
        /*103*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, "1234567", "", 0, 69, 1, 51, 134, 154 },
        /*104*/ { BARCODE_UPCE_CHK, -1, "12345670", "", 0, 50, 1, 51, 134, 116 },
        /*105*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, "12345670", "", 0, 69, 1, 51, 134, 154 },
        /*106*/ { BARCODE_UPCE, -1, "1234567+12", "", 0, 50, 1, 78, 184, 116 },
        /*107*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, "1234567+12", "", 0, 69, 1, 78, 184, 154 },
        /*108*/ { BARCODE_UPCE_CHK, -1, "12345670+12", "", 0, 50, 1, 78, 184, 116 },
        /*109*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, "12345670+12", "", 0, 69, 1, 78, 184, 154 },
        /*110*/ { BARCODE_UPCE, -1, "1234567+12345", "", 0, 50, 1, 105, 238, 116 },
        /*111*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, "1234567+12345", "", 0, 69, 1, 105, 238, 154 },
        /*112*/ { BARCODE_UPCE_CHK, -1, "12345670+12345", "", 0, 50, 1, 105, 238, 116 },
        /*113*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, "12345670+12345", "", 0, 69, 1, 105, 238, 154 },
        /*114*/ { BARCODE_POSTNET, -1, "12345678901", "", 0, 12, 2, 123, 246, 24 },
        /*115*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, "12345678901", "", 0, 5, 2, 123, 246, 10 },
        /*116*/ { BARCODE_MSI_PLESSEY, -1, "1234567890", "", 0, 50, 1, 127, 254, 116 },
        /*117*/ { BARCODE_MSI_PLESSEY, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 127, 254, 116 },
        /*118*/ { BARCODE_FIM, -1, "A", "", 0, 50, 1, 17, 34, 100 },
        /*119*/ { BARCODE_FIM, COMPLIANT_HEIGHT, "A", "", 0, 20, 1, 17, 34, 40 },
        /*120*/ { BARCODE_LOGMARS, -1, "1234567890", "", 0, 50, 1, 191, 382, 116 },
        /*121*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, "1234567890", "", 0, 45.5, 1, 191, 382, 107 },
        /*122*/ { BARCODE_PHARMA, -1, "123456", "", 0, 50, 1, 58, 116, 100 },
        /*123*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, "123456", "", 0, 16, 1, 58, 116, 32 },
        /*124*/ { BARCODE_PZN, -1, "123456", "", 0, 50, 1, 142, 284, 116 },
        /*125*/ { BARCODE_PZN, COMPLIANT_HEIGHT, "123456", "", 0, 40, 1, 142, 284, 96 },
        /*126*/ { BARCODE_PHARMA_TWO, -1, "12345678", "", 0, 10, 2, 29, 58, 20 },
        /*127*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, "12345678", "", 0, 8, 2, 29, 58, 16 },
        /*128*/ { BARCODE_CEPNET, -1, "12345678", "", 0, 5, 2, 93, 186, 10 },
        /*129*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, "12345678", "", 0, 5, 2, 93, 186, 10 },
        /*130*/ { BARCODE_PDF417, -1, "1234567890", "", 0, 21, 7, 103, 206, 42 },
        /*131*/ { BARCODE_PDF417, COMPLIANT_HEIGHT, "1234567890", "", 0, 21, 7, 103, 206, 42 },
        /*132*/ { BARCODE_PDF417COMP, -1, "1234567890", "", 0, 21, 7, 69, 138, 42 },
        /*133*/ { BARCODE_PDF417COMP, COMPLIANT_HEIGHT, "1234567890", "", 0, 21, 7, 69, 138, 42 },
        /*134*/ { BARCODE_MAXICODE, -1, "1234567890", "", 0, 165, 33, 30, 299, 298 },
        /*135*/ { BARCODE_MAXICODE, COMPLIANT_HEIGHT, "1234567890", "", 0, 165, 33, 30, 299, 298 },
        /*136*/ { BARCODE_QRCODE, -1, "1234567890AB", "", 0, 21, 21, 21, 42, 42 },
        /*137*/ { BARCODE_QRCODE, COMPLIANT_HEIGHT, "1234567890AB", "", 0, 21, 21, 21, 42, 42 },
        /*138*/ { BARCODE_CODE128AB, -1, "1234567890", "", 0, 50, 1, 145, 290, 116 },
        /*139*/ { BARCODE_CODE128AB, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 145, 290, 116 },
        /*140*/ { BARCODE_AUSPOST, -1, "12345678901234567890123", "", 0, 8, 3, 133, 266, 16 },
        /*141*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, "12345678901234567890123", "", 0, 9.5, 3, 133, 266, 19 },
        /*142*/ { BARCODE_AUSREPLY, -1, "12345678", "", 0, 8, 3, 73, 146, 16 },
        /*143*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, "12345678", "", 0, 9.5, 3, 73, 146, 19 },
        /*144*/ { BARCODE_AUSROUTE, -1, "12345678", "", 0, 8, 3, 73, 146, 16 },
        /*145*/ { BARCODE_AUSROUTE, COMPLIANT_HEIGHT, "12345678", "", 0, 9.5, 3, 73, 146, 19 },
        /*146*/ { BARCODE_AUSREDIRECT, -1, "12345678", "", 0, 8, 3, 73, 146, 16 },
        /*147*/ { BARCODE_AUSREDIRECT, COMPLIANT_HEIGHT, "12345678", "", 0, 9.5, 3, 73, 146, 19 },
        /*148*/ { BARCODE_ISBNX, -1, "123456789", "", 0, 50, 1, 95, 226, 116 },
        /*149*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, "123456789", "", 0, 69, 1, 95, 226, 154 },
        /*150*/ { BARCODE_ISBNX, -1, "123456789+12", "", 0, 50, 1, 122, 276, 116 },
        /*151*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, "123456789+12", "", 0, 69, 1, 122, 276, 154 },
        /*152*/ { BARCODE_ISBNX, -1, "123456789+12345", "", 0, 50, 1, 149, 330, 116 },
        /*153*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, "123456789+12345", "", 0, 69, 1, 149, 330, 154 },
        /*154*/ { BARCODE_RM4SCC, -1, "1234567890", "", 0, 8, 3, 91, 182, 16 },
        /*155*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, "1234567890", "", 0, 8, 3, 91, 182, 16 },
        /*156*/ { BARCODE_DATAMATRIX, -1, "ABC", "", 0, 10, 10, 10, 20, 20 },
        /*157*/ { BARCODE_DATAMATRIX, COMPLIANT_HEIGHT, "ABC", "", 0, 10, 10, 10, 20, 20 },
        /*158*/ { BARCODE_EAN14, -1, "1234567890123", "", 0, 50, 1, 134, 268, 116 },
        /*159*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, "1234567890123", "", 0, 64, 1, 134, 268, 144 },
        /*160*/ { BARCODE_VIN, -1, "12345678701234567", "", 0, 50, 1, 246, 492, 116 },
        /*161*/ { BARCODE_VIN, COMPLIANT_HEIGHT, "12345678701234567", "", 0, 50, 1, 246, 492, 116 },
        /*162*/ { BARCODE_CODABLOCKF, -1, "1234567890", "", 0, 20, 2, 101, 242, 44 },
        /*163*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, "1234567890", "", 0, 20, 2, 101, 242, 44 },
        /*164*/ { BARCODE_NVE18, -1, "12345678901234567", "", 0, 50, 1, 156, 312, 116 },
        /*165*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, "12345678901234567", "", 0, 64, 1, 156, 312, 144 },
        /*166*/ { BARCODE_JAPANPOST, -1, "1234567890", "", 0, 8, 3, 133, 266, 16 },
        /*167*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, "1234567890", "", 0, 6, 3, 133, 266, 12 },
        /*168*/ { BARCODE_KOREAPOST, -1, "123456", "", 0, 50, 1, 167, 334, 116 },
        /*169*/ { BARCODE_KOREAPOST, COMPLIANT_HEIGHT, "123456", "", 0, 50, 1, 167, 334, 116 },
        /*170*/ { BARCODE_DBAR_STK, -1, "1234567890123", "", 0, 13, 3, 50, 100, 26 },
        /*171*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, "1234567890123", "", 0, 13, 3, 50, 100, 26 },
        /*172*/ { BARCODE_DBAR_OMNSTK, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138 },
        /*173*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, "1234567890123", "", 0, 69, 5, 50, 100, 138 },
        /*174*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142 },
        /*175*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142 },
        /*176*/ { BARCODE_PLANET, -1, "12345678901", "", 0, 12, 2, 123, 246, 24 },
        /*177*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, "12345678901", "", 0, 5, 2, 123, 246, 10 },
        /*178*/ { BARCODE_MICROPDF417, -1, "1234567890", "", 0, 12, 6, 82, 164, 24 },
        /*179*/ { BARCODE_MICROPDF417, COMPLIANT_HEIGHT, "1234567890", "", 0, 12, 6, 82, 164, 24 },
        /*180*/ { BARCODE_USPS_IMAIL, -1, "12345678901234567890", "", 0, 8, 3, 129, 258, 16 },
        /*181*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, "12345678901234567890", "", 0, 6, 3, 129, 258, 12 },
        /*182*/ { BARCODE_PLESSEY, -1, "1234567890", "", 0, 50, 1, 227, 454, 116 },
        /*183*/ { BARCODE_PLESSEY, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 227, 454, 116 },
        /*184*/ { BARCODE_TELEPEN_NUM, -1, "1234567890", "", 0, 50, 1, 128, 256, 116 },
        /*185*/ { BARCODE_TELEPEN_NUM, COMPLIANT_HEIGHT, "1234567890", "", 0, 32, 1, 128, 256, 80 },
        /*186*/ { BARCODE_ITF14, -1, "1234567890", "", 0, 50, 1, 135, 330, 136 },
        /*187*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, "1234567890", "", 0, 64, 1, 135, 330, 164 },
        /*188*/ { BARCODE_KIX, -1, "123456ABCDE", "", 0, 8, 3, 87, 174, 16 },
        /*189*/ { BARCODE_KIX, COMPLIANT_HEIGHT, "123456ABCDE", "", 0, 8, 3, 87, 174, 16 },
        /*190*/ { BARCODE_AZTEC, -1, "1234567890AB", "", 0, 15, 15, 15, 30, 30 },
        /*191*/ { BARCODE_AZTEC, COMPLIANT_HEIGHT, "1234567890AB", "", 0, 15, 15, 15, 30, 30 },
        /*192*/ { BARCODE_DAFT, -1, "DAFTDAFTDAFTDAFT", "", 0, 8, 3, 31, 62, 16 },
        /*193*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, "DAFTDAFTDAFTDAFT", "", 0, 8, 3, 31, 62, 16 },
        /*194*/ { BARCODE_DPD, -1, "0123456789012345678901234567", "", 0, 50, 1, 189, 378, 122 },
        /*195*/ { BARCODE_DPD, COMPLIANT_HEIGHT, "0123456789012345678901234567", "", 0, 66.5, 1, 189, 378, 155 },
        /*196*/ { BARCODE_MICROQR, -1, "12345", "", 0, 11, 11, 11, 22, 22 },
        /*197*/ { BARCODE_MICROQR, COMPLIANT_HEIGHT, "12345", "", 0, 11, 11, 11, 22, 22 },
        /*198*/ { BARCODE_HIBC_128, -1, "1234567890", "", 0, 50, 1, 123, 246, 116 },
        /*199*/ { BARCODE_HIBC_128, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 123, 246, 116 },
        /*200*/ { BARCODE_HIBC_39, -1, "1234567890", "", 0, 50, 1, 223, 446, 116 },
        /*201*/ { BARCODE_HIBC_39, COMPLIANT_HEIGHT, "1234567890", "", 0, 50, 1, 223, 446, 116 },
        /*202*/ { BARCODE_HIBC_DM, -1, "ABC", "", 0, 12, 12, 12, 24, 24 },
        /*203*/ { BARCODE_HIBC_DM, COMPLIANT_HEIGHT, "ABC", "", 0, 12, 12, 12, 24, 24 },
        /*204*/ { BARCODE_HIBC_QR, -1, "1234567890AB", "", 0, 21, 21, 21, 42, 42 },
        /*205*/ { BARCODE_HIBC_QR, COMPLIANT_HEIGHT, "1234567890AB", "", 0, 21, 21, 21, 42, 42 },
        /*206*/ { BARCODE_HIBC_PDF, -1, "1234567890", "", 0, 24, 8, 103, 206, 48 },
        /*207*/ { BARCODE_HIBC_PDF, COMPLIANT_HEIGHT, "1234567890", "", 0, 24, 8, 103, 206, 48 },
        /*208*/ { BARCODE_HIBC_MICPDF, -1, "1234567890", "", 0, 28, 14, 38, 76, 56 },
        /*209*/ { BARCODE_HIBC_MICPDF, COMPLIANT_HEIGHT, "1234567890", "", 0, 28, 14, 38, 76, 56 },
        /*210*/ { BARCODE_HIBC_BLOCKF, -1, "1234567890", "", 0, 30, 3, 101, 242, 64 },
        /*211*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, "1234567890", "", 0, 30, 3, 101, 242, 64 },
        /*212*/ { BARCODE_HIBC_AZTEC, -1, "1234567890AB", "", 0, 19, 19, 19, 38, 38 },
        /*213*/ { BARCODE_HIBC_AZTEC, COMPLIANT_HEIGHT, "1234567890AB", "", 0, 19, 19, 19, 38, 38 },
        /*214*/ { BARCODE_DOTCODE, -1, "ABC", "", 0, 11, 11, 16, 33, 23 },
        /*215*/ { BARCODE_DOTCODE, COMPLIANT_HEIGHT, "ABC", "", 0, 11, 11, 16, 33, 23 },
        /*216*/ { BARCODE_HANXIN, -1, "1234567890AB", "", 0, 23, 23, 23, 46, 46 },
        /*217*/ { BARCODE_HANXIN, COMPLIANT_HEIGHT, "1234567890AB", "", 0, 23, 23, 23, 46, 46 },
        /*218*/ { BARCODE_MAILMARK_2D, -1, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 48, 48 },
        /*219*/ { BARCODE_MAILMARK_2D, COMPLIANT_HEIGHT, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 48, 48 },
        /*220*/ { BARCODE_UPU_S10, -1, "EE876543216CA", "", 0, 50, 1, 156, 312, 116 },
        /*221*/ { BARCODE_UPU_S10, COMPLIANT_HEIGHT, "EE876543216CA", "", 0, 50, 1, 156, 312, 116 },
        /*222*/ { BARCODE_MAILMARK_4S, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20 },
        /*223*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, "01000000000000000AA00AA0A", "", 0, 8, 3, 155, 310, 16 },
        /*224*/ { BARCODE_AZRUNE, -1, "255", "", 0, 11, 11, 11, 22, 22 },
        /*225*/ { BARCODE_AZRUNE, COMPLIANT_HEIGHT, "255", "", 0, 11, 11, 11, 22, 22 },
        /*226*/ { BARCODE_CODE32, -1, "12345678", "", 0, 50, 1, 103, 206, 116 },
        /*227*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, "12345678", "", 0, 20, 1, 103, 206, 56 },
        /*228*/ { BARCODE_EAN13_CC, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 116 },
        /*229*/ { BARCODE_EANX_CC, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 116 },
        /*230*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, "123456789012", "[20]01", 0, 81, 7, 99, 226, 178 },
        /*231*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "123456789012", "[20]01", 0, 81, 7, 99, 226, 178 },
        /*232*/ { BARCODE_EAN13_CC, -1, "123456789012+12", "[20]01", 0, 50, 7, 125, 276, 116 },
        /*233*/ { BARCODE_EANX_CC, -1, "123456789012+12", "[20]01", 0, 50, 7, 125, 276, 116 },
        /*234*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, "123456789012+12", "[20]01", 0, 81, 7, 125, 276, 178 },
        /*235*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "123456789012+12", "[20]01", 0, 81, 7, 125, 276, 178 },
        /*236*/ { BARCODE_EAN13_CC, -1, "123456789012+12345", "[20]01", 0, 50, 7, 152, 330, 116 },
        /*237*/ { BARCODE_EANX_CC, -1, "123456789012+12345", "[20]01", 0, 50, 7, 152, 330, 116 },
        /*238*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, "123456789012+12345", "[20]01", 0, 81, 7, 152, 330, 178 },
        /*239*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "123456789012+12345", "[20]01", 0, 81, 7, 152, 330, 178 },
        /*240*/ { BARCODE_EAN8_CC, -1, "1234567", "[20]01", 0, 50, 8, 72, 162, 116 },
        /*241*/ { BARCODE_EANX_CC, -1, "1234567", "[20]01", 0, 50, 8, 72, 162, 116 },
        /*242*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, "1234567", "[20]01", 0, 69, 8, 72, 162, 154 },
        /*243*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "1234567", "[20]01", 0, 69, 8, 72, 162, 154 },
        /*244*/ { BARCODE_EAN8_CC, -1, "1234567+12", "[20]01", ZINT_WARN_NONCOMPLIANT, 50, 8, 98, 212, 116 },
        /*245*/ { BARCODE_EANX_CC, -1, "1234567+12", "[20]01", ZINT_WARN_NONCOMPLIANT, 50, 8, 98, 212, 116 },
        /*246*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, "1234567+12", "[20]01", ZINT_WARN_NONCOMPLIANT, 69, 8, 98, 212, 154 },
        /*247*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "1234567+12", "[20]01", ZINT_WARN_NONCOMPLIANT, 69, 8, 98, 212, 154 },
        /*248*/ { BARCODE_EAN8_CC, -1, "1234567+12345", "[20]01", ZINT_WARN_NONCOMPLIANT, 50, 8, 125, 266, 116 },
        /*249*/ { BARCODE_EANX_CC, -1, "1234567+12345", "[20]01", ZINT_WARN_NONCOMPLIANT, 50, 8, 125, 266, 116 },
        /*250*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, "1234567+12345", "[20]01", ZINT_WARN_NONCOMPLIANT, 69, 8, 125, 266, 154 },
        /*251*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "1234567+12345", "[20]01", ZINT_WARN_NONCOMPLIANT, 69, 8, 125, 266, 154 },
        /*252*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]01", 0, 50, 5, 145, 290, 116 },
        /*253*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, "[01]12345678901231", "[20]01", 0, 71, 5, 145, 290, 158 },
        /*254*/ { BARCODE_DBAR_OMN_CC, -1, "1234567890123", "[20]01", 0, 21, 5, 100, 200, 58 },
        /*255*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, "1234567890123", "[20]01", 0, 40, 5, 100, 200, 96 },
        /*256*/ { BARCODE_DBAR_LTD_CC, -1, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 54 },
        /*257*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 54 },
        /*258*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 98 },
        /*259*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 98 },
        /*260*/ { BARCODE_UPCA_CC, -1, "12345678901", "[20]01", 0, 50, 7, 99, 226, 116 },
        /*261*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, "12345678901", "[20]01", 0, 81, 7, 99, 226, 178 },
        /*262*/ { BARCODE_UPCA_CC, -1, "12345678901+12", "[20]01", 0, 50, 7, 127, 276, 116 },
        /*263*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, "12345678901+12", "[20]01", 0, 81, 7, 127, 276, 178 },
        /*264*/ { BARCODE_UPCA_CC, -1, "12345678901+12345", "[20]01", 0, 50, 7, 154, 330, 116 },
        /*265*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, "12345678901+12345", "[20]01", 0, 81, 7, 154, 330, 178 },
        /*266*/ { BARCODE_UPCE_CC, -1, "1234567", "[20]01", 0, 50, 9, 55, 134, 116 },
        /*267*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, "1234567", "[20]01", 0, 85, 9, 55, 134, 186 },
        /*268*/ { BARCODE_UPCE_CC, -1, "1234567+12", "[20]01", 0, 50, 9, 81, 184, 116 },
        /*269*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, "1234567+12", "[20]01", 0, 85, 9, 81, 184, 186 },
        /*270*/ { BARCODE_UPCE_CC, -1, "1234567+12345", "[20]01", 0, 50, 9, 108, 238, 116 },
        /*271*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, "1234567+12345", "[20]01", 0, 85, 9, 108, 238, 186 },
        /*272*/ { BARCODE_DBAR_STK_CC, -1, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48 },
        /*273*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48 },
        /*274*/ { BARCODE_DBAR_OMNSTK_CC, -1, "1234567890123", "[20]01", 0, 80, 11, 56, 112, 160 },
        /*275*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, "1234567890123", "[20]01", 0, 80, 11, 56, 112, 160 },
        /*276*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156 },
        /*277*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156 },
        /*278*/ { BARCODE_CHANNEL, -1, "01", "", 0, 50, 1, 19, 38, 116 },
        /*279*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, "01", "", 0, 20, 1, 19, 38, 56 },
        /*280*/ { BARCODE_CODEONE, -1, "12345678901234567890", "", 0, 16, 16, 18, 36, 32 },
        /*281*/ { BARCODE_CODEONE, COMPLIANT_HEIGHT, "12345678901234567890", "", 0, 16, 16, 18, 36, 32 },
        /*282*/ { BARCODE_GRIDMATRIX, -1, "ABC", "", 0, 18, 18, 18, 36, 36 },
        /*283*/ { BARCODE_GRIDMATRIX, COMPLIANT_HEIGHT, "ABC", "", 0, 18, 18, 18, 36, 36 },
        /*284*/ { BARCODE_UPNQR, -1, "1234567890AB", "", 0, 77, 77, 77, 154, 154 },
        /*285*/ { BARCODE_UPNQR, COMPLIANT_HEIGHT, "1234567890AB", "", 0, 77, 77, 77, 154, 154 },
        /*286*/ { BARCODE_ULTRA, -1, "1234567890", "", 0, 13, 13, 18, 36, 26 },
        /*287*/ { BARCODE_ULTRA, COMPLIANT_HEIGHT, "1234567890", "", 0, 13, 13, 18, 36, 26 },
        /*288*/ { BARCODE_RMQR, -1, "12345", "", 0, 11, 11, 27, 54, 22 },
        /*289*/ { BARCODE_RMQR, COMPLIANT_HEIGHT, "12345", "", 0, 11, 11, 27, 54, 22 },
        /*290*/ { BARCODE_BC412, -1, "1234567", "", 0, 16.5, 1, 102, 204, 49 },
        /*291*/ { BARCODE_BC412, COMPLIANT_HEIGHT, "1234567", "", 0, 16.5, 1, 102, 204, 49 },
        /*292*/ { BARCODE_DXFILMEDGE, -1, "120476", "", 0, 6, 2, 23, 46, 12 },
        /*293*/ { BARCODE_DXFILMEDGE, COMPLIANT_HEIGHT, "120476", "", 0, 6, 2, 23, 46, 12 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;
    int ret_buf;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/,
                                    -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                                    text, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret_buf = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret_buf, "i:%d ZBarcode_Buffer(%s) ret_buf %d != 0 (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret_buf, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n",
                    i, testUtilBarcodeName(data[i].symbology));

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) { /* ZINT_DEBUG_TEST_PRINT 16 */
            testUtilBitmapPrint(symbol, NULL, NULL);
        }

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, \"%s\", \"%s\", %s, %.8g, %d, %d, %d, %d },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilOutputOptionsName(data[i].output_options),
                    data[i].data, data[i].composite, testUtilErrorName(ret),
                    symbol->height, symbol->rows, symbol->width, symbol->bitmap_width, symbol->bitmap_height);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width,
                        "i:%d (%s) symbol->bitmap_width %d != %d\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width,
                        data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height,
                        "i:%d (%s) symbol->bitmap_height %d != %d\n",
                        i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height,
                        data[i].expected_bitmap_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_upcean_hrt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int show_hrt; /* Using -1 in data as show_hrt, 1 in data as show_hrt but empty space */
        int output_options;
        const char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_text_row;
        int expected_text_col;
        int expected_text_len;
        int expected_text2_row;
        int expected_text2_col;
        int expected_text2_len;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_EAN13, -1, -1, "123456789012", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 22, -1, -1, -1 },
        /*  1*/ { BARCODE_EANX, -1, -1, "123456789012", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 22, -1, -1, -1 }, /* EAN-13 */
        /*  2*/ { BARCODE_EAN13, -1, BARCODE_CONTENT_SEGS, "123456789012", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 22, -1, -1, -1 },
        /*  3*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "123456789012", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 22, -1, -1, -1 }, /* EAN-13 */
        /*  4*/ { BARCODE_EAN13, 0, -1, "123456789012", 0, 50, 1, 95, 226, 110, 102 /*text_row*/, 0, 22, -1, -1, -1 },
        /*  5*/ { BARCODE_EANX, 0, -1, "123456789012", 0, 50, 1, 95, 226, 110, 102 /*text_row*/, 0, 22, -1, -1, -1 }, /* EAN-13 */
        /*  6*/ { BARCODE_EAN13, -1, EANUPC_GUARD_WHITESPACE, "123456789012", 0, 50, 1, 95, 226, 116, 108 /*text_row*/, 212, 14, -1, -1, -1 },
        /*  7*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "123456789012", 0, 50, 1, 95, 226, 116, 108 /*text_row*/, 212, 14, -1, -1, -1 }, /* EAN-13 */
        /*  8*/ { BARCODE_EAN13, 0, EANUPC_GUARD_WHITESPACE, "123456789012", 0, 50, 1, 95, 226, 110, 108 /*text_row*/, 212, 14, -1, -1, -1 },
        /*  9*/ { BARCODE_EANX, 0, EANUPC_GUARD_WHITESPACE, "123456789012", 0, 50, 1, 95, 226, 110, 108 /*text_row*/, 212, 14, -1, -1, -1 }, /* EAN-13 */
        /* 10*/ { BARCODE_EAN13, 1 /*empty*/, -1, "123456789012", 0, 50, 1, 95, 226, 116, 108 /*text_row*/, 212, 14, -1, -1, -1 },
        /* 11*/ { BARCODE_EANX, 1 /*empty*/, -1, "123456789012", 0, 50, 1, 95, 226, 116, 108 /*text_row*/, 212, 14, -1, -1, -1 }, /* EAN-13 */
        /* 12*/ { BARCODE_EANX_CHK, -1, -1, "1234567890128", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 22, -1, -1, -1 }, /* EAN-13 */
        /* 13*/ { BARCODE_EANX_CHK, 0, -1, "1234567890128", 0, 50, 1, 95, 226, 110, 102 /*text_row*/, 0, 22, -1, -1, -1 }, /* EAN-13 */
        /* 14*/ { BARCODE_EANX_CHK, -1, -1, "1234567890128+12", 0, 50, 1, 122, 276, 116, 102 /*text_row*/, 0, 22, 5, 212, 64 }, /* EAN-13 + EAN-2 */
        /* 15*/ { BARCODE_EANX_CHK, -1, BARCODE_CONTENT_SEGS, "1234567890128+12", 0, 50, 1, 122, 276, 116, 102 /*text_row*/, 0, 22, 5, 212, 64 }, /* EAN-13 + EAN-2 */
        /* 16*/ { BARCODE_EANX_CHK, 0, -1, "1234567890128+12", 0, 50, 1, 122, 276, 110, 102 /*text_row*/, 0, 22, 5, 212, 64 }, /* EAN-13 + EAN-2 */
        /* 17*/ { BARCODE_EANX_CHK, -1, EANUPC_GUARD_WHITESPACE, "1234567890128+12", 0, 50, 1, 122, 276, 116, 102 /*text_row*/, 0, 22, 6, 264, 12 }, /* EAN-13 + EAN-2 */
        /* 18*/ { BARCODE_EANX_CHK, 0, EANUPC_GUARD_WHITESPACE, "1234567890128+12", 0, 50, 1, 122, 276, 110, 102 /*text_row*/, 0, 22, 6, 264, 12 }, /* EAN-13 + EAN-2 */
        /* 19*/ { BARCODE_EANX_CHK, 1 /*empty*/, -1, "1234567890128+12", 0, 50, 1, 122, 276, 116, 6 /*text_row*/, 264, 12, -1, -1, -1 }, /* EAN-13 + EAN-2 */
        /* 20*/ { BARCODE_EAN13, -1, -1, "1234567890128+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 22, 5, 212, 118 },
        /* 21*/ { BARCODE_EANX, -1, -1, "1234567890128+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 22, 5, 212, 118 }, /* EAN-13 + EAN-5 */
        /* 22*/ { BARCODE_EAN13, -1, BARCODE_CONTENT_SEGS, "1234567890128+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 22, 5, 212, 118 },
        /* 23*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "1234567890128+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 22, 5, 212, 118 }, /* EAN-13 + EAN-5 */
        /* 24*/ { BARCODE_EAN13, 0, -1, "1234567890128+12345", 0, 50, 1, 149, 330, 110, 102 /*text_row*/, 0, 22, 5, 212, 118 },
        /* 25*/ { BARCODE_EANX, 0, -1, "1234567890128+12345", 0, 50, 1, 149, 330, 110, 102 /*text_row*/, 0, 22, 5, 212, 118 }, /* EAN-13 + EAN-5 */
        /* 26*/ { BARCODE_ISBNX, -1, -1, "9784567890120+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 22, 5, 212, 118 }, /* ISBNX + EAN-5 */
        /* 27*/ { BARCODE_ISBNX, -1, BARCODE_CONTENT_SEGS, "9784567890120+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 22, 5, 212, 118 }, /* ISBNX + EAN-5 */
        /* 28*/ { BARCODE_ISBNX, 0, -1, "9784567890120+12345", 0, 50, 1, 149, 330, 110, 102 /*text_row*/, 0, 22, 5, 212, 118 }, /* ISBNX + EAN-5 */
        /* 29*/ { BARCODE_ISBNX, -1, EANUPC_GUARD_WHITESPACE, "9784567890120+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 22, 6, 304, 26 }, /* ISBNX + EAN-5 */
        /* 30*/ { BARCODE_ISBNX, 0, EANUPC_GUARD_WHITESPACE, "9784567890120+12345", 0, 50, 1, 149, 330, 110, 102 /*text_row*/, 0, 22, 6, 304, 26 }, /* ISBNX + EAN-5 */
        /* 31*/ { BARCODE_ISBNX, 1 /*empty*/, -1, "9784567890120+12345", 0, 50, 1, 149, 330, 116, 6 /*text_row*/, 304, 26, -1, -1, -1 }, /* ISBNX + EAN-5 */
        /* 32*/ { BARCODE_EAN8, -1, -1, "123456", 0, 50, 1, 67, 162, 116, 106 /*text_row*/, 20, 58, -1, -1, -1 },
        /* 33*/ { BARCODE_EANX, -1, -1, "123456", 0, 50, 1, 67, 162, 116, 106 /*text_row*/, 20, 58, -1, -1, -1 }, /* EAN-8 */
        /* 34*/ { BARCODE_EAN8, -1, BARCODE_CONTENT_SEGS, "123456", 0, 50, 1, 67, 162, 116, 106 /*text_row*/, 20, 58, -1, -1, -1 },
        /* 35*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "123456", 0, 50, 1, 67, 162, 116, 106 /*text_row*/, 20, 58, -1, -1, -1 }, /* EAN-8 */
        /* 36*/ { BARCODE_EAN8, 0, -1, "123456", 0, 50, 1, 67, 162, 110, 106 /*text_row*/, 20, 58, -1, -1, -1 },
        /* 37*/ { BARCODE_EANX, 0, -1, "123456", 0, 50, 1, 67, 162, 110, 106 /*text_row*/, 20, 58, -1, -1, -1 }, /* EAN-8 */
        /* 38*/ { BARCODE_EAN8, -1, EANUPC_GUARD_WHITESPACE, "123456", 0, 50, 1, 67, 162, 116, 106 /*text_row*/, 0, 14, 106, 148, 14 },
        /* 39*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "123456", 0, 50, 1, 67, 162, 116, 106 /*text_row*/, 0, 14, 106, 148, 14 }, /* EAN-8 */
        /* 40*/ { BARCODE_EAN8, 0, EANUPC_GUARD_WHITESPACE, "123456", 0, 50, 1, 67, 162, 110, 106 /*text_row*/, 0, 14, 106, 148, 14 },
        /* 41*/ { BARCODE_EANX, 0, EANUPC_GUARD_WHITESPACE, "123456", 0, 50, 1, 67, 162, 110, 106 /*text_row*/, 0, 14, 106, 148, 14 }, /* EAN-8 */
        /* 42*/ { BARCODE_EAN8, 1 /*empty*/, -1, "123456", 0, 50, 1, 67, 162, 116, 106 /*text_row*/, 0, 14, 106, 148, 14 },
        /* 43*/ { BARCODE_EANX, 1 /*empty*/, -1, "123456", 0, 50, 1, 67, 162, 116, 106 /*text_row*/, 0, 14, 106, 148, 14 }, /* EAN-8 */
        /* 44*/ { BARCODE_EAN8, -1, -1, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116, 106 /*text_row*/, 20, 58, 5, 148, 68 },
        /* 45*/ { BARCODE_EANX, -1, -1, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116, 106 /*text_row*/, 20, 58, 5, 148, 68 }, /* EAN-8 + EAN-2 */
        /* 46*/ { BARCODE_EAN8, -1, BARCODE_CONTENT_SEGS, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116, 106 /*text_row*/, 20, 58, 5, 148, 68 },
        /* 47*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116, 106 /*text_row*/, 20, 58, 5, 148, 68 }, /* EAN-8 + EAN-2 */
        /* 48*/ { BARCODE_EAN8, 0, -1, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 110, 106 /*text_row*/, 20, 58, 5, 148, 68 },
        /* 49*/ { BARCODE_EANX, 0, -1, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 110, 106 /*text_row*/, 20, 58, 5, 148, 68 }, /* EAN-8 + EAN-2 */
        /* 50*/ { BARCODE_EAN8, -1, EANUPC_GUARD_WHITESPACE, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116, 106 /*text_row*/, 0, 14, 6, 202, 8 },
        /* 51*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116, 106 /*text_row*/, 0, 14, 6, 202, 8 }, /* EAN-8 + EAN-2 */
        /* 52*/ { BARCODE_EAN8, 0, EANUPC_GUARD_WHITESPACE, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 110, 106 /*text_row*/, 0, 14, 6, 202, 8 },
        /* 53*/ { BARCODE_EANX, 0, EANUPC_GUARD_WHITESPACE, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 110, 106 /*text_row*/, 0, 14, 6, 202, 8 }, /* EAN-8 + EAN-2 */
        /* 54*/ { BARCODE_EAN8, 1 /*empty*/, -1, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116, 106 /*text_row*/, 0, 14, 6, 202, 8 },
        /* 55*/ { BARCODE_EANX, 1 /*empty*/, -1, "123456+12", ZINT_WARN_NONCOMPLIANT, 50, 1, 94, 212, 116, 106 /*text_row*/, 0, 14, 6, 202, 8 }, /* EAN-8 + EAN-2 */
        /* 56*/ { BARCODE_EAN8, -1, -1, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116, 106 /*text_row*/, 20, 58, 5, 148, 122 },
        /* 57*/ { BARCODE_EANX, -1, -1, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116, 106 /*text_row*/, 20, 58, 5, 148, 122 }, /* EAN-8 + EAN-5 */
        /* 58*/ { BARCODE_EAN8, -1, BARCODE_CONTENT_SEGS, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116, 106 /*text_row*/, 20, 58, 5, 148, 122 },
        /* 59*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116, 106 /*text_row*/, 20, 58, 5, 148, 122 }, /* EAN-8 + EAN-5 */
        /* 60*/ { BARCODE_EAN8, 0, -1, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 110, 106 /*text_row*/, 20, 58, 5, 148, 122 },
        /* 61*/ { BARCODE_EANX, 0, -1, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 110, 106 /*text_row*/, 20, 58, 5, 148, 122 }, /* EAN-8 + EAN-5 */
        /* 62*/ { BARCODE_EAN8, -1, EANUPC_GUARD_WHITESPACE, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116, 106 /*text_row*/, 0, 14, 6, 256, 10 },
        /* 63*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116, 106 /*text_row*/, 0, 14, 6, 256, 10 }, /* EAN-8 + EAN-5 */
        /* 64*/ { BARCODE_EAN8, 0, EANUPC_GUARD_WHITESPACE, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 110, 106 /*text_row*/, 0, 14, 6, 256, 10 },
        /* 65*/ { BARCODE_EANX, 0, EANUPC_GUARD_WHITESPACE, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 110, 106 /*text_row*/, 0, 14, 6, 256, 10 }, /* EAN-8 + EAN-5 */
        /* 66*/ { BARCODE_EAN8, 1 /*empty*/, -1, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116, 106 /*text_row*/, 0, 14, 6, 256, 10 },
        /* 67*/ { BARCODE_EANX, 1 /*empty*/, -1, "123456+12345", ZINT_WARN_NONCOMPLIANT, 50, 1, 121, 266, 116, 106 /*text_row*/, 0, 14, 6, 256, 10 }, /* EAN-8 + EAN-5 */
        /* 68*/ { BARCODE_EAN_5ADDON, -1, -1, "1234", 0, 50, 1, 47, 104, 116, 0 /*text_row*/, 18, 5, -1, -1, -1 },
        /* 69*/ { BARCODE_EANX, -1, -1, "1234", 0, 50, 1, 47, 104, 116, 0 /*text_row*/, 18, 5, -1, -1, -1 }, /* EAN-5 */
        /* 70*/ { BARCODE_EAN_5ADDON, -1, BARCODE_CONTENT_SEGS, "1234", 0, 50, 1, 47, 104, 116, 0 /*text_row*/, 18, 5, -1, -1, -1 },
        /* 71*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "1234", 0, 50, 1, 47, 104, 116, 0 /*text_row*/, 18, 5, -1, -1, -1 }, /* EAN-5 */
        /* 72*/ { BARCODE_EAN_5ADDON, 0, -1, "1234", 0, 50, 1, 47, 104, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 },
        /* 73*/ { BARCODE_EANX, 0, -1, "1234", 0, 50, 1, 47, 104, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 }, /* EAN-5 */
        /* 74*/ { BARCODE_EAN_5ADDON, -1, EANUPC_GUARD_WHITESPACE, "1234", 0, 50, 1, 47, 104, 116, 6 /*text_row*/, 94, 10, -1, -1, -1 },
        /* 75*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "1234", 0, 50, 1, 47, 104, 116, 6 /*text_row*/, 94, 10, -1, -1, -1 }, /* EAN-5 */
        /* 76*/ { BARCODE_EAN_5ADDON, 0, EANUPC_GUARD_WHITESPACE, "1234", 0, 50, 1, 47, 104, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 },
        /* 77*/ { BARCODE_EANX, 0, EANUPC_GUARD_WHITESPACE, "1234", 0, 50, 1, 47, 104, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 }, /* EAN-5 */
        /* 78*/ { BARCODE_EAN_5ADDON, 1 /*empty*/, -1, "1234", 0, 50, 1, 47, 104, 116, 6 /*text_row*/, 94, 10, -1, -1, -1 },
        /* 79*/ { BARCODE_EANX, 1 /*empty*/, -1, "1234", 0, 50, 1, 47, 104, 116, 6 /*text_row*/, 94, 10, -1, -1, -1 }, /* EAN-5 */
        /* 80*/ { BARCODE_EAN_2ADDON, -1, -1, "12", 0, 50, 1, 20, 50, 116, 0 /*text_row*/, 14, 2, -1, -1, -1 },
        /* 81*/ { BARCODE_EANX, -1, -1, "12", 0, 50, 1, 20, 50, 116, 0 /*text_row*/, 14, 2, -1, -1, -1 }, /* EAN-2 */
        /* 82*/ { BARCODE_EAN_2ADDON, -1, BARCODE_CONTENT_SEGS, "12", 0, 50, 1, 20, 50, 116, 0 /*text_row*/, 14, 2, -1, -1, -1 },
        /* 83*/ { BARCODE_EANX, -1, BARCODE_CONTENT_SEGS, "12", 0, 50, 1, 20, 50, 116, 0 /*text_row*/, 14, 2, -1, -1, -1 }, /* EAN-2 */
        /* 84*/ { BARCODE_EAN_2ADDON, 0, -1, "12", 0, 50, 1, 20, 50, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 },
        /* 85*/ { BARCODE_EANX, 0, -1, "12", 0, 50, 1, 20, 50, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 }, /* EAN-2 */
        /* 86*/ { BARCODE_EAN_2ADDON, -1, EANUPC_GUARD_WHITESPACE, "12", 0, 50, 1, 20, 50, 116, 6 /*text_row*/, 40, 10, -1, -1, -1 },
        /* 87*/ { BARCODE_EANX, -1, EANUPC_GUARD_WHITESPACE, "12", 0, 50, 1, 20, 50, 116, 6 /*text_row*/, 40, 10, -1, -1, -1 }, /* EAN-2 */
        /* 88*/ { BARCODE_EAN_2ADDON, 0, EANUPC_GUARD_WHITESPACE, "12", 0, 50, 1, 20, 50, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 },
        /* 89*/ { BARCODE_EANX, 0, EANUPC_GUARD_WHITESPACE, "12", 0, 50, 1, 20, 50, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 }, /* EAN-2 */
        /* 90*/ { BARCODE_EAN_2ADDON, 1 /*empty*/, -1, "12", 0, 50, 1, 20, 50, 116, 6 /*text_row*/, 40, 10, -1, -1, -1 },
        /* 91*/ { BARCODE_EANX, 1 /*empty*/, -1, "12", 0, 50, 1, 20, 50, 116, 6 /*text_row*/, 40, 10, -1, -1, -1 }, /* EAN-2 */
        /* 92*/ { BARCODE_UPCA, -1, -1, "123456789012", 0, 50, 1, 95, 226, 116, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 93*/ { BARCODE_UPCA, -1, BARCODE_CONTENT_SEGS, "123456789012", 0, 50, 1, 95, 226, 116, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 94*/ { BARCODE_UPCA, 0, -1, "123456789012", 0, 50, 1, 95, 226, 110, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 95*/ { BARCODE_UPCA, -1, -1, "123456789012+12", 0, 50, 1, 124, 276, 116, 104 /*text_row*/, 0, 18, 5, 208, 68 },
        /* 96*/ { BARCODE_UPCA, -1, BARCODE_CONTENT_SEGS, "123456789012+12", 0, 50, 1, 124, 276, 116, 104 /*text_row*/, 0, 18, 5, 208, 68 },
        /* 97*/ { BARCODE_UPCA, 0, -1, "123456789012+12", 0, 50, 1, 124, 276, 110, 104 /*text_row*/, 0, 18, 5, 208, 68 },
        /* 98*/ { BARCODE_UPCA, -1, EANUPC_GUARD_WHITESPACE, "123456789012+12", 0, 50, 1, 124, 276, 116, 104 /*text_row*/, 0, 18, 5, 266, 10 },
        /* 99*/ { BARCODE_UPCA, 0, EANUPC_GUARD_WHITESPACE, "123456789012+12", 0, 50, 1, 124, 276, 110, 104 /*text_row*/, 0, 18, 5, 266, 10 },
        /*100*/ { BARCODE_UPCA, 1 /*empty*/, -1, "123456789012+12", 0, 50, 1, 124, 276, 116, 5 /*text_row*/, 266, 10, -1, -1, -1 },
        /*101*/ { BARCODE_UPCA_CHK, -1, -1, "123456789012+12345", 0, 50, 1, 151, 330, 116, 104 /*text_row*/, 0, 18, 5, 208, 122 },
        /*102*/ { BARCODE_UPCA_CHK, -1, BARCODE_CONTENT_SEGS, "123456789012+12345", 0, 50, 1, 151, 330, 116, 104 /*text_row*/, 0, 18, 5, 208, 122 },
        /*103*/ { BARCODE_UPCA_CHK, 0, -1, "123456789012+12345", 0, 50, 1, 151, 330, 110, 104 /*text_row*/, 0, 18, 5, 208, 122 },
        /*104*/ { BARCODE_UPCA_CHK, -1, EANUPC_GUARD_WHITESPACE, "123456789012+12345", 0, 50, 1, 151, 330, 116, 104 /*text_row*/, 0, 18, 6, 320, 10 },
        /*105*/ { BARCODE_UPCA_CHK, 0, EANUPC_GUARD_WHITESPACE, "123456789012+12345", 0, 50, 1, 151, 330, 110, 104 /*text_row*/, 0, 18, 6, 320, 10 },
        /*106*/ { BARCODE_UPCA_CHK, 1 /*empty*/, -1, "123456789012+12345", 0, 50, 1, 151, 330, 116, 6 /*text_row*/, 320, 10, -1, -1, -1 },
        /*107*/ { BARCODE_UPCE, -1, -1, "1234567", 0, 50, 1, 51, 134, 116, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /*108*/ { BARCODE_UPCE, -1, BARCODE_CONTENT_SEGS, "1234567", 0, 50, 1, 51, 134, 116, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /*109*/ { BARCODE_UPCE, 0, -1, "1234567", 0, 50, 1, 51, 134, 110, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /*110*/ { BARCODE_UPCE_CHK, -1, -1, "12345670+12", 0, 50, 1, 78, 184, 116, 104 /*text_row*/, 0, 18, 5, 120, 64 },
        /*111*/ { BARCODE_UPCE_CHK, -1, BARCODE_CONTENT_SEGS, "12345670+12", 0, 50, 1, 78, 184, 116, 104 /*text_row*/, 0, 18, 5, 120, 64 },
        /*112*/ { BARCODE_UPCE_CHK, 0, -1, "12345670+12", 0, 50, 1, 78, 184, 110, 104 /*text_row*/, 0, 18, 5, 120, 64 },
        /*113*/ { BARCODE_UPCE_CHK, -1, EANUPC_GUARD_WHITESPACE, "12345670+12", 0, 50, 1, 78, 184, 116, 104 /*text_row*/, 0, 18, 6, 174, 10 },
        /*114*/ { BARCODE_UPCE_CHK, 0, EANUPC_GUARD_WHITESPACE, "12345670+12", 0, 50, 1, 78, 184, 110, 104 /*text_row*/, 0, 18, 6, 174, 10 },
        /*115*/ { BARCODE_UPCE_CHK, 1 /*empty*/, -1, "12345670+12", 0, 50, 1, 78, 184, 116, 6 /*text_row*/, 174, 10, -1, -1, -1 },
        /*116*/ { BARCODE_UPCE, -1, -1, "1234567+12345", 0, 50, 1, 105, 238, 116, 104 /*text_row*/, 0, 18, 5, 120, 118 },
        /*117*/ { BARCODE_UPCE, -1, BARCODE_CONTENT_SEGS, "1234567+12345", 0, 50, 1, 105, 238, 116, 104 /*text_row*/, 0, 18, 5, 120, 118 },
        /*118*/ { BARCODE_UPCE, 0, -1, "1234567+12345", 0, 50, 1, 105, 238, 110, 104 /*text_row*/, 0, 18, 5, 120, 118 },
        /*119*/ { BARCODE_UPCE, -1, EANUPC_GUARD_WHITESPACE, "1234567+12345", 0, 50, 1, 105, 238, 116, 104 /*text_row*/, 0, 18, 6, 228, 10 },
        /*120*/ { BARCODE_UPCE, 0, EANUPC_GUARD_WHITESPACE, "1234567+12345", 0, 50, 1, 105, 238, 110, 104 /*text_row*/, 0, 18, 6, 228, 10 },
        /*121*/ { BARCODE_UPCE, 1 /*empty*/, -1, "1234567+12345", 0, 50, 1, 105, 238, 116, 6 /*text_row*/, 228, 10, -1, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].show_hrt != -1) { /* Default is show_hrt */
            symbol->show_hrt = data[i].show_hrt;
        }
        if (data[i].output_options != -1) {
            symbol->output_options = data[i].output_options;
        }
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) { /* ZINT_DEBUG_TEST_PRINT 16 */
            testUtilBitmapPrint(symbol, NULL, NULL);
        }

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height,
                    "i:%d (%s) symbol->bitmap_height %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);

        assert_nonzero(symbol->bitmap_height > data[i].expected_text_row,
                    "i:%d symbol->bitmap_height %d <= data[i].expected_text_row %d\n",
                    i, symbol->bitmap_height, data[i].expected_text_row);
        if (data[i].expected_text_row == -1) { /* EAN-2/5 just truncates bitmap if no text */
            assert_zero(data[i].show_hrt, "i:%d Expected text row -1 but show_hrt set\n", i);
            if (i && data[i - 1].symbology == symbol->symbology && data[i - 1].show_hrt
                    && data[i - 1].expected_text_row != -1) {
                assert_nonzero(data[i].expected_bitmap_height > data[i - 1].expected_text_row,
                            "i:%d (%s) expected_bitmap_height %d <= previous expected_text_row %d\n",
                            i, testUtilBarcodeName(data[i].symbology), data[i].expected_bitmap_height,
                            data[i - 1].expected_text_row);
            }
        } else {
            int text_bits_set = 0;
            int row = data[i].expected_text_row;
            int column;
            for (column = data[i].expected_text_col;
                    column < data[i].expected_text_col + data[i].expected_text_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    text_bits_set = 1;
                    break;
                }
            }
            if (data[i].show_hrt == -1) { /* Using -1 in data as show_hrt, 1 in data as show_hrt but empty space */
                assert_nonzero(text_bits_set, "i:%d (%s) text_bits_set zero\n",
                            i, testUtilBarcodeName(data[i].symbology));
            } else {
                assert_zero(text_bits_set, "i:%d (%s) text_bits_set non-zero\n",
                            i, testUtilBarcodeName(data[i].symbology));
            }
        }

        if (data[i].expected_text2_row != -1) {
            int text2_bits_set = 0;
            int row = data[i].expected_text2_row;
            int column;
            for (column = data[i].expected_text2_col;
                    column < data[i].expected_text2_col + data[i].expected_text2_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    text2_bits_set = 1;
                    break;
                }
            }
            if (data[i].show_hrt == -1) { /* Using -1 in data as show_hrt, 1 in data as show_hrt but empty space */
                assert_nonzero(text2_bits_set, "i:%d (%s) text2_bits_set zero\n",
                            i, testUtilBarcodeName(data[i].symbology));
            } else {
                assert_zero(text2_bits_set, "i:%d (%s) text2_bits_set non-zero\n",
                            i, testUtilBarcodeName(data[i].symbology));
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_row_separator(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int border_width;
        int option_1;
        int option_3;
        const char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_separator_row;
        int expected_separator_col;
        int expected_separator_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, -1, -1, -1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 },
        /*  1*/ { BARCODE_CODABLOCKF, -1, -1, 0, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, /* Same as default */
        /*  2*/ { BARCODE_CODABLOCKF, -1, -1, 1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, /* Same as default */
        /*  3*/ { BARCODE_CODABLOCKF, -1, -1, 2, "A", 0, 20, 2, 101, 242, 44, 20, 42, 4 },
        /*  4*/ { BARCODE_CODABLOCKF, -1, -1, 3, "A", 0, 20, 2, 101, 242, 44, 19, 42, 6 },
        /*  5*/ { BARCODE_CODABLOCKF, -1, -1, 4, "A", 0, 20, 2, 101, 242, 44, 18, 42, 8 },
        /*  6*/ { BARCODE_CODABLOCKF, -1, -1, 5, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, /* > 4 ignored, same as default */
        /*  7*/ { BARCODE_CODABLOCKF, -1, 1, -1, "A", 0, 5, 1, 46, 132, 14, 0, 20 + 2, 2 }, /* CODE128 top separator, add 2 to skip over end of start char; note no longer includes HRT */
        /*  8*/ { BARCODE_CODABLOCKF, 0, -1, -1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, /* Border width zero, same as default */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int j, separator_bits_set;


        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }

        ret = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%s) symbol->bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n", i, testUtilBarcodeName(data[i].symbology),
                    symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n", i, testUtilBarcodeName(data[i].symbology),
                    symbol->bitmap_height, data[i].expected_bitmap_height);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        for (j = data[i].expected_separator_row; j < data[i].expected_separator_row + data[i].expected_separator_height; j++) {
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
            assert_nonzero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);
        }

        if (symbol->rows > 1) {
            j = data[i].expected_separator_row - 1;
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col + 2); /* Need to add 2 to skip to 1st blank of start row character */
            assert_zero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) before non-zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);
        }

        j = data[i].expected_separator_row + data[i].expected_separator_height;
        separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col + 2); /* Need to add 2 to skip to 1st blank of start row character */
        assert_zero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) after non-zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_stacking(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        int option_1;
        int option_3;
        const char *data;
        const char *data2;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_separator_row;
        int expected_separator_col;
        int expected_separator_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, "A", "B", 50, 2, 46, 92, 116, -1, -1, -1 },
        /*  1*/ { BARCODE_CODE128, BARCODE_BIND, -1, -1, "A", "B", 50, 2, 46, 92, 116, 49, 4, 2 },
        /*  2*/ { BARCODE_CODE128, BARCODE_BIND, -1, 2, "A", "B", 50, 2, 46, 92, 116, 48, 4, 4 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int length2;
        int j, separator_bits_set;


        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, data[i].output_options, data[i].data, -1, debug);
        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);

        length2 = (int) strlen(data[i].data2);
        ret = ZBarcode_Encode(symbol, TCU(data[i].data2), length2);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        if (data[i].expected_separator_row != -1) {
            for (j = data[i].expected_separator_row; j < data[i].expected_separator_row + data[i].expected_separator_height; j++) {
                separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
                assert_nonzero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
            }

            if (symbol->rows > 1) {
                j = data[i].expected_separator_row - 1;
                separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
                assert_zero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) before non-zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
            }

            j = data[i].expected_separator_row + data[i].expected_separator_height;
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
            assert_zero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) after non-zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_stacking_too_many(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data[] = "A";
    char expected_errtxt[] = "Error 770: Too many stacked symbols";

    testStartSymbol(p_ctx->func_name, &symbol);

    symbol = ZBarcode_Create();
    assert_nonnull(symbol, "Symbol not created\n");

    for (i = 0; i < 200; i++) {
        length = testUtilSetSymbol(symbol, BARCODE_CODE128, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1, data, -1, debug);
        ret = ZBarcode_Encode(symbol, TCU(data), length);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);
    }
    assert_equal(symbol->rows, 200, "symbol->rows %d != 200\n", symbol->rows);

    length = testUtilSetSymbol(symbol, BARCODE_CODE128, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1, data, -1, debug);
    ret = ZBarcode_Encode(symbol, TCU(data), length);
    assert_equal(ret, ZINT_ERROR_TOO_LONG, "ret %d != ZINT_ERROR_TOO_LONG\n", ret);
    assert_zero(strcmp(symbol->errtxt, expected_errtxt), "symbol->errtxt(%s) != expected_errtxt(%s)\n", symbol->errtxt, expected_errtxt);

    ZBarcode_Delete(symbol);

    testFinish();
}

static void test_output_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int whitespace_width;
        int whitespace_height;
        int border_width;
        int output_options;
        int rotate_angle;
        float scale;
        const char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_set;
        int expected_set_row;
        int expected_set_col;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, -1, 0, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  1*/ { BARCODE_CODE128, -1, -1, -1, -1, 180, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 115, 4 },
        /*  2*/ { BARCODE_CODE128, -1, -1, 2, -1, 0, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  3*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BIND, 0, 0, "A123", 0, 50, 1, 79, 158, 124, 1, 0, 4 },
        /*  4*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BIND, 0, 0, "A123", 0, 50, 1, 79, 158, 124, 0, 4, 4 },
        /*  5*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BOX, 0, 0, "A123", 0, 50, 1, 79, 166, 124, 1, 4, 4 },
        /*  6*/ { BARCODE_CODE128, -1, -1, 0, BARCODE_BIND, 0, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  7*/ { BARCODE_CODE128, -1, -1, 0, BARCODE_BOX, 0, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 4, 4 },
        /*  8*/ { BARCODE_CODE128, -1, -1, -1, -1, 0, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 8 },
        /*  9*/ { BARCODE_CODE128, 3, -1, -1, -1, 0, 0, "A123", 0, 50, 1, 79, 170, 116, 1, 0, 8 },
        /* 10*/ { BARCODE_CODE128, 3, 1, -1, -1, 0, 0, "A123", 0, 50, 1, 79, 170, 120, 0, 0, 8 },
        /* 11*/ { BARCODE_CODE128, 3, -1, 4, -1, 0, 0, "A123", 0, 50, 1, 79, 170, 116, 1, 0, 8 },
        /* 12*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BIND, 0, 0, "A123", 0, 50, 1, 79, 170, 132, 1, 0, 0 },
        /* 13*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BIND, 0, 0, "A123", 0, 50, 1, 79, 170, 132, 0, 8, 0 },
        /* 14*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BOX, 0, 0, "A123", 0, 50, 1, 79, 186, 132, 1, 8, 0 },
        /* 15*/ { BARCODE_CODE128, -1, -1, -1, BARCODE_DOTTY_MODE, 0, 0, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 16*/ { BARCODE_CODE128, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /* 17*/ { BARCODE_CODE128, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 180, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 115, 4 },
        /* 18*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BOX | OUT_BUFFER_INTERMEDIATE, 0, 0, "A123", 0, 50, 1, 79, 186, 132, 1, 8, 0 },
        /* 19*/ { BARCODE_QRCODE, -1, -1, -1, -1, 0, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 20*/ { BARCODE_QRCODE, -1, -1, -1, -1, 180, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 39, 2 },
        /* 21*/ { BARCODE_QRCODE, -1, -1, 3, -1, 0, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 22*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, 0, 0, "A123", 0, 21, 21, 21, 42, 54, 1, 2, 2 },
        /* 23*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, 0, 0, "A123", 0, 21, 21, 21, 42, 54, 0, 20, 0 },
        /* 24*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX, 0, 0, "A123", 0, 21, 21, 21, 54, 54, 1, 20, 0 },
        /* 25*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, 0, 0, "A123", 0, 21, 21, 21, 42, 54, 1, 6, 0 },
        /* 26*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND | BARCODE_QUIET_ZONES, 0, 0, "A123", 0, 21, 21, 21, 58, 70, 0, 6, 0 },
        /* 27*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX | BARCODE_QUIET_ZONES, 0, 0, "A123", 0, 21, 21, 21, 70, 70, 1, 6, 0 },
        /* 28*/ { BARCODE_QRCODE, -1, -1, -1, -1, 0, 0, "A123", 0, 21, 21, 21, 42, 42, 1, 0, 0 },
        /* 29*/ { BARCODE_QRCODE, 5, -1, -1, -1, 0, 0, "A123", 0, 21, 21, 21, 62, 42, 0, 0, 0 },
        /* 30*/ { BARCODE_QRCODE, 5, -1, 6, -1, 0, 0, "A123", 0, 21, 21, 21, 62, 42, 0, 0, 0 },
        /* 31*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, 0, 0, "A123", 0, 21, 21, 21, 62, 66, 1, 0, 0 },
        /* 32*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, 0, 0, "A123", 0, 21, 21, 21, 62, 66, 0, 12, 0 },
        /* 33*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BOX, 0, 0, "A123", 0, 21, 21, 21, 86, 66, 1, 12, 0 },
        /* 34*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, 0, 0, "A123", 0, 21, 21, 21, 62, 66, 1, 12, 10 },
        /* 35*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND | BARCODE_QUIET_ZONES, 0, 0, "A123", 0, 21, 21, 21, 78, 82, 0, 12, 10 },
        /* 36*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND | BARCODE_QUIET_ZONES, 0, 0, "A123", 0, 21, 21, 21, 78, 82, 1, 20, 19 },
        /* 37*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BOX | BARCODE_QUIET_ZONES, 0, 0, "A123", 0, 21, 21, 21, 102, 82, 0, 20, 19 },
        /* 38*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 1, 1 },
        /* 39*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, 0.5, "A123", 0, 21, 21, 21, 43, 43, 1, 1, 1 }, /* Scale 0.5 -> 1 */
        /* 40*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 2, 2 },
        /* 41*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 41, 1 },
        /* 42*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 40, 2 },
        /* 43*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 1, 1 },
        /* 44*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 2, 2 },
        /* 45*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 43, 59, 1, 2, 2 },
        /* 46*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 59, 59, 1, 9, 9 },
        /* 47*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 63, 59, 0, 9, 9 },
        /* 48*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 63, 59, 1, 0, 0 },
        /* 49*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 63, 59, 0, 8, 11 },
        /* 50*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 63, 59, 1, 9, 11 },
        /* 51*/ { BARCODE_QRCODE, 1, 1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 63, 63, 0, 8, 11 },
        /* 52*/ { BARCODE_QRCODE, 1, 1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 63, 63, 0, 9, 11 },
        /* 53*/ { BARCODE_QRCODE, 1, 1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, 0, "A123", 0, 21, 21, 21, 63, 63, 1, 11, 11 },
        /* 54*/ { BARCODE_QRCODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, 0, "A123", 0, 21, 21, 21, 42, 42, 1, 1, 1 },
        /* 55*/ { BARCODE_QRCODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 56*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 0, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 1, 1 },
        /* 57*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 0, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 2, 2 },
        /* 58*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 180, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 41, 1 },
        /* 59*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 180, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 40, 2 },
        /* 60*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 0, 0, "A123", 0, 165, 33, 30, 299, 298, 0, 4, 4 },
        /* 61*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 0, 0, "A123", 0, 165, 33, 30, 299, 298, 1, 4, 14 },
        /* 62*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 270, 0, "A123", 0, 165, 33, 30, 298, 299, 1, 4, 4 },
        /* 63*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 270, 0, "A123", 0, 165, 33, 30, 298, 299, 0, 4, 14 },
        /* 64*/ { BARCODE_MAXICODE, -1, -1, 5, -1, 0, 0, "A123", 0, 165, 33, 30, 299, 298, 0, 0, 0 },
        /* 65*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 1, 0, 0 },
        /* 66*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 0, 50, 0 },
        /* 67*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 0, 347, 50 },
        /* 68*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 1, 348, 50 },
        /* 69*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BOX, 0, 0, "A123", 0, 165, 33, 30, 299 + 50 * 2, 298 + 50 * 2, 1, 50, 0 },
        /* 70*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BOX, 0, 0, "A123", 0, 165, 33, 30, 299 + 50 * 2, 298 + 50 * 2, 0, 347, 50 },
        /* 71*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 1, 50, 15 },
        /* 72*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND | BARCODE_QUIET_ZONES, 0, 0, "A123", 0, 165, 33, 30, 299 + 10 * 2, 298 + (50 + 10) * 2, 0, 50, 15 },
        /* 73*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 0, 0, "A123", 0, 165, 33, 30, 299, 298, 1, 0, 14 },
        /* 74*/ { BARCODE_MAXICODE, 6, -1, -1, -1, 0, 0, "A123", 0, 165, 33, 30, 299 + 60 * 2, 298, 0, 0, 14 },
        /* 75*/ { BARCODE_MAXICODE, 6, -1, -1, -1, 0, 0, "A123", 0, 165, 33, 30, 299 + 60 * 2, 298, 0, 0, 47 },
        /* 76*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BIND, 0, 0, "A123", 0, 165, 33, 30, 299 + 60 * 2, 298 + 50 * 2, 1, 0, 47 },
        /* 77*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BIND, 0, 0, "A123", 0, 165, 33, 30, 299 + 60 * 2, 298 + 50 * 2, 0, 50, 0 },
        /* 78*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BOX, 0, 0, "A123", 0, 165, 33, 30, 299 + (60 + 50) * 2, 298 + 50 * 2, 1, 50, 0 },
        /* 79*/ { BARCODE_MAXICODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, 0, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 80*/ { BARCODE_MAXICODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, 0, "A123", 0, 165, 33, 30, 299, 298, 0, 4, 4 },
        /* 81*/ { BARCODE_MAXICODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, 0, "A123", 0, 165, 33, 30, 299, 298, 1, 4, 14 },
        /* 82*/ { BARCODE_MAXICODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 270, 0, "A123", 0, 165, 33, 30, 298, 299, 1, 4, 4 },
        /* 83*/ { BARCODE_MAXICODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 270, 0, "A123", 0, 165, 33, 30, 298, 299, 0, 4, 14 },
        /* 84*/ { BARCODE_ITF14, -1, -1, -1, -1, 0, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 85*/ { BARCODE_ITF14, -1, -1, -1, -1, 90, 0, "123", 0, 50, 1, 135, 136, 330, 1, 0, 110 },
        /* 86*/ { BARCODE_ITF14, -1, -1, 0, -1, 0, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 87*/ { BARCODE_ITF14, -1, -1, 0, BARCODE_BOX, 0, 0, "123", 0, 50, 1, 135, 310, 116, 0, 100, 0 },
        /* 88*/ { BARCODE_ITF14, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 89*/ { BARCODE_ITF14, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 90, 0, "123", 0, 50, 1, 135, 136, 330, 1, 0, 110 },
        /* 90*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, 0, 0, "A123", 0, 20, 2, 101, 242, 44, 1, 43, 24 },
        /* 91*/ { BARCODE_CODABLOCKF, -1, -1, -1, BARCODE_BIND_TOP, 0, 0, "A123", 0, 20, 2, 101, 242, 42, 0, 41, 24 },
        /* 92*/ { BARCODE_CODE16K, -1, -1, -1, -1, 0, 0, "A123", 0, 20, 2, 70, 162, 44, 1, 43, 0 },
        /* 93*/ { BARCODE_CODE16K, -1, -1, -1, BARCODE_BIND_TOP, 0, 0, "A123", 0, 20, 2, 70, 162, 42, 0, 41, 0 },
        /* 94*/ { BARCODE_CODE49, -1, -1, -1, -1, 0, 0, "A123", 0, 20, 2, 70, 162, 44, 1, 43, 0 },
        /* 95*/ { BARCODE_CODE49, -1, -1, -1, BARCODE_BIND_TOP, 0, 0, "A123", 0, 20, 2, 70, 162, 42, 0, 41, 0 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].whitespace_height != -1) {
            symbol->whitespace_height = data[i].whitespace_height;
        }
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }
        if (data[i].scale) {
            symbol->scale = data[i].scale;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, data[i].rotate_angle);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_nonnull(symbol->bitmap, "i:%d (%s) symbol->bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

            if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n", i, testUtilBarcodeName(data[i].symbology),
                        symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n", i, testUtilBarcodeName(data[i].symbology),
                        symbol->bitmap_height, data[i].expected_bitmap_height);

            if (data[i].expected_set != -1) {
                assert_nonzero(data[i].expected_set_row < data[i].expected_bitmap_height, "i:%d (%s) expected_set_row %d >= expected_bitmap_height %d\n",
                        i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_row, data[i].expected_bitmap_height);
                ret = is_row_column_black(symbol, data[i].expected_set_row, data[i].expected_set_col);
                if (data[i].expected_set) {
                    assert_nonzero(ret, "i:%d (%s) is_row_column_black(%d, %d) zero\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_row, data[i].expected_set_col);
                } else {
                    assert_zero(ret, "i:%d (%s) is_row_column_black(%d, %d) non-zero\n", i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_row, data[i].expected_set_col);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_dcontent_string_wrap(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        const char *data;
        const char *text;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_no_text_row;
        int expected_no_text_col;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, "12", "              E", 50, 1, 46, 92, 116, 104, 0 },
        /*  1*/ { BARCODE_CODE128, BOLD_TEXT, "12", "           E", 50, 1, 46, 92, 116, 104, 0 },
        /*  2*/ { BARCODE_CODE128, SMALL_TEXT, "12", "                   E", 50, 1, 46, 92, 112, 103, 0 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int text_bits_set, row, column;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        /* Cheat by overwriting text */
        strcpy((char *) symbol->text, data[i].text);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Print(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

        text_bits_set = 0;
        row = data[i].expected_no_text_row;
        for (column = data[i].expected_no_text_col; column < data[i].expected_no_text_col + 16; column++) {
            if (is_row_column_black(symbol, row, column)) {
                text_bits_set = 1;
                break;
            }
        }
        assert_zero(text_bits_set, "i:%d (%d) text_bits_set non-zero\n", i, data[i].symbology);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_code128_utf8(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        const char *data;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_text_row;
        int expected_text_col;
        int expected_text_len;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { "é", 50, 1, 57, 114, 116, 110, 53, 6 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int text_bits_set, row, column;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE128, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, BARCODE_CODE128, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0\n", i, BARCODE_CODE128, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, BARCODE_CODE128);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, BARCODE_CODE128, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, BARCODE_CODE128, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, BARCODE_CODE128, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, BARCODE_CODE128, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, BARCODE_CODE128, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Print(%d) ret %d != 0\n", i, BARCODE_CODE128, ret);
        assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

        text_bits_set = 0;
        row = data[i].expected_text_row;
        for (column = data[i].expected_text_col; column < data[i].expected_text_col + data[i].expected_text_len; column++) {
            if (is_row_column_black(symbol, row, column)) {
                text_bits_set++;
            }
        }
        assert_equal(text_bits_set, data[i].expected_text_len, "i:%d (%d) text_bits_set %d != expected_text_len %d\n", i, BARCODE_CODE128, text_bits_set, data[i].expected_text_len);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_scale(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_2;
        int border_width;
        int output_options;
        float height;
        float scale;
        const char *data;
        const char *composite;

        int ret_raster;
        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_set_row;
        int expected_set_rows;
        int expected_set_col;
        int expected_set_len;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE16K, -1, -1, -1, 0, 0, "1234567890123456789012345678901234567890", "", 0, 50, 5, 70, 162, 104, 0 /*set_row*/, 43, 30, 2 }, /* With no scaling */
        /*  1*/ { BARCODE_CODE16K, -1, -1, -1, 0, 0.5, "1234567890123456789012345678901234567890", "", 0, 50, 5, 70, 162 * 0.5, 104 * 0.5, 0 /*set_row*/, (43 * 0.5) + 1, 30 * 0.5, 2 * 0.5 },
        /*  2*/ { BARCODE_CODE16K, -1, -1, -1, 37, 0.5, "1234567890123456789012345678901234567890", "", 0, 35, 5, 70, 162 * 0.5, 37, 0 /*set_row*/, 16, 30 * 0.5, 2 * 0.5 }, /* Height specified */
        /*  3*/ { BARCODE_CODE49, -1, -1, -1, 0, 0, "A1234567890B", "", 0, 30, 3, 70, 162, 64, 0 /*set_row*/, 43, 134, 2 }, /* With no scaling */
        /*  4*/ { BARCODE_CODE49, -1, -1, -1, 0, 1.5, "A1234567890B", "", 0, 30, 3, 70, 162 * 1.5, 64 * 1.5, 0 /*set_row*/, 43 * 1.5 + 1, 134 * 1.5, 2 * 1.5 },
        /*  5*/ { BARCODE_CODE49, -1, -1, -1, 16, 1.5, "A1234567890B", "", 0, 16, 3, 70, 162 * 1.5, 54, 0 /*set_row*/, 37, 134 * 1.5, 2 * 1.5 }, /* Height specified */
        /*  6*/ { BARCODE_PHARMA_TWO, -1, -1, -1, 0, 0, "1234", "", 0, 10, 2, 13, 26, 20, 0 /*set_row*/, 20, 20, 2 }, /* With no scaling */
        /*  7*/ { BARCODE_PHARMA_TWO, -1, -1, -1, 0, 3, "1234", "", 0, 10, 2, 13, 26 * 3, 20 * 3, 0 /*set_row*/, 20 * 3, 20 * 3, 2 * 3 },
        /*  8*/ { BARCODE_PHARMA_TWO, -1, -1, -1, 13, 3, "1234", "", 0, 13, 2, 13, 26 * 3, 78, 0 /*set_row*/, 78, 20 * 3, 2 * 3 }, /* Height specified */
        /*  9*/ { BARCODE_PDF417, -1, -1, -1, 0, 0, "1", "", 0, 15, 5, 103, 206, 30, 0 /*set_row*/, 30, 170, 14 }, /* With no scaling */
        /* 10*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.6, "1", "", 0, 15, 5, 103, 206 * 0.6, 30 * 0.6, 0 /*set_row*/, 30 * 0.6, 170 * 0.6 + 1, 14 * 0.6 }, /* +1 set_col due to some scaling inversion difference */
        /* 11*/ { BARCODE_PDF417, -1, -1, -1, 0, 1.2, "1", "", 0, 15, 5, 103, 206 * 1.2, 30 * 1.2, 0 /*set_row*/, 30 * 1.2, 170 * 1.2 + 1, 14 * 1.2 }, /* +1 set_col due to some scaling inversion difference */
        /* 12*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.5, "1", "", 0, 15, 5, 103, 206 * 0.5, 30 * 0.5, 0 /*set_row*/, 30 * 0.5, 170 * 0.5, 14 * 0.5 },
        /* 13*/ { BARCODE_PDF417, -1, -1, -1, 0, 1.0, "1", "", 0, 15, 5, 103, 206 * 1.0, 30 * 1.0, 0 /*set_row*/, 30 * 1.0, 170 * 1.0, 14 * 1.0 },
        /* 14*/ { BARCODE_PDF417, -1, -1, -1, 0, 1.5, "1", "", 0, 15, 5, 103, 206 * 1.5, 30 * 1.5, 0 /*set_row*/, 30 * 1.5, 170 * 1.5, 14 * 1.5 },
        /* 15*/ { BARCODE_PDF417, -1, -1, -1, 0, 2.0, "1", "", 0, 15, 5, 103, 206 * 2.0, 30 * 2.0, 0 /*set_row*/, 30 * 2.0, 170 * 2.0, 14 * 2.0 },
        /* 16*/ { BARCODE_PDF417, -1, -1, -1, 0, 2.5, "1", "", 0, 15, 5, 103, 206 * 2.5, 30 * 2.5, 0 /*set_row*/, 30 * 2.5, 170 * 2.5, 14 * 2.5 },
        /* 17*/ { BARCODE_PDF417, -1, -1, -1, 0, 3.0, "1", "", 0, 15, 5, 103, 206 * 3.0, 30 * 3.0, 0 /*set_row*/, 30 * 3.0, 170 * 3.0, 14 * 3.0 },
        /* 18*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 0, "1", "", 0, 15, 5, 103, 218, 42, 0 /*set_row*/, 42, 176, 14 }, /* With no scaling */
        /* 19*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 0.6, "1", "", 0, 15, 5, 103, 218 * 0.6, 42 * 0.6, 0 /*set_row*/, 42 * 0.6, 176 * 0.6 + 1, 14 * 0.6 }, /* +1 set_col due to some scaling inversion difference */
        /* 20*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 1.6, "1", "", 0, 15, 5, 103, 218 * 1.6, 42 * 1.6, 0 /*set_row*/, 42 * 1.6, 176 * 1.6 + 1, 14 * 1.6 }, /* +1 set_col due to some scaling inversion difference */
        /* 21*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 1.5, "1", "", 0, 15, 5, 103, 218 * 1.5, 42 * 1.5, 0 /*set_row*/, 42 * 1.5, 176 * 1.5, 14 * 1.5 },
        /* 22*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 2.5, "1", "", 0, 15, 5, 103, 218 * 2.5, 42 * 2.5, 0 /*set_row*/, 42 * 2.5, 176 * 2.5, 14 * 2.5 },
        /* 23*/ { BARCODE_PDF417, -1, 3, OUT_BUFFER_INTERMEDIATE, 0, 1.3, "1", "", 0, 15, 5, 103, 206 * 1.3, 30 * 1.3, 0 /*set_row*/, 30 * 1.3, 170 * 1.3, 14 * 1.3 },
        /* 24*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.5, "1", "", 0, 15, 5, 103, 206 * 0.5, 30 * 0.5, 0 /*set_row*/, 30 * 0.5, 170 * 0.5, 14 * 0.5 },
        /* 25*/ { BARCODE_PDF417, -1, -1, -1, 1, 0.5, "1", "", 0, 5, 5, 103, 206 * 0.5, 5, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 26*/ { BARCODE_PDF417, -1, -1, -1, 5, 0.5, "1", "", 0, 5, 5, 103, 206 * 0.5, 5, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 27*/ { BARCODE_PDF417, -1, -1, -1, 6, 0.5, "1", "", 0, 5, 5, 103, 206 * 0.5, 5, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 28*/ { BARCODE_PDF417, -1, -1, -1, 7, 0.5, "1", "", 0, 5, 5, 103, 206 * 0.5, 5, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 29*/ { BARCODE_PDF417, -1, -1, -1, 8, 0.5, "1", "", 0, 10, 5, 103, 206 * 0.5, 10, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 30*/ { BARCODE_PDF417, -1, -1, -1, 9, 0.5, "1", "", 0, 10, 5, 103, 206 * 0.5, 10, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 31*/ { BARCODE_PDF417, -1, -1, -1, 10, 0.5, "1", "", 0, 10, 5, 103, 206 * 0.5, 10, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 32*/ { BARCODE_PDF417, -1, -1, -1, 11, 0.5, "1", "", 0, 10, 5, 103, 206 * 0.5, 10, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 33*/ { BARCODE_PDF417, -1, -1, -1, 12, 0.5, "1", "", 0, 10, 5, 103, 206 * 0.5, 10, 0 /*set_row*/, 5, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 34*/ { BARCODE_PDF417, -1, -1, -1, 13, 0.5, "1", "", 0, 15, 5, 103, 206 * 0.5, 15, 0 /*set_row*/, 15, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 35*/ { BARCODE_PDF417, -1, -1, -1, 14, 0.5, "1", "", 0, 15, 5, 103, 206 * 0.5, 15, 0 /*set_row*/, 15, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 36*/ { BARCODE_PDF417, -1, -1, -1, 15, 0.5, "1", "", 0, 15, 5, 103, 206 * 0.5, 15, 0 /*set_row*/, 15, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 37*/ { BARCODE_PDF417, -1, -1, -1, 16, 0.5, "1", "", 0, 15, 5, 103, 206 * 0.5, 15, 0 /*set_row*/, 15, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 38*/ { BARCODE_PDF417, -1, -1, -1, 17, 0.5, "1", "", 0, 15, 5, 103, 206 * 0.5, 15, 0 /*set_row*/, 15, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 39*/ { BARCODE_PDF417, -1, -1, -1, 18, 0.5, "1", "", 0, 20, 5, 103, 206 * 0.5, 20, 0 /*set_row*/, 20, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 40*/ { BARCODE_PDF417, -1, -1, -1, 19, 0.5, "1", "", 0, 20, 5, 103, 206 * 0.5, 20, 0 /*set_row*/, 20, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 41*/ { BARCODE_PDF417, -1, -1, -1, 20, 0.5, "1", "", 0, 20, 5, 103, 206 * 0.5, 20, 0 /*set_row*/, 20, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 42*/ { BARCODE_PDF417, -1, -1, -1, 21, 0.5, "1", "", 0, 20, 5, 103, 206 * 0.5, 20, 0 /*set_row*/, 20, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 43*/ { BARCODE_PDF417, -1, -1, -1, 38, 0.5, "1", "", 0, 40, 5, 103, 206 * 0.5, 40, 0 /*set_row*/, 40, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 44*/ { BARCODE_PDF417, -1, -1, -1, 39, 0.5, "1", "", 0, 40, 5, 103, 206 * 0.5, 40, 0 /*set_row*/, 40, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 45*/ { BARCODE_PDF417, -1, -1, -1, 40, 0.5, "1", "", 0, 40, 5, 103, 206 * 0.5, 40, 0 /*set_row*/, 40, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 46*/ { BARCODE_PDF417, -1, -1, -1, 41, 0.5, "1", "", 0, 40, 5, 103, 206 * 0.5, 40, 0 /*set_row*/, 40, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 47*/ { BARCODE_PDF417, -1, -1, -1, 42, 0.5, "1", "", 0, 40, 5, 103, 206 * 0.5, 40, 0 /*set_row*/, 40, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 48*/ { BARCODE_PDF417, -1, -1, -1, 43, 0.5, "1", "", 0, 45, 5, 103, 206 * 0.5, 45, 0 /*set_row*/, 45, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 49*/ { BARCODE_PDF417, -1, -1, -1, 44, 0.5, "1", "", 0, 45, 5, 103, 206 * 0.5, 45, 0 /*set_row*/, 45, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 50*/ { BARCODE_PDF417, -1, -1, -1, 45, 0.5, "1", "", 0, 45, 5, 103, 206 * 0.5, 45, 0 /*set_row*/, 45, 170 * 0.5, 14 * 0.5 }, /* Height specified */
        /* 51*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, 0, "123456789012", "", 0, 50, 1, 79, 158, 116, 104 /*set_row*/, 114, 20, 2 }, /* With no scaling */
        /* 52*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, 1.5, "123456789012", "", 0, 50, 1, 79, 158 * 1.5, 116 * 1.5, 104 * 1.5 /*set_row*/, 114 * 1.5, 20 * 1.5, 1 * 1.5 },
        /* 53*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, 2.0, "123456789012", "", 0, 50, 1, 79, 158 * 2.0, 116 * 2.0, 104 * 2.0 /*set_row*/, 114 * 2.0, 20 * 2.0 + 1, 1 * 2.0 + 1 },
        /* 54*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, 3.5, "123456789012", "", 0, 50, 1, 79, 158 * 3.5, 116 * 3.5, 104 * 3.5 /*set_row*/, 114 * 3.5, 20 * 3.5 + 1, 1 * 3.5 + 0.5 },
        /* 55*/ { BARCODE_UPCA, -1, -1, -1, 0, 0, "12345678904", "", 0, 50, 1, 95, 226, 116, 104 /*set_row*/, 114, 5, 2 }, /* With no scaling */
        /* 56*/ { BARCODE_UPCA, -1, -1, -1, 0, 2.5, "12345678904", "", 0, 50, 1, 95, 226 * 2.5, 116 * 2.5, 104 * 2.5 /*set_row*/, 114 * 2.5, 5 * 2.5 + 1.5, 2 * 2.5 },
        /* 57*/ { BARCODE_UPCA, -1, -1, -1, 0, 4.5, "12345678904", "", 0, 50, 1, 95, 226 * 4.5, 116 * 4.5, 104 * 4.5 /*set_row*/, 114 * 4.5, 5 * 4.5 + 1.5, 2 * 4.5 },
        /* 58*/ { BARCODE_CODABLOCKF, -1, -1, -1, 0, 0, "1234567890123456", "", 0, 30, 3, 101, 242, 64, 0 /*set_row*/, 64, 42, 2 }, /* With no scaling */
        /* 59*/ { BARCODE_CODABLOCKF, -1, -1, -1, 0, 0.5, "1234567890123456", "", 0, 30, 3, 101, 242 * 0.5, 64 * 0.5, 0 /*set_row*/, 64 * 0.5, 42 * 0.5, 2 * 0.5 },
        /* 60*/ { BARCODE_CODABLOCKF, -1, -1, -1, 34, 0.5, "1234567890123456", "", 0, 33, 3, 101, 242 * 0.5, 35, 0 /*set_row*/, 35, 42 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 61*/ { BARCODE_CODABLOCKF, -1, -1, -1, 47, 0.5, "1234567890123456", "", 0, 48, 3, 101, 242 * 0.5, 50, 0 /*set_row*/, 50, 42 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 62*/ { BARCODE_DBAR_OMNSTK, -1, -1, 0, 0, 0, "1", "", 0, 69, 5, 50, 100, 138, 0 /*set_row*/, 66, 16, 2 }, /* With no scaling */
        /* 63*/ { BARCODE_DBAR_OMNSTK, -1, -1, 0, 0, 0.5, "1", "", 0, 69, 5, 50, 100 * 0.5, 138 * 0.5, 0 /*set_row*/, 66 * 0.5, 16 * 0.5, 2 * 0.5 },
        /* 64*/ { BARCODE_DBAR_OMNSTK, -1, -1, 0, 18, 0.5, "1", "", 0, 19, 5, 50, 100 * 0.5, 19, 0 /*set_row*/, 8, 16 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 65*/ { BARCODE_DBAR_EXPSTK, 1, -1, 0, 0, 0, "[01]12345678901231", "", 0, 108, 9, 53, 106, 216, 0 /*set_row*/, 68, 36, 2 }, /* With no scaling */
        /* 66*/ { BARCODE_DBAR_EXPSTK, 1, -1, 0, 0, 0.5, "[01]12345678901231", "", 0, 108, 9, 53, 106 * 0.5, 216 * 0.5, 0 /*set_row*/, 68 * 0.5, 36 * 0.5, 2 * 0.5 },
        /* 67*/ { BARCODE_DBAR_EXPSTK, 1, -1, 0, 10, 0.5, "[01]12345678901231", "", 0, 9, 9, 53, 106 * 0.5, 9, 0 /*set_row*/, 1, 36 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 68*/ { BARCODE_DBAR_EXPSTK, 1, -1, 0, 99, 0.5, "[01]12345678901231", "", 0, 99, 9, 53, 106 * 0.5, 99, 0 /*set_row*/, 31, 36 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 69*/ { BARCODE_DBAR_EXPSTK, 1, -1, 0, 100, 0.5, "[01]12345678901231", "", 0, 99, 9, 53, 106 * 0.5, 99, 0 /*set_row*/, 31, 36 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 70*/ { BARCODE_DBAR_EXPSTK, 1, -1, 0, 101, 0.5, "[01]12345678901231", "", 0, 102, 9, 53, 106 * 0.5, 102, 0 /*set_row*/, 32, 36 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 71*/ { BARCODE_DBAR_EXPSTK, 1, -1, 0, 102, 0.5, "[01]12345678901231", "", 0, 102, 9, 53, 106 * 0.5, 102, 0 /*set_row*/, 32, 36 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 72*/ { BARCODE_DBAR_EXPSTK, 1, -1, 0, 103, 0.5, "[01]12345678901231", "", 0, 102, 9, 53, 106 * 0.5, 102, 0 /*set_row*/, 32, 36 * 0.5, 2 * 0.5 }, /* Height specified */
        /* 73*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 0, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 134, 116, 104 /*set_row*/, 116, 5, 2 }, /* With no scaling */
        /* 74*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 0.5, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 134 * 0.5, 55, 18 /*set_row*/, 55, 59, 2 * 0.5 },
        /* 75*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 2.0, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 134 * 2, 116 * 2, 104 * 2 + 1 /*set_row*/, 116 * 2, 5 * 2, 2 * 2 }, /* +1 set_row */
        /* 76*/ { BARCODE_UPCE_CC, -1, -1, -1, 2, 0.5, "1234567", "[17]010615[10]A123456\"", 0, 19, 10, 55, 134 * 0.5, 24, 16 /*set_row*/, 24, 59, 2 * 0.5 }, /* Height specified */
        /* 77*/ { BARCODE_MAXICODE, -1, -1, -1, 0, 0, "1234567890", "", 0, 165, 33, 30, 299, 298, 3 /*set_row*/, 7, 10, 9 }, /* With no scaling */
        /* 78*/ { BARCODE_MAXICODE, -1, -1, -1, 0, 0.1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 165, 33, 30, 60, 65, 0 /*set_row*/, 1, 3, 1 },
        /* 79*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 0, "12345", "", 0, 12, 2, 63, 146, 30, 3 /*set_row*/, 27, 10, 2 }, /* With no scaling */
        /* 80*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 0.1, "12345", "", 0, 12, 2, 63, 146 * 0.5, 30 * 0.5 - 1, 3 * 0.5 /*set_row*/, 27 * 0.5, 10 * 0.5, 2 * 0.5 }, /* -1 height due to yoffset/boffset flooring */
        /* 81*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 0.9, "12345", "", 0, 12, 2, 63, 146 * 0.9, 30 * 0.9, 3 * 0.9 + 1 /*set_row*/, 27 * 0.9, 10 * 0.9, 2 * 0.9 + 1 }, /* +1's due to interpolation */
        /* 82*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 2.3, "12345", "", 0, 12, 2, 63, 146 * 2.3, 30 * 2.3, 3 * 2.3 + 1 /*set_row*/, 27 * 2.3 - 1, 10 * 2.3, 2 * 2.3 + 1 }, /* -1/+1's due to interpolation */
        /* 83*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 3.1, "12345", "", 0, 12, 2, 63, 146 * 3.1, 30 * 3.1, 3 * 3.1 + 1 /*set_row*/, 27 * 3.1, 10 * 3.1, 2 * 3.2 + 1 }, /* +1's due to interpolation */
        /* 84*/ { BARCODE_ITF14, -1, 4, BARCODE_BIND, 61.8, 0, "12345", "", 0, 62, 1, 135, 310, 156, 8 /*set_row*/, 132, 20, 2 }, /* With no scaling */
        /* 85*/ { BARCODE_ITF14, -1, 4, BARCODE_BIND, 61.8, 2, "12345", "", 0, 61.75, 1, 135, 310 * 2, 156 * 2 - 1, 8 * 2 /*set_row*/, 132 * 2 - 1, 20 * 2, 2 * 2 }, /* -1's due to height rounding */
        /* 86*/ { BARCODE_ITF14, -1, 4, BARCODE_BIND, 61.8, 2.1, "12345", "", 0, 62, 1, 135, 310 * 2.1, 156 * 2.1, 8 * 2.1 /*set_row*/, 132 * 2.1, 20 * 2.1, 2 * 2.1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int row, column;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }
        if (data[i].height) {
            symbol->height = data[i].height;
        }
        if (data[i].scale) {
            symbol->scale = data[i].scale;
        }
        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, text, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_nonzero(ret < ZINT_ERROR, "i:%d ZBarcode_Encode(%s) ret %d >= ZINT_ERROR (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Buffer(%s) ret %d != %d (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret_raster, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%s) symbol->bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Print(%s) ret %d != %d (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret_raster, symbol->errtxt);
        assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

        assert_nonzero(symbol->bitmap_height >= data[i].expected_set_rows, "i:%d (%s) symbol->bitmap_height %d < expected_set_rows %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_set_rows);
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row, "i:%d (%s) expected_set_rows %d <= expected_set_row %d\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_rows, data[i].expected_set_row);
        for (row = data[i].expected_set_row; row < data[i].expected_set_rows; row++) {
            int bits_set = 0;
            for (column = data[i].expected_set_col; column < data[i].expected_set_col + data[i].expected_set_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    bits_set++;
                }
            }
            assert_equal(bits_set, data[i].expected_set_len, "i:%d (%s) row %d,%d bits_set %d != expected_set_len %d\n",
                        i, testUtilBarcodeName(data[i].symbology), row, data[i].expected_set_col, bits_set, data[i].expected_set_len);
        }
        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_guard_descent(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        float guard_descent;
        const char *data;

        int ret_raster;
        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;

        int expected_set;
        int expected_set_row;
        int expected_set_rows;
        int expected_set_col;
        int expected_set_len;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 134, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 18, 2 },
        /*  1*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 134, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 18, 2 },
        /*  2*/ { BARCODE_UPCE, 0, "1234567", 0, 50, 1, 51, 134, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 18, 2 },
        /*  3*/ { BARCODE_UPCE, 1.5, "1234567", 0, 50, 1, 51, 134, 116, 1 /*set*/, 100 /*set_row*/, 100 + 3, 18, 2 },
        /*  4*/ { BARCODE_UPCE, 1.5, "1234567", 0, 50, 1, 51, 134, 116, 0 /*set*/, 100 + 3 /*set_row*/, 100 + 16, 18, 2 },
        /*  5*/ { BARCODE_UPCE, 6, "1234567", 0, 50, 1, 51, 134, 116, 1 /*set*/, 100 /*set_row*/, 100 + 12, 18, 2 },
        /*  6*/ { BARCODE_UPCE, 6, "1234567", 0, 50, 1, 51, 134, 116, 0 /*set*/, 100 + 12 /*set_row*/, 100 + 16, 18, 2 },
        /*  7*/ { BARCODE_UPCE, 8, "1234567", 0, 50, 1, 51, 134, 116, 1 /*set*/, 100 /*set_row*/, 100 + 16, 18, 2 },
        /*  8*/ { BARCODE_UPCE, 8.5, "1234567", 0, 50, 1, 51, 134, 117, 1 /*set*/, 100 /*set_row*/, 100 + 17, 18, 2 },
        /*  9*/ { BARCODE_UPCE, 20, "1234567", 0, 50, 1, 51, 134, 140, 1 /*set*/, 100 /*set_row*/, 100 + 40, 18, 2 },
        /* 10*/ { BARCODE_UPCE, -1, "1234567+12", 0, 50, 1, 78, 184, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 18, 2 },
        /* 11*/ { BARCODE_UPCE, -1, "1234567+12", 0, 50, 1, 78, 184, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 134, 2 },
        /* 12*/ { BARCODE_UPCE, -1, "1234567+12", 0, 50, 1, 78, 184, 116, 0 /*set*/, 110 /*set_row*/, 110 + 6, 134, 2 },
        /* 13*/ { BARCODE_UPCE, 9, "1234567+12", 0, 50, 1, 78, 184, 118, 1 /*set*/, 110 /*set_row*/, 110 + 8, 134, 2 },
        /* 14*/ { BARCODE_UPCA, -1, "12345678901", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 206, 2 },
        /* 15*/ { BARCODE_UPCA, 0, "12345678901", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 /*set_row*/, 100 + 10, 206, 2 },
        /* 16*/ { BARCODE_UPCA, 2.75, "12345678901", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 5, 206, 2 },
        /* 17*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 206, 2 },
        /* 18*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 262, 4 },
        /* 19*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 116, 0 /*set*/, 110 /*set_row*/, 110 + 6, 262, 4 },
        /* 20*/ { BARCODE_UPCA, 10, "12345678901+12", 0, 50, 1, 124, 276, 120, 1 /*set*/, 110 /*set_row*/, 110 + 10, 262, 4 },
        /* 21*/ { BARCODE_EAN13, -1, "123456789012", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 114, 2 },
        /* 22*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 114, 2 },
        /* 23*/ { BARCODE_EAN13, -1, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 114, 2 },
        /* 24*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 114, 2 },
        /* 25*/ { BARCODE_EAN13, 0, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 114, 2 },
        /* 26*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 114, 2 },
        /* 27*/ { BARCODE_EAN13, 2, "123456789012", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 4, 114, 2 },
        /* 28*/ { BARCODE_EANX, 2, "123456789012", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 4, 114, 2 },
        /* 29*/ { BARCODE_EAN13, 2, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 + 4 /*set_row*/, 100 + 16, 114, 2 },
        /* 30*/ { BARCODE_EANX, 2, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 + 4 /*set_row*/, 100 + 16, 114, 2 },
        /* 31*/ { BARCODE_EAN13, -1, "123456789012+12345", 0, 50, 1, 149, 330, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 308, 4 },
        /* 32*/ { BARCODE_EANX, -1, "123456789012+12345", 0, 50, 1, 149, 330, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 308, 4 },
        /* 33*/ { BARCODE_EAN13, -1, "123456789012+12345", 0, 50, 1, 149, 330, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 308, 4 },
        /* 34*/ { BARCODE_EANX, -1, "123456789012+12345", 0, 50, 1, 149, 330, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 308, 4 },
        /* 35*/ { BARCODE_EAN13, 0, "123456789012+12345", 0, 50, 1, 149, 330, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 308, 4 },
        /* 36*/ { BARCODE_EANX, 0, "123456789012+12345", 0, 50, 1, 149, 330, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 308, 4 },
        /* 37*/ { BARCODE_EAN13, 18, "123456789012+12345", 0, 50, 1, 149, 330, 136, 1 /*set*/, 100 /*set_row*/, 100 + 36, 308, 4 },
        /* 38*/ { BARCODE_EANX, 18, "123456789012+12345", 0, 50, 1, 149, 330, 136, 1 /*set*/, 100 /*set_row*/, 100 + 36, 308, 4 },
        /* 39*/ { BARCODE_ISBNX, -1, "123456789", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 206, 2 },
        /* 40*/ { BARCODE_ISBNX, -1, "123456789", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 206, 2 },
        /* 41*/ { BARCODE_ISBNX, 0, "123456789", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 206, 2 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int row, column;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].guard_descent != -1.0f) {
            symbol->guard_descent = data[i].guard_descent;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Buffer(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Print(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

        assert_nonzero(symbol->bitmap_height >= data[i].expected_set_rows, "i:%d (%s) symbol->bitmap_height %d < expected_set_rows %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_set_rows);
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row, "i:%d (%s) expected_set_rows %d < expected_set_row %d\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_rows, data[i].expected_set_row);
        for (row = data[i].expected_set_row; row < data[i].expected_set_rows; row++) {
            int bits_set = 0;
            for (column = data[i].expected_set_col; column < data[i].expected_set_col + data[i].expected_set_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    bits_set++;
                }
            }
            if (data[i].expected_set) {
                assert_equal(bits_set, data[i].expected_set_len, "i:%d (%d) row %d bits_set %d != expected_set_len %d\n", i, data[i].symbology, row, bits_set, data[i].expected_set_len);
            } else {
                assert_zero(bits_set, "i:%d (%d) row %d bits_set %d != 0\n", i, data[i].symbology, row, bits_set);
            }
        }
        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_quiet_zones(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        int option_1;
        int option_2;
        int show_hrt;
        const char *data;
        const char *composite;

        int ret_raster;
        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;

        int expected_set;
        int expected_set_row;
        int expected_set_rows;
        int expected_set_col;
        int expected_set_len;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, -1, -1, "1234", "", 0, 50, 1, 62, 124, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*  1*/ { BARCODE_CODE11, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 62, 164, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*  2*/ { BARCODE_CODE11, BARCODE_QUIET_ZONES | BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 62, 124, 116, 1 /*set*/, 0, 100, 0, 2 }, /* BARCODE_NO_QUIET_ZONES trumps BARCODE_QUIET_ZONES */
        /*  3*/ { BARCODE_C25STANDARD, -1, -1, -1, -1, "1234", "", 0, 50, 1, 57, 114, 116, 1 /*set*/, 0, 100, 0, 8 },
        /*  4*/ { BARCODE_C25STANDARD, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 57, 154, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*  5*/ { BARCODE_C25INTER, -1, -1, -1, -1, "1234", "", 0, 50, 1, 45, 90, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*  6*/ { BARCODE_C25INTER, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 45, 130, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*  7*/ { BARCODE_C25IATA, -1, -1, -1, -1, "1234", "", 0, 50, 1, 65, 130, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*  8*/ { BARCODE_C25IATA, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 65, 170, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*  9*/ { BARCODE_C25LOGIC, -1, -1, -1, -1, "1234", "", 0, 50, 1, 49, 98, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 10*/ { BARCODE_C25LOGIC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 49, 138, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 11*/ { BARCODE_C25IND, -1, -1, -1, -1, "1234", "", 0, 50, 1, 75, 150, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 12*/ { BARCODE_C25IND, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 75, 190, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 13*/ { BARCODE_CODE39, -1, -1, -1, -1, "1234", "", 0, 50, 1, 77, 154, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 14*/ { BARCODE_CODE39, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 77, 194, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 15*/ { BARCODE_EXCODE39, -1, -1, -1, -1, "1234", "", 0, 50, 1, 77, 154, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 16*/ { BARCODE_EXCODE39, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 77, 194, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 17*/ { BARCODE_EAN13, -1, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 110, 212, 14 }, /* EAN-13 */
        /* 18*/ { BARCODE_EANX, -1, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 110, 212, 14 }, /* EAN-13 */
        /* 19*/ { BARCODE_EAN13, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 110, 212, 14 },
        /* 20*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 110, 212, 14 },
        /* 21*/ { BARCODE_EAN13, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 212, 116, 1 /*set*/, 0, 110, 210, 2 },
        /* 22*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 1, 95, 212, 116, 1 /*set*/, 0, 110, 210, 2 },
        /* 23*/ { BARCODE_EAN13, -1, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 212, 14 }, /* Hide text */
        /* 24*/ { BARCODE_EANX, -1, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 212, 14 }, /* Hide text */
        /* 25*/ { BARCODE_EAN13, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 212, 14 }, /* Hide text */
        /* 26*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 212, 14 }, /* Hide text */
        /* 27*/ { BARCODE_EAN13, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 190, 110, 1 /*set*/, 0, 110, 188, 2 }, /* Hide text */
        /* 28*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 1, 95, 190, 110, 1 /*set*/, 0, 110, 188, 2 }, /* Hide text */
        /* 29*/ { BARCODE_EAN13, -1, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 276, 116, 0 /*set*/, 16, 110, 266, 10 },
        /* 30*/ { BARCODE_EANX, -1, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 276, 116, 0 /*set*/, 16, 110, 266, 10 },
        /* 31*/ { BARCODE_EAN13, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 276, 116, 0 /*set*/, 16, 110, 266, 10 },
        /* 32*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 276, 116, 0 /*set*/, 16, 110, 266, 10 },
        /* 33*/ { BARCODE_EAN13, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 266, 116, 1 /*set*/, 16, 110, 262, 4 },
        /* 34*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012+12", "", 0, 50, 1, 122, 266, 116, 1 /*set*/, 16, 110, 262, 4 },
        /* 35*/ { BARCODE_EAN13, -1, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, /* Hide text */
        /* 36*/ { BARCODE_EANX, -1, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, /* Hide text */
        /* 37*/ { BARCODE_EAN13, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, /* Hide text */
        /* 38*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, /* Hide text */
        /* 39*/ { BARCODE_EAN13, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 244, 110, 1 /*set*/, 16, 110, 240, 4 }, /* Hide text */
        /* 40*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012+12", "", 0, 50, 1, 122, 244, 110, 1 /*set*/, 16, 110, 240, 4 }, /* Hide text */
        /* 41*/ { BARCODE_EANX_CHK, -1, -1, -1, -1, "0234567890129+12345", "", 0, 50, 1, 149, 330, 116, 0 /*set*/, 16, 110, 320, 10 },
        /* 42*/ { BARCODE_EANX_CHK, BARCODE_QUIET_ZONES, -1, -1, -1, "0234567890129+12345", "", 0, 50, 1, 149, 330, 116, 0 /*set*/, 16, 110, 320, 10 },
        /* 43*/ { BARCODE_EANX_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "0234567890129+12345", "", 0, 50, 1, 149, 320, 116, 1 /*set*/, 16, 110, 318, 2 },
        /* 44*/ { BARCODE_EAN8, -1, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 162, 116, 0 /*set*/, 0, 100, 0, 14 }, /* EAN-8 */
        /* 45*/ { BARCODE_EANX, -1, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 162, 116, 0 /*set*/, 0, 100, 0, 14 }, /* EAN-8 */
        /* 46*/ { BARCODE_EAN8, BARCODE_QUIET_ZONES, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 162, 116, 0 /*set*/, 0, 100, 0, 14 }, /* EAN-8 */
        /* 47*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 162, 116, 0 /*set*/, 0, 100, 0, 14 }, /* EAN-8 */
        /* 48*/ { BARCODE_EAN8, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 134, 116, 1 /*set*/, 0, 100, 0, 2 }, /* EAN-8 */
        /* 49*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "0234567", "", 0, 50, 1, 67, 134, 116, 1 /*set*/, 0, 100, 0, 2 }, /* EAN-8 */
        /* 50*/ { BARCODE_EAN_5ADDON, -1, -1, -1, -1, "02345", "", 0, 50, 1, 47, 104, 116, 0 /*set*/, 0, 116, 94, 10 }, /* EAN-5 */
        /* 51*/ { BARCODE_EANX, -1, -1, -1, -1, "02345", "", 0, 50, 1, 47, 104, 116, 0 /*set*/, 0, 116, 94, 10 }, /* EAN-5 */
        /* 52*/ { BARCODE_EAN_5ADDON, BARCODE_QUIET_ZONES, -1, -1, -1, "02345", "", 0, 50, 1, 47, 104, 116, 0 /*set*/, 0, 116, 94, 10 }, /* EAN-5 */
        /* 53*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "02345", "", 0, 50, 1, 47, 104, 116, 0 /*set*/, 0, 116, 94, 10 }, /* EAN-5 */
        /* 54*/ { BARCODE_EAN_5ADDON, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "02345", "", 0, 50, 1, 47, 94, 116, 1 /*set*/, 16, 116, 92, 2 }, /* EAN-5 */
        /* 55*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "02345", "", 0, 50, 1, 47, 94, 116, 1 /*set*/, 16, 116, 92, 2 }, /* EAN-5 */
        /* 56*/ { BARCODE_EAN_2ADDON, -1, -1, -1, -1, "02", "", 0, 50, 1, 20, 50, 116, 0 /*set*/, 0, 100, 40, 10 }, /* EAN-2 */
        /* 57*/ { BARCODE_EANX, -1, -1, -1, -1, "02", "", 0, 50, 1, 20, 50, 116, 0 /*set*/, 0, 100, 40, 10 }, /* EAN-2 */
        /* 58*/ { BARCODE_EAN_2ADDON, BARCODE_QUIET_ZONES, -1, -1, -1, "02", "", 0, 50, 1, 20, 50, 116, 0 /*set*/, 0, 116, 40, 10 }, /* EAN-2 */
        /* 59*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, -1, "02", "", 0, 50, 1, 20, 50, 116, 0 /*set*/, 0, 116, 40, 10 }, /* EAN-2 */
        /* 60*/ { BARCODE_EAN_2ADDON, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "02", "", 0, 50, 1, 20, 40, 116, 1 /*set*/, 16, 116, 36, 4 }, /* EAN-2 */
        /* 61*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "02", "", 0, 50, 1, 20, 40, 116, 1 /*set*/, 16, 116, 36, 4 }, /* EAN-2 */
        /* 62*/ { BARCODE_GS1_128, -1, -1, -1, -1, "[20]02", "", 0, 50, 1, 68, 136, 116, 1 /*set*/, 0, 100, 0, 4 },
        /* 63*/ { BARCODE_GS1_128, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 50, 1, 68, 176, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 64*/ { BARCODE_CODABAR, -1, -1, -1, -1, "A0B", "", 0, 50, 1, 32, 64, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 65*/ { BARCODE_CODABAR, BARCODE_QUIET_ZONES, -1, -1, -1, "A0B", "", 0, 50, 1, 32, 104, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 66*/ { BARCODE_CODE128, -1, -1, -1, -1, "1234", "", 0, 50, 1, 57, 114, 116, 1 /*set*/, 0, 100, 0, 4 },
        /* 67*/ { BARCODE_CODE128, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 57, 154, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 68*/ { BARCODE_DPLEIT, -1, -1, -1, -1, "1234", "", 0, 72, 1, 135, 270, 160, 1 /*set*/, 0, 100, 0, 2 },
        /* 69*/ { BARCODE_DPLEIT, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 72, 1, 135, 310, 160, 0 /*set*/, 0, 100, 0, 20 },
        /* 70*/ { BARCODE_DPIDENT, -1, -1, -1, -1, "1234", "", 0, 72, 1, 117, 234, 160, 1 /*set*/, 0, 100, 0, 2 },
        /* 71*/ { BARCODE_DPIDENT, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 72, 1, 117, 274, 160, 0 /*set*/, 0, 100, 0, 20 },
        /* 72*/ { BARCODE_CODE16K, -1, -1, -1, -1, "1234", "", 0, 20, 2, 70, 162, 44, 0 /*set*/, 2, 20, 0, 20 },
        /* 73*/ { BARCODE_CODE16K, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 70, 162, 44, 0 /*set*/, 2, 20, 0, 20 },
        /* 74*/ { BARCODE_CODE16K, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 70, 140, 44, 1 /*set*/, 2, 20, 0, 6 },
        /* 75*/ { BARCODE_CODE49, -1, -1, -1, -1, "1234", "", 0, 20, 2, 70, 162, 44, 0 /*set*/, 2, 20, 0, 20 },
        /* 76*/ { BARCODE_CODE49, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 70, 162, 44, 0 /*set*/, 2, 20, 0, 20 },
        /* 77*/ { BARCODE_CODE49, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 70, 140, 44, 1 /*set*/, 2, 20, 0, 2 },
        /* 78*/ { BARCODE_CODE93, -1, -1, -1, -1, "1234", "", 0, 50, 1, 73, 146, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 79*/ { BARCODE_CODE93, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 73, 186, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 80*/ { BARCODE_FLAT, -1, -1, -1, -1, "1234", "", 0, 50, 1, 36, 72, 100, 1 /*set*/, 0, 100, 0, 2 },
        /* 81*/ { BARCODE_FLAT, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 36, 72, 100, 1 /*set*/, 0, 100, 0, 2 },
        /* 82*/ { BARCODE_FLAT, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 36, 72, 100, 1 /*set*/, 0, 100, 0, 2 },
        /* 83*/ { BARCODE_DBAR_OMN, -1, -1, -1, -1, "1234", "", 0, 50, 1, 96, 192, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 84*/ { BARCODE_DBAR_OMN, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 96, 192, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 85*/ { BARCODE_DBAR_OMN, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 96, 192, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 86*/ { BARCODE_DBAR_LTD, -1, -1, -1, -1, "1234", "", 0, 50, 1, 79, 158, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 87*/ { BARCODE_DBAR_LTD, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 79, 158, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 88*/ { BARCODE_DBAR_LTD, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 79, 158, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 89*/ { BARCODE_DBAR_EXP, -1, -1, -1, -1, "[20]02", "", 0, 34, 1, 102, 204, 84, 0 /*set*/, 0, 84, 0, 2 },
        /* 90*/ { BARCODE_DBAR_EXP, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 34, 1, 102, 204, 84, 0 /*set*/, 0, 84, 0, 2 },
        /* 91*/ { BARCODE_DBAR_EXP, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 34, 1, 102, 204, 84, 0 /*set*/, 0, 84, 0, 2 },
        /* 92*/ { BARCODE_TELEPEN, -1, -1, -1, -1, "1234", "", 0, 50, 1, 112, 224, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 93*/ { BARCODE_TELEPEN, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 112, 264, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 94*/ { BARCODE_UPCA, -1, -1, -1, -1, "01457137763", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 100, 0, 18 },
        /* 95*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, -1, "01457137763", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 100, 0, 18 },
        /* 96*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "01457137763", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 100, 0, 18 },
        /* 97*/ { BARCODE_UPCA, -1, -1, -1, 0, "01457137763", "", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 0, 18 }, /* Hide text */
        /* 98*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, 0, "01457137763", "", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 0, 18 }, /* Hide text */
        /* 99*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "01457137763", "", 0, 50, 1, 95, 190, 110, 1 /*set*/, 0, 110, 0, 2 }, /* Hide text */
        /*100*/ { BARCODE_UPCA, -1, -1, -1, -1, "01457137763+12", "", 0, 50, 1, 124, 276, 116, 0 /*set*/, 16, 100, 266, 10 },
        /*101*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, -1, "01457137763+12", "", 0, 50, 1, 124, 276, 116, 0 /*set*/, 16, 100, 266, 10 },
        /*102*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "01457137763+12", "", 0, 50, 1, 124, 266, 116, 1 /*set*/, 16, 100, 262, 4 },
        /*103*/ { BARCODE_UPCA, -1, -1, -1, 0, "01457137763+12", "", 0, 50, 1, 124, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, /* Hide text */
        /*104*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, 0, "01457137763+12", "", 0, 50, 1, 124, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, /* Hide text */
        /*105*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "01457137763+12", "", 0, 50, 1, 124, 248, 110, 1 /*set*/, 16, 100, 244, 4 }, /* Hide text */
        /*106*/ { BARCODE_UPCA_CHK, -1, -1, -1, -1, "014571377638+12345", "", 0, 50, 1, 151, 330, 116, 0 /*set*/, 16, 100, 320, 10 },
        /*107*/ { BARCODE_UPCA_CHK, BARCODE_QUIET_ZONES, -1, -1, -1, "014571377638+12345", "", 0, 50, 1, 151, 330, 116, 0 /*set*/, 16, 100, 320, 10 },
        /*108*/ { BARCODE_UPCA_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "014571377638+12345", "", 0, 50, 1, 151, 320, 116, 1 /*set*/, 16, 100, 318, 2 },
        /*109*/ { BARCODE_UPCA_CHK, -1, -1, -1, 0, "014571377638+12345", "", 0, 50, 1, 151, 330, 110, 0 /*set*/, 16, 110, 320, 10 }, /* Hide text */
        /*110*/ { BARCODE_UPCA_CHK, BARCODE_QUIET_ZONES, -1, -1, 0, "014571377638+12345", "", 0, 50, 1, 151, 330, 110, 0 /*set*/, 16, 110, 320, 10 }, /* Hide text */
        /*111*/ { BARCODE_UPCA_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "014571377638+12345", "", 0, 50, 1, 151, 302, 110, 1 /*set*/, 16, 100, 300, 2 }, /* Hide text */
        /*112*/ { BARCODE_UPCE, -1, -1, -1, -1, "145713", "", 0, 50, 1, 51, 134, 116, 0 /*set*/, 0, 100, 120, 18 },
        /*113*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, -1, "145713", "", 0, 50, 1, 51, 134, 116, 0 /*set*/, 0, 100, 120, 18 },
        /*114*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "145713", "", 0, 50, 1, 51, 134, 116, 0 /*set*/, 0, 100, 120, 18 },
        /*115*/ { BARCODE_UPCE, -1, -1, -1, 0, "145713", "", 0, 50, 1, 51, 134, 110, 0 /*set*/, 0, 100, 120, 18 }, /* Hide text */
        /*116*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, 0, "145713", "", 0, 50, 1, 51, 134, 110, 0 /*set*/, 0, 100, 120, 18 }, /* Hide text */
        /*117*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "145713", "", 0, 50, 1, 51, 102, 110, 1 /*set*/, 0, 110, 100, 2 }, /* Hide text */
        /*118*/ { BARCODE_UPCE_CHK, -1, -1, -1, -1, "1457132+12", "", 0, 50, 1, 78, 184, 116, 0 /*set*/, 16, 100, 174, 10 },
        /*119*/ { BARCODE_UPCE_CHK, BARCODE_QUIET_ZONES, -1, -1, -1, "1457132+12", "", 0, 50, 1, 78, 184, 116, 0 /*set*/, 16, 100, 174, 10 },
        /*120*/ { BARCODE_UPCE_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1457132+12", "", 0, 50, 1, 78, 174, 116, 1 /*set*/, 16, 100, 170, 4 },
        /*121*/ { BARCODE_UPCE_CHK, -1, -1, -1, 0, "1457132+12", "", 0, 50, 1, 78, 184, 110, 0 /*set*/, 16, 110, 174, 10 }, /* Hide text */
        /*122*/ { BARCODE_UPCE_CHK, BARCODE_QUIET_ZONES, -1, -1, 0, "1457132+12", "", 0, 50, 1, 78, 184, 110, 0 /*set*/, 16, 110, 174, 10 }, /* Hide text */
        /*123*/ { BARCODE_UPCE_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "1457132+12", "", 0, 50, 1, 78, 156, 110, 1 /*set*/, 16, 100, 152, 4 }, /* Hide text */
        /*124*/ { BARCODE_UPCE, -1, -1, -1, -1, "145713+12345", "", 0, 50, 1, 105, 238, 116, 0 /*set*/, 16, 100, 228, 10 },
        /*125*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, -1, "145713+12345", "", 0, 50, 1, 105, 238, 116, 0 /*set*/, 16, 100, 228, 10 },
        /*126*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "145713+12345", "", 0, 50, 1, 105, 228, 116, 1 /*set*/, 16, 100, 216, 2 },
        /*127*/ { BARCODE_UPCE, -1, -1, -1, 0, "145713+12345", "", 0, 50, 1, 105, 238, 110, 0 /*set*/, 16, 110, 228, 10 }, /* Hide text */
        /*128*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, 0, "145713+12345", "", 0, 50, 1, 105, 238, 110, 0 /*set*/, 16, 110, 228, 10 }, /* Hide text */
        /*129*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "145713+12345", "", 0, 50, 1, 105, 210, 110, 1 /*set*/, 16, 100, 208, 2 }, /* Hide text */
        /*130*/ { BARCODE_POSTNET, -1, -1, -1, -1, "12345", "", 0, 12, 2, 63, 126, 24, 1 /*set*/, 0, 24, 0, 2 },
        /*131*/ { BARCODE_POSTNET, BARCODE_QUIET_ZONES, -1, -1, -1, "12345", "", 0, 12, 2, 63, 146, 30, 0 /*set*/, 0, 30, 0, 10 },
        /*132*/ { BARCODE_MSI_PLESSEY, -1, -1, -1, -1, "1234", "", 0, 50, 1, 55, 110, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*133*/ { BARCODE_MSI_PLESSEY, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 55, 158, 116, 0 /*set*/, 0, 100, 0, 24 },
        /*134*/ { BARCODE_FIM, -1, -1, -1, -1, "A", "", 0, 50, 1, 17, 34, 100, 1 /*set*/, 0, 100, 0, 2 },
        /*135*/ { BARCODE_FIM, BARCODE_QUIET_ZONES, -1, -1, -1, "A", "", 0, 50, 1, 17, 50, 100, 0 /*set*/, 0, 100, 0, 10 },
        /*136*/ { BARCODE_LOGMARS, -1, -1, -1, -1, "1234", "", 0, 50, 1, 95, 190, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*137*/ { BARCODE_LOGMARS, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 95, 230, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*138*/ { BARCODE_PHARMA, -1, -1, -1, -1, "1234", "", 0, 50, 1, 38, 76, 100, 1 /*set*/, 0, 100, 0, 2 },
        /*139*/ { BARCODE_PHARMA, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 38, 100, 100, 0 /*set*/, 0, 100, 0, 12 },
        /*140*/ { BARCODE_PZN, -1, -1, -1, -1, "1234", "", 0, 50, 1, 142, 284, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*141*/ { BARCODE_PZN, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 142, 324, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*142*/ { BARCODE_PHARMA_TWO, -1, -1, -1, -1, "1234", "", 0, 10, 2, 13, 26, 20, 1 /*set*/, 10, 20, 0, 2 },
        /*143*/ { BARCODE_PHARMA_TWO, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 10, 2, 13, 50, 20, 0 /*set*/, 10, 20, 0, 12 },
        /*144*/ { BARCODE_CEPNET, -1, -1, -1, -1, "12345678", "", 0, 5, 2, 93, 186, 10, 1 /*set*/, 0, 10, 0, 2 },
        /*145*/ { BARCODE_CEPNET, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678", "", 0, 5, 2, 93, 226, 16, 0 /*set*/, 0, 16, 0, 20 },
        /*146*/ { BARCODE_PDF417, -1, -1, -1, -1, "1234", "", 0, 18, 6, 103, 206, 36, 1 /*set*/, 0, 36, 0, 16 },
        /*147*/ { BARCODE_PDF417, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 18, 6, 103, 214, 44, 0 /*set*/, 0, 44, 0, 4 },
        /*148*/ { BARCODE_PDF417COMP, -1, -1, -1, -1, "1234", "", 0, 18, 6, 69, 138, 36, 1 /*set*/, 0, 36, 0, 16 },
        /*149*/ { BARCODE_PDF417COMP, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 18, 6, 69, 146, 44, 0 /*set*/, 0, 44, 0, 4 },
        /*150*/ { BARCODE_MAXICODE, -1, -1, -1, -1, "1234", "", 0, 165, 33, 30, 299, 298, 1 /*set*/, 21, 25, 0, 9 },
        /*151*/ { BARCODE_MAXICODE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 165, 33, 30, 319, 318, 0 /*set*/, 0, 318, 0, 9 },
        /*152*/ { BARCODE_MAXICODE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 165, 33, 30, 319, 318, 0 /*set*/, 0, 9, 0, 319 },
        /*153*/ { BARCODE_QRCODE, -1, -1, -1, -1, "1234", "", 0, 21, 21, 21, 42, 42, 1 /*set*/, 0, 2, 0, 14 },
        /*154*/ { BARCODE_QRCODE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 21, 21, 58, 58, 0 /*set*/, 0, 8, 0, 58 },
        /*155*/ { BARCODE_CODE128AB, -1, -1, -1, -1, "1234", "", 0, 50, 1, 79, 158, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*156*/ { BARCODE_CODE128AB, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 79, 198, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*157*/ { BARCODE_AUSPOST, -1, -1, -1, -1, "12345678", "", 0, 8, 3, 73, 146, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*158*/ { BARCODE_AUSPOST, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678", "", 0, 8, 3, 73, 186, 28, 0 /*set*/, 0, 28, 0, 20 },
        /*159*/ { BARCODE_AUSREPLY, -1, -1, -1, -1, "1234", "", 0, 8, 3, 73, 146, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*160*/ { BARCODE_AUSREPLY, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 73, 186, 28, 0 /*set*/, 0, 28, 0, 20 },
        /*161*/ { BARCODE_AUSROUTE, -1, -1, -1, -1, "1234", "", 0, 8, 3, 73, 146, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*162*/ { BARCODE_AUSROUTE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 73, 186, 28, 0 /*set*/, 0, 28, 0, 20 },
        /*163*/ { BARCODE_AUSREDIRECT, -1, -1, -1, -1, "1234", "", 0, 8, 3, 73, 146, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*164*/ { BARCODE_AUSREDIRECT, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 73, 186, 28, 0 /*set*/, 0, 28, 0, 20 },
        /*165*/ { BARCODE_ISBNX, -1, -1, -1, -1, "123456789X", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 16, 110, 212, 14 },
        /*166*/ { BARCODE_ISBNX, BARCODE_QUIET_ZONES, -1, -1, -1, "123456789X", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 16, 110, 212, 14 },
        /*167*/ { BARCODE_ISBNX, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "123456789X", "", 0, 50, 1, 95, 212, 116, 1 /*set*/, 16, 110, 210, 2 },
        /*168*/ { BARCODE_RM4SCC, -1, -1, -1, -1, "1234", "", 0, 8, 3, 43, 86, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*169*/ { BARCODE_RM4SCC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 43, 98, 28, 0 /*set*/, 0, 28, 0, 6 },
        /*170*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, "1234", "", 0, 10, 10, 10, 20, 20, 1 /*set*/, 0, 20, 0, 2 },
        /*171*/ { BARCODE_DATAMATRIX, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 10, 10, 10, 24, 24, 0 /*set*/, 0, 24, 0, 2 },
        /*172*/ { BARCODE_EAN14, -1, -1, -1, -1, "1234", "", 0, 50, 1, 134, 268, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*173*/ { BARCODE_EAN14, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 134, 308, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*174*/ { BARCODE_VIN, -1, -1, -1, -1, "12345678701234567", "", 0, 50, 1, 246, 492, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*175*/ { BARCODE_VIN, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678701234567", "", 0, 50, 1, 246, 532, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*176*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, "1234", "", 0, 20, 2, 101, 242, 44, 0 /*set*/, 0, 44, 0, 20 },
        /*177*/ { BARCODE_CODABLOCKF, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 101, 242, 44, 0 /*set*/, 0, 44, 0, 20 },
        /*178*/ { BARCODE_CODABLOCKF, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 101, 202, 44, 1 /*set*/, 0, 44, 0, 4 },
        /*179*/ { BARCODE_NVE18, -1, -1, -1, -1, "1234", "", 0, 50, 1, 156, 312, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*180*/ { BARCODE_NVE18, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 156, 352, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*181*/ { BARCODE_JAPANPOST, -1, -1, -1, -1, "1234", "", 0, 8, 3, 133, 266, 16, 1 /*set*/, 0, 16, 0, 2 },
        /*182*/ { BARCODE_JAPANPOST, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 133, 278, 28, 0 /*set*/, 0, 28, 0, 6 },
        /*183*/ { BARCODE_KOREAPOST, -1, -1, -1, -1, "1234", "", 0, 50, 1, 167, 334, 116, 0 /*set*/, 0, 100, 0, 8 },
        /*184*/ { BARCODE_KOREAPOST, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 167, 374, 116, 0 /*set*/, 0, 100, 0, 28 },
        /*185*/ { BARCODE_DBAR_STK, -1, -1, -1, -1, "1234", "", 0, 13, 3, 50, 100, 26, 1 /*set*/, 12, 26, 0, 2 },
        /*186*/ { BARCODE_DBAR_STK, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 13, 3, 50, 100, 26, 1 /*set*/, 12, 26, 0, 2 },
        /*187*/ { BARCODE_DBAR_STK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 13, 3, 50, 100, 26, 1 /*set*/, 12, 26, 0, 2 },
        /*188*/ { BARCODE_DBAR_OMNSTK, -1, -1, -1, -1, "1234", "", 0, 69, 5, 50, 100, 138, 1 /*set*/, 72, 138, 0, 2 },
        /*189*/ { BARCODE_DBAR_OMNSTK, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 69, 5, 50, 100, 138, 1 /*set*/, 72, 138, 0, 2 },
        /*190*/ { BARCODE_DBAR_OMNSTK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 69, 5, 50, 100, 138, 1 /*set*/, 72, 138, 0, 2 },
        /*191*/ { BARCODE_DBAR_EXPSTK, -1, -1, -1, -1, "[20]12", "", 0, 34, 1, 102, 204, 68, 1 /*set*/, 0, 68, 2, 2 },
        /*192*/ { BARCODE_DBAR_EXPSTK, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]12", "", 0, 34, 1, 102, 204, 68, 1 /*set*/, 0, 68, 2, 2 },
        /*193*/ { BARCODE_DBAR_EXPSTK, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "[20]12", "", 0, 34, 1, 102, 204, 68, 1 /*set*/, 0, 68, 2, 2 },
        /*194*/ { BARCODE_PLANET, -1, -1, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, 1 /*set*/, 0, 24, 0, 2 },
        /*195*/ { BARCODE_PLANET, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678901", "", 0, 12, 2, 123, 266, 30, 0 /*set*/, 0, 30, 0, 10 },
        /*196*/ { BARCODE_MICROPDF417, -1, -1, -1, -1, "1234", "", 0, 22, 11, 38, 76, 44, 1 /*set*/, 0, 44, 0, 4 },
        /*197*/ { BARCODE_MICROPDF417, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 22, 11, 38, 80, 48, 0 /*set*/, 0, 48, 0, 2 },
        /*198*/ { BARCODE_USPS_IMAIL, -1, -1, -1, -1, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*199*/ { BARCODE_USPS_IMAIL, BARCODE_QUIET_ZONES, -1, -1, -1, "12345678901234567890", "", 0, 8, 3, 129, 276, 20, 0 /*set*/, 0, 20, 0, 9 },
        /*200*/ { BARCODE_PLESSEY, -1, -1, -1, -1, "1234", "", 0, 50, 1, 131, 262, 116, 1 /*set*/, 0, 100, 0, 6 },
        /*201*/ { BARCODE_PLESSEY, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 131, 310, 116, 0 /*set*/, 0, 100, 0, 24 },
        /*202*/ { BARCODE_TELEPEN_NUM, -1, -1, -1, -1, "1234", "", 0, 50, 1, 80, 160, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*203*/ { BARCODE_TELEPEN_NUM, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 80, 200, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*204*/ { BARCODE_ITF14, -1, -1, -1, -1, "1234", "", 0, 50, 1, 135, 330, 136, 0 /*set*/, 10, 110, 10, 20 },
        /*205*/ { BARCODE_ITF14, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 135, 330, 136, 0 /*set*/, 10, 110, 10, 20 },
        /*206*/ { BARCODE_ITF14, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 135, 290, 136, 1 /*set*/, 0, 120, 10, 2 },
        /*207*/ { BARCODE_KIX, -1, -1, -1, -1, "1234", "", 0, 8, 3, 31, 62, 16, 1 /*set*/, 6, 10, 0, 2 },
        /*208*/ { BARCODE_KIX, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 8, 3, 31, 74, 28, 0 /*set*/, 0, 28, 0, 6 },
        /*209*/ { BARCODE_AZTEC, -1, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 1 /*set*/, 2, 6, 0, 4 },
        /*210*/ { BARCODE_AZTEC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 1 /*set*/, 2, 6, 0, 4 },
        /*211*/ { BARCODE_AZTEC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 1 /*set*/, 2, 6, 0, 4 },
        /*212*/ { BARCODE_DAFT, -1, -1, -1, -1, "FADT", "", 0, 8, 3, 7, 14, 16, 1 /*set*/, 0, 16, 0, 2 },
        /*213*/ { BARCODE_DAFT, BARCODE_QUIET_ZONES, -1, -1, -1, "FADT", "", 0, 8, 3, 7, 14, 16, 1 /*set*/, 0, 16, 0, 2 },
        /*214*/ { BARCODE_DAFT, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "FADT", "", 0, 8, 3, 7, 14, 16, 1 /*set*/, 0, 16, 0, 2 },
        /*215*/ { BARCODE_DPD, -1, -1, -1, -1, "1234567890123456789012345678", "", 0, 50, 1, 189, 378, 122, 1 /*set*/, 0, 100, 0, 4 },
        /*216*/ { BARCODE_DPD, BARCODE_QUIET_ZONES, -1, -1, -1, "1234567890123456789012345678", "", 0, 50, 1, 189, 428, 122, 0 /*set*/, 0, 100, 0, 24 },
        /*217*/ { BARCODE_MICROQR, -1, -1, -1, -1, "1234", "", 0, 11, 11, 11, 22, 22, 1 /*set*/, 0, 14, 0, 2 },
        /*218*/ { BARCODE_MICROQR, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 11, 11, 11, 30, 30, 0 /*set*/, 0, 30, 0, 4 },
        /*219*/ { BARCODE_HIBC_128, -1, -1, -1, -1, "1234", "", 0, 50, 1, 90, 180, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*220*/ { BARCODE_HIBC_128, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 90, 220, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*221*/ { BARCODE_HIBC_39, -1, -1, -1, -1, "1234", "", 0, 50, 1, 127, 254, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*222*/ { BARCODE_HIBC_39, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 127, 294, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*223*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, "1234", "", 0, 12, 12, 12, 24, 24, 1 /*set*/, 0, 24, 0, 2 },
        /*224*/ { BARCODE_HIBC_DM, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 12, 12, 12, 28, 28, 0 /*set*/, 0, 28, 0, 2 },
        /*225*/ { BARCODE_HIBC_QR, -1, -1, -1, -1, "1234", "", 0, 21, 21, 21, 42, 42, 1 /*set*/, 0, 2, 0, 14 },
        /*226*/ { BARCODE_HIBC_QR, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 21, 21, 58, 58, 0 /*set*/, 0, 58, 0, 8 },
        /*227*/ { BARCODE_HIBC_PDF, -1, -1, -1, -1, "1234", "", 0, 21, 7, 103, 206, 42, 1 /*set*/, 0, 42, 0, 16 },
        /*228*/ { BARCODE_HIBC_PDF, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 7, 103, 214, 50, 0 /*set*/, 0, 50, 0, 4 },
        /*229*/ { BARCODE_HIBC_MICPDF, -1, -1, -1, -1, "1234", "", 0, 12, 6, 82, 164, 24, 1 /*set*/, 0, 24, 0, 4 },
        /*230*/ { BARCODE_HIBC_MICPDF, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 12, 6, 82, 168, 28, 0 /*set*/, 0, 28, 0, 2 },
        /*231*/ { BARCODE_HIBC_BLOCKF, -1, -1, -1, -1, "1234", "", 0, 20, 2, 101, 242, 44, 0 /*set*/, 0, 44, 0, 20 },
        /*232*/ { BARCODE_HIBC_BLOCKF, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 101, 242, 44, 0 /*set*/, 0, 44, 0, 20 },
        /*233*/ { BARCODE_HIBC_BLOCKF, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 20, 2, 101, 202, 44, 1 /*set*/, 0, 44, 0, 4 },
        /*234*/ { BARCODE_HIBC_AZTEC, -1, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 1 /*set*/, 8, 10, 0, 2 },
        /*235*/ { BARCODE_HIBC_AZTEC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 1 /*set*/, 8, 10, 0, 2 },
        /*236*/ { BARCODE_HIBC_AZTEC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 15, 15, 15, 30, 30, 1 /*set*/, 8, 10, 0, 2 },
        /*237*/ { BARCODE_DOTCODE, -1, -1, -1, -1, "1234", "", 0, 10, 10, 13, 27, 21, 1 /*set*/, 5, 6, 1, 1 },
        /*238*/ { BARCODE_DOTCODE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 10, 10, 13, 39, 33, 0 /*set*/, 0, 33, 0, 7 },
        /*239*/ { BARCODE_HANXIN, -1, -1, -1, -1, "1234", "", 0, 23, 23, 23, 46, 46, 1 /*set*/, 0, 2, 0, 14 },
        /*240*/ { BARCODE_HANXIN, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 23, 23, 23, 58, 58, 0 /*set*/, 0, 58, 0, 6 },
        /*241*/ { BARCODE_MAILMARK_2D, -1, -1, -1, -1, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 48, 48, 1 /*set*/, 0, 48, 0, 2 },
        /*242*/ { BARCODE_MAILMARK_2D, BARCODE_QUIET_ZONES, -1, -1, -1, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 64, 64, 0 /*set*/, 0, 64, 0, 8 },
        /*243*/ { BARCODE_UPU_S10, -1, -1, -1, -1, "EE876543216CA", "", 0, 50, 1, 156, 312, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*244*/ { BARCODE_UPU_S10, BARCODE_QUIET_ZONES, -1, -1, -1, "EE876543216CA", "", 0, 50, 1, 156, 352, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*245*/ { BARCODE_MAILMARK_4S, -1, -1, -1, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, 1 /*set*/, 0, 20, 0, 2 },
        /*246*/ { BARCODE_MAILMARK_4S, BARCODE_QUIET_ZONES, -1, -1, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 322, 32, 0 /*set*/, 0, 32, 0, 6 },
        /*247*/ { BARCODE_AZRUNE, -1, -1, -1, -1, "123", "", 0, 11, 11, 11, 22, 22, 1 /*set*/, 0, 6, 0, 4 },
        /*248*/ { BARCODE_AZRUNE, BARCODE_QUIET_ZONES, -1, -1, -1, "123", "", 0, 11, 11, 11, 22, 22, 1 /*set*/, 0, 6, 0, 4 },
        /*249*/ { BARCODE_AZRUNE, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "123", "", 0, 11, 11, 11, 22, 22, 1 /*set*/, 0, 6, 0, 4 },
        /*250*/ { BARCODE_CODE32, -1, -1, -1, -1, "1234", "", 0, 50, 1, 103, 206, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*251*/ { BARCODE_CODE32, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 103, 246, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*252*/ { BARCODE_EAN13_CC, -1, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 226, 116, 0 /*set*/, 24, 110, 212, 14 },
        /*253*/ { BARCODE_EANX_CC, -1, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 226, 116, 0 /*set*/, 24, 110, 212, 14 },
        /*254*/ { BARCODE_EAN13_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 226, 116, 0 /*set*/, 24, 110, 212, 14 },
        /*255*/ { BARCODE_EANX_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 226, 116, 0 /*set*/, 24, 110, 212, 14 },
        /*256*/ { BARCODE_EAN13_CC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 214, 116, 0 /*set*/, 24, 110, 212, 2 },
        /*257*/ { BARCODE_EANX_CC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "023456789012", "", 0, 50, 7, 99, 214, 116, 0 /*set*/, 24, 110, 212, 2 },
        /*258*/ { BARCODE_EAN13_CC, -1, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 226, 110, 0 /*set*/, 24, 110, 0, 22 }, /* Hide text */
        /*259*/ { BARCODE_EANX_CC, -1, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 226, 110, 0 /*set*/, 24, 110, 0, 22 }, /* Hide text */
        /*260*/ { BARCODE_EAN13_CC, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 226, 110, 0 /*set*/, 24, 110, 0, 22 }, /* Hide text */
        /*261*/ { BARCODE_EANX_CC, BARCODE_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 226, 110, 0 /*set*/, 24, 110, 0, 22 }, /* Hide text */
        /*262*/ { BARCODE_EAN13_CC, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 198, 110, 1 /*set*/, 24, 110, 6, 2 }, /* Hide text */
        /*263*/ { BARCODE_EANX_CC, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "023456789012", "", 0, 50, 7, 99, 198, 110, 1 /*set*/, 24, 110, 6, 2 }, /* Hide text */
        /*264*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, "[20]02", "", 0, 50, 5, 99, 198, 116, 1 /*set*/, 14, 100, 24, 4 }, /* CC-A */
        /*265*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 50, 5, 99, 204, 116, 0 /*set*/, 14, 100, 24, 2 },
        /*266*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]02", "", 0, 50, 5, 99, 204, 116, 1 /*set*/, 14, 100, 26, 4 },
        /*267*/ { BARCODE_GS1_128_CC, -1, -1, -1, -1, "[91]1", "", 0, 50, 5, 100, 200, 116, 1 /*set*/, 14, 100, 20, 4 }, /* CC-A */
        /*268*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[91]1", "", 0, 50, 5, 100, 222, 116, 0 /*set*/, 14, 100, 20, 2 },
        /*269*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[91]1", "", 0, 50, 5, 100, 222, 116, 1 /*set*/, 14, 100, 22, 4 },
        /*270*/ { BARCODE_GS1_128_CC, -1, 2, -1, -1, "[91]1", "", 0, 50, 6, 100, 200, 116, 1 /*set*/, 18, 100, 20, 4 }, /* CC-B */
        /*271*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "[91]1", "", 0, 50, 6, 100, 222, 116, 0 /*set*/, 18, 100, 20, 2 },
        /*272*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "[91]1", "", 0, 50, 6, 100, 222, 116, 1 /*set*/, 18, 100, 22, 4 },
        /*273*/ { BARCODE_GS1_128_CC, -1, 3, -1, -1, "[20]02", "", 0, 50, 15, 86, 172, 116, 1 /*set*/, 80, 100, 14, 4 }, /* CC-C */
        /*274*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, 3, -1, -1, "[20]02", "", 0, 50, 15, 86, 198, 116, 0 /*set*/, 80, 100, 14, 4 },
        /*275*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, 3, -1, -1, "[20]02", "", 0, 50, 15, 86, 198, 116, 1 /*set*/, 80, 100, 20, 4 },
        /*276*/ { BARCODE_DBAR_OMN_CC, -1, -1, -1, -1, "1234", "", 0, 21, 5, 100, 200, 58, 1 /*set*/, 14, 42, 10, 2 }, /* CC-A */
        /*277*/ { BARCODE_DBAR_OMN_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 5, 100, 202, 58, 0 /*set*/, 14, 42, 10, 2 },
        /*278*/ { BARCODE_DBAR_OMN_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 21, 5, 100, 202, 58, 1 /*set*/, 14, 42, 12, 2 },
        /*279*/ { BARCODE_DBAR_OMN_CC, -1, 2, -1, -1, "1234", "", 0, 23, 6, 100, 200, 62, 1 /*set*/, 18, 46, 10, 2 }, /* CC-B */
        /*280*/ { BARCODE_DBAR_OMN_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 23, 6, 100, 202, 62, 0 /*set*/, 18, 46, 10, 2 },
        /*281*/ { BARCODE_DBAR_OMN_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 23, 6, 100, 202, 62, 1 /*set*/, 18, 46, 12, 2 },
        /*282*/ { BARCODE_DBAR_LTD_CC, -1, -1, -1, -1, "1234", "", 0, 19, 6, 79, 158, 54, 1 /*set*/, 18, 38, 2, 2 }, /* CC-A */
        /*283*/ { BARCODE_DBAR_LTD_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 19, 6, 79, 158, 54, 1 /*set*/, 18, 38, 2, 2 }, /* Same */
        /*284*/ { BARCODE_DBAR_LTD_CC, -1, 2, -1, -1, "1234", "", 0, 23, 8, 88, 176, 62, 1 /*set*/, 26, 46, 20, 2 }, /* CC-B */
        /*285*/ { BARCODE_DBAR_LTD_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 23, 8, 88, 178, 62, 0 /*set*/, 26, 46, 20, 2 },
        /*286*/ { BARCODE_DBAR_LTD_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 23, 8, 88, 178, 62, 1 /*set*/, 26, 46, 22, 2 },
        /*287*/ { BARCODE_DBAR_EXP_CC, -1, -1, -1, -1, "[20]12", "", 0, 41, 5, 102, 204, 98, 1 /*set*/, 14, 82, 2, 2 }, /* CC-A */
        /*288*/ { BARCODE_DBAR_EXP_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]12", "", 0, 41, 5, 102, 204, 98, 1 /*set*/, 14, 82, 2, 2 }, /* Same */
        /*289*/ { BARCODE_DBAR_EXP_CC, -1, 2, -1, -1, "[20]12", "", 0, 43, 6, 102, 204, 102, 1 /*set*/, 18, 86, 2, 2 }, /* CC-B */
        /*290*/ { BARCODE_DBAR_EXP_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "[20]12", "", 0, 43, 6, 102, 204, 102, 1 /*set*/, 18, 86, 2, 2 }, /* Same */
        /*291*/ { BARCODE_UPCA_CC, -1, -1, -1, -1, "01457137763", "", 0, 50, 7, 99, 226, 116, 1 /*set*/, 24, 100, 206, 2 },
        /*292*/ { BARCODE_UPCA_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "01457137763", "", 0, 50, 7, 99, 226, 116, 1 /*set*/, 24, 100, 206, 2 },
        /*293*/ { BARCODE_UPCA_CC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "01457137763", "", 0, 50, 7, 99, 226, 116, 1 /*set*/, 24, 100, 206, 2 },
        /*294*/ { BARCODE_UPCA_CC, -1, -1, -1, 0, "01457137763", "", 0, 50, 7, 99, 226, 110, 0 /*set*/, 24, 110, 0, 18 }, /* Hide text */
        /*295*/ { BARCODE_UPCA_CC, BARCODE_QUIET_ZONES, -1, -1, 0, "01457137763", "", 0, 50, 7, 99, 226, 110, 0 /*set*/, 24, 110, 0, 18 }, /* Hide text */
        /*296*/ { BARCODE_UPCA_CC, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "01457137763", "", 0, 50, 7, 99, 198, 110, 1 /*set*/, 24, 110, 6, 2 }, /* Hide text */
        /*297*/ { BARCODE_UPCE_CC, -1, -1, -1, -1, "145713", "", 0, 50, 9, 55, 134, 116, 1 /*set*/, 32, 100, 118, 2 },
        /*298*/ { BARCODE_UPCE_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "145713", "", 0, 50, 9, 55, 134, 116, 1 /*set*/, 32, 100, 118, 2 },
        /*299*/ { BARCODE_UPCE_CC, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "145713", "", 0, 50, 9, 55, 134, 116, 1 /*set*/, 32, 100, 118, 2 },
        /*300*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, "145713", "", 0, 50, 9, 55, 134, 110, 0 /*set*/, 32, 110, 0, 18 }, /* Hide text */
        /*301*/ { BARCODE_UPCE_CC, BARCODE_QUIET_ZONES, -1, -1, 0, "145713", "", 0, 50, 9, 55, 134, 110, 0 /*set*/, 32, 110, 0, 18 }, /* Hide text */
        /*302*/ { BARCODE_UPCE_CC, BARCODE_NO_QUIET_ZONES, -1, -1, 0, "145713", "", 0, 50, 9, 55, 110, 110, 1 /*set*/, 32, 110, 6, 2 }, /* Hide text */
        /*303*/ { BARCODE_DBAR_STK_CC, -1, -1, -1, -1, "1234", "", 0, 24, 9, 56, 112, 48, 0 /*set*/, 20, 48, 100, 10 }, /* CC-A */
        /*304*/ { BARCODE_DBAR_STK_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 24, 9, 56, 114, 48, 0 /*set*/, 20, 48, 100, 14 },
        /*305*/ { BARCODE_DBAR_STK_CC, -1, 2, -1, -1, "1234", "", 0, 30, 12, 56, 112, 60, 0 /*set*/, 34, 60, 100, 10 }, /* CC-B */
        /*306*/ { BARCODE_DBAR_STK_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 30, 12, 56, 114, 60, 0 /*set*/, 34, 60, 100, 14 },
        /*307*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, -1, -1, "1234", "", 0, 80, 11, 56, 112, 160, 0 /*set*/, 20, 48, 100, 10 }, /* CC-A */
        /*308*/ { BARCODE_DBAR_OMNSTK_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 80, 11, 56, 114, 160, 0 /*set*/, 20, 48, 100, 14 },
        /*309*/ { BARCODE_DBAR_OMNSTK_CC, -1, 2, -1, -1, "1234", "", 0, 86, 14, 56, 112, 172, 0 /*set*/, 34, 172, 100, 10 }, /* CC-B */
        /*310*/ { BARCODE_DBAR_OMNSTK_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "1234", "", 0, 86, 14, 56, 114, 172, 0 /*set*/, 34, 172, 100, 14 },
        /*311*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, -1, -1, "[20]12", "", 0, 41, 5, 102, 204, 82, 1 /*set*/, 14, 82, 2, 2 }, /* CC-A */
        /*312*/ { BARCODE_DBAR_EXPSTK_CC, BARCODE_QUIET_ZONES, -1, -1, -1, "[20]12", "", 0, 41, 5, 102, 204, 82, 1 /*set*/, 14, 82, 2, 2 }, /* Same */
        /*313*/ { BARCODE_DBAR_EXPSTK_CC, -1, 2, -1, -1, "[20]12", "", 0, 43, 6, 102, 204, 86, 1 /*set*/, 18, 86, 202, 2 }, /* CC-B */
        /*314*/ { BARCODE_DBAR_EXPSTK_CC, BARCODE_QUIET_ZONES, 2, -1, -1, "[20]12", "", 0, 43, 6, 102, 204, 86, 1 /*set*/, 18, 86, 202, 2 }, /* Same */
        /*315*/ { BARCODE_CHANNEL, -1, -1, -1, -1, "1234", "", 0, 50, 1, 27, 54, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*316*/ { BARCODE_CHANNEL, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 50, 1, 27, 60, 116, 0 /*set*/, 0, 100, 0, 2 },
        /*317*/ { BARCODE_CODEONE, -1, -1, -1, -1, "1234", "", 0, 16, 16, 18, 36, 32, 1 /*set*/, 0, 6, 0, 2 }, /* Versions A to H - no quiet zone */
        /*318*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 16, 16, 18, 36, 32, 1 /*set*/, 0, 6, 0, 2 },
        /*319*/ { BARCODE_CODEONE, BARCODE_NO_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 16, 16, 18, 36, 32, 1 /*set*/, 0, 6, 0, 2 },
        /*320*/ { BARCODE_CODEONE, -1, -1, 9, -1, "1234", "", 0, 8, 8, 11, 22, 16, 1 /*set*/, 10, 16, 0, 2 }, /* Version S (& T) have quiet zones */
        /*321*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, -1, 9, -1, "1234", "", 0, 8, 8, 11, 26, 18, 0 /*set*/, 0, 16, 0, 2 },
        /*322*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, -1, 9, -1, "1234", "", 0, 8, 8, 11, 26, 18, 0 /*set*/, 16, 18, 0, 26 }, /* Bottom 1X */
        /*323*/ { BARCODE_GRIDMATRIX, -1, -1, -1, -1, "1234", "", 0, 18, 18, 18, 36, 36, 1 /*set*/, 0, 2, 0, 12 },
        /*324*/ { BARCODE_GRIDMATRIX, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 18, 18, 18, 60, 60, 0 /*set*/, 0, 60, 0, 12 },
        /*325*/ { BARCODE_UPNQR, -1, -1, -1, -1, "1234", "", 0, 77, 77, 77, 154, 154, 1 /*set*/, 0, 14, 0, 2 },
        /*326*/ { BARCODE_UPNQR, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 77, 77, 77, 170, 170, 0 /*set*/, 0, 170, 0, 8 },
        /*327*/ { BARCODE_ULTRA, -1, -1, -1, -1, "1234", "", 0, 13, 13, 15, 30, 26, 1 /*set*/, 0, 2, 0, 30 },
        /*328*/ { BARCODE_ULTRA, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 13, 13, 15, 34, 30, 0 /*set*/, 0, 2, 0, 34 },
        /*329*/ { BARCODE_RMQR, -1, -1, -1, -1, "1234", "", 0, 11, 11, 27, 54, 22, 1 /*set*/, 0, 14, 0, 2 },
        /*330*/ { BARCODE_RMQR, BARCODE_QUIET_ZONES, -1, -1, -1, "1234", "", 0, 11, 11, 27, 62, 30, 0 /*set*/, 0, 30, 0, 4 },
        /*331*/ { BARCODE_BC412, -1, -1, -1, -1, "1234567", "", 0, 16.5, 1, 102, 204, 49, 1 /*set*/, 0, 32, 0, 2 },
        /*332*/ { BARCODE_BC412, BARCODE_QUIET_ZONES, -1, -1, -1, "1234567", "", 0, 16.5, 1, 102, 244, 49, 0 /*set*/, 0, 32, 0, 2 },
        /*333*/ { BARCODE_DXFILMEDGE, -1, -1, -1, -1, "120476", "", 0, 6, 2, 23, 46, 12, 1 /*set*/, 0, 6, 0, 3 },
        /*334*/ { BARCODE_DXFILMEDGE, BARCODE_QUIET_ZONES, -1, -1, -1, "120476", "", 0, 6, 2, 23, 52, 12, 0 /*set*/, 0, 6, 0, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;
    static const char composite[] = "[20]12";

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int row, column;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }

        if (z_is_composite(symbol->symbology)) {
            text = *(data[i].composite) ? data[i].composite : composite;
            length = (int) strlen(text);
            assert_nonzero(strlen(data[i].data) < 128, "i:%d linear data length %d >= 128\n", i, (int) strlen(data[i].data));
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Buffer(%d) ret %d != %d (%s)\n",
            i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Print(%s) ret %d != %d (%s)\n",
            i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret_raster, symbol->errtxt);
        assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

        assert_nonzero(symbol->bitmap_height >= data[i].expected_set_rows,
            "i:%d (%s) symbol->bitmap_height %d < expected_set_rows %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_set_rows);
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row,
            "i:%d (%s) expected_set_rows %d < expected_set_row %d\n",
            i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_rows, data[i].expected_set_row);
        for (row = data[i].expected_set_row; row < data[i].expected_set_rows; row++) {
            int bits_set = 0;
            for (column = data[i].expected_set_col; column < data[i].expected_set_col + data[i].expected_set_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    bits_set++;
                }
            }
            if (data[i].expected_set) {
                assert_equal(bits_set, data[i].expected_set_len, "i:%d (%s) row %d bits_set %d != expected_set_len %d\n",
                    i, testUtilBarcodeName(data[i].symbology), row, bits_set, data[i].expected_set_len);
            } else {
                assert_zero(bits_set, "i:%d (%s) row %d bits_set %d != 0\n",
                    i, testUtilBarcodeName(data[i].symbology), row, bits_set);
            }
        }
        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_text_gap(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        int option_2;
        int show_hrt;
        float text_gap;
        float scale;
        const char *data;
        const char *composite;

        int ret;
        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;

        int expected_set;
        int expected_set_row;
        int expected_set_rows; /* Last row no. + 1 */
        int expected_set_col;
        int expected_set_len;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, -1, 1, 0, "1234", "", 0, 50, 1, 62, 124, 116, 1 /*set*/, 104, 105, 55, 6 }, /* Default */
        /*  1*/ { BARCODE_CODE11, -1, -1, -1, 1, 0, "1234", "", 0, 50, 1, 62, 124, 116, 0 /*set*/, 103, 104, 0, 124 }, /* Default */
        /*  2*/ { BARCODE_CODE11, -1, -1, -1, 0.1, 0, "1234", "", 0, 50, 1, 62, 124, 115, 1 /*set*/, 102, 103, 55, 6 },
        /*  3*/ { BARCODE_CODE11, -1, -1, -1, 0.3, 0, "1234", "", 0, 50, 1, 62, 124, 115, 1 /*set*/, 102, 103, 55, 6 },
        /*  4*/ { BARCODE_CODE11, -1, -1, -1, 0.4, 0, "1234", "", 0, 50, 1, 62, 124, 115, 1 /*set*/, 102, 103, 55, 6 },
        /*  5*/ { BARCODE_CODE11, -1, -1, -1, 0.5, 0, "1234", "", 0, 50, 1, 62, 124, 115, 1 /*set*/, 103, 104, 55, 6 },
        /*  6*/ { BARCODE_CODE11, -1, -1, -1, 0.5, 0, "1234", "", 0, 50, 1, 62, 124, 115, 0 /*set*/, 102, 103, 0, 124 },
        /*  7*/ { BARCODE_CODE11, -1, -1, -1, 0.6, 0, "1234", "", 0, 50, 1, 62, 124, 116, 1 /*set*/, 103, 104, 55, 6 },
        /*  8*/ { BARCODE_CODE11, -1, -1, -1, 0.9, 0, "1234", "", 0, 50, 1, 62, 124, 116, 1 /*set*/, 103, 104, 55, 6 },
        /*  9*/ { BARCODE_CODE11, -1, -1, -1, 0.9, 0, "1234", "", 0, 50, 1, 62, 124, 116, 0 /*set*/, 102, 103, 0, 124 },
        /* 10*/ { BARCODE_CODE11, -1, -1, -1, 1.5, 0, "1234", "", 0, 50, 1, 62, 124, 117, 1 /*set*/, 105, 106, 55, 6 },
        /* 11*/ { BARCODE_CODE11, -1, -1, -1, 1.5, 0, "1234", "", 0, 50, 1, 62, 124, 117, 0 /*set*/, 104, 105, 0, 124 },
        /* 12*/ { BARCODE_CODE11, -1, -1, -1, 2.0, 0, "1234", "", 0, 50, 1, 62, 124, 118, 1 /*set*/, 106, 107, 55, 6 },
        /* 13*/ { BARCODE_CODE11, -1, -1, -1, 2.0, 0, "1234", "", 0, 50, 1, 62, 124, 118, 0 /*set*/, 105, 106, 0, 124 },
        /* 14*/ { BARCODE_CODE11, -1, -1, -1, 3.0, 0, "1234", "", 0, 50, 1, 62, 124, 120, 1 /*set*/, 108, 109, 55, 6 },
        /* 15*/ { BARCODE_CODE11, -1, -1, -1, 3.0, 0, "1234", "", 0, 50, 1, 62, 124, 120, 0 /*set*/, 107, 108, 0, 124 },
        /* 16*/ { BARCODE_CODE11, -1, -1, -1, 4.0, 0, "1234", "", 0, 50, 1, 62, 124, 122, 1 /*set*/, 110, 111, 55, 6 },
        /* 17*/ { BARCODE_CODE11, -1, -1, -1, 4.0, 0, "1234", "", 0, 50, 1, 62, 124, 122, 0 /*set*/, 109, 110, 0, 124 },
        /* 18*/ { BARCODE_CODE11, -1, -1, -1, 5.0, 0, "1234", "", 0, 50, 1, 62, 124, 124, 1 /*set*/, 112, 113, 55, 6 },
        /* 19*/ { BARCODE_CODE11, -1, -1, -1, 5.0, 0, "1234", "", 0, 50, 1, 62, 124, 124, 0 /*set*/, 111, 112, 0, 124 },
        /* 20*/ { BARCODE_CODE11, -1, -1, -1, 10.0, 0, "1234", "", 0, 50, 1, 62, 124, 134, 1 /*set*/, 122, 123, 55, 6 },
        /* 21*/ { BARCODE_CODE11, -1, -1, -1, 10.0, 0, "1234", "", 0, 50, 1, 62, 124, 134, 0 /*set*/, 121, 122, 0, 124 },
        /* 22*/ { BARCODE_CODE11, -1, -1, -1, -1.0, 0, "1234", "", 0, 50, 1, 62, 124, 112, 1 /*set*/, 100, 101, 55, 6 },
        /* 23*/ { BARCODE_CODE11, -1, -1, -1, -0.5, 0, "1234", "", 0, 50, 1, 62, 124, 113, 1 /*set*/, 101, 102, 55, 6 },
        /* 24*/ { BARCODE_CODE11, -1, -1, -1, -0.5, 0, "1234", "", 0, 50, 1, 62, 124, 113, 0 /*set*/, 100, 101, 0, 124 },
        /* 25*/ { BARCODE_CODE11, -1, -1, -1, 1, 3.0, "1234", "", 0, 50, 1, 62, 372, 348, 1 /*set*/, 312, 315, 165, 18 }, /* Scale default */
        /* 26*/ { BARCODE_CODE11, -1, -1, -1, 1, 3.0, "1234", "", 0, 50, 1, 62, 372, 348, 0 /*set*/, 311, 312, 0, 372 }, /* Scale default */
        /* 27*/ { BARCODE_CODE11, -1, -1, -1, 1.5, 3.0, "1234", "", 0, 50, 1, 62, 372, 351, 1 /*set*/, 315, 318, 165, 18 }, /* Scale */
        /* 28*/ { BARCODE_CODE11, -1, -1, -1, 1.5, 3.0, "1234", "", 0, 50, 1, 62, 372, 351, 0 /*set*/, 314, 315, 0, 372 }, /* Scale */
        /* 29*/ { BARCODE_UPCA, -1, -1, -1, 1, 0, "01457130763", "", 0, 50, 1, 95, 226, 116, 1 /*set*/, 102, 104, 82, 9 }, /* Default */
        /* 30*/ { BARCODE_UPCA, -1, -1, -1, 1, 0, "01457130763", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 101, 102, 38, 72 }, /* Default */
        /* 31*/ { BARCODE_UPCA, -1, -1, -1, 0.5, 0, "01457130763", "", 0, 50, 1, 95, 226, 115, 1 /*set*/, 101, 103, 82, 9 },
        /* 32*/ { BARCODE_UPCA, -1, -1, -1, 0.75, 0, "01457130763", "", 0, 50, 1, 95, 226, 116, 1 /*set*/, 101, 103, 82, 9 },
        /* 33*/ { BARCODE_UPCA, -1, -1, -1, 0.9, 0, "01457130763", "", 0, 50, 1, 95, 226, 116, 1 /*set*/, 101, 103, 82, 9 },
        /* 34*/ { BARCODE_UPCA, -1, -1, -1, 0.9, 0, "01457130763", "", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100, 101, 38, 72 },
        /* 35*/ { BARCODE_UPCA, -1, -1, -1, 2.0, 0, "01457130763", "", 0, 50, 1, 95, 226, 118, 1 /*set*/, 104, 106, 82, 9 },
        /* 36*/ { BARCODE_UPCA, -1, -1, -1, 2.0, 2.5, "01457130763", "", 0, 50, 1, 95, 565, 295, 1 /*set*/, 260, 265, 206, 22 }, /* Scale */
        /* 37*/ { BARCODE_UPCA, -1, -1, -1, 2.0, 2.5, "01457130763", "", 0, 50, 1, 95, 565, 295, 0 /*set*/, 259, 260, 95, 180 }, /* Scale */
        /* 38*/ { BARCODE_UPCA, -1, -1, -1, 1, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 116, 0 /*set*/, 14, 16, 208, 68 }, /* Default */
        /* 39*/ { BARCODE_UPCA, -1, -1, -1, 1, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 116, 1 /*set*/, 16, 100, 244, 4 }, /* Default */
        /* 40*/ { BARCODE_UPCA, -1, -1, -1, 0.5, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 115, 0 /*set*/, 14, 15, 208, 68 },
        /* 41*/ { BARCODE_UPCA, -1, -1, -1, 0.5, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 115, 1 /*set*/, 16, 100, 244, 4 },
        /* 42*/ { BARCODE_UPCA, -1, -1, -1, 0.9, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 116, 0 /*set*/, 14, 16, 208, 68 },
        /* 43*/ { BARCODE_UPCA, -1, -1, -1, 0.9, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 116, 1 /*set*/, 16, 100, 244, 4 },
        /* 44*/ { BARCODE_UPCA, -1, -1, -1, 1.5, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 117, 0 /*set*/, 14, 17, 208, 68 },
        /* 45*/ { BARCODE_UPCA, -1, -1, -1, 1.5, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 117, 1 /*set*/, 17, 100, 244, 4 },
        /* 46*/ { BARCODE_UPCA, -1, -1, -1, 2.5, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 119, 0 /*set*/, 14, 19, 208, 68 },
        /* 47*/ { BARCODE_UPCA, -1, -1, -1, 2.5, 0, "01457130763+10", "", 0, 50, 1, 124, 276, 119, 1 /*set*/, 19, 100, 244, 4 },
        /* 48*/ { BARCODE_UPCA_CC, -1, -1, -1, 1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 116, 0 /*set*/, 38, 40, 208, 68 }, /* Default */
        /* 49*/ { BARCODE_UPCA_CC, -1, -1, -1, 1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 116, 1 /*set*/, 40, 100, 258, 4 }, /* Default */
        /* 50*/ { BARCODE_UPCA_CC, -1, -1, -1, 1.0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 116, 0 /*set*/, 38, 40, 208, 68 }, /* Same as default */
        /* 51*/ { BARCODE_UPCA_CC, -1, -1, -1, 1.0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 116, 1 /*set*/, 40, 100, 258, 4 },
        /* 52*/ { BARCODE_UPCA_CC, -1, -1, -1, 3.0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 120, 0 /*set*/, 38, 44, 208, 68 },
        /* 53*/ { BARCODE_UPCA_CC, -1, -1, -1, 3.0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 120, 1 /*set*/, 44, 100, 258, 4 },
        /* 54*/ { BARCODE_UPCA_CC, -1, -1, 0, 1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 110, 0 /*set*/, 38, 40, 208, 68 }, /* Hide text default */
        /* 55*/ { BARCODE_UPCA_CC, -1, -1, 0, 1, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 110, 1 /*set*/, 40, 100, 258, 4 }, /* Hide text default */
        /* 56*/ { BARCODE_UPCA_CC, -1, -1, 0, 3.0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 110, 0 /*set*/, 38, 44, 208, 68 }, /* Hide text */
        /* 57*/ { BARCODE_UPCA_CC, -1, -1, 0, 3.0, 0, "01457130763+10", "[91]12", 0, 50, 7, 127, 276, 110, 1 /*set*/, 44, 100, 258, 4 }, /* Hide text */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {
        int row, column;

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        symbol->text_gap = data[i].text_gap;
        if (data[i].scale != 0.0f) {
            symbol->scale = data[i].scale;
        }

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, text, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Print(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret, symbol->errtxt);
        assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

        assert_nonzero(symbol->bitmap_height >= data[i].expected_set_rows, "i:%d (%s) symbol->bitmap_height %d < expected_set_rows %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_set_rows);
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row, "i:%d (%s) expected_set_rows %d < expected_set_row %d\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].expected_set_rows, data[i].expected_set_row);
        assert_nonzero(symbol->bitmap_width >= data[i].expected_set_col + data[i].expected_set_len,
                    "i:%d (%s) symbol->bitmap_width %d < expected_set_col %d + expected_set_len %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_set_col, data[i].expected_set_len);
        for (row = data[i].expected_set_row; row < data[i].expected_set_rows; row++) {
            int bits_set = 0;
            for (column = data[i].expected_set_col; column < data[i].expected_set_col + data[i].expected_set_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    bits_set++;
                }
            }
            if (data[i].expected_set) {
                assert_equal(bits_set, data[i].expected_set_len, "i:%d (%s) row %d,%d bits_set %d != expected_set_len %d\n",
                            i, testUtilBarcodeName(data[i].symbology), row, data[i].expected_set_col, bits_set, data[i].expected_set_len);
            } else {
                assert_zero(bits_set, "i:%d (%d) row %d bits_set %d != 0\n", i, data[i].symbology, row, bits_set);
            }
        }
        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_buffer_plot(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_1;
        int option_2;
        int whitespace_width;
        int output_options;
        float height;
        const char *fgcolour;
        const char *bgcolour;
        const char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        const char *expected_bitmap;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, 0, 1, -1, -1, 15, "", "", "1", 0, 16, 4, 86, 86, 16,
                    "11111111010101000111101010111100001111101010111110011101010111000000111111101000101001"
                    "11111111010101000111101010111100001111101010111110011101010111000000111111101000101001"
                    "11111111010101000111101010111100001111101010111110011101010111000000111111101000101001"
                    "11111111010101000111101010111100001111101010111110011101010111000000111111101000101001"
                    "11111111010101000111110101011000001111000001000101011111101010111000111111101000101001"
                    "11111111010101000111110101011000001111000001000101011111101010111000111111101000101001"
                    "11111111010101000111110101011000001111000001000101011111101010111000111111101000101001"
                    "11111111010101000111110101011000001111000001000101011111101010111000111111101000101001"
                    "11111111010101000110101011111000001111011111101011011010101111100000111111101000101001"
                    "11111111010101000110101011111000001111011111101011011010101111100000111111101000101001"
                    "11111111010101000110101011111000001111011111101011011010101111100000111111101000101001"
                    "11111111010101000110101011111000001111011111101011011010101111100000111111101000101001"
                    "11111111010101000101011110011110001010000010001000011010111101111100111111101000101001"
                    "11111111010101000101011110011110001010000010001000011010111101111100111111101000101001"
                    "11111111010101000101011110011110001010000010001000011010111101111100111111101000101001"
                    "11111111010101000101011110011110001010000010001000011010111101111100111111101000101001"
        },
        /*  1*/ { BARCODE_PDF417, 0, 1, -1, -1, 15, "FF0000", "00FF0099", "1", 0, 16, 4, 86, 86, 16,
                    "RRRRRRRRGRGRGRGGGRRRRGRGRGRRRRGGGGRRRRRGRGRGRRRRRGGRRRGRGRGRRRGGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRGRGRGRRRRGGGGRRRRRGRGRGRRRRRGGRRRGRGRGRRRGGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRGRGRGRRRRGGGGRRRRRGRGRGRRRRRGGRRRGRGRGRRRGGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRGRGRGRRRRGGGGRRRRRGRGRGRRRRRGGRRRGRGRGRRRGGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRRGRGRGRRGGGGGRRRRGGGGGRGGGRGRGRRRRRRGRGRGRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRRGRGRGRRGGGGGRRRRGGGGGRGGGRGRGRRRRRRGRGRGRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRRGRGRGRRGGGGGRRRRGGGGGRGGGRGRGRRRRRRGRGRGRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRRGRGRGRRGGGGGRRRRGGGGGRGGGRGRGRRRRRRGRGRGRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRGRGRGRRRRRGGGGGRRRRGRRRRRRGRGRRGRRGRGRGRRRRRGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRGRGRGRRRRRGGGGGRRRRGRRRRRRGRGRRGRRGRGRGRRRRRGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRGRGRGRRRRRGGGGGRRRRGRRRRRRGRGRRGRRGRGRGRRRRRGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRGRGRGRRRRRGGGGGRRRRGRRRRRRGRGRRGRRGRGRGRRRRRGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRGRGRRRRGGRRRRGGGRGRGGGGGRGGGRGGGGRRGRGRRRRGRRRRRGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRGRGRRRRGGRRRRGGGRGRGGGGGRGGGRGGGGRRGRGRRRRGRRRRRGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRGRGRRRRGGRRRRGGGRGRGGGGGRGGGRGGGGRRGRGRRRRGRRRRRGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRGRGRRRRGGRRRRGGGRGRGGGGGRGGGRGGGGRRGRGRRRRGRRRRRGGRRRRRRRGRGGGRGRGGR"
        },
        /*  2*/ { BARCODE_PDF417, 0, 1, 1, -1, 15, "FFFF0033", "00FF00", "1", 0, 16, 4, 86, 88, 16,
                    "GYYYYYYYYGYGYGYGGGYYYYGYGYGYYYYGGGGYYYYYGYGYGYYYYYGGYYYGYGYGYYYGGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYGYGYGYYYYGGGGYYYYYGYGYGYYYYYGGYYYGYGYGYYYGGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYGYGYGYYYYGGGGYYYYYGYGYGYYYYYGGYYYGYGYGYYYGGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYGYGYGYYYYGGGGYYYYYGYGYGYYYYYGGYYYGYGYGYYYGGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYYGYGYGYYGGGGGYYYYGGGGGYGGGYGYGYYYYYYGYGYGYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYYGYGYGYYGGGGGYYYYGGGGGYGGGYGYGYYYYYYGYGYGYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYYGYGYGYYGGGGGYYYYGGGGGYGGGYGYGYYYYYYGYGYGYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYYGYGYGYYGGGGGYYYYGGGGGYGGGYGYGYYYYYYGYGYGYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYGYGYGYYYYYGGGGGYYYYGYYYYYYGYGYYGYYGYGYGYYYYYGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYGYGYGYYYYYGGGGGYYYYGYYYYYYGYGYYGYYGYGYGYYYYYGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYGYGYGYYYYYGGGGGYYYYGYYYYYYGYGYYGYYGYGYGYYYYYGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYGYGYGYYYYYGGGGGYYYYGYYYYYYGYGYYGYYGYGYGYYYYYGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYGYGYYYYGGYYYYGGGYGYGGGGGYGGGYGGGGYYGYGYYYYGYYYYYGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYGYGYYYYGGYYYYGGGYGYGGGGGYGGGYGGGGYYGYGYYYYGYYYYYGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYGYGYYYYGGYYYYGGGYGYGGGGGYGGGYGGGGYYGYGYYYYGYYYYYGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYGYGYYYYGGYYYYGGGYGYGGGGGYGGGYGGGGYYGYGYYYYGYYYYYGGYYYYYYYGYGGGYGYGGYG"
        },
        /*  3*/ { BARCODE_ULTRA, -1, -1, -1, -1, 13, "FF00007F", "00FF0000", "1", 0, 13, 13, 13, 13, 13,
                    "1111111111111"
                    "10Y10GYCGYYC1"
                    "11C10MGYCGGG1"
                    "10G10GYCMCYC1"
                    "11Y10YMMGYGY1"
                    "10M10CGGCMYM1"
                    "1101010101011"
                    "10G10CYMGCCC1"
                    "11C10MCGCMMM1"
                    "10Y10CGCGYCY1"
                    "11M10GMMMMGC1"
                    "10C10MYYYGMY1"
                    "1111111111111"
        },
        /*  4*/ { BARCODE_ULTRA, -1, -1, 1, -1, 13, "", "00FF0000", "1", 0, 13, 13, 13, 15, 13,
                    "G1111111111111G"
                    "G10Y10GYCGYYC1G"
                    "G11C10MGYCGGG1G"
                    "G10G10GYCMCYC1G"
                    "G11Y10YMMGYGY1G"
                    "G10M10CGGCMYM1G"
                    "G1101010101011G"
                    "G10G10CYMGCCC1G"
                    "G11C10MCGCMMM1G"
                    "G10Y10CGCGYCY1G"
                    "G11M10GMMMMGC1G"
                    "G10C10MYYYGMY1G"
                    "G1111111111111G"
        },
        /*  5*/ { BARCODE_CHANNEL, -1, -1, 1, -1, 5, "30313233", "CFCECDCC", "1", 0, 5, 1, 19, 21, 5,
                    "CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132303132CFCECD303132303132CFCECDCFCECDCFCECD303132CFCECD"
                    "CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132303132CFCECD303132303132CFCECDCFCECDCFCECD303132CFCECD"
                    "CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132303132CFCECD303132303132CFCECDCFCECDCFCECD303132CFCECD"
                    "CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132303132CFCECD303132303132CFCECDCFCECDCFCECD303132CFCECD"
                    "CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132CFCECD303132303132CFCECD303132303132CFCECDCFCECDCFCECD303132CFCECD"
        },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    int row, column;
    int fg_len, bg_len;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].height) {
            symbol->height = data[i].height;
        }
        if (*data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (*data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }
        /* Keep dumps small */
        symbol->show_hrt = 0;
        symbol->scale = 0.5f;

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); /* ZINT_DEBUG_TEST_PRINT 16 */

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, %s, %g, \"%s\", \"%s\", \"%s\", %s, %g, %d, %d, %d, %d,\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].option_1, data[i].option_2, data[i].whitespace_width, testUtilOutputOptionsName(data[i].output_options),
                    data[i].height, data[i].fgcolour, data[i].bgcolour, data[i].data, testUtilErrorName(data[i].ret),
                    symbol->height, symbol->rows, symbol->width, symbol->bitmap_width, symbol->bitmap_height);
            testUtilBitmapPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %g != %g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);

            ret = testUtilBitmapCmp(symbol, data[i].expected_bitmap, &row, &column);
            assert_zero(ret, "i:%d (%s) testUtilBitmapCmp ret %d != 0 column %d row %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, column, row, data[i].data);

            fg_len = (int) strlen(data[i].fgcolour);
            bg_len = (int) strlen(data[i].bgcolour);
            if (fg_len > 6 || bg_len > 6) {
                assert_nonnull(symbol->alphamap, "i:%d ZBarcode_Buffer(%s) alphamap NULL\n", i, testUtilBarcodeName(data[i].symbology));
                /* TODO: check alphamap */
            } else {
                assert_null(symbol->alphamap, "i:%d ZBarcode_Buffer(%s) alphamap not NULL\n", i, testUtilBarcodeName(data[i].symbology));
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_height(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int output_options;
        float height;
        const char *data;
        const char *composite;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        const char *expected_errtxt;

        const char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, 1, "1234567890", "", 0, 1, 1, 108, 216, 2, "", "" },
        /*  1*/ { BARCODE_CODE11, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 108, 216, 2, "", "TODO: Find doc" },
        /*  2*/ { BARCODE_CODE11, -1, 4, "1234567890", "", 0, 4, 1, 108, 216, 8, "", "" },
        /*  3*/ { BARCODE_CODE11, -1, 10, "1234567890", "", 0, 10, 1, 108, 216, 20, "", "" },
        /*  4*/ { BARCODE_C25STANDARD, -1, 1, "1234567890", "", 0, 1, 1, 117, 234, 2, "", "" },
        /*  5*/ { BARCODE_C25STANDARD, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 117, 234, 2, "", "No doc for C25 except C25INTER" },
        /*  6*/ { BARCODE_C25STANDARD, -1, 4, "1234567890", "", 0, 4, 1, 117, 234, 8, "", "" },
        /*  7*/ { BARCODE_C25STANDARD, -1, 11, "1234567890", "", 0, 11, 1, 117, 234, 22, "", "" },
        /*  8*/ { BARCODE_C25INTER, -1, 1, "1234567890", "", 0, 1, 1, 99, 198, 2, "", "" },
        /*  9*/ { BARCODE_C25INTER, -1, 15, "1234567890", "", 0, 15, 1, 99, 198, 30, "", "" },
        /* 10*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 15, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15, 1, 99, 198, 30, "Warning 247: Height not compliant with standards (too small)", "Min height data-length dependent" },
        /* 11*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 15.5, "1234567890", "", 0, 15.5, 1, 99, 198, 31, "", "" },
        /* 12*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 17.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 17.5, 1, 117, 234, 35, "Warning 247: Height not compliant with standards (too small)", "Min height data-length dependent" },
        /* 13*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 17.75, "12345678901", "", 0, 18, 1, 117, 234, 36, "", "" },
        /* 14*/ { BARCODE_C25IATA, -1, 1, "1234567890", "", 0, 1, 1, 149, 298, 2, "", "" },
        /* 15*/ { BARCODE_C25IATA, -1, 4, "1234567890", "", 0, 4, 1, 149, 298, 8, "", "" },
        /* 16*/ { BARCODE_C25IATA, -1, 30, "1234567890", "", 0, 30, 1, 149, 298, 60, "", "" },
        /* 17*/ { BARCODE_C25LOGIC, -1, 1, "1234567890", "", 0, 1, 1, 109, 218, 2, "", "" },
        /* 18*/ { BARCODE_C25LOGIC, -1, 4, "1234567890", "", 0, 4, 1, 109, 218, 8, "", "" },
        /* 19*/ { BARCODE_C25LOGIC, -1, 41, "1234567890", "", 0, 41, 1, 109, 218, 82, "", "" },
        /* 20*/ { BARCODE_C25IND, -1, 1, "1234567890", "", 0, 1, 1, 159, 318, 2, "", "" },
        /* 21*/ { BARCODE_C25IND, -1, 4, "1234567890", "", 0, 4, 1, 159, 318, 8, "", "" },
        /* 22*/ { BARCODE_C25IND, -1, 21, "1234567890", "", 0, 21, 1, 159, 318, 42, "", "" },
        /* 23*/ { BARCODE_CODE39, -1, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "", "" },
        /* 24*/ { BARCODE_CODE39, -1, 4, "1", "", 0, 4, 1, 38, 76, 8, "", "" },
        /* 25*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 4, "1", "", ZINT_WARN_NONCOMPLIANT, 4, 1, 38, 76, 8, "Warning 247: Height not compliant with standards (too small)", "Min height data-length dependent" },
        /* 26*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 4.4, "1", "", 0, 4.5, 1, 38, 76, 9, "", "" },
        /* 27*/ { BARCODE_CODE39, -1, 17, "1234567890", "", 0, 17, 1, 155, 310, 34, "", "" },
        /* 28*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 17, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17, 1, 155, 310, 34, "Warning 247: Height not compliant with standards (too small)", "Min height data-length dependent" },
        /* 29*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 17.85, "1234567890", "", 0, 18, 1, 155, 310, 36, "", "" },
        /* 30*/ { BARCODE_EXCODE39, -1, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "", "" },
        /* 31*/ { BARCODE_EXCODE39, -1, 17.8, "1234567890", "", 0, 18, 1, 155, 310, 36, "", "" },
        /* 32*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, 17.8, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 18, 1, 155, 310, 36, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 33*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, 17.9, "1234567890", "", 0, 18, 1, 155, 310, 36, "", "" },
        /* 34*/ { BARCODE_EAN13, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "", "EAN-13" },
        /* 35*/ { BARCODE_EANX, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "", "EAN-13" },
        /* 36*/ { BARCODE_EAN13, -1, 69, "123456789012", "", 0, 69, 1, 95, 226, 148, "", "" },
        /* 37*/ { BARCODE_EANX, -1, 69, "123456789012", "", 0, 69, 1, 95, 226, 148, "", "" },
        /* 38*/ { BARCODE_EAN13, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 39*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 40*/ { BARCODE_EAN13, COMPLIANT_HEIGHT, 69.25, "123456789012", "", 0, 69.5, 1, 95, 226, 149, "", "" },
        /* 41*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 69.25, "123456789012", "", 0, 69.5, 1, 95, 226, 149, "", "" },
        /* 42*/ { BARCODE_EAN8, -1, 55, "1234567", "", 0, 55, 1, 67, 162, 120, "", "EAN-8" },
        /* 43*/ { BARCODE_EANX, -1, 55, "1234567", "", 0, 55, 1, 67, 162, 120, "", "EAN-8" },
        /* 44*/ { BARCODE_EAN8, COMPLIANT_HEIGHT, 55, "1234567", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 67, 162, 120, "Warning 247: Height not compliant with standards (too small)", "EAN-8" },
        /* 45*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 55, "1234567", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 67, 162, 120, "Warning 247: Height not compliant with standards (too small)", "EAN-8" },
        /* 46*/ { BARCODE_EAN8, COMPLIANT_HEIGHT, 55.25, "1234567", "", 0, 55.5, 1, 67, 162, 121, "", "EAN-8" },
        /* 47*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 55.25, "1234567", "", 0, 55.5, 1, 67, 162, 121, "", "EAN-8" },
        /* 48*/ { BARCODE_EANX_CHK, -1, 1, "1234567890128", "", 0, 1, 1, 95, 226, 12, "", "EAN-13" },
        /* 49*/ { BARCODE_EANX_CHK, -1, 69, "1234567890128", "", 0, 69, 1, 95, 226, 148, "", "" },
        /* 50*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, 69, "1234567890128", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 51*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, 69.25, "1234567890128", "", 0, 69.5, 1, 95, 226, 149, "", "" },
        /* 52*/ { BARCODE_GS1_128, -1, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "", "" },
        /* 53*/ { BARCODE_GS1_128, -1, 5.7, "[01]12345678901231", "", 0, 5.5, 1, 134, 268, 11, "", "" },
        /* 54*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, 5.7, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 5.5, 1, 134, 268, 11, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 55*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, 5.725, "[01]12345678901231", "", 0, 5.5, 1, 134, 268, 11, "", "Note considered compliant even though rendered height same as before" },
        /* 56*/ { BARCODE_GS1_128, -1, 50, "[01]12345678901231", "", 0, 50, 1, 134, 268, 100, "", "" },
        /* 57*/ { BARCODE_CODABAR, -1, 1, "A0B", "", 0, 1, 1, 32, 64, 2, "", "" },
        /* 58*/ { BARCODE_CODABAR, -1, 4, "A0B", "", 0, 4, 1, 32, 64, 8, "", "" },
        /* 59*/ { BARCODE_CODABAR, -1, 26, "A0B", "", 0, 26, 1, 32, 64, 52, "", "" },
        /* 60*/ { BARCODE_CODABAR, COMPLIANT_HEIGHT, 11, "A0B", "", ZINT_WARN_NONCOMPLIANT, 11, 1, 32, 64, 22, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 61*/ { BARCODE_CODABAR, COMPLIANT_HEIGHT, 12, "A0B", "", 0, 12, 1, 32, 64, 24, "", "" },
        /* 62*/ { BARCODE_CODE128, -1, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "", "" },
        /* 63*/ { BARCODE_CODE128, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "", "" },
        /* 64*/ { BARCODE_CODE128, -1, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "", "" },
        /* 65*/ { BARCODE_CODE128, -1, 7, "1234567890", "", 0, 7, 1, 90, 180, 14, "", "" },
        /* 66*/ { BARCODE_DPLEIT, -1, 1, "1234567890123", "", 0, 1, 1, 135, 270, 2, "", "" },
        /* 67*/ { BARCODE_DPLEIT, COMPLIANT_HEIGHT, 1, "1234567890123", "", 0, 1, 1, 135, 270, 2, "", "TODO: Find doc" },
        /* 68*/ { BARCODE_DPLEIT, -1, 4, "1234567890123", "", 0, 4, 1, 135, 270, 8, "", "" },
        /* 69*/ { BARCODE_DPIDENT, -1, 1, "12345678901", "", 0, 1, 1, 117, 234, 2, "", "" },
        /* 70*/ { BARCODE_DPIDENT, COMPLIANT_HEIGHT, 1, "12345678901", "", 0, 1, 1, 117, 234, 2, "", "TODO: Find doc" },
        /* 71*/ { BARCODE_DPIDENT, -1, 4, "12345678901", "", 0, 4, 1, 117, 234, 8, "", "" },
        /* 72*/ { BARCODE_CODE16K, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "", "2 rows" },
        /* 73*/ { BARCODE_CODE16K, -1, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "", "" },
        /* 74*/ { BARCODE_CODE16K, -1, 4, "1234567890", "", 0, 4, 2, 70, 162, 12, "", "" },
        /* 75*/ { BARCODE_CODE16K, -1, 16.75, "1234567890", "", 0, 17, 2, 70, 162, 38, "", "" },
        /* 76*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 16.75, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17, 2, 70, 162, 38, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 77*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "", "" },
        /* 78*/ { BARCODE_CODE16K, -1, -1, "12345678901234567890123456789012345678901234567890", "", 0, 60, 6, 70, 162, 124, "", "6 rows" },
        /* 79*/ { BARCODE_CODE16K, -1, 52.5, "12345678901234567890123456789012345678901234567890", "", 0, 54, 6, 70, 162, 112, "", "" },
        /* 80*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 52.5, "12345678901234567890123456789012345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 54, 6, 70, 162, 112, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 81*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 53, "12345678901234567890123456789012345678901234567890", "", 0, 54, 6, 70, 162, 112, "", "" },
        /* 82*/ { BARCODE_CODE49, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "", "2 rows" },
        /* 83*/ { BARCODE_CODE49, -1, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "", "" },
        /* 84*/ { BARCODE_CODE49, -1, 16.75, "1234567890", "", 0, 17, 2, 70, 162, 38, "", "" },
        /* 85*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 16.75, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17, 2, 70, 162, 38, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 86*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "", "" },
        /* 87*/ { BARCODE_CODE49, -1, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "", "3 rows" },
        /* 88*/ { BARCODE_CODE49, -1, 25.75, "12345678901234567890", "", 0, 25.5, 3, 70, 162, 55, "", "" },
        /* 89*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 25.75, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 25.5, 3, 70, 162, 55, "Warning 247: Height not compliant with standards (too small)", "" },
        /* 90*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 26, "12345678901234567890", "", 0, 25.5, 3, 70, 162, 55, "", "" },
        /* 91*/ { BARCODE_CODE93, -1, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "", "" },
        /* 92*/ { BARCODE_CODE93, -1, 9.9, "1", "", 0, 10, 1, 46, 92, 20, "", "" },
        /* 93*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 9.89, "1", "", ZINT_WARN_NONCOMPLIANT, 10, 1, 46, 92, 20, "Warning 247: Height not compliant with standards (too small)", "Min height data-length dependent" },
        /* 94*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 10, "1", "", 0, 10, 1, 46, 92, 20, "", "" },
        /* 95*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 22, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 22, 1, 127, 254, 44, "Warning 247: Height not compliant with standards (too small)", "Min height data-length dependent" },
        /* 96*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 22.1, "1234567890", "", 0, 22, 1, 127, 254, 44, "", "" },
        /* 97*/ { BARCODE_FLAT, -1, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "", "" },
        /* 98*/ { BARCODE_FLAT, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "", "TODO: Find doc" },
        /* 99*/ { BARCODE_FLAT, -1, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "", "" },
        /*100*/ { BARCODE_DBAR_OMN, -1, 1, "1234567890123", "", 0, 1, 1, 96, 192, 2, "", "" },
        /*101*/ { BARCODE_DBAR_OMN, -1, 12.9, "1234567890123", "", 0, 13, 1, 96, 192, 26, "", "" },
        /*102*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, 12.9, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 13, 1, 96, 192, 26, "Warning 247: Height not compliant with standards (too small)", "" },
        /*103*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, 13, "1234567890123", "", 0, 13, 1, 96, 192, 26, "", "" },
        /*104*/ { BARCODE_DBAR_LTD, -1, 1, "1234567890123", "", 0, 1, 1, 79, 158, 2, "", "" },
        /*105*/ { BARCODE_DBAR_LTD, -1, 9, "1234567890123", "", 0, 9, 1, 79, 158, 18, "", "" },
        /*106*/ { BARCODE_DBAR_LTD, COMPLIANT_HEIGHT, 9, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 9, 1, 79, 158, 18, "Warning 247: Height not compliant with standards (too small)", "" },
        /*107*/ { BARCODE_DBAR_LTD, COMPLIANT_HEIGHT, 10, "1234567890123", "", 0, 10, 1, 79, 158, 20, "", "" },
        /*108*/ { BARCODE_DBAR_EXP, -1, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "", "" },
        /*109*/ { BARCODE_DBAR_EXP, -1, 33, "[01]12345678901231", "", 0, 33, 1, 134, 268, 66, "", "" },
        /*110*/ { BARCODE_DBAR_EXP, COMPLIANT_HEIGHT, 33, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 33, 1, 134, 268, 66, "Warning 247: Height not compliant with standards (too small)", "" },
        /*111*/ { BARCODE_DBAR_EXP, COMPLIANT_HEIGHT, 34, "[01]12345678901231", "", 0, 34, 1, 134, 268, 68, "", "" },
        /*112*/ { BARCODE_TELEPEN, -1, 1, "1234567890", "", 0, 1, 1, 208, 416, 2, "", "" },
        /*113*/ { BARCODE_TELEPEN, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 208, 416, 2, "", "No known min" },
        /*114*/ { BARCODE_TELEPEN, -1, 4, "1234567890", "", 0, 4, 1, 208, 416, 8, "", "" },
        /*115*/ { BARCODE_UPCA, -1, 1, "12345678901", "", 0, 1, 1, 95, 226, 12, "", "" },
        /*116*/ { BARCODE_UPCA, -1, 69, "12345678901", "", 0, 69, 1, 95, 226, 148, "", "" },
        /*117*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, 69, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "Warning 247: Height not compliant with standards (too small)", "" },
        /*118*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, 69.25, "12345678901", "", 0, 69.5, 1, 95, 226, 149, "", "" },
        /*119*/ { BARCODE_UPCA_CHK, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "", "" },
        /*120*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "Warning 247: Height not compliant with standards (too small)", "" },
        /*121*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, 69.5, "123456789012", "", 0, 69.5, 1, 95, 226, 149, "", "" },
        /*122*/ { BARCODE_UPCE, -1, 1, "1234567", "", 0, 1, 1, 51, 134, 12, "", "" },
        /*123*/ { BARCODE_UPCE, -1, 69, "1234567", "", 0, 69, 1, 51, 134, 148, "", "" },
        /*124*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, 69, "1234567", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 51, 134, 148, "Warning 247: Height not compliant with standards (too small)", "" },
        /*125*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, 69.25, "1234567", "", 0, 69.5, 1, 51, 134, 149, "", "" },
        /*126*/ { BARCODE_UPCE_CHK, -1, 1, "12345670", "", 0, 1, 1, 51, 134, 12, "", "" },
        /*127*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, 69.24, "12345670", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 51, 134, 148, "Warning 247: Height not compliant with standards (too small)", "" },
        /*128*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, 69.25, "12345670", "", 0, 69.5, 1, 51, 134, 149, "", "" },
        /*129*/ { BARCODE_POSTNET, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "", "" },
        /*130*/ { BARCODE_POSTNET, -1, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "", "" },
        /*131*/ { BARCODE_POSTNET, -1, 4.5, "12345678901", "", 0, 5, 2, 123, 246, 10, "", "" },
        /*132*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 4.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 123, 246, 9, "Warning 498: Height not compliant with standards", "" },
        /*133*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 4.6, "12345678901", "", 0, 5, 2, 123, 246, 10, "", "" },
        /*134*/ { BARCODE_POSTNET, -1, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "", "" },
        /*135*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "", "" },
        /*136*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 9.1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 9, 2, 123, 246, 18, "Warning 498: Height not compliant with standards", "" },
        /*137*/ { BARCODE_POSTNET, -1, 20, "12345678901", "", 0, 20, 2, 123, 246, 40, "", "" },
        /*138*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 20, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 20, 2, 123, 246, 40, "Warning 498: Height not compliant with standards", "" },
        /*139*/ { BARCODE_MSI_PLESSEY, -1, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "", "" },
        /*140*/ { BARCODE_MSI_PLESSEY, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "", "TODO: Find doc" },
        /*141*/ { BARCODE_MSI_PLESSEY, -1, 4, "1234567890", "", 0, 4, 1, 127, 254, 8, "", "" },
        /*142*/ { BARCODE_FIM, -1, 1, "A", "", 0, 1, 1, 17, 34, 2, "", "" },
        /*143*/ { BARCODE_FIM, -1, 12.7, "A", "", 0, 12.5, 1, 17, 34, 25, "", "" },
        /*144*/ { BARCODE_FIM, COMPLIANT_HEIGHT, 12.7, "A", "", ZINT_WARN_NONCOMPLIANT, 12.5, 1, 17, 34, 25, "Warning 247: Height not compliant with standards (too small)", "" },
        /*145*/ { BARCODE_FIM, COMPLIANT_HEIGHT, 12.8, "A", "", 0, 13, 1, 17, 34, 26, "", "" },
        /*146*/ { BARCODE_LOGMARS, -1, 1, "1234567890", "", 0, 1, 1, 191, 382, 2, "", "" },
        /*147*/ { BARCODE_LOGMARS, -1, 6, "1234567890", "", 0, 6, 1, 191, 382, 12, "", "" },
        /*148*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 6, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6, 1, 191, 382, 12, "Warning 247: Height not compliant with standards (too small)", "" },
        /*149*/ { BARCODE_LOGMARS, -1, 6.25, "1234567890", "", 0, 6.5, 1, 191, 382, 13, "", "" },
        /*150*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 6.25, "1234567890", "", 0, 6.5, 1, 191, 382, 13, "", "" },
        /*151*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 116, "1234567890", "", 0, 116, 1, 191, 382, 232, "", "" },
        /*152*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 117, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 117, 1, 191, 382, 234, "Warning 248: Height not compliant with standards (maximum 116.7)", "" },
        /*153*/ { BARCODE_PHARMA, -1, 1, "123456", "", 0, 1, 1, 58, 116, 2, "", "" },
        /*154*/ { BARCODE_PHARMA, -1, 15, "123456", "", 0, 15, 1, 58, 116, 30, "", "" },
        /*155*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, 15, "123456", "", ZINT_WARN_NONCOMPLIANT, 15, 1, 58, 116, 30, "Warning 247: Height not compliant with standards (too small)", "" },
        /*156*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, 16, "123456", "", 0, 16, 1, 58, 116, 32, "", "" },
        /*157*/ { BARCODE_PZN, -1, 1, "123456", "", 0, 1, 1, 142, 284, 2, "", "" },
        /*158*/ { BARCODE_PZN, -1, 17.7, "123456", "", 0, 17.5, 1, 142, 284, 35, "", "" },
        /*159*/ { BARCODE_PZN, COMPLIANT_HEIGHT, 17.7, "123456", "", ZINT_WARN_NONCOMPLIANT, 17.5, 1, 142, 284, 35, "Warning 247: Height not compliant with standards (too small)", "" },
        /*160*/ { BARCODE_PZN, COMPLIANT_HEIGHT, 17.8, "123456", "", 0, 18, 1, 142, 284, 36, "", "" },
        /*161*/ { BARCODE_PHARMA_TWO, -1, -1, "12345678", "", 0, 10, 2, 29, 58, 20, "", "" },
        /*162*/ { BARCODE_PHARMA_TWO, -1, 1, "12345678", "", 0, 1, 2, 29, 58, 2, "", "" },
        /*163*/ { BARCODE_PHARMA_TWO, -1, 3.9, "12345678", "", 0, 4, 2, 29, 58, 8, "", "" },
        /*164*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 3.9, "12345678", "", ZINT_WARN_NONCOMPLIANT, 4, 2, 29, 58, 8, "Warning 247: Height not compliant with standards (too small)", "" },
        /*165*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 4, "12345678", "", 0, 4, 2, 29, 58, 8, "", "" },
        /*166*/ { BARCODE_PHARMA_TWO, -1, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "", "" },
        /*167*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "", "" },
        /*168*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 15.1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 15, 2, 29, 58, 30, "Warning 248: Height not compliant with standards (maximum 15)", "" },
        /*169*/ { BARCODE_CEPNET, -1, -1, "12345678", "", 0, 5, 2, 93, 186, 10, "", "" },
        /*170*/ { BARCODE_CEPNET, -1, 1, "12345678", "", 0, 1.5, 2, 93, 186, 3, "", "" },
        /*171*/ { BARCODE_CEPNET, -1, 4.5, "12345678", "", 0, 4.5, 2, 93, 186, 9, "", "" },
        /*172*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 4.5, "12345678", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 93, 186, 9, "Warning 498: Height not compliant with standards", "" },
        /*173*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 4.6, "12345678", "", 0, 5, 2, 93, 186, 10, "", "" },
        /*174*/ { BARCODE_CEPNET, -1, 9, "12345678", "", 0, 9, 2, 93, 186, 18, "", "" },
        /*175*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 9, "12345678", "", 0, 9, 2, 93, 186, 18, "", "" },
        /*176*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 9.1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 9, 2, 93, 186, 18, "Warning 498: Height not compliant with standards", "" },
        /*177*/ { BARCODE_CEPNET, -1, 20, "12345678", "", 0, 20, 2, 93, 186, 40, "", "" },
        /*178*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 20, "12345678", "", ZINT_WARN_NONCOMPLIANT, 20, 2, 93, 186, 40, "Warning 498: Height not compliant with standards", "" },
        /*179*/ { BARCODE_PDF417, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "Warning 247: Height not compliant with standards (too small)", "8 rows" },
        /*180*/ { BARCODE_PDF417, -1, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "Warning 247: Height not compliant with standards (too small)", "" },
        /*181*/ { BARCODE_PDF417, -1, 9, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 8, 8, 120, 240, 16, "Warning 247: Height not compliant with standards (too small)", "" },
        /*182*/ { BARCODE_PDF417, -1, 10, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 8, 120, 240, 24, "Warning 247: Height not compliant with standards (too small)", "" },
        /*183*/ { BARCODE_PDF417, -1, 11, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 8, 120, 240, 24, "Warning 247: Height not compliant with standards (too small)", "" },
        /*184*/ { BARCODE_PDF417, -1, 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 8, 120, 240, 24, "Warning 247: Height not compliant with standards (too small)", "" },
        /*185*/ { BARCODE_PDF417, -1, 13, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 8, 120, 240, 24, "Warning 247: Height not compliant with standards (too small)", "" },
        /*186*/ { BARCODE_PDF417, -1, 14, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 16, 8, 120, 240, 32, "Warning 247: Height not compliant with standards (too small)", "" },
        /*187*/ { BARCODE_PDF417, -1, 15, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 16, 8, 120, 240, 32, "Warning 247: Height not compliant with standards (too small)", "" },
        /*188*/ { BARCODE_PDF417, -1, 16, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 16, 8, 120, 240, 32, "Warning 247: Height not compliant with standards (too small)", "" },
        /*189*/ { BARCODE_PDF417, -1, 17, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 16, 8, 120, 240, 32, "Warning 247: Height not compliant with standards (too small)", "" },
        /*190*/ { BARCODE_PDF417, -1, 18, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 8, 120, 240, 40, "Warning 247: Height not compliant with standards (too small)", "" },
        /*191*/ { BARCODE_PDF417, -1, 19, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 8, 120, 240, 40, "Warning 247: Height not compliant with standards (too small)", "" },
        /*192*/ { BARCODE_PDF417, -1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 8, 120, 240, 40, "Warning 247: Height not compliant with standards (too small)", "" },
        /*193*/ { BARCODE_PDF417, -1, 21, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 8, 120, 240, 40, "Warning 247: Height not compliant with standards (too small)", "" },
        /*194*/ { BARCODE_PDF417, -1, 22, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 24, 8, 120, 240, 48, "Warning 247: Height not compliant with standards (too small)", "" },
        /*195*/ { BARCODE_PDF417, -1, 23, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 24, 8, 120, 240, 48, "Warning 247: Height not compliant with standards (too small)", "" },
        /*196*/ { BARCODE_PDF417, -1, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 24, 8, 120, 240, 48, "", "" },
        /*197*/ { BARCODE_PDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 36, 12, 120, 240, 72, "", "12 rows" },
        /*198*/ { BARCODE_PDF417, -1, 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 12, 120, 240, 24, "Warning 247: Height not compliant with standards (too small)", "" },
        /*199*/ { BARCODE_PDF417COMP, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 4.5, 9, 86, 172, 9, "Warning 247: Height not compliant with standards (too small)", "9 rows" },
        /*200*/ { BARCODE_PDF417COMP, -1, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 22.5, 9, 86, 172, 45, "Warning 247: Height not compliant with standards (too small)", "" },
        /*201*/ { BARCODE_MAXICODE, -1, 1, "1234567890", "", 0, 16.5, 33, 30, 299, 298, "", "Fixed size, symbol->height ignored" },
        /*202*/ { BARCODE_QRCODE, -1, 1, "ABCD", "", 0, 21, 21, 21, 42, 42, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*203*/ { BARCODE_CODE128AB, -1, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "", "" },
        /*204*/ { BARCODE_CODE128AB, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "", "" },
        /*205*/ { BARCODE_CODE128AB, -1, 4, "1234567890", "", 0, 4, 1, 145, 290, 8, "", "" },
        /*206*/ { BARCODE_AUSPOST, -1, -1, "12345678901234567890123", "", 0, 8, 3, 133, 266, 16, "", "" },
        /*207*/ { BARCODE_AUSPOST, -1, 1, "12345678901234567890123", "", 0, 2.5, 3, 133, 266, 5, "", "" },
        /*208*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 1, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 133, 266, 3, "Warning 499: Height not compliant with standards", "" },
        /*209*/ { BARCODE_AUSPOST, -1, 6.9, "12345678901234567890123", "", 0, 6.5, 3, 133, 266, 13, "", "" },
        /*210*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 6.9, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 7, 3, 133, 266, 14, "Warning 499: Height not compliant with standards", "" },
        /*211*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 7, "12345678901234567890123", "", 0, 7, 3, 133, 266, 14, "", "" },
        /*212*/ { BARCODE_AUSPOST, -1, 14, "12345678901234567890123", "", 0, 14.5, 3, 133, 266, 29, "", "" },
        /*213*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 14, "12345678901234567890123", "", 0, 13.5, 3, 133, 266, 27, "", "" },
        /*214*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 14.1, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 13.5, 3, 133, 266, 27, "Warning 499: Height not compliant with standards", "" },
        /*215*/ { BARCODE_AUSREPLY, -1, 14, "12345678", "", 0, 14.5, 3, 73, 146, 29, "", "" },
        /*216*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, 14, "12345678", "", 0, 13.5, 3, 73, 146, 27, "", "" },
        /*217*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, 14.25, "12345678", "", ZINT_WARN_NONCOMPLIANT, 14.5, 3, 73, 146, 29, "Warning 499: Height not compliant with standards", "" },
        /*218*/ { BARCODE_AUSROUTE, -1, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "", "" },
        /*219*/ { BARCODE_AUSROUTE, COMPLIANT_HEIGHT, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "", "" },
        /*220*/ { BARCODE_AUSREDIRECT, -1, 14, "12345678", "", 0, 14.5, 3, 73, 146, 29, "", "" },
        /*221*/ { BARCODE_AUSREDIRECT, COMPLIANT_HEIGHT, 14, "12345678", "", 0, 13.5, 3, 73, 146, 27, "", "" },
        /*222*/ { BARCODE_ISBNX, -1, 1, "123456789", "", 0, 1, 1, 95, 226, 12, "", "" },
        /*223*/ { BARCODE_ISBNX, -1, 69, "123456789", "", 0, 69, 1, 95, 226, 148, "", "" },
        /*224*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, 69, "123456789", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "Warning 247: Height not compliant with standards (too small)", "" },
        /*225*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, 69.5, "123456789", "", 0, 69.5, 1, 95, 226, 149, "", "" },
        /*226*/ { BARCODE_RM4SCC, -1, -1, "1234567890", "", 0, 8, 3, 91, 182, 16, "", "" },
        /*227*/ { BARCODE_RM4SCC, -1, 1, "1234567890", "", 0, 2.5, 3, 91, 182, 5, "", "" },
        /*228*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 91, 182, 3, "Warning 499: Height not compliant with standards", "" },
        /*229*/ { BARCODE_RM4SCC, -1, 4, "1234567890", "", 0, 4, 3, 91, 182, 8, "", "" },
        /*230*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4, 3, 91, 182, 8, "Warning 499: Height not compliant with standards", "" },
        /*231*/ { BARCODE_RM4SCC, -1, 6, "1234567890", "", 0, 6.5, 3, 91, 182, 13, "", "" },
        /*232*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 6, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 5.5, 3, 91, 182, 11, "Warning 499: Height not compliant with standards", "" },
        /*233*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 6.5, "1234567890", "", 0, 6.5, 3, 91, 182, 13, "", "" },
        /*234*/ { BARCODE_RM4SCC, -1, 10.8, "1234567890", "", 0, 10.5, 3, 91, 182, 21, "", "" },
        /*235*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 10.8, "1234567890", "", 0, 11, 3, 91, 182, 22, "", "" },
        /*236*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 11, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 91, 182, 22, "Warning 499: Height not compliant with standards", "" },
        /*237*/ { BARCODE_RM4SCC, -1, 16, "1234567890", "", 0, 16, 3, 91, 182, 32, "", "" },
        /*238*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 3, 91, 182, 32, "Warning 499: Height not compliant with standards", "" },
        /*239*/ { BARCODE_DATAMATRIX, -1, 1, "ABCD", "", 0, 12, 12, 12, 24, 24, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*240*/ { BARCODE_EAN14, -1, 1, "1234567890123", "", 0, 1, 1, 134, 268, 2, "", "" },
        /*241*/ { BARCODE_EAN14, -1, 5.7, "1234567890123", "", 0, 5.5, 1, 134, 268, 11, "", "" },
        /*242*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, 5.7, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 5.5, 1, 134, 268, 11, "Warning 247: Height not compliant with standards (too small)", "" },
        /*243*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, 5.8, "1234567890123", "", 0, 6, 1, 134, 268, 12, "", "" },
        /*244*/ { BARCODE_VIN, -1, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "", "" },
        /*245*/ { BARCODE_VIN, COMPLIANT_HEIGHT, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "", "Doc unlikely" },
        /*246*/ { BARCODE_VIN, -1, 4, "12345678701234567", "", 0, 4, 1, 246, 492, 8, "", "" },
        /*247*/ { BARCODE_CODABLOCKF, -1, -1, "1234567890", "", 0, 20, 2, 101, 242, 44, "", "2 rows" },
        /*248*/ { BARCODE_CODABLOCKF, -1, 1, "1234567890", "", 0, 1, 2, 101, 242, 6, "", "" },
        /*249*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1, 2, 101, 242, 6, "Warning 247: Height not compliant with standards (too small)", "" },
        /*250*/ { BARCODE_CODABLOCKF, -1, 15.5, "1234567890", "", 0, 16, 2, 101, 242, 36, "", "" },
        /*251*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 15.5, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 2, 101, 242, 36, "Warning 247: Height not compliant with standards (too small)", "" },
        /*252*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 16, "1234567890", "", 0, 16, 2, 101, 242, 36, "", "" },
        /*253*/ { BARCODE_CODABLOCKF, -1, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 110, 11, 156, 352, 224, "", "11 rows, 14 cols" },
        /*254*/ { BARCODE_CODABLOCKF, -1, 87, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 88, 11, 156, 352, 180, "", "" },
        /*255*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 87, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", ZINT_WARN_NONCOMPLIANT, 88, 11, 156, 352, 180, "Warning 247: Height not compliant with standards (too small)", "" },
        /*256*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 88, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 88, 11, 156, 352, 180, "", "" },
        /*257*/ { BARCODE_CODABLOCKF, -1, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "", "" },
        /*258*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "", "" },
        /*259*/ { BARCODE_CODABLOCKF, -1, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 110, 11, 167, 374, 224, "", "11 rows, 15 cols" },
        /*260*/ { BARCODE_CODABLOCKF, -1, 93, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 93.5, 11, 167, 374, 191, "", "" },
        /*261*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 93, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", ZINT_WARN_NONCOMPLIANT, 93.5, 11, 167, 374, 191, "Warning 247: Height not compliant with standards (too small)", "" },
        /*262*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 93.5, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 93.5, 11, 167, 374, 191, "", "" },
        /*263*/ { BARCODE_NVE18, -1, 1, "12345678901234567", "", 0, 1, 1, 156, 312, 2, "", "" },
        /*264*/ { BARCODE_NVE18, -1, 5.7, "12345678901234567", "", 0, 5.5, 1, 156, 312, 11, "", "" },
        /*265*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, 5.7, "12345678901234567", "", ZINT_WARN_NONCOMPLIANT, 5.5, 1, 156, 312, 11, "Warning 247: Height not compliant with standards (too small)", "" },
        /*266*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, 5.8, "12345678901234567", "", 0, 6, 1, 156, 312, 12, "", "" },
        /*267*/ { BARCODE_JAPANPOST, -1, -1, "1234567890", "", 0, 8, 3, 133, 266, 16, "", "" },
        /*268*/ { BARCODE_JAPANPOST, -1, 1, "1234567890", "", 0, 2.5, 3, 133, 266, 5, "", "" },
        /*269*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 133, 266, 3, "Warning 499: Height not compliant with standards", "" },
        /*270*/ { BARCODE_JAPANPOST, -1, 4.8, "1234567890", "", 0, 5, 3, 133, 266, 10, "", "" },
        /*271*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 4.8, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4.5, 3, 133, 266, 9, "Warning 499: Height not compliant with standards", "" },
        /*272*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 4.9, "1234567890", "", 0, 4.5, 3, 133, 266, 9, "", "" },
        /*273*/ { BARCODE_JAPANPOST, -1, 7, "1234567890", "", 0, 7, 3, 133, 266, 14, "", "" },
        /*274*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 7, "1234567890", "", 0, 7.5, 3, 133, 266, 15, "", "" },
        /*275*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 7.5, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 7.5, 3, 133, 266, 15, "Warning 499: Height not compliant with standards", "" },
        /*276*/ { BARCODE_JAPANPOST, -1, 16, "1234567890", "", 0, 16, 3, 133, 266, 32, "", "" },
        /*277*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16.5, 3, 133, 266, 33, "Warning 499: Height not compliant with standards", "" },
        /*278*/ { BARCODE_KOREAPOST, -1, 1, "123456", "", 0, 1, 1, 167, 334, 2, "", "" },
        /*279*/ { BARCODE_KOREAPOST, COMPLIANT_HEIGHT, 1, "123456", "", 0, 1, 1, 167, 334, 2, "", "TODO: Find doc" },
        /*280*/ { BARCODE_KOREAPOST, -1, 4, "123456", "", 0, 4, 1, 167, 334, 8, "", "" },
        /*281*/ { BARCODE_DBAR_STK, -1, -1, "1234567890123", "", 0, 13, 3, 50, 100, 26, "", "" },
        /*282*/ { BARCODE_DBAR_STK, -1, 1, "1234567890123", "", 0, 2, 3, 50, 100, 4, "", "" },
        /*283*/ { BARCODE_DBAR_STK, -1, 12, "1234567890123", "", 0, 12, 3, 50, 100, 24, "", "" },
        /*284*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, 12, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 12, 3, 50, 100, 24, "Warning 379: Height not compliant with standards", "" },
        /*285*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, 13, "1234567890123", "", 0, 13, 3, 50, 100, 26, "", "" },
        /*286*/ { BARCODE_DBAR_OMNSTK, -1, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138, "", "" },
        /*287*/ { BARCODE_DBAR_OMNSTK, -1, 1, "1234567890123", "", 0, 4, 5, 50, 100, 8, "", "" },
        /*288*/ { BARCODE_DBAR_OMNSTK, -1, 68, "1234567890123", "", 0, 68, 5, 50, 100, 136, "", "" },
        /*289*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, 68, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 68, 5, 50, 100, 136, "Warning 247: Height not compliant with standards (too small)", "" },
        /*290*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, 69, "1234567890123", "", 0, 69, 5, 50, 100, 138, "", "" },
        /*291*/ { BARCODE_DBAR_EXPSTK, -1, -1, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "", "2 rows" },
        /*292*/ { BARCODE_DBAR_EXPSTK, -1, 1, "[01]12345678901231", "", 0, 4, 5, 102, 204, 8, "", "" },
        /*293*/ { BARCODE_DBAR_EXPSTK, -1, 70, "[01]12345678901231", "", 0, 70, 5, 102, 204, 140, "", "" },
        /*294*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 70, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 70, 5, 102, 204, 140, "Warning 247: Height not compliant with standards (too small)", "" },
        /*295*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 71, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "", "" },
        /*296*/ { BARCODE_DBAR_EXPSTK, -1, -1, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "", "3 rows" },
        /*297*/ { BARCODE_DBAR_EXPSTK, -1, 1, "[01]09501101530003[3920]123456789012345", "", 0, 7.5, 9, 102, 204, 15, "", "" },
        /*298*/ { BARCODE_DBAR_EXPSTK, -1, 107.5, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "", "" },
        /*299*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 107.5, "[01]09501101530003[3920]123456789012345", "", ZINT_WARN_NONCOMPLIANT, 108, 9, 102, 204, 216, "Warning 247: Height not compliant with standards (too small)", "" },
        /*300*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 108, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "", "" },
        /*301*/ { BARCODE_PLANET, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "", "" },
        /*302*/ { BARCODE_PLANET, -1, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "", "" },
        /*303*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 1.5, 2, 123, 246, 3, "Warning 498: Height not compliant with standards", "" },
        /*304*/ { BARCODE_PLANET, -1, 4.5, "12345678901", "", 0, 5, 2, 123, 246, 10, "", "" },
        /*305*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 4.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 123, 246, 9, "Warning 498: Height not compliant with standards", "" },
        /*306*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 4.6, "12345678901", "", 0, 5, 2, 123, 246, 10, "", "" },
        /*307*/ { BARCODE_PLANET, -1, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "", "" },
        /*308*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "", "" },
        /*309*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 9.1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 9, 2, 123, 246, 18, "Warning 498: Height not compliant with standards", "" },
        /*310*/ { BARCODE_PLANET, -1, 24, "12345678901", "", 0, 24, 2, 123, 246, 48, "", "" },
        /*311*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 24, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 24, 2, 123, 246, 48, "Warning 498: Height not compliant with standards", "" },
        /*312*/ { BARCODE_MICROPDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 20, 10, 82, 164, 40, "", "10 rows" },
        /*313*/ { BARCODE_MICROPDF417, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "Warning 247: Height not compliant with standards (too small)", "" },
        /*314*/ { BARCODE_MICROPDF417, -1, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "Warning 247: Height not compliant with standards (too small)", "" },
        /*315*/ { BARCODE_MICROPDF417, -1, 19, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 10, 82, 164, 40, "Warning 247: Height not compliant with standards (too small)", "" },
        /*316*/ { BARCODE_MICROPDF417, -1, 30, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 30, 10, 82, 164, 60, "", "" },
        /*317*/ { BARCODE_MICROPDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 40, 20, 55, 110, 80, "", "20 rows" },
        /*318*/ { BARCODE_MICROPDF417, -1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 20, 55, 110, 40, "Warning 247: Height not compliant with standards (too small)", "" },
        /*319*/ { BARCODE_USPS_IMAIL, -1, -1, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, "", "" },
        /*320*/ { BARCODE_USPS_IMAIL, -1, 1, "12345678901234567890", "", 0, 2.5, 3, 129, 258, 5, "", "" },
        /*321*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 1, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 129, 258, 3, "Warning 499: Height not compliant with standards", "" },
        /*322*/ { BARCODE_USPS_IMAIL, -1, 4.8, "12345678901234567890", "", 0, 5, 3, 129, 258, 10, "", "" },
        /*323*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 4.8, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 4.5, 3, 129, 258, 9, "Warning 499: Height not compliant with standards", "" },
        /*324*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 4.9, "12345678901234567890", "", 0, 4.5, 3, 129, 258, 9, "", "" },
        /*325*/ { BARCODE_USPS_IMAIL, -1, 7.7, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, "", "" },
        /*326*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 7.7, "12345678901234567890", "", 0, 7.5, 3, 129, 258, 15, "", "" },
        /*327*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 7.8, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 7.5, 3, 129, 258, 15, "Warning 499: Height not compliant with standards", "" },
        /*328*/ { BARCODE_PLESSEY, -1, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "", "" },
        /*329*/ { BARCODE_PLESSEY, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "", "TODO: Find doc" },
        /*330*/ { BARCODE_PLESSEY, -1, 4, "1234567890", "", 0, 4, 1, 227, 454, 8, "", "" },
        /*331*/ { BARCODE_TELEPEN_NUM, -1, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "", "" },
        /*332*/ { BARCODE_TELEPEN_NUM, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "", "No known min" },
        /*333*/ { BARCODE_TELEPEN_NUM, -1, 4, "1234567890", "", 0, 4, 1, 128, 256, 8, "", "" },
        /*334*/ { BARCODE_ITF14, -1, 1, "1234567890", "", 0, 1, 1, 135, 330, 22, "", "" },
        /*335*/ { BARCODE_ITF14, -1, 5.7, "1234567890", "", 0, 5.5, 1, 135, 330, 31, "", "" },
        /*336*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, 5.7, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 5.5, 1, 135, 330, 31, "Warning 247: Height not compliant with standards (too small)", "" },
        /*337*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, 5.8, "1234567890", "", 0, 6, 1, 135, 330, 32, "", "" },
        /*338*/ { BARCODE_KIX, -1, -1, "1234567890", "", 0, 8, 3, 79, 158, 16, "", "" },
        /*339*/ { BARCODE_KIX, -1, 1, "1234567890", "", 0, 2.5, 3, 79, 158, 5, "", "" },
        /*340*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 79, 158, 3, "Warning 499: Height not compliant with standards", "" },
        /*341*/ { BARCODE_KIX, -1, 6.4, "1234567890", "", 0, 6.5, 3, 79, 158, 13, "", "" },
        /*342*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 6.4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6.5, 3, 79, 158, 13, "Warning 499: Height not compliant with standards", "" },
        /*343*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 6.5, "1234567890", "", 0, 6.5, 3, 79, 158, 13, "", "" },
        /*344*/ { BARCODE_KIX, -1, 10.8, "1234567890", "", 0, 10.5, 3, 79, 158, 21, "", "" },
        /*345*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 10.8, "1234567890", "", 0, 11, 3, 79, 158, 22, "", "" },
        /*346*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 10.9, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 79, 158, 22, "Warning 499: Height not compliant with standards", "" },
        /*347*/ { BARCODE_KIX, -1, 16, "1234567890", "", 0, 16, 3, 79, 158, 32, "", "" },
        /*348*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 3, 79, 158, 32, "Warning 499: Height not compliant with standards", "" },
        /*349*/ { BARCODE_AZTEC, -1, 1, "1234567890AB", "", 0, 15, 15, 15, 30, 30, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*350*/ { BARCODE_DAFT, -1, -1, "DAFTDAFTDAFTDAFT", "", 0, 8, 3, 31, 62, 16, "", "" },
        /*351*/ { BARCODE_DAFT, -1, 1, "DAFTDAFTDAFTDAFT", "", 0, 2.5, 3, 31, 62, 5, "", "" },
        /*352*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, 1, "DAFTDAFTDAFTDAFT", "", 0, 2.5, 3, 31, 62, 5, "", "" },
        /*353*/ { BARCODE_DAFT, -1, 4, "DAFTDAFTDAFTDAFT", "", 0, 4, 3, 31, 62, 8, "", "" },
        /*354*/ { BARCODE_DAFT, -1, 6, "DAFTDAFTDAFTDAFT", "", 0, 6.5, 3, 31, 62, 13, "", "" },
        /*355*/ { BARCODE_DAFT, -1, 12, "DAFTDAFTDAFTDAFT", "", 0, 12, 3, 31, 62, 24, "", "" },
        /*356*/ { BARCODE_DAFT, -1, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "", "" },
        /*357*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "", "" },
        /*358*/ { BARCODE_DPD, -1, 1, "0123456789012345678901234567", "", 0, 1, 1, 189, 378, 8, "", "" },
        /*359*/ { BARCODE_DPD, -1, 62, "0123456789012345678901234567", "", 0, 62, 1, 189, 378, 130, "", "" },
        /*360*/ { BARCODE_DPD, COMPLIANT_HEIGHT, 62, "0123456789012345678901234567", "", ZINT_WARN_NONCOMPLIANT, 62, 1, 189, 378, 130, "Warning 247: Height not compliant with standards (too small)", "" },
        /*361*/ { BARCODE_DPD, COMPLIANT_HEIGHT, 62.5, "0123456789012345678901234567", "", 0, 62.5, 1, 189, 378, 131, "", "" },
        /*362*/ { BARCODE_MICROQR, -1, 1, "12345", "", 0, 11, 11, 11, 22, 22, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*363*/ { BARCODE_HIBC_128, -1, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "", "" },
        /*364*/ { BARCODE_HIBC_128, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "", "" },
        /*365*/ { BARCODE_HIBC_128, -1, 4, "1234567890", "", 0, 4, 1, 123, 246, 8, "", "" },
        /*366*/ { BARCODE_HIBC_39, -1, 1, "1234567890", "", 0, 1, 1, 223, 446, 2, "", "" },
        /*367*/ { BARCODE_HIBC_39, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 223, 446, 2, "Warning 247: Height not compliant with standards (too small)", "" },
        /*368*/ { BARCODE_HIBC_39, -1, 4, "1234567890", "", 0, 4, 1, 223, 446, 8, "", "" },
        /*369*/ { BARCODE_HIBC_DM, -1, 1, "ABC", "", 0, 12, 12, 12, 24, 24, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*370*/ { BARCODE_HIBC_QR, -1, 1, "1234567890AB", "", 0, 21, 21, 21, 42, 42, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*371*/ { BARCODE_HIBC_PDF, -1, -1, "ABCDEF", "", 0, 21, 7, 103, 206, 42, "", "7 rows" },
        /*372*/ { BARCODE_HIBC_PDF, -1, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "Warning 247: Height not compliant with standards (too small)", "" },
        /*373*/ { BARCODE_HIBC_PDF, -1, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "Warning 247: Height not compliant with standards (too small)", "" },
        /*374*/ { BARCODE_HIBC_PDF, -1, 36, "ABCDEF", "", 0, 35, 7, 103, 206, 70, "", "" },
        /*375*/ { BARCODE_HIBC_MICPDF, -1, -1, "ABCDEF", "", 0, 12, 6, 82, 164, 24, "", "6 rows" },
        /*376*/ { BARCODE_HIBC_MICPDF, -1, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "Warning 247: Height not compliant with standards (too small)", "" },
        /*377*/ { BARCODE_HIBC_MICPDF, -1, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "Warning 247: Height not compliant with standards (too small)", "" },
        /*378*/ { BARCODE_HIBC_MICPDF, -1, 47, "ABCDEF", "", 0, 48, 6, 82, 164, 96, "", "" },
        /*379*/ { BARCODE_HIBC_BLOCKF, -1, -1, "1234567890", "", 0, 30, 3, 101, 242, 64, "", "3 rows" },
        /*380*/ { BARCODE_HIBC_BLOCKF, -1, 1, "1234567890", "", 0, 1.5, 3, 101, 242, 7, "", "" },
        /*381*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 101, 242, 7, "Warning 247: Height not compliant with standards (too small)", "" },
        /*382*/ { BARCODE_HIBC_BLOCKF, -1, 4, "1234567890", "", 0, 4.5, 3, 101, 242, 13, "", "" },
        /*383*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4.5, 3, 101, 242, 13, "Warning 247: Height not compliant with standards (too small)", "" },
        /*384*/ { BARCODE_HIBC_BLOCKF, -1, 23, "1234567890", "", 0, 22.5, 3, 101, 242, 49, "", "" },
        /*385*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 23, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 22.5, 3, 101, 242, 49, "Warning 247: Height not compliant with standards (too small)", "" },
        /*386*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 24, "1234567890", "", 0, 24, 3, 101, 242, 52, "", "" },
        /*387*/ { BARCODE_HIBC_BLOCKF, -1, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "", "" },
        /*388*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "", "" },
        /*389*/ { BARCODE_HIBC_AZTEC, -1, 1, "1234567890AB", "", 0, 19, 19, 19, 38, 38, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*390*/ { BARCODE_DOTCODE, -1, 1, "ABC", "", 0, 11, 11, 16, 33, 23, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*391*/ { BARCODE_HANXIN, -1, 1, "1234567890AB", "", 0, 23, 23, 23, 46, 46, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*392*/ { BARCODE_MAILMARK_2D, -1, 1, "012100123412345678AB19XY1A 0", "", 0, 24, 24, 24, 48, 48, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*393*/ { BARCODE_UPU_S10, -1, 1, "EE876543216CA", "", 0, 1, 1, 156, 312, 2, "", "" },
        /*394*/ { BARCODE_UPU_S10, COMPLIANT_HEIGHT, 1, "EE876543216CA", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 156, 312, 2, "Warning 247: Height not compliant with standards (too small)", "" },
        /*395*/ { BARCODE_UPU_S10, -1, 24.5, "EE876543216CA", "", 0, 24.5, 1, 156, 312, 49, "", "" },
        /*396*/ { BARCODE_UPU_S10, COMPLIANT_HEIGHT, 24.5, "EE876543216CA", "", ZINT_WARN_NONCOMPLIANT, 24.5, 1, 156, 312, 49, "Warning 247: Height not compliant with standards (too small)", "" },
        /*397*/ { BARCODE_UPU_S10, -1, 25, "EE876543216CA", "", 0, 25, 1, 156, 312, 50, "", "" },
        /*398*/ { BARCODE_UPU_S10, COMPLIANT_HEIGHT, 25, "EE876543216CA", "", 0, 25, 1, 156, 312, 50, "", "" },
        /*399*/ { BARCODE_MAILMARK_4S, -1, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "", "" },
        /*400*/ { BARCODE_MAILMARK_4S, -1, 1, "01000000000000000AA00AA0A", "", 0, 2.5, 3, 155, 310, 5, "", "" },
        /*401*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 1, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 155, 310, 3, "Warning 499: Height not compliant with standards", "" },
        /*402*/ { BARCODE_MAILMARK_4S, -1, 6.4, "01000000000000000AA00AA0A", "", 0, 6.5, 3, 155, 310, 13, "", "" },
        /*403*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 6.4, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 6.5, 3, 155, 310, 13, "Warning 499: Height not compliant with standards", "" },
        /*404*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 6.5, "01000000000000000AA00AA0A", "", 0, 6.5, 3, 155, 310, 13, "", "" },
        /*405*/ { BARCODE_MAILMARK_4S, -1, 10, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "", "" },
        /*406*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 10, "01000000000000000AA00AA0A", "", 0, 9.5, 3, 155, 310, 19, "", "" },
        /*407*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 11, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 155, 310, 22, "Warning 499: Height not compliant with standards", "" },
        /*408*/ { BARCODE_MAILMARK_4S, -1, 15, "01000000000000000AA00AA0A", "", 0, 15, 3, 155, 310, 30, "", "" },
        /*409*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 15, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 15, 3, 155, 310, 30, "Warning 499: Height not compliant with standards", "" },
        /*410*/ { BARCODE_MAILMARK_4S, -1, 20, "01000000000000000AA00AA0A", "", 0, 20, 3, 155, 310, 40, "", "" },
        /*411*/ { BARCODE_MAILMARK_4S, COMPLIANT_HEIGHT, 20, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 20, 3, 155, 310, 40, "Warning 499: Height not compliant with standards", "" },
        /*412*/ { BARCODE_AZRUNE, -1, 1, "1", "", 0, 11, 11, 11, 22, 22, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*413*/ { BARCODE_CODE32, -1, 1, "12345678", "", 0, 1, 1, 103, 206, 2, "", "" },
        /*414*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 103, 206, 2, "Warning 247: Height not compliant with standards (too small)", "" },
        /*415*/ { BARCODE_CODE32, -1, 19, "12345678", "", 0, 19, 1, 103, 206, 38, "", "" },
        /*416*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 19, "12345678", "", ZINT_WARN_NONCOMPLIANT, 19, 1, 103, 206, 38, "Warning 247: Height not compliant with standards (too small)", "" },
        /*417*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 20, "12345678", "", 0, 20, 1, 103, 206, 40, "", "" },
        /*418*/ { BARCODE_EAN13_CC, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 110, "", "EAN-13, CC-A 3 rows" },
        /*419*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 110, "", "EAN-13, CC-A 3 rows" },
        /*420*/ { BARCODE_EAN13_CC, -1, 1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "", "" },
        /*421*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "", "" },
        /*422*/ { BARCODE_EAN13_CC, -1, 81, "123456789012", "[20]01", 0, 81, 7, 99, 226, 172, "", "" },
        /*423*/ { BARCODE_EANX_CC, -1, 81, "123456789012", "[20]01", 0, 81, 7, 99, 226, 172, "", "" },
        /*424*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 81, "123456789012", "[20]01", ZINT_WARN_NONCOMPLIANT, 81, 7, 99, 226, 172, "Warning 247: Height not compliant with standards (too small)", "" },
        /*425*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 81, "123456789012", "[20]01", ZINT_WARN_NONCOMPLIANT, 81, 7, 99, 226, 172, "Warning 247: Height not compliant with standards (too small)", "" },
        /*426*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 81.25, "123456789012", "[20]01", 0, 81.5, 7, 99, 226, 173, "", "" },
        /*427*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 81.25, "123456789012", "[20]01", 0, 81.5, 7, 99, 226, 173, "", "" },
        /*428*/ { BARCODE_EAN13_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 50, 9, 99, 226, 110, "", "EAN-13, CC-A 5 rows" },
        /*429*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 50, 9, 99, 226, 110, "", "EAN-13, CC-A 5 rows" },
        /*430*/ { BARCODE_EAN13_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 16.5, 9, 99, 226, 43, "", "" },
        /*431*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 16.5, 9, 99, 226, 43, "", "" },
        /*432*/ { BARCODE_EAN13_CC, -1, 85, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85, 9, 99, 226, 180, "", "" },
        /*433*/ { BARCODE_EANX_CC, -1, 85, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85, 9, 99, 226, 180, "", "" },
        /*434*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 85, "123456789012", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 85, 9, 99, 226, 180, "Warning 247: Height not compliant with standards (too small)", "" },
        /*435*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 85, "123456789012", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 85, 9, 99, 226, 180, "Warning 247: Height not compliant with standards (too small)", "" },
        /*436*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 85.25, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85.5, 9, 99, 226, 181, "", "" },
        /*437*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 85.25, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85.5, 9, 99, 226, 181, "", "" },
        /*438*/ { BARCODE_EAN13_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 11, 99, 226, 110, "", "EAN-13, CC-A 7 rows" },
        /*439*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 11, 99, 226, 110, "", "EAN-13, CC-A 7 rows" },
        /*440*/ { BARCODE_EAN13_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 11, 99, 226, 51, "", "" },
        /*441*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 11, 99, 226, 51, "", "" },
        /*442*/ { BARCODE_EAN13_CC, -1, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89, 11, 99, 226, 188, "", "" },
        /*443*/ { BARCODE_EANX_CC, -1, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89, 11, 99, 226, 188, "", "" },
        /*444*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 89, 11, 99, 226, 188, "Warning 247: Height not compliant with standards (too small)", "" },
        /*445*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 89, 11, 99, 226, 188, "Warning 247: Height not compliant with standards (too small)", "" },
        /*446*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 89.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89.5, 11, 99, 226, 189, "", "" },
        /*447*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 89.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89.5, 11, 99, 226, 189, "", "" },
        /*448*/ { BARCODE_EAN13_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 14, 99, 226, 110, "", "EAN-13, CC-B 10 rows" },
        /*449*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 14, 99, 226, 110, "", "EAN-13, CC-B 10 rows" },
        /*450*/ { BARCODE_EAN13_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 14, 99, 226, 63, "", "" },
        /*451*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 14, 99, 226, 63, "", "" },
        /*452*/ { BARCODE_EAN13_CC, -1, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95, 14, 99, 226, 200, "", "" },
        /*453*/ { BARCODE_EANX_CC, -1, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95, 14, 99, 226, 200, "", "" },
        /*454*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 95, 14, 99, 226, 200, "Warning 247: Height not compliant with standards (too small)", "" },
        /*455*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 95, 14, 99, 226, 200, "Warning 247: Height not compliant with standards (too small)", "" },
        /*456*/ { BARCODE_EAN13_CC, COMPLIANT_HEIGHT, 95.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95.5, 14, 99, 226, 201, "", "" },
        /*457*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 95.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95.5, 14, 99, 226, 201, "", "" },
        /*458*/ { BARCODE_EAN8_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234", 0, 50, 10, 72, 162, 110, "", "EAN-8, CC-A 4 rows" },
        /*459*/ { BARCODE_EANX_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234", 0, 50, 10, 72, 162, 110, "", "EAN-8, CC-A 4 rows" },
        /*460*/ { BARCODE_EAN8_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234", 0, 18.5, 10, 72, 162, 47, "", "" },
        /*461*/ { BARCODE_EANX_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234", 0, 18.5, 10, 72, 162, 47, "", "" },
        /*462*/ { BARCODE_EAN8_CC, -1, 73, "1234567", "[20]01[90]123456789012345678901234", 0, 73, 10, 72, 162, 156, "", "" },
        /*463*/ { BARCODE_EANX_CC, -1, 73, "1234567", "[20]01[90]123456789012345678901234", 0, 73, 10, 72, 162, 156, "", "" },
        /*464*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, 73, "1234567", "[20]01[90]123456789012345678901234", ZINT_WARN_NONCOMPLIANT, 73, 10, 72, 162, 156, "Warning 247: Height not compliant with standards (too small)", "" },
        /*465*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 73, "1234567", "[20]01[90]123456789012345678901234", ZINT_WARN_NONCOMPLIANT, 73, 10, 72, 162, 156, "Warning 247: Height not compliant with standards (too small)", "" },
        /*466*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, 73.25, "1234567", "[20]01[90]123456789012345678901234", 0, 73.5, 10, 72, 162, 157, "", "" },
        /*467*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 73.25, "1234567", "[20]01[90]123456789012345678901234", 0, 73.5, 10, 72, 162, 157, "", "" },
        /*468*/ { BARCODE_EAN8_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50, 24, 82, 178, 110, "", "EAN-8, CC-B 15 rows" },
        /*469*/ { BARCODE_EANX_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50, 24, 82, 178, 110, "", "EAN-8, CC-B 15 rows" },
        /*470*/ { BARCODE_EAN8_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 46.5, 24, 82, 178, 103, "", "" },
        /*471*/ { BARCODE_EANX_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 46.5, 24, 82, 178, 103, "", "" },
        /*472*/ { BARCODE_EAN8_CC, -1, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101, 24, 82, 178, 212, "", "" },
        /*473*/ { BARCODE_EANX_CC, -1, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101, 24, 82, 178, 212, "", "" },
        /*474*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 101, 24, 82, 178, 212, "Warning 247: Height not compliant with standards (too small)", "" },
        /*475*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 101, 24, 82, 178, 212, "Warning 247: Height not compliant with standards (too small)", "" },
        /*476*/ { BARCODE_EAN8_CC, COMPLIANT_HEIGHT, 101.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101.5, 24, 82, 178, 213, "", "" },
        /*477*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 101.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101.5, 24, 82, 178, 213, "", "" },
        /*478*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 50, 5, 145, 290, 100, "", "CC-A 3 rows" },
        /*479*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 145, 290, 15, "", "" },
        /*480*/ { BARCODE_GS1_128_CC, -1, 12.5, "[01]12345678901231", "[20]01", 0, 12.5, 5, 145, 290, 25, "", "" },
        /*481*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 12.5, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 12.5, 5, 145, 290, 25, "Warning 247: Height not compliant with standards (too small)", "" },
        /*482*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 12.75, "[01]12345678901231", "[20]01", 0, 13, 5, 145, 290, 26, "", "" },
        /*483*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 9, 145, 290, 100, "", "CC-A 7 rows" },
        /*484*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 15.5, 9, 145, 290, 31, "", "" },
        /*485*/ { BARCODE_GS1_128_CC, -1, 20.7, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 9, 145, 290, 41, "", "" },
        /*486*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 20.7, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 20.5, 9, 145, 290, 41, "Warning 247: Height not compliant with standards (too small)", "" },
        /*487*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 20.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 21, 9, 145, 290, 42, "", "" },
        /*488*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 12, 145, 290, 100, "", "CC-B 10 rows" },
        /*489*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 145, 290, 43, "", "" },
        /*490*/ { BARCODE_GS1_128_CC, -1, 26.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 12, 145, 290, 53, "", "" },
        /*491*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 26.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 26.5, 12, 145, 290, 53, "Warning 247: Height not compliant with standards (too small)", "" },
        /*492*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 26.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 27, 12, 145, 290, 54, "", "" },
        /*493*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "", "CC-C 30 rows" },
        /*494*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "", "" },
        /*495*/ { BARCODE_GS1_128_CC, -1, 96.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 96.5, 32, 154, 308, 193, "", "" },
        /*496*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 96.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 96.5, 32, 154, 308, 193, "Warning 247: Height not compliant with standards (too small)", "" },
        /*497*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 96.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 97, 32, 154, 308, 194, "", "" },
        /*498*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]1234567890", 0, 21, 5, 100, 200, 42, "", "CC-A 3 rows" },
        /*499*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]1234567890", 0, 7.5, 5, 100, 200, 15, "", "" },
        /*500*/ { BARCODE_DBAR_OMN_CC, -1, 19.9, "1234567890123", "[20]01[90]1234567890", 0, 20, 5, 100, 200, 40, "", "" },
        /*501*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 19.9, "1234567890123", "[20]01[90]1234567890", ZINT_WARN_NONCOMPLIANT, 20, 5, 100, 200, 40, "Warning 247: Height not compliant with standards (too small)", "" },
        /*502*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 20, "1234567890123", "[20]01[90]1234567890", 0, 20, 5, 100, 200, 40, "", "" },
        /*503*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]12345678901234567890", 0, 23, 6, 100, 200, 46, "", "CC-A 4 rows" },
        /*504*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]12345678901234567890", 0, 9.5, 6, 100, 200, 19, "", "" },
        /*505*/ { BARCODE_DBAR_OMN_CC, -1, 21.9, "1234567890123", "[20]01[90]12345678901234567890", 0, 22, 6, 100, 200, 44, "", "" },
        /*506*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 21.9, "1234567890123", "[20]01[90]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 22, 6, 100, 200, 44, "Warning 247: Height not compliant with standards (too small)", "" },
        /*507*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 22, "1234567890123", "[20]01[90]12345678901234567890", 0, 22, 6, 100, 200, 44, "", "" },
        /*508*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35, 12, 100, 200, 70, "", "CC-B 10 rows" },
        /*509*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 100, 200, 43, "", "" },
        /*510*/ { BARCODE_DBAR_OMN_CC, -1, 33.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 34, 12, 100, 200, 68, "", "" },
        /*511*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 33.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 34, 12, 100, 200, 68, "Warning 247: Height not compliant with standards (too small)", "" },
        /*512*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 34, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 34, 12, 100, 200, 68, "", "" },
        /*513*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 39, 14, 100, 200, 78, "", "CC-B 12 rows" },
        /*514*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 25.5, 14, 100, 200, 51, "", "" },
        /*515*/ { BARCODE_DBAR_OMN_CC, -1, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 38, 14, 100, 200, 76, "", "" },
        /*516*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 38, 14, 100, 200, 76, "Warning 247: Height not compliant with standards (too small)", "" },
        /*517*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 38, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 38, 14, 100, 200, 76, "", "" },
        /*518*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "", "CC-A 4 rows" },
        /*519*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01", 0, 9.5, 6, 79, 158, 19, "", "" },
        /*520*/ { BARCODE_DBAR_LTD_CC, -1, 18, "1234567890123", "[20]01", 0, 18, 6, 79, 158, 36, "", "" },
        /*521*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 18, "1234567890123", "[20]01", ZINT_WARN_NONCOMPLIANT, 18, 6, 79, 158, 36, "Warning 247: Height not compliant with standards (too small)", "" },
        /*522*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 19, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "", "" },
        /*523*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "", "CC-A 7 rows" },
        /*524*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 15.5, 9, 79, 158, 31, "", "" },
        /*525*/ { BARCODE_DBAR_LTD_CC, -1, 24.9, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "", "" },
        /*526*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 24.9, "1234567890123", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 25, 9, 79, 158, 50, "Warning 247: Height not compliant with standards (too small)", "" },
        /*527*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 25, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "", "" },
        /*528*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "", "CC-B 20 rows" },
        /*529*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 41.5, 22, 88, 176, 83, "", "" },
        /*530*/ { BARCODE_DBAR_LTD_CC, -1, 50.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "", "" },
        /*531*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 50.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 51, 22, 88, 176, 102, "Warning 247: Height not compliant with standards (too small)", "" },
        /*532*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 51, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "", "" },
        /*533*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "", "CC-A 3 rows" },
        /*534*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 134, 268, 15, "", "" },
        /*535*/ { BARCODE_DBAR_EXP_CC, -1, 40, "[01]12345678901231", "[20]01", 0, 40, 5, 134, 268, 80, "", "" },
        /*536*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 40, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 40, 5, 134, 268, 80, "Warning 247: Height not compliant with standards (too small)", "" },
        /*537*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 41, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "", "" },
        /*538*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "", "CC-A 5 rows" },
        /*539*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 11.5, 7, 134, 268, 23, "", "" },
        /*540*/ { BARCODE_DBAR_EXP_CC, -1, 44, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 44, 7, 134, 268, 88, "", "" },
        /*541*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 44, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 44, 7, 134, 268, 88, "Warning 247: Height not compliant with standards (too small)", "" },
        /*542*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 45, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "", "" },
        /*543*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "", "CC-B 10 rows" },
        /*544*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 134, 268, 43, "", "" },
        /*545*/ { BARCODE_DBAR_EXP_CC, -1, 54, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 54, 12, 134, 268, 108, "", "" },
        /*546*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 54, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 54, 12, 134, 268, 108, "Warning 247: Height not compliant with standards (too small)", "" },
        /*547*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 55, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "", "" },
        /*548*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01", 0, 50, 7, 99, 226, 110, "", "CC-A 3 rows" },
        /*549*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01", 0, 12.5, 7, 99, 226, 35, "", "" },
        /*550*/ { BARCODE_UPCA_CC, -1, 81.24, "12345678901", "[20]01", 0, 81, 7, 99, 226, 172, "", "" },
        /*551*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 81.24, "12345678901", "[20]01", ZINT_WARN_NONCOMPLIANT, 81, 7, 99, 226, 172, "Warning 247: Height not compliant with standards (too small)", "" },
        /*552*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 81.25, "12345678901", "[20]01", 0, 81.5, 7, 99, 226, 173, "", "" },
        /*553*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 10, 99, 226, 110, "", "CC-A 6 rows" },
        /*554*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 18.5, 10, 99, 226, 47, "", "" },
        /*555*/ { BARCODE_UPCA_CC, -1, 87.24, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 87, 10, 99, 226, 184, "", "" },
        /*556*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 87.24, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 87, 10, 99, 226, 184, "Warning 247: Height not compliant with standards (too small)", "" },
        /*557*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 87.25, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 87.5, 10, 99, 226, 185, "", "" },
        /*558*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 50, 16, 99, 226, 110, "", "CC-B 12 rows" },
        /*559*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 30.5, 16, 99, 226, 71, "", "" },
        /*560*/ { BARCODE_UPCA_CC, -1, 99, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 99, 16, 99, 226, 208, "", "" },
        /*561*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 99, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", ZINT_WARN_NONCOMPLIANT, 99, 16, 99, 226, 208, "Warning 247: Height not compliant with standards (too small)", "" },
        /*562*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 99.25, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 99.5, 16, 99, 226, 209, "", "" },
        /*563*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678", 0, 50, 11, 55, 134, 110, "", "CC-A 7 rows" },
        /*564*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678", 0, 20.5, 11, 55, 134, 51, "", "" },
        /*565*/ { BARCODE_UPCE_CC, -1, 89, "1234567", "[20]01[90]123456789012345678", 0, 89, 11, 55, 134, 188, "", "" },
        /*566*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 89, "1234567", "[20]01[90]123456789012345678", ZINT_WARN_NONCOMPLIANT, 89, 11, 55, 134, 188, "Warning 247: Height not compliant with standards (too small)", "" },
        /*567*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 89.25, "1234567", "[20]01[90]123456789012345678", 0, 89.5, 11, 55, 134, 189, "", "" },
        /*568*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 16, 55, 134, 110, "", "CC-A 12 rows" },
        /*569*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 30.5, 16, 55, 134, 71, "", "" },
        /*570*/ { BARCODE_UPCE_CC, -1, 99, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 99, 16, 55, 134, 208, "", "" },
        /*571*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 99, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 99, 16, 55, 134, 208, "Warning 247: Height not compliant with standards (too small)", "" },
        /*572*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 99.25, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 99.5, 16, 55, 134, 209, "", "" },
        /*573*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 21, 55, 134, 110, "", "CC-B 17 rows" },
        /*574*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 40.5, 21, 55, 134, 91, "", "" },
        /*575*/ { BARCODE_UPCE_CC, -1, 109, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 109, 21, 55, 134, 228, "", "" },
        /*576*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 109, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 109, 21, 55, 134, 228, "Warning 247: Height not compliant with standards (too small)", "" },
        /*577*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 109.25, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 109.5, 21, 55, 134, 229, "", "" },
        /*578*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 134, 115, "", "CC-B 23 rows" },
        /*579*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 134, 115, "", "" },
        /*580*/ { BARCODE_UPCE_CC, -1, 121, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 121, 27, 55, 134, 252, "", "" },
        /*581*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 121, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", ZINT_WARN_NONCOMPLIANT, 121, 27, 55, 134, 252, "Warning 247: Height not compliant with standards (too small)", "" },
        /*582*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 121.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 121.5, 27, 55, 134, 253, "", "" },
        /*583*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "", "CC-A 5 rows" },
        /*584*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01", 0, 13, 9, 56, 112, 26, "", "" },
        /*585*/ { BARCODE_DBAR_STK_CC, -1, 23.9, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "", "" },
        /*586*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 23.9, "1234567890123", "[20]01", ZINT_WARN_NONCOMPLIANT, 24, 9, 56, 112, 48, "Warning 379: Height not compliant with standards", "" },
        /*587*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 24, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "", "" },
        /*588*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "", "CC-A 12 rows" },
        /*589*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 27, 16, 56, 112, 54, "", "" },
        /*590*/ { BARCODE_DBAR_STK_CC, -1, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "", "" },
        /*591*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 38, 16, 56, 112, 76, "Warning 379: Height not compliant with standards", "" },
        /*592*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 38, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "", "" },
        /*593*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "", "CC-B 17 rows" },
        /*594*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 37, 21, 56, 112, 74, "", "" },
        /*595*/ { BARCODE_DBAR_STK_CC, -1, 47.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "", "" },
        /*596*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 47.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 48, 21, 56, 112, 96, "Warning 379: Height not compliant with standards", "" },
        /*597*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 48, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "", "" },
        /*598*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "", "CC-A 6 rows" },
        /*599*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]1234567890123456", 0, 17, 12, 56, 112, 34, "", "" },
        /*600*/ { BARCODE_DBAR_OMNSTK_CC, -1, 81, "1234567890123", "[20]01[90]1234567890123456", 0, 81, 12, 56, 112, 162, "", "" },
        /*601*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 81, "1234567890123", "[20]01[90]1234567890123456", ZINT_WARN_NONCOMPLIANT, 81, 12, 56, 112, 162, "Warning 247: Height not compliant with standards (too small)", "" },
        /*602*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 82, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "", "" },
        /*603*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "", "CC-A 12 rows" },
        /*604*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 29, 18, 56, 112, 58, "", "" },
        /*605*/ { BARCODE_DBAR_OMNSTK_CC, -1, 93.5, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "", "" },
        /*606*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 93.5, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", ZINT_WARN_NONCOMPLIANT, 94, 18, 56, 112, 188, "Warning 247: Height not compliant with standards (too small)", "" },
        /*607*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 94, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "", "" },
        /*608*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "", "CC-B 17 rows" },
        /*609*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 39, 23, 56, 112, 78, "", "" },
        /*610*/ { BARCODE_DBAR_OMNSTK_CC, -1, 103, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 103, 23, 56, 112, 206, "", "" },
        /*611*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 103, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 103, 23, 56, 112, 206, "Warning 247: Height not compliant with standards (too small)", "" },
        /*612*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 104, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "", "" },
        /*613*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "", "3 rows, CC-A 3 rows" },
        /*614*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 11, 9, 102, 204, 22, "", "" },
        /*615*/ { BARCODE_DBAR_EXPSTK_CC, -1, 77, "[01]12345678901231", "[20]01", 0, 77, 9, 102, 204, 154, "", "" },
        /*616*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 77, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 77, 9, 102, 204, 154, "Warning 247: Height not compliant with standards (too small)", "" },
        /*617*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 78, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "", "" },
        /*618*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "", "5 rows, CC-A 3 rows" },
        /*619*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 21.5, 21, 102, 204, 43, "", "" },
        /*620*/ { BARCODE_DBAR_EXPSTK_CC, -1, 188, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "", "" },
        /*621*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 188, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", ZINT_WARN_NONCOMPLIANT, 189, 21, 102, 204, 378, "Warning 247: Height not compliant with standards (too small)", "" },
        /*622*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 189, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "", "" },
        /*623*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "", "3 rows, CC-A 4 rows" },
        /*624*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 13, 10, 102, 204, 26, "", "" },
        /*625*/ { BARCODE_DBAR_EXPSTK_CC, -1, 79, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 79, 10, 102, 204, 158, "", "" },
        /*626*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 79, "[01]12345678901231", "[20]01[90]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 79, 10, 102, 204, 158, "Warning 247: Height not compliant with standards (too small)", "" },
        /*627*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 80, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "", "" },
        /*628*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "", "3 rows, CC-B 10 rows" },
        /*629*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 25, 16, 102, 204, 50, "", "" },
        /*630*/ { BARCODE_DBAR_EXPSTK_CC, -1, 91, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 91, 16, 102, 204, 182, "", "" },
        /*631*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 91, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 91, 16, 102, 204, 182, "Warning 247: Height not compliant with standards (too small)", "" },
        /*632*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 92, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "", "" },
        /*633*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "", "5 rows, CC-B 10 rows" },
        /*634*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35.5, 28, 102, 204, 71, "", "" },
        /*635*/ { BARCODE_DBAR_EXPSTK_CC, -1, 202, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "", "" },
        /*636*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 202, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 203, 28, 102, 204, 406, "Warning 247: Height not compliant with standards (too small)", "" },
        /*637*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 203, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "", "" },
        /*638*/ { BARCODE_CHANNEL, -1, 1, "1", "", 0, 1, 1, 19, 38, 2, "", "" },
        /*639*/ { BARCODE_CHANNEL, -1, 3.75, "123", "", 0, 4, 1, 23, 46, 8, "", "" },
        /*640*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, 3.75, "123", "", ZINT_WARN_NONCOMPLIANT, 4, 1, 23, 46, 8, "Warning 247: Height not compliant with standards (too small)", "Min height data-length dependent" },
        /*641*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, 4, "123", "", 0, 4, 1, 23, 46, 8, "", "" },
        /*642*/ { BARCODE_CODEONE, -1, 1, "12345678901234567890", "", 0, 16, 16, 18, 36, 32, "", "Fixed height, symbol->height ignored" },
        /*643*/ { BARCODE_GRIDMATRIX, -1, 1, "ABC", "", 0, 18, 18, 18, 36, 36, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*644*/ { BARCODE_UPNQR, -1, 1, "1234567890AB", "", 0, 77, 77, 77, 154, 154, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*645*/ { BARCODE_ULTRA, -1, 1, "1234567890", "", 0, 13, 13, 18, 36, 26, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*646*/ { BARCODE_RMQR, -1, 1, "12345", "", 0, 11, 11, 27, 54, 22, "", "Fixed width-to-height ratio, symbol->height ignored" },
        /*647*/ { BARCODE_BC412, -1, 1, "1234567", "", 0, 1, 1, 102, 204, 2, "", "" },
        /*648*/ { BARCODE_BC412, -1, 13.6, "1234567", "", 0, 13.5, 1, 102, 204, 27, "", "" },
        /*649*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 13.6, "1234567", "", ZINT_WARN_NONCOMPLIANT, 13.5, 1, 102, 204, 27, "Warning 247: Height not compliant with standards (too small)", "" },
        /*650*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 13.65, "1234567", "", 0, 13.5, 1, 102, 204, 27, "", "" },
        /*651*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 21.3, "1234567", "", 0, 21.5, 1, 102, 204, 43, "", "" },
        /*652*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 21.35, "1234567", "", ZINT_WARN_NONCOMPLIANT, 21.5, 1, 102, 204, 43, "Warning 248: Height not compliant with standards (maximum 21.32)", "" },
        /*653*/ { BARCODE_DXFILMEDGE, -1, 1, "120476", "", 0, 1, 2, 23, 46, 2, "", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].output_options != -1) {
            symbol->output_options = data[i].output_options;
        }
        if (data[i].height != -1) {
            symbol->height = data[i].height;
        }
        symbol->input_mode = UNICODE_MODE;
        symbol->show_hrt = 0; /* Note: disabling HRT */
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %.5g, \"%s\", \"%s\", %s, %.8g, %d, %d, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilOutputOptionsName(data[i].output_options),
                    data[i].height, data[i].data, data[i].composite, testUtilErrorName(data[i].ret),
                    symbol->height, symbol->rows, symbol->width, symbol->bitmap_width, symbol->bitmap_height, symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n",
                        i, symbol->errtxt, data[i].expected_errtxt);
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_height_per_row(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        float height;
        float scale;
        const char *data;
        const char *composite;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;

        const char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, "1234567890", "", 0, 21, 7, 103, 206, 42, "" },
        /*  1*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*  2*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 7, 7, 103, 206, 14, "" },
        /*  3*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.25, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 10.5, 7, 103, 206, 21, "" },
        /*  4*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 10.5, 7, 103, 206, 21, "" },
        /*  5*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.7, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 10.5, 7, 103, 206, 21, "" },
        /*  6*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.74, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 10.5, 7, 103, 206, 21, "" },
        /*  7*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.75, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 14, 7, 103, 206, 28, "" },
        /*  8*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 14, 7, 103, 206, 28, "" },
        /*  9*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.1, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 14, 7, 103, 206, 28, "" },
        /* 10*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.25, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17.5, 7, 103, 206, 35, "" },
        /* 11*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17.5, 7, 103, 206, 35, "" },
        /* 12*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.75, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 21, 7, 103, 206, 42, "" },
        /* 13*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890", "", 0, 21, 7, 103, 206, 42, "Default" },
        /* 14*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 24.5, 7, 103, 206, 49, "" },
        /* 15*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1000, -1, "1234567890", "", 0, 7000, 7, 103, 206, 14000, "" },
        /* 16*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, 5, 0.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 2.5, 5, 120, 240, 5, "5 rows" },
        /* 17*/ { BARCODE_PDF417COMP, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 24.5, 7, 69, 138, 49, "" },
        /* 18*/ { BARCODE_HIBC_PDF, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 28, 8, 103, 206, 56, "" },
        /* 19*/ { BARCODE_CODE16K, -1, -1, -1, -1, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "" },
        /* 20*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890", "", 0, 1, 2, 70, 162, 6, "(0.5 * 2 rows + 2 binds) * 2 scale = 6 (separator will cover rows)" },
        /* 21*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.7, -1, "1234567890", "", 0, 1, 2, 70, 162, 6, "" },
        /* 22*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.75, -1, "1234567890", "", 0, 2, 2, 70, 162, 8, "" },
        /* 23*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890", "", 0, 2, 2, 70, 162, 8, "" },
        /* 24*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.1, -1, "1234567890", "", 0, 2, 2, 70, 162, 8, "" },
        /* 25*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.2, -1, "1234567890", "", 0, 2, 2, 70, 162, 8, "" },
        /* 26*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.25, -1, "1234567890", "", 0, 3, 2, 70, 162, 10, "" },
        /* 27*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.5, -1, "1234567890", "", 0, 3, 2, 70, 162, 10, "" },
        /* 28*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "1234567890", "", 0, 4, 2, 70, 162, 12, "" },
        /* 29*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 10, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "Default" },
        /* 30*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 10.5, -1, "1234567890", "", 0, 21, 2, 70, 162, 46, "" },
        /* 31*/ { BARCODE_CODE49, -1, -1, -1, -1, -1, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "" },
        /* 32*/ { BARCODE_CODE49, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "12345678901234567890", "", 0, 6, 3, 70, 162, 16, "(2 * 3 rows + 2 binds) * 2 scale = 16" },
        /* 33*/ { BARCODE_CODE49, HEIGHTPERROW_MODE, -1, -1, -1, 10, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "Default" },
        /* 34*/ { BARCODE_CODE49, HEIGHTPERROW_MODE, -1, -1, -1, 10.5, -1, "12345678901234567890", "", 0, 31.5, 3, 70, 162, 67, "" },
        /* 35*/ { BARCODE_CODABLOCKF, -1, -1, -1, -1, -1, -1, "1234567890123456789", "", 0, 40, 4, 101, 242, 84, "" },
        /* 36*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890123456789", "", 0, 2, 4, 101, 242, 8, "(0.5 * 4 rows + 2 binds) * 2 scale = 8" },
        /* 37*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890123456789", "", 0, 4, 4, 101, 242, 12, "" },
        /* 38*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890123456789", "", 0, 12, 4, 101, 242, 28, "" },
        /* 39*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 10, -1, "1234567890123456789", "", 0, 40, 4, 101, 242, 84, "Default when <= 12 cols" },
        /* 40*/ { BARCODE_CODABLOCKF, -1, 2, -1, -1, -1, -1, "12345678901234567890123456", "", 0, 20, 2, 200, 440, 44, "2 rows" },
        /* 41*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, 2, -1, -1, 10.15, -1, "12345678901234567890123456", "", 0, 20, 2, 200, 440, 44, "Default for 13 cols" },
        /* 42*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, 2, -1, -1, 2000, -1, "12345678901234567890123456", "", 0, 4000, 2, 200, 440, 8004, "" },
        /* 43*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 10.5, -1, "1234567890123456789", "", 0, 42, 4, 101, 242, 88, "" },
        /* 44*/ { BARCODE_HIBC_BLOCKF, HEIGHTPERROW_MODE, -1, -1, -1, 10.5, -1, "1234567890123456789", "", 0, 42, 4, 101, 242, 88, "" },
        /* 45*/ { BARCODE_MICROPDF417, -1, -1, -1, -1, -1, -1, "1234567890", "", 0, 12, 6, 82, 164, 24, "" },
        /* 46*/ { BARCODE_MICROPDF417, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "" },
        /* 47*/ { BARCODE_MICROPDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "1234567890", "", 0, 12, 6, 82, 164, 24, "Default" },
        /* 48*/ { BARCODE_MICROPDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.5, -1, "1234567890", "", 0, 15, 6, 82, 164, 30, "" },
        /* 49*/ { BARCODE_MICROPDF417, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890", "", 0, 18, 6, 82, 164, 36, "" },
        /* 50*/ { BARCODE_HIBC_MICPDF, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890", "", 0, 42, 14, 38, 76, 84, "" },
        /* 51*/ { BARCODE_DBAR_EXPSTK, -1, -1, -1, -1, -1, -1, "[8110]106141416543213500110000310123196000", "", 0, 145, 13, 102, 204, 290, "" },
        /* 52*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "[8110]106141416543213500110000310123196000", "", 0, 11, 13, 102, 204, 22, "(0.5 * 4 rows + 9 seps) * 2 scale = 22" },
        /* 53*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 9, -1, "[8110]106141416543213500110000310123196000", "", 0, 45, 13, 102, 204, 90, "" },
        /* 54*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 9.5, -1, "[8110]106141416543213500110000310123196000", "", 0, 47, 13, 102, 204, 94, "" },
        /* 55*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 10, -1, "[8110]106141416543213500110000310123196000", "", 0, 49, 13, 102, 204, 98, "" },
        /* 56*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 34, -1, "[8110]106141416543213500110000310123196000", "", 0, 145, 13, 102, 204, 290, "Default" },
        /* 57*/ { BARCODE_DBAR_EXPSTK, HEIGHTPERROW_MODE, -1, -1, -1, 50, -1, "[8110]106141416543213500110000310123196000", "", 0, 209, 13, 102, 204, 418, "" },
        /* 58*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, -1, -1, -1, -1, "[8110]106141416543213500110000310123196000", "[8112]017777777666666223456789", 0, 154, 18, 102, 204, 308, "" },
        /* 59*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, -1, -1, 0.5, -1, "[8110]106141416543213500110000310123196000", "[8112]017777777666666223456789", 0, 20, 18, 102, 204, 40, "(0.5 * 4 rows + 10 seps + 2 * 4 cc) * 2 scale = 40" },
        /* 60*/ { BARCODE_DBAR_EXPSTK_CC, HEIGHTPERROW_MODE, -1, -1, -1, 34, -1, "[8110]106141416543213500110000310123196000", "[8112]017777777666666223456789", 0, 154, 18, 102, 204, 308, "Default" },
        /* 61*/ { BARCODE_DBAR_EXPSTK_CC, HEIGHTPERROW_MODE, -1, -1, -1, 35, -1, "[8110]106141416543213500110000310123196000", "[8112]017777777666666223456789", 0, 158, 18, 102, 204, 316, "" },
        /* 62*/ { BARCODE_PHARMA_TWO, -1, -1, -1, -1, -1, -1, "1234", "", 0, 10, 2, 13, 26, 20, "" },
        /* 63*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234", "", 0, 1, 2, 13, 26, 2, "" },
        /* 64*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.1, -1, "1234", "", 0, 4, 2, 13, 26, 8, "" },
        /* 65*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.2, -1, "1234", "", 0, 4, 2, 13, 26, 8, "" },
        /* 66*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.25, -1, "1234", "", 0, 5, 2, 13, 26, 10, "" },
        /* 67*/ { BARCODE_DBAR_OMNSTK, -1, -1, -1, -1, -1, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /* 68*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890123", "", 0, 4, 5, 50, 100, 8, "(0.5 * 2 rows + 3 separators) * 2 scale = 8" },
        /* 69*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890123", "", 0, 5, 5, 50, 100, 10, "" },
        /* 70*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 3.2, -1, "1234567890123", "", 0, 9, 5, 50, 100, 18, "" },
        /* 71*/ { BARCODE_EAN13_CC, -1, -1, -1, -1, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 110, "" },
        /* 72*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 226, 110, "" },
        /* 73*/ { BARCODE_EAN13_CC, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "(0.5 * 1 row + 2 * 3 seps + 2 * 3 cc rows + 5 guards) * 2 scale = 35" },
        /* 74*/ { BARCODE_EANX_CC, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "(0.5 * 1 row + 2 * 3 seps + 2 * 3 cc rows + 5 guards) * 2 scale = 35" },
        /* 75*/ { BARCODE_EAN13_CC, -1, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "0.5 height below fixed height" },
        /* 76*/ { BARCODE_EANX_CC, -1, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "0.5 height below fixed height" },
        /* 77*/ { BARCODE_EAN13_CC, HEIGHTPERROW_MODE, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 16, 7, 99, 226, 42, "" },
        /* 78*/ { BARCODE_EANX_CC, HEIGHTPERROW_MODE, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 16, 7, 99, 226, 42, "" },
        /* 79*/ { BARCODE_EAN13_CC, -1, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "4 height below fixed height" },
        /* 80*/ { BARCODE_EANX_CC, -1, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 226, 35, "4 height below fixed height" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    const char *text;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (data[i].height != -1) {
            symbol->height = data[i].height;
        }
        if (data[i].scale != -1) {
            symbol->scale = data[i].scale;
        }
        symbol->show_hrt = 0; /* Note: disabling HRT */

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, text, -1, debug);

        ret = ZBarcode_Encode(symbol, TCU(text), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (p_ctx->index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, %d, %.5g, %.5g, \"%s\", \"%s\", %s, %.8g, %d, %d, %d, %d, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                    data[i].option_1, data[i].option_2, data[i].option_3, data[i].height, data[i].scale,
                    data[i].data, data[i].composite, testUtilErrorName(data[i].ret),
                    symbol->height, symbol->rows, symbol->width, symbol->bitmap_width, symbol->bitmap_height, data[i].comment);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_hrt_content_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int show_hrt;
        int option_2;
        int output_options;
        const char *data;
        int length;

        int ret;
        int expected_bitmap_height;
        int expected_bitmap_width;
        const char *expected;
        int expected_length;
        const char *expected_content;
        int expected_content_length;
        const char *expected_errtxt;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, BARCODE_MEMORY_FILE, "12345\00067890", 11, 0, 116, 268, "12345 67890", -1, "", -1, "" },
        /*  1*/ { BARCODE_CODE128, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345\00067890", 11, 0, 116, 268, "12345 67890", -1, "12345\00067890", 11, "" },
        /*  2*/ { BARCODE_EXCODE39, -1, -1, BARCODE_MEMORY_FILE, "12345\00067890", 11, 0, 116, 362, "12345 67890", -1, "", -1, "" },
        /*  3*/ { BARCODE_EXCODE39, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345\00067890", 11, 0, 116, 362, "12345 67890", -1, "12345\00067890", 11, "" },
        /*  4*/ { BARCODE_TELEPEN, -1, -1, BARCODE_MEMORY_FILE, "12345\00067890", 11, 0, 116, 448, "12345 67890", -1, "", -1, "" },
        /*  5*/ { BARCODE_TELEPEN, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345\00067890", 11, 0, 116, 448, "12345 67890", -1, "12345\00067890n", 12, "" },
        /*  6*/ { BARCODE_EAN13, -1, -1, BARCODE_MEMORY_FILE, "123456789012", -1, 0, 116, 226, "1234567890128", -1, "", -1, "" },
        /*  7*/ { BARCODE_EANX, -1, -1, BARCODE_MEMORY_FILE, "123456789012", -1, 0, 116, 226, "1234567890128", -1, "", -1, "" },
        /*  8*/ { BARCODE_EAN13, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "123456789012", -1, 0, 116, 226, "1234567890128", -1, "1234567890128", -1, "" },
        /*  9*/ { BARCODE_EANX, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "123456789012", -1, 0, 116, 226, "1234567890128", -1, "1234567890128", -1, "" },
        /* 10*/ { BARCODE_EAN13, -1, -1, BARCODE_MEMORY_FILE, "123456789012+12", -1, 0, 116, 276, "1234567890128+12", -1, "", -1, "" },
        /* 11*/ { BARCODE_EANX, -1, -1, BARCODE_MEMORY_FILE, "123456789012+12", -1, 0, 116, 276, "1234567890128+12", -1, "", -1, "" },
        /* 12*/ { BARCODE_EAN13, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "123456789012+12", -1, 0, 116, 276, "1234567890128+12", -1, "123456789012812", -1, "" },
        /* 13*/ { BARCODE_EANX, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "123456789012+12", -1, 0, 116, 276, "1234567890128+12", -1, "123456789012812", -1, "" },
        /* 14*/ { BARCODE_CODE39, -1, -1, BARCODE_MEMORY_FILE, "ABC14", -1, 0, 116, 180, "*ABC14*", -1, "", -1, "" },
        /* 15*/ { BARCODE_CODE39, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "ABC14", -1, 0, 116, 180, "*ABC14*", -1, "ABC14", -1, "" },
        /* 16*/ { BARCODE_CODE39, -1, 1, BARCODE_MEMORY_FILE, "ABC14", -1, 0, 116, 206, "*ABC14_*", -1, "", -1, "" }, /* Check digit space rendered as underscore */
        /* 17*/ { BARCODE_CODE39, -1, 1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "ABC14", -1, 0, 116, 206, "*ABC14_*", -1, "ABC14 ", -1, "" },
        /* 18*/ { BARCODE_POSTNET, -1, -1, BARCODE_MEMORY_FILE, "12345", -1, 0, 24, 126, "", -1, "", -1, "" },
        /* 19*/ { BARCODE_POSTNET, -1, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345", -1, 0, 24, 126, "", -1, "123455", -1, "" }, /* HRT not printed */
        /* 20*/ { BARCODE_POSTNET, 0, -1, BARCODE_MEMORY_FILE, "12345", -1, 0, 24, 126, "", -1, "", -1, "" },
        /* 21*/ { BARCODE_POSTNET, 0, -1, BARCODE_MEMORY_FILE | BARCODE_CONTENT_SEGS, "12345", -1, 0, 24, 126, "", -1, "123455", -1, "" }, /* HRT not printed */
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

        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/,
                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, data[i].output_options,
                    data[i].data, data[i].length, debug);
        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;
        expected_content_length = data[i].expected_content_length == -1 ? (int) strlen(data[i].expected_content) : data[i].expected_content_length;

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%s) ret %d != 0 (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        assert_equal(symbol->text_length, expected_length, "i:%d text_length %d != expected_length %d (%s)\n",
                    i, symbol->text_length, expected_length, symbol->text);
        assert_zero(memcmp(symbol->text, data[i].expected, expected_length), "i:%d memcmp(%s, %s, %d) != 0\n",
                    i, symbol->text, data[i].expected, expected_length);
        if (symbol->output_options & BARCODE_CONTENT_SEGS) {
            assert_nonnull(symbol->content_segs, "i:%d content_segs NULL\n", i);
            assert_nonnull(symbol->content_segs[0].source, "i:%d content_segs[0].source NULL\n", i);
            assert_equal(symbol->content_segs[0].length, expected_content_length,
                        "i:%d content_segs[0].length %d != expected_content_length %d\n",
                        i, symbol->content_segs[0].length, expected_content_length);
            assert_zero(memcmp(symbol->content_segs[0].source, data[i].expected_content, expected_content_length),
                        "i:%d memcmp(%.*s, %.*s, %d) != 0\n",
                        i, symbol->content_segs[0].length, symbol->content_segs[0].source, expected_content_length,
                        data[i].expected_content, expected_content_length);
        } else {
            assert_null(symbol->content_segs, "i:%d content_segs not NULL\n", i);
        }

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Print(%s) ret %d != %d (%s)\n",
                    i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n",
                    i, symbol->errtxt, data[i].expected_errtxt);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
                    i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITER_MILLES   1
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)
#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)

/* Not a real test, just performance indicator for scaling */
static void test_perf_scale(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int border_width;
        int output_options;
        int option_1;
        int option_2;
        float height;
        float scale;
        const char *data;
        int ret;

        int expected_rows;
        int expected_width;
        const char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, 0, 1.3,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    0, 40, 307, "PDF417 960 chars, text/numeric, 1.3" },
        /*  1*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, -1, -1, 0, 1.1, "12345", 0, 2, 63, "POSTNET 5 chars, quiet zones, 1.1" },
        /*  2*/ { BARCODE_ITF14, -1, 4, BARCODE_BIND, -1, -1, 61.8, 3.1, "12345", 0, 1, 135, "ITF14 bind 4, height 61.8, 3.1" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start;
    clock_t total_create = 0, total_encode = 0, total_buffer = 0, total_buf_inter = 0, total_print = 0;
    clock_t diff_create, diff_encode, diff_buffer, diff_buf_inter, diff_print;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = diff_buf_inter = diff_print = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
            if (data[i].height) {
                symbol->height = data[i].height;
            }
            if (data[i].scale) {
                symbol->scale = data[i].scale;
            }

            start = clock();
            ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

            symbol->output_options |= OUT_BUFFER_INTERMEDIATE;
            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buf_inter += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer OUT_BUFFER_INTERMEDIATE ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; /* Undo */

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

            ZBarcode_Delete(symbol);
        }

        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_buf_inter), TEST_PERF_TIME(diff_print), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
        total_buf_inter += diff_buf_inter;
        total_print += diff_print;
    }
    if (p_ctx->index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter), TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_options", test_options },
        { "test_buffer", test_buffer },
        { "test_upcean_hrt", test_upcean_hrt },
        { "test_row_separator", test_row_separator },
        { "test_stacking", test_stacking },
        { "test_stacking_too_many", test_stacking_too_many },
        { "test_output_options", test_output_options },
        { "test_dcontent_string_wrap", test_dcontent_string_wrap },
        { "test_code128_utf8", test_code128_utf8 },
        { "test_scale", test_scale },
        { "test_guard_descent", test_guard_descent },
        { "test_quiet_zones", test_quiet_zones },
        { "test_text_gap", test_text_gap },
        { "test_buffer_plot", test_buffer_plot },
        { "test_height", test_height },
        { "test_height_per_row", test_height_per_row },
        { "test_hrt_content_segs", test_hrt_content_segs },
        { "test_perf_scale", test_perf_scale },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
