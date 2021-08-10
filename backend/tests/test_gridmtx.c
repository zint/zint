/*
    libzint - the open source barcode library
    Copyright (C) 2019-2021 Robin Stuart <rstuart114@gmail.com>

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
/* vim: set ts=4 sw=4 et norl : */

#include "testcommon.h"

static void test_large(int index, int debug) {

    struct item {
        int option_2;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -1, "1", 2751, 0, 162, 162 },
        /*  1*/ { -1, "1", 2752, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { -1, "1", 2755, ZINT_ERROR_TOO_LONG, -1, -1 }, // Triggers buffer > 9191
        /*  3*/ { -1, "A", 1836, 0, 162, 162 },
        /*  4*/ { -1, "A", 1837, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  5*/ { -1, "\200", 1143, 0, 162, 162 },
        /*  6*/ { -1, "\200", 1144, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  7*/ { 1, "1", 18, 0, 18, 18 },
        /*  8*/ { 1, "1", 19, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  9*/ { 1, "A", 13, 0, 18, 18 },
        /* 10*/ { 1, "A", 14, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 11*/ { 1, "\200", 7, 0, 18, 18 },
        /* 12*/ { 1, "\200", 8, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 13*/ { 11, "1", 1995, 0, 138, 138 },
        /* 14*/ { 11, "1", 1996, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[2755 + 1];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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

static void test_options(int index, int debug) {

    struct item {
        char *data;
        int option_1;
        int option_2;
        int ret_encode;
        int ret_vector;
        int expected_size;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "12345", 0, 0, 0, 0, 18 },
        /*  1*/ { "12345", 0, 1, 0, 0, 18 },
        /*  2*/ { "12345", 0, 2, 0, 0, 30 },
        /*  3*/ { "12345", 0, 14, 0, 0, 18 }, // Version > max version 13 so ignored
        /*  4*/ { "12345", 0, 13, 0, 0, 162 },
        /*  5*/ { "1234567890123456789", 0, 1, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { "1234567890123456789", 0, 2, 0, 0, 30 },
        /*  7*/ { "123456789012345678", 0, 0, 0, 0, 30 }, // Version auto-set to 2
        /*  8*/ { "123456789012345678", 0, 1, 0, 0, 18 },
        /*  9*/ { "123456789012345678", 5, 1, 0, 0, 18 }, // Version specified so overrides ECC level which gets reduced to 4
        /* 10*/ { "123456789012345678", 5, 0, 0, 0, 30 }, // Version not specified so increased to allow for ECC level
        /* 11*/ { "123456789012345678", 6, 0, 0, 0, 30 }, // ECC > max ECC 5 so ignored and auto-settings version 2, ECC 4 used
        /* 12*/ { "123456789012345678", 1, 0, 0, 0, 30 }, // ECC < min ECC 2, ECC 2 used
        /* 13*/ { "123456789012345678", 4, 1, 0, 0, 18 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
            assert_equal(symbol->width, data[i].expected_size, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_size);
            assert_equal(symbol->rows, data[i].expected_size, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_size);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int eci;
        int output_options;
        int option_3;
        char *data;
        int ret;
        int expected_eci;
        char *expected;
        char *comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 2312 0xA8A6, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in GB 2312 0xA6C2, UTF-8 CEB2
    // ÿ U+00FF in ISO 8859-1 0xFF, not in GB 2312, outside first byte and second byte range, UTF-8 C3BF
    // ㈩ U+3229 in GB 2312 0x226E
    // 一 U+4E00 in GB 2312 0x523B
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, -1, "é", 0, 0, "30 01 69 00", "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, -1, -1, "é", 0, 3, "60 01 58 00 74 40", "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 29, -1, -1, "é", 0, 29, "60 0E 44 2A 37 7C 00", "ECI-29 H1 (GB 2312)" },
        /*  3*/ { UNICODE_MODE, 26, -1, -1, "é", 0, 26, "60 0D 18 01 61 6A 20", "ECI-26 B2 (UTF-8)" },
        /*  4*/ { UNICODE_MODE, 26, -1, ZINT_FULL_MULTIBYTE, "é", 0, 26, "60 0D 05 28 4F 7C 00", "ECI-26 H1 (UTF-8) (full multibyte)" },
        /*  5*/ { DATA_MODE, 0, -1, -1, "é", 0, 0, "30 03 43 54 40", "B2 (UTF-8)" },
        /*  6*/ { DATA_MODE, 0, -1, ZINT_FULL_MULTIBYTE, "é", 0, 0, "0A 51 1F 78 00", "H1 (UTF-8) (full multibyte)" },
        /*  7*/ { DATA_MODE, 0, -1, -1, "\351", 0, 0, "30 01 69 00", "B1 (ISO 8859-1) (0xE9)" },
        /*  8*/ { UNICODE_MODE, 0, -1, -1, "β", 0, 0, "08 40 2F 78 00", "H1 (GB 2312)" },
        /*  9*/ { UNICODE_MODE, 9, -1, -1, "β", 0, 9, "60 04 58 00 71 00", "ECI-9 B1 (ISO 8859-7)" },
        /* 10*/ { UNICODE_MODE, 29, -1, -1, "β", 0, 29, "60 0E 44 20 17 7C 00", "ECI-29 H1 (GB 2312)" },
        /* 11*/ { UNICODE_MODE, 26, -1, -1, "β", 0, 26, "60 0D 18 01 67 2C 40", "ECI-26 H1 (UTF-8)" },
        /* 12*/ { UNICODE_MODE, 26, -1, ZINT_FULL_MULTIBYTE, "β", 0, 26, "60 0D 05 6B 17 7C 00", "ECI-26 H1 (UTF-8) (full multibyte)" },
        /* 13*/ { DATA_MODE, 0, -1, -1, "β", 0, 0, "30 03 4E 59 00", "B2 (UTF-8)" },
        /* 14*/ { DATA_MODE, 0, -1, ZINT_FULL_MULTIBYTE, "β", 0, 0, "0B 56 2F 78 00", "H1 (UTF-8) (full multibyte)" },
        /* 15*/ { UNICODE_MODE, 0, -1, -1, "ÿ", 0, 0, "30 01 7F 00", "B1 (ISO 8859-1)" },
        /* 16*/ { UNICODE_MODE, 0, -1, -1, "ÿÿÿ", 0, 0, "30 05 7F 7F 7F 60", "B3 (ISO 8859-1)" },
        /* 17*/ { UNICODE_MODE, 0, -1, -1, "㈩一", 0, 0, "08 15 68 0E 7F 70 00", "H2 (GB 2312)" },
        /* 18*/ { UNICODE_MODE, 29, -1, -1, "㈩一", 0, 29, "60 0E 44 0A 74 07 3F 78 00", "ECI-29 H2 (GB 2312)" },
        /* 19*/ { DATA_MODE, 0, -1, -1, "\177\177", 0, 0, "30 02 7F 3F 40", "B2 (ASCII)" },
        /* 20*/ { DATA_MODE, 0, -1, -1, "\177\177\177", 0, 0, "30 04 7F 3F 5F 60", "B3 (ASCII)" },
        /* 21*/ { UNICODE_MODE, 0, -1, -1, "123", 0, 0, "10 1E 7F 68", "N3 (ASCII)" },
        /* 22*/ { UNICODE_MODE, 0, -1, -1, " 123", 0, 0, "11 7A 03 6F 7D 00", "N4 (ASCII)" },
        /* 23*/ { UNICODE_MODE, 0, -1, -1, "1+23", 0, 0, "11 7B 03 6F 7D 00", "N4 (ASCII)" },
        /* 24*/ { UNICODE_MODE, 0, -1, -1, "12.3", 0, 0, "11 7C 63 6F 7D 00", "N4 (ASCII)" },
        /* 25*/ { UNICODE_MODE, 0, -1, -1, "123,", 0, 0, "10 1E 7F 73 76 5E 60", "N3 L1 (ASCII)" },
        /* 26*/ { UNICODE_MODE, 0, -1, -1, "123,4", 0, 0, "14 1E 7F 51 48 3F 50", "N5 (ASCII)" },
        /* 27*/ { UNICODE_MODE, 0, -1, -1, "\015\012123", 0, 0, "11 7D 63 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 28*/ { UNICODE_MODE, 0, -1, -1, "1\015\01223", 0, 0, "11 7E 03 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 29*/ { UNICODE_MODE, 0, -1, -1, "12\015\0123", 0, 0, "11 7E 23 6F 7D 00", "N4 (ASCII) (EOL)" },
        /* 30*/ { UNICODE_MODE, 0, -1, -1, "123\015\012", 0, 0, "10 1E 7F 7C 01 06 42 40", "N3 B2 (ASCII) (EOL)" },
        /* 31*/ { UNICODE_MODE, 0, -1, -1, "123\015\0124", 0, 0, "14 1E 7F 5D 48 3F 50", "N5 (ASCII) (EOL)" },
        /* 32*/ { UNICODE_MODE, 0, -1, -1, "2.2.0", 0, 0, "15 7C 46 73 78 40 07 7A", "N5 (ASCII)" },
        /* 33*/ { UNICODE_MODE, 0, -1, -1, "2.2.0.5", 0, 0, "30 0C 32 17 0C 45 63 01 38 6A 00", "B7 (ASCII)" },
        /* 34*/ { UNICODE_MODE, 0, -1, -1, "2.2.0.56", 0, 0, "13 7C 46 73 78 40 07 71 46 0F 74", "N8 (ASCII)" },
        /* 35*/ { UNICODE_MODE, 0, -1, -1, "20.12.13.\015\012", 0, 0, "11 7C 66 27 79 0D 2F 7F 00 45 60 68 28 00", "N8 B3 (ASCII)" },
        /* 36*/ { UNICODE_MODE, 0, -1, -1, "ABCDE\011F", 0, 0, "20 01 08 32 3E 49 17 30", "U7 (ASCII)" },
        /* 37*/ { UNICODE_MODE, 0, -1, -1, "1 1234ABCD12.2abcd-12", 0, 0, "13 7A 23 41 2A 3F 68 01 08 3E 4F 66 1E 5F 70 00 44 1F 2F 6E 0F 0F 74", "N6 U4 N4 L4 N3 (ASCII)" },
        /* 38*/ { UNICODE_MODE, 0, -1, -1, "1 123ABCDE12.2abcd-12", 0, 0, "28 1F 40 42 06 28 59 43 27 01 05 7D 56 42 49 16 34 7F 6D 30 08 2F 60", "M21 (ASCII)" },
        /* 39*/ { UNICODE_MODE, 0, -1, -1, "国外通信教材 Matlab6.5", 0, 0, "09 63 27 20 4E 24 1F 05 21 58 22 13 7E 1E 4C 78 09 56 00 3D 3F 4A 45 3F 50", "H6 U2 L5 N3 (GB 2312) (Same as D.2 example)" },
        /* 40*/ { UNICODE_MODE, 0, -1, -1, "AAT", 0, 0, "20 00 4F 30", "U3 (ASCII)" },
        /* 41*/ { UNICODE_MODE, 0, -1, -1, "aat", 0, 0, "18 00 4F 30", "L3 (ASCII)" },
        /* 42*/ { UNICODE_MODE, 0, -1, -1, "AAT2556", 0, 0, "20 00 4F 58 7F 65 47 7A", "U3 N4 (ASCII) (note same bit count as M7)" },
        /* 43*/ { UNICODE_MODE, 0, -1, -1, "AAT2556 ", 0, 0, "29 22 4E 42 0A 14 37 6F 60", "M8 (ASCII)" },
        /* 44*/ { UNICODE_MODE, 0, -1, -1, "AAT2556 电", 0, 0, "29 22 4E 42 0A 14 37 6F 62 2C 1F 7E 00", "M8 H1 (GB 2312)" },
        /* 45*/ { UNICODE_MODE, 0, -1, -1, " 200", 0, 0, "11 7A 06 23 7D 00", "N4 (ASCII)" },
        /* 46*/ { UNICODE_MODE, 0, -1, -1, " 200mA至", 0, 0, "2F 60 40 00 60 2B 78 63 41 7F 40", "M6 H1 (GB 2312)" },
        /* 47*/ { UNICODE_MODE, 0, -1, -1, "2A tel:86 019 82512738", 0, 0, "28 22 5F 4F 29 48 5F 6D 7E 6F 55 57 1F 28 63 0F 5A 11 64 0F 74", "M2 L5(with control) N15 (ASCII)" },
        /* 48*/ { UNICODE_MODE, 0, -1, -1, "至2A tel:86 019 82512738", 0, 0, "30 07 56 60 4C 48 13 6A 32 17 7B 3F 5B 75 35 67 6A 18 63 76 44 39 03 7D 00", "B4 L5(with control) N15 (GB 2312)" },
        /* 49*/ { UNICODE_MODE, 0, -1, -1, "AAT2556 电池充电器＋降压转换器 200mA至2A tel:86 019 82512738", 0, 0, "(62) 29 22 22 1C 4E 41 42 7E 0A 40 14 00 37 7E 6F 00 62 7E 2C 00 1C 7E 4B 00 41 7E 18 00", "M8 H11 M6 B4 L5(with control) N15 (GB 2312) (*NOT SAME* as D3 example Figure D.1, M8 H11 M6 H1 M3 L4(with control) N15, which uses a few more bits)" },
        /* 50*/ { UNICODE_MODE, 0, -1, -1, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::", 0, 0, "(588) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B512 (ASCII)" },
        /* 51*/ { UNICODE_MODE, 0, -1, -1, "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\177", 0, 0, "(591) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B513 (ASCII)" },
        /* 52*/ { UNICODE_MODE, 0, -1, -1, ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::至", 0, 0, "(591) 37 68 68 68 68 68 74 7C 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B511 H1 (GB 2312)" },
        /* 53*/ { UNICODE_MODE, 0, -1, -1, ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::至:", 0, 0, "(592) 37 68 68 68 68 68 74 7E 74 74 74 74 74 3A 3A 3A 3A 3A 3A 3A 1D 1D 1D 1D 1D 1D 1D 0E", "B513 (GB 2312)" },
        /* 54*/ { UNICODE_MODE, 0, -1, -1, "电电123456", 0, 0, "09 30 72 61 7F 70 41 76 72 1F 68", "H2 (GB 2312) N6" },
        /* 55*/ { UNICODE_MODE, 0, -1, -1, "电电abcdef", 0, 0, "09 30 72 61 7F 71 00 08 43 10 5D 40", "H2 (GB 2312) L6" },
        /* 56*/ { UNICODE_MODE, 0, -1, -1, "电电电电电\011\011\011", 0, 0, "09 30 72 61 65 43 4B 07 16 0F 7F 14 02 04 42 21 10", "H5 (GB 2312) B3" },
        /* 57*/ { UNICODE_MODE, 0, -1, -1, "1234567电电", 0, 0, "14 1E 6E 22 5E 3F 59 30 72 61 7F 70 00", "N7 H2 (GB 2312)" },
        /* 58*/ { UNICODE_MODE, 0, -1, -1, "12345678mA 2", 0, 0, "12 1E 6E 23 06 3F 76 02 5F 02 7E 00", "N8 M4" },
        /* 59*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFG电电", 0, 0, "20 01 08 32 0A 37 05 43 4B 07 7F 40", "U7 H2 (GB 2312)" },
        /* 60*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHIJ8mA 2", 0, 0, "20 01 08 32 0A 31 68 27 70 46 02 5F 02 7E 00", "U10 M5" },
        /* 61*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHIJ\011\011\011\011", 0, 0, "20 01 08 32 0A 31 68 27 78 03 04 42 21 10 48 00", "U10 B4" },
        /* 62*/ { UNICODE_MODE, 0, -1, -1, "8mA B123456789", 0, 0, "29 0C 05 3E 17 7C 40 7B 39 0C 2B 7E 40", "M5 N9" },
        /* 63*/ { UNICODE_MODE, 0, -1, -1, "8mA aABCDEFGH", 0, 0, "29 0C 05 3E 49 7D 00 04 21 48 29 47 6C", "M5 U8" },
        /* 64*/ { UNICODE_MODE, 0, -1, -1, "\011\011\011\011123456", 0, 0, "30 06 09 04 42 21 12 03 6D 64 3F 50", "B4 N6" },
        /* 65*/ { UNICODE_MODE, 0, -1, -1, "\011\011\011\011ABCDEF", 0, 0, "30 06 09 04 42 21 14 00 11 06 21 3B", "B4 U6" },
        /* 66*/ { UNICODE_MODE, 0, -1, -1, "\011\011\011\0118mA 2", 0, 0, "30 06 09 04 42 21 15 11 40 57 60 5F 40", "B4 M5" },
        /* 67*/ { UNICODE_MODE, 0, -1, -1, "电电电电电\015\012", 0, 0, "09 30 72 61 65 43 4B 07 16 0F 73 03 7E 00", "H7 (GB 2312)" },
        /* 68*/ { UNICODE_MODE, 0, -1, -1, "电电电电电12", 0, 0, "09 30 72 61 65 43 4B 07 16 0F 7B 37 7E 00", "H7 (GB 2312)" },
        /* 69*/ { UNICODE_MODE, 0, -1, -1, "1234567.8\015\012123456", 0, 0, "10 1E 6E 23 79 30 67 77 0F 37 11 7E 40", "N17" },
        /* 70*/ { UNICODE_MODE, 0, -1, -1, "˘", ZINT_WARN_USES_ECI, 4, "Warning 60 02 18 00 51 00", "ECI-4 B1 (ISO 8859-2)" },
        /* 71*/ { UNICODE_MODE, 4, -1, -1, "˘", 0, 4, "60 02 18 00 51 00", "ECI-4 B1 (ISO 8859-2)" },
        /* 72*/ { UNICODE_MODE, 0, -1, -1, "Ħ", ZINT_WARN_USES_ECI, 5, "Warning 60 02 58 00 50 40", "ECI-5 B1 (ISO 8859-3)" },
        /* 73*/ { UNICODE_MODE, 5, -1, -1, "Ħ", 0, 5, "60 02 58 00 50 40", "ECI-5 B1 (ISO 8859-3)" },
        /* 74*/ { UNICODE_MODE, 6, -1, -1, "ĸ", 0, 6, "60 03 18 00 51 00", "ECI-6 B1 (ISO 8859-4)" },
        /* 75*/ { UNICODE_MODE, 7, -1, -1, "Ж", 0, 7, "60 03 58 00 5B 00", "ECI-7 B1 (ISO 8859-5)" },
        /* 76*/ { UNICODE_MODE, 0, -1, -1, "Ș", ZINT_WARN_USES_ECI, 18, "Warning 60 09 18 00 55 00", "ECI-18 B1 (ISO 8859-16)" },
        /* 77*/ { UNICODE_MODE, 18, -1, -1, "Ș", 0, 18, "60 09 18 00 55 00", "ECI-18 B1 (ISO 8859-16)" },
        /* 78*/ { UNICODE_MODE, 0, -1, -1, "テ", 0, 0, "08 34 6F 78 00", "H1 (GB 2312)" },
        /* 79*/ { UNICODE_MODE, 20, -1, -1, "テ", 0, 20, "60 0A 18 01 41 59 20", "ECI-20 B2 (SHIFT JIS)" },
        /* 80*/ { UNICODE_MODE, 20, -1, -1, "テテ", 0, 20, "60 0A 18 03 41 59 30 36 28 00", "ECI-20 B4 (SHIFT JIS)" },
        /* 81*/ { UNICODE_MODE, 20, -1, -1, "\\\\", 0, 20, "60 0A 18 03 40 57 70 15 78 00", "ECI-20 B4 (SHIFT JIS)" },
        /* 82*/ { UNICODE_MODE, 0, -1, -1, "…", 0, 0, "08 01 5F 78 00", "H1 (GB 2312)" },
        /* 83*/ { UNICODE_MODE, 21, -1, -1, "…", 0, 21, "60 0A 58 00 42 40", "ECI-21 B1 (Win 1250)" },
        /* 84*/ { UNICODE_MODE, 0, -1, -1, "Ґ", ZINT_WARN_USES_ECI, 22, "Warning 60 0B 18 00 52 40", "ECI-22 B1 (Win 1251)" },
        /* 85*/ { UNICODE_MODE, 22, -1, -1, "Ґ", 0, 22, "60 0B 18 00 52 40", "ECI-22 B1 (Win 1251)" },
        /* 86*/ { UNICODE_MODE, 0, -1, -1, "˜", ZINT_WARN_USES_ECI, 23, "Warning 60 0B 58 00 4C 00", "ECI-23 B1 (Win 1252)" },
        /* 87*/ { UNICODE_MODE, 23, -1, -1, "˜", 0, 23, "60 0B 58 00 4C 00", "ECI-23 B1 (Win 1252)" },
        /* 88*/ { UNICODE_MODE, 24, -1, -1, "پ", 0, 24, "60 0C 18 00 40 40", "ECI-24 B1 (Win 1256)" },
        /* 89*/ { UNICODE_MODE, 0, -1, -1, "က", ZINT_WARN_USES_ECI, 26, "Warning 60 0D 18 02 70 60 10 00", "ECI-26 B3 (UTF-8)" },
        /* 90*/ { UNICODE_MODE, 25, -1, -1, "က", 0, 25, "60 0C 58 01 08 00 00", "ECI-25 B2 (UCS-2BE)" },
        /* 91*/ { UNICODE_MODE, 25, -1, -1, "ကက", 0, 25, "60 0C 58 03 08 00 02 00 00 00", "ECI-25 B4 (UCS-2BE)" },
        /* 92*/ { UNICODE_MODE, 25, -1, -1, "12", 0, 25, "60 0C 58 03 00 0C 20 03 10 00", "ECI-25 B4 (UCS-2BE ASCII)" },
        /* 93*/ { UNICODE_MODE, 27, -1, -1, "@", 0, 27, "60 0D 4F 77 2E 60", "ECI-27 L1 (ASCII)" },
        /* 94*/ { UNICODE_MODE, 0, -1, -1, "龘", ZINT_WARN_USES_ECI, 26, "Warning 60 0D 18 02 74 6F 53 00", "ECI-26 B3 (UTF-8)" },
        /* 95*/ { UNICODE_MODE, 28, -1, -1, "龘", 0, 28, "60 0E 18 01 7C 75 20", "ECI-28 B2 (Big5)" },
        /* 96*/ { UNICODE_MODE, 28, -1, -1, "龘龘", 0, 28, "60 0E 18 03 7C 75 3F 1D 28 00", "ECI-28 B4 (Big5)" },
        /* 97*/ { UNICODE_MODE, 0, -1, -1, "齄", 0, 0, "0F 4B 6F 78 00", "H1 (GB 2312)" },
        /* 98*/ { UNICODE_MODE, 29, -1, -1, "齄", 0, 29, "60 0E 47 65 77 7C 00", "ECI-29 H1 (GB 2312)" },
        /* 99*/ { UNICODE_MODE, 29, -1, -1, "齄齄", 0, 29, "60 0E 47 65 77 4B 6F 78 00", "ECI-29 H2 (GB 2312)" },
        /*100*/ { UNICODE_MODE, 0, -1, -1, "가", ZINT_WARN_USES_ECI, 26, "Warning 60 0D 18 02 75 2C 10 00", "ECI-26 B3 (UTF-8)" },
        /*101*/ { UNICODE_MODE, 30, -1, -1, "가", 0, 30, "60 0F 18 01 58 28 20", "ECI-30 B2 (EUC-KR)" },
        /*102*/ { UNICODE_MODE, 30, -1, -1, "가가", 0, 30, "60 0F 18 03 58 28 36 0A 08 00", "ECI-30 B4 (EUC-KR)" },
        /*103*/ { UNICODE_MODE, 170, -1, -1, "?", 0, 170, "60 55 0F 77 26 60", "ECI-170 L1 (ASCII invariant)" },
        /*104*/ { DATA_MODE, 899, -1, -1, "\200", 0, 899, "63 41 58 00 40 00", "ECI-899 B1 (8-bit binary)" },
        /*105*/ { UNICODE_MODE, 900, -1, -1, "é", 0, 900, "63 42 18 01 61 6A 20", "ECI-900 B2 (no conversion)" },
        /*106*/ { UNICODE_MODE, 1024, -1, -1, "é", 0, 1024, "64 08 00 30 03 43 54 40", "ECI-1024 B2 (no conversion)" },
        /*107*/ { UNICODE_MODE, 32768, -1, -1, "é", 0, 32768, "66 08 00 01 40 0E 0E 52 00", "ECI-32768 B2 (no conversion)" },
        /*108*/ { UNICODE_MODE, 811800, -1, -1, "é", ZINT_ERROR_INVALID_OPTION, 811800, "Error 533: Invalid ECI", "" },
        /*109*/ { UNICODE_MODE, 3, -1, -1, "β", ZINT_ERROR_INVALID_DATA, 3, "Error 535: Invalid character in input data for ECI 3", "" },
        /*110*/ { UNICODE_MODE, 0, READER_INIT, -1, "12", 0, 0, "51 11 71 7E 40", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, data[i].input_mode, data[i].eci, -1 /*option_1*/, -1, data[i].option_3, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %s, %s, \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilOutputOptionsName(data[i].output_options), testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), ret < ZINT_ERROR ? symbol->eci : -1, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
            }
            assert_zero(strcmp((char *) symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        char *data;
        int input_mode;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { "1234", UNICODE_MODE, -1, -1, 0, 18, 18, "",
                    "111111000000111111"
                    "101111001100101001"
                    "100101000010100001"
                    "111011011100101111"
                    "101011011000111011"
                    "111111000000111111"
                    "000000111111000000"
                    "001100100001001110"
                    "001110111111011110"
                    "001010100011000000"
                    "010100101001000000"
                    "000000111111000000"
                    "111111000000111111"
                    "101001001100101001"
                    "111001000000101111"
                    "111001000100110001"
                    "111111000000100001"
                    "111111000000111111"
               },
        /*  1*/ { "Grid Matrix", UNICODE_MODE, 5, -1, 0, 30, 30, "AIMD014 Figure 1 **NOT SAME** different encodation, uses Upper and Lower whereas figure uses Mixed and Lower",
                    "111111000000111111000000111111"
                    "110111010110110111010110110011"
                    "100011011110111111011110111111"
                    "110001000000100001000000100001"
                    "101101000000100001000000100001"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "010000101111001000101101010000"
                    "000100111111000000110011010110"
                    "001110100001011000111111001000"
                    "001010100001001010111011000010"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "110001001110100101001000110001"
                    "111111011110111111000010111111"
                    "110111000000100101000010100101"
                    "100111000000100011011100100011"
                    "111111000000111111000000111111"
                    "000000111111000000111111000000"
                    "010100101111001110101101010110"
                    "011100111111000000110001001100"
                    "001010100001000110110111011110"
                    "000010100001011100110001000000"
                    "000000111111000000111111000000"
                    "111111000000111111000000111111"
                    "110101010000110101010010110101"
                    "110111000000110011001010101111"
                    "111101010010110101010010111011"
                    "101111010010100001010010110111"
                    "111111000000111111000000111111"
               },
        /*  2*/ { "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738", UNICODE_MODE, 3, 3, 0, 42, 42, "AIMD014 Figure D.1 **NOT SAME** different encodation, see test_input dataset",
                    "111111000000111111000000111111000000111111"
                    "101101001100101111001010101011001100101101"
                    "110001011010110101010000100011000000100001"
                    "110001001110111111010000101111010010101111"
                    "101101010100100111011000110111011000111101"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001010100111000000100011000110100101001110"
                    "011010101101001100101011000110110001011110"
                    "000010111011001110100101001110110101000000"
                    "000110111111011010100011011100100001000000"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101101000110111111011100111001000010101111"
                    "110111011100111011000010110101010100111111"
                    "100011010000111101001000100001011110100001"
                    "100111010110110111011100110101000110100001"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001010100001011110110011011110100111001110"
                    "010010111001000000100011001110111111011110"
                    "010100111111011000100101010110111111000000"
                    "001010101011000100110011001110111011000000"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101001000000111001011100111011000010101101"
                    "111011001000111001001010101101011110111101"
                    "110011010100101001010010101101001110100001"
                    "100001001000100011011000100101000100100001"
                    "111111000000111111000000111111000000111111"
                    "000000111111000000111111000000111111000000"
                    "001110100011000110100101000000100001001110"
                    "010000100111001010111101000010100001010100"
                    "010100100111010000101011000000100001000100"
                    "001110100001000110111111001100101001000100"
                    "000000111111000000111111000000111111000000"
                    "111111000000111111000000111111000000111111"
                    "101011001110101001001100101011001010101111"
                    "100011011010100001011100101001010000110101"
                    "111101000110110001000100100111010110110011"
                    "100001001000110011011110110001000100100101"
                    "111111000000111111000000111111000000111111"
               },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_GRIDMATRIX, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { \"%s\", %s, %d, %d, %s, %d, %d, \"%s\",\n",
                    i, data[i].data, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("               },\n");
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

#include <time.h>

#define TEST_PERF_ITERATIONS    1000

// Not a real test, just performance indicator
static void test_perf(int index, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, -1, -1,
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738",
                    0, 66, 66, "97 chars, mixed modes" },
        /*  1*/ { BARCODE_GRIDMATRIX, UNICODE_MODE, -1, -1,
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738"
                    "AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738 AAT2556 电池充电器+降压转换器 200mA至2A tel:86 019 82512738",
                    0, 162, 162, "970 chars, mixed modes" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start, total_encode = 0, total_buffer = 0, diff_encode, diff_buffer;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) {
        int j;

        if (index != -1 && i != index) continue;

        diff_encode = diff_buffer = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

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

            ZBarcode_Delete(symbol);
        }

        printf("%s: diff_encode %gms, diff_buffer %gms\n", data[i].comment, diff_encode * 1000.0 / CLOCKS_PER_SEC, diff_buffer * 1000.0 / CLOCKS_PER_SEC);

        total_encode += diff_encode;
        total_buffer += diff_buffer;
    }
    if (index != -1) {
        printf("totals: encode %gms, buffer %gms\n", total_encode * 1000.0 / CLOCKS_PER_SEC, total_buffer * 1000.0 / CLOCKS_PER_SEC);
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_options", test_options, 1, 0, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_perf", test_perf, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
