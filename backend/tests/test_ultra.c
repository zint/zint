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

static void test_reader_init(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        int output_options;
        int option_3;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, READER_INIT, 0, "A", 0, 13, 14, "(3) 257 269 65", "8-bit FNC3 A" },
        /*  1*/ { UNICODE_MODE, READER_INIT, ULTRA_COMPRESSION, "A", 0, 13, 14, "(3) 272 271 65", "ASCII FNC3 A Note: draft spec inconsistent and FNC3 may be 272 in ASCII mode (and FNC1 271)" },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = testUtilSetSymbol(symbol, BARCODE_ULTRA, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, data[i].option_3, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options), testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
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
        int option_1;
        int option_3;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, -1, "A", 0, "(2) 257 65", "" },
        /*  1*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "A", 0, "(2) 272 65", "" },
        /*  2*/ { UNICODE_MODE, 0, -1, -1, "12", 0, "(3) 257 49 50", "" },
        /*  3*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "12", 0, "(2) 272 140", "" },
        /*  4*/ { UNICODE_MODE, 0, -1, -1, "123", 0, "(4) 257 49 50 51", "" },
        /*  5*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "123", 0, "(3) 272 140 51", "" },
        /*  6*/ { UNICODE_MODE, 0, -1, -1, "ABC", 0, "(4) 257 65 66 67", "" },
        /*  7*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ABC", 0, "(4) 272 65 66 67", "" },
        /*  8*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ULTRACODE_123456789!", 0, "(17) 272 85 76 84 82 65 67 79 68 69 95 140 162 184 206 57 33", "" },
        /*  9*/ { UNICODE_MODE, 0, -1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(250) 257 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "249 chars EC2" },
        /* 10*/ { UNICODE_MODE, 0, -1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", ZINT_ERROR_TOO_LONG, "Error 591: Data too long for selected error correction capacity", "250 chars EC2" },
        /* 11*/ { UNICODE_MODE, 0, 1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(274) 257 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "273 chars EC0" },
        /* 12*/ { UNICODE_MODE, 0, 1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", ZINT_ERROR_TOO_LONG, "Error 591: Data too long for selected error correction capacity", "274 chars EC0" },
        /* 13*/ { UNICODE_MODE, 0, -1, -1, "é", 0, "(2) 257 233", "" },
        /* 14*/ { UNICODE_MODE, 0, -1, -1, "β", ZINT_WARN_USES_ECI, "Warning (2) 263 226", "" },
        /* 15*/ { UNICODE_MODE, 9, -1, -1, "β", 0, "(2) 263 226", "" },
        /* 16*/ { UNICODE_MODE, 9, -1, -1, "βAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(250) 263 226 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "249 chars EC2" },
        /* 17*/ { UNICODE_MODE, 9, -1, ULTRA_COMPRESSION, "A", 0, "(2) 272 65", "Note ECI ignored and not outputted if ULTRA_COMPRESSION and all ASCII" },
        /* 18*/ { UNICODE_MODE, 15, -1, -1, "Ŗ", 0, "(2) 268 170", "" },
        /* 19*/ { DATA_MODE, 898, -1, -1, "\001\002\003\004\377", 0, "(7) 278 130 1 2 3 4 255", "" },
        /* 20*/ { DATA_MODE, 899, -1, -1, "\001\002\003\004\377", 0, "(6) 280 1 2 3 4 255", "" },
        /* 21*/ { DATA_MODE, 900, -1, -1, "\001\002\003\004\377", 0, "(9) 257 274 137 128 1 2 3 4 255", "" },
        /* 22*/ { DATA_MODE, 9999, -1, -1, "\001\002\003\004\377", 0, "(9) 257 274 227 227 1 2 3 4 255", "" },
        /* 23*/ { DATA_MODE, 10000, -1, -1, "\001\002\003\004\377", 0, "(10) 257 275 129 128 128 1 2 3 4 255", "" },
        /* 24*/ { DATA_MODE, 811799, -1, -1, "\001\002\003\004\377", 0, "(10) 257 275 209 145 227 1 2 3 4 255", "" },
        /* 25*/ { DATA_MODE, 811800, -1, -1, "\001\002\003\004\377", ZINT_ERROR_INVALID_OPTION, "Error 590: ECI value not supported by Ultracode", "" },
        /* 26*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "123,456,789/12,/3,4,/5//", 0, "(15) 272 140 231 173 234 206 257 140 44 262 242 44 264 47 47", "Mode: a (24)" },
        /* 27*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS", 0, "(32) 257 256 46 151 78 210 205 208 258 5 148 28 72 2 167 52 127 193 83 75 211 267 76 65 32", "Mode: cccccc88cccccccccc8888aaa8cccccc (32)" },
        /* 28*/ { UNICODE_MODE, 0, -1, -1, "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS", 0, "(33) 257 72 69 73 77 65 83 205 208 65 32 75 69 78 78 65 82 65 72 193 83 75 211 76 65 32 205", "" },
        /* 29*/ { UNICODE_MODE, 10, -1, ULTRA_COMPRESSION, "אולטרה-קוד1234", 0, "(14) 264 224 229 236 232 248 228 45 247 229 227 267 140 162", "Mode: 8888888888aaaa (14); Figure G.3" },
        /* 30*/ { UNICODE_MODE, 10, -1, -1, "אולטרה-קוד1234", 0, "(15) 264 224 229 236 232 248 228 45 247 229 227 49 50 51 52", "" },
        /* 31*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "https://aimglobal.org/jcrv3tX", 0, "(16) 282 266 1 74 41 19 6 168 270 212 59 106 144 56 265 70", "Mode: c (21); Figure G.4a" },
        /* 32*/ { UNICODE_MODE, 0, -1, -1, "https://aimglobal.org/jcrv3tX", 0, "(22) 282 97 105 109 103 108 111 98 97 108 46 111 114 103 47 106 99 114 118 51 116 88", "" },
        /* 33*/ { GS1_MODE, 0, -1, -1, "[01]03453120000011[17]121125[10]ABCD1234", 0, "(20) 273 129 131 173 159 148 128 128 139 145 140 139 153 138 65 66 67 68 140 162", "Mode: a (34); Figure G.6 uses C43 for 6 of last 7 chars (same codeword count)" },
        /* 34*/ { GS1_MODE, 0, -1, -1, "[17]120508[10]ABCD1234[410]9501101020917", 0, "(21) 273 145 140 133 136 138 65 66 67 68 140 162 272 169 137 178 139 129 130 137 145", "Mode: a (35)" },
        /* 35*/ { GS1_MODE, 0, -1, -1, "[17]120508[10]ABCDEFGHI[410]9501101020917", 0, "(24) 273 145 140 133 136 138 65 66 67 68 69 70 71 72 73 272 169 137 178 139 129 130 137 145", "Mode: a (36)" },
        /* 36*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ftp://", 0, "(4) 272 278 269 165", "Mode: c (6)" },
        /* 37*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, ".cgi", 0, "(4) 272 278 274 131", "Mode: c (4)" },
        /* 38*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ftp://a.cgi", 0, "(6) 272 280 269 123 274 131", "Mode: c (11)" },
        /* 39*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "e: file:f.shtml !", 0, "(12) 272 280 30 94 236 235 72 233 39 52 267 250", "Mode: c (17)" },
        /* 40*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "Aaatel:", 0, "(6) 272 280 262 76 6 89", "Mode: c (7)" },
        /* 41*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "Aatel:a", 0, "(6) 272 280 262 76 271 161", "Mode: c (7)" },
        /* 42*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "Atel:aAa", 0, "(8) 272 275 6 89 275 148 0 42", "Mode: c (8)" },
        /* 43*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "tel:AAaa", 0, "(8) 272 275 271 161 6 28 262 118", "Mode: c (8)" },
        /* 44*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "AAaatel:aA", 0, "(10) 272 276 0 42 0 41 118 46 6 156", "Mode: c (10)" },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = testUtilSetSymbol(symbol, BARCODE_ULTRA, data[i].input_mode, data[i].eci, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
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
        int eci;
        int option_1;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ULTRACODE_123456789!", 0, 13, 22, "AIMD/TSC15032-43 Figure G.1 **NOT SAME** different compression",
                    "7777777777777777777777"
                    "7857865353533131551857"
                    "7767853515611616136717"
                    "7837836661565555363857"
                    "7717855333616336135717"
                    "7837836515535515366857"
                    "7787878787878787878787"
                    "7867816561133113551817"
                    "7737835155311665165737"
                    "7867866561155551653857"
                    "7737833315616663515717"
                    "7817851653331136333857"
                    "7777777777777777777777"
                },
        /*  1*/ { UNICODE_MODE, 0, -1, -1, "ULTRACODE_123456789!", 0, 13, 24, "AIMD/TSC15032-43 Figure G.1 **NOT SAME** no compression; verified against bwipp",
                    "777777777777777777777777"
                    "785786533153313111181157"
                    "776783361661161666676617"
                    "783786115156555511383357"
                    "776785556561633656175517"
                    "783781311653551535581657"
                    "778787878787878787878787"
                    "786781656113311311181117"
                    "775783333531166566676537"
                    "781786651315555113383357"
                    "776785515161666351175517"
                    "781786166533113663683357"
                    "777777777777777777777777"
                },
        /*  2*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS", 0, 19, 23, "AIMD/TSC15032-43 Figure G.2 **NOT SAME** different compression",
                    "77777777777777777777777"
                    "78878663151561555158557"
                    "77878315565635366667617"
                    "78878666656561115538357"
                    "77578535365656556367117"
                    "78378153656135163558357"
                    "77178787878787878787877"
                    "78678156315513136168357"
                    "77378533531631615537117"
                    "78878361155313351368657"
                    "77678515613665166537117"
                    "78178651131551335158357"
                    "77678787878787878787877"
                    "78378535163551333638617"
                    "77178111531613611567137"
                    "78878566665531335618357"
                    "77878151331365561537137"
                    "78878333656153153368617"
                    "77777777777777777777777"
                },
        /*  3*/ { DATA_MODE, 0, -1, -1, "\110\105\111\115\101\123\315\320\101\040\113\105\116\116\101\122\101\110\301\123\113\323\114\101\040\315\123\114\101\116\104\123", 0, 19, 23, "AIMD/TSC15032-43 Figure G.2 **NOT SAME** no compression; verified against bwipp",
                    "77777777777777777777777"
                    "78878633151153313358137"
                    "77878315666661161167617"
                    "78878663155553555538557"
                    "77578531366336136167367"
                    "78378155555515653358537"
                    "77178787878787878787877"
                    "78678135513311133138357"
                    "77378513331166611617117"
                    "78878351153555533558557"
                    "77678613615636356367117"
                    "78178156336355515538657"
                    "77678787878787878787877"
                    "78378615133513355138117"
                    "77178136511651166517637"
                    "78878365635335515358557"
                    "77878613551651656517637"
                    "78878361115516163138317"
                    "77777777777777777777777"
                },
        /*  4*/ { UNICODE_MODE, 10, -1, ULTRA_COMPRESSION, "אולטרה-קוד1234", 0, 13, 19, "AIMD/TSC15032-43 Figure G.3 Same except DCC correct whereas DCC in Figure G.3 is incorrent",
                    "7777777777777777777"
                    "7857865565566616657"
                    "7737853333613351517"
                    "7867815155551565167"
                    "7757853333633356657"
                    "7837866561515535537"
                    "7787878787878787877"
                    "7867813561166666517"
                    "7737831653311131137"
                    "7817865336156555357"
                    "7767816565663636117"
                    "7817851316355311357"
                    "7777777777777777777"
                },
        /*  5*/ { DATA_MODE, 0, -1, -1, "\340\345\354\350\370\344\055\367\345\343\061\062\063\064", 0, 13, 20, "AIMD/TSC15032-43 Figure G.3 **NOT SAME** no compression; verified against bwipp",
                    "77777777777777777777"
                    "78578611115666161157"
                    "77678333656133516617"
                    "78178655165515651157"
                    "77578516516333565617"
                    "78378163335155353557"
                    "77878787878787878787"
                    "78678153311666661117"
                    "77378315553111316637"
                    "78578631611565551357"
                    "77378166136636365117"
                    "78178613653553116357"
                    "77777777777777777777"
                },
        /*  6*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "https://aimglobal.org/jcrv3tX", 0, 13, 20, "AIMD/TSC15032-43 Figure G.4a **NOT SAME** different compression; also DCC incorrect in figure",
                    "77777777777777777777"
                    "78578655115631563137"
                    "77678563356513315617"
                    "78178611665136133337"
                    "77578565116663516517"
                    "78378311355315331357"
                    "77878787878787878787"
                    "78678113111111615617"
                    "77378331553353561537"
                    "78578655311165333157"
                    "77378311136331165617"
                    "78178163363613633157"
                    "77777777777777777777"
                },
        /*  7*/ { GS1_MODE, 0, -1, -1, "[01]03453120000011[17]121125[10]ABCD1234", 0, 13, 23, "AIMD/TSC15032-43 Figure G.6 **NOT SAME** different compression and ECC; also DCC incorrect in figure",
                    "77777777777777777777777"
                    "78578616535355353318157"
                    "77678553116631616667617"
                    "78378331365353335558567"
                    "77578563516616556637657"
                    "78378656335135665368337"
                    "77878787878787878787877"
                    "78678161311353355118517"
                    "77578313153616611667137"
                    "78178635531563535558357"
                    "77378516316135616367117"
                    "78178335533356531518357"
                    "77777777777777777777777"
                },
        /*  8*/ { UNICODE_MODE, 0, -1, -1, "A", 0, 13, 13, "Verified against bwipp",
                    "7777777777777"
                    "7857863335517"
                    "7717835163667"
                    "7867861551557"
                    "7757856115637"
                    "7837813636557"
                    "7787878787877"
                    "7867811361117"
                    "7717833133337"
                    "7857811355157"
                    "7737865163617"
                    "7817833536357"
                    "7777777777777"
                },
        /*  9*/ { UNICODE_MODE, 0, 2, -1, "12345678901234567890123", 0, 13, 25, "Length 23 == 26 MCC (C) with EC1 so 7 ECC by Table 12",
                    "7777777777777777777777777"
                    "7857863655511111111811117"
                    "7767831563666666666766667"
                    "7857863315511333113833117"
                    "7717855133656155561755567"
                    "7837811366535516355816357"
                    "7787878787878787878787877"
                    "7867813331111111111811117"
                    "7757851515366666666766637"
                    "7837816363113331133831157"
                    "7757851535651153511753517"
                    "7817835653363636636836657"
                    "7777777777777777777777777"
                },
        /* 10*/ { UNICODE_MODE, 0, 1, -1, "1", 0, 13, 11, "Figure 3a min 2-row, EC0; verified against bwipp",
                    "77777777777"
                    "78578661517"
                    "77178355667"
                    "78378666517"
                    "77678551657"
                    "78378135537"
                    "77878787877"
                    "78678151117"
                    "77178333337"
                    "78378115117"
                    "77578631357"
                    "78178365567"
                    "77777777777"
                },
        /* 11*/ { UNICODE_MODE, 0, 6, -1, "123456789012345678901", 0, 13, 27, "Figure 3a max 2-row, EC5",
                    "777777777777777777777777777"
                    "785786316551651111181111117"
                    "771783535313166666676666667"
                    "783786166556351133381133317"
                    "771785311313665615575615557"
                    "786781655165353551683551637"
                    "778787878787878787878787877"
                    "783781165561111111181111117"
                    "771785336136536666676666637"
                    "783781113655351333181333157"
                    "775783635331635115375115367"
                    "781785553563556363686363637"
                    "777777777777777777777777777"
                },
        /* 12*/ { UNICODE_MODE, 0, 6, -1, "1234567890123456789012345678901234567890123456789012", 0, 19, 36, "Figure 3b max 3-row, EC5",
                    "777777777777777777777777777777777777"
                    "788786363653513111181111111111111117"
                    "778783511165156356676666666666666667"
                    "788786155316333511383133133113313317"
                    "775785516633156156175515515361551557"
                    "781781335356661335686653331656665337"
                    "773787878787878787878787878787878787"
                    "785781313116156311181111111111111117"
                    "776786665563633166676666666666666667"
                    "788783316155566513381133133133113317"
                    "773785165316651651573615515515361557"
                    "781781556535515533186566653331656667"
                    "773787878787878787878787878787878787"
                    "786781333331113511181111111111111117"
                    "771785161113336666676666666666666657"
                    "788783313565163513381331133133133137"
                    "778785661613551651575153615515515317"
                    "788783155555336565383316566653331637"
                    "777777777777777777777777777777777777"
                },
        /* 13*/ { UNICODE_MODE, 0, 6, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123", 0, 25, 49, "Figure 3c max 4-row, EC5",
                    "7777777777777777777777777777777777777777777777777"
                    "7887861565635135151811111111111111181111111111117"
                    "7787835151513316566766666666666666676666666666667"
                    "7887861536666531351831331313313133183133131331317"
                    "7787856653153315566755156551565515675515655156557"
                    "7887813366335656655813565135651356581356513565137"
                    "7787878787878787878787878787878787878787878787877"
                    "7857836636366363311811111111111111181111111111117"
                    "7737855565651635656766666666666666676666666666667"
                    "7817861653316513563831313313133131383131331313317"
                    "7757856561153165131755131551315513175513155131557"
                    "7867863135615536653836663366633666383666336663367"
                    "7787878787878787878787878787878787878787878787877"
                    "7837856156651155111811111111111111181111111111117"
                    "7717815565563561566766666666666666676666666666667"
                    "7867866631136356613831313313133131383131331313317"
                    "7737831365663161551756551565515655175655156551567"
                    "7817865156355516135865135651356513586513565135657"
                    "7787878787878787878787878787878787878787878787877"
                    "7887853555355533511811111111111111181111111111117"
                    "7787835616631351166766666666666666676666666666657"
                    "7887863363316563613813313133131331381331313313167"
                    "7787831111653311551731551315513155173155131551357"
                    "7887816565561165166863366633666336686336663366637"
                    "7777777777777777777777777777777777777777777777777"
                },
        /* 14*/ { UNICODE_MODE, 0, 6, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 0, 31, 66, "Figure 3d max 5-row, EC5 **NOT SAME** Max columns due to 282 limit is 60 not 61 as shown",
                    "777777777777777777777777777777777777777777777777777777777777777777"
                    "788786655166656555386351111111111118111111111111111811111111111117"
                    "778783331611363336575566666666666667666666666666666766666666666667"
                    "788786555153116611183153333333333338333333333333333833333333333337"
                    "778785361635661566576361515151515157151515151515151751515151515157"
                    "788781513513555315383653636363636368363636363636363863636363636367"
                    "778787878787878787878787878787878787878787878787878787878787878787"
                    "788785316656355536686131111111111118111111111111111811111111111157"
                    "778786653365566155373316666666666667666666666666666766666666666617"
                    "788783361516133633685633131313131318313131313131313813131313131357"
                    "775785513161356156176361313131313137131313131313131731313131313117"
                    "786786166533165615383615656565656568565656565656565865656565656557"
                    "775787878787878787878787878787878787878787878787878787878787878787"
                    "781783651151633136586611111111111118111111111111111811111111111157"
                    "776786533613515615373566666666666667666666666666666766666666666617"
                    "788781365531653133181613131313131318313131313131313813131313131357"
                    "773783113156366615576151515151515157151515151515151751515151515117"
                    "786786351535111563385536363636363638636363636363636836363636363657"
                    "773787878787878787878787878787878787878787878787878787878787878787"
                    "785785611111515333685311111111111118111111111111111811111111111157"
                    "771783336366133651571166666666666667666666666666666766666666666617"
                    "788786513535351316183313131313131318313131313131313813131313131357"
                    "778783131151115535575555555555555557555555555555555755555555555517"
                    "788785665366536111386661616161616168161616161616161861616161616157"
                    "778787878787878787878787878787878787878787878787878787878787878787"
                    "788786355163151355683611111111111118111111111111111811111111111137"
                    "778783563615515136176566666666666667666666666666666766666666666617"
                    "788785655136333561585613131313131318313131313131313813131313131357"
                    "778781136561611613373365656565656567565656565656565765656565656517"
                    "788783663313553565586153535353535358353535353535353853535353535357"
                    "777777777777777777777777777777777777777777777777777777777777777777"
                },
    };
    int data_size = ARRAY_SIZE(data);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        int length = testUtilSetSymbol(symbol, BARCODE_ULTRA, data[i].input_mode, data[i].eci, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
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

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_reader_init", test_reader_init, 1, 1, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
