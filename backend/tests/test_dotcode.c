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

static void test_input(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        int eci;
        unsigned char *data;
        int length;
        int ret;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "A", -1, 0, "66 21", "" },
        /*  1*/ { UNICODE_MODE, 3, "A", -1, 0, "6C 03 66 21", "" },
        /*  2*/ { UNICODE_MODE, 40, "A", -1, 0, "6C 28 00 00 66 21", "" },
        /*  3*/ { UNICODE_MODE, 113, "A", -1, 0, "6C 28 00 49 66 21", "" },
        /*  4*/ { UNICODE_MODE, 899, "A", -1, 0, "6C 28 07 44 66 21", "" },
        /*  5*/ { UNICODE_MODE, 12769, "A", -1, 0, "6C 28 70 49 66 21", "" },
        /*  6*/ { UNICODE_MODE, 811799, "A", -1, 0, "6C 67 40 50 66 21", "" },
        /*  7*/ { UNICODE_MODE, -1, "\000", 1, 0, "65 40", "LatchA (0x65) NUL" },
        /*  8*/ { UNICODE_MODE, -1, "\010", -1, 0, "65 48", "LatchA (0x65) BS" },
        /*  9*/ { UNICODE_MODE, -1, "\011", -1, 0, "65 49", "Lead special; LatchA (0x65) HT" },
        /* 10*/ { UNICODE_MODE, -1, "\034", -1, 0, "65 5C", "Lead special; LatchA (0x65) FS" },
        /* 11*/ { UNICODE_MODE, -1, "\035", -1, 0, "65 5D", "Lead special; LatchA (0x65) GS" },
        /* 12*/ { UNICODE_MODE, -1, "\036", -1, 0, "65 5E", "Lead special; LatchA (0x65) RS" },
        /* 13*/ { UNICODE_MODE, -1, "\037", -1, 0, "65 5F", "LatchA (0x65) US" },
        /* 14*/ { UNICODE_MODE, -1, "\177", -1, 0, "66 5F", "ShiftB (0x66) DEL" },
        /* 15*/ { UNICODE_MODE, -1, "[)>\03605\035A\036\004", -1, 0, "6A 61 21", "[)>RS 05 GS A RS EOT; LatchB (0x6A) Macro97 (0x61) A" },
        /* 16*/ { UNICODE_MODE, -1, "[)>\03606\035\011\034\035\036\036\004", -1, 0, "6A 62 61 62 63 64", "[)>RS 06 GS HT FS GS RS RS EOT; LatchB (0x6A) Macro98 (0x62) HT FS GS RS" },
        /* 17*/ { UNICODE_MODE, -1, "[)>\03612\03512345\036\004", -1, 0, "6A 63 11 67 17 2D", "[)>RS 12 GS A RS EOT; LatchB (0x6A) Macro99 (0x63) 1 2xShiftC (0x67) 23 45" },
        /* 18*/ { UNICODE_MODE, -1, "[)>\03601Blah\004", -1, 0, "6A 64 10 11 22 4C 41 48", "[)>RS 01 Blah EOT; LatchB (0x6A) Macro100 (0x64) 0 1 B l a h" },
        /* 19*/ { UNICODE_MODE, -1, "[)>\03605\035A\004", -1, 0, "6A 64 10 15 63 21", "[)>RS 05 GS A EOT; LatchB (0x6A) Macro100 (0x64) 0 5 HT A" },
        /* 20*/ { UNICODE_MODE, -1, "[)>\03606A\004", -1, 0, "6A 64 10 16 21", "[)>RS 06 A EOT; LatchB (0x6A) Macro100 (0x64) 0 6 A" },
        /* 21*/ { UNICODE_MODE, -1, "[)>\036991\036\004", -1, 0, "6A 64 19 19 11 64", "[)>RS 99 1 RS EOT; LatchB (0x6A) Macro100 (0x64) 9 9 1 RS" },
        /* 22*/ { UNICODE_MODE, -1, "1712345610", -1, 0, "6B 64 0C 22 38", "FNC1 (0x6B) 17..10 12 34 56" },
        /* 23*/ { GS1_MODE, -1, "[17]123456[10]123", -1, 0, "64 0C 22 38 0C 66 13", "17..10 12 34 56 12 ShiftB (0x66) 3" },
        /* 24*/ { GS1_MODE, -1, "[90]ABC[90]abc[90]123", -1, 0, "5A 6A 21 22 23 6B 19 10 41 42 43 6B 19 67 01 17", "90 LatchB (0x6A) A B C FNC1 (0x6B) 9 0 a b c FNC1 (0x6B) 9 2xShitfC (0x67) 01 23" },
        /* 25*/ { UNICODE_MODE, -1, "99aA[{00\000", 9, 0, "6B 63 6A 41 21 3B 5B 10 10 65 40", "FNC1 (0x6B) 99 LatchB (0x6A) a A [ { 0 0 ShiftA (0x65) NUL" },
        /* 26*/ { UNICODE_MODE, -1, "\015\012", -1, 0, "66 60", "ShiftB (0x66) CR/LF" },
        /* 27*/ { UNICODE_MODE, -1, "A\015\012", -1, 0, "67 21 60", "2xShiftB (0x67) A CR/LF" },
        /* 28*/ { UNICODE_MODE, -1, "\015\015\012", -1, 0, "65 4D 4D 4A", "LatchA (0x65) CR CR LF" },
        /* 29*/ { UNICODE_MODE, -1, "ABCDE12345678", -1, 0, "6A 21 22 23 24 25 69 0C 22 38 4E", "LatchB (0x6A) A B C D 4xShiftC 12 34 56 78" },
        /* 30*/ { UNICODE_MODE, -1, "\000ABCD1234567890", 15, 0, "65 40 21 22 23 24 6A 0C 22 38 4E 5A", "LatchA (0x65) NULL A B C D LatchC (0x6A) 12 34 56 78 90" },
        /* 31*/ { DATA_MODE, -1, "\141\142\143\144\145\200\201\202\203\204\377", -1, 0, "6A 41 42 43 44 45 70 31 5A 35 21 5A 5F 02 31", "LatchB (0x6A) a b c d e BinaryLatch (0x70) 0x80 0x81 0x82 0x83 0x84 0xFF" },
        /* 32*/ { DATA_MODE, -1, "\200\061\062\240\063\064\201\202\065\066", -1, 0, "6E 40 0C 6F 00 22 70 03 10 42 6E 15 16", "UpperShiftA (0x6E) NUL 12 UpperShiftB (0x6F) SP 34 BinaryLatch (0x70) 0x81 0x82 TermB (0x6E) 5 6" },
        /* 33*/ { DATA_MODE, -1, "\200\201\202\203\061\062\063\064", -1, 0, "70 13 56 0A 59 2C 67 0C 22", "BinaryLatch (0x70) 0x80 0x81 0x82 0x83 Intr2xShiftC (0x67) 12 3" },
        /* 34*/ { DATA_MODE, -1, "\001\200\201\202\203\204\200\201\202\203\204", -1, 0, "65 41 70 31 5A 35 21 5A 5F 31 5A 35 21 5A 5F", "LatchA (0x65) SOH BinaryLatch (0x70) 0x80 0x81 0x82 0x83 0x80 0x81 0x82 0x83" },
        /* 35*/ { UNICODE_MODE, -1, "\001abc\011\015\012\036", -1, 0, "65 41 65 41 42 43 61 60 64", "LatchA (0x65) SOH 6xShiftB (0x65) a b c HT CR/LF RS" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DOTCODE;
        symbol->input_mode = data[i].input_mode;
        if (data[i].eci != -1) {
            symbol->eci = data[i].eci;
        }
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt
        symbol->debug |= debug;

        int length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %d, %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    data[i].length, testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            if (ret < 5) {
                assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        int option_2;
        unsigned char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "2741", -1, 0, 10, 13, "ISS DotCode Rev 4.0 Figure 7B Mask = 2 prime (5)",
                    "1010001010101"
                    "0001000000000"
                    "1000100010101"
                    "0100000101000"
                    "0000101000100"
                    "0100010000010"
                    "1000101010001"
                    "0101010001000"
                    "1000100010101"
                    "0101000100010"
               },
        /*  1*/ { GS1_MODE, 64, "[01]00012345678905[17]201231[10]ABC123456", -1, 0, 9, 64, "ISS DotCode Rev 4.0 Figure 1 (left)",
                    "1010000000101000101010000010000010001010100010101000101000001010"
                    "0100010001010001010001000001010100010100010001000100010101000001"
                    "1010001010000000101010100010001010000010101000000010100010100000"
                    "0000000101000101010001010100000001000100010001000100010100000001"
                    "1000001000000010100010101000001010100000101010100000100010001010"
                    "0101000101000001000001010001010100000001000100010101000101010001"
                    "1000000010100010001000001000100010101000100010000010101000100000"
                    "0001010100010001010100010001010000010001010000000101010001010101"
                    "1000100010001000100010100010001010001000101000101000100010000010"
               },
        /*  2*/ { GS1_MODE, -1, "[01]00012345678905[17]201231[10]ABC123456", -1, 0, 20, 29, "ISS DotCode Rev 4.0 Figure 1 (right)",
                    "10101000101010100010101000101"
                    "00010100010100010100000001010"
                    "00001010100010000000101010000"
                    "01000001010101010101010101000"
                    "10101010100000000010100010101"
                    "01000100000000000101010101010"
                    "00100010001010101000100000101"
                    "00000101010000000001010000010"
                    "10001010100010101010001010100"
                    "01010000000000010000000100010"
                    "00100000000010100000100000000"
                    "00010000000101000001000001000"
                    "10101000101000001010001010001"
                    "01010001010001010101000000010"
                    "00001000001010001010100000101"
                    "01000100010100000000010100010"
                    "10100010000010101000101010001"
                    "00010101000001010100010100010"
                    "10000010101000100000001000001"
                    "01000100010101010000000101010"
               },
        /*  3*/ { GS1_MODE, -1, "[17]070620[10]ABC123456", -1, 0, 16, 23, "ISS DotCode Rev 4.0 Figure 6, Mask = 1",
                    "10000000001010001000101"
                    "01010101000100000101000"
                    "00100010000000100000001"
                    "01010001000001000001000"
                    "10101010100000001010101"
                    "00000100010100000100010"
                    "00000000001010101010001"
                    "00010001010001000001000"
                    "00101010101000001010001"
                    "01000100000001010000000"
                    "10101000101000101000001"
                    "00010101000100010101010"
                    "10001000001010100000101"
                    "01010001010001000001010"
                    "10000010101010100010101"
                    "01000101000101010101010"
               },
        /*  4*/ { GS1_MODE, 40, "[01]00012345678905", -1, 0, 7, 40, "ISS DotCode Rev 4.0 Figure 8, 7x40 **NOT SAME** but same if force mask 1 instead of mask 6",
                    "1010000010001010000010100000001010100010"
                    "0101010001000100010100000101010000010101"
                    "0000001010100010001010001000100000100010"
                    "0001000001000101010100000100010100010001"
                    "1000101010100000100000101000000010101010"
                    "0100010000010001000101000101010001000001"
                    "1010000010001000100010101000101000100010"
               },
        /*  5*/ { GS1_MODE, 18, "[01]00012345678905", -1, 0, 17, 18, "ISS DotCode Rev 4.0 Figure 8, 17x18 **NOT SAME** no matter what mask; but verified manually against bwipp and tec-it",
                    "101000001000101010"
                    "010100000101010001"
                    "000000101000001010"
                    "000100010101000101"
                    "001010000000100010"
                    "010100000100010101"
                    "100010001000001010"
                    "010001000100010100"
                    "001000001010000010"
                    "010100000001010001"
                    "000000101010001010"
                    "000101000001000101"
                    "100010001010100010"
                    "000100010000000101"
                    "100010001010001010"
                    "010001010001000101"
                    "100010001000100010"
               },
        /*  6*/ { GS1_MODE, 35, "[01]00012345678905", -1, 0, 8, 35, "ISS DotCode Rev 4.0 Figure 8, 8x35; **NOT SAME** using mask 3 prime (7) not 3 so extra dot in bottom right corner compared to figure",
                    "10100010000000000010100000100010101"
                    "00010101010001000000010100010100000"
                    "10001000101010101010001010000010101"
                    "01010001000100000101000100010101010"
                    "10101000100000101000100010001000001"
                    "00010100010000010001010001010000000"
                    "10000010101010101010000010000010001"
                    "01000001000101000100010100010001010"
               },
        /*  7*/ { GS1_MODE, 17, "[01]00012345678905", -1, 0, 18, 17, "ISS DotCode Rev 4.0 Figure 8, 18x17 **NOT SAME** no matter what mask; verified manually against bwipp and tec-it",
                    "10101000001000001"
                    "01000001010100010"
                    "00000000100010001"
                    "00010101000101010"
                    "10101000001010000"
                    "01000100010000000"
                    "00000010000000100"
                    "01010000000001000"
                    "10101010101000101"
                    "00000000010101010"
                    "00101010100000000"
                    "01000101000001010"
                    "10001000000010001"
                    "00000001010100010"
                    "00100010001000101"
                    "01010100010101000"
                    "10101010101010101"
                    "01010101000101010"
               },
        /*  8*/ { UNICODE_MODE, 35, "Dots can be Square!", -1, 0, 18, 35, "ISS DotCode Rev 4.0 Figure 11 **NOT SAME**; verified manually against bwipp and tec-it",
                    "10000010101000000000000000101010101"
                    "01010101000101000100010100000001000"
                    "00001000000010101000101010101010000"
                    "01000001000100000001010001000000000"
                    "00100010101010101000000010100000101"
                    "00000001010001010101010100010101000"
                    "10101000100010000010101010001010001"
                    "00010001010100010101000101000101010"
                    "00101010001000001010101000100000101"
                    "00010100010001010000000001010001010"
                    "00101000100010100000100000000000101"
                    "01010100010001010100010100000100000"
                    "10000010000000000010001000101010100"
                    "00010000000100010001000000010001010"
                    "10001000001010101010001010101000001"
                    "01000101010101000100000100010101000"
                    "10101000101000001000100010101000101"
                    "01000001000001000101010001000000010"
               },
        /*  9*/ { GS1_MODE, -1, "[99]8766", -1, 0, 10, 13, "ISS DotCode Rev 4.0 Table G.1 Mask 0 prime (4); all mask scores match Table G.1",
                    "1000001010001"
                    "0001010000010"
                    "0000000010001"
                    "0100010101000"
                    "0010101000101"
                    "0100010101010"
                    "0010000010000"
                    "0101010000010"
                    "1010000000101"
                    "0101000101010"
               },
        /* 10*/ { UNICODE_MODE, 6, "A", -1, 0, 19, 6, "ISS DotCode Rev 4.0 5.2.1.4 2) Table 4, 1 padding dot available; same as bwipp and tec-it except corners lit",
                    "101010"
                    "000101"
                    "101010"
                    "000001"
                    "100010"
                    "000100"
                    "001010"
                    "000101"
                    "101000"
                    "010000"
                    "100010"
                    "010000"
                    "000010"
                    "000101"
                    "101010"
                    "000001"
                    "101000"
                    "010001"
                    "101010"
               },
        /* 11*/ { UNICODE_MODE, 94, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRS", -1, 0, 37, 94, "Interleaved R-S; same as bwipp and tec-it except corners lit",
                    "1000001010000000100010000010101010101000001000100000001010101000001000001000101010001000101010"
                    "0101010000000101000001010001010001010100010001000001000000010101010000000101010100010001010101"
                    "0010101000100010000010101010000000101010000010101000001000100010100000100010100010001000101000"
                    "0000000100010101000001010000010000010101010100010100000100000101000100010001000001010001010001"
                    "0010100000100010101000001000101010000010001000001010100000101000101010000010001000101010100010"
                    "0100000101010001010001000101000001000100000101010001000101000100010100000100010100010001010100"
                    "1010000000100010100000101010000010101000001000001010001010000010001010100000101010100000100000"
                    "0001010100000101000001000000010001010100000101000100010101000001000101000100010000010101010001"
                    "1010001010001010000000100000101010100000100010101000000010100010001010100010000010100010001010"
                    "0001000100010100010001010000000101010100010000010100000100000101010100010000010100010001000001"
                    "1000001010001010100010000010001000100010100010100010000010001000101010100000101000001000101010"
                    "0101000001000101000100000001010001010101010001000001000000010001010100010101010000010100000001"
                    "0000100000001010100010101000000010001010001010000000101010001000101010001000101000100000100010"
                    "0000000100000100000101010101000000010101010001000100010001000101010001000100010000010100010101"
                    "0010101000001010001000101000000000101010001010000010101010000010001000100000001010101010001010"
                    "0100010001010100010000010100010001010100010000010001000100000100010101000101000100000101010001"
                    "1000001000100010101010101000000010001000001010000010101010000010101010000000100010001000101000"
                    "0000010000010101010000010101000000010101000001000101000100010001000101010001000100010000010100"
                    "1000001000101010000010001000001000101010100000100000001010001010101000000010001010101010001000"
                    "0100010001010001000100010000010100000100010100010100010001000001010100010101000100000101000101"
                    "1010000000101000100000001000101010100010000000101010101010000010101000000010100010001000100010"
                    "0100000101010000010100010001010100010000010101000100000100000101000001010100010000000101010001"
                    "1000001000001010001010001000101000001000101010000010101000100010100000101000100000001000101010"
                    "0101010100010001010101000000010101010000010100000100010100010001000100000100010001010100010100"
                    "1000100000101000000010100010101000001010100010100000100000101010000010101000100000001010000000"
                    "0001000001000101000101010000010001000100000101010001000100010100000101000101000100000101010001"
                    "0000000010001000101000100000100000101010101000000010101010001000100010001000101010001000100010"
                    "0101010100010000000100010101010001010100000001010101000001000001000001010100000101010001010101"
                    "1000100010101000100000101010001000101010000010001010001010001010000010100010100010000000100000"
                    "0000010101010000010101000000010101000001000101000100010001000101010001000100010000010100010000"
                    "1000101010000010001000001000101010100000100000001010001010101000000010001010101010001000000010"
                    "0100000101000001010001010000000100010100010100010001000001010100010101000100000101000101010000"
                    "0000001010001000100010100010000010101000100010001000101000001000100000101000100010100010001010"
                    "0101010000010100010000010100000101010100000101000001000000010101000101000101010000000101010101"
                    "1000001010001010001000101000001000101010000010101000100010100000101000100000001000101010100000"
                    "0001000101010100000001010101000001010000010001010001000100010000010001000101010001010001000001"
                    "1010001000001010101000000010101000101000001000001010100000101010001000000010100000001010101010"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DOTCODE;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        symbol->debug |= debug;

        int length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %d, %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_2, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesDump(symbol, "                    ", "\n");
            printf("               },\n");
        } else {
            if (ret < 5) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                if (ret == 0) {
                    int width, row;
                    ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                    assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// #181 Christian Hartlage / Nico Gunkel OSS-Fuzz
static void test_fuzz(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int length;
        int input_mode;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "(\207'", -1, DATA_MODE, 0 }, // 0x28,0x87,0x27 Note: should but doesn't trigger sanitize error if no length check, for some reason; UPDATE: use up-to-date gcc (9)!
        /*  1*/ {
                    "\133\061\106\133\061\106\070\161\116\133\116\116\067\040\116\016\000\116\125\111\125\125\316\125\125\116\116\116\116\117\116\125"
                    "\111\125\103\316\125\125\116\116\116\116\117\000\000\116\136\116\116\001\116\316\076\116\116\057\136\116\116\134\000\000\116\116"
                    "\116\230\116\116\116\116\125\125\125\257\257\257\000\001\116\130\212\212\212\212\212\212\212\377\377\210\212\212\177\000\212\212"
                    "\212\212\212\212\175\212\212\212\212\212\212\116\117\001\116\116\112\116\116\116\116\176\136\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\005\377\377\005\125\125\125\325\001\116\116\116\266\116\020\000\200\000\116\116\177\000\000\377"
                    "\377\257\257\257\125\112\117\116\001\000\000\044\241\001\116\116\116\136\116\116\116\056\116\125\111\125\125\316\125\125\116\116"
                    "\116\116\057\000\000\116\136\116\116\001\116\116\076\342\116\057\136\116\116\134\000\000\116\116\116\241\116\116\116\116\125\125"
                    "\125\257\257\257\000\001\116\130\212\212\212\212\212\212\212\212\172\212\071\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\110\071\071\051\071\065\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330\330"
                    "\330\330\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\071\065\071\071\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\071\071\072\071\071\277\071\071\077\071\071\071\071\071\071\071\071\154\071\071\071\071\071\071\071\071\071\071\071"
                    "\071\071\071\011\071\071\071\071\071\071\071\071\071\071\071\071\071\071\105\105\105\105\105\105\105\105\105\105\105\105\105\071"
                    "\071\071\071\071\071", // Original OSS-Fuzz triggering data for index out of bounds (encoding of HT/FS/GS/RS when shifting to code set B)
                    421, UNICODE_MODE, ZINT_WARN_USES_ECI },
        /*  2*/ { "\233:", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // Original OSS-Fuzz triggering data for codeword_array buffer overflow, L777
        /*  3*/ { "\241\034", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // As above L793
        /*  4*/ { "\270\036", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // As above L799
        /*  5*/ { "\237\032", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // As above L904
        /*  6*/ { "\237", -1, UNICODE_MODE, ZINT_WARN_USES_ECI }, // As above L1090
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DOTCODE;
        if (data[i].input_mode != -1) {
            symbol->input_mode = data[i].input_mode;
        }
        symbol->debug |= debug;

        int length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_large(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int option_2;
        char datum;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 200, '0', 2940, 0 }, // 2940 largest Code Set C data that fits in 200x199 HxW
        /*  1*/ { 200, '0', 2941, ZINT_ERROR_INVALID_OPTION },
        /*  2*/ { 200, '9', 200, 0 }, // Changes a number of mask scores re pre-Rev. 4 version, but best score still the same (7)
        /*  3*/ { 30, '\001', 71, 0 }, // Codeword length 72, ECC length 39, for ND + 1 == 112
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char data_buf[4096];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_DOTCODE;
        symbol->input_mode = DATA_MODE;
        symbol->option_2 = data[i].option_2;
        symbol->debug |= debug;

        int length = data[i].length;

        memset(data_buf, data[i].datum, length);

        ret = ZBarcode_Encode(symbol, data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_fuzz", test_fuzz, 1, 0, 1 },
        { "test_large", test_large, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
