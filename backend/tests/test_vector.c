/*
    libzint - the open source barcode library
    Copyright (C) 2019-2022 Robin Stuart <rstuart114@gmail.com>

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

static struct zint_vector_rect *find_rect(struct zint_symbol *symbol, float x, float y, float width, float height) {
    struct zint_vector_rect *rect;

    if (symbol->vector == NULL) {
        return NULL;
    }
    for (rect = symbol->vector->rectangles; rect != NULL; rect = rect->next) {
        //printf("x %.8g, y %.8g, width %.8g, height %.8g\n", rect->x, rect->y, rect->width, rect->height);
        if (rect->x == stripf(x) && rect->y == stripf(y)) {
            if (height && width) {
                if (rect->height == stripf(height) && rect->width == stripf(width)) {
                    break;
                }
            } else if (height) {
                if (rect->height == stripf(height)) {
                    break;
                }
            } else if (width) {
                if (rect->width == stripf(width)) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    return rect;
}

static struct zint_vector_circle *find_circle(struct zint_symbol *symbol, float x, float y, float diameter) {
    struct zint_vector_circle *circle;

    if (symbol->vector == NULL) {
        return NULL;
    }
    for (circle = symbol->vector->circles; circle != NULL; circle = circle->next) {
        //printf("x %.8g, y %.8g, diamter %.8g\n", circle->x, circle->y, circle->diameter);
        if (circle->x == stripf(x) && circle->y == stripf(y)) {
            if (diameter) {
                if (circle->diameter == stripf(diameter)) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    return circle;
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
        float expected_vector_width;
        float expected_vector_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123456", "7890ab", 0, "A", 0, 1, 46, 92, 118.9 },
        /*  1*/ { BARCODE_CODE128, "12345", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  2*/ { BARCODE_CODE128, NULL, "1234567", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  3*/ { BARCODE_CODE128, "12345 ", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  4*/ { BARCODE_CODE128, NULL, "EEFFGG", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
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

        ret = ZBarcode_Encode_and_Buffer_Vector(symbol, (unsigned char *) data[i].data, length, data[i].rotate_angle);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_buffer_vector(int index, int generate, int debug) {

    struct item {
        int symbology;
        char *data;
        char *composite;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, "1234567890", "", 50, 1, 108, 216, 118.900002 },
        /*  1*/ { BARCODE_C25STANDARD, "1234567890", "", 50, 1, 117, 234, 118.900002 },
        /*  2*/ { BARCODE_C25INTER, "1234567890", "", 50, 1, 99, 198, 118.900002 },
        /*  3*/ { BARCODE_C25IATA, "1234567890", "", 50, 1, 149, 298, 118.900002 },
        /*  4*/ { BARCODE_C25LOGIC, "1234567890", "", 50, 1, 109, 218, 118.900002 },
        /*  5*/ { BARCODE_C25IND, "1234567890", "", 50, 1, 159, 318, 118.900002 },
        /*  6*/ { BARCODE_CODE39, "1234567890", "", 50, 1, 155, 310, 118.900002 },
        /*  7*/ { BARCODE_EXCODE39, "1234567890", "", 50, 1, 155, 310, 118.900002 },
        /*  8*/ { BARCODE_EANX, "123456789012", "", 50, 1, 95, 226, 116.400002 },
        /*  9*/ { BARCODE_EANX_CHK, "1234567890128", "", 50, 1, 95, 226, 116.400002 },
        /* 10*/ { BARCODE_EANX, "123456789012+12", "", 50, 1, 122, 276, 116.400002 },
        /* 11*/ { BARCODE_EANX_CHK, "1234567890128+12", "", 50, 1, 122, 276, 116.400002 },
        /* 12*/ { BARCODE_EANX, "123456789012+12345", "", 50, 1, 149, 330, 116.400002 },
        /* 13*/ { BARCODE_EANX_CHK, "1234567890128+12345", "", 50, 1, 149, 330, 116.400002 },
        /* 14*/ { BARCODE_EANX, "1234567", "", 50, 1, 67, 162, 116.400002 },
        /* 15*/ { BARCODE_EANX_CHK, "12345670", "", 50, 1, 67, 162, 116.400002 },
        /* 16*/ { BARCODE_EANX, "1234567+12", "", 50, 1, 94, 216, 116.400002 },
        /* 17*/ { BARCODE_EANX_CHK, "12345670+12", "", 50, 1, 94, 216, 116.400002 },
        /* 18*/ { BARCODE_EANX, "1234567+12345", "", 50, 1, 121, 270, 116.400002 },
        /* 19*/ { BARCODE_EANX_CHK, "12345670+12345", "", 50, 1, 121, 270, 116.400002 },
        /* 20*/ { BARCODE_EANX, "1234", "", 50, 1, 47, 118, 116.400002 },
        /* 21*/ { BARCODE_EANX_CHK, "1234", "", 50, 1, 47, 118, 116.400002 },
        /* 22*/ { BARCODE_EANX, "12", "", 50, 1, 20, 64, 116.400002 },
        /* 23*/ { BARCODE_EANX_CHK, "12", "", 50, 1, 20, 64, 116.400002 },
        /* 24*/ { BARCODE_GS1_128, "[01]12345678901231", "", 50, 1, 134, 268, 118.900002 },
        /* 25*/ { BARCODE_CODABAR, "A00000000B", "", 50, 1, 102, 204, 118.900002 },
        /* 26*/ { BARCODE_CODE128, "1234567890", "", 50, 1, 90, 180, 118.900002 },
        /* 27*/ { BARCODE_DPLEIT, "1234567890123", "", 50, 1, 135, 270, 118.900002 },
        /* 28*/ { BARCODE_DPIDENT, "12345678901", "", 50, 1, 117, 234, 118.900002 },
        /* 29*/ { BARCODE_CODE16K, "1234567890", "", 20, 2, 70, 162, 44 },
        /* 30*/ { BARCODE_CODE49, "1234567890", "", 20, 2, 70, 162, 44 },
        /* 31*/ { BARCODE_CODE93, "1234567890", "", 50, 1, 127, 254, 118.900002 },
        /* 32*/ { BARCODE_FLAT, "1234567890", "", 50, 1, 90, 180, 100 },
        /* 33*/ { BARCODE_DBAR_OMN, "1234567890123", "", 50, 1, 96, 192, 118.900002 },
        /* 34*/ { BARCODE_DBAR_LTD, "1234567890123", "", 50, 1, 79, 158, 118.900002 },
        /* 35*/ { BARCODE_DBAR_EXP, "[01]12345678901231", "", 34, 1, 134, 268, 86.9000015 },
        /* 36*/ { BARCODE_TELEPEN, "1234567890", "", 50, 1, 208, 416, 118.900002 },
        /* 37*/ { BARCODE_UPCA, "12345678901", "", 50, 1, 95, 226, 116.400002 },
        /* 38*/ { BARCODE_UPCA_CHK, "123456789012", "", 50, 1, 95, 226, 116.400002 },
        /* 39*/ { BARCODE_UPCA, "12345678901+12", "", 50, 1, 124, 276, 116.400002 },
        /* 40*/ { BARCODE_UPCA_CHK, "123456789012+12", "", 50, 1, 124, 276, 116.400002 },
        /* 41*/ { BARCODE_UPCA, "12345678901+12345", "", 50, 1, 151, 330, 116.400002 },
        /* 42*/ { BARCODE_UPCA_CHK, "123456789012+12345", "", 50, 1, 151, 330, 116.400002 },
        /* 43*/ { BARCODE_UPCE, "1234567", "", 50, 1, 51, 134, 116.400002 },
        /* 44*/ { BARCODE_UPCE_CHK, "12345670", "", 50, 1, 51, 134, 116.400002 },
        /* 45*/ { BARCODE_UPCE, "1234567+12", "", 50, 1, 78, 184, 116.400002 },
        /* 46*/ { BARCODE_UPCE_CHK, "12345670+12", "", 50, 1, 78, 184, 116.400002 },
        /* 47*/ { BARCODE_UPCE, "1234567+12345", "", 50, 1, 105, 238, 116.400002 },
        /* 48*/ { BARCODE_UPCE_CHK, "12345670+12345", "", 50, 1, 105, 238, 116.400002 },
        /* 49*/ { BARCODE_POSTNET, "12345678901", "", 12, 2, 123, 246, 24 },
        /* 50*/ { BARCODE_MSI_PLESSEY, "1234567890", "", 50, 1, 127, 254, 118.900002 },
        /* 51*/ { BARCODE_FIM, "A", "", 50, 1, 17, 34, 100 },
        /* 52*/ { BARCODE_LOGMARS, "1234567890", "", 50, 1, 191, 382, 118.900002 },
        /* 53*/ { BARCODE_PHARMA, "123456", "", 50, 1, 58, 116, 100 },
        /* 54*/ { BARCODE_PZN, "123456", "", 50, 1, 142, 284, 118.900002 },
        /* 55*/ { BARCODE_PHARMA_TWO, "12345678", "", 10, 2, 29, 58, 20 },
        /* 56*/ { BARCODE_CEPNET, "12345678", "", 5.375, 2, 93, 186, 10.75 },
        /* 57*/ { BARCODE_PDF417, "1234567890", "", 21, 7, 103, 206, 42 },
        /* 58*/ { BARCODE_PDF417COMP, "1234567890", "", 21, 7, 69, 138, 42 },
        /* 59*/ { BARCODE_MAXICODE, "1234567890", "", 165, 33, 30, 60, 57.7333984 },
        /* 60*/ { BARCODE_QRCODE, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /* 61*/ { BARCODE_CODE128B, "1234567890", "", 50, 1, 145, 290, 118.900002 },
        /* 62*/ { BARCODE_AUSPOST, "12345678901234567890123", "", 8, 3, 133, 266, 16 },
        /* 63*/ { BARCODE_AUSREPLY, "12345678", "", 8, 3, 73, 146, 16 },
        /* 64*/ { BARCODE_AUSROUTE, "12345678", "", 8, 3, 73, 146, 16 },
        /* 65*/ { BARCODE_AUSREDIRECT, "12345678", "", 8, 3, 73, 146, 16 },
        /* 66*/ { BARCODE_ISBNX, "123456789", "", 50, 1, 95, 226, 116.400002 },
        /* 67*/ { BARCODE_ISBNX, "123456789+12", "", 50, 1, 122, 276, 116.400002 },
        /* 68*/ { BARCODE_ISBNX, "123456789+12345", "", 50, 1, 149, 330, 116.400002 },
        /* 69*/ { BARCODE_RM4SCC, "1234567890", "", 8, 3, 91, 182, 16 },
        /* 70*/ { BARCODE_DATAMATRIX, "ABC", "", 10, 10, 10, 20, 20 },
        /* 71*/ { BARCODE_EAN14, "1234567890123", "", 50, 1, 134, 268, 118.900002 },
        /* 72*/ { BARCODE_VIN, "12345678701234567", "", 50, 1, 246, 492, 118.900002 },
        /* 73*/ { BARCODE_CODABLOCKF, "1234567890", "", 20, 2, 101, 242, 44 },
        /* 74*/ { BARCODE_NVE18, "12345678901234567", "", 50, 1, 156, 312, 118.900002 },
        /* 75*/ { BARCODE_JAPANPOST, "1234567890", "", 8, 3, 133, 266, 16 },
        /* 76*/ { BARCODE_KOREAPOST, "123456", "", 50, 1, 167, 334, 118.900002 },
        /* 77*/ { BARCODE_DBAR_STK, "1234567890123", "", 13, 3, 50, 100, 26 },
        /* 78*/ { BARCODE_DBAR_OMNSTK, "1234567890123", "", 69, 5, 50, 100, 138 },
        /* 79*/ { BARCODE_DBAR_EXPSTK, "[01]12345678901231", "", 71, 5, 102, 204, 142 },
        /* 80*/ { BARCODE_PLANET, "12345678901", "", 12, 2, 123, 246, 24 },
        /* 81*/ { BARCODE_MICROPDF417, "1234567890", "", 12, 6, 82, 164, 24 },
        /* 82*/ { BARCODE_USPS_IMAIL, "12345678901234567890", "", 8, 3, 129, 258, 16 },
        /* 83*/ { BARCODE_PLESSEY, "1234567890", "", 50, 1, 227, 454, 118.900002 },
        /* 84*/ { BARCODE_TELEPEN_NUM, "1234567890", "", 50, 1, 128, 256, 118.900002 },
        /* 85*/ { BARCODE_ITF14, "1234567890", "", 50, 1, 135, 330, 138.899994 },
        /* 86*/ { BARCODE_KIX, "123456ABCDE", "", 8, 3, 87, 174, 16 },
        /* 87*/ { BARCODE_AZTEC, "1234567890AB", "", 15, 15, 15, 30, 30 },
        /* 88*/ { BARCODE_DAFT, "DAFTDAFTDAFTDAFT", "", 8, 3, 31, 62, 16 },
        /* 89*/ { BARCODE_DPD, "0123456789012345678901234567", "", 50, 1, 189, 378, 118.900002 },
        /* 90*/ { BARCODE_MICROQR, "12345", "", 11, 11, 11, 22, 22 },
        /* 91*/ { BARCODE_HIBC_128, "1234567890", "", 50, 1, 123, 246, 118.900002 },
        /* 92*/ { BARCODE_HIBC_39, "1234567890", "", 50, 1, 223, 446, 118.900002 },
        /* 93*/ { BARCODE_HIBC_DM, "ABC", "", 12, 12, 12, 24, 24 },
        /* 94*/ { BARCODE_HIBC_QR, "1234567890AB", "", 21, 21, 21, 42, 42 },
        /* 95*/ { BARCODE_HIBC_PDF, "1234567890", "", 24, 8, 103, 206, 48 },
        /* 96*/ { BARCODE_HIBC_MICPDF, "1234567890", "", 28, 14, 38, 76, 56 },
        /* 97*/ { BARCODE_HIBC_BLOCKF, "1234567890", "", 30, 3, 101, 242, 64 },
        /* 98*/ { BARCODE_HIBC_AZTEC, "1234567890AB", "", 19, 19, 19, 38, 38 },
        /* 99*/ { BARCODE_DOTCODE, "ABC", "", 11, 11, 16, 32, 22 },
        /*100*/ { BARCODE_HANXIN, "1234567890AB", "", 23, 23, 23, 46, 46 },
        /*101*/ { BARCODE_MAILMARK, "01000000000000000AA00AA0A", "", 10, 3, 155, 310, 20 },
        /*102*/ { BARCODE_AZRUNE, "255", "", 11, 11, 11, 22, 22 },
        /*103*/ { BARCODE_CODE32, "12345678", "", 50, 1, 103, 206, 118.900002 },
        /*104*/ { BARCODE_EANX_CC, "123456789012", "[20]01", 50, 7, 99, 234, 116.400002 },
        /*105*/ { BARCODE_EANX_CC, "123456789012+12", "[20]01", 50, 7, 126, 284, 116.400002 },
        /*106*/ { BARCODE_EANX_CC, "123456789012+12345", "[20]01", 50, 7, 153, 338, 116.400002 },
        /*107*/ { BARCODE_EANX_CC, "1234567", "[20]01", 50, 8, 72, 172, 116.400002 },
        /*108*/ { BARCODE_EANX_CC, "1234567+12", "[20]01", 50, 8, 99, 226, 116.400002 },
        /*109*/ { BARCODE_EANX_CC, "1234567+12345", "[20]01", 50, 8, 126, 280, 116.400002 },
        /*110*/ { BARCODE_GS1_128_CC, "[01]12345678901231", "[20]01", 50, 5, 145, 290, 118.900002 },
        /*111*/ { BARCODE_DBAR_OMN_CC, "1234567890123", "[20]01", 21, 5, 100, 200, 60.9000015 },
        /*112*/ { BARCODE_DBAR_LTD_CC, "1234567890123", "[20]01", 19, 6, 79, 158, 56.9000015 },
        /*113*/ { BARCODE_DBAR_EXP_CC, "[01]12345678901231", "[20]01", 41, 5, 134, 268, 100.900002 },
        /*114*/ { BARCODE_UPCA_CC, "12345678901", "[20]01", 50, 7, 99, 234, 116.400002 },
        /*115*/ { BARCODE_UPCA_CC, "12345678901+12", "[20]01", 50, 7, 128, 284, 116.400002 },
        /*116*/ { BARCODE_UPCA_CC, "12345678901+12345", "[20]01", 50, 7, 155, 338, 116.400002 },
        /*117*/ { BARCODE_UPCE_CC, "1234567", "[20]01", 50, 9, 55, 142, 116.400002 },
        /*118*/ { BARCODE_UPCE_CC, "1234567+12", "[20]01", 50, 9, 82, 192, 116.400002 },
        /*119*/ { BARCODE_UPCE_CC, "1234567+12345", "[20]01", 50, 9, 109, 246, 116.400002 },
        /*120*/ { BARCODE_DBAR_STK_CC, "1234567890123", "[20]01", 24, 9, 56, 112, 48 },
        /*121*/ { BARCODE_DBAR_OMNSTK_CC, "1234567890123", "[20]01", 80, 11, 56, 112, 160 },
        /*122*/ { BARCODE_DBAR_EXPSTK_CC, "[01]12345678901231", "[20]01", 78, 9, 102, 204, 156 },
        /*123*/ { BARCODE_CHANNEL, "01", "", 50, 1, 19, 38, 118.900002 },
        /*124*/ { BARCODE_CODEONE, "12345678901234567890", "", 16, 16, 18, 36, 32 },
        /*125*/ { BARCODE_GRIDMATRIX, "ABC", "", 18, 18, 18, 36, 36 },
        /*126*/ { BARCODE_UPNQR, "1234567890AB", "", 77, 77, 77, 154, 154 },
        /*127*/ { BARCODE_ULTRA, "1234567890", "", 13, 13, 18, 36, 26 },
        /*128*/ { BARCODE_RMQR, "12345", "", 11, 11, 27, 54, 22 },
        /*129*/ { BARCODE_BC412, "1234567", "", 16.666668, 1, 102, 204, 52.233337 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;

    testStart("test_buffer_vector");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

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
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", \"%s\", %.8g, %d, %d, %.8g, %.8g },\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].data, data[i].composite,
                    symbol->height, symbol->rows, symbol->width, symbol->vector->width, symbol->vector->height);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
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
        float expected_vector_width;
        float expected_vector_height;
        float expected_string_x;
        float expected_addon_string_x;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 116.4, 12, -1 }, /* EAN-13 */
        /*  1*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 226, 110, -1, -1 }, /* EAN-13 */
        /*  2*/ { BARCODE_EANX_CHK, -1, "1234567890128", 0, 50, 1, 95, 226, 116.4, 12, -1 }, /* EAN-13 */
        /*  3*/ { BARCODE_EANX_CHK, 0, "1234567890128", 0, 50, 1, 95, 226, 110, -1, -1 }, /* EAN-13 */
        /*  4*/ { BARCODE_ISBNX, -1, "9784567890120", 0, 50, 1, 95, 226, 116.4, 12, -1 },
        /*  5*/ { BARCODE_ISBNX, 0, "9784567890120", 0, 50, 1, 95, 226, 110, -1, -1 },
        /*  6*/ { BARCODE_EANX, -1, "1234567", 0, 50, 1, 67, 162, 116.4, 48, -1 }, /* EAN-8 */
        /*  7*/ { BARCODE_EANX, 0, "1234567", 0, 50, 1, 67, 162, 110, -1, -1 }, /* EAN-8 */
        /*  8*/ { BARCODE_EANX, -1, "1234", 0, 50, 1, 47, 118, 116.4, 61, -1 }, /* EAN-5 */
        /*  9*/ { BARCODE_EANX, 0, "1234", 0, 50, 1, 47, 118, 100, -1, -1 }, /* EAN-5 */
        /* 10*/ { BARCODE_EANX, -1, "12", 0, 50, 1, 20, 64, 116.4, 34, -1 }, /* EAN-2 */
        /* 11*/ { BARCODE_EANX, 0, "12", 0, 50, 1, 20, 64, 100, -1, -1 }, /* EAN-2 */
        /* 12*/ { BARCODE_UPCA, -1, "12345678901", 0, 50, 1, 95, 226, 116.4, 8, -1 },
        /* 13*/ { BARCODE_UPCA, 0, "12345678901", 0, 50, 1, 95, 226, 110, -1, -1 },
        /* 14*/ { BARCODE_UPCA_CHK, -1, "123456789012", 0, 50, 1, 95, 226, 116.4, 8, -1 },
        /* 15*/ { BARCODE_UPCA_CHK, 0, "123456789012", 0, 50, 1, 95, 226, 110, -1, -1 },
        /* 16*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 134, 116.4, 8, -1 },
        /* 17*/ { BARCODE_UPCE, 0, "1234567", 0, 50, 1, 51, 134, 110, -1, -1 },
        /* 18*/ { BARCODE_UPCE_CHK, -1, "12345670", 0, 50, 1, 51, 134, 116.4, 8, -1 },
        /* 19*/ { BARCODE_UPCE_CHK, 0, "12345670", 0, 50, 1, 51, 134, 110, -1, -1 },
        /* 20*/ { BARCODE_EANX, -1, "123456789012+12", 0, 50, 1, 122, 276.0, 116.4, 12, 70 }, /* EAN-13 + EAN-2 */
        /* 21*/ { BARCODE_EANX, 0, "123456789012+12", 0, 50, 1, 122, 276.0, 110, -1, -1 }, /* EAN-13 + EAN-2 */
        /* 22*/ { BARCODE_ISBNX, -1, "9784567890120+12", 0, 50, 1, 122, 276.0, 116.4, 12, 70 }, /* ISBN + EAN-2 */
        /* 23*/ { BARCODE_ISBNX, 0, "9784567890120+12", 0, 50, 1, 122, 276.0, 110, -1, -1 }, /* ISBN + EAN-2 */
        /* 24*/ { BARCODE_EANX, -1, "123456789012+12345", 0, 50, 1, 149, 330.0, 116.4, 12, 70 }, /* EAN-13 + EAN-5 */
        /* 25*/ { BARCODE_EANX, 0, "123456789012+12345", 0, 50, 1, 149, 330.0, 110, -1, -1 }, /* EAN-13 + EAN-5 */
        /* 26*/ { BARCODE_ISBNX, -1, "9784567890120+12345", 0, 50, 1, 149, 330.0, 116.4, 12, 70 }, /* ISBN + EAN-5 */
        /* 27*/ { BARCODE_ISBNX, 0, "9784567890120+12345", 0, 50, 1, 149, 330.0, 110, -1, -1 }, /* ISBN + EAN-5 */
        /* 28*/ { BARCODE_EANX, -1, "1234567+12", 0, 50, 1, 94, 216.0, 116.4, 48, 114 }, /* EAN-8 + EAN-2 */
        /* 29*/ { BARCODE_EANX, 0, "1234567+12", 0, 50, 1, 94, 216.0, 110, -1, -1 }, /* EAN-8 + EAN-2 */
        /* 30*/ { BARCODE_EANX, -1, "1234567+12345", 0, 50, 1, 121, 270.0, 116.4, 48, 114 }, /* EAN-8 + EAN-5 */
        /* 31*/ { BARCODE_EANX, 0, "1234567+12345", 0, 50, 1, 121, 270.0, 110, -1, -1 }, /* EAN-8 + EAN-5 */
        /* 32*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 116.4, 8, 72 },
        /* 33*/ { BARCODE_UPCA, 0, "12345678901+12", 0, 50, 1, 124, 276, 110, -1, -1 },
        /* 34*/ { BARCODE_UPCA, -1, "12345678901+12345", 0, 50, 1, 151, 330, 116.4, 8, 72 },
        /* 35*/ { BARCODE_UPCA, 0, "12345678901+12345", 0, 50, 1, 151, 330, 110, -1, -1 },
        /* 36*/ { BARCODE_UPCE, -1, "1234567+12", 0, 50, 1, 78, 184.0, 116.4, 8, 66 },
        /* 37*/ { BARCODE_UPCE, 0, "1234567+12", 0, 50, 1, 78, 184.0, 110, -1, -1 },
        /* 38*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 105, 238.0, 116.4, 8, 66 },
        /* 39*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 105, 238.0, 110, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_upcean_hrt");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        if (data[i].show_hrt != -1) {
            symbol->show_hrt = data[i].show_hrt;
        }
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);

        assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
        assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.8g != %.8g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            sprintf(symbol->outfile, "test_upcean_hrt%d.svg", i);
            ZBarcode_Print(symbol, 0);
        }

        if (data[i].show_hrt) {
            assert_nonnull(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
            assert_equal(symbol->vector->strings->x, data[i].expected_string_x,
                "i:%d (%s) symbol->vector->strings->x %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->vector->strings->x, data[i].expected_string_x);

            if (data[i].expected_addon_string_x != -1) {
                assert_nonnull(symbol->vector->strings->next, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings->next NULL\n", i, data[i].symbology);
                assert_equal(symbol->vector->strings->next->x, data[i].expected_addon_string_x,
                    "i:%d (%s) symbol->vector->strings->next->x %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->vector->strings->next->x, data[i].expected_addon_string_x);
            }
        } else {
            assert_null(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
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
        int expected_separator_x;
        int expected_separator_y;
        int expected_separator_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, -1, -1, -1, "A", 0, 20, 2, 101, 42, 21, 2 },
        /*  1*/ { BARCODE_CODABLOCKF, -1, -1, 0, "A", 0, 20, 2, 101, 42, 21, 2 }, /* Same as default */
        /*  2*/ { BARCODE_CODABLOCKF, -1, -1, 1, "A", 0, 20, 2, 101, 42, 21, 2 }, /* Same as default */
        /*  3*/ { BARCODE_CODABLOCKF, -1, -1, 2, "A", 0, 20, 2, 101, 42, 20, 4 },
        /*  4*/ { BARCODE_CODABLOCKF, -1, -1, 3, "A", 0, 20, 2, 101, 42, 19, 6 },
        /*  5*/ { BARCODE_CODABLOCKF, -1, -1, 4, "A", 0, 20, 2, 101, 42, 18, 8 },
        /*  6*/ { BARCODE_CODABLOCKF, -1, -1, 5, "A", 0, 20, 2, 101, 42, 21, 2 }, /* > 4 ignored, same as default */
        /*  7*/ { BARCODE_CODABLOCKF, -1, 1, -1, "A", 0, 5, 1, 46, 20, 0, 2 }, /* CODE128 top separator */
        /*  8*/ { BARCODE_CODABLOCKF, 0, -1, -1, "A", 0, 20, 2, 101, 42, 21, 2 }, /* Border width zero, same as default */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_vector_rect *rect;

    testStart("test_row_separator");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].border_width != -1) {
            symbol->border_width = data[i].border_width;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

        rect = find_rect(symbol, data[i].expected_separator_x, data[i].expected_separator_y, 0, data[i].expected_separator_height);
        assert_nonnull(rect, "i:%d (%d) find_rect(%d, %d, 0, %d) NULL\n", i, data[i].symbology, data[i].expected_separator_x, data[i].expected_separator_y, data[i].expected_separator_height);

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
        int expected_separator_x;
        int expected_separator_y;
        int expected_separator_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, "A", "B", 50, 2, 46, 92, 116, -1, -1, -1 },
        /*  1*/ { BARCODE_CODE128, BARCODE_BIND, -1, -1, "A", "B", 50, 2, 46, 92, 116, 0, 49, 2 },
        /*  2*/ { BARCODE_CODE128, BARCODE_BIND, -1, 2, "A", "B", 50, 2, 46, 92, 116, 0, 48, 4 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_vector_rect *rect;

    testStart("test_stacking");

    for (i = 0; i < data_size; i++) {
        int length2;

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

        if (data[i].expected_separator_y != -1) {
            if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
                sprintf(symbol->outfile, "test_stacking_%d.svg", i);
                ZBarcode_Print(symbol, 0);
            }

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            rect = find_rect(symbol, data[i].expected_separator_x, data[i].expected_separator_y, 0, data[i].expected_separator_height);
            assert_nonnull(rect, "i:%d (%d) find_rect(%d, %d, 0, %d) NULL\n", i, data[i].symbology, data[i].expected_separator_x, data[i].expected_separator_y, data[i].expected_separator_height);
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
        char *data;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        int expected_set;
        float expected_set_x;
        float expected_set_y;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, -1, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 118.9, 0, 0, 4 },
        /*  1*/ { BARCODE_CODE128, -1, -1, 2, -1, "A123", 0, 50, 1, 79, 158, 118.9, 0, 0, 4 },
        /*  2*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BIND, "A123", 0, 50, 1, 79, 158, 126.9, 1, 0, 4 },
        /*  3*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BIND, "A123", 0, 50, 1, 79, 158, 126.9, 0, 4, 4 },
        /*  4*/ { BARCODE_CODE128, -1, -1, 2, BARCODE_BOX, "A123", 0, 50, 1, 79, 166, 126.9, 1, 4, 4 },
        /*  5*/ { BARCODE_CODE128, -1, -1, 0, BARCODE_BIND, "A123", 0, 50, 1, 79, 158, 118.9, 0, 0, 4 },
        /*  6*/ { BARCODE_CODE128, -1, -1, 0, BARCODE_BOX, "A123", 0, 50, 1, 79, 158, 118.9, 0, 4, 4 },
        /*  7*/ { BARCODE_CODE128, -1, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 118.9, 0, 2, 0 },
        /*  8*/ { BARCODE_CODE128, 1, -1, -1, -1, "A123", 0, 50, 1, 79, 162, 118.9, 1, 2, 0 },
        /*  9*/ { BARCODE_CODE128, 1, 2, -1, -1, "A123", 0, 50, 1, 79, 162, 126.9, 0, 2, 0 },
        /* 10*/ { BARCODE_CODE128, 1, 2, -1, -1, "A123", 0, 50, 1, 79, 162, 126.9, 1, 2, 4 },
        /* 11*/ { BARCODE_CODE128, -1, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 118.9, 0, 6, 8 },
        /* 12*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BIND, "A123", 0, 50, 1, 79, 170, 134.89999, 1, 6, 8 },
        /* 13*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BIND, "A123", 0, 50, 1, 79, 170, 134.89999, 0, 14, 8 },
        /* 14*/ { BARCODE_CODE128, 3, -1, 4, BARCODE_BOX, "A123", 0, 50, 1, 79, 186, 134.89999, 1, 14, 8 },
        /* 15*/ { BARCODE_CODE128, -1, -1, -1, BARCODE_DOTTY_MODE, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 16*/ { BARCODE_QRCODE, -1, -1, -1, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 6 },
        /* 17*/ { BARCODE_QRCODE, -1, -1, 3, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 6 },
        /* 18*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, "A123", 0, 21, 21, 21, 42, 54, 1, 0, 6 },
        /* 19*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND, "A123", 0, 21, 21, 21, 42, 54, 0, 22, 8 },
        /* 20*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX, "A123", 0, 21, 21, 21, 54, 54, 1, 22, 8 },
        /* 21*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 58, 70, 0, 0, 6 },
        /* 22*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 58, 70, 1, 0, 0 },
        /* 23*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 70, 70, 0, 22, 8 },
        /* 24*/ { BARCODE_QRCODE, -1, -1, 3, BARCODE_BOX | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 70, 70, 1, 30, 16 },
        /* 25*/ { BARCODE_QRCODE, -1, -1, -1, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 10, 12 },
        /* 26*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, "A123", 0, 21, 21, 21, 62, 66, 1, 10, 12 },
        /* 27*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND, "A123", 0, 21, 21, 21, 62, 66, 0, 22, 12 },
        /* 28*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BOX, "A123", 0, 21, 21, 21, 86, 66, 1, 22, 12 },
        /* 29*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 78, 82, 0, 10, 12 },
        /* 30*/ { BARCODE_QRCODE, 5, -1, 6, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 21, 21, 21, 78, 82, 1, 18, 20 },
        /* 31*/ { BARCODE_QRCODE, -1, -1, -1, BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 50 },
        /* 32*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 50 },
        /* 33*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 58, 1, 0, 50 },
        /* 34*/ { BARCODE_QRCODE, -1, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 58, 0, 54, 8 },
        /* 35*/ { BARCODE_QRCODE, 1, -1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 62, 58, 1, 54, 8 },
        /* 36*/ { BARCODE_MAXICODE, -1, -1, -1, -1, "A123", 0, 165, 33, 30, 60, 57.733398, 0, 0, 67.7334 },
        /* 37*/ { BARCODE_MAXICODE, -1, -1, 5, -1, "A123", 0, 165, 33, 30, 60, 57.733398, 0, 0, 67.7334 },
        /* 38*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 60, 77.733398, 1, 0, 67.7334 },
        /* 39*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 60, 77.733398, 0, 70, 10 },
        /* 40*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BOX, "A123", 0, 165, 33, 30, 80, 77.733398, 1, 70, 10 },
        /* 41*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 165, 33, 30, 64, 81.733398, 0, 0, 67.7334 },
        /* 42*/ { BARCODE_MAXICODE, -1, -1, 5, BARCODE_BIND | BARCODE_QUIET_ZONES, "A123", 0, 165, 33, 30, 64, 81.733398, 1, 0, 71.7334 },
        /* 43*/ { BARCODE_MAXICODE, -1, -1, -1, -1, "A123", 0, 165, 33, 30, 60, 57.733398, 0, 0, 67.7334 },
        /* 44*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 84, 77.733398, 1, 0, 67.7334 },
        /* 45*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 84, 77.733398, 0, 94, 10 },
        /* 46*/ { BARCODE_MAXICODE, 6, -1, 5, BARCODE_BOX, "A123", 0, 165, 33, 30, 104, 77.733398, 1, 94, 10 },
        /* 47*/ { BARCODE_MAXICODE, -1, -1, -1, BARCODE_DOTTY_MODE, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 48*/ { BARCODE_ITF14, -1, -1, -1, -1, "123", 0, 50, 1, 135, 330, 138.89999, 1, 320, 10 },
        /* 49*/ { BARCODE_ITF14, -1, -1, 0, -1, "123", 0, 50, 1, 135, 330, 138.89999, 1, 320, 10 },
        /* 50*/ { BARCODE_ITF14, -1, -1, 0, BARCODE_BOX, "123", 0, 50, 1, 135, 310, 118.9, 0, 300, 0 }, /* No zero-width/height rectangles */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_vector_rect *rect;

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
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < ZINT_ERROR) {
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
                sprintf(symbol->outfile, "test_output_options_%d.svg", i);
                ZBarcode_Print(symbol, 0);
            }

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

            if (data[i].expected_set != -1) {
                rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, 0, 0);
                if (data[i].expected_set) {
                    assert_nonnull(rect, "i:%d (%d) find_rect(%g, %g, 0, 0) NULL\n", i, data[i].symbology, data[i].expected_set_x, data[i].expected_set_y);
                } else {
                    assert_null(rect, "i:%d (%d) find_rect(%g, %g, 0, 0) not NULL\n", i, data[i].symbology, data[i].expected_set_x, data[i].expected_set_y);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Checks that symbol lead-in (composite offset) isn't used to calc string position for non-composite barcodes */
static void test_noncomposite_string_x(int index, int debug) {

    struct item {
        int symbology;
        char *data;

        int expected_width;
        float expected_string_x;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /* 0*/ { BARCODE_DBAR_OMN, "1234567890123", 96, 96 },
        /* 1*/ { BARCODE_DBAR_LTD, "1234567890123", 79, 79 },
        /* 2*/ { BARCODE_DBAR_EXP, "[01]12345678901231", 134, 134 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_noncomposite_string_x");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_nonnull(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
        assert_equal(symbol->vector->strings->x, data[i].expected_string_x,
            "i:%d (%s) symbol->vector->strings->x %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->vector->strings->x, data[i].expected_string_x);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Checks UPCA/UPCE main_symbol_width_x (used for addon formatting) set whether whitespace width set or not */
static void test_upcean_whitespace_width(int index, int debug) {

    struct item {
        int symbology;
        char *data;
        int whitespace_width;

        int expected_width;
        float expected_vector_width;
        int expected_string_cnt;
        float expected_string_y;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /* 0*/ { BARCODE_UPCA, "12345678904+12345", 0, 151, 330.0, 5, 15.0 },
        /* 1*/ { BARCODE_UPCA, "12345678904+12345", 11, 151, 330.0 + 4 * 11, 5, 15.0 },
        /* 2*/ { BARCODE_UPCE, "1234567+12", 0, 78, 184.0, 4, 15.0 },
        /* 3*/ { BARCODE_UPCE, "1234567+12", 8, 78, 184.0 + 4 * 8, 4, 15.0 }, /* Note: change from previous behaviour where if whitespace < 10 then set to 10 */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_vector_string *string;
    int string_cnt;

    testStart("test_upcean_whitespace_width");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->whitespace_width = data[i].whitespace_width;
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            sprintf(symbol->outfile, "test_upcean_whitespace_width_%d.svg", i);
            ZBarcode_Print(symbol, 0);
        }

        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);

        assert_nonnull(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
        /* Get add-on string (last) */
        for (string = symbol->vector->strings, string_cnt = 1; string->next; string_cnt++) {
            string = string->next;
        }
        assert_equal(string_cnt, data[i].expected_string_cnt, "i:%d (%s) string_cnt %d != %d\n", i, testUtilBarcodeName(data[i].symbology), string_cnt, data[i].expected_string_cnt);
        assert_equal(string->y, data[i].expected_string_y, "i:%d (%s) string->y %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), string->y, data[i].expected_string_y);

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

        int ret_vector;
        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        int expected_set;
        float expected_set_x;
        float expected_set_y;
        float expected_set_width;
        float expected_set_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, 0, 0, "1", "", 0, 15, 5, 103, 206, 30, 1, 52, 0, 8, 6 }, /* With no scaling */
        /*  1*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.1, "1", "", 0, 15, 5, 103, 206 * 0.1, 3, 1, 5.2000003, 0, 8 * 0.1, 6 * 0.1 },
        /*  2*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.3, "1", "", 0, 15, 5, 103, 61.8000031, 30 * 0.3, 1, 52 * 0.3, 0, 2.4000001, 1.8000001 },
        /*  3*/ { BARCODE_PDF417, -1, -1, -1, 0, 0.6, "1", "", 0, 15, 5, 103, 123.600006, 30 * 0.6, 1, 52 * 0.6, 0, 4.8000002, 3.6000001 },
        /*  4*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 0, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 142, 116.400002, 1, 34, 36, 2, 64 }, /* With no scaling */
        /*  5*/ { BARCODE_UPCE_CC, -1, -1, -1, 0, 0.1, "1234567", "[17]010615[10]A123456\"", 0, 50, 10, 55, 142 * 0.1, 11.6400003, 1, 34 * 0.1, 3.6000001, 2 * 0.1, 64 * 0.1 },
        /*  6*/ { BARCODE_UPCE_CC, -1, -1, -1, 0.1, 0.1, "1234567", "[17]010615[10]A123456\"", 0, 18.5, 10, 55, 142 * 0.1, 5.34000015, 1, 34 * 0.1, 3.6000001, 2 * 0.1, 0.1 }, /* Height specified */
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;
    struct zint_vector_rect *rect;

    testStart("test_scale");

    for (i = 0; i < data_size; i++) {

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
        assert_nonzero(ret < ZINT_ERROR, "i:%d ZBarcode_Encode(%d) ret %d >= ZINT_ERROR (%s)\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d (%s)\n", i, data[i].symbology, ret, data[i].ret_vector, symbol->errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) {
            sprintf(symbol->outfile, "test_scale_%d.svg", i);
            ZBarcode_Print(symbol, 0);
        }

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

        assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.9g != %.9g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
        assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.9g != %.9g\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

        if (data[i].expected_set != -1) {
            rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            if (data[i].expected_set) {
                assert_nonnull(rect, "i:%d (%d) find_rect(%g, %g, %g, %g) NULL\n", i, data[i].symbology,
                    data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            } else {
                assert_null(rect, "i:%d (%d) find_rect(%g, %g, %g, %g) not NULL\n", i, data[i].symbology,
                    data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            }
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
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        float expected_set_x;
        float expected_set_y;
        float expected_set_width;
        float expected_set_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 134, 116.4, 18, 0, 2, 110 },
        /*  1*/ { BARCODE_UPCE, 0, "1234567", 0, 50, 1, 51, 134, 116.4, 18, 0, 2, 100 },
        /*  2*/ { BARCODE_UPCE, 3, "1234567", 0, 50, 1, 51, 134, 116.4, 18, 0, 2, 106 },
        /*  3*/ { BARCODE_UPCE, 8, "1234567", 0, 50, 1, 51, 134, 116.4, 18, 0, 2, 116 },
        /*  4*/ { BARCODE_UPCE, 8.2, "1234567", 0, 50, 1, 51, 134, 116.4, 18, 0, 2, 116.4 },
        /*  5*/ { BARCODE_UPCE, 8.25, "1234567", 0, 50, 1, 51, 134, 116.5, 18, 0, 2, 116.5 },
        /*  6*/ { BARCODE_UPCE, 8.3, "1234567", 0, 50, 1, 51, 134, 116.6, 18, 0, 2, 116.6 },
        /*  7*/ { BARCODE_UPCE, 19.6, "1234567", 0, 50, 1, 51, 134, 139.2, 18, 0, 2, 139.2 },
        /*  8*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 105, 238, 116.4, 118, 0, 2, 110 },
        /*  9*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 105, 238, 116.4, 134, 19, 2, 81 },
        /* 10*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 105, 238, 116.4, 118, 0, 2, 100 },
        /* 11*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 105, 238, 116.4, 134, 19, 2, 81 },
        /* 12*/ { BARCODE_UPCE, 4, "1234567+12345", 0, 50, 1, 105, 238, 116.4, 118, 0, 2, 108 },
        /* 13*/ { BARCODE_UPCE, 4, "1234567+12345", 0, 50, 1, 105, 238, 116.4, 134, 19, 2, 81 },
        /* 14*/ { BARCODE_UPCA, -1, "12345678901", 0, 50, 1, 95, 226, 116.4, 188, 0, 4, 110 },
        /* 15*/ { BARCODE_UPCA, 0, "12345678901", 0, 50, 1, 95, 226, 116.4, 188, 0, 4, 100 },
        /* 16*/ { BARCODE_UPCA, 6, "12345678901", 0, 50, 1, 95, 226, 116.4, 188, 0, 4, 112 },
        /* 17*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 116.4, 188, 0, 4, 110 },
        /* 18*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 276, 116.4, 262, 19, 4, 81 },
        /* 19*/ { BARCODE_UPCA, 0, "12345678901+12", 0, 50, 1, 124, 276, 116.4, 188, 0, 4, 100 },
        /* 20*/ { BARCODE_UPCA, 0, "12345678901+12", 0, 50, 1, 124, 276, 116.4, 262, 19, 4, 81 },
        /* 21*/ { BARCODE_UPCA, 9, "12345678901+12", 0, 50, 1, 124, 276, 118, 188, 0, 4, 118 },
        /* 22*/ { BARCODE_UPCA, 9, "12345678901+12", 0, 50, 1, 124, 276, 118, 262, 19, 4, 81 },
        /* 23*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 110 },
        /* 24*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 100 },
        /* 25*/ { BARCODE_EANX, 7, "123456789012", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 114 },
        /* 26*/ { BARCODE_EANX, -1, "123456789012+12", 0, 50, 1, 122, 276, 116.4, 22, 0, 2, 110 },
        /* 27*/ { BARCODE_EANX, -1, "123456789012+12", 0, 50, 1, 122, 276, 116.4, 262, 19, 4, 91 },
        /* 28*/ { BARCODE_EANX, 0, "123456789012+12", 0, 50, 1, 122, 276, 116.4, 22, 0, 2, 100 },
        /* 29*/ { BARCODE_EANX, 0, "123456789012+12", 0, 50, 1, 122, 276, 116.4, 262, 19, 4, 81 },
        /* 30*/ { BARCODE_EANX, 8.21, "123456789012+12", 0, 50, 1, 122, 276, 116.42, 22, 0, 2, 116.42 },
        /* 31*/ { BARCODE_EANX, 8.21, "123456789012+12", 0, 50, 1, 122, 276, 116.42, 262, 19, 4, 97.42 },
        /* 32*/ { BARCODE_ISBNX, -1, "123456789", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 110 },
        /* 33*/ { BARCODE_ISBNX, 0, "123456789", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 100 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_vector_rect *rect;

    testStart("test_guard_descent");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].guard_descent != -1.0f) {
            symbol->guard_descent = data[i].guard_descent;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < ZINT_ERROR) {
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) { /* ZINT_DEBUG_TEST_PRINT 16 */
                sprintf(symbol->outfile, "test_guard_descent_%d.svg", i);
                ZBarcode_Print(symbol, 0);
            }

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

            rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
            assert_nonnull(rect, "i:%d (%d) find_rect(%g, %g, %g, %g) NULL\n",
                                    i, data[i].symbology, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
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
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
        float expected_set_x;
        float expected_set_y;
        float expected_set_width;
        float expected_set_height;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, -1, -1, "1234", 0, 50, 1, 62, 124, 118.9, 0, 0, 2, 100 },
        /*  1*/ { BARCODE_CODE11, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 62, 164, 118.9, 20, 0, 2, 100 },
        /*  2*/ { BARCODE_CODE11, BARCODE_QUIET_ZONES | BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 62, 124, 118.9, 0, 0, 2, 100 }, /* BARCODE_NO_QUIET_ZONES trumps BARCODE_QUIET_ZONES */
        /*  3*/ { BARCODE_C25STANDARD, -1, -1, -1, "1234", 0, 50, 1, 57, 114, 118.9, 0, 0, 8, 100 },
        /*  4*/ { BARCODE_C25STANDARD, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 57, 154, 118.9, 20, 0, 8, 100 },
        /*  5*/ { BARCODE_C25INTER, -1, -1, -1, "1234", 0, 50, 1, 45, 90, 118.9, 0, 0, 2, 100 },
        /*  6*/ { BARCODE_C25INTER, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 45, 130, 118.9, 20, 0, 2, 100 },
        /*  7*/ { BARCODE_C25IATA, -1, -1, -1, "1234", 0, 50, 1, 65, 130, 118.9, 0, 0, 2, 100 },
        /*  8*/ { BARCODE_C25IATA, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 65, 170, 118.9, 20, 0, 2, 100 },
        /*  9*/ { BARCODE_C25LOGIC, -1, -1, -1, "1234", 0, 50, 1, 49, 98, 118.9, 0, 0, 2, 100 },
        /* 10*/ { BARCODE_C25LOGIC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 49, 138, 118.9, 20, 0, 2, 100 },
        /* 11*/ { BARCODE_C25IND, -1, -1, -1, "1234", 0, 50, 1, 75, 150, 118.9, 0, 0, 6, 100 },
        /* 12*/ { BARCODE_C25IND, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 75, 190, 118.9, 20, 0, 6, 100 },
        /* 13*/ { BARCODE_CODE39, -1, -1, -1, "1234", 0, 50, 1, 77, 154, 118.9, 0, 0, 2, 100 },
        /* 14*/ { BARCODE_CODE39, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 77, 194, 118.9, 20, 0, 2, 100 },
        /* 15*/ { BARCODE_EXCODE39, -1, -1, -1, "1234", 0, 50, 1, 77, 154, 118.9, 0, 0, 2, 100 },
        /* 16*/ { BARCODE_EXCODE39, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 77, 194, 118.9, 20, 0, 2, 100 },
        /* 17*/ { BARCODE_EANX, -1, -1, -1, "023456789012", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 110 },
        /* 18*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "023456789012", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 110 },
        /* 19*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "023456789012", 0, 50, 1, 95, 212, 116.4, 22, 0, 2, 110 },
        /* 20*/ { BARCODE_EANX, -1, -1, 0, "023456789012", 0, 50, 1, 95, 226, 110, 22, 0, 2, 110 }, /* Hide text */
        /* 21*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, 0, "023456789012", 0, 50, 1, 95, 226, 110, 22, 0, 2, 110 }, /* Hide text */
        /* 22*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, 0, "023456789012", 0, 50, 1, 95, 190, 110, 0, 0, 2, 110 }, /* Hide text */
        /* 23*/ { BARCODE_EANX, -1, -1, -1, "023456789012+12", 0, 50, 1, 122, 276, 116.4, 262, 19, 4, 91 },
        /* 24*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "023456789012+12", 0, 50, 1, 122, 276, 116.4, 262, 19, 4, 91 },
        /* 25*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "023456789012+12", 0, 50, 1, 122, 266, 116.4, 262, 19, 4, 91 },
        /* 26*/ { BARCODE_EANX, -1, -1, 0, "023456789012+12", 0, 50, 1, 122, 276, 110, 262, 19, 4, 91 }, /* Hide text */
        /* 27*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, 0, "023456789012+12", 0, 50, 1, 122, 276, 110, 262, 19, 4, 91 }, /* Hide text */
        /* 28*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, 0, "023456789012+12", 0, 50, 1, 122, 244, 110, 240, 19, 4, 91 }, /* Hide text */
        /* 29*/ { BARCODE_EANX_CHK, -1, -1, -1, "0234567890129+12345", 0, 50, 1, 149, 330, 116.4, 318, 19, 2, 91 },
        /* 30*/ { BARCODE_EANX_CHK, BARCODE_QUIET_ZONES, -1, -1, "0234567890129+12345", 0, 50, 1, 149, 330, 116.4, 318, 19, 2, 91 },
        /* 31*/ { BARCODE_EANX_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, "0234567890129+12345", 0, 50, 1, 149, 320, 116.4, 318, 19, 2, 91 },
        /* 32*/ { BARCODE_EANX, -1, -1, -1, "0234567", 0, 50, 1, 67, 162, 116.4, 14, 0, 2, 110 }, /* EAN-8 */
        /* 33*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "0234567", 0, 50, 1, 67, 162, 116.4, 14, 0, 2, 110 }, /* EAN-8 */
        /* 34*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "0234567", 0, 50, 1, 67, 134, 116.4, 0, 0, 2, 110 }, /* EAN-8 */
        /* 35*/ { BARCODE_EANX, -1, -1, -1, "02345", 0, 50, 1, 47, 118, 116.4, 14, 0, 2, 100 }, /* EAN-5 */
        /* 36*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "02345", 0, 50, 1, 47, 118, 116.4, 14, 0, 2, 100 }, /* EAN-5 */
        /* 37*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "02345", 0, 50, 1, 47, 94, 116.4, 0, 0, 2, 100 }, /* EAN-5 */
        /* 38*/ { BARCODE_EANX, -1, -1, -1, "02", 0, 50, 1, 20, 64, 116.4, 14, 0, 2, 100 }, /* EAN-2 */
        /* 39*/ { BARCODE_EANX, BARCODE_QUIET_ZONES, -1, -1, "02", 0, 50, 1, 20, 64, 116.4, 14, 0, 2, 100 }, /* EAN-2 */
        /* 40*/ { BARCODE_EANX, BARCODE_NO_QUIET_ZONES, -1, -1, "02", 0, 50, 1, 20, 40, 116.4, 0, 0, 2, 100 }, /* EAN-2 */
        /* 41*/ { BARCODE_GS1_128, -1, -1, -1, "[20]02", 0, 50, 1, 68, 136, 118.9, 0, 0, 4, 100 },
        /* 42*/ { BARCODE_GS1_128, BARCODE_QUIET_ZONES, -1, -1, "[20]02", 0, 50, 1, 68, 176, 118.9, 20, 0, 4, 100 },
        /* 43*/ { BARCODE_CODABAR, -1, -1, -1, "A0B", 0, 50, 1, 32, 64, 118.9, 0, 0, 2, 100 },
        /* 44*/ { BARCODE_CODABAR, BARCODE_QUIET_ZONES, -1, -1, "A0B", 0, 50, 1, 32, 104, 118.9, 20, 0, 2, 100 },
        /* 45*/ { BARCODE_CODE128, -1, -1, -1, "1234", 0, 50, 1, 57, 114, 118.9, 0, 0, 4, 100 },
        /* 46*/ { BARCODE_CODE128, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 57, 154, 118.9, 20, 0, 4, 100 },
        /* 47*/ { BARCODE_DPLEIT, -1, -1, -1, "1234", 0, 50, 1, 135, 270, 118.9, 0, 0, 2, 100 },
        /* 48*/ { BARCODE_DPLEIT, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 135, 310, 118.9, 20, 0, 2, 100 },
        /* 49*/ { BARCODE_DPIDENT, -1, -1, -1, "1234", 0, 50, 1, 117, 234, 118.9, 0, 0, 2, 100 },
        /* 50*/ { BARCODE_DPIDENT, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 117, 274, 118.9, 20, 0, 2, 100 },
        /* 51*/ { BARCODE_CODE16K, -1, -1, -1, "1234", 0, 20, 2, 70, 162, 44, 20, 2, 6, 19 },
        /* 52*/ { BARCODE_CODE16K, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 70, 162, 44, 20, 2, 6, 19 },
        /* 53*/ { BARCODE_CODE16K, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 70, 140, 44, 0, 2, 6, 19 },
        /* 54*/ { BARCODE_CODE49, -1, -1, -1, "1234", 0, 20, 2, 70, 162, 44, 20, 2, 2, 19 },
        /* 55*/ { BARCODE_CODE49, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 70, 162, 44, 20, 2, 2, 19 },
        /* 56*/ { BARCODE_CODE49, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 70, 140, 44, 0, 2, 2, 19 },
        /* 57*/ { BARCODE_CODE93, -1, -1, -1, "1234", 0, 50, 1, 73, 146, 118.9, 0, 0, 2, 100 },
        /* 58*/ { BARCODE_CODE93, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 73, 186, 118.9, 20, 0, 2, 100 },
        /* 59*/ { BARCODE_FLAT, -1, -1, -1, "1234", 0, 50, 1, 36, 72, 100, 0, 0, 2, 100 },
        /* 60*/ { BARCODE_FLAT, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 36, 72, 100, 0, 0, 2, 100 },
        /* 61*/ { BARCODE_FLAT, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 36, 72, 100, 0, 0, 2, 100 },
        /* 62*/ { BARCODE_DBAR_OMN, -1, -1, -1, "1234", 0, 50, 1, 96, 192, 118.9, 2, 0, 2, 100 },
        /* 63*/ { BARCODE_DBAR_OMN, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 96, 192, 118.9, 2, 0, 2, 100 },
        /* 64*/ { BARCODE_DBAR_OMN, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 96, 192, 118.9, 2, 0, 2, 100 },
        /* 65*/ { BARCODE_DBAR_LTD, -1, -1, -1, "1234", 0, 50, 1, 79, 158, 118.9, 2, 0, 2, 100 },
        /* 66*/ { BARCODE_DBAR_LTD, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 79, 158, 118.9, 2, 0, 2, 100 },
        /* 67*/ { BARCODE_DBAR_LTD, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 79, 158, 118.9, 2, 0, 2, 100 },
        /* 68*/ { BARCODE_DBAR_EXP, -1, -1, -1, "[20]02", 0, 34, 1, 102, 204, 86.900002, 2, 0, 2, 68 },
        /* 69*/ { BARCODE_DBAR_EXP, BARCODE_QUIET_ZONES, -1, -1, "[20]02", 0, 34, 1, 102, 204, 86.900002, 2, 0, 2, 68 },
        /* 70*/ { BARCODE_DBAR_EXP, BARCODE_NO_QUIET_ZONES, -1, -1, "[20]02", 0, 34, 1, 102, 204, 86.900002, 2, 0, 2, 68 },
        /* 71*/ { BARCODE_TELEPEN, -1, -1, -1, "1234", 0, 50, 1, 112, 224, 118.9, 0, 0, 2, 100 },
        /* 72*/ { BARCODE_TELEPEN, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 112, 264, 118.9, 20, 0, 2, 100 },
        /* 73*/ { BARCODE_UPCA, -1, -1, -1, "01457137763", 0, 50, 1, 95, 226, 116.4, 18, 0, 2, 110 },
        /* 74*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, "01457137763", 0, 50, 1, 95, 226, 116.4, 18, 0, 2, 110 },
        /* 75*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, "01457137763", 0, 50, 1, 95, 226, 116.4, 18, 0, 2, 110 },
        /* 76*/ { BARCODE_UPCA, -1, -1, 0, "01457137763", 0, 50, 1, 95, 226, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 77*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, 0, "01457137763", 0, 50, 1, 95, 226, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 78*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, 0, "01457137763", 0, 50, 1, 95, 190, 110, 0, 0, 2, 110 }, /* Hide text */
        /* 79*/ { BARCODE_UPCA, -1, -1, -1, "01457137763+12", 0, 50, 1, 124, 276, 116.4, 18, 0, 2, 110 },
        /* 80*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, -1, "01457137763+12", 0, 50, 1, 124, 276, 116.4, 18, 0, 2, 110 },
        /* 81*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, -1, "01457137763+12", 0, 50, 1, 124, 266, 116.4, 18, 0, 2, 110 },
        /* 82*/ { BARCODE_UPCA, -1, -1, 0, "01457137763+12", 0, 50, 1, 124, 276, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 83*/ { BARCODE_UPCA, BARCODE_QUIET_ZONES, -1, 0, "01457137763+12", 0, 50, 1, 124, 276, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 84*/ { BARCODE_UPCA, BARCODE_NO_QUIET_ZONES, -1, 0, "01457137763+12", 0, 50, 1, 124, 248, 110, 0, 0, 2, 110 }, /* Hide text */
        /* 85*/ { BARCODE_UPCA_CHK, -1, -1, -1, "014571377638+12345", 0, 50, 1, 151, 330, 116.4, 18, 0, 2, 110 },
        /* 86*/ { BARCODE_UPCA_CHK, BARCODE_QUIET_ZONES, -1, -1, "014571377638+12345", 0, 50, 1, 151, 330, 116.4, 18, 0, 2, 110 },
        /* 87*/ { BARCODE_UPCA_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, "014571377638+12345", 0, 50, 1, 151, 320, 116.4, 18, 0, 2, 110 },
        /* 88*/ { BARCODE_UPCA_CHK, -1, -1, 0, "014571377638+12345", 0, 50, 1, 151, 330, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 89*/ { BARCODE_UPCA_CHK, BARCODE_QUIET_ZONES, -1, 0, "014571377638+12345", 0, 50, 1, 151, 330, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 90*/ { BARCODE_UPCA_CHK, BARCODE_NO_QUIET_ZONES, -1, 0, "014571377638+12345", 0, 50, 1, 151, 302, 110, 0, 0, 2, 110 }, /* Hide text */
        /* 91*/ { BARCODE_UPCE, -1, -1, -1, "8145713", 0, 50, 1, 51, 134, 116.4, 18, 0, 2, 110 },
        /* 92*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, "8145713", 0, 50, 1, 51, 134, 116.4, 18, 0, 2, 110 },
        /* 93*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, "8145713", 0, 50, 1, 51, 134, 116.4, 18, 0, 2, 110 },
        /* 94*/ { BARCODE_UPCE, -1, -1, 0, "8145713", 0, 50, 1, 51, 134, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 95*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, 0, "8145713", 0, 50, 1, 51, 134, 110, 18, 0, 2, 110 }, /* Hide text */
        /* 96*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, 0, "8145713", 0, 50, 1, 51, 102, 110, 0, 0, 2, 110 }, /* Hide text */
        /* 97*/ { BARCODE_UPCE_CHK, -1, -1, -1, "81457132+12", 0, 50, 1, 78, 184, 116.4, 170, 19, 4, 81 },
        /* 98*/ { BARCODE_UPCE_CHK, BARCODE_QUIET_ZONES, -1, -1, "81457132+12", 0, 50, 1, 78, 184, 116.4, 170, 19, 4, 81 },
        /* 99*/ { BARCODE_UPCE_CHK, BARCODE_NO_QUIET_ZONES, -1, -1, "81457132+12", 0, 50, 1, 78, 174, 116.4, 170, 19, 4, 81 },
        /*100*/ { BARCODE_UPCE_CHK, -1, -1, 0, "81457132+12", 0, 50, 1, 78, 184, 110, 170, 19, 4, 81 }, /* Hide text */
        /*101*/ { BARCODE_UPCE_CHK, BARCODE_QUIET_ZONES, -1, 0, "81457132+12", 0, 50, 1, 78, 184, 110, 170, 19, 4, 81 }, /* Hide text */
        /*102*/ { BARCODE_UPCE_CHK, BARCODE_NO_QUIET_ZONES, -1, 0, "81457132+12", 0, 50, 1, 78, 156, 110, 152, 19, 4, 81 }, /* Hide text */
        /*103*/ { BARCODE_UPCE, -1, -1, -1, "8145713+12345", 0, 50, 1, 105, 238, 116.4, 226, 19, 2, 81 },
        /*104*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, -1, "8145713+12345", 0, 50, 1, 105, 238, 116.4, 226, 19, 2, 81 },
        /*105*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, -1, "8145713+12345", 0, 50, 1, 105, 228, 116.4, 226, 19, 2, 81 },
        /*106*/ { BARCODE_UPCE, -1, -1, 0, "8145713+12345", 0, 50, 1, 105, 238, 110, 226, 19, 2, 81 }, /* Hide text */
        /*107*/ { BARCODE_UPCE, BARCODE_QUIET_ZONES, -1, 0, "8145713+12345", 0, 50, 1, 105, 238, 110, 226, 19, 2, 81 }, /* Hide text */
        /*108*/ { BARCODE_UPCE, BARCODE_NO_QUIET_ZONES, -1, 0, "8145713+12345", 0, 50, 1, 105, 210, 110, 208, 19, 2, 81 }, /* Hide text */
        /*109*/ { BARCODE_POSTNET, -1, -1, -1, "12345", 0, 12, 2, 63, 126, 24, 0, 0, 2, 24 },
        /*110*/ { BARCODE_POSTNET, BARCODE_QUIET_ZONES, -1, -1, "12345", 0, 12, 2, 63, 146, 30.4, 10, 3.2, 2, 24 },
        /*111*/ { BARCODE_MSI_PLESSEY, -1, -1, -1, "1234", 0, 50, 1, 55, 110, 118.9, 0, 0, 4, 100 },
        /*112*/ { BARCODE_MSI_PLESSEY, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 55, 158, 118.9, 24, 0, 4, 100 },
        /*113*/ { BARCODE_FIM, -1, -1, -1, "A", 0, 50, 1, 17, 34, 100, 0, 0, 2, 100 },
        /*114*/ { BARCODE_FIM, BARCODE_QUIET_ZONES, -1, -1, "A", 0, 50, 1, 17, 50.955414, 100, 10.585987, 0, 2, 100 },
        /*115*/ { BARCODE_LOGMARS, -1, -1, -1, "1234", 0, 50, 1, 95, 190, 118.9, 0, 0, 2, 100 },
        /*116*/ { BARCODE_LOGMARS, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 95, 230, 118.9, 20, 0, 2, 100 },
        /*117*/ { BARCODE_PHARMA, -1, -1, -1, "1234", 0, 50, 1, 38, 76, 100, 0, 0, 2, 100 },
        /*118*/ { BARCODE_PHARMA, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 38, 100, 100, 12, 0, 2, 100 },
        /*119*/ { BARCODE_PZN, -1, -1, -1, "1234", 0, 50, 1, 142, 284, 118.9, 0, 0, 2, 100 },
        /*120*/ { BARCODE_PZN, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 142, 324, 118.9, 20, 0, 2, 100 },
        /*121*/ { BARCODE_PHARMA_TWO, -1, -1, -1, "1234", 0, 10, 2, 13, 26, 20, 8, 0, 2, 10 },
        /*122*/ { BARCODE_PHARMA_TWO, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 10, 2, 13, 50, 20, 20, 0, 2, 10 },
        /*123*/ { BARCODE_CEPNET, -1, -1, -1, "12345678", 0, 5.375, 2, 93, 186, 10.75, 0, 0, 2, 10.75 },
        /*124*/ { BARCODE_CEPNET, BARCODE_QUIET_ZONES, -1, -1, "12345678", 0, 5.375, 2, 93, 226, 17.15, 20, 3.2, 2, 10.75 },
        /*125*/ { BARCODE_PDF417, -1, -1, -1, "1234", 0, 18, 6, 103, 206, 36, 0, 0, 16, 36 },
        /*126*/ { BARCODE_PDF417, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 18, 6, 103, 214, 44, 4, 4, 16, 36 },
        /*127*/ { BARCODE_PDF417COMP, -1, -1, -1, "1234", 0, 18, 6, 69, 138, 36, 0, 0, 16, 36 },
        /*128*/ { BARCODE_PDF417COMP, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 18, 6, 69, 146, 44, 4, 4, 16, 36 },
        /*129*/ { BARCODE_MAXICODE, -1, -1, -1, "1234", 0, 165, 33, 30, 60, 57.733398, 29, 28.866699, 16.430941, 0 },
        /*130*/ { BARCODE_MAXICODE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 165, 33, 30, 64, 61.733398, 31, 30.866699, 16.430941, 0 },
        /*131*/ { BARCODE_QRCODE, -1, -1, -1, "1234", 0, 21, 21, 21, 42, 42, 0, 0, 14, 2 },
        /*132*/ { BARCODE_QRCODE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 21, 21, 21, 58, 58, 8, 8, 14, 2 },
        /*133*/ { BARCODE_CODE128B, -1, -1, -1, "1234", 0, 50, 1, 79, 158, 118.9, 0, 0, 4, 100 },
        /*134*/ { BARCODE_CODE128B, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 79, 198, 118.9, 20, 0, 4, 100 },
        /*135*/ { BARCODE_AUSPOST, -1, -1, -1, "12345678", 0, 8, 3, 73, 146, 16, 0, 0, 2, 10 },
        /*136*/ { BARCODE_AUSPOST, BARCODE_QUIET_ZONES, -1, -1, "12345678", 0, 8, 3, 73, 186, 29.333332, 20, 6.6666665, 2, 10 },
        /*137*/ { BARCODE_AUSREPLY, -1, -1, -1, "1234", 0, 8, 3, 73, 146, 16, 0, 0, 2, 10 },
        /*138*/ { BARCODE_AUSREPLY, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 73, 186, 29.333332, 20, 6.6666665, 2, 10 },
        /*139*/ { BARCODE_AUSROUTE, -1, -1, -1, "1234", 0, 8, 3, 73, 146, 16, 0, 0, 2, 10 },
        /*140*/ { BARCODE_AUSROUTE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 73, 186, 29.333332, 20, 6.6666665, 2, 10 },
        /*141*/ { BARCODE_AUSREDIRECT, -1, -1, -1, "1234", 0, 8, 3, 73, 146, 16, 0, 0, 2, 10 },
        /*142*/ { BARCODE_AUSREDIRECT, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 73, 186, 29.333332, 20, 6.6666665, 2, 10 },
        /*143*/ { BARCODE_ISBNX, -1, -1, -1, "123456789X", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 110 },
        /*144*/ { BARCODE_ISBNX, BARCODE_QUIET_ZONES, -1, -1, "123456789X", 0, 50, 1, 95, 226, 116.4, 22, 0, 2, 110 },
        /*145*/ { BARCODE_ISBNX, BARCODE_NO_QUIET_ZONES, -1, -1, "123456789X", 0, 50, 1, 95, 212, 116.4, 22, 0, 2, 110 },
        /*146*/ { BARCODE_RM4SCC, -1, -1, -1, "1234", 0, 8, 3, 43, 86, 16, 0, 0, 2, 10 },
        /*147*/ { BARCODE_RM4SCC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 43, 98.283463, 28.283464, 6.1417322, 6.1417322, 2, 10 },
        /*148*/ { BARCODE_DATAMATRIX, -1, -1, -1, "1234", 0, 10, 10, 10, 20, 20, 0, 0, 2, 2 },
        /*149*/ { BARCODE_DATAMATRIX, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 10, 10, 10, 24, 24, 2, 2, 2, 2 },
        /*150*/ { BARCODE_EAN14, -1, -1, -1, "1234", 0, 50, 1, 134, 268, 118.9, 0, 0, 4, 100 },
        /*151*/ { BARCODE_EAN14, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 134, 308, 118.9, 20, 0, 4, 100 },
        /*152*/ { BARCODE_VIN, -1, -1, -1, "12345678701234567", 0, 50, 1, 246, 492, 118.9, 0, 0, 2, 100 },
        /*153*/ { BARCODE_VIN, BARCODE_QUIET_ZONES, -1, -1, "12345678701234567", 0, 50, 1, 246, 532, 118.9, 20, 0, 2, 100 },
        /*154*/ { BARCODE_CODABLOCKF, -1, -1, -1, "1234", 0, 20, 2, 101, 242, 44, 20, 2, 4, 40 },
        /*155*/ { BARCODE_CODABLOCKF, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 101, 242, 44, 20, 2, 4, 40 },
        /*156*/ { BARCODE_CODABLOCKF, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 101, 202, 44, 0, 2, 4, 40 },
        /*157*/ { BARCODE_NVE18, -1, -1, -1, "1234", 0, 50, 1, 156, 312, 118.9, 0, 0, 4, 100 },
        /*158*/ { BARCODE_NVE18, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 156, 352, 118.9, 20, 0, 4, 100 },
        /*159*/ { BARCODE_JAPANPOST, -1, -1, -1, "1234", 0, 8, 3, 133, 266, 16, 0, 0, 2, 16 },
        /*160*/ { BARCODE_JAPANPOST, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 133, 279.33334, 29.333332, 6.6666665, 6.6666665, 2, 16 },
        /*161*/ { BARCODE_KOREAPOST, -1, -1, -1, "1234", 0, 50, 1, 167, 334, 118.9, 8, 0, 2, 100 },
        /*162*/ { BARCODE_KOREAPOST, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 167, 374, 118.9, 28, 0, 2, 100 },
        /*163*/ { BARCODE_DBAR_STK, -1, -1, -1, "1234", 0, 13, 3, 50, 100, 26, 2, 0, 2, 10 },
        /*164*/ { BARCODE_DBAR_STK, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 13, 3, 50, 100, 26, 2, 0, 2, 10 },
        /*165*/ { BARCODE_DBAR_STK, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 13, 3, 50, 100, 26, 2, 0, 2, 10 },
        /*166*/ { BARCODE_DBAR_OMNSTK, -1, -1, -1, "1234", 0, 69, 5, 50, 100, 138, 2, 0, 2, 66 },
        /*167*/ { BARCODE_DBAR_OMNSTK, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 69, 5, 50, 100, 138, 2, 0, 2, 66 },
        /*168*/ { BARCODE_DBAR_OMNSTK, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 69, 5, 50, 100, 138, 2, 0, 2, 66 },
        /*169*/ { BARCODE_DBAR_EXPSTK, -1, -1, -1, "[20]12", 0, 34, 1, 102, 204, 68, 2, 0, 2, 68 },
        /*170*/ { BARCODE_DBAR_EXPSTK, BARCODE_QUIET_ZONES, -1, -1, "[20]12", 0, 34, 1, 102, 204, 68, 2, 0, 2, 68 },
        /*171*/ { BARCODE_DBAR_EXPSTK, BARCODE_NO_QUIET_ZONES, -1, -1, "[20]12", 0, 34, 1, 102, 204, 68, 2, 0, 2, 68 },
        /*172*/ { BARCODE_PLANET, -1, -1, -1, "12345678901", 0, 12, 2, 123, 246, 24, 0, 0, 2, 24 },
        /*173*/ { BARCODE_PLANET, BARCODE_QUIET_ZONES, -1, -1, "12345678901", 0, 12, 2, 123, 266, 30.4, 10, 3.2, 2, 24 },
        /*174*/ { BARCODE_MICROPDF417, -1, -1, -1, "1234", 0, 22, 11, 38, 76, 44, 0, 0, 4, 4 },
        /*175*/ { BARCODE_MICROPDF417, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 22, 11, 38, 80, 48, 2, 2, 4, 4 },
        /*176*/ { BARCODE_USPS_IMAIL, -1, -1, -1, "12345678901234567890", 0, 8, 3, 129, 258, 16, 0, 0, 2, 10 },
        /*177*/ { BARCODE_USPS_IMAIL, BARCODE_QUIET_ZONES, -1, -1, "12345678901234567890", 0, 8, 3, 129, 277.5, 20.056, 9.75, 2.0280001, 2, 10 },
        /*178*/ { BARCODE_PLESSEY, -1, -1, -1, "1234", 0, 50, 1, 131, 262, 118.9, 0, 0, 6, 100 },
        /*179*/ { BARCODE_PLESSEY, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 131, 310, 118.9, 24, 0, 6, 100 },
        /*180*/ { BARCODE_TELEPEN_NUM, -1, -1, -1, "1234", 0, 50, 1, 80, 160, 118.9, 0, 0, 2, 100 },
        /*181*/ { BARCODE_TELEPEN_NUM, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 80, 200, 118.9, 20, 0, 2, 100 },
        /*182*/ { BARCODE_ITF14, -1, -1, -1, "1234", 0, 50, 1, 135, 330, 138.89999, 30, 10, 2, 100 },
        /*183*/ { BARCODE_ITF14, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 135, 330, 138.89999, 30, 10, 2, 100 },
        /*184*/ { BARCODE_ITF14, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 135, 290, 138.89999, 10, 10, 2, 100 },
        /*185*/ { BARCODE_KIX, -1, -1, -1, "1234", 0, 8, 3, 31, 62, 16, 8, 0, 2, 10 },
        /*186*/ { BARCODE_KIX, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 8, 3, 31, 74.283463, 28.283464, 14.141732, 6.1417322, 2, 10 },
        /*187*/ { BARCODE_AZTEC, -1, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 6, 0, 6, 2 },
        /*188*/ { BARCODE_AZTEC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 6, 0, 6, 2 },
        /*189*/ { BARCODE_AZTEC, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 6, 0, 6, 2 },
        /*190*/ { BARCODE_DAFT, -1, -1, -1, "FADT", 0, 8, 3, 7, 14, 16, 0, 0, 2, 16 },
        /*191*/ { BARCODE_DAFT, BARCODE_QUIET_ZONES, -1, -1, "FADT", 0, 8, 3, 7, 14, 16, 0, 0, 2, 16 },
        /*192*/ { BARCODE_DAFT, BARCODE_NO_QUIET_ZONES, -1, -1, "FADT", 0, 8, 3, 7, 14, 16, 0, 0, 2, 16 },
        /*193*/ { BARCODE_DPD, -1, -1, -1, "1234567890123456789012345678", 0, 50, 1, 189, 378, 118.9, 0, 0, 4, 100 },
        /*194*/ { BARCODE_DPD, BARCODE_QUIET_ZONES, -1, -1, "1234567890123456789012345678", 0, 50, 1, 189, 428, 118.9, 25, 0, 4, 100 },
        /*195*/ { BARCODE_MICROQR, -1, -1, -1, "1234", 0, 11, 11, 11, 22, 22, 0, 0, 14, 2 },
        /*196*/ { BARCODE_MICROQR, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 11, 11, 11, 30, 30, 4, 4, 14, 2 },
        /*197*/ { BARCODE_HIBC_128, -1, -1, -1, "1234", 0, 50, 1, 90, 180, 118.9, 0, 0, 4, 100 },
        /*198*/ { BARCODE_HIBC_128, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 90, 220, 118.9, 20, 0, 4, 100 },
        /*199*/ { BARCODE_HIBC_39, -1, -1, -1, "1234", 0, 50, 1, 127, 254, 118.9, 0, 0, 2, 100 },
        /*200*/ { BARCODE_HIBC_39, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 127, 294, 118.9, 20, 0, 2, 100 },
        /*201*/ { BARCODE_HIBC_DM, -1, -1, -1, "1234", 0, 12, 12, 12, 24, 24, 0, 0, 2, 2 },
        /*202*/ { BARCODE_HIBC_DM, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 12, 12, 12, 28, 28, 2, 2, 2, 2 },
        /*203*/ { BARCODE_HIBC_QR, -1, -1, -1, "1234", 0, 21, 21, 21, 42, 42, 0, 0, 14, 2 },
        /*204*/ { BARCODE_HIBC_QR, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 21, 21, 21, 58, 58, 8, 8, 14, 2 },
        /*205*/ { BARCODE_HIBC_PDF, -1, -1, -1, "1234", 0, 21, 7, 103, 206, 42, 0, 0, 16, 42 },
        /*206*/ { BARCODE_HIBC_PDF, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 21, 7, 103, 214, 50, 4, 4, 16, 42 },
        /*207*/ { BARCODE_HIBC_MICPDF, -1, -1, -1, "1234", 0, 12, 6, 82, 164, 24, 0, 0, 4, 4 },
        /*208*/ { BARCODE_HIBC_MICPDF, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 12, 6, 82, 168, 28, 2, 2, 4, 4 },
        /*209*/ { BARCODE_HIBC_BLOCKF, -1, -1, -1, "1234", 0, 20, 2, 101, 242, 44, 20, 2, 4, 40 },
        /*210*/ { BARCODE_HIBC_BLOCKF, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 101, 242, 44, 20, 2, 4, 40 },
        /*211*/ { BARCODE_HIBC_BLOCKF, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 20, 2, 101, 202, 44, 0, 2, 4, 40 },
        /*212*/ { BARCODE_HIBC_AZTEC, -1, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 22, 0, 2, 2 },
        /*213*/ { BARCODE_HIBC_AZTEC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 22, 0, 2, 2 },
        /*214*/ { BARCODE_HIBC_AZTEC, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 15, 15, 15, 30, 30, 22, 0, 2, 2 },
        /*215*/ { BARCODE_DOTCODE, -1, -1, -1, "1234", 0, 10, 10, 13, 26, 20, 5, 1, 1.6, 0 },
        /*216*/ { BARCODE_DOTCODE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 10, 10, 13, 38, 32, 11, 7, 1.6, 0 },
        /*217*/ { BARCODE_HANXIN, -1, -1, -1, "1234", 0, 23, 23, 23, 46, 46, 0, 0, 14, 2 },
        /*218*/ { BARCODE_HANXIN, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 23, 23, 23, 58, 58, 6, 6, 14, 2 },
        /*219*/ { BARCODE_MAILMARK, -1, -1, -1, "01000000000000000AA00AA0A", 0, 10, 3, 155, 310, 20, 0, 0, 2, 20 },
        /*220*/ { BARCODE_MAILMARK, BARCODE_QUIET_ZONES, -1, -1, "01000000000000000AA00AA0A", 0, 10, 3, 155, 322.28348, 32.283463, 6.1417322, 6.1417322, 2, 20 },
        /*221*/ { BARCODE_AZRUNE, -1, -1, -1, "123", 0, 11, 11, 11, 22, 22, 0, 0, 8, 2 },
        /*222*/ { BARCODE_AZRUNE, BARCODE_QUIET_ZONES, -1, -1, "123", 0, 11, 11, 11, 22, 22, 0, 0, 8, 2 },
        /*223*/ { BARCODE_AZRUNE, BARCODE_NO_QUIET_ZONES, -1, -1, "123", 0, 11, 11, 11, 22, 22, 0, 0, 8, 2 },
        /*224*/ { BARCODE_CODE32, -1, -1, -1, "1234", 0, 50, 1, 103, 206, 118.9, 0, 0, 2, 100 },
        /*225*/ { BARCODE_CODE32, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 103, 246, 118.9, 20, 0, 2, 100 },
        /*226*/ { BARCODE_EANX_CC, -1, -1, -1, "023456789012", 0, 50, 7, 99, 234, 116.4, 32, 24, 2, 86 },
        /*227*/ { BARCODE_EANX_CC, BARCODE_QUIET_ZONES, -1, -1, "023456789012", 0, 50, 7, 99, 234, 116.4, 32, 24, 2, 86 },
        /*228*/ { BARCODE_EANX_CC, BARCODE_NO_QUIET_ZONES, -1, -1, "023456789012", 0, 50, 7, 99, 220, 116.4, 32, 24, 2, 86 },
        /*229*/ { BARCODE_EANX_CC, -1, -1, 0, "023456789012", 0, 50, 7, 99, 234, 110, 32, 24, 2, 86 }, /* Hide text */
        /*230*/ { BARCODE_EANX_CC, BARCODE_QUIET_ZONES, -1, 0, "023456789012", 0, 50, 7, 99, 234, 110, 32, 24, 2, 86 }, /* Hide text */
        /*231*/ { BARCODE_EANX_CC, BARCODE_NO_QUIET_ZONES, -1, 0, "023456789012", 0, 50, 7, 99, 198, 110, 10, 24, 2, 86 }, /* Hide text */
        /*232*/ { BARCODE_GS1_128_CC, -1, -1, -1, "[20]02", 0, 50, 5, 99, 198, 118.9, 24, 14, 4, 86 },
        /*233*/ { BARCODE_GS1_128_CC, BARCODE_QUIET_ZONES, -1, -1, "[20]02", 0, 50, 5, 99, 238, 118.9, 44, 14, 4, 86 },
        /*234*/ { BARCODE_DBAR_OMN_CC, -1, -1, -1, "1234", 0, 21, 5, 100, 200, 60.900002, 10, 14, 2, 28 },
        /*235*/ { BARCODE_DBAR_OMN_CC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 21, 5, 100, 204, 60.900002, 12, 14, 2, 28 },
        /*236*/ { BARCODE_DBAR_LTD_CC, -1, -1, -1, "1234", 0, 19, 6, 79, 158, 56.900002, 2, 18, 2, 20 },
        /*237*/ { BARCODE_DBAR_LTD_CC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 19, 6, 79, 162, 56.900002, 4, 18, 2, 20 },
        /*238*/ { BARCODE_DBAR_EXP_CC, -1, -1, -1, "[20]12", 0, 41, 5, 102, 204, 100.9, 2, 14, 2, 68 },
        /*239*/ { BARCODE_DBAR_EXP_CC, BARCODE_QUIET_ZONES, -1, -1, "[20]12", 0, 41, 5, 102, 208, 100.9, 4, 14, 2, 68 },
        /*240*/ { BARCODE_UPCA_CC, -1, -1, -1, "01457137763", 0, 50, 7, 99, 234, 116.4, 24, 20, 2, 90 },
        /*241*/ { BARCODE_UPCA_CC, BARCODE_QUIET_ZONES, -1, -1, "01457137763", 0, 50, 7, 99, 234, 116.4, 24, 20, 2, 90 },
        /*242*/ { BARCODE_UPCA_CC, BARCODE_NO_QUIET_ZONES, -1, -1, "01457137763", 0, 50, 7, 99, 234, 116.4, 24, 20, 2, 90 },
        /*243*/ { BARCODE_UPCA_CC, -1, -1, 0, "01457137763", 0, 50, 7, 99, 234, 110, 24, 20, 2, 90 }, /* Hide text */
        /*244*/ { BARCODE_UPCA_CC, BARCODE_QUIET_ZONES, -1, 0, "01457137763", 0, 50, 7, 99, 234, 110, 24, 20, 2, 90 }, /* Hide text */
        /*245*/ { BARCODE_UPCA_CC, BARCODE_NO_QUIET_ZONES, -1, 0, "01457137763", 0, 50, 7, 99, 198, 110, 6, 20, 2, 90 }, /* Hide text */
        /*246*/ { BARCODE_UPCE_CC, -1, -1, -1, "8145713", 0, 50, 9, 55, 142, 116.4, 24, 28, 2, 82 },
        /*247*/ { BARCODE_UPCE_CC, BARCODE_QUIET_ZONES, -1, -1, "8145713", 0, 50, 9, 55, 142, 116.4, 24, 28, 2, 82 },
        /*248*/ { BARCODE_UPCE_CC, BARCODE_NO_QUIET_ZONES, -1, -1, "8145713", 0, 50, 9, 55, 142, 116.4, 24, 28, 2, 82 },
        /*249*/ { BARCODE_UPCE_CC, -1, -1, 0, "8145713", 0, 50, 9, 55, 142, 110, 24, 28, 2, 82 }, /* Hide text */
        /*250*/ { BARCODE_UPCE_CC, BARCODE_QUIET_ZONES, -1, 0, "8145713", 0, 50, 9, 55, 142, 110, 24, 28, 2, 82 }, /* Hide text */
        /*251*/ { BARCODE_UPCE_CC, BARCODE_NO_QUIET_ZONES, -1, 0, "8145713", 0, 50, 9, 55, 110, 110, 6, 28, 2, 82 }, /* Hide text */
        /*252*/ { BARCODE_DBAR_STK_CC, -1, -1, -1, "1234", 0, 24, 9, 56, 112, 48, 0, 34, 2, 14 },
        /*253*/ { BARCODE_DBAR_STK_CC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 24, 9, 56, 116, 48, 2, 34, 2, 14 },
        /*254*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, -1, "1234", 0, 80, 11, 56, 112, 160, 0, 94, 2, 66 },
        /*255*/ { BARCODE_DBAR_OMNSTK_CC, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 80, 11, 56, 116, 160, 2, 94, 2, 66 },
        /*256*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, -1, "[20]12", 0, 41, 5, 102, 204, 82, 2, 14, 2, 68 },
        /*257*/ { BARCODE_DBAR_EXPSTK_CC, BARCODE_QUIET_ZONES, -1, -1, "[20]12", 0, 41, 5, 102, 208, 82, 4, 14, 2, 68 },
        /*258*/ { BARCODE_CHANNEL, -1, -1, -1, "1234", 0, 50, 1, 27, 54, 118.9, 0, 0, 2, 100 },
        /*259*/ { BARCODE_CHANNEL, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 50, 1, 27, 60, 118.9, 2, 0, 2, 100 },
        /*260*/ { BARCODE_CODEONE, -1, -1, -1, "1234", 0, 16, 16, 18, 36, 32, 0, 0, 2, 2 }, /* Versions A to H - no quiet zone */
        /*261*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 16, 16, 18, 36, 32, 0, 0, 2, 2 },
        /*262*/ { BARCODE_CODEONE, BARCODE_NO_QUIET_ZONES, -1, -1, "1234", 0, 16, 16, 18, 36, 32, 0, 0, 2, 2 },
        /*263*/ { BARCODE_CODEONE, -1, 9, -1, "1234", 0, 8, 8, 11, 22, 16, 10, 0, 2, 2 }, /* Version S (& T) have quiet zones */
        /*264*/ { BARCODE_CODEONE, BARCODE_QUIET_ZONES, 9, -1, "1234", 0, 8, 8, 11, 26, 16, 12, 0, 2, 2 },
        /*265*/ { BARCODE_GRIDMATRIX, -1, -1, -1, "123", 0, 18, 18, 18, 36, 36, 0, 0, 12, 2 },
        /*266*/ { BARCODE_GRIDMATRIX, BARCODE_QUIET_ZONES, -1, -1, "123", 0, 18, 18, 18, 60, 60, 12, 12, 12, 2 },
        /*267*/ { BARCODE_UPNQR, -1, -1, -1, "1234", 0, 77, 77, 77, 154, 154, 0, 0, 14, 2 },
        /*268*/ { BARCODE_UPNQR, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 77, 77, 77, 170, 170, 8, 8, 14, 2 },
        /*269*/ { BARCODE_ULTRA, -1, -1, -1, "1234", 0, 13, 13, 15, 30, 26, 0, 0, 30, 2 },
        /*270*/ { BARCODE_ULTRA, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 13, 13, 15, 34, 30, 2, 2, 30, 2 },
        /*271*/ { BARCODE_RMQR, -1, -1, -1, "1234", 0, 11, 11, 27, 54, 22, 0, 0, 14, 2 },
        /*272*/ { BARCODE_RMQR, BARCODE_QUIET_ZONES, -1, -1, "1234", 0, 11, 11, 27, 62, 30, 4, 4, 14, 2 },
        /*273*/ { BARCODE_BC412, -1, -1, -1, "1234567", 0, 16.666668, 1, 102, 204, 52.233337, 0, 0, 2, 33.333336 },
        /*274*/ { BARCODE_BC412, BARCODE_QUIET_ZONES, -1, -1, "1234567", 0, 16.666668, 1, 102, 244, 52.233337, 20, 0, 2, 33.333336 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    const char *text;
    static const char composite[] = "[20]12";

    struct zint_vector_rect *rect;
    struct zint_vector_circle *circle;

    testStart("test_quiet_zones");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, data[i].data, -1, debug);
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
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < ZINT_ERROR) {
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            if (index != -1 && (debug & ZINT_DEBUG_TEST_PRINT)) { /* ZINT_DEBUG_TEST_PRINT 16 */
                sprintf(symbol->outfile, "test_quiet_zones_%d.svg", i);
                ZBarcode_Print(symbol, 0);
            }

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %.8g != %.8g\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

            if (symbol->symbology == BARCODE_MAXICODE || symbol->symbology == BARCODE_DOTCODE) {
                circle = find_circle(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width);
                assert_nonnull(circle, "i:%d (%d) find_circle(%g, %g, %g) NULL\n",
                                        i, data[i].symbology, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width);
            } else {
                rect = find_rect(symbol, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
                assert_nonnull(rect, "i:%d (%d) find_rect(%g, %g, %g, %g) NULL\n",
                                        i, data[i].symbology, data[i].expected_set_x, data[i].expected_set_y, data[i].expected_set_width, data[i].expected_set_height);
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
        float expected_vector_width;
        float expected_vector_height;

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
        /* 10*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 15, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15, 1, 99, 198, 30, "" },
        /* 11*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 15.5, "1234567890", "", 0, 15.5, 1, 99, 198, 31, "" },
        /* 12*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 17.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 17.5, 1, 117, 234, 35, "" },
        /* 13*/ { BARCODE_C25INTER, COMPLIANT_HEIGHT, 17.75, "12345678901", "", 0, 17.75, 1, 117, 234, 35.5, "" },
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
        /* 26*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 4.4, "1", "", 0, 4.4000001, 1, 38, 76, 8.8000002, "" },
        /* 27*/ { BARCODE_CODE39, -1, 17, "1234567890", "", 0, 17, 1, 155, 310, 34, "" },
        /* 28*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 17, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17, 1, 155, 310, 34, "Min height data-length dependent" },
        /* 29*/ { BARCODE_CODE39, COMPLIANT_HEIGHT, 17.85, "1234567890", "", 0, 17.85, 1, 155, 310, 35.700001, "" },
        /* 30*/ { BARCODE_EXCODE39, -1, 1, "1234567890", "", 0, 1, 1, 155, 310, 2, "" },
        /* 31*/ { BARCODE_EXCODE39, -1, 17.8, "1234567890", "", 0, 17.799999, 1, 155, 310, 35.599998, "" },
        /* 32*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, 17.8, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17.799999, 1, 155, 310, 35.599998, "" },
        /* 33*/ { BARCODE_EXCODE39, COMPLIANT_HEIGHT, 17.9, "1234567890", "", 0, 17.9, 1, 155, 310, 35.799999, "" },
        /* 34*/ { BARCODE_EANX, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 35*/ { BARCODE_EANX, -1, 69, "123456789012", "", 0, 69, 1, 95, 226, 148, "" },
        /* 36*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /* 37*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 69.25, "123456789012", "", 0, 69.25, 1, 95, 226, 148.5, "" },
        /* 38*/ { BARCODE_EANX, -1, 55, "1234567", "", 0, 55, 1, 67, 162, 120, "EAN-8" },
        /* 39*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 55, "1234567", "", ZINT_WARN_NONCOMPLIANT, 55, 1, 67, 162, 120, "EAN-8" },
        /* 40*/ { BARCODE_EANX, COMPLIANT_HEIGHT, 55.25, "1234567", "", 0, 55.25, 1, 67, 162, 120.5, "EAN-8" },
        /* 41*/ { BARCODE_EANX_CHK, -1, 1, "1234567890128", "", 0, 1, 1, 95, 226, 12, "EAN-13" },
        /* 42*/ { BARCODE_EANX_CHK, -1, 69, "1234567890128", "", 0, 69, 1, 95, 226, 148, "" },
        /* 43*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, 69, "1234567890128", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /* 44*/ { BARCODE_EANX_CHK, COMPLIANT_HEIGHT, 69.25, "1234567890128", "", 0, 69.25, 1, 95, 226, 148.5, "" },
        /* 45*/ { BARCODE_GS1_128, -1, 1, "[01]12345678901231", "", 0, 1, 1, 134, 268, 2, "" },
        /* 46*/ { BARCODE_GS1_128, -1, 5.7, "[01]12345678901231", "", 0, 5.6999998, 1, 134, 268, 11.4, "" },
        /* 47*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, 5.7, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 5.6999998, 1, 134, 268, 11.4, "" },
        /* 48*/ { BARCODE_GS1_128, COMPLIANT_HEIGHT, 5.725, "[01]12345678901231", "", 0, 5.7249999, 1, 134, 268, 11.45, "Note considered compliant even though rendered height same as before" },
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
        /* 68*/ { BARCODE_CODE16K, -1, 16.75, "1234567890", "", 0, 16.75, 2, 70, 162, 37.5, "" },
        /* 69*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 16.75, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16.75, 2, 70, 162, 37.5, "" },
        /* 70*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 71*/ { BARCODE_CODE16K, -1, -1, "12345678901234567890123456789012345678901234567890", "", 0, 60, 6, 70, 162, 124, "6 rows" },
        /* 72*/ { BARCODE_CODE16K, -1, 52.5, "12345678901234567890123456789012345678901234567890", "", 0, 52.5, 6, 70, 162, 109, "" },
        /* 73*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 52.5, "12345678901234567890123456789012345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 52.5, 6, 70, 162, 109, "" },
        /* 74*/ { BARCODE_CODE16K, COMPLIANT_HEIGHT, 53, "12345678901234567890123456789012345678901234567890", "", 0, 53, 6, 70, 162, 110, "" },
        /* 75*/ { BARCODE_CODE49, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "2 rows" },
        /* 76*/ { BARCODE_CODE49, -1, 1, "1234567890", "", 0, 1, 2, 70, 162, 6, "" },
        /* 77*/ { BARCODE_CODE49, -1, 16.75, "1234567890", "", 0, 16.75, 2, 70, 162, 37.5, "" },
        /* 78*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 16.75, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16.75, 2, 70, 162, 37.5, "" },
        /* 79*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 17, "1234567890", "", 0, 17, 2, 70, 162, 38, "" },
        /* 80*/ { BARCODE_CODE49, -1, -1, "12345678901234567890", "", 0, 30, 3, 70, 162, 64, "3 rows" },
        /* 81*/ { BARCODE_CODE49, -1, 25.75, "12345678901234567890", "", 0, 25.75, 3, 70, 162, 55.5, "" },
        /* 82*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 25.75, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 25.75, 3, 70, 162, 55.5, "" },
        /* 83*/ { BARCODE_CODE49, COMPLIANT_HEIGHT, 26, "12345678901234567890", "", 0, 26, 3, 70, 162, 56, "" },
        /* 84*/ { BARCODE_CODE93, -1, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "" },
        /* 85*/ { BARCODE_CODE93, -1, 9.9, "1", "", 0, 9.8999996, 1, 46, 92, 19.799999, "" },
        /* 86*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 9.9, "1", "", ZINT_WARN_NONCOMPLIANT, 9.8999996, 1, 46, 92, 19.799999, "Min height data-length dependent" },
        /* 87*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 10, "1", "", 0, 10, 1, 46, 92, 20, "" },
        /* 88*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 22, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 22, 1, 127, 254, 44, "Min height data-length dependent" },
        /* 89*/ { BARCODE_CODE93, COMPLIANT_HEIGHT, 22.1, "1234567890", "", 0, 22.1, 1, 127, 254, 44.200001, "" },
        /* 90*/ { BARCODE_FLAT, -1, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "" },
        /* 91*/ { BARCODE_FLAT, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 90, 180, 2, "TODO: Find doc" },
        /* 92*/ { BARCODE_FLAT, -1, 4, "1234567890", "", 0, 4, 1, 90, 180, 8, "" },
        /* 93*/ { BARCODE_DBAR_OMN, -1, 1, "1234567890123", "", 0, 1, 1, 96, 192, 2, "" },
        /* 94*/ { BARCODE_DBAR_OMN, -1, 12.9, "1234567890123", "", 0, 12.9, 1, 96, 192, 25.799999, "" },
        /* 95*/ { BARCODE_DBAR_OMN, COMPLIANT_HEIGHT, 12.9, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 12.9, 1, 96, 192, 25.799999, "" },
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
        /*111*/ { BARCODE_UPCA, COMPLIANT_HEIGHT, 69.25, "12345678901", "", 0, 69.25, 1, 95, 226, 148.5, "" },
        /*112*/ { BARCODE_UPCA_CHK, -1, 1, "123456789012", "", 0, 1, 1, 95, 226, 12, "" },
        /*113*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, 69, "123456789012", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /*114*/ { BARCODE_UPCA_CHK, COMPLIANT_HEIGHT, 69.5, "123456789012", "", 0, 69.5, 1, 95, 226, 149, "" },
        /*115*/ { BARCODE_UPCE, -1, 1, "1234567", "", 0, 1, 1, 51, 134, 12, "" },
        /*116*/ { BARCODE_UPCE, -1, 69, "1234567", "", 0, 69, 1, 51, 134, 148, "" },
        /*117*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, 69, "1234567", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 51, 134, 148, "" },
        /*118*/ { BARCODE_UPCE, COMPLIANT_HEIGHT, 69.25, "1234567", "", 0, 69.25, 1, 51, 134, 148.5, "" },
        /*119*/ { BARCODE_UPCE_CHK, -1, 1, "12345670", "", 0, 1, 1, 51, 134, 12, "" },
        /*120*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, 69.24, "12345670", "", ZINT_WARN_NONCOMPLIANT, 69.239998, 1, 51, 134, 148.48, "" },
        /*121*/ { BARCODE_UPCE_CHK, COMPLIANT_HEIGHT, 69.25, "12345670", "", 0, 69.25, 1, 51, 134, 148.5, "" },
        /*122*/ { BARCODE_POSTNET, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "" },
        /*123*/ { BARCODE_POSTNET, -1, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "" },
        /*124*/ { BARCODE_POSTNET, -1, 4.5, "12345678901", "", 0, 4.5, 2, 123, 246, 9, "" },
        /*125*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 4.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 123, 246, 9, "" },
        /*126*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 4.6, "12345678901", "", 0, 4.5999999, 2, 123, 246, 9.1999998, "" },
        /*127*/ { BARCODE_POSTNET, -1, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*128*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*129*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 9.1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 9.1000004, 2, 123, 246, 18.200001, "" },
        /*130*/ { BARCODE_POSTNET, -1, 20, "12345678901", "", 0, 20, 2, 123, 246, 40, "" },
        /*131*/ { BARCODE_POSTNET, COMPLIANT_HEIGHT, 20, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 20, 2, 123, 246, 40, "" },
        /*132*/ { BARCODE_MSI_PLESSEY, -1, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "" },
        /*133*/ { BARCODE_MSI_PLESSEY, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 127, 254, 2, "TODO: Find doc" },
        /*134*/ { BARCODE_MSI_PLESSEY, -1, 4, "1234567890", "", 0, 4, 1, 127, 254, 8, "" },
        /*135*/ { BARCODE_FIM, -1, 1, "A", "", 0, 1, 1, 17, 34, 2, "" },
        /*136*/ { BARCODE_FIM, -1, 12.7, "A", "", 0, 12.7, 1, 17, 34, 25.4, "" },
        /*137*/ { BARCODE_FIM, COMPLIANT_HEIGHT, 12.7, "A", "", ZINT_WARN_NONCOMPLIANT, 12.7, 1, 17, 34, 25.4, "" },
        /*138*/ { BARCODE_FIM, COMPLIANT_HEIGHT, 12.8, "A", "", 0, 12.8, 1, 17, 34, 25.6, "" },
        /*139*/ { BARCODE_LOGMARS, -1, 1, "1234567890", "", 0, 1, 1, 191, 382, 2, "" },
        /*140*/ { BARCODE_LOGMARS, -1, 6, "1234567890", "", 0, 6, 1, 191, 382, 12, "" },
        /*141*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 6, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6, 1, 191, 382, 12, "" },
        /*142*/ { BARCODE_LOGMARS, -1, 6.25, "1234567890", "", 0, 6.25, 1, 191, 382, 12.5, "" },
        /*143*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 6.25, "1234567890", "", 0, 6.25, 1, 191, 382, 12.5, "" },
        /*144*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 116, "1234567890", "", 0, 116, 1, 191, 382, 232, "" },
        /*145*/ { BARCODE_LOGMARS, COMPLIANT_HEIGHT, 117, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 117, 1, 191, 382, 234, "" },
        /*146*/ { BARCODE_PHARMA, -1, 1, "123456", "", 0, 1, 1, 58, 116, 2, "" },
        /*147*/ { BARCODE_PHARMA, -1, 15, "123456", "", 0, 15, 1, 58, 116, 30, "" },
        /*148*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, 15, "123456", "", ZINT_WARN_NONCOMPLIANT, 15, 1, 58, 116, 30, "" },
        /*149*/ { BARCODE_PHARMA, COMPLIANT_HEIGHT, 16, "123456", "", 0, 16, 1, 58, 116, 32, "" },
        /*150*/ { BARCODE_PZN, -1, 1, "123456", "", 0, 1, 1, 142, 284, 2, "" },
        /*151*/ { BARCODE_PZN, -1, 17.7, "123456", "", 0, 17.700001, 1, 142, 284, 35.400002, "" },
        /*152*/ { BARCODE_PZN, COMPLIANT_HEIGHT, 17.7, "123456", "", ZINT_WARN_NONCOMPLIANT, 17.700001, 1, 142, 284, 35.400002, "" },
        /*153*/ { BARCODE_PZN, COMPLIANT_HEIGHT, 17.8, "123456", "", 0, 17.799999, 1, 142, 284, 35.599998, "" },
        /*154*/ { BARCODE_PHARMA_TWO, -1, -1, "12345678", "", 0, 10, 2, 29, 58, 20, "" },
        /*155*/ { BARCODE_PHARMA_TWO, -1, 1, "12345678", "", 0, 1, 2, 29, 58, 2, "" },
        /*156*/ { BARCODE_PHARMA_TWO, -1, 3.9, "12345678", "", 0, 3.9000001, 2, 29, 58, 7.8000002, "" },
        /*157*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 3.9, "12345678", "", ZINT_WARN_NONCOMPLIANT, 3.9000001, 2, 29, 58, 7.8000002, "" },
        /*158*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 4, "12345678", "", 0, 4, 2, 29, 58, 8, "" },
        /*159*/ { BARCODE_PHARMA_TWO, -1, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "" },
        /*160*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 15, "12345678", "", 0, 15, 2, 29, 58, 30, "" },
        /*161*/ { BARCODE_PHARMA_TWO, COMPLIANT_HEIGHT, 15.1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 15.1, 2, 29, 58, 30.200001, "" },
        /*162*/ { BARCODE_CEPNET, -1, -1, "12345678", "", 0, 5.375, 2, 93, 186, 10.75, "" },
        /*163*/ { BARCODE_CEPNET, -1, 1, "12345678", "", 0, 1.25, 2, 93, 186, 2.5, "" },
        /*164*/ { BARCODE_CEPNET, -1, 4.5, "12345678", "", 0, 4.5, 2, 93, 186, 9, "" },
        /*165*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 4.5, "12345678", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 93, 186, 9, "" },
        /*166*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 4.6, "12345678", "", 0, 4.5999999, 2, 93, 186, 9.1999998, "" },
        /*167*/ { BARCODE_CEPNET, -1, 9, "12345678", "", 0, 9, 2, 93, 186, 18, "" },
        /*168*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 9, "12345678", "", 0, 9, 2, 93, 186, 18, "" },
        /*169*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 9.1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 9.1000004, 2, 93, 186, 18.200001, "" },
        /*170*/ { BARCODE_CEPNET, -1, 20, "12345678", "", 0, 20, 2, 93, 186, 40, "" },
        /*171*/ { BARCODE_CEPNET, COMPLIANT_HEIGHT, 20, "12345678", "", ZINT_WARN_NONCOMPLIANT, 20, 2, 93, 186, 40, "" },
        /*172*/ { BARCODE_PDF417, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "8 rows" },
        /*173*/ { BARCODE_PDF417, -1, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 4, 8, 120, 240, 8, "" },
        /*174*/ { BARCODE_PDF417, -1, 9, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 9, 8, 120, 240, 18, "" },
        /*175*/ { BARCODE_PDF417, -1, 10, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 10, 8, 120, 240, 20, "" },
        /*176*/ { BARCODE_PDF417, -1, 11, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 11, 8, 120, 240, 22, "" },
        /*177*/ { BARCODE_PDF417, -1, 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 8, 120, 240, 24, "" },
        /*178*/ { BARCODE_PDF417, -1, 13, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 13, 8, 120, 240, 26, "" },
        /*179*/ { BARCODE_PDF417, -1, 14, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 14, 8, 120, 240, 28, "" },
        /*180*/ { BARCODE_PDF417, -1, 15, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 15, 8, 120, 240, 30, "" },
        /*181*/ { BARCODE_PDF417, -1, 16, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 16, 8, 120, 240, 32, "" },
        /*182*/ { BARCODE_PDF417, -1, 17, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 17, 8, 120, 240, 34, "" },
        /*183*/ { BARCODE_PDF417, -1, 18, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 18, 8, 120, 240, 36, "" },
        /*184*/ { BARCODE_PDF417, -1, 19, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 19, 8, 120, 240, 38, "" },
        /*185*/ { BARCODE_PDF417, -1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 8, 120, 240, 40, "" },
        /*186*/ { BARCODE_PDF417, -1, 21, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 21, 8, 120, 240, 42, "" },
        /*187*/ { BARCODE_PDF417, -1, 22, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 22, 8, 120, 240, 44, "" },
        /*188*/ { BARCODE_PDF417, -1, 23, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 23, 8, 120, 240, 46, "" },
        /*189*/ { BARCODE_PDF417, -1, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 24, 8, 120, 240, 48, "" },
        /*190*/ { BARCODE_PDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 36, 12, 120, 240, 72, "12 rows" },
        /*191*/ { BARCODE_PDF417, -1, 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 12, 12, 120, 240, 24, "" },
        /*192*/ { BARCODE_PDF417COMP, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 4.5, 9, 86, 172, 9, "9 rows" },
        /*193*/ { BARCODE_PDF417COMP, -1, 24, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJ", "", ZINT_WARN_NONCOMPLIANT, 24, 9, 86, 172, 48, "" },
        /*194*/ { BARCODE_MAXICODE, -1, 1, "1234567890", "", 0, 16.5, 33, 30, 60, 57.733398, "Fixed size, symbol->height ignored" },
        /*195*/ { BARCODE_QRCODE, -1, 1, "ABCD", "", 0, 21, 21, 21, 42, 42, "Fixed width-to-height ratio, symbol->height ignored" },
        /*196*/ { BARCODE_CODE128B, -1, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "" },
        /*197*/ { BARCODE_CODE128B, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 145, 290, 2, "" },
        /*198*/ { BARCODE_CODE128B, -1, 4, "1234567890", "", 0, 4, 1, 145, 290, 8, "" },
        /*199*/ { BARCODE_AUSPOST, -1, -1, "12345678901234567890123", "", 0, 8, 3, 133, 266, 16, "" },
        /*200*/ { BARCODE_AUSPOST, -1, 1, "12345678901234567890123", "", 0, 2, 3, 133, 266, 4, "" },
        /*201*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 1, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 1.9230771, 3, 133, 266, 3.8461542, "" },
        /*202*/ { BARCODE_AUSPOST, -1, 6.9, "12345678901234567890123", "", 0, 6.9000001, 3, 133, 266, 13.8, "" },
        /*203*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 6.9, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 6.9000001, 3, 133, 266, 13.8, "" },
        /*204*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 7, "12345678901234567890123", "", 0, 7, 3, 133, 266, 14, "" },
        /*205*/ { BARCODE_AUSPOST, -1, 14, "12345678901234567890123", "", 0, 14, 3, 133, 266, 28, "" },
        /*206*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 14, "12345678901234567890123", "", 0, 14, 3, 133, 266, 28, "" },
        /*207*/ { BARCODE_AUSPOST, COMPLIANT_HEIGHT, 14.1, "12345678901234567890123", "", ZINT_WARN_NONCOMPLIANT, 14.099999, 3, 133, 266, 28.199999, "" },
        /*208*/ { BARCODE_AUSREPLY, -1, 14, "12345678", "", 0, 14, 3, 73, 146, 28, "" },
        /*209*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, 14, "12345678", "", 0, 14, 3, 73, 146, 28, "" },
        /*210*/ { BARCODE_AUSREPLY, COMPLIANT_HEIGHT, 14.25, "12345678", "", ZINT_WARN_NONCOMPLIANT, 14.25, 3, 73, 146, 28.5, "" },
        /*211*/ { BARCODE_AUSROUTE, -1, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "" },
        /*212*/ { BARCODE_AUSROUTE, COMPLIANT_HEIGHT, 7, "12345678", "", 0, 7, 3, 73, 146, 14, "" },
        /*213*/ { BARCODE_AUSREDIRECT, -1, 14, "12345678", "", 0, 14, 3, 73, 146, 28, "" },
        /*214*/ { BARCODE_AUSREDIRECT, COMPLIANT_HEIGHT, 14, "12345678", "", 0, 14, 3, 73, 146, 28, "" },
        /*215*/ { BARCODE_ISBNX, -1, 1, "123456789", "", 0, 1, 1, 95, 226, 12, "" },
        /*216*/ { BARCODE_ISBNX, -1, 69, "123456789", "", 0, 69, 1, 95, 226, 148, "" },
        /*217*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, 69, "123456789", "", ZINT_WARN_NONCOMPLIANT, 69, 1, 95, 226, 148, "" },
        /*218*/ { BARCODE_ISBNX, COMPLIANT_HEIGHT, 69.5, "123456789", "", 0, 69.5, 1, 95, 226, 149, "" },
        /*219*/ { BARCODE_RM4SCC, -1, -1, "1234567890", "", 0, 8, 3, 91, 182, 16, "" },
        /*220*/ { BARCODE_RM4SCC, -1, 1, "1234567890", "", 0, 2, 3, 91, 182, 4, "" },
        /*221*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.9615386, 3, 91, 182, 3.9230771, "" },
        /*222*/ { BARCODE_RM4SCC, -1, 4, "1234567890", "", 0, 4, 3, 91, 182, 8, "" },
        /*223*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4, 3, 91, 182, 8, "" },
        /*224*/ { BARCODE_RM4SCC, -1, 6, "1234567890", "", 0, 6, 3, 91, 182, 12, "" },
        /*225*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 6, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6, 3, 91, 182, 12, "" },
        /*226*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 6.5, "1234567890", "", 0, 6.5, 3, 91, 182, 13, "" },
        /*227*/ { BARCODE_RM4SCC, -1, 10.8, "1234567890", "", 0, 10.8, 3, 91, 182, 21.6, "" },
        /*228*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 10.8, "1234567890", "", 0, 10.8, 3, 91, 182, 21.6, "" },
        /*229*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 11, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 91, 182, 22, "" },
        /*230*/ { BARCODE_RM4SCC, -1, 16, "1234567890", "", 0, 16, 3, 91, 182, 32, "" },
        /*231*/ { BARCODE_RM4SCC, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 3, 91, 182, 32, "" },
        /*232*/ { BARCODE_DATAMATRIX, -1, 1, "ABCD", "", 0, 12, 12, 12, 24, 24, "Fixed width-to-height ratio, symbol->height ignored" },
        /*233*/ { BARCODE_EAN14, -1, 1, "1234567890123", "", 0, 1, 1, 134, 268, 2, "" },
        /*234*/ { BARCODE_EAN14, -1, 5.7, "1234567890123", "", 0, 5.6999998, 1, 134, 268, 11.4, "" },
        /*235*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, 5.7, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 5.6999998, 1, 134, 268, 11.4, "" },
        /*236*/ { BARCODE_EAN14, COMPLIANT_HEIGHT, 5.8, "1234567890123", "", 0, 5.8000002, 1, 134, 268, 11.6, "" },
        /*237*/ { BARCODE_VIN, -1, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "" },
        /*238*/ { BARCODE_VIN, COMPLIANT_HEIGHT, 1, "12345678701234567", "", 0, 1, 1, 246, 492, 2, "Doc unlikely" },
        /*239*/ { BARCODE_VIN, -1, 4, "12345678701234567", "", 0, 4, 1, 246, 492, 8, "" },
        /*240*/ { BARCODE_CODABLOCKF, -1, -1, "1234567890", "", 0, 20, 2, 101, 242, 44, "2 rows" },
        /*241*/ { BARCODE_CODABLOCKF, -1, 1, "1234567890", "", 0, 1, 2, 101, 242, 6, "" },
        /*242*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1, 2, 101, 242, 6, "" },
        /*243*/ { BARCODE_CODABLOCKF, -1, 15.5, "1234567890", "", 0, 15.5, 2, 101, 242, 35, "" },
        /*244*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 15.5, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15.5, 2, 101, 242, 35, "" },
        /*245*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 16, "1234567890", "", 0, 16, 2, 101, 242, 36, "" },
        /*246*/ { BARCODE_CODABLOCKF, -1, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 110, 11, 156, 352, 224, "11 rows, 14 cols" },
        /*247*/ { BARCODE_CODABLOCKF, -1, 87, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 87, 11, 156, 352, 178, "" },
        /*248*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 87, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", ZINT_WARN_NONCOMPLIANT, 87, 11, 156, 352, 178, "" },
        /*249*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 88, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 88, 11, 156, 352, 180, "" },
        /*250*/ { BARCODE_CODABLOCKF, -1, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "" },
        /*251*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 220, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs", "", 0, 220, 11, 156, 352, 444, "" },
        /*252*/ { BARCODE_CODABLOCKF, -1, -1, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 110, 11, 167, 374, 224, "11 rows, 15 cols" },
        /*253*/ { BARCODE_CODABLOCKF, -1, 93, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 92.999992, 11, 167, 374, 189.99998, "" },
        /*254*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 93, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", ZINT_WARN_NONCOMPLIANT, 92.999992, 11, 167, 374, 189.99998, "" },
        /*255*/ { BARCODE_CODABLOCKF, COMPLIANT_HEIGHT, 93.5, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv", "", 0, 93.5, 11, 167, 374, 191, "" },
        /*256*/ { BARCODE_NVE18, -1, 1, "12345678901234567", "", 0, 1, 1, 156, 312, 2, "" },
        /*257*/ { BARCODE_NVE18, -1, 5.7, "12345678901234567", "", 0, 5.6999998, 1, 156, 312, 11.4, "" },
        /*258*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, 5.7, "12345678901234567", "", ZINT_WARN_NONCOMPLIANT, 5.6999998, 1, 156, 312, 11.4, "" },
        /*259*/ { BARCODE_NVE18, COMPLIANT_HEIGHT, 5.8, "12345678901234567", "", 0, 5.8000002, 1, 156, 312, 11.6, "" },
        /*260*/ { BARCODE_JAPANPOST, -1, -1, "1234567890", "", 0, 8, 3, 133, 266, 16, "" },
        /*261*/ { BARCODE_JAPANPOST, -1, 1, "1234567890", "", 0, 2, 3, 133, 266, 4, "" },
        /*262*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 133, 266, 3, "" },
        /*263*/ { BARCODE_JAPANPOST, -1, 4.8, "1234567890", "", 0, 4.8000002, 3, 133, 266, 9.6000004, "" },
        /*264*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 4.8, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4.8000002, 3, 133, 266, 9.6000004, "" },
        /*265*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 4.9, "1234567890", "", 0, 4.9000001, 3, 133, 266, 9.8000002, "" },
        /*266*/ { BARCODE_JAPANPOST, -1, 7, "1234567890", "", 0, 7, 3, 133, 266, 14, "" },
        /*267*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 7, "1234567890", "", 0, 7, 3, 133, 266, 14, "" },
        /*268*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 7.5, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 7.5, 3, 133, 266, 15, "" },
        /*269*/ { BARCODE_JAPANPOST, -1, 16, "1234567890", "", 0, 16, 3, 133, 266, 32, "" },
        /*270*/ { BARCODE_JAPANPOST, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15.999999, 3, 133, 266, 31.999998, "" },
        /*271*/ { BARCODE_KOREAPOST, -1, 1, "123456", "", 0, 1, 1, 167, 334, 2, "" },
        /*272*/ { BARCODE_KOREAPOST, COMPLIANT_HEIGHT, 1, "123456", "", 0, 1, 1, 167, 334, 2, "TODO: Find doc" },
        /*273*/ { BARCODE_KOREAPOST, -1, 4, "123456", "", 0, 4, 1, 167, 334, 8, "" },
        /*274*/ { BARCODE_DBAR_STK, -1, -1, "1234567890123", "", 0, 13, 3, 50, 100, 26, "" },
        /*275*/ { BARCODE_DBAR_STK, -1, 1, "1234567890123", "", 0, 2.2, 3, 50, 100, 4.4000001, "" },
        /*276*/ { BARCODE_DBAR_STK, -1, 12, "1234567890123", "", 0, 12, 3, 50, 100, 24, "" },
        /*277*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, 12, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 12, 3, 50, 100, 24, "" },
        /*278*/ { BARCODE_DBAR_STK, COMPLIANT_HEIGHT, 13, "1234567890123", "", 0, 13, 3, 50, 100, 26, "" },
        /*279*/ { BARCODE_DBAR_OMNSTK, -1, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /*280*/ { BARCODE_DBAR_OMNSTK, -1, 1, "1234567890123", "", 0, 4, 5, 50, 100, 8, "" },
        /*281*/ { BARCODE_DBAR_OMNSTK, -1, 68, "1234567890123", "", 0, 68, 5, 50, 100, 136, "" },
        /*282*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, 68, "1234567890123", "", ZINT_WARN_NONCOMPLIANT, 68, 5, 50, 100, 136, "" },
        /*283*/ { BARCODE_DBAR_OMNSTK, COMPLIANT_HEIGHT, 69, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /*284*/ { BARCODE_DBAR_EXPSTK, -1, -1, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "2 rows" },
        /*285*/ { BARCODE_DBAR_EXPSTK, -1, 1, "[01]12345678901231", "", 0, 4, 5, 102, 204, 8, "" },
        /*286*/ { BARCODE_DBAR_EXPSTK, -1, 70, "[01]12345678901231", "", 0, 70, 5, 102, 204, 140, "" },
        /*287*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 70, "[01]12345678901231", "", ZINT_WARN_NONCOMPLIANT, 70, 5, 102, 204, 140, "" },
        /*288*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 71, "[01]12345678901231", "", 0, 71, 5, 102, 204, 142, "" },
        /*289*/ { BARCODE_DBAR_EXPSTK, -1, -1, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "3 rows" },
        /*290*/ { BARCODE_DBAR_EXPSTK, -1, 1, "[01]09501101530003[3920]123456789012345", "", 0, 7.5, 9, 102, 204, 15, "" },
        /*291*/ { BARCODE_DBAR_EXPSTK, -1, 107.5, "[01]09501101530003[3920]123456789012345", "", 0, 107.5, 9, 102, 204, 215, "" },
        /*292*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 107.5, "[01]09501101530003[3920]123456789012345", "", ZINT_WARN_NONCOMPLIANT, 107.5, 9, 102, 204, 215, "" },
        /*293*/ { BARCODE_DBAR_EXPSTK, COMPLIANT_HEIGHT, 108, "[01]09501101530003[3920]123456789012345", "", 0, 108, 9, 102, 204, 216, "" },
        /*294*/ { BARCODE_PLANET, -1, -1, "12345678901", "", 0, 12, 2, 123, 246, 24, "" },
        /*295*/ { BARCODE_PLANET, -1, 1, "12345678901", "", 0, 1, 2, 123, 246, 2, "" },
        /*296*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 1.25, 2, 123, 246, 2.5, "" },
        /*297*/ { BARCODE_PLANET, -1, 4.5, "12345678901", "", 0, 4.5, 2, 123, 246, 9, "" },
        /*298*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 4.5, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 4.5, 2, 123, 246, 9, "" },
        /*299*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 4.6, "12345678901", "", 0, 4.5999999, 2, 123, 246, 9.1999998, "" },
        /*300*/ { BARCODE_PLANET, -1, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*301*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 9, "12345678901", "", 0, 9, 2, 123, 246, 18, "" },
        /*302*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 9.1, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 9.1000004, 2, 123, 246, 18.200001, "" },
        /*303*/ { BARCODE_PLANET, -1, 24, "12345678901", "", 0, 24, 2, 123, 246, 48, "" },
        /*304*/ { BARCODE_PLANET, COMPLIANT_HEIGHT, 24, "12345678901", "", ZINT_WARN_NONCOMPLIANT, 24, 2, 123, 246, 48, "" },
        /*305*/ { BARCODE_MICROPDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 20, 10, 82, 164, 40, "10 rows" },
        /*306*/ { BARCODE_MICROPDF417, -1, 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "" },
        /*307*/ { BARCODE_MICROPDF417, -1, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 5, 10, 82, 164, 10, "" },
        /*308*/ { BARCODE_MICROPDF417, -1, 19, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 19, 10, 82, 164, 38, "" },
        /*309*/ { BARCODE_MICROPDF417, -1, 30, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 30, 10, 82, 164, 60, "" },
        /*310*/ { BARCODE_MICROPDF417, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", 0, 40, 20, 55, 110, 80, "20 rows" },
        /*311*/ { BARCODE_MICROPDF417, -1, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "", ZINT_WARN_NONCOMPLIANT, 20, 20, 55, 110, 40, "" },
        /*312*/ { BARCODE_USPS_IMAIL, -1, -1, "12345678901234567890", "", 0, 8, 3, 129, 258, 16, "" },
        /*313*/ { BARCODE_USPS_IMAIL, -1, 1, "12345678901234567890", "", 0, 2, 3, 129, 258, 4, "" },
        /*314*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 1, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5104165, 3, 129, 258, 3.020833, "" },
        /*315*/ { BARCODE_USPS_IMAIL, -1, 4.8, "12345678901234567890", "", 0, 4.8000002, 3, 129, 258, 9.6000004, "" },
        /*316*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 4.8, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 4.8000002, 3, 129, 258, 9.6000004, "" },
        /*317*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 4.9, "12345678901234567890", "", 0, 4.9000001, 3, 129, 258, 9.8000002, "" },
        /*318*/ { BARCODE_USPS_IMAIL, -1, 7.7, "12345678901234567890", "", 0, 7.6999998, 3, 129, 258, 15.4, "" },
        /*319*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 7.7, "12345678901234567890", "", 0, 7.7000003, 3, 129, 258, 15.400001, "" },
        /*320*/ { BARCODE_USPS_IMAIL, COMPLIANT_HEIGHT, 7.8, "12345678901234567890", "", ZINT_WARN_NONCOMPLIANT, 7.7999997, 3, 129, 258, 15.599999, "" },
        /*321*/ { BARCODE_PLESSEY, -1, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "" },
        /*322*/ { BARCODE_PLESSEY, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 227, 454, 2, "TODO: Find doc" },
        /*323*/ { BARCODE_PLESSEY, -1, 4, "1234567890", "", 0, 4, 1, 227, 454, 8, "" },
        /*324*/ { BARCODE_TELEPEN_NUM, -1, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "" },
        /*325*/ { BARCODE_TELEPEN_NUM, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 128, 256, 2, "No known min" },
        /*326*/ { BARCODE_TELEPEN_NUM, -1, 4, "1234567890", "", 0, 4, 1, 128, 256, 8, "" },
        /*327*/ { BARCODE_ITF14, -1, 1, "1234567890", "", 0, 1, 1, 135, 330, 22, "" },
        /*328*/ { BARCODE_ITF14, -1, 5.7, "1234567890", "", 0, 5.6999998, 1, 135, 330, 31.4, "" },
        /*329*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, 5.7, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 5.6999998, 1, 135, 330, 31.4, "" },
        /*330*/ { BARCODE_ITF14, COMPLIANT_HEIGHT, 5.8, "1234567890", "", 0, 5.8000002, 1, 135, 330, 31.6, "" },
        /*331*/ { BARCODE_KIX, -1, -1, "1234567890", "", 0, 8, 3, 79, 158, 16, "" },
        /*332*/ { BARCODE_KIX, -1, 1, "1234567890", "", 0, 2, 3, 79, 158, 4, "" },
        /*333*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.9615386, 3, 79, 158, 3.9230771, "" },
        /*334*/ { BARCODE_KIX, -1, 6.4, "1234567890", "", 0, 6.4000001, 3, 79, 158, 12.8, "" },
        /*335*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 6.4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 6.3999996, 3, 79, 158, 12.799999, "" },
        /*336*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 6.5, "1234567890", "", 0, 6.5, 3, 79, 158, 13, "" },
        /*337*/ { BARCODE_KIX, -1, 10.8, "1234567890", "", 0, 10.8, 3, 79, 158, 21.6, "" },
        /*338*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 10.8, "1234567890", "", 0, 10.8, 3, 79, 158, 21.6, "" },
        /*339*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 10.9, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 10.9, 3, 79, 158, 21.799999, "" },
        /*340*/ { BARCODE_KIX, -1, 16, "1234567890", "", 0, 16, 3, 79, 158, 32, "" },
        /*341*/ { BARCODE_KIX, COMPLIANT_HEIGHT, 16, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 16, 3, 79, 158, 32, "" },
        /*342*/ { BARCODE_AZTEC, -1, 1, "1234567890AB", "", 0, 15, 15, 15, 30, 30, "Fixed width-to-height ratio, symbol->height ignored" },
        /*343*/ { BARCODE_DAFT, -1, -1, "DAFTDAFTDAFTDAFT", "", 0, 8, 3, 31, 62, 16, "" },
        /*344*/ { BARCODE_DAFT, -1, 1, "DAFTDAFTDAFTDAFT", "", 0, 2, 3, 31, 62, 4, "" },
        /*345*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, 1, "DAFTDAFTDAFTDAFT", "", 0, 2, 3, 31, 62, 4, "" },
        /*346*/ { BARCODE_DAFT, -1, 4, "DAFTDAFTDAFTDAFT", "", 0, 4, 3, 31, 62, 8, "" },
        /*347*/ { BARCODE_DAFT, -1, 6, "DAFTDAFTDAFTDAFT", "", 0, 6, 3, 31, 62, 12, "" },
        /*348*/ { BARCODE_DAFT, -1, 12, "DAFTDAFTDAFTDAFT", "", 0, 12, 3, 31, 62, 24, "" },
        /*349*/ { BARCODE_DAFT, -1, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "" },
        /*350*/ { BARCODE_DAFT, COMPLIANT_HEIGHT, 16, "DAFTDAFTDAFTDAFT", "", 0, 16, 3, 31, 62, 32, "" },
        /*351*/ { BARCODE_DPD, -1, 1, "0123456789012345678901234567", "", 0, 1, 1, 189, 378, 2, "" },
        /*352*/ { BARCODE_DPD, -1, 62, "0123456789012345678901234567", "", 0, 62, 1, 189, 378, 124, "" },
        /*353*/ { BARCODE_DPD, COMPLIANT_HEIGHT, 62, "0123456789012345678901234567", "", ZINT_WARN_NONCOMPLIANT, 62, 1, 189, 378, 124, "" },
        /*354*/ { BARCODE_DPD, COMPLIANT_HEIGHT, 62.5, "0123456789012345678901234567", "", 0, 62.5, 1, 189, 378, 125, "" },
        /*355*/ { BARCODE_MICROQR, -1, 1, "12345", "", 0, 11, 11, 11, 22, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*356*/ { BARCODE_HIBC_128, -1, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "" },
        /*357*/ { BARCODE_HIBC_128, COMPLIANT_HEIGHT, 1, "1234567890", "", 0, 1, 1, 123, 246, 2, "" },
        /*358*/ { BARCODE_HIBC_128, -1, 4, "1234567890", "", 0, 4, 1, 123, 246, 8, "" },
        /*359*/ { BARCODE_HIBC_39, -1, 1, "1234567890", "", 0, 1, 1, 223, 446, 2, "" },
        /*360*/ { BARCODE_HIBC_39, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 223, 446, 2, "" },
        /*361*/ { BARCODE_HIBC_39, -1, 4, "1234567890", "", 0, 4, 1, 223, 446, 8, "" },
        /*362*/ { BARCODE_HIBC_DM, -1, 1, "ABC", "", 0, 12, 12, 12, 24, 24, "Fixed width-to-height ratio, symbol->height ignored" },
        /*363*/ { BARCODE_HIBC_QR, -1, 1, "1234567890AB", "", 0, 21, 21, 21, 42, 42, "Fixed width-to-height ratio, symbol->height ignored" },
        /*364*/ { BARCODE_HIBC_PDF, -1, -1, "ABCDEF", "", 0, 21, 7, 103, 206, 42, "7 rows" },
        /*365*/ { BARCODE_HIBC_PDF, -1, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*366*/ { BARCODE_HIBC_PDF, -1, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 4, 7, 103, 206, 8, "" },
        /*367*/ { BARCODE_HIBC_PDF, -1, 36, "ABCDEF", "", 0, 36, 7, 103, 206, 72, "" },
        /*368*/ { BARCODE_HIBC_MICPDF, -1, -1, "ABCDEF", "", 0, 12, 6, 82, 164, 24, "6 rows" },
        /*369*/ { BARCODE_HIBC_MICPDF, -1, 1, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 3, 6, 82, 164, 6, "" },
        /*370*/ { BARCODE_HIBC_MICPDF, -1, 4, "ABCDEF", "", ZINT_WARN_NONCOMPLIANT, 4, 6, 82, 164, 8, "" },
        /*371*/ { BARCODE_HIBC_MICPDF, -1, 47, "ABCDEF", "", 0, 47, 6, 82, 164, 94, "" },
        /*372*/ { BARCODE_HIBC_BLOCKF, -1, -1, "1234567890", "", 0, 30, 3, 101, 242, 64, "3 rows" },
        /*373*/ { BARCODE_HIBC_BLOCKF, -1, 1, "1234567890", "", 0, 1.5, 3, 101, 242, 7, "" },
        /*374*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 1.5, 3, 101, 242, 7, "" },
        /*375*/ { BARCODE_HIBC_BLOCKF, -1, 4, "1234567890", "", 0, 4, 3, 101, 242, 12, "" },
        /*376*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 4, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 4, 3, 101, 242, 12, "" },
        /*377*/ { BARCODE_HIBC_BLOCKF, -1, 23, "1234567890", "", 0, 23, 3, 101, 242, 50, "" },
        /*378*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 23, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 23, 3, 101, 242, 50, "" },
        /*379*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 24, "1234567890", "", 0, 24, 3, 101, 242, 52, "" },
        /*380*/ { BARCODE_HIBC_BLOCKF, -1, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "" },
        /*381*/ { BARCODE_HIBC_BLOCKF, COMPLIANT_HEIGHT, 60, "1234567890", "", 0, 60, 3, 101, 242, 124, "" },
        /*382*/ { BARCODE_HIBC_AZTEC, -1, 1, "1234567890AB", "", 0, 19, 19, 19, 38, 38, "Fixed width-to-height ratio, symbol->height ignored" },
        /*383*/ { BARCODE_DOTCODE, -1, 1, "ABC", "", 0, 11, 11, 16, 32, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*384*/ { BARCODE_HANXIN, -1, 1, "1234567890AB", "", 0, 23, 23, 23, 46, 46, "Fixed width-to-height ratio, symbol->height ignored" },
        /*385*/ { BARCODE_MAILMARK, -1, -1, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*386*/ { BARCODE_MAILMARK, -1, 1, "01000000000000000AA00AA0A", "", 0, 2.5, 3, 155, 310, 5, "" },
        /*387*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 1, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 1.9615386, 3, 155, 310, 3.9230771, "" },
        /*388*/ { BARCODE_MAILMARK, -1, 6.4, "01000000000000000AA00AA0A", "", 0, 6.4000001, 3, 155, 310, 12.8, "" },
        /*389*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 6.4, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 6.3999996, 3, 155, 310, 12.799999, "" },
        /*390*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 6.5, "01000000000000000AA00AA0A", "", 0, 6.5, 3, 155, 310, 13, "" },
        /*391*/ { BARCODE_MAILMARK, -1, 10, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*392*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 10, "01000000000000000AA00AA0A", "", 0, 10, 3, 155, 310, 20, "" },
        /*393*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 11, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 11, 3, 155, 310, 22, "" },
        /*394*/ { BARCODE_MAILMARK, -1, 15, "01000000000000000AA00AA0A", "", 0, 15, 3, 155, 310, 30, "" },
        /*395*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 15, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 15, 3, 155, 310, 30, "" },
        /*396*/ { BARCODE_MAILMARK, -1, 20, "01000000000000000AA00AA0A", "", 0, 20, 3, 155, 310, 40, "" },
        /*397*/ { BARCODE_MAILMARK, COMPLIANT_HEIGHT, 20, "01000000000000000AA00AA0A", "", ZINT_WARN_NONCOMPLIANT, 20, 3, 155, 310, 40, "" },
        /*398*/ { BARCODE_AZRUNE, -1, 1, "1", "", 0, 11, 11, 11, 22, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*399*/ { BARCODE_CODE32, -1, 1, "12345678", "", 0, 1, 1, 103, 206, 2, "" },
        /*400*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 1, "12345678", "", ZINT_WARN_NONCOMPLIANT, 1, 1, 103, 206, 2, "" },
        /*401*/ { BARCODE_CODE32, -1, 19, "12345678", "", 0, 19, 1, 103, 206, 38, "" },
        /*402*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 19, "12345678", "", ZINT_WARN_NONCOMPLIANT, 19, 1, 103, 206, 38, "" },
        /*403*/ { BARCODE_CODE32, COMPLIANT_HEIGHT, 20, "12345678", "", 0, 20, 1, 103, 206, 40, "" },
        /*404*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 234, 110, "EAN-13, CC-A 3 rows" },
        /*405*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01", 0, 12.5, 7, 99, 234, 35, "" },
        /*406*/ { BARCODE_EANX_CC, -1, 81, "123456789012", "[20]01", 0, 81, 7, 99, 234, 172, "" },
        /*407*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 81, "123456789012", "[20]01", ZINT_WARN_NONCOMPLIANT, 81, 7, 99, 234, 172, "" },
        /*408*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 81.25, "123456789012", "[20]01", 0, 81.25, 7, 99, 234, 172.5, "" },
        /*409*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 50, 9, 99, 234, 110, "EAN-13, CC-A 5 rows" },
        /*410*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 16.5, 9, 99, 234, 43, "" },
        /*411*/ { BARCODE_EANX_CC, -1, 85, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85, 9, 99, 234, 180, "" },
        /*412*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 85, "123456789012", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 85, 9, 99, 234, 180, "" },
        /*413*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 85.25, "123456789012", "[20]01[90]123456789012345678901234567890", 0, 85.25, 9, 99, 234, 180.5, "" },
        /*414*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 11, 99, 234, 110, "EAN-13, CC-A 7 rows" },
        /*415*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.5, 11, 99, 234, 51, "" },
        /*416*/ { BARCODE_EANX_CC, -1, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89, 11, 99, 234, 188, "" },
        /*417*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 89, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 89, 11, 99, 234, 188, "" },
        /*418*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 89.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 89.25, 11, 99, 234, 188.5, "" },
        /*419*/ { BARCODE_EANX_CC, -1, -1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 14, 99, 234, 110, "EAN-13, CC-B 10 rows" },
        /*420*/ { BARCODE_EANX_CC, -1, 1, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 14, 99, 234, 63, "" },
        /*421*/ { BARCODE_EANX_CC, -1, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95, 14, 99, 234, 200, "" },
        /*422*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 95, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 95, 14, 99, 234, 200, "" },
        /*423*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 95.25, "123456789012", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 95.25, 14, 99, 234, 200.5, "" },
        /*424*/ { BARCODE_EANX_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234", 0, 50, 10, 72, 172, 110, "EAN-8, CC-A 4 rows" },
        /*425*/ { BARCODE_EANX_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234", 0, 18.5, 10, 72, 172, 47, "" },
        /*426*/ { BARCODE_EANX_CC, -1, 73, "1234567", "[20]01[90]123456789012345678901234", 0, 73, 10, 72, 172, 156, "" },
        /*427*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 73, "1234567", "[20]01[90]123456789012345678901234", ZINT_WARN_NONCOMPLIANT, 73, 10, 72, 172, 156, "" },
        /*428*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 73.25, "1234567", "[20]01[90]123456789012345678901234", 0, 73.25, 10, 72, 172, 156.5, "" },
        /*429*/ { BARCODE_EANX_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50, 24, 82, 192, 110, "EAN-8, CC-B 15 rows" },
        /*430*/ { BARCODE_EANX_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 46.5, 24, 82, 192, 103, "" },
        /*431*/ { BARCODE_EANX_CC, -1, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101, 24, 82, 192, 212, "" },
        /*432*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 101, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 101, 24, 82, 192, 212, "" },
        /*433*/ { BARCODE_EANX_CC, COMPLIANT_HEIGHT, 101.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 101.25, 24, 82, 192, 212.5, "" },
        /*434*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 50, 5, 145, 290, 100, "CC-A 3 rows" },
        /*435*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 145, 290, 15, "" },
        /*436*/ { BARCODE_GS1_128_CC, -1, 12.5, "[01]12345678901231", "[20]01", 0, 12.5, 5, 145, 290, 25, "" },
        /*437*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 12.5, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 12.5, 5, 145, 290, 25, "" },
        /*438*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 12.75, "[01]12345678901231", "[20]01", 0, 12.75, 5, 145, 290, 25.5, "" },
        /*439*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 50, 9, 145, 290, 100, "CC-A 7 rows" },
        /*440*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 15.5, 9, 145, 290, 31, "" },
        /*441*/ { BARCODE_GS1_128_CC, -1, 20.7, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.700001, 9, 145, 290, 41.400002, "" },
        /*442*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 20.7, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 20.700001, 9, 145, 290, 41.400002, "" },
        /*443*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 20.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 20.75, 9, 145, 290, 41.5, "" },
        /*444*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 12, 145, 290, 100, "CC-B 10 rows" },
        /*445*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 145, 290, 43, "" },
        /*446*/ { BARCODE_GS1_128_CC, -1, 26.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.5, 12, 145, 290, 53, "" },
        /*447*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 26.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 26.5, 12, 145, 290, 53, "" },
        /*448*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 26.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 26.75, 12, 145, 290, 53.5, "" },
        /*449*/ { BARCODE_GS1_128_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "CC-C 30 rows" },
        /*450*/ { BARCODE_GS1_128_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 91.5, 32, 154, 308, 183, "" },
        /*451*/ { BARCODE_GS1_128_CC, -1, 96.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 96.5, 32, 154, 308, 193, "" },
        /*452*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 96.5, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 96.5, 32, 154, 308, 193, "" },
        /*453*/ { BARCODE_GS1_128_CC, COMPLIANT_HEIGHT, 96.75, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[92]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[93]123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890[94]123456789012345678901234567890", 0, 96.75, 32, 154, 308, 193.5, "" },
        /*454*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]1234567890", 0, 21, 5, 100, 200, 42, "CC-A 3 rows" },
        /*455*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]1234567890", 0, 7.5, 5, 100, 200, 15, "" },
        /*456*/ { BARCODE_DBAR_OMN_CC, -1, 19.9, "1234567890123", "[20]01[90]1234567890", 0, 19.9, 5, 100, 200, 39.799999, "" },
        /*457*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 19.9, "1234567890123", "[20]01[90]1234567890", ZINT_WARN_NONCOMPLIANT, 19.9, 5, 100, 200, 39.799999, "" },
        /*458*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 20, "1234567890123", "[20]01[90]1234567890", 0, 20, 5, 100, 200, 40, "" },
        /*459*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]12345678901234567890", 0, 23, 6, 100, 200, 46, "CC-A 4 rows" },
        /*460*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]12345678901234567890", 0, 9.5, 6, 100, 200, 19, "" },
        /*461*/ { BARCODE_DBAR_OMN_CC, -1, 21.9, "1234567890123", "[20]01[90]12345678901234567890", 0, 21.9, 6, 100, 200, 43.799999, "" },
        /*462*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 21.9, "1234567890123", "[20]01[90]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 21.9, 6, 100, 200, 43.799999, "" },
        /*463*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 22, "1234567890123", "[20]01[90]12345678901234567890", 0, 22, 6, 100, 200, 44, "" },
        /*464*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35, 12, 100, 200, 70, "CC-B 10 rows" },
        /*465*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 100, 200, 43, "" },
        /*466*/ { BARCODE_DBAR_OMN_CC, -1, 33.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 33.900002, 12, 100, 200, 67.800003, "" },
        /*467*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 33.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 33.900002, 12, 100, 200, 67.800003, "" },
        /*468*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 34, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 34, 12, 100, 200, 68, "" },
        /*469*/ { BARCODE_DBAR_OMN_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 39, 14, 100, 200, 78, "CC-B 12 rows" },
        /*470*/ { BARCODE_DBAR_OMN_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 25.5, 14, 100, 200, 51, "" },
        /*471*/ { BARCODE_DBAR_OMN_CC, -1, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 37.900002, 14, 100, 200, 75.800003, "" },
        /*472*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 37.900002, 14, 100, 200, 75.800003, "" },
        /*473*/ { BARCODE_DBAR_OMN_CC, COMPLIANT_HEIGHT, 38, "1234567890123", "[20]01[90]123456789012345678901234567890[91]123456789012345678901234567890", 0, 38, 14, 100, 200, 76, "" },
        /*474*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "CC-A 4 rows" },
        /*475*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01", 0, 9.5, 6, 79, 158, 19, "" },
        /*476*/ { BARCODE_DBAR_LTD_CC, -1, 18, "1234567890123", "[20]01", 0, 18, 6, 79, 158, 36, "" },
        /*477*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 18, "1234567890123", "[20]01", ZINT_WARN_NONCOMPLIANT, 18, 6, 79, 158, 36, "" },
        /*478*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 19, "1234567890123", "[20]01", 0, 19, 6, 79, 158, 38, "" },
        /*479*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "CC-A 7 rows" },
        /*480*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 15.5, 9, 79, 158, 31, "" },
        /*481*/ { BARCODE_DBAR_LTD_CC, -1, 24.9, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 24.9, 9, 79, 158, 49.799999, "" },
        /*482*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 24.9, "1234567890123", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 24.9, 9, 79, 158, 49.799999, "" },
        /*483*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 25, "1234567890123", "[20]01[90]123456789012345678901234567890", 0, 25, 9, 79, 158, 50, "" },
        /*484*/ { BARCODE_DBAR_LTD_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "CC-B 20 rows" },
        /*485*/ { BARCODE_DBAR_LTD_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 41.5, 22, 88, 176, 83, "" },
        /*486*/ { BARCODE_DBAR_LTD_CC, -1, 50.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 50.900002, 22, 88, 176, 101.8, "" },
        /*487*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 50.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 50.900002, 22, 88, 176, 101.8, "" },
        /*488*/ { BARCODE_DBAR_LTD_CC, COMPLIANT_HEIGHT, 51, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567890", 0, 51, 22, 88, 176, 102, "" },
        /*489*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "CC-A 3 rows" },
        /*490*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 7.5, 5, 134, 268, 15, "" },
        /*491*/ { BARCODE_DBAR_EXP_CC, -1, 40, "[01]12345678901231", "[20]01", 0, 40, 5, 134, 268, 80, "" },
        /*492*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 40, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 40, 5, 134, 268, 80, "" },
        /*493*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 41, "[01]12345678901231", "[20]01", 0, 41, 5, 134, 268, 82, "" },
        /*494*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "CC-A 5 rows" },
        /*495*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 11.5, 7, 134, 268, 23, "" },
        /*496*/ { BARCODE_DBAR_EXP_CC, -1, 44, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 44, 7, 134, 268, 88, "" },
        /*497*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 44, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", ZINT_WARN_NONCOMPLIANT, 44, 7, 134, 268, 88, "" },
        /*498*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 45, "[01]12345678901231", "[20]01[90]123456789012345678901234567890", 0, 45, 7, 134, 268, 90, "" },
        /*499*/ { BARCODE_DBAR_EXP_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "CC-B 10 rows" },
        /*500*/ { BARCODE_DBAR_EXP_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 21.5, 12, 134, 268, 43, "" },
        /*501*/ { BARCODE_DBAR_EXP_CC, -1, 54, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 54, 12, 134, 268, 108, "" },
        /*502*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 54, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 54, 12, 134, 268, 108, "" },
        /*503*/ { BARCODE_DBAR_EXP_CC, COMPLIANT_HEIGHT, 55, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 55, 12, 134, 268, 110, "" },
        /*504*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01", 0, 50, 7, 99, 234, 110, "CC-A 3 rows" },
        /*505*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01", 0, 12.5, 7, 99, 234, 35, "" },
        /*506*/ { BARCODE_UPCA_CC, -1, 81.24, "12345678901", "[20]01", 0, 81.239998, 7, 99, 234, 172.48, "" },
        /*507*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 81.24, "12345678901", "[20]01", ZINT_WARN_NONCOMPLIANT, 81.239998, 7, 99, 234, 172.48, "" },
        /*508*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 81.25, "12345678901", "[20]01", 0, 81.25, 7, 99, 234, 172.5, "" },
        /*509*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 10, 99, 234, 110, "CC-A 6 rows" },
        /*510*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 18.5, 10, 99, 234, 47, "" },
        /*511*/ { BARCODE_UPCA_CC, -1, 87.24, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 87.239998, 10, 99, 234, 184.48, "" },
        /*512*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 87.24, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 87.239998, 10, 99, 234, 184.48, "" },
        /*513*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 87.25, "12345678901", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 87.25, 10, 99, 234, 184.5, "" },
        /*514*/ { BARCODE_UPCA_CC, -1, -1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 50, 16, 99, 234, 110, "CC-B 12 rows" },
        /*515*/ { BARCODE_UPCA_CC, -1, 1, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 30.5, 16, 99, 234, 71, "" },
        /*516*/ { BARCODE_UPCA_CC, -1, 99, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 99, 16, 99, 234, 208, "" },
        /*517*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 99, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", ZINT_WARN_NONCOMPLIANT, 99, 16, 99, 234, 208, "" },
        /*518*/ { BARCODE_UPCA_CC, COMPLIANT_HEIGHT, 99.25, "12345678901", "[20]01[90]123456789012345678901234567890[91]123456789012345678912345678901234567", 0, 99.25, 16, 99, 234, 208.5, "" },
        /*519*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678", 0, 50, 11, 55, 142, 110, "CC-A 7 rows" },
        /*520*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678", 0, 20.5, 11, 55, 142, 51, "" },
        /*521*/ { BARCODE_UPCE_CC, -1, 89, "1234567", "[20]01[90]123456789012345678", 0, 89, 11, 55, 142, 188, "" },
        /*522*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 89, "1234567", "[20]01[90]123456789012345678", ZINT_WARN_NONCOMPLIANT, 89, 11, 55, 142, 188, "" },
        /*523*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 89.25, "1234567", "[20]01[90]123456789012345678", 0, 89.25, 11, 55, 142, 188.5, "" },
        /*524*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 50, 16, 55, 142, 110, "CC-A 12 rows" },
        /*525*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 30.5, 16, 55, 142, 71, "" },
        /*526*/ { BARCODE_UPCE_CC, -1, 99, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 99, 16, 55, 142, 208, "" },
        /*527*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 99, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 99, 16, 55, 142, 208, "" },
        /*528*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 99.25, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 99.25, 16, 55, 142, 208.5, "" },
        /*529*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 50, 21, 55, 142, 110, "CC-B 17 rows" },
        /*530*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 40.5, 21, 55, 142, 91, "" },
        /*531*/ { BARCODE_UPCE_CC, -1, 109, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 109, 21, 55, 142, 228, "" },
        /*532*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 109, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 109, 21, 55, 142, 228, "" },
        /*533*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 109.25, "1234567", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 109.25, 21, 55, 142, 228.5, "" },
        /*534*/ { BARCODE_UPCE_CC, -1, -1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 142, 115, "CC-B 23 rows" },
        /*535*/ { BARCODE_UPCE_CC, -1, 1, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 52.5, 27, 55, 142, 115, "" },
        /*536*/ { BARCODE_UPCE_CC, -1, 121, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 121, 27, 55, 142, 252, "" },
        /*537*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 121, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", ZINT_WARN_NONCOMPLIANT, 121, 27, 55, 142, 252, "" },
        /*538*/ { BARCODE_UPCE_CC, COMPLIANT_HEIGHT, 121.25, "1234567", "[20]01[90]123456789012345678901234567890[91]1234567890123456789012345678901234567", 0, 121.25, 27, 55, 142, 252.5, "" },
        /*539*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "CC-A 5 rows" },
        /*540*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01", 0, 13.2, 9, 56, 112, 26.4, "" },
        /*541*/ { BARCODE_DBAR_STK_CC, -1, 23.9, "1234567890123", "[20]01", 0, 23.9, 9, 56, 112, 47.799999, "" },
        /*542*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 23.9, "1234567890123", "[20]01", ZINT_WARN_NONCOMPLIANT, 23.9, 9, 56, 112, 47.799999, "" },
        /*543*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 24, "1234567890123", "[20]01", 0, 24, 9, 56, 112, 48, "" },
        /*544*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "CC-A 12 rows" },
        /*545*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 27.200001, 16, 56, 112, 54.400002, "" },
        /*546*/ { BARCODE_DBAR_STK_CC, -1, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 37.900002, 16, 56, 112, 75.800003, "" },
        /*547*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 37.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", ZINT_WARN_NONCOMPLIANT, 37.900002, 16, 56, 112, 75.800003, "" },
        /*548*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 38, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678", 0, 38, 16, 56, 112, 76, "" },
        /*549*/ { BARCODE_DBAR_STK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "CC-B 17 rows" },
        /*550*/ { BARCODE_DBAR_STK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 37.200001, 21, 56, 112, 74.400002, "" },
        /*551*/ { BARCODE_DBAR_STK_CC, -1, 47.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 47.900002, 21, 56, 112, 95.800003, "" },
        /*552*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 47.9, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 47.900002, 21, 56, 112, 95.800003, "" },
        /*553*/ { BARCODE_DBAR_STK_CC, COMPLIANT_HEIGHT, 48, "1234567890123", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 48, 21, 56, 112, 96, "" },
        /*554*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "CC-A 6 rows" },
        /*555*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]1234567890123456", 0, 17, 12, 56, 112, 34, "" },
        /*556*/ { BARCODE_DBAR_OMNSTK_CC, -1, 81, "1234567890123", "[20]01[90]1234567890123456", 0, 81, 12, 56, 112, 162, "" },
        /*557*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 81, "1234567890123", "[20]01[90]1234567890123456", ZINT_WARN_NONCOMPLIANT, 81, 12, 56, 112, 162, "" },
        /*558*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 82, "1234567890123", "[20]01[90]1234567890123456", 0, 82, 12, 56, 112, 164, "" },
        /*559*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "CC-A 12 rows" },
        /*560*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 29, 18, 56, 112, 58, "" },
        /*561*/ { BARCODE_DBAR_OMNSTK_CC, -1, 93.5, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 93.5, 18, 56, 112, 187, "" },
        /*562*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 93.5, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", ZINT_WARN_NONCOMPLIANT, 93.5, 18, 56, 112, 187, "" },
        /*563*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 94, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567", 0, 94, 18, 56, 112, 188, "" },
        /*564*/ { BARCODE_DBAR_OMNSTK_CC, -1, -1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "CC-B 17 rows" },
        /*565*/ { BARCODE_DBAR_OMNSTK_CC, -1, 1, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 39, 23, 56, 112, 78, "" },
        /*566*/ { BARCODE_DBAR_OMNSTK_CC, -1, 103, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 103, 23, 56, 112, 206, "" },
        /*567*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 103, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", ZINT_WARN_NONCOMPLIANT, 103, 23, 56, 112, 206, "" },
        /*568*/ { BARCODE_DBAR_OMNSTK_CC, COMPLIANT_HEIGHT, 104, "1234567890123", "[20]01[90]123456789012345678901234567890[91]1234567890", 0, 104, 23, 56, 112, 208, "" },
        /*569*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "3 rows, CC-A 3 rows" },
        /*570*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01", 0, 11, 9, 102, 204, 22, "" },
        /*571*/ { BARCODE_DBAR_EXPSTK_CC, -1, 77, "[01]12345678901231", "[20]01", 0, 77, 9, 102, 204, 154, "" },
        /*572*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 77, "[01]12345678901231", "[20]01", ZINT_WARN_NONCOMPLIANT, 77, 9, 102, 204, 154, "" },
        /*573*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 78, "[01]12345678901231", "[20]01", 0, 78, 9, 102, 204, 156, "" },
        /*574*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "5 rows, CC-A 3 rows" },
        /*575*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 21.5, 21, 102, 204, 43, "" },
        /*576*/ { BARCODE_DBAR_EXPSTK_CC, -1, 188, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 188, 21, 102, 204, 376, "" },
        /*577*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 188, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", ZINT_WARN_NONCOMPLIANT, 188, 21, 102, 204, 376, "" },
        /*578*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 189, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01", 0, 189, 21, 102, 204, 378, "" },
        /*579*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "3 rows, CC-A 4 rows" },
        /*580*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 13, 10, 102, 204, 26, "" },
        /*581*/ { BARCODE_DBAR_EXPSTK_CC, -1, 79, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 79, 10, 102, 204, 158, "" },
        /*582*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 79, "[01]12345678901231", "[20]01[90]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 79, 10, 102, 204, 158, "" },
        /*583*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 80, "[01]12345678901231", "[20]01[90]12345678901234567890", 0, 80, 10, 102, 204, 160, "" },
        /*584*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "3 rows, CC-B 10 rows" },
        /*585*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 25, 16, 102, 204, 50, "" },
        /*586*/ { BARCODE_DBAR_EXPSTK_CC, -1, 91, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 91, 16, 102, 204, 182, "" },
        /*587*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 91, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 91, 16, 102, 204, 182, "" },
        /*588*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 92, "[01]12345678901231", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 92, 16, 102, 204, 184, "" },
        /*589*/ { BARCODE_DBAR_EXPSTK_CC, -1, -1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "5 rows, CC-B 10 rows" },
        /*590*/ { BARCODE_DBAR_EXPSTK_CC, -1, 1, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 35.5, 28, 102, 204, 71, "" },
        /*591*/ { BARCODE_DBAR_EXPSTK_CC, -1, 202, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 202, 28, 102, 204, 404, "" },
        /*592*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 202, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", ZINT_WARN_NONCOMPLIANT, 202, 28, 102, 204, 404, "" },
        /*593*/ { BARCODE_DBAR_EXPSTK_CC, COMPLIANT_HEIGHT, 203, "[91]ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFG", "[20]01[90]123456789012345678901234567890[91]12345678901234567890", 0, 203, 28, 102, 204, 406, "" },
        /*594*/ { BARCODE_CHANNEL, -1, 1, "1", "", 0, 1, 1, 19, 38, 2, "" },
        /*595*/ { BARCODE_CHANNEL, -1, 3.75, "123", "", 0, 3.75, 1, 23, 46, 7.5, "" },
        /*596*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, 3.75, "123", "", ZINT_WARN_NONCOMPLIANT, 3.75, 1, 23, 46, 7.5, "Min height data-length dependent" },
        /*597*/ { BARCODE_CHANNEL, COMPLIANT_HEIGHT, 4, "123", "", 0, 4, 1, 23, 46, 8, "" },
        /*598*/ { BARCODE_CODEONE, -1, 1, "12345678901234567890", "", 0, 16, 16, 18, 36, 32, "Fixed height, symbol->height ignored" },
        /*599*/ { BARCODE_GRIDMATRIX, -1, 1, "ABC", "", 0, 18, 18, 18, 36, 36, "Fixed width-to-height ratio, symbol->height ignored" },
        /*600*/ { BARCODE_UPNQR, -1, 1, "1234567890AB", "", 0, 77, 77, 77, 154, 154, "Fixed width-to-height ratio, symbol->height ignored" },
        /*601*/ { BARCODE_ULTRA, -1, 1, "1234567890", "", 0, 13, 13, 18, 36, 26, "Fixed width-to-height ratio, symbol->height ignored" },
        /*602*/ { BARCODE_RMQR, -1, 1, "12345", "", 0, 11, 11, 27, 54, 22, "Fixed width-to-height ratio, symbol->height ignored" },
        /*603*/ { BARCODE_BC412, -1, 1, "1234567", "", 0, 1, 1, 102, 204, 2, "" },
        /*604*/ { BARCODE_BC412, -1, 13.6, "1234567", "", 0, 13.6, 1, 102, 204, 27.200001, "" },
        /*605*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 13.6, "1234567", "", ZINT_WARN_NONCOMPLIANT, 13.6, 1, 102, 204, 27.200001, "" },
        /*606*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 13.65, "1234567", "", 0, 13.65, 1, 102, 204, 27.299999, "" },
        /*607*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 21.3, "1234567", "", 0, 21.3, 1, 102, 204, 42.599998, "" },
        /*608*/ { BARCODE_BC412, COMPLIANT_HEIGHT, 21.35, "1234567", "", ZINT_WARN_NONCOMPLIANT, 21.35, 1, 102, 204, 42.700001, "" },
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
        symbol->show_hrt = 0; /* Note: disabling HRT */
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

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%s) vector NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %.5g, \"%s\", \"%s\", %s, %.8g, %d, %d, %.8g, %.8g, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilOutputOptionsName(data[i].output_options),
                    data[i].height, data[i].data, data[i].composite, testUtilErrorName(data[i].ret),
                    symbol->height, symbol->rows, symbol->width, symbol->vector->width, symbol->vector->height, data[i].comment);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_height_per_row(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        float height;
        float scale;
        char *data;
        char *composite;
        int ret;

        float expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;

        const char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_PDF417, -1, -1, -1, -1, -1, -1, "1234567890", "", 0, 21, 7, 103, 206, 42, "" },
        /*  1*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 3.5, 7, 103, 206, 7, "" },
        /*  2*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 7, 7, 103, 206, 14, "" },
        /*  3*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.25, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 8.75, 7, 103, 206, 17.5, "" },
        /*  4*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 10.5, 7, 103, 206, 21, "" },
        /*  5*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.7, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 11.900001, 7, 103, 206, 23.800001, "" },
        /*  6*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.74, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 12.18, 7, 103, 206, 24.360001, "" },
        /*  7*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1.75, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 12.25, 7, 103, 206, 24.5, "" },
        /*  8*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 14, 7, 103, 206, 28, "" },
        /*  9*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.1, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 14.699999, 7, 103, 206, 29.399998, "" },
        /* 10*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.25, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 15.75, 7, 103, 206, 31.5, "" },
        /* 11*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 17.5, 7, 103, 206, 35, "" },
        /* 12*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 2.75, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 19.25, 7, 103, 206, 38.5, "" },
        /* 13*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 3, -1, "1234567890", "", 0, 21, 7, 103, 206, 42, "Default" },
        /* 14*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 24.5, 7, 103, 206, 49, "" },
        /* 15*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, -1, 1000, -1, "1234567890", "", 0, 7000, 7, 103, 206, 14000, "" },
        /* 16*/ { BARCODE_PDF417, HEIGHTPERROW_MODE, -1, -1, 5, 0.5, -1, "1234567890", "", ZINT_WARN_NONCOMPLIANT, 2.5, 5, 120, 240, 5, "5 rows" },
        /* 17*/ { BARCODE_PDF417COMP, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 24.5, 7, 69, 138, 49, "" },
        /* 18*/ { BARCODE_HIBC_PDF, HEIGHTPERROW_MODE, -1, -1, -1, 3.5, -1, "1234567890", "", 0, 28, 8, 103, 206, 56, "" },
        /* 19*/ { BARCODE_CODE16K, -1, -1, -1, -1, -1, -1, "1234567890", "", 0, 20, 2, 70, 162, 44, "" },
        /* 20*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890", "", 0, 1, 2, 70, 162, 6, "(0.5 * 2 rows + 2 binds) * 2 scale = 6 (separator will cover rows)" },
        /* 21*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.7, -1, "1234567890", "", 0, 1.4, 2, 70, 162, 6.8000002, "" },
        /* 22*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 0.75, -1, "1234567890", "", 0, 1.5, 2, 70, 162, 7, "" },
        /* 23*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890", "", 0, 2, 2, 70, 162, 8, "" },
        /* 24*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.1, -1, "1234567890", "", 0, 2.2, 2, 70, 162, 8.3999996, "" },
        /* 25*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.2, -1, "1234567890", "", 0, 2.4000001, 2, 70, 162, 8.8000002, "" },
        /* 26*/ { BARCODE_CODE16K, HEIGHTPERROW_MODE, -1, -1, -1, 1.25, -1, "1234567890", "", 0, 2.5, 2, 70, 162, 9, "" },
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
        /* 41*/ { BARCODE_CODABLOCKF, HEIGHTPERROW_MODE, 2, -1, -1, 10.15, -1, "12345678901234567890123456", "", 0, 20.299999, 2, 200, 440, 44.599998, "Default for 13 cols" },
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
        /* 64*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.1, -1, "1234", "", 0, 4.1999998, 2, 13, 26, 8.3999996, "" },
        /* 65*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.2, -1, "1234", "", 0, 4.4000001, 2, 13, 26, 8.8000002, "" },
        /* 66*/ { BARCODE_PHARMA_TWO, HEIGHTPERROW_MODE, -1, -1, -1, 2.25, -1, "1234", "", 0, 4.5, 2, 13, 26, 9, "" },
        /* 67*/ { BARCODE_DBAR_OMNSTK, -1, -1, -1, -1, -1, -1, "1234567890123", "", 0, 69, 5, 50, 100, 138, "" },
        /* 68*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "1234567890123", "", 0, 4, 5, 50, 100, 8, "(0.5 * 2 rows + 3 separators) * 2 scale = 8" },
        /* 69*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 1, -1, "1234567890123", "", 0, 5, 5, 50, 100, 10, "" },
        /* 70*/ { BARCODE_DBAR_OMNSTK, HEIGHTPERROW_MODE, -1, -1, -1, 3.2, -1, "1234567890123", "", 0, 9.3999996, 5, 50, 100, 18.799999, "" },
        /* 71*/ { BARCODE_EANX_CC, -1, -1, -1, -1, -1, -1, "123456789012", "[20]01", 0, 50, 7, 99, 234, 110, "" },
        /* 72*/ { BARCODE_EANX_CC, HEIGHTPERROW_MODE, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 234, 35, "(0.5 * 1 row + 2 * 3 seps + 2 * 3 cc rows + 5 guards) * 2 scale = 35" },
        /* 73*/ { BARCODE_EANX_CC, -1, -1, -1, -1, 0.5, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 234, 35, "0.5 height below fixed height" },
        /* 74*/ { BARCODE_EANX_CC, HEIGHTPERROW_MODE, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 16, 7, 99, 234, 42, "" },
        /* 75*/ { BARCODE_EANX_CC, -1, -1, -1, -1, 4, -1, "123456789012", "[20]01", 0, 12.5, 7, 99, 234, 35, "4 height below fixed height" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char *text;

    testStart("test_height_per_row");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d\n", i);

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

        ret = ZBarcode_Encode(symbol, (unsigned char *) text, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode(%s) ret %d != %d (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, data[i].ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%s) ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), ret, symbol->errtxt);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%s) vector NULL\n", i, testUtilBarcodeName(data[i].symbology));

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, %d, %.5g, %.5g, \"%s\", \"%s\", %s, %.8g, %d, %d, %.8g, %.8g, \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                    data[i].option_1, data[i].option_2, data[i].option_3, data[i].height, data[i].scale,
                    data[i].data, data[i].composite, testUtilErrorName(data[i].ret),
                    symbol->height, symbol->rows, symbol->width, symbol->vector->width, symbol->vector->height, data[i].comment);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %.8g != %.8g\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %.8g != %.8g\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_options", test_options, 1, 0, 1 },
        { "test_buffer_vector", test_buffer_vector, 1, 1, 1 },
        { "test_upcean_hrt", test_upcean_hrt, 1, 0, 1 },
        { "test_row_separator", test_row_separator, 1, 0, 1 },
        { "test_stacking", test_stacking, 1, 0, 1 },
        { "test_output_options", test_output_options, 1, 0, 1 },
        { "test_noncomposite_string_x", test_noncomposite_string_x, 1, 0, 1 },
        { "test_upcean_whitespace_width", test_upcean_whitespace_width, 1, 0, 1 },
        { "test_scale", test_scale, 1, 0, 1 },
        { "test_guard_descent", test_guard_descent, 1, 0, 1 },
        { "test_quiet_zones", test_quiet_zones, 1, 0, 1 },
        { "test_height", test_height, 1, 1, 1 },
        { "test_height_per_row", test_height_per_row, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
