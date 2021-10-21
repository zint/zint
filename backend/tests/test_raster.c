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

static int is_row_column_black(struct zint_symbol *symbol, int row, int column) {
    int i;
    if (symbol->output_options & OUT_BUFFER_INTERMEDIATE) {
        i = row * symbol->bitmap_width + column;
        return symbol->bitmap[i] == '1'; // Black
    }
    i = (row * symbol->bitmap_width + column) * 3;
    return symbol->bitmap[i] == 0 && symbol->bitmap[i + 1] == 0 && symbol->bitmap[i + 2] == 0; // Black
}

static void test_options(int index, int debug) {

    struct item {
        int symbology;
        char *fgcolour;
        char *bgcolour;
        int rotate_angle;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
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
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

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

static void test_buffer(int index, int generate, int debug) {

    struct item {
        int symbology;
        int output_options;
        char *data;
        char *composite;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "1234567890", "", 50, 1, 108, 216, 116 },
        /*  1*/ { BARCODE_CODE11, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 108, 216, 116 },
        /*  2*/ { BARCODE_C25STANDARD, -1, "1234567890", "", 50, 1, 117, 234, 116 },
        /*  3*/ { BARCODE_C25STANDARD, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 117, 234, 116 },
        /*  4*/ { BARCODE_C25INTER, -1, "1234567890", "", 50, 1, 99, 198, 116 },
        /*  5*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 99, 198, 116 },
        /*  6*/ { BARCODE_C25IATA, -1, "1234567890", "", 50, 1, 149, 298, 116 },
        /*  7*/ { BARCODE_C25IATA, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 149, 298, 116 },
        /*  8*/ { BARCODE_C25LOGIC, -1, "1234567890", "", 50, 1, 109, 218, 116 },
        /*  9*/ { BARCODE_C25LOGIC, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 109, 218, 116 },
        /* 10*/ { BARCODE_C25IND, -1, "1234567890", "", 50, 1, 159, 318, 116 },
        /* 11*/ { BARCODE_C25IND, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 159, 318, 116 },
        /* 12*/ { BARCODE_CODE39, -1, "1234567890", "", 50, 1, 155, 310, 116 },
        /* 13*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 155, 310, 116 },
        /* 14*/ { BARCODE_EXCODE39, -1, "1234567890", "", 50, 1, 155, 310, 116 },
        /* 15*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 155, 310, 116 },
        /* 16*/ { BARCODE_EANX, -1, "123456789012", "", 50, 1, 95, 226, 116 },
        /* 17*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "123456789012", "", 69, 1, 95, 226, 154 },
        /* 18*/ { BARCODE_EANX_CHK, -1, "1234567890128", "", 50, 1, 95, 226, 116 },
        /* 19*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "1234567890128", "", 69, 1, 95, 226, 154 },
        /* 20*/ { BARCODE_EANX, -1, "123456789012+12", "", 50, 1, 122, 276, 116 },
        /* 21*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "123456789012+12", "", 69, 1, 122, 276, 154 },
        /* 22*/ { BARCODE_EANX_CHK, -1, "1234567890128+12", "", 50, 1, 122, 276, 116 },
        /* 23*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "1234567890128+12", "", 69, 1, 122, 276, 154 },
        /* 24*/ { BARCODE_EANX, -1, "123456789012+12345", "", 50, 1, 149, 330, 116 },
        /* 25*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "123456789012+12345", "", 69, 1, 149, 330, 154 },
        /* 26*/ { BARCODE_EANX_CHK, -1, "1234567890128+12345", "", 50, 1, 149, 330, 116 },
        /* 27*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "1234567890128+12345", "", 69, 1, 149, 330, 154 },
        /* 28*/ { BARCODE_EANX, -1, "1234567", "", 50, 1, 67, 162, 116 },
        /* 29*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "1234567", "", 55, 1, 67, 162, 126 },
        /* 30*/ { BARCODE_EANX_CHK, -1, "12345670", "", 50, 1, 67, 162, 116 },
        /* 31*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "12345670", "", 55, 1, 67, 162, 126 },
        /* 32*/ { BARCODE_EANX, -1, "1234567+12", "", 50, 1, 94, 216, 116 },
        /* 33*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "1234567+12", "", 55, 1, 94, 216, 126 },
        /* 34*/ { BARCODE_EANX_CHK, -1, "12345670+12", "", 50, 1, 94, 216, 116 },
        /* 35*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "12345670+12", "", 55, 1, 94, 216, 126 },
        /* 36*/ { BARCODE_EANX, -1, "1234567+12345", "", 50, 1, 121, 270, 116 },
        /* 37*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "1234567+12345", "", 55, 1, 121, 270, 126 },
        /* 38*/ { BARCODE_EANX_CHK, -1, "12345670+12345", "", 50, 1, 121, 270, 116 },
        /* 39*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "12345670+12345", "", 55, 1, 121, 270, 126 },
        /* 40*/ { BARCODE_EANX, -1, "1234", "", 50, 1, 47, 118, 116 },
        /* 41*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "1234", "", 66.5, 1, 47, 118, 149 },
        /* 42*/ { BARCODE_EANX_CHK, -1, "1234", "", 50, 1, 47, 118, 116 },
        /* 43*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "1234", "", 66.5, 1, 47, 118, 149 },
        /* 44*/ { BARCODE_EANX, -1, "12", "", 50, 1, 20, 64, 116 },
        /* 45*/ { BARCODE_EANX, COMPLIANT_HEIGHT, "12", "", 66.5, 1, 20, 64, 149 },
        /* 46*/ { BARCODE_EANX_CHK, -1, "12", "", 50, 1, 20, 64, 116 },
        /* 47*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, "12", "", 66.5, 1, 20, 64, 149 },
        /* 48*/ { BARCODE_GS1_128, -1, "[01]12345678901231", "", 50, 1, 134, 268, 116 },
        /* 49*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, "[01]12345678901231", "", 64, 1, 134, 268, 144 },
        /* 50*/ { BARCODE_CODABAR, -1, "A00000000B", "", 50, 1, 102, 204, 116 },
        /* 51*/ { BARCODE_CODABAR, COMPLIANT_HEIGHT, "A00000000B", "", 50, 1, 102, 204, 116 },
        /* 52*/ { BARCODE_CODE128, -1, "1234567890", "", 50, 1, 90, 180, 116 },
        /* 53*/ { BARCODE_CODE128, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 90, 180, 116 },
        /* 54*/ { BARCODE_DPLEIT, -1, "1234567890123", "", 50, 1, 135, 270, 116 },
        /* 55*/ { BARCODE_DPLEIT, COMPLIANT_HEIGHT, "1234567890123", "", 50, 1, 135, 270, 116 },
        /* 56*/ { BARCODE_DPIDENT, -1, "12345678901", "", 50, 1, 117, 234, 116 },
        /* 57*/ { BARCODE_DPIDENT, COMPLIANT_HEIGHT, "12345678901", "", 50, 1, 117, 234, 116 },
        /* 58*/ { BARCODE_CODE16K, -1, "1234567890", "", 20, 2, 70, 162, 44 },
        /* 59*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, "1234567890", "", 21, 2, 70, 162, 46 },
        /* 60*/ { BARCODE_CODE49, -1, "1234567890", "", 20, 2, 70, 162, 44 },
        /* 61*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, "1234567890", "", 21, 2, 70, 162, 46 },
        /* 62*/ { BARCODE_CODE93, -1, "1234567890", "", 50, 1, 127, 254, 116 },
        /* 63*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, "1234567890", "", 40, 1, 127, 254, 96 },
        /* 64*/ { BARCODE_FLAT, -1, "1234567890", "", 50, 1, 90, 180, 100 },
        /* 65*/ { BARCODE_FLAT, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 90, 180, 100 },
        /* 66*/ { BARCODE_DBAR_OMN, -1, "1234567890123", "", 50, 1, 96, 192, 116 },
        /* 67*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, "1234567890123", "", 33, 1, 96, 192, 82 },
        /* 68*/ { BARCODE_DBAR_LTD, -1, "1234567890123", "", 50, 1, 79, 158, 116 },
        /* 69*/ { BARCODE_DBAR_LTD, COMPLIANT_HEIGHT, "1234567890123", "", 10, 1, 79, 158, 36 },
        /* 70*/ { BARCODE_DBAR_EXP, -1, "[01]12345678901231", "", 34, 1, 134, 268, 84 },
        /* 71*/ { BARCODE_DBAR_EXP, COMPLIANT_HEIGHT, "[01]12345678901231", "", 34, 1, 134, 268, 84 },
        /* 72*/ { BARCODE_TELEPEN, -1, "1234567890", "", 50, 1, 208, 416, 116 },
        /* 73*/ { BARCODE_TELEPEN, COMPLIANT_HEIGHT, "1234567890", "", 32, 1, 208, 416, 80 },
        /* 74*/ { BARCODE_UPCA, -1, "12345678901", "", 50, 1, 95, 226, 116 },
        /* 75*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, "12345678901", "", 69, 1, 95, 226, 154 },
        /* 76*/ { BARCODE_UPCA_CHK, -1, "123456789012", "", 50, 1, 95, 226, 116 },
        /* 77*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, "123456789012", "", 69, 1, 95, 226, 154 },
        /* 78*/ { BARCODE_UPCA, -1, "12345678901+12", "", 50, 1, 124, 276, 116 },
        /* 79*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, "12345678901+12", "", 69, 1, 124, 276, 154 },
        /* 80*/ { BARCODE_UPCA_CHK, -1, "123456789012+12", "", 50, 1, 124, 276, 116 },
        /* 81*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, "123456789012+12", "", 69, 1, 124, 276, 154 },
        /* 82*/ { BARCODE_UPCA, -1, "12345678901+12345", "", 50, 1, 151, 330, 116 },
        /* 83*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, "12345678901+12345", "", 69, 1, 151, 330, 154 },
        /* 84*/ { BARCODE_UPCA_CHK, -1, "123456789012+12345", "", 50, 1, 151, 330, 116 },
        /* 85*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, "123456789012+12345", "", 69, 1, 151, 330, 154 },
        /* 86*/ { BARCODE_UPCE, -1, "1234567", "", 50, 1, 51, 134, 116 },
        /* 87*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, "1234567", "", 69, 1, 51, 134, 154 },
        /* 88*/ { BARCODE_UPCE_CHK, -1, "12345670", "", 50, 1, 51, 134, 116 },
        /* 89*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, "12345670", "", 69, 1, 51, 134, 154 },
        /* 90*/ { BARCODE_UPCE, -1, "1234567+12", "", 50, 1, 78, 184, 116 },
        /* 91*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, "1234567+12", "", 69, 1, 78, 184, 154 },
        /* 92*/ { BARCODE_UPCE_CHK, -1, "12345670+12", "", 50, 1, 78, 184, 116 },
        /* 93*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, "12345670+12", "", 69, 1, 78, 184, 154 },
        /* 94*/ { BARCODE_UPCE, -1, "1234567+12345", "", 50, 1, 105, 238, 116 },
        /* 95*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, "1234567+12345", "", 69, 1, 105, 238, 154 },
        /* 96*/ { BARCODE_UPCE_CHK, -1, "12345670+12345", "", 50, 1, 105, 238, 116 },
        /* 97*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, "12345670+12345", "", 69, 1, 105, 238, 154 },
        /* 98*/ { BARCODE_POSTNET, -1, "12345678901", "", 12, 2, 123, 246, 24 },
        /* 99*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, "12345678901", "", 5, 2, 123, 246, 10 },
        /*100*/ { BARCODE_MSI_PLESSEY, -1, "1234567890", "", 50, 1, 127, 254, 116 },
        /*101*/ { BARCODE_MSI_PLESSEY, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 127, 254, 116 },
        /*102*/ { BARCODE_FIM, -1, "A", "", 50, 1, 17, 34, 100 },
        /*103*/ { BARCODE_FIM, COMPLIANT_HEIGHT, "A", "", 20, 1, 17, 34, 40 },
        /*104*/ { BARCODE_LOGMARS, -1, "1234567890", "", 50, 1, 191, 382, 116 },
        /*105*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, "1234567890", "", 45.5, 1, 191, 382, 107 },
        /*106*/ { BARCODE_PHARMA, -1, "123456", "", 50, 1, 58, 116, 100 },
        /*107*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, "123456", "", 16, 1, 58, 116, 32 },
        /*108*/ { BARCODE_PZN, -1, "123456", "", 50, 1, 142, 284, 116 },
        /*109*/ { BARCODE_PZN, COMPLIANT_HEIGHT, "123456", "", 40, 1, 142, 284, 96 },
        /*110*/ { BARCODE_PHARMA_TWO, -1, "12345678", "", 10, 2, 29, 58, 20 },
        /*111*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, "12345678", "", 8, 2, 29, 58, 16 },
        /*112*/ { BARCODE_PDF417, -1, "1234567890", "", 21, 7, 103, 206, 42 },
        /*113*/ { BARCODE_PDF417, COMPLIANT_HEIGHT, "1234567890", "", 21, 7, 103, 206, 42 },
        /*114*/ { BARCODE_PDF417COMP, -1, "1234567890", "", 21, 7, 69, 138, 42 },
        /*115*/ { BARCODE_PDF417COMP, COMPLIANT_HEIGHT, "1234567890", "", 21, 7, 69, 138, 42 },
        /*116*/ { BARCODE_MAXICODE, -1, "1234567890", "", 165, 33, 30, 299, 298 },
        /*117*/ { BARCODE_MAXICODE, COMPLIANT_HEIGHT, "1234567890", "", 165, 33, 30, 299, 298 },
        /*118*/ { BARCODE_QRCODE, -1, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /*119*/ { BARCODE_QRCODE, COMPLIANT_HEIGHT, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /*120*/ { BARCODE_CODE128B, -1, "1234567890", "", 50, 1, 145, 290, 116 },
        /*121*/ { BARCODE_CODE128B, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 145, 290, 116 },
        /*122*/ { BARCODE_AUSPOST, -1, "12345678901234567890123", "", 8, 3, 133, 266, 16 },
        /*123*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, "12345678901234567890123", "", 9.5, 3, 133, 266, 19 },
        /*124*/ { BARCODE_AUSREPLY, -1, "12345678", "", 8, 3, 73, 146, 16 },
        /*125*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, "12345678", "", 9.5, 3, 73, 146, 19 },
        /*126*/ { BARCODE_AUSROUTE, -1, "12345678", "", 8, 3, 73, 146, 16 },
        /*127*/ { BARCODE_AUSROUTE, COMPLIANT_HEIGHT, "12345678", "", 9.5, 3, 73, 146, 19 },
        /*128*/ { BARCODE_AUSREDIRECT, -1, "12345678", "", 8, 3, 73, 146, 16 },
        /*129*/ { BARCODE_AUSREDIRECT, COMPLIANT_HEIGHT, "12345678", "", 9.5, 3, 73, 146, 19 },
        /*130*/ { BARCODE_ISBNX, -1, "123456789", "", 50, 1, 95, 226, 116 },
        /*131*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, "123456789", "", 69, 1, 95, 226, 154 },
        /*132*/ { BARCODE_ISBNX, -1, "123456789+12", "", 50, 1, 122, 276, 116 },
        /*133*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, "123456789+12", "", 69, 1, 122, 276, 154 },
        /*134*/ { BARCODE_ISBNX, -1, "123456789+12345", "", 50, 1, 149, 330, 116 },
        /*135*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, "123456789+12345", "", 69, 1, 149, 330, 154 },
        /*136*/ { BARCODE_RM4SCC, -1, "1234567890", "", 8, 3, 91, 182, 16 },
        /*137*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, "1234567890", "", 8, 3, 91, 182, 16 },
        /*138*/ { BARCODE_DATAMATRIX, -1, "ABC", "", 10, 10, 10, 20, 20 },
        /*139*/ { BARCODE_DATAMATRIX, COMPLIANT_HEIGHT, "ABC", "", 10, 10, 10, 20, 20 },
        /*140*/ { BARCODE_EAN14, -1, "1234567890123", "", 50, 1, 134, 268, 116 },
        /*141*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, "1234567890123", "", 64, 1, 134, 268, 144 },
        /*142*/ { BARCODE_VIN, -1, "12345678701234567", "", 50, 1, 246, 492, 116 },
        /*143*/ { BARCODE_VIN, COMPLIANT_HEIGHT, "12345678701234567", "", 50, 1, 246, 492, 116 },
        /*144*/ { BARCODE_CODABLOCKF, -1, "1234567890", "", 20, 2, 101, 242, 44 },
        /*145*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, "1234567890", "", 20, 2, 101, 242, 44 },
        /*146*/ { BARCODE_NVE18, -1, "12345678901234567", "", 50, 1, 156, 312, 116 },
        /*147*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, "12345678901234567", "", 64, 1, 156, 312, 144 },
        /*148*/ { BARCODE_JAPANPOST, -1, "1234567890", "", 8, 3, 133, 266, 16 },
        /*149*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, "1234567890", "", 6, 3, 133, 266, 12 },
        /*150*/ { BARCODE_KOREAPOST, -1, "123456", "", 50, 1, 167, 334, 116 },
        /*151*/ { BARCODE_KOREAPOST, COMPLIANT_HEIGHT, "123456", "", 50, 1, 167, 334, 116 },
        /*152*/ { BARCODE_DBAR_STK, -1, "1234567890123", "", 13, 3, 50, 100, 26 },
        /*153*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, "1234567890123", "", 13, 3, 50, 100, 26 },
        /*154*/ { BARCODE_DBAR_OMNSTK, -1, "1234567890123", "", 69, 5, 50, 100, 138 },
        /*155*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, "1234567890123", "", 69, 5, 50, 100, 138 },
        /*156*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901231", "", 71, 5, 102, 204, 142 },
        /*157*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, "[01]12345678901231", "", 71, 5, 102, 204, 142 },
        /*158*/ { BARCODE_PLANET, -1, "12345678901", "", 12, 2, 123, 246, 24 },
        /*159*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, "12345678901", "", 5, 2, 123, 246, 10 },
        /*160*/ { BARCODE_MICROPDF417, -1, "1234567890", "", 12, 6, 82, 164, 24 },
        /*161*/ { BARCODE_MICROPDF417, COMPLIANT_HEIGHT, "1234567890", "", 12, 6, 82, 164, 24 },
        /*162*/ { BARCODE_USPS_IMAIL, -1, "12345678901234567890", "", 8, 3, 129, 258, 16 },
        /*163*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, "12345678901234567890", "", 6, 3, 129, 258, 12 },
        /*164*/ { BARCODE_PLESSEY, -1, "1234567890", "", 50, 1, 227, 454, 116 },
        /*165*/ { BARCODE_PLESSEY, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 227, 454, 116 },
        /*166*/ { BARCODE_TELEPEN_NUM, -1, "1234567890", "", 50, 1, 128, 256, 116 },
        /*167*/ { BARCODE_TELEPEN_NUM, COMPLIANT_HEIGHT, "1234567890", "", 32, 1, 128, 256, 80 },
        /*168*/ { BARCODE_ITF14, -1, "1234567890", "", 50, 1, 135, 330, 136 },
        /*169*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, "1234567890", "", 64, 1, 135, 330, 164 },
        /*170*/ { BARCODE_KIX, -1, "123456ABCDE", "", 8, 3, 87, 174, 16 },
        /*171*/ { BARCODE_KIX, COMPLIANT_HEIGHT, "123456ABCDE", "", 8, 3, 87, 174, 16 },
        /*172*/ { BARCODE_AZTEC, -1, "1234567890AB", "", 15, 15, 15, 30, 30 },
        /*173*/ { BARCODE_AZTEC, COMPLIANT_HEIGHT, "1234567890AB", "", 15, 15, 15, 30, 30 },
        /*174*/ { BARCODE_DAFT, -1, "DAFTDAFTDAFTDAFT", "", 8, 3, 31, 62, 16 },
        /*175*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, "DAFTDAFTDAFTDAFT", "", 8, 3, 31, 62, 16 },
        /*176*/ { BARCODE_DPD, -1, "0123456789012345678901234567", "", 50, 1, 189, 378, 116 },
        /*177*/ { BARCODE_DPD, COMPLIANT_HEIGHT, "0123456789012345678901234567", "", 66.5, 1, 189, 378, 149 },
        /*178*/ { BARCODE_MICROQR, -1, "12345", "", 11, 11, 11, 22, 22 },
        /*179*/ { BARCODE_MICROQR, COMPLIANT_HEIGHT, "12345", "", 11, 11, 11, 22, 22 },
        /*180*/ { BARCODE_HIBC_128, -1, "1234567890", "", 50, 1, 123, 246, 116 },
        /*181*/ { BARCODE_HIBC_128, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 123, 246, 116 },
        /*182*/ { BARCODE_HIBC_39, -1, "1234567890", "", 50, 1, 223, 446, 116 },
        /*183*/ { BARCODE_HIBC_39, COMPLIANT_HEIGHT, "1234567890", "", 50, 1, 223, 446, 116 },
        /*184*/ { BARCODE_HIBC_DM, -1, "ABC", "", 12, 12, 12, 24, 24 },
        /*185*/ { BARCODE_HIBC_DM, COMPLIANT_HEIGHT, "ABC", "", 12, 12, 12, 24, 24 },
        /*186*/ { BARCODE_HIBC_QR, -1, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /*187*/ { BARCODE_HIBC_QR, COMPLIANT_HEIGHT, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /*188*/ { BARCODE_HIBC_PDF, -1, "1234567890", "", 24, 8, 103, 206, 48 },
        /*189*/ { BARCODE_HIBC_PDF, COMPLIANT_HEIGHT, "1234567890", "", 24, 8, 103, 206, 48 },
        /*190*/ { BARCODE_HIBC_MICPDF, -1, "1234567890", "", 28, 14, 38, 76, 56 },
        /*191*/ { BARCODE_HIBC_MICPDF, COMPLIANT_HEIGHT, "1234567890", "", 28, 14, 38, 76, 56 },
        /*192*/ { BARCODE_HIBC_BLOCKF, -1, "1234567890", "", 30, 3, 101, 242, 64 },
        /*193*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, "1234567890", "", 30, 3, 101, 242, 64 },
        /*194*/ { BARCODE_HIBC_AZTEC, -1, "1234567890AB", "", 19, 19, 19, 38, 38 },
        /*195*/ { BARCODE_HIBC_AZTEC, COMPLIANT_HEIGHT, "1234567890AB", "", 19, 19, 19, 38, 38 },
        /*196*/ { BARCODE_DOTCODE, -1, "ABC", "", 11, 11, 16, 33, 23 },
        /*197*/ { BARCODE_DOTCODE, COMPLIANT_HEIGHT, "ABC", "", 11, 11, 16, 33, 23 },
        /*198*/ { BARCODE_HANXIN, -1, "1234567890AB", "", 23, 23, 23, 46, 46 },
        /*199*/ { BARCODE_HANXIN, COMPLIANT_HEIGHT, "1234567890AB", "", 23, 23, 23, 46, 46 },
        /*200*/ { BARCODE_MAILMARK, -1, "01000000000000000AA00AA0A", "", 10, 3, 155, 310, 20 },
        /*201*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, "01000000000000000AA00AA0A", "", 8, 3, 155, 310, 16 },
        /*202*/ { BARCODE_AZRUNE, -1, "255", "", 11, 11, 11, 22, 22 },
        /*203*/ { BARCODE_AZRUNE, COMPLIANT_HEIGHT, "255", "", 11, 11, 11, 22, 22 },
        /*204*/ { BARCODE_CODE32, -1, "12345678", "", 50, 1, 103, 206, 116 },
        /*205*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, "12345678", "", 20, 1, 103, 206, 56 },
        /*206*/ { BARCODE_EANX_CC, -1, "123456789012", "[20]01", 50, 7, 99, 234, 116 },
        /*207*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "123456789012", "[20]01", 81, 7, 99, 234, 178 },
        /*208*/ { BARCODE_EANX_CC, -1, "123456789012+12", "[20]01", 50, 7, 126, 284, 116 },
        /*209*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "123456789012+12", "[20]01", 81, 7, 126, 284, 178 },
        /*210*/ { BARCODE_EANX_CC, -1, "123456789012+12345", "[20]01", 50, 7, 153, 338, 116 },
        /*211*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "123456789012+12345", "[20]01", 81, 7, 153, 338, 178 },
        /*212*/ { BARCODE_EANX_CC, -1, "1234567", "[20]01", 50, 8, 72, 172, 116 },
        /*213*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "1234567", "[20]01", 69, 8, 72, 172, 154 },
        /*214*/ { BARCODE_EANX_CC, -1, "1234567+12", "[20]01", 50, 8, 99, 226, 116 },
        /*215*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "1234567+12", "[20]01", 69, 8, 99, 226, 154 },
        /*216*/ { BARCODE_EANX_CC, -1, "1234567+12345", "[20]01", 50, 8, 126, 280, 116 },
        /*217*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, "1234567+12345", "[20]01", 69, 8, 126, 280, 154 },
        /*218*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]01", 50, 5, 145, 290, 116 },
        /*219*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, "[01]12345678901231", "[20]01", 71, 5, 145, 290, 158 },
        /*220*/ { BARCODE_DBAR_OMN_CC, -1, "1234567890123", "[20]01", 21, 5, 100, 200, 58 },
        /*221*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, "1234567890123", "[20]01", 40, 5, 100, 200, 96 },
        /*222*/ { BARCODE_DBAR_LTD_CC, -1, "1234567890123", "[20]01", 19, 6, 79, 158, 54 },
        /*223*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, "1234567890123", "[20]01", 19, 6, 79, 158, 54 },
        /*224*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231", "[20]01", 41, 5, 134, 268, 98 },
        /*225*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, "[01]12345678901231", "[20]01", 41, 5, 134, 268, 98 },
        /*226*/ { BARCODE_UPCA_CC, -1, "12345678901", "[20]01", 50, 7, 99, 234, 116 },
        /*227*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, "12345678901", "[20]01", 81, 7, 99, 234, 178 },
        /*228*/ { BARCODE_UPCA_CC, -1, "12345678901+12", "[20]01", 50, 7, 128, 284, 116 },
        /*229*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, "12345678901+12", "[20]01", 81, 7, 128, 284, 178 },
        /*230*/ { BARCODE_UPCA_CC, -1, "12345678901+12345", "[20]01", 50, 7, 155, 338, 116 },
        /*231*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, "12345678901+12345", "[20]01", 81, 7, 155, 338, 178 },
        /*232*/ { BARCODE_UPCE_CC, -1, "1234567", "[20]01", 50, 9, 55, 142, 116 },
        /*233*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, "1234567", "[20]01", 85, 9, 55, 142, 186 },
        /*234*/ { BARCODE_UPCE_CC, -1, "1234567+12", "[20]01", 50, 9, 82, 192, 116 },
        /*235*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, "1234567+12", "[20]01", 85, 9, 82, 192, 186 },
        /*236*/ { BARCODE_UPCE_CC, -1, "1234567+12345", "[20]01", 50, 9, 109, 246, 116 },
        /*237*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, "1234567+12345", "[20]01", 85, 9, 109, 246, 186 },
        /*238*/ { BARCODE_DBAR_STK_CC, -1, "1234567890123", "[20]01", 24, 9, 56, 112, 48 },
        /*239*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, "1234567890123", "[20]01", 24, 9, 56, 112, 48 },
        /*240*/ { BARCODE_DBAR_OMNSTK_CC, -1, "1234567890123", "[20]01", 80, 11, 56, 112, 160 },
        /*241*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, "1234567890123", "[20]01", 80, 11, 56, 112, 160 },
        /*242*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231", "[20]01", 78, 9, 102, 204, 156 },
        /*243*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, "[01]12345678901231", "[20]01", 78, 9, 102, 204, 156 },
        /*244*/ { BARCODE_CHANNEL, -1, "01", "", 50, 1, 19, 38, 116 },
        /*245*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, "01", "", 20, 1, 19, 38, 56 },
        /*246*/ { BARCODE_CODEONE, -1, "12345678901234567890", "", 16, 16, 18, 36, 32 },
        /*247*/ { BARCODE_CODEONE, COMPLIANT_HEIGHT, "12345678901234567890", "", 16, 16, 18, 36, 32 },
        /*248*/ { BARCODE_GRIDMATRIX, -1, "ABC", "", 18, 18, 18, 36, 36 },
        /*249*/ { BARCODE_GRIDMATRIX, COMPLIANT_HEIGHT, "ABC", "", 18, 18, 18, 36, 36 },
        /*250*/ { BARCODE_UPNQR, -1, "1234567890AB", "", 77, 77, 77, 154, 154 },
        /*251*/ { BARCODE_UPNQR, COMPLIANT_HEIGHT, "1234567890AB", "", 77, 77, 77, 154, 154 },
        /*252*/ { BARCODE_ULTRA, -1, "1234567890", "", 13, 13, 18, 36, 26 },
        /*253*/ { BARCODE_ULTRA, COMPLIANT_HEIGHT, "1234567890", "", 13, 13, 18, 36, 26 },
        /*254*/ { BARCODE_RMQR, -1, "12345", "", 11, 11, 27, 54, 22 },
        /*255*/ { BARCODE_RMQR, COMPLIANT_HEIGHT, "12345", "", 11, 11, 27, 54, 22 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;

    testStart("test_buffer");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d\n", i); // ZINT_DEBUG_TEST_PRINT 16

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, text, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        if (generate) {
            printf("        /*%3d*/ { %s, %s, \"%s\", \"%s\", %.8g, %d, %d, %d, %d },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilOutputOptionsName(data[i].output_options),
                    data[i].data, data[i].composite,
                    symbol->height, symbol->rows, symbol->width, symbol->bitmap_width, symbol->bitmap_height);
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

static void test_upcean_hrt(int index, int debug) {

    struct item {
        int symbology;
        int show_hrt;
        char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_text_row;
        int expected_text_col;
        int expected_text_len;
        int expected_addon_text_row;
        int expected_addon_text_col;
        int expected_addon_text_len;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  1*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 226, 110, 102 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  2*/ { BARCODE_EANX_CHK, -1, "1234567890128", 0, 50, 1, 95, 226, 116, 102 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  3*/ { BARCODE_EANX_CHK, 0, "1234567890128", 0, 50, 1, 95, 226, 110, 102 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  4*/ { BARCODE_EANX_CHK, -1, "1234567890128+12", 0, 50, 1, 122, 276, 116, 102 /*text_row*/, 0, 20, 5, 212, 64 }, // EAN-13 + EAN-2
        /*  5*/ { BARCODE_EANX_CHK, 0, "1234567890128+12", 0, 50, 1, 122, 276, 110, 102 /*text_row*/, 0, 20, 5, 212, 64 }, // EAN-13 + EAN-2
        /*  6*/ { BARCODE_EANX, -1, "1234567890128+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 20, 5, 212, 118 }, // EAN-13 + EAN-5
        /*  7*/ { BARCODE_EANX, 0, "1234567890128+12345", 0, 50, 1, 149, 330, 110, 102 /*text_row*/, 0, 20, 5, 212, 118 }, // EAN-13 + EAN-5
        /*  8*/ { BARCODE_ISBNX, -1, "9784567890120+12345", 0, 50, 1, 149, 330, 116, 102 /*text_row*/, 0, 20, 5, 212, 118 }, // ISBNX + EAN-5
        /*  9*/ { BARCODE_ISBNX, 0, "9784567890120+12345", 0, 50, 1, 149, 330, 110, 102 /*text_row*/, 0, 20, 5, 212, 118 }, // ISBNX + EAN-5
        /* 10*/ { BARCODE_EANX, -1, "123456", 0, 50, 1, 67, 162, 116, 102 /*text_row*/, 20, 58, -1, -1, -1 }, // EAN-8
        /* 11*/ { BARCODE_EANX, 0, "123456", 0, 50, 1, 67, 162, 110, 102 /*text_row*/, 20, 58, -1, -1, -1 }, // EAN-8
        /* 12*/ { BARCODE_EANX, -1, "123456+12", 0, 50, 1, 94, 216, 116, 102 /*text_row*/, 20, 58, 5, 148, 68 }, // EAN-8 + EAN-2
        /* 13*/ { BARCODE_EANX, 0, "123456+12", 0, 50, 1, 94, 216, 110, 102 /*text_row*/, 20, 58, 5, 148, 68 }, // EAN-8 + EAN-2
        /* 14*/ { BARCODE_EANX, -1, "123456+12345", 0, 50, 1, 121, 270, 116, 102 /*text_row*/, 20, 58, 5, 148, 122 }, // EAN-8 + EAN-5
        /* 15*/ { BARCODE_EANX, 0, "123456+12345", 0, 50, 1, 121, 270, 110, 102 /*text_row*/, 20, 58, 5, 148, 122 }, // EAN-8 + EAN-5
        /* 16*/ { BARCODE_EANX, -1, "1234", 0, 50, 1, 47, 118, 116, 102 /*text_row*/, 40, 36, -1, -1, -1 }, // EAN-5
        /* 17*/ { BARCODE_EANX, 0, "1234", 0, 50, 1, 47, 118, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 }, // EAN-5
        /* 18*/ { BARCODE_EANX, -1, "12", 0, 50, 1, 20, 64, 116, 102 /*text_row*/, 20, 20, -1, -1, -1 }, // EAN-2
        /* 19*/ { BARCODE_EANX, 0, "12", 0, 50, 1, 20, 64, 100, -1 /*text_row*/, -1, -1, -1, -1, -1 }, // EAN-2
        /* 20*/ { BARCODE_UPCA, -1, "123456789012", 0, 50, 1, 95, 226, 116, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 21*/ { BARCODE_UPCA, 0, "123456789012", 0, 50, 1, 95, 226, 110, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 22*/ { BARCODE_UPCA, -1, "123456789012+12", 0, 50, 1, 124, 276, 116, 104 /*text_row*/, 0, 18, 5, 208, 68 },
        /* 23*/ { BARCODE_UPCA, 0, "123456789012+12", 0, 50, 1, 124, 276, 110, 104 /*text_row*/, 0, 18, 5, 208, 68 },
        /* 24*/ { BARCODE_UPCA_CHK, -1, "123456789012+12345", 0, 50, 1, 151, 330, 116, 104 /*text_row*/, 0, 18, 5, 208, 122 },
        /* 25*/ { BARCODE_UPCA_CHK, 0, "123456789012+12345", 0, 50, 1, 151, 330, 110, 104 /*text_row*/, 0, 18, 5, 208, 122 },
        /* 26*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 134, 116, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 27*/ { BARCODE_UPCE, 0, "1234567", 0, 50, 1, 51, 134, 110, 104 /*text_row*/, 0, 18, -1, -1, -1 },
        /* 28*/ { BARCODE_UPCE_CHK, -1, "12345670+12", 0, 50, 1, 78, 184, 116, 104 /*text_row*/, 0, 18, 5, 120, 64 },
        /* 29*/ { BARCODE_UPCE_CHK, 0, "12345670+12", 0, 50, 1, 78, 184, 110, 104 /*text_row*/, 0, 18, 5, 120, 64 },
        /* 30*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 105, 238, 116, 104 /*text_row*/, 0, 18, 5, 120, 118 },
        /* 31*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 105, 238, 110, 104 /*text_row*/, 0, 18, 5, 120, 118 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_upcean_hrt");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i); // ZINT_DEBUG_TEST_PRINT 16

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%s) symbol->bitmap_width %d != %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%s) symbol->bitmap_height %d != %d\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->bitmap_height, data[i].expected_bitmap_height);

        assert_nonzero(symbol->bitmap_height > data[i].expected_text_row, "i:%d symbol->bitmap_height %d <= data[i].expected_text_row %d\n", i, symbol->bitmap_height, data[i].expected_text_row);
        if (data[i].expected_text_row == -1) { /* EAN-2/5 just truncates bitmap if no text */
            assert_zero(data[i].show_hrt, "i:%d Expected text row -1 but show_hrt set\n", i);
            if (i && data[i - 1].symbology == symbol->symbology && data[i - 1].show_hrt && data[i - 1].expected_text_row != -1) {
                assert_nonzero(data[i].expected_bitmap_height < data[i - 1].expected_text_row, "i:%d (%s) expected_bitmap_height %d >= previous expected_text_row %d\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].expected_bitmap_height, data[i - 1].expected_text_row);
            }
        } else {
            int text_bits_set = 0;
            int row = data[i].expected_text_row;
            int column;
            for (column = data[i].expected_text_col; column < data[i].expected_text_col + data[i].expected_text_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    text_bits_set = 1;
                    break;
                }
            }
            if (symbol->show_hrt) {
                assert_nonzero(text_bits_set, "i:%d (%s) text_bits_set zero\n", i, testUtilBarcodeName(data[i].symbology));
            } else {
                assert_zero(text_bits_set, "i:%d (%s) text_bits_set non-zero\n", i, testUtilBarcodeName(data[i].symbology));
            }
        }

        if (data[i].expected_addon_text_row != -1) {
            int addon_text_bits_set = 0;
            int row = data[i].expected_addon_text_row;
            int column;
            for (column = data[i].expected_addon_text_col; column < data[i].expected_addon_text_col + data[i].expected_addon_text_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    addon_text_bits_set = 1;
                    break;
                }
            }
            if (symbol->show_hrt) {
                assert_nonzero(addon_text_bits_set, "i:%d (%s) addon_text_bits_set zero\n", i, testUtilBarcodeName(data[i].symbology));
            } else {
                assert_zero(addon_text_bits_set, "i:%d (%s) addon_text_bits_set non-zero\n", i, testUtilBarcodeName(data[i].symbology));
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_row_separator(int index, int debug) {

    struct item {
        int symbology;
        int border_width;
        int option_1;
        int option_3;
        char *data;
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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, -1, -1, -1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 },
        /*  1*/ { BARCODE_CODABLOCKF, -1, -1, 0, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // Same as default
        /*  2*/ { BARCODE_CODABLOCKF, -1, -1, 1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // Same as default
        /*  3*/ { BARCODE_CODABLOCKF, -1, -1, 2, "A", 0, 20, 2, 101, 242, 44, 20, 42, 4 },
        /*  4*/ { BARCODE_CODABLOCKF, -1, -1, 3, "A", 0, 20, 2, 101, 242, 44, 19, 42, 6 },
        /*  5*/ { BARCODE_CODABLOCKF, -1, -1, 4, "A", 0, 20, 2, 101, 242, 44, 18, 42, 8 },
        /*  6*/ { BARCODE_CODABLOCKF, -1, -1, 5, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // > 4 ignored, same as default
        /*  7*/ { BARCODE_CODABLOCKF, -1, 1, -1, "A", 0, 5, 1, 46, 132, 14, 0, 20 + 2, 2 }, // CODE128 top separator, add 2 to skip over end of start char; note no longer includes HRT
        /*  8*/ { BARCODE_CODABLOCKF, 0, -1, -1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // Border width zero, same as default
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_row_separator");

    for (i = 0; i < data_size; i++) {
        int j, separator_bits_set;


        if (index != -1 && i != index) continue;

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

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        for (j = data[i].expected_separator_row; j < data[i].expected_separator_row + data[i].expected_separator_height; j++) {
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
            assert_nonzero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);
        }

        if (symbol->rows > 1) {
            j = data[i].expected_separator_row - 1;
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col + 2); // Need to add 2 to skip to 1st blank of start row character
            assert_zero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) before non-zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);
        }

        j = data[i].expected_separator_row + data[i].expected_separator_height;
        separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col + 2); // Need to add 2 to skip to 1st blank of start row character
        assert_zero(separator_bits_set, "i:%d (%s) separator_bits_set (%d, %d) after non-zero\n", i, testUtilBarcodeName(data[i].symbology), j, data[i].expected_separator_col);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_stacking(int index, int debug) {

    struct item {
        int symbology;
        int output_options;
        int option_1;
        int option_3;
        char *data;
        char *data2;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_separator_row;
        int expected_separator_col;
        int expected_separator_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, "A", "B", 50, 2, 46, 92, 116, -1, -1, -1 },
        /*  1*/ { BARCODE_CODE128, BARCODE_BIND, -1, -1, "A", "B", 50, 2, 46, 92, 116, 49, 4, 2 },
        /*  2*/ { BARCODE_CODE128, BARCODE_BIND, -1, 2, "A", "B", 50, 2, 46, 92, 116, 48, 4, 4 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_stacking");

    for (i = 0; i < data_size; i++) {
        int length2;
        int j, separator_bits_set;


        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, data[i].output_options, data[i].data, -1, debug);
        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);

        length2 = (int) strlen(data[i].data2);
        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data2, length2);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ret %d != zero\n", i, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

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

static void test_output_options(int index, int debug) {

    struct item {
        int symbology;
        int whitespace_width;
        int whitespace_height;
        int border_width;
        int output_options;
        int rotate_angle;
        char *data;
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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, -1, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  1*/ { BARCODE_CODE128, -1, -1, -1, -1, 180, "A123", 0, 50, 1, 79, 158, 116, 0, 115, 4 },
        /*  2*/ { BARCODE_CODE128, -1, -1, 2, -1, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  3*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 158, 124, 1, 0, 4 },
        /*  4*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 158, 124, 0, 4, 4 },
        /*  5*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BOX, 0, "A123", 0, 50, 1, 79, 166, 124, 1, 4, 4 },
        /*  6*/ { BARCODE_CODE128, -1, -1, 0, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /*  7*/ { BARCODE_CODE128, -1, -1, 0, BARCODE_BOX, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 4, 4 },
        /*  8*/ { BARCODE_CODE128, -1, -1, -1, -1, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 8 },
        /*  9*/ { BARCODE_CODE128, 3, -1, -1, -1, 0, "A123", 0, 50, 1, 79, 170, 116, 1, 0, 8 },
        /* 10*/ { BARCODE_CODE128, 3, 1, -1, -1, 0, "A123", 0, 50, 1, 79, 170, 120, 0, 0, 8 },
        /* 11*/ { BARCODE_CODE128, 3, -1, 4, -1, 0, "A123", 0, 50, 1, 79, 170, 116, 1, 0, 8 },
        /* 12*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 170, 132, 1, 0, 0 },
        /* 13*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BIND, 0, "A123", 0, 50, 1, 79, 170, 132, 0, 8, 0 },
        /* 14*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BOX, 0, "A123", 0, 50, 1, 79, 186, 132, 1, 8, 0 },
        /* 15*/ { BARCODE_CODE128, -1, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 16*/ { BARCODE_CODE128, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 50, 1, 79, 158, 116, 0, 0, 4 },
        /* 17*/ { BARCODE_CODE128, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 180, "A123", 0, 50, 1, 79, 158, 116, 0, 115, 4 },
        /* 18*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BOX | OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 50, 1, 79, 186, 132, 1, 8, 0 },
        /* 19*/ { BARCODE_QRCODE, -1, -1, -1, -1, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 20*/ { BARCODE_QRCODE, -1, -1, -1, -1, 180, "A123", 0, 21, 21, 21, 42, 42, 0, 39, 2 },
        /* 21*/ { BARCODE_QRCODE, -1, -1, 3, -1, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 22*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, 0, "A123", 0, 21, 21, 21, 42, 54, 1, 2, 2 },
        /* 23*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, 0, "A123", 0, 21, 21, 21, 42, 54, 0, 20, 0 },
        /* 24*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX, 0, "A123", 0, 21, 21, 21, 54, 54, 1, 20, 0 },
        /* 25*/ { BARCODE_QRCODE, -1, -1, -1, -1, 0, "A123", 0, 21, 21, 21, 42, 42, 1, 0, 0 },
        /* 26*/ { BARCODE_QRCODE, 5, -1, -1, -1, 0, "A123", 0, 21, 21, 21, 62, 42, 0, 0, 0 },
        /* 27*/ { BARCODE_QRCODE, 5, -1, 6, -1, 0, "A123", 0, 21, 21, 21, 62, 42, 0, 0, 0 },
        /* 28*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, 0, "A123", 0, 21, 21, 21, 62, 66, 1, 0, 0 },
        /* 29*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, 0, "A123", 0, 21, 21, 21, 62, 66, 0, 12, 0 },
        /* 30*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BOX, 0, "A123", 0, 21, 21, 21, 86, 66, 1, 12, 0 },
        /* 31*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 1, 1 },
        /* 32*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 2, 2 },
        /* 33*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 41, 1 },
        /* 34*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 40, 2 },
        /* 35*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 1, 1 },
        /* 36*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 2, 2 },
        /* 37*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 43, 59, 1, 2, 2 },
        /* 38*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 59, 59, 1, 9, 9 },
        /* 39*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 63, 59, 0, 9, 9 },
        /* 40*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 63, 59, 1, 0, 0 },
        /* 41*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 63, 59, 1, 9, 11 },
        /* 42*/ { BARCODE_QRCODE, 1, 1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 63, 63, 0, 0, 0 },
        /* 43*/ { BARCODE_QRCODE, 1, 1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, 0, "A123", 0, 21, 21, 21, 63, 63, 1, 9, 13 },
        /* 44*/ { BARCODE_QRCODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 21, 21, 21, 42, 42, 1, 1, 1 },
        /* 45*/ { BARCODE_QRCODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 46*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 21, 21, 21, 43, 43, 1, 1, 1 },
        /* 47*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 21, 21, 21, 43, 43, 0, 2, 2 },
        /* 48*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 180, "A123", 0, 21, 21, 21, 43, 43, 1, 41, 1 },
        /* 49*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE | OUT_BUFFER_INTERMEDIATE, 180, "A123", 0, 21, 21, 21, 43, 43, 0, 40, 2 },
        /* 50*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 0, "A123", 0, 165, 33, 30, 299, 298, 0, 4, 4 },
        /* 51*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 0, "A123", 0, 165, 33, 30, 299, 298, 1, 4, 14 },
        /* 52*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 270, "A123", 0, 165, 33, 30, 298, 299, 1, 4, 4 },
        /* 53*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 270, "A123", 0, 165, 33, 30, 298, 299, 0, 4, 14 },
        /* 54*/ { BARCODE_MAXICODE, -1, -1, 5, -1, 0, "A123", 0, 165, 33, 30, 299, 298, 0, 0, 0 },
        /* 55*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 1, 0, 0 },
        /* 56*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 0, 50, 0 },
        /* 57*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 0, 347, 50 },
        /* 58*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 299, 298 + 50 * 2, 1, 348, 50 },
        /* 59*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BOX, 0, "A123", 0, 165, 33, 30, 299 + 50 * 2, 298 + 50 * 2, 1, 50, 0 },
        /* 60*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BOX, 0, "A123", 0, 165, 33, 30, 299 + 50 * 2, 298 + 50 * 2, 0, 347, 50 },
        /* 61*/ { BARCODE_MAXICODE, -1, -1, -1, -1, 0, "A123", 0, 165, 33, 30, 299, 298, 1, 0, 14 },
        /* 62*/ { BARCODE_MAXICODE, 6, -1, -1, -1, 0, "A123", 0, 165, 33, 30, 299 + 60 * 2, 298, 0, 0, 14 },
        /* 63*/ { BARCODE_MAXICODE, 6, -1, -1, -1, 0, "A123", 0, 165, 33, 30, 299 + 60 * 2, 298, 0, 0, 47 },
        /* 64*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 299 + 60 * 2, 298 + 50 * 2, 1, 0, 47 },
        /* 65*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BIND, 0, "A123", 0, 165, 33, 30, 299 + 60 * 2, 298 + 50 * 2, 0, 50, 0 },
        /* 66*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BOX, 0, "A123", 0, 165, 33, 30, 299 + (60 + 50) * 2, 298 + 50 * 2, 1, 50, 0 },
        /* 67*/ { BARCODE_MAXICODE, -1, -1, -1, BARCODE_DOTTY_MODE, 0, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 68*/ { BARCODE_MAXICODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 165, 33, 30, 299, 298, 0, 4, 4 },
        /* 69*/ { BARCODE_MAXICODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "A123", 0, 165, 33, 30, 299, 298, 1, 4, 14 },
        /* 70*/ { BARCODE_MAXICODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 270, "A123", 0, 165, 33, 30, 298, 299, 1, 4, 4 },
        /* 71*/ { BARCODE_MAXICODE, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 270, "A123", 0, 165, 33, 30, 298, 299, 0, 4, 14 },
        /* 72*/ { BARCODE_ITF14, -1, -1, -1, -1, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 73*/ { BARCODE_ITF14, -1, -1, -1, -1, 90, "123", 0, 50, 1, 135, 136, 330, 1, 0, 110 },
        /* 74*/ { BARCODE_ITF14, -1, -1, 0, -1, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 75*/ { BARCODE_ITF14, -1, -1, 0, BARCODE_BOX, 0, "123", 0, 50, 1, 135, 310, 116, 0, 100, 0 },
        /* 76*/ { BARCODE_ITF14, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 0, "123", 0, 50, 1, 135, 330, 136, 1, 110, 0 },
        /* 77*/ { BARCODE_ITF14, -1, -1, -1, OUT_BUFFER_INTERMEDIATE, 90, "123", 0, 50, 1, 135, 136, 330, 1, 0, 110 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_output_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

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

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, data[i].rotate_angle);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_nonnull(symbol->bitmap, "i:%d (%s) symbol->bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

            if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

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

static void test_draw_string_wrap(int index, int debug) {

    struct item {
        int symbology;
        int output_options;
        char *data;
        char *text;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_no_text_row;
        int expected_no_text_col;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, "12", "              E", 50, 1, 46, 92, 116, 104, 0 },
        /*  1*/ { BARCODE_CODE128, BOLD_TEXT, "12", "           E", 50, 1, 46, 92, 116, 104, 0 },
        /*  2*/ { BARCODE_CODE128, SMALL_TEXT, "12", "                   E", 50, 1, 46, 92, 112, 103, 0 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_draw_string_wrap");

    for (i = 0; i < data_size; i++) {
        int text_bits_set, row, column;

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        // Cheat by overwriting text
        strcpy((char *) symbol->text, data[i].text);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Print(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

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

static void test_code128_utf8(int index, int debug) {

    struct item {
        char *data;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_text_row;
        int expected_text_col;
        int expected_text_len;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "é", 50, 1, 57, 114, 116, 110, 53, 6 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_code128_utf8");

    for (i = 0; i < data_size; i++) {
        int text_bits_set, row, column;

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODE128, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, BARCODE_CODE128, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0\n", i, BARCODE_CODE128, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, BARCODE_CODE128);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, BARCODE_CODE128, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, BARCODE_CODE128, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, BARCODE_CODE128, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, BARCODE_CODE128, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, BARCODE_CODE128, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Print(%d) ret %d != 0\n", i, BARCODE_CODE128, ret);
        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

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

static void test_scale(int index, int debug) {

    struct item {
        int symbology;
        int option_2;
        int border_width;
        int output_options;
        float height;
        float scale;
        char *data;
        char *composite;

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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, 0, 0, "1", "", 0, 18, 6, 103, 206, 36, 0, 36, 170, 14 }, // With no scaling
        /*  1*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.6, "1", "", 0, 18, 6, 103, 206 * 0.6, 36 * 0.6, 0 /*set_row*/, 36 * 0.6, 170 * 0.6 + 1, 14 * 0.6 }, // +1 set_col due to some scaling inversion difference
        /*  2*/ { BARCODE_PDF417, -1, -1, -1, 0, 1.2, "1", "", 0, 18, 6, 103, 206 * 1.2, 36 * 1.2, 0 /*set_row*/, 36 * 1.2, 170 * 1.2 + 1, 14 * 1.2 }, // +1 set_col due to some scaling inversion difference
        /*  3*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.5, "1", "", 0, 18, 6, 103, 206 * 0.5, 36 * 0.5, 0 /*set_row*/, 36 * 0.5, 170 * 0.5, 14 * 0.5 },
        /*  4*/ { BARCODE_PDF417, -1, -1, -1, 0, 1.0, "1", "", 0, 18, 6, 103, 206 * 1.0, 36 * 1.0, 0 /*set_row*/, 36 * 1.0, 170 * 1.0, 14 * 1.0 },
        /*  5*/ { BARCODE_PDF417, -1, -1, -1, 0, 1.5, "1", "", 0, 18, 6, 103, 206 * 1.5, 36 * 1.5, 0 /*set_row*/, 36 * 1.5, 170 * 1.5, 14 * 1.5 },
        /*  6*/ { BARCODE_PDF417, -1, -1, -1, 0, 2.0, "1", "", 0, 18, 6, 103, 206 * 2.0, 36 * 2.0, 0 /*set_row*/, 36 * 2.0, 170 * 2.0, 14 * 2.0 },
        /*  7*/ { BARCODE_PDF417, -1, -1, -1, 0, 2.5, "1", "", 0, 18, 6, 103, 206 * 2.5, 36 * 2.5, 0 /*set_row*/, 36 * 2.5, 170 * 2.5, 14 * 2.5 },
        /*  8*/ { BARCODE_PDF417, -1, -1, -1, 0, 3.0, "1", "", 0, 18, 6, 103, 206 * 3.0, 36 * 3.0, 0 /*set_row*/, 36 * 3.0, 170 * 3.0, 14 * 3.0 },
        /*  9*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 0, "1", "", 0, 18, 6, 103, 218, 48, 0 /*set_row*/, 48, 176, 14 }, // With no scaling
        /* 10*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 0.6, "1", "", 0, 18, 6, 103, 218 * 0.6, 48 * 0.6, 0 /*set_row*/, 48 * 0.6, 176 * 0.6 + 1, 14 * 0.6 }, // +1 set_col due to some scaling inversion difference
        /* 11*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 1.6, "1", "", 0, 18, 6, 103, 218 * 1.6, 48 * 1.6, 0 /*set_row*/, 48 * 1.6, 176 * 1.6 + 1, 14 * 1.6 }, // +1 set_col due to some scaling inversion difference
        /* 12*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 1.5, "1", "", 0, 18, 6, 103, 218 * 1.5, 48 * 1.5, 0 /*set_row*/, 48 * 1.5, 176 * 1.5, 14 * 1.5 },
        /* 13*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, 2.5, "1", "", 0, 18, 6, 103, 218 * 2.5, 48 * 2.5, 0 /*set_row*/, 48 * 2.5, 176 * 2.5, 14 * 2.5 },
        /* 14*/ { BARCODE_PDF417, -1, 3, OUT_BUFFER_INTERMEDIATE, 0, 1.3, "1", "", 0, 18, 6, 103, 206 * 1.3, 36 * 1.3, 0 /*set_row*/, 36 * 1.3, 170 * 1.3, 14 * 1.3 },
        /* 15*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, 0, "123456789012", "", 0, 50, 1, 79, 158, 116, 104 /*set_row*/, 114, 20, 2 }, // With no scaling
        /* 16*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, 1.5, "123456789012", "", 0, 50, 1, 79, 158 * 1.5, 116 * 1.5, 104 * 1.5 /*set_row*/, 114 * 1.5, 20 * 1.5, 1 * 1.5 },
        /* 17*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, 2.0, "123456789012", "", 0, 50, 1, 79, 158 * 2.0, 116 * 2.0, 104 * 2.0 /*set_row*/, 114 * 2.0, 20 * 2.0, 1 * 2.0 },
        /* 18*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, 3.5, "123456789012", "", 0, 50, 1, 79, 158 * 3.5, 116 * 3.5, 104 * 3.5 /*set_row*/, 114 * 3.5, 20 * 3.5, 1 * 3.5 },
        /* 19*/ { BARCODE_UPCA, -1, -1, -1, 0, 0, "12345678904", "", 0, 50, 1, 95, 226, 116, 104 /*set_row*/, 114, 5, 2 }, // With no scaling
        /* 20*/ { BARCODE_UPCA, -1, -1, -1, 0, 2.5, "12345678904", "", 0, 50, 1, 95, 226 * 2.5, 116 * 2.5, 104 * 2.5 /*set_row*/, 114 * 2.5, 5 * 2.5, 2 * 2.5 },
        /* 21*/ { BARCODE_UPCA, -1, -1, -1, 0, 4.5, "12345678904", "", 0, 50, 1, 95, 226 * 4.5, 116 * 4.5, 104 * 4.5 /*set_row*/, 114 * 4.5, 5 * 4.5, 2 * 4.5 },
        /* 22*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 0, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 142, 116, 104 /*set_row*/, 115, 11, 2 }, // With no scaling
        /* 23*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 2.0, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 142 * 2, 116 * 2, 104 * 2 + 1 /*set_row*/, 115 * 2, 11 * 2, 2 * 2 }, // +1 set_row
        /* 24*/ { BARCODE_MAXICODE, -1, -1, -1, 0, 0, "1234567890", "", 0, 165, 33, 30, 299, 298, 3 /*set_row*/, 7, 10, 9 }, // With no scaling
        /* 25*/ { BARCODE_MAXICODE, -1, -1, -1, 0, 0.1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 165, 33, 30, 60, 65, 0 /*set_row*/, 1, 3, 1 },
        /* 26*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 0, "12345", "", 0, 12, 2, 63, 146, 30, 3 /*set_row*/, 27, 10, 2 }, // With no scaling
        /* 27*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 0.1, "12345", "", 0, 12, 2, 63, 146 * 0.5, 30 * 0.5 - 1, 3 * 0.5 /*set_row*/, 27 * 0.5, 10 * 0.5, 2 * 0.5 }, // -1 height due to yoffset/boffset flooring
        /* 28*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 0.9, "12345", "", 0, 12, 2, 63, 146 * 0.9, 30 * 0.9, 3 * 0.9 + 1 /*set_row*/, 27 * 0.9, 10 * 0.9, 2 * 0.9 + 1 }, // +1's due to interpolation
        /* 29*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 2.3, "12345", "", 0, 12, 2, 63, 146 * 2.3, 30 * 2.3, 3 * 2.3 + 1 /*set_row*/, 27 * 2.3 - 1, 10 * 2.3, 2 * 2.3 + 1 }, // -1/+1's due to interpolation
        /* 30*/ { BARCODE_POSTNET, -1, -1, BARCODE_QUIET_ZONES, 0, 3.1, "12345", "", 0, 12, 2, 63, 146 * 3.1, 30 * 3.1, 3 * 3.1 + 1 /*set_row*/, 27 * 3.1, 10 * 3.1, 2 * 3.2 + 1 }, // +1's due to interpolation
        /* 31*/ { BARCODE_ITF14, -1, 4, BARCODE_BIND, 61.8, 0, "12345", "", 0, 62, 1, 135, 310, 156, 8 /*set_row*/, 132, 20, 2 }, // With no scaling
        /* 32*/ { BARCODE_ITF14, -1, 4, BARCODE_BIND, 61.8, 2, "12345", "", 0, 61.75, 1, 135, 310 * 2, 156 * 2 - 1, 8 * 2 /*set_row*/, 132 * 2 - 1, 20 * 2, 2 * 2 }, // -1's due to height rounding
        /* 33*/ { BARCODE_ITF14, -1, 4, BARCODE_BIND, 61.8, 2.1, "12345", "", 0, 62, 1, 135, 310 * 2.1, 156 * 2.1, 8 * 2.1 /*set_row*/, 132 * 2.1, 20 * 2.1, 2 * 2.1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;

    testStart("test_scale");

    for (i = 0; i < data_size; i++) {
        int row, column;

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
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
        length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Buffer(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Print(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

        assert_nonzero(symbol->bitmap_height >= data[i].expected_set_rows, "i:%d (%d) symbol->bitmap_height %d < expected_set_rows %d\n",
                i, data[i].symbology, symbol->bitmap_height, data[i].expected_set_rows);
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row, "i:%d (%d) expected_set_rows %d <= expected_set_row %d\n",
                i, data[i].symbology, data[i].expected_set_rows, data[i].expected_set_row);
        for (row = data[i].expected_set_row; row < data[i].expected_set_rows; row++) {
            int bits_set = 0;
            for (column = data[i].expected_set_col; column < data[i].expected_set_col + data[i].expected_set_len; column++) {
                if (is_row_column_black(symbol, row, column)) {
                    bits_set++;
                }
            }
            assert_equal(bits_set, data[i].expected_set_len, "i:%d (%d) row %d bits_set %d != expected_set_len %d\n", i, data[i].symbology, row, bits_set, data[i].expected_set_len);
        }
        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_guard_descent(int index, int debug) {

    struct item {
        int symbology;
        float guard_descent;
        char *data;

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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
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
        /* 11*/ { BARCODE_UPCE, -1, "1234567+12", 0, 50, 1, 78, 184, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 134, 2 },
        /* 12*/ { BARCODE_UPCE, 9, "1234567+12", 0, 50, 1, 78, 184, 118, 0 /*set*/, 100 /*set_row*/, 100 + 18, 134, 2 },
        /* 13*/ { BARCODE_UPCA, -1, "12345678901", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 206, 2 },
        /* 14*/ { BARCODE_UPCA, 0, "12345678901", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 /*set_row*/, 100 + 10, 206, 2 },
        /* 15*/ { BARCODE_UPCA, 2.75, "12345678901", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 5, 206, 2 },
        /* 16*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 206, 2 },
        /* 17*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 262, 4 },
        /* 18*/ { BARCODE_UPCA, 10, "12345678901+12", 0, 50, 1, 124, 276, 120, 0 /*set*/, 100 /*set_row*/, 100 + 20, 262, 4 },
        /* 19*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 114, 2 },
        /* 20*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 114, 2 },
        /* 21*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 114, 2 },
        /* 22*/ { BARCODE_EANX, 2, "123456789012", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 4, 114, 2 },
        /* 23*/ { BARCODE_EANX, 2, "123456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 + 4 /*set_row*/, 100 + 16, 114, 2 },
        /* 24*/ { BARCODE_EANX, -1, "123456789012+12345", 0, 50, 1, 149, 330, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 308, 4 },
        /* 25*/ { BARCODE_EANX, -1, "123456789012+12345", 0, 50, 1, 149, 330, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 308, 4 },
        /* 26*/ { BARCODE_EANX, 0, "123456789012+12345", 0, 50, 1, 149, 330, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 308, 4 },
        /* 27*/ { BARCODE_EANX, 18, "123456789012+12345", 0, 50, 1, 149, 330, 136, 1 /*set*/, 100 /*set_row*/, 100 + 36, 308, 4 },
        /* 28*/ { BARCODE_ISBNX, -1, "123456789", 0, 50, 1, 95, 226, 116, 1 /*set*/, 100 /*set_row*/, 100 + 10, 206, 2 },
        /* 29*/ { BARCODE_ISBNX, -1, "123456789", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 + 10 /*set_row*/, 100 + 16, 206, 2 },
        /* 30*/ { BARCODE_ISBNX, 0, "123456789", 0, 50, 1, 95, 226, 116, 0 /*set*/, 100 /*set_row*/, 100 + 16, 206, 2 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_guard_descent");

    for (i = 0; i < data_size; i++) {
        int row, column;

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].guard_descent != -1.0f) {
            symbol->guard_descent = data[i].guard_descent;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Buffer(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Print(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

        assert_nonzero(symbol->bitmap_height >= data[i].expected_set_rows, "i:%d (%d) symbol->bitmap_height %d < expected_set_rows %d\n",
                i, data[i].symbology, symbol->bitmap_height, data[i].expected_set_rows);
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row, "i:%d (%d) expected_set_rows %d < expected_set_row %d\n",
                i, data[i].symbology, data[i].expected_set_rows, data[i].expected_set_row);
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

static void test_quiet_zones(int index, int debug) {

    struct item {
        int symbology;
        int output_options;
        int option_2;
        int show_hrt;
        char *data;

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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, -1, "1234", 0, 50, 1, 62, 124, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*  1*/ { BARCODE_CODE11, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 62, 164, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*  2*/ { BARCODE_CODE11, BARCODE_QUIET_ZONES | BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 62, 124, 116, 1 /*set*/, 0, 100, 0, 2 }, // BARCODE_NO_QUIET_ZONES trumps BARCODE_QUIET_ZONES
        /*  3*/ { BARCODE_C25STANDARD, -1, -1, -1, "1234", 0, 50, 1, 57, 114, 116, 1 /*set*/, 0, 100, 0, 8 },
        /*  4*/ { BARCODE_C25STANDARD, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 57, 154, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*  5*/ { BARCODE_C25INTER, -1, -1, -1, "1234", 0, 50, 1, 45, 90, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*  6*/ { BARCODE_C25INTER, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 45, 130, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*  7*/ { BARCODE_C25IATA, -1, -1, -1, "1234", 0, 50, 1, 65, 130, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*  8*/ { BARCODE_C25IATA, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 65, 170, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*  9*/ { BARCODE_C25LOGIC, -1, -1, -1, "1234", 0, 50, 1, 49, 98, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 10*/ { BARCODE_C25LOGIC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 49, 138, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 11*/ { BARCODE_C25IND, -1, -1, -1, "1234", 0, 50, 1, 75, 150, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 12*/ { BARCODE_C25IND, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 75, 190, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 13*/ { BARCODE_CODE39, -1, -1, -1, "1234", 0, 50, 1, 77, 154, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 14*/ { BARCODE_CODE39, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 77, 194, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 15*/ { BARCODE_EXCODE39, -1, -1, -1, "1234", 0, 50, 1, 77, 154, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 16*/ { BARCODE_EXCODE39, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 77, 194, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 17*/ { BARCODE_EANX, -1, -1, -1, "023456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 110, 212, 14 }, // EAN-13
        /* 18*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "023456789012", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 110, 212, 14 },
        /* 19*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "023456789012", 0, 50, 1, 95, 212, 116, 1 /*set*/, 0, 110, 210, 2 },
        /* 20*/ { BARCODE_EANX, -1, -1, 0, "023456789012", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 212, 14 }, // Hide text
        /* 21*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, 0, "023456789012", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 212, 14 }, // Hide text
        /* 22*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, 0, "023456789012", 0, 50, 1, 95, 190, 110, 1 /*set*/, 0, 110, 188, 2 }, // Hide text
        /* 23*/ { BARCODE_EANX, -1, -1, -1, "023456789012+12", 0, 50, 1, 122, 276, 116, 0 /*set*/, 16, 110, 266, 10 },
        /* 24*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "023456789012+12", 0, 50, 1, 122, 276, 116, 0 /*set*/, 16, 110, 266, 10 },
        /* 25*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "023456789012+12", 0, 50, 1, 122, 266, 116, 1 /*set*/, 16, 110, 262, 4 },
        /* 26*/ { BARCODE_EANX, -1, -1, 0, "023456789012+12", 0, 50, 1, 122, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, // Hide text
        /* 27*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, 0, "023456789012+12", 0, 50, 1, 122, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, // Hide text
        /* 28*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, 0, "023456789012+12", 0, 50, 1, 122, 244, 110, 1 /*set*/, 16, 110, 240, 4 }, // Hide text
        /* 29*/ { BARCODE_EANX_CHK, -1, -1, -1, "0234567890129+12345", 0, 50, 1, 149, 330, 116, 0 /*set*/, 16, 110, 320, 10 },
        /* 30*/ { BARCODE_EANX_CHK, BARCODE_QUIET_ZONES, -1, -1, "0234567890129+12345", 0, 50, 1, 149, 330, 116, 0 /*set*/, 16, 110, 320, 10 },
        /* 31*/ { BARCODE_EANX_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, "0234567890129+12345", 0, 50, 1, 149, 320, 116, 1 /*set*/, 16, 110, 318, 2 },
        /* 32*/ { BARCODE_EANX, -1, -1, -1, "0234567", 0, 50, 1, 67, 162, 116, 0 /*set*/, 0, 100, 0, 14 }, // EAN-8
        /* 33*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "0234567", 0, 50, 1, 67, 162, 116, 0 /*set*/, 0, 100, 0, 14 }, // EAN-8
        /* 34*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "0234567", 0, 50, 1, 67, 134, 116, 1 /*set*/, 0, 100, 0, 2 }, // EAN-8
        /* 35*/ { BARCODE_EANX, -1, -1, -1, "02345", 0, 50, 1, 47, 118, 116, 0 /*set*/, 0, 100, 0, 14 }, // EAN-5
        /* 36*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "02345", 0, 50, 1, 47, 118, 116, 0 /*set*/, 0, 100, 0, 14 }, // EAN-5
        /* 37*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "02345", 0, 50, 1, 47, 94, 116, 1 /*set*/, 0, 100, 0, 2 }, // EAN-5
        /* 38*/ { BARCODE_EANX, -1, -1, -1, "02", 0, 50, 1, 20, 64, 116, 0 /*set*/, 0, 100, 0, 14 }, // EAN-2
        /* 39*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "02", 0, 50, 1, 20, 64, 116, 0 /*set*/, 0, 100, 0, 14 }, // EAN-2
        /* 40*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "02", 0, 50, 1, 20, 40, 116, 1 /*set*/, 0, 100, 0, 2 }, // EAN-2
        /* 41*/ { BARCODE_GS1_128, -1, -1, -1, "[20]02", 0, 50, 1, 68, 136, 116, 1 /*set*/, 0, 100, 0, 4 },
        /* 42*/ { BARCODE_GS1_128, BARCODE_QUIET_ZONES, -1, -1, "[20]02", 0, 50, 1, 68, 176, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 43*/ { BARCODE_CODABAR, -1, -1, -1, "A0B", 0, 50, 1, 32, 64, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 44*/ { BARCODE_CODABAR, BARCODE_QUIET_ZONES, -1, -1, "A0B", 0, 50, 1, 32, 104, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 45*/ { BARCODE_CODE128, -1, -1, -1, "1234", 0, 50, 1, 57, 114, 116, 1 /*set*/, 0, 100, 0, 4 },
        /* 46*/ { BARCODE_CODE128, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 57, 154, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 47*/ { BARCODE_DPLEIT, -1, -1, -1, "1234", 0, 50, 1, 135, 270, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 48*/ { BARCODE_DPLEIT, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 135, 310, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 49*/ { BARCODE_DPIDENT, -1, -1, -1, "1234", 0, 50, 1, 117, 234, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 50*/ { BARCODE_DPIDENT, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 117, 274, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 51*/ { BARCODE_CODE16K, -1, -1, -1, "1234", 0, 20, 2, 70, 162, 44, 0 /*set*/, 2, 20, 0, 20 },
        /* 52*/ { BARCODE_CODE16K, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 70, 162, 44, 0 /*set*/, 2, 20, 0, 20 },
        /* 53*/ { BARCODE_CODE16K, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 70, 140, 44, 1 /*set*/, 2, 20, 0, 6 },
        /* 54*/ { BARCODE_CODE49, -1, -1, -1, "1234", 0, 20, 2, 70, 162, 44, 0 /*set*/, 2, 20, 0, 20 },
        /* 55*/ { BARCODE_CODE49, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 70, 162, 44, 0 /*set*/, 2, 20, 0, 20 },
        /* 56*/ { BARCODE_CODE49, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 70, 140, 44, 1 /*set*/, 2, 20, 0, 2 },
        /* 57*/ { BARCODE_CODE93, -1, -1, -1, "1234", 0, 50, 1, 73, 146, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 58*/ { BARCODE_CODE93, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 73, 186, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 59*/ { BARCODE_FLAT, -1, -1, -1, "1234", 0, 50, 1, 36, 72, 100, 1 /*set*/, 0, 100, 0, 2 },
        /* 60*/ { BARCODE_FLAT, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 36, 72, 100, 1 /*set*/, 0, 100, 0, 2 },
        /* 61*/ { BARCODE_FLAT, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 36, 72, 100, 1 /*set*/, 0, 100, 0, 2 },
        /* 62*/ { BARCODE_DBAR_OMN, -1, -1, -1, "1234", 0, 50, 1, 96, 192, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 63*/ { BARCODE_DBAR_OMN, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 96, 192, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 64*/ { BARCODE_DBAR_OMN, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 96, 192, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 65*/ { BARCODE_DBAR_LTD, -1, -1, -1, "1234", 0, 50, 1, 79, 158, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 66*/ { BARCODE_DBAR_LTD, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 79, 158, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 67*/ { BARCODE_DBAR_LTD, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 79, 158, 116, 0 /*set*/, 0, 100, 0, 2 },
        /* 68*/ { BARCODE_DBAR_EXP, -1, -1, -1, "[20]02", 0, 34, 1, 102, 204, 84, 0 /*set*/, 0, 84, 0, 2 },
        /* 69*/ { BARCODE_DBAR_EXP, BARCODE_QUIET_ZONES, -1, -1, "[20]02", 0, 34, 1, 102, 204, 84, 0 /*set*/, 0, 84, 0, 2 },
        /* 70*/ { BARCODE_DBAR_EXP, BARCODE_NO_QUIET_ZONES, -1, -1, "[20]02", 0, 34, 1, 102, 204, 84, 0 /*set*/, 0, 84, 0, 2 },
        /* 71*/ { BARCODE_TELEPEN, -1, -1, -1, "1234", 0, 50, 1, 112, 224, 116, 1 /*set*/, 0, 100, 0, 2 },
        /* 72*/ { BARCODE_TELEPEN, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 112, 264, 116, 0 /*set*/, 0, 100, 0, 20 },
        /* 73*/ { BARCODE_UPCA, -1, -1, -1, "01457137763", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 100, 0, 18 },
        /* 74*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, "01457137763", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 100, 0, 18 },
        /* 75*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, "01457137763", 0, 50, 1, 95, 226, 116, 0 /*set*/, 0, 100, 0, 18 },
        /* 76*/ { BARCODE_UPCA, -1, -1, 0, "01457137763", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 0, 18 }, // Hide text
        /* 77*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, 0, "01457137763", 0, 50, 1, 95, 226, 110, 0 /*set*/, 0, 110, 0, 18 }, // Hide text
        /* 78*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, 0, "01457137763", 0, 50, 1, 95, 190, 110, 1 /*set*/, 0, 110, 0, 2 }, // Hide text
        /* 79*/ { BARCODE_UPCA, -1, -1, -1, "01457137763+12", 0, 50, 1, 124, 276, 116, 0 /*set*/, 16, 100, 266, 10 },
        /* 80*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, "01457137763+12", 0, 50, 1, 124, 276, 116, 0 /*set*/, 16, 100, 266, 10 },
        /* 81*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, "01457137763+12", 0, 50, 1, 124, 266, 116, 1 /*set*/, 16, 100, 262, 4 },
        /* 82*/ { BARCODE_UPCA, -1, -1, 0, "01457137763+12", 0, 50, 1, 124, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, // Hide text
        /* 83*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, 0, "01457137763+12", 0, 50, 1, 124, 276, 110, 0 /*set*/, 16, 110, 266, 10 }, // Hide text
        /* 84*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, 0, "01457137763+12", 0, 50, 1, 124, 248, 110, 1 /*set*/, 16, 100, 244, 4 }, // Hide text
        /* 85*/ { BARCODE_UPCA_CHK, -1, -1, -1, "014571377638+12345", 0, 50, 1, 151, 330, 116, 0 /*set*/, 16, 100, 320, 10 },
        /* 86*/ { BARCODE_UPCA_CHK, BARCODE_QUIET_ZONES, -1, -1, "014571377638+12345", 0, 50, 1, 151, 330, 116, 0 /*set*/, 16, 100, 320, 10 },
        /* 87*/ { BARCODE_UPCA_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, "014571377638+12345", 0, 50, 1, 151, 320, 116, 1 /*set*/, 16, 100, 318, 2 },
        /* 88*/ { BARCODE_UPCA_CHK, -1, -1, 0, "014571377638+12345", 0, 50, 1, 151, 330, 110, 0 /*set*/, 16, 110, 320, 10 }, // Hide text
        /* 89*/ { BARCODE_UPCA_CHK, BARCODE_QUIET_ZONES, -1, 0, "014571377638+12345", 0, 50, 1, 151, 330, 110, 0 /*set*/, 16, 110, 320, 10 }, // Hide text
        /* 90*/ { BARCODE_UPCA_CHK, BARCODE_NO_QUIET_ZONES, -1, 0, "014571377638+12345", 0, 50, 1, 151, 302, 110, 1 /*set*/, 16, 100, 300, 2 }, // Hide text
        /* 91*/ { BARCODE_UPCE, -1, -1, -1, "8145713", 0, 50, 1, 51, 134, 116, 0 /*set*/, 0, 100, 120, 18 },
        /* 92*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, "8145713", 0, 50, 1, 51, 134, 116, 0 /*set*/, 0, 100, 120, 18 },
        /* 93*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, "8145713", 0, 50, 1, 51, 134, 116, 0 /*set*/, 0, 100, 120, 18 },
        /* 94*/ { BARCODE_UPCE, -1, -1, 0, "8145713", 0, 50, 1, 51, 134, 110, 0 /*set*/, 0, 100, 120, 18 }, // Hide text
        /* 95*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, 0, "8145713", 0, 50, 1, 51, 134, 110, 0 /*set*/, 0, 100, 120, 18 }, // Hide text
        /* 96*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, 0, "8145713", 0, 50, 1, 51, 102, 110, 1 /*set*/, 0, 110, 100, 2 }, // Hide text
        /* 97*/ { BARCODE_UPCE_CHK, -1, -1, -1, "81457132+12", 0, 50, 1, 78, 184, 116, 0 /*set*/, 16, 100, 174, 10 },
        /* 98*/ { BARCODE_UPCE_CHK, BARCODE_QUIET_ZONES, -1, -1, "81457132+12", 0, 50, 1, 78, 184, 116, 0 /*set*/, 16, 100, 174, 10 },
        /* 99*/ { BARCODE_UPCE_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, "81457132+12", 0, 50, 1, 78, 174, 116, 1 /*set*/, 16, 100, 170, 4 },
        /*100*/ { BARCODE_UPCE_CHK, -1, -1, 0, "81457132+12", 0, 50, 1, 78, 184, 110, 0 /*set*/, 16, 110, 174, 10 }, // Hide text
        /*101*/ { BARCODE_UPCE_CHK, BARCODE_QUIET_ZONES, -1, 0, "81457132+12", 0, 50, 1, 78, 184, 110, 0 /*set*/, 16, 110, 174, 10 }, // Hide text
        /*102*/ { BARCODE_UPCE_CHK, BARCODE_NO_QUIET_ZONES, -1, 0, "81457132+12", 0, 50, 1, 78, 156, 110, 1 /*set*/, 16, 100, 152, 4 }, // Hide text
        /*103*/ { BARCODE_UPCE, -1, -1, -1, "8145713+12345", 0, 50, 1, 105, 238, 116, 0 /*set*/, 16, 100, 228, 10 },
        /*104*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, "8145713+12345", 0, 50, 1, 105, 238, 116, 0 /*set*/, 16, 100, 228, 10 },
        /*105*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, "8145713+12345", 0, 50, 1, 105, 228, 116, 1 /*set*/, 16, 100, 216, 2 },
        /*106*/ { BARCODE_UPCE, -1, -1, 0, "8145713+12345", 0, 50, 1, 105, 238, 110, 0 /*set*/, 16, 110, 228, 10 }, // Hide text
        /*107*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, 0, "8145713+12345", 0, 50, 1, 105, 238, 110, 0 /*set*/, 16, 110, 228, 10 }, // Hide text
        /*108*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, 0, "8145713+12345", 0, 50, 1, 105, 210, 110, 1 /*set*/, 16, 100, 208, 2 }, // Hide text
        /*109*/ { BARCODE_POSTNET, -1, -1, -1, "12345", 0, 12, 2, 63, 126, 24, 1 /*set*/, 0, 24, 0, 2 },
        /*110*/ { BARCODE_POSTNET, BARCODE_QUIET_ZONES, -1, -1, "12345", 0, 12, 2, 63, 146, 30, 0 /*set*/, 0, 30, 0, 10 },
        /*111*/ { BARCODE_MSI_PLESSEY, -1, -1, -1, "1234", 0, 50, 1, 55, 110, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*112*/ { BARCODE_MSI_PLESSEY, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 55, 158, 116, 0 /*set*/, 0, 100, 0, 24 },
        /*113*/ { BARCODE_FIM, -1, -1, -1, "A", 0, 50, 1, 17, 34, 100, 1 /*set*/, 0, 100, 0, 2 },
        /*114*/ { BARCODE_FIM, BARCODE_QUIET_ZONES, -1, -1, "A", 0, 50, 1, 17, 50, 100, 0 /*set*/, 0, 100, 0, 10 },
        /*115*/ { BARCODE_LOGMARS, -1, -1, -1, "1234", 0, 50, 1, 95, 190, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*116*/ { BARCODE_LOGMARS, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 95, 230, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*117*/ { BARCODE_PHARMA, -1, -1, -1, "1234", 0, 50, 1, 38, 76, 100, 1 /*set*/, 0, 100, 0, 2 },
        /*118*/ { BARCODE_PHARMA, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 38, 100, 100, 0 /*set*/, 0, 100, 0, 12 },
        /*119*/ { BARCODE_PZN, -1, -1, -1, "1234", 0, 50, 1, 142, 284, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*120*/ { BARCODE_PZN, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 142, 324, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*121*/ { BARCODE_PHARMA_TWO, -1, -1, -1, "1234", 0, 10, 2, 13, 26, 20, 1 /*set*/, 10, 20, 0, 2 },
        /*122*/ { BARCODE_PHARMA_TWO, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 10, 2, 13, 50, 20, 0 /*set*/, 10, 20, 0, 12 },
        /*123*/ { BARCODE_PDF417, -1, -1, -1, "1234", 0, 18, 6, 103, 206, 36, 1 /*set*/, 0, 36, 0, 16 },
        /*124*/ { BARCODE_PDF417, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 18, 6, 103, 214, 44, 0 /*set*/, 0, 44, 0, 4 },
        /*125*/ { BARCODE_PDF417COMP, -1, -1, -1, "1234", 0, 18, 6, 69, 138, 36, 1 /*set*/, 0, 36, 0, 16 },
        /*126*/ { BARCODE_PDF417COMP, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 18, 6, 69, 146, 44, 0 /*set*/, 0, 44, 0, 4 },
        /*127*/ { BARCODE_MAXICODE, -1, -1, -1, "1234", 0, 165, 33, 30, 299, 298, 1 /*set*/, 21, 25, 0, 9 },
        /*128*/ { BARCODE_MAXICODE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 165, 33, 30, 319, 318, 0 /*set*/, 0, 318, 0, 9 },
        /*129*/ { BARCODE_MAXICODE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 165, 33, 30, 319, 318, 0 /*set*/, 0, 9, 0, 319 },
        /*130*/ { BARCODE_QRCODE, -1, -1, -1, "1234", 0, 21, 21, 21, 42, 42, 1 /*set*/, 0, 2, 0, 14 },
        /*131*/ { BARCODE_QRCODE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 21, 21, 21, 58, 58, 0 /*set*/, 0, 8, 0, 58 },
        /*132*/ { BARCODE_CODE128B, -1, -1, -1, "1234", 0, 50, 1, 79, 158, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*133*/ { BARCODE_CODE128B, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 79, 198, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*134*/ { BARCODE_AUSPOST, -1, -1, -1, "12345678", 0, 8, 3, 73, 146, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*135*/ { BARCODE_AUSPOST, BARCODE_QUIET_ZONES, -1, -1, "12345678", 0, 8, 3, 73, 186, 28, 0 /*set*/, 0, 28, 0, 20 },
        /*136*/ { BARCODE_AUSREPLY, -1, -1, -1, "1234", 0, 8, 3, 73, 146, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*137*/ { BARCODE_AUSREPLY, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 73, 186, 28, 0 /*set*/, 0, 28, 0, 20 },
        /*138*/ { BARCODE_AUSROUTE, -1, -1, -1, "1234", 0, 8, 3, 73, 146, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*139*/ { BARCODE_AUSROUTE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 73, 186, 28, 0 /*set*/, 0, 28, 0, 20 },
        /*140*/ { BARCODE_AUSREDIRECT, -1, -1, -1, "1234", 0, 8, 3, 73, 146, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*141*/ { BARCODE_AUSREDIRECT, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 73, 186, 28, 0 /*set*/, 0, 28, 0, 20 },
        /*142*/ { BARCODE_ISBNX, -1, -1, -1, "123456789X", 0, 50, 1, 95, 226, 116, 0 /*set*/, 16, 110, 212, 14 },
        /*143*/ { BARCODE_ISBNX, BARCODE_QUIET_ZONES, -1, -1, "123456789X", 0, 50, 1, 95, 226, 116, 0 /*set*/, 16, 110, 212, 14 },
        /*144*/ { BARCODE_ISBNX, BARCODE_NO_QUIET_ZONES, -1, -1, "123456789X", 0, 50, 1, 95, 212, 116, 1 /*set*/, 16, 110, 210, 2 },
        /*145*/ { BARCODE_RM4SCC, -1, -1, -1, "1234", 0, 8, 3, 43, 86, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*146*/ { BARCODE_RM4SCC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 43, 98, 28, 0 /*set*/, 0, 28, 0, 6 },
        /*147*/ { BARCODE_DATAMATRIX, -1, -1, -1, "1234", 0, 10, 10, 10, 20, 20, 1 /*set*/, 0, 20, 0, 2 },
        /*148*/ { BARCODE_DATAMATRIX, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 10, 10, 10, 24, 24, 0 /*set*/, 0, 24, 0, 2 },
        /*149*/ { BARCODE_EAN14, -1, -1, -1, "1234", 0, 50, 1, 134, 268, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*150*/ { BARCODE_EAN14, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 134, 308, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*151*/ { BARCODE_VIN, -1, -1, -1, "12345678701234567", 0, 50, 1, 246, 492, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*152*/ { BARCODE_VIN, BARCODE_QUIET_ZONES, -1, -1, "12345678701234567", 0, 50, 1, 246, 532, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*153*/ { BARCODE_CODABLOCKF, -1, -1, -1, "1234", 0, 20, 2, 101, 242, 44, 0 /*set*/, 0, 44, 0, 20 },
        /*154*/ { BARCODE_CODABLOCKF, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 101, 242, 44, 0 /*set*/, 0, 44, 0, 20 },
        /*155*/ { BARCODE_CODABLOCKF, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 101, 202, 44, 1 /*set*/, 0, 44, 0, 4 },
        /*156*/ { BARCODE_NVE18, -1, -1, -1, "1234", 0, 50, 1, 156, 312, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*157*/ { BARCODE_NVE18, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 156, 352, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*158*/ { BARCODE_JAPANPOST, -1, -1, -1, "1234", 0, 8, 3, 133, 266, 16, 1 /*set*/, 0, 16, 0, 2 },
        /*159*/ { BARCODE_JAPANPOST, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 133, 278, 28, 0 /*set*/, 0, 28, 0, 6 },
        /*160*/ { BARCODE_KOREAPOST, -1, -1, -1, "1234", 0, 50, 1, 167, 334, 116, 0 /*set*/, 0, 100, 0, 8 },
        /*161*/ { BARCODE_KOREAPOST, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 167, 374, 116, 0 /*set*/, 0, 100, 0, 28 },
        /*162*/ { BARCODE_DBAR_STK, -1, -1, -1, "1234", 0, 13, 3, 50, 100, 26, 1 /*set*/, 12, 26, 0, 2 },
        /*163*/ { BARCODE_DBAR_STK, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 13, 3, 50, 100, 26, 1 /*set*/, 12, 26, 0, 2 },
        /*164*/ { BARCODE_DBAR_STK, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 13, 3, 50, 100, 26, 1 /*set*/, 12, 26, 0, 2 },
        /*165*/ { BARCODE_DBAR_OMNSTK, -1, -1, -1, "1234", 0, 69, 5, 50, 100, 138, 1 /*set*/, 72, 138, 0, 2 },
        /*166*/ { BARCODE_DBAR_OMNSTK, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 69, 5, 50, 100, 138, 1 /*set*/, 72, 138, 0, 2 },
        /*167*/ { BARCODE_DBAR_OMNSTK, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 69, 5, 50, 100, 138, 1 /*set*/, 72, 138, 0, 2 },
        /*168*/ { BARCODE_DBAR_EXPSTK, -1, -1, -1, "[20]12", 0, 34, 1, 102, 204, 68, 1 /*set*/, 0, 68, 2, 2 },
        /*169*/ { BARCODE_DBAR_EXPSTK, BARCODE_QUIET_ZONES, -1, -1, "[20]12", 0, 34, 1, 102, 204, 68, 1 /*set*/, 0, 68, 2, 2 },
        /*170*/ { BARCODE_DBAR_EXPSTK, BARCODE_NO_QUIET_ZONES, -1, -1, "[20]12", 0, 34, 1, 102, 204, 68, 1 /*set*/, 0, 68, 2, 2 },
        /*171*/ { BARCODE_PLANET, -1, -1, -1, "12345678901", 0, 12, 2, 123, 246, 24, 1 /*set*/, 0, 24, 0, 2 },
        /*172*/ { BARCODE_PLANET, BARCODE_QUIET_ZONES, -1, -1, "12345678901", 0, 12, 2, 123, 266, 30, 0 /*set*/, 0, 30, 0, 10 },
        /*173*/ { BARCODE_MICROPDF417, -1, -1, -1, "1234", 0, 22, 11, 38, 76, 44, 1 /*set*/, 0, 44, 0, 4 },
        /*174*/ { BARCODE_MICROPDF417, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 22, 11, 38, 80, 48, 0 /*set*/, 0, 48, 0, 2 },
        /*175*/ { BARCODE_USPS_IMAIL, -1, -1, -1, "12345678901234567890", 0, 8, 3, 129, 258, 16, 1 /*set*/, 0, 10, 0, 2 },
        /*176*/ { BARCODE_USPS_IMAIL, BARCODE_QUIET_ZONES, -1, -1, "12345678901234567890", 0, 8, 3, 129, 276, 20, 0 /*set*/, 0, 20, 0, 9 },
        /*177*/ { BARCODE_PLESSEY, -1, -1, -1, "1234", 0, 50, 1, 131, 262, 116, 1 /*set*/, 0, 100, 0, 6 },
        /*178*/ { BARCODE_PLESSEY, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 131, 310, 116, 0 /*set*/, 0, 100, 0, 24 },
        /*179*/ { BARCODE_TELEPEN_NUM, -1, -1, -1, "1234", 0, 50, 1, 80, 160, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*180*/ { BARCODE_TELEPEN_NUM, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 80, 200, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*181*/ { BARCODE_ITF14, -1, -1, -1, "1234", 0, 50, 1, 135, 330, 136, 0 /*set*/, 10, 110, 10, 20 },
        /*182*/ { BARCODE_ITF14, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 135, 330, 136, 0 /*set*/, 10, 110, 10, 20 },
        /*183*/ { BARCODE_ITF14, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 135, 290, 136, 1 /*set*/, 0, 120, 10, 2 },
        /*184*/ { BARCODE_KIX, -1, -1, -1, "1234", 0, 8, 3, 31, 62, 16, 1 /*set*/, 6, 10, 0, 2 },
        /*185*/ { BARCODE_KIX, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 31, 74, 28, 0 /*set*/, 0, 28, 0, 6 },
        /*186*/ { BARCODE_AZTEC, -1, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 1 /*set*/, 2, 6, 0, 4 },
        /*187*/ { BARCODE_AZTEC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 1 /*set*/, 2, 6, 0, 4 },
        /*188*/ { BARCODE_AZTEC, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 1 /*set*/, 2, 6, 0, 4 },
        /*189*/ { BARCODE_DAFT, -1, -1, -1, "FADT", 0, 8, 3, 7, 14, 16, 1 /*set*/, 0, 16, 0, 2 },
        /*190*/ { BARCODE_DAFT, BARCODE_QUIET_ZONES, -1, -1, "FADT", 0, 8, 3, 7, 14, 16, 1 /*set*/, 0, 16, 0, 2 },
        /*191*/ { BARCODE_DAFT, BARCODE_NO_QUIET_ZONES, -1, -1, "FADT", 0, 8, 3, 7, 14, 16, 1 /*set*/, 0, 16, 0, 2 },
        /*192*/ { BARCODE_DPD, -1, -1, -1, "1234567890123456789012345678", 0, 50, 1, 189, 378, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*193*/ { BARCODE_DPD, BARCODE_QUIET_ZONES, -1, -1, "1234567890123456789012345678", 0, 50, 1, 189, 428, 116, 0 /*set*/, 0, 100, 0, 24 },
        /*194*/ { BARCODE_MICROQR, -1, -1, -1, "1234", 0, 11, 11, 11, 22, 22, 1 /*set*/, 0, 14, 0, 2 },
        /*195*/ { BARCODE_MICROQR, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 11, 11, 11, 30, 30, 0 /*set*/, 0, 30, 0, 4 },
        /*196*/ { BARCODE_HIBC_128, -1, -1, -1, "1234", 0, 50, 1, 90, 180, 116, 1 /*set*/, 0, 100, 0, 4 },
        /*197*/ { BARCODE_HIBC_128, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 90, 220, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*198*/ { BARCODE_HIBC_39, -1, -1, -1, "1234", 0, 50, 1, 127, 254, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*199*/ { BARCODE_HIBC_39, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 127, 294, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*200*/ { BARCODE_HIBC_DM, -1, -1, -1, "1234", 0, 12, 12, 12, 24, 24, 1 /*set*/, 0, 24, 0, 2 },
        /*201*/ { BARCODE_HIBC_DM, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 12, 12, 12, 28, 28, 0 /*set*/, 0, 28, 0, 2 },
        /*202*/ { BARCODE_HIBC_QR, -1, -1, -1, "1234", 0, 21, 21, 21, 42, 42, 1 /*set*/, 0, 2, 0, 14 },
        /*203*/ { BARCODE_HIBC_QR, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 21, 21, 21, 58, 58, 0 /*set*/, 0, 58, 0, 8 },
        /*204*/ { BARCODE_HIBC_PDF, -1, -1, -1, "1234", 0, 21, 7, 103, 206, 42, 1 /*set*/, 0, 42, 0, 16 },
        /*205*/ { BARCODE_HIBC_PDF, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 21, 7, 103, 214, 50, 0 /*set*/, 0, 50, 0, 4 },
        /*206*/ { BARCODE_HIBC_MICPDF, -1, -1, -1, "1234", 0, 12, 6, 82, 164, 24, 1 /*set*/, 0, 24, 0, 4 },
        /*207*/ { BARCODE_HIBC_MICPDF, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 12, 6, 82, 168, 28, 0 /*set*/, 0, 28, 0, 2 },
        /*208*/ { BARCODE_HIBC_BLOCKF, -1, -1, -1, "1234", 0, 20, 2, 101, 242, 44, 0 /*set*/, 0, 44, 0, 20 },
        /*209*/ { BARCODE_HIBC_BLOCKF, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 101, 242, 44, 0 /*set*/, 0, 44, 0, 20 },
        /*210*/ { BARCODE_HIBC_BLOCKF, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 101, 202, 44, 1 /*set*/, 0, 44, 0, 4 },
        /*211*/ { BARCODE_HIBC_AZTEC, -1, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 1 /*set*/, 8, 10, 0, 2 },
        /*212*/ { BARCODE_HIBC_AZTEC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 1 /*set*/, 8, 10, 0, 2 },
        /*213*/ { BARCODE_HIBC_AZTEC, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 1 /*set*/, 8, 10, 0, 2 },
        /*214*/ { BARCODE_DOTCODE, -1, -1, -1, "1234", 0, 10, 10, 13, 27, 21, 1 /*set*/, 5, 6, 1, 1 },
        /*215*/ { BARCODE_DOTCODE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 10, 10, 13, 39, 33, 0 /*set*/, 0, 33, 0, 7 },
        /*216*/ { BARCODE_HANXIN, -1, -1, -1, "1234", 0, 23, 23, 23, 46, 46, 1 /*set*/, 0, 2, 0, 14 },
        /*217*/ { BARCODE_HANXIN, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 23, 23, 23, 58, 58, 0 /*set*/, 0, 58, 0, 6 },
        /*218*/ { BARCODE_MAILMARK, -1, -1, -1, "01000000000000000AA00AA0A", 0, 10, 3, 155, 310, 20, 1 /*set*/, 0, 20, 0, 2 },
        /*219*/ { BARCODE_MAILMARK, BARCODE_QUIET_ZONES, -1, -1, "01000000000000000AA00AA0A", 0, 10, 3, 155, 322, 32, 0 /*set*/, 0, 32, 0, 6 },
        /*220*/ { BARCODE_AZRUNE, -1, -1, -1, "123", 0, 11, 11, 11, 22, 22, 1 /*set*/, 0, 6, 0, 4 },
        /*221*/ { BARCODE_AZRUNE, BARCODE_QUIET_ZONES, -1, -1, "123", 0, 11, 11, 11, 22, 22, 1 /*set*/, 0, 6, 0, 4 },
        /*222*/ { BARCODE_AZRUNE, BARCODE_NO_QUIET_ZONES, -1, -1, "123", 0, 11, 11, 11, 22, 22, 1 /*set*/, 0, 6, 0, 4 },
        /*223*/ { BARCODE_CODE32, -1, -1, -1, "1234", 0, 50, 1, 103, 206, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*224*/ { BARCODE_CODE32, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 103, 246, 116, 0 /*set*/, 0, 100, 0, 20 },
        /*225*/ { BARCODE_EANX_CC, -1, -1, -1, "023456789012", 0, 50, 7, 99, 234, 116, 0 /*set*/, 24, 110, 218, 16 },
        /*226*/ { BARCODE_EANX_CC, BARCODE_QUIET_ZONES, -1, -1, "023456789012", 0, 50, 7, 99, 234, 116, 0 /*set*/, 24, 110, 218, 16 },
        /*227*/ { BARCODE_EANX_CC, BARCODE_NO_QUIET_ZONES, -1, -1, "023456789012", 0, 50, 7, 99, 220, 116, 0 /*set*/, 24, 110, 218, 2 },
        /*228*/ { BARCODE_EANX_CC, -1, -1, 0, "023456789012", 0, 50, 7, 99, 234, 110, 0 /*set*/, 24, 110, 0, 28 }, // Hide text
        /*229*/ { BARCODE_EANX_CC, BARCODE_QUIET_ZONES, -1, 0, "023456789012", 0, 50, 7, 99, 234, 110, 0 /*set*/, 24, 110, 0, 28 }, // Hide text
        /*230*/ { BARCODE_EANX_CC, BARCODE_NO_QUIET_ZONES, -1, 0, "023456789012", 0, 50, 7, 99, 198, 110, 1 /*set*/, 24, 110, 6, 2 }, // Hide text
        /*231*/ { BARCODE_GS1_128_CC, -1, -1, -1, "[20]02", 0, 50, 5, 99, 198, 116, 1 /*set*/, 14, 100, 24, 4 },
        /*232*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, -1, -1, "[20]02", 0, 50, 5, 99, 238, 116, 0 /*set*/, 14, 100, 24, 20 },
        /*233*/ { BARCODE_DBAR_OMN_CC, -1, -1, -1, "1234", 0, 21, 5, 100, 200, 58, 1 /*set*/, 14, 42, 10, 2 },
        /*234*/ { BARCODE_DBAR_OMN_CC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 21, 5, 100, 204, 58, 0 /*set*/, 14, 42, 10, 2 },
        /*235*/ { BARCODE_DBAR_LTD_CC, -1, -1, -1, "1234", 0, 19, 6, 79, 158, 54, 1 /*set*/, 18, 38, 2, 2 },
        /*236*/ { BARCODE_DBAR_LTD_CC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 19, 6, 79, 162, 54, 0 /*set*/, 18, 38, 2, 2 },
        /*237*/ { BARCODE_DBAR_EXP_CC, -1, -1, -1, "[20]12", 0, 41, 5, 102, 204, 98, 1 /*set*/, 14, 82, 2, 2 },
        /*238*/ { BARCODE_DBAR_EXP_CC, BARCODE_QUIET_ZONES, -1, -1, "[20]12", 0, 41, 5, 102, 208, 98, 0 /*set*/, 14, 82, 2, 2 },
        /*239*/ { BARCODE_UPCA_CC, -1, -1, -1, "01457137763", 0, 50, 7, 99, 234, 116, 1 /*set*/, 24, 100, 212, 2 },
        /*240*/ { BARCODE_UPCA_CC, BARCODE_QUIET_ZONES, -1, -1, "01457137763", 0, 50, 7, 99, 234, 116, 1 /*set*/, 24, 100, 212, 2 },
        /*241*/ { BARCODE_UPCA_CC, BARCODE_NO_QUIET_ZONES, -1, -1, "01457137763", 0, 50, 7, 99, 234, 116, 1 /*set*/, 24, 100, 212, 2 },
        /*242*/ { BARCODE_UPCA_CC, -1, -1, 0, "01457137763", 0, 50, 7, 99, 234, 110, 0 /*set*/, 24, 110, 0, 24 }, // Hide text
        /*243*/ { BARCODE_UPCA_CC, BARCODE_QUIET_ZONES, -1, 0, "01457137763", 0, 50, 7, 99, 234, 110, 0 /*set*/, 24, 110, 0, 24 }, // Hide text
        /*244*/ { BARCODE_UPCA_CC, BARCODE_NO_QUIET_ZONES, -1, 0, "01457137763", 0, 50, 7, 99, 198, 110, 1 /*set*/, 24, 110, 6, 2 }, // Hide text
        /*245*/ { BARCODE_UPCE_CC, -1, -1, -1, "8145713", 0, 50, 9, 55, 142, 116, 1 /*set*/, 32, 100, 124, 2 },
        /*246*/ { BARCODE_UPCE_CC, BARCODE_QUIET_ZONES, -1, -1, "8145713", 0, 50, 9, 55, 142, 116, 1 /*set*/, 32, 100, 124, 2 },
        /*247*/ { BARCODE_UPCE_CC, BARCODE_NO_QUIET_ZONES, -1, -1, "8145713", 0, 50, 9, 55, 142, 116, 1 /*set*/, 32, 100, 124, 2 },
        /*248*/ { BARCODE_UPCE_CC, -1, -1, 0, "8145713", 0, 50, 9, 55, 142, 110, 0 /*set*/, 32, 110, 0, 24 }, // Hide text
        /*249*/ { BARCODE_UPCE_CC, BARCODE_QUIET_ZONES, -1, 0, "8145713", 0, 50, 9, 55, 142, 110, 0 /*set*/, 32, 110, 0, 24 }, // Hide text
        /*250*/ { BARCODE_UPCE_CC, BARCODE_NO_QUIET_ZONES, -1, 0, "8145713", 0, 50, 9, 55, 110, 110, 1 /*set*/, 32, 110, 6, 2 }, // Hide text
        /*251*/ { BARCODE_DBAR_STK_CC, -1, -1, -1, "1234", 0, 24, 9, 56, 112, 48, 1 /*set*/, 34, 48, 0, 2 },
        /*252*/ { BARCODE_DBAR_STK_CC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 24, 9, 56, 116, 48, 0 /*set*/, 34, 48, 0, 2 },
        /*253*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, -1, "1234", 0, 80, 11, 56, 112, 160, 1 /*set*/, 94, 160, 0, 2 },
        /*254*/ { BARCODE_DBAR_OMNSTK_CC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 80, 11, 56, 116, 160, 0 /*set*/, 94, 160, 0, 2 },
        /*255*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, -1, "[20]12", 0, 41, 5, 102, 204, 82, 1 /*set*/, 14, 82, 2, 2 },
        /*256*/ { BARCODE_DBAR_EXPSTK_CC, BARCODE_QUIET_ZONES, -1, -1, "[20]12", 0, 41, 5, 102, 208, 82, 0 /*set*/, 14, 82, 2, 2 },
        /*257*/ { BARCODE_CHANNEL, -1, -1, -1, "1234", 0, 50, 1, 27, 54, 116, 1 /*set*/, 0, 100, 0, 2 },
        /*258*/ { BARCODE_CHANNEL, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 27, 60, 116, 0 /*set*/, 0, 100, 0, 2 },
        /*259*/ { BARCODE_CODEONE, -1, -1, -1, "1234", 0, 16, 16, 18, 36, 32, 1 /*set*/, 0, 6, 0, 2 }, // Versions A to H - no quiet zone
        /*260*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 16, 16, 18, 36, 32, 1 /*set*/, 0, 6, 0, 2 },
        /*261*/ { BARCODE_CODEONE, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 16, 16, 18, 36, 32, 1 /*set*/, 0, 6, 0, 2 },
        /*262*/ { BARCODE_CODEONE, -1, 9, -1, "1234", 0, 8, 8, 11, 22, 16, 1 /*set*/, 10, 16, 0, 2 }, // Version S (& T) have quiet zones
        /*263*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, 9, -1, "1234", 0, 8, 8, 11, 26, 16, 0 /*set*/, 0, 16, 0, 2 },
        /*264*/ { BARCODE_GRIDMATRIX, -1, -1, -1, "1234", 0, 18, 18, 18, 36, 36, 1 /*set*/, 0, 2, 0, 12 },
        /*265*/ { BARCODE_GRIDMATRIX, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 18, 18, 18, 60, 60, 0 /*set*/, 0, 60, 0, 12 },
        /*266*/ { BARCODE_UPNQR, -1, -1, -1, "1234", 0, 77, 77, 77, 154, 154, 1 /*set*/, 0, 14, 0, 2 },
        /*267*/ { BARCODE_UPNQR, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 77, 77, 77, 170, 170, 0 /*set*/, 0, 170, 0, 8 },
        /*268*/ { BARCODE_ULTRA, -1, -1, -1, "1234", 0, 13, 13, 15, 30, 26, 1 /*set*/, 0, 2, 0, 30 },
        /*269*/ { BARCODE_ULTRA, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 13, 13, 15, 34, 30, 0 /*set*/, 0, 2, 0, 34 },
        /*270*/ { BARCODE_RMQR, -1, -1, -1, "1234", 0, 11, 11, 27, 54, 22, 1 /*set*/, 0, 14, 0, 2 },
        /*271*/ { BARCODE_RMQR, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 11, 11, 27, 62, 30, 0 /*set*/, 0, 30, 0, 4 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;
    static char composite[] = "[20]12";

    testStart("test_quiet_zones");

    for (i = 0; i < data_size; i++) {
        int row, column;

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, UNICODE_MODE, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }

        if (is_composite(symbol->symbology)) {
            text = composite;
            length = (int) strlen(text);
            assert_nonzero(strlen(data[i].data) < 128, "i:%d linear data length %d >= 128\n", i, (int) strlen(data[i].data));
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Buffer(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        ret = ZBarcode_Print(symbol, 0);
        assert_equal(ret, data[i].ret_raster, "i:%d ZBarcode_Print(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_raster, symbol->errtxt);
        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

        assert_nonzero(symbol->bitmap_height >= data[i].expected_set_rows, "i:%d (%d) symbol->bitmap_height %d < expected_set_rows %d\n",
                i, data[i].symbology, symbol->bitmap_height, data[i].expected_set_rows);
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row, "i:%d (%d) expected_set_rows %d < expected_set_row %d\n",
                i, data[i].symbology, data[i].expected_set_rows, data[i].expected_set_row);
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

static void test_buffer_plot(int index, int generate, int debug) {

    struct item {
        int symbology;
        int option_1;
        int option_2;
        int whitespace_width;
        int output_options;
        float height;
        char *fgcolour;
        char *bgcolour;
        char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        char *expected_bitmap;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, 0, 1, -1, -1, 15, "", "", "1", 0, 15, 5, 86, 86, 15,
                    "11111111010101000111101010111100001110101001110000011101010111000000111111101000101001"
                    "11111111010101000111101010111100001110101001110000011101010111000000111111101000101001"
                    "11111111010101000111101010111100001110101001110000011101010111000000111111101000101001"
                    "11111111010101000111111010101110001011111101001100011111101010111000111111101000101001"
                    "11111111010101000111111010101110001011111101001100011111101010111000111111101000101001"
                    "11111111010101000111111010101110001011111101001100011111101010111000111111101000101001"
                    "11111111010101000110101011111000001111110101101000011101010111111000111111101000101001"
                    "11111111010101000110101011111000001111110101101000011101010111111000111111101000101001"
                    "11111111010101000110101011111000001111110101101000011101010111111000111111101000101001"
                    "11111111010101000101011110011110001110111101100100011010111101111100111111101000101001"
                    "11111111010101000101011110011110001110111101100100011010111101111100111111101000101001"
                    "11111111010101000101011110011110001110111101100100011010111101111100111111101000101001"
                    "11111111010101000111010111001100001110100111000110011101011100110000111111101000101001"
                    "11111111010101000111010111001100001110100111000110011101011100110000111111101000101001"
                    "11111111010101000111010111001100001110100111000110011101011100110000111111101000101001"
                },
        /*  1*/ { BARCODE_PDF417, 0, 1, -1, -1, 15, "FF0000", "00FF0099", "1", 0, 15, 5, 86, 86, 15,
                    "RRRRRRRRGRGRGRGGGRRRRGRGRGRRRRGGGGRRRGRGRGGRRRGGGGGRRRGRGRGRRRGGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRGRGRGRRRRGGGGRRRGRGRGGRRRGGGGGRRRGRGRGRRRGGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRGRGRGRRRRGGGGRRRGRGRGGRRRGGGGGRRRGRGRGRRRGGGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRRRGRGRGRRRGGGRGRRRRRRGRGGRRGGGRRRRRRGRGRGRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRRRGRGRGRRRGGGRGRRRRRRGRGGRRGGGRRRRRRGRGRGRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRRRRGRGRGRRRGGGRGRRRRRRGRGGRRGGGRRRRRRGRGRGRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRGRGRGRRRRRGGGGGRRRRRRGRGRRGRGGGGRRRGRGRGRRRRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRGRGRGRRRRRGGGGGRRRRRRGRGRRGRGGGGRRRGRGRGRRRRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRGRGRGRRRRRGGGGGRRRRRRGRGRRGRGGGGRRRGRGRGRRRRRRGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRGRGRRRRGGRRRRGGGRRRGRRRRGRRGGRGGGRRGRGRRRRGRRRRRGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRGRGRRRRGGRRRRGGGRRRGRRRRGRRGGRGGGRRGRGRRRRGRRRRRGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRGRGRRRRGGRRRRGGGRRRGRRRRGRRGGRGGGRRGRGRRRRGRRRRRGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRGRGRRRGGRRGGGGRRRGRGGRRRGGGRRGGRRRGRGRRRGGRRGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRGRGRRRGGRRGGGGRRRGRGGRRRGGGRRGGRRRGRGRRRGGRRGGGGRRRRRRRGRGGGRGRGGR"
                    "RRRRRRRRGRGRGRGGGRRRGRGRRRGGRRGGGGRRRGRGGRRRGGGRRGGRRRGRGRRRGGRRGGGGRRRRRRRGRGGGRGRGGR"
                },
        /*  2*/ { BARCODE_PDF417, 0, 1, 1, -1, 15, "FFFF0033", "00FF00", "1", 0, 15, 5, 86, 88, 15,
                    "GYYYYYYYYGYGYGYGGGYYYYGYGYGYYYYGGGGYYYGYGYGGYYYGGGGGYYYGYGYGYYYGGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYGYGYGYYYYGGGGYYYGYGYGGYYYGGGGGYYYGYGYGYYYGGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYGYGYGYYYYGGGGYYYGYGYGGYYYGGGGGYYYGYGYGYYYGGGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYYYGYGYGYYYGGGYGYYYYYYGYGGYYGGGYYYYYYGYGYGYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYYYGYGYGYYYGGGYGYYYYYYGYGGYYGGGYYYYYYGYGYGYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYYYYGYGYGYYYGGGYGYYYYYYGYGGYYGGGYYYYYYGYGYGYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYGYGYGYYYYYGGGGGYYYYYYGYGYYGYGGGGYYYGYGYGYYYYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYGYGYGYYYYYGGGGGYYYYYYGYGYYGYGGGGYYYGYGYGYYYYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYGYGYGYYYYYGGGGGYYYYYYGYGYYGYGGGGYYYGYGYGYYYYYYGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYGYGYYYYGGYYYYGGGYYYGYYYYGYYGGYGGGYYGYGYYYYGYYYYYGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYGYGYYYYGGYYYYGGGYYYGYYYYGYYGGYGGGYYGYGYYYYGYYYYYGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYGYGYYYYGGYYYYGGGYYYGYYYYGYYGGYGGGYYGYGYYYYGYYYYYGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYGYGYYYGGYYGGGGYYYGYGGYYYGGGYYGGYYYGYGYYYGGYYGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYGYGYYYGGYYGGGGYYYGYGGYYYGGGYYGGYYYGYGYYYGGYYGGGGYYYYYYYGYGGGYGYGGYG"
                    "GYYYYYYYYGYGYGYGGGYYYGYGYYYGGYYGGGGYYYGYGGYYYGGGYYGGYYYGYGYYYGGYYGGGGYYYYYYYGYGGGYGYGGYG"
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
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    int row, column;
    int fg_len, bg_len;

    testStart("test_buffer_plot");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d\n", i); // ZINT_DEBUG_TEST_PRINT 16

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

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        if (generate) {
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
                // TODO: check alphamap
            } else {
                assert_null(symbol->alphamap, "i:%d ZBarcode_Buffer(%s) alphamap not NULL\n", i, testUtilBarcodeName(data[i].symbology));
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_height(int index, int generate, int debug) {

    struct item {
        int symbology;
        int output_options;
        float height;
        char *data;
        char *composite;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;

        const char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, 1, "1234567890", "", 0, 1, 1, 108, 216, 2, "" },
        /*  1*/ { BARCODE_CODE11, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 108, 216, 2, "TODO: Find doc" },
        /*  2*/ { BARCODE_CODE11, -1, 4, "1234567890", "", 0, 4, 1, 108, 216, 8, "" },
        /*  3*/ { BARCODE_CODE11, -1, 10, "1234567890", "", 0, 10, 1, 108, 216, 20, "" },
        /*  4*/ { BARCODE_C25STANDARD, -1, 1, "1234567890", "", 0, 1, 1, 117, 234, 2, "" },
        /*  5*/ { BARCODE_C25STANDARD, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 117, 234, 2, "No doc for C25 except C25INTER" },
        /*  6*/ { BARCODE_C25STANDARD, -1, 4, "1234567890", "", 0, 4, 1, 117, 234, 8, "" },
        /*  7*/ { BARCODE_C25STANDARD, -1, 11, "1234567890", "", 0, 11, 1, 117, 234, 22, "" },
        /*  8*/ { BARCODE_C25INTER, -1, 1, "1234567890", "", 0, 1, 1, 99, 198, 2, "" },
        /*  9*/ { BARCODE_C25INTER, -1, 15, "1234567890", "", 0, 15, 1, 99, 198, 30, "" },
        /* 10*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 15, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15, 1, 99, 198, 30, "Min height data-length dependent" },
        /* 11*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 15.5, "1234567890", "", 0, 15.5, 1, 99, 198, 31, "" },
        /* 12*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 17.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 17.5, 1, 117, 234, 35, "Min height data-length dependent" },
        /* 13*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 17.75, "12345678901", "", 0, 18, 1, 117, 234, 36, "" },
        /* 14*/ { BARCODE_C25IATA, -1, 1, "1234567890", "", 0, 1, 1, 149, 298, 2, "" },
        /* 15*/ { BARCODE_C25IATA, -1, 4, "1234567890", "", 0, 4, 1, 149, 298, 8, "" },
        /* 16*/ { BARCODE_C25IATA, -1, 30, "1234567890", "", 0, 30, 1, 149, 298, 60, "" },
        /* 17*/ { BARCODE_C25LOGIC, -1, 1, "1234567890", "", 0, 1, 1, 109, 218, 2, "" },
        /* 18*/ { BARCODE_C25LOGIC, -1, 4, "1234567890", "", 0, 4, 1, 109, 218, 8, "" },
        /* 19*/ { BARCODE_C25LOGIC, -1, 41, "1234567890", "", 0, 41, 1, 109, 218, 82, "" },
        /* 20*/ { BARCODE_C25IND, -1, 1, "1234567890", "", 0, 1, 1, 159, 318, 2, "" },
        /* 21*/ { BARCODE_C25IND, -1, 4, "1234567890", "", 0, 4, 1, 159, 318, 8, "" },
        /* 22*/ { BARCODE_C25IND, -1, 21, "1234567890", "", 0, 21, 1, 159, 318, 42, "" },
        /* 23*/ { BARCODE_CODE39, -1, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "" },
        /* 24*/ { BARCODE_CODE39, -1, 4, "1", "", 0, 4, 1, 38, 76, 8, "" },
        /* 25*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 4, "1", "", ZINT_WARN_NONCOMPLIANT, 4, 1, 38, 76, 8, "Min height data-length dependent" },
        /* 26*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 4.4, "1", "", 0, 4.5, 1, 38, 76, 9, "" },
        /* 27*/ { BARCODE_CODE39, -1, 17, "1234567890", "", 0, 17, 1, 155, 310, 34, "" },
        /* 28*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 17, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17, 1, 155, 310, 34, "Min height data-length dependent" },
        /* 29*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 17.85, "1234567890", "", 0, 18, 1, 155, 310, 36, "" },
        /* 30*/ { BARCODE_EXCODE39, -1, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "" },
        /* 31*/ { BARCODE_EXCODE39, -1, 17.8, "1234567890", "", 0, 18, 1, 155, 310, 36, "" },
        /* 32*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, 17.8, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 18, 1, 155, 310, 36, "" },
        /* 33*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, 17.9, "1234567890", "", 0, 18, 1, 155, 310, 36, "" },
        /* 34*/ { BARCODE_EANX, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 35*/ { BARCODE_EANX, -1, 69, "123456789012", "", 0, 69, 1, 95, 226, 148, "" },
        /* 36*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /* 37*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 69.25, "123456789012", "", 0, 69.5, 1, 95, 226, 149, "" },
        /* 38*/ { BARCODE_EANX, -1, 55, "1234567", "", 0, 55, 1, 67, 162, 120, "EAN-8" },
        /* 39*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 55, "1234567", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 67, 162, 120, "EAN-8" },
        /* 40*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 55.25, "1234567", "", 0, 55.5, 1, 67, 162, 121, "EAN-8" },
        /* 41*/ { BARCODE_EANX_CHK, -1, 1, "1234567890128", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 42*/ { BARCODE_EANX_CHK, -1, 69, "1234567890128", "", 0, 69, 1, 95, 226, 148, "" },
        /* 43*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, 69, "1234567890128", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /* 44*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, 69.25, "1234567890128", "", 0, 69.5, 1, 95, 226, 149, "" },
        /* 45*/ { BARCODE_GS1_128, -1, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "" },
        /* 46*/ { BARCODE_GS1_128, -1, 5.7, "[01]12345678901231", "", 0, 5.5, 1, 134, 268, 11, "" },
        /* 47*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, 5.7, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 5.5, 1, 134, 268, 11, "" },
        /* 48*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, 5.725, "[01]12345678901231", "", 0, 5.5, 1, 134, 268, 11, "Note considered compliant even though rendered height same as before" },
        /* 49*/ { BARCODE_GS1_128, -1, 50, "[01]12345678901231", "", 0, 50, 1, 134, 268, 100, "" },
        /* 50*/ { BARCODE_CODABAR, -1, 1, "A0B", "", 0, 1, 1, 32, 64, 2, "" },
        /* 51*/ { BARCODE_CODABAR, -1, 4, "A0B", "", 0, 4, 1, 32, 64, 8, "" },
        /* 52*/ { BARCODE_CODABAR, -1, 26, "A0B", "", 0, 26, 1, 32, 64, 52, "" },
        /* 53*/ { BARCODE_CODABAR, COMPLIANT_HEIGHT, 26, "A0B", "", ZINT_WARN_NONCOMPLIANT, 26, 1, 32, 64, 52, "" },
        /* 54*/ { BARCODE_CODABAR, COMPLIANT_HEIGHT, 27, "A0B", "", 0, 27, 1, 32, 64, 54, "" },
        /* 55*/ { BARCODE_CODE128, -1, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 56*/ { BARCODE_CODE128, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 57*/ { BARCODE_CODE128, -1, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "" },
        /* 58*/ { BARCODE_CODE128, -1, 7, "1234567890", "", 0, 7, 1, 90, 180, 14, "" },
        /* 59*/ { BARCODE_DPLEIT, -1, 1, "1234567890123", "", 0, 1, 1, 135, 270, 2, "" },
        /* 60*/ { BARCODE_DPLEIT, COMPLIANT_HEIGHT, 1, "1234567890123", "", 0, 1, 1, 135, 270, 2, "TODO: Find doc" },
        /* 61*/ { BARCODE_DPLEIT, -1, 4, "1234567890123", "", 0, 4, 1, 135, 270, 8, "" },
        /* 62*/ { BARCODE_DPIDENT, -1, 1, "12345678901", "", 0, 1, 1, 117, 234, 2, "" },
        /* 63*/ { BARCODE_DPIDENT, COMPLIANT_HEIGHT, 1, "12345678901", "", 0, 1, 1, 117, 234, 2, "TODO: Find doc" },
        /* 64*/ { BARCODE_DPIDENT, -1, 4, "12345678901", "", 0, 4, 1, 117, 234, 8, "" },
        /* 65*/ { BARCODE_CODE16K, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "2 rows" },
        /* 66*/ { BARCODE_CODE16K, -1, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "" },
        /* 67*/ { BARCODE_CODE16K, -1, 4, "1234567890", "", 0, 4, 2, 70, 162, 12, "" },
        /* 68*/ { BARCODE_CODE16K, -1, 16.75, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 69*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 16.75, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17, 2, 70, 162, 38, "" },
        /* 70*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 71*/ { BARCODE_CODE16K, -1, -1, "12345678901234567890123456789012345678901234567890", "", 0, 60, 6, 70, 162, 124, "6 rows" },
        /* 72*/ { BARCODE_CODE16K, -1, 52.5, "12345678901234567890123456789012345678901234567890", "", 0, 54, 6, 70, 162, 112, "" },
        /* 73*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 52.5, "12345678901234567890123456789012345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 54, 6, 70, 162, 112, "" },
        /* 74*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 53, "12345678901234567890123456789012345678901234567890", "", 0, 54, 6, 70, 162, 112, "" },
        /* 75*/ { BARCODE_CODE49, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "2 rows" },
        /* 76*/ { BARCODE_CODE49, -1, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "" },
        /* 77*/ { BARCODE_CODE49, -1, 16.75, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 78*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 16.75, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17, 2, 70, 162, 38, "" },
        /* 79*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 80*/ { BARCODE_CODE49, -1, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "3 rows" },
        /* 81*/ { BARCODE_CODE49, -1, 25.75, "12345678901234567890", "", 0, 25.5, 3, 70, 162, 55, "" },
        /* 82*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 25.75, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 25.5, 3, 70, 162, 55, "" },
        /* 83*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 26, "12345678901234567890", "", 0, 25.5, 3, 70, 162, 55, "" },
        /* 84*/ { BARCODE_CODE93, -1, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "" },
        /* 85*/ { BARCODE_CODE93, -1, 9.9, "1", "", 0, 10, 1, 46, 92, 20, "" },
        /* 86*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 9.9, "1", "", ZINT_WARN_NONCOMPLIANT, 10, 1, 46, 92, 20, "Min height data-length dependent" },
        /* 87*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 10, "1", "", 0, 10, 1, 46, 92, 20, "" },
        /* 88*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 22, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 22, 1, 127, 254, 44, "Min height data-length dependent" },
        /* 89*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 22.1, "1234567890", "", 0, 22, 1, 127, 254, 44, "" },
        /* 90*/ { BARCODE_FLAT, -1, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 91*/ { BARCODE_FLAT, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "TODO: Find doc" },
        /* 92*/ { BARCODE_FLAT, -1, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "" },
        /* 93*/ { BARCODE_DBAR_OMN, -1, 1, "1234567890123", "", 0, 1, 1, 96, 192, 2, "" },
        /* 94*/ { BARCODE_DBAR_OMN, -1, 12.9, "1234567890123", "", 0, 13, 1, 96, 192, 26, "" },
        /* 95*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, 12.9, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 13, 1, 96, 192, 26, "" },
        /* 96*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, 13, "1234567890123", "", 0, 13, 1, 96, 192, 26, "" },
        /* 97*/ { BARCODE_DBAR_LTD, -1, 1, "1234567890123", "", 0, 1, 1, 79, 158, 2, "" },
        /* 98*/ { BARCODE_DBAR_LTD, -1, 9, "1234567890123", "", 0, 9, 1, 79, 158, 18, "" },
        /* 99*/ { BARCODE_DBAR_LTD, COMPLIANT_HEIGHT, 9, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 9, 1, 79, 158, 18, "" },
        /*100*/ { BARCODE_DBAR_LTD, COMPLIANT_HEIGHT, 10, "1234567890123", "", 0, 10, 1, 79, 158, 20, "" },
        /*101*/ { BARCODE_DBAR_EXP, -1, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "" },
        /*102*/ { BARCODE_DBAR_EXP, -1, 33, "[01]12345678901231", "", 0, 33, 1, 134, 268, 66, "" },
        /*103*/ { BARCODE_DBAR_EXP, COMPLIANT_HEIGHT, 33, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 33, 1, 134, 268, 66, "" },
        /*104*/ { BARCODE_DBAR_EXP, COMPLIANT_HEIGHT, 34, "[01]12345678901231", "", 0, 34, 1, 134, 268, 68, "" },
        /*105*/ { BARCODE_TELEPEN, -1, 1, "1234567890", "", 0, 1, 1, 208, 416, 2, "" },
        /*106*/ { BARCODE_TELEPEN, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 208, 416, 2, "No known min" },
        /*107*/ { BARCODE_TELEPEN, -1, 4, "1234567890", "", 0, 4, 1, 208, 416, 8, "" },
        /*108*/ { BARCODE_UPCA, -1, 1, "12345678901", "", 0, 1, 1, 95, 226, 12, "" },
        /*109*/ { BARCODE_UPCA, -1, 69, "12345678901", "", 0, 69, 1, 95, 226, 148, "" },
        /*110*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, 69, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /*111*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, 69.25, "12345678901", "", 0, 69.5, 1, 95, 226, 149, "" },
        /*112*/ { BARCODE_UPCA_CHK, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "" },
        /*113*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /*114*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, 69.5, "123456789012", "", 0, 69.5, 1, 95, 226, 149, "" },
        /*115*/ { BARCODE_UPCE, -1, 1, "1234567", "", 0, 1, 1, 51, 134, 12, "" },
        /*116*/ { BARCODE_UPCE, -1, 69, "1234567", "", 0, 69, 1, 51, 134, 148, "" },
        /*117*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, 69, "1234567", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 51, 134, 148, "" },
        /*118*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, 69.25, "1234567", "", 0, 69.5, 1, 51, 134, 149, "" },
        /*119*/ { BARCODE_UPCE_CHK, -1, 1, "12345670", "", 0, 1, 1, 51, 134, 12, "" },
        /*120*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, 69.24, "12345670", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 51, 134, 148, "" },
        /*121*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, 69.25, "12345670", "", 0, 69.5, 1, 51, 134, 149, "" },
        /*122*/ { BARCODE_POSTNET, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "" },
        /*123*/ { BARCODE_POSTNET, -1, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "" },
        /*124*/ { BARCODE_POSTNET, -1, 4.5, "12345678901", "", 0, 5, 2, 123, 246, 10, "" },
        /*125*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 4.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 123, 246, 9, "" },
        /*126*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 4.6, "12345678901", "", 0, 5, 2, 123, 246, 10, "" },
        /*127*/ { BARCODE_POSTNET, -1, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*128*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*129*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 9.1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 9, 2, 123, 246, 18, "" },
        /*130*/ { BARCODE_POSTNET, -1, 20, "12345678901", "", 0, 20, 2, 123, 246, 40, "" },
        /*131*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 20, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 20, 2, 123, 246, 40, "" },
        /*132*/ { BARCODE_MSI_PLESSEY, -1, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "" },
        /*133*/ { BARCODE_MSI_PLESSEY, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "TODO: Find doc" },
        /*134*/ { BARCODE_MSI_PLESSEY, -1, 4, "1234567890", "", 0, 4, 1, 127, 254, 8, "" },
        /*135*/ { BARCODE_FIM, -1, 1, "A", "", 0, 1, 1, 17, 34, 2, "" },
        /*136*/ { BARCODE_FIM, -1, 12.7, "A", "", 0, 12.5, 1, 17, 34, 25, "" },
        /*137*/ { BARCODE_FIM, COMPLIANT_HEIGHT, 12.7, "A", "", ZINT_WARN_NONCOMPLIANT, 12.5, 1, 17, 34, 25, "" },
        /*138*/ { BARCODE_FIM, COMPLIANT_HEIGHT, 12.8, "A", "", 0, 13, 1, 17, 34, 26, "" },
        /*139*/ { BARCODE_LOGMARS, -1, 1, "1234567890", "", 0, 1, 1, 191, 382, 2, "" },
        /*140*/ { BARCODE_LOGMARS, -1, 6, "1234567890", "", 0, 6, 1, 191, 382, 12, "" },
        /*141*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 6, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6, 1, 191, 382, 12, "" },
        /*142*/ { BARCODE_LOGMARS, -1, 6.25, "1234567890", "", 0, 6.5, 1, 191, 382, 13, "" },
        /*143*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 6.25, "1234567890", "", 0, 6.5, 1, 191, 382, 13, "" },
        /*144*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 116, "1234567890", "", 0, 116, 1, 191, 382, 232, "" },
        /*145*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 117, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 117, 1, 191, 382, 234, "" },
        /*146*/ { BARCODE_PHARMA, -1, 1, "123456", "", 0, 1, 1, 58, 116, 2, "" },
        /*147*/ { BARCODE_PHARMA, -1, 15, "123456", "", 0, 15, 1, 58, 116, 30, "" },
        /*148*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, 15, "123456", "", ZINT_WARN_NONCOMPLIANT, 15, 1, 58, 116, 30, "" },
        /*149*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, 16, "123456", "", 0, 16, 1, 58, 116, 32, "" },
        /*150*/ { BARCODE_PZN, -1, 1, "123456", "", 0, 1, 1, 142, 284, 2, "" },
        /*151*/ { BARCODE_PZN, -1, 17.7, "123456", "", 0, 17.5, 1, 142, 284, 35, "" },
        /*152*/ { BARCODE_PZN, COMPLIANT_HEIGHT, 17.7, "123456", "", ZINT_WARN_NONCOMPLIANT, 17.5, 1, 142, 284, 35, "" },
        /*153*/ { BARCODE_PZN, COMPLIANT_HEIGHT, 17.8, "123456", "", 0, 18, 1, 142, 284, 36, "" },
        /*154*/ { BARCODE_PHARMA_TWO, -1, -1, "12345678", "", 0, 10, 2, 29, 58, 20, "" },
        /*155*/ { BARCODE_PHARMA_TWO, -1, 1, "12345678", "", 0, 1, 2, 29, 58, 2, "" },
        /*156*/ { BARCODE_PHARMA_TWO, -1, 3.9, "12345678", "", 0, 4, 2, 29, 58, 8, "" },
        /*157*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 3.9, "12345678", "", ZINT_WARN_NONCOMPLIANT, 4, 2, 29, 58, 8, "" },
        /*158*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 4, "12345678", "", 0, 4, 2, 29, 58, 8, "" },
        /*159*/ { BARCODE_PHARMA_TWO, -1, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "" },
        /*160*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "" },
        /*161*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 15.1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 15, 2, 29, 58, 30, "" },
        /*162*/ { BARCODE_PDF417, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "8 rows" },
        /*163*/ { BARCODE_PDF417, -1, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "" },
        /*164*/ { BARCODE_PDF417, -1, 23, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 24, 8, 120, 240, 48, "" },
        /*165*/ { BARCODE_PDF417, -1, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 24, 8, 120, 240, 48, "" },
        /*166*/ { BARCODE_PDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 36, 12, 120, 240, 72, "12 rows" },
        /*167*/ { BARCODE_PDF417, -1, 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 12, 120, 240, 24, "" },
        /*168*/ { BARCODE_PDF417COMP, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 4.5, 9, 86, 172, 9, "9 rows" },
        /*169*/ { BARCODE_PDF417COMP, -1, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 22.5, 9, 86, 172, 45, "" },
        /*170*/ { BARCODE_MAXICODE, -1, 1, "1234567890", "", 0, 16.5, 33, 30, 299, 298, "Fixed size, symbol->height ignored" },
        /*171*/ { BARCODE_QRCODE, -1, 1, "ABCD", "", 0, 21, 21, 21, 42, 42, "Fixed width-to-height ratio, symbol->height ignored" },
        /*172*/ { BARCODE_CODE128B, -1, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "" },
        /*173*/ { BARCODE_CODE128B, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "" },
        /*174*/ { BARCODE_CODE128B, -1, 4, "1234567890", "", 0, 4, 1, 145, 290, 8, "" },
        /*175*/ { BARCODE_AUSPOST, -1, -1, "12345678901234567890123", "", 0, 8, 3, 133, 266, 16, "" },
        /*176*/ { BARCODE_AUSPOST, -1, 1, "12345678901234567890123", "", 0, 2.5, 3, 133, 266, 5, "" },
        /*177*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 1, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 133, 266, 3, "" },
        /*178*/ { BARCODE_AUSPOST, -1, 6.9, "12345678901234567890123", "", 0, 6.5, 3, 133, 266, 13, "" },
        /*179*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 6.9, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 7, 3, 133, 266, 14, "" },
        /*180*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 7, "12345678901234567890123", "", 0, 7, 3, 133, 266, 14, "" },
        /*181*/ { BARCODE_AUSPOST, -1, 14, "12345678901234567890123", "", 0, 14.5, 3, 133, 266, 29, "" },
        /*182*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 14, "12345678901234567890123", "", 0, 13.5, 3, 133, 266, 27, "" },
        /*183*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 14.1, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 13.5, 3, 133, 266, 27, "" },
        /*184*/ { BARCODE_AUSREPLY, -1, 14, "12345678", "", 0, 14.5, 3, 73, 146, 29, "" },
        /*185*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, 14, "12345678", "", 0, 13.5, 3, 73, 146, 27, "" },
        /*186*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, 14.25, "12345678", "", ZINT_WARN_NONCOMPLIANT, 14.5, 3, 73, 146, 29, "" },
        /*187*/ { BARCODE_AUSROUTE, -1, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "" },
        /*188*/ { BARCODE_AUSROUTE, COMPLIANT_HEIGHT, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "" },
        /*189*/ { BARCODE_AUSREDIRECT, -1, 14, "12345678", "", 0, 14.5, 3, 73, 146, 29, "" },
        /*190*/ { BARCODE_AUSREDIRECT, COMPLIANT_HEIGHT, 14, "12345678", "", 0, 13.5, 3, 73, 146, 27, "" },
        /*191*/ { BARCODE_ISBNX, -1, 1, "123456789", "", 0, 1, 1, 95, 226, 12, "" },
        /*192*/ { BARCODE_ISBNX, -1, 69, "123456789", "", 0, 69, 1, 95, 226, 148, "" },
        /*193*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, 69, "123456789", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /*194*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, 69.5, "123456789", "", 0, 69.5, 1, 95, 226, 149, "" },
        /*195*/ { BARCODE_RM4SCC, -1, -1, "1234567890", "", 0, 8, 3, 91, 182, 16, "" },
        /*196*/ { BARCODE_RM4SCC, -1, 1, "1234567890", "", 0, 2.5, 3, 91, 182, 5, "" },
        /*197*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 91, 182, 3, "" },
        /*198*/ { BARCODE_RM4SCC, -1, 4, "1234567890", "", 0, 4, 3, 91, 182, 8, "" },
        /*199*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4, 3, 91, 182, 8, "" },
        /*200*/ { BARCODE_RM4SCC, -1, 6, "1234567890", "", 0, 6.5, 3, 91, 182, 13, "" },
        /*201*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 6, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 5.5, 3, 91, 182, 11, "" },
        /*202*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 6.5, "1234567890", "", 0, 6.5, 3, 91, 182, 13, "" },
        /*203*/ { BARCODE_RM4SCC, -1, 10.8, "1234567890", "", 0, 10.5, 3, 91, 182, 21, "" },
        /*204*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 10.8, "1234567890", "", 0, 11, 3, 91, 182, 22, "" },
        /*205*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 11, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 91, 182, 22, "" },
        /*206*/ { BARCODE_RM4SCC, -1, 16, "1234567890", "", 0, 16, 3, 91, 182, 32, "" },
        /*207*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 3, 91, 182, 32, "" },
        /*208*/ { BARCODE_DATAMATRIX, -1, 1, "ABCD", "", 0, 12, 12, 12, 24, 24, "Fixed width-to-height ratio, symbol->height ignored" },
        /*209*/ { BARCODE_EAN14, -1, 1, "1234567890123", "", 0, 1, 1, 134, 268, 2, "" },
        /*210*/ { BARCODE_EAN14, -1, 5.7, "1234567890123", "", 0, 5.5, 1, 134, 268, 11, "" },
        /*211*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, 5.7, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 5.5, 1, 134, 268, 11, "" },
        /*212*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, 5.8, "1234567890123", "", 0, 6, 1, 134, 268, 12, "" },
        /*213*/ { BARCODE_VIN, -1, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "" },
        /*214*/ { BARCODE_VIN, COMPLIANT_HEIGHT, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "Doc unlikely" },
        /*215*/ { BARCODE_VIN, -1, 4, "12345678701234567", "", 0, 4, 1, 246, 492, 8, "" },
        /*216*/ { BARCODE_CODABLOCKF, -1, -1, "1234567890", "", 0, 20, 2, 101, 242, 44, "2 rows" },
        /*217*/ { BARCODE_CODABLOCKF, -1, 1, "1234567890", "", 0, 1, 2, 101, 242, 6, "" },
        /*218*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1, 2, 101, 242, 6, "" },
        /*219*/ { BARCODE_CODABLOCKF, -1, 15.5, "1234567890", "", 0, 16, 2, 101, 242, 36, "" },
        /*220*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 15.5, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 2, 101, 242, 36, "" },
        /*221*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 16, "1234567890", "", 0, 16, 2, 101, 242, 36, "" },
        /*222*/ { BARCODE_CODABLOCKF, -1, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 110, 11, 156, 352, 224, "11 rows, 14 cols" },
        /*223*/ { BARCODE_CODABLOCKF, -1, 87, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 88, 11, 156, 352, 180, "" },
        /*224*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 87, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", ZINT_WARN_NONCOMPLIANT, 88, 11, 156, 352, 180, "" },
        /*225*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 88, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 88, 11, 156, 352, 180, "" },
        /*226*/ { BARCODE_CODABLOCKF, -1, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "" },
        /*227*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "" },
        /*228*/ { BARCODE_CODABLOCKF, -1, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 110, 11, 167, 374, 224, "11 rows, 15 cols" },
        /*229*/ { BARCODE_CODABLOCKF, -1, 93, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 93.5, 11, 167, 374, 191, "" },
        /*230*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 93, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", ZINT_WARN_NONCOMPLIANT, 93.5, 11, 167, 374, 191, "" },
        /*231*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 93.5, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 93.5, 11, 167, 374, 191, "" },
        /*232*/ { BARCODE_NVE18, -1, 1, "12345678901234567", "", 0, 1, 1, 156, 312, 2, "" },
        /*233*/ { BARCODE_NVE18, -1, 5.7, "12345678901234567", "", 0, 5.5, 1, 156, 312, 11, "" },
        /*234*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, 5.7, "12345678901234567", "", ZINT_WARN_NONCOMPLIANT, 5.5, 1, 156, 312, 11, "" },
        /*235*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, 5.8, "12345678901234567", "", 0, 6, 1, 156, 312, 12, "" },
        /*236*/ { BARCODE_JAPANPOST, -1, -1, "1234567890", "", 0, 8, 3, 133, 266, 16, "" },
        /*237*/ { BARCODE_JAPANPOST, -1, 1, "1234567890", "", 0, 2.5, 3, 133, 266, 5, "" },
        /*238*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 133, 266, 3, "" },
        /*239*/ { BARCODE_JAPANPOST, -1, 4.8, "1234567890", "", 0, 5, 3, 133, 266, 10, "" },
        /*240*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 4.8, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4.5, 3, 133, 266, 9, "" },
        /*241*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 4.9, "1234567890", "", 0, 4.5, 3, 133, 266, 9, "" },
        /*242*/ { BARCODE_JAPANPOST, -1, 7, "1234567890", "", 0, 7, 3, 133, 266, 14, "" },
        /*243*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 7, "1234567890", "", 0, 7.5, 3, 133, 266, 15, "" },
        /*244*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 7.5, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 7.5, 3, 133, 266, 15, "" },
        /*245*/ { BARCODE_JAPANPOST, -1, 16, "1234567890", "", 0, 16, 3, 133, 266, 32, "" },
        /*246*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16.5, 3, 133, 266, 33, "" },
        /*247*/ { BARCODE_KOREAPOST, -1, 1, "123456", "", 0, 1, 1, 167, 334, 2, "" },
        /*248*/ { BARCODE_KOREAPOST, COMPLIANT_HEIGHT, 1, "123456", "", 0, 1, 1, 167, 334, 2, "TODO: Find doc" },
        /*249*/ { BARCODE_KOREAPOST, -1, 4, "123456", "", 0, 4, 1, 167, 334, 8, "" },
        /*250*/ { BARCODE_DBAR_STK, -1, -1, "1234567890123", "", 0, 13, 3, 50, 100, 26, "" },
        /*251*/ { BARCODE_DBAR_STK, -1, 1, "1234567890123", "", 0, 2, 3, 50, 100, 4, "" },
        /*252*/ { BARCODE_DBAR_STK, -1, 12, "1234567890123", "", 0, 12, 3, 50, 100, 24, "" },
        /*253*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, 12, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 12, 3, 50, 100, 24, "" },
        /*254*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, 13, "1234567890123", "", 0, 13, 3, 50, 100, 26, "" },
        /*255*/ { BARCODE_DBAR_OMNSTK, -1, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /*256*/ { BARCODE_DBAR_OMNSTK, -1, 1, "1234567890123", "", 0, 4, 5, 50, 100, 8, "" },
        /*257*/ { BARCODE_DBAR_OMNSTK, -1, 68, "1234567890123", "", 0, 68, 5, 50, 100, 136, "" },
        /*258*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, 68, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 68, 5, 50, 100, 136, "" },
        /*259*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, 69, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /*260*/ { BARCODE_DBAR_EXPSTK, -1, -1, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "2 rows" },
        /*261*/ { BARCODE_DBAR_EXPSTK, -1, 1, "[01]12345678901231", "", 0, 4, 5, 102, 204, 8, "" },
        /*262*/ { BARCODE_DBAR_EXPSTK, -1, 70, "[01]12345678901231", "", 0, 70, 5, 102, 204, 140, "" },
        /*263*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 70, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 70, 5, 102, 204, 140, "" },
        /*264*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 71, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "" },
        /*265*/ { BARCODE_DBAR_EXPSTK, -1, -1, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "3 rows" },
        /*266*/ { BARCODE_DBAR_EXPSTK, -1, 1, "[01]09501101530003[3920]123456789012345", "", 0, 7.5, 9, 102, 204, 15, "" },
        /*267*/ { BARCODE_DBAR_EXPSTK, -1, 107.5, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "" },
        /*268*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 107.5, "[01]09501101530003[3920]123456789012345", "", ZINT_WARN_NONCOMPLIANT, 108, 9, 102, 204, 216, "" },
        /*269*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 108, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "" },
        /*270*/ { BARCODE_PLANET, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "" },
        /*271*/ { BARCODE_PLANET, -1, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "" },
        /*272*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 1.5, 2, 123, 246, 3, "" },
        /*273*/ { BARCODE_PLANET, -1, 4.5, "12345678901", "", 0, 5, 2, 123, 246, 10, "" },
        /*274*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 4.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 123, 246, 9, "" },
        /*275*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 4.6, "12345678901", "", 0, 5, 2, 123, 246, 10, "" },
        /*276*/ { BARCODE_PLANET, -1, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*277*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*278*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 9.1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 9, 2, 123, 246, 18, "" },
        /*279*/ { BARCODE_PLANET, -1, 24, "12345678901", "", 0, 24, 2, 123, 246, 48, "" },
        /*280*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 24, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 24, 2, 123, 246, 48, "" },
        /*281*/ { BARCODE_MICROPDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 20, 10, 82, 164, 40, "10 rows" },
        /*282*/ { BARCODE_MICROPDF417, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "" },
        /*283*/ { BARCODE_MICROPDF417, -1, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "" },
        /*284*/ { BARCODE_MICROPDF417, -1, 19, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 10, 82, 164, 40, "" },
        /*285*/ { BARCODE_MICROPDF417, -1, 30, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 30, 10, 82, 164, 60, "" },
        /*286*/ { BARCODE_MICROPDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 40, 20, 55, 110, 80, "20 rows" },
        /*287*/ { BARCODE_MICROPDF417, -1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 20, 55, 110, 40, "" },
        /*288*/ { BARCODE_USPS_IMAIL, -1, -1, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, "" },
        /*289*/ { BARCODE_USPS_IMAIL, -1, 1, "12345678901234567890", "", 0, 2.5, 3, 129, 258, 5, "" },
        /*290*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 1, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 129, 258, 3, "" },
        /*291*/ { BARCODE_USPS_IMAIL, -1, 4.8, "12345678901234567890", "", 0, 5, 3, 129, 258, 10, "" },
        /*292*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 4.8, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 4.5, 3, 129, 258, 9, "" },
        /*293*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 4.9, "12345678901234567890", "", 0, 4.5, 3, 129, 258, 9, "" },
        /*294*/ { BARCODE_USPS_IMAIL, -1, 7.7, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, "" },
        /*295*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 7.7, "12345678901234567890", "", 0, 7.5, 3, 129, 258, 15, "" },
        /*296*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 7.8, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 7.5, 3, 129, 258, 15, "" },
        /*297*/ { BARCODE_PLESSEY, -1, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "" },
        /*298*/ { BARCODE_PLESSEY, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "TODO: Find doc" },
        /*299*/ { BARCODE_PLESSEY, -1, 4, "1234567890", "", 0, 4, 1, 227, 454, 8, "" },
        /*300*/ { BARCODE_TELEPEN_NUM, -1, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "" },
        /*301*/ { BARCODE_TELEPEN_NUM, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "No known min" },
        /*302*/ { BARCODE_TELEPEN_NUM, -1, 4, "1234567890", "", 0, 4, 1, 128, 256, 8, "" },
        /*303*/ { BARCODE_ITF14, -1, 1, "1234567890", "", 0, 1, 1, 135, 330, 22, "" },
        /*304*/ { BARCODE_ITF14, -1, 5.7, "1234567890", "", 0, 5.5, 1, 135, 330, 31, "" },
        /*305*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, 5.7, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 5.5, 1, 135, 330, 31, "" },
        /*306*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, 5.8, "1234567890", "", 0, 6, 1, 135, 330, 32, "" },
        /*307*/ { BARCODE_KIX, -1, -1, "1234567890", "", 0, 8, 3, 79, 158, 16, "" },
        /*308*/ { BARCODE_KIX, -1, 1, "1234567890", "", 0, 2.5, 3, 79, 158, 5, "" },
        /*309*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 79, 158, 3, "" },
        /*310*/ { BARCODE_KIX, -1, 6.4, "1234567890", "", 0, 6.5, 3, 79, 158, 13, "" },
        /*311*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 6.4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6.5, 3, 79, 158, 13, "" },
        /*312*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 6.5, "1234567890", "", 0, 6.5, 3, 79, 158, 13, "" },
        /*313*/ { BARCODE_KIX, -1, 10.8, "1234567890", "", 0, 10.5, 3, 79, 158, 21, "" },
        /*314*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 10.8, "1234567890", "", 0, 11, 3, 79, 158, 22, "" },
        /*315*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 10.9, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 79, 158, 22, "" },
        /*316*/ { BARCODE_KIX, -1, 16, "1234567890", "", 0, 16, 3, 79, 158, 32, "" },
        /*317*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 3, 79, 158, 32, "" },
        /*318*/ { BARCODE_AZTEC, -1, 1, "1234567890AB", "", 0, 15, 15, 15, 30, 30, "Fixed width-to-height ratio, symbol->height ignored" },
        /*319*/ { BARCODE_DAFT, -1, -1, "DAFTDAFTDAFTDAFT", "", 0, 8, 3, 31, 62, 16, "" },
        /*320*/ { BARCODE_DAFT, -1, 1, "DAFTDAFTDAFTDAFT", "", 0, 2.5, 3, 31, 62, 5, "" },
        /*321*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, 1, "DAFTDAFTDAFTDAFT", "", 0, 2.5, 3, 31, 62, 5, "" },
        /*322*/ { BARCODE_DAFT, -1, 4, "DAFTDAFTDAFTDAFT", "", 0, 4, 3, 31, 62, 8, "" },
        /*323*/ { BARCODE_DAFT, -1, 6, "DAFTDAFTDAFTDAFT", "", 0, 6.5, 3, 31, 62, 13, "" },
        /*324*/ { BARCODE_DAFT, -1, 12, "DAFTDAFTDAFTDAFT", "", 0, 12, 3, 31, 62, 24, "" },
        /*325*/ { BARCODE_DAFT, -1, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "" },
        /*326*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "" },
        /*327*/ { BARCODE_DPD, -1, 1, "0123456789012345678901234567", "", 0, 1, 1, 189, 378, 2, "" },
        /*328*/ { BARCODE_DPD, -1, 62, "0123456789012345678901234567", "", 0, 62, 1, 189, 378, 124, "" },
        /*329*/ { BARCODE_DPD, COMPLIANT_HEIGHT, 62, "0123456789012345678901234567", "", ZINT_WARN_NONCOMPLIANT, 62, 1, 189, 378, 124, "" },
        /*330*/ { BARCODE_DPD, COMPLIANT_HEIGHT, 62.5, "0123456789012345678901234567", "", 0, 62.5, 1, 189, 378, 125, "" },
        /*331*/ { BARCODE_MICROQR, -1, 1, "12345", "", 0, 11, 11, 11, 22, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*332*/ { BARCODE_HIBC_128, -1, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "" },
        /*333*/ { BARCODE_HIBC_128, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "" },
        /*334*/ { BARCODE_HIBC_128, -1, 4, "1234567890", "", 0, 4, 1, 123, 246, 8, "" },
        /*335*/ { BARCODE_HIBC_39, -1, 1, "1234567890", "", 0, 1, 1, 223, 446, 2, "" },
        /*336*/ { BARCODE_HIBC_39, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 223, 446, 2, "" },
        /*337*/ { BARCODE_HIBC_39, -1, 4, "1234567890", "", 0, 4, 1, 223, 446, 8, "" },
        /*338*/ { BARCODE_HIBC_DM, -1, 1, "ABC", "", 0, 12, 12, 12, 24, 24, "Fixed width-to-height ratio, symbol->height ignored" },
        /*339*/ { BARCODE_HIBC_QR, -1, 1, "1234567890AB", "", 0, 21, 21, 21, 42, 42, "Fixed width-to-height ratio, symbol->height ignored" },
        /*340*/ { BARCODE_HIBC_PDF, -1, -1, "ABCDEF", "", 0, 21, 7, 103, 206, 42, "7 rows" },
        /*341*/ { BARCODE_HIBC_PDF, -1, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*342*/ { BARCODE_HIBC_PDF, -1, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*343*/ { BARCODE_HIBC_PDF, -1, 36, "ABCDEF", "", 0, 35, 7, 103, 206, 70, "" },
        /*344*/ { BARCODE_HIBC_MICPDF, -1, -1, "ABCDEF", "", 0, 12, 6, 82, 164, 24, "6 rows" },
        /*345*/ { BARCODE_HIBC_MICPDF, -1, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "" },
        /*346*/ { BARCODE_HIBC_MICPDF, -1, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "" },
        /*347*/ { BARCODE_HIBC_MICPDF, -1, 47, "ABCDEF", "", 0, 48, 6, 82, 164, 96, "" },
        /*348*/ { BARCODE_HIBC_BLOCKF, -1, -1, "1234567890", "", 0, 30, 3, 101, 242, 64, "3 rows" },
        /*349*/ { BARCODE_HIBC_BLOCKF, -1, 1, "1234567890", "", 0, 1.5, 3, 101, 242, 7, "" },
        /*350*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 101, 242, 7, "" },
        /*351*/ { BARCODE_HIBC_BLOCKF, -1, 4, "1234567890", "", 0, 4.5, 3, 101, 242, 13, "" },
        /*352*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4.5, 3, 101, 242, 13, "" },
        /*353*/ { BARCODE_HIBC_BLOCKF, -1, 23, "1234567890", "", 0, 22.5, 3, 101, 242, 49, "" },
        /*354*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 23, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 22.5, 3, 101, 242, 49, "" },
        /*355*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 24, "1234567890", "", 0, 24, 3, 101, 242, 52, "" },
        /*356*/ { BARCODE_HIBC_BLOCKF, -1, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "" },
        /*357*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "" },
        /*358*/ { BARCODE_HIBC_AZTEC, -1, 1, "1234567890AB", "", 0, 19, 19, 19, 38, 38, "Fixed width-to-height ratio, symbol->height ignored" },
        /*359*/ { BARCODE_DOTCODE, -1, 1, "ABC", "", 0, 11, 11, 16, 33, 23, "Fixed width-to-height ratio, symbol->height ignored" },
        /*360*/ { BARCODE_HANXIN, -1, 1, "1234567890AB", "", 0, 23, 23, 23, 46, 46, "Fixed width-to-height ratio, symbol->height ignored" },
        /*361*/ { BARCODE_MAILMARK, -1, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*362*/ { BARCODE_MAILMARK, -1, 1, "01000000000000000AA00AA0A", "", 0, 2.5, 3, 155, 310, 5, "" },
        /*363*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 1, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 155, 310, 3, "" },
        /*364*/ { BARCODE_MAILMARK, -1, 6.4, "01000000000000000AA00AA0A", "", 0, 6.5, 3, 155, 310, 13, "" },
        /*365*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 6.4, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 6.5, 3, 155, 310, 13, "" },
        /*366*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 6.5, "01000000000000000AA00AA0A", "", 0, 6.5, 3, 155, 310, 13, "" },
        /*367*/ { BARCODE_MAILMARK, -1, 10, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*368*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 10, "01000000000000000AA00AA0A", "", 0, 9.5, 3, 155, 310, 19, "" },
        /*369*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 11, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 155, 310, 22, "" },
        /*370*/ { BARCODE_MAILMARK, -1, 15, "01000000000000000AA00AA0A", "", 0, 15, 3, 155, 310, 30, "" },
        /*371*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 15, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 15, 3, 155, 310, 30, "" },
        /*372*/ { BARCODE_MAILMARK, -1, 20, "01000000000000000AA00AA0A", "", 0, 20, 3, 155, 310, 40, "" },
        /*373*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 20, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 20, 3, 155, 310, 40, "" },
        /*374*/ { BARCODE_AZRUNE, -1, 1, "1", "", 0, 11, 11, 11, 22, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*375*/ { BARCODE_CODE32, -1, 1, "12345678", "", 0, 1, 1, 103, 206, 2, "" },
        /*376*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 103, 206, 2, "" },
        /*377*/ { BARCODE_CODE32, -1, 19, "12345678", "", 0, 19, 1, 103, 206, 38, "" },
        /*378*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 19, "12345678", "", ZINT_WARN_NONCOMPLIANT, 19, 1, 103, 206, 38, "" },
        /*379*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 20, "12345678", "", 0, 20, 1, 103, 206, 40, "" },
        /*380*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 234, 110, "EAN-13, CC-A 3 rows" },
        /*381*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01", 0, 12.5, 7, 99, 234, 35, "" },
        /*382*/ { BARCODE_EANX_CC, -1, 81, "123456789012", "[20]01", 0, 81, 7, 99, 234, 172, "" },
        /*383*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 81, "123456789012", "[20]01", ZINT_WARN_NONCOMPLIANT, 81, 7, 99, 234, 172, "" },
        /*384*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 81.25, "123456789012", "[20]01", 0, 81.5, 7, 99, 234, 173, "" },
        /*385*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 50, 9, 99, 234, 110, "EAN-13, CC-A 5 rows" },
        /*386*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 16.5, 9, 99, 234, 43, "" },
        /*387*/ { BARCODE_EANX_CC, -1, 85, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85, 9, 99, 234, 180, "" },
        /*388*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 85, "123456789012", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 85, 9, 99, 234, 180, "" },
        /*389*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 85.25, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85.5, 9, 99, 234, 181, "" },
        /*390*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 11, 99, 234, 110, "EAN-13, CC-A 7 rows" },
        /*391*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 11, 99, 234, 51, "" },
        /*392*/ { BARCODE_EANX_CC, -1, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89, 11, 99, 234, 188, "" },
        /*393*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 89, 11, 99, 234, 188, "" },
        /*394*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 89.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89.5, 11, 99, 234, 189, "" },
        /*395*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 14, 99, 234, 110, "EAN-13, CC-B 10 rows" },
        /*396*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 14, 99, 234, 63, "" },
        /*397*/ { BARCODE_EANX_CC, -1, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95, 14, 99, 234, 200, "" },
        /*398*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 95, 14, 99, 234, 200, "" },
        /*399*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 95.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95.5, 14, 99, 234, 201, "" },
        /*400*/ { BARCODE_EANX_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234", 0, 50, 10, 72, 172, 110, "EAN-8, CC-A 4 rows" },
        /*401*/ { BARCODE_EANX_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234", 0, 18.5, 10, 72, 172, 47, "" },
        /*402*/ { BARCODE_EANX_CC, -1, 73, "1234567", "[20]01[90]123456789012345678901234", 0, 73, 10, 72, 172, 156, "" },
        /*403*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 73, "1234567", "[20]01[90]123456789012345678901234", ZINT_WARN_NONCOMPLIANT, 73, 10, 72, 172, 156, "" },
        /*404*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 73.25, "1234567", "[20]01[90]123456789012345678901234", 0, 73.5, 10, 72, 172, 157, "" },
        /*405*/ { BARCODE_EANX_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50, 24, 82, 192, 110, "EAN-8, CC-B 15 rows" },
        /*406*/ { BARCODE_EANX_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 46.5, 24, 82, 192, 103, "" },
        /*407*/ { BARCODE_EANX_CC, -1, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101, 24, 82, 192, 212, "" },
        /*408*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 101, 24, 82, 192, 212, "" },
        /*409*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 101.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101.5, 24, 82, 192, 213, "" },
        /*410*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 50, 5, 145, 290, 100, "CC-A 3 rows" },
        /*411*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 145, 290, 15, "" },
        /*412*/ { BARCODE_GS1_128_CC, -1, 12.5, "[01]12345678901231", "[20]01", 0, 12.5, 5, 145, 290, 25, "" },
        /*413*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 12.5, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 12.5, 5, 145, 290, 25, "" },
        /*414*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 12.75, "[01]12345678901231", "[20]01", 0, 13, 5, 145, 290, 26, "" },
        /*415*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 9, 145, 290, 100, "CC-A 7 rows" },
        /*416*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 15.5, 9, 145, 290, 31, "" },
        /*417*/ { BARCODE_GS1_128_CC, -1, 20.7, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 9, 145, 290, 41, "" },
        /*418*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 20.7, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 20.5, 9, 145, 290, 41, "" },
        /*419*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 20.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 21, 9, 145, 290, 42, "" },
        /*420*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 12, 145, 290, 100, "CC-B 10 rows" },
        /*421*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 145, 290, 43, "" },
        /*422*/ { BARCODE_GS1_128_CC, -1, 26.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 12, 145, 290, 53, "" },
        /*423*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 26.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 26.5, 12, 145, 290, 53, "" },
        /*424*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 26.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 27, 12, 145, 290, 54, "" },
        /*425*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "CC-C 30 rows" },
        /*426*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "" },
        /*427*/ { BARCODE_GS1_128_CC, -1, 96.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 96.5, 32, 154, 308, 193, "" },
        /*428*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 96.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 96.5, 32, 154, 308, 193, "" },
        /*429*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 96.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 97, 32, 154, 308, 194, "" },
        /*430*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]1234567890", 0, 21, 5, 100, 200, 42, "CC-A 3 rows" },
        /*431*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]1234567890", 0, 7.5, 5, 100, 200, 15, "" },
        /*432*/ { BARCODE_DBAR_OMN_CC, -1, 19.9, "1234567890123", "[20]01[90]1234567890", 0, 20, 5, 100, 200, 40, "" },
        /*433*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 19.9, "1234567890123", "[20]01[90]1234567890", ZINT_WARN_NONCOMPLIANT, 20, 5, 100, 200, 40, "" },
        /*434*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 20, "1234567890123", "[20]01[90]1234567890", 0, 20, 5, 100, 200, 40, "" },
        /*435*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]12345678901234567890", 0, 23, 6, 100, 200, 46, "CC-A 4 rows" },
        /*436*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]12345678901234567890", 0, 9.5, 6, 100, 200, 19, "" },
        /*437*/ { BARCODE_DBAR_OMN_CC, -1, 21.9, "1234567890123", "[20]01[90]12345678901234567890", 0, 22, 6, 100, 200, 44, "" },
        /*438*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 21.9, "1234567890123", "[20]01[90]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 22, 6, 100, 200, 44, "" },
        /*439*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 22, "1234567890123", "[20]01[90]12345678901234567890", 0, 22, 6, 100, 200, 44, "" },
        /*440*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35, 12, 100, 200, 70, "CC-B 10 rows" },
        /*441*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 100, 200, 43, "" },
        /*442*/ { BARCODE_DBAR_OMN_CC, -1, 33.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 34, 12, 100, 200, 68, "" },
        /*443*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 33.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 34, 12, 100, 200, 68, "" },
        /*444*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 34, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 34, 12, 100, 200, 68, "" },
        /*445*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 39, 14, 100, 200, 78, "CC-B 12 rows" },
        /*446*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 25.5, 14, 100, 200, 51, "" },
        /*447*/ { BARCODE_DBAR_OMN_CC, -1, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 38, 14, 100, 200, 76, "" },
        /*448*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 38, 14, 100, 200, 76, "" },
        /*449*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 38, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 38, 14, 100, 200, 76, "" },
        /*450*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "CC-A 4 rows" },
        /*451*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01", 0, 9.5, 6, 79, 158, 19, "" },
        /*452*/ { BARCODE_DBAR_LTD_CC, -1, 18, "1234567890123", "[20]01", 0, 18, 6, 79, 158, 36, "" },
        /*453*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 18, "1234567890123", "[20]01", ZINT_WARN_NONCOMPLIANT, 18, 6, 79, 158, 36, "" },
        /*454*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 19, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "" },
        /*455*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "CC-A 7 rows" },
        /*456*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 15.5, 9, 79, 158, 31, "" },
        /*457*/ { BARCODE_DBAR_LTD_CC, -1, 24.9, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "" },
        /*458*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 24.9, "1234567890123", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 25, 9, 79, 158, 50, "" },
        /*459*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 25, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "" },
        /*460*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "CC-B 20 rows" },
        /*461*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 41.5, 22, 88, 176, 83, "" },
        /*462*/ { BARCODE_DBAR_LTD_CC, -1, 50.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "" },
        /*463*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 50.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 51, 22, 88, 176, 102, "" },
        /*464*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 51, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "" },
        /*465*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "CC-A 3 rows" },
        /*466*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 134, 268, 15, "" },
        /*467*/ { BARCODE_DBAR_EXP_CC, -1, 40, "[01]12345678901231", "[20]01", 0, 40, 5, 134, 268, 80, "" },
        /*468*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 40, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 40, 5, 134, 268, 80, "" },
        /*469*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 41, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "" },
        /*470*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "CC-A 5 rows" },
        /*471*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 11.5, 7, 134, 268, 23, "" },
        /*472*/ { BARCODE_DBAR_EXP_CC, -1, 44, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 44, 7, 134, 268, 88, "" },
        /*473*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 44, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 44, 7, 134, 268, 88, "" },
        /*474*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 45, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "" },
        /*475*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "CC-B 10 rows" },
        /*476*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 134, 268, 43, "" },
        /*477*/ { BARCODE_DBAR_EXP_CC, -1, 54, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 54, 12, 134, 268, 108, "" },
        /*478*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 54, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 54, 12, 134, 268, 108, "" },
        /*479*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 55, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "" },
        /*480*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01", 0, 50, 7, 99, 234, 110, "CC-A 3 rows" },
        /*481*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01", 0, 12.5, 7, 99, 234, 35, "" },
        /*482*/ { BARCODE_UPCA_CC, -1, 81.24, "12345678901", "[20]01", 0, 81, 7, 99, 234, 172, "" },
        /*483*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 81.24, "12345678901", "[20]01", ZINT_WARN_NONCOMPLIANT, 81, 7, 99, 234, 172, "" },
        /*484*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 81.25, "12345678901", "[20]01", 0, 81.5, 7, 99, 234, 173, "" },
        /*485*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 10, 99, 234, 110, "CC-A 6 rows" },
        /*486*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 18.5, 10, 99, 234, 47, "" },
        /*487*/ { BARCODE_UPCA_CC, -1, 87.24, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 87, 10, 99, 234, 184, "" },
        /*488*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 87.24, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 87, 10, 99, 234, 184, "" },
        /*489*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 87.25, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 87.5, 10, 99, 234, 185, "" },
        /*490*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 50, 16, 99, 234, 110, "CC-B 12 rows" },
        /*491*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 30.5, 16, 99, 234, 71, "" },
        /*492*/ { BARCODE_UPCA_CC, -1, 99, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 99, 16, 99, 234, 208, "" },
        /*493*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 99, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", ZINT_WARN_NONCOMPLIANT, 99, 16, 99, 234, 208, "" },
        /*494*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 99.25, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 99.5, 16, 99, 234, 209, "" },
        /*495*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678", 0, 50, 11, 55, 142, 110, "CC-A 7 rows" },
        /*496*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678", 0, 20.5, 11, 55, 142, 51, "" },
        /*497*/ { BARCODE_UPCE_CC, -1, 89, "1234567", "[20]01[90]123456789012345678", 0, 89, 11, 55, 142, 188, "" },
        /*498*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 89, "1234567", "[20]01[90]123456789012345678", ZINT_WARN_NONCOMPLIANT, 89, 11, 55, 142, 188, "" },
        /*499*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 89.25, "1234567", "[20]01[90]123456789012345678", 0, 89.5, 11, 55, 142, 189, "" },
        /*500*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 16, 55, 142, 110, "CC-A 12 rows" },
        /*501*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 30.5, 16, 55, 142, 71, "" },
        /*502*/ { BARCODE_UPCE_CC, -1, 99, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 99, 16, 55, 142, 208, "" },
        /*503*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 99, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 99, 16, 55, 142, 208, "" },
        /*504*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 99.25, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 99.5, 16, 55, 142, 209, "" },
        /*505*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 21, 55, 142, 110, "CC-B 17 rows" },
        /*506*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 40.5, 21, 55, 142, 91, "" },
        /*507*/ { BARCODE_UPCE_CC, -1, 109, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 109, 21, 55, 142, 228, "" },
        /*508*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 109, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 109, 21, 55, 142, 228, "" },
        /*509*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 109.25, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 109.5, 21, 55, 142, 229, "" },
        /*510*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 142, 115, "CC-B 23 rows" },
        /*511*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 142, 115, "" },
        /*512*/ { BARCODE_UPCE_CC, -1, 121, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 121, 27, 55, 142, 252, "" },
        /*513*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 121, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", ZINT_WARN_NONCOMPLIANT, 121, 27, 55, 142, 252, "" },
        /*514*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 121.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 121.5, 27, 55, 142, 253, "" },
        /*515*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "CC-A 5 rows" },
        /*516*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01", 0, 13, 9, 56, 112, 26, "" },
        /*517*/ { BARCODE_DBAR_STK_CC, -1, 23.9, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "" },
        /*518*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 23.9, "1234567890123", "[20]01", ZINT_WARN_NONCOMPLIANT, 24, 9, 56, 112, 48, "" },
        /*519*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 24, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "" },
        /*520*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "CC-A 12 rows" },
        /*521*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 27, 16, 56, 112, 54, "" },
        /*522*/ { BARCODE_DBAR_STK_CC, -1, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "" },
        /*523*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 38, 16, 56, 112, 76, "" },
        /*524*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 38, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "" },
        /*525*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "CC-B 17 rows" },
        /*526*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 37, 21, 56, 112, 74, "" },
        /*527*/ { BARCODE_DBAR_STK_CC, -1, 47.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "" },
        /*528*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 47.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 48, 21, 56, 112, 96, "" },
        /*529*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 48, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "" },
        /*530*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "CC-A 6 rows" },
        /*531*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]1234567890123456", 0, 17, 12, 56, 112, 34, "" },
        /*532*/ { BARCODE_DBAR_OMNSTK_CC, -1, 81, "1234567890123", "[20]01[90]1234567890123456", 0, 81, 12, 56, 112, 162, "" },
        /*533*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 81, "1234567890123", "[20]01[90]1234567890123456", ZINT_WARN_NONCOMPLIANT, 81, 12, 56, 112, 162, "" },
        /*534*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 82, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "" },
        /*535*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "CC-A 12 rows" },
        /*536*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 29, 18, 56, 112, 58, "" },
        /*537*/ { BARCODE_DBAR_OMNSTK_CC, -1, 93.5, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "" },
        /*538*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 93.5, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", ZINT_WARN_NONCOMPLIANT, 94, 18, 56, 112, 188, "" },
        /*539*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 94, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "" },
        /*540*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "CC-B 17 rows" },
        /*541*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 39, 23, 56, 112, 78, "" },
        /*542*/ { BARCODE_DBAR_OMNSTK_CC, -1, 103, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 103, 23, 56, 112, 206, "" },
        /*543*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 103, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 103, 23, 56, 112, 206, "" },
        /*544*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 104, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "" },
        /*545*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "3 rows, CC-A 3 rows" },
        /*546*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 11, 9, 102, 204, 22, "" },
        /*547*/ { BARCODE_DBAR_EXPSTK_CC, -1, 77, "[01]12345678901231", "[20]01", 0, 77, 9, 102, 204, 154, "" },
        /*548*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 77, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 77, 9, 102, 204, 154, "" },
        /*549*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 78, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "" },
        /*550*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "5 rows, CC-A 3 rows" },
        /*551*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 21.5, 21, 102, 204, 43, "" },
        /*552*/ { BARCODE_DBAR_EXPSTK_CC, -1, 188, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "" },
        /*553*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 188, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", ZINT_WARN_NONCOMPLIANT, 189, 21, 102, 204, 378, "" },
        /*554*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 189, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "" },
        /*555*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "3 rows, CC-A 4 rows" },
        /*556*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 13, 10, 102, 204, 26, "" },
        /*557*/ { BARCODE_DBAR_EXPSTK_CC, -1, 79, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 79, 10, 102, 204, 158, "" },
        /*558*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 79, "[01]12345678901231", "[20]01[90]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 79, 10, 102, 204, 158, "" },
        /*559*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 80, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "" },
        /*560*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "3 rows, CC-B 10 rows" },
        /*561*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 25, 16, 102, 204, 50, "" },
        /*562*/ { BARCODE_DBAR_EXPSTK_CC, -1, 91, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 91, 16, 102, 204, 182, "" },
        /*563*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 91, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 91, 16, 102, 204, 182, "" },
        /*564*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 92, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "" },
        /*565*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "5 rows, CC-B 10 rows" },
        /*566*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35.5, 28, 102, 204, 71, "" },
        /*567*/ { BARCODE_DBAR_EXPSTK_CC, -1, 202, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "" },
        /*568*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 202, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 203, 28, 102, 204, 406, "" },
        /*569*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 203, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "" },
        /*570*/ { BARCODE_CHANNEL, -1, 1, "1", "", 0, 1, 1, 19, 38, 2, "" },
        /*571*/ { BARCODE_CHANNEL, -1, 3.75, "123", "", 0, 4, 1, 23, 46, 8, "" },
        /*572*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, 3.75, "123", "", ZINT_WARN_NONCOMPLIANT, 4, 1, 23, 46, 8, "Min height data-length dependent" },
        /*573*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, 4, "123", "", 0, 4, 1, 23, 46, 8, "" },
        /*574*/ { BARCODE_CODEONE, -1, 1, "12345678901234567890", "", 0, 16, 16, 18, 36, 32, "Fixed height, symbol->height ignored" },
        /*575*/ { BARCODE_GRIDMATRIX, -1, 1, "ABC", "", 0, 18, 18, 18, 36, 36, "Fixed width-to-height ratio, symbol->height ignored" },
        /*576*/ { BARCODE_UPNQR, -1, 1, "1234567890AB", "", 0, 77, 77, 77, 154, 154, "Fixed width-to-height ratio, symbol->height ignored" },
        /*577*/ { BARCODE_ULTRA, -1, 1, "1234567890", "", 0, 13, 13, 18, 36, 26, "Fixed width-to-height ratio, symbol->height ignored" },
        /*578*/ { BARCODE_RMQR, -1, 1, "12345", "", 0, 11, 11, 27, 54, 22, "Fixed width-to-height ratio, symbol->height ignored" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;

    testStart("test_height");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d\n", i);

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
        symbol->show_hrt = 0; // Note: disabling HRT
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %.5g, \"%s\", \"%s\", %s, %.8g, %d, %d, %d, %d, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilOutputOptionsName(data[i].output_options),
                    data[i].height, data[i].data, data[i].composite, testUtilErrorName(data[i].ret),
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

#include <time.h>

#define TEST_PERF_ITER_MILLES   1
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)
#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)

// Not a real test, just performance indicator for scaling
static void test_perf_scale(int index, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int border_width;
        int output_options;
        int option_1;
        int option_2;
        float height;
        float scale;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    struct item data[] = {
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
    int data_size = ARRAY_SIZE(data);
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

        if (index != -1 && i != index) continue;

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
            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
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
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; // Undo

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

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
    if (index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter), TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_options", test_options, 1, 0, 1 },
        { "test_buffer", test_buffer, 1, 1, 1 },
        { "test_upcean_hrt", test_upcean_hrt, 1, 0, 1 },
        { "test_row_separator", test_row_separator, 1, 0, 1 },
        { "test_stacking", test_stacking, 1, 0, 1 },
        { "test_output_options", test_output_options, 1, 0, 1 },
        { "test_draw_string_wrap", test_draw_string_wrap, 1, 0, 1 },
        { "test_code128_utf8", test_code128_utf8, 1, 0, 1 },
        { "test_scale", test_scale, 1, 0, 1 },
        { "test_guard_descent", test_guard_descent, 1, 0, 1 },
        { "test_quiet_zones", test_quiet_zones, 1, 0, 1 },
        { "test_buffer_plot", test_buffer_plot, 1, 1, 1 },
        { "test_height", test_height, 1, 1, 1 },
        { "test_perf_scale", test_perf_scale, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
