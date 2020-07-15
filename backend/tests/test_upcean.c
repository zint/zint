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

static void test_upce_length(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_UPCE, "12345", 0 },
        /* 1*/ { BARCODE_UPCE_CHK, "12345", ZINT_ERROR_INVALID_CHECK },
        /* 2*/ { BARCODE_UPCE_CHK, "12344", 0 }, // 4 is correct check digit
        /* 3*/ { BARCODE_UPCE, "123456", 0 },
        /* 4*/ { BARCODE_UPCE_CHK, "123456", ZINT_ERROR_INVALID_CHECK },
        /* 5*/ { BARCODE_UPCE_CHK, "123457", 0 }, // 7 is correct check digit
        /* 6*/ { BARCODE_UPCE, "1234567", 0 },
        /* 7*/ { BARCODE_UPCE_CHK, "1234567", ZINT_ERROR_INVALID_CHECK },
        /* 8*/ { BARCODE_UPCE_CHK, "1234565", 0 }, // 5 is correct check digit
        /* 9*/ { BARCODE_UPCE, "12345678", ZINT_ERROR_TOO_LONG },
        /*10*/ { BARCODE_UPCE_CHK, "12345678", ZINT_ERROR_INVALID_CHECK },
        /*11*/ { BARCODE_UPCE_CHK, "12345670", 0 }, // 0 is correct check digit
        /*12*/ { BARCODE_UPCE, "123456789", ZINT_ERROR_TOO_LONG },
        /*13*/ { BARCODE_UPCE_CHK, "123456789", ZINT_ERROR_TOO_LONG },
        /*14*/ { BARCODE_UPCE, "123406", ZINT_ERROR_INVALID_DATA }, // If last digit (emode) 6, 2nd last can't be zero
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Note requires ZINT_SANITIZE to be set
static void test_upca_print(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_UPCA, "01234567890", 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        strcpy(symbol->outfile, "out.gif");
        ret = ZBarcode_Print(symbol, 0);
        assert_zero(ret, "i:%d %s ZBarcode_Print %s ret %d != 0 (%s)\n", i, testUtilBarcodeName(data[i].symbology), symbol->outfile, ret, symbol->errtxt);

        assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_isbn(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        unsigned char *data;
        int ret_encode;
        int ret_vector;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "0", 0, 0 }, // Left zero-padded if < 10 chars
        /*  1*/ { "12345678", ZINT_ERROR_INVALID_CHECK, -1 },
        /*  2*/ { "12345679", 0, 0 }, // 9 is correct check digit
        /*  3*/ { "123456789", 0, 0 },
        /*  4*/ { "0123456789", 0, 0 },
        /*  5*/ { "1234567890", ZINT_ERROR_INVALID_CHECK, -1 },
        /*  6*/ { "123456789X", 0, 0 }, // X is correct check digit
        /*  7*/ { "123456789x", 0, 0 }, // x is correct check digit
        /*  8*/ { "8175257660", 0, 0 }, // 0 is correct check digit
        /*  9*/ { "0590764845", 0, 0 }, // 5 is correct check digit
        /* 10*/ { "0906495741", 0, 0 }, // 1 is correct check digit
        /* 11*/ { "0140430016", 0, 0 }, // 6 is correct check digit
        /* 12*/ { "0571086187", 0, 0 }, // 7 is correct check digit
        /* 13*/ { "0486600882", 0, 0 }, // 2 is correct check digit
        /* 14*/ { "12345678901", ZINT_ERROR_TOO_LONG, -1 },
        /* 15*/ { "123456789012", ZINT_ERROR_TOO_LONG, -1 },
        /* 16*/ { "1234567890123", ZINT_ERROR_INVALID_DATA, -1 },
        /* 17*/ { "9784567890120", 0, 0 }, // 0 is correct check digit
        /* 18*/ { "9783161484100", 0, 0 }, // 0 is correct check digit
        /* 19*/ { "9781846688225", 0, 0 }, // 5 is correct check digit
        /* 20*/ { "9781847657954", 0, 0 }, // 4 is correct check digit
        /* 21*/ { "9781846688188", 0, 0 }, // 8 is correct check digit
        /* 22*/ { "9781847659293", 0, 0 }, // 3 is correct check digit
        /* 23*/ { "97845678901201", ZINT_ERROR_TOO_LONG, -1 },
        /* 24*/ { "3954994+12", 0, 0 },
        /* 25*/ { "3954994+12345", 0, 0 },
        /* 26*/ { "3954994+123456", ZINT_ERROR_TOO_LONG, -1 },
        /* 27*/ { "3954994+", 0, 0 },
        /* 28*/ { "61954993+1", 0, 0 },
        /* 29*/ { "61954993+123", 0, 0 },
        /* 30*/ { "361954999+12", 0, 0 },
        /* 31*/ { "361954999+1234", 0, 0 },
        /* 32*/ { "361954999+12", 0, 0 },
        /* 33*/ { "199900003X+12", 0, 0 },
        /* 34*/ { "199900003x+12", 0, 0 },
        /* 35*/ { "199900003X+12345", 0, 0 },
        /* 36*/ { "199900003x+12345", 0, 0 },
        /* 37*/ { "9791234567896+12", 0, 0 },
        /* 38*/ { "9791234567896+12345", 0, 0 },
        /* 39*/ { "9791234567896+", 0, 0 },
        /* 40*/ { "97912345678961+", ZINT_ERROR_TOO_LONG, -1 },
        /* 41*/ { "97912345678961+12345", ZINT_ERROR_TOO_LONG, -1 },
        /* 42*/ { "9791234567896+123456", ZINT_ERROR_TOO_LONG, -1 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_ISBNX;
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d (%s)\n", i, ret, data[i].ret_vector, symbol->errtxt);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_vector_same(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        int ret_encode;
        int ret_vector;
    };
    struct item data[] = {
        /* 0*/ { BARCODE_UPCE, "123456", 0, 0 },
        /* 1*/ { BARCODE_UPCE_CHK, "1234565", 0, 0 }, // 5 is correct check digit
        /* 2*/ { BARCODE_ISBNX, "0195049969", 0, 0 }, // 9 is correct check digit
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_vector *vectors[4];
        int vectors_size = sizeof(vectors) / sizeof(struct zint_vector*);

        for (int j = 0; j < vectors_size; j++) {
            struct zint_symbol *symbol = ZBarcode_Create();
            assert_nonnull(symbol, "Symbol not created\n");

            symbol->symbology = data[i].symbology;
            symbol->debug |= debug;

            int length = strlen(data[i].data);

            ret = ZBarcode_Encode(symbol, data[i].data, length);
            assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d (%s)\n", i, ret, data[i].ret_vector, symbol->errtxt);

            assert_nonnull(symbol->vector, "i:%d symbol->vector NULL\n", i);
            vectors[j] = testUtilVectorCpy(symbol->vector);

            ZBarcode_Delete(symbol);
        }

        for (int j = 1; j < vectors_size; j++) {
            ret = testUtilVectorCmp(vectors[j - 1], vectors[j]);
            assert_zero(ret, "i:%d testUtilVectorCmp ret %d != 0\n", i, ret);
        }

        for (int j = 0; j < vectors_size; j++) {
            struct zint_symbol symbol_vector;
            symbol_vector.vector = vectors[j];
            vector_free(&symbol_vector);
        }
    }

    testFinish();
}

