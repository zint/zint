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

static struct zint_vector_rect *find_rect(struct zint_symbol *symbol, int x, int y, int height, int width) {
    struct zint_vector_rect *rect;

    if (symbol->vector == NULL) {
        return NULL;
    }
    for (rect = symbol->vector->rectangles; rect != NULL; rect = rect->next) {
        if (rect->x == x && rect->y == y) {
            if (height && width) {
                if (rect->height == height && rect->width == width) {
                    break;
                }
            } else if (height) {
                if (rect->height == height) {
                    break;
                }
            } else if (width) {
                if (rect->width == width) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    return rect;
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
        float expected_vector_width;
        float expected_vector_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE128, "123456", "7890ab", 0, "A", 0, 1, 46, 92, 118 },
        /*  1*/ { BARCODE_CODE128, "12345", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  2*/ { BARCODE_CODE128, NULL, "1234567", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  3*/ { BARCODE_CODE128, "12345 ", NULL, 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
        /*  4*/ { BARCODE_CODE128, NULL, "EEFFGG", 0, "A", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1 },
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

        ret = ZBarcode_Encode_and_Buffer_Vector(symbol, data[i].data, length, data[i].rotate_angle);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %f != %f\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %f != %f\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_buffer_vector(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        char *composite;

        int expected_height;
        int expected_rows;
        int expected_width;
        float expected_vector_width;
        float expected_vector_height;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, "0000000000", "", 50, 1, 99, 198.0, 118.0 },
        /*  1*/ { BARCODE_C25MATRIX, "0000000000", "", 50, 1, 117, 234.0, 118.0 },
        /*  2*/ { BARCODE_C25INTER, "0000000000", "", 50, 1, 99, 198.0, 118.0 },
        /*  3*/ { BARCODE_C25IATA, "0000000000", "", 50, 1, 149, 298.0, 118.0 },
        /*  4*/ { BARCODE_C25LOGIC, "0000000000", "", 50, 1, 109, 218.0, 118.0 },
        /*  5*/ { BARCODE_C25IND, "0000000000", "", 50, 1, 159, 318.0, 118.0 },
        /*  6*/ { BARCODE_CODE39, "0000000000", "", 50, 1, 155, 310.0, 118.0 },
        /*  7*/ { BARCODE_EXCODE39, "0000000000", "", 50, 1, 155, 310.0, 118.0 },
        /*  8*/ { BARCODE_EANX, "123456789012", "", 50, 1, 95, 230.0, 118.0 },
        /*  9*/ { BARCODE_EANX_CHK, "1234567890128", "", 50, 1, 95, 230.0, 118.0 },
        /* 10*/ { BARCODE_EANX, "123456789012+12", "", 50, 1, 124, 288.0, 118.0 },
        /* 11*/ { BARCODE_EANX, "123456789012+12345", "", 50, 1, 151, 342.0, 118.0 },
        /* 12*/ { BARCODE_EANX, "1234567", "", 50, 1, 67, 134.0, 118.0 },
        /* 13*/ { BARCODE_EANX, "1234567+12", "", 50, 1, 96, 192.0, 118.0 },
        /* 14*/ { BARCODE_EANX, "1234567+12345", "", 50, 1, 123, 246.0, 118.0 },
        /* 15*/ { BARCODE_EANX, "1234", "", 50, 1, 47, 94.0, 118.0 },
        /* 16*/ { BARCODE_EANX, "12", "", 50, 1, 20, 40.0, 118.0 },
        /* 17*/ { BARCODE_EAN128, "[01]12345678901234", "", 50, 1, 134, 268.0, 118.0 },
        /* 18*/ { BARCODE_CODABAR, "A00000000B", "", 50, 1, 102, 204.0, 118.0 },
        /* 19*/ { BARCODE_CODE128, "0000000000", "", 50, 1, 90, 180.0, 118.0 },
        /* 20*/ { BARCODE_DPLEIT, "1234567890123", "", 50, 1, 135, 270.0, 118.0 },
        /* 21*/ { BARCODE_DPIDENT, "12345678901", "", 50, 1, 117, 234.0, 118.0 },
        /* 22*/ { BARCODE_CODE16K, "0000000000", "", 20, 2, 70, 162.0, 44.0 },
        /* 23*/ { BARCODE_CODE49, "0000000000", "", 20, 2, 70, 162.0, 44.0 },
        /* 24*/ { BARCODE_CODE93, "0000000000", "", 50, 1, 127, 254.0, 118.0 },
        /* 25*/ { BARCODE_FLAT, "1234567890", "", 50, 1, 90, 180.0, 100.0 },
        /* 26*/ { BARCODE_RSS14, "1234567890123", "", 50, 1, 96, 192.0, 118.0 },
        /* 27*/ { BARCODE_RSS_LTD, "1234567890123", "", 50, 1, 74, 148.0, 118.0 },
        /* 28*/ { BARCODE_RSS_EXP, "[01]12345678901234", "", 34, 1, 134, 268.0, 86.0 },
        /* 29*/ { BARCODE_TELEPEN, "0000000000", "", 50, 1, 208, 416.0, 118.0 },
        /* 30*/ { BARCODE_UPCA, "12345678904", "", 50, 1, 95, 230.0, 118.0 },
        /* 31*/ { BARCODE_UPCA_CHK, "12345678905", "", 50, 1, 95, 230.0, 118.0 },
        /* 32*/ { BARCODE_UPCA, "12345678904+12", "", 50, 1, 124, 288.0, 118.0 },
        /* 33*/ { BARCODE_UPCA, "12345678904+12345", "", 50, 1, 151, 342.0, 118.0 },
        /* 34*/ { BARCODE_UPCE, "1234567", "", 50, 1, 51, 142.0, 118.0 },
        /* 35*/ { BARCODE_UPCE_CHK, "12345670", "", 50, 1, 51, 142.0, 118.0 },
        /* 36*/ { BARCODE_UPCE, "1234567+12", "", 50, 1, 80, 200.0, 118.0 },
        /* 37*/ { BARCODE_UPCE, "1234567+12345", "", 50, 1, 107, 254.0, 118.0 },
        /* 38*/ { BARCODE_POSTNET, "00000000000", "", 12, 2, 185, 370.0, 24.0 },
        /* 39*/ { BARCODE_MSI_PLESSEY, "0000000000", "", 50, 1, 127, 254.0, 118.0 },
        /* 40*/ { BARCODE_FIM, "A", "", 50, 1, 17, 34.0, 100.0 },
        /* 41*/ { BARCODE_LOGMARS, "0000000000", "", 50, 1, 191, 382.0, 118.0 },
        /* 42*/ { BARCODE_PHARMA, "123456", "", 50, 1, 58, 116.0, 100.0 },
        /* 43*/ { BARCODE_PZN, "123456", "", 50, 1, 142, 284.0, 118.0 },
        /* 44*/ { BARCODE_PHARMA_TWO, "12345678", "", 10, 2, 29, 58.0, 20.0 },
        /* 45*/ { BARCODE_PDF417, "0000000000", "", 21, 7, 103, 206.0, 42.0 },
        /* 46*/ { BARCODE_PDF417TRUNC, "0000000000", "", 21, 7, 68, 136.0, 42.0 },
        /* 47*/ { BARCODE_MAXICODE, "0000000000", "", 165, 33, 30, 74.0, 72.0 },
        /* 48*/ { BARCODE_QRCODE, "1234567890AB", "", 21, 21, 21, 42.0, 42.0 },
        /* 49*/ { BARCODE_CODE128B, "0000000000", "", 50, 1, 145, 290.0, 118.0 },
        /* 50*/ { BARCODE_AUSPOST, "12345678901234567890123", "", 8, 3, 133, 266.0, 16.0 },
        /* 51*/ { BARCODE_AUSREPLY, "12345678", "", 8, 3, 73, 146.0, 16.0 },
        /* 52*/ { BARCODE_AUSROUTE, "12345678", "", 8, 3, 73, 146.0, 16.0 },
        /* 53*/ { BARCODE_AUSREDIRECT, "12345678", "", 8, 3, 73, 146.0, 16.0 },
        /* 54*/ { BARCODE_ISBNX, "123456789", "", 50, 1, 95, 230.0, 118.0 },
        /* 55*/ { BARCODE_ISBNX, "123456789+12", "", 50, 1, 124, 288.0, 118.0 },
        /* 56*/ { BARCODE_ISBNX, "123456789+12345", "", 50, 1, 151, 342.0, 118.0 },
        /* 57*/ { BARCODE_RM4SCC, "0000000000", "", 8, 3, 91, 182.0, 16.0 },
        /* 58*/ { BARCODE_DATAMATRIX, "ABC", "", 10, 10, 10, 20.0, 20.0 },
        /* 59*/ { BARCODE_EAN14, "1234567890123", "", 50, 1, 134, 268.0, 118.0 },
        /* 60*/ { BARCODE_VIN, "00000000000000000", "", 50, 1, 246, 492.0, 118.0 },
        /* 61*/ { BARCODE_CODABLOCKF, "0000000000", "", 20, 2, 101, 242.0, 44.0 },
        /* 62*/ { BARCODE_NVE18, "12345678901234567", "", 50, 1, 156, 312.0, 118.0 },
        /* 63*/ { BARCODE_JAPANPOST, "0000000000", "", 8, 3, 133, 266.0, 16.0 },
        /* 64*/ { BARCODE_KOREAPOST, "123456", "", 50, 1, 167, 334.0, 118.0 },
        /* 65*/ { BARCODE_RSS14STACK, "0000000000000", "", 13, 3, 50, 100.0, 26.0 },
        /* 66*/ { BARCODE_RSS14STACK_OMNI, "0000000000000", "", 69, 5, 50, 100.0, 138.0 },
        /* 67*/ { BARCODE_RSS_EXPSTACK, "[01]12345678901234", "", 71, 5, 102, 204.0, 142.0 },
        /* 68*/ { BARCODE_PLANET, "00000000000", "", 12, 2, 185, 370.0, 24.0 },
        /* 69*/ { BARCODE_MICROPDF417, "0000000000", "", 12, 6, 82, 164.0, 24.0 },
        /* 70*/ { BARCODE_ONECODE, "12345678901234567890", "", 8, 3, 129, 258.0, 16.0 },
        /* 71*/ { BARCODE_PLESSEY, "0000000000", "", 50, 1, 227, 454.0, 118.0 },
        /* 72*/ { BARCODE_TELEPEN_NUM, "0000000000", "", 50, 1, 128, 256.0, 118.0 },
        /* 73*/ { BARCODE_ITF14, "0000000000", "", 50, 1, 135, 382.0, 150.0 },
        /* 74*/ { BARCODE_KIX, "123456ABCDE", "", 8, 3, 87, 174.0, 16.0 },
        /* 75*/ { BARCODE_AZTEC, "1234567890AB", "", 15, 15, 15, 30.0, 30.0 },
        /* 76*/ { BARCODE_DAFT, "DAFTDAFTDAFTDAFT", "", 8, 3, 31, 62.0, 16.0 },
        /* 77*/ { BARCODE_MICROQR, "12345", "", 11, 11, 11, 22.0, 22.0 },
        /* 78*/ { BARCODE_HIBC_128, "0000000000", "", 50, 1, 134, 268.0, 118.0 },
        /* 79*/ { BARCODE_HIBC_39, "0000000000", "", 50, 1, 223, 446.0, 118.0 },
        /* 80*/ { BARCODE_HIBC_DM, "ABC", "", 12, 12, 12, 24.0, 24.0 },
        /* 81*/ { BARCODE_HIBC_QR, "1234567890AB", "", 21, 21, 21, 42.0, 42.0 },
        /* 82*/ { BARCODE_HIBC_PDF, "0000000000", "", 27, 9, 103, 206.0, 54.0 },
        /* 83*/ { BARCODE_HIBC_MICPDF, "0000000000", "", 34, 17, 38, 76.0, 68.0 },
        /* 84*/ { BARCODE_HIBC_BLOCKF, "0000000000", "", 30, 3, 101, 242.0, 64.0 },
        /* 85*/ { BARCODE_HIBC_AZTEC, "1234567890AB", "", 19, 19, 19, 38.0, 38.0 },
        /* 86*/ { BARCODE_DOTCODE, "ABC", "", 11, 11, 16, 32.0, 22.0 },
        /* 87*/ { BARCODE_HANXIN, "1234567890AB", "", 23, 23, 23, 46.0, 46.0 },
        /* 88*/ { BARCODE_MAILMARK, "01000000000000000AA00AA0A", "", 10, 3, 155, 310.0, 20.0 },
        /* 89*/ { BARCODE_AZRUNE, "255", "", 11, 11, 11, 22.0, 22.0 },
        /* 90*/ { BARCODE_CODE32, "12345678", "", 50, 1, 103, 206.0, 118.0 },
        /* 91*/ { BARCODE_EANX_CC, "123456789012", "[20]01", 50, 7, 99, 238.0, 118.0 },
        /* 92*/ { BARCODE_EANX_CC, "123456789012+12", "[20]01", 50, 7, 128, 296.0, 118.0 },
        /* 93*/ { BARCODE_EANX_CC, "123456789012+12345", "[20]01", 50, 7, 155, 350.0, 118.0 },
        /* 94*/ { BARCODE_EAN128_CC, "[01]12345678901234", "[20]01", 50, 5, 145, 290.0, 118.0 },
        /* 95*/ { BARCODE_RSS14_CC, "1234567890123", "[20]01", 21, 5, 100, 200.0, 60.0 },
        /* 96*/ { BARCODE_RSS_LTD_CC, "1234567890123", "[20]01", 19, 6, 74, 148.0, 56.0 },
        /* 97*/ { BARCODE_RSS_EXP_CC, "[01]12345678901234", "[20]01", 41, 5, 134, 268.0, 100.0 },
        /* 98*/ { BARCODE_UPCA_CC, "12345678901", "[20]01", 50, 7, 99, 238.0, 118.0 },
        /* 99*/ { BARCODE_UPCA_CC, "12345678901+12", "[20]01", 50, 7, 128, 296.0, 118.0 },
        /*100*/ { BARCODE_UPCA_CC, "12345678901+12345", "[20]01", 50, 7, 155, 350.0, 118.0 },
        /*101*/ { BARCODE_UPCE_CC, "1234567", "[20]01", 50, 9, 55, 150.0, 118.0 },
        /*102*/ { BARCODE_UPCE_CC, "1234567+12", "[20]01", 50, 9, 84, 208.0, 118.0 },
        /*103*/ { BARCODE_UPCE_CC, "1234567+12345", "[20]01", 50, 9, 111, 262.0, 118.0 },
        /*104*/ { BARCODE_RSS14STACK_CC, "0000000000000", "[20]01", 24, 9, 56, 112.0, 48.0 },
        /*105*/ { BARCODE_RSS14_OMNI_CC, "0000000000000", "[20]01", 80, 11, 56, 112.0, 160.0 },
        /*106*/ { BARCODE_RSS_EXPSTACK_CC, "[01]12345678901234", "[20]01", 78, 9, 102, 204.0, 156.0 },
        /*107*/ { BARCODE_CHANNEL, "00", "", 50, 1, 19, 38.0, 118.0 },
        /*108*/ { BARCODE_CODEONE, "12345678901234567890", "", 22, 22, 22, 44.0, 44.0 },
        /*109*/ { BARCODE_GRIDMATRIX, "ABC", "", 18, 18, 18, 36.0, 36.0 },
        /*110*/ { BARCODE_UPNQR, "1234567890AB", "", 77, 77, 77, 154.0, 154.0 },
        /*111*/ { BARCODE_ULTRA, "0000000000", "", 13, 13, 18, 36.0, 26.0 },
        /*112*/ { BARCODE_RMQR, "12345", "", 11, 11, 27, 54.0, 22.0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

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

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", \"%s\", %d, %d, %d, %.1f, %.1f },\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].data, data[i].composite,
                    symbol->height, symbol->rows, symbol->width, symbol->vector->width, symbol->vector->height);
        } else {
            assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %f != %f\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %f != %f\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);
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
        float expected_vector_width;
        float expected_vector_height;
        float expected_string_x;
        float expected_addon_string_x;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_EANX, -1, "123456789012", 0, 50, 1, 95, 230.0, 118.0, 10, -1 }, // EAN-13
        /*  1*/ { BARCODE_EANX, 0, "123456789012", 0, 50, 1, 95, 230.0, 118.0, -1, -1 }, // EAN-13
        /*  2*/ { BARCODE_EANX_CHK, -1, "1234567890128", 0, 50, 1, 95, 230.0, 118.0, 10, -1 }, // EAN-13
        /*  3*/ { BARCODE_EANX_CHK, 0, "1234567890128", 0, 50, 1, 95, 230.0, 118.0, -1, -1 }, // EAN-13
        /*  4*/ { BARCODE_EANX, -1, "123456789012+12", 0, 50, 1, 124, 288.0, 118.0, 10, 70 }, // EAN-13 + EAN-2
        /*  5*/ { BARCODE_EANX, 0, "123456789012+12", 0, 50, 1, 124, 288.0, 118.0, -1, -1 }, // EAN-13 + EAN-2
        /*  6*/ { BARCODE_EANX, -1, "123456789012+12345", 0, 50, 1, 151, 342.0, 118.0, 10, 70 }, // EAN-13 + EAN-5
        /*  7*/ { BARCODE_EANX, 0, "123456789012+12345", 0, 50, 1, 151, 342.0, 118.0, -1, -1 }, // EAN-13 + EAN-5
        /*  8*/ { BARCODE_ISBNX, -1, "9784567890120+12345", 0, 50, 1, 151, 342.0, 118.0, 10, 70 }, // ISBNX + EAN-5
        /*  9*/ { BARCODE_ISBNX, 0, "9784567890120+12345", 0, 50, 1, 151, 342.0, 118.0, -1, -1 }, // ISBNX + EAN-5
        /* 10*/ { BARCODE_EANX, -1, "1234567", 0, 50, 1, 67, 134.0, 118.0, 34, -1 }, // EAN-8
        /* 11*/ { BARCODE_EANX, 0, "1234567", 0, 50, 1, 67, 134.0, 118.0, -1, -1 }, // EAN-8
        /* 12*/ { BARCODE_EANX, -1, "1234567+12", 0, 50, 1, 96, 192.0, 118.0, 34, 100 }, // EAN-8 + EAN-2
        /* 13*/ { BARCODE_EANX, 0, "1234567+12", 0, 50, 1, 96, 192.0, 118.0, -1, -1 }, // EAN-8 + EAN-2
        /* 14*/ { BARCODE_EANX, -1, "1234567+12345", 0, 50, 1, 123, 246.0, 118.0, 34, 100 }, // EAN-8 + EAN-5
        /* 15*/ { BARCODE_EANX, 0, "1234567+12345", 0, 50, 1, 123, 246.0, 118.0, -1, -1 }, // EAN-8 + EAN-5
        /* 16*/ { BARCODE_UPCA, -1, "12345678901", 0, 50, 1, 95, 230.0, 118.0, 10, -1 },
        /* 17*/ { BARCODE_UPCA, 0, "12345678901", 0, 50, 1, 95, 230.0, 118.0, -1, -1 },
        /* 18*/ { BARCODE_UPCA, -1, "12345678901+12", 0, 50, 1, 124, 288.0, 118.0, 10, 74 },
        /* 19*/ { BARCODE_UPCA, 0, "12345678901+12", 0, 50, 1, 124, 288.0, 118.0, -1, -1 },
        /* 20*/ { BARCODE_UPCA_CHK, -1, "123456789012+12345", 0, 50, 1, 151, 342.0, 118.0, 10, 74 },
        /* 21*/ { BARCODE_UPCA_CHK, 0, "123456789012+12345", 0, 50, 1, 151, 342.0, 118.0, -1, -1 },
        /* 22*/ { BARCODE_UPCE, -1, "1234567", 0, 50, 1, 51, 142.0, 118.0, 10, -1 },
        /* 23*/ { BARCODE_UPCE, 0, "1234567", 0, 50, 1, 51, 142.0, 118.0, -1, -1 },
        /* 24*/ { BARCODE_UPCE_CHK, -1, "12345670+12", 0, 50, 1, 80, 200.0, 118.0, 10, 68 },
        /* 25*/ { BARCODE_UPCE_CHK, 0, "12345670+12", 0, 50, 1, 80, 200.0, 118.0, -1, -1 },
        /* 26*/ { BARCODE_UPCE, -1, "1234567+12345", 0, 50, 1, 107, 254.0, 118.0, 10, 68 },
        /* 27*/ { BARCODE_UPCE, 0, "1234567+12345", 0, 50, 1, 107, 254.0, 118.0, -1, -1 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

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

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        if (data[i].show_hrt) {
            assert_nonnull(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
            assert_equal(symbol->vector->strings->x, data[i].expected_string_x,
                "i:%d (%s) symbol->vector->strings->x %f != %f\n", i, testUtilBarcodeName(data[i].symbology), symbol->vector->strings->x, data[i].expected_string_x);

            if (data[i].expected_addon_string_x != -1) {
                assert_nonnull(symbol->vector->strings->next, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings->next NULL\n", i, data[i].symbology);
                assert_equal(symbol->vector->strings->next->x, data[i].expected_addon_string_x,
                    "i:%d (%s) symbol->vector->strings->next->x %f != %f\n", i, testUtilBarcodeName(data[i].symbology), symbol->vector->strings->next->x, data[i].expected_addon_string_x);
            }
        } else {
            assert_null(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
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
        int expected_separator_row;
        int expected_separator_col;
        int expected_separator_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODABLOCKF, -1, -1, "A", 0, 20, 2, 101, 21, 42, 2 },
        /*  1*/ { BARCODE_CODABLOCKF, -1, 0, "A", 0, 20, 2, 101, 21, 42, 2 }, // Same as default
        /*  2*/ { BARCODE_CODABLOCKF, -1, 1, "A", 0, 20, 2, 101, 21, 42, 2 }, // Same as default
        /*  3*/ { BARCODE_CODABLOCKF, -1, 2, "A", 0, 20, 2, 101, 20, 42, 4 },
        /*  4*/ { BARCODE_CODABLOCKF, -1, 3, "A", 0, 20, 2, 101, 19, 42, 6 },
        /*  5*/ { BARCODE_CODABLOCKF, -1, 4, "A", 0, 20, 2, 101, 18, 42, 8 },
        /*  6*/ { BARCODE_CODABLOCKF, -1, 5, "A", 0, 20, 2, 101, 21, 42, 2 }, // > 4 ignored, same as default
        /*  7*/ { BARCODE_CODABLOCKF, 1, -1, "A", 0, 5, 1, 46, 0, 20, 2 }, // CODE128 top separator
    };
    int data_size = ARRAY_SIZE(data);

    struct zint_vector_rect *rect;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

        rect = find_rect(symbol, data[i].expected_separator_col, data[i].expected_separator_row, data[i].expected_separator_height, 0);
        assert_nonnull(rect, "i:%d (%d) find_rect(%d, %d, %d) NULL\n", i, data[i].symbology, data[i].expected_separator_col, data[i].expected_separator_row, data[i].expected_separator_height);

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
        float expected_vector_width;
        float expected_vector_height;
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
        /*  5*/ { BARCODE_CODE128, -1, -1, -1, "A123", 0, 50, 1, 79, 158, 118, 0, 6, 8 },
        /*  6*/ { BARCODE_CODE128, 3, 4, BARCODE_BIND, "A123", 0, 50, 1, 79, 170, 134, 1, 6, 8 },
        /*  7*/ { BARCODE_CODE128, 3, 4, BARCODE_BIND, "A123", 0, 50, 1, 79, 170, 134, 0, 14, 8 },
        /*  8*/ { BARCODE_CODE128, 3, 4, BARCODE_BOX, "A123", 0, 50, 1, 79, 186, 134, 1, 14, 8 },
        /*  9*/ { BARCODE_CODE128, -1, -1, BARCODE_DOTTY_MODE, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
        /* 10*/ { BARCODE_QRCODE, -1, -1, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 6 },
        /* 11*/ { BARCODE_QRCODE, -1, 3, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 6 },
        /* 12*/ { BARCODE_QRCODE, -1, 3, BARCODE_BIND, "A123", 0, 21, 21, 21, 42, 54, 1, 0, 6 },
        /* 13*/ { BARCODE_QRCODE, -1, 3, BARCODE_BIND, "A123", 0, 21, 21, 21, 42, 54, 0, 22, 6 },
        /* 14*/ { BARCODE_QRCODE, -1, 3, BARCODE_BOX, "A123", 0, 21, 21, 21, 54, 54, 1, 22, 6 },
        /* 15*/ { BARCODE_QRCODE, -1, -1, -1, "A123", 0, 21, 21, 21, 42, 42, 0, 10, 12 },
        /* 16*/ { BARCODE_QRCODE, 5, 6, BARCODE_BIND, "A123", 0, 21, 21, 21, 62, 66, 1, 10, 12 },
        /* 17*/ { BARCODE_QRCODE, 5, 6, BARCODE_BIND, "A123", 0, 21, 21, 21, 62, 66, 0, 22, 12 },
        /* 18*/ { BARCODE_QRCODE, 5, 6, BARCODE_BOX, "A123", 0, 21, 21, 21, 86, 66, 1, 22, 12 },
        /* 19*/ { BARCODE_QRCODE, -1, -1, BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 50 },
        /* 20*/ { BARCODE_QRCODE, -1, 4, BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 42, 0, 0, 50 },
        /* 21*/ { BARCODE_QRCODE, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 58, 1, 0, 50 },
        /* 22*/ { BARCODE_QRCODE, -1, 4, BARCODE_BIND | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 42, 58, 0, 54, 0 },
        /* 23*/ { BARCODE_QRCODE, 1, 4, BARCODE_BOX | BARCODE_DOTTY_MODE, "A123", 0, 21, 21, 21, 62, 58, 1, 54, 0 },
        /* 24*/ { BARCODE_MAXICODE, -1, -1, -1, "A123", 0, 165, 33, 30, 74, 72, 0, 0, 82 },
        /* 25*/ { BARCODE_MAXICODE, -1, 5, -1, "A123", 0, 165, 33, 30, 74, 72, 0, 0, 82 },
        /* 26*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 74, 92, 1, 0, 82 },
        /* 27*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 74, 92, 0, 84, 0 },
        /* 28*/ { BARCODE_MAXICODE, -1, 5, BARCODE_BOX, "A123", 0, 165, 33, 30, 94, 92, 1, 84, 0 },
        /* 29*/ { BARCODE_MAXICODE, -1, -1, -1, "A123", 0, 165, 33, 30, 74, 72, 0, 0, 82 },
        /* 30*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 98, 92, 1, 0, 82 },
        /* 31*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BIND, "A123", 0, 165, 33, 30, 98, 92, 0, 108, 0 },
        /* 32*/ { BARCODE_MAXICODE, 6, 5, BARCODE_BOX, "A123", 0, 165, 33, 30, 118, 92, 1, 108, 0 },
        /* 33*/ { BARCODE_MAXICODE, -1, -1, BARCODE_DOTTY_MODE, "A123", ZINT_ERROR_INVALID_OPTION, -1, -1, -1, -1, -1, -1, -1, -1 },
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

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != %d\n", i, data[i].symbology, ret, data[i].ret);

        if (ret < 5) {
            assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

            if (index != -1) {
                sprintf(symbol->outfile, "test_output_options_%d.svg", i);
                ZBarcode_Print(symbol, 0);
            }

            assert_equal(symbol->height, data[i].expected_height, "i:%d (%d) symbol->height %d != %d\n", i, data[i].symbology, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%d) symbol->rows %d != %d\n", i, data[i].symbology, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d (%d) symbol->width %d != %d\n", i, data[i].symbology, symbol->width, data[i].expected_width);

            assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %f != %f\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
            assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %f != %f\n",
                i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

            if (data[i].expected_set != -1) {
                rect = find_rect(symbol, data[i].expected_set_row, data[i].expected_set_col, 0, 0);
                if (data[i].expected_set) {
                    assert_nonnull(rect, "i:%d (%d) find_rect(%d, %d, 0, 0) NULL\n", i, data[i].symbology, data[i].expected_set_row, data[i].expected_set_col);
                } else {
                    assert_null(rect, "i:%d (%d) find_rect(%d, %d, 0, 0) not NULL\n", i, data[i].symbology, data[i].expected_set_row, data[i].expected_set_col);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Checks that symbol lead-in (composite offset) isn't used to calc string position for non-composite barcodes
static void test_noncomposite_string_x(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;

        int expected_width;
        float expected_string_x;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_RSS14, "1234567890123", 96, 96 },
        /* 1*/ { BARCODE_RSS_LTD, "1234567890123", 74, 74 },
        /* 2*/ { BARCODE_RSS_EXP, "[01]12345678901234", 134, 134 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_nonnull(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
        assert_equal(symbol->vector->strings->x, data[i].expected_string_x,
            "i:%d (%s) symbol->vector->strings->x %f != %f\n", i, testUtilBarcodeName(data[i].symbology), symbol->vector->strings->x, data[i].expected_string_x);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Checks UPCA/UPCE main_symbol_width_x (used for addon formatting) set whether whitespace width set or not
static void test_upcean_whitespace_width(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        int whitespace_width;

        int expected_width;
        float expected_vector_width;
        int expected_string_cnt;
        float expected_string_y;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_UPCA, "12345678904+12345", 0, 151, 342.0, 5, 16.0 },
        /* 1*/ { BARCODE_UPCA, "12345678904+12345", 11, 151, 342.0 + 4 * (11 - 10), 5, 16.0 },
        /* 2*/ { BARCODE_UPCE, "1234567+12", 0, 80, 200.0, 4, 16.0 },
        /* 3*/ { BARCODE_UPCE, "1234567+12", 8, 80, 200.0 + 4 * (8 - 10), 4, 16.0 }, // Note: change from previous behaviour where if whitespace < 10 then set to 10
    };
    int data_size = sizeof(data) / sizeof(struct item);

    struct zint_vector_string *string;
    int string_cnt;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->whitespace_width = data[i].whitespace_width;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode(%d) ret %d != 0 %s\n", i, data[i].symbology, ret, symbol->errtxt);

        ret = ZBarcode_Buffer_Vector(symbol, 0);
        assert_zero(ret, "i:%d ZBarcode_Buffer_Vector(%d) ret %d != 0\n", i, data[i].symbology, ret);
        assert_nonnull(symbol->vector, "i:%d ZBarcode_Buffer_Vector(%d) vector NULL\n", i, data[i].symbology);

        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %f != %f\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);

        assert_nonnull(symbol->vector->strings, "i:%d ZBarcode_Buffer_Vector(%d) vector->strings NULL\n", i, data[i].symbology);
        // Get add-on string (last)
        for (string = symbol->vector->strings, string_cnt = 1; string->next; string_cnt++) {
            string = string->next;
        }
        assert_equal(string_cnt, data[i].expected_string_cnt, "i:%d (%s) string_cnt %d != %d\n", i, testUtilBarcodeName(data[i].symbology), string_cnt, data[i].expected_string_cnt);
        assert_equal(string->y, data[i].expected_string_y, "i:%d (%s) string->y %f != %f\n", i, testUtilBarcodeName(data[i].symbology), string->y, data[i].expected_string_y);

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
        { "test_output_options", test_output_options, 1, 0, 1 },
        { "test_noncomposite_string_x", test_noncomposite_string_x, 1, 0, 1 },
        { "test_upcean_whitespace_width", test_upcean_whitespace_width, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
