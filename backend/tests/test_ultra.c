/*
    libzint - the open source barcode library
    Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>

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

//#define TEST_INPUT_GENERATE_EXPECTED 1
//#define TEST_ENCODE_GENERATE_EXPECTED 1

#include "testcommon.h"

static void test_input(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_3;
        unsigned char* data;
        int ret;
        char* expected;
        char* comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, -1, "A", 0, "(2) 257 65", "" },
        /*  1*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "A", 0, "(2) 272 65", "" },
        /*  2*/ { UNICODE_MODE, 0, -1, -1, "ABC", 0, "(4) 257 65 66 67", "" },
        /*  3*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ABC", 0, "(4) 272 65 66 67", "" },
        /*  4*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ULTRACODE_123456789!", 0, "(17) 272 85 76 84 82 65 67 79 68 69 95 140 162 184 206 57 33", "" },
        /*  5*/ { UNICODE_MODE, 0, -1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(250) 257 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "249 chars EC2" },
        /*  6*/ { UNICODE_MODE, 0, -1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", ZINT_ERROR_TOO_LONG, "Error 591: Data too long for selected error correction capacity", "250 chars EC2" },
        /*  7*/ { UNICODE_MODE, 0, 1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(274) 257 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "273 chars EC0" },
        /*  8*/ { UNICODE_MODE, 0, 1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", ZINT_ERROR_TOO_LONG, "Error 591: Data too long for selected error correction capacity", "274 chars EC0" },
        /*  9*/ { UNICODE_MODE, 0, -1, -1, "é", 0, "(2) 257 233", "" },
        /* 10*/ { UNICODE_MODE, 0, -1, -1, "β", ZINT_WARN_USES_ECI, "Warning (2) 263 226", "" },
        /* 11*/ { UNICODE_MODE, 9, -1, -1, "β", 0, "(2) 263 226", "" },
        /* 12*/ { UNICODE_MODE, 9, -1, -1, "βAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, "(250) 263 226 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65 65", "249 chars EC2" },
        /* 13*/ { UNICODE_MODE, 9, -1, ULTRA_COMPRESSION, "A", 0, "(2) 272 65", "Note ECI ignored and not outputted if ULTRA_COMPRESSION and all ASCII" },
        /* 14*/ { UNICODE_MODE, 15, -1, -1, "Ŗ", 0, "(2) 268 170", "" },
        /* 15*/ { GS1_MODE, 0, -1, -1, "[01]03453120000011[17]20121125[10]ABCD1234", ZINT_ERROR_INVALID_DATA, "Error 259: Invalid data length for AI (17)", "AIMD/TSC15032-43 G.6 Invalid [17] 4-digit year used in example" },
        /* 16*/ { GS1_MODE, 0, -1, -1, "[01]03453120000011[17]121125[10]ABCD1234", 0, "(20) 273 129 131 173 159 148 128 128 139 145 140 139 153 138 65 66 67 68 140 162", "" },
        /* 17*/ { GS1_MODE, 0, -1, -1, "[01]03453120000011[17]121125[10]ABCD1234", 0, "(20) 273 129 131 173 159 148 128 128 139 145 140 139 153 138 65 66 67 68 140 162", "" },
        /* 18*/ { GS1_MODE, 0, -1, -1, "[17]120508[10]ABCD1234[410]9501101020917", 0, "(21) 273 145 140 133 136 138 65 66 67 68 140 162 272 169 137 178 139 129 130 137 145", "" },
        /* 19*/ { GS1_MODE, 0, -1, -1, "[17]120508[10]ABCDEFGHI[410]9501101020917", 0, "(21) 273 145 140 133 136 138 65 66 67 68 140 162 272 169 137 178 139 129 130 137 145", "" },
        /* 20*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ftp://", 0, "(4) 272 280 269 165", "" },
        /* 21*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, ".cgi", 0, "(4) 272 280 274 131", "" },
        /* 22*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ftp://a.cgi", 0, "(6) 272 280 269 165 6 107", "" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_ULTRA;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_3 != -1) {
            symbol->option_3 = data[i].option_3;
        }
        //symbol->debug = ZINT_DEBUG_PRINT | ZINT_DEBUG_TEST;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, testUtilOption3Name(data[i].option_3), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_3;
        unsigned char* data;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ULTRACODE_123456789!", 0, 13, 22, "AIMD/TSC15032-43 Figure G.1 **NOT SAME** different compression",
                    "7777777777777777777777"
                    "7057065353533131551057"
                    "7767053515611616136717"
                    "7017036661565555363057"
                    "7767055333616336135717"
                    "7037036515535515366057"
                    "7707070707070707070707"
                    "7067016561133113551017"
                    "7737035155311665165737"
                    "7057066561155551653057"
                    "7767033315616663515717"
                    "7017051653331136333057"
                    "7777777777777777777777"
               },
        /*  1*/ { UNICODE_MODE, 0, -1, -1, "ULTRACODE_123456789!", 0, 13, 24, "AIMD/TSC15032-43 Figure G.1 **NOT SAME** no compression; verified against bwipp",
                    "777777777777777777777777"
                    "705706533153313111101157"
                    "776703361661161666676617"
                    "703706115156555511303357"
                    "775705556561633656175517"
                    "703701311653551535501657"
                    "770707070707070707070707"
                    "706701656113311311101117"
                    "775703333531166566676537"
                    "701706651315555113303357"
                    "773705515161666351175517"
                    "701706166533113663603357"
                    "777777777777777777777777"
               },
        /*  2*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "HEIMASÍÐA KENNARAHÁSKÓLA ÍSLANDS", 0, 19, 23, "AIMD/TSC15032-43 Figure G.2 **NOT SAME** different compression",
                    "77777777777777777777777"
                    "70070663151561555150557"
                    "77070315565635366667617"
                    "70070666656561115530357"
                    "77570535365656556367117"
                    "70370153656135163550357"
                    "77170707070707070707077"
                    "70570156315513136160357"
                    "77370533531631615537117"
                    "70070361155313351360657"
                    "77670515613665166537117"
                    "70170651131551335150357"
                    "77570707070707070707077"
                    "70670535163551333630617"
                    "77170111531613611567137"
                    "70070566665531335610357"
                    "77070151331365561537137"
                    "70070333656153153360617"
                    "77777777777777777777777"
               },
        /*  3*/ { DATA_MODE, 0, -1, -1, "\110\105\111\115\101\123\315\320\101\040\113\105\116\116\101\122\101\110\301\123\113\323\114\101\040\315\123\114\101\116\104\123", 0, 19, 23, "AIMD/TSC15032-43 Figure G.2 **NOT SAME** no compression; verified against bwipp",
                    "77777777777777777777777"
                    "70070633151153313350137"
                    "77070315666661161167617"
                    "70070663155553555530557"
                    "77570531366336136167367"
                    "70370155555515653350537"
                    "77170707070707070707077"
                    "70570135513311133130357"
                    "77370513331166611617117"
                    "70070351153555533550557"
                    "77670613615636356367117"
                    "70170156336355515530657"
                    "77570707070707070707077"
                    "70670615133513355130117"
                    "77170136511651166517637"
                    "70070365635335515350557"
                    "77070613551651656517637"
                    "70070361115516163130317"
                    "77777777777777777777777"
               },
        /*  4*/ { UNICODE_MODE, 10, -1, ULTRA_COMPRESSION, "אולטרה-קוד1234", 0, 13, 19, "AIMD/TSC15032-43 Figure G.3 Same except DCC correct whereas DCC in Figure G.3 is incorrent",
                    "7777777777777777777"
                    "7057065565566616657"
                    "7737053333613351517"
                    "7067015155551565167"
                    "7757053333633356657"
                    "7037066561515535537"
                    "7707070707070707077"
                    "7067013561166666517"
                    "7737031653311131137"
                    "7017065336156555357"
                    "7767016565663636117"
                    "7017051316355311357"
                    "7777777777777777777"
               },
        /*  5*/ { DATA_MODE, 0, -1, -1, "\340\345\354\350\370\344\055\367\345\343\061\062\063\064", 0, 13, 20, "AIMD/TSC15032-43 Figure G.3 **NOT SAME** no compression; verified against bwipp",
                    "77777777777777777777"
                    "70570611115666161157"
                    "77670333656133516617"
                    "70170655165515651157"
                    "77570516516333565617"
                    "70370163335155353557"
                    "77070707070707070707"
                    "70670153311666661117"
                    "77370315553111316637"
                    "70570631611565551357"
                    "77370166136636365117"
                    "70170613653553116357"
                    "77777777777777777777"
               },
        /*  6*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "https://aimglobal.org/jcrv3tX", 0, 13, 22, "AIMD/TSC15032-43 Figure G.4a **NOT SAME** different compression",
                    "7777777777777777777777"
                    "7057065133563156313037"
                    "7767051556651331566757"
                    "7017066611513613331017"
                    "7767053165666351655767"
                    "7037035651531533133057"
                    "7707070707070707070707"
                    "7067015511111161565017"
                    "7737036635335356151737"
                    "7057063111116533366057"
                    "7767031536633116511717"
                    "7017063363361363356057"
                    "7777777777777777777777"
               },
        /*  7*/ { UNICODE_MODE, 0, -1, -1, "A", 0, 13, 13, "Verified against bwipp",
                    "7777777777777"
                    "7057063335517"
                    "7717035163667"
                    "7067061551557"
                    "7757056115637"
                    "7037013636557"
                    "7707070707077"
                    "7067011361117"
                    "7717033133337"
                    "7057011355157"
                    "7737065163617"
                    "7017033536357"
                    "7777777777777"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_ULTRA;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_3 != -1) {
            symbol->option_3 = data[i].option_3;
        }
        //symbol->debug = ZINT_DEBUG_PRINT;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, %d, %d, \"%s\",\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, testUtilOption3Name(data[i].option_3),
                testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
        testUtilModulesDump(symbol, "                    ", "\n");
        printf("               },\n");
        #else
        if (ret < 5) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            if (ret == 0) {
                int width, row;
                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
            }
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main()
{
    test_input();
    test_encode();

    testReport();

    return 0;
}
