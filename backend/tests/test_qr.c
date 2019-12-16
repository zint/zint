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

//#define TEST_QR_INPUT_GENERATE_EXPECTED 1
//#define TEST_QR_GS1_GENERATE_EXPECTED 1
//#define TEST_QR_OPTIMIZE_GENERATE_EXPECTED 1
//#define TEST_QR_ENCODE_GENERATE_EXPECTED 1
//#define TEST_MICROQR_INPUT_GENERATE_EXPECTED 1
//#define TEST_MICROQR_PADDING_GENERATE_EXPECTED 1
//#define TEST_MICROQR_OPTIMIZE_GENERATE_EXPECTED 1
//#define TEST_MICROQR_ENCODE_GENERATE_EXPECTED 1
//#define TEST_UPNQR_INPUT_GENERATE_EXPECTED 1
//#define TEST_UPNQR_ENCODE_GENERATE_EXPECTED 1
//#define TEST_RMQR_INPUT_GENERATE_EXPECTED 1
//#define TEST_RMQR_GS1_GENERATE_EXPECTED 1
//#define TEST_RMQR_OPTIMIZE_GENERATE_EXPECTED 1
//#define TEST_RMQR_ENCODE_GENERATE_EXPECTED 1

static void test_qr_options(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_1;
        int option_2;
        int ret_encode;
        int ret_vector;
        int expected_size;
        int compare_previous;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "12345", -1, -1, 0, 0, 21, -1 }, // ECC auto-set to 1 (L), version auto-set to 1
        /*  1*/ { "12345", 5, -1, 0, 0, 21, 0 }, // ECC > 4 ignored
        /*  2*/ { "12345", -1, 41, 0, 0, 21, 0 }, // Version > 40 ignored
        /*  3*/ { "12345", -1, 2, 0, 0, 25, -1 }, // ECC auto-set to 4 (Q), version 2
        /*  4*/ { "12345", 4, 2, 0, 0, 25, 0 }, // ECC 4 (Q), version 2
        /*  5*/ { "12345", 1, 2, 0, 0, 25, 1 }, // ECC 1 (L), version 2
        /*  6*/ { "貫やぐ識禁", -1, -1, 0, 0, 21, -1 }, // ECC auto-set to 1 (L), version auto-set to 1
        /*  7*/ { "貫やぐ識禁", 1, -1, 0, 0, 21, 0 }, // Version auto-set to 1
        /*  8*/ { "貫やぐ識禁", -1, 1, 0, 0, 21, 0 }, // ECC auto-set to 1 (L)
        /*  9*/ { "貫やぐ識禁", 1, 1, 0, 0, 21, 0 },
        /* 10*/ { "貫やぐ識禁", 2, 1, ZINT_ERROR_TOO_LONG, -1, 0, -1 }, // ECC 2 (M), version 1
        /* 11*/ { "貫やぐ識禁", 2, -1, 0, 0, 25, -1 }, // Version auto-set to 2
        /* 12*/ { "貫やぐ識禁", 2, 2, 0, 0, 25, 0 },
        /* 13*/ { "貫やぐ識禁", 1, 2, 0, 0, 25, 1 },
        /* 14*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", -1, -1, 0, 0, 29, -1 }, // ECC auto-set to 1 (L), version auto-set to 3
        /* 15*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 1, 3, 0, 0, 29, 0 },
        /* 16*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 2, -1, 0, 0, 33, -1 }, // ECC 2 (M), version auto-set to 4
        /* 17*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 2, 4, 0, 0, 33, 0 },
        /* 18*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 3, -1, 0, 0, 37, -1 }, // ECC 3 (Q), version auto-set to 5
        /* 19*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 3, 5, 0, 0, 37, 0 },
        /* 20*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 4, -1, 0, 0, 41, -1 }, // ECC 4 (H), version auto-set to 6
        /* 21*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 4, 6, 0, 0, 41, 0 },
        /* 22*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", -1, -1, 0, 0, 69, -1 }, // ECC auto-set to 1, version auto-set to 13
        /* 23*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 1, 13, 0, 0, 69, 0 },
        /* 24*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 4, -1, 0, 0, 101, -1 }, // ECC 4, version auto-set to 21
        /* 25*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 4, 21, 0, 0, 101, 0 },
        /* 26*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", -1, -1, 0, 0, 105, -1 }, // ECC auto-set to 1, version auto-set to 22
        /* 27*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 1, 22, 0, 0, 105, 0 },
        /* 28*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 4, -1, 0, 0, 153, 1 }, // ECC 4, version auto-set 34
        /* 29*/ { "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 4, 34, 0, 0, 153, 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    struct zint_symbol previous_symbol;

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_QRCODE;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);
        if (data[i].compare_previous != -1) {
            ret = testUtilSymbolCmp(symbol, &previous_symbol);
            assert_equal(!ret, !data[i].compare_previous, "i:%d testUtilSymbolCmp !ret %d != %d\n", i, ret, data[i].compare_previous);
        }
        memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

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

