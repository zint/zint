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
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"

//#define TEST_BUFFER_VECTOR_GENERATE_EXPECTED 1

static void test_buffer_vector(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
        char* composite;

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
        /* 10*/ { BARCODE_EAN128, "[01]12345678901234", "", 50, 1, 134, 268.0, 118.0 },
        /* 11*/ { BARCODE_CODABAR, "A00000000B", "", 50, 1, 102, 204.0, 118.0 },
        /* 12*/ { BARCODE_CODE128, "0000000000", "", 50, 1, 90, 180.0, 118.0 },
        /* 13*/ { BARCODE_DPLEIT, "1234567890123", "", 50, 1, 135, 270.0, 118.0 },
        /* 14*/ { BARCODE_DPIDENT, "12345678901", "", 50, 1, 117, 234.0, 118.0 },
        /* 15*/ { BARCODE_CODE16K, "0000000000", "", 20, 2, 70, 212.0, 48.0 },
        /* 16*/ { BARCODE_CODE49, "0000000000", "", 20, 2, 70, 188.0, 48.0 },
        /* 17*/ { BARCODE_CODE93, "0000000000", "", 50, 1, 127, 254.0, 118.0 },
        /* 18*/ { BARCODE_FLAT, "1234567890", "", 50, 1, 90, 180.0, 100.0 },
        /* 19*/ { BARCODE_RSS14, "1234567890123", "", 50, 1, 96, 192.0, 118.0 },
        /* 20*/ { BARCODE_RSS_LTD, "1234567890123", "", 50, 1, 74, 148.0, 118.0 },
        /* 21*/ { BARCODE_RSS_EXP, "[01]12345678901234", "", 34, 1, 134, 268.0, 86.0 },
        /* 22*/ { BARCODE_TELEPEN, "0000000000", "", 50, 1, 208, 416.0, 118.0 },
        /* 23*/ { BARCODE_UPCA, "12345678904", "", 50, 1, 95, 230.0, 118.0 },
        /* 24*/ { BARCODE_UPCA_CHK, "12345678905", "", 50, 1, 95, 230.0, 118.0 },
        /* 25*/ { BARCODE_UPCE, "1234567", "", 50, 1, 51, 142.0, 118.0 },
        /* 26*/ { BARCODE_UPCE_CHK, "12345670", "", 50, 1, 51, 142.0, 118.0 },
        /* 27*/ { BARCODE_POSTNET, "00000000000", "", 12, 2, 185, 370.0, 24.0 },
        /* 28*/ { BARCODE_MSI_PLESSEY, "0000000000", "", 50, 1, 127, 254.0, 118.0 },
        /* 29*/ { BARCODE_FIM, "A", "", 50, 1, 17, 34.0, 100.0 },
        /* 30*/ { BARCODE_LOGMARS, "0000000000", "", 50, 1, 207, 414.0, 118.0 },
        /* 31*/ { BARCODE_PHARMA, "123456", "", 50, 1, 58, 116.0, 100.0 },
        /* 32*/ { BARCODE_PZN, "123456", "", 50, 1, 142, 284.0, 118.0 },
        /* 33*/ { BARCODE_PHARMA_TWO, "12345678", "", 10, 2, 29, 58.0, 20.0 },
        /* 34*/ { BARCODE_PDF417, "0000000000", "", 21, 7, 103, 206.0, 42.0 },
        /* 35*/ { BARCODE_PDF417TRUNC, "0000000000", "", 21, 7, 68, 136.0, 42.0 },
        /* 36*/ { BARCODE_MAXICODE, "0000000000", "", 165, 33, 30, 74.0, 72.0 },
        /* 37*/ { BARCODE_QRCODE, "1234567890AB", "", 21, 21, 21, 42.0, 42.0 },
        /* 38*/ { BARCODE_CODE128B, "0000000000", "", 50, 1, 145, 290.0, 118.0 },
        /* 39*/ { BARCODE_AUSPOST, "12345678901234567890123", "", 8, 3, 133, 266.0, 16.0 },
        /* 40*/ { BARCODE_AUSREPLY, "12345678", "", 8, 3, 73, 146.0, 16.0 },
        /* 41*/ { BARCODE_AUSROUTE, "12345678", "", 8, 3, 73, 146.0, 16.0 },
        /* 42*/ { BARCODE_AUSREDIRECT, "12345678", "", 8, 3, 73, 146.0, 16.0 },
        /* 43*/ { BARCODE_ISBNX, "123456789", "", 50, 1, 95, 230.0, 118.0 },
        /* 44*/ { BARCODE_RM4SCC, "0000000000", "", 8, 3, 91, 182.0, 16.0 },
        /* 45*/ { BARCODE_DATAMATRIX, "ABC", "", 10, 10, 10, 20.0, 20.0 },
        /* 46*/ { BARCODE_EAN14, "1234567890123", "", 50, 1, 134, 268.0, 118.0 },
        /* 47*/ { BARCODE_VIN, "00000000000000000", "", 50, 1, 246, 492.0, 118.0 },
        /* 48*/ { BARCODE_CODABLOCKF, "0000000000", "", 10, 1, 90, 188.0, 28.0 },
        /* 49*/ { BARCODE_NVE18, "12345678901234567", "", 50, 1, 156, 312.0, 118.0 },
        /* 50*/ { BARCODE_JAPANPOST, "0000000000", "", 8, 3, 133, 266.0, 16.0 },
        /* 51*/ { BARCODE_KOREAPOST, "123456", "", 50, 1, 167, 334.0, 118.0 },
        /* 52*/ { BARCODE_RSS14STACK, "0000000000000", "", 13, 3, 50, 100.0, 26.0 },
        /* 53*/ { BARCODE_RSS14STACK_OMNI, "0000000000000", "", 69, 5, 50, 100.0, 138.0 },
        /* 54*/ { BARCODE_RSS_EXPSTACK, "[01]12345678901234", "", 71, 5, 102, 204.0, 142.0 },
        /* 55*/ { BARCODE_PLANET, "00000000000", "", 12, 2, 185, 370.0, 24.0 },
        /* 56*/ { BARCODE_MICROPDF417, "0000000000", "", 12, 6, 82, 164.0, 24.0 },
        /* 57*/ { BARCODE_ONECODE, "12345678901234567890", "", 8, 3, 129, 258.0, 16.0 },
        /* 58*/ { BARCODE_PLESSEY, "0000000000", "", 50, 1, 227, 454.0, 118.0 },
        /* 59*/ { BARCODE_TELEPEN_NUM, "0000000000", "", 50, 1, 128, 256.0, 118.0 },
        /* 60*/ { BARCODE_ITF14, "0000000000", "", 50, 1, 135, 382.0, 150.0 },
        /* 61*/ { BARCODE_KIX, "123456ABCDE", "", 8, 3, 87, 174.0, 16.0 },
        /* 62*/ { BARCODE_AZTEC, "1234567890AB", "", 15, 15, 15, 30.0, 30.0 },
        /* 63*/ { BARCODE_DAFT, "DAFTDAFTDAFTDAFT", "", 8, 3, 31, 62.0, 16.0 },
        /* 64*/ { BARCODE_MICROQR, "12345", "", 11, 11, 11, 22.0, 22.0 },
        /* 65*/ { BARCODE_HIBC_128, "0000000000", "", 50, 1, 134, 268.0, 118.0 },
        /* 66*/ { BARCODE_HIBC_39, "0000000000", "", 50, 1, 223, 446.0, 118.0 },
        /* 67*/ { BARCODE_HIBC_DM, "ABC", "", 12, 12, 12, 24.0, 24.0 },
        /* 68*/ { BARCODE_HIBC_QR, "1234567890AB", "", 21, 21, 21, 42.0, 42.0 },
        /* 69*/ { BARCODE_HIBC_PDF, "0000000000", "", 27, 9, 103, 206.0, 54.0 },
        /* 70*/ { BARCODE_HIBC_MICPDF, "0000000000", "", 34, 17, 38, 76.0, 68.0 },
        /* 71*/ { BARCODE_HIBC_BLOCKF, "0000000000", "", 40, 4, 90, 188.0, 88.0 },
        /* 72*/ { BARCODE_HIBC_AZTEC, "1234567890AB", "", 19, 19, 19, 38.0, 38.0 },
        /* 73*/ { BARCODE_DOTCODE, "ABC", "", 11, 11, 16, 32.0, 22.0 },
        /* 74*/ { BARCODE_HANXIN, "1234567890AB", "", 23, 23, 23, 46.0, 46.0 },
        /* 75*/ { BARCODE_MAILMARK, "01000000000000000AA00AA0A", "", 10, 3, 155, 310.0, 20.0 },
        /* 76*/ { BARCODE_AZRUNE, "255", "", 11, 11, 11, 22.0, 22.0 },
        /* 77*/ { BARCODE_CODE32, "12345678", "", 50, 1, 103, 206.0, 118.0 },
        /* 78*/ { BARCODE_EANX_CC, "123456789012", "[20]01", 50, 7, 99, 238.0, 118.0 },
        /* 79*/ { BARCODE_EAN128_CC, "[01]12345678901234", "[20]01", 50, 5, 145, 290.0, 118.0 },
        /* 80*/ { BARCODE_RSS14_CC, "1234567890123", "[20]01", 21, 5, 100, 200.0, 60.0 },
        /* 81*/ { BARCODE_RSS_LTD_CC, "1234567890123", "[20]01", 19, 6, 74, 148.0, 56.0 },
        /* 82*/ { BARCODE_RSS_EXP_CC, "[01]12345678901234", "[20]01", 41, 5, 134, 268.0, 100.0 },
        /* 83*/ { BARCODE_UPCA_CC, "12345678901", "[20]01", 50, 7, 99, 238.0, 118.0 },
        /* 84*/ { BARCODE_UPCE_CC, "1234567", "[20]01", 50, 9, 55, 150.0, 118.0 },
        /* 85*/ { BARCODE_RSS14STACK_CC, "0000000000000", "[20]01", 24, 9, 56, 112.0, 48.0 },
        /* 86*/ { BARCODE_RSS14_OMNI_CC, "0000000000000", "[20]01", 80, 11, 56, 112.0, 160.0 },
        /* 87*/ { BARCODE_RSS_EXPSTACK_CC, "[01]12345678901234", "[20]01", 78, 9, 102, 204.0, 156.0 },
        /* 88*/ { BARCODE_CHANNEL, "00", "", 50, 1, 19, 38.0, 118.0 },
        /* 89*/ { BARCODE_CODEONE, "12345678901234567890", "", 22, 22, 22, 44.0, 44.0 },
        /* 90*/ { BARCODE_GRIDMATRIX, "ABC", "", 18, 18, 18, 36.0, 36.0 },
        /* 91*/ { BARCODE_UPNQR, "1234567890AB", "", 77, 77, 77, 154.0, 154.0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char* text;

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;

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

        #ifdef TEST_BUFFER_VECTOR_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", \"%s\", %d, %d, %d, %.1f, %.1f },\n",
                i, testUtilBarcodeName(data[i].symbology), data[i].data, data[i].composite,
                symbol->height, symbol->rows, symbol->width, symbol->vector->width, symbol->vector->height);

        #else

        assert_equal(symbol->height, data[i].expected_height, "i:%d (%s) symbol->height %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->height, data[i].expected_height);
        assert_equal(symbol->rows, data[i].expected_rows, "i:%d (%s) symbol->rows %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->rows, data[i].expected_rows);
        assert_equal(symbol->width, data[i].expected_width, "i:%d (%s) symbol->width %d != %d\n", i, testUtilBarcodeName(data[i].symbology), symbol->width, data[i].expected_width);
        assert_equal(symbol->vector->width, data[i].expected_vector_width, "i:%d (%s) symbol->vector->width %f != %f\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->width, data[i].expected_vector_width);
        assert_equal(symbol->vector->height, data[i].expected_vector_height, "i:%d (%s) symbol->vector->height %f != %f\n",
            i, testUtilBarcodeName(data[i].symbology), symbol->vector->height, data[i].expected_vector_height);

        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Checks that symbol lead-in (composite offset) isn't used to calc string position for non-composite barcodes
static void test_noncomposite_string_x(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;

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

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;

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
static void test_upcean_whitespace_width(void)
{
    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char* data;
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

    struct zint_vector_string* string;
    int string_cnt;

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->input_mode = UNICODE_MODE;
        symbol->whitespace_width = data[i].whitespace_width;

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

int main()
{
    test_buffer_vector();
    test_noncomposite_string_x();
    test_upcean_whitespace_width();

    testReport();

    return 0;
}
