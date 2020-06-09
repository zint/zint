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

#include "testcommon.h"

static void test_input(int index, int generate, int debug) {

    testStart("");

    int ret;
    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_3;
        unsigned char *data;
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
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
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
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
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
        int eci;
        int option_1;
        int option_3;
        unsigned char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "ULTRACODE_123456789!", 0, 13, 22, "AIMD/TSC15032-43 Figure G.1 **NOT SAME** different compression",
                    "7777777777777777777777"
                    "7057065353533131551057"
                    "7767053515611616136717"
                    "7037036661565555363057"
                    "7717055333616336135717"
                    "7037036515535515366057"
                    "7707070707070707070707"
                    "7067016561133113551017"
                    "7737035155311665165737"
                    "7067066561155551653057"
                    "7737033315616663515717"
                    "7017051653331136333057"
                    "7777777777777777777777"
                },
        /*  1*/ { UNICODE_MODE, 0, -1, -1, "ULTRACODE_123456789!", 0, 13, 24, "AIMD/TSC15032-43 Figure G.1 **NOT SAME** no compression; verified against bwipp",
                    "777777777777777777777777"
                    "705706533153313111101157"
                    "776703361661161666676617"
                    "703706115156555511303357"
                    "776705556561633656175517"
                    "703701311653551535501657"
                    "770707070707070707070707"
                    "706701656113311311101117"
                    "775703333531166566676537"
                    "701706651315555113303357"
                    "776705515161666351175517"
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
                    "70670156315513136160357"
                    "77370533531631615537117"
                    "70070361155313351360657"
                    "77670515613665166537117"
                    "70170651131551335150357"
                    "77670707070707070707077"
                    "70370535163551333630617"
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
                    "70670135513311133130357"
                    "77370513331166611617117"
                    "70070351153555533550557"
                    "77670613615636356367117"
                    "70170156336355515530657"
                    "77670707070707070707077"
                    "70370615133513355130117"
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
        /*  6*/ { UNICODE_MODE, 0, -1, ULTRA_COMPRESSION, "https://aimglobal.org/jcrv3tX", 0, 13, 20, "AIMD/TSC15032-43 Figure G.4a **NOT SAME** different compression; also DCC incorrect in figure",
                    "77777777777777777777"
                    "70570655115631563137"
                    "77670563356513315617"
                    "70170611665136133337"
                    "77570565116663516517"
                    "70370311355315331357"
                    "77070707070707070707"
                    "70670113111111615617"
                    "77370331553353561537"
                    "70570655311165333157"
                    "77370311136331165617"
                    "70170163363613633157"
                    "77777777777777777777"
                },
        /*  7*/ { GS1_MODE, 0, -1, -1, "[01]03453120000011[17]121125[10]ABCD1234", 0, 13, 23, "AIMD/TSC15032-43 Figure G.6 **NOT SAME** different compression and ECC; also DCC incorrect in figure",
                    "77777777777777777777777"
                    "70570616535355353310157"
                    "77670553116631616667617"
                    "70370331365353335550567"
                    "77570563516616556637657"
                    "70370656335135665360337"
                    "77070707070707070707077"
                    "70670161311353355110517"
                    "77570313153616611667137"
                    "70170635531563535550357"
                    "77370516316135616367117"
                    "70170335533356531510357"
                    "77777777777777777777777"
                },
        /*  8*/ { UNICODE_MODE, 0, -1, -1, "A", 0, 13, 13, "Verified against bwipp",
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
        /*  9*/ { UNICODE_MODE, 0, 2, -1, "12345678901234567890123", 0, 13, 25, "Length 23 == 26 MCC (C) with EC1 so 7 ECC by Table 12",
                    "7777777777777777777777777"
                    "7057063655511111111011117"
                    "7767031563666666666766667"
                    "7057063315511333113033117"
                    "7717055133656155561755567"
                    "7037011366535516355016357"
                    "7707070707070707070707077"
                    "7067013331111111111011117"
                    "7757051515366666666766637"
                    "7037016363113331133031157"
                    "7757051535651153511753517"
                    "7017035653363636636036657"
                    "7777777777777777777777777"
                },
        /* 10*/ { UNICODE_MODE, 0, 1, -1, "1", 0, 13, 11, "Figure 3a min 2-row, EC0; verified against bwipp",
                    "77777777777"
                    "70570661517"
                    "77170355667"
                    "70370666517"
                    "77670551657"
                    "70370135537"
                    "77070707077"
                    "70670151117"
                    "77170333337"
                    "70370115117"
                    "77570631357"
                    "70170365567"
                    "77777777777"
                },
        /* 11*/ { UNICODE_MODE, 0, 6, -1, "123456789012345678901", 0, 13, 27, "Figure 3a max 2-row, EC5",
                    "777777777777777777777777777"
                    "705706316551651111101111117"
                    "771703535313166666676666667"
                    "703706166556351133301133317"
                    "771705311313665615575615557"
                    "706701655165353551603551637"
                    "770707070707070707070707077"
                    "703701165561111111101111117"
                    "771705336136536666676666637"
                    "703701113655351333101333157"
                    "775703635331635115375115367"
                    "701705553563556363606363637"
                    "777777777777777777777777777"
                },
        /* 12*/ { UNICODE_MODE, 0, 6, -1, "1234567890123456789012345678901234567890123456789012", 0, 19, 36, "Figure 3b max 3-row, EC5",
                    "777777777777777777777777777777777777"
                    "700706363653513111101111111111111117"
                    "770703511165156356676666666666666667"
                    "700706155316333511303133133113313317"
                    "775705516633156156175515515361551557"
                    "701701335356661335606653331656665337"
                    "773707070707070707070707070707070707"
                    "705701313116156311101111111111111117"
                    "776706665563633166676666666666666667"
                    "700703316155566513301133133133113317"
                    "773705165316651651573615515515361557"
                    "701701556535515533106566653331656667"
                    "773707070707070707070707070707070707"
                    "706701333331113511101111111111111117"
                    "771705161113336666676666666666666657"
                    "700703313565163513301331133133133137"
                    "770705661613551651575153615515515317"
                    "700703155555336565303316566653331637"
                    "777777777777777777777777777777777777"
                },
        /* 13*/ { UNICODE_MODE, 0, 6, -1, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123", 0, 25, 49, "Figure 3c max 4-row, EC5",
                    "7777777777777777777777777777777777777777777777777"
                    "7007061565635135151011111111111111101111111111117"
                    "7707035151513316566766666666666666676666666666667"
                    "7007061536666531351031331313313133103133131331317"
                    "7707056653153315566755156551565515675515655156557"
                    "7007013366335656655013565135651356501356513565137"
                    "7707070707070707070707070707070707070707070707077"
                    "7057036636366363311011111111111111101111111111117"
                    "7737055565651635656766666666666666676666666666667"
                    "7017061653316513563031313313133131303131331313317"
                    "7757056561153165131755131551315513175513155131557"
                    "7067063135615536653036663366633666303666336663367"
                    "7707070707070707070707070707070707070707070707077"
                    "7037056156651155111011111111111111101111111111117"
                    "7717015565563561566766666666666666676666666666667"
                    "7067066631136356613031313313133131303131331313317"
                    "7737031365663161551756551565515655175655156551567"
                    "7017065156355516135065135651356513506513565135657"
                    "7707070707070707070707070707070707070707070707077"
                    "7007053555355533511011111111111111101111111111117"
                    "7707035616631351166766666666666666676666666666657"
                    "7007063363316563613013313133131331301331313313167"
                    "7707031111653311551731551315513155173155131551357"
                    "7007016565561165166063366633666336606336663366637"
                    "7777777777777777777777777777777777777777777777777"
                },
        /* 14*/ { UNICODE_MODE, 0, 6, -1, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 0, 31, 66, "Figure 3d max 5-row, EC5 **NOT SAME** Max columns due to 282 limit is 60 not 61 as shown",
                    "777777777777777777777777777777777777777777777777777777777777777777"
                    "700706655166656555306351111111111110111111111111111011111111111117"
                    "770703331611363336575566666666666667666666666666666766666666666667"
                    "700706555153116611103153333333333330333333333333333033333333333337"
                    "770705361635661566576361515151515157151515151515151751515151515157"
                    "700701513513555315303653636363636360363636363636363063636363636367"
                    "770707070707070707070707070707070707070707070707070707070707070707"
                    "700705316656355536606131111111111110111111111111111011111111111157"
                    "770706653365566155373316666666666667666666666666666766666666666617"
                    "700703361516133633605633131313131310313131313131313013131313131357"
                    "775705513161356156176361313131313137131313131313131731313131313117"
                    "706706166533165615303615656565656560565656565656565065656565656557"
                    "775707070707070707070707070707070707070707070707070707070707070707"
                    "701703651151633136506611111111111110111111111111111011111111111157"
                    "776706533613515615373566666666666667666666666666666766666666666617"
                    "700701365531653133101613131313131310313131313131313013131313131357"
                    "773703113156366615576151515151515157151515151515151751515151515117"
                    "706706351535111563305536363636363630636363636363636036363636363657"
                    "773707070707070707070707070707070707070707070707070707070707070707"
                    "705705611111515333605311111111111110111111111111111011111111111157"
                    "771703336366133651571166666666666667666666666666666766666666666617"
                    "700706513535351316103313131313131310313131313131313013131313131357"
                    "770703131151115535575555555555555557555555555555555755555555555517"
                    "700705665366536111306661616161616160161616161616161061616161616157"
                    "770707070707070707070707070707070707070707070707070707070707070707"
                    "700706355163151355603611111111111110111111111111111011111111111137"
                    "770703563615515136176566666666666667666666666666666766666666666617"
                    "700705655136333561505613131313131310313131313131313013131313131357"
                    "770701136561611613373365656565656567565656565656565765656565656517"
                    "700703663313553565506153535353535350353535353535353053535353535357"
                    "777777777777777777777777777777777777777777777777777777777777777777"
                },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        struct zint_symbol *symbol = ZBarcode_Create();
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
        symbol->debug |= debug;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].comment);
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
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
