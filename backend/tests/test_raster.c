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
        char *data;
        char *composite;

        float expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, "1234567890", "", 50, 1, 108, 216, 116 },
        /*  1*/ { BARCODE_C25STANDARD, "1234567890", "", 50, 1, 117, 234, 116 },
        /*  2*/ { BARCODE_C25INTER, "1234567890", "", 50, 1, 99, 198, 116 },
        /*  3*/ { BARCODE_C25IATA, "1234567890", "", 50, 1, 149, 298, 116 },
        /*  4*/ { BARCODE_C25LOGIC, "1234567890", "", 50, 1, 109, 218, 116 },
        /*  5*/ { BARCODE_C25IND, "1234567890", "", 50, 1, 159, 318, 116 },
        /*  6*/ { BARCODE_CODE39, "1234567890", "", 50, 1, 155, 310, 116 },
        /*  7*/ { BARCODE_EXCODE39, "1234567890", "", 50, 1, 155, 310, 116 },
        /*  8*/ { BARCODE_EANX, "123456789012", "", 50, 1, 95, 226, 116 },
        /*  9*/ { BARCODE_EANX_CHK, "1234567890128", "", 50, 1, 95, 226, 116 },
        /* 10*/ { BARCODE_EANX, "123456789012+12", "", 50, 1, 122, 276, 116 },
        /* 11*/ { BARCODE_EANX_CHK, "1234567890128+12", "", 50, 1, 122, 276, 116 },
        /* 12*/ { BARCODE_EANX, "123456789012+12345", "", 50, 1, 149, 330, 116 },
        /* 13*/ { BARCODE_EANX_CHK, "1234567890128+12345", "", 50, 1, 149, 330, 116 },
        /* 14*/ { BARCODE_EANX, "1234567", "", 50, 1, 67, 162, 116 },
        /* 15*/ { BARCODE_EANX_CHK, "12345670", "", 50, 1, 67, 162, 116 },
        /* 16*/ { BARCODE_EANX, "1234567+12", "", 50, 1, 94, 216, 116 },
        /* 17*/ { BARCODE_EANX_CHK, "12345670+12", "", 50, 1, 94, 216, 116 },
        /* 18*/ { BARCODE_EANX, "1234567+12345", "", 50, 1, 121, 270, 116 },
        /* 19*/ { BARCODE_EANX_CHK, "12345670+12345", "", 50, 1, 121, 270, 116 },
        /* 20*/ { BARCODE_EANX, "1234", "", 50, 1, 47, 118, 116 },
        /* 21*/ { BARCODE_EANX_CHK, "1234", "", 50, 1, 47, 118, 116 },
        /* 22*/ { BARCODE_EANX, "12", "", 50, 1, 20, 64, 116 },
        /* 23*/ { BARCODE_EANX_CHK, "12", "", 50, 1, 20, 64, 116 },
        /* 24*/ { BARCODE_GS1_128, "[01]12345678901231", "", 50, 1, 134, 268, 116 },
        /* 25*/ { BARCODE_CODABAR, "A00000000B", "", 50, 1, 102, 204, 116 },
        /* 26*/ { BARCODE_CODE128, "1234567890", "", 50, 1, 90, 180, 116 },
        /* 27*/ { BARCODE_DPLEIT, "1234567890123", "", 50, 1, 135, 270, 116 },
        /* 28*/ { BARCODE_DPIDENT, "12345678901", "", 50, 1, 117, 234, 116 },
        /* 29*/ { BARCODE_CODE16K, "1234567890", "", 20, 2, 70, 162, 44 },
        /* 30*/ { BARCODE_CODE49, "1234567890", "", 20, 2, 70, 162, 44 },
        /* 31*/ { BARCODE_CODE93, "1234567890", "", 50, 1, 127, 254, 116 },
        /* 32*/ { BARCODE_FLAT, "1234567890", "", 50, 1, 90, 180, 100 },
        /* 33*/ { BARCODE_DBAR_OMN, "1234567890123", "", 50, 1, 96, 192, 116 },
        /* 34*/ { BARCODE_DBAR_LTD, "1234567890123", "", 50, 1, 79, 158, 116 },
        /* 35*/ { BARCODE_DBAR_EXP, "[01]12345678901231", "", 34, 1, 134, 268, 84 },
        /* 36*/ { BARCODE_TELEPEN, "1234567890", "", 50, 1, 208, 416, 116 },
        /* 37*/ { BARCODE_UPCA, "12345678901", "", 50, 1, 95, 226, 116 },
        /* 38*/ { BARCODE_UPCA_CHK, "123456789012", "", 50, 1, 95, 226, 116 },
        /* 39*/ { BARCODE_UPCA, "12345678901+12", "", 50, 1, 124, 276, 116 },
        /* 40*/ { BARCODE_UPCA_CHK, "123456789012+12", "", 50, 1, 124, 276, 116 },
        /* 41*/ { BARCODE_UPCA, "12345678901+12345", "", 50, 1, 151, 330, 116 },
        /* 42*/ { BARCODE_UPCA_CHK, "123456789012+12345", "", 50, 1, 151, 330, 116 },
        /* 43*/ { BARCODE_UPCE, "1234567", "", 50, 1, 51, 134, 116 },
        /* 44*/ { BARCODE_UPCE_CHK, "12345670", "", 50, 1, 51, 134, 116 },
        /* 45*/ { BARCODE_UPCE, "1234567+12", "", 50, 1, 78, 184, 116 },
        /* 46*/ { BARCODE_UPCE_CHK, "12345670+12", "", 50, 1, 78, 184, 116 },
        /* 47*/ { BARCODE_UPCE, "1234567+12345", "", 50, 1, 105, 238, 116 },
        /* 48*/ { BARCODE_UPCE_CHK, "12345670+12345", "", 50, 1, 105, 238, 116 },
        /* 49*/ { BARCODE_POSTNET, "12345678901", "", 12, 2, 123, 246, 24 },
        /* 50*/ { BARCODE_MSI_PLESSEY, "1234567890", "", 50, 1, 127, 254, 116 },
        /* 51*/ { BARCODE_FIM, "A", "", 50, 1, 17, 34, 100 },
        /* 52*/ { BARCODE_LOGMARS, "1234567890", "", 50, 1, 191, 382, 116 },
        /* 53*/ { BARCODE_PHARMA, "123456", "", 50, 1, 58, 116, 100 },
        /* 54*/ { BARCODE_PZN, "123456", "", 50, 1, 142, 284, 116 },
        /* 55*/ { BARCODE_PHARMA_TWO, "12345678", "", 10, 2, 29, 58, 20 },
        /* 56*/ { BARCODE_PDF417, "1234567890", "", 21, 7, 103, 206, 42 },
        /* 57*/ { BARCODE_PDF417COMP, "1234567890", "", 21, 7, 69, 138, 42 },
        /* 58*/ { BARCODE_MAXICODE, "1234567890", "", 165, 33, 30, 299, 298 },
        /* 59*/ { BARCODE_QRCODE, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /* 60*/ { BARCODE_CODE128B, "1234567890", "", 50, 1, 145, 290, 116 },
        /* 61*/ { BARCODE_AUSPOST, "12345678901234567890123", "", 8, 3, 133, 266, 16 },
        /* 62*/ { BARCODE_AUSREPLY, "12345678", "", 8, 3, 73, 146, 16 },
        /* 63*/ { BARCODE_AUSROUTE, "12345678", "", 8, 3, 73, 146, 16 },
        /* 64*/ { BARCODE_AUSREDIRECT, "12345678", "", 8, 3, 73, 146, 16 },
        /* 65*/ { BARCODE_ISBNX, "123456789", "", 50, 1, 95, 226, 116 },
        /* 66*/ { BARCODE_ISBNX, "123456789+12", "", 50, 1, 122, 276, 116 },
        /* 67*/ { BARCODE_ISBNX, "123456789+12345", "", 50, 1, 149, 330, 116 },
        /* 68*/ { BARCODE_RM4SCC, "1234567890", "", 8, 3, 91, 182, 16 },
        /* 69*/ { BARCODE_DATAMATRIX, "ABC", "", 10, 10, 10, 20, 20 },
        /* 70*/ { BARCODE_EAN14, "1234567890123", "", 50, 1, 134, 268, 116 },
        /* 71*/ { BARCODE_VIN, "12345678701234567", "", 50, 1, 246, 492, 116 },
        /* 72*/ { BARCODE_CODABLOCKF, "1234567890", "", 20, 2, 101, 242, 44 },
        /* 73*/ { BARCODE_NVE18, "12345678901234567", "", 50, 1, 156, 312, 116 },
        /* 74*/ { BARCODE_JAPANPOST, "1234567890", "", 8, 3, 133, 266, 16 },
        /* 75*/ { BARCODE_KOREAPOST, "123456", "", 50, 1, 167, 334, 116 },
        /* 76*/ { BARCODE_DBAR_STK, "1234567890123", "", 13, 3, 50, 100, 26 },
        /* 77*/ { BARCODE_DBAR_OMNSTK, "1234567890123", "", 69, 5, 50, 100, 138 },
        /* 78*/ { BARCODE_DBAR_EXPSTK, "[01]12345678901231", "", 71, 5, 102, 204, 142 },
        /* 79*/ { BARCODE_PLANET, "12345678901", "", 12, 2, 123, 246, 24 },
        /* 80*/ { BARCODE_MICROPDF417, "1234567890", "", 12, 6, 82, 164, 24 },
        /* 81*/ { BARCODE_USPS_IMAIL, "12345678901234567890", "", 8, 3, 129, 258, 16 },
        /* 82*/ { BARCODE_PLESSEY, "1234567890", "", 50, 1, 227, 454, 116 },
        /* 83*/ { BARCODE_TELEPEN_NUM, "1234567890", "", 50, 1, 128, 256, 116 },
        /* 84*/ { BARCODE_ITF14, "1234567890", "", 50, 1, 135, 330, 136 },
        /* 85*/ { BARCODE_KIX, "123456ABCDE", "", 8, 3, 87, 174, 16 },
        /* 86*/ { BARCODE_AZTEC, "1234567890AB", "", 15, 15, 15, 30, 30 },
        /* 87*/ { BARCODE_DAFT, "DAFTDAFTDAFTDAFT", "", 8, 3, 31, 62, 16 },
        /* 88*/ { BARCODE_DPD, "0123456789012345678901234567", "", 50, 1, 189, 378, 116 },
        /* 89*/ { BARCODE_MICROQR, "12345", "", 11, 11, 11, 22, 22 },
        /* 90*/ { BARCODE_HIBC_128, "1234567890", "", 50, 1, 123, 246, 116 },
        /* 91*/ { BARCODE_HIBC_39, "1234567890", "", 50, 1, 223, 446, 116 },
        /* 92*/ { BARCODE_HIBC_DM, "ABC", "", 12, 12, 12, 24, 24 },
        /* 93*/ { BARCODE_HIBC_QR, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /* 94*/ { BARCODE_HIBC_PDF, "1234567890", "", 24, 8, 103, 206, 48 },
        /* 95*/ { BARCODE_HIBC_MICPDF, "1234567890", "", 28, 14, 38, 76, 56 },
        /* 96*/ { BARCODE_HIBC_BLOCKF, "1234567890", "", 30, 3, 101, 242, 64 },
        /* 97*/ { BARCODE_HIBC_AZTEC, "1234567890AB", "", 19, 19, 19, 38, 38 },
        /* 98*/ { BARCODE_DOTCODE, "ABC", "", 11, 11, 16, 33, 23 },
        /* 99*/ { BARCODE_HANXIN, "1234567890AB", "", 23, 23, 23, 46, 46 },
        /*100*/ { BARCODE_MAILMARK, "01000000000000000AA00AA0A", "", 10, 3, 155, 310, 20 },
        /*101*/ { BARCODE_AZRUNE, "255", "", 11, 11, 11, 22, 22 },
        /*102*/ { BARCODE_CODE32, "12345678", "", 50, 1, 103, 206, 116 },
        /*103*/ { BARCODE_EANX_CC, "123456789012", "[20]01", 50, 7, 99, 234, 116 },
        /*104*/ { BARCODE_EANX_CC, "123456789012+12", "[20]01", 50, 7, 126, 284, 116 },
        /*105*/ { BARCODE_EANX_CC, "123456789012+12345", "[20]01", 50, 7, 153, 338, 116 },
        /*106*/ { BARCODE_EANX_CC, "1234567", "[20]01", 50, 8, 72, 172, 116 },
        /*107*/ { BARCODE_EANX_CC, "1234567+12", "[20]01", 50, 8, 99, 226, 116 },
        /*108*/ { BARCODE_EANX_CC, "1234567+12345", "[20]01", 50, 8, 126, 280, 116 },
        /*109*/ { BARCODE_GS1_128_CC, "[01]12345678901231", "[20]01", 50, 5, 145, 290, 116 },
        /*110*/ { BARCODE_DBAR_OMN_CC, "1234567890123", "[20]01", 21, 5, 100, 200, 58 },
        /*111*/ { BARCODE_DBAR_LTD_CC, "1234567890123", "[20]01", 19, 6, 79, 158, 54 },
        /*112*/ { BARCODE_DBAR_EXP_CC, "[01]12345678901231", "[20]01", 41, 5, 134, 268, 98 },
        /*113*/ { BARCODE_UPCA_CC, "12345678901", "[20]01", 50, 7, 99, 234, 116 },
        /*114*/ { BARCODE_UPCA_CC, "12345678901+12", "[20]01", 50, 7, 128, 284, 116 },
        /*115*/ { BARCODE_UPCA_CC, "12345678901+12345", "[20]01", 50, 7, 155, 338, 116 },
        /*116*/ { BARCODE_UPCE_CC, "1234567", "[20]01", 50, 9, 55, 142, 116 },
        /*117*/ { BARCODE_UPCE_CC, "1234567+12", "[20]01", 50, 9, 82, 192, 116 },
        /*118*/ { BARCODE_UPCE_CC, "1234567+12345", "[20]01", 50, 9, 109, 246, 116 },
        /*119*/ { BARCODE_DBAR_STK_CC, "1234567890123", "[20]01", 24, 9, 56, 112, 48 },
        /*120*/ { BARCODE_DBAR_OMNSTK_CC, "1234567890123", "[20]01", 80, 11, 56, 112, 160 },
        /*121*/ { BARCODE_DBAR_EXPSTK_CC, "[01]12345678901231", "[20]01", 78, 9, 102, 204, 156 },
        /*122*/ { BARCODE_CHANNEL, "01", "", 50, 1, 19, 38, 116 },
        /*123*/ { BARCODE_CODEONE, "12345678901234567890", "", 16, 16, 18, 36, 32 },
        /*124*/ { BARCODE_GRIDMATRIX, "ABC", "", 18, 18, 18, 36, 36 },
        /*125*/ { BARCODE_UPNQR, "1234567890AB", "", 77, 77, 77, 154, 154 },
        /*126*/ { BARCODE_ULTRA, "1234567890", "", 13, 13, 18, 36, 26 },
        /*127*/ { BARCODE_RMQR, "12345", "", 11, 11, 27, 54, 22 },
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

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->debug |= debug;

        if (strlen(data[i].composite)) {
            text = data[i].composite;
            strcpy(symbol->primary, data[i].data);
        } else {
            text = data[i].data;
        }
        length = (int) strlen(text);

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%s) bitmap NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) testUtilBitmapPrint(symbol, NULL, NULL); // ZINT_DEBUG_TEST_PRINT 16

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", \"%s\", %.8g, %d, %d, %d, %d },\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].data, data[i].composite,
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
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, 0, "1", "", 0, 18, 6, 103, 206, 36, 0, 36, 170, 14 }, // With no scaling
        /*  1*/ { BARCODE_PDF417, -1, -1, -1, 0.6, "1", "", 0, 18, 6, 103, 206 * 0.6, 36 * 0.6, 0 /*set_row*/, 36 * 0.6, 170 * 0.6 + 1, 14 * 0.6 }, // +1 set_col due to some scaling inversion difference
        /*  2*/ { BARCODE_PDF417, -1, -1, -1, 1.2, "1", "", 0, 18, 6, 103, 206 * 1.2, 36 * 1.2, 0 /*set_row*/, 36 * 1.2, 170 * 1.2 + 1, 14 * 1.2 }, // +1 set_col due to some scaling inversion difference
        /*  3*/ { BARCODE_PDF417, -1, -1, -1, 0.5, "1", "", 0, 18, 6, 103, 206 * 0.5, 36 * 0.5, 0 /*set_row*/, 36 * 0.5, 170 * 0.5, 14 * 0.5 },
        /*  4*/ { BARCODE_PDF417, -1, -1, -1, 1.0, "1", "", 0, 18, 6, 103, 206 * 1.0, 36 * 1.0, 0 /*set_row*/, 36 * 1.0, 170 * 1.0, 14 * 1.0 },
        /*  5*/ { BARCODE_PDF417, -1, -1, -1, 1.5, "1", "", 0, 18, 6, 103, 206 * 1.5, 36 * 1.5, 0 /*set_row*/, 36 * 1.5, 170 * 1.5, 14 * 1.5 },
        /*  6*/ { BARCODE_PDF417, -1, -1, -1, 2.0, "1", "", 0, 18, 6, 103, 206 * 2.0, 36 * 2.0, 0 /*set_row*/, 36 * 2.0, 170 * 2.0, 14 * 2.0 },
        /*  7*/ { BARCODE_PDF417, -1, -1, -1, 2.5, "1", "", 0, 18, 6, 103, 206 * 2.5, 36 * 2.5, 0 /*set_row*/, 36 * 2.5, 170 * 2.5, 14 * 2.5 },
        /*  8*/ { BARCODE_PDF417, -1, -1, -1, 3.0, "1", "", 0, 18, 6, 103, 206 * 3.0, 36 * 3.0, 0 /*set_row*/, 36 * 3.0, 170 * 3.0, 14 * 3.0 },
        /*  9*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0, "1", "", 0, 18, 6, 103, 218, 48, 0 /*set_row*/, 48, 176, 14 }, // With no scaling
        /* 10*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 0.6, "1", "", 0, 18, 6, 103, 218 * 0.6, 48 * 0.6, 0 /*set_row*/, 48 * 0.6, 176 * 0.6 + 1, 14 * 0.6 }, // +1 set_col due to some scaling inversion difference
        /* 11*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 1.6, "1", "", 0, 18, 6, 103, 218 * 1.6, 48 * 1.6, 0 /*set_row*/, 48 * 1.6, 176 * 1.6 + 1, 14 * 1.6 }, // +1 set_col due to some scaling inversion difference
        /* 12*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 1.5, "1", "", 0, 18, 6, 103, 218 * 1.5, 48 * 1.5, 0 /*set_row*/, 48 * 1.5, 176 * 1.5, 14 * 1.5 },
        /* 13*/ { BARCODE_PDF417, -1, 3, BARCODE_BOX, 2.5, "1", "", 0, 18, 6, 103, 218 * 2.5, 48 * 2.5, 0 /*set_row*/, 48 * 2.5, 176 * 2.5, 14 * 2.5 },
        /* 14*/ { BARCODE_PDF417, -1, 3, OUT_BUFFER_INTERMEDIATE, 1.3, "1", "", 0, 18, 6, 103, 206 * 1.3, 36 * 1.3, 0 /*set_row*/, 36 * 1.3, 170 * 1.3 + 1, 14 * 1.3 }, // +1 set_col due to some scaling inversion difference
        /* 15*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 0, "123456789012", "", 0, 50, 1, 79, 158, 116, 104 /*set_row*/, 114, 20, 2 }, // With no scaling
        /* 16*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 1.5, "123456789012", "", 0, 50, 1, 79, 158 * 1.5, 116 * 1.5, 104 * 1.5 /*set_row*/, 114 * 1.5, 20 * 1.5, 1 * 1.5 },
        /* 17*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 2.0, "123456789012", "", 0, 50, 1, 79, 158 * 2.0, 116 * 2.0, 104 * 2.0 /*set_row*/, 114 * 2.0, 20 * 2.0, 1 * 2.0 },
        /* 18*/ { BARCODE_DBAR_LTD, -1, -1, BOLD_TEXT, 3.5, "123456789012", "", 0, 50, 1, 79, 158 * 3.5, 116 * 3.5, 104 * 3.5 /*set_row*/, 114 * 3.5, 20 * 3.5, 1 * 3.5 },
        /* 19*/ { BARCODE_UPCA, -1, -1, -1, 0, "12345678904", "", 0, 50, 1, 95, 226, 116, 104 /*set_row*/, 114, 5, 2 }, // With no scaling
        /* 20*/ { BARCODE_UPCA, -1, -1, -1, 2.5, "12345678904", "", 0, 50, 1, 95, 226 * 2.5, 116 * 2.5, 104 * 2.5 /*set_row*/, 114 * 2.5, 5 * 2.5, 2 * 2.5 },
        /* 21*/ { BARCODE_UPCA, -1, -1, -1, 4.5, "12345678904", "", 0, 50, 1, 95, 226 * 4.5, 116 * 4.5, 104 * 4.5 /*set_row*/, 114 * 4.5, 5 * 4.5, 2 * 4.5 },
        /* 22*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 142, 116, 104 /*set_row*/, 115, 11, 2 }, // With no scaling
        /* 23*/ { BARCODE_UPCE_CC, -1, -1, -1, 2.0, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 142 * 2, 116 * 2, 104 * 2 + 1 /*set_row*/, 115 * 2, 11 * 2, 2 * 2 }, // +1 set_row
        /* 24*/ { BARCODE_MAXICODE, -1, -1, -1, 0, "1234567890", "", 0, 165, 33, 30, 299, 298, 3 /*set_row*/, 7, 10, 9 }, // With no scaling
        /* 25*/ { BARCODE_MAXICODE, -1, -1, -1, 0.1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 165, 33, 30, 60, 65, 0 /*set_row*/, 1, 3, 1 },
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
        assert_nonzero(data[i].expected_set_rows > data[i].expected_set_row, "i:%d (%d) expected_set_rows %d < expected_set_row %d\n",
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
        /*  0*/ { BARCODE_CODE11, 1, "1234567890", "", 0, 1, 1, 108, 216, 2, "" },
        /*  1*/ { BARCODE_CODE11, 4, "1234567890", "", 0, 4, 1, 108, 216, 8, "" },
        /*  2*/ { BARCODE_CODE11, 10, "1234567890", "", 0, 10, 1, 108, 216, 20, "" },
        /*  3*/ { BARCODE_C25STANDARD, 1, "1234567890", "", 0, 1, 1, 117, 234, 2, "" },
        /*  4*/ { BARCODE_C25STANDARD, 4, "1234567890", "", 0, 4, 1, 117, 234, 8, "" },
        /*  5*/ { BARCODE_C25STANDARD, 11, "1234567890", "", 0, 11, 1, 117, 234, 22, "" },
        /*  6*/ { BARCODE_C25INTER, 1, "1234567890", "", 0, 1, 1, 99, 198, 2, "" },
        /*  7*/ { BARCODE_C25INTER, 4, "1234567890", "", 0, 4, 1, 99, 198, 8, "" },
        /*  8*/ { BARCODE_C25INTER, 15, "1234567890", "", 0, 15, 1, 99, 198, 30, "" },
        /*  9*/ { BARCODE_C25IATA, 1, "1234567890", "", 0, 1, 1, 149, 298, 2, "" },
        /* 10*/ { BARCODE_C25IATA, 4, "1234567890", "", 0, 4, 1, 149, 298, 8, "" },
        /* 11*/ { BARCODE_C25IATA, 30, "1234567890", "", 0, 30, 1, 149, 298, 60, "" },
        /* 12*/ { BARCODE_C25LOGIC, 1, "1234567890", "", 0, 1, 1, 109, 218, 2, "" },
        /* 13*/ { BARCODE_C25LOGIC, 4, "1234567890", "", 0, 4, 1, 109, 218, 8, "" },
        /* 14*/ { BARCODE_C25LOGIC, 41, "1234567890", "", 0, 41, 1, 109, 218, 82, "" },
        /* 15*/ { BARCODE_C25IND, 1, "1234567890", "", 0, 1, 1, 159, 318, 2, "" },
        /* 16*/ { BARCODE_C25IND, 4, "1234567890", "", 0, 4, 1, 159, 318, 8, "" },
        /* 17*/ { BARCODE_C25IND, 21, "1234567890", "", 0, 21, 1, 159, 318, 42, "" },
        /* 18*/ { BARCODE_CODE39, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "" },
        /* 19*/ { BARCODE_CODE39, 4, "1", "", 0, 4, 1, 38, 76, 8, "" },
        /* 20*/ { BARCODE_CODE39, 17, "1234567890", "", 0, 17, 1, 155, 310, 34, "" },
        /* 21*/ { BARCODE_EXCODE39, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "" },
        /* 22*/ { BARCODE_EXCODE39, 4, "1234567890", "", 0, 4, 1, 155, 310, 8, "" },
        /* 23*/ { BARCODE_EXCODE39, 17, "1234567890", "", 0, 17, 1, 155, 310, 34, "" },
        /* 24*/ { BARCODE_EANX, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 25*/ { BARCODE_EANX, 4, "123456789012", "", 0, 4, 1, 95, 226, 18, "" },
        /* 26*/ { BARCODE_EANX, 69, "123456789012", "", 0, 69, 1, 95, 226, 148, "" },
        /* 27*/ { BARCODE_EANX, 55, "1234567", "", 0, 55, 1, 67, 162, 120, "EAN-8" },
        /* 28*/ { BARCODE_EANX, 66, "12345", "", 0, 66, 1, 47, 118, 132, "EAN-8" },
        /* 29*/ { BARCODE_EANX_CHK, 1, "1234567890128", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 30*/ { BARCODE_EANX_CHK, 69, "1234567890128", "", 0, 69, 1, 95, 226, 148, "" },
        /* 31*/ { BARCODE_GS1_128, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "" },
        /* 32*/ { BARCODE_GS1_128, 4, "[01]12345678901231", "", 0, 4, 1, 134, 268, 8, "" },
        /* 33*/ { BARCODE_GS1_128, 5, "[01]12345678901231", "", 0, 5, 1, 134, 268, 10, "" },
        /* 34*/ { BARCODE_GS1_128, 50, "[01]12345678901231", "", 0, 50, 1, 134, 268, 100, "" },
        /* 35*/ { BARCODE_CODABAR, 1, "A0B", "", 0, 1, 1, 32, 64, 2, "" },
        /* 36*/ { BARCODE_CODABAR, 4, "A0B", "", 0, 4, 1, 32, 64, 8, "" },
        /* 37*/ { BARCODE_CODABAR, 26, "A0B", "", 0, 26, 1, 32, 64, 52, "" },
        /* 38*/ { BARCODE_CODE128, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 39*/ { BARCODE_CODE128, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "" },
        /* 40*/ { BARCODE_CODE128, 7, "1234567890", "", 0, 7, 1, 90, 180, 14, "" },
        /* 41*/ { BARCODE_DPLEIT, 1, "1234567890123", "", 0, 1, 1, 135, 270, 2, "" },
        /* 42*/ { BARCODE_DPLEIT, 4, "1234567890123", "", 0, 4, 1, 135, 270, 8, "" },
        /* 43*/ { BARCODE_DPIDENT, 1, "12345678901", "", 0, 1, 1, 117, 234, 2, "" },
        /* 44*/ { BARCODE_DPIDENT, 4, "12345678901", "", 0, 4, 1, 117, 234, 8, "" },
        /* 45*/ { BARCODE_CODE16K, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "2 rows" },
        /* 46*/ { BARCODE_CODE16K, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "" },
        /* 47*/ { BARCODE_CODE16K, 4, "1234567890", "", 0, 4, 2, 70, 162, 12, "" },
        /* 48*/ { BARCODE_CODE16K, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 49*/ { BARCODE_CODE16K, 40, "1234567890", "", 0, 40, 2, 70, 162, 84, "" },
        /* 50*/ { BARCODE_CODE16K, -1, "12345678901234567890123456789012345678901234567890", "", 0, 60, 6, 70, 162, 124, "6 rows" },
        /* 51*/ { BARCODE_CODE16K, 30, "12345678901234567890123456789012345678901234567890", "", 0, 30, 6, 70, 162, 64, "" },
        /* 52*/ { BARCODE_CODE16K, 120, "12345678901234567890123456789012345678901234567890", "", 0, 120, 6, 70, 162, 244, "" },
        /* 53*/ { BARCODE_CODE49, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "2 rows" },
        /* 54*/ { BARCODE_CODE49, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "" },
        /* 55*/ { BARCODE_CODE49, 4, "1234567890", "", 0, 4, 2, 70, 162, 12, "" },
        /* 56*/ { BARCODE_CODE49, 18, "1234567890", "", 0, 18, 2, 70, 162, 40, "" },
        /* 57*/ { BARCODE_CODE49, 40, "1234567890", "", 0, 40, 2, 70, 162, 84, "" },
        /* 58*/ { BARCODE_CODE49, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "3 rows" },
        /* 59*/ { BARCODE_CODE49, 15, "12345678901234567890", "", 0, 15, 3, 70, 162, 34, "" },
        /* 60*/ { BARCODE_CODE49, 45, "12345678901234567890", "", 0, 45, 3, 70, 162, 94, "" },
        /* 61*/ { BARCODE_CODE93, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "" },
        /* 62*/ { BARCODE_CODE93, 4, "1234567890", "", 0, 4, 1, 127, 254, 8, "" },
        /* 63*/ { BARCODE_CODE93, 26, "1234567890", "", 0, 26, 1, 127, 254, 52, "" },
        /* 64*/ { BARCODE_FLAT, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 65*/ { BARCODE_FLAT, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "" },
        /* 66*/ { BARCODE_DBAR_OMN, 1, "1234567890123", "", 0, 1, 1, 96, 192, 2, "" },
        /* 67*/ { BARCODE_DBAR_OMN, 4, "1234567890123", "", 0, 4, 1, 96, 192, 8, "" },
        /* 68*/ { BARCODE_DBAR_OMN, 12, "1234567890123", "", 0, 12, 1, 96, 192, 24, "" },
        /* 69*/ { BARCODE_DBAR_OMN, 33, "1234567890123", "", 0, 33, 1, 96, 192, 66, "" },
        /* 70*/ { BARCODE_DBAR_LTD, 1, "1234567890123", "", 0, 1, 1, 79, 158, 2, "" },
        /* 71*/ { BARCODE_DBAR_LTD, 4, "1234567890123", "", 0, 4, 1, 79, 158, 8, "" },
        /* 72*/ { BARCODE_DBAR_LTD, 9, "1234567890123", "", 0, 9, 1, 79, 158, 18, "" },
        /* 73*/ { BARCODE_DBAR_LTD, 10, "1234567890123", "", 0, 10, 1, 79, 158, 20, "" },
        /* 74*/ { BARCODE_DBAR_EXP, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "" },
        /* 75*/ { BARCODE_DBAR_EXP, 33, "[01]12345678901231", "", 0, 33, 1, 134, 268, 66, "" },
        /* 76*/ { BARCODE_DBAR_EXP, 34, "[01]12345678901231", "", 0, 34, 1, 134, 268, 68, "" },
        /* 77*/ { BARCODE_TELEPEN, 1, "1234567890", "", 0, 1, 1, 208, 416, 2, "" },
        /* 78*/ { BARCODE_TELEPEN, 4, "1234567890", "", 0, 4, 1, 208, 416, 8, "" },
        /* 79*/ { BARCODE_UPCA, 1, "12345678901", "", 0, 1, 1, 95, 226, 12, "" },
        /* 80*/ { BARCODE_UPCA, 4, "12345678901", "", 0, 4, 1, 95, 226, 18, "" },
        /* 81*/ { BARCODE_UPCA, 69, "12345678901", "", 0, 69, 1, 95, 226, 148, "" },
        /* 82*/ { BARCODE_UPCA_CHK, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "" },
        /* 83*/ { BARCODE_UPCE, 1, "1234567", "", 0, 1, 1, 51, 134, 12, "" },
        /* 84*/ { BARCODE_UPCE, 4, "1234567", "", 0, 4, 1, 51, 134, 18, "" },
        /* 85*/ { BARCODE_UPCE, 69, "1234567", "", 0, 69, 1, 51, 134, 148, "" },
        /* 86*/ { BARCODE_UPCE_CHK, 1, "12345670", "", 0, 1, 1, 51, 134, 12, "" },
        /* 87*/ { BARCODE_POSTNET, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "" },
        /* 88*/ { BARCODE_POSTNET, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "" },
        /* 89*/ { BARCODE_POSTNET, 4, "12345678901", "", 0, 4, 2, 123, 246, 8, "" },
        /* 90*/ { BARCODE_POSTNET, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /* 91*/ { BARCODE_POSTNET, 20, "12345678901", "", 0, 20, 2, 123, 246, 40, "" },
        /* 92*/ { BARCODE_MSI_PLESSEY, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "" },
        /* 93*/ { BARCODE_MSI_PLESSEY, 4, "1234567890", "", 0, 4, 1, 127, 254, 8, "" },
        /* 94*/ { BARCODE_FIM, 1, "A", "", 0, 1, 1, 17, 34, 2, "" },
        /* 95*/ { BARCODE_FIM, 4, "A", "", 0, 4, 1, 17, 34, 8, "" },
        /* 96*/ { BARCODE_FIM, 12, "A", "", 0, 12, 1, 17, 34, 24, "" },
        /* 97*/ { BARCODE_LOGMARS, 1, "1234567890", "", 0, 1, 1, 191, 382, 2, "" },
        /* 98*/ { BARCODE_LOGMARS, 4, "1234567890", "", 0, 4, 1, 191, 382, 8, "" },
        /* 99*/ { BARCODE_LOGMARS, 6, "1234567890", "", 0, 6, 1, 191, 382, 12, "" },
        /*100*/ { BARCODE_LOGMARS, 45, "1234567890", "", 0, 45, 1, 191, 382, 90, "" },
        /*101*/ { BARCODE_PHARMA, 1, "123456", "", 0, 1, 1, 58, 116, 2, "" },
        /*102*/ { BARCODE_PHARMA, 4, "123456", "", 0, 4, 1, 58, 116, 8, "" },
        /*103*/ { BARCODE_PHARMA, 15, "123456", "", 0, 15, 1, 58, 116, 30, "" },
        /*104*/ { BARCODE_PHARMA, 16, "123456", "", 0, 16, 1, 58, 116, 32, "" },
        /*105*/ { BARCODE_PZN, 1, "123456", "", 0, 1, 1, 142, 284, 2, "" },
        /*106*/ { BARCODE_PZN, 4, "123456", "", 0, 4, 1, 142, 284, 8, "" },
        /*107*/ { BARCODE_PZN, 17, "123456", "", 0, 17, 1, 142, 284, 34, "" },
        /*108*/ { BARCODE_PZN, 106, "123456", "", 0, 106, 1, 142, 284, 212, "" },
        /*109*/ { BARCODE_PHARMA_TWO, -1, "12345678", "", 0, 10, 2, 29, 58, 20, "" },
        /*110*/ { BARCODE_PHARMA_TWO, 1, "12345678", "", 0, 1, 2, 29, 58, 2, "" },
        /*111*/ { BARCODE_PHARMA_TWO, 4, "12345678", "", 0, 4, 2, 29, 58, 8, "" },
        /*112*/ { BARCODE_PHARMA_TWO, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "" },
        /*113*/ { BARCODE_PHARMA_TWO, 20, "12345678", "", 0, 20, 2, 29, 58, 40, "" },
        /*114*/ { BARCODE_PHARMA_TWO, 30, "12345678", "", 0, 30, 2, 29, 58, 60, "" },
        /*115*/ { BARCODE_PDF417, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "8 rows" },
        /*116*/ { BARCODE_PDF417, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "" },
        /*117*/ { BARCODE_PDF417, 23, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 24, 8, 120, 240, 48, "" },
        /*118*/ { BARCODE_PDF417, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 24, 8, 120, 240, 48, "" },
        /*119*/ { BARCODE_PDF417, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 36, 12, 120, 240, 72, "12 rows" },
        /*120*/ { BARCODE_PDF417, 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 12, 120, 240, 24, "" },
        /*121*/ { BARCODE_PDF417COMP, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 4.5, 9, 86, 172, 9, "9 rows" },
        /*122*/ { BARCODE_PDF417COMP, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 22.5, 9, 86, 172, 45, "" },
        /*123*/ { BARCODE_MAXICODE, 1, "1234567890", "", 0, 16.5, 33, 30, 299, 298, "Fixed size, symbol->height ignored" },
        /*124*/ { BARCODE_QRCODE, 1, "ABCD", "", 0, 21, 21, 21, 42, 42, "Fixed width-to-height ratio, symbol->height ignored" },
        /*125*/ { BARCODE_CODE128B, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "" },
        /*126*/ { BARCODE_CODE128B, 4, "1234567890", "", 0, 4, 1, 145, 290, 8, "" },
        /*127*/ { BARCODE_AUSPOST, -1, "12345678901234567890123", "", 0, 8, 3, 133, 266, 16, "" },
        /*128*/ { BARCODE_AUSPOST, 1, "12345678901234567890123", "", 0, 2.5, 3, 133, 266, 5, "" },
        /*129*/ { BARCODE_AUSPOST, 4, "12345678901234567890123", "", 0, 4, 3, 133, 266, 8, "" },
        /*130*/ { BARCODE_AUSPOST, 7, "12345678901234567890123", "", 0, 7, 3, 133, 266, 14, "" },
        /*131*/ { BARCODE_AUSPOST, 14, "12345678901234567890123", "", 0, 14.5, 3, 133, 266, 29, "" },
        /*132*/ { BARCODE_AUSREPLY, 14, "12345678", "", 0, 14.5, 3, 73, 146, 29, "" },
        /*133*/ { BARCODE_AUSROUTE, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "" },
        /*134*/ { BARCODE_AUSREDIRECT, 14, "12345678", "", 0, 14.5, 3, 73, 146, 29, "" },
        /*135*/ { BARCODE_ISBNX, 1, "123456789", "", 0, 1, 1, 95, 226, 12, "" },
        /*136*/ { BARCODE_ISBNX, 4, "123456789", "", 0, 4, 1, 95, 226, 18, "" },
        /*137*/ { BARCODE_ISBNX, 69, "123456789", "", 0, 69, 1, 95, 226, 148, "" },
        /*138*/ { BARCODE_RM4SCC, -1, "1234567890", "", 0, 8, 3, 91, 182, 16, "" },
        /*139*/ { BARCODE_RM4SCC, 1, "1234567890", "", 0, 2.5, 3, 91, 182, 5, "" },
        /*140*/ { BARCODE_RM4SCC, 4, "1234567890", "", 0, 4, 3, 91, 182, 8, "" },
        /*141*/ { BARCODE_RM4SCC, 6, "1234567890", "", 0, 6.5, 3, 91, 182, 13, "" },
        /*142*/ { BARCODE_RM4SCC, 11, "1234567890", "", 0, 11, 3, 91, 182, 22, "" },
        /*143*/ { BARCODE_RM4SCC, 16, "1234567890", "", 0, 16, 3, 91, 182, 32, "" },
        /*144*/ { BARCODE_DATAMATRIX, 1, "ABCD", "", 0, 12, 12, 12, 24, 24, "Fixed width-to-height ratio, symbol->height ignored" },
        /*145*/ { BARCODE_EAN14, 1, "1234567890123", "", 0, 1, 1, 134, 268, 2, "" },
        /*146*/ { BARCODE_EAN14, 4, "1234567890123", "", 0, 4, 1, 134, 268, 8, "" },
        /*147*/ { BARCODE_EAN14, 5, "1234567890123", "", 0, 5, 1, 134, 268, 10, "" },
        /*148*/ { BARCODE_VIN, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "" },
        /*149*/ { BARCODE_VIN, 4, "12345678701234567", "", 0, 4, 1, 246, 492, 8, "" },
        /*150*/ { BARCODE_CODABLOCKF, -1, "1234567890", "", 0, 20, 2, 101, 242, 44, "2 rows" },
        /*151*/ { BARCODE_CODABLOCKF, 1, "1234567890", "", 0, 1, 2, 101, 242, 6, "" },
        /*152*/ { BARCODE_CODABLOCKF, 4, "1234567890", "", 0, 4, 2, 101, 242, 12, "" },
        /*153*/ { BARCODE_CODABLOCKF, 30, "1234567890", "", 0, 30, 2, 101, 242, 64, "" },
        /*154*/ { BARCODE_CODABLOCKF, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 110, 11, 156, 352, 224, "11 rows, 14 cols" },
        /*155*/ { BARCODE_CODABLOCKF, 88, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 88, 11, 156, 352, 180, "" },
        /*156*/ { BARCODE_CODABLOCKF, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "" },
        /*157*/ { BARCODE_CODABLOCKF, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 110, 11, 167, 374, 224, "11 rows, 15 cols" },
        /*158*/ { BARCODE_CODABLOCKF, 88, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 88, 11, 167, 374, 180, "" },
        /*159*/ { BARCODE_NVE18, 1, "12345678901234567", "", 0, 1, 1, 156, 312, 2, "" },
        /*160*/ { BARCODE_NVE18, 4, "12345678901234567", "", 0, 4, 1, 156, 312, 8, "" },
        /*161*/ { BARCODE_NVE18, 6, "12345678901234567", "", 0, 6, 1, 156, 312, 12, "" },
        /*162*/ { BARCODE_JAPANPOST, -1, "1234567890", "", 0, 8, 3, 133, 266, 16, "" },
        /*163*/ { BARCODE_JAPANPOST, 1, "1234567890", "", 0, 2.5, 3, 133, 266, 5, "" },
        /*164*/ { BARCODE_JAPANPOST, 4, "1234567890", "", 0, 4, 3, 133, 266, 8, "" },
        /*165*/ { BARCODE_JAPANPOST, 7, "1234567890", "", 0, 7, 3, 133, 266, 14, "" },
        /*166*/ { BARCODE_JAPANPOST, 16, "1234567890", "", 0, 16, 3, 133, 266, 32, "" },
        /*167*/ { BARCODE_KOREAPOST, 1, "123456", "", 0, 1, 1, 167, 334, 2, "" },
        /*168*/ { BARCODE_KOREAPOST, 4, "123456", "", 0, 4, 1, 167, 334, 8, "" },
        /*169*/ { BARCODE_DBAR_STK, -1, "1234567890123", "", 0, 13, 3, 50, 100, 26, "" },
        /*170*/ { BARCODE_DBAR_STK, 1, "1234567890123", "", 0, 2, 3, 50, 100, 4, "" },
        /*171*/ { BARCODE_DBAR_STK, 4, "1234567890123", "", 0, 4.5, 3, 50, 100, 9, "" },
        /*172*/ { BARCODE_DBAR_STK, 12, "1234567890123", "", 0, 12, 3, 50, 100, 24, "" },
        /*173*/ { BARCODE_DBAR_STK, 13, "1234567890123", "", 0, 13, 3, 50, 100, 26, "" },
        /*174*/ { BARCODE_DBAR_STK, 24, "1234567890123", "", 0, 24, 3, 50, 100, 48, "" },
        /*175*/ { BARCODE_DBAR_STK, 26, "1234567890123", "", 0, 26, 3, 50, 100, 52, "" },
        /*176*/ { BARCODE_DBAR_OMNSTK, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /*177*/ { BARCODE_DBAR_OMNSTK, 1, "1234567890123", "", 0, 4, 5, 50, 100, 8, "" },
        /*178*/ { BARCODE_DBAR_OMNSTK, 4, "1234567890123", "", 0, 4, 5, 50, 100, 8, "" },
        /*179*/ { BARCODE_DBAR_OMNSTK, 68, "1234567890123", "", 0, 68, 5, 50, 100, 136, "" },
        /*180*/ { BARCODE_DBAR_OMNSTK, 69, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /*181*/ { BARCODE_DBAR_OMNSTK, 150, "1234567890123", "", 0, 150, 5, 50, 100, 300, "" },
        /*182*/ { BARCODE_DBAR_EXPSTK, -1, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "2 rows" },
        /*183*/ { BARCODE_DBAR_EXPSTK, 1, "[01]12345678901231", "", 0, 4, 5, 102, 204, 8, "" },
        /*184*/ { BARCODE_DBAR_EXPSTK, 4, "[01]12345678901231", "", 0, 4, 5, 102, 204, 8, "" },
        /*185*/ { BARCODE_DBAR_EXPSTK, 71, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "" },
        /*186*/ { BARCODE_DBAR_EXPSTK, 76, "[01]12345678901231", "", 0, 76, 5, 102, 204, 152, "" },
        /*187*/ { BARCODE_DBAR_EXPSTK, -1, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "3 rows" },
        /*188*/ { BARCODE_DBAR_EXPSTK, 1, "[01]09501101530003[3920]123456789012345", "", 0, 7.5, 9, 102, 204, 15, "" },
        /*189*/ { BARCODE_DBAR_EXPSTK, 54, "[01]09501101530003[3920]123456789012345", "", 0, 54, 9, 102, 204, 108, "" },
        /*190*/ { BARCODE_DBAR_EXPSTK, 90, "[01]09501101530003[3920]123456789012345", "", 0, 90, 9, 102, 204, 180, "" },
        /*191*/ { BARCODE_DBAR_EXPSTK, 160, "[01]09501101530003[3920]123456789012345", "", 0, 160.5, 9, 102, 204, 321, "" },
        /*192*/ { BARCODE_PLANET, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "" },
        /*193*/ { BARCODE_PLANET, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "" },
        /*194*/ { BARCODE_PLANET, 4, "12345678901", "", 0, 4, 2, 123, 246, 8, "" },
        /*195*/ { BARCODE_PLANET, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*196*/ { BARCODE_PLANET, 24, "12345678901", "", 0, 24, 2, 123, 246, 48, "" },
        /*197*/ { BARCODE_MICROPDF417, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 20, 10, 82, 164, 40, "10 rows" },
        /*198*/ { BARCODE_MICROPDF417, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "" },
        /*199*/ { BARCODE_MICROPDF417, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "" },
        /*200*/ { BARCODE_MICROPDF417, 19, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 10, 82, 164, 40, "" },
        /*201*/ { BARCODE_MICROPDF417, 30, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 30, 10, 82, 164, 60, "" },
        /*202*/ { BARCODE_MICROPDF417, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 40, 20, 55, 110, 80, "20 rows" },
        /*203*/ { BARCODE_MICROPDF417, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 20, 55, 110, 40, "" },
        /*204*/ { BARCODE_USPS_IMAIL, -1, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, "" },
        /*205*/ { BARCODE_USPS_IMAIL, 1, "12345678901234567890", "", 0, 2.5, 3, 129, 258, 5, "" },
        /*206*/ { BARCODE_USPS_IMAIL, 4, "12345678901234567890", "", 0, 4, 3, 129, 258, 8, "" },
        /*207*/ { BARCODE_USPS_IMAIL, 7, "12345678901234567890", "", 0, 7, 3, 129, 258, 14, "" },
        /*208*/ { BARCODE_PLESSEY, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "" },
        /*209*/ { BARCODE_PLESSEY, 4, "1234567890", "", 0, 4, 1, 227, 454, 8, "" },
        /*210*/ { BARCODE_TELEPEN_NUM, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "" },
        /*211*/ { BARCODE_TELEPEN_NUM, 4, "1234567890", "", 0, 4, 1, 128, 256, 8, "" },
        /*212*/ { BARCODE_ITF14, 1, "1234567890", "", 0, 1, 1, 135, 330, 22, "" },
        /*213*/ { BARCODE_ITF14, 4, "1234567890", "", 0, 4, 1, 135, 330, 28, "" },
        /*214*/ { BARCODE_ITF14, 64, "1234567890", "", 0, 64, 1, 135, 330, 148, "" },
        /*215*/ { BARCODE_KIX, -1, "1234567890", "", 0, 8, 3, 79, 158, 16, "" },
        /*216*/ { BARCODE_KIX, 1, "1234567890", "", 0, 2.5, 3, 79, 158, 5, "" },
        /*217*/ { BARCODE_KIX, 4, "1234567890", "", 0, 4, 3, 79, 158, 8, "" },
        /*218*/ { BARCODE_KIX, 6, "1234567890", "", 0, 6.5, 3, 79, 158, 13, "" },
        /*219*/ { BARCODE_KIX, 10, "1234567890", "", 0, 10.5, 3, 79, 158, 21, "" },
        /*220*/ { BARCODE_KIX, 16, "1234567890", "", 0, 16, 3, 79, 158, 32, "" },
        /*221*/ { BARCODE_AZTEC, 1, "1234567890AB", "", 0, 15, 15, 15, 30, 30, "Fixed width-to-height ratio, symbol->height ignored" },
        /*222*/ { BARCODE_DAFT, -1, "DAFTDAFTDAFTDAFT", "", 0, 8, 3, 31, 62, 16, "" },
        /*223*/ { BARCODE_DAFT, 1, "DAFTDAFTDAFTDAFT", "", 0, 2.5, 3, 31, 62, 5, "" },
        /*224*/ { BARCODE_DAFT, 4, "DAFTDAFTDAFTDAFT", "", 0, 4, 3, 31, 62, 8, "" },
        /*225*/ { BARCODE_DAFT, 6, "DAFTDAFTDAFTDAFT", "", 0, 6.5, 3, 31, 62, 13, "" },
        /*226*/ { BARCODE_DAFT, 12, "DAFTDAFTDAFTDAFT", "", 0, 12, 3, 31, 62, 24, "" },
        /*227*/ { BARCODE_DAFT, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "" },
        /*228*/ { BARCODE_DPD, 1, "0123456789012345678901234567", "", 0, 1, 1, 189, 378, 2, "" },
        /*229*/ { BARCODE_DPD, 4, "0123456789012345678901234567", "", 0, 4, 1, 189, 378, 8, "" },
        /*230*/ { BARCODE_DPD, 62, "0123456789012345678901234567", "", 0, 62, 1, 189, 378, 124, "" },
        /*231*/ { BARCODE_MICROQR, 1, "12345", "", 0, 11, 11, 11, 22, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*232*/ { BARCODE_HIBC_128, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "" },
        /*233*/ { BARCODE_HIBC_128, 4, "1234567890", "", 0, 4, 1, 123, 246, 8, "" },
        /*234*/ { BARCODE_HIBC_39, 1, "1234567890", "", 0, 1, 1, 223, 446, 2, "" },
        /*235*/ { BARCODE_HIBC_39, 4, "1234567890", "", 0, 4, 1, 223, 446, 8, "" },
        /*236*/ { BARCODE_HIBC_DM, 1, "ABC", "", 0, 12, 12, 12, 24, 24, "Fixed width-to-height ratio, symbol->height ignored" },
        /*237*/ { BARCODE_HIBC_QR, 1, "1234567890AB", "", 0, 21, 21, 21, 42, 42, "Fixed width-to-height ratio, symbol->height ignored" },
        /*238*/ { BARCODE_HIBC_PDF, -1, "ABCDEF", "", 0, 21, 7, 103, 206, 42, "7 rows" },
        /*239*/ { BARCODE_HIBC_PDF, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*240*/ { BARCODE_HIBC_PDF, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*241*/ { BARCODE_HIBC_PDF, 36, "ABCDEF", "", 0, 35, 7, 103, 206, 70, "" },
        /*242*/ { BARCODE_HIBC_MICPDF, -1, "ABCDEF", "", 0, 12, 6, 82, 164, 24, "6 rows" },
        /*243*/ { BARCODE_HIBC_MICPDF, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "" },
        /*244*/ { BARCODE_HIBC_MICPDF, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "" },
        /*245*/ { BARCODE_HIBC_MICPDF, 47, "ABCDEF", "", 0, 48, 6, 82, 164, 96, "" },
        /*246*/ { BARCODE_HIBC_BLOCKF, -1, "1234567890", "", 0, 30, 3, 101, 242, 64, "3 rows" },
        /*247*/ { BARCODE_HIBC_BLOCKF, 1, "1234567890", "", 0, 1.5, 3, 101, 242, 7, "" },
        /*248*/ { BARCODE_HIBC_BLOCKF, 4, "1234567890", "", 0, 4.5, 3, 101, 242, 13, "" },
        /*249*/ { BARCODE_HIBC_BLOCKF, 23, "1234567890", "", 0, 22.5, 3, 101, 242, 49, "" },
        /*250*/ { BARCODE_HIBC_BLOCKF, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "" },
        /*251*/ { BARCODE_HIBC_AZTEC, 1, "1234567890AB", "", 0, 19, 19, 19, 38, 38, "Fixed width-to-height ratio, symbol->height ignored" },
        /*252*/ { BARCODE_DOTCODE, 1, "ABC", "", 0, 11, 11, 16, 33, 23, "Fixed width-to-height ratio, symbol->height ignored" },
        /*253*/ { BARCODE_HANXIN, 1, "1234567890AB", "", 0, 23, 23, 23, 46, 46, "Fixed width-to-height ratio, symbol->height ignored" },
        /*254*/ { BARCODE_MAILMARK, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*255*/ { BARCODE_MAILMARK, 1, "01000000000000000AA00AA0A", "", 0, 2.5, 3, 155, 310, 5, "" },
        /*256*/ { BARCODE_MAILMARK, 4, "01000000000000000AA00AA0A", "", 0, 4, 3, 155, 310, 8, "" },
        /*257*/ { BARCODE_MAILMARK, 6, "01000000000000000AA00AA0A", "", 0, 6, 3, 155, 310, 12, "" },
        /*258*/ { BARCODE_MAILMARK, 10, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*259*/ { BARCODE_MAILMARK, 15, "01000000000000000AA00AA0A", "", 0, 15, 3, 155, 310, 30, "" },
        /*260*/ { BARCODE_MAILMARK, 20, "01000000000000000AA00AA0A", "", 0, 20, 3, 155, 310, 40, "" },
        /*261*/ { BARCODE_AZRUNE, 1, "1", "", 0, 11, 11, 11, 22, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*262*/ { BARCODE_CODE32, 1, "12345678", "", 0, 1, 1, 103, 206, 2, "" },
        /*263*/ { BARCODE_CODE32, 4, "12345678", "", 0, 4, 1, 103, 206, 8, "" },
        /*264*/ { BARCODE_CODE32, 19, "12345678", "", 0, 19, 1, 103, 206, 38, "" },
        /*265*/ { BARCODE_EANX_CC, -1, "123456789012", "[20]01", 0, 50, 7, 99, 234, 110, "EAN-13, CC-A 3 rows" },
        /*266*/ { BARCODE_EANX_CC, 1, "123456789012", "[20]01", 0, 12.5, 7, 99, 234, 35, "" },
        /*267*/ { BARCODE_EANX_CC, 4, "123456789012", "[20]01", 0, 12.5, 7, 99, 234, 35, "" },
        /*268*/ { BARCODE_EANX_CC, 81, "123456789012", "[20]01", 0, 81, 7, 99, 234, 172, "" },
        /*269*/ { BARCODE_EANX_CC, -1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 50, 9, 99, 234, 110, "EAN-13, CC-A 5 rows" },
        /*270*/ { BARCODE_EANX_CC, 1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 16.5, 9, 99, 234, 43, "" },
        /*271*/ { BARCODE_EANX_CC, 4, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 16.5, 9, 99, 234, 43, "" },
        /*272*/ { BARCODE_EANX_CC, 20, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 20, 9, 99, 234, 50, "" },
        /*273*/ { BARCODE_EANX_CC, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 11, 99, 234, 110, "EAN-13, CC-A 7 rows" },
        /*274*/ { BARCODE_EANX_CC, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 11, 99, 234, 51, "" },
        /*275*/ { BARCODE_EANX_CC, 4, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 11, 99, 234, 51, "" },
        /*276*/ { BARCODE_EANX_CC, 40, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 40, 11, 99, 234, 90, "" },
        /*277*/ { BARCODE_EANX_CC, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 14, 99, 234, 110, "EAN-13, CC-B 10 rows" },
        /*278*/ { BARCODE_EANX_CC, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 14, 99, 234, 63, "" },
        /*279*/ { BARCODE_EANX_CC, 30, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 30, 14, 99, 234, 70, "" },
        /*280*/ { BARCODE_EANX_CC, -1, "1234567", "[20]01[90]123456789012345678901234", 0, 50, 10, 72, 172, 110, "EAN-8, CC-A 4 rows" },
        /*281*/ { BARCODE_EANX_CC, 1, "1234567", "[20]01[90]123456789012345678901234", 0, 18.5, 10, 72, 172, 47, "" },
        /*282*/ { BARCODE_EANX_CC, 4, "1234567", "[20]01[90]123456789012345678901234", 0, 18.5, 10, 72, 172, 47, "" },
        /*283*/ { BARCODE_EANX_CC, 69, "1234567", "[20]01[90]123456789012345678901234", 0, 69, 10, 72, 172, 148, "" },
        /*284*/ { BARCODE_EANX_CC, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50, 24, 82, 192, 110, "EAN-8, CC-B 15 rows" },
        /*285*/ { BARCODE_EANX_CC, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 46.5, 24, 82, 192, 103, "" },
        /*286*/ { BARCODE_EANX_CC, 32, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 46.5, 24, 82, 192, 103, "" },
        /*287*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]01", 0, 50, 5, 145, 290, 100, "CC-A 3 rows" },
        /*288*/ { BARCODE_GS1_128_CC, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 145, 290, 15, "" },
        /*289*/ { BARCODE_GS1_128_CC, 4, "[01]12345678901231", "[20]01", 0, 7.5, 5, 145, 290, 15, "" },
        /*290*/ { BARCODE_GS1_128_CC, 12, "[01]12345678901231", "[20]01", 0, 12, 5, 145, 290, 24, "" },
        /*291*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 9, 145, 290, 100, "CC-A 7 rows" },
        /*292*/ { BARCODE_GS1_128_CC, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 15.5, 9, 145, 290, 31, "" },
        /*293*/ { BARCODE_GS1_128_CC, 52, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 52, 9, 145, 290, 104, "" },
        /*294*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 12, 145, 290, 100, "CC-B 10 rows" },
        /*295*/ { BARCODE_GS1_128_CC, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 145, 290, 43, "" },
        /*296*/ { BARCODE_GS1_128_CC, 40, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 40, 12, 145, 290, 80, "" },
        /*297*/ { BARCODE_GS1_128_CC, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "CC-C 30 rows" },
        /*298*/ { BARCODE_GS1_128_CC, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "" },
        /*299*/ { BARCODE_GS1_128_CC, 100, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 100, 32, 154, 308, 200, "" },
        /*300*/ { BARCODE_DBAR_OMN_CC, -1, "1234567890123", "[20]01[90]1234567890", 0, 21, 5, 100, 200, 42, "CC-A 3 rows" },
        /*301*/ { BARCODE_DBAR_OMN_CC, 1, "1234567890123", "[20]01[90]1234567890", 0, 7.5, 5, 100, 200, 15, "" },
        /*302*/ { BARCODE_DBAR_OMN_CC, 4, "1234567890123", "[20]01[90]1234567890", 0, 7.5, 5, 100, 200, 15, "" },
        /*303*/ { BARCODE_DBAR_OMN_CC, -1, "1234567890123", "[20]01[90]12345678901234567890", 0, 23, 6, 100, 200, 46, "CC-A 4 rows" },
        /*304*/ { BARCODE_DBAR_OMN_CC, 1, "1234567890123", "[20]01[90]12345678901234567890", 0, 9.5, 6, 100, 200, 19, "" },
        /*305*/ { BARCODE_DBAR_OMN_CC, 4, "1234567890123", "[20]01[90]12345678901234567890", 0, 9.5, 6, 100, 200, 19, "" },
        /*306*/ { BARCODE_DBAR_OMN_CC, 20, "1234567890123", "[20]01[90]12345678901234567890", 0, 20, 6, 100, 200, 40, "" },
        /*307*/ { BARCODE_DBAR_OMN_CC, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35, 12, 100, 200, 70, "CC-B 10 rows" },
        /*308*/ { BARCODE_DBAR_OMN_CC, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 100, 200, 43, "" },
        /*309*/ { BARCODE_DBAR_OMN_CC, 40, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 40, 12, 100, 200, 80, "" },
        /*310*/ { BARCODE_DBAR_OMN_CC, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 39, 14, 100, 200, 78, "CC-B 12 rows" },
        /*311*/ { BARCODE_DBAR_OMN_CC, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 25.5, 14, 100, 200, 51, "" },
        /*312*/ { BARCODE_DBAR_OMN_CC, 36, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 36, 14, 100, 200, 72, "" },
        /*313*/ { BARCODE_DBAR_LTD_CC, -1, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "CC-A 4 rows" },
        /*314*/ { BARCODE_DBAR_LTD_CC, 1, "1234567890123", "[20]01", 0, 9.5, 6, 79, 158, 19, "" },
        /*315*/ { BARCODE_DBAR_LTD_CC, 4, "1234567890123", "[20]01", 0, 9.5, 6, 79, 158, 19, "" },
        /*316*/ { BARCODE_DBAR_LTD_CC, 18, "1234567890123", "[20]01", 0, 18, 6, 79, 158, 36, "" },
        /*317*/ { BARCODE_DBAR_LTD_CC, 19, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "" },
        /*318*/ { BARCODE_DBAR_LTD_CC, -1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "CC-A 7 rows" },
        /*319*/ { BARCODE_DBAR_LTD_CC, 1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 15.5, 9, 79, 158, 31, "" },
        /*320*/ { BARCODE_DBAR_LTD_CC, 26, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 26, 9, 79, 158, 52, "" },
        /*321*/ { BARCODE_DBAR_LTD_CC, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "CC-B 20 rows" },
        /*322*/ { BARCODE_DBAR_LTD_CC, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 41.5, 22, 88, 176, 83, "" },
        /*323*/ { BARCODE_DBAR_LTD_CC, 50, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50, 22, 88, 176, 100, "" },
        /*324*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "CC-A 3 rows" },
        /*325*/ { BARCODE_DBAR_EXP_CC, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 134, 268, 15, "" },
        /*326*/ { BARCODE_DBAR_EXP_CC, 4, "[01]12345678901231", "[20]01", 0, 7.5, 5, 134, 268, 15, "" },
        /*327*/ { BARCODE_DBAR_EXP_CC, 40, "[01]12345678901231", "[20]01", 0, 40, 5, 134, 268, 80, "" },
        /*328*/ { BARCODE_DBAR_EXP_CC, 41, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "" },
        /*329*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "CC-A 5 rows" },
        /*330*/ { BARCODE_DBAR_EXP_CC, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 11.5, 7, 134, 268, 23, "" },
        /*331*/ { BARCODE_DBAR_EXP_CC, 50, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 50, 7, 134, 268, 100, "" },
        /*332*/ { BARCODE_DBAR_EXP_CC, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "CC-B 10 rows" },
        /*333*/ { BARCODE_DBAR_EXP_CC, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 134, 268, 43, "" },
        /*334*/ { BARCODE_DBAR_EXP_CC, 46, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 46, 12, 134, 268, 92, "" },
        /*335*/ { BARCODE_UPCA_CC, -1, "12345678901", "[20]01", 0, 50, 7, 99, 234, 110, "CC-A 3 rows" },
        /*336*/ { BARCODE_UPCA_CC, 1, "12345678901", "[20]01", 0, 12.5, 7, 99, 234, 35, "" },
        /*337*/ { BARCODE_UPCA_CC, 4, "12345678901", "[20]01", 0, 12.5, 7, 99, 234, 35, "" },
        /*338*/ { BARCODE_UPCA_CC, 81, "12345678901", "[20]01", 0, 81, 7, 99, 234, 172, "" },
        /*339*/ { BARCODE_UPCA_CC, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 10, 99, 234, 110, "CC-A 6 rows" },
        /*340*/ { BARCODE_UPCA_CC, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 18.5, 10, 99, 234, 47, "" },
        /*341*/ { BARCODE_UPCA_CC, 4, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 18.5, 10, 99, 234, 47, "" },
        /*342*/ { BARCODE_UPCA_CC, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 50, 16, 99, 234, 110, "CC-B 12 rows" },
        /*343*/ { BARCODE_UPCA_CC, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 30.5, 16, 99, 234, 71, "" },
        /*344*/ { BARCODE_UPCA_CC, 51, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 51, 16, 99, 234, 112, "" },
        /*345*/ { BARCODE_UPCE_CC, -1, "1234567", "[20]01[90]123456789012345678", 0, 50, 11, 55, 142, 110, "CC-A 7 rows" },
        /*346*/ { BARCODE_UPCE_CC, 1, "1234567", "[20]01[90]123456789012345678", 0, 20.5, 11, 55, 142, 51, "" },
        /*347*/ { BARCODE_UPCE_CC, 4, "1234567", "[20]01[90]123456789012345678", 0, 20.5, 11, 55, 142, 51, "" },
        /*348*/ { BARCODE_UPCE_CC, 85, "1234567", "[20]01[90]123456789012345678", 0, 85, 11, 55, 142, 180, "" },
        /*349*/ { BARCODE_UPCE_CC, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 16, 55, 142, 110, "CC-A 12 rows" },
        /*350*/ { BARCODE_UPCE_CC, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 30.5, 16, 55, 142, 71, "" },
        /*351*/ { BARCODE_UPCE_CC, 49, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 49, 16, 55, 142, 108, "" },
        /*352*/ { BARCODE_UPCE_CC, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 21, 55, 142, 110, "CC-B 17 rows" },
        /*353*/ { BARCODE_UPCE_CC, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 40.5, 21, 55, 142, 91, "" },
        /*354*/ { BARCODE_UPCE_CC, 4, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 40.5, 21, 55, 142, 91, "" },
        /*355*/ { BARCODE_UPCE_CC, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 142, 115, "CC-B 23 rows" },
        /*356*/ { BARCODE_UPCE_CC, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 142, 115, "" },
        /*357*/ { BARCODE_UPCE_CC, 52, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 142, 115, "" },
        /*358*/ { BARCODE_DBAR_STK_CC, -1, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "CC-A 5 rows" },
        /*359*/ { BARCODE_DBAR_STK_CC, 1, "1234567890123", "[20]01", 0, 13, 9, 56, 112, 26, "" },
        /*360*/ { BARCODE_DBAR_STK_CC, 4, "1234567890123", "[20]01", 0, 13, 9, 56, 112, 26, "" },
        /*361*/ { BARCODE_DBAR_STK_CC, 24, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "" },
        /*362*/ { BARCODE_DBAR_STK_CC, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "CC-A 12 rows" },
        /*363*/ { BARCODE_DBAR_STK_CC, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 27, 16, 56, 112, 54, "" },
        /*364*/ { BARCODE_DBAR_STK_CC, 25, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 27, 16, 56, 112, 54, "" },
        /*365*/ { BARCODE_DBAR_STK_CC, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "CC-B 17 rows" },
        /*366*/ { BARCODE_DBAR_STK_CC, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 37, 21, 56, 112, 74, "" },
        /*367*/ { BARCODE_DBAR_STK_CC, 59, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 59, 21, 56, 112, 118, "" },
        /*368*/ { BARCODE_DBAR_OMNSTK_CC, -1, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "CC-A 6 rows" },
        /*369*/ { BARCODE_DBAR_OMNSTK_CC, 1, "1234567890123", "[20]01[90]1234567890123456", 0, 17, 12, 56, 112, 34, "" },
        /*370*/ { BARCODE_DBAR_OMNSTK_CC, 4, "1234567890123", "[20]01[90]1234567890123456", 0, 17, 12, 56, 112, 34, "" },
        /*371*/ { BARCODE_DBAR_OMNSTK_CC, 80, "1234567890123", "[20]01[90]1234567890123456", 0, 80, 12, 56, 112, 160, "" },
        /*372*/ { BARCODE_DBAR_OMNSTK_CC, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "CC-A 12 rows" },
        /*373*/ { BARCODE_DBAR_OMNSTK_CC, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 29, 18, 56, 112, 58, "" },
        /*374*/ { BARCODE_DBAR_OMNSTK_CC, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "CC-B 17 rows" },
        /*375*/ { BARCODE_DBAR_OMNSTK_CC, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 39, 23, 56, 112, 78, "" },
        /*376*/ { BARCODE_DBAR_OMNSTK_CC, 4, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 39, 23, 56, 112, 78, "" },
        /*377*/ { BARCODE_DBAR_OMNSTK_CC, 52, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 52, 23, 56, 112, 104, "" },
        /*378*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "3 rows, CC-A 3 rows" },
        /*379*/ { BARCODE_DBAR_EXPSTK_CC, 1, "[01]12345678901231", "[20]01", 0, 11, 9, 102, 204, 22, "" },
        /*380*/ { BARCODE_DBAR_EXPSTK_CC, 4, "[01]12345678901231", "[20]01", 0, 11, 9, 102, 204, 22, "" },
        /*381*/ { BARCODE_DBAR_EXPSTK_CC, 77, "[01]12345678901231", "[20]01", 0, 77, 9, 102, 204, 154, "" },
        /*382*/ { BARCODE_DBAR_EXPSTK_CC, 78, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "" },
        /*383*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "5 rows, CC-A 3 rows" },
        /*384*/ { BARCODE_DBAR_EXPSTK_CC, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 21.5, 21, 102, 204, 43, "" },
        /*385*/ { BARCODE_DBAR_EXPSTK_CC, 190, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "" },
        /*386*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "3 rows, CC-A 4 rows" },
        /*387*/ { BARCODE_DBAR_EXPSTK_CC, 1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 13, 10, 102, 204, 26, "" },
        /*388*/ { BARCODE_DBAR_EXPSTK_CC, 38, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 38, 10, 102, 204, 76, "" },
        /*389*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "3 rows, CC-B 10 rows" },
        /*390*/ { BARCODE_DBAR_EXPSTK_CC, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 25, 16, 102, 204, 50, "" },
        /*391*/ { BARCODE_DBAR_EXPSTK_CC, 4, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 25, 16, 102, 204, 50, "" },
        /*392*/ { BARCODE_DBAR_EXPSTK_CC, 47, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 47, 16, 102, 204, 94, "" },
        /*393*/ { BARCODE_DBAR_EXPSTK_CC, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "5 rows, CC-B 10 rows" },
        /*394*/ { BARCODE_DBAR_EXPSTK_CC, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35.5, 28, 102, 204, 71, "" },
        /*395*/ { BARCODE_CHANNEL, 1, "1", "", 0, 1, 1, 19, 38, 2, "" },
        /*396*/ { BARCODE_CHANNEL, 4, "123", "", 0, 4, 1, 23, 46, 8, "" },
        /*397*/ { BARCODE_CODEONE, 1, "12345678901234567890", "", 0, 16, 16, 18, 36, 32, "Fixed height, symbol->height ignored" },
        /*398*/ { BARCODE_GRIDMATRIX, 1, "ABC", "", 0, 18, 18, 18, 36, 36, "Fixed width-to-height ratio, symbol->height ignored" },
        /*399*/ { BARCODE_UPNQR, 1, "1234567890AB", "", 0, 77, 77, 77, 154, 154, "Fixed width-to-height ratio, symbol->height ignored" },
        /*400*/ { BARCODE_ULTRA, 1, "1234567890", "", 0, 13, 13, 18, 36, 26, "Fixed width-to-height ratio, symbol->height ignored" },
        /*401*/ { BARCODE_RMQR, 1, "12345", "", 0, 11, 11, 27, 54, 22, "Fixed width-to-height ratio, symbol->height ignored" },
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
            printf("        /*%3d*/ { %s, %.5g, \"%s\", \"%s\", %s, %.8g, %d, %d, %d, %d, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].height, data[i].data, data[i].composite, testUtilErrorName(data[i].ret),
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
        { "test_buffer_plot", test_buffer_plot, 1, 1, 1 },
        { "test_height", test_height, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
