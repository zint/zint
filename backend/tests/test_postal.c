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

// USPS Publication 25 (July 2003) Designing Letter and Reply Mail https://web.archive.org/web/20050118015758/http://www.siemons.com/forms/pdf/designing_letter_reply_mail.pdf
// USPS DMM Domestic Mail Manual https://pe.usps.com/DMM300
// USPS Publication 197 (Sept 2004) Confirm User Guide https://web.archive.org/web/20060505214851/https://mailtracking.usps.com/mtr/resources/documents/Guide.pdf

#include "testcommon.h"

static void test_large(int index, int debug) {

    struct item {
        int symbology;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_FLAT, "1", 90, 0, 1, 810 },
        /*  1*/ { BARCODE_FLAT, "1", 91, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_POSTNET, "1", 11, 0, 2, 123 },
        /*  3*/ { BARCODE_POSTNET, "1", 12, ZINT_WARN_NONCOMPLIANT, 2, 133 },
        /*  4*/ { BARCODE_POSTNET, "1", 38, ZINT_WARN_NONCOMPLIANT, 2, 393 },
        /*  5*/ { BARCODE_POSTNET, "1", 39, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_FIM, "D", 1, 0, 1, 17 },
        /*  7*/ { BARCODE_FIM, "D", 2, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_RM4SCC, "1", 50, 0, 3, 411 },
        /*  9*/ { BARCODE_RM4SCC, "1", 51, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { BARCODE_JAPANPOST, "1", 20, 0, 3, 133 },
        /* 11*/ { BARCODE_JAPANPOST, "1", 21, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 12*/ { BARCODE_KOREAPOST, "1", 6, 0, 1, 162 },
        /* 13*/ { BARCODE_KOREAPOST, "1", 7, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { BARCODE_PLANET, "1", 13, 0, 2, 143 },
        /* 15*/ { BARCODE_PLANET, "1", 14, ZINT_WARN_NONCOMPLIANT, 2, 153 },
        /* 16*/ { BARCODE_PLANET, "1", 38, ZINT_WARN_NONCOMPLIANT, 2, 393 },
        /* 17*/ { BARCODE_PLANET, "1", 39, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 18*/ { BARCODE_KIX, "1", 18, 0, 3, 143 },
        /* 19*/ { BARCODE_KIX, "1", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 20*/ { BARCODE_DAFT, "D", 50, 0, 3, 99 },
        /* 21*/ { BARCODE_DAFT, "D", 51, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[4096];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_koreapost(int index, int debug) {

    struct item {
        char *data;
        int ret_encode;
        int ret_vector;

        float expected_height;
        int expected_rows;
        int expected_width;
    };
    struct item data[] = {
        /* 0*/ { "123456", 0, 0, 50, 1, 167 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_koreapost");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_KOREAPOST, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->height, data[i].expected_height, "i:%d symbol->height %.8g != %.8g\n", i, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_japanpost(int index, int debug) {

    struct item {
        char *data;
        int ret_encode;
        int ret_vector;

        float expected_height;
        int expected_rows;
        int expected_width;
        char *comment;
    };
    struct item data[] = {
        /* 0*/ { "123", 0, 0, 8, 3, 133, "Check 3" },
        /* 1*/ { "123456-AB", 0, 0, 8, 3, 133, "Check 10" },
        /* 2*/ { "123456", 0, 0, 8, 3, 133, "Check 11" },
        /* 3*/ { "999980-KZ", 0, 0, 8, 3, 133, "Check 18" },
        /* 4*/ { "987654-TU", 0, 0, 8, 3, 133, "Check 0" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_japanpost");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_JAPANPOST, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret_encode);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->height, data[i].expected_height, "i:%d symbol->height %.8g != %.8g\n", i, symbol->height, data[i].expected_height);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int debug) {

    struct item {
        int symbology;
        int option_2;
        float height;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        float expected_height;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_FLAT, -1, 0, "1234567890", 0, 1, 90, 50 },
        /*  1*/ { BARCODE_FLAT, -1, 0, "A", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /*  2*/ { BARCODE_POSTNET, -1, 0, "12345", 0, 2, 63, 12 },
        /*  3*/ { BARCODE_POSTNET, -1, 0, "123457689", 0, 2, 103, 12 },
        /*  4*/ { BARCODE_POSTNET, -1, 0, "12345768901", 0, 2, 123, 12 },
        /*  5*/ { BARCODE_POSTNET, -1, 0, "0", ZINT_WARN_NONCOMPLIANT, 2, 23, 12 },
        /*  6*/ { BARCODE_POSTNET, -1, 0, "1234", ZINT_WARN_NONCOMPLIANT, 2, 53, 12 },
        /*  7*/ { BARCODE_POSTNET, -1, 0, "123456", ZINT_WARN_NONCOMPLIANT, 2, 73, 12 },
        /*  8*/ { BARCODE_POSTNET, -1, 0, "123456789012", ZINT_WARN_NONCOMPLIANT, 2, 133, 12 },
        /*  9*/ { BARCODE_POSTNET, -1, 0, "1234A", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /* 10*/ { BARCODE_POSTNET, -1, 0.9, "12345", 0, 2, 63, 1 },
        /* 11*/ { BARCODE_FIM, -1, 0, "a", 0, 1, 17, 50 },
        /* 12*/ { BARCODE_FIM, -1, 0, "b", 0, 1, 17, 50 },
        /* 13*/ { BARCODE_FIM, -1, 0, "c", 0, 1, 17, 50 },
        /* 14*/ { BARCODE_FIM, -1, 0, "d", 0, 1, 17, 50 },
        /* 15*/ { BARCODE_FIM, -1, 0, "ad", ZINT_ERROR_TOO_LONG, -1, -1, -1 },
        /* 16*/ { BARCODE_FIM, -1, 0, "e", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /* 17*/ { BARCODE_RM4SCC, -1, 0, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 3, 299, 8 },
        /* 18*/ { BARCODE_RM4SCC, -1, 0, "a", 0, 3, 19, 8 }, // Converts to upper
        /* 19*/ { BARCODE_RM4SCC, -1, 0, ",", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /* 20*/ { BARCODE_JAPANPOST, -1, 0, "1234567890-ABCDEFGH", 0, 3, 133, 8 },
        /* 21*/ { BARCODE_JAPANPOST, -1, 0, "a", 0, 3, 133, 8 }, // Converts to upper
        /* 22*/ { BARCODE_JAPANPOST, -1, 0, ",", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /* 23*/ { BARCODE_KOREAPOST, -1, 0, "123456", 0, 1, 167, 50 },
        /* 24*/ { BARCODE_KOREAPOST, -1, 0, "A", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /* 25*/ { BARCODE_PLANET, -1, 0, "12345678901", 0, 2, 123, 12 },
        /* 26*/ { BARCODE_PLANET, -1, 0, "1234567890123", 0, 2, 143, 12 },
        /* 27*/ { BARCODE_PLANET, -1, 0, "0", ZINT_WARN_NONCOMPLIANT, 2, 23, 12 },
        /* 28*/ { BARCODE_PLANET, -1, 0, "1234567890", ZINT_WARN_NONCOMPLIANT, 2, 113, 12 },
        /* 29*/ { BARCODE_PLANET, -1, 0, "123456789012", ZINT_WARN_NONCOMPLIANT, 2, 133, 12 },
        /* 30*/ { BARCODE_PLANET, -1, 0, "12345678901234", ZINT_WARN_NONCOMPLIANT, 2, 153, 12 },
        /* 31*/ { BARCODE_PLANET, -1, 0, "1234567890A", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /* 32*/ { BARCODE_KIX, -1, 0, "0123456789ABCDEFGH", 0, 3, 143, 8 },
        /* 33*/ { BARCODE_KIX, -1, 0, "a", 0, 3, 7, 8 }, // Converts to upper
        /* 34*/ { BARCODE_KIX, -1, 0, ",", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /* 35*/ { BARCODE_DAFT, -1, 0, "DAFT", 0, 3, 7, 8 },
        /* 36*/ { BARCODE_DAFT, -1, 0, "a", 0, 3, 1, 8 }, // Converts to upper
        /* 37*/ { BARCODE_DAFT, -1, 0, "B", ZINT_ERROR_INVALID_DATA, -1, -1, -1 },
        /* 38*/ { BARCODE_DAFT, -1, 1.9, "DAFT", 0, 3, 7, 2 },
        /* 39*/ { BARCODE_DAFT, 500, 0.9, "DAFT", 0, 3, 7, 2 }, // 50% ratio
        /* 40*/ { BARCODE_DAFT, 500, 0.4, "DAFT", 0, 3, 7, 8 }, // 50% ratio
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].height) {
            symbol->height = data[i].height;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
            assert_equal(symbol->height, data[i].expected_height, "i:%d symbol->height %g != %g\n", i, symbol->height, data[i].expected_height);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        int symbology;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_FLAT, "1304056", 0, 1, 63, "Verified manually against tec-it",
                    "100000000001000000000000000000100000000000000000010000000001000"
                },
        /*  1*/ { BARCODE_POSTNET, "12345678901", 0, 2, 123, "USPS Publication 25 (2003) Exhibit 4-1",
                    "100000001010000010001000001010000010000010001000100000101000001000000010100000100010001000001010000000000000101000100000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                },
        /*  2*/ { BARCODE_POSTNET, "555551237", 0, 2, 103, "Verified manually against tec-it",
                    "1000100010000010001000001000100000100010000010001000000000101000001000100000101000100000001000001000101"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                },
        /*  3*/ { BARCODE_FIM, "C", 0, 1, 17, "USPS DMM Exhibit 8.2.0 FIM C",
                    "10100010001000101"
                },
        /*  4*/ { BARCODE_RM4SCC, "BX11LT1A", 0, 3, 75, "Verified manually against tec-it",
                    "100010001010100000000010100000101010000010100010000000101000100010100000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001010000010000010001000100010001010000010101000000010001010001000000010101"
                },
        /*  5*/ { BARCODE_RM4SCC, "W1J0TR01", 0, 3, 75, "Verified manually against tec-it",
                    "101010000000001010100000100000101010001000100010000000101000001010101000001"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "000010100000100010001000100000101010100000100000100000101000100010001010001"
                },
        /*  6*/ { BARCODE_JAPANPOST, "15400233-16-4-205", 0, 3, 133, "Zip/Barcode Manual p.6 1st example; verified manually against tec-it",
                    "1000101000100010101000100000100000100010001010001010001000101000001010001000101000001000100010100000100010000010000010000010001010001"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "1010101000100010100010100000100000101000101000101000001000101000100010001000100010001000101000100000100010001000001000001000100010101"
                },
        /*  7*/ { BARCODE_JAPANPOST, "350110622-1A308", 0, 3, 133, "Zip/Barcode Manual p.6 2nd example; verified manually against tec-it",
                    "1000001010100010100000101000101000100000001010100010100010001000101000001000100000001010100000100010000010000010000010000010100010001"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "1010101000100010100000101000101000100000100010101000101000001000101000100000100000101000100000001010001000001000001000001000100010101"
                },
        /*  8*/ { BARCODE_JAPANPOST, "12345671-2-3", 0, 3, 133, "Verified manually against tec-it",
                    "1000101000100010001010101000100010001010101000101000001000100010001000001010000010000010000010000010000010000010000010000010100010001"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "1010101000101000101000100010100010100010001010101000001000101000001000101000001000001000001000001000001000001000001000001000100010101"
                },
        /*  9*/ { BARCODE_KOREAPOST, "010230", 0, 1, 167, "Verified manually against tec-it",
                    "10001000100000000000100010000000000010001000100000001000000010001000100010001000100000000000100000000001000100010001000100010001000000000001000000010001000000010001000"
                },
        /* 10*/ { BARCODE_KOREAPOST, "923457", 0, 1, 168, "Verified manually against tec-it",
                    "000010001000100000001000100000001000000010001000000010001000000010001000100000000000100010001000000010000000100010001000100010000000100000001000100010001000000000001000"
                },
        /* 11*/ { BARCODE_PLANET, "4012345235636", 0, 2, 143, "USPS Publication 197 (2004) Exhibit 4; verified manually against tec-it",
                    "10100010100000001010101010100000101000100010100000101000101000100010001010100010001010000010100010001010000010101010000010100000101010000010101"
                    "10101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                },
        /* 12*/ { BARCODE_PLANET, "40123452356", 0, 2, 123, "Verified manually against tec-it",
                    "101000101000000010101010101000001010001000101000001010001010001000100010101000100010100000101000100010100000101010001000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                },
        /* 13*/ { BARCODE_PLANET, "5020140235635", 0, 2, 143, "USPS Publication 197 (2004) Exhibit 6; verified manually against tec-it",
                    "10100010001000001010101010001000000010101010101000001000101000000010101010100010001010000010100010001010000010101010000010100010001010001010001"
                    "10101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                },
        /* 14*/ { BARCODE_KIX, "2500GG30250", 0, 3, 87, "PostNL Handleiding KIX code Section 2.1 Example 1",
                    "000010100000101000001010000010100010100000101000000010100000101000001010000010100000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001010001010000000001010000010101000100010001000100000100000101000101000101000000000101"
                },
        /* 15*/ { BARCODE_KIX, "2130VA80430", 0, 3, 87, "PostNL Handleiding KIX code Section 2.1 Example 2",
                    "000010100000101000001010000010101010000000100010001000100000101000001010000010100000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001010000010001010000010000010100010001010001000001010000000101010001000100000100000101"
                },
        /* 16*/ { BARCODE_KIX, "1231GF156X2", 0, 3, 87, "PostNL Handleiding KIX code Section 2.1 Example 3",
                    "000010100000101000001010000010100010100000101000000010100000101000100010101000000000101"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001000100010100010000010001000101000100010000010001000101010000000001010100000100010100"
                },
        /* 17*/ { BARCODE_KIX, "1231FZ13Xhs", 0, 3, 87, "PostNL Handleiding KIX code Section 2.1 Example 4",
                    "000010100000101000001010000010100010100010100000000010100000101010100000001010001000100"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101"
                    "001000100010100010000010001000101000001010100000001000101000001010000010101000001000100"
                },
        /* 18*/ { BARCODE_DAFT, "DAFTTFADFATDTATFT", 0, 3, 33, "Verified manually against tec-it",
                    "001010000010100010100000001000100"
                    "101010101010101010101010101010101"
                    "100010000010001010000010000000100"
                },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char bwipp_buf[8192];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    testStart("test_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
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

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, -1, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_koreapost", test_koreapost, 1, 0, 1 },
        { "test_japanpost", test_japanpost, 1, 0, 1 },
        { "test_input", test_input, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
