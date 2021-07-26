/*
    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

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

    struct item {
        int symbology;
        int option_2;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "1", 121, 0, 1, 999 }, // 8 (Start) + 121*8 + 2*8 (Checks) + 7 (Stop) == 999
        /*  1*/ { BARCODE_CODE11, -1, "1", 122, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_CODE39, -1, "1", 85, 0, 1, 1130 }, // 13 (Start) + 85*13 + 12 (Stop) == 1130
        /*  3*/ { BARCODE_CODE39, -1, "1", 86, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_EXCODE39, -1, "1", 85, 0, 1, 1130 },
        /*  5*/ { BARCODE_EXCODE39, -1, "1", 86, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_EXCODE39, -1, "a", 42, 0, 1, 1117 }, // Takes 2 encoding chars per char
        /*  7*/ { BARCODE_EXCODE39, -1, "a", 43, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_EXCODE39, -1, "a", 85, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  9*/ { BARCODE_LOGMARS, -1, "1", 30, 0, 1, 511 }, // 16 (Start) + 30*16 + 15 (Stop) == 511
        /* 10*/ { BARCODE_LOGMARS, -1, "1", 31, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 11*/ { BARCODE_CODE93, -1, "1", 107, 0, 1, 1000 }, // 9 (Start) + 107*9 + 2*9 (Checks) + 10 (Stop) == 1000
        /* 12*/ { BARCODE_CODE93, -1, "1", 108, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 13*/ { BARCODE_CODE93, -1, "a", 53, 0, 1, 991 }, // Takes 2 encoding chars per char
        /* 14*/ { BARCODE_CODE93, -1, "a", 54, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 15*/ { BARCODE_CODE93, -1, "a", 107, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 16*/ { BARCODE_PZN, -1, "1", 7, 0, 1, 142 },
        /* 17*/ { BARCODE_PZN, -1, "1", 8, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 18*/ { BARCODE_VIN, -1, "1", 17, 0, 1, 246 },
        /* 19*/ { BARCODE_VIN, -1, "1", 18, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 20*/ { BARCODE_VIN, -1, "1", 16, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 21*/ { BARCODE_VIN, 1, "1", 17, 0, 1, 259 },
        /* 22*/ { BARCODE_HIBC_39, -1, "1", 67, 0, 1, 1135 }, // 69 - 2 ('+' and check digit)
        /* 23*/ { BARCODE_HIBC_39, -1, "1", 68, ZINT_ERROR_TOO_LONG, -1, -1 },
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

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);

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

static void test_hrt(int index, int debug) {

    struct item {
        int symbology;
        int option_2;
        char *data;
        int length;

        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "123-45", -1, "123-4552" }, // 2 checksums
        /*  1*/ { BARCODE_CODE11, 1, "123-45", -1, "123-455" }, // 1 check digit
        /*  2*/ { BARCODE_CODE11, 2, "123-45", -1, "123-45" }, // No checksums
        /*  3*/ { BARCODE_CODE11, -1, "123456789012", -1, "123456789012-8" }, // First check digit 10 (A) goes to hyphen
        /*  4*/ { BARCODE_CODE39, -1, "ABC1234", -1, "*ABC1234*" },
        /*  5*/ { BARCODE_CODE39, -1, "abc1234", -1, "*ABC1234*" }, // Converts to upper
        /*  6*/ { BARCODE_CODE39, -1, "123456789", -1, "*123456789*" },
        /*  7*/ { BARCODE_CODE39, 1, "123456789", -1, "*1234567892*" }, // With check digit
        /*  8*/ { BARCODE_EXCODE39, -1, "ABC1234", -1, "ABC1234" },
        /*  9*/ { BARCODE_EXCODE39, -1, "abc1234", -1, "abc1234" },
        /* 10*/ { BARCODE_EXCODE39, 1, "abc1234", -1, "abc1234" }, // With check digit (not displayed)
        /* 11*/ { BARCODE_EXCODE39, -1, "a%\000\001$\177z\033\037!+/\\@A~", 16, "a%  $ z  !+/\\@A~" }, // NUL, ctrls and DEL replaced with spaces
        /* 12*/ { BARCODE_LOGMARS, -1, "ABC1234", -1, "ABC1234" },
        /* 13*/ { BARCODE_LOGMARS, -1, "abc1234", -1, "ABC1234" }, // Converts to upper
        /* 14*/ { BARCODE_LOGMARS, 1, "abc1234", -1, "ABC12340" }, // With check digit
        /* 15*/ { BARCODE_LOGMARS, 1, "12345/ABCDE", -1, "12345/ABCDET" }, // With check digit
        /* 16*/ { BARCODE_CODE93, -1, "ABC1234", -1, "ABC1234S5" }, // 2 checksums added (note check digits not shown by bwipp or tec-it)
        /* 17*/ { BARCODE_CODE93, -1, "abc1234", -1, "abc1234ZG" },
        /* 18*/ { BARCODE_CODE93, -1, "A\001a\000b\177d\037e", 9, "A a b d e1R" }, // NUL, ctrls and DEL replaced with spaces
        /* 19*/ { BARCODE_PZN, -1, "12345", -1, "PZN -00123458" }, // Pads with zeroes if length < 7
        /* 20*/ { BARCODE_PZN, -1, "123456", -1, "PZN -01234562" },
        /* 21*/ { BARCODE_PZN, -1, "1234567", -1, "PZN -12345678" },
        /* 22*/ { BARCODE_VIN, -1, "1FTCR10UXTPA78180", -1, "1FTCR10UXTPA78180" },
        /* 23*/ { BARCODE_VIN, 1, "2FTPX28L0XCA15511", -1, "2FTPX28L0XCA15511" }, // Include Import char - no change
        /* 24*/ { BARCODE_HIBC_39, -1, "ABC1234", -1, "*+ABC1234+*" },
        /* 25*/ { BARCODE_HIBC_39, -1, "abc1234", -1, "*+ABC1234+*" }, // Converts to upper
        /* 26*/ { BARCODE_HIBC_39, -1, "123456789", -1, "*+1234567890*" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_hrt");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_zero(ret, "i:%d ZBarcode_Encode ret %d != 0 %s\n", i, ret, symbol->errtxt);

        assert_zero(strcmp((char *) symbol->text, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->text, data[i].expected);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(int index, int debug) {

    struct item {
        int symbology;
        int option_2;
        char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "-", -1, 0, 1, 37 },
        /*  1*/ { BARCODE_CODE11, -1, "A", -1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  2*/ { BARCODE_CODE11, 3, "1", -1, ZINT_ERROR_INVALID_OPTION, -1, -1 },
        /*  3*/ { BARCODE_CODE39, -1, "a", -1, 0, 1, 38 }, // Converts to upper
        /*  4*/ { BARCODE_CODE39, -1, ",", 1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  5*/ { BARCODE_CODE39, -1, "\000", 1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /*  6*/ { BARCODE_CODE39, 0, "1", -1, 0, 1, 38 },
        /*  7*/ { BARCODE_CODE39, 1, "1", -1, 0, 1, 51 }, // Check digit
        /*  8*/ { BARCODE_CODE39, 2, "1", -1, 0, 1, 38 }, // option_2 > 1 gnored
        /*  9*/ { BARCODE_EXCODE39, -1, "A", -1, 0, 1, 38 },
        /* 10*/ { BARCODE_EXCODE39, -1, "a", -1, 0, 1, 51 },
        /* 11*/ { BARCODE_EXCODE39, -1, ",", -1, 0, 1, 51 },
        /* 12*/ { BARCODE_EXCODE39, -1, "\000", 1, 0, 1, 51 },
        /* 13*/ { BARCODE_EXCODE39, -1, "é", -1, ZINT_ERROR_INVALID_DATA, -1, -1, },
        /* 14*/ { BARCODE_LOGMARS, -1, "A", -1, 0, 1, 47 },
        /* 15*/ { BARCODE_LOGMARS, -1, "a", -1, 0, 1, 47 },
        /* 16*/ { BARCODE_LOGMARS, -1, ",", -1, ZINT_ERROR_INVALID_DATA, -1, -1, },
        /* 17*/ { BARCODE_LOGMARS, -1, "\000", 1, ZINT_ERROR_INVALID_DATA, -1, -1, },
        /* 18*/ { BARCODE_CODE93, -1, "A", -1, 0, 1, 46 },
        /* 19*/ { BARCODE_CODE93, -1, "a", -1, 0, 1, 55 },
        /* 20*/ { BARCODE_CODE93, -1, ",", -1, 0, 1, 55 },
        /* 21*/ { BARCODE_CODE93, -1, "\000", 1, 0, 1, 55 },
        /* 22*/ { BARCODE_CODE93, -1, "é", -1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 23*/ { BARCODE_PZN, -1, "1", -1, 0, 1, 142 },
        /* 24*/ { BARCODE_PZN, -1, "A", -1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 25*/ { BARCODE_PZN, -1, "1000006", -1, ZINT_ERROR_INVALID_DATA, -1, -1 }, // Check digit == 10 so can't be used
        /* 26*/ { BARCODE_VIN, -1, "5GZCZ43D13S812715", -1, 0, 1, 246 },
        /* 27*/ { BARCODE_VIN, -1, "5GZCZ43D23S812715", -1, ZINT_ERROR_INVALID_CHECK, -1, -1 }, // North American with invalid check character
        /* 28*/ { BARCODE_VIN, -1, "WP0ZZZ99ZTS392124", -1, 0, 1, 246 }, // Not North American so no check
        /* 29*/ { BARCODE_VIN, -1, "WPOZZZ99ZTS392124", -1, ZINT_ERROR_INVALID_DATA, -1, -1 }, // O not allowed
        /* 30*/ { BARCODE_HIBC_39, -1, "a", -1, 0, 1, 79 }, // Converts to upper
        /* 31*/ { BARCODE_HIBC_39, -1, ",", -1, ZINT_ERROR_INVALID_DATA, -1, -1 },
        /* 32*/ { BARCODE_HIBC_39, -1, "\000", 1, ZINT_ERROR_INVALID_DATA, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        int symbology;
        int option_2;
        char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_CODE11, -1, "123-45", -1, 0, 1, 78, "2 check digits (52); verified manually against tec-it",
                    "101100101101011010010110110010101011010101101101101101011011010100101101011001"
                },
        /*  1*/ { BARCODE_CODE11, -1, "93", -1, 0, 1, 44, "2 check digits (--); verified manually against tec-it",
                    "10110010110101011001010101101010110101011001"
                },
        /*  2*/ { BARCODE_CODE11, 1, "123-455", -1, 0, 1, 78, "1 check digit (2); verified manually against tec-it",
                    "101100101101011010010110110010101011010101101101101101011011010100101101011001"
                },
        /*  3*/ { BARCODE_CODE11, 2, "123-4552", -1, 0, 1, 78, "0 check digits; verified manually against tec-it",
                    "101100101101011010010110110010101011010101101101101101011011010100101101011001"
                },
        /*  4*/ { BARCODE_CODE11, 1, "123-45", -1, 0, 1, 70, "1 check digit; verified manually against tec-it",
                    "1011001011010110100101101100101010110101011011011011010110110101011001"
                },
        /*  5*/ { BARCODE_CODE11, 2, "123-45", -1, 0, 1, 62, "0 check digits; verified manually against tec-it",
                    "10110010110101101001011011001010101101010110110110110101011001"
                },
        /*  6*/ { BARCODE_CODE39, -1, "1A", -1, 0, 1, 51, "ISO/IEC 16388:2007 Figure 1",
                    "100101101101011010010101101101010010110100101101101"
                },
        /*  7*/ { BARCODE_CODE39, 1, "1A", -1, 0, 1, 64, "With check digit (B)",
                    "1001011011010110100101011011010100101101011010010110100101101101"
                },
        /*  8*/ { BARCODE_CODE39, 1, "Z1", -1, 0, 1, 64, "Check digit '-'",
                    "1001011011010100110110101011010010101101001010110110100101101101"
                },
        /*  9*/ { BARCODE_CODE39, 1, "Z2", -1, 0, 1, 64, "Check digit '.'",
                    "1001011011010100110110101010110010101101100101011010100101101101"
                },
        /* 10*/ { BARCODE_CODE39, 1, "Z3", -1, 0, 1, 64, "Check digit space, displayed as underscore",
                    "1001011011010100110110101011011001010101001101011010100101101101"
                },
        /* 11*/ { BARCODE_CODE39, 1, "Z4", -1, 0, 1, 64, "Check digit '$'",
                    "1001011011010100110110101010100110101101001001001010100101101101"
                },
        /* 12*/ { BARCODE_CODE39, 1, "Z5", -1, 0, 1, 64, "Check digit '/'",
                    "1001011011010100110110101011010011010101001001010010100101101101"
                },
        /* 13*/ { BARCODE_CODE39, 1, "Z6", -1, 0, 1, 64, "Check digit '+'",
                    "1001011011010100110110101010110011010101001010010010100101101101"
                },
        /* 14*/ { BARCODE_CODE39, 1, "Z7", -1, 0, 1, 64, "Check digit '%'",
                    "1001011011010100110110101010100101101101010010010010100101101101"
                },
        /* 15*/ { BARCODE_CODE39, -1, "+A/E%U$A/D%T+Z", -1, 0, 1, 207, "Same as BARCODE_EXCODE39 'a%\000\001$\177z' below",
                    "100101101101010010100100101101010010110100100101001011010110010101010010010010110010101011010010010010101101010010110100100101001010101100101101010010010010101011011001010010100100101001101101010100101101101"
                },
        /* 16*/ { BARCODE_EXCODE39, -1, "1A", -1, 0, 1, 51, "ISO/IEC 16388:2007 Figure 1",
                    "100101101101011010010101101101010010110100101101101"
                },
        /* 17*/ { BARCODE_EXCODE39, 1, "1A", -1, 0, 1, 64, "With check digit",
                    "1001011011010110100101011011010100101101011010010110100101101101"
                },
        /* 18*/ { BARCODE_EXCODE39, 1, "Z4", -1, 0, 1, 64, "Check digit $",
                    "1001011011010100110110101010100110101101001001001010100101101101"
                },
        /* 19*/ { BARCODE_EXCODE39, -1, "a%\000\001$\177z", 7, 0, 1, 207, "Verified manually against tec-it",
                    "100101101101010010100100101101010010110100100101001011010110010101010010010010110010101011010010010010101101010010110100100101001010101100101101010010010010101011011001010010100100101001101101010100101101101"
                },
        /* 20*/ { BARCODE_EXCODE39, -1, "\033\037!+/\\@A~", -1, 0, 1, 246, "Verified manually against tec-it",
                    "100101101101010100100100101101010010110101001001001011010110010101001001010010110101001011010010010100101101010100110100100101001011010110100101010010010010101101010011010100100100101001101010110110101001011010100100100101011010110010100101101101"
                },
        /* 21*/ { BARCODE_LOGMARS, -1, "1A", -1, 0, 1, 63, "Verified manually against tec-it",
                    "100010111011101011101000101011101110101000101110100010111011101"
                },
        /* 22*/ { BARCODE_LOGMARS, 1, "1A", -1, 0, 1, 79, "With check digit; verified manually against tec-it",
                    "1000101110111010111010001010111011101010001011101011101000101110100010111011101"
                },
        /* 23*/ { BARCODE_LOGMARS, -1, "ABC", -1, 0, 1, 79, "MIL-STD-1189 Rev. B Figure 1",
                    "1000101110111010111010100010111010111010001011101110111010001010100010111011101"
                },
        /* 24*/ { BARCODE_LOGMARS, -1, "SAMPLE 1", -1, 0, 1, 159, "MIL-STD-1189 Rev. B Figure 2 top",
                    "100010111011101010111010111000101110101000101110111011101010001010111011101000101011101010001110111010111000101010001110101110101110100010101110100010111011101"
                },
        /* 25*/ { BARCODE_LOGMARS, 1, "12345/ABCDE", -1, 0, 1, 223, "MIL-STD-1189 Rev. B Section 6.2.1 check character example; verified manually against tec-it",
                    "1000101110111010111010001010111010111000101011101110111000101010101000111010111011101000111010101000100010100010111010100010111010111010001011101110111010001010101011100010111011101011100010101010111011100010100010111011101"
                },
        /* 26*/ { BARCODE_CODE93, -1, "1A", -1, 0, 1, 55, "Verified manually against tec-it",
                    "1010111101010010001101010001101000101001110101010111101"
                },
        /* 27*/ { BARCODE_CODE93, -1, "TEST93", -1, 0, 1, 91, "Verified manually against tec-it",
                    "1010111101101001101100100101101011001101001101000010101010000101011101101001000101010111101"
                },
        /* 28*/ { BARCODE_CODE93, -1, "\000a\177", 3, 0, 1, 91, "Verified manually against tec-it",
                    "1010111101110110101100101101001100101101010001110110101101001101011011101010010001010111101"
                },
        /* 29*/ { BARCODE_PZN, -1, "1234567", -1, 0, 1, 142, "Example from IFA Info Code 39 EN V2.1; verified manually against tec-it",
                    "1001011011010100101011011011010010101101011001010110110110010101010100110101101101001101010101100110101010100101101101101001011010100101101101"
                },
        /* 30*/ { BARCODE_PZN, -1, "2758089", -1, 0, 1, 142, "Example from IFA Info Check Digit Calculations EN 15 July 2019; verified manually against tec-it",
                    "1001011011010100101011011010110010101101010010110110110100110101011010010110101010011011010110100101101010110010110101011001011010100101101101"
                },
        /* 31*/ { BARCODE_VIN, -1, "1FTCR10UXTPA78180", -1, 0, 1, 246, "https://www.vinquery.com/img/vinbarcode/vinbarcode4.jpg",
                    "100101101101011010010101101011011001010101011011001011011010010101101010110010110100101011010100110110101100101010110100101101011010101101100101011011010010110101001011010100101101101101001011010110100101011011010010110101010011011010100101101101"
                },
        /* 32*/ { BARCODE_VIN, 1, "2FTPX28L0XCA15511", -1, 0, 1, 259, "With Import 'I' prefix; https://www.vinquery.com/img/vinbarcode/vinbarcode1.jpg",
                    "1001011011010101101001101010110010101101011011001010101011011001010110110100101001011010110101100101011011010010110101011010100110101001101101010010110101101101101001010110101001011011010010101101101001101010110100110101011010010101101101001010110100101101101"
                },
        /* 33*/ { BARCODE_HIBC_39, -1, "A123BJC5D6E71", -1, 0, 1, 271, "ANSI/HIBC 2.6 - 2016 Figure 2, same",
                    "1000101110111010100010100010001011101010001011101110100010101110101110001010111011101110001010101011101000101110101011100011101011101110100010101110100011101010101011100010111010111000111010101110101110001010101000101110111011101000101011101010100011101110100010111011101"
                },
        /* 34*/ { BARCODE_HIBC_39, -1, "$$52001510X3G", -1, 0, 1, 271, "ANSI/HIBC 2.6 - 2016 Figure 6, same",
                    "1000101110111010100010100010001010001000100010101000100010001010111010001110101010111000101011101010001110111010101000111011101011101000101011101110100011101010111010001010111010100011101110101000101110101110111011100010101010101000111011101010111000101110100010111011101"
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

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %d, %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
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

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, -1, debug)) {
                    ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
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
        { "test_hrt", test_hrt, 1, 0, 1 },
        { "test_input", test_input, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
