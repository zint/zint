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

static void test_large(int index, int debug) {

    testStart("");

    int ret;
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
        /*  0*/ { BARCODE_DATAMATRIX, "1", 3116, 0, 144, 144 },
        /*  1*/ { BARCODE_DATAMATRIX, "1", 3117, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_DATAMATRIX, "A", 2335, 0, 144, 144 },
        /*  3*/ { BARCODE_DATAMATRIX, "A", 2336, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_DATAMATRIX, "\200", 1555, 0, 144, 144 },
        /*  5*/ { BARCODE_DATAMATRIX, "\200", 1556, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_HIBC_DM, "1", 110, 0, 32, 32 },
        /*  7*/ { BARCODE_HIBC_DM, "1", 111, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);

    char data_buf[3118];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Note need ZINT_SANITIZE set for these
static void test_buffer(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int eci;
        int input_mode;
        int output_options;
        char *data;
        int ret;
        char *comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 16383, UNICODE_MODE, READER_INIT, "1", 0, "" },
        /*  1*/ { 3, UNICODE_MODE, 0, "000106j 05 Galeria A NaÃ§Ã£o0000000000", 0, "From Okapi, consecutive use of upper shift; #176" },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_DATAMATRIX, data[i].input_mode, data[i].eci, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_options(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, -1, -1, "1", 0, 10, 10 },
        /*  1*/ { BARCODE_DATAMATRIX, 2, -1, -1, "1", ZINT_ERROR_INVALID_OPTION, -1, -1 },
        /*  2*/ { BARCODE_DATAMATRIX, -1, 1, -1, "1", 0, 10, 10 },
        /*  3*/ { BARCODE_DATAMATRIX, -1, 2, -1, "1", 0, 12, 12 },
        /*  4*/ { BARCODE_DATAMATRIX, -1, 48, -1, "1", 0, 26, 64 },
        /*  5*/ { BARCODE_DATAMATRIX, -1, 49, -1, "1", 0, 10, 10 }, // Ignored
        /*  6*/ { BARCODE_DATAMATRIX, -1, -1, -1, "ABCDEFGHIJK", 0, 8, 32 },
        /*  7*/ { BARCODE_DATAMATRIX, -1, -1, DM_SQUARE, "ABCDEFGHIJK", 0, 16, 16 },
        /*  8*/ { BARCODE_DATAMATRIX, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 32, 32 },
        /*  9*/ { BARCODE_DATAMATRIX, -1, -1, DM_DMRE, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 20, 44 },
        /* 10*/ { BARCODE_DATAMATRIX, -1, -1, 9999, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 32, 32 }, // Ignored
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, symbol->errtxt);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, symbol->errtxt);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        int input_mode;
        int output_options;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, UNICODE_MODE, READER_INIT, "A", 0, 10, 10, "EA 42 81 19 A4 53 21 DF", "TODO: Check this" },
        /*  1*/ { BARCODE_DATAMATRIX, GS1_MODE, READER_INIT, "[91]A", ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 521: Cannot encode in GS1 mode and Reader Initialisation at the same time", "" },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < 5) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
                assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        int eci;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_eci;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, -1, "0466010592130100000k*AGUATY80", 0, 0, 18, 18, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 93 B0 1C 3C 76 FB D4 AB 16 11", "#208" },
        /*  1*/ { UNICODE_MODE, 0, 5, -1, "0466010592130100000k*AGUATY80", 0, 0, 18, 18, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 93 B0 1C 3C 76 FB D4 AB 16 11", "" },
        /*  2*/ { UNICODE_MODE, 0, -1, -1, "0466010592130100000k*AGUATY8", 0, 0, 18, 18, "(32) 86 C4 83 87 DE 8F 83 82 82 E6 19 5C 07 B7 82 5F D4 3D 65 B5 97 30 00 FC 2C 4C 30 52", "" },
        /*  3*/ { UNICODE_MODE, 0, -1, -1, "0466010592130100000k*AGUATY80U", 0, 0, 20, 20, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 FE 56 81 76 4F AB 22 B8 6F 0A", "" },
        /*  4*/ { UNICODE_MODE, 0, 5, -1, "0466010592130100000k*AGUATY80U", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 522: Input too long for selected symbol size", "" },
        /*  5*/ { UNICODE_MODE, 0, 6, -1, "0466010592130100000k*AGUATY80U", 0, 0, 20, 20, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 FE 56 81 76 4F AB 22 B8 6F 0A", "" },
        /*  6*/ { UNICODE_MODE, 0, -1, -1, "0466010592130100000k*AGUATY80UA", 0, 0, 20, 20, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C E6 07 B7 82 5F D4 3D 1E 5F FE 81 1E 1B B0 FE E7 54", "" },
        /*  7*/ { UNICODE_MODE, 0, -1, -1, "A*0>B1*", 0, 0, 14, 14, "EE 57 AD 0E DE FE 2B 81 F8 05 75 94 1E 5F 24 0C A0 D3", "X12 symbols_left 3, process_p 1" },
        /*  8*/ { UNICODE_MODE, 0, -1, -1, "A*0>B1*2", 0, 0, 14, 14, "EE 57 AD 0E DE FE 2B 33 E7 BB FB 78 F9 F5 4B 11 BB 5A", "X12 symbols_left 3, process_p 2" },
        /*  9*/ { UNICODE_MODE, 0, -1, -1, "A*0>B1*2>", 0, 0, 14, 14, "EE 57 AD 0E DE 07 33 FE 75 99 1B 4D 76 0E 9E 49 E0 37", "X12 symbols_left 1, process_p 0" },
        /* 10*/ { UNICODE_MODE, 0, -1, -1, "ABCDEF", 0, 0, 12, 12, "E6 59 E9 6D 24 3D 15 EF AA 21 F9 59", "C40 last_shift 0, symbols_left 0, process_p 0" },
        /* 11*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFG", 0, 0, 14, 14, "E6 59 E9 6D 24 FE 48 81 8C 7E 09 5E 10 64 BC 5F 4C 91", "C40 last_shift 0, symbols_left 3, process_p 1" },
        /* 12*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGH", 0, 0, 14, 14, "E6 59 E9 6D 24 80 49 FE DD 85 9E 5B E9 8F 4D F3 02 9C", "C40 last_shift 0, symbols_left 3, process_p 2" },
        /* 13*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHI", 0, 0, 14, 14, "E6 59 E9 6D 24 80 5F FE 01 DE 20 9F AA C2 FF 8F 08 97", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 14*/ { UNICODE_MODE, 0, -1, -1, "ABCDEF\001G", 0, 0, 14, 14, "E6 59 E9 6D 24 00 3D FE 5D 5A F5 0A 8A 4E 1D 63 07 B9", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 15*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFG\001", 0, 0, 14, 14, "E6 59 E9 6D 24 7D 02 FE 14 A3 27 63 01 2F B1 94 FE FA", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 16*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFG\001H", 0, 0, 14, 14, "E6 59 E9 6D 24 7D 02 49 C2 E6 DD 06 89 51 BA 8E 9D 1F", "C40 last_shift 0, symbols_left 1, process_p 1" },
        /* 17*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGH\001", 0, 0, 14, 14, "E6 59 E9 6D 24 80 49 02 4F 4D 86 23 5F 1B F9 8C 67 7E", "C40 last_shift 1, symbols_left 1, process_p 1" },
        /* 18*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGH\001I", 0, 0, 8, 32, "E6 59 E9 6D 24 80 49 09 B1 FE 27 19 F1 CA B7 85 D0 25 0D 5E 24", "C40 last_shift 1, symbols_left 3, process_p 2" },
        /* 19*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHI\001", 0, 0, 8, 32, "E6 59 E9 6D 24 80 5F FE 02 81 47 6C 3E 49 D3 FA 46 47 53 6E E5", "Switches to ASC for last char" },
        /* 20*/ { UNICODE_MODE, 0, -1, -1, "ABCDEF+G", 0, 0, 14, 14, "E6 59 E9 6D 24 07 E5 FE 6B 35 71 7F 3D 57 59 46 F7 B9", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 21*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFG+", 0, 0, 14, 14, "E6 59 E9 6D 24 7D 33 FE 33 F5 97 60 73 48 13 2E E5 74", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 22*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFG+H", 0, 0, 14, 14, "E6 59 E9 6D 24 7D 33 49 E5 B0 6D 05 FB 36 18 34 86 91", "C40 last_shift 0, symbols_left 1, process_p 1" },
        /* 23*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGH+", 0, 0, 14, 14, "E6 59 E9 6D 24 80 49 2C 67 1F 09 CA 1A CD 0D 55 80 21", "C40 last_shift 2, symbols_left 1, process_p 1" },
        /* 24*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGH+I", 0, 0, 8, 32, "E6 59 E9 6D 24 80 4A 41 F1 FE 41 81 CF 13 E2 64 43 2F E1 D1 11", "C40 last_shift 2, symbols_left 3, process_p 2" },
        /* 25*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHI+", 0, 0, 8, 32, "E6 59 E9 6D 24 80 5F FE 2C 81 F8 BC 8D 12 17 7E 22 27 DE 7F E2", "Switches to ASC for last char" },
        /* 26*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFjG", 0, 0, 14, 14, "E6 59 E9 6D 24 0E 25 FE DA 14 D7 15 47 69 9D 4A 54 6D", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 27*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGj", 0, 0, 14, 14, "E6 59 E9 6D 24 7D 5B FE B5 F3 24 0A 99 26 D6 CC A8 40", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 28*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGjH", 0, 0, 14, 14, "E6 59 E9 6D 24 7D 5B 49 63 B6 DE 6F 11 58 DD D6 CB A5", "C40 last_shift 0, symbols_left 1, process_p 1" },
        /* 29*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHj", 0, 0, 14, 14, "E6 59 E9 6D 24 80 49 6B 12 00 5B FD B0 3A D9 DF 26 B6", "C40 last_shift 3, symbols_left 1, process_p 1" },
        /* 30*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHjI", 0, 0, 8, 32, "E6 59 E9 6D 24 80 4B 41 F1 FE FB 10 AC 51 A1 56 8F 20 98 18 1B", "C40 last_shift 3, symbols_left 3, process_p 2" },
        /* 31*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHIj", 0, 0, 8, 32, "E6 59 E9 6D 24 80 5F FE 6B 81 17 79 06 42 7E 96 B2 70 79 F8 3C", "Switches to ASC for last char" },
        /* 32*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHIJÊ", 0, 0, 16, 16, "E6 59 E9 6D 24 80 5F FE 4B EB 4B 81 DD D9 F9 C9 C5 38 F3 4B DB 80 92 A7", "Switches to ASC for last 2 chars" },
        /* 33*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHIJKÊ", 0, 0, 16, 16, "E6 59 E9 6D 24 80 5F 93 82 BF 19 FE E7 50 32 B4 0B CC 8C 07 D2 78 8D F5", "C40 last_shift 0, symbols_left 3, process_p 2" },
        /* 34*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHIJKª", 0, 0, 16, 16, "E6 59 E9 6D 24 80 5F 93 82 BB B2 FE 11 5C 60 32 A6 DE FC 7B 30 F1 03 56", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 35*/ { UNICODE_MODE, 0, -1, -1, "ABCDEFGHIJKê", 0, 0, 16, 16, "E6 59 E9 6D 24 80 5F 93 82 BB DB FE 78 43 69 3C C2 FE F5 2E 1B 4F B6 04", "C40 last_shift 0, symbols_left 1, process_p 0" },
        /* 36*/ { UNICODE_MODE, 0, -1, -1, "abcdef", 0, 0, 12, 12, "EF 59 E9 6D 24 E2 CC D9 B4 55 E2 6A", "TEX last_shift 0, symbols_left 0, process_p 0" },
        /* 37*/ { UNICODE_MODE, 0, -1, -1, "abcdefg", 0, 0, 14, 14, "EF 59 E9 6D 24 FE 68 81 A9 65 CD 3A A2 E9 E0 B7 E1 E5", "TEX last_shift 0, symbols_left 3, process_p 1" },
        /* 38*/ { UNICODE_MODE, 0, -1, -1, "abcdefgh", 0, 0, 14, 14, "EF 59 E9 6D 24 80 49 FE 06 E4 44 D2 32 58 90 31 E9 F8", "TEX last_shift 0, symbols_left 3, process_p 2" },
        /* 39*/ { UNICODE_MODE, 0, -1, -1, "abcdefghi", 0, 0, 14, 14, "EF 59 E9 6D 24 80 5F FE DA BF FA 16 71 15 22 4D E3 F3", "TEX last_shift 0, symbols_left 1, process_p 0" },
        /* 40*/ { UNICODE_MODE, 0, -1, -1, "abcdef\001g", 0, 0, 14, 14, "EF 59 E9 6D 24 00 3D FE 86 3B 2F 83 51 99 C0 A1 EC DD", "TEX last_shift 0, symbols_left 1, process_p 0" },
        /* 41*/ { UNICODE_MODE, 0, -1, -1, "abcdefg\001", 0, 0, 14, 14, "EF 59 E9 6D 24 7D 02 FE CF C2 FD EA DA F8 6C 56 15 9E", "TEX last_shift 0, symbols_left 1, process_p 0" },
        /* 42*/ { UNICODE_MODE, 0, -1, -1, "abcdefg\001h", 0, 0, 14, 14, "EF 59 E9 6D 24 7D 02 69 7A 9B EB A4 5E DE 99 25 01 8C", "TEX last_shift 0, symbols_left 1, process_p 1" },
        /* 43*/ { UNICODE_MODE, 0, -1, -1, "abcdefgh\001", 0, 0, 14, 14, "EF 59 E9 6D 24 80 49 02 94 2C 5C AA 84 CC 24 4E 8C 1A", "TEX last_shift 1, symbols_left 1, process_p 1" },
        /* 44*/ { UNICODE_MODE, 0, -1, -1, "abcdefgh\001i", 0, 0, 8, 32, "EF 59 E9 6D 24 80 49 09 B1 FE 2D DE FF 05 A9 AE 0B 91 4B C5 70", "TEX last_shift 1, symbols_left 3, process_p 2" },
        /* 45*/ { UNICODE_MODE, 0, -1, -1, "abcdefghi\001", 0, 0, 8, 32, "EF 59 E9 6D 24 80 5F FE 02 81 4D AB 30 86 CD D1 9D F3 15 F5 B1", "Switches to ASC for last char" },
        /* 46*/ { UNICODE_MODE, 0, -1, -1, "abcdefJg", 0, 0, 14, 14, "EF 59 E9 6D 24 0E 25 FE 01 75 0D 9C 9C BE 40 88 BF 09", "TEX last_shift 0, symbols_left 1, process_p 0" },
        /* 47*/ { UNICODE_MODE, 0, -1, -1, "abcdefgJ", 0, 0, 14, 14, "EF 59 E9 6D 24 7D 5B FE 6E 92 FE 83 42 F1 0B 0E 43 24", "TEX last_shift 0, symbols_left 1, process_p 0" },
        /* 48*/ { UNICODE_MODE, 0, -1, -1, "abcdefgJh", 0, 0, 14, 14, "EF 59 E9 6D 24 7D 5B 69 DB CB E8 CD C6 D7 FE 7D 57 36", "TEX last_shift 0, symbols_left 1, process_p 1" },
        /* 49*/ { UNICODE_MODE, 0, -1, -1, "abcdefghJ", 0, 0, 14, 14, "EF 59 E9 6D 24 80 49 4B AA 7D 6D 5F 67 B5 FA 74 BA 25", "TEX last_shift 3, symbols_left 1, process_p 1" },
        /* 50*/ { UNICODE_MODE, 0, -1, -1, "abcdefghJi", 0, 0, 8, 32, "EF 59 E9 6D 24 80 4B 41 F1 FE F1 D7 A2 9E BF 7D 54 94 DE 83 4F", "TEX last_shift 3, symbols_left 3, process_p 2" },
        /* 51*/ { UNICODE_MODE, 0, -1, -1, "abcdefghiJ", 0, 0, 8, 32, "EF 59 E9 6D 24 80 5F FE 4B 81 B3 A5 20 E3 DC F9 74 40 09 30 46", "Switches to ASC for last char" },
        /* 52*/ { UNICODE_MODE, 0, -1, -1, "abcdefghijkÊ", 0, 0, 16, 16, "EF 59 E9 6D 24 80 5F 93 82 BB DB FE 3E C8 EC 73 58 A7 42 46 10 49 25 99", "TEX last_shift 0, symbols_left 1, process_p 0" },
        /* 53*/ { UNICODE_MODE, 0, -1, -1, "abcdefghijkª", 0, 0, 16, 16, "EF 59 E9 6D 24 80 5F 93 82 BB B2 FE 57 D7 E5 7D 3C 87 4B 13 3B F7 90 CB", "TEX last_shift 0, symbols_left 1, process_p 0" },
        /* 54*/ { UNICODE_MODE, 0, -1, -1, "abcdefghijkê", 0, 0, 16, 16, "EF 59 E9 6D 24 80 5F 93 82 BF 19 FE A1 DB B7 FB 91 95 3B 6F D9 7E 1E 68", "TEX last_shift 2, symbols_left 3, process_p 2" },
        /* 55*/ { UNICODE_MODE, 0, -1, -1, "@AB@CD@E", 0, 0, 14, 14, "F0 00 10 80 0C 40 05 81 45 D9 9B 1F BC 09 CD E4 7F F4", "EDIFACT symbols_left 1, process_p 0" },
        /* 56*/ { UNICODE_MODE, 0, -1, -1, "@AB@CD@EF", 0, 0, 14, 14, "F0 00 10 80 0C 40 05 47 AC D8 F1 F0 DE 6C 30 5E 30 D4", "EDIFACT symbols_left 1, process_p 1" },
        /* 57*/ { UNICODE_MODE, 0, -1, -1, "@AB@CD@EF@", 0, 0, 8, 32, "F0 00 10 80 0C 40 05 18 07 C0 6C 60 CA 7E 7B F3 38 A1 9D D0 CC", "EDIFACT symbols_left 3, process_p 2" },
        /* 58*/ { UNICODE_MODE, 0, -1, -1, "@AB@CD@EF@G", 0, 0, 8, 32, "F0 00 10 80 0C 40 05 18 01 DF 71 FB 95 EA E6 4B 36 E0 23 9B 4C", "EDIFACT symbols_left 3, process_p 3" },
        /* 59*/ { UNICODE_MODE, 0, -1, -1, "@AB@CD@EF@GH", 0, 0, 8, 32, "F0 00 10 80 0C 40 05 18 01 C8 77 0F 96 AD 39 FB F3 04 3B BF 99", "EDIFACT symbols_left 0, process_p 0" },
        /* 60*/ { UNICODE_MODE, 0, -1, -1, "@AB@CD@EF@GH@", 0, 0, 16, 16, "F0 00 10 80 0C 40 05 18 01 C8 41 81 4A 43 1E F1 26 2E 4B EB B8 6A 2B 64", "EDIFACT symbols_left 2, process_p 1" },
        /* 61*/ { UNICODE_MODE, 0, -1, -1, "@AB@CD@EF@GH@I", 0, 0, 16, 16, "F0 00 10 80 0C 40 05 18 01 C8 41 4A 49 B3 34 91 8C 2A C4 0E 16 2F 45 9B", "EDIFACT symbols_left 2, process_p 2" },
        /* 62*/ { DATA_MODE, 0, -1, -1, "\377\376", 0, 0, 12, 12, "EB 80 EB 7F 81 6F A8 0F 21 6F 5F 88", "FN4 A7F FN4 A7E" },
        /* 63*/ { DATA_MODE, 0, -1, -1, "\377\376\375", 0, 0, 12, 12, "E7 2F C0 55 E9 52 B7 8D 38 76 E8 6E", "BAS BFF BFE BFD" },
        /* 64*/ { DATA_MODE, 3, -1, -1, "\101\102\103\104\300\105\310", 0, 3, 16, 16, "F1 04 E7 5E 2D C4 5B F1 03 1D 36 81 64 0E C0 77 9A 18 52 B2 F9 F0 04 39", "ECI 4 BAS B41 B42 B43 B44 BC0 B45 BC8" },
        /* 65*/ { UNICODE_MODE, 26, -1, -1, "ABCDÀEÈ", 0, 26, 12, 26, "F1 1B E7 60 2D C4 5B F1 06 58 B3 C7 21 81 57 ED 3D C0 12 2E 6C 80 58 CC 2C 05 0D 31 FC 2D", "ECI 27 BAS B41 B42 B43 B44 BC3 B80 B45 BC3 B88" },
        /* 66*/ { UNICODE_MODE, 0, -1, -1, "β", ZINT_WARN_USES_ECI, 9, 12, 12, "Warning F1 0A EB 63 81 41 56 DA C0 3D 2D CC", "ECI 10 FN4 A62" },
        /* 67*/ { UNICODE_MODE, 127, -1, -1, "A", 0, 127, 12, 12, "F1 80 01 42 81 14 A2 86 07 F5 27 30", "ECI 128 A41" },
        /* 68*/ { UNICODE_MODE, 16382, -1, -1, "A", 0, 16382, 12, 12, "F1 BF FE 42 81 29 57 AA A0 92 B2 45", "ECI 16383 A41" },
        /* 69*/ { UNICODE_MODE, 810899, -1, -1, "A", 0, 810899, 12, 12, "F1 CC 51 05 42 BB A5 A7 8A C6 6E 0F", "ECI 810900 A41" },
        /* 70*/ { UNICODE_MODE, 26, -1, -1, "abcdefghi1234FGHIJKLMNabc@@@@@@@@@é", 0, 26, 24, 24, "(60) F1 1B EF 59 E9 6D 24 80 5F FE 8E A4 E6 79 F6 8D 31 A0 6C FE 62 63 64 F0 00 00 00 00", "Mix of modes TEX ASC C40 ASC EDI BAS" },
        /* 71*/ { UNICODE_MODE | ESCAPE_MODE, -1, -1, -1, "[)>\\R05\\GA\\R\\E", 0, 0, 10, 10, "EC 42 81 5D 17 49 F6 B6", "Macro05 A41" },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = testUtilSetSymbol(symbol, BARCODE_DATAMATRIX, data[i].input_mode, data[i].eci, -1 /*option_1*/, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, \"%s\", %s, %d, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_2, data[i].option_3, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), ret < 5 ? symbol->eci : -1, symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < 5) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d width %d != %d\n", i, symbol->width, data[i].expected_width);
                assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    testStart("");

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    int ret;
    struct item {
        int symbology;
        int input_mode;
        int eci;
        int output_options;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    // Verified manually against ISO/IEC 16022:2006, GS1 General Specifications 20.0 (GGS), ANSI/HIBC LIC 2.6-2016 (HIBC/LIC) and ANSI/HIBC PAS 1.3-2010 (HIBC/PAS), with noted exceptions
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "1234abcd", 0, 14, 14, 1, "",
                    "10101010101010"
                    "11001010001111"
                    "11000101100100"
                    "11001001100001"
                    "11011001110000"
                    "10100101011001"
                    "10101110011000"
                    "10011101100101"
                    "10100001001000"
                    "10101000001111"
                    "11101100000010"
                    "11010010100101"
                    "10011111000100"
                    "11111111111111"
                },
        /*  1*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "A1B2C3D4E5F6G7H8I9J0K1L2", 0, 18, 18, 1, "ISO 16022:2006 Figure 1",
                    "101010101010101010"
                    "101000101010001111"
                    "101100000111000010"
                    "100000101110100111"
                    "100100000100011100"
                    "101011111010101111"
                    "100110011111110110"
                    "110011101111111101"
                    "111111111101111100"
                    "101110110100101101"
                    "100010110101001110"
                    "101101111001100001"
                    "110001101010011110"
                    "110110100000100011"
                    "101101001101011010"
                    "100010011001011011"
                    "100011000000100100"
                    "111111111111111111"
                },
        /*  2*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "123456", 0, 10, 10, 1, "ISO 16022:2006 Figure O.2",
                    "1010101010"
                    "1100101101"
                    "1100000100"
                    "1100011101"
                    "1100001000"
                    "1000001111"
                    "1110110000"
                    "1111011001"
                    "1001110100"
                    "1111111111"
                },
        /*  3*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "30Q324343430794<OQQ", 0, 16, 16, 1, "ISO 16022:2006 Figure R.1",
                    "1010101010101010"
                    "1010101010000001"
                    "1010101011101100"
                    "1010101010110011"
                    "1010101010001100"
                    "1010101010001101"
                    "1010101010000100"
                    "1010101001101001"
                    "1010101010000110"
                    "1000001001011001"
                    "1111111110000100"
                    "1101100110010101"
                    "1111111001100100"
                    "1110010111100101"
                    "1110010010100010"
                    "1111111111111111"
                },
        /*  4*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09501101530003[17]150119[10]AB-123", 0, 20, 20, 1, "GGS Figure 2.6.14-3",
                    "10101010101010101010"
                    "11001111010100000111"
                    "10001010001001010100"
                    "10110011010100010001"
                    "11101010000001101010"
                    "10000100111011010111"
                    "10011010101101010110"
                    "11010001001110101001"
                    "11101000110100101100"
                    "11001111010111001101"
                    "10001010000001100000"
                    "11010000100010111011"
                    "10110010011000001000"
                    "10011010000011010011"
                    "11111010101110100110"
                    "11010010111011100001"
                    "11010100101100111110"
                    "11000001110010010101"
                    "10011011100101011010"
                    "11111111111111111111"
                },
        /*  5*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]04012345678901[21]ABCDEFG123456789", 0, 20, 20, 1, "GGS Figure 2.6.14-4",
                    "10101010101010101010"
                    "11011000001101000111"
                    "10001001100001110100"
                    "10110110110000010001"
                    "11100010000111110110"
                    "10101010110011101101"
                    "11111000100000100010"
                    "10010010001011110001"
                    "10101010110011010110"
                    "11011110011010001111"
                    "10001010011101010010"
                    "10111111011110110011"
                    "11110100101101011000"
                    "11010111011100100111"
                    "10000000011001100000"
                    "11101111110100001011"
                    "11010001001000101010"
                    "11010011101000100101"
                    "10001000100001111010"
                    "11111111111111111111"
                },
        /*  6*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]04012345678901[17]170101[10]ABC123", 0, 20, 20, 1, "GGS Figure 4.15-1 (and 5.1-6)",
                    "10101010101010101010"
                    "11011000010100000111"
                    "10001001100001010100"
                    "10110111001100000001"
                    "11100010000101101110"
                    "10101100110001010101"
                    "11111010101000100110"
                    "10010011001000100011"
                    "10101000110010111010"
                    "11001111001010101111"
                    "10001010000111000010"
                    "10110000010101000011"
                    "11110010000001011000"
                    "11011000000110101111"
                    "11111010111011110110"
                    "11001001001110101111"
                    "11011010000001110010"
                    "11010111010111101101"
                    "10001000000101111010"
                    "11111111111111111111"
                },
        /*  7*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, GS1_GS_SEPARATOR, -1, -1, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", 0, 32, 32, 1, "GGS Figure 4.15.1-1; BWIPP does not support GS1_GS_SEPARATOR",
                    "10101010101010101010101010101010"
                    "11001111010000111101100000101001"
                    "10001010011111001011011001000010"
                    "10111011001001111101111101000101"
                    "11100101000010001000011011011110"
                    "10000101001101111010111000100101"
                    "10010001000100101000000010011110"
                    "10010110011101011000101100101111"
                    "11101010110010001100011100110100"
                    "11011100110110111101010000001001"
                    "10001010000101001100001111001110"
                    "10110011010101111000011101111101"
                    "11101100100110101110111010101110"
                    "11100100000001111000001111101001"
                    "10100010111011101110111100110100"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11100011100111111000011111010111"
                    "11001100001100001001110100001010"
                    "10111011010001111010100110101101"
                    "11101011010011001101110011011010"
                    "10010000011010011110111111111011"
                    "11000101110110101110001110011000"
                    "10000010111010011001011100011111"
                    "10110011101000001100011001110010"
                    "10111010000110011100110100001001"
                    "10010001100011101101100010101110"
                    "11101001011010111000000100111001"
                    "10000010111010001001110111010000"
                    "11001101000110011110000111010101"
                    "10111100011001001010011100011000"
                    "11111111111111111111111111111111"
                },
        /*  8*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", 0, 32, 32, 0, "GGS Figure 4.15.1-2; BWIPP different encodation (does not use 0 padded Text)",
                    "10101010101010101010101010101010"
                    "11001111010000111101100000101001"
                    "10001010011111001011011001000010"
                    "10111011001001111101111101000101"
                    "11100101000010001000011011011110"
                    "10000101001101111010111000100101"
                    "10010001110100101000000010011110"
                    "10010110101101011000101100101111"
                    "11101010000010001100011100110100"
                    "11011100110110111101010000000001"
                    "10001010000101001100000111001010"
                    "10110011010101111000011101001001"
                    "11101100100110101110111101100110"
                    "11100100000001111000010101110111"
                    "10100010111011101101011111011100"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11111011100111111011010111011111"
                    "11010100001100101101010101100010"
                    "10100011010001111101001011000101"
                    "11101011010011101001011111100010"
                    "10010000011001011100110010100111"
                    "11000101110000001000000000001100"
                    "10000010110010111010010010011001"
                    "10110011010000001010100111110000"
                    "10111010001101111111010011010001"
                    "10010000001110101110101110001110"
                    "11101010000000111000010000000001"
                    "10000101011111101011001111101100"
                    "11000111011001111110111101000101"
                    "10111010011011101010011100011000"
                    "11111111111111111111111111111111"
                },
        /*  9*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09512345678901[15]170810[21]abcde", 0, 20, 20, 1, "GGS Figure 5.6.2-1",
                    "10101010101010101010"
                    "11001111010111100111"
                    "10001010100101010100"
                    "10110111010010011001"
                    "11100010010101101110"
                    "10101100101000111101"
                    "11111010011000000110"
                    "10010011100000101101"
                    "10101001101011000010"
                    "11000110000100011111"
                    "10001011010001100010"
                    "10110000101001000011"
                    "11110011011001011100"
                    "11011101001000101011"
                    "11111101001000110110"
                    "11011100110000101111"
                    "11000100001111110000"
                    "11011111110010101101"
                    "10011010101001110010"
                    "11111111111111111111"
                },
        /* 10*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]00012345678905[17]040115", 0, 12, 26, 1, "GGS Figure 5.6.2-1 (left)",
                    "10101010101010101010101010"
                    "11001000010011010100111111"
                    "10001001100010001111001010"
                    "10110111011000001001001111"
                    "11100010100100100010001100"
                    "10101100110101011101101001"
                    "11111000111110101001010010"
                    "10011111101010111001100001"
                    "10101110101000110000010010"
                    "11001101010110000111100111"
                    "10001001100010100010100000"
                    "11111111111111111111111111"
                },
        /* 11*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, DM_SQUARE, "[01]00012345678905[17]040115", 0, 18, 18, 1, "GGS Figure 5.6.2-1 (right)",
                    "101010101010101010"
                    "110010000100010101"
                    "100010011010111110"
                    "101101110001101111"
                    "111000100010101100"
                    "101011001100010001"
                    "111110000010101010"
                    "100101100110101101"
                    "101010101001110110"
                    "110011110001110011"
                    "100000111010111100"
                    "111100000000010111"
                    "111011010101010010"
                    "111111111001001011"
                    "100011100101010000"
                    "111100011101000011"
                    "100000101110000100"
                    "111111111111111111"
                },
        /* 12*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]00012345678905[17]180401[21]ABCDEFGHIJKL12345678[91]ABCDEFGHI123456789[92]abcdefghi", 0, 32, 32, 0, "GGS Figure 5.6.3.2-3 (left) **NOT SAME** different encodation; BWIP different encodation, same no. of codewords",
                    "10101010101010101010101010101010"
                    "11001000010111111000100110101011"
                    "10001001100001101100110010100010"
                    "10110111001101111110011001000111"
                    "11100010001100101100101001011110"
                    "10101101101011111110000000100101"
                    "11111010010010101101000010011110"
                    "10010100101111011101101100101111"
                    "10101000101101101111111100110100"
                    "11001110011000111111110000001001"
                    "10001010001010101101001111001110"
                    "11110001000000111101011100101101"
                    "11010001111011001000011010000010"
                    "11011100101001111001000111111111"
                    "10111100101001101111011101000010"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11010100111011111001101111100111"
                    "11100111010011001011100001001010"
                    "11111001010111011101111000110011"
                    "11110000000010101101001110000110"
                    "11101110000001111011101000010101"
                    "11110101001101101101110000001000"
                    "10101011000111111010111001100111"
                    "11000101010010001100000011101010"
                    "11111101110111011001111011001101"
                    "11010111011010001000011101001010"
                    "10100111111110111101010111100011"
                    "10111011111010001001001100101110"
                    "10010101001110111101000101111101"
                    "11110110001001001010110111010110"
                    "11111111111111111111111111111111"
                },
        /* 13*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, 30, -1, "[01]00012345678905[17]180401[21]ABCDEFGHIJKL12345678[91]abcdefghi", 0, 16, 48, 1, "GGS Figure 5.6.3.2-3 (right) **NOT SAME** different encodation",
                    "101010101010101010101010101010101010101010101010"
                    "110010000101111001000011101101100100111011001111"
                    "100010011000011101111100100100011000110010111100"
                    "101101110011011100100111100111101110111110100011"
                    "111000100011001100111010101000011110001110111110"
                    "101011011010110100101101100100010101101110101111"
                    "111110100100100101111000101001100001101001001010"
                    "100101001011111001111001110110100101101100010111"
                    "101010001011111100111100100010000000011110001100"
                    "110011100101101010111011110111110111100111011001"
                    "100010100011010010111100100100101000001000011110"
                    "111100010010101101110001100001000001010110001001"
                    "110100001000101000011100101010101100011001001010"
                    "110111001110010000011111101111000110100011011011"
                    "101110001010001011101010101101111111111000000100"
                    "111111111111111111111111111111111111111111111111"
                },
        /* 14*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, DM_SQUARE, "[00]395011010013000129[403]123+1021JK+0320+12[421]5281500KM", 0, 24, 24, 1, "GGS Figure 6.6.5-6 **NOT SAME** figure has unnecessary FNC1 at end of data",
                    "101010101010101010101010"
                    "110001110100011010101101"
                    "100010100100101000011000"
                    "101000110001001011100001"
                    "111010110110100001100010"
                    "100001001001010100001111"
                    "100110110111100000000100"
                    "100101110011001001100001"
                    "110000010110101011100010"
                    "110011001100011101110101"
                    "100000100011111010000000"
                    "101010110110011011000001"
                    "111010001001101000110000"
                    "110001000100011110101101"
                    "101011011101110000101000"
                    "110111000100011101011111"
                    "110101001010111101000010"
                    "110000111110111111111001"
                    "100101110010001010110110"
                    "111011010010000000100111"
                    "100110111101001000000100"
                    "101111000010010011111101"
                    "111001011011101100011010"
                    "111111111111111111111111"
                },
        /* 15*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[00]093123450000000012[421]0362770[401]931234518430GR[403]MEL", 0, 24, 24, 1, "GGS Figure 6.6.5-7 **NOT SAME** different encodation",
                    "101010101010101010101010"
                    "110011100101100110110101"
                    "100010001001111010000100"
                    "101001110100001011100011"
                    "111000110111000001101010"
                    "101011010010100110000111"
                    "101110011000011000001100"
                    "100101101011010001101001"
                    "110000100101011011101110"
                    "110010010110101101110001"
                    "100010101101001011010000"
                    "101010001000100000011001"
                    "101000101010000010000100"
                    "100110101110101100010101"
                    "100010010000111001000000"
                    "101000001001111000011011"
                    "111110000111001110000010"
                    "110001110101001101101111"
                    "110111110101100111101110"
                    "110100010010101011110101"
                    "110011000001011110100010"
                    "100111010001010011000101"
                    "101110011001110010101010"
                    "111111111111111111111111"
                },
        /* 16*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "A123BJC5D6E71", 0, 16, 16, 1, "HIBC/LIC Figure 3 **NOT SAME** different encodation, same no. of codewords",
                    "1010101010101010"
                    "1110000011011011"
                    "1100001110001000"
                    "1110101011011111"
                    "1100110100001000"
                    "1011000001001001"
                    "1100010011110100"
                    "1000101001010101"
                    "1010110011110000"
                    "1011000001001111"
                    "1000010001001110"
                    "1001111110001111"
                    "1000110101010010"
                    "1101101110100101"
                    "1100101101000010"
                    "1111111111111111"
                },
        /* 17*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "A123BJC5D6E71/$$52001510X3", 0, 20, 20, 1, "HIBC/LIC Section 4.3.3 **NOT SAME** different encodation; also figure has weird CRLF after check digit",
                    "10101010101010101010"
                    "11100000100101100001"
                    "11000011111010101100"
                    "11101011100011000101"
                    "11001100011011000100"
                    "10110010010000101011"
                    "11000000100101100010"
                    "10000110010100000101"
                    "10111010001100001110"
                    "11111100101000000011"
                    "11110110001001111110"
                    "11100111110010000011"
                    "11000010001110101000"
                    "10110110001001010001"
                    "11100011101111010110"
                    "10000010110000110001"
                    "10000100001100100110"
                    "10111011000001111101"
                    "10110110110000011010"
                    "11111111111111111111"
                },
        /* 18*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "H123ABC01234567890", 0, 12, 26, 1, "HIBC/LIC Figure C2, same",
                    "10101010101010101010101010"
                    "10111011011011110101001101"
                    "10010110000001001100110100"
                    "10010001010100001011110001"
                    "11010101011010110100111100"
                    "10000101110000001110001101"
                    "11011011110011001011100000"
                    "10010001101011100010001001"
                    "10000001101101100110101010"
                    "11001111011110011111010001"
                    "10010010001100110000011010"
                    "11111111111111111111111111"
                },
        /* 19*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, DM_SQUARE, "/ACMRN123456/V200912190833", 0, 20, 20, 1, "HIBC/PAS Section 2.2 Patient Id, same",
                    "10101010101010101010"
                    "10001000010011001001"
                    "11100110001010110100"
                    "10000010111001010101"
                    "11011100101010111100"
                    "10010001110010100001"
                    "11011110100100100110"
                    "10110000100000101111"
                    "11111100011100001000"
                    "11001011011010001101"
                    "11001010111110110000"
                    "11000001111100001111"
                    "11110010001100000100"
                    "10011011100010110011"
                    "11111100110000111110"
                    "11000110111111110001"
                    "11000001011001100110"
                    "10101010010101100101"
                    "10000100100110010010"
                    "11111111111111111111"
                },
        /* 20*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R06\\G+/ACMRN123456/V2009121908334\\R\\E", 0, 20, 20, 1, "HIBC/PAS Section 2.2 Patient Id Macro, same",
                    "10101010101010101010"
                    "10000000001110001111"
                    "11010101001010011100"
                    "11000000011100110101"
                    "11011001101011001100"
                    "11001100000100010001"
                    "11110111101011000100"
                    "11010010001101100001"
                    "11110010010110011110"
                    "11010010010000010011"
                    "10010001100010110000"
                    "11101100100001000111"
                    "11101010000011111100"
                    "11000010000101001011"
                    "11001110111110010010"
                    "11000010110100011101"
                    "11001011001001011100"
                    "10010110010000010101"
                    "11100110001010111010"
                    "11111111111111111111"
                },
        /* 21*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/EO523201", 0, 14, 14, 1, "HIBC/PAS Section 2.2 Purchase Order, same",
                    "10101010101010"
                    "10011001010101"
                    "11101000011010"
                    "10001100011101"
                    "11101100101100"
                    "10100001101111"
                    "10010001010110"
                    "10000001011001"
                    "11100000010100"
                    "11011010100101"
                    "10111110101110"
                    "11110000101101"
                    "10010010000100"
                    "11111111111111"
                },
        /* 22*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, DM_SQUARE, "/EU720060FF0/O523201", 0, 18, 18, 1, "HIBC/PAS Section 2.2 2nd Purchase Order, same",
                    "101010101010101010"
                    "100110010100100001"
                    "111011110110010110"
                    "100000101110011001"
                    "111001001010000100"
                    "100000000000011101"
                    "100101100000101110"
                    "111000000111111011"
                    "110110111000101010"
                    "101001000111000111"
                    "100011110101010110"
                    "111111001101010011"
                    "100000000001101000"
                    "110100100011011111"
                    "111000100110101110"
                    "111010100101000011"
                    "111000010011001010"
                    "111111111111111111"
                },
        /* 23*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/EU720060FF0/O523201/Z34H159/M9842431340", 0, 22, 22, 1, "HIBC/PAS Section 2.2 3rd Purchase Order (left), same",
                    "1010101010101010101010"
                    "1001100101001000000011"
                    "1110111101100001111010"
                    "1000001011101100111111"
                    "1110010010010000111100"
                    "1000000000011100000111"
                    "1001011010011000001110"
                    "1110000010001001101001"
                    "1101100110001010100100"
                    "1010010011011101000101"
                    "1000100011010000001110"
                    "1111010100101000010111"
                    "1000001001011011101110"
                    "1111110111111101100011"
                    "1001010110011010000000"
                    "1101010100110100010011"
                    "1001010011000110000000"
                    "1111001010100101110111"
                    "1100110010110011010000"
                    "1100001011100001000111"
                    "1010110000010001001000"
                    "1111111111111111111111"
                },
        /* 24*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R06\\G+/EU720060FF0/O523201/Z34H159/M9842431340V\\R\\E", 0, 22, 22, 1, "HIBC/PAS Section 2.2 3rd Purchase Order (right), same",
                    "1010101010101010101010"
                    "1000000000111010011101"
                    "1101011100101001011100"
                    "1100010000000001101001"
                    "1111110110000111100000"
                    "1100100000110011001101"
                    "1001011001000010000110"
                    "1000100101110111110111"
                    "1100001001110111111100"
                    "1011111001001010001101"
                    "1000011000010100101010"
                    "1111001101110100101101"
                    "1110001101101100001100"
                    "1001010101111010110011"
                    "1000110111011100101010"
                    "1111110011011111010101"
                    "1101000011100111101110"
                    "1011000010010100110111"
                    "1001110101111101000000"
                    "1110101001011011000111"
                    "1001110110011101101000"
                    "1111111111111111111111"
                },
        /* 25*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/E+/KN12345", 0, 16, 16, 1, "HIBC/PAS Section 2.2 Asset Tag **NOT SAME** check digit 'A' in figure is for '/KN12345', but actual data is as given here, when check digit is 'J'",
                    "1010101010101010"
                    "1001101010001111"
                    "1110001000101100"
                    "1000110100101101"
                    "1101000000110010"
                    "1000101001000001"
                    "1110000111001100"
                    "1010001101111101"
                    "1111101010101000"
                    "1101100101010001"
                    "1100001011010010"
                    "1100001111001001"
                    "1100010100000110"
                    "1010001101001101"
                    "1001000000000010"
                    "1111111111111111"
                },
        /* 26*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/LAH123/NC903", 0, 16, 16, 1, "HIBC/PAS Section 2.2 Surgical Instrument, same",
                    "1010101010101010"
                    "1001010001010001"
                    "1110010100000100"
                    "1000001100000011"
                    "1110001100101000"
                    "1000111111100001"
                    "1011001110000100"
                    "1100110000001101"
                    "1000001110010000"
                    "1011001110111111"
                    "1001011010011010"
                    "1111000110111011"
                    "1010010101000100"
                    "1011001110110101"
                    "1100000101010010"
                    "1111111111111111"
                },
        /* 27*/ { BARCODE_DATAMATRIX, DATA_MODE, 3, -1, -1, -1, "\101\300", 0, 12, 12, 1, "AÀ",
                    "101010101010"
                    "100010101111"
                    "100001011110"
                    "110000010001"
                    "101100110000"
                    "110010100111"
                    "101011011100"
                    "110100111101"
                    "101100110100"
                    "101011100101"
                    "100011011010"
                    "111111111111"
                },
        /* 28*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, -1, -1, -1, "AÀ", 0, 14, 14, 1, "AÀ",
                    "10101010101010"
                    "10001010100001"
                    "10110101100100"
                    "10110001000101"
                    "10111000100010"
                    "11101011110011"
                    "10011100001100"
                    "10001100101111"
                    "10110110111110"
                    "10000111010001"
                    "10000001111000"
                    "11110100110001"
                    "11000110001100"
                    "11111111111111"
                },
        /* 29*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, -1, "abcdefgh+", 0, 14, 14, 0, "TEX last_shift 2, symbols_left 1, process_p 1; BWIPP different encodation (does not use 0 padded Text)",
                    "10101010101010"
                    "10100110111011"
                    "10110010100010"
                    "10011000100101"
                    "11101000000000"
                    "11000110110111"
                    "11000010110100"
                    "10101011010111"
                    "10111110000100"
                    "11011001001111"
                    "11110111100000"
                    "11001011110101"
                    "10010111010100"
                    "11111111111111"
                },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];
    char bwipp_buf[8192];
    char bwipp_msg[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, -1 /*option_1*/, data[i].option_2, data[i].option_3, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %s, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilOutputOptionsName(data[i].output_options),
                    data[i].option_2, testUtilOption3Name(data[i].option_3), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesDump(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < 5) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                    }
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
        { "test_buffer", test_buffer, 1, 0, 1 },
        { "test_options", test_options, 1, 0, 1 },
        { "test_reader_init", test_reader_init, 1, 1, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
