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
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"

static int is_row_column_black(struct zint_symbol *symbol, int row, int column) {
    int i = (row * symbol->bitmap_width + column) * 3;
    return symbol->bitmap[i] == 0 && symbol->bitmap[i + 1] == 0 && symbol->bitmap[i + 2] == 0; // Black
}

static void test_options(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        char *fgcolour;
        char *bgcolour;
        int rotate_angle;
        unsigned char *data;
        int ret;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123456", "7890AB", 0, "A", 0, 1, 46, 92, 118 },
        /*  1*/ { BARCODE_CODE128, "123456", "7890ab", 90, "A", 0, 1, 46, 118, 92 },
        /*  2*/ { BARCODE_CODE128, NULL, NULL, 180, "A", 0, 1, 46, 92, 118 },
        /*  3*/ { BARCODE_CODE128, NULL, NULL, 270, "A", 0, 1, 46, 118, 92 },
        /*  4*/ { BARCODE_CODE128, NULL, NULL, 181, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  5*/ { BARCODE_CODE128, "12345", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  6*/ { BARCODE_CODE128, NULL, "1234567", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  7*/ { BARCODE_CODE128, "12345 ", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  8*/ { BARCODE_CODE128, NULL, "EEFFGG", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_CODE128, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        if (data[i].fgcolour) {
            strcpy(symbol->fgcolour, data[i].fgcolour);
        }
        if (data[i].bgcolour) {
            strcpy(symbol->bgcolour, data[i].bgcolour);
        }

        ret = ZBarcode_Encode_and_Buffer(symbol, data[i].data, length, data[i].rotate_angle);
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

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        char *composite;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, "0000000000", "", 50, 1, 99, 198, 118 },
        /*  1*/ { BARCODE_C25MATRIX, "0000000000", "", 50, 1, 117, 234, 118 },
        /*  2*/ { BARCODE_C25INTER, "0000000000", "", 50, 1, 99, 198, 118 },
        /*  3*/ { BARCODE_C25IATA, "0000000000", "", 50, 1, 149, 298, 118 },
        /*  4*/ { BARCODE_C25LOGIC, "0000000000", "", 50, 1, 109, 218, 118 },
        /*  5*/ { BARCODE_C25IND, "0000000000", "", 50, 1, 159, 318, 118 },
        /*  6*/ { BARCODE_CODE39, "0000000000", "", 50, 1, 155, 310, 118 },
        /*  7*/ { BARCODE_EXCODE39, "0000000000", "", 50, 1, 155, 310, 118 },
        /*  8*/ { BARCODE_EANX, "123456789012", "", 50, 1, 95, 230, 118 },
        /*  9*/ { BARCODE_EANX_CHK, "1234567890128", "", 50, 1, 95, 230, 118 },
        /* 10*/ { BARCODE_EANX, "123456789012+12", "", 50, 1, 124, 288, 118 },
        /* 11*/ { BARCODE_EANX, "123456789012+12345", "", 50, 1, 151, 342, 118 },
        /* 12*/ { BARCODE_EANX, "1234567", "", 50, 1, 67, 134, 118 },
        /* 13*/ { BARCODE_EANX, "1234567+12", "", 50, 1, 96, 192, 118 },
        /* 14*/ { BARCODE_EANX, "1234567+12345", "", 50, 1, 123, 246, 118 },
        /* 15*/ { BARCODE_EANX, "1234", "", 50, 1, 47, 94, 118 },
        /* 16*/ { BARCODE_EANX, "12", "", 50, 1, 20, 40, 118 },
        /* 17*/ { BARCODE_EAN128, "[01]12345678901234", "", 50, 1, 134, 268, 118 },
        /* 18*/ { BARCODE_CODABAR, "A00000000B", "", 50, 1, 102, 204, 118 },
        /* 19*/ { BARCODE_CODE128, "0000000000", "", 50, 1, 90, 180, 118 },
        /* 20*/ { BARCODE_DPLEIT, "1234567890123", "", 50, 1, 135, 270, 118 },
        /* 21*/ { BARCODE_DPIDENT, "12345678901", "", 50, 1, 117, 234, 118 },
        /* 22*/ { BARCODE_CODE16K, "0000000000", "", 20, 2, 70, 162, 44 },
        /* 23*/ { BARCODE_CODE49, "0000000000", "", 20, 2, 70, 162, 44 },
        /* 24*/ { BARCODE_CODE93, "0000000000", "", 50, 1, 127, 254, 118 },
        /* 25*/ { BARCODE_FLAT, "1234567890", "", 50, 1, 90, 180, 100 },
        /* 26*/ { BARCODE_RSS14, "1234567890123", "", 50, 1, 96, 192, 118 },
        /* 27*/ { BARCODE_RSS_LTD, "1234567890123", "", 50, 1, 74, 148, 118 },
        /* 28*/ { BARCODE_RSS_EXP, "[01]12345678901234", "", 34, 1, 134, 268, 86 },
        /* 29*/ { BARCODE_TELEPEN, "0000000000", "", 50, 1, 208, 416, 118 },
        /* 30*/ { BARCODE_UPCA, "12345678904", "", 50, 1, 95, 230, 118 },
        /* 31*/ { BARCODE_UPCA_CHK, "12345678905", "", 50, 1, 95, 230, 118 },
        /* 32*/ { BARCODE_UPCA, "12345678904+12", "", 50, 1, 124, 288, 118 },
        /* 33*/ { BARCODE_UPCA, "12345678904+12345", "", 50, 1, 151, 342, 118 },
        /* 34*/ { BARCODE_UPCE, "1234567", "", 50, 1, 51, 142, 118 },
        /* 35*/ { BARCODE_UPCE_CHK, "12345670", "", 50, 1, 51, 142, 118 },
        /* 36*/ { BARCODE_UPCE, "1234567+12", "", 50, 1, 80, 200, 118 },
        /* 37*/ { BARCODE_UPCE, "1234567+12345", "", 50, 1, 107, 254, 118 },
        /* 38*/ { BARCODE_POSTNET, "00000000000", "", 12, 2, 185, 370, 24 },
        /* 39*/ { BARCODE_MSI_PLESSEY, "0000000000", "", 50, 1, 127, 254, 118 },
        /* 40*/ { BARCODE_FIM, "A", "", 50, 1, 17, 34, 100 },
        /* 41*/ { BARCODE_LOGMARS, "0000000000", "", 50, 1, 191, 382, 118 },
        /* 42*/ { BARCODE_PHARMA, "123456", "", 50, 1, 58, 116, 100 },
        /* 43*/ { BARCODE_PZN, "123456", "", 50, 1, 142, 284, 118 },
        /* 44*/ { BARCODE_PHARMA_TWO, "12345678", "", 10, 2, 29, 58, 20 },
        /* 45*/ { BARCODE_PDF417, "0000000000", "", 21, 7, 103, 206, 42 },
        /* 46*/ { BARCODE_PDF417TRUNC, "0000000000", "", 21, 7, 68, 136, 42 },
        /* 47*/ { BARCODE_MAXICODE, "0000000000", "", 165, 33, 30, 300, 300 },
        /* 48*/ { BARCODE_QRCODE, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /* 49*/ { BARCODE_CODE128B, "0000000000", "", 50, 1, 145, 290, 118 },
        /* 50*/ { BARCODE_AUSPOST, "12345678901234567890123", "", 8, 3, 133, 266, 16 },
        /* 51*/ { BARCODE_AUSREPLY, "12345678", "", 8, 3, 73, 146, 16 },
        /* 52*/ { BARCODE_AUSROUTE, "12345678", "", 8, 3, 73, 146, 16 },
        /* 53*/ { BARCODE_AUSREDIRECT, "12345678", "", 8, 3, 73, 146, 16 },
        /* 54*/ { BARCODE_ISBNX, "123456789", "", 50, 1, 95, 230, 118 },
        /* 55*/ { BARCODE_ISBNX, "123456789+12", "", 50, 1, 124, 288, 118 },
        /* 56*/ { BARCODE_ISBNX, "123456789+12345", "", 50, 1, 151, 342, 118 },
        /* 57*/ { BARCODE_RM4SCC, "0000000000", "", 8, 3, 91, 182, 16 },
        /* 58*/ { BARCODE_DATAMATRIX, "ABC", "", 10, 10, 10, 20, 20 },
        /* 59*/ { BARCODE_EAN14, "1234567890123", "", 50, 1, 134, 268, 118 },
        /* 60*/ { BARCODE_VIN, "00000000000000000", "", 50, 1, 246, 492, 118 },
        /* 61*/ { BARCODE_CODABLOCKF, "0000000000", "", 20, 2, 101, 242, 44 },
        /* 62*/ { BARCODE_NVE18, "12345678901234567", "", 50, 1, 156, 312, 118 },
        /* 63*/ { BARCODE_JAPANPOST, "0000000000", "", 8, 3, 133, 266, 16 },
        /* 64*/ { BARCODE_KOREAPOST, "123456", "", 50, 1, 167, 334, 118 },
        /* 65*/ { BARCODE_RSS14STACK, "0000000000000", "", 13, 3, 50, 100, 26 },
        /* 66*/ { BARCODE_RSS14STACK_OMNI, "0000000000000", "", 69, 5, 50, 100, 138 },
        /* 67*/ { BARCODE_RSS_EXPSTACK, "[01]12345678901234", "", 71, 5, 102, 204, 142 },
        /* 68*/ { BARCODE_PLANET, "00000000000", "", 12, 2, 185, 370, 24 },
        /* 69*/ { BARCODE_MICROPDF417, "0000000000", "", 12, 6, 82, 164, 24 },
        /* 70*/ { BARCODE_ONECODE, "12345678901234567890", "", 8, 3, 129, 258, 16 },
        /* 71*/ { BARCODE_PLESSEY, "0000000000", "", 50, 1, 227, 454, 118 },
        /* 72*/ { BARCODE_TELEPEN_NUM, "0000000000", "", 50, 1, 128, 256, 118 },
        /* 73*/ { BARCODE_ITF14, "0000000000", "", 50, 1, 135, 382, 150 },
        /* 74*/ { BARCODE_KIX, "123456ABCDE", "", 8, 3, 87, 174, 16 },
        /* 75*/ { BARCODE_AZTEC, "1234567890AB", "", 15, 15, 15, 30, 30 },
        /* 76*/ { BARCODE_DAFT, "DAFTDAFTDAFTDAFT", "", 8, 3, 31, 62, 16 },
        /* 77*/ { BARCODE_MICROQR, "12345", "", 11, 11, 11, 22, 22 },
        /* 78*/ { BARCODE_HIBC_128, "0000000000", "", 50, 1, 134, 268, 118 },
        /* 79*/ { BARCODE_HIBC_39, "0000000000", "", 50, 1, 223, 446, 118 },
        /* 80*/ { BARCODE_HIBC_DM, "ABC", "", 12, 12, 12, 24, 24 },
        /* 81*/ { BARCODE_HIBC_QR, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /* 82*/ { BARCODE_HIBC_PDF, "0000000000", "", 27, 9, 103, 206, 54 },
        /* 83*/ { BARCODE_HIBC_MICPDF, "0000000000", "", 34, 17, 38, 76, 68 },
        /* 84*/ { BARCODE_HIBC_BLOCKF, "0000000000", "", 30, 3, 101, 242, 64 },
        /* 85*/ { BARCODE_HIBC_AZTEC, "1234567890AB", "", 19, 19, 19, 38, 38 },
        /* 86*/ { BARCODE_DOTCODE, "ABC", "", 11, 11, 16, 33, 23 },
        /* 87*/ { BARCODE_HANXIN, "1234567890AB", "", 23, 23, 23, 46, 46 },
        /* 88*/ { BARCODE_MAILMARK, "01000000000000000AA00AA0A", "", 10, 3, 155, 310, 20 },
        /* 89*/ { BARCODE_AZRUNE, "255", "", 11, 11, 11, 22, 22 },
        /* 90*/ { BARCODE_CODE32, "12345678", "", 50, 1, 103, 206, 118 },
        /* 91*/ { BARCODE_EANX_CC, "123456789012", "[20]01", 50, 7, 99, 238, 118 },
        /* 92*/ { BARCODE_EANX_CC, "123456789012+12", "[20]01", 50, 7, 128, 296, 118 },
        /* 93*/ { BARCODE_EANX_CC, "123456789012+12345", "[20]01", 50, 7, 155, 350, 118 },
        /* 94*/ { BARCODE_EAN128_CC, "[01]12345678901234", "[20]01", 50, 5, 145, 290, 118 },
        /* 95*/ { BARCODE_RSS14_CC, "1234567890123", "[20]01", 21, 5, 100, 200, 60 },
        /* 96*/ { BARCODE_RSS_LTD_CC, "1234567890123", "[20]01", 19, 6, 74, 148, 56 },
        /* 97*/ { BARCODE_RSS_EXP_CC, "[01]12345678901234", "[20]01", 41, 5, 134, 268, 100 },
        /* 98*/ { BARCODE_UPCA_CC, "12345678901", "[20]01", 50, 7, 99, 238, 118 },
        /* 99*/ { BARCODE_UPCA_CC, "12345678901+12", "[20]01", 50, 7, 128, 296, 118 },
        /*100*/ { BARCODE_UPCA_CC, "12345678901+12345", "[20]01", 50, 7, 155, 350, 118 },
        /*101*/ { BARCODE_UPCE_CC, "1234567", "[20]01", 50, 9, 55, 150, 118 },
        /*102*/ { BARCODE_UPCE_CC, "1234567+12", "[20]01", 50, 9, 84, 208, 118 },
        /*103*/ { BARCODE_UPCE_CC, "1234567+12345", "[20]01", 50, 9, 111, 262, 118 },
        /*104*/ { BARCODE_RSS14STACK_CC, "0000000000000", "[20]01", 24, 9, 56, 112, 48 },
        /*105*/ { BARCODE_RSS14_OMNI_CC, "0000000000000", "[20]01", 80, 11, 56, 112, 160 },
        /*106*/ { BARCODE_RSS_EXPSTACK_CC, "[01]12345678901234", "[20]01", 78, 9, 102, 204, 156 },
        /*107*/ { BARCODE_CHANNEL, "00", "", 50, 1, 19, 38, 118 },
        /*108*/ { BARCODE_CODEONE, "12345678901234567890", "", 22, 22, 22, 44, 44 },
        /*109*/ { BARCODE_GRIDMATRIX, "ABC", "", 18, 18, 18, 36, 36 },
        /*110*/ { BARCODE_UPNQR, "1234567890AB", "", 77, 77, 77, 154, 154 },
        /*111*/ { BARCODE_ULTRA, "0000000000", "", 13, 13, 18, 36, 26 },
        /*112*/ { BARCODE_RMQR, "12345", "", 11, 11, 27, 54, 22 },
    };
    int data_size = ARRAY_SIZE(data);

    char *text;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
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
        int length = strlen(text);

        ret = ZBarcode_Encode(symbol, text, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->bitmap, "i:%d ZBarcode_Buffer(%d) bitmap NULL\n", i, data[i].symbology);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", \"%s\", %d, %d, %d, %d, %d },\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].data, data[i].composite,
                    symbol->height, symbol->rows, symbol->width, symbol->bitmap_width, symbol->bitmap_height);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
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

    testStart("");

    int ret;
    struct item {
        int symbology;
        int show_hrt;
        unsigned char *data;
        int ret;

        int expected_height;
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
        /*  0*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 230, 118, 103 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  1*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 230, 118, 103 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  2*/ { BARCODE_EANX_CHK, -1, "1234567890128", 0, 50, 1, 95, 230, 118, 103 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  3*/ { BARCODE_EANX_CHK, 0, "1234567890128", 0, 50, 1, 95, 230, 118, 103 /*text_row*/, 0, 20, -1, -1, -1 }, // EAN-13
        /*  4*/ { BARCODE_EANX_CHK, -1, "1234567890128+12", 0, 50, 1, 124, 288, 118, 103 /*text_row*/, 0, 20, 5, 210, 78 }, // EAN-13 + EAN-2
        /*  5*/ { BARCODE_EANX_CHK, 0, "1234567890128+12", 0, 50, 1, 124, 288, 118, 103 /*text_row*/, 0, 20, 5, 210, 78 }, // EAN-13 + EAN-2
        /*  6*/ { BARCODE_EANX, -1, "1234567890128+12345", 0, 50, 1, 151, 342, 118, 103 /*text_row*/, 0, 20, 5, 210, 130 }, // EAN-13 + EAN-5
        /*  7*/ { BARCODE_EANX, 0, "1234567890128+12345", 0, 50, 1, 151, 342, 118, 103 /*text_row*/, 0, 20, 5, 210, 130 }, // EAN-13 + EAN-5
        /*  8*/ { BARCODE_ISBNX, -1, "9784567890120+12345", 0, 50, 1, 151, 342, 118, 103 /*text_row*/, 0, 20, 5, 210, 130 }, // ISBNX + EAN-5
        /*  9*/ { BARCODE_ISBNX, 0, "9784567890120+12345", 0, 50, 1, 151, 342, 118, 103 /*text_row*/, 0, 20, 5, 210, 130 }, // ISBNX + EAN-5
        /* 10*/ { BARCODE_EANX, -1, "123456", 0, 50, 1, 67, 134, 118, 103 /*text_row*/, 6, 58, -1, -1, -1 }, // EAN-8
        /* 11*/ { BARCODE_EANX, 0, "123456", 0, 50, 1, 67, 134, 118, 103 /*text_row*/, 6, 58, -1, -1, -1 }, // EAN-8
        /* 12*/ { BARCODE_EANX, -1, "123456+12", 0, 50, 1, 96, 192, 118, 103 /*text_row*/, 6, 58, 5, 134, 58 }, // EAN-8 + EAN-2
        /* 13*/ { BARCODE_EANX, 0, "123456+12", 0, 50, 1, 96, 192, 118, 103 /*text_row*/, 6, 58, 5, 134, 58 }, // EAN-8 + EAN-2
        /* 14*/ { BARCODE_EANX, -1, "123456+12345", 0, 50, 1, 123, 246, 118, 103 /*text_row*/, 6, 58, 5, 134, 112 }, // EAN-8 + EAN-5
        /* 15*/ { BARCODE_EANX, 0, "123456+12345", 0, 50, 1, 123, 246, 118, 103 /*text_row*/, 6, 58, 5, 134, 112 }, // EAN-8 + EAN-5
        /* 16*/ { BARCODE_UPCA, -1, "123456789012", 0, 50, 1, 95, 230, 118, 103 /*text_row*/, 0, 20, -1, -1, -1 },
        /* 17*/ { BARCODE_UPCA, 0, "123456789012", 0, 50, 1, 95, 230, 118, 103 /*text_row*/, 0, 20, -1, -1, -1 },
        /* 18*/ { BARCODE_UPCA, -1, "123456789012+12", 0, 50, 1, 124, 288, 118, 103 /*text_row*/, 0, 20, 5, 210, 78 },
        /* 19*/ { BARCODE_UPCA, 0, "123456789012+12", 0, 50, 1, 124, 288, 118, 103 /*text_row*/, 0, 20, 5, 210, 78 },
        /* 20*/ { BARCODE_UPCA_CHK, -1, "123456789012+12345", 0, 50, 1, 151, 342, 118, 103 /*text_row*/, 0, 20, 5, 210, 130 },
        /* 21*/ { BARCODE_UPCA_CHK, 0, "123456789012+12345", 0, 50, 1, 151, 342, 118, 103 /*text_row*/, 0, 20, 5, 210, 130 },
        /* 22*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 142, 118, 103 /*text_row*/, 0, 20, -1, -1, -1 },
        /* 23*/ { BARCODE_UPCE, 0, "1234567", 0, 50, 1, 51, 142, 118, 103 /*text_row*/, 0, 20, -1, -1, -1 },
        /* 24*/ { BARCODE_UPCE_CHK, -1, "12345670+12", 0, 50, 1, 80, 200, 118, 103 /*text_row*/, 0, 20, 5, 122, 78 },
        /* 25*/ { BARCODE_UPCE_CHK, 0, "12345670+12", 0, 50, 1, 80, 200, 118, 103 /*text_row*/, 0, 20, 5, 122, 78 },
        /* 26*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 107, 254, 118, 103 /*text_row*/, 0, 20, 5, 122, 130 },
        /* 27*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 107, 254, 118, 103 /*text_row*/, 0, 20, 5, 122, 130 },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode_and_Buffer(symbol, data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        if (index != -1) testUtilBitmapPrint(symbol);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        if (data[i].expected_text_row != -1) {
            int text_bits_set = 0;
            int row = data[i].expected_text_row;
            for (int column = data[i].expected_text_col; column < data[i].expected_text_col + data[i].expected_text_len; column++) {
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
            for (int column = data[i].expected_addon_text_col; column < data[i].expected_addon_text_col + data[i].expected_addon_text_len; column++) {
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

    testStart("");

    int ret;
    struct item {
        int symbology;
        int option_1;
        int option_3;
        unsigned char *data;
        int ret;

        int expected_height;
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
        /*  0*/ { BARCODE_CODABLOCKF, -1, -1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 },
        /*  1*/ { BARCODE_CODABLOCKF, -1, 0, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // Same as default
        /*  2*/ { BARCODE_CODABLOCKF, -1, 1, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // Same as default
        /*  3*/ { BARCODE_CODABLOCKF, -1, 2, "A", 0, 20, 2, 101, 242, 44, 20, 42, 4 },
        /*  4*/ { BARCODE_CODABLOCKF, -1, 3, "A", 0, 20, 2, 101, 242, 44, 19, 42, 6 },
        /*  5*/ { BARCODE_CODABLOCKF, -1, 4, "A", 0, 20, 2, 101, 242, 44, 18, 42, 8 },
        /*  6*/ { BARCODE_CODABLOCKF, -1, 5, "A", 0, 20, 2, 101, 242, 44, 21, 42, 2 }, // > 4 ignored, same as default
        /*  7*/ { BARCODE_CODABLOCKF, 1, -1, "A", 0, 5, 1, 46, 132, 14, 0, 20 + 2, 2 }, // CODE128 top separator, add 2 to skip over end of start char
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode_and_Buffer(symbol, data[i].data, length, 0);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        int j, separator_bits_set;

        //testUtilBitmapPrint(symbol);

        for (j = data[i].expected_separator_row; j < data[i].expected_separator_row + data[i].expected_separator_height; j++) {
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col);
            assert_nonzero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
        }

        if (symbol->rows > 1) {
            j = data[i].expected_separator_row - 1;
            separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col + 2); // Need to add 2 to skip to 1st blank of start row character
            assert_zero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) before non-zero\n", i, data[i].symbology, j, data[i].expected_separator_col);
        }

        j = data[i].expected_separator_row + data[i].expected_separator_height;
        separator_bits_set = is_row_column_black(symbol, j, data[i].expected_separator_col + 2); // Need to add 2 to skip to 1st blank of start row character
        assert_zero(separator_bits_set, "i:%d (%d) separator_bits_set (%d, %d) after non-zero\n", i, data[i].symbology, j, data[i].expected_separator_col);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_output_options(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int whitespace_width;
        int border_width;
        int output_options;
        unsigned char *data;
        int ret;

        int expected_height;
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
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 118, 0, 0, 4 },
        /*  1*/ { BARCODE_CODE128, -1, 2, -1, "A123", 0, 50, 1, 79, 158, 118, 0, 0, 4 },
        /*  2*/ { BARCODE_CODE128, -1, 2, BARCODE_BIND, "A123", 0, 50, 1, 79, 158, 126, 1, 0, 4 },
        /*  3*/ { BARCODE_CODE128, -1, 2, BARCODE_BIND, "A123", 0, 50, 1, 79, 158, 126, 0, 4, 4 },
        /*  4*/ { BARCODE_CODE128, -1, 2, BARCODE_BOX, "A123", 0, 50, 1, 79, 166, 126, 1, 4, 4 },
        /*  5*/ { BARCODE_CODE128, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 118, 0, 0, 8 },
        /*  6*/ { BARCODE_CODE128, 3, -1, -1, "A123", 0, 50, 1, 79, 170, 118, 1, 0, 8 },
        /*  7*/ { BARCODE_CODE128, 3, 4, -1, "A123", 0, 50, 1, 79, 170, 118, 1, 0, 8 },
        /*  8*/ { BARCODE_CODE128, 3, 4, BARCODE_BIND, "A123", 0, 50, 1, 79, 170, 134, 1, 0, 0 },
        /*  9*/ { BARCODE_CODE128, 3, 4, BARCODE_BIND, "A123", 0, 50, 1, 79, 170, 134, 0, 8, 0 },
        /* 10*/ { BARCODE_CODE128, 3, 4, BARCODE_BOX, "A123", 0, 50, 1, 79, 186, 134, 1, 8, 0 },
        /* 11*/ { BARCODE_CODE128, -1, -1, BARCODE_DOTTY_MODE, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 12*/ { BARCODE_QRCODE, -1, -1, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 13*/ { BARCODE_QRCODE, -1, 3, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 2, 2 },
        /* 14*/ { BARCODE_QRCODE, -1, 3, BARCODE_BIND, "A123", 0, 21, 21, 21, 42, 54, 1, 2, 2 },
        /* 15*/ { BARCODE_QRCODE, -1, 3, BARCODE_BIND, "A123", 0, 21, 21, 21, 42, 54, 0, 20, 0 },
        /* 16*/ { BARCODE_QRCODE, -1, 3, BARCODE_BOX, "A123", 0, 21, 21, 21, 54, 54, 1, 20, 0 },
        /* 17*/ { BARCODE_QRCODE, -1, -1, -1, "A123", 0, 21, 21, 21, 42, 42, 1, 0, 0 },
        /* 18*/ { BARCODE_QRCODE, 5, -1, -1, "A123", 0, 21, 21, 21, 62, 42, 0, 0, 0 },
        /* 19*/ { BARCODE_QRCODE, 5, 6, -1, "A123", 0, 21, 21, 21, 62, 42, 0, 0, 0 },
        /* 20*/ { BARCODE_QRCODE, 5, 6, BARCODE_BIND, "A123", 0, 21, 21, 21, 62, 66, 1, 0, 0 },
        /* 21*/ { BARCODE_QRCODE, 5, 6, BARCODE_BIND, "A123", 0, 21, 21, 21, 62, 66, 0, 12, 0 },
        /* 22*/ { BARCODE_QRCODE, 5, 6, BARCODE_BOX, "A123", 0, 21, 21, 21, 86, 66, 1, 12, 0 },
        /* 23*/ { BARCODE_QRCODE, -1, -1, BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 43, 43, -1, -1, -1 }, // TODO: investigate +1 size
        /* 24*/ { BARCODE_QRCODE, -1, 4, BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 43, 43, -1, -1, -1 },
        /* 25*/ { BARCODE_QRCODE, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 43, 59, -1, -1, -1 }, // TODO: fix (bind/box in dotty mode)
        /* 26*/ { BARCODE_QRCODE, 1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 63, 59, -1, -1, -1 }, // TODO: fix (bind/box in dotty mode)
        /* 27*/ { BARCODE_MAXICODE, -1, -1, -1, "A123", 0, 165, 33, 30, 300, 300, 0, 0, 0 },
        /* 28*/ { BARCODE_MAXICODE, -1, 5, -1, "A123", 0, 165, 33, 30, 300, 300, 0, 0, 0 },
        /* 29*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 300, 320, 1, 0, 0 },
        /* 30*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 300, 320, 0, 10, 0 },
        /* 31*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BOX, "A123", 0, 165, 33, 30, 320, 320, 1, 10, 0 },
        /* 32*/ { BARCODE_MAXICODE, -1, -1, -1, "A123", 0, 165, 33, 30, 300, 300, 1, 0, 14 },
        /* 33*/ { BARCODE_MAXICODE, 6, -1, -1, "A123", 0, 165, 33, 30, 324, 300, 0, 0, 14 },
        /* 34*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 324, 320, 1, 10, 25 },
        /* 35*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 324, 320, 0, 10, 9 },
        /* 36*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BOX, "A123", 0, 165, 33, 30, 344, 320, 1, 10, 9 },
        /* 37*/ { BARCODE_MAXICODE, -1, -1, BARCODE_DOTTY_MODE, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    struct zint_vector_rect *rect;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);
        if (data[i].whitespace_width != -1) {
            symbol->whitespace_width = data[i].whitespace_width;
        }
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < 5) {
            assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

            if (index != -1) testUtilBitmapPrint(symbol);

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
            assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
            assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

            if (data[i].expected_set != -1) {
                ret = is_row_column_black(symbol, data[i].expected_set_row, data[i].expected_set_col);
                if (data[i].expected_set) {
                    assert_nonzero(ret, "i:%d (%d) is_row_column_black(%d, %d) non-zero\n", i, data[i].symbology, data[i].expected_set_row, data[i].expected_set_col);
                } else {
                    assert_zero(ret, "i:%d (%d) is_row_column_black(%d, %d) zero\n", i, data[i].symbology, data[i].expected_set_row, data[i].expected_set_col);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_draw_string_wrap(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int output_options;
        unsigned char *data;
        unsigned char* text;

        int expected_height;
        int expected_rows;
        int expected_width;
        int expected_bitmap_width;
        int expected_bitmap_height;
        int expected_no_text_row;
        int expected_no_text_col;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, "12", "              E", 50, 1, 46, 92, 118, 104, 0 },
        /*  1*/ { BARCODE_CODE128, BOLD_TEXT, "12", "           E", 50, 1, 46, 92, 118, 104, 0 },
        /*  2*/ { BARCODE_CODE128, SMALL_TEXT, "12", "                   E", 50, 1, 46, 92, 118, 103, 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        // Cheat by overwriting text
        strcpy(symbol->text, data[i].text);

        ret = ZBarcode_Buffer(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->bitmap, "i:%d (%d) symbol->bitmap NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);
        assert_equal(symbol->bitmap_width, data[i].expected_bitmap_width, "i:%d (%d) symbol->bitmap_width %d != %d\n", i, data[i].symbology, symbol->bitmap_width, data[i].expected_bitmap_width);
        assert_equal(symbol->bitmap_height, data[i].expected_bitmap_height, "i:%d (%d) symbol->bitmap_height %d != %d\n", i, data[i].symbology, symbol->bitmap_height, data[i].expected_bitmap_height);

        //testUtilBitmapPrint(symbol);

        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Print(%d) ret %d != 0\n", i, data[i].symbology, ret);

        int text_bits_set = 0;
        int row = data[i].expected_no_text_row;
        for (int column = data[i].expected_no_text_col; column < data[i].expected_no_text_col + 16; column++) {
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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_options", test_options, 1, 0, 1 },
        { "test_buffer", test_buffer, 1, 1, 1 },
        { "test_upcean_hrt", test_upcean_hrt, 1, 0, 1 },
        { "test_row_separator", test_row_separator, 1, 0, 1 },
        { "test_output_options", test_output_options, 1, 0, 1 },
        { "test_draw_string_wrap", test_draw_string_wrap, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