// https://www.isbn-international.org/content/isbn-users-manual ISBN Users' Manual
// https://bisg.org/page/BarcodingGuidelines BISG Barcode Guidelines for the US Book Industry (BISG BGUSBI)
static void test_encode(int index, int generate, int debug) {

    testStart("");

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    int ret;
    struct item {
        int symbology;
        int option_2;
        unsigned char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_UPCA, -1, "01234567890", 0, 1, 95, "GGS Figure 5.1-1 UPC-A (also Figure 5.2.2.3-1., 5.2.6.6-2., 6.4.9-1. and BS EN 797:1996 Figure 3)",
                    "10100011010011001001001101111010100011011000101010101000010001001001000111010011100101001110101"
                },
        /*  1*/ { BARCODE_EANX, -1, "4512345678906", 0, 1, 95, "GGS Figure 5.1-1 EAN-13",
                    "10101100010110011001001101111010011101011100101010101000010001001001000111010011100101010000101"
                },
        /*  2*/ { BARCODE_EANX_CHK, -1, "9501101531000", 0, 1, 95, "GGS Figure 5.2.2.1-2. EAN-13 (also Figure 6.5.2.2-1., 6.5.3.2-1., 6.5.3.2-2., 6.5.6.4-1.)",
                    "10101100010100111011001100110010100111001100101010100111010000101100110111001011100101110010101"
                },
        /*  3*/ { BARCODE_EANX, -1, "9501234", 0, 1, 67, "GGS Figure 5.2.2.2-1. EAN-8",
                    "1010001011011000100011010011001010101101100100001010111001010000101"
                },
        /*  4*/ { BARCODE_UPCE, -1, "0012345", 0, 1, 51, "GGS Figure 5.2.2.4-1. UPC-E (also BS EN 797:1996 Figure 4)",
                    "101010011100110010011011011110100111010110001010101"
                },
        /*  5*/ { BARCODE_EANX, -1, "9771384524017+12", 0, 1, 122, "GGS Figure 5.2.2.5.1-2. EAN-13 barcode with two-digit add-on",
                    "10101110110010001011001101111010001001010001101010100111011011001011100111001011001101000100101000000010110011001010010011"
                },
        /*  6*/ { BARCODE_EANX, -1, "9780877799306+54321", 0, 1, 149, "GGS Figure 5.2.2.5.2-2. EAN-13 barcode with five-digit add-on",
                    "10101110110001001010011101101110010001011101101010100010011101001110100100001011100101010000101000000010110111001010100011010100001010010011010011001"
                },
        /*  7*/ { BARCODE_EANX, -1, "501234567890", 0, 1, 95, "GGS Figure 5.2.6.6-1. EAN-13 (also BS EN 797:1996 Figure 1)",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101"
                },
        /*  8*/ { BARCODE_EANX, -1, "2012345", 0, 1, 67, "GGS Figure 5.2.6.6-3. EAN-8",
                    "1010010011000110100110010010011010101000010101110010011101100110101"
                },
        /*  9*/ { BARCODE_UPCE_CHK, -1, "00783491", 0, 1, 51, "GGS Figure 5.2.6.6-4. UPC-E",
                    "101010011100100010110111010000101000110001011010101"
                },
        /* 10*/ { BARCODE_UPCA, -1, "01234567890+24", 0, 1, 124, "GGS Figure 5.2.6.6-5. UPC-A barcode with two-digit add-on",
                    "1010001101001100100100110111101010001101100010101010100001000100100100011101001110010100111010100000000010110010011010100011"
                },
        /* 11*/ { BARCODE_EANX, -1, "1234567891019+45678", 0, 1, 149, "GGS Figure 5.2.6.6-6. EAN-13 barcode with five-digit add-on",
                    "10100100110111101001110101100010000101001000101010100100011101001100110111001011001101110100101000000010110011101010110001010101111010010001010110111"
                },
        /* 12*/ { BARCODE_EANX, -1, "5024425377399", 0, 1, 95, "GGS Figure 5.10.5.12-1. EAN-13",
                    "10100011010011011001110101000110010011011100101010100001010001001000100100001011101001110100101"
                },
        /* 13*/ { BARCODE_UPCA_CHK, -1, "614141000302", 0, 1, 95, "GGS Figure 5.10.6.3-1. UPC-A",
                    "10101011110011001010001100110010100011001100101010111001011100101110010100001011100101101100101"
                },
        /* 14*/ { BARCODE_UPCA, -1, "61414123440", 0, 1, 95, "GGS Figure 6.4.17-2. UPC-A Daisy 5pc Set",
                    "10101011110011001010001100110010100011001100101010110110010000101011100101110011100101110010101"
                },
        /* 15*/ { BARCODE_EANX, -1, "5410038302178", 0, 1, 95, "GGS Figure 6.5.5.3-1. EAN-13",
                    "10101000110110011010011100011010111101000100101010100001011100101101100110011010001001001000101"
                },
        /* 16*/ { BARCODE_EANX, -1, "501234567890+12", 0, 1, 122, "BS EN 797:1996 Figure 5 EAN-13 with 2-digit add-on",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101000000010110011001010010011"
                },
        /* 17*/ { BARCODE_EANX, -1, "501234567890+86104", 0, 1, 149, "BS EN 797:1996 Figure 6 EAN-13 with 5-digit add-on",
                    "10100011010110011001101101111010100011011100101010101000010001001001000111010011100101110010101000000010110001001010101111010011001010001101010011101"
                },
        /* 18*/ { BARCODE_UPCA_CHK, -1, "614141234417+12345", 0, 1, 151, "Example of UPC-A with 5-digit add-on",
                    "1010101111001100101000110011001010001100110010101011011001000010101110010111001100110100010010100000000010110110011010010011010100001010100011010110001"
                },
        /* 19*/ { BARCODE_UPCE, -1, "1234567+12", 0, 1, 78, "Example of UPC-E with 2-digit add-on",
                    "101001001101111010100011011100100001010010001010101000000010110011001010010011"
                },
        /* 20*/ { BARCODE_UPCE_CHK, -1, "12345670+12345", 0, 1, 105, "Example of UPC-E with 5-digit add-on",
                    "101001001101111010100011011100100001010010001010101000000010110110011010010011010100001010100011010110001"
                },
        /* 21*/ { BARCODE_EANX, -1, "1234567+12", 0, 1, 94, "Example of EAN-8 with 2-digit add-on",
                    "1010011001001001101111010100011010101001110101000010001001110010101000000010110011001010010011"
                },
        /* 22*/ { BARCODE_EANX_CHK, -1, "12345670+12345", 0, 1, 121, "Example of EAN-8 with 5-digit add-on",
                    "1010011001001001101111010100011010101001110101000010001001110010101000000010110110011010010011010100001010100011010110001"
                },
        /* 23*/ { BARCODE_EANX, -1, "54321", 0, 1, 47, "Example of EAN-5",
                    "10110111001010100011010100001010010011010011001"
                },
        /* 24*/ { BARCODE_EANX, -1, "21", 0, 1, 20, "Example of EAN-2",
                    "10110010011010110011"
                },
        /* 25*/ { BARCODE_EANX_CHK, 9, "76543210+21", 0, 1, 96, "Example of EAN-8 barcode with two-digit add-on, addon_gap 9",
                    "101011101101011110110001010001101010100001011011001100110111001010100000000010110010011010110011"
                },
        /* 26*/ { BARCODE_EANX, 12, "7654321+54321", 0, 1, 126, "Example of EAN-8 barcode with five-digit add-on, addon_gap 12",
                    "101011101101011110110001010001101010100001011011001100110111001010100000000000010110111001010100011010100001010010011010011001"
                },
        /* 27*/ { BARCODE_UPCA_CHK, 10, "210987654329+21", 0, 1, 125, "Example of UPC-A with 2-digit add-on, addon_gap 10",
                    "10100100110011001000110100010110110111011101101010101000010011101011100100001011011001110100101000000000010110010011010110011"
                },
        /* 28*/ { BARCODE_UPCA, 12, "21098765432+12121", 0, 1, 154, "Example of UPC-A with 5-digit add-on, addon_gap 12",
                    "1010010011001100100011010001011011011101110110101010100001001110101110010000101101100111010010100000000000010110011001010010011010110011010011011010011001"
                },
        /* 29*/ { BARCODE_UPCE_CHK, 8, "06543217+21", 0, 1, 79, "Example of UPC-A with 2-digit add-on, addon_gap 8",
                    "1010000101011000100111010111101001101100110010101010000000010110010011010110011"
                },
        /* 30*/ { BARCODE_UPCE, 11, "1654321+12121", 0, 1, 109, "Example of UPC-A with 5-digit add-on, addon_gap 11",
                    "1010101111011100101000110111101001101101100110101010000000000010110011001010010011010110011010011011010011001"
                },
        /* 31*/ { BARCODE_ISBNX, -1, "9789295055124", 0, 1, 95, "ISBN Users' Manual 7th Ed. 13.2",
                    "10101110110001001001011100100110010111011000101010111001010011101001110110011011011001011100101"
                },
        /* 32*/ { BARCODE_ISBNX, -1, "9780123456786+50995", 0, 1, 149, "BISG BGUSBI Figure 1: Bookland EAN (with Price Add-On) (and Figures 4, 5, 6)",
                    "10101110110001001010011100110010011011011110101010101110010011101010000100010010010001010000101000000010110110001010100111010001011010001011010111001"
                },
        /* 33*/ { BARCODE_ISBNX, -1, "9781402894626+50495", 0, 1, 149, "BISG BGUSBI Figure 2",
                    "10101110110001001011001101000110100111001001101010100100011101001011100101000011011001010000101000000010110111001010001101010100011010001011010111001"
                },
        /* 34*/ { BARCODE_ISBNX, 10, "9780940016644+50750", 0, 1, 152, "BISG BGUSBI Figure 3, addon_gap 10",
                    "10101110110001001010011100010110011101000110101010111001011001101010000101000010111001011100101000000000010110111001010001101010010001010110001010001101"
                },
        /* 35*/ { BARCODE_ISBNX, -1, "9791234567896+12", 0, 1, 122, "Example of ISBN with 2-digit add-on",
                    "10101110110010111011001100100110100001010001101010100111010100001000100100100011101001010000101000000010110011001010010011"
                },
    };
    int data_size = ARRAY_SIZE(data);

    char bwipp_buf[4096];
    char bwipp_msg[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, %d, \"%s\",\n",
                                i, testUtilBarcodeName(data[i].symbology), data[i].option_2, data[i].data, testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesDump(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < 5) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                if (ret == 0) {
                    int width, row;
                    ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                    assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
                }

                if (do_bwipp && testUtilCanBwipp(symbol->symbology, -1, data[i].option_2, -1, debug)) {
                    ret = testUtilBwipp(symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(data[i].symbology), ret);

                    ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(data[i].symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// #181 Christian Hartlage OSS-Fuzz
static void test_fuzz(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        int symbology;
        unsigned char *data;
        int length;
        int ret;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%2d*\/", line(".") - line("'<"))
    struct item data[] = {
        /* 0*/ { BARCODE_EANX, "55++15", -1, ZINT_ERROR_INVALID_DATA },
        /* 1*/ { BARCODE_EANX, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 2*/ { BARCODE_EANX_CHK, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 3*/ { BARCODE_UPCA, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 4*/ { BARCODE_UPCA_CHK, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 5*/ { BARCODE_UPCE, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 6*/ { BARCODE_UPCE_CHK, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 7*/ { BARCODE_ISBNX, "+123456789012345678", -1, ZINT_ERROR_TOO_LONG },
        /* 8*/ { BARCODE_EANX, "+12345", -1, 0 },
        /* 9*/ { BARCODE_EANX, "+123456", -1, ZINT_ERROR_TOO_LONG },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = data[i].symbology;
        symbol->debug |= debug;

        int length = data[i].length;
        if (length == -1) {
            length = strlen(data[i].data);
        }

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_upce_length", test_upce_length, 1, 0, 1 },
        { "test_upca_print", test_upca_print, 1, 0, 1 },
        { "test_isbn", test_isbn, 1, 0, 1 },
        { "test_vector_same", test_vector_same, 1, 0, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_fuzz", test_fuzz, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