static void test_qr_input(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        int eci;
        unsigned char* data;
        int ret;
        int expected_eci;
        char* expected;
        char* comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS, UTF-8 E0B881
    // Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447, UTF-8 D096
    // ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS, UTF-8 E0BA81
    // ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5
    // ･ U+FF65 half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xA5 (\245), UTF-8 EFBDA5
    // ¿ U+00BF in ISO 8859-1 0xBF (\277), not in Shift JIS; 0xBF same codepoint as single-byte half-width katakana ｿ (U+FF7F) in Shift JIS (below), UTF-8 C2BF
    // ｿ U+FF7F half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xBF (\277), UTF-8 EFBDBF
    // ‾ U+203E overline, not in ISO/Win, in Shift JIS single-byte 0x7E (\176) (tilde), UTF-8 E280BE
    // 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9
    // 茗 U+8317 kanji, in Shift JIS 0xE4AA (\344\252), UTF-8 E88C97
    // テ U+30C6 katakana, in Shift JIS 0x8365 (\203\145), UTF-8 E38386
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, "é", 0, 0, "40 1E 90 EC 11 EC 11 EC 11", "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, "é", 0, 3, "70 34 01 E9 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 20, "é", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "é not in Shift JIS" },
        /*  3*/ { UNICODE_MODE, 26, "é", 0, 26, "71 A4 02 C3 A9 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /*  4*/ { DATA_MODE, 0, "é", 0, 0, "40 2C 3A 90 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /*  5*/ { DATA_MODE, 0, "\351", 0, 0, "40 1E 90 EC 11 EC 11 EC 11", "B1 (ISO 8859-1)" },
        /*  6*/ { UNICODE_MODE, 0, "β", 0, 0, "80 11 00 00 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /*  7*/ { UNICODE_MODE, 9, "β", 0, 9, "70 94 01 E2 00 EC 11 EC 11", "ECI-9 B1 (ISO 8859-7)" },
        /*  8*/ { UNICODE_MODE, 20, "β", 0, 20, "71 48 01 10 00 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /*  9*/ { UNICODE_MODE, 26, "β", 0, 26, "71 A4 02 CE B2 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 10*/ { DATA_MODE, 0, "β", 0, 0, "40 2C EB 20 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 11*/ { UNICODE_MODE, 0, "ก", ZINT_WARN_USES_ECI, 13, "Warning 70 D4 01 A1 00 EC 11 EC 11", "ECI-13 B1 (ISO 8859-11)" },
        /* 12*/ { UNICODE_MODE, 13, "ก", 0, 13, "70 D4 01 A1 00 EC 11 EC 11", "ECI-13 B1 (ISO 8859-11)" },
        /* 13*/ { UNICODE_MODE, 20, "ก", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "ก not in Shift JIS" },
        /* 14*/ { UNICODE_MODE, 26, "ก", 0, 26, "71 A4 03 E0 B8 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 15*/ { DATA_MODE, 0, "ก", 0, 0, "40 3E 0B 88 10 EC 11 EC 11", "B3 (UTF-8)" },
        /* 16*/ { UNICODE_MODE, 0, "Ж", 0, 0, "80 11 23 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 17*/ { UNICODE_MODE, 7, "Ж", 0, 7, "70 74 01 B6 00 EC 11 EC 11", "ECI-7 B1 (ISO 8859-5)" },
        /* 18*/ { UNICODE_MODE, 20, "Ж", 0, 20, "71 48 01 12 38 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 19*/ { UNICODE_MODE, 26, "Ж", 0, 26, "71 A4 02 D0 96 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 20*/ { DATA_MODE, 0, "Ж", 0, 0, "40 2D 09 60 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 21*/ { UNICODE_MODE, 0, "ກ", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E0 BA 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 22*/ { UNICODE_MODE, 20, "ກ", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "ກ not in Shift JIS" },
        /* 23*/ { UNICODE_MODE, 26, "ກ", 0, 26, "71 A4 03 E0 BA 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 24*/ { DATA_MODE, 0, "ກ", 0, 0, "40 3E 0B A8 10 EC 11 EC 11", "B3 (UTF-8)" },
        /* 25*/ { UNICODE_MODE, 0, "\\", 0, 0, "40 15 C0 EC 11 EC 11 EC 11", "B1 (ASCII)" },
        /* 26*/ { UNICODE_MODE, 20, "\\", 0, 20, "71 48 01 00 F8 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 27*/ { UNICODE_MODE, 20, "[", 0, 20, "71 44 01 5B 00 EC 11 EC 11", "B1 (ASCII)" },
        /* 28*/ { UNICODE_MODE, 20, "\177", 0, 20, "71 44 01 7F 00 EC 11 EC 11", "ECI-20 B1 (ASCII)" },
        /* 29*/ { UNICODE_MODE, 0, "¥", 0, 0, "40 1A 50 EC 11 EC 11 EC 11", "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous)" },
        /* 30*/ { UNICODE_MODE, 3, "¥", 0, 3, "70 34 01 A5 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /* 31*/ { UNICODE_MODE, 20, "¥", 0, 20, "71 44 01 5C 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) (to single-byte backslash codepoint 5C, so byte mode)" },
        /* 32*/ { UNICODE_MODE, 26, "¥", 0, 26, "71 A4 02 C2 A5 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 33*/ { DATA_MODE, 0, "¥", 0, 0, "40 2C 2A 50 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 34*/ { UNICODE_MODE, 0, "･", 0, 0, "40 1A 50 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /* 35*/ { UNICODE_MODE, 3, "･", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 36*/ { UNICODE_MODE, 20, "･", 0, 20, "71 44 01 A5 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) single-byte codepoint A5" },
        /* 37*/ { UNICODE_MODE, 26, "･", 0, 26, "71 A4 03 EF BD A5 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 38*/ { DATA_MODE, 0, "･", 0, 0, "40 3E FB DA 50 EC 11 EC 11", "B3 (UTF-8)" },
        /* 39*/ { UNICODE_MODE, 0, "¿", 0, 0, "40 1B F0 EC 11 EC 11 EC 11", "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous)" },
        /* 40*/ { UNICODE_MODE, 3, "¿", 0, 3, "70 34 01 BF 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /* 41*/ { UNICODE_MODE, 20, "¿", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "¿ not in Shift JIS" },
        /* 42*/ { UNICODE_MODE, 26, "¿", 0, 26, "71 A4 02 C2 BF 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 43*/ { DATA_MODE, 0, "¿", 0, 0, "40 2C 2B F0 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 44*/ { UNICODE_MODE, 0, "ｿ", 0, 0, "40 1B F0 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint BF (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 45*/ { UNICODE_MODE, 3, "ｿ", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 46*/ { UNICODE_MODE, 20, "ｿ", 0, 20, "71 44 01 BF 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) single-byte codepoint BF" },
        /* 47*/ { UNICODE_MODE, 26, "ｿ", 0, 26, "71 A4 03 EF BD BF 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 48*/ { DATA_MODE, 0, "ｿ", 0, 0, "40 3E FB DB F0 EC 11 EC 11", "B3 (UTF-8)" },
        /* 49*/ { UNICODE_MODE, 0, "~", 0, 0, "40 17 E0 EC 11 EC 11 EC 11", "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 50*/ { UNICODE_MODE, 3, "~", 0, 3, "70 34 01 7E 00 EC 11 EC 11", "ECI-3 B1 (ASCII)" },
        /* 51*/ { UNICODE_MODE, 20, "~", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "tilde not in Shift JIS (codepoint used for overline)" },
        /* 52*/ { UNICODE_MODE, 0, "‾", 0, 0, "40 17 E0 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint 7E (same bytes as ~ ASCII above, so ambiguous)" },
        /* 53*/ { UNICODE_MODE, 3, "‾", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 54*/ { UNICODE_MODE, 20, "‾", 0, 20, "71 44 01 7E 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) (to single-byte tilde codepoint 7E, so byte mode)" },
        /* 55*/ { UNICODE_MODE, 26, "‾", 0, 26, "71 A4 03 E2 80 BE 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 56*/ { DATA_MODE, 0, "‾", 0, 0, "40 3E 28 0B E0 EC 11 EC 11", "B3 (UTF-8)" },
        /* 57*/ { UNICODE_MODE, 0, "点", 0, 0, "80 16 CF 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 58*/ { UNICODE_MODE, 3, "点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 59*/ { UNICODE_MODE, 20, "点", 0, 20, "71 48 01 6C F8 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 60*/ { UNICODE_MODE, 26, "点", 0, 26, "71 A4 03 E7 82 B9 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 61*/ { DATA_MODE, 0, "点", 0, 0, "40 3E 78 2B 90 EC 11 EC 11", "B3 (UTF-8)" },
        /* 62*/ { DATA_MODE, 0, "\223\137", 0, 0, "80 16 CF 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 63*/ { UNICODE_MODE, 0, "¥･点", 0, 0, "40 45 CA 59 35 F0 EC 11 EC", "B4 (Shift JIS) (optimized to byte mode only)" },
        /* 64*/ { UNICODE_MODE, 3, "¥･点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid characters in input data", "" },
        /* 65*/ { UNICODE_MODE, 20, "¥･点", 0, 20, "71 44 04 5C A5 93 5F 00 EC", "ECI-20 B4 (Shift JIS)" },
        /* 66*/ { UNICODE_MODE, 26, "¥･点", 0, 26, "71 A4 08 C2 A5 EF BD A5 E7 82 B9 00 EC", "ECI-26 B8 (UTF-8)" },
        /* 67*/ { DATA_MODE, 0, "\134\245\223\137", 0, 0, "40 45 CA 59 35 F0 EC 11 EC", "B8 (Shift JIS)" },
        /* 68*/ { DATA_MODE, 0, "¥･点", 0, 0, "40 8C 2A 5E FB DA 5E 78 2B 90 EC 11 EC", "B8 (UTF-8)" },
        /* 69*/ { UNICODE_MODE, 0, "点茗", 0, 0, "80 26 CF EA A8 00 EC 11 EC", "K2 (Shift JIS)" },
        /* 70*/ { UNICODE_MODE, 0, "点茗テ", 0, 0, "80 36 CF EA A8 34 A0 EC 11", "K3 (Shift JIS)" },
        /* 71*/ { UNICODE_MODE, 0, "点茗テ点", 0, 0, "80 46 CF EA A8 34 AD 9F 00", "K4 (Shift JIS)" },
        /* 72*/ { UNICODE_MODE, 0, "点茗テ点茗", 0, 0, "80 56 CF EA A8 34 AD 9F D5 50 00 EC 11", "K5 (Shift JIS)" },
        /* 73*/ { UNICODE_MODE, 0, "点茗テ点茗テ", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 40 EC", "K6 (Shift JIS)" },
        /* 74*/ { UNICODE_MODE, 0, "点茗テ点茗テｿ", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 50 06 FC 00 EC", "K6 B1 (Shift JIS)" },
        /* 75*/ { DATA_MODE, 0, "\223\137\344\252\203\145\223\137\344\252\203\145\277", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 50 06 FC 00 EC", "K6 B1 (Shift JIS)" },
        /* 76*/ { DATA_MODE, 0, "点茗テ点茗テｿ", 0, 0, "41 5E 78 2B 9E 88 C9 7E 38 38 6E 78 2B 9E 88 C9 7E 38 38 6E FB DB F0 EC 11 EC 11 EC", "B21 (UTF-8)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_QRCODE;
        symbol->input_mode = data[i].input_mode;
        symbol->eci = data[i].eci;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_QR_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, %d, \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                ret < 5 ? symbol->eci : -1, symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {

            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_gs1(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int ret;
        char* expected;
        char* comment;
    };
    struct item data[] = {
        /*  0*/ { "[01]12345678901234", 0, "51 04 00 B3 AA 37 DE 87 B4", "N16" },
        /*  1*/ { "[01]04912345123459[15]970331[30]128[10]ABC123", 0, "51 07 40 A7 AC EA 80 15 9E 4F CA 52 D2 D3 84 09 D5 E0 28 FD 82 F0 C0 EC 11 EC 11 EC", "N29 A9" },
        /*  2*/ { "[91]12%[20]12", 0, "52 05 99 60 5F B5 35 80 01 08 00 EC 11", "A10(11)" },
        /*  3*/ { "[91]123%[20]12", 0, "52 06 19 60 5E 2B 76 A0 5A 05 E0 EC 11", "A11(12)" },
        /*  4*/ { "[91]1234%[20]12", 0, "52 06 99 60 5E 22 F6 A6 B0 00 21 00 EC", "A12(13)" },
        /*  5*/ { "[91]12345%[20]12", 0, "51 01 F8 F3 A9 48 0F B5 35 80 01 08 00", "N7 A6(7) (same bit count as A13(14))" },
        /*  6*/ { "[91]%%[20]12", 0, "52 05 99 6D A9 B5 35 80 01 08 00 EC 11", "A9(11)" },
        /*  7*/ { "[91]%%%[20]12", 0, "52 06 99 6D A9 B5 36 A6 B0 00 21 00 EC", "A10(13)" },
        /*  8*/ { "[91]A%%%%1234567890123AA%", 0, "52 05 99 63 D1 B5 36 A6 D4 98 40 D1 ED C8 C5 40 C3 20 21 CC DA 80", "A7(11) N13 A3(4)" },
        /*  9*/ { "[91]%23%%6789%%%34567%%%%234%%%%%", 0, "(34) 52 17 19 6D A8 17 76 A6 D4 22 A5 C7 6A 6D 4D A8 22 C7 39 61 DA 9B 53 6A 6B 01 17 B5", "A31(46)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_QRCODE;
        symbol->input_mode = GS1_MODE;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_QR_GS1_GENERATE_EXPECTED
        printf("        /*%3d*/ { \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {

            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_optimize(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int option_1;
        int ret;
        char* expected;
        char* comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "1", -1, 0, "10 04 40 EC 11 EC 11 EC 11", "N1" },
        /*  1*/ { UNICODE_MODE, "AAA", -1, 0, "20 19 CC 28 00 EC 11 EC 11", "A3" },
        /*  2*/ { UNICODE_MODE, "0123456789", -1, 0, "10 28 0C 56 6A 69 00 EC 11", " N10 (nayuki.io - pure numeric)" },
        /*  3*/ { UNICODE_MODE, "ABCDEF", -1, 0, "20 31 CD 45 2A 14 00 EC 11", "A6 (nayuki.io - pure alphanumeric)" },
        /*  4*/ { UNICODE_MODE, "wxyz", -1, 0, "40 47 77 87 97 A0 EC 11 EC", "B4 (nayuki.io - pure byte)" },
        /*  5*/ { UNICODE_MODE, "「魔法少女まどか☆マギカ」って、　ИАИ　ｄｅｓｕ　κα？", 1, 0, "(55) 81 D0 1A C0 09 F8 0A ED 56 B8 57 02 8E 12 90 2C 86 F4 31 A1 8A 01 B0 50 42 88 00 10", "K29 (nayuki.io - pure kanji)" },
        /*  6*/ { UNICODE_MODE, "012345A", -1, 0, "20 38 01 0B A2 E4 A0 EC 11", "A7 (nayuki.io - alpha/numeric)" },
        /*  7*/ { UNICODE_MODE, "0123456A", -1, 0, "10 1C 0C 56 58 80 25 00 EC", "N7 A1 (nayuki.io - alpha/numeric) (note same bits as A8)" },
        /*  8*/ { UNICODE_MODE, "012a", -1, 0, "40 43 03 13 26 10 EC 11 EC", "B4 (nayuki.io - numeric/byte)" },
        /*  9*/ { UNICODE_MODE, "0123a", -1, 0, "10 10 0C 34 01 61 00 EC 11", "N4 B1 (nayuki.io - numeric/byte)" },
        /* 10*/ { UNICODE_MODE, "ABCDEa", -1, 0, "40 64 14 24 34 44 56 10 EC", "B6 (nayuki.io - alphanumeric/byte)" },
        /* 11*/ { UNICODE_MODE, "ABCDEFa", -1, 0, "20 31 CD 45 2A 15 00 58 40", "A6 B1 (nayuki.io - alphanumeric/byte)" },
        /* 12*/ { UNICODE_MODE, "THE SQUARE ROOT OF 2 IS 1.41421356237309504880168872420969807856967187537694807317667973799", 1, 0, "(55) 20 D5 2A 53 54 1A A8 4C DC DF 14 29 EC 47 CA D9 9A 88 05 71 10 59 E3 56 32 5D 45 F0", " A26 N65 (nayuki.io - alpha/numeric)" },
        /* 13*/ { UNICODE_MODE, "Golden ratio φ = 1.6180339887498948482045868343656381177203091798057628621354486227052604628189024497072072041893911374......", 1, 0, "(80) 41 44 76 F6 C6 46 56 E2 07 26 17 46 96 F2 08 3D 32 03 D2 03 12 E1 19 26 A0 87 DC BB", "B20 N100 A6 (nayuki.io - alpha/numeric/byte)" },
        /* 14*/ { UNICODE_MODE, "こんにちwa、世界！ αβγδ", 1, 0, "(34) 41 B8 2B 18 2F 18 2C 98 2B F7 76 18 14 19 0A 28 A4 58 14 92 08 3B F8 3C 08 3C 18 3C", "B27 (nayuki.io - kanji/european **NOT SAME** K4 B2 K4 A1 K4, less bits as nayuki (1.5.0) miscounting byte-mode input as UTF-8)" },
        /* 15*/ { UNICODE_MODE, "こんにちテwa、世界！ αβγδ", 1, 0, "(34) 80 50 98 85 C4 29 21 3F 0D 2A 09 BB B0 C0 A0 C8 51 45 22 C0 A4 90 41 DF C1 E0 41 E0", "K5 B19 (nayuki.io - kanji/european + extra leading kanji **NOT SAME** K5 B2 K4 A1 K4, same reason as above)" },
        /* 16*/ { UNICODE_MODE, "67128177921547861663com.acme35584af52fa3-88d0-093b-6c14-b37ddafb59c528908608sg.com.dash.www0530329356521790265903SG.COM.NETS46968696003522G33250183309051017567088693441243693268766948304B2AE13344004SG.SGQR209710339366720B439682.63667470805057501195235502733744600368027857918629797829126902859SG8236HELLO FOO2517Singapore3272B815", 1, 0, "(232) 10 52 9F 46 70 B3 5D DE 9A 1F A1 7B 1B 7B 69 73 0B 1B 6B 29 99 A9 A9 C1 A3 0B 31 A9", "N20 B47 N9 B15 N22 A11 N14 A1 N47 A19 N15 A8 N65 A20 B8 A8 (nayuki.io - SGQR alpha/numeric/byte)" },
        /* 17*/ { UNICODE_MODE, "纪", -1, ZINT_WARN_USES_ECI, "Warning 71 A4 03 E7 BA AA 00 EC 11", "ECI-26 B3 (UTF-8 E7BAAA, U+7EAA, not in Shift JIS)" },
        /* 18*/ { DATA_MODE, "纪", -1, 0, "40 3E 7B AA A0 EC 11 EC 11", "B3 (UTF-8 or Shift JIS, note ambiguous as 0xE7BA 0xAA happens to be valid Shift JIS 郤ｪ as well)" },
        /* 19*/ { UNICODE_MODE, "郤ｪ", -1, 0, "40 3E 7B AA A0 EC 11 EC 11", "B3 (Shift JIS or UTF-8 E7BAAA 纪, see above)" },
        /* 20*/ { UNICODE_MODE, "2004年大西洋颶風季是有纪录以来造成人员伤亡和财产损失最为惨重的大西洋飓风季之一，于2004年6月1日正式开始，同年11月30日结束，传统上这样的日期界定了一年中绝大多数热带气旋在大西洋形成的时间段lll ku", 1, ZINT_WARN_USES_ECI, "Warning (324) 71 A1 00 43 21 10 04 4B 96 E6 D3 96 92 9F A2 96 FF 9A D2 2F A6 8A DB A6 8A A3 96 B6", "ECI-26 N4 B274 (nayuki.io - kanji/byte/numeric **NOT SAME* N4 K9 B6 K5 etc mixing Shift JIS and UTF-8)" },
        /* 21*/ { UNICODE_MODE, "AB123456A", -1, 0, "20 49 CD 05 E2 2C 73 94 00", "A9" },
        /* 22*/ { UNICODE_MODE, "AB1234567890A", -1, 0, "20 69 CD 05 E2 2C 73 94 33 2A 50 00 EC", "A13" },
        /* 23*/ { UNICODE_MODE, "AB123456789012A", -1, 0, "20 79 CD 05 E2 2C 73 94 33 2A 0B CA 00", "A15" },
        /* 24*/ { UNICODE_MODE, "AB1234567890123A", -1, 0, "20 11 CD 10 34 7B 72 31 50 30 C8 02 50", "A2 N13 A1" },
        /* 25*/ { UNICODE_MODE, "テaABCD1", -1, 0, "40 88 36 56 14 14 24 34 43 10 EC 11 EC", "B8" },
        /* 26*/ { UNICODE_MODE, "テaABCDE1", -1, 0, "40 38 36 56 12 03 1C D4 52 9D C0 EC 11", "B3 A6" },
        /* 27*/ { UNICODE_MODE, "テéaABCDE1", -1, ZINT_WARN_USES_ECI, "Warning 71 A4 06 E3 83 86 C3 A9 61 20 31 CD 45 29 DC 00", "B6 A6" },
        /* 28*/ { UNICODE_MODE, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", -1, 0, "(44) 80 83 A8 85 88 25 CA 2F 40 B0 53 C2 44 98 41 00 4A 02 0E A8 F8 F5 0D 30 4C 35 A1 CC", "K8 N1 K8 B3" },
        /* 29*/ { UNICODE_MODE, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", -1, 0, "(589) 80 20 EA 21 62 09 72 8B D0 2C 14 F0 91 26 10 40 04 A0 08 3A A3 E3 D4 34 C1 30 D6 87", "K8 N1 K8 N1 K10 N2 K33 N2 K16 N1 K89 N2 K14 B5 K28 N2 K40 N1 K65" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_QRCODE;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        symbol->debug = ZINT_DEBUG_TEST;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_QR_OPTIMIZE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %d, %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].option_1,
                testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        #else
        assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "01234567", 2, 1, 0, 21, 21, "ISO 18004 Annex I I.2 **NOT SAME** uses mask 000 not 010; if force to use 010, same",
                    "111111100011101111111"
                    "100000101110001000001"
                    "101110100110001011101"
                    "101110100101101011101"
                    "101110101101101011101"
                    "100000100001001000001"
                    "111111101010101111111"
                    "000000000000000000000"
                    "101010100010100010010"
                    "110100001011010100010"
                    "000110111011011101110"
                    "110011010101110110010"
                    "001001110111011100001"
                    "000000001010001000010"
                    "111111100000100010001"
                    "100000100010001001011"
                    "101110101110101011101"
                    "101110100101010101110"
                    "101110101101011100101"
                    "100000100001110111000"
                    "111111101001011100101"
               },
        /*  1*/ { UNICODE_MODE, "12345678901234567890123456789012345678901", -1, -1, 0, 21, 21, "Max capacity ECC 1 Version 1 41 numbers",
                    "111111100011001111111"
                    "100000100001001000001"
                    "101110101110001011101"
                    "101110100001001011101"
                    "101110100011001011101"
                    "100000100100001000001"
                    "111111101010101111111"
                    "000000001100100000000"
                    "111011111000011000100"
                    "111000000110111000001"
                    "001001111011110001000"
                    "001110011011011111001"
                    "001100101000001000011"
                    "000000001101100001010"
                    "111111101011011001011"
                    "100000101100010001011"
                    "101110101101010111100"
                    "101110100000111000010"
                    "101110101001100011101"
                    "100000101010111011000"
                    "111111101000100010101"
               },
        /*  2*/ { UNICODE_MODE, "12345678901234567890123456789012345678901", 2, -1, 0, 25, 25, "ECC 2 auto-sets version 2",
                    "1111111010111111001111111"
                    "1000001001001110001000001"
                    "1011101001000001101011101"
                    "1011101010010001001011101"
                    "1011101010011010101011101"
                    "1000001010110000101000001"
                    "1111111010101010101111111"
                    "0000000010001101100000000"
                    "1000101111110100111111001"
                    "1001010011110111010110000"
                    "1100011011001001111100010"
                    "1000100101101010111101101"
                    "1001011100010110100111011"
                    "1111010110101001100101010"
                    "0001101010011111010110111"
                    "0001010000110100101011101"
                    "1100111100101101111110011"
                    "0000000011000111100010001"
                    "1111111010100000101010000"
                    "1000001001001010100011111"
                    "1011101011010110111111000"
                    "1011101001101000011011001"
                    "1011101001111111001010110"
                    "1000001001110101001011000"
                    "1111111011101101111101001"
               },
        /*  3*/ { UNICODE_MODE, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", 4, 10, 0, 57, 57, "Max capacity ECC 4 Version 10 74 kanji",
                    "111111101100111011000010011000100111001001000011001111111"
                    "100000101111111001101100110100110101110011011101001000001"
                    "101110101011001110000011101100101110111011110011001011101"
                    "101110100011001100100010101101000000100111001001001011101"
                    "101110100010101101010001101111101111111001010101001011101"
                    "100000101111100000011000101000111010111011111110001000001"
                    "111111101010101010101010101010101010101010101010101111111"
                    "000000001101101001101010111000101011001011001011100000000"
                    "001110101001110001011101101111101000011110010001111100111"
                    "111000010100110111100000110100100100100011011111001010111"
                    "010101110001111101010100011111010101011011111010011011010"
                    "001100001100010011000100001010100000010011111001011011001"
                    "010011110000011110101000101010110010011010001111110010010"
                    "100111011011110001111001011000011100011001010001001001000"
                    "101000110100110100001111010110101111110110111011011111011"
                    "110010000010100100010011000001111100011011111101111101101"
                    "010110101010000000001010001010110110001100001011010010010"
                    "000100010110111010111001111111000100111011111011001110010"
                    "010101110011111000011101111110000001110111011110110001011"
                    "111011001011101111000111000010000011010110111011011011101"
                    "110100111001000100101110001110000110001001011011100100011"
                    "010000001001000100111101101010111010111011010000011011010"
                    "001010101100100110101110001110011101100110001111111110001"
                    "110000000010010101100110110001100001010001011111010111001"
                    "110111100101101010101111111110110010001010100011011001100"
                    "011000011000011010000010011010000110010011011000011010011"
                    "000011111001011000101110011111111001011001001111111111000"
                    "010110001010110111111010101000101101000111001110100010111"
                    "101110101000111000001101111010100010011001010011101011001"
                    "100110001011111111011001011000111100011011111101100010011"
                    "000111111010000101100111001111110111101010001011111111111"
                    "111010011010000011111011100111010110011110001111110111000"
                    "001011110011010001011000011101011000011011010001011111011"
                    "010001000000011001100100110111111010110011111011001001010"
                    "100100101101111101000110100000110000101100011011011011001"
                    "101000000010101010110110100000100100101010001011010010010"
                    "111011101011000000100001011101000111011001011010111111011"
                    "010111001001001010011100011010011101010110111111011011011"
                    "011001110100111111110110001110110110001001000010001011101"
                    "000100001111101001101110110100101011111100010101011100010"
                    "111101100111011000111001110100100110001011101011010011011"
                    "011100000010100110111111110001110111011111010010101001011"
                    "011100101000100110110101100111010010110001011111011111101"
                    "110000010000001100000001000111010111010001011011100000011"
                    "010001110100100101000001111101001111011011010000010101111"
                    "111110010100001100001111110010110111011110001111101011011"
                    "101001111101101110110110011000100011010011010011110110111"
                    "111110001011001011010011001100110100001001000011011001010"
                    "000000101000111000011000101111100011010011111000111111001"
                    "000000001100010001110100111000110000011010001111100011111"
                    "111111100011101101000011011010110100011001010001101011000"
                    "100000100001001011111100001000111010110011010010100011011"
                    "101110101110111101110110011111101000110011111101111111101"
                    "101110101110110100011011111000011110000100001010000000010"
                    "101110101101001111001011110010011001011011111011111011100"
                    "100000100010000001101111010001101010110110010001011111000"
                    "111111100000011111000100011011010011011001011011001011001"
               },
        /*  4*/ { UNICODE_MODE, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", 4, 27, 0, 125, 125, "Max capacity ECC 4 Version 27 385 kanji",
                    "11111110101001001100111100100011110001010011110000001100010110100011101010111000011101101001011111001111101101101001101111111"
                    "10000010110001101110011001101111000101001011011001100110101000101010011110000000101000100101101110110000011110100110001000001"
                    "10111010100000000100000101000101111001011001010100100100100000000101100011010001100111101010010101101101101101101101001011101"
                    "10111010001100111011000110111100100000010101010111010010101000111001000001110000001100011101100001111100110000011000001011101"
                    "10111010010000100010110110100010111111110011001000011100100111111101110110001011001001101111111101100101001101101011001011101"
                    "10000010100100110000111100010111100010101111100000011100011010001100011100110110111111101000100111001111101001111111001000001"
                    "11111110101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101111111"
                    "00000000111011101111001100100010100010100111111111100111011010001111000110000111001000111000101100110000011110100101100000000"
                    "00111010111100111100001111100100111110110101011011110011101011111110110110010100010111101111110101001101101101101101111100111"
                    "01010001001110001010011100111101111111010010000011001110010000110110000010011011001100000110101111111100110000011111010100100"
                    "01001011010111110101110001011100000100110001011100000001111001011100111000001001001101101001101111101111101111100000101111111"
                    "01001101011110100110000011010101111011000110001011001100001100100000100101101010011111111101111101001001101001111111101001000"
                    "11101011010000001011000000010000011011101110000010110000011111101011011101111011001000100100001100110000011110100000011110101"
                    "10100100011000101010111100001001010011100101101000100110010110101111000100000000110111111101101111001101101101101101101011101"
                    "00101111000111111110001110110110101011110111111111000001011010111111010010110101001100000110100001111110110100001110101100100"
                    "10100100100111101010111001110101110111001100100010100010011001000001000111011110001101101001111111101001101111100000100001111"
                    "00100111011001010100000010011101001110110001001011110011111000000100010001011001111111001111111101001001100001011101011111000"
                    "01001001000110000001110100000101100100100000100010111010010000110001101010000100001000100100001100110100011110100001111011101"
                    "01000011110000011000110011111010110110101101000111111101110100010011100000011011110111111101100101001011101011101100101101101"
                    "11011101001001011001010101110001000010111111010000011000001110110000101100100111001100000110111001111100110100001110100000100"
                    "00101111011110000000010110100100111000010000110000110101000000110100011100010110100101101001111111101001101001100101011111111"
                    "10000000100011100111101111111010111011011000011100011001111100011110010001101011011111001111101101001111100111011101111100000"
                    "01000010110100011000111111001011101101100101001011111100100001100101111110101011001000100101010010100101011110100001101011101"
                    "10010001110100100101110011111111111110011000101101000001000101001100011010001111010111111101101101001101101101101100110101101"
                    "01000110111000011111001000100000101100101111110010001110001011010010010111111001101101100110111000111100101001101011101000100"
                    "01110001111111110101100001111010001100000010110000100011001101001101100101001111100101101001100111101101101101000101111101111"
                    "10100010000110110010001011000001011101000011100001110110110001000011110110110000111101001110101101011000100110111100000100000"
                    "00001100000110110010011101000010001100000100001010001000011011010001110010011010101000100101001010100101011101100001101011101"
                    "10101110100000110101111000101010111101000000000011011000110000000110100001100111001111011101101101101101100101101101001101101"
                    "11011000111001110111001111011011001101110101001101110101011011111001010100101010001101100110101000111110101111001010101000101"
                    "10010111001010111101100110101011001000000000101011000110101101011001100001110111000011101000111101100001001100101101001101111"
                    "11111101101011111000100001100001111101000111101010101001010100010011100011111110011101001111001101011100100101011100011100000"
                    "01001111110111111101000000101100111111001010100110110011001111111101001011101111000000101111101011100101011011100001111111101"
                    "11101000110000111010100001011011100011000000110010011000011010001011111110010111001111001000100101101111100100001101100011010"
                    "11001010100101001110110100001100101010011111001000110110001110101110111011011111001101111010111000101001101111110010101010101"
                    "00111000101010110000101110101101100010101111100010101100111010001110000100110110000011101000101101111111001001101101100011111"
                    "11101111101111111100110011010001111111011010110000111001100011111110110011010001111100101111101100000100100111011111111110000"
                    "01111001001001110011100110110000001100111110101100001001101101010111101110100001100001011001100011101011011101100000101011101"
                    "11000011011111110000011000001011001000110001111001011000011001100000111110100101001101100101111101101101100101101100101101010"
                    "01111000001110010011000110011001101101001011001101000000111111101101011110011001001101111110100000111101101001010011000000101"
                    "01110111000001111000011001110000101001000011111011011100011010011111001010110100000011100001101101101111001111101101111101111"
                    "10010101101010110110000000010100101100001111010100011001010010010100001001101101111101101101110100001100100100111111101101101"
                    "01010010100001110011000010000101001100111111011011100011000111000000100000101111000000111000001111101101011101101000101111101"
                    "11001100111011000101111100100101100000010000100011010010100111110001110100010101001101100101101101101111100001001101100101010"
                    "10011011100101110010001000111011001100000010011101100010101111010001001100010011101101111111011000100101111101010101111000101"
                    "00101101101001000011101111001011010001110101111000001010111011111011011010110011000011000100101101111001001100101100100101011"
                    "11111010100000100111000001010111111000100101001001001010011110101101011010110111011100101101010000001100100101111111101001101"
                    "01000001001110011000110000001010110111100011010010110010110101111101110101111001000001101000001111110111011100001001010111101"
                    "10100010011011000110000101000101100010100000000000110111100101000101100101110010111101100101100101001111101111001110101101010"
                    "10101100001100010010001000111101101111000111100010000010111111000000101010001111001101011010111000111001111000110101101000101"
                    "01011111100111101101111111010110000101100010101111011111111111000001101101000110000110100101001111111101101101001100000111011"
                    "10110001100010010001011111110000000011011010000111001010111100001010000010110110011101001101010000000010100011111111101001101"
                    "11100011100101110101110111010010010100010000010110010101100101001100011011111010001001101001101111101111001100001000101111101"
                    "01010100001001111100100100101000010011011111100010001010110010000001110001011101011100000100100101011001101001101111100100100"
                    "01100111110000010010010111100110010001001101000010000000001001110000000100100101001011111011111100111110011001100101001100001"
                    "01111101001000111110011001000000100001010111011100000100000101101001111100010001000101100101001111100101101110101101111110011"
                    "01110110000101110101001100110111010110000001001000010001001001011001000000110010001001001101101001101010110101111110101001001"
                    "01000001100000101010110100110110100001101111011010111001000011110011110110101100101111101001101111101101001001101001100111111"
                    "11100011000100100100110100011011111110100111111111001101011111101100011100110010111101101101000101011101101001011111001101100"
                    "11011101101010001101000110100000110000100001100111101110011101101111000000000010110011011011111100101000011110100101101100101"
                    "10001111110101110110000011111001111110111101001100001000101111111010110101010110111101101111100111001101101100101101111111111"
                    "01101000101110101100010100110011100011001100001101010000110010001000000000011110111000111000101001110010110101111111100011100"
                    "01111010110111000101111011011001101010100111010101111111111010101110111001001111001101101010101111101101101001000001101011011"
                    "01101000110110110001111111010010100011001100011101001000101010001100100101001000110101001000100101000101101100111111100010100"
                    "11111111110000011110001010001011111111111000011010000101011011111001011100011011010011001111111100111000011100100001111111101"
                    "10110001101000101000010011101000011111100111110010100011010001111111000011000010010100111101100111000101101011101100111111101"
                    "00100010111111100000101101110111001011101101110101000010111010011011010000110000011001100110001001110010110000001111101001000"
                    "10100000000111101111111111010111001001001000101000111010111011101011000101011100000100001001101111101101101101100001110110111"
                    "00000111100001000100111000101100010100110111001100111011111110110100010111011000000101011101000101011111100100111101001101000"
                    "01100101010110000111010100100100011000100110100110101101010110001011101011100110111010100010111100100000011011000001101101101"
                    "01100011000000011011101000011001010000100001000111110011001001111111100001100010100101011101000101000101101101101100111110001"
                    "11010101011001001111010000101000100110110101010000010000110001110010101010100111111001100111101001101010110001101111101001100"
                    "00000110011110010110110110000001111100011010110001011111000010100010011001101111010100001001101111111101101101100100110111111"
                    "10000000100011111010001011010111010011000010011000111000000111100110010000010011000101101100100101001111100101111101001101000"
                    "01010111111100000011100101000111101011100110001100001110101001110001111011110011100111100101011010100001011110100001101101101"
                    "10100101111100111001000011001010100010000001101001111111010110001000011001011111100100111101100101001101101101101100111101101"
                    "01011110111000011000001111001001001010111100010011000110011001101010010011001000101001100111111000101100101000001011001001000"
                    "01100100011111110101001101001110101110011110110100100011010010010111100000010110001101101000101111100101101101100100110111101"
                    "10101110110000100001110011011110111111000100100111100010100100000101110010001001110001001111100101001110100001111101001100101"
                    "00001000111100100001101100111011100100010100001110000000011100110101110111010010111110100101111010101001011110100001100011110"
                    "10110111101110110001111000011001100001001110100111001010100100010000100000000110110100111101101100110111100101101101111101001"
                    "11011101001101100101101110111000100001110100001001111101010110110011010111000011010001100111111001101100101000001011001001000"
                    "10001111100110100010000111100000011100001100001010001110110000110001100101110110010111001001001101001101001111101100110110111"
                    "11111001010001111001100001001111010011001111001000011001010110011101100101011110001000101110100100101110100001111101001101100"
                    "01000011011111100011000000011010010111000101100000111011001101100101001001101111110110100101101010110001011110100001100010001"
                    "11010001111100110000000001000000110001001000010001001000010111010111111111000110011101101100001101101111100101101101111101001"
                    "11010111001011001000010101101101011100000000101010101110001001000010011100101111011100001111111000011101101100010011001000111"
                    "00111001100000111000001110111011011100110001000001101100100001001101100011110110010111110000101101101101001111101100101101000"
                    "11101111111111110100110010001111111111010101010111010001111011111101110110000001111001001111110100101110100001011111111111101"
                    "01101000111101111110100110011011100010110111001101100001110110001000001001010001111110101000101011010001011110100000100010001"
                    "11001010111111100011111001010110101010110011011000011000010110101110011010111101000101101010101101101111100101100101101011111"
                    "01111000100110000000100111100111100011000100101100000000111110001011011011101001010100011000111001111101101100001011100011101"
                    "01101111111001100001111001011000111111000100011011011100000111111001001000111100001111111111101101101101001111111101111110101"
                    "10000101111000100001100001011111110010011111110101100001011011001101101100010101111001001111110101101110100001001111101101101"
                    "01100011110001110111000010100110010100100100011011000011000111000101000110010111001110111100001110100001011110100000000011101"
                    "11001000001011000011111100001101011110010010100010011010111000000101110101111101000101101101101101101111100101101101111101000"
                    "10000110101011110011101000000011110110000011011101001010110001111000001001110011100101100110111001111001101101101100100000111"
                    "00011100101111000101101110101010111111101110111000010010101011001111011110100011010111101001101101001101000111100101101100101"
                    "11001111000100100111100001110110110010111010101001100010011110001110011101000111010001011111110000111110111001001111101101110"
                    "01000000101100011010010000000011011001110001010010110010101001001101110111100001010110101000001111100001000110101001000011001"
                    "10011110100111000111100100110101111000100101100001101111111111111101000001011010110101000101100101101101101101010111111100111"
                    "10000000100100010010001001110101001001010111100011011010101111110110001100110111011101101110111001011001110101101100100000111"
                    "01000111001101101111111110101111001101110010001111110111101110000010101101110110010101101001101110100101101101100101001101100"
                    "10010101100000010010011111000001111011001111000110011010100010011110100011000110010001001101110001011110101001011110101100001"
                    "11010010101111110111110110000010111010000000010111110101111111001001011011111010001110101000001111100101000110100001000111111"
                    "01011100111111111111100100111001101111001110100010010010100011101010010000100101010101001101100100001101101101001111111101011"
                    "01101011111100010110010111000111101111001110000011101000000111010011100100011101010011110110111101101001101011101100100000100"
                    "01011101110110111110011001100000001101010011011100110100010011100011011100000001001101101101101111100100100101111101001101101"
                    "00011010111011110000001100010110011111100011101001110001010000101010100000101010001001000001101001011011101001000110101000001"
                    "00000001001110101010110100100110011001101010011011010001000111101111010111010100101110111101001111100101111110101001000111111"
                    "01100010001100100000110100010010001000100010011110101101001001011010111100001010110101000111100101101101101101001111111101001"
                    "00001100011010001100000110101000001110000111000110000110000101010100100001111010100011100110111101001001100011111101100000101"
                    "10001111110101110010100010111000101001111101001100111000111111011100100101000110110101101001100111100101000101101111011111110"
                    "10010001100110101111010100010011010111001111101100110000110100001011010000100110101001011100101000111010111001001000101001101"
                    "01001111100001000001011011100000110111100101010100111111100101001111110001101111010100100011001111100101101100101111000111100"
                    "10101100000010110111011110011011101101001101011100000000111110000111000101100000110101000111100101001100010101001101111101011"
                    "10000011101010011111101011011010111111011010111010101101011011111010110101010011000010101111111101001001100010100011111110100"
                    "00000000110110101010110010111001100011100101110011010011000010001000000010110010011101101000100110100100111101111101100011010"
                    "11111110011101100111101101110111101011001010010101011010100110101101101001001000011001011010101001111010100001001101101011101"
                    "10000010001001101110011110000111100011001110101001010010110110001011000100001100001100101000101110000101001100110000100011111"
                    "10111010101110000011111001100101111110110000001101110011100111111101101110100000010101101111100101001111110101101100111111000"
                    "10111010100101000001110101011100111010000000000110000101010011110001010010011110111010101000111100101000101010111101101100100"
                    "10111010101000111001001000010000000000000111000110100011000101101001010000100010101101111111000001001100101101111110111111010"
                    "10000010010111001111010001100001010001010110110001100000111101011100000010010111101001001100101101111011011001000001101001110"
                    "11111110000000010001110110000001010111011111000000111111010101110100101000110111000101101011001100000101101101101001100111111"
        },
        /*  5*/ { UNICODE_MODE, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", 4, 40, 0, 177, 177, "Max capacity ECC 4 Version 40 784 kanji",
                    "111111101010001111111101101110111010110111001110101000010001011011011101001110110011111011010000010101001010011110010000010110111111001001011111101000010010111111001010001111111"
                    "100000101010110001001000101111011001001100100110110000000111110101111011110001101110000111000100101111010011001111100111111001001011011011110011011111111001000010010010101000001"
                    "101110101001001101111001110010010100000000111001001011111000001001111111101010000111011010011010010001111010111001100011000011110100101110001010110001011110011011011010001011101"
                    "101110100011110010000001101101010011100111111111001101100000101100100000111110101110000010101000000010000100011001100001011010011011011001110011001011010011111101101001101011101"
                    "101110100101110010111000011011111101111011011111010111111111110110000100000111110101111110000011101011110001110011111111111111000010010111011111111011101011001011010100001011101"
                    "100000101001101111111111110110001000000111010010011110011000101101011100000010001001100011011101110000101111011110001010010001011111011001011000101000010010111111001010101000001"
                    "111111101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101111111"
                    "000000001010101000111100111110001110000001101000010000111000100000101011100111110000100011011011100100000000100010001111111011001011011011111000111111111001000010010111100000000"
                    "001110101100010001001111000111111110001110001111010001111111111011011010100101110010111111101101110110100110000111111011011011111101101010011111110001011110011011011000011100111"
                    "010000000011010000110010100101111011001000111011000000110110100100010001011011000111000100101100101110001111010001010001010110011011011001110000011011011001010000111111111001111"
                    "011001100011010111001100111001100111110000011011000101111011101100110101100100000110001111011011010010111100101001110011111011100000000110001111111011101011001011010011011110000"
                    "100001011010011111011000010011100000110011111100011101010010010001101001001000001100100011000111001100010000000101110010001011011111011001010001001000010010111111001001010001011"
                    "011100100100100100111110111101111011100110011110001111100000110001001110110101111000001100011010010000011110110001000111111011001010011011111010111111111001000010010111101010101"
                    "110001000100011100001000001010100100010011100101001000100011010010101111101001111110101011101010100010101010110100101011010011111101101010011011000000000110011111011010010011011"
                    "100001101000001001011000111001001101101001110001001001100111010000111001110100000111000010100000110010010011110111101101010001011011011001011000011011011001010000111111111001111"
                    "001000011111011110001101000110101000010010100110000010100000101000001111110101001111011010010001000110111111100001101011101110000000000110001111011011101011001011010011011110000"
                    "110001101011101010001001110101111100011110011100010000010011100110000010110100000001110011010000111111110100011111001010011011011111111001010001001000010010111111001001010001011"
                    "010101001011001100101001001100000010110010011010110001010110011010111110000100010101110001000010110011001000111111001111100101101010011011111010111111011001011000010011101011001"
                    "100011100111101001011100111011111101001010110000010101101001110001010010000100110010010011011111011100011101001010110011010011111011101011000011000000000110011111011010010011011"
                    "100101000111111110000111000000001000011100000010111011010001000000110100001101101001110010011101001000111011000101101101001110011011011001011000011011011001010000111111101011111"
                    "011111111011000001101111101011101011110100111010000010100111100111010110100111101011010101100001000111000100111000110011010011010001000110001111011011101011001011010011011110000"
                    "100110011001110000000000100111100010001010111110111000010011001010001100011011101000100001110000001111000100100111000010011010111111111001010001001011011011110011101101010001111"
                    "111001100111011110111110001100010010001110000000111110011100000110000010010110100110111101000111101010001000100000100111100101101000010010111010111111011001011000010011001011001"
                    "011011000111000100000101010101010111000101101000001111001100001001101001101110101110010001000011000000011110110100100011011010011011101011000011000000000110011111011010011101011"
                    "010111110111010111010011100001000000001101101000010000000001001000101101000110011010000001100111010001011110010001100100111111101011011011011000011011011001010000111111101011111"
                    "011010011000101101000111011111100011111101010110100111110111100100101111101111001100010000100101100100001100110000001011001011010001000110001111010100101110001011011011011110010"
                    "010100110110001011100001100100111001101101010100111101000011011000100010110010011110111011101111000001010001001100001000011010111111111001011001001011011011110011101100110001111"
                    "011100011110011110110001100011101001110010011100101001100001100011101101011001001010011110010010010101111011100001011111111101001000010010111111111111011001011000010011011011001"
                    "110011111101100110111010010011111001010000001100001001111111101000010000001110011111111110100000100100001100100111111011010011010011101101001111100000000110011111011010111111011"
                    "111010001010001001111001001110001111100110011111101000111000101000111010101101001101100010110010010001011110001010001100101110001011011011001000111011011011110001000111100011101"
                    "101010101010000100101010101110101010000011111111110101111010111100101001101110000110101010111011010101001100011010101011001011010001011110011010110100101110001011011011101010010"
                    "011110001010001011000110010110001111001001111101101011101000100011011111001000100111100010111000011110001101101110001000011011011111111001011000101011011011110011101100100011111"
                    "010111111001110000101101100111111111011100101100001001111111111110100110111010010110111111110111101110111100110111111111010001111000010010101111111111011001011000010011111111001"
                    "011101000011010100010000000101000001000000001101010101111011101110111100001100010100000010110011101100111100100110110011001010010011101101011011011010010011011111111001111101010"
                    "110110101000001010101010001100010111100011100001011001010010010001100110100110010101100001001010110111000011010000010010101110001011010011111000011011011011110001000110001011101"
                    "100001001000010111011100111000000010111011101001100100001000111110000111000101100011110011011010000011001010110111011011011011010001011110010111010100101110001011011011111110010"
                    "001000100011000110000011110010101101011010101101010111101011011100011010000101101001011000111101011000011101100011011000000111111111011011001010001011011011110011101101000011111"
                    "101010011001011110001001111001101110101110011110110100011011101111101001000000101110010101111101101011110110100101101111011000011000010010110111110111101101001010011011011010011"
                    "001001100010001010001010100010111111111110011010011010101001110011001110000000011001011101101011101101000000010011100001001010010111101101011011011010010011011111111001111101010"
                    "010111011111110111011101100010010000110110000011110000000010110110000001100000101000001010100101111001101001001111011010110111001011010011111000011011011011110001000110001011101"
                    "010011111110100010001111101011000110111111111000110101011111011010001100010111010011111010010101011100000101010110001011011000110000111110011011110100101110001011011011111110010"
                    "110000001100100101000100010110110001001000101010001111111110010100100000000001010010010001001111010001001100001110001000011111011111011011001010001011010011111101101011000011011"
                    "001011110111001011011111111010111101111010000100100110101110110000010011110110001100110100111010000111101111100010110111011000011110010011010111110111101101001010011011011010011"
                    "101000000011001000011100100011111000010111000011000111010010100010110000100100101111110001011011011001000100010000100001011011110011101101011011011010010011011111111001111101010"
                    "011001101011010001100101011011101011010000100101010110111011000100110101111010111101100010000100101110001011000100100011101011001101010011110101011011011011110001000110001011101"
                    "101001010110110000011010010000110110001011100100010001101111111001000111000101110111010100010000101000100110010010111011011001010010111110011011110001011110011011011001111111010"
                    "011000101101010111000100100110010110010110101101110011011101110011100001001101110010100001011001000100101100110001100000011111011111011011101010001011010011111101101011000011011"
                    "010011000001101111010000000000001111101011100100001011110111100010011110111100001100111001011000110000000010110100000111010001011000010011010111110111101101001010011011011010011"
                    "100110100000111001000111110111010000111111101100101010101111101010001110101101110011111000101001011111110000010101101000001010011011001001011011011010010011011111111001010001010"
                    "001011010001011000101001100000101001001000010000000000101111000001100111101010000111010000000111111100101110110010001011111011001011010011100101011111111001000010010111001010101"
                    "100110111011100111010001111111010110110001100011011110100011010010010010011110101011011001001111010001010101110000000011011001010010101110011011110001011110011011011001011111010"
                    "110100011101010000011101101010000011100001100011010001100101100001010101010000010110011010111100101111111011101110011000010111111011011011101010001011010011111101101010100011011"
                    "101000101111001111010000001010011101001101011000010111011010111101111000101111100000111101111111100000011111101100000111101000000110010111011111110111101101001010011011011110011"
                    "110011010010001111111100001100100010110010101111001110101011111101011111001110110010111111010000010101001010011110000000010011011011001001011011001000010010111111001001110001011"
                    "011111111010110001001001001011010000001110100110010110101100110010011101110001101111100111000100101111010010101000100111111011001011011011100101011111111001000010010110001010101"
                    "110100001001001101111000010010110101100011111011001010000110101011111011101010000111111100011010010001111010111011101011000001110000101110011011110001011110011011011001111111010"
                    "110011111011110010000000001011111011100010111111001101011111101110000000111110101111111110101000000010000101111111111001011110011111011001101111101011010011111101101011111111011"
                    "001010001101110010111000111110001101011101011010010000001000110100100110000111110101100010000011101011110000010010001111111001100010010111011000111011101011001011010010100010000"
                    "110110101001101111111110110110101001100011010001011001011010101100011010000010001001101011011101110000101111111010101010010011011011011001011010101000010010111111001001101011011"
                    "011010001010101000111101111010001110100001101000010000011000100001001101100111110001100011011011100100000000000110001111111010101001011011111000111111111001000010010111100010101"
                    "111011111100010001001111100111111110101110001110010101001111101000011000100101110010111111101101110110100111100011111011011011011111101010011111110001011110011011011000111111010"
                    "010000011011010000110010100000111010001100111111100001000001010010010011011011000110001110101100101110001111010101100001010111011001011001110000011011011001010000111111011001111"
                    "011001101011010111001101111111001111010100011000100010111101001011110101100100000111000101011011010010111100101101111011111001100010000110001111111011101011001011010010011110000"
                    "100001001010011111011001010011100001110101111111100000100100011100001101001000001100011101000111001100010001000110111010001011011101011001010011001000010010111111001000010001011"
                    "011111100100100100111111111011110010100010011111011110011111111101001110100101111001011010011010010000011110010100110111111101001010011011111101011111111001000010010111101010101"
                    "110001001100011100001000001101010101110001100010110010111000000110101001110001111110101101101010100010101011110010110011010011111011101010011011000000000110011111000011010011011"
                    "100000101000001001011000011101001101001001110001001111110011100110111111100100000111011010100000110010010011110001110101010111011001011001010000011011011001010000111111011001111"
                    "001000000111011110001101100110101000010100100000101010111001110100001011110101001110011010010001000110111111100000000011101000000010000110001111111011101011001011001010011110000"
                    "110011101011101010001000010011011100111100011110011100010110000010000000101100000000100111010000111111110101011101101010011011011011111001010011001000010010111111010001010001011"
                    "010100001011001100101001001110001010010110011100001101000000110010111010011100010100011011000010110011001000111111110111100101101000011011111010111111011001011000010011001011001"
                    "100000100111101001011101011001011101101000110001100111100001111001010100000100110010100011011111011100011101001111101011011011111011110011011011011000000110011111011010110011011"
                    "100100000111111110000111100011110001111110000100010101000110011110110100010101101001000100011101001000101011000111010101010110011111011001011000010011011001010000101111011001111"
                    "011101100011000001101111001001110011110100111111101000100011010111010000100111101011001111100001000111011100111111110011001011010011011110001111101011101011001011011011111110000"
                    "100101011001110000000001000111111011101110110010010000010010010010001010011011101001100011110000001111011101000101101010011010111001110001010001001011011011110011110100010001111"
                    "111000100111011110111110101101111011101100001011101110010001010110000000001110100111101101000111101010010000100111000111111101101000011010111010111111011001011000011011101011001"
                    "011001000111000100000101010010110111100011101110110111001010110001101101110110101110001011000011000000010111110010100011011010011111101011000011010000000110011111011010110011011"
                    "010101101111010111010011100001011000101011100110001000010110111000101001010110011011100011100111010001001111110101111100110111101011011011011000001011011001010000110111011001111"
                    "011000011000101101000110111100101010011101001001001111100001100100101101101111001101011010100101100100010101010001111011001011010101010110001111001100101110001011011010011110010"
                    "010101101110001011100000100001100000101101010110000101010010000000100010110010011110110011101111000001000000101111000000010010111001110001010001011011011011110011111101110001111"
                    "011101001110011110110000000010001000010010010010011001111111001011101111001001001010110000010010010101110010100001011111111101001110001010111010100111011001011000000011101011001"
                    "110011100101100110100010110011111000110000011001100001101010011000010010010110011110010010100000100100010100000100101011011011010001111101000011011000000110011111011011010011011"
                    "111001000010001001110001001010010110000110001011011000100000000000111000100101001100010010110010010001000111001001110100110110001011000011011000001011011011110001010110001011101"
                    "101011111010000100111011101011101011100011101110110101110011110100101011111110000110010100111011010101010100111000101011001011010101010110001111010100101110001011001011011110010"
                    "011110011010001011011111110101100111001001110000100011100101010011011101011000100110100000111000011110000101001101010000010011011011101001011001011011011011110011110101110001111"
                    "010111111001110000101100100011111110011100101100110001101111111110100100111010010110111111110111101110101100110011111111011001111010010010111111111111011001011000011011111111001"
                    "011110001011010100011001100010001000100000001100101101101000100110111000001100010101100010110011101100101101100110001011011010010111111101011000111010010011011111111001100011010"
                    "110110101000001010100011001010101111000011110001101001011010101001100010101110010101101011001010110111001011110110101010110110001011000011101010111011011011110001001111101011101"
                    "100010001000010111010100111110001010111011110001111100011000110110000101011101100011100011011010000011000010110010001011001011010011011110001000101100101110001011010011100010010"
                    "001011111011000110001010110111111100011010111101001111111111110100111100001101101000111110111101011000001100000011111000010111111001011011011111111011011011110011111101111111111"
                    "101001000001011110001001111100011110001110001111111100101100011110001011011000101111111101111101101011110111000100111111011000011110001010110010111111101101001010011010001010011"
                    "001011100010001010010010000101001110111110011010111011011000010011001000010000011000010001101011101101011001110110100001011010010111111101011011010010010011011111101001011101010"
                    "010111010111110111010100000101000000010110001010001000010101011111000011110000101001100000100101111001101001001001100010110111001011011011111000001011011011110001011111101011101"
                    "010010111110100010001110001001100110111111101000111100001111101010101100010111010011110001110101011100010100110110011011001000110000110110010111011100101110001011011010011110010"
                    "110000010100100101010100110010101001001000110011001110001011001100100000010001110011011011101111010001000101101010000000010111011111001011001001011011010011111011111011010001011"
                    "001000111111001011001111111001110101111010011101111111111001111001110011101111001100011111011010000111111110100101000111001000011110001011010011110111101101001110011010111000011"
                    "101001000011001000000101000011011000010111011011011110001101110011010000111101101111000011111011011001011100010010101001011011110011111101011101001010010011011011110001011011010"
                    "010010101011010001100100011010001010110000110101001111100000001101110101101010111100000000100100101110000011000110000011110011001101010011111010010011011011110011001110001111101"
                    "101101001110110000000010010010011111101011100101011001011110101001000111001100110111110010010000101000100110010111000011001001010010101110011011011001011110011011011001011011011"
                    "011100100101010111000100000110101110010110100101011011011001011011000001000100110010011001011001000100100100110111000000011111111111001010001000000011110011111011100011100001000"
                    "010110011001101111000000100011111111001011100101011011001001101011011110100101001100110101111000110110000010110011100111001000011000011011010011111110001101001000011011011010011"
                    "100101100000111001001110010010110000011111100101011011110101110011101110111100110011111100101001011011110000010001110000011011111011000001111011011011110011011111101000011011001"
                    "000011011001011000101000100010011000101000000000011000011100100000100111101011000111101010100111111010101110110001010011110011001011011010010000010111011001000010001111001000101"
                    "100111111011100111001001011001000111010111111011011110001101000011010010001111101011011010001111010111001101110000011011011001010010101110011011111000111110011101010001111011010"
                    "111000001101010000001100001110101010000111100011010000011100011001110101010000110110110000011100101001111011101010001000011111111011010011001110011011010011111011111011000011011"
                    "101111110111001111001000001000111100101001011000011110010000101100111000110110000000110101111111100100001111101000111111101000000110011110110011110111001101001110001011011100001"
                    "110000000010001111110100001111100010110000110111000110101010111101011111010110010011110000110000010001010010011100100000011011111011011001011001011001110010111111001001011011001"
                    "011001101010110001010001001001100000001110111110001111101001001010011101111000001111000011100100101011000010101100110111100010001011011010000001001111011001000000010111001000110"
                    "111001010001001101110000010110100101100111111011000011000011110011011011101010000111010101011010010001111010111110101011011001010000111111111111111001011110011011011001011111011"
                    "110100101011110010011000001010000011100110101111010100100011100110000000100110101110000000101000000100001101111111101001011111111111001001001010010011110011111111111010100001011"
                    "001111000101110010110000111010010101011101000010011000011101101100100110011111010100111001000011101111110000010110000111101000000010011111011101110011001011001111001011011010001"
                    "111010100011101001111110110101001001100001010001010000101011110100011010011011101000111001111101110010111111111101101010011011011011011001011001001000010010111001011001110011011"
                    "010111010110101010101101111010111110100001110000010001101101010001101101111111010001110000011011100110010000000100001111100010001001010011000111011111011001000010011110001110111"
                    "111011111000010011001111100111111110101110000110000100101111101000011000110100110011111110101101110100111111100011111011011011011111101010011111100001111110011011000001111111000"
                    "011110001011010110111010100110001010001100100111100001101000110010010011011011000110100010101100101100010111010110001001011111111001011001011000110011111001010110111110100011111"
                    "011110101101010001000101111010101111010000010000100011011010101011010101111101000111101010111011010000100100101010101011101001000010011111001010101010001011001011010011101010010"
                    "100010001010011001001001010110001001110001101111100000101000111100101101000001101101100010100111001100010001000110001010011011011101010001111000110001010010111011001000100011001"
                    "011011111110100010111111111011111010100110011111011111111111111101101010111100111001111111111010010010000110010011111111100100001010001010011111111110111001000100010111111110110"
                    "110110011010011000001000001011001101110111110010110011110001000111101001111001111111111001101010100100100011110110000011011011011011110011011001100001100110011011000011111001000"
                    "100000111110001101001000011100010101001111111001001111100111100111011111100100000110011010000000110000010011110110011101010110111001011001010010011010011001010000111111000011111"
                    "001000001111011000001101100010010000010010101000101010111110110101101101111101001111011101010011000000110110000000000011101001000010000111101101111011101011001111001010011010001"
                    "110011110001101010000000010110100100111010010110011100000001100010100110101100000110011111010110111001111100111110000010011011011011111000010011001001010010111001010000010111010"
                    "010011011001001100111001001111001010010000001100001101111000110011111110011101110011001111100010110101000001011000100111100100001000011011111011011110111001011100010011101011010"
                    "101100101011101011010101011100111101101000101001100110011000111001110110000100010000110100011011011110011101001101101011011011011011110011011111111001000111111011011011011011011"
                    "100100011001111000011111100000111001111110011100010100010111011110010100010100101011010110111011001110101010100101100101010110111111011000110010010011011000010110101111011011101"
                    "010001101001000111111111001001001011110100111111101001111101010111010110100110101110000100000101000101011100111101111011001011010011011111001011101010001011101001011010011010011"
                    "100101001111110100010001000101100011101100101010010001110000010010001000011010101100100101110000001001011101000100111010011011011001110001010001001011011100110001110101010001110"
                    "111111100001011100101110101101110011101110000011101110100001110110000000001111000000001011000001101100010000100110110111111100001010011011111111011111011011011010011011001111011"
                    "011101001101000000010101010011010111100101110110110110000100010001101001110110001110110101100001000000010110010110110011011011011011101011011111010001100010011111011010110011011"
                    "010000101001010011000011100111001000101111100110001000100011111001001011010111011001011010000001110111001111010011110100110110101111011011011010001010011111110010110111011101110"
                    "010100000000101111011111111100101011011111001001001110100011100101001101101110001101001010000110000000010101010000000011001011010011010111101001101100001011101001011011111010011"
                    "011011101000001001101001000001011000101101010110000101010000000000100110110011011101101110101011100001000000101001101000010011111101110001010111011011011011010011111100010111111"
                    "010000001000011010111001000100001000110000010010011000010110101010001011001000001101101010010000010001110011100011110111111100001010001010011110100111111000011010000011101111000"
                    "110000100101100110100010010111011001010000011001100001001011111000010110010111111010010010000010000110010101100011101011011011010011111101000011011000000111011001011010110001011"
                    "110100000000001101110001001111110110100110001011011000010000000000111110100100101101111101110110010011000111101111010100110110101111000011111100001010011111110011010111011011110"
                    "100001100010000010111011101011110011100001101110110101100011010100101111111111100011010110011111110101010100111101110011001011010011010111101011110101001011001011001010011110001"
                    "011101011000001011011111010001111111101111110000100011010110010011011001011000000000101011011111011000000100001011101000010011111101101000010111011010111001010101110101110111110"
                    "011000100110110100101100100101111110011100101100110001111001011111000000111010010110110100010101101100101101110111000111011000011010010011011000111111011111111010011011101001011"
                    "010001000010110110011000100000110000000110001100101100101000100111111010001101010100011010010010001110101100100110100011011011010011111100100011011010010111011001111001010011011"
                    "111001101111000110100011001011011110100001110001101001100010101001100110101111110101111011101010110001001010110001101010110111001111000011111010011011111011010111001110011011101"
                    "100100011110111101010101111010101010111011110001111101110001110110100011011100000110011011011011000011000011010101100011001011010011011110001011001101001100101001010011011110010"
                    "000111111001011110001011010111111101011010111101001111101111110100111100001100101000111110111111111000001101100111111000010111111101011011111111111011011011010011111100111111111"
                    "101110001101100010001000011010001110101100001111111100101000111110001101011000001111100011011111001001110111000110001111011000011010001011011000111111001111101010011011100010011"
                    "000010101011110000010011100010101110111000011010111011001010110011001010010001111000101010001101101101011001010010101001011011110001111100111010110011010000011111101001101011001"
                    "011010001011111011010100000110001000010100001010001000001000111111000011110000001110100010000011111111101000001010001010110111001111011011011000101010011011110001011110100011111"
                    "010011111101010000001110001011111111111101101000111100011111101010101010011110110101111111010110111010010101010111111011001001010010110111001111111101001011001011011011111110011"
                    "111101001100101111010100010101100000001010110011001110011100001100100010000001110010100010101110010011000100001111000000010111111101001010001011011011010011011011111011110011011"
                    "001001101101010001001110011010001100111000011101111111101011011001110001111110101111001001111111000011111110000101000111001000011010001011010110110111101010001110011011001010011"
                    "101111000111111010000101000101111001110001011011011110011010010011010000101100001010110011111111110101010101110110111001011011110001111101011011001010010111111011110000110101010"
                    "010001100101110101100100111010010010010110110101001111110000001101110101110010111111101010100100101000000011100011111011110011001111010011111010010011011011010011001111001011101"
                    "101111011101111110000011110101101111001001100101011001001011101001000111000100110010010100010010101110110111110010110011001001010010101110010001011001011011011011010000011001011"
                    "011110111011001001000100100111100110010000100101011011001101011011000011000100110011000001011001001100100101110111001000011111111011001010001111000011101000011011111011010111000"
                    "010101000010011011000000100000011111101101100101011011011100001011011100111101001001001101111100111110000011110100110111001000011010011011010010111110010111001000011011001000011"
                    "100111100001010001001111110111011000011001100101011011101000010011101100111100110111001000101111100011101001110110111000011011111001000001111011011011111011011111110000111101001"
                    "000011010111110000101001000111010001101100000000011000000101000000100001110011000000110000100000100010111110110011111011110011001111011010011000001111011011011010001111001010101"
                    "100110111011010111001000111111101111110011111011011110011111100011010000000111111001010000001010110111000101110110010011011001010000110110011111011000101001010101010000011111010"
                    "111000010001111110001101001110110011000111100011010000011011011001110001010000101100000010001110011001101010101110001000011111111011001011001001000011011111101011111011010001011"
                    "101111111100000111001001101111100100001101011000011110001001101100111100101110011101000111101001001100000110001011010111101000000010011110110011111111011000011110011011011000001"
                    "110001001001111101110100001001011011010000110111000110110101111101011111001110000000101010101100100001000011111110111000011011111001000001011101010001111011011111011000111011001"
                    "011001110001010101000000101010001001101110111110001111111000001010011101110000001011100001111101110011010010001110011111100010001111011010010010001111011100110000001111001110110"
                    "111011011101110001101000110000010101000111111011000011001110110011011111101010010001100011000010110001110011111101011011011001010000110111111011011001010011001011010000011011011"
                    "110111110001010010010001001110101010000110101111010100111001000110000110111110101111011000101101011100001100111011001001011111111011001001001000000011101010010111111011000001011"
                    "001101000010111010111001111011101101011101000010011000010001101100100010001111010101001101001001010111100000110101110111101000000010011111010011111011011000011111000011011010001"
                    "111000110111000001100111010010111001000001010001010000101101110100011100011011101000101101111100111010111111111101111010011011011001011001010011010000000011100001011000111011011"
                    "010111010010111010111100011110001110100001110000010001101100110001101101100111000111110010011100100110011001100101000111100010001111011011011000001111011011011010001111001000111"
                    "111001110001000011010110100111010110101110000110000100100101001000011010101100100110110010111001101100100110100110011011011011011001110010001011111001101000011011010000011011000"
                    "011110010100100110100010100000110011001100100111100001100100010010010101000011001000111000100111110100000111110110001001011111111011001001011110011011101010010110111111000010111"
                    "111001100100100001001101111100110110010000010000100011011000101011010011111101010000000100101111011000100100001110110011101001000010010111010011111010010111111011000011011101110"
                    "000100010011011001111000010111111000110001101111100000101010111100101111010001111101110000101001110100000001000100111010011011011001011001111001010001011011011011011001111010101"
                    "010101101110101010110110011011111010100110011111011111101111111101101010100100100100111111101001001010001110010111111111100100001110011010011111101110101001010100001111111110110"
                    "000000001011110001011001101110001101110111110010110011111000100111101001101001111000100011111110101100100010110110001011011011011001110011011000110001101000011011011011100011000"
                    "111111100111010101010000111110101101101111111001001111101010100111011111111100000000101010010101101000011010110010101101010110111111001001011010111010010111001000111110101011011"
                    "100000100111000001110100100110001000010010101000101010101000110101101101100101000011100010011110110000110110000110001011101001000010010111101000111011111011011111000011100010001"
                    "101110101000010010001001010011111101011010010110011100011111100010100110110100000010111110101011001001100101111011111010011011011001111000001111110001011100110001011001111111010"
                    "101110101000101101011000101111110010010000001100001101101001010011111110011100010010011011010001110101001001111110100111100100001110011011111001001110101001010100001010101001011"
                    "101110101010001011100101111010101100101000101001100110010011111001110110010100001111001101110110111110001101101110111011011011011011110011010111110001011111101011011011011111100"
                    "100000100000010000101110000100011000011110011100010100000011111110010100001100010010001001110010000000110010100111101101010110111111001000101010011011011000011110111110000000000"
                    "111111100000001110110110101010011010010100111111101001111101110111010110101111000111011001110100010111000100111000011011001011010011010011010101111010000011100001000011111011001"

        },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_QRCODE;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_QR_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %d, %d, %s, %d, %d, \"%s\",\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].data, data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
                symbol->rows, symbol->width, data[i].comment);
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

static void test_microqr_options(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_1;
        int option_2;
        int ret_encode;
        int ret_vector;
        int expected_size;
        int compare_previous;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "12345", -1, -1, 0, 0, 11, -1 }, // ECC auto-set to 1 (L), version auto-set to 1
        /*  1*/ { "12345", 1, -1, 0, 0, 11, 0 }, // ECC 1 (L), version auto-set to 1
        /*  2*/ { "12345", 2, -1, 0, 0, 13, 1 }, // ECC 2 (M), version auto-set to 2
        /*  3*/ { "12345", 3, -1, 0, 0, 17, 1 }, // ECC 3 (Q), version auto-set to 3
        /*  4*/ { "12345", 4, -1, ZINT_ERROR_INVALID_OPTION, -1, 0, -1 },
        /*  5*/ { "12345", -1, 1, 0, 0, 11, -1 }, // ECC auto-set to 1, version 1
        /*  6*/ { "12345", -1, 2, 0, 0, 13, 1 }, // ECC auto-set to 2, version 2
        /*  7*/ { "12345", 2, 2, 0, 0, 13, 0 },
        /*  8*/ { "12345", -1, 3, 0, 0, 15, 1 }, // ECC auto-set to 2, version 3
        /*  9*/ { "12345", 2, 3, 0, 0, 15, 0 },
        /* 10*/ { "12345", 1, 3, 0, 0, 15, 1 },
        /* 11*/ { "12345", -1, 4, 0, 0, 17, 1 }, // ECC auto-set to 3, version 4
        /* 12*/ { "12345", 3, 4, 0, 0, 17, 0 }, // ECC auto-set to 3, version 4
        /* 13*/ { "12345", 2, 4, 0, 0, 17, 1 },
        /* 14*/ { "12345", -1, 5, 0, 0, 11, -1 }, // Size > 4 ignored
        /* 15*/ { "12345", 1, 5, 0, 0, 11, 0 }, // Ignored also if ECC given
        /* 16*/ { "12345", 1, 1, 0, 0, 11, 0 }, // ECC 1, version 1
        /* 17*/ { "12345", 1, 2, 0, 0, 13, 1 }, // ECC 1, version 2
        /* 18*/ { "12345", 1, 3, 0, 0, 15, 1 }, // ECC 1, version 3
        /* 19*/ { "12345", 1, 4, 0, 0, 17, 1 }, // ECC 1, version 4
        /* 20*/ { "12345", 2, 1, ZINT_ERROR_INVALID_OPTION, -1, 0, -1 },
        /* 21*/ { "12345", 2, 2, 0, -1, 13, -1 }, // ECC 2, version 2
        /* 22*/ { "12345", 2, 3, 0, -1, 15, 1 }, // ECC 2, version 3
        /* 23*/ { "12345", 2, 4, 0, -1, 17, 1 }, // ECC 2, version 4
        /* 24*/ { "12345", 3, 1, ZINT_ERROR_INVALID_OPTION, -1, 0, -1 },
        /* 25*/ { "12345", 3, 2, ZINT_ERROR_INVALID_OPTION, -1, 0, -1 },
        /* 26*/ { "12345", 3, 3, ZINT_ERROR_INVALID_OPTION, -1, 0, -1 },
        /* 27*/ { "12345", 3, 4, 0, -1, 17, -1 }, // ECC 3, version 4
        /* 28*/ { "12345", 4, 4, ZINT_ERROR_INVALID_OPTION, -1, 0, -1 },
        /* 29*/ { "12345", 5, -1, 0, 0, 11, -1 }, // ECC > 4 ignored
        /* 30*/ { "12345", 5, 1, 0, 0, 11, 0 }, // Ignored also if size given
        /* 31*/ { "123456", 1, 1, ZINT_ERROR_TOO_LONG, -1, 0, -1 },
        /* 32*/ { "123456", 1, -1, 0, 0, 13, -1 }, // ECC 1 (L), version auto-set to 2
        /* 33*/ { "123456", 1, 2, 0, 0, 13, 0 },
        /* 34*/ { "ABCDEF", 2, 2, ZINT_ERROR_TOO_LONG, -1, 0, -1 },
        /* 35*/ { "ABCDEF", 1, -1, 0, 0, 13, -1 }, // ECC 1 (L), version auto-set to 2
        /* 36*/ { "ABCDEF", 1, 2, 0, 0, 13, 0 },
        /* 37*/ { "ABCDE", 2, -1, 0, 0, 13, -1 }, // ECC 2 (M), version auto-set to 2
        /* 38*/ { "ABCDE", 2, 2, 0, 0, 13, 0 }, // ECC 2 (M), version auto-set to 2
        /* 39*/ { "ABCDEABCDEABCD", 1, -1, 0, 0, 15, -1 }, // 14 alphanumerics, ECC 1, version auto-set to 3
        /* 40*/ { "ABCDEABCDEABCD", 1, 3, 0, 0, 15, 0 },
        /* 41*/ { "ABCDEABCDEABCD", 2, 3, ZINT_ERROR_TOO_LONG, -1, 0, -1 },
        /* 42*/ { "ABCDEABCDEA", 2, 3, 0, 0, 15, -1 }, // 11 alphanumerics, ECC 2, version 3
        /* 43*/ { "ABCDEFGHIJABCDEFGHIJA", 1, -1, 0, 0, 17, -1 }, // 21 alphanumerics, ECC 1, version auto-set to 4
        /* 44*/ { "ABCDEFGHIJABCDEFGHIJA", 1, 4, 0, 0, 17, 0 },
        /* 45*/ { "ABCDEFGHIJABCDEFGHIJA", 2, 4, ZINT_ERROR_TOO_LONG, -1, 0, -1 },
        /* 46*/ { "ABCDEFGHIJABCDEFGH", 2, 4, 0, 0, 17, -1 }, // 18 alphanumerics, ECC 2, version 4
        /* 47*/ { "ABCDEFGHIJABCDEFGH", 3, 4, ZINT_ERROR_TOO_LONG, -1, 0, -1 },
        /* 48*/ { "ABCDEFGHIJABC", 3, 4, 0, 0, 17, -1 }, // 13 alphanumerics, ECC 3 (Q), version 4
    };
    int data_size = sizeof(data) / sizeof(struct item);

    struct zint_symbol previous_symbol;

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);
        if (data[i].compare_previous != -1) {
            ret = testUtilSymbolCmp(symbol, &previous_symbol);
            assert_equal(!ret, !data[i].compare_previous, "i:%d testUtilSymbolCmp !ret %d != %d\n", i, ret, data[i].compare_previous);
        }
        memcpy(&previous_symbol, symbol, sizeof(previous_symbol));

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

static void test_microqr_input(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int ret;
        char* expected;
        char* comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS, UTF-8 E0B881
    // Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447 (\204\107), UTF-8 D096
    // ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS, UTF-8 E0BA81
    // ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5
    // ･ U+FF65 half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xA5 (\245), UTF-8 EFBDA5
    // ¿ U+00BF in ISO 8859-1 0xBF (\277), not in Shift JIS; 0xBF same codepoint as single-byte half-width katakana ｿ (U+FF7F) in Shift JIS (below), UTF-8 C2BF
    // ｿ U+FF7F half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xBF (\277), UTF-8 EFBDBF
    // ‾ U+203E overline, not in ISO/Win, in Shift JIS single-byte 0x7E (\176) (tilde), UTF-8 E280BE
    // 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9
    // 茗 U+8317 kanji, in Shift JIS 0xE4AA (\344\252), UTF-8 E88C97
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "é", 0, "87 A4 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1)" },
        /*  1*/ { DATA_MODE, "é", 0, "8B 0E A4 00 EC 11 EC 11 00", "B2 (UTF-8)" },
        /*  2*/ { UNICODE_MODE, "β", 0, "C8 80 00 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /*  3*/ { UNICODE_MODE, "ก", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input data", "ก not in Shift JIS" },
        /*  4*/ { UNICODE_MODE, "Ж", 0, "C8 91 C0 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /*  5*/ { UNICODE_MODE, "ກ", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input data", "ກ not in Shift JIS" },
        /*  6*/ { UNICODE_MODE, "\\", 0, "85 70 00 EC 11 EC 11 EC 00", "B1 (ASCII)" },
        /*  7*/ { UNICODE_MODE, "¥", 0, "86 94 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous)" },
        /*  8*/ { UNICODE_MODE, "･", 0, "86 94 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /*  9*/ { UNICODE_MODE, "¿", 0, "86 FC 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous)" },
        /* 10*/ { UNICODE_MODE, "ｿ", 0, "86 FC 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 11*/ { UNICODE_MODE, "~", 0, "85 F8 00 EC 11 EC 11 EC 00", "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 12*/ { UNICODE_MODE, "‾", 0, "85 F8 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) (same bytes as ~ ASCII above, so ambiguous)" },
        /* 13*/ { UNICODE_MODE, "点", 0, "CB 67 C0 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 14*/ { DATA_MODE, "\223\137", 0, "CB 67 C0 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 15*/ { DATA_MODE, "点", 0, "8F 9E 0A E4 00 EC 11 EC 00", "B3 (UTF-8)" },
        /* 16*/ { UNICODE_MODE, "茗", 0, "CE AA 80 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 17*/ { DATA_MODE, "\344\252", 0, "CE AA 80 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 18*/ { DATA_MODE, "茗", 0, "8F A2 32 5C 00 EC 11 EC 00", "B3 (UTF-8)" },
        /* 19*/ { UNICODE_MODE, "¥点", 0, "8D 72 4D 7C 00 EC 11 EC 00", "B3 (Shift JIS) (optimized from B1 K1)" },
        /* 20*/ { DATA_MODE, "\134\223\137", 0, "8D 72 4D 7C 00 EC 11 EC 00", "B3 (Shift JIS) (optimized from B1 K1)" },
        /* 21*/ { DATA_MODE, "¥点", 0, "97 0A 97 9E 0A E4 00 EC 00", "B5 (UTF-8)" },
        /* 22*/ { UNICODE_MODE, "点茗", 0, "D3 67 F5 54 00 EC 11 EC 00", "K2 (Shift JIS)" },
        /* 23*/ { DATA_MODE, "\223\137\344\252", 0, "D3 67 F5 54 00 EC 11 EC 00", "K2 (Shift JIS)" },
        /* 24*/ { DATA_MODE, "点茗", 0, "9B 9E 0A E7 A2 32 5C 00 00", "B6 (UTF-8)" },
        /* 25*/ { UNICODE_MODE, "点茗･", 0, "D3 67 F5 55 0D 28 00 EC 00", "K2 B1 (Shift JIS)" },
        /* 26*/ { DATA_MODE, "\223\137\344\252\245", 0, "D3 67 F5 55 0D 28 00 EC 00", "K2 B1 (Shift JIS)" },
        /* 27*/ { DATA_MODE, "点茗･", 0, "A7 9E 0A E7 A2 32 5F BE F6 94 00", "B9 (UTF-8)" },
        /* 28*/ { UNICODE_MODE, "¥点茗･", 0, "99 72 4D 7F 92 AA 94 00 00", "B6 (Shift JIS) (optimized from B1 K2 B1)" },
        /* 29*/ { DATA_MODE, "\134\223\137\344\252\245", 0, "99 72 4D 7F 92 AA 94 00 00", "B6 (Shift JIS) (optimized from B1 K2 B1)" },
        /* 30*/ { DATA_MODE, "¥点茗･", 0, "4B C2 A5 E7 82 B9 E8 8C 97 EF BD A5 00 00", "B11 (UTF-8)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
        symbol->input_mode = data[i].input_mode;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_MICROQR_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Check MICROQR padding (4-bit final codeword for M1 and M3 in particular)
static void test_microqr_padding(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_1;
        int ret;
        char* expected;
        char* comment;
    };
    struct item data[] = {
        /*  0*/ { "1", -1, 0, "22 00 00", "M1, bits left 13" },
        /*  1*/ { "12", -1, 0, "43 00 00", "M1, bits left 10" },
        /*  2*/ { "123", -1, 0, "63 D8 00", "M1, bits left 7" },
        /*  3*/ { "1234", -1, 0, "83 DA 00", "M1, bits left 3" },
        /*  4*/ { "12345", -1, 0, "A3 DA D0", "M1, bits left 0" },
        /*  5*/ { "12345678", 1, 0, "40 F6 E4 4E 00", "M2-L, bits left 8" },
        /*  6*/ { "123456789", 1, 0, "48 F6 E4 62 A0", "M2-L, bits left 5" },
        /*  7*/ { "1234567890", 1, 0, "50 F6 E4 62 A0", "M2-L, bits left 1" },
        /*  8*/ { "12345678", 2, 0, "40 F6 E4 4E", "M2-M, bits left 0" },
        /*  9*/ { "ABCDEF", 1, 0, "E3 9A 8A 54 28", "M2-L, bits left 3" },
        /* 10*/ { "ABCDE", 2, 0, "D3 9A 8A 4E", "M2-M, bits left 0" },
        /* 11*/ { "123456789012345678901", 1, 0, "2A 3D B9 18 A8 18 AC D4 DC 28 00", "M3-L, bits left 7" },
        /* 12*/ { "1234567890123456789012", 1, 0, "2C 3D B9 18 A8 18 AC D4 DC 29 00", "M3-L, bits left 3" },
        /* 13*/ { "12345678901234567890123", 1, 0, "2E 3D B9 18 A8 18 AC D4 DC 29 70", "M3-L, bits left 0" },
        /* 14*/ { "123456789012345678", 2, 0, "24 3D B9 18 A8 18 AC D4 C0", "M3-M, bits left 1" },
        /* 15*/ { "ABCDEFGHIJKLMN", 1, 0, "78 E6 A2 95 0A B8 59 EB 99 7E A0", "M3-L, bits left 1" },
        /* 16*/ { "ABCDEFGHIJK", 2, 0, "6C E6 A2 95 0A B8 59 EA 80", "M3-M, bits left 1" },
        /* 17*/ { "1234567890123456789012345678901234", 1, 0, "11 0F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 F6 80", "M4-L, bits left 5" },
        /* 18*/ { "12345678901234567890123456789012345", 1, 0, "11 8F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 F6 B4", "M4-L, bits left 1" },
        /* 19*/ { "123456789012345ABCDEFGHIJK", 1, 0, "07 8F 6E 46 2A 06 2B 25 67 35 14 A8 55 C2 CF 54", "M4-L, bits left 0" },
        /* 20*/ { "123456789012345678901234567890", 2, 0, "0F 0F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0", "M4-M, bits left 3" },
        /* 21*/ { "123456789012345678901", 3, 0, "0A 8F 6E 46 2A 06 2B 35 37 0A", "M4-Q, bits left 1" },
        /* 22*/ { "ABCDEFGHIJKLMNOPQRSTU", 1, 0, "35 39 A8 A5 42 AE 16 7A E6 5F AC 51 95 B4 25 E0", "M4-L, bits left 4" },
        /* 23*/ { "ABCDEFGHIJKLMNOPQR", 2, 0, "32 39 A8 A5 42 AE 16 7A E6 5F AC 51 95 A0", "M4-M, bits left 5" },
        /* 24*/ { "ABCDEFGHIJKLM", 3, 0, "2D 39 A8 A5 42 AE 16 7A E6 56", "M4-Q, bits left 0" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
        symbol->input_mode = UNICODE_MODE;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_MICROQR_PADDING_GENERATE_EXPECTED
        printf("        /*%3d*/ { \"%s\", %d, %s, \"%s\", \"%s\" },\n",
                i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].option_1,
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

static void test_microqr_optimize(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int option_1;
        int option_2;
        int ret;
        char* expected;
        char* comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "1", -1, -1, 0, "22 00 00", "N1" },
        /*  1*/ { UNICODE_MODE, "A123", 1, 2, 0, "92 86 3D 80 EC", "A1 N3" },
        /*  2*/ { UNICODE_MODE, "AAAAAA", 1, -1, 0, "E3 98 73 0E 60", "A6" },
        /*  3*/ { UNICODE_MODE, "AA123456", 1, -1, 0, "A3 98 61 ED C8", "A2 N6" },
        /*  4*/ { UNICODE_MODE, "01a", 1, 3, 0, "04 06 16 10 00 EC 11 EC 11 EC 00", "N3 B1" },
        /*  5*/ { UNICODE_MODE, "01a", 1, 4, 0, "43 30 31 61 00 00 EC 11 EC 11 EC 11 EC 11 EC 11", "B3" },
        /*  6*/ { UNICODE_MODE, "こんwa、αβ", 1, -1, 0, "46 82 B1 82 F1 77 61 66 00 10 FF 88 00 00 EC 11", "B6 K3" },
        /*  7*/ { UNICODE_MODE, "こんにwa、αβ", 1, -1, 0, "66 13 10 B8 85 25 09 DD 85 98 00 43 FE 20 00 00", "K3 B2 K3" },
        /*  8*/ { UNICODE_MODE, "こんAB123\177", 1, 3, 0, "D0 4C 42 E2 91 CD 06 3D C2 FE 00", "K2 A2 N3 B1" },
        /*  9*/ { UNICODE_MODE, "こんAB123\177", 1, 4, 0, "64 13 10 B8 92 9C D0 5E 1A 0B F8 00 EC 11 EC 11", "K2 A5 B1" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        symbol->debug = ZINT_DEBUG_TEST;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_MICROQR_OPTIMIZE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %d, %d, %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].option_1, data[i].option_2,
                testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        #else
        assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_microqr_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "12345", -1, -1, 0, 11, 11, "Max capacity M1 5 numbers",
                    "11111110101"
                    "10000010110"
                    "10111010100"
                    "10111010000"
                    "10111010111"
                    "10000010011"
                    "11111110100"
                    "00000000011"
                    "11001110011"
                    "01010001100"
                    "11110000011"
               },
        /*  1*/ { UNICODE_MODE, "1234567890", -1, -1, 0, 13, 13, "Max capacity M2-L 10 numbers",
                    "1111111010101"
                    "1000001010110"
                    "1011101010001"
                    "1011101011011"
                    "1011101000110"
                    "1000001011101"
                    "1111111010011"
                    "0000000011101"
                    "1101101010100"
                    "0111100000010"
                    "1111001001101"
                    "0110010100110"
                    "1001101111111"
               },
        /*  2*/ { UNICODE_MODE, "12345678", 2, -1, 0, 13, 13, "Max capacity M2-M 8 numbers",
                    "1111111010101"
                    "1000001011000"
                    "1011101011101"
                    "1011101001101"
                    "1011101001000"
                    "1000001010001"
                    "1111111000001"
                    "0000000000111"
                    "1110011110000"
                    "0111000000000"
                    "1110010000111"
                    "0101011001100"
                    "1100110101001"
               },
        /*  3*/ { UNICODE_MODE, "12345678901234567890123", -1, -1, 0, 15, 15, "Max capacity M3-L 23 numbers",
                    "111111101010101"
                    "100000100110110"
                    "101110100011111"
                    "101110100100110"
                    "101110101101010"
                    "100000101010111"
                    "111111101111110"
                    "000000001000010"
                    "111101100000100"
                    "011110110100111"
                    "110111110001111"
                    "001111011000101"
                    "110000101011000"
                    "010011000101101"
                    "100111010001111"
               },
        /*  4*/ { UNICODE_MODE, "123456789012345678", 2, -1, 0, 15, 15, "Max capacity M3-L 18 numbers",
                    "111111101010101"
                    "100000100010110"
                    "101110101101111"
                    "101110101110110"
                    "101110101011010"
                    "100000101000111"
                    "111111100001110"
                    "000000001110010"
                    "100001101110100"
                    "010101111000111"
                    "101010111001111"
                    "001101001000100"
                    "110001101011001"
                    "001110000101101"
                    "110011111001111"
               },
        /*  5*/ { UNICODE_MODE, "12345678901234567890123456789012345", -1, -1, 0, 17, 17, "Max capacity M4-L 35 numbers",
                    "11111110101010101"
                    "10000010111010001"
                    "10111010000011001"
                    "10111010101000100"
                    "10111010000011001"
                    "10000010111100001"
                    "11111110101011100"
                    "00000000000111001"
                    "10010111011011001"
                    "00101001011010011"
                    "11101001100101000"
                    "00100011110000100"
                    "11101101010110010"
                    "01100001101110010"
                    "11100000000010011"
                    "01110010010111010"
                    "11001001111110111"
               },
        /*  6*/ { UNICODE_MODE, "123456789012345678901234567890", 2, -1, 0, 17, 17, "Max capacity M4-M 30 numbers",
                    "11111110101010101"
                    "10000010011010001"
                    "10111010000011001"
                    "10111010001000100"
                    "10111010100011001"
                    "10000010011100001"
                    "11111110001011100"
                    "00000000000111001"
                    "10100101011011001"
                    "01000111000010011"
                    "11001000110101000"
                    "00101011111000100"
                    "10100001001110011"
                    "00000001010110011"
                    "11010010101010000"
                    "00001111000111000"
                    "11100110111110111"
               },
        /*  7*/ { UNICODE_MODE, "123456789012345678901", 3, -1, 0, 17, 17, "Max capacity M4-Q 21 numbers",
                    "11111110101010101"
                    "10000010010101101"
                    "10111010010010101"
                    "10111010100010111"
                    "10111010000101010"
                    "10000010110001101"
                    "11111110010010000"
                    "00000000101101010"
                    "10110001110101010"
                    "00000010001001111"
                    "10011101011110100"
                    "00001100000100111"
                    "11111001110010001"
                    "01110100011101101"
                    "11110001010001110"
                    "00000001110011011"
                    "11011110011010100"
               },
        /*  8*/ { UNICODE_MODE, "点茗テ点茗テ点茗テ", -1, -1, 0, 17, 17, "Max capacity M4-L 9 Kanji",
                    "11111110101010101"
                    "10000010111110010"
                    "10111010000011101"
                    "10111010110011010"
                    "10111010001011001"
                    "10000010110110110"
                    "11111110101100100"
                    "00000000010011011"
                    "10010111011110100"
                    "00001010100100110"
                    "11101010001000010"
                    "00010111101000010"
                    "11000100010110000"
                    "01001111010011101"
                    "10000100101100011"
                    "01011000000010111"
                    "11001111011101001"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_MICROQR_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %d, %d, %s, %d, %d, \"%s\",\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].data, data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
                symbol->rows, symbol->width, data[i].comment);
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

static void test_upnqr_input(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int ret;
        char* expected;
        char* comment;
    };
    // Ą U+0104 in ISO 8859-2 0xA1, in other ISO 8859 and Win 1250, UTF-8 C484
    // Ŕ U+0154 in ISO 8859-2 0xC0, in Win 1250 but not other ISO 8859 or Win page, UTF-8 C594
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "ĄŔ", 0, "(415) 70 44 00 02 A1 C0 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC", "ECI-4 B2 (ISO 8859-2)" },
        /*  1*/ { UNICODE_MODE, "é", 0, "(415) 70 44 00 01 E9 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11", "ECI-4 B1 (ISO 8859-2)" },
        /*  2*/ { UNICODE_MODE, "β", ZINT_ERROR_INVALID_DATA, "Error 572: Invalid characters in input data", "β not in ISO 8859-2" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_UPNQR;
        symbol->input_mode = data[i].input_mode;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);
        assert_equal(symbol->eci, 4, "i:%d ZBarcode_Encode symbol->eci %d != 4\n", i, symbol->eci);

        #ifdef TEST_UPNQR_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_upnqr_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };

    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "Ą˘Ł¤ĽŚ§¨ŠŞŤŹŽŻ°ą˛ł´ľśˇ¸šşťź˝žżŔÁÂĂÄĹĆÇČÉĘËĚÍÎĎĐŃŇÓÔŐÖ×ŘŮÚŰÜÝŢßŕáâăäĺćçčéęëěíîďđńňóôőö÷řůúűüýţ˙", -1, -1, 0, 77, 77, "ISO 8859-2",
                    "11111110000111101001000110101100101001111110111011001111111000110000001111111"
                    "10000010001011100100110111111011110100001011110000100001001110011010101000001"
                    "10111010110101111111101101111101001010110101111011011110100001100100101011101"
                    "10111010100110111001110101101000011101101000110011100111001111101000101011101"
                    "10111010100110010010011011111101001001111101101111101001101111001011101011101"
                    "10000010101011011010010010001010010101101010111000100011001110100010001000001"
                    "11111110101010101010101010101010101010101010101010101010101010101010101111111"
                    "00000000110101001100101010001100001011110010011000111100111000000000100000000"
                    "10111110010111011110010011111001111100101111111111100001010110011010001111100"
                    "10000001110110110001011001111100101000111010111001001101111100111000111100000"
                    "11111110110010111100110011101011110100001111110110101111010110000010001011010"
                    "00001100011100001010101111010101001111110100011101010110101000110000010100000"
                    "11010011100111011010010001110000010010101001110101111101001110000010101001010"
                    "10000100100110010010011000011101010111111100111011010111100100100001011100000"
                    "11101110110010111100110111110010001010001011110000110001011110000011101011010"
                    "00001100011110101011101111011101101010010010011011011010110000111001110100000"
                    "11011111111100011010010101110001111101001111110011000011010110000010001001010"
                    "10010000100101010010011100001100101000011010111000101001111100101000111100010"
                    "11110110111100011100110011100011110101001111110110101011110110000010001111001"
                    "00111100010001101011101111000010101111100010001101110100011000111100110010011"
                    "11000111111100011010010111101111011010110111100110111100110110000111111111001"
                    "10000000111001110010011100010010001111100010101011110110011100101010111010000"
                    "11111110100100111100110001101111110010010111110011111000010110000001110010010"
                    "00111000000011101011111101001011010100010010000101011010111000111101011100000"
                    "11111111110110011010001011111110000001001111101111101011010110000001111110010"
                    "11101000100001111010011010001011010000011010101000111001111100101111100011100"
                    "10001010111100101100110110101110000101001111111010100011110100000101101010010"
                    "01011000101001110011101110001011001000000011011000111000011011111100100010101"
                    "01101111101010001010011011111110011011010110011111111010110001000100111111001"
                    "01100101111101101010010101101011001010000011000101100000011011001101101001110"
                    "01010110111100110000110110010110010101010110010101001010110010000010010101111"
                    "11110000101111101011011001101011010110001010000000110000011001111111001010010"
                    "01000010001010001010001111001110000011000111110110011011110110100100110110101"
                    "10110101111001101010100101101011010110011010100110010000111100101110101101110"
                    "01111110011100110001111110010110000011001111101000111011010111001000001100100"
                    "01111101101001101001101001001011010110011001011110010000001101110011100101110"
                    "00011110001000001001011110001110000011001100001101111010100101111010100000000"
                    "00110001111011111101110101101011010110001101010111110001000101110001110010010"
                    "00111111111101100001110110010110000011011010010001011011101100110010000101001"
                    "11111100101101010011000001101111010110000101100110110000000001011101110110011"
                    "10011110101001010101000111101100000011010000010100011010111001100100100011001"
                    "00110000111011111100100100001011010110000110100110010000011001001111101001110"
                    "00111111111100101111110110110110000011010111101000111010101100100000011000101"
                    "00111100101101001100100001001100010111100010111110010100011110000111101111111"
                    "00011110101000110101001110001101100010010110101100111110110110001100110010001"
                    "00110001011010010000110111100000110110000011110110010000010110101111110111110"
                    "11111111111100101001111011111011100010010110101111111110110100001000111110101"
                    "00111000101101001011000110001111010111100101001000110110000011110110100011111"
                    "00011010110001110111001110101000000010110000001010111000101001111101101010001"
                    "00111000100011110100110010001001010110100101011000110100000011011111100011110"
                    "11111111100101101100111011111000000011010010001111110110111000101000111110101"
                    "00111101101101001000000100110000001010000011011000111000010011001111100011111"
                    "00011111100000110000101100100101110101010110000110000010110001110101010110001"
                    "00111100100011110000110010000100100000000011001100101000011011000110100011110"
                    "11000010000101001110011100001101110101010110000110001010110000101001010110101"
                    "00101100101110101001000110101000101111100101011000110100001011010111100011111"
                    "00011110001010110000101011101111111010110000000110011100101001111101110110001"
                    "00011100001000010000110010100000101111100101001100110110000011010110000011100"
                    "11000011000101101110011100010011100010010010000100010000011000111000110110110"
                    "00000101111010001001000010001001010010010101011010111010100011010110000011100"
                    "00011110011001110000101100100101000101001000000000000011001001111110010110010"
                    "00111100000101110000110101000000010000011101001010101001100011010010100011010"
                    "11100110111101101110001011001111100101001010000010000011111000111101010111001"
                    "00010001110110000001001101010000001001100101011000111100000011010111100011111"
                    "10010010000001111000111010111101111000110000000110001100101001111001010110101"
                    "10010000000111110000111011001000101001100101000101111110000011010001100011110"
                    "01001110100011111110011101000111110100010010001101001000011010111010110110010"
                    "00001001111000001001000001001000010111110100010010100110100001010111100010101"
                    "01111010010011110000101011111101100010101001101111111101001111011101111110101"
                    "00000000110101101000111110001000110011111100101000100111100101010101100011101"
                    "11111110001101111010001010101111100100001011101010100001011101011010101011100"
                    "10000010111110000001101010001000001011111101001000111110000011010001100010010"
                    "10111010110101110101011111111101111100100000011111100101101000111011111111101"
                    "10111010110001100110111110110000101001111101010001101110100010110000010010010"
                    "10111010101011111111010111011111110100001010011011000001111000110011100101010"
                    "10000010011100010001111111000000001011100000001000111111100001011101111000001"
                    "11111110110101111101111001011101111100101101111101100101000101100100011101000"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_UPNQR;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_UPNQR_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %d, %d, %s, %d, %d, \"%s\",\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].data, data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
                symbol->rows, symbol->width, data[i].comment);
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

static void test_rmqr_options(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int option_1;
        int option_2;
        int ret_encode;
        int ret_vector;
        int expected_rows;
        int expected_width;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "12345", -1, -1, 0, 0, 11, 27 }, // ECC auto-set to H, version auto-set to 11 (R11x27)
        /*  1*/ { "12345", 4, 11, 0, 0, 11, 27 },
        /*  2*/ { "12345", 1, -1, ZINT_ERROR_INVALID_OPTION, -1, 0, 0 }, // ECC L not available
        /*  3*/ { "12345", 3, -1, ZINT_ERROR_INVALID_OPTION, -1, 0, 0 }, // ECC Q not available
        /*  4*/ { "123456789", 4, 11, 0, 0, 11, 27 }, // Max capacity ECC H, version 11, 9 numbers
        /*  5*/ { "1234567890", 4, 11, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /*  6*/ { "12345678901234", 2, 11, 0, 0, 11, 27 }, // Max capacity ECC M, version 11, 14 numbers
        /*  7*/ { "123456789012345", 2, 11, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /*  8*/ { "ABCDEF", 4, 11, 0, 0, 11, 27 }, // Max capacity ECC H, version 11, 6 letters
        /*  9*/ { "ABCDEFG", 4, 11, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 10*/ { "ABCDEFGH", 2, 11, 0, 0, 11, 27 }, // Max capacity ECC M, version 11, 8 letters
        /* 11*/ { "ABCDEFGHI", 2, 11, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 12*/ { "\177\177\177\177", 4, 11, 0, 0, 11, 27 }, // Max capacity ECC H, version 11, 4 bytes
        /* 13*/ { "\177\177\177\177\177", 4, 11, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 14*/ { "\177\177\177\177\177\177", 2, 11, 0, 0, 11, 27 }, // Max capacity ECC M, version 11, 6 bytes
        /* 15*/ { "\177\177\177\177\177\177\177", 2, 11, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 16*/ { "点茗", 4, 11, 0, 0, 11, 27 }, // Max capacity ECC H, version 11, 2 kanji
        /* 17*/ { "点茗点", 4, 11, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 18*/ { "点茗点", 2, 11, 0, 0, 11, 27 }, // Max capacity ECC M, version 11, 3 kanji
        /* 19*/ { "点茗点茗", 2, 11, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 20*/ { "12345", -1, 1, 0, 0, 7, 43 }, // ECC auto-set to M, version 1 (R7x43)
        /* 21*/ { "12345", 2, 1, 0, 0, 7, 43 },
        /* 22*/ { "12345", 4, 1, 0, 0, 7, 43 }, // Max capacity ECC H, version 1, 5 numbers
        /* 23*/ { "123456", 4, 1, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 24*/ { "123456789012", 2, 1, 0, 0, 7, 43 }, // Max capacity ECC M, version 1, 12 numbers
        /* 25*/ { "1234567890123", 2, 1, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 26*/ { "ABC", 4, 1, 0, 0, 7, 43 }, // Max capacity ECC H, version 1, 3 letters
        /* 27*/ { "ABCD", 4, 1, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 28*/ { "ABCDEFG", 2, 1, 0, 0, 7, 43 }, // Max capacity ECC M, version 1, 7 letters
        /* 29*/ { "ABCDEFGH", 2, 1, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 30*/ { "\177\177", 4, 1, 0, 0, 7, 43 }, // Max capacity ECC H, version 1, 2 bytes
        /* 31*/ { "\177\177\177", 4, 1, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 32*/ { "\177\177\177\177\177", 2, 1, 0, 0, 7, 43 }, // Max capacity ECC M, version 1, 5 bytes
        /* 33*/ { "\177\177\177\177\177\177", 2, 1, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 34*/ { "点", 4, 1, 0, 0, 7, 43 }, // Max capacity ECC H, version 1, 1 kanji
        /* 35*/ { "点茗", 4, 1, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 36*/ { "点茗点", 2, 1, 0, 0, 7, 43 }, // Max capacity ECC M, version 1, 3 kanji
        /* 37*/ { "点茗点茗", 2, 1, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 38*/ { "12345678901234567890123", 4, 7, 0, 0, 9, 59 }, // Max capacity ECC H, version 7 (R9x59), 23 numbers
        /* 39*/ { "123456789012345678901234", 4, 7, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 40*/ { "点茗点茗点茗", 4, 7, 0, 0, 9, 59 }, // Max capacity ECC H, version 7, 6 kanji
        /* 41*/ { "点茗点茗点茗点", 4, 7, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 42*/ { "点茗点茗点茗点茗", 4, 13, 0, 0, 11, 59 }, // Max capacity ECC H, version 13 (R11x59), 8 kanji
        /* 43*/ { "点茗点茗点茗点茗点", 4, 13, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 44*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点", 4, 20, 0, 0, 13, 77 }, // Max capacity ECC H, version 20 (R13x77), 17 kanji
        /* 45*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗", 4, 20, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 46*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点点茗点茗点茗点", 4, 26, 0, 0, 15, 99 }, // Max capacity ECC H, version 26 (R15x99), 24 kanji
        /* 47*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点点茗", 4, 26, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 48*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", 4, 32, 0, 0, 17, 139 }, // Max capacity ECC H, version 32 (R17x139), 46 kanji
        /* 49*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", 4, 32, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 50*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", -1, 32, 0, 0, 17, 139 }, // ECC auto-set to M, version 32
        /* 51*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", -1, 32, 0, 0, 17, 139 }, // Max capacity ECC M, version 32, 92 kanji
        /* 52*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", 4, 32, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 53*/ { "点茗点", -1, 33, 0, 0, 7, 43 }, // ECC auto-set to M, version 33 (R7xAuto-width) auto-sets R7x43
        /* 54*/ { "点茗点", 4, 33, 0, 0, 7, 59 }, // ECC set to H, version 33 (R7xAuto-width) auto-sets R7x59
        /* 55*/ { "点茗点", -1, 34, 0, 0, 9, 43 }, // ECC auto-set to H, version 34 (R9xAuto-width) auto-sets R9x43
        /* 56*/ { "点茗点", -1, 35, 0, 0, 11, 27 }, // ECC auto-set to M, version 35 (R11xAuto-width) auto-sets R11x27
        /* 57*/ { "点茗点茗点茗点", 4, 35, 0, 0, 11, 59 }, // ECC set to H, version 35 (R11xAuto-width) auto-sets R11x59
        /* 58*/ { "点茗点茗点茗点", -1, 35, 0, 0, 11, 43 }, // ECC auto-set to M, version 35 (R11xAuto-width) auto-sets R11x43
        /* 59*/ { "点茗点茗点茗点茗", -1, 36, 0, 0, 13, 43 }, // ECC auto-set to M, version 36 (R13xAuto-width) auto-sets R13x43
        /* 60*/ { "点茗点茗点茗点茗", 4, 36, 0, 0, 13, 59 }, // ECC set to H, version 36 (R13xAuto-width) auto-sets R13x59
        /* 61*/ { "点茗点茗点茗点茗点", -1, 37, 0, 0, 15, 43 }, // ECC auto-set to M, version 37 (R15xAuto-width) auto-sets R15x43
        /* 62*/ { "点茗点茗点茗点茗点", 4, 37, 0, 0, 15, 59 }, // ECC set to H, version 37 (R15xAuto-width) auto-sets R15x59
        /* 63*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗", -1, 38, 0, 0, 17, 43 }, // ECC auto-set to M, version 38 (R17xAuto-width) auto-sets R17x43
        /* 64*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗", 4, 38, 0, 0, 17, 77 }, // ECC set to H, version 38 (R17xAuto-width) auto-sets R17x77
        /* 65*/ { "点茗点", -1, 39, ZINT_ERROR_INVALID_OPTION, -1, 0, 0 },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_RMQR;
        symbol->input_mode = UNICODE_MODE;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }
        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

        if (data[i].ret_vector != -1) {
            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_input(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int ret;
        char* expected;
        char* comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS, UTF-8 E0B881
    // Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447, UTF-8 D096
    // ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS, UTF-8 E0BA81
    // ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5
    // ･ U+FF65 half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xA5 (\245), UTF-8 EFBDA5
    // ¿ U+00BF in ISO 8859-1 0xBF (\277), not in Shift JIS; 0xBF same codepoint as single-byte half-width katakana ｿ (U+FF7F) in Shift JIS (below), UTF-8 C2BF
    // ｿ U+FF7F half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xBF (\277), UTF-8 EFBDBF
    // ‾ U+203E overline, not in ISO/Win, in Shift JIS single-byte 0x7E (\176) (tilde), UTF-8 E280BE
    // 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9
    // 茗 U+8317 kanji, in Shift JIS 0xE4AA (\344\252), UTF-8 E88C97
    // テ U+30C6 katakana, in Shift JIS 0x8365 (\203\145), UTF-8 E38386
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "é", 0, "67 A4 00 EC 11", "B1 (ISO 8859-1)" },
        /*  1*/ { DATA_MODE, "é", 0, "6B 0E A4 00 EC", "B2 (UTF-8)" },
        /*  2*/ { DATA_MODE, "\351", 0, "67 A4 00 EC 11", "B1 (ISO 8859-1)" },
        /*  3*/ { UNICODE_MODE, "β", 0, "88 80 00 EC 11", "K1 (Shift JIS)" },
        /*  4*/ { UNICODE_MODE, "ก", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input data", "ก not in ISO 8859-1 or Shift JIS" },
        /*  5*/ { UNICODE_MODE, "Ж", 0, "88 91 C0 EC 11", "K1 (Shift JIS)" },
        /*  6*/ { UNICODE_MODE, "¥･点", 0, "71 72 96 4D 7C", "B2 K1 (Shift JIS) (optimized to byte mode only)" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_RMQR;
        symbol->input_mode = data[i].input_mode;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_RMQR_INPUT_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_gs1(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int ret;
        char* expected;
        char* comment;
    };
    struct item data[] = {
        /*  0*/ { "[01]12345678901234", 0, "A6 00 59 D5 1B EF 43 DA 00 EC 11 EC", "N16" },
        /*  1*/ { "[01]04912345123459[15]970331[30]128[10]ABC123", 0, "A5 D0 29 EB 3A A0 05 67 93 F2 94 B4 B4 E2 4E AF 01 47 EC 17 86", "N29 A9" },
        /*  2*/ { "[91]12%[20]12", 0, "A4 9C 79 32 25 1D 24 32 48 00 EC 11", "N4 B2 N4" },
        /*  3*/ { "[91]123%[20]12", 0, "A4 BC 79 74 3D A9 31 21 92 40 EC 11", "N5 A2 N4" },
        /*  4*/ { "[91]1234%[20]12", 0, "A4 DC 79 D4 C8 94 74 90 C9 20 EC 11", "N6 B2 N4" },
        /*  5*/ { "[91]12345%[20]12", 0, "A4 FC 79 D4 A8 7B 52 62 43 24 80 EC", "N7 A2(3) N4" },
        /*  6*/ { "[91]1A%[20]12", 0, "A8 E6 58 1B ED 49 89 0C 92 00 EC 11", "A6(7) N4" },
        /*  7*/ { "[91]%%[20]12", 0, "A4 56 D9 92 92 8E 92 19 24 00 EC 11", "N2 B3 N4" },
        /*  8*/ { "[91]%%%[20]12", 0, "A4 56 DA 12 92 92 8E 92 19 24 00 EC", "N2 B4 N4" },
        /*  9*/ { "[91]A%%%%12345678A%A", 0, "A8 A6 58 F4 4C C6 4A 4A 4A 48 1E DC 89 C8 87 A3 5C 00 EC", "A4(5) B3 N8 A3(4)" },
        /* 10*/ { "[91]%23%%6789%%%34567%%%%234%%%%%", 0, "(33) AA 63 2D B5 02 EE D4 DA 84 54 B8 ED 4D A9 B5 04 58 E7 2C 3B 53 6A 6D 4D 60 22 F6 A3", "A27(38) B4" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_RMQR;
        symbol->input_mode = GS1_MODE;
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        #ifdef TEST_RMQR_GS1_GENERATE_EXPECTED
        printf("        /*%3d*/ { \"%s\", %s, \"%s\", \"%s\" },\n",
                i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        #else
        if (ret < 5) {

            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_optimize(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int option_1;
        int ret;
        char* expected;
        char* comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "1", -1, 0, "21 10 EC 11 EC", "N1" },
        /*  1*/ { UNICODE_MODE, "AAA", -1, 0, "46 73 0A 00 EC", "A3" },
        /*  2*/ { UNICODE_MODE, "0123456789", -1, 0, "2A 03 15 9A 9A 40 EC", " N10 (nayuki.io - pure numeric)" },
        /*  3*/ { UNICODE_MODE, "ABCDEF", -1, 0, "4C 73 51 4A 85 00 EC", "A6 (nayuki.io - pure alphanumeric)" },
        /*  4*/ { UNICODE_MODE, "wxyz", -1, 0, "71 DD E1 E5 E8", "B4 (nayuki.io - pure byte)" },
        /*  5*/ { UNICODE_MODE, "「魔法少女まどか☆マギカ」って、　ИАИ　ｄｅｓｕ　κα？", -1, 0, "(53) 8E 80 D6 00 4F C0 57 6A B5 C2 B8 14 70 94 81 64 37 A1 8D 0C 50 0D 82 82 14 40 00 80", "K29 (nayuki.io - pure kanji)" },
        /*  6*/ { UNICODE_MODE, "012A", -1, 0, "48 00 43 20 EC", "A4" },
        /*  7*/ { UNICODE_MODE, "0123A", -1, 0, "24 03 0D 09 40", "N4 A1 (nayuki.io - alpha/numeric)" },
        /*  8*/ { UNICODE_MODE, "0a", -1, 0, "68 C1 84 00 EC", "B2 (nayuki.io - numeric/byte)" },
        /*  9*/ { UNICODE_MODE, "01a", -1, 0, "22 02 CB 08 EC", "N2 B1 (nayuki.io - numeric/byte)" },
        /* 10*/ { UNICODE_MODE, "ABCa", -1, 0, "71 05 09 0D 84", "B4 (nayuki.io - alphanumeric/byte)" },
        /* 11*/ { UNICODE_MODE, "ABCDa", -1, 0, "48 73 51 4B 2C 20 EC", "A4 B1 (same bits as B5)" },
        /* 12*/ { UNICODE_MODE, "THE SQUARE ROOT OF 2 IS 1.41421356237309504880168872420969807856967187537694807317667973799", -1, 0, "(48) 46 A9 52 9A A0 D5 42 66 E6 F8 A1 4F 62 3E 56 CC D4 40 2B 98 2C F1 AB 19 2E A2 F8 61", " A26 N65 (nayuki.io - alpha/numeric)" },
        /* 13*/ { UNICODE_MODE, "Golden ratio φ = 1.6180339887498948482045868343656381177203091798057628621354486227052604628189024497072072041893911374......", -1, 0, "(73) 65 11 DB DB 19 19 5B 88 1C 98 5D 1A 5B C8 20 F4 C8 0F 48 0C 4B 8B 24 D4 10 FB 97 6E", "B20 N100 A6 (nayuki.io - alpha/numeric/byte)" },
        /* 14*/ { UNICODE_MODE, "こんにちwa、世界！ αβγδ", -1, 0, "84 09 88 5C 42 92 13 F6 B7 76 18 14 19 0A 28 A4 58 14 92 08 40 FF 88 00 40 22 02", "K4 B7 K4 (nayuki.io - kanji/european)" },
        /* 15*/ { UNICODE_MODE, "6547861663com.acme35584af52fa3-88d0-093b-6c14-b37ddafb59c528908608sg.com.dash.www05303790265903SG.COM.NETS46967004B2AE13344004SG.SGQR209710339382.6359SG8236HELLO FOO2517Singapore3272B815", -1, 0, "(152) 20 AA 3B 12 29 8D 97 B1 B7 B6 97 30 B1 B6 B2 99 9A 9A 9C 1A 30 B3 1A 99 33 30 99 96", "N10 B47 N9 B15 N14 A38 N12 A25 B8 A8 (nayuki.io - SGQR alpha/numeric/byte)" },
        /* 16*/ { UNICODE_MODE, "2004年大西洋颶風季是有以来造成人失惨重的大季之一，于2004年6月1日正式始，同年11月30日束，上的日期界定了一年中大多数气旋在大西洋形成的段lll ku", -1, 0, "(127) 20 43 21 21 87 27 32 95 77 90 AD F0 33 D5 CF 0E BA 58 46 17 22 19 0C 62 5D 62 DB 14", "N4 K24 N4 B6 K7 B6 K26 B6 (nayuki.io - kanji/byte/numeric, non-Shift JIS removed)" },
        /* 17*/ { UNICODE_MODE, "AB1234567A", -1, 0, "4A 39 A0 BC 45 8E 72 8A 00 EC 11 EC", "A10" },
        /* 18*/ { UNICODE_MODE, "AB12345678A", -1, 0, "42 39 A5 03 DB 91 39 04 A0 EC 11 EC", "A2 N8 A1" },
        /* 19*/ { UNICODE_MODE, "テaA1", -1, 0, "76 0D 95 85 04 C4 00", "B4" },
        /* 20*/ { UNICODE_MODE, "テaAB1", -1, 0, "6E 0D 95 85 19 CD 04", "B3 A3" },
        /* 21*/ { UNICODE_MODE, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機", -1, 0, "(152) 82 0E A2 16 20 97 28 BD 02 C1 4F 09 12 61 08 04 A0 83 AA 3E 3D 43 4C 13 0D 68 73 1F", "K8 N1 K8 N1 K10 N2 K33 N2 K16 N1 K7" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    char escaped[1024];

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_RMQR;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        symbol->debug = ZINT_DEBUG_TEST;

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_RMQR_OPTIMIZE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %d, %s, \"%s\", \"%s\" },\n",
                i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].option_1,
                testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        #else
        assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        #endif

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_encode(void)
{
    testStart("");

    int ret;
    struct item {
        int input_mode;
        unsigned char* data;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char* comment;
        char* expected;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "0123456", 4, 11, 0, 11, 27, "Draft ISO 2018-6-8 Annex H I.2, currently no image to compare to",
                    "111111101010101010101010111"
                    "100000100110111010101101101"
                    "101110100001100000010110101"
                    "101110101111100000001001010"
                    "101110100100011000101000111"
                    "100000101110011000111110010"
                    "111111100110100101001011111"
                    "000000001101001111010010001"
                    "111000011011111000011110101"
                    "100100100010000011111010001"
                    "111010101010101010101011111"
               },
        /*  1*/ { UNICODE_MODE, "1234567890123456", 2, 17, 0, 13, 27, "Draft ISO 2018-6-8 6.2 Figure 1 **NOT SAME**",
                    "111111101010101010101010111"
                    "100000100001111000111110101"
                    "101110101100010111110111001"
                    "101110100110001110010100000"
                    "101110101111001001101001001"
                    "100000100010001110011110000"
                    "111111100000001100000010101"
                    "000000001111100000010001100"
                    "111001101110000001110011111"
                    "011111011101010000100010001"
                    "100000101000011010101010101"
                    "101101101100110111000010001"
                    "111010101010101010101011111"
               },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    for (int i = 0; i < data_size; i++) {

        struct zint_symbol* symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_RMQR;
        symbol->input_mode = data[i].input_mode;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        if (data[i].option_2 != -1) {
            symbol->option_2 = data[i].option_2;
        }

        int length = strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        #ifdef TEST_RMQR_ENCODE_GENERATE_EXPECTED
        printf("        /*%3d*/ { %s, \"%s\", %d, %d, %s, %d, %d, \"%s\",\n",
                i, testUtilInputModeName(data[i].input_mode), data[i].data, data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
                symbol->rows, symbol->width, data[i].comment);
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
    test_qr_options();
    test_qr_input();
    test_qr_gs1();
    test_qr_optimize();
    test_qr_encode();

    test_microqr_options();
    test_microqr_input();
    test_microqr_padding();
    test_microqr_optimize();
    test_microqr_encode();

    test_upnqr_input();
    test_upnqr_encode();

    test_rmqr_options();
    test_rmqr_input();
    test_rmqr_gs1();
    test_rmqr_optimize();
    test_rmqr_encode();

    testReport();

    return 0;
}
