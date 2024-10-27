/*
    libzint - the open source barcode library
    Copyright (C) 2019-2024 Robin Stuart <rstuart114@gmail.com>

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
/* SPDX-License-Identifier: BSD-3-Clause */

#include "testcommon.h"

static void test_qr_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_1;
        int option_2;
        int option_3;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 1, 32, -1, "A", 2840, 0, 145, 145, "" },
        /*  1*/ { 1, 32, -1, "A", 2841, ZINT_ERROR_TOO_LONG, -1, -1, "Error 569: Input too long for Version 32-L, requires 1956 codewords (maximum 1955)" },
        /*  2*/ { 1, 33, -1, "A", 3009, 0, 149, 149, "" },
        /*  3*/ { 1, 33, -1, "A", 3010, ZINT_ERROR_TOO_LONG, -1, -1, "Error 569: Input too long for Version 33-L, requires 2072 codewords (maximum 2071)" },
        /*  4*/ { 1, 34, -1, "A", 3183, 0, 153, 153, "" },
        /*  5*/ { 1, 34, -1, "A", 3184, ZINT_ERROR_TOO_LONG, -1, -1, "Error 569: Input too long for Version 34-L, requires 2192 codewords (maximum 2191)" },
        /*  6*/ { 1, 35, -1, "A", 3351, 0, 157, 157, "" },
        /*  7*/ { 1, 35, -1, "A", 3352, ZINT_ERROR_TOO_LONG, -1, -1, "Error 569: Input too long for Version 35-L, requires 2307 codewords (maximum 2306)" },
        /*  8*/ { 1, 36, -1, "A", 3537, 0, 161, 161, "" },
        /*  9*/ { 1, 36, -1, "A", 3538, ZINT_ERROR_TOO_LONG, -1, -1, "Error 569: Input too long for Version 36-L, requires 2435 codewords (maximum 2434)" },
        /* 10*/ { 1, 37, -1, "A", 3729, 0, 165, 165, "" },
        /* 11*/ { 1, 37, -1, "A", 3730, ZINT_ERROR_TOO_LONG, -1, -1, "Error 569: Input too long for Version 37-L, requires 2567 codewords (maximum 2566)" },
        /* 12*/ { 1, 38, -1, "A", 3927, 0, 169, 169, "" },
        /* 13*/ { 1, 38, -1, "A", 3928, ZINT_ERROR_TOO_LONG, -1, -1, "Error 569: Input too long for Version 38-L, requires 2703 codewords (maximum 2702)" },
        /* 14*/ { 1, 39, -1, "A", 4087, 0, 173, 173, "" },
        /* 15*/ { 1, 39, -1, "A", 4088, ZINT_ERROR_TOO_LONG, -1, -1, "Error 569: Input too long for Version 39-L, requires 2813 codewords (maximum 2812)" },
        /* 16*/ { 1, 40, -1, "A", 4296, 0, 177, 177, "" }, /* ISO/IEC 18004:2015 Section 6.1 (e) (2) */
        /* 17*/ { 1, 40, -1, "A", 4297, ZINT_ERROR_TOO_LONG, -1, -1, "Error 567: Input too long, requires 2957 codewords (maximum 2956)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[ZINT_MAX_DATA_LEN];

    char escaped[ZINT_MAX_DATA_LEN];
    char cmp_buf[177 * 177 + 1];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_qr_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3 | FAST_MODE, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        symbol->input_mode |= FAST_MODE;
        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                char modules_dump[177 * 177 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data_buf, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data_buf, length, debug)) {
                int cmp_len, ret_len;
                char modules_dump[177 * 177 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilZXingCPP(i, symbol, data_buf, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data_buf, length, NULL /*primary*/, escaped, &ret_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        struct zint_structapp structapp;
        char *data;
        int ret_encode;
        int ret_vector;
        int expected_size;
        int compare_previous;
        char *expected;
    };
    /* 貫 U+8CAB kanji, in Shift JIS 0x8AD1 (\212\321), UTF-8 E8B2AB */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, -1, -1, { 0, 0, "" }, "12345", 0, 0, 21, -1, "" }, /* ECC auto-set to 1 (L), version auto-set to 1 */
        /*  1*/ { -1, -1, 5, -1, { 0, 0, "" }, "12345", 0, 0, 21, 0, "" }, /* ECC > 4 ignored */
        /*  2*/ { -1, -1, -1, 41, { 0, 0, "" }, "12345", 0, 0, 21, 0, "" }, /* Version > 40 ignored */
        /*  3*/ { -1, -1, -1, 2, { 0, 0, "" }, "12345", 0, 0, 25, -1, "" }, /* ECC auto-set to 4 (Q), version 2 */
        /*  4*/ { -1, -1, 4, 2, { 0, 0, "" }, "12345", 0, 0, 25, 0, "" }, /* ECC 4 (Q), version 2 */
        /*  5*/ { -1, -1, 1, 2, { 0, 0, "" }, "12345", 0, 0, 25, 1, "" }, /* ECC 1 (L), version 2 */
        /*  6*/ { -1, -1, -1, -1, { 0, 0, "" }, "貫やぐ識禁", 0, 0, 21, -1, "" }, /* ECC auto-set to 1 (L), version auto-set to 1 */
        /*  7*/ { -1, -1, 1, -1, { 0, 0, "" }, "貫やぐ識禁", 0, 0, 21, 0, "" }, /* Version auto-set to 1 */
        /*  8*/ { -1, -1, -1, 1, { 0, 0, "" }, "貫やぐ識禁", 0, 0, 21, 0, "" }, /* ECC auto-set to 1 (L) */
        /*  9*/ { -1, -1, 1, 1, { 0, 0, "" }, "貫やぐ識禁", 0, 0, 21, 0, "" },
        /* 10*/ { -1, -1, 2, 1, { 0, 0, "" }, "貫やぐ識禁", ZINT_ERROR_TOO_LONG, -1, 0, -1, "Error 569: Input too long for Version 1-M, requires 17 codewords (maximum 16)" }, /* ECC 2 (M), version 1 */
        /* 11*/ { -1, -1, 2, -1, { 0, 0, "" }, "貫やぐ識禁", 0, 0, 25, -1, "" }, /* Version auto-set to 2 */
        /* 12*/ { -1, -1, 2, 2, { 0, 0, "" }, "貫やぐ識禁", 0, 0, 25, 0, "" },
        /* 13*/ { -1, -1, 1, 2, { 0, 0, "" }, "貫やぐ識禁", 0, 0, 25, 1, "" },
        /* 14*/ { -1, -1, -1, -1, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 29, -1, "" }, /* ECC auto-set to 1 (L), version auto-set to 3 */
        /* 15*/ { -1, -1, 1, 3, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 29, 0, "" },
        /* 16*/ { -1, -1, 2, -1, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 33, -1, "" }, /* ECC 2 (M), version auto-set to 4 */
        /* 17*/ { -1, -1, 2, 4, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 33, 0, "" },
        /* 18*/ { -1, -1, 3, -1, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 37, -1, "" }, /* ECC 3 (Q), version auto-set to 5 */
        /* 19*/ { -1, -1, 3, 5, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 37, 0, "" },
        /* 20*/ { -1, -1, 4, -1, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 41, -1, "" }, /* ECC 4 (H), version auto-set to 6 */
        /* 21*/ { -1, -1, 4, 6, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 41, 0, "" },
        /* 22*/ { -1, -1, -1, -1, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 0, 0, 69, -1, "" }, /* ECC auto-set to 1, version auto-set to 13 */
        /* 23*/ { -1, -1, 1, 13, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 0, 0, 69, 0, "" },
        /* 24*/ { -1, -1, 4, -1, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 0, 0, 101, -1, "" }, /* ECC 4, version auto-set to 21 */
        /* 25*/ { -1, -1, 4, 21, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 0, 0, 101, 0, "" },
        /* 26*/ { -1, -1, -1, -1, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, 0, 105, -1, "" }, /* ECC auto-set to 1, version auto-set to 22 */
        /* 27*/ { -1, -1, 1, 22, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, 0, 105, 0, "" },
        /* 28*/ { -1, -1, 4, -1, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, 0, 153, 1, "" }, /* ECC 4, version auto-set 34 */
        /* 29*/ { -1, -1, 4, 34, { 0, 0, "" }, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, 0, 153, 0, "" },
        /* 30*/ { -1, -1, 4, -1, { 0, 0, "" }, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, 0, 177, -1, "" }, /* 1852 alphanumerics max for ECC 4 (H) */
        /* 31*/ { -1, -1, 1, -1, { 0, 0, "" }, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, 0, 177, -1, "" }, /* 4296 alphanumerics max for ECC 1 (L) */
        /* 32*/ { -1, -1, 4, -1, { 0, 0, "" }, "貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫", 0, -1, 0, -1, "" }, /* 424 Kanji, ECC 4 (Q), version 1 */
        /* 33*/ { -1, -1, 4, -1, { 0, 0, "" }, "貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫", ZINT_ERROR_TOO_LONG, -1, 0, -1, "Error 561: Input too long for ECC level H, requires 1278 codewords (maximum 1276)" }, /* 425 Kanji, ECC 4 (H), version 1 */
        /* 34*/ { -1, -1, 4, 1, { 0, 0, "" }, "12345678901234567", 0, 0, 21, -1, "" },
        /* 35*/ { -1, -1, 4, 1, { 1, 2, "" }, "12345678901234567", ZINT_ERROR_TOO_LONG, -1, 0, -1, "Error 569: Input too long for Version 1-H, requires 12 codewords (maximum 9)" },
        /* 36*/ { -1, -1, 4, 1, { 1, 2, "" }, "123456789012", ZINT_ERROR_TOO_LONG, -1, 0, -1, "Error 569: Input too long for Version 1-H, requires 10 codewords (maximum 9)" },
        /* 37*/ { -1, -1, 4, 1, { 1, 2, "" }, "12345678901", 0, 0, 21, -1, "" },
        /* 38*/ { -1, -1, 4, 1, { 3, 16, "123" }, "12345678901", 0, 0, 21, -1, "" },
        /* 39*/ { -1, -1, 4, 1, { 3, 17, "123" }, "12345678901", ZINT_ERROR_INVALID_OPTION, -1, 0, -1, "Error 750: Structured Append count '17' out of range (2 to 16)" },
        /* 40*/ { -1, -1, 4, 1, { 3, 2, "123" }, "12345678901", ZINT_ERROR_INVALID_OPTION, -1, 0, -1, "Error 751: Structured Append index '3' out of range (1 to count 2)" },
        /* 41*/ { -1, -1, 4, 1, { 1, 2, "1234" }, "12345678901", ZINT_ERROR_INVALID_OPTION, -1, 0, -1, "Error 752: Structured Append ID length 4 too long (3 digit maximum)" },
        /* 42*/ { -1, -1, 4, 1, { 1, 2, "12A" }, "12345678901", ZINT_ERROR_INVALID_OPTION, -1, 0, -1, "Error 753: Invalid Structured Append ID (digits only)" },
        /* 43*/ { -1, -1, 4, 1, { 1, 2, "256" }, "12345678901", ZINT_ERROR_INVALID_OPTION, -1, 0, -1, "Error 754: Structured Append ID value '256' out of range (0 to 255)" },
        /* 44*/ { GS1_MODE, 3, -1, -1, { 0, 0, "" }, "[20]12", ZINT_WARN_NONCOMPLIANT, 0, 21, -1, "Warning 755: Using ECI in GS1 mode not supported by GS1 standards" },
        /* 45*/ { GS1_MODE, -1, -1, -1, { 1, 2, "" }, "[20]12", ZINT_WARN_NONCOMPLIANT, 0, 21, -1, "Warning 756: Using Structured Append in GS1 mode not supported by GS1 standards" },
        /* 46*/ { GS1_MODE, 3, -1, -1, { 1, 2, "" }, "[20]12", ZINT_WARN_NONCOMPLIANT, 0, 21, -1, "Warning 755: Using ECI in GS1 mode not supported by GS1 standards" }, /* ECI trumps Structured Append */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_symbol previous_symbol;

    testStartSymbol("test_qr_options", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        if (p_ctx->index == -1 && data[i].compare_previous != -1) {
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

static void test_qr_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_3;
        char *data;
        int ret;
        int expected_eci;
        char *expected;
        int bwipp_cmp;
        int zxingcpp_cmp;
        char *comment;
    };
    /* é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9 */
    /* β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2 */
    /* ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS, UTF-8 E0B881 */
    /* Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447, UTF-8 D096 */
    /* ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS, UTF-8 E0BA81 */
    /* ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5 */
    /* ･ U+FF65 half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xA5 (\245), UTF-8 EFBDA5 */
    /* ¿ U+00BF in ISO 8859-1 0xBF (\277), not in Shift JIS; 0xBF same codepoint as single-byte half-width katakana ｿ (U+FF7F) in Shift JIS (below), UTF-8 C2BF */
    /* ｿ U+FF7F half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xBF (\277), UTF-8 EFBDBF */
    /* ‾ U+203E overline, not in ISO/Win, in Shift JIS single-byte 0x7E (\176) (tilde), UTF-8 E280BE */
    /* 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9 */
    /* 茗 U+8317 kanji, in Shift JIS 0xE4AA (\344\252), UTF-8 E88C97 */
    /* テ U+30C6 katakana, in Shift JIS 0x8365 (\203\145), UTF-8 E38386 */
    /* Á U+00C1, UTF-8 C381; ȁ U+0201, UTF-8 C881; Ȃ U+0202, UTF-8 C882; ¢ U+00A2, UTF-8 C2A2; á U+00E1, UTF-8 C3A1 */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, 4, 6 << 8, "é", 0, 0, "40 1E 90 EC 11 EC 11 EC 11", 1, 1, "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, 4, -1, "é", 0, 3, "70 34 01 E9 00 EC 11 EC 11", 1, 1, "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 20, -1, -1, "é", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "é not in Shift JIS" },
        /*  3*/ { UNICODE_MODE, 26, 4, -1, "é", 0, 26, "71 A4 02 C3 A9 00 EC 11 EC", 1, 1, "ECI-26 B2 (UTF-8)" },
        /*  4*/ { DATA_MODE, 0, 4, 7 << 8, "é", 0, 0, "40 2C 3A 90 EC 11 EC 11 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /*  5*/ { DATA_MODE, 0, 4, 6 << 8, "\351", 0, 0, "40 1E 90 EC 11 EC 11 EC 11", 1, 1, "B1 (ISO 8859-1)" },
        /*  6*/ { UNICODE_MODE, 0, 4, 2 << 8, "β", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 11 00 00 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS)" },
        /*  7*/ { UNICODE_MODE, 9, 4, 7 << 8, "β", 0, 9, "70 94 01 E2 00 EC 11 EC 11", 1, 1, "ECI-9 B1 (ISO 8859-7)" },
        /*  8*/ { UNICODE_MODE, 20, 4, -1, "β", 0, 20, "71 48 01 10 00 00 EC 11 EC", 1, 1, "ECI-20 K1 (Shift JIS)" },
        /*  9*/ { UNICODE_MODE, 26, 4, 5 << 8, "β", 0, 26, "71 A4 02 CE B2 00 EC 11 EC", 1, 1, "ECI-26 B2 (UTF-8)" },
        /* 10*/ { DATA_MODE, 0, 4, 4 << 8, "β", 0, 0, "40 2C EB 20 EC 11 EC 11 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 11*/ { UNICODE_MODE, 0, 4, 5 << 8, "ก", ZINT_WARN_USES_ECI, 13, "Warning 70 D4 01 A1 00 EC 11 EC 11", 1, 1, "ECI-13 B1 (ISO 8859-11)" },
        /* 12*/ { UNICODE_MODE, 13, 4, 5 << 8, "ก", 0, 13, "70 D4 01 A1 00 EC 11 EC 11", 1, 1, "ECI-13 B1 (ISO 8859-11)" },
        /* 13*/ { UNICODE_MODE, 20, -1, -1, "ก", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "ก not in Shift JIS" },
        /* 14*/ { UNICODE_MODE, 26, 4, 8 << 8, "ก", 0, 26, "71 A4 03 E0 B8 81 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 15*/ { DATA_MODE, 0, 4, 3 << 8, "ก", 0, 0, "40 3E 0B 88 10 EC 11 EC 11", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 16*/ { UNICODE_MODE, 0, 4, 8 << 8, "Ж", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 11 23 80 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS)" },
        /* 17*/ { UNICODE_MODE, 7, 4, -1, "Ж", 0, 7, "70 74 01 B6 00 EC 11 EC 11", 1, 1, "ECI-7 B1 (ISO 8859-5)" },
        /* 18*/ { UNICODE_MODE, 20, 4, 7 << 8, "Ж", 0, 20, "71 48 01 12 38 00 EC 11 EC", 1, 1, "ECI-20 K1 (Shift JIS)" },
        /* 19*/ { UNICODE_MODE, 26, 4, -1, "Ж", 0, 26, "71 A4 02 D0 96 00 EC 11 EC", 1, 1, "ECI-26 B2 (UTF-8)" },
        /* 20*/ { DATA_MODE, 0, 4, -1, "Ж", 0, 0, "40 2D 09 60 EC 11 EC 11 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 21*/ { UNICODE_MODE, 0, 4, -1, "ກ", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E0 BA 81 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 22*/ { UNICODE_MODE, 20, -1, -1, "ກ", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "ກ not in Shift JIS" },
        /* 23*/ { UNICODE_MODE, 26, 4, -1, "ກ", 0, 26, "71 A4 03 E0 BA 81 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 24*/ { DATA_MODE, 0, 4, 8 << 8, "ກ", 0, 0, "40 3E 0B A8 10 EC 11 EC 11", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 25*/ { UNICODE_MODE, 0, 4, 2 << 8, "\\", 0, 0, "40 15 C0 EC 11 EC 11 EC 11", 1, 1, "B1 (ASCII)" },
        /* 26*/ { UNICODE_MODE, 20, 4, 4 << 8, "\\", 0, 20, "71 48 01 00 F8 00 EC 11 EC", 1, 0, "ECI-20 K1 (Shift JIS); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 27*/ { UNICODE_MODE, 20, 4, -1, "[", 0, 20, "71 44 01 5B 00 EC 11 EC 11", 1, 1, "B1 (ASCII)" },
        /* 28*/ { UNICODE_MODE, 20, 4, 3 << 8, "\177", 0, 20, "71 44 01 7F 00 EC 11 EC 11", 1, 1, "ECI-20 B1 (ASCII)" },
        /* 29*/ { UNICODE_MODE, 0, 4, 2 << 8, "¥", 0, 0, "40 1A 50 EC 11 EC 11 EC 11", 1, 1, "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous)" },
        /* 30*/ { UNICODE_MODE, 3, 4, 3 << 8, "¥", 0, 3, "70 34 01 A5 00 EC 11 EC 11", 1, 1, "ECI-3 B1 (ISO 8859-1)" },
        /* 31*/ { UNICODE_MODE, 20, 4, 5 << 8, "¥", 0, 20, "71 44 01 5C 00 EC 11 EC 11", 1, 0, "ECI-20 B1 (Shift JIS) (to single-byte backslash codepoint 5C, so byte mode); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 32*/ { UNICODE_MODE, 26, 4, 6 << 8, "¥", 0, 26, "71 A4 02 C2 A5 00 EC 11 EC", 1, 1, "ECI-26 B2 (UTF-8)" },
        /* 33*/ { DATA_MODE, 0, 4, 2 << 8, "¥", 0, 0, "40 2C 2A 50 EC 11 EC 11 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 34*/ { UNICODE_MODE, 0, 4, 2 << 8, "･", ZINT_WARN_NONCOMPLIANT, 0, "Warning 40 1A 50 EC 11 EC 11 EC 11", 1, 1, "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /* 35*/ { UNICODE_MODE, 3, -1, -1, "･", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 36*/ { UNICODE_MODE, 20, 4, 7 << 8, "･", 0, 20, "71 44 01 A5 00 EC 11 EC 11", 1, 1, "ECI-20 B1 (Shift JIS) single-byte codepoint A5" },
        /* 37*/ { UNICODE_MODE, 26, 4, -1, "･", 0, 26, "71 A4 03 EF BD A5 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 38*/ { DATA_MODE, 0, 4, 8 << 8, "･", 0, 0, "40 3E FB DA 50 EC 11 EC 11", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 39*/ { UNICODE_MODE, 0, 4, -1, "¿", 0, 0, "40 1B F0 EC 11 EC 11 EC 11", 1, 1, "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous)" },
        /* 40*/ { UNICODE_MODE, 3, 4, 8 << 8, "¿", 0, 3, "70 34 01 BF 00 EC 11 EC 11", 1, 1, "ECI-3 B1 (ISO 8859-1)" },
        /* 41*/ { UNICODE_MODE, 20, -1, -1, "¿", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "¿ not in Shift JIS" },
        /* 42*/ { UNICODE_MODE, 26, 4, 2 << 8, "¿", 0, 26, "71 A4 02 C2 BF 00 EC 11 EC", 1, 1, "ECI-26 B2 (UTF-8)" },
        /* 43*/ { DATA_MODE, 0, 4, 3 << 8, "¿", 0, 0, "40 2C 2B F0 EC 11 EC 11 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 44*/ { UNICODE_MODE, 0, 4, -1, "ｿ", ZINT_WARN_NONCOMPLIANT, 0, "Warning 40 1B F0 EC 11 EC 11 EC 11", 1, 1, "B1 (Shift JIS) single-byte codepoint BF (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 45*/ { UNICODE_MODE, 3, -1, -1, "ｿ", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 46*/ { UNICODE_MODE, 20, 4, 6 << 8, "ｿ", 0, 20, "71 44 01 BF 00 EC 11 EC 11", 1, 1, "ECI-20 B1 (Shift JIS) single-byte codepoint BF" },
        /* 47*/ { UNICODE_MODE, 26, 4, 4 << 8, "ｿ", 0, 26, "71 A4 03 EF BD BF 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 48*/ { DATA_MODE, 0, 4, -1, "ｿ", 0, 0, "40 3E FB DB F0 EC 11 EC 11", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 49*/ { UNICODE_MODE, 0, 4, -1, "~", 0, 0, "40 17 E0 EC 11 EC 11 EC 11", 1, 1, "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 50*/ { UNICODE_MODE, 3, 4, -1, "~", 0, 3, "70 34 01 7E 00 EC 11 EC 11", 1, 1, "ECI-3 B1 (ASCII)" },
        /* 51*/ { UNICODE_MODE, 20, -1, -1, "~", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "tilde not in Shift JIS (codepoint used for overline)" },
        /* 52*/ { UNICODE_MODE, 0, 4, -1, "‾", ZINT_WARN_NONCOMPLIANT, 0, "Warning 40 17 E0 EC 11 EC 11 EC 11", 1, 0, "B1 (Shift JIS) single-byte codepoint 7E (same bytes as ~ ASCII above, so ambiguous); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 53*/ { UNICODE_MODE, 3, -1, -1, "‾", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 54*/ { UNICODE_MODE, 20, 4, 7 << 8, "‾", 0, 20, "71 44 01 7E 00 EC 11 EC 11", 1, 0, "ECI-20 B1 (Shift JIS) (to single-byte tilde codepoint 7E, so byte mode); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 55*/ { UNICODE_MODE, 26, 4, 1 << 8, "‾", 0, 26, "71 A4 03 E2 80 BE 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 56*/ { DATA_MODE, 0, 4, -1, "‾", 0, 0, "40 3E 28 0B E0 EC 11 EC 11", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 57*/ { UNICODE_MODE, 0, 4, -1, "点", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 16 CF 80 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS)" },
        /* 58*/ { UNICODE_MODE, 3, -1, -1, "点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 59*/ { UNICODE_MODE, 20, 4, 3 << 8, "点", 0, 20, "71 48 01 6C F8 00 EC 11 EC", 1, 1, "ECI-20 K1 (Shift JIS)" },
        /* 60*/ { UNICODE_MODE, 26, 4, -1, "点", 0, 26, "71 A4 03 E7 82 B9 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 61*/ { DATA_MODE, 0, 4, 6 << 8, "点", 0, 0, "40 3E 78 2B 90 EC 11 EC 11", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 62*/ { DATA_MODE, 0, 4, 4 << 8, "\223\137", 0, 0, "40 29 35 F0 EC 11 EC 11 EC", 0, 0, "B2 (Shift JIS) (not full multibyte); BWIPP same as FULL_MULTIBYTE; ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 63*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "\223\137", 0, 0, "80 16 CF 80 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS)" },
        /* 64*/ { UNICODE_MODE, 0, 4, -1, "¥･点", ZINT_WARN_NONCOMPLIANT, 0, "Warning 40 45 CA 59 35 F0 EC 11 EC", 1, 0, "B4 (Shift JIS) (optimized to byte mode only); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 65*/ { UNICODE_MODE, 3, -1, -1, "¥･点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 66*/ { UNICODE_MODE, 20, 4, 4 << 8, "¥･点", 0, 20, "71 44 04 5C A5 93 5F 00 EC", 1, 0, "ECI-20 B4 (Shift JIS); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 67*/ { UNICODE_MODE, 26, 3, 7 << 8, "¥･点", 0, 26, "71 A4 08 C2 A5 EF BD A5 E7 82 B9 00 EC", 1, 1, "ECI-26 B8 (UTF-8)" },
        /* 68*/ { DATA_MODE, 0, 4, -1, "\134\245\223\137", 0, 0, "40 45 CA 59 35 F0 EC 11 EC", 1, 0, "B8 (Shift JIS); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 69*/ { DATA_MODE, 0, 3, -1, "¥･点", 0, 0, "40 8C 2A 5E FB DA 5E 78 2B 90 EC 11 EC", 1, 0, "B8 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 70*/ { UNICODE_MODE, 0, 4, 8 << 8, "点茗", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 26 CF EA A8 00 EC 11 EC", 1, 1, "K2 (Shift JIS)" },
        /* 71*/ { UNICODE_MODE, 0, 4, -1, "点茗テ", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 36 CF EA A8 34 A0 EC 11", 1, 1, "K3 (Shift JIS)" },
        /* 72*/ { UNICODE_MODE, 0, 4, -1, "点茗テ点", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 46 CF EA A8 34 AD 9F 00", 1, 1, "K4 (Shift JIS)" },
        /* 73*/ { UNICODE_MODE, 0, 3, -1, "点茗テ点茗", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 56 CF EA A8 34 AD 9F D5 50 00 EC 11", 1, 1, "K5 (Shift JIS)" },
        /* 74*/ { UNICODE_MODE, 0, 3, 1 << 8, "点茗テ点茗テ", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 66 CF EA A8 34 AD 9F D5 50 69 40 EC", 1, 1, "K6 (Shift JIS)" },
        /* 75*/ { UNICODE_MODE, 0, 2, 4 << 8, "点茗テ点茗テｿ", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 66 CF EA A8 34 AD 9F D5 50 69 50 06 FC 00 EC", 1, 1, "K6 B1 (Shift JIS)" },
        /* 76*/ { DATA_MODE, 0, 2, 8 << 8, "\223\137\344\252\203\145\223\137\344\252\203\145\277", 0, 0, "40 D9 35 FE 4A A8 36 59 35 FE 4A A8 36 5B F0 EC", 0, 0, "B13 (Shift JIS); BWIPP same as FULL_MULTIBYTE; ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 77*/ { DATA_MODE, 0, 2, ZINT_FULL_MULTIBYTE | (4 << 8), "\223\137\344\252\203\145\223\137\344\252\203\145\277", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 50 06 FC 00 EC", 1, 1, "K6 B1 (Shift JIS) (full multibyte)" },
        /* 78*/ { DATA_MODE, 0, 2, 2 << 8, "点茗テ点茗テｿ", 0, 0, "41 5E 78 2B 9E 88 C9 7E 38 38 6E 78 2B 9E 88 C9 7E 38 38 6E FB DB F0 EC 11 EC 11 EC", 1, 0, "B21 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 79*/ { DATA_MODE, 0, 3, 8 << 8, "ÁȁȁȁȁȁȁȂ¢", 0, 0, "41 2C 38 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 2C 2A 20 EC 11", 1, 0, "B18 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 80*/ { DATA_MODE, 0, 3, ZINT_FULL_MULTIBYTE | (8 << 8), "ÁȁȁȁȁȁȁȂ¢", 0, 0, "41 2C 38 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 2C 2A 20 EC 11", 1, 1, "B18 (UTF-8) (full multibyte)" },
        /* 81*/ { DATA_MODE, 0, 3, 4 << 8, "ÁȁȁȁȁȁȁȁȂ¢", 0, 0, "41 4C 38 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 2C 2A 20", 0, 0, "B20 (UTF-8); BWIPP same as FULL_MULTIBYTE; ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 82*/ { DATA_MODE, 0, 3, ZINT_FULL_MULTIBYTE | (4 << 8), "ÁȁȁȁȁȁȁȁȂ¢", 0, 0, "40 1C 38 09 04 40 22 01 10 08 80 44 02 20 11 00 88 0A 12 00 D1 00", 1, 1, "B1 K9 B1 (UTF-8) (full multibyte)" },
        /* 83*/ { UNICODE_MODE, 0, 3, 8 << 8, "ÁȁȁȁȁȁȁȂ¢", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 12 C3 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00", 1, 1, "B18 (UTF-8)" },
        /* 84*/ { UNICODE_MODE, 0, 3, ZINT_FULL_MULTIBYTE | (8 << 8), "ÁȁȁȁȁȁȁȂ¢", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 12 C3 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00", 1, 1, "B18 (UTF-8)" },
        /* 85*/ { UNICODE_MODE, 0, 2, 1 << 8, "ÁȁȁȁȁȁȁȁȂ¢", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 14 C3 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00 EC 11 EC 11", 0, 1, "B20 (UTF-8); BWIPP same as FULL_MULTIBYTE" },
        /* 86*/ { UNICODE_MODE, 0, 2, ZINT_FULL_MULTIBYTE | (1 << 8), "ÁȁȁȁȁȁȁȁȂ¢", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 01 C3 80 90 44 02 20 11 00 88 04 40 22 01 10 08 80 A1 20 0D 10 00 EC 11 EC 11", 1, 1, "B1 K9 B1 (UTF-8) (full multibyte)" },
        /* 87*/ { UNICODE_MODE, 0, 4, -1, "áA", 0, 0, "40 2E 14 10 EC 11 EC 11 EC", 0, 1, "B2 (ISO 8859-1); BWIPP same as FULL_MULTIBYTE" },
        /* 88*/ { UNICODE_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "áA", 0, 0, "80 1C 00 80 EC 11 EC 11 EC", 1, 1, "K1 (ISO 8859-1) (full multibyte)" },
        /* 89*/ { UNICODE_MODE, 0, 1, -1, "A0B1C2D3E4F5G6H7I8J9KLMNOPQRSTUVWXYZ $%*+-./:", 0, 0, "(34) 21 69 C2 3E 08 79 26 27 A5 50 B5 98 23 32 6C 0E 65 FA C5 19 5B 42 6B 2D C1 C3 B9 E7", 1, 1, "A45" },
        /* 90*/ { UNICODE_MODE, 0, 4, 7 << 8, "˘", ZINT_WARN_USES_ECI, 4, "Warning 70 44 01 A2 00 EC 11 EC 11", 1, 1, "ECI-4 B1 (ISO 8859-2)" },
        /* 91*/ { UNICODE_MODE, 4, 4, 7 << 8, "˘", 0, 4, "70 44 01 A2 00 EC 11 EC 11", 1, 1, "ECI-4 B1 (ISO 8859-2)" },
        /* 92*/ { UNICODE_MODE, 0, 4, 7 << 8, "Ħ", ZINT_WARN_USES_ECI, 5, "Warning 70 54 01 A1 00 EC 11 EC 11", 1, 1, "ECI-5 B1 (ISO 8859-3)" },
        /* 93*/ { UNICODE_MODE, 5, 4, 7 << 8, "Ħ", 0, 5, "70 54 01 A1 00 EC 11 EC 11", 1, 1, "ECI-5 B1 (ISO 8859-3)" },
        /* 94*/ { UNICODE_MODE, 0, 4, -1, "ĸ", ZINT_WARN_USES_ECI, 6, "Warning 70 64 01 A2 00 EC 11 EC 11", 1, 1, "ECI-6 B1 (ISO 8859-4)" },
        /* 95*/ { UNICODE_MODE, 6, 4, -1, "ĸ", 0, 6, "70 64 01 A2 00 EC 11 EC 11", 1, 1, "ECI-6 B1 (ISO 8859-4)" },
        /* 96*/ { UNICODE_MODE, 0, 4, 8 << 8, "Ș", ZINT_WARN_USES_ECI, 18, "Warning 71 24 01 AA 00 EC 11 EC 11", 1, 1, "ECI-18 B1 (ISO 8859-16)" },
        /* 97*/ { UNICODE_MODE, 18, 4, 8 << 8, "Ș", 0, 18, "71 24 01 AA 00 EC 11 EC 11", 1, 1, "ECI-18 B1 (ISO 8859-16)" },
        /* 98*/ { UNICODE_MODE, 0, 4, 2 << 8, "テ", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 10 D2 80 EC 11 EC 11 EC", 1, 1, "K1 (SHIFT JIS)" },
        /* 99*/ { UNICODE_MODE, 20, 4, 2 << 8, "テ", 0, 20, "71 48 01 0D 28 00 EC 11 EC", 1, 1, "ECI-20 K1 (SHIFT JIS)" },
        /*100*/ { UNICODE_MODE, 20, 4, 4 << 8, "テテ", 0, 20, "71 48 02 0D 28 69 40 EC 11", 1, 1, "ECI-20 K2 (SHIFT JIS)" },
        /*101*/ { UNICODE_MODE, 20, 4, -1, "\\\\", 0, 20, "71 48 02 00 F8 07 C0 EC 11", 1, 0, "ECI-20 K2 (SHIFT JIS); ZXing-C++ doesn't map Shift JIS ASCII" },
        /*102*/ { UNICODE_MODE, 0, 4, -1, "…", ZINT_WARN_NONCOMPLIANT, 0, "Warning 80 10 11 80 EC 11 EC 11 EC", 1, 1, "K1 (SHIFT JIS)" },
        /*103*/ { UNICODE_MODE, 21, 4, -1, "…", 0, 21, "71 54 01 85 00 EC 11 EC 11", 1, 1, "ECI-21 B1 (Win 1250)" },
        /*104*/ { UNICODE_MODE, 0, 4, 7 << 8, "Ґ", ZINT_WARN_USES_ECI, 22, "Warning 71 64 01 A5 00 EC 11 EC 11", 1, 1, "ECI-22 B1 (Win 1251)" },
        /*105*/ { UNICODE_MODE, 22, 4, 7 << 8, "Ґ", 0, 22, "71 64 01 A5 00 EC 11 EC 11", 1, 1, "ECI-22 B1 (Win 1251)" },
        /*106*/ { UNICODE_MODE, 0, 4, 6 << 8, "˜", ZINT_WARN_USES_ECI, 23, "Warning 71 74 01 98 00 EC 11 EC 11", 1, 1, "ECI-23 B1 (Win 1252)" },
        /*107*/ { UNICODE_MODE, 23, 4, 6 << 8, "˜", 0, 23, "71 74 01 98 00 EC 11 EC 11", 1, 1, "ECI-23 B1 (Win 1252)" },
        /*108*/ { UNICODE_MODE, 24, 4, 5 << 8, "پ", 0, 24, "71 84 01 81 00 EC 11 EC 11", 1, 1, "ECI-24 B1 (Win 1256)" },
        /*109*/ { UNICODE_MODE, 0, 4, 7 << 8, "က", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E1 80 80 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /*110*/ { UNICODE_MODE, 25, 4, -1, "က", 0, 25, "71 94 02 10 00 00 EC 11 EC", 1, 1, "ECI-25 B2 (UCS-2BE)" },
        /*111*/ { UNICODE_MODE, 25, 4, 4 << 8, "ကက", 0, 25, "71 94 04 10 00 10 00 00 EC", 1, 1, "ECI-25 B4 (UCS-2BE)" },
        /*112*/ { UNICODE_MODE, 25, 4, -1, "12", 0, 25, "71 94 04 00 31 00 32 00 EC", 1, 1, "ECI-25 B4 (UCS-2BE ASCII)" },
        /*113*/ { UNICODE_MODE, 27, 4, 4 << 8, "@", 0, 27, "71 B4 01 40 00 EC 11 EC 11", 1, 1, "ECI-27 B1 (ASCII)" },
        /*114*/ { UNICODE_MODE, 0, 4, -1, "龘", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E9 BE 98 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /*115*/ { UNICODE_MODE, 28, 4, 3 << 8, "龘", 0, 28, "71 C4 02 F9 D5 00 EC 11 EC", 1, 1, "ECI-28 B2 (Big5)" },
        /*116*/ { UNICODE_MODE, 28, 4, 4 << 8, "龘龘", 0, 28, "71 C4 04 F9 D5 F9 D5 00 EC", 1, 1, "ECI-28 B4 (Big5)" },
        /*117*/ { UNICODE_MODE, 0, 4, 8 << 8, "齄", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E9 BD 84 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /*118*/ { UNICODE_MODE, 29, 4, -1, "齄", 0, 29, "71 D4 02 F7 FE 00 EC 11 EC", 1, 1, "ECI-29 B2 (GB 2312)" },
        /*119*/ { UNICODE_MODE, 29, 4, 4 << 8, "齄齄", 0, 29, "71 D4 04 F7 FE F7 FE 00 EC", 1, 1, "ECI-29 B4 (GB 2312)" },
        /*120*/ { UNICODE_MODE, 0, 4, 8 << 8, "가", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 EA B0 80 00 EC 11", 1, 1, "ECI-26 B3 (UTF-8)" },
        /*121*/ { UNICODE_MODE, 30, 4, -1, "가", 0, 30, "71 E4 02 B0 A1 00 EC 11 EC", 1, 1, "ECI-30 B2 (EUC-KR)" },
        /*122*/ { UNICODE_MODE, 30, 4, -1, "가가", 0, 30, "71 E4 04 B0 A1 B0 A1 00 EC", 1, 1, "ECI-30 B4 (EUC-KR)" },
        /*123*/ { UNICODE_MODE, 170, 4, 2 << 8, "?", 0, 170, "78 0A A4 01 3F 00 EC 11 EC", 1, 1, "ECI-170 B1 (ASCII invariant)" },
        /*124*/ { DATA_MODE, 899, 4, 3 << 8, "\200", 0, 899, "78 38 34 01 80 00 EC 11 EC", 1, 1, "ECI-899 B1 (8-bit binary)" },
        /*125*/ { UNICODE_MODE, 900, 4, 8 << 8, "é", 0, 900, "78 38 44 02 C3 A9 00 EC 11", 1, 1, "ECI-900 B2 (no conversion)" },
        /*126*/ { UNICODE_MODE, 16384, 4, 8 << 8, "é", 0, 16384, "7C 04 00 04 02 C3 A9 00 EC", 1, 1, "ECI-16384 B2 (no conversion)" },
        /*127*/ { UNICODE_MODE, 3, 4, -1, "product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97", 0, 3, "(86) 70 34 39 70 72 6F 64 75 63 74 3A 47 6F 6F 67 6C 65 20 50 69 78 65 6C 20 34 61 20 2D", 0, 1, "ECI-3 B57 A8; BWIPP different encodation (B65)" },
        /*128*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "\201\176", 0, 0, "80 10 1F 00 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS 0x817E)" },
        /*129*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "\201\177", 0, 0, "40 28 17 F0 EC 11 EC 11 EC", 1, 1, "B2 (0x817F previously used Kanji mode, now excludes trailing 0x7F)" },
        /*130*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "\201\200", 0, 0, "80 10 20 00 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS 0x8180)" },
        /*131*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "\237\176", 0, 0, "80 1B 5F 00 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS 0x9F7E)" },
        /*132*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (1 << 8), "\237\177", 0, 0, "40 29 F7 F0 EC 11 EC 11 EC", 1, 1, "B2 (0x9F7F previously used Kanji mode, now excludes trailing 0x7F)" },
        /*133*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (5 << 8), "\340\176", 0, 0, "80 1B BF 00 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS 0xE07E)" },
        /*134*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "\340\177", 0, 0, "40 2E 07 F0 EC 11 EC 11 EC", 1, 1, "B2 (0xE07F previously used Kanji mode, now excludes trailing 0x7F)" },
        /*135*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (4 << 8), "\352\244", 0, 0, "80 1F 92 00 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS 0xEAA4, last valid codepoint)" },
        /*136*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (6 << 8), "\353\277", 0, 0, "80 1F FF 80 EC 11 EC 11 EC", 1, 1, "K1 (0xEBBF undefined in Shift JIS but not checked and uses Kanji mode)" },
        /*137*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "\353\300", 0, 0, "40 2E BC 00 EC 11 EC 11 EC", 1, 1, "B2 (0xEBC0 was always excluded)" },
        /*138*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE, "\201\300", 0, 0, "80 10 40 00 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS 0x81C0)" },
        /*139*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (2 << 8), "\201\374", 0, 0, "80 10 5E 00 EC 11 EC 11 EC", 1, 1, "K1 (Shift JIS 0x81FC)" },
        /*140*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (8 << 8), "\201\375", 0, 0, "40 28 1F D0 EC 11 EC 11 EC", 1, 1, "B2 (0x81FD previously used Kanji mode, now excludes trailing 0xFD)" },
        /*141*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (4 << 8), "\201\376", 0, 0, "40 28 1F E0 EC 11 EC 11 EC", 1, 1, "B2 (0x81FE previously used Kanji mode, now excludes trailing 0xFE)" },
        /*142*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (6 << 8), "\201\377", 0, 0, "40 28 1F F0 EC 11 EC 11 EC", 1, 1, "B2 (0x81FF previously used Kanji mode, now excludes trailing 0xFF)" },
        /*143*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (6 << 8), "\201\377", 0, 0, "40 28 1F F0 EC 11 EC 11 EC", 1, 1, "B2 (0x81FF previously used Kanji mode, now excludes trailing 0xFF)" },
        /*144*/ { DATA_MODE, 0, 4, ZINT_FULL_MULTIBYTE | (7 << 8), "\201\255", 0, 0, "80 10 36 80 EC 11 EC 11 EC", 1, 1, "K1 (0x81AD undefined in Shift JIS but not checked and uses Kanji mode)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_qr_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, data[i].input_mode, data[i].eci, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, %d, \"%s\", %d, %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1,
                    testUtilOption3Name(BARCODE_QRCODE, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), ret < ZINT_ERROR ? symbol->eci : -1, symbol->errtxt,
                    data[i].bwipp_cmp, data[i].zxingcpp_cmp, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, -1, data[i].option_3 | ZINT_FULL_MULTIBYTE, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        char modules_dump[177 * 177 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, data[i].option_1, -1, data[i].option_3 | ZINT_FULL_MULTIBYTE, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    if (!data[i].zxingcpp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[177 * 177 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_gs1(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_3;
        char *data;
        int ret;
        char *expected;
        int bwipp_cmp;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { GS1_MODE, 4, 7 << 8, "[01]12345678901231", 0, "51 04 00 B3 AA 37 DE 87 B1", 1, "N16" },
        /*  1*/ { GS1_MODE | GS1PARENS_MODE, 4, 7 << 8, "(01)12345678901231", 0, "51 04 00 B3 AA 37 DE 87 B1", 1, "N16" },
        /*  2*/ { GS1_MODE, 2, 4 << 8, "[01]04912345123459[15]970331[30]128[10]ABC123", 0, "51 07 40 A7 AC EA 80 15 9E 4F CA 52 D2 D3 84 09 D5 E0 28 FD 82 F0 C0 EC 11 EC 11 EC", 1, "N29 A9" },
        /*  3*/ { GS1_MODE | GS1PARENS_MODE, 2, 4 << 8, "(01)04912345123459(15)970331(30)128(10)ABC123", 0, "51 07 40 A7 AC EA 80 15 9E 4F CA 52 D2 D3 84 09 D5 E0 28 FD 82 F0 C0 EC 11 EC 11 EC", 1, "N29 A9" },
        /*  4*/ { GS1_MODE, 3, -1, "[91]12%[20]12", 0, "52 05 99 60 5F B5 35 80 01 08 00 EC 11", 1, "A10(11)" },
        /*  5*/ { GS1_MODE, 3, 1 << 8, "[91]123%[20]12", 0, "52 06 19 60 5E 2B 76 A0 5A 05 E0 EC 11", 1, "A11(12)" },
        /*  6*/ { GS1_MODE, 3, 6 << 8, "[91]1234%[20]12", 0, "52 06 99 60 5E 22 F6 A6 B0 00 21 00 EC", 1, "A12(13)" },
        /*  7*/ { GS1_MODE, 3, -1, "[91]12345%[20]12", 0, "51 01 F8 F3 A9 48 0F B5 35 80 01 08 00", 1, "N7 A6(7) (same bit count as A13(14))" },
        /*  8*/ { GS1_MODE, 3, 8 << 8, "[91]%%[20]12", 0, "52 05 99 6D A9 B5 35 80 01 08 00 EC 11", 1, "A9(11)" },
        /*  9*/ { GS1_MODE, 3, 6 << 8, "[91]%%%[20]12", 0, "52 06 99 6D A9 B5 36 A6 B0 00 21 00 EC", 1, "A10(13)" },
        /* 10*/ { GS1_MODE, 3, 6 << 8, "[91]A%%%%1234567890123AA%", 0, "54 07 39 31 41 25 25 25 25 10 34 7B 72 31 50 30 C8 08 73 36 A0 00", 0, "B7 N13 A3(4); BWIPP different encoding (same no. codewords)" },
        /* 11*/ { GS1_MODE, 1, -1, "[91]%23%%6789%%%34567%%%%234%%%%%", 0, "(34) 52 0C 99 6D A8 17 76 A6 D4 22 A5 C7 6A 6D 4D A8 22 C7 38 E8 18 4A 4A 4A 4A 64 66 68", 0, "A19(25) B12; BWIPP different encoding (same no. codewords, less padding)" },
        /* 12*/ { GS1_MODE, 2, 5 << 8, "[91]ABCDEFGHI[92]ABCDEF", 0, "52 0A 19 63 9A 8A 54 2A E1 6A 06 5C E6 A2 95 0A", 1, "A20(23)" },
        /* 13*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%", 0, "54 01 25 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11", 0, "B1; BWIPP different encoding (A2)" },
        /* 14*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%A", 0, "54 02 25 41 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC", 0, "B2; BWIPP different encoding (A3)" },
        /* 15*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]A%", 0, "54 02 41 25 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC", 0, "B2; BWIPP different encoding (A3)" },
        /* 16*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%AA", 0, "52 02 6D 43 98 00 EC 11 EC 11 EC 11 EC 11 EC 11", 1, "A3(4)" },
        /* 17*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]A%A", 0, "52 02 1E 8D 70 00 EC 11 EC 11 EC 11 EC 11 EC 11", 1, "A3(4)" },
        /* 18*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]AA%", 0, "52 02 1C CD A8 00 EC 11 EC 11 EC 11 EC 11 EC 11", 1, "A3(4)" },
        /* 19*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%%", 0, "54 02 25 25 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC", 0, "B2; BWIPP different encoding (A4)" },
        /* 20*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%%A", 0, "54 03 25 25 41 00 EC 11 EC 11 EC 11 EC 11 EC 11", 0, "B3; BWIPP different encoding (A5)" },
        /* 21*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%A%", 0, "54 03 25 41 25 00 EC 11 EC 11 EC 11 EC 11 EC 11", 0, "B3; BWIPP different encoding (A5)" },
        /* 22*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]A%%", 0, "54 03 41 25 25 00 EC 11 EC 11 EC 11 EC 11 EC 11", 0, "B3; BWIPP different encoding (A5)" },
        /* 23*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%AAA", 0, "52 02 ED 43 98 50 00 EC 11 EC 11 EC 11 EC 11 EC", 1, "A4(5)" },
        /* 24*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]A%AA", 0, "52 02 9E 8D 70 50 00 EC 11 EC 11 EC 11 EC 11 EC", 1, "A4(5)" },
        /* 25*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]A%AA", 0, "52 02 9E 8D 70 50 00 EC 11 EC 11 EC 11 EC 11 EC", 1, "A4(5)" },
        /* 26*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]AA%A", 0, "52 02 9C CD A8 50 00 EC 11 EC 11 EC 11 EC 11 EC", 1, "A4(5)" },
        /* 27*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]AAA%", 0, "52 02 9C C3 D1 30 00 EC 11 EC 11 EC 11 EC 11 EC", 1, "A4(5)" },
        /* 28*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%%AA", 0, "54 04 25 25 41 41 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A6) (same no. codewords)" },
        /* 29*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]%A%A", 0, "54 04 25 41 25 41 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A6) (same no. codewords)" },
        /* 30*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]A%%A", 0, "54 04 41 25 25 41 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A6) (same no. codewords)" },
        /* 31*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]A%A%", 0, "54 04 41 25 41 25 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A6) (same no. codewords)" },
        /* 32*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 5 << 8, "[]AA%%", 0, "54 04 41 41 25 25 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A6) (same no. codewords)" },
        /* 33*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAA", 0, "52 03 ED 4D A8 73 0A 00 EC 11 EC 11 EC 11 EC 11", 1, "A5(7)" },
        /* 34*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%AA", 0, "52 03 ED 43 D1 AE 0A 00 EC 11 EC 11 EC 11 EC 11", 1, "A5(7)" },
        /* 35*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%%AA", 0, "52 03 9E 8D A9 AE 0A 00 EC 11 EC 11 EC 11 EC 11", 1, "A5(7)" },
        /* 36*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%A%A", 0, "52 03 9E 8D 71 B5 0A 00 EC 11 EC 11 EC 11 EC 11", 1, "A5(7)" },
        /* 37*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AA%%A", 0, "52 03 9C CD A9 B5 0A 00 EC 11 EC 11 EC 11 EC 11", 1, "A5(7)" },
        /* 38*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AA%A%", 0, "52 03 9C CD A8 7A 26 00 EC 11 EC 11 EC 11 EC 11", 1, "A5(7)" },
        /* 39*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAA%%", 0, "52 03 9C C3 D1 B5 26 00 EC 11 EC 11 EC 11 EC 11", 1, "A5(7)" },
        /* 40*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%", 0, "54 03 25 25 25 00 EC 11 EC 11 EC 11 EC 11 EC 11", 0, "B3; BWIPP different encoding (A6)" },
        /* 41*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%A", 0, "54 04 25 25 25 41 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A7)" },
        /* 42*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%A%", 0, "54 04 25 25 41 25 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A7)" },
        /* 43*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%%", 0, "54 04 25 41 25 25 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A7)" },
        /* 44*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%%%", 0, "54 04 41 25 25 25 00 EC 11 EC 11 EC 11 EC 11 EC", 0, "B4; BWIPP different encoding (A7)" },
        /* 45*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%AA", 0, "54 05 25 25 25 41 41 00 EC 11 EC 11 EC 11 EC 11", 0, "B5; BWIPP different encoding (A8)" },
        /* 46*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AA%%%", 0, "54 05 41 41 25 25 25 00 EC 11 EC 11 EC 11 EC 11", 0, "B5; BWIPP different encoding (A8)" },
        /* 47*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%AAA", 0, "54 06 25 25 25 41 41 41 00 EC 11 EC 11 EC 11 EC", 0, "B6; BWIPP different encoding (A9)" },
        /* 48*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%A%A%", 0, "54 06 41 25 41 25 41 25 00 EC 11 EC 11 EC 11 EC", 0, "B6; BWIPP different encoding (A9)" },
        /* 49*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAAA", 0, "52 04 6D 4D A8 73 0E 60 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 50*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%AAA", 0, "52 04 6D 43 D1 AE 0E 60 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 51*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AA%AA", 0, "52 04 6D 43 99 B5 0E 60 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 52*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAA%A", 0, "52 04 6D 43 98 7A 35 C0 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 53*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAAA%", 0, "52 04 6D 43 98 73 36 A0 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 54*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%%AAA", 0, "52 04 1E 8D A9 AE 0E 60 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 55*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AA%%AA", 0, "52 04 1C CD A9 B5 0E 60 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 56*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAA%%A", 0, "52 04 1C C3 D1 B5 35 C0 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 57*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAAA%%", 0, "52 04 1C C3 99 B5 36 A0 00 EC 11 EC 11 EC 11 EC", 1, "A6(8)" },
        /* 58*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAAAA", 0, "52 04 ED 4D A8 73 0E 61 40 EC 11 EC 11 EC 11 EC", 1, "A7(9)" },
        /* 59*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%AAAA", 0, "52 04 ED 43 D1 AE 0E 61 40 EC 11 EC 11 EC 11 EC", 1, "A7(9)" },
        /* 60*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AA%AAA", 0, "52 04 ED 43 99 B5 0E 61 40 EC 11 EC 11 EC 11 EC", 1, "A7(9)" },
        /* 61*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAA%AA", 0, "52 04 ED 43 98 7A 35 C1 40 EC 11 EC 11 EC 11 EC", 1, "A7(9)" },
        /* 62*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAAA%A", 0, "52 04 ED 43 98 73 36 A1 40 EC 11 EC 11 EC 11 EC", 1, "A7(9)" },
        /* 63*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAAAA%", 0, "52 04 ED 43 98 73 0F 44 C0 EC 11 EC 11 EC 11 EC", 1, "A7(9)" },
        /* 64*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%AAAAA", 0, "52 05 ED 4D A9 B5 0E 61 CC 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 65*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%A%AAAA", 0, "52 05 ED 4D A8 7A 35 C1 CC 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 66*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AA%AAA", 0, "52 05 ED 4D A8 73 36 A1 CC 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 67*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAA%AA", 0, "52 05 ED 4D A8 73 0F 46 B8 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 68*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAAA%A", 0, "52 05 ED 4D A8 73 0E 66 D4 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 69*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAAAA%", 0, "52 05 ED 4D A8 73 0E 61 E8 98 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 70*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%%AAAA", 0, "52 05 ED 43 D1 B5 35 C1 CC 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 71*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AA%%AAA", 0, "52 05 ED 43 99 B5 36 A1 CC 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 72*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAA%%AA", 0, "52 05 ED 43 98 7A 36 A6 B8 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 73*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAAA%%A", 0, "52 05 ED 43 98 73 36 A6 D4 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 74*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAAAA%%", 0, "52 05 ED 43 98 73 0F 46 D4 98 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 75*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%%%AAAA", 0, "52 05 9E 8D A9 B5 35 C1 CC 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 76*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AA%%%AAA", 0, "52 05 9C CD A9 B5 36 A1 CC 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 77*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAA%%%AA", 0, "52 05 9C C3 D1 B5 36 A6 B8 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 78*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAAA%%%A", 0, "52 05 9C C3 99 B5 36 A6 D4 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 79*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAAAA%%%", 0, "52 05 9C C3 98 7A 36 A6 D4 98 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 80*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%A%A%AA", 0, "52 05 9E 8D 71 B5 0F 46 B8 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 81*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%A%AA%A", 0, "52 05 9E 8D 71 B5 0E 66 D4 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 82*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%A%AAA%", 0, "52 05 9E 8D 71 B5 0E 61 E8 98 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 83*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%A%AAA", 0, "52 05 ED 43 D1 AE 36 A1 CC 28 00 EC 11 EC 11 EC", 1, "A8(11)" },
        /* 84*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%%AAAAAA", 0, "52 07 6D 4D A9 B5 36 A1 CC 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 85*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%A%AAAAA", 0, "52 07 6D 4D A9 B5 0F 46 B8 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 86*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%AA%AAAA", 0, "52 07 6D 4D A9 B5 0E 66 D4 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 87*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%AAA%AAA", 0, "52 07 6D 4D A9 B5 0E 61 E8 D7 07 30 00 EC 11 EC", 1, "A10(14)" },
        /* 88*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%AAAA%AA", 0, "52 07 6D 4D A9 B5 0E 61 CC DA 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 89*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%AAAAA%A", 0, "52 07 6D 4D A9 B5 0E 61 CC 3D 1A E0 00 EC 11 EC", 1, "A10(14)" },
        /* 90*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%%AAAAAA%", 0, "52 07 6D 4D A9 B5 0E 61 CC 39 9B 50 00 EC 11 EC", 1, "A10(14)" },
        /* 91*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%A%%AAAAA", 0, "52 07 6D 4D A8 7A 36 A6 B8 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 92*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AA%%AAAA", 0, "52 07 6D 4D A8 73 36 A6 D4 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 93*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAA%%AAA", 0, "52 07 6D 4D A8 73 0F 46 D4 D7 07 30 00 EC 11 EC", 1, "A10(14)" },
        /* 94*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAAA%%AA", 0, "52 07 6D 4D A8 73 0E 66 D4 DA 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 95*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAAAA%%A", 0, "52 07 6D 4D A8 73 0E 61 E8 DA 9A E0 00 EC 11 EC", 1, "A10(14)" },
        /* 96*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%%AAAAAA%%", 0, "52 07 6D 4D A8 73 0E 61 CC DA 9B 50 00 EC 11 EC", 1, "A10(14)" },
        /* 97*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%%%AAAAA", 0, "52 07 6D 43 D1 B5 36 A6 B8 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 98*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AA%%%AAAA", 0, "52 07 6D 43 99 B5 36 A6 D4 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /* 99*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAA%%%AAA", 0, "52 07 6D 43 98 7A 36 A6 D4 D7 07 30 00 EC 11 EC", 1, "A10(14)" },
        /*100*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAAA%%%AA", 0, "52 07 6D 43 98 73 36 A6 D4 DA 87 30 00 EC 11 EC", 1, "A10(14)" },
        /*101*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAAAA%%%A", 0, "52 07 6D 43 98 73 0F 46 D4 DA 9A E0 00 EC 11 EC", 1, "A10(14)" },
        /*102*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%AAAAAA%%%", 0, "52 07 6D 43 98 73 0E 66 D4 DA 9B 50 00 EC 11 EC", 1, "A10(14)" },
        /*103*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%%%%AAAAA", 0, "52 07 1E 8D A9 B5 36 A6 B8 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /*104*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AA%%%%AAAA", 0, "52 07 1C CD A9 B5 36 A6 D4 39 87 30 00 EC 11 EC", 1, "A10(14)" },
        /*105*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAA%%%%AAA", 0, "52 07 1C C3 D1 B5 36 A6 D4 D7 07 30 00 EC 11 EC", 1, "A10(14)" },
        /*106*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAAA%%%%AA", 0, "52 07 1C C3 99 B5 36 A6 D4 DA 87 30 00 EC 11 EC", 1, "A10(14)" },
        /*107*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAAAA%%%%A", 0, "52 07 1C C3 98 7A 36 A6 D4 DA 9A E0 00 EC 11 EC", 1, "A10(14)" },
        /*108*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]AAAAAA%%%%", 0, "52 07 1C C3 98 73 36 A6 D4 DA 9B 50 00 EC 11 EC", 1, "A10(14)" },
        /*109*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%A%A%A%AA", 0, "52 07 1E 8D 71 B5 0F 46 B8 DA 87 30 00 EC 11 EC", 1, "A10(14)" },
        /*110*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%A%A%AA%A", 0, "52 07 1E 8D 71 B5 0F 46 B8 3D 1A E0 00 EC 11 EC", 1, "A10(14)" },
        /*111*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%A%A%AAA%", 0, "52 07 1E 8D 71 B5 0F 46 B8 39 9B 50 00 EC 11 EC", 1, "A10(14)" },
        /*112*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%A%A%AAA", 0, "52 07 6D 43 D1 AE 36 A1 E8 D7 07 30 00 EC 11 EC", 1, "A10(14)" },
        /*113*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]A%%AA%A%AA", 0, "52 07 1E 8D A9 AE 0F 46 B8 DA 87 30 00 EC 11 EC", 1, "A10(14)" },
        /*114*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%%AA%AAA", 0, "52 07 6D 43 D1 B5 35 C1 E8 D7 07 30 00 EC 11 EC", 1, "A10(14)" },
        /*115*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%%AAA%AA", 0, "52 07 6D 43 D1 B5 35 C1 CC DA 87 30 00 EC 11 EC", 1, "A10(14)" },
        /*116*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%%AAAA%A", 0, "52 07 6D 43 D1 B5 35 C1 CC 3D 1A E0 00 EC 11 EC", 1, "A10(14)" },
        /*117*/ { GS1_MODE | GS1NOCHECK_MODE, 2, 3 << 8, "[]%A%%AAAAA%", 0, "52 07 6D 43 D1 B5 35 C1 CC 39 9B 50 00 EC 11 EC", 1, "A10(14)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_qr_gs1", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, data[i].input_mode, -1 /*eci*/, data[i].option_1, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %s, \"%s\", %s, \"%s\", %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1,
                    testUtilOption3Name(BARCODE_QRCODE, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->errtxt, data[i].bwipp_cmp, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, -1, data[i].option_3, debug)) {
                if (!data[i].bwipp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    char modules_dump[177 * 177 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, data[i].option_1, -1, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                }
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                if (0 /*!data[i].zxingcpp_cmp*/) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    int cmp_len, ret_len;
                    char modules_dump[177 * 177 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_optimize(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_3; /* OR-ed with ZINT_FULL_MULTIBYTE */
        char *data;
        int ret;
        char *expected;
        int bwipp_cmp;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 4, -1, "1", 0, "10 04 40 EC 11 EC 11 EC 11", 1, "N1" },
        /*  1*/ { UNICODE_MODE, 4, 5 << 8, "AAA", 0, "20 19 CC 28 00 EC 11 EC 11", 1, "A3" },
        /*  2*/ { UNICODE_MODE, 4, 1 << 8, "0123456789", 0, "10 28 0C 56 6A 69 00 EC 11", 1, " N10 (nayuki.io - pure numeric)" },
        /*  3*/ { UNICODE_MODE, 4, -1, "ABCDEF", 0, "20 31 CD 45 2A 14 00 EC 11", 1, "A6 (nayuki.io - pure alphanumeric)" },
        /*  4*/ { UNICODE_MODE, 4, -1, "wxyz", 0, "40 47 77 87 97 A0 EC 11 EC", 1, "B4 (nayuki.io - pure byte)" },
        /*  5*/ { UNICODE_MODE, 1, -1, "「魔法少女まどか☆マギカ」って、　ИАИ　ｄｅｓｕ　κα？", ZINT_WARN_NONCOMPLIANT, "Warning (55) 81 D0 1A C0 09 F8 0A ED 56 B8 57 02 8E 12 90 2C 86 F4 31 A1 8A 01 B0 50 42 88 00 10", 1, "K29 (nayuki.io - pure kanji)" },
        /*  6*/ { UNICODE_MODE, 4, 1 << 8, "012345A", 0, "20 38 01 0B A2 E4 A0 EC 11", 1, "A7 (nayuki.io - alpha/numeric)" },
        /*  7*/ { UNICODE_MODE, 4, -1, "0123456A", 0, "10 1C 0C 56 58 80 25 00 EC", 1, "N7 A1 (nayuki.io - alpha/numeric) (note same bits as A8)" },
        /*  8*/ { UNICODE_MODE, 4, 1 << 8, "012a", 0, "40 43 03 13 26 10 EC 11 EC", 1, "B4 (nayuki.io - numeric/byte)" },
        /*  9*/ { UNICODE_MODE, 4, 4 << 8, "0123a", 0, "10 10 0C 34 01 61 00 EC 11", 1, "N4 B1 (nayuki.io - numeric/byte)" },
        /* 10*/ { UNICODE_MODE, 4, 4 << 8, "ABCDEa", 0, "40 64 14 24 34 44 56 10 EC", 1, "B6 (nayuki.io - alphanumeric/byte)" },
        /* 11*/ { UNICODE_MODE, 4, -1, "ABCDEFa", 0, "20 31 CD 45 2A 15 00 58 40", 1, "A6 B1 (nayuki.io - alphanumeric/byte)" },
        /* 12*/ { UNICODE_MODE, 1, 1 << 8, "THE SQUARE ROOT OF 2 IS 1.41421356237309504880168872420969807856967187537694807317667973799", 0, "(55) 20 D5 2A 53 54 1A A8 4C DC DF 14 29 EC 47 CA D9 9A 88 05 71 10 59 E3 56 32 5D 45 F0", 1, " A26 N65 (nayuki.io - alpha/numeric)" },
        /* 13*/ { UNICODE_MODE, 1, -1, "Golden ratio φ = 1.6180339887498948482045868343656381177203091798057628621354486227052604628189024497072072041893911374......", ZINT_WARN_NONCOMPLIANT, "Warning (80) 41 44 76 F6 C6 46 56 E2 07 26 17 46 96 F2 08 3D 32 03 D2 03 12 E1 19 26 A0 87 DC BB", 1, "B20 N100 A6 (nayuki.io - alpha/numeric/byte)" },
        /* 14*/ { UNICODE_MODE, 1, -1, "こんにちwa、世界！ αβγδ", ZINT_WARN_NONCOMPLIANT, "Warning (34) 41 B8 2B 18 2F 18 2C 98 2B F7 76 18 14 19 0A 28 A4 58 14 92 08 3B F8 3C 08 3C 18 3C", 1, "B27 (nayuki.io - kanji/european **NOT SAME** K4 B2 K4 A1 K4, less bits as nayuki (1.5.0) miscounting byte-mode input as UTF-8)" },
        /* 15*/ { UNICODE_MODE, 1, 2 << 8, "こんにちテwa、世界！ αβγδ", ZINT_WARN_NONCOMPLIANT, "Warning (34) 80 50 98 85 C4 29 21 3F 0D 2A 09 BB B0 C0 A0 C8 51 45 22 C0 A4 90 41 DF C1 E0 41 E0", 1, "K5 B19 (nayuki.io - kanji/european + extra leading kanji **NOT SAME** K5 B2 K4 A1 K4, same reason as above)" },
        /* 16*/ { UNICODE_MODE, 1, 8 << 8, "67128177921547861663com.acme35584af52fa3-88d0-093b-6c14-b37ddafb59c528908608sg.com.dash.www0530329356521790265903SG.COM.NETS46968696003522G33250183309051017567088693441243693268766948304B2AE13344004SG.SGQR209710339366720B439682.63667470805057501195235502733744600368027857918629797829126902859SG8236HELLO FOO2517Singapore3272B815", 0, "(232) 10 52 9F 46 70 B3 5D DE 9A 1F A1 7B 1B 7B 69 73 0B 1B 6B 29 99 A9 A9 C1 A3 0B 31 A9", 1, "N20 B47 N9 B15 N22 A11 N14 A1 N47 A19 N15 A8 N65 A20 B8 A8 (nayuki.io - SGQR alpha/numeric/byte)" },
        /* 17*/ { UNICODE_MODE, 4, 1 << 8, "纪", ZINT_WARN_USES_ECI, "Warning 71 A4 03 E7 BA AA 00 EC 11", 1, "ECI-26 B3 (UTF-8 E7BAAA, U+7EAA, not in Shift JIS)" },
        /* 18*/ { DATA_MODE, 4, 1 << 8, "纪", 0, "40 3E 7B AA A0 EC 11 EC 11", 1, "B3 (UTF-8 or Shift JIS, note ambiguous as 0xE7BA 0xAA happens to be valid Shift JIS 郤ｪ as well)" },
        /* 19*/ { UNICODE_MODE, 4, 1 << 8, "郤ｪ", ZINT_WARN_NONCOMPLIANT, "Warning 40 3E 7B AA A0 EC 11 EC 11", 1, "B3 (Shift JIS or UTF-8 E7BAAA 纪, see above)" },
        /* 20*/ { UNICODE_MODE, 1, -1, "2004年大西洋颶風季是有纪录以来造成人员伤亡和财产损失最为惨重的大西洋飓风季之一，于2004年6月1日正式开始，同年11月30日结束，传统上这样的日期界定了一年中绝大多数热带气旋在大西洋形成的时间段lll ku", ZINT_WARN_USES_ECI, "Warning (324) 71 A1 00 43 21 10 04 4B 96 E6 D3 96 92 9F A2 96 FF 9A D2 2F A6 8A DB A6 8A A3 96 B6", 0, "ECI-26 N4 B274 (nayuki.io - kanji/byte/numeric **NOT SAME* N4 K9 B6 K5 etc mixing Shift JIS and UTF-8); BWIPP different encodation (N4 K1 B272)" },
        /* 21*/ { UNICODE_MODE, 4, 1 << 8, "AB123456A", 0, "20 49 CD 05 E2 2C 73 94 00", 1, "A9" },
        /* 22*/ { UNICODE_MODE, 3, -1, "AB1234567890A", 0, "20 69 CD 05 E2 2C 73 94 33 2A 50 00 EC", 1, "A13" },
        /* 23*/ { UNICODE_MODE, 3, -1, "AB123456789012A", 0, "20 79 CD 05 E2 2C 73 94 33 2A 0B CA 00", 1, "A15" },
        /* 24*/ { UNICODE_MODE, 3, -1, "AB1234567890123A", 0, "20 11 CD 10 34 7B 72 31 50 30 C8 02 50", 1, "A2 N13 A1" },
        /* 25*/ { UNICODE_MODE, 3, 2 << 8, "テaABCD1", ZINT_WARN_NONCOMPLIANT, "Warning 40 88 36 56 14 14 24 34 43 10 EC 11 EC", 1, "B8" },
        /* 26*/ { UNICODE_MODE, 3, -1, "テaABCDE1", ZINT_WARN_NONCOMPLIANT, "Warning 40 38 36 56 12 03 1C D4 52 9D C0 EC 11", 0, "B3 A6; BWIPP different encodation (B9)" },
        /* 27*/ { UNICODE_MODE, 2, -1, "テéaABCDE1", ZINT_WARN_USES_ECI, "Warning 71 A4 06 E3 83 86 C3 A9 61 20 31 CD 45 29 DC 00", 1, "B6 A6" },
        /* 28*/ { UNICODE_MODE, 2, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", ZINT_WARN_NONCOMPLIANT, "Warning (44) 80 83 A8 85 88 25 CA 2F 40 B0 53 C2 44 98 41 00 4A 02 0E A8 F8 F5 0D 30 4C 35 A1 CC", 0, "K8 N1 K8 B3; BWIPP different encodation (K8 N1 K8 N1 K1)" },
        /* 29*/ { UNICODE_MODE, 1, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", ZINT_WARN_NONCOMPLIANT, "Warning (589) 80 20 EA 21 62 09 72 8B D0 2C 14 F0 91 26 10 40 04 A0 08 3A A3 E3 D4 34 C1 30 D6 87", 0, "K8 N1 K8 N1 K10 N2 K33 N2 K16 N1 K89 N2 K14 B5 K28 N2 K40 N1 K65; BWIPP different encodation (same except A2 B6 instead of B5)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_qr_optimize", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, data[i].input_mode, -1 /*eci*/, data[i].option_1, -1, data[i].option_3 | ZINT_FULL_MULTIBYTE, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, \"%s\", %s, \"%s\", %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_3, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].bwipp_cmp, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, -1, data[i].option_3 | ZINT_FULL_MULTIBYTE, debug)) {
                if (!data[i].bwipp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    char modules_dump[177 * 177 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, data[i].option_1, -1, data[i].option_3 | ZINT_FULL_MULTIBYTE, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                }
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                int cmp_len, ret_len;
                char modules_dump[177 * 177 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    /* や U+3084 kanji, in Shift JIS 0x82E2 (\202\342), UTF-8 E38284; its 2nd byte 0xE2 + 0x40-FC also form Shift JIS */
    static const struct item data[] = {
        /*  0*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, -1, { 0, 0, "" }, "QR Code Symbol", -1, 0, 21, 21, 0, "ISO 18004 Figure 1 **NOT SAME** uses mask 110 instead of 101; BWIPP uses 101",
                    "111111101001101111111"
                    "100000101001101000001"
                    "101110101100101011101"
                    "101110100010001011101"
                    "101110101001101011101"
                    "100000100001101000001"
                    "111111101010101111111"
                    "000000000100100000000"
                    "100111111101010010111"
                    "100010001110001000111"
                    "010100110000101101011"
                    "110111001000010111110"
                    "011111111110001011011"
                    "000000001000011010101"
                    "111111101101101010100"
                    "100000101010011011100"
                    "101110101001110001110"
                    "101110101111001001100"
                    "101110100100101010011"
                    "100000100001101111111"
                    "111111101001011000000"
                },
        /*  1*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, 6 << 8, { 0, 0, "" }, "QR Code Symbol", -1, 0, 21, 21, 1, "ISO 18004 Figure 1, explicit mask 101, same",
                    "111111100001101111111"
                    "100000101001101000001"
                    "101110101110101011101"
                    "101110101010001011101"
                    "101110100000101011101"
                    "100000100010101000001"
                    "111111101010101111111"
                    "000000001100100000000"
                    "100000101111011001110"
                    "100010001110001000111"
                    "011101111001100100010"
                    "110100001011010100110"
                    "011111111110001011011"
                    "000000001000000010110"
                    "111111100111111000110"
                    "100000100010011011100"
                    "101110100000111000111"
                    "101110100100001010100"
                    "101110100100101010011"
                    "100000100001110111100"
                    "111111101011001010010"
                },
        /*  2*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, -1, -1, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 0, 33, 33, 1, "ISO 18004 Figure 29 (top), same (mask 100)",
                    "111111101100110010010010101111111"
                    "100000100010111010111000101000001"
                    "101110100000001101101100001011101"
                    "101110101010000111000110001011101"
                    "101110101101100011010010001011101"
                    "100000101100010100001101101000001"
                    "111111101010101010101010101111111"
                    "000000001010000000011100100000000"
                    "100010111100001100100011011111001"
                    "100101000111001001000110000101100"
                    "010001100011111010101000011011001"
                    "101101011101010010000010010000000"
                    "001111110011010110010011101001100"
                    "011001000101001000111100110101001"
                    "101001111001111101001111000110111"
                    "100100010001000111100101111100000"
                    "110010111101110000011110111111100"
                    "010000010111100010001000010000111"
                    "111111111111010101000110010001111"
                    "001100010000000111100101010101110"
                    "111101111011101000111001010010001"
                    "100110000101001010010111000100001"
                    "000110111110111010010001011001000"
                    "001011010011101000011111011101111"
                    "111011111000010111001001111110000"
                    "000000001110110011111100100010100"
                    "111111101000110100101000101010011"
                    "100000100001010010001011100010000"
                    "101110101111011010000010111111100"
                    "101110100000111000111100000000101"
                    "101110100101010100001000010110100"
                    "100000100010110111000110101001001"
                    "111111101101101011010000111100011"
                },
        /*  3*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, -1, -1, { 1, 4, "1" }, "ABCDEFGHIJKLMN", -1, 0, 21, 21, 1, "ISO 18004 Figure 29 (bottom 1st), same",
                    "111111100110001111111"
                    "100000101001101000001"
                    "101110100010001011101"
                    "101110100110001011101"
                    "101110101110101011101"
                    "100000100110101000001"
                    "111111101010101111111"
                    "000000000011100000000"
                    "101010100011000010010"
                    "100011011111010011011"
                    "100010111110101110101"
                    "010110000101011000000"
                    "110000111110110111001"
                    "000000001011010001010"
                    "111111100101110011101"
                    "100000100100001110101"
                    "101110101101011000101"
                    "101110100110100110110"
                    "101110101011010011101"
                    "100000100101110000001"
                    "111111101000110110101"
                },
        /*  4*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, -1, 8 << 8, { 2, 4, "1" }, "OPQRSTUVWXYZ0123", -1, 0, 21, 21, 1, "ISO 18004 Figure 29 (bottom 2nd), same with explicit mask 111 (auto 011)",
                    "111111100011101111111"
                    "100000100001101000001"
                    "101110100001101011101"
                    "101110100101101011101"
                    "101110100100101011101"
                    "100000101000001000001"
                    "111111101010101111111"
                    "000000000111000000000"
                    "100101101001010100000"
                    "010111001001110011011"
                    "011110101011010010111"
                    "010100011110100110101"
                    "011100101100111110101"
                    "000000001011011100000"
                    "111111100110100011100"
                    "100000101001010100001"
                    "101110100100101101111"
                    "101110101100000010001"
                    "101110100110101001101"
                    "100000100111100011111"
                    "111111101011011110100"
                },
        /*  5*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, -1, -1, { 3, 4, "1" }, "456789ABCDEFGHIJ", -1, 0, 21, 21, 1, "ISO 18004 Figure 29 (bottom 3rd), same",
                    "111111100101001111111"
                    "100000100011101000001"
                    "101110100010101011101"
                    "101110100111101011101"
                    "101110100010101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000000001000000000"
                    "100101101101010100000"
                    "011011000001100111011"
                    "101010111101010110111"
                    "011001000101010100101"
                    "011111100110000110101"
                    "000000001010000111000"
                    "111111100100101101100"
                    "100000101101011001101"
                    "101110100010001101011"
                    "101110101101101110110"
                    "101110100100011001101"
                    "100000100011100101011"
                    "111111101011000011000"
                },
        /*  6*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, -1, -1, { 4, 4, "1" }, "KLMNOPQRSTUVWXYZ", -1, 0, 21, 21, 1, "ISO 18004 Figure 29 (bottom 4th), same",
                    "111111101011101111111"
                    "100000101010101000001"
                    "101110100011001011101"
                    "101110101100001011101"
                    "101110100111001011101"
                    "100000100110001000001"
                    "111111101010101111111"
                    "000000001101000000000"
                    "101101110110101001011"
                    "010001011000010000101"
                    "010000111010100101001"
                    "110101001110001000100"
                    "001101100000110000100"
                    "000000001010101001001"
                    "111111101111010111110"
                    "100000101000101000011"
                    "101110100101011000001"
                    "101110101001111100101"
                    "101110101001101111100"
                    "100000100010110010010"
                    "111111101110110101110"
                },
        /*  7*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, 1, -1, { 0, 0, "" }, "01234567", -1, 0, 21, 21, 0, "ISO 18004 Annex I I.2, same (mask 010); BWIPP uses mask 000",
                    "111111100101101111111"
                    "100000100111101000001"
                    "101110101000001011101"
                    "101110101100001011101"
                    "101110101011101011101"
                    "100000101000101000001"
                    "111111101010101111111"
                    "000000001001100000000"
                    "101111100100101111100"
                    "000101011010100101100"
                    "001000110101010011111"
                    "000010000100000111100"
                    "000111111001010010000"
                    "000000001011111001100"
                    "111111100110101100000"
                    "100000101011111000101"
                    "101110101000100101100"
                    "101110101100100100000"
                    "101110101011010010100"
                    "100000100000000110110"
                    "111111101111010010100"
                },
        /*  8*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, 1, 1 << 8, { 0, 0, "" }, "01234567", -1, 0, 21, 21, 1, "ISO 18004 Annex I Figure I.2, explicit mask 000, same as BWIPP",
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
        /*  9*/ { BARCODE_QRCODE, GS1_MODE, -1, 1, -1, -1, { 0, 0, "" }, "[01]09501101530003[8200]http://example.com", -1, 0, 25, 25, 0, "GS1 General Specifications 20.0 Figure 5.1-7 (note pre-21.0.1) **NOT SAME** figure uses Byte encodation only; BWIPP uses mask 001",
                    "1111111001101101001111111"
                    "1000001010010101001000001"
                    "1011101011111010101011101"
                    "1011101001100000101011101"
                    "1011101010010011101011101"
                    "1000001011001011101000001"
                    "1111111010101010101111111"
                    "0000000011000111100000000"
                    "1101001100100010001110110"
                    "0100110000101101111100111"
                    "0011001001100011101111010"
                    "0000100110010101000111101"
                    "1110001000010010100010100"
                    "0110010101110101100010110"
                    "1011001100010101011110011"
                    "0100110011000110001010110"
                    "1111101100100000111110101"
                    "0000000010011011100011010"
                    "1111111010000111101010011"
                    "1000001000010111100010110"
                    "1011101001001001111110011"
                    "1011101010100100100000000"
                    "1011101000000010001010001"
                    "1000001010110101100111010"
                    "1111111011101100010010111"
                },
        /* 10*/ { BARCODE_QRCODE, GS1_MODE, -1, 1, -1, 2 << 8, { 0, 0, "" }, "[01]09501101530003[8200]http://example.com", -1, 0, 25, 25, 1, "GS1 General Specifications 20.0 Figure 5.1-7 (note pre-21.0.1), explicit mask 001, same as BWIPP",
                    "1111111010111000001111111"
                    "1000001011100100101000001"
                    "1011101000111101101011101"
                    "1011101000110101101011101"
                    "1011101011100010001011101"
                    "1000001010001100101000001"
                    "1111111010101010101111111"
                    "0000000010110110000000000"
                    "1110011011100101011110011"
                    "0001100101111000101001101"
                    "0010111000010010011110100"
                    "0111100001010010000000101"
                    "1011011101000111110111110"
                    "0111100100000100010011000"
                    "1100001011010010011001011"
                    "0001100110010011011111100"
                    "1110011101010001111111011"
                    "0000000011011100100010010"
                    "1111111001010010101011001"
                    "1000001011100110100011000"
                    "1011101000001110111111011"
                    "1011101001110001110101010"
                    "1011101011110011111011111"
                    "1000001011110010100000010"
                    "1111111010111001000111101"
                },
        /* 11*/ { BARCODE_QRCODE, GS1_MODE, -1, 1, -1, -1, { 0, 0, "" }, "[01]09501101530003[10]640311[21]20FOOPC20", -1, 0, 25, 25, 0, "GS1 General Specifications 22.0 Figure 5.1-7 (note changed 21.0.1) **NOT SAME** figure uses Byte encodation only; BWIPP uses mask 101",
                    "1111111011101111001111111"
                    "1000001011100111001000001"
                    "1011101010110011101011101"
                    "1011101011010000001011101"
                    "1011101000000001101011101"
                    "1000001010011101001000001"
                    "1111111010101010101111111"
                    "0000000001001001100000000"
                    "1100111000010100000101111"
                    "1000100011101110001111100"
                    "1111101000011100000000101"
                    "1000010000110100010011011"
                    "1100111110101100110100000"
                    "1011000011000100011101001"
                    "0001111001000001101101010"
                    "0000010100101001110110010"
                    "1100001111110110111110101"
                    "0000000011101010100010000"
                    "1111111001111111101011000"
                    "1000001011110100100010110"
                    "1011101010001101111110101"
                    "1011101001100110100111011"
                    "1011101001100001010101110"
                    "1000001010001010110001100"
                    "1111111010110110111110011"
                },
        /* 12*/ { BARCODE_QRCODE, GS1_MODE, -1, 1, -1, 6 << 8, { 0, 0, "" }, "[01]09501101530003[10]640311[21]20FOOPC20", -1, 0, 25, 25, 1, "GS1 General Specifications 22.0 Figure 5.1-7 (note changed 21.0.1) **NOT SAME** figure uses Byte encodation only, explicit mask 101, same as BWIPP",
                    "1111111000011110101111111"
                    "1000001001100001001000001"
                    "1011101000001011001011101"
                    "1011101010001011001011101"
                    "1011101011000110101011101"
                    "1000001000011011001000001"
                    "1111111010101010101111111"
                    "0000000000110000000000000"
                    "1100011101010011000011000"
                    "1100000111001010101011000"
                    "0111011000100100111000010"
                    "0001100001001101101010100"
                    "1101001111011101000101110"
                    "1101000101000010011011001"
                    "1001001001111001010101101"
                    "1011000111110010101101001"
                    "1011001000110001111111101"
                    "0000000011101100100010000"
                    "1111111011110001101011001"
                    "1000001010001101100011001"
                    "1011101001001010111111101"
                    "1011101001000010000011111"
                    "1011101001011001101101001"
                    "1000001011110011001000011"
                    "1111111011000111001111101"
                },
        /* 13*/ { BARCODE_QRCODE, GS1_MODE, -1, 2, -1, -1, { 0, 0, "" }, "[01]00857674002010[8200]http://www.gs1.org/", -1, 0, 29, 29, 0, "GS1 General Specifications 22.0 Figure 5.7.3-1, same (mask 011); BWIPP uses mask 101",
                    "11111110100101110101001111111"
                    "10000010111101001000001000001"
                    "10111010010000001110001011101"
                    "10111010101110110010101011101"
                    "10111010001010101011001011101"
                    "10000010011111011001101000001"
                    "11111110101010101010101111111"
                    "00000000110110011010100000000"
                    "10110111001001010000101001011"
                    "00100000101101110111011001111"
                    "11000110001011001101000011010"
                    "11010101110110001010010000111"
                    "11100111011100110101111011100"
                    "01001000100100101111101001110"
                    "10001011100111011100101111101"
                    "01000001011110101001001001011"
                    "10010010010100111011001000101"
                    "00000101101000101110001011001"
                    "10001111110011111111110010010"
                    "00111100001011100011110101000"
                    "01001110111001101000111111001"
                    "00000000100000011001100010100"
                    "11111110111110110001101011110"
                    "10000010110111110010100011001"
                    "10111010010010000110111111010"
                    "10111010100111000011111111110"
                    "10111010101000101001111001001"
                    "10000010010111010001110010100"
                    "11111110101111111011110100110"
                },
        /* 14*/ { BARCODE_QRCODE, GS1_MODE, -1, 2, -1, 6 << 8, { 0, 0, "" }, "[01]00857674002010[8200]http://www.gs1.org/", -1, 0, 29, 29, 1, "GS1 General Specifications 22.0 Figure 5.7.3-1, explicit mask 101, same as BWIPP",
                    "11111110001000011000101111111"
                    "10000010111011101110101000001"
                    "10111010101011010101001011101"
                    "10111010110110001010001011101"
                    "10111010011100011101101011101"
                    "10000010010100010010101000001"
                    "11111110101010101010101111111"
                    "00000000110000111100000000000"
                    "10000010110010001011111001110"
                    "00011000010101001111100001000"
                    "01110010111101111011110101100"
                    "00011100111101000001011011110"
                    "10001010110001011000010110001"
                    "11101100000010001001001111010"
                    "01010010111100000111110100110"
                    "01111001100110010001110001100"
                    "00100110100010001101111110011"
                    "11001100100011100101000000000"
                    "11100010011110010010011111111"
                    "10011000101101000101010011100"
                    "10010111100010110011111110010"
                    "00000000111000100001100010011"
                    "11111110001000000111101011000"
                    "10000010011100111001100010000"
                    "10111010011111101011111110111"
                    "10111010000001100101011001010"
                    "10111010010011110010100010010"
                    "10000010001111101001001010011"
                    "11111110111001001101000010000"
                },
        /* 15*/ { BARCODE_HIBC_QR, -1, 0, 2, -1, -1, { 0, 0, "" }, "H123ABC01234567890", -1, 0, 21, 21, 1, "ANSI/HIBC 2.6 - 2016 Figure C5 same (mask 001)",
                    "111111101010001111111"
                    "100000100100101000001"
                    "101110101011001011101"
                    "101110100010101011101"
                    "101110100001101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000000110100000000"
                    "101000110001000100101"
                    "000110001111001000010"
                    "000101100101100111110"
                    "011100001100001101010"
                    "000101101111110110111"
                    "000000001011000001111"
                    "111111101111110111001"
                    "100000100101111000001"
                    "101110100010110101010"
                    "101110100111010101000"
                    "101110101101011001111"
                    "100000100100101111001"
                    "111111101111011001111"
                },
        /* 16*/ { BARCODE_HIBC_QR, -1, 0, 2, -1, -1, { 0, 0, "" }, "/EU720060FF0/O523201", -1, 0, 25, 25, 0, "HIBC/PAS Section 2.2 2nd Purchase Order **NOT SAME** uses mask 100 instead of 011; BWIPP uses mask 011",
                    "1111111011011110101111111"
                    "1000001001001111001000001"
                    "1011101001010010001011101"
                    "1011101010000000101011101"
                    "1011101010011000001011101"
                    "1000001010010000001000001"
                    "1111111010101010101111111"
                    "0000000010101101100000000"
                    "1000101111010100011111001"
                    "1001110011101110100100100"
                    "1010111101000001001001110"
                    "0011010010110010111000100"
                    "1100011110000110101110011"
                    "1000100000101000011011110"
                    "0011111100111111101010010"
                    "0001100110010101011100101"
                    "1111101011001100111110001"
                    "0000000011000111100010010"
                    "1111111010000000101011100"
                    "1000001001101011100011101"
                    "1011101011110111111110000"
                    "1011101000101000000010001"
                    "1011101000111111110100110"
                    "1000001000010100100011111"
                    "1111111010101101111000001"
                },
        /* 17*/ { BARCODE_HIBC_QR, -1, 0, 2, -1, 4 << 8, { 0, 0, "" }, "/EU720060FF0/O523201", -1, 0, 25, 25, 1, "HIBC/PAS Section 2.2 2nd Purchase Order same, explicit mask 011",
                    "1111111010011001101111111"
                    "1000001011010011001000001"
                    "1011101000000111001011101"
                    "1011101010111000001011101"
                    "1011101000000100001011101"
                    "1000001000111010101000001"
                    "1111111010101010101111111"
                    "0000000011001110000000000"
                    "1011011101111110101001011"
                    "1110110100101001100011100"
                    "1001011110100010101010010"
                    "0110000111100111101101110"
                    "1011011001000001101001011"
                    "0100110100110100000111101"
                    "0110101001101010111111000"
                    "1001010110101101100100010"
                    "0011111111010000111110010"
                    "0000000011101101100010111"
                    "1111111010111000101011011"
                    "1000001010001000100010001"
                    "1011101001011101111110101"
                    "1011101011101111000101001"
                    "1011101011011100010111010"
                    "1000001001000001110110101"
                    "1111111011101010111111001"
                },
        /* 18*/ { BARCODE_HIBC_QR, -1, 0, 2, -1, -1, { 0, 0, "" }, "/KN12345", -1, 0, 21, 21, 1, "HIBC/PAS Section 2.2 Asset Tag **NOT SAME** uses mask 000 instead of 100",
                    "111111100000101111111"
                    "100000101010101000001"
                    "101110100011001011101"
                    "101110100001101011101"
                    "101110101110101011101"
                    "100000100010101000001"
                    "111111101010101111111"
                    "000000000111100000000"
                    "101010100011000010010"
                    "111010010000001100001"
                    "111010101110100011001"
                    "011000011010001011001"
                    "100001110010101001001"
                    "000000001001010110001"
                    "111111100001011100011"
                    "100000100101110101000"
                    "101110101001011000111"
                    "101110100010001001110"
                    "101110101100100111001"
                    "100000100100001100111"
                    "111111101000101110101"
                },
        /* 19*/ { BARCODE_HIBC_QR, -1, 0, 2, -1, 5 << 8, { 0, 0, "" }, "/KN12345", -1, 0, 21, 21, 1, "HIBC/PAS Section 2.2 Asset Tag, same, explicit mask 100",
                    "111111101010101111111"
                    "100000100111001000001"
                    "101110100110101011101"
                    "101110101011101011101"
                    "101110101100101011101"
                    "100000101111001000001"
                    "111111101010101111111"
                    "000000001101100000000"
                    "100010111001011111001"
                    "010111011101100001100"
                    "010111100011001110100"
                    "001010001000011001011"
                    "110011100000111011011"
                    "000000001100111011100"
                    "111111101100110001110"
                    "100000100111100111010"
                    "101110101011001010101"
                    "101110100111100100011"
                    "101110100001001010100"
                    "100000100110011110101"
                    "111111101010111100111"
                },
        /* 20*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 1, -1, -1, { 0, 0, "" }, "12345678901234567890123456789012345678901", -1, 0, 21, 21, 1, "Max capacity ECC 1 Version 1 41 numbers",
                    "111111101001001111111"
                    "100000101100101000001"
                    "101110101011101011101"
                    "101110101011001011101"
                    "101110100001001011101"
                    "100000101001101000001"
                    "111111101010101111111"
                    "000000000110100000000"
                    "110011100010000101111"
                    "010101001011010101100"
                    "100100110110011100101"
                    "011100001001001101011"
                    "011110111010011010001"
                    "000000001000001100111"
                    "111111100110110100110"
                    "100000101110000011001"
                    "101110101111000101110"
                    "101110100101010101111"
                    "101110100100001110000"
                    "100000101000101001010"
                    "111111101010110000111"
                },
        /* 21*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, -1, -1, { 0, 0, "" }, "12345678901234567890123456789012345678901", -1, 0, 25, 25, 1, "ECC 2 auto-sets version 2",
                    "1111111011001110101111111"
                    "1000001001000000001000001"
                    "1011101011001111101011101"
                    "1011101001100000101011101"
                    "1011101001101011001011101"
                    "1000001010111110101000001"
                    "1111111010101010101111111"
                    "0000000001111100000000000"
                    "1010001100000101000100101"
                    "0111010101111001011000001"
                    "0010011101000111110010011"
                    "1001010100011011001100011"
                    "1000101101100111010110101"
                    "0001010000100111101011011"
                    "1111101100010001011000110"
                    "0000100001000101011010011"
                    "1101001101011100111111101"
                    "0000000011001001100010000"
                    "1111111010101110101010001"
                    "1000001000111011100010001"
                    "1011101000100111111110110"
                    "1011101001100110010101000"
                    "1011101011110001000100111"
                    "1000001000000100111010110"
                    "1111111010011100001100111"
                },
        /* 22*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 4, 10, -1, { 0, 0, "" }, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", -1, ZINT_WARN_NONCOMPLIANT, 57, 57, 1, "Max capacity ECC 4 Version 10 74 kanji",
                    "111111100111100000011001000011111100010010011011001111111"
                    "100000100011100101110000101000101001101111000001001000001"
                    "101110101001011100010001111110111100101001100011001011101"
                    "101110101101000010101100100011001110101001000101001011101"
                    "101110100011100100011000101111100110110000011101001011101"
                    "100000100000100111011111101000111101111100111010001000001"
                    "111111101010101010101010101010101010101010101010101111111"
                    "000000000001110101110110101000110111010111010111100000000"
                    "000110110011100011001111111111111010001100000011100001100"
                    "110110011010111001101110111010101010101101010001000100110"
                    "011100111000110100011101010110011100010010110011010010011"
                    "001011001011010100000011001101100111010100111110011100001"
                    "001000101011000101110011110001101001000001010100101001001"
                    "111011000111101101100101000100000000000101001101010101011"
                    "111010100110100110011101000100111101100100101001001101001"
                    "111100001100101010011101001111110010010101110011110011100"
                    "011111100011001001000011000011111111000101000010011011011"
                    "000011010001111101111110111000000011111100111100001001010"
                    "001110101000100011000110100101011010101100000101101010000"
                    "100111010111110011011011011110011111001010100111000111110"
                    "100110101011010110111100011100010100011011001001110110001"
                    "011110000111001010110011100100110100110101011110010101011"
                    "000011100101101111100111000111010100101111000110110111000"
                    "110111000101010010100001110110100110010110011000010000001"
                    "101100111110110001110100100101101001010001111000000010111"
                    "000100000100000110011110000110011010001111000100000110000"
                    "010011111011001010111100001111101011001011011101111111010"
                    "011010001100111001110100101000100011001001000000100010110"
                    "100110101001110001000100111010101011010000011010101010000"
                    "100010001100111000011110011000111011011100111010100011011"
                    "011111111001011110111100011111101100110001010000111110100"
                    "100110000110011111100111111011001010000010010011101011011"
                    "011001100001000011001010001111001010001001000011001101001"
                    "011111001110010111101010111001110100111101110101000111011"
                    "101101100100110100001111101001111001100101010010010010000"
                    "101111000101101101110001100111100011101101001100010101010"
                    "100000110000011011111010000110011100000010000001100100000"
                    "001011010101010110000000000110000001001010100011000111000"
                    "001011100110101101100100011100100100011011010000011001111"
                    "001010000001100111100000111010100101110010011011010010011"
                    "110100101110010001110000111101101111000010100010011010010"
                    "011011000101100001111000110110110000011000010101101110011"
                    "000111110011111101101110111100001001101010000100000100110"
                    "101100001100010000011101011011001011001101000111111100000"
                    "000011100110110111010011101111011101001001000010000111101"
                    "110000011010000010000001111100111001010000000001100101010"
                    "101001110100100111111111010001101010011010011010111111110"
                    "111110001100001100010100001011110011001110000100011110010"
                    "000000110011100011000011111111111000001000100011111110010"
                    "000000001000001101101000101000101100000110010011100011100"
                    "111111101001111111010001001010100110001011000011101011010"
                    "100000100111000101110010001000110100111101011100100011010"
                    "101110101111110100111111011111100001111010110100111110100"
                    "101110101001110011011100111111011001000011001101000111010"
                    "101110100110010100010000101001000010000000100000100000111"
                    "100000100110011101110011001101110110101010001101000011011"
                    "111111100010001101010110001001000001001011001001011001011"
                },
        /* 23*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 4, 27, -1, { 0, 0, "" }, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", -1, ZINT_WARN_NONCOMPLIANT, 125, 125, 1, "Max capacity ECC 4 Version 27 385 kanji",
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
        /* 24*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 4, 40, -1, { 0, 0, "" }, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点" "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", -1, ZINT_WARN_NONCOMPLIANT, 177, 177, 1, "Max capacity ECC 4 Version 40 784 kanji",
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
        /* 25*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 4, -1, ZINT_FULL_MULTIBYTE, { 0, 0, "" }, "áA", -1, 0, 21, 21, 1, "Mask automatic (001)",
                    "111111100101101111111"
                    "100000101001101000001"
                    "101110101010101011101"
                    "101110101001101011101"
                    "101110101101001011101"
                    "100000101010001000001"
                    "111111101010101111111"
                    "000000000001100000000"
                    "001001111011010111110"
                    "101000001010010000100"
                    "100111100000110001111"
                    "110000000001000010000"
                    "011010110111010001111"
                    "000000001100011101100"
                    "111111101011111111000"
                    "100000101101010101110"
                    "101110100001000111111"
                    "101110100101010000000"
                    "101110101001111000111"
                    "100000100100111010000"
                    "111111100011001000110"
                },
        /* 26*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 4, -1, ZINT_FULL_MULTIBYTE | (8 << 8), { 0, 0, "" }, "áA", -1, 0, 21, 21, 1, "Mask 111",
                    "111111101000101111111"
                    "100000101110101000001"
                    "101110100110101011101"
                    "101110101100101011101"
                    "101110101010001011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000000110100000000"
                    "000100100111000111011"
                    "111101011111000101110"
                    "100000100111110110111"
                    "101100011101011110011"
                    "001111100010000100101"
                    "000000001011011010100"
                    "111111100111100011011"
                    "100000100000000000100"
                    "101110100110000000111"
                    "101110101001001100011"
                    "101110100100101101101"
                    "100000100011111101000"
                    "111111100111010100101"
                },
        /* 27*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 4, -1, ZINT_FULL_MULTIBYTE | (9 << 8), { 0, 0, "" }, "áA", -1, 0, 21, 21, 1, "Mask > 111 ignored",
                    "111111100101101111111"
                    "100000101001101000001"
                    "101110101010101011101"
                    "101110101001101011101"
                    "101110101101001011101"
                    "100000101010001000001"
                    "111111101010101111111"
                    "000000000001100000000"
                    "001001111011010111110"
                    "101000001010010000100"
                    "100111100000110001111"
                    "110000000001000010000"
                    "011010110111010001111"
                    "000000001100011101100"
                    "111111101011111111000"
                    "100000101101010101110"
                    "101110100001000111111"
                    "101110100101010000000"
                    "101110101001111000111"
                    "100000100100111010000"
                    "111111100011001000110"
                },
        /* 28*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, 1, -1, { 0, 0, "" }, "1234567890", -1, 0, 21, 21, 0, "test_print example, automatic mask 001 (same score as mask 010); BWIPP uses mask 010",
                    "111111101001101111111"
                    "100000100100101000001"
                    "101110101001001011101"
                    "101110100101101011101"
                    "101110100001101011101"
                    "100000101101101000001"
                    "111111101010101111111"
                    "000000000000100000000"
                    "101000110010000100101"
                    "101010001111011101011"
                    "111010101101110110010"
                    "110111010101011100011"
                    "110111110101111111001"
                    "000000001010000000000"
                    "111111101110001000010"
                    "100000100000100010001"
                    "101110100110001000111"
                    "101110100111011001000"
                    "101110101101110110111"
                    "100000100001011000010"
                    "111111101011111111111"
                },
        /* 29*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, 1, 3 << 8, { 0, 0, "" }, "1234567890", -1, 0, 21, 21, 1, "test_print example, explicit mask 010",
                    "111111100010101111111"
                    "100000100000001000001"
                    "101110101010001011101"
                    "101110101001001011101"
                    "101110101010101011101"
                    "100000101001001000001"
                    "111111101010101111111"
                    "000000001100000000000"
                    "101111100001001111100"
                    "001110001011111001111"
                    "100001110110101101001"
                    "010011010001111000111"
                    "101100101110100100010"
                    "000000001110100100100"
                    "111111100101010011001"
                    "100000101100000110101"
                    "101110101101010011100"
                    "101110101011111101100"
                    "101110101110101101100"
                    "100000100101111100110"
                    "111111101000100100100"
                },
        /* 30*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 1, 2, 2 << 8, { 0, 0, "" }, "?ややややwやややや ややややや", -1, ZINT_WARN_NONCOMPLIANT, 25, 25, 1, "Data with Shift JIS '2nd byte 1st byte' matches; explicit mask 001 (auto 000) to match BWIPP",
                    "1111111010111110001111111"
                    "1000001011100101001000001"
                    "1011101000111110101011101"
                    "1011101000101100001011101"
                    "1011101011111111101011101"
                    "1000001010001001001000001"
                    "1111111010101010101111111"
                    "0000000010110101000000000"
                    "1110011011101111111110011"
                    "1001010001100001111100001"
                    "1101101110011110101101000"
                    "1011110011010001011000101"
                    "0011101011011101110101111"
                    "0010100010011010000110000"
                    "1111111111110110111101010"
                    "0010010111100101001001111"
                    "1111011011011011111111000"
                    "0000000010101010100010011"
                    "1111111000011110101011011"
                    "1000001011001011100010111"
                    "1011101000101110111111111"
                    "1011101000110000111000010"
                    "1011101010111101110101011"
                    "1000001010100110000011100"
                    "1111111011001111010111001"
                },
        /* 31*/ { BARCODE_QRCODE, UNICODE_MODE, 3, 2, -1, 5 << 8, { 0, 0, "" }, "sn:7QPB4MN", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 3 Example 1 **NOT SAME** different encodation; explicit mask 100 (auto 011) to match BWIPP",
                    "111111101011101111111"
                    "100000100001001000001"
                    "101110100110001011101"
                    "101110101010101011101"
                    "101110101000101011101"
                    "100000101001001000001"
                    "111111101010101111111"
                    "000000001111100000000"
                    "100010111111011111001"
                    "110010011111101101110"
                    "001110111011001010100"
                    "000010010100010011000"
                    "010111101100100011011"
                    "000000001100100111111"
                    "111111101100100111110"
                    "100000100011111110111"
                    "101110101011001110011"
                    "101110100010100110111"
                    "101110100011011011100"
                    "100000100101001111011"
                    "111111101000010100101"
                },
        /* 32*/ { BARCODE_QRCODE, UNICODE_MODE, 3, 2, -1, -1, { 0, 0, "" }, "price:£20.00", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 3 Example 2",
                    "111111101111101111111"
                    "100000101101001000001"
                    "101110100001101011101"
                    "101110101100101011101"
                    "101110100101001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000001101000000000"
                    "101101110000001001011"
                    "000010001100101111100"
                    "101011110111000010110"
                    "101000010101110100010"
                    "010100101010111100100"
                    "000000001101001100001"
                    "111111101010001111100"
                    "100000101101000110111"
                    "101110100001000101101"
                    "101110101000110101010"
                    "101110101011011100000"
                    "100000100101101011110"
                    "111111101100010111000"
                },
        /* 33*/ { BARCODE_QRCODE, UNICODE_MODE, 3, 1, -1, -1, { 0, 0, "" }, "C:\\DOCS\\EXAMPLE.TXT", -1, 0, 25, 25, 0, "AIM ITS/04-023:2022 ECI 3 Example 3 **NOT SAME** different encodation; BWIPP same encodation as Zint but uses mask 000 instead of 100",
                    "1111111011101001001111111"
                    "1000001011100010101000001"
                    "1011101010110001001011101"
                    "1011101011010111101011101"
                    "1011101000000100001011101"
                    "1000001010011100101000001"
                    "1111111010101010101111111"
                    "0000000001001001000000000"
                    "1100111000010110100101111"
                    "1001100001101010001000010"
                    "0010111100011110110100010"
                    "1111000100110100011101010"
                    "1110111100101101101000111"
                    "1000000010000110001111001"
                    "0001011110100001000100111"
                    "0011010001101011100100010"
                    "1110001111110110111110001"
                    "0000000010101001100011001"
                    "1111111000111110101011100"
                    "1000001010010101100010101"
                    "1011101011001101111110001"
                    "1011101001000111100001001"
                    "1011101001100001111001110"
                    "1000001011001010011000001"
                    "1111111011010111011011111"
                },
        /* 34*/ { BARCODE_QRCODE, UNICODE_MODE, 3, 1, -1, 1 << 8, { 0, 0, "" }, "C:\\DOCS\\EXAMPLE.TXT", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 3 Example 3 **NOT SAME** different encodation",
                    "1111111001001101101111111"
                    "1000001000111001101000001"
                    "1011101011101010001011101"
                    "1011101001110011001011101"
                    "1011101000100000101011101"
                    "1000001001000111101000001"
                    "1111111010101010101111111"
                    "0000000011101101100000000"
                    "1110111110110010011000100"
                    "0010110010110001010011001"
                    "1001101111000101101111001"
                    "1011100000010000111001110"
                    "1010011000001001001100011"
                    "0011010001011101010100010"
                    "1010001101111010011111100"
                    "0111110101001111000000110"
                    "1010101011010010111110101"
                    "0000000011110010100010010"
                    "1111111011100101101010111"
                    "1000001010110001100010001"
                    "1011101011101001111110101"
                    "1011101000011100111010010"
                    "1011101010111010100010101"
                    "1000001011101110111100101"
                    "1111111011110011111111011"
                },
        /* 35*/ { BARCODE_QRCODE, UNICODE_MODE, 4, 1, -1, -1, { 0, 0, "" }, "Študentska št. 2198390", -1, 0, 25, 25, 0, "AIM ITS/04-023:2022 ECI 4 Example 1 **NOT SAME** different encodation; BWIPP same encodation as Zint but uses mask 011 instead of 100",
                    "1111111010000110001111111"
                    "1000001010011110101000001"
                    "1011101011001111101011101"
                    "1011101011101011101011101"
                    "1011101001101010001011101"
                    "1000001011100100001000001"
                    "1111111010101010101111111"
                    "0000000000110110100000000"
                    "1100111000101010000101111"
                    "1111100110000001110111011"
                    "1110111101100010100110011"
                    "0000000111001010111100111"
                    "1010101010010110000110000"
                    "1110110000101000110011010"
                    "0010001010011011000110001"
                    "0001100010110000011100110"
                    "1110011100101110111111001"
                    "0000000011000111100011001"
                    "1111111000100001101011000"
                    "1000001011001001100010110"
                    "1011101010010111111110011"
                    "1011101000101011101101001"
                    "1011101001111111000111010"
                    "1000001011110101110010101"
                    "1111111010101100000011011"
                },
        /* 36*/ { BARCODE_QRCODE, UNICODE_MODE, 4, 1, -1, 4 << 8, { 0, 0, "" }, "Študentska št. 2198390", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 4 Example 1 **NOT SAME** different encodation",
                    "1111111011000001001111111"
                    "1000001000000010101000001"
                    "1011101010011010101011101"
                    "1011101011010011001011101"
                    "1011101011110110001011101"
                    "1000001001001110101000001"
                    "1111111010101010101111111"
                    "0000000001010101000000000"
                    "1111001010000000110011101"
                    "1000100001000110110000011"
                    "1101011110000001000101111"
                    "0101010010011111101001101"
                    "1101101101010001000001000"
                    "0010100100110100101111001"
                    "0111011111001110010011011"
                    "1001010010001000100100001"
                    "0010001000110010111111010"
                    "0000000011101101100011100"
                    "1111111000011001101011111"
                    "1000001000101010100011010"
                    "1011101000111101111110110"
                    "1011101011101100101010001"
                    "1011101010011100100100110"
                    "1000001010100000100111111"
                    "1111111011101011000100011"
                },
        /* 37*/ { BARCODE_QRCODE, UNICODE_MODE, 4, 1, -1, -1, { 0, 0, "" }, "Szczegółowe dane kontaktowe:+48 22 694 60 00", -1, 0, 29, 29, 0, "AIM ITS/04-023:2022 ECI 4 Example 2 **NOT SAME** different encodation; BWIPP same encodation as example but uses mask 100 instead of 111",
                    "11111110000111011111001111111"
                    "10000010011001101100101000001"
                    "10111010000010110011101011101"
                    "10111010100010100010101011101"
                    "10111010110001101001101011101"
                    "10000010000111011101001000001"
                    "11111110101010101010101111111"
                    "00000000001100110111100000000"
                    "11000111010100010110100011000"
                    "11100000010010101001011111100"
                    "10010111001000101101010000111"
                    "11001101110010001001101110011"
                    "10000110010110110101011000001"
                    "01101101110001100011000111011"
                    "11001111001111001101000101110"
                    "10111100100100001111000110011"
                    "00001010101101001100000100100"
                    "10010001001011011000000101011"
                    "11110110011100110001101101010"
                    "10001101011010000001001111010"
                    "10110111101010010100111111010"
                    "00000000111000100001100011101"
                    "11111110110110010100101010100"
                    "10000010111100011011100011000"
                    "10111010001001100101111110001"
                    "10111010010011011000100101100"
                    "10111010000001100111000011110"
                    "10000010110011101010011100110"
                    "11111110101010100011111001100"
                },
        /* 38*/ { BARCODE_QRCODE, UNICODE_MODE, 5, 1, -1, -1, { 0, 0, "" }, "Liĥtenŝtejno", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 5 Example 1; BWIPP uses mask 011 instead of 101",
                    "111111100111101111111"
                    "100000100101001000001"
                    "101110100100001011101"
                    "101110101111001011101"
                    "101110101010101011101"
                    "100000100010101000001"
                    "111111101010101111111"
                    "000000000110100000000"
                    "110001110100100011000"
                    "111011000001010001101"
                    "100100111011011010111"
                    "100101010110011011000"
                    "100011101010110011111"
                    "000000001100010101100"
                    "111111101001011000010"
                    "100000101010011000011"
                    "101110100011100001110"
                    "101110100000010000000"
                    "101110100111010010011"
                    "100000101010010111111"
                    "111111101011110000010"
                },
        /* 39*/ { BARCODE_QRCODE, UNICODE_MODE, 5, 1, -1, 4 << 8, { 0, 0, "" }, "Liĥtenŝtejno", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 5 Example 1 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101111001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000111000000000"
                    "111100101010010011101"
                    "110101001111011111100"
                    "001001110110110111010"
                    "010111000100101001110"
                    "111000110001101000100"
                    "000000001101111100001"
                    "111111100111101110100"
                    "100000100100010110010"
                    "101110100110001100011"
                    "101110101010100010110"
                    "101110101100001001000"
                    "100000101011111110010"
                    "111111101101000110100"
                },
        /* 40*/ { BARCODE_QRCODE, UNICODE_MODE, 6, 1, -1, -1, { 0, 0, "" }, "Lietuvą", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 6 Example 1",
                    "111111101000101111111"
                    "100000101010001000001"
                    "101110100000101011101"
                    "101110100000001011101"
                    "101110101111101011101"
                    "100000101101101000001"
                    "111111101010101111111"
                    "000000001000100000000"
                    "111001101010011110011"
                    "111111001101001011011"
                    "100011101001100011101"
                    "000011011001000111100"
                    "110100101001101011111"
                    "000000001110000101000"
                    "111111100010011011001"
                    "100000101010100110101"
                    "101110100110011010101"
                    "101110100001011001000"
                    "101110101101110011011"
                    "100000101011011111011"
                    "111111101111111010101"
                },
        /* 41*/ { BARCODE_QRCODE, UNICODE_MODE, 7, 1, -1, -1, { 0, 0, "" }, "Россия", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 7 Example 1",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "101000010001010010111"
                    "110010110011011110101"
                    "101011010000111110011"
                    "100001101011001100101"
                    "000000001010011110100"
                    "111111101001101000110"
                    "100000100110001000101"
                    "101110100000111110001"
                    "101110101010011001111"
                    "101110100111011010001"
                    "100000101011100101011"
                    "111111101100010100110"
                },
        /* 42*/ { BARCODE_QRCODE, UNICODE_MODE, 7, 1, -1, -1, { 0, 0, "" }, "Монголулс", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 7 Example 2; BWIPP uses mask 000 instead of 111",
                    "111111100101101111111"
                    "100000101101001000001"
                    "101110101100101011101"
                    "101110100101001011101"
                    "101110101000101011101"
                    "100000101001101000001"
                    "111111101010101111111"
                    "000000001111100000000"
                    "110100110110001110110"
                    "111110010000001011100"
                    "010110100010110011010"
                    "100000011011011001111"
                    "000011110010110100101"
                    "000000001101011100100"
                    "111111101010010100110"
                    "100000100001111000101"
                    "101110100001000110001"
                    "101110101110000011111"
                    "101110100110100011101"
                    "100000101011000010111"
                    "111111101001110100110"
                },
        /* 43*/ { BARCODE_QRCODE, UNICODE_MODE, 7, 1, -1, 1 << 8, { 0, 0, "" }, "Монголулс", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 7 Example 2 **NOT SAME** explicit mask 000",
                    "111111100101101111111"
                    "100000100111001000001"
                    "101110101101101011101"
                    "101110100101001011101"
                    "101110100010101011101"
                    "100000100000101000001"
                    "111111101010101111111"
                    "000000001101100000000"
                    "111011111111011000100"
                    "111110010000001011100"
                    "000100110000100001000"
                    "101001010010010000110"
                    "000011110010110100101"
                    "000000001111001110110"
                    "111111101011011101111"
                    "100000101001111000101"
                    "101110101011010100011"
                    "101110100111001010110"
                    "101110101110100011101"
                    "100000101001010000101"
                    "111111101000111101111"
                },
        /* 44*/ { BARCODE_QRCODE, UNICODE_MODE, 8, 1, -1, -1, { 0, 0, "" }, "جواز السفر", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 8 Example 1",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "010000010011001010111"
                    "100011101101010100110"
                    "111110010000101101111"
                    "100010100111001100101"
                    "000000001101011110100"
                    "111111101011110011010"
                    "100000100001010111110"
                    "101110100010100000110"
                    "101110101011000111111"
                    "101110100000100010101"
                    "100000101110000010011"
                    "111111101000010100010"
                },
        /* 45*/ { BARCODE_QRCODE, UNICODE_MODE, 8, 1, -1, -1, { 0, 0, "" }, "المنشأ: المملكة العربية السعودية", -1, 0, 29, 29, 0, "AIM ITS/04-023:2022 ECI 8 Example 2; BWIPP uses mask 000 instead of 010",
                    "11111110001011110110001111111"
                    "10000010101001111101001000001"
                    "10111010000011010110101011101"
                    "10111010111010110111101011101"
                    "10111010010000011110001011101"
                    "10000010110110000000101000001"
                    "11111110101010101010101111111"
                    "00000000011100101110100000000"
                    "11111011110100000011110101010"
                    "10000100101011110000000111110"
                    "11110011101001110111101000100"
                    "10001100000011010010111101011"
                    "00001011111010111011000011001"
                    "00000001110000010110100111101"
                    "01101010010110000000001000100"
                    "01101100101100111011111101010"
                    "00001111001100000101100100101"
                    "10110100000011100101110111110"
                    "10001110001001100100001011010"
                    "10000101111011001010111101011"
                    "10011010001010101001111110010"
                    "00000000110000001110100011110"
                    "11111110110110000000101010000"
                    "10000010001100111000100010010"
                    "10111010101100000110111111111"
                    "10111010101011110100110010010"
                    "10111010100001100000001011110"
                    "10000010100011001101001000101"
                    "11111110110010101010110010100"
                },
        /* 46*/ { BARCODE_QRCODE, UNICODE_MODE, 8, 1, -1, 1 << 8, { 0, 0, "" }, "المنشأ: المملكة العربية السعودية", -1, 0, 29, 29, 1, "AIM ITS/04-023:2022 ECI 8 Example 2 **NOT SAME** explicit mask 000",
                    "11111110010011001110101111111"
                    "10000010001110111010001000001"
                    "10111010111011101110001011101"
                    "10111010011101110000101011101"
                    "10111010001000100110101011101"
                    "10000010010001000111101000001"
                    "11111110101010101010101111111"
                    "00000000111011101001100000000"
                    "11101111101100111011011000100"
                    "01000001101100110111000000110"
                    "11001011010001001111010000011"
                    "01001001000100010101111010011"
                    "00110011000010000011111011110"
                    "11000100110111010001100000101"
                    "01010010101110111000110000011"
                    "10101001101011111100111010010"
                    "00110111110100111101011100010"
                    "01110001000100100010110000110"
                    "10110110110001011100110011101"
                    "01000000111100001101111010011"
                    "10100010110010010001111110101"
                    "00000000110111001001100010110"
                    "11111110101110111000101010111"
                    "10000010101011111111100011010"
                    "10111010110100111110111111000"
                    "10111010001100110011110101010"
                    "10111010111001011000110011001"
                    "10000010100100001010001111101"
                    "11111110101010010010001010011"
                },
        /* 47*/ { BARCODE_QRCODE, UNICODE_MODE, 9, 1, -1, -1, { 0, 0, "" }, "Μέρος #. α123", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 9 Example 1 **NOT SAME** example uses mask 111 instead of 000",
                    "111111100100101111111"
                    "100000100111001000001"
                    "101110101100101011101"
                    "101110100101001011101"
                    "101110100011001011101"
                    "100000100001101000001"
                    "111111101010101111111"
                    "000000001100100000000"
                    "111011111111111000100"
                    "101100010001010100111"
                    "101101101011000010100"
                    "100010000101010110110"
                    "010111101110110010101"
                    "000000001001010110110"
                    "111111101000100011111"
                    "100000101000001010100"
                    "101110101100100101100"
                    "101110100010010010110"
                    "101110101010100101001"
                    "100000101111110111001"
                    "111111101101001101111"
                },
        /* 48*/ { BARCODE_QRCODE, UNICODE_MODE, 9, 1, -1, 8 << 8, { 0, 0, "" }, "Μέρος #. α123", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 9 Example 1 with explicit mask 111; BWIPP uses mask 000",
                    "111111100100101111111"
                    "100000101101001000001"
                    "101110101101101011101"
                    "101110100101001011101"
                    "101110101001001011101"
                    "100000101000101000001"
                    "111111101010101111111"
                    "000000001110100000000"
                    "110100110110101110110"
                    "101100010001010100111"
                    "111111111001010000110"
                    "101011001100011111111"
                    "010111101110110010101"
                    "000000001011000100100"
                    "111111101001101010110"
                    "100000100000001010100"
                    "101110100110110111110"
                    "101110101011011011111"
                    "101110100010100101001"
                    "100000101101100101011"
                    "111111101100000100110"
                },
        /* 49*/ { BARCODE_QRCODE, UNICODE_MODE, 9, 1, -1, 1 << 8, { 0, 0, "" }, "Μέρος #. α123", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 9 Example 1 **NOT SAME** explicit mask 000",
                    "111111100100101111111"
                    "100000100111001000001"
                    "101110101100101011101"
                    "101110100101001011101"
                    "101110100011001011101"
                    "100000100001101000001"
                    "111111101010101111111"
                    "000000001100100000000"
                    "111011111111111000100"
                    "101100010001010100111"
                    "101101101011000010100"
                    "100010000101010110110"
                    "010111101110110010101"
                    "000000001001010110110"
                    "111111101000100011111"
                    "100000101000001010100"
                    "101110101100100101100"
                    "101110100010010010110"
                    "101110101010100101001"
                    "100000101111110111001"
                    "111111101101001101111"
                },
        /* 50*/ { BARCODE_QRCODE, UNICODE_MODE, 10, 1, -1, -1, { 0, 0, "" }, "דרכון", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 10 Example 1; BWIPP uses mask 000 instead of 101",
                    "111111100000101111111"
                    "100000100010101000001"
                    "101110100011101011101"
                    "101110101010101011101"
                    "101110101100101011101"
                    "100000100101001000001"
                    "111111101010101111111"
                    "000000000000000000000"
                    "110001110001000011000"
                    "101110010001110011011"
                    "001110111110101100010"
                    "011100010001111000100"
                    "010100111101111110011"
                    "000000001100100111000"
                    "111111101111010011010"
                    "100000101100001000000"
                    "101110100001010010101"
                    "101110100101111101000"
                    "101110100001110011011"
                    "100000101111111010011"
                    "111111101110100011010"
                },
        /* 51*/ { BARCODE_QRCODE, UNICODE_MODE, 10, 1, -1, 1 << 8, { 0, 0, "" }, "דרכון", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 10 Example 1 **NOT SAME** explicit mask 000",
                    "111111100101101111111"
                    "100000100111001000001"
                    "101110101101101011101"
                    "101110100101001011101"
                    "101110100010101011101"
                    "100000100000101000001"
                    "111111101010101111111"
                    "000000001101100000000"
                    "111011111111011000100"
                    "010001001110001100100"
                    "000000110000100010011"
                    "101001000100001101010"
                    "000001101000101011001"
                    "000000001001010010110"
                    "111111101001011101011"
                    "100000101011110111111"
                    "101110101111011100100"
                    "101110100000001000110"
                    "101110101100100110001"
                    "100000101010001111101"
                    "111111101000101101011"
                },
        /* 52*/ { BARCODE_QRCODE, UNICODE_MODE, 10, 1, -1, -1, { 0, 0, "" }, "מספר חלק: A20200715001", -1, 0, 25, 25, 0, "AIM ITS/04-023:2022 ECI 10 Example 2 **NOT SAME** different encodation; BWIPP uses mask 000 instead of 100",
                    "1111111011101100001111111"
                    "1000001011100001001000001"
                    "1011101010110001001011101"
                    "1011101011010000001011101"
                    "1011101000000110001011101"
                    "1000001010011010001000001"
                    "1111111010101010101111111"
                    "0000000001001001100000000"
                    "1100111000010110000101111"
                    "1011100011101011010011100"
                    "0001001000011110110110001"
                    "0111110010110101001111111"
                    "1001111010101101000111010"
                    "1001010100000110100001110"
                    "0011011100100000110010011"
                    "0001110001101010111110110"
                    "1111011000010110111110001"
                    "0000000010101000100011101"
                    "1111111001111111101010000"
                    "1000001010010101100011011"
                    "1011101010001100111110000"
                    "1011101001000111001000001"
                    "1011101001000001110101010"
                    "1000001011001010100011101"
                    "1111111011010110001000011"
                },
        /* 53*/ { BARCODE_QRCODE, UNICODE_MODE, 10, 1, -1, 1 << 8, { 0, 0, "" }, "מספר חלק: A20200715001", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 10 Example 2 **NOT SAME** different encodation",
                    "1111111001001000101111111"
                    "1000001000111010001000001"
                    "1011101011101010001011101"
                    "1011101001110100101011101"
                    "1011101000100010101011101"
                    "1000001001000001001000001"
                    "1111111010101010101111111"
                    "0000000011101101000000000"
                    "1110111110110010111000100"
                    "0000110000110000001000111"
                    "1010011011000101101101010"
                    "0011010110010001101011011"
                    "1101011110001001100011110"
                    "0010000111011101111010101"
                    "1000001111111011101001000"
                    "0101010101001110011010010"
                    "1011111100110010111110101"
                    "0000000011110011100010110"
                    "1111111010100100101011011"
                    "1000001010110001100011111"
                    "1011101010101000111110100"
                    "1011101000011100010011010"
                    "1011101010011010101110001"
                    "1000001011101110000111001"
                    "1111111011110010101100111"
                },
        /* 54*/ { BARCODE_QRCODE, UNICODE_MODE, 11, 1, -1, -1, { 0, 0, "" }, "Amerika Birleşik Devletleri", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 11 Example 1",
                    "1111111000111100101111111"
                    "1000001001101101001000001"
                    "1011101000001000101011101"
                    "1011101010000010001011101"
                    "1011101011001001001011101"
                    "1000001000011011001000001"
                    "1111111010101010101111111"
                    "0000000000110001100000000"
                    "1100011101001011000011000"
                    "1010000001011011001011111"
                    "0101111100100110111011101"
                    "0011010001010001101001101"
                    "1000001111010100011010101"
                    "1110010010000010100001010"
                    "1001101010001111011011100"
                    "1001000001110001000111111"
                    "1011111100001111111110001"
                    "0000000010100000100011001"
                    "1111111011001001101010001"
                    "1000001010110101100010010"
                    "1011101001101010111111100"
                    "1011101000100010000101101"
                    "1011101001000110011011101"
                    "1000001010110000111010110"
                    "1111111010110101100010101"
                },
        /* 55*/ { BARCODE_QRCODE, UNICODE_MODE, 11, 1, -1, -1, { 0, 0, "" }, "Biniş kartı #120921039", -1, 0, 25, 25, 0, "AIM ITS/04-023:2022 ECI 11 Example 2 **NOT SAME** different encodation; BWIPP same encodation as Zint but uses mask 101 instead of 111",
                    "1111111000100110101111111"
                    "1000001011100110001000001"
                    "1011101010000001001011101"
                    "1011101001001011001011101"
                    "1011101011101110101011101"
                    "1000001010101101001000001"
                    "1111111010101010101111111"
                    "0000000010110000100000000"
                    "1101001100011100101110110"
                    "0011000111011101000100011"
                    "1111101100011000111100101"
                    "0101010011111101011011100"
                    "0001011110110010001101111"
                    "0101100100010100111010100"
                    "1011001100010110101010000"
                    "0111000011010000001000110"
                    "1110001000101100111110101"
                    "0000000010101111100010000"
                    "1111111010011010101010011"
                    "1000001001001011100011001"
                    "1011101000100000111111110"
                    "1011101010010011100100110"
                    "1011101000100000001111101"
                    "1000001011100010011100111"
                    "1111111010001001010010011"
                },
        /* 56*/ { BARCODE_QRCODE, UNICODE_MODE, 11, 1, -1, 6 << 8, { 0, 0, "" }, "Biniş kartı #120921039", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 11 Example 2 **NOT SAME** different encodation",
                    "1111111001110011101111111"
                    "1000001000011111101000001"
                    "1011101001110000101011101"
                    "1011101010110100101011101"
                    "1011101010101001101011101"
                    "1000001001100010001000001"
                    "1111111010101010101111111"
                    "0000000001001001000000000"
                    "1100011101101101000011000"
                    "1100110000100010111011100"
                    "1000101011011111111011101"
                    "1010010100110010010100101"
                    "0100001011100111011000101"
                    "1100010101101101000011011"
                    "1010111101100111011011110"
                    "1000110100101111110111001"
                    "1001001111101011111111101"
                    "0000000011100000100011001"
                    "1111111011001111101011001"
                    "1000001010110010100010110"
                    "1011101001010001111110000"
                    "1011101001101100011011001"
                    "1011101001100111001000101"
                    "1000001010101101010011110"
                    "1111111011011100000111001"
                },
        /* 57*/ { BARCODE_QRCODE, UNICODE_MODE, 12, 1, -1, -1, { 0, 0, "" }, "Kūrybiškumą", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 12 Example 1; BWIPP uses mask 111 instead of 100",
                    "111111101111101111111"
                    "100000101010101000001"
                    "101110101000001011101"
                    "101110101111001011101"
                    "101110100000101011101"
                    "100000101101001000001"
                    "111111101010101111111"
                    "000000000111100000000"
                    "110011100101000101111"
                    "100110001010110101100"
                    "010011100011011101001"
                    "000000001101001101100"
                    "001101101010110101011"
                    "000000001101101010011"
                    "111111100101001110110"
                    "100000101100101100100"
                    "101110101110000100000"
                    "101110100111101101111"
                    "101110100100111100100"
                    "100000101011110001011"
                    "111111101001101100001"
                },
        /* 58*/ { BARCODE_QRCODE, UNICODE_MODE, 12, 1, -1, 8 << 8, { 0, 0, "" }, "Kūrybiškumą", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 12 Example 1 **NOT SAME** explicit mask 111",
                    "111111100101101111111"
                    "100000101101001000001"
                    "101110101100101011101"
                    "101110100101001011101"
                    "101110101000101011101"
                    "100000101001101000001"
                    "111111101010101111111"
                    "000000001111100000000"
                    "110100110110001110110"
                    "001011000111011000001"
                    "101100111100100010110"
                    "011011010110010110111"
                    "011111111000100111001"
                    "000000001010010101100"
                    "111111101001101010010"
                    "100000100110111110110"
                    "101110100110000100000"
                    "101110101011001001011"
                    "101110100001010001001"
                    "100000101011110001011"
                    "111111101010110111010"
                },
        /* 59*/ { BARCODE_QRCODE, UNICODE_MODE, 13, 1, -1, -1, { 0, 0, "" }, "บาร๋แค่ด", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 13 Example 1; BWIPP uses mask 001 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "011110001111001101110"
                    "101100111101010101010"
                    "001101001100110110111"
                    "010011101111001000001"
                    "000000001100001010100"
                    "111111101111111010110"
                    "100000100110000111000"
                    "101110100010111110000"
                    "101110101000011011111"
                    "101110100111001000101"
                    "100000101011100101011"
                    "111111101010000100010"
                },
        /* 60*/ { BARCODE_QRCODE, UNICODE_MODE, 13, 1, -1, 2 << 8, { 0, 0, "" }, "บาร๋แค่ด", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 13 Example 1 **NOT SAME** explicit mask 001",
                    "111111101111101111111"
                    "100000101101101000001"
                    "101110100111001011101"
                    "101110100101101011101"
                    "101110101000101011101"
                    "100000101010001000001"
                    "111111101010101111111"
                    "000000001111000000000"
                    "111001101111111110011"
                    "001011011010011000100"
                    "101011111010010010010"
                    "010001010000101010100"
                    "000110111010011101011"
                    "000000001011001101100"
                    "111111100011100110101"
                    "100000101011010010010"
                    "101110100101111001000"
                    "101110100100000111100"
                    "101110101010011101111"
                    "100000101100100010011"
                    "111111101110011000001"
                },
        /* 61*/ { BARCODE_QRCODE, UNICODE_MODE, 15, 1, -1, -1, { 0, 0, "" }, "uzņēmums", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 15 Example 1; BWIPP uses mask 011 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "110000001011000000010"
                    "000011101101010101010"
                    "011011001110100011011"
                    "000010110011011111101"
                    "000000001100010010000"
                    "111111101101111100010"
                    "100000100000010111101"
                    "101110100110100100000"
                    "101110101110011011111"
                    "101110100111001111001"
                    "100000101111100101011"
                    "111111101000000101010"
                },
        /* 62*/ { BARCODE_QRCODE, UNICODE_MODE, 15, 1, -1, 4 << 8, { 0, 0, "" }, "uzņēmums", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 15 Example 1 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101110001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000110000000000"
                    "111100101010010011101"
                    "000001011010110001100"
                    "110010111100100100100"
                    "010101000000101101010"
                    "001100111101010001100"
                    "000000001010011100001"
                    "111111100100001101100"
                    "100000100001100110011"
                    "101110100111010101110"
                    "101110101000010101110"
                    "101110101001000001000"
                    "100000101001101011010"
                    "111111101001110100100"
                },
        /* 63*/ { BARCODE_QRCODE, UNICODE_MODE, 16, 1, -1, -1, { 0, 0, "" }, "ṁórṡáċ", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 16 Example 1; BWIPP uses mask 001 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "000100000111010001111"
                    "011010111101011101001"
                    "001001001000111111011"
                    "110111110011001001101"
                    "000000001100011010100"
                    "111111101001101010010"
                    "100000100010001111010"
                    "101110100100111111011"
                    "101110101000011011001"
                    "101110100111011101001"
                    "100000101101100001011"
                    "111111101010010001010"
                },
        /* 64*/ { BARCODE_QRCODE, UNICODE_MODE, 16, 1, -1, 2 << 8, { 0, 0, "" }, "ṁórṡáċ", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 16 Example 1 **NOT SAME** explicit mask 001",
                    "111111101111101111111"
                    "100000101101101000001"
                    "101110100111001011101"
                    "101110100101101011101"
                    "101110101000101011101"
                    "100000101010001000001"
                    "111111101010101111111"
                    "000000001111000000000"
                    "111001101111111110011"
                    "010001010010000100101"
                    "011101111010011010001"
                    "010101010100100011000"
                    "100010100110011100111"
                    "000000001011011101100"
                    "111111100101110110001"
                    "100000101111011010000"
                    "101110100011111000011"
                    "101110100100000111010"
                    "101110101010001000011"
                    "100000101010100110011"
                    "111111101110001101001"
                },
        /* 65*/ { BARCODE_QRCODE, UNICODE_MODE, 17, 1, -1, -1, { 0, 0, "" }, "Price: €13.50", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 17 Example 1",
                    "111111100100101111111"
                    "100000101101001000001"
                    "101110101101101011101"
                    "101110100101001011101"
                    "101110101000101011101"
                    "100000101000001000001"
                    "111111101010101111111"
                    "000000001110100000000"
                    "110100110111101110110"
                    "100010000000010110000"
                    "100010101001110011100"
                    "101110010101111010011"
                    "011001111010101010101"
                    "000000001101101010000"
                    "111111101000001110010"
                    "100000100111100111000"
                    "101110100110111100011"
                    "101110101011111011001"
                    "101110100100110010001"
                    "100000101000100101111"
                    "111111101101110110110"
                },
        /* 66*/ { BARCODE_QRCODE, UNICODE_MODE, 18, 1, -1, -1, { 0, 0, "" }, "Te słowa są głębokie", -1, 0, 25, 25, 0, "AIM ITS/04-023:2022 ECI 18 Example 1; BWIPP uses mask 001 instead of 100",
                    "1111111010000111001111111"
                    "1000001010011000001000001"
                    "1011101011001100001011101"
                    "1011101011101111001011101"
                    "1011101001101100101011101"
                    "1000001011100001101000001"
                    "1111111010101010101111111"
                    "0000000000110100000000000"
                    "1100111000101011100101111"
                    "0001100010000101011111001"
                    "1010101111100111010011000"
                    "1010100011001100111100100"
                    "1101011110010111101001011"
                    "1001100100101101101010001"
                    "0011101111111111100111000"
                    "0011100111110000001101100"
                    "1100101100101100111110001"
                    "0000000011100100100011001"
                    "1111111000100001101010000"
                    "1000001010001001100011111"
                    "1011101011010110111111101"
                    "1011101001101010101000111"
                    "1011101000011111101110010"
                    "1000001011110101011011101"
                    "1111111010001100000011111"
                },
        /* 67*/ { BARCODE_QRCODE, UNICODE_MODE, 18, 1, -1, 2 << 8, { 0, 0, "" }, "Te słowa są głębokie", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 18 Example 1 **NOT SAME** explicit mask 001",
                    "1111111011110110101111111"
                    "1000001010010110001000001"
                    "1011101001000010001011101"
                    "1011101000011110101011101"
                    "1011101010011101001011101"
                    "1000001011101111101000001"
                    "1111111010101010101111111"
                    "0000000011000101100000000"
                    "1110011011011010011110011"
                    "1111100100001011010001000"
                    "0100101001101001011101001"
                    "1011010010111101001101010"
                    "1100101111100110011000101"
                    "0111100010100011100100000"
                    "1101101001110001101001001"
                    "0010010110000001111100010"
                    "1101011101011101111111111"
                    "0000000011101010100011000"
                    "1111111000101111101010001"
                    "1000001011111000100010001"
                    "1011101000100111111110011"
                    "1011101001100100100110110"
                    "1011101010010001100000011"
                    "1000001010000100101010011"
                    "1111111011111101110010001"
                },
        /* 68*/ { BARCODE_QRCODE, UNICODE_MODE, 20, 1, -1, -1, { 0, 0, "" }, "バーコード", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 20 Example 1; BWIPP uses mask 011 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "101011011011011001000"
                    "110011111001000000101"
                    "000111011100111101111"
                    "100111110011001011101"
                    "000000001000011110000"
                    "111111101101110110001"
                    "100000100110000111110"
                    "101110100110111000101"
                    "101110101000011010001"
                    "101110100011001010101"
                    "100000101011100110011"
                    "111111101010000101010"
                },
        /* 69*/ { BARCODE_QRCODE, UNICODE_MODE, 20, 1, -1, 4 << 8, { 0, 0, "" }, "バーコード", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 20 Example 1 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101110001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000110000000000"
                    "111100101010010011101"
                    "011010001010101000110"
                    "000010101000110001011"
                    "001001010010110011110"
                    "101001111101000101100"
                    "000000001110010000001"
                    "111111100100000111111"
                    "100000100111110110000"
                    "101110100111001001011"
                    "101110101110010100000"
                    "101110101101000100100"
                    "100000101101101000010"
                    "111111101011110100100"
                },
        /* 70*/ { BARCODE_QRCODE, UNICODE_MODE, 20, 1, -1, -1, { 0, 0, "" }, "東京都", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 20 Example 2; BWIPP uses mask 000 instead of 100",
                    "111111101111101111111"
                    "100000101010101000001"
                    "101110101000001011101"
                    "101110101111001011101"
                    "101110100000101011101"
                    "100000101101001000001"
                    "111111101010101111111"
                    "000000000111100000000"
                    "110011100101000101111"
                    "011110011101100001100"
                    "111011100111001111100"
                    "101110011100011110100"
                    "010010111000111000011"
                    "000000001100111011111"
                    "111111100000110011101"
                    "100000101001100101000"
                    "101110101101001101101"
                    "101110100011100111001"
                    "101110100101001100100"
                    "100000101010011010011"
                    "111111101100111011001"
                },
        /* 71*/ { BARCODE_QRCODE, UNICODE_MODE, 20, 1, -1, 1 << 8, { 0, 0, "" }, "東京都", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 20 Example 2 **NOT SAME** explicit mask 000",
                    "111111100101101111111"
                    "100000100111001000001"
                    "101110101101101011101"
                    "101110100101001011101"
                    "101110100010101011101"
                    "100000100000101000001"
                    "111111101010101111111"
                    "000000001101100000000"
                    "111011111111011000100"
                    "110011010000001100001"
                    "010110101010100010001"
                    "111100001110001100110"
                    "000000101010101010001"
                    "000000001001010110010"
                    "111111101101011110000"
                    "100000101011110111010"
                    "101110101111011111111"
                    "101110100110001010100"
                    "101110101000100001001"
                    "100000101000001000001"
                    "111111101110101001011"
                },
        /* 72*/ { BARCODE_QRCODE, UNICODE_MODE, 21, 1, -1, -1, { 0, 0, "" }, "Študentska št. 2198390", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 21 Example 1 **NOT SAME** different encodation",
                    "1111111010000110001111111"
                    "1000001010011110101000001"
                    "1011101011001111101011101"
                    "1011101011101011101011101"
                    "1011101001101010001011101"
                    "1000001011100100001000001"
                    "1111111010101010101111111"
                    "0000000000110110100000000"
                    "1100111000101010000101111"
                    "1000000010000001110111000"
                    "1001011011100010100110011"
                    "0110100101001010111100110"
                    "1111111100010110000110000"
                    "1011000111101000110011010"
                    "0000001011011011000110001"
                    "0011000100010000011100110"
                    "1101101101001110111111001"
                    "0000000011000110100011001"
                    "1111111001100000101011000"
                    "1000001011001001100010100"
                    "1011101011110111111110011"
                    "1011101001001011001101011"
                    "1011101001011111000111010"
                    "1000001010010101110010101"
                    "1111111010101100000011011"
                },
        /* 73*/ { BARCODE_QRCODE, UNICODE_MODE, 22, 1, -1, -1, { 0, 0, "" }, "Россия", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 22 Example 1",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "101000001111010010111"
                    "011101110101011111101"
                    "110011010010111010011"
                    "101010101111001100101"
                    "000000001000011110100"
                    "111111101001101001010"
                    "100000100010001110111"
                    "101110100000111110001"
                    "101110101010011001101"
                    "101110100011011011001"
                    "100000101001100001011"
                    "111111101100010100110"
                },
        /* 74*/ { BARCODE_QRCODE, UNICODE_MODE, 22, 1, -1, -1, { 0, 0, "" }, "Монголулс", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 22 Example 2; BWIPP uses mask 000 instead of 100",
                    "111111101111101111111"
                    "100000101010101000001"
                    "101110101000001011101"
                    "101110101111001011101"
                    "101110100000101011101"
                    "100000101101001000001"
                    "111111101010101111111"
                    "000000000111100000000"
                    "110011100101000101111"
                    "110010000001100111001"
                    "101001110101011101001"
                    "011011001100000100100"
                    "110110111100100110111"
                    "000000001100100011011"
                    "111111100000101001110"
                    "100000101011101100101"
                    "101110101111000110001"
                    "101110100110100111001"
                    "101110100101011111100"
                    "100000101001000100111"
                    "111111101000101111101"
                },
        /* 75*/ { BARCODE_QRCODE, UNICODE_MODE, 22, 1, -1, 1 << 8, { 0, 0, "" }, "Монголулс", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 22 Example 2 **NOT SAME** explicit mask 000",
                    "111111100101101111111"
                    "100000100111001000001"
                    "101110101101101011101"
                    "101110100101001011101"
                    "101110100010101011101"
                    "100000100000101000001"
                    "111111101010101111111"
                    "000000001101100000000"
                    "111011111111011000100"
                    "011111001100001010100"
                    "000100111000110000100"
                    "001001011110010110110"
                    "100100101110110100101"
                    "000000001001001110110"
                    "111111101101000100011"
                    "100000101001111110111"
                    "101110101101010100011"
                    "101110100011001010100"
                    "101110101000110010001"
                    "100000101011010110101"
                    "111111101010111101111"
                },
        /* 76*/ { BARCODE_QRCODE, UNICODE_MODE, 23, 1, -1, -1, { 0, 0, "" }, "bœuf", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 23 Example 1; BWIPP uses mask 000 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "010001001111010100010"
                    "010001100011001110001"
                    "111111001100111110011"
                    "010100110101011100001"
                    "000000001010011111100"
                    "111111101011101010110"
                    "100000100100001100101"
                    "101110100110111000101"
                    "101110101100011111001"
                    "101110100101011111001"
                    "100000101001100100011"
                    "111111101000010110110"
                },
        /* 77*/ { BARCODE_QRCODE, UNICODE_MODE, 23, 1, -1, 1 << 8, { 0, 0, "" }, "bœuf", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 23 Example 1 ** NOT SAME** explicit mask 000",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "010001001111010100010"
                    "000011110001011100011"
                    "110110000101110111010"
                    "010100110101011100001"
                    "000000001000001101110"
                    "111111101010100011111"
                    "100000101100001100101"
                    "101110101100101010111"
                    "101110100101010110000"
                    "101110101101011111001"
                    "100000101011110110001"
                    "111111101001011111111"
                },
        /* 78*/ { BARCODE_QRCODE, UNICODE_MODE, 24, 1, -1, -1, { 0, 0, "" }, "جواز السفر", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 24 Example 1",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "001010011111001010111"
                    "101100101101000110100"
                    "100010010001110100110"
                    "100111110011011100101"
                    "000000001011001100110"
                    "111111101010111010011"
                    "100000101001010110010"
                    "101110101110110010000"
                    "101110100010001110100"
                    "101110101100111010101"
                    "100000101010001000001"
                    "111111101101011101011"
                },
        /* 79*/ { BARCODE_QRCODE, UNICODE_MODE, 24, 1, -1, -1, { 0, 0, "" }, "المنشأ: المملكة العربية السعودية", -1, 0, 29, 29, 0, "AIM ITS/04-023:2022 ECI 24 Example 2; BWIPP uses mask 000 instead of 100",
                    "11111110111010000111101111111"
                    "10000010111000001100101000001"
                    "10111010101101011000101011101"
                    "10111010110100111111101011101"
                    "10111010000001101101101011101"
                    "10000010100111110001001000001"
                    "11111110101010101010101111111"
                    "00000000010010100000100000000"
                    "11001110000101110010100101111"
                    "10011100111010000000111010010"
                    "10011111000111111001100110111"
                    "11111101101101011100110010010"
                    "01010011101011001010000110011"
                    "01100001000001100111111010011"
                    "01001010010000001110000110101"
                    "01111000100010110101110011011"
                    "10100011111101111100000101111"
                    "11111101001010001100010110000"
                    "00010110111111101010000101011"
                    "00001000110101000100101011010"
                    "11101010100011011010111111100"
                    "00000000110001111101100010000"
                    "11111110000000001110101010001"
                    "10000010110010110110100010011"
                    "10111010111101110111111110001"
                    "10111010011010011101011011110"
                    "10111010010111111110000101111"
                    "10000010111101000011000110100"
                    "11111110111011010111000011010"
                },
        /* 80*/ { BARCODE_QRCODE, UNICODE_MODE, 24, 1, -1, 1 << 8, { 0, 0, "" }, "المنشأ: المملكة العربية السعودية", -1, 0, 29, 29, 1, "AIM ITS/04-023:2022 ECI 24 Example 2 **NOT SAME** explicit mask 000",
                    "11111110010011001110101111111"
                    "10000010001110111010001000001"
                    "10111010111011101110001011101"
                    "10111010011101110110101011101"
                    "10111010001000100100101011101"
                    "10000010010001000111101000001"
                    "11111110101010101010101111111"
                    "00000000111011101001100000000"
                    "11101111101100111011111000100"
                    "00101000001100110110001100100"
                    "00101011110001001111010000001"
                    "10110100100100010101111011011"
                    "00011010100010000011001111010"
                    "11010101110111010001001100101"
                    "11111110100110111000110000011"
                    "00110001101011111100111010010"
                    "11101010110100110101001100110"
                    "01001001111100111010100000110"
                    "10100010001001011100110011101"
                    "01000001111100001101100010011"
                    "10100011101010010011111110101"
                    "00000000100111001011100010110"
                    "11111110110110111000101010111"
                    "10000010111011111111100011010"
                    "10111010110100111110111111000"
                    "10111010001100101011101101000"
                    "10111010100001001000110011001"
                    "10000010110100001010001111101"
                    "11111110110010011110001010011"
                },
        /* 81*/ { BARCODE_QRCODE, UNICODE_MODE, 25, 1, -1, -1, { 0, 0, "" }, "条码", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 25 Example 1",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "000001010011010100010"
                    "010011110001011100011"
                    "100001001011110110010"
                    "001100101101011101001"
                    "000000001100001100010"
                    "111111101010100010011"
                    "100000101110001101000"
                    "101110101100101011100"
                    "101110100001010000000"
                    "101110101001011011001"
                    "100000101011110101101"
                    "111111101111011010111"
                },
        /* 82*/ { BARCODE_QRCODE, UNICODE_MODE, 25, 1, -1, -1, { 0, 0, "" }, "バーコード", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 25 Example 2; BWIPP uses mask 000 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "011011010111010011011"
                    "110010110011000001010"
                    "111100010110111010011"
                    "000001100011001000101"
                    "000000001001011110100"
                    "111111101001111010110"
                    "100000100001001011000"
                    "101110100100111000110"
                    "101110101001011100001"
                    "101110100000010101001"
                    "100000101001000001011"
                    "111111101110000101110"
                },
        /* 83*/ { BARCODE_QRCODE, UNICODE_MODE, 25, 1, -1, 1 << 8, { 0, 0, "" }, "バーコード", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 25 Example 2 **NOT SAME** explicit mask 000",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "011011010111010011011"
                    "100000100001010011000"
                    "110101011111110011010"
                    "000001100011001000101"
                    "000000001011001100110"
                    "111111101000110011111"
                    "100000101001001011000"
                    "101110101110101010100"
                    "101110100000010101000"
                    "101110101000010101001"
                    "100000101011010011001"
                    "111111101111001100111"
                },
        /* 84*/ { BARCODE_QRCODE, UNICODE_MODE, 25, 1, -1, -1, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 25 Example 3",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "011011000011010101111"
                    "110010110001001101011"
                    "011111001111110000110"
                    "100010110011001100101"
                    "000000001100001101010"
                    "111111101100100111111"
                    "100000101100001010000"
                    "101110101010101000100"
                    "101110100001010100100"
                    "101110101011011110101"
                    "100000101101110000101"
                    "111111101001011111011"
                },
        /* 85*/ { BARCODE_QRCODE, UNICODE_MODE, 26, 1, -1, -1, { 0, 0, "" }, "条码", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 26 Example 1; BWIPP uses mask 000 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "010111001111010000111"
                    "111010100011011000001"
                    "111010010000111101011"
                    "011001110101001000001"
                    "000000001100011111100"
                    "111111101111101010110"
                    "100000100110001110111"
                    "101110100010111011110"
                    "101110101110011010101"
                    "101110100011011110001"
                    "100000101111100101011"
                    "111111101110010100110"
                },
        /* 86*/ { BARCODE_QRCODE, UNICODE_MODE, 26, 1, -1, 1 << 8, { 0, 0, "" }, "条码", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 26 Example 1 **NOT SAME** explicit mask 000",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "010111001111010000111"
                    "101000110001001010011"
                    "110011011001110100010"
                    "011001110101001000001"
                    "000000001110001101110"
                    "111111101110100011111"
                    "100000101110001110111"
                    "101110101000101001100"
                    "101110100111010011100"
                    "101110101011011110001"
                    "100000101101110111001"
                    "111111101111011101111"
                },
        /* 87*/ { BARCODE_QRCODE, UNICODE_MODE, 26, 2, -1, -1, { 0, 0, "" }, "バーコード", -1, 0, 25, 25, 0, "AIM ITS/04-023:2022 ECI 26 Example 2; BWIPP uses mask 101 instead of 010",
                    "1111111001111101001111111"
                    "1000001001001111001000001"
                    "1011101011101101001011101"
                    "1011101011111011101011101"
                    "1011101010011001001011101"
                    "1000001010100000001000001"
                    "1111111010101010101111111"
                    "0000000010100011000000000"
                    "1011111001011000101111100"
                    "0000010010010110000001110"
                    "0110001111000111000001001"
                    "1010010010111101010011000"
                    "1111111010111010101110110"
                    "1000110001100000010001110"
                    "1001011111111001010010010"
                    "1010100110110010100111001"
                    "1010101001110000111111001"
                    "0000000011001111100010101"
                    "1111111001000111101011111"
                    "1000001011001100100011000"
                    "1011101010101010111110100"
                    "1011101011100000101001111"
                    "1011101010011001110111001"
                    "1000001001110010100111010"
                    "1111111010110001001010111"
                },
        /* 88*/ { BARCODE_QRCODE, UNICODE_MODE, 26, 2, -1, 6 << 8, { 0, 0, "" }, "バーコード", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 26 Example 2 **NOT SAME** explicit mask 101",
                    "1111111001001011101111111"
                    "1000001010001110001000001"
                    "1011101011101101001011101"
                    "1011101010011000001011101"
                    "1011101000011001001011101"
                    "1000001001100001001000001"
                    "1111111010101010101111111"
                    "0000000011100010000000000"
                    "1000001011011000111001110"
                    "0011110001110101100010010"
                    "0110001111000111000001001"
                    "1011010011111100010010000"
                    "1001001100001100011000000"
                    "1001110000100001010000110"
                    "1001011111111001010010010"
                    "1001000101010001000100101"
                    "1010101001110000111111001"
                    "0000000010001110100011101"
                    "1111111001110001101011001"
                    "1000001000001101100010000"
                    "1011101000101010111110100"
                    "1011101000000011001010011"
                    "1011101000011001110111001"
                    "1000001000110011100110010"
                    "1111111010000111111100001"
                },
        /* 89*/ { BARCODE_QRCODE, UNICODE_MODE, 26, 2, -1, -1, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 26 Example 3; BWIPP uses mask 111 instead of 001",
                    "111111101011001111111"
                    "100000100001001000001"
                    "101110101100101011101"
                    "101110100001001011101"
                    "101110100111101011101"
                    "100000101001101000001"
                    "111111101010101111111"
                    "000000000110100000000"
                    "101000110110000100101"
                    "111001010101011001110"
                    "110010100011110011010"
                    "010000010011001100100"
                    "010101100111100100011"
                    "000000001110001100100"
                    "111111101110001000001"
                    "100000100010100110001"
                    "101110100110000101110"
                    "101110100100001000110"
                    "101110101011101000111"
                    "100000100010011011011"
                    "111111101011111001101"
                },
        /* 90*/ { BARCODE_QRCODE, UNICODE_MODE, 26, 2, -1, 8 << 8, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 26 Example 3 **NOT SAME** explicit mask 111",
                    "111111100110001111111"
                    "100000100110001000001"
                    "101110100000101011101"
                    "101110100100001011101"
                    "101110100000101011101"
                    "100000101101101000001"
                    "111111101010101111111"
                    "000000000001100000000"
                    "100101101010010100000"
                    "101100000000001100100"
                    "110101100100110100010"
                    "001100001111010000111"
                    "000000110010110001001"
                    "000000001001001011100"
                    "111111100010010100010"
                    "100000101111110011011"
                    "101110100001000010110"
                    "101110101000010100101"
                    "101110100110111101101"
                    "100000100101011100011"
                    "111111101111100101110"
                },
        /* 91*/ { BARCODE_QRCODE, UNICODE_MODE, 27, 2, -1, 5 << 8, { 0, 0, "" }, "sn:7QPB4MN", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 27 Example 1 **NOT SAME** different encodation; explicit mask 100 (auto 011) to match BWIPP",
                    "111111101110101111111"
                    "100000100110001000001"
                    "101110100011001011101"
                    "101110101101001011101"
                    "101110101100101011101"
                    "100000101001001000001"
                    "111111101010101111111"
                    "000000001011100000000"
                    "100010111101011111001"
                    "011010010001101101110"
                    "101110101111001010100"
                    "110010010110010011000"
                    "001111110110100011011"
                    "000000001010100111111"
                    "111111101000100111110"
                    "100000100111111110111"
                    "101110101011001110010"
                    "101110100000100110101"
                    "101110100101011011100"
                    "100000100101001111011"
                    "111111101010010100101"
                },
        /* 92*/ { BARCODE_QRCODE, UNICODE_MODE, 28, 1, -1, -1, { 0, 0, "" }, "條碼", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 28 Example 1",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "000101001011010101110"
                    "000000101101011101011"
                    "100101001011110111010"
                    "010001100101011101101"
                    "000000001000001100110"
                    "111111101000100011011"
                    "100000101110001101110"
                    "101110101100101010110"
                    "101110100011010101100"
                    "101110101001011001001"
                    "100000101101110000101"
                    "111111101101011111111"
                },
        /* 93*/ { BARCODE_QRCODE, UNICODE_MODE, 29, 1, -1, -1, { 0, 0, "" }, "条码", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 29 Example 1",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "100001010001010100110"
                    "110111111101011100111"
                    "000011001001110110110"
                    "001101101111011100101"
                    "000000001000001100110"
                    "111111101100100011011"
                    "100000101110001100000"
                    "101110101000101011110"
                    "101110100011010010100"
                    "101110101101011000101"
                    "100000101111110111001"
                    "111111101101011100011"
                },
        /* 94*/ { BARCODE_QRCODE, UNICODE_MODE, 29, 1, -1, -1, { 0, 0, "" }, "北京", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 29 Example 2",
                    "111111100100101111111"
                    "100000101001001000001"
                    "101110100100001011101"
                    "101110101001001011101"
                    "101110100011101011101"
                    "100000101110101000001"
                    "111111101010101111111"
                    "000000000011100000000"
                    "111110111100110101010"
                    "001011001010100100000"
                    "010100101111010011010"
                    "011101010100000110100"
                    "001010110001010011100"
                    "000000001101111100000"
                    "111111101010101101010"
                    "100000100111111100110"
                    "101110101100100101111"
                    "101110101000100000010"
                    "101110101101010111000"
                    "100000101100000111011"
                    "111111101011010110010"
                },
        /* 95*/ { BARCODE_QRCODE, UNICODE_MODE, 30, 1, -1, -1, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 30 Example 1; BWIPP uses mask 000 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "000010000001010001111"
                    "000111110001011101001"
                    "001010010010111110111"
                    "100110110111001101101"
                    "000000001000011010100"
                    "111111101101101110110"
                    "100000100110001011111"
                    "101110100010111011100"
                    "101110101110011111101"
                    "101110100001011000101"
                    "100000101001100010011"
                    "111111101110010101110"
                },
        /* 96*/ { BARCODE_QRCODE, UNICODE_MODE, 30, 1, -1, 1 << 8, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 30 Example 1 **NOT SAME** explicit mask 000",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "000010000001010001111"
                    "010101100011001111011"
                    "000011011011110111110"
                    "100110110111001101101"
                    "000000001010001000110"
                    "111111101100100111111"
                    "100000101110001011111"
                    "101110101000101001110"
                    "101110100111010110100"
                    "101110101001011000101"
                    "100000101011110000001"
                    "111111101111011100111"
                },
        /* 97*/ { BARCODE_QRCODE, UNICODE_MODE, 30, 1, -1, -1, { 0, 0, "" }, "서울", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 30 Example 2",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "110000001101010101110"
                    "011011110001011101011"
                    "100111001111110110110"
                    "000000101101011100101"
                    "000000001100001101110"
                    "111111101000100010011"
                    "100000101110001100111"
                    "101110101100101011110"
                    "101110100001010011100"
                    "101110101001011101001"
                    "100000101001110110101"
                    "111111101011011101011"
                },
        /* 98*/ { BARCODE_QRCODE, UNICODE_MODE, 31, 1, -1, -1, { 0, 0, "" }, "条码", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 31 Example 1",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "100011011101010100110"
                    "001000101111011100111"
                    "111101011011110110110"
                    "011100111111011100101"
                    "000000001100001100110"
                    "111111101010100011011"
                    "100000101000001100001"
                    "101110101010101011110"
                    "101110100011010010100"
                    "101110101001011000101"
                    "100000101011110111001"
                    "111111101101011100011"
                },
        /* 99*/ { BARCODE_QRCODE, UNICODE_MODE, 31, 1, -1, -1, { 0, 0, "" }, "北京", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 31 Example 2; BWIPP uses mask 011 instead of 010",
                    "111111100100101111111"
                    "100000101001001000001"
                    "101110100100001011101"
                    "101110101001001011101"
                    "101110100011101011101"
                    "100000101110101000001"
                    "111111101010101111111"
                    "000000000011100000000"
                    "111110111100110101010"
                    "001001000110100100000"
                    "101011111101010011010"
                    "100011000110000110100"
                    "011011100001010011100"
                    "000000001001111100000"
                    "111111101100101101010"
                    "100000100001111100111"
                    "101110101110100101111"
                    "101110101000100000010"
                    "101110101001010111000"
                    "100000101000000111011"
                    "111111101011010110010"
                },
        /*100*/ { BARCODE_QRCODE, UNICODE_MODE, 31, 1, -1, 4 << 8, { 0, 0, "" }, "北京", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 31 Example 2 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101110001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000110000000000"
                    "111100101010010011101"
                    "001001000110100100000"
                    "000110110000111110111"
                    "010101010000110000010"
                    "011011100001010011100"
                    "000000001100010001101"
                    "111111100010011011100"
                    "100000100001111100111"
                    "101110100011001000010"
                    "101110101110010110100"
                    "101110101001010111000"
                    "100000101101101010110"
                    "111111101101100000100"
                },
        /*101*/ { BARCODE_QRCODE, UNICODE_MODE, 31, 1, -1, -1, { 0, 0, "" }, "條碼", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 31 Example 3",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "000100011111010101110"
                    "000100110101011101111"
                    "011101011001110110010"
                    "110010111001011101001"
                    "000000001000001100010"
                    "111111101010100010011"
                    "100000101010001100101"
                    "101110101110101010110"
                    "101110100001010001100"
                    "101110101101011011001"
                    "100000101001110110001"
                    "111111101111011110111"
                },
        /*102*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, -1, { 0, 0, "" }, "条码", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 32 Example 1",
                    "111111100100101111111"
                    "100000101001001000001"
                    "101110100100001011101"
                    "101110101001001011101"
                    "101110100011101011101"
                    "100000101110101000001"
                    "111111101010101111111"
                    "000000000011100000000"
                    "111110111100110101010"
                    "100111001100100101000"
                    "110011110101010010110"
                    "101111010110000111000"
                    "100010101001010010100"
                    "000000001101111101000"
                    "111111101000101101010"
                    "100000100111111101100"
                    "101110101010100101100"
                    "101110101110100011001"
                    "101110101011010110100"
                    "100000101100000110111"
                    "111111101111010010010"
                },
        /*103*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, -1, { 0, 0, "" }, "北京", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 32 Example 2; BWIPP uses mask 011 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "001101010111010101110"
                    "000010110101001111001"
                    "111000000010111110011"
                    "100101110111011101101"
                    "000000001010011111100"
                    "111111101111101010010"
                    "100000100110001101010"
                    "101110100100111001111"
                    "101110101100011000110"
                    "101110100011011001001"
                    "100000101101100100111"
                    "111111101000010001010"
                },
        /*104*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, 4 << 8, { 0, 0, "" }, "北京", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 32 Example 2 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101110001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000110000000000"
                    "111100101010010011101"
                    "111100000110100100000"
                    "110011100100111110111"
                    "110110001100110000010"
                    "101011111001010011100"
                    "000000001100010001101"
                    "111111100110011011100"
                    "100000100111111100100"
                    "101110100101001000001"
                    "101110101010010110111"
                    "101110101101010111000"
                    "100000101011101010110"
                    "111111101001100000100"
                },
        /*105*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, -1, { 0, 0, "" }, "條碼", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 32 Example 3; BWIPP uses mask 011 instead of 001",
                    "111111101111101111111"
                    "100000101101101000001"
                    "101110100111001011101"
                    "101110100101101011101"
                    "101110101000101011101"
                    "100000101010001000001"
                    "111111101010101111111"
                    "000000001111000000000"
                    "111001101111111110011"
                    "100100001010000000100"
                    "100100110100001000101"
                    "101011010000100011000"
                    "010111110100001000011"
                    "000000001101011001000"
                    "111111100011110111001"
                    "100000101001011001100"
                    "101110100101111111111"
                    "101110100000000100101"
                    "101110101100001110011"
                    "100000101010100011011"
                    "111111101110001011101"
                },
        /*106*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, 4 << 8, { 0, 0, "" }, "條碼", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 32 Example 3 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101110001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000110000000000"
                    "111100101010010011101"
                    "000000001110100100000"
                    "010010100010111110011"
                    "111001000010110001010"
                    "001100101111010011000"
                    "000000001100010000001"
                    "111111100110011010100"
                    "100000100101111101000"
                    "101110100011001001001"
                    "101110101010010110111"
                    "101110101111010101000"
                    "100000101011101010010"
                    "111111101011100110000"
                },
        /*107*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, -1, { 0, 0, "" }, "པེ་ཅིང།", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 32 Example 4",
                    "1111111011110110001111111"
                    "1000001000011010101000001"
                    "1011101010110101101011101"
                    "1011101011000001101011101"
                    "1011101011010001101011101"
                    "1000001001010001101000001"
                    "1111111010101010101111111"
                    "0000000001001001100000000"
                    "1111001010010001110011101"
                    "0101100111011100000101011"
                    "1111001000011100111101110"
                    "0011010100010111100100100"
                    "1000111001000101101001100"
                    "0111000001110000000111110"
                    "0100101011000000100011011"
                    "1001000110001111010111011"
                    "0000111000000011111110010"
                    "0000000011110001100010100"
                    "1111111001100001101010111"
                    "1000001001100011100010110"
                    "1011101001001100111110100"
                    "1011101011110010101011100"
                    "1011101011100110011010010"
                    "1000001010111111111010111"
                    "1111111010011101100000011"
                },
        /*108*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, -1, { 0, 0, "" }, "バーコード", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 32 Example 5; BWIPP uses mask 011 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "110111001101011011111"
                    "101101100111010000010"
                    "000110011000110011011"
                    "111110100101011001101"
                    "000000001011010010100"
                    "111111101101101110110"
                    "100000100011010111010"
                    "101110100000111010111"
                    "101110101011010100110"
                    "101110100010000010001"
                    "100000101001010101111"
                    "111111101110010110110"
                },
        /*109*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, 4 << 8, { 0, 0, "" }, "バーコード", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 32 Example 5 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101110001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000110000000000"
                    "111100101010010011101"
                    "000110011100101010001"
                    "011100110110100001100"
                    "001000010110111101010"
                    "110000101011010111100"
                    "000000001101011100101"
                    "111111100100011111000"
                    "100000100010100110100"
                    "101110100001001011001"
                    "101110101101011010111"
                    "101110101100001100000"
                    "100000101111011011110"
                    "111111101111100111000"
                },
        /*110*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, -1, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 32 Example 6; BWIPP uses mask 011 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101110101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001001000000000"
                    "110100110011101110110"
                    "100110010010101101110"
                    "100011100010110100100"
                    "111110001111101010011"
                    "101000110100111011001"
                    "000000001111100001000"
                    "111111101110111010010"
                    "100000100010111101010"
                    "101110100000001010111"
                    "101110101001010001110"
                    "101110100100001011001"
                    "100000101110100001011"
                    "111111101101101010110"
                },
        /*111*/ { BARCODE_QRCODE, UNICODE_MODE, 32, 1, -1, 4 << 8, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 32 Example 6 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101111001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000111000000000"
                    "111100101010010011101"
                    "010111000011011100000"
                    "010010110011000101010"
                    "110000000001100100010"
                    "100110111010110101000"
                    "000000001001101111001"
                    "111111100111001011100"
                    "100000100011001100100"
                    "101110100001111011001"
                    "101110101111011111111"
                    "101110101010000101000"
                    "100000101000101111010"
                    "111111101100011011000"
                },
        /*112*/ { BARCODE_QRCODE, UNICODE_MODE, 33, 1, -1, -1, { 0, 0, "" }, "条码", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 33 Example 1; BWIPP uses mask 111 instead of 010",
                    "111111100100101111111"
                    "100000101001001000001"
                    "101110100100001011101"
                    "101110101001001011101"
                    "101110100011101011101"
                    "100000101110101000001"
                    "111111101010101111111"
                    "000000000011100000000"
                    "111110111100110101010"
                    "110010000100100101100"
                    "000001111101010010010"
                    "110101000100000110100"
                    "001111101111010011100"
                    "000000001011111101100"
                    "111111101000101100010"
                    "100000100001111101110"
                    "101110101110100101000"
                    "101110101010100100101"
                    "101110101111010110100"
                    "100000101010000010111"
                    "111111101101010001110"
                },
        /*113*/ { BARCODE_QRCODE, UNICODE_MODE, 33, 1, -1, 8 << 8, { 0, 0, "" }, "条码", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 33 Example 1 **NOT SAME** explicit mask 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "000011010101010100010"
                    "011101100001001110001"
                    "001101011100111110011"
                    "000001100001011101101"
                    "000000001000011110000"
                    "111111101111101011010"
                    "100000100000001100000"
                    "101110100010111001011"
                    "101110101010011100010"
                    "101110100001011000101"
                    "100000101001100001011"
                    "111111101010010110110"
                },
        /*114*/ { BARCODE_QRCODE, UNICODE_MODE, 33, 1, -1, -1, { 0, 0, "" }, "バーコード", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 33 Example 2",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "011110010011001100111"
                    "111100101001010011100"
                    "010000011111101011010"
                    "010000111111001010101"
                    "000000001101010011010"
                    "111111101100110011011"
                    "100000101101010011000"
                    "101110101110101000101"
                    "101110100101001100111"
                    "101110101100101011001"
                    "100000101100111010101"
                    "111111101110111010111"
                },
        /*115*/ { BARCODE_QRCODE, UNICODE_MODE, 33, 1, -1, -1, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 33 Example 3; BWIPP uses mask 000 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "111111011101010101011"
                    "010100100001011101101"
                    "101101001110111011011"
                    "001101111001001000101"
                    "000000001110011111100"
                    "111111101001101100010"
                    "100000100110001000100"
                    "101110100010111110111"
                    "101110101100011011010"
                    "101110100101011101101"
                    "100000101111100110011"
                    "111111101000010101110"
                },
        /*116*/ { BARCODE_QRCODE, UNICODE_MODE, 33, 1, -1, 1 << 8, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 33 Example 3 **NOT SAME** explicit mask 000",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100101101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001010000000000"
                    "111011111010111000100"
                    "111111011101010101011"
                    "000110110011001111111"
                    "100100000111110010010"
                    "001101111001001000101"
                    "000000001100001101110"
                    "111111101000100101011"
                    "100000101110001000100"
                    "101110101000101100101"
                    "101110100101010010011"
                    "101110101101011101101"
                    "100000101101110100001"
                    "111111101001011100111"
                },
        /*117*/ { BARCODE_QRCODE, UNICODE_MODE, 34, 1, -1, -1, { 0, 0, "" }, "条码", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 34 Example 1",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "001000011111000101010"
                    "000111110001000000110"
                    "010001000010110100011"
                    "000000111011001010101"
                    "000000001110000111000"
                    "111111101101100011110"
                    "100000100000010101011"
                    "101110100000101000111"
                    "101110101110011001010"
                    "101110100011001010001"
                    "100000101011100100011"
                    "111111101110000110010"
                },
        /*118*/ { BARCODE_QRCODE, UNICODE_MODE, 34, 1, -1, -1, { 0, 0, "" }, "バーコード", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 34 Example 2",
                    "1111111000100010001111111"
                    "1000001001000100101000001"
                    "1011101010010011001011101"
                    "1011101001001100001011101"
                    "1011101001001010101011101"
                    "1000001000111101101000001"
                    "1111111010101010101111111"
                    "0000000010010100000000000"
                    "1110111110001010111000100"
                    "1101000011011101010011010"
                    "1111111100111110101100101"
                    "1001010011101111010011010"
                    "0011101010110010101010101"
                    "0110100100110001010101010"
                    "1011101001100000101101011"
                    "0100110111010001010101000"
                    "1011111001101010111110101"
                    "0000000010111110100010110"
                    "1111111010111011101011111"
                    "1000001011001101100011011"
                    "1011101010010010111110101"
                    "1011101001110000110101011"
                    "1011101011000100001010101"
                    "1000001010110001010101001"
                    "1111111010001000110010111"
                },
        /*119*/ { BARCODE_QRCODE, UNICODE_MODE, 34, 1, -1, -1, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 34 Example 3",
                    "111111100010101111111"
                    "100000100000101000001"
                    "101110101010001011101"
                    "101110100000101011101"
                    "101110100100101011101"
                    "100000100111001000001"
                    "111111101010101111111"
                    "000000001011000000000"
                    "111011111010111000100"
                    "001101000001001101010"
                    "101010100101001010110"
                    "010101011000001101010"
                    "100001100001010010101"
                    "000000001001000101010"
                    "111111101001111010111"
                    "100000101010100101011"
                    "101110101110001010101"
                    "101110100001010101111"
                    "101110101000101111001"
                    "100000101101010000101"
                    "111111101010101010111"
                },
        /*120*/ { BARCODE_QRCODE, UNICODE_MODE, 35, 1, -1, -1, { 0, 0, "" }, "条码", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 35 Example 1",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101111101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "110100110011101110110"
                    "000011001001010100010"
                    "000000100001011010010"
                    "111010000100111010011"
                    "111110100011001111101"
                    "000000001000000010000"
                    "111111101011100011010"
                    "100000100010010100001"
                    "101110100010111001011"
                    "101110101000011100010"
                    "101110100011001010101"
                    "100000101011100111011"
                    "111111101110000011110"
                },
        /*121*/ { BARCODE_QRCODE, UNICODE_MODE, 35, 1, -1, -1, { 0, 0, "" }, "バーコード", -1, 0, 25, 25, 1, "AIM ITS/04-023:2022 ECI 35 Example 2",
                    "1111111000100110101111111"
                    "1000001001000111001000001"
                    "1011101010010110101011101"
                    "1011101001001101001011101"
                    "1011101001001000101011101"
                    "1000001000111001001000001"
                    "1111111010101010101111111"
                    "0000000010010111000000000"
                    "1110111110001010111000100"
                    "0111100101011101010101010"
                    "1111111010111010000011001"
                    "1100110101101100001101000"
                    "1111001010110010101010110"
                    "0101100100110101101101010"
                    "1001011110000011001010111"
                    "0101010100010100101101000"
                    "1010001101101011111110101"
                    "0000000011111101100011010"
                    "1111111011011010101010111"
                    "1000001011001101100011001"
                    "1011101011010010111110101"
                    "1011101001010001010100111"
                    "1011101011100100101011001"
                    "1000001010110001010010101"
                    "1111111011001000101010111"
                },
        /*122*/ { BARCODE_QRCODE, UNICODE_MODE, 35, 1, -1, -1, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 0, "AIM ITS/04-023:2022 ECI 35 Example 3; BWIPP uses mask 011 instead of 111",
                    "111111100010101111111"
                    "100000101010101000001"
                    "101110101011001011101"
                    "101110100000101011101"
                    "101110101110101011101"
                    "100000101110001000001"
                    "111111101010101111111"
                    "000000001001000000000"
                    "110100110011101110110"
                    "011011011001010011010"
                    "100001110010111111100"
                    "001111010100011101011"
                    "111011100110101100101"
                    "000000001101000111100"
                    "111111101111100110010"
                    "100000100001010000101"
                    "101110100110111100111"
                    "101110101100000100010"
                    "101110100111111010101"
                    "100000101000111111011"
                    "111111101101000011110"
                },
        /*123*/ { BARCODE_QRCODE, UNICODE_MODE, 35, 1, -1, 4 << 8, { 0, 0, "" }, "바코드", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 35 Example 3 **NOT SAME** explicit mask 011",
                    "111111101100101111111"
                    "100000100100101000001"
                    "101110101010101011101"
                    "101110101001001011101"
                    "101110101111001011101"
                    "100000100000001000001"
                    "111111101010101111111"
                    "000000000111000000000"
                    "111100101010010011101"
                    "101010001000100010100"
                    "010000100011001110010"
                    "000001011010010011010"
                    "110101101000100010100"
                    "000000001011001001101"
                    "111111100110010111100"
                    "100000100000100001011"
                    "101110100111001101001"
                    "101110101010001010011"
                    "101110101001110100100"
                    "100000101110110001010"
                    "111111101100110010000"
                },
        /*124*/ { BARCODE_QRCODE, UNICODE_MODE, 170, 1, -1, 5 << 8, { 0, 0, "" }, "sn:7QPB4MN", -1, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 170 Example 1 **NOT SAME** different encodation; explicit mask 100 (auto 011) to match BWIPP",
                    "111111101000101111111"
                    "100000101101001000001"
                    "101110101111101011101"
                    "101110101010101011101"
                    "101110100111101011101"
                    "100000101010101000001"
                    "111111101010101111111"
                    "000000000000000000000"
                    "110011100000100101111"
                    "111011000010110110111"
                    "000100110100110111010"
                    "000011000001110111001"
                    "100111111111001110010"
                    "000000001010101111110"
                    "111111100001001010101"
                    "100000101001010111000"
                    "101110101100000101011"
                    "101110100110000001111"
                    "101110100111100111101"
                    "100000101110111100111"
                    "111111101001001000001"
                },
        /*125*/ { BARCODE_QRCODE, DATA_MODE, 899, 1, -1, -1, { 0, 0, "" }, "\000\001\002\133\134\135\375\376\377", 9, 0, 21, 21, 1, "AIM ITS/04-023:2022 ECI 899 Example 1",
                    "111111101111101111111"
                    "100000101101101000001"
                    "101110100111001011101"
                    "101110100101101011101"
                    "101110101000101011101"
                    "100000101010001000001"
                    "111111101010101111111"
                    "000000001111000000000"
                    "111001101111111110011"
                    "101010000100010000000"
                    "110000111010001001101"
                    "001100010100111100111"
                    "010100101010001010111"
                    "000000001000011110000"
                    "111111100011100101110"
                    "100000101000011100011"
                    "101110100011101011111"
                    "101110100101111010000"
                    "101110101100000111110"
                    "100000101111111000011"
                    "111111101000010110101"
                },
        /*126*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, -1, { 0, 0, "" }, "ABCDEFGHI", 9, 0, 21, 21, 0, "Automatic mask 011; BWIPP uses mask 111",
                    "111111100000001111111"
                    "100000100111101000001"
                    "101110100000001011101"
                    "101110100110001011101"
                    "101110101100101011101"
                    "100000100011001000001"
                    "111111101010101111111"
                    "000000001101100000000"
                    "001100111101111010000"
                    "100111011101100101110"
                    "010011111000101001010"
                    "110100001001011011010"
                    "011101110001100101111"
                    "000000001011110001011"
                    "111111101100111110111"
                    "100000100100101110000"
                    "101110100111101011111"
                    "101110101111110010010"
                    "101110101000100101100"
                    "100000100001111110000"
                    "111111100101000111010"
                },
        /*127*/ { BARCODE_QRCODE, UNICODE_MODE | FAST_MODE, -1, -1, -1, -1, { 0, 0, "" }, "ABCDEFGHI", 9, 0, 21, 21, 0, "Fast automatic mask 010; BWIPP uses mask 111",
                    "111111101000001111111"
                    "100000101010001000001"
                    "101110101110101011101"
                    "101110100110001011101"
                    "101110100001001011101"
                    "100000101101101000001"
                    "111111101010101111111"
                    "000000001000000000000"
                    "001110101011011100111"
                    "100111011101100101110"
                    "111110110101000100111"
                    "000010011111101101100"
                    "011101110001100101111"
                    "000000001110011100110"
                    "111111100010001000001"
                    "100000100100101110000"
                    "101110101010000110010"
                    "101110101001000100100"
                    "101110101000100101100"
                    "100000100100010011101"
                    "111111100011110001100"
                },
        /*128*/ { BARCODE_QRCODE, UNICODE_MODE, -1, 2, -1, 1 << 8, { 0, 0, "" }, "202404110011看看16", -1, ZINT_WARN_NONCOMPLIANT, 21, 21, 1, "Shift-JIS range (BWIPP via bwip-js issue #335)",
                    "111111100000101111111"
                    "100000101111101000001"
                    "101110100111001011101"
                    "101110100100001011101"
                    "101110101110101011101"
                    "100000100100101000001"
                    "111111101010101111111"
                    "000000000111100000000"
                    "101010100001000010010"
                    "110111010011011110011"
                    "000000111000101100000"
                    "010100000001011100010"
                    "111100101000100001110"
                    "000000001101000101010"
                    "111111100110010010101"
                    "100000100001000100001"
                    "101110101010001010001"
                    "101110100011000110110"
                    "101110101100111100001"
                    "100000100011010111000"
                    "111111101110111010101"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_qr_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci,
                                    data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/,
                                    data[i].data, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, %d, %s, { %d, %d, \"%s\" }, \"%s\", %d, %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                    data[i].eci, data[i].option_1, data[i].option_2, testUtilOption3Name(data[i].symbology, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length, testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[32768];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_encode_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_QRCODE, UNICODE_MODE, 4, -1, 8 << 8, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 21, 21, 1, "Standard example",
                    "111111101101001111111"
                    "100000101100101000001"
                    "101110100010101011101"
                    "101110101011101011101"
                    "101110101011101011101"
                    "100000101110101000001"
                    "111111101010101111111"
                    "000000000100000000000"
                    "000100100011000111011"
                    "001000000000000001001"
                    "111100111101110000101"
                    "010111011010000001010"
                    "111110101101111011011"
                    "000000001100110110011"
                    "111111100001100101101"
                    "100000100110101111000"
                    "101110100101001110111"
                    "101110101100010100011"
                    "101110100110100011101"
                    "100000100100111101100"
                    "111111100010000110010"
                },
        /*  1*/ { BARCODE_QRCODE, UNICODE_MODE, 4, -1, 8 << 8, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 21, 21, 1, "Standard example auto-ECI",
                    "111111101101001111111"
                    "100000101100101000001"
                    "101110100010101011101"
                    "101110101011101011101"
                    "101110101011101011101"
                    "100000101110101000001"
                    "111111101010101111111"
                    "000000000100000000000"
                    "000100100011000111011"
                    "001000000000000001001"
                    "111100111101110000101"
                    "010111011010000001010"
                    "111110101101111011011"
                    "000000001100110110011"
                    "111111100001100101101"
                    "100000100110101111000"
                    "101110100101001110111"
                    "101110101100010100011"
                    "101110100110100011101"
                    "100000100100111101100"
                    "111111100010000110010"
                },
        /*  2*/ { BARCODE_QRCODE, UNICODE_MODE, 4, -1, 8 << 8, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 21, 21, 1, "Standard example inverted",
                    "111111101111101111111"
                    "100000101011001000001"
                    "101110100000101011101"
                    "101110101110101011101"
                    "101110101101101011101"
                    "100000101010001000001"
                    "111111101010101111111"
                    "000000000011000000000"
                    "000100100000000111011"
                    "001100000111100101100"
                    "110010100000011001011"
                    "011000000001100101011"
                    "101000101011111010001"
                    "000000001001111100000"
                    "111111100101010110010"
                    "100000100110100011001"
                    "101110100111101010101"
                    "101110101000001000011"
                    "101110100001001011001"
                    "100000100100110110011"
                    "111111100001000011110"
                },
        /*  3*/ { BARCODE_QRCODE, UNICODE_MODE, 4, -1, 8 << 8, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 21, 21, 1, "Standard example inverted auto-ECI",
                    "111111101111101111111"
                    "100000101011001000001"
                    "101110100000101011101"
                    "101110101110101011101"
                    "101110101101101011101"
                    "100000101010001000001"
                    "111111101010101111111"
                    "000000000011000000000"
                    "000100100000000111011"
                    "001100000111100101100"
                    "110010100000011001011"
                    "011000000001100101011"
                    "101000101011111010001"
                    "000000001001111100000"
                    "111111100101010110010"
                    "100000100110100011001"
                    "101110100111101010101"
                    "101110101000001000011"
                    "101110100001001011001"
                    "100000100100110110011"
                    "111111100001000011110"
                },
        /*  4*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a -128 GB的存储空间-黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a - 128 GB Speicher - Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 53, 53, 0, "AIM ITS/04-023:2022 Annex A example; BWIPP different encodation",
                    "11111110010010001110011110000111001001011110001111111"
                    "10000010100101111001000000001000010110100111001000001"
                    "10111010010101001110001011100010000010111101001011101"
                    "10111010111101111000000010111011000100010010101011101"
                    "10111010001100111110110111111110011010010110001011101"
                    "10000010111000011101010110001111000011110110001000001"
                    "11111110101010101010101010101010101010101010101111111"
                    "00000000011101001000110110001101000000001111000000000"
                    "11111011100100000100000011111000011010101000010101010"
                    "11100000100111010011110110011110110010011011001001101"
                    "00110110000101000010110100111001100110000111110110101"
                    "10100100101111110111100110011000111111000010010011011"
                    "00001110111010001100110011100001100110011001110111000"
                    "00100100001010101011010001000110110110111010011011000"
                    "00111011011011100011010010001010100111110101001110110"
                    "10111001010100100110000111101101100011010010100101001"
                    "01000010001111110011000000001101011011010000011110001"
                    "00100100001100001110010100101100010000011010000001011"
                    "10001110001010011011010111100011001011110010100110110"
                    "01100001001101001110011111010010010001101000001011101"
                    "11110011100011000010011011100011110100010001100101000"
                    "00010101000010110010010000100000011000011000001111010"
                    "10100110010110000101011010110000000101100100011011110"
                    "01000000010101000000110101111100000111011010110101011"
                    "01101111111100000100010111111010011101110000111111001"
                    "10001000101101010011110010001100111001010100100011110"
                    "10011010111011100010110110101000001010000111101011010"
                    "10011000100111010001101010001111011100001111100011101"
                    "10111111111101111010000011111001101010111110111110100"
                    "01001100101111011100010101001100111100101001111101000"
                    "00001111111111100011101010101011010101110100010000101"
                    "10101000110010100110011011101011010011001011010110000"
                    "10100110010001110011001111010101001010100001101011111"
                    "10010101111110001110010011100000111001110011000011000"
                    "01111010110001011001011000001011001011100011110110111"
                    "01110001010100101000001010111110011001001000111011101"
                    "10011110000101101010101111100001110010111000000101010"
                    "01010001100011010110110100100100100010001011100101000"
                    "11011011001100001101010010111110010011000110010001100"
                    "01000001001101000000000011101101100111010010010111000"
                    "10100111010100000100010110011000001111110100000000001"
                    "10100100110001010011100111110000110000011011010010110"
                    "11011111010110100110110001001001010110110111011111000"
                    "01100000111011110111101110110011010110001100001101101"
                    "00010010010100111000010011111011110000011111111111010"
                    "00000000101011011000011010001100101010001011100010010"
                    "11111110110011000011001110101000111011000001101011100"
                    "10000010001011100110011110001011000101010011100010111"
                    "10111010100100010011001011111110101110111100111110100"
                    "10111010101000001110010011011100110000001010000101010"
                    "10111010100110111101011111100111011111010000111111010"
                    "10000010110010101000001000110100010100101010011010001"
                    "11111110100110011010100111101001110010111100100111000"
                },
        /*  5*/ { BARCODE_QRCODE, DATA_MODE, 2, -1, 3 << 8, { 0, 0, "" }, { { TU("\266"), 1, 0 }, { TU("\266"), 1, 7 }, { TU("\266"), 1, 0 } }, 0, 21, 21, 1, "Standard example + extra seg, data mode",
                    "111111100111001111111"
                    "100000100010001000001"
                    "101110101010001011101"
                    "101110101101101011101"
                    "101110101000101011101"
                    "100000101101001000001"
                    "111111101010101111111"
                    "000000001100000000000"
                    "101111100001001111100"
                    "011001000111100100111"
                    "100010111100110100110"
                    "001100001101101101101"
                    "100110100000111001010"
                    "000000001010110011111"
                    "111111100101001100101"
                    "100000101000000100110"
                    "101110101101000100100"
                    "101110101001111010100"
                    "101110101110101001100"
                    "100000100101111110000"
                    "111111101110100001010"
                },
        /*  6*/ { BARCODE_QRCODE, UNICODE_MODE, 2, -1, 3 << 8, { 0, 0, "" }, { { TU("éé"), -1, 0 }, { TU("กขฯ"), -1, 0 }, { TU("βββ"), -1, 0 } }, ZINT_WARN_USES_ECI, 21, 21, 1, "Auto-ECI",
                    "111111100001001111111"
                    "100000100111001000001"
                    "101110101011101011101"
                    "101110101000101011101"
                    "101110101100101011101"
                    "100000101100101000001"
                    "111111101010101111111"
                    "000000001100000000000"
                    "101111100110101111100"
                    "111100011010011100001"
                    "011000100111011011111"
                    "101011010000010111110"
                    "100100101010111101001"
                    "000000001000000110101"
                    "111111100011000110111"
                    "100000101100001001001"
                    "101110101010110101100"
                    "101110101000010111100"
                    "101110101011000110100"
                    "100000100101000010100"
                    "111111101100100100110"
                },
        /*  7*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, { 2, 3, "123" }, { { TU("éé"), -1, 23 }, { TU("กขฯ"), -1, 13 }, { TU("βββ"), -1, 9 } }, 0, 25, 25, 1, "Structured Append",
                    "1111111010110011001111111"
                    "1000001010010000001000001"
                    "1011101011111011101011101"
                    "1011101010001101001011101"
                    "1011101010110111001011101"
                    "1000001001010010001000001"
                    "1111111010101010101111111"
                    "0000000011110000000000000"
                    "0110101101111000101011111"
                    "0001100000010100111110001"
                    "1101111111001011001100111"
                    "0010000101000110000101000"
                    "0000011011100010100010101"
                    "0011010001011111011101001"
                    "1000101100001000101010011"
                    "0100100111100101111000101"
                    "1000011110011100111110000"
                    "0000000010110000100011101"
                    "1111111011001110101011111"
                    "1000001001010111100010111"
                    "1011101010111111111110001"
                    "1011101001011111001001100"
                    "1011101011110010101101101"
                    "1000001010100000101100001"
                    "1111111000101101110011001"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_qr_encode_segs", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/,
                            data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/,
                            NULL, 0, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            char escaped1[4096];
            char escaped2[4096];
            int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
            int length1 = data[i].segs[1].length == -1 ? (int) ustrlen(data[i].segs[1].source) : data[i].segs[1].length;
            int length2 = data[i].segs[2].length == -1 ? (int) ustrlen(data[i].segs[2].source) : data[i].segs[2].length;
            printf("        /*%3d*/ { %s, %s, %d, %d, %s, { %d, %d, \"%s\" }, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                    data[i].option_1, data[i].option_2, testUtilOption3Name(data[i].symbology, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwippSegs(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].segs, seg_count, NULL, cmp_buf, sizeof(cmp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwippSegs ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp) {
                    if ((symbol->input_mode & 0x07) == DATA_MODE) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) {
                            printf("i:%d multiple segments in DATA_MODE not currently supported for ZXing-C++ testing (%s)\n",
                                    i, testUtilBarcodeName(symbol->symbology));
                        }
                    } else {
                        int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
                        if (testUtilCanZXingCPP(i, symbol, (const char *) data[i].segs[0].source, length, debug)) {
                            int cmp_len, ret_len;
                            char modules_dump[32768];
                            assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                            ret = testUtilZXingCPP(i, symbol, (const char *) data[i].segs[0].source, length,
                                    modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                            assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                            ret = testUtilZXingCPPCmpSegs(symbol, cmp_msg, cmp_buf, cmp_len, data[i].segs, seg_count,
                                    NULL /*primary*/, escaped, &ret_len);
                            assert_zero(ret, "i:%d %s testUtilZXingCPPCmpSegs %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                           i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                        }
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITER_MILLES   5
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)
#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)

/* Not a real test, just performance indicator */
static void test_qr_perf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

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
    static const struct item data[] = {
        /*  0*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, "12345678901234", 0, 21, 21, "14 chars, Numeric mode" },
        /*  1*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, "ABC 123 ABC 123 ABCD", 0, 21, 21, "20 chars, Alphanumeric" },
        /*  2*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, "ABCde fG H 123456 IJKlmn, 1234567890 opQ Rst uvwxyz. 1234", 0, 29, 29, "57 chars, Alphanumeric" },
        /*  3*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    ZINT_WARN_NONCOMPLIANT, 37, 37, "107 chars, Mixed modes" },
        /*  4*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    ZINT_WARN_NONCOMPLIANT, 53, 53, "214 chars, Mixed modes" },
        /*  5*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    ZINT_WARN_NONCOMPLIANT, 73, 73, "428 chars, Mixed modes" },
        /*  6*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    ZINT_WARN_NONCOMPLIANT, 105, 105, "963 chars, Mixed modes" },
        /*  7*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    ZINT_WARN_NONCOMPLIANT, 133, 133, "1498 chars, Mixed modes" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start;
    clock_t total_create = 0, total_encode = 0, total_buffer = 0;
    clock_t diff_create, diff_encode, diff_buffer;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    printf("FAST_MODE\n");
    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            int input_mode = data[i].input_mode == -1 ? FAST_MODE : (data[i].input_mode | FAST_MODE);
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

            start = clock();
            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            #if 0
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            #endif

            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

            ZBarcode_Delete(symbol);
        }

        printf("%*s: encode % 8gms, buffer % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
    }
    if (p_ctx->index != -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_create));
    }

    printf("OPTIMIZED\n");
    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
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

        printf("%*s: encode % 8gms, buffer % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
    }
    if (p_ctx->index != -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_create));
    }
}

static void test_microqr_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_1;
        int option_2;
        char *data;
        int ret_encode;
        int ret_vector;
        int expected_size;
        char *expected_errtxt;
        int compare_previous;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, "12345", 0, 0, 11, "", -1 }, /* ECC auto-set to 1 (L), version auto-set to 1 */
        /*  1*/ { 1, -1, "12345", 0, 0, 11, "", 0 }, /* ECC 1 (L), version auto-set to 1 */
        /*  2*/ { 2, -1, "12345", 0, 0, 13, "", 1 }, /* ECC 2 (M), version auto-set to 2 */
        /*  3*/ { 3, -1, "12345", 0, 0, 17, "", 1 }, /* ECC 3 (Q), version auto-set to 3 */
        /*  4*/ { 4, -1, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, "Error 566: Error correction level H not available", -1 },
        /*  5*/ { -1, 1, "12345", 0, 0, 11, "", -1 }, /* ECC auto-set to 1, version 1 */
        /*  6*/ { -1, 2, "12345", 0, 0, 13, "", 1 }, /* ECC auto-set to 2, version 2 */
        /*  7*/ { 2, 2, "12345", 0, 0, 13, "", 0 },
        /*  8*/ { -1, 3, "12345", 0, 0, 15, "", 1 }, /* ECC auto-set to 2, version 3 */
        /*  9*/ { 2, 3, "12345", 0, 0, 15, "", 0 },
        /* 10*/ { 1, 3, "12345", 0, 0, 15, "", 1 },
        /* 11*/ { -1, 4, "12345", 0, 0, 17, "", 1 }, /* ECC auto-set to 3, version 4 */
        /* 12*/ { 3, 4, "12345", 0, 0, 17, "", 0 }, /* ECC auto-set to 3, version 4 */
        /* 13*/ { 2, 4, "12345", 0, 0, 17, "", 1 },
        /* 14*/ { -1, 5, "12345", 0, 0, 11, "", -1 }, /* Size > 4 ignored */
        /* 15*/ { 1, 5, "12345", 0, 0, 11, "", 0 }, /* Ignored also if ECC given */
        /* 16*/ { 1, 1, "12345", 0, 0, 11, "", 0 }, /* ECC 1, version 1 */
        /* 17*/ { 1, 2, "12345", 0, 0, 13, "", 1 }, /* ECC 1, version 2 */
        /* 18*/ { 1, 3, "12345", 0, 0, 15, "", 1 }, /* ECC 1, version 3 */
        /* 19*/ { 1, 4, "12345", 0, 0, 17, "", 1 }, /* ECC 1, version 4 */
        /* 20*/ { 2, 1, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, "Error 574: Version M1 supports error correction level L only", -1 },
        /* 21*/ { 2, 2, "12345", 0, -1, 13, "", -1 }, /* ECC 2, version 2 */
        /* 22*/ { 2, 3, "12345", 0, -1, 15, "", 1 }, /* ECC 2, version 3 */
        /* 23*/ { 2, 4, "12345", 0, -1, 17, "", 1 }, /* ECC 2, version 4 */
        /* 24*/ { 3, 1, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, "Error 574: Version M1 supports error correction level L only", -1 },
        /* 25*/ { 3, 2, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, "Error 563: Error correction level Q requires Version M4", -1 },
        /* 26*/ { 3, 3, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, "Error 563: Error correction level Q requires Version M4", -1 },
        /* 27*/ { 3, 4, "12345", 0, -1, 17, "", -1 }, /* ECC 3, version 4 */
        /* 28*/ { 4, 4, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, "Error 566: Error correction level H not available", -1 },
        /* 29*/ { 5, -1, "12345", 0, 0, 11, "", -1 }, /* ECC > 4 ignored */
        /* 30*/ { 5, 1, "12345", 0, 0, 11, "", 0 }, /* Ignored also if size given */
        /* 31*/ { 1, 1, "123456", ZINT_ERROR_TOO_LONG, -1, 0, "Error 570: Input too long for Version M1-L, requires 4 codewords (maximum 3)", -1 },
        /* 32*/ { 1, -1, "123456", 0, 0, 13, "", -1 }, /* ECC 1 (L), version auto-set to 2 */
        /* 33*/ { 1, 2, "123456", 0, 0, 13, "", 0 },
        /* 34*/ { 2, 2, "ABCDEF", ZINT_ERROR_TOO_LONG, -1, 0, "Error 570: Input too long for Version M2-M, requires 5 codewords (maximum 4)", -1 },
        /* 35*/ { 1, -1, "ABCDEF", 0, 0, 13, "", -1 }, /* ECC 1 (L), version auto-set to 2 */
        /* 36*/ { 1, 2, "ABCDEF", 0, 0, 13, "", 0 },
        /* 37*/ { 2, -1, "ABCDE", 0, 0, 13, "", -1 }, /* ECC 2 (M), version auto-set to 2 */
        /* 38*/ { 2, 2, "ABCDE", 0, 0, 13, "", 0 }, /* ECC 2 (M), version auto-set to 2 */
        /* 39*/ { 1, -1, "ABCDEABCDEABCD", 0, 0, 15, "", -1 }, /* 14 alphanumerics, ECC 1, version auto-set to 3 */
        /* 40*/ { 1, 3, "ABCDEABCDEABCD", 0, 0, 15, "", 0 },
        /* 41*/ { 2, 3, "ABCDEABCDEABCD", ZINT_ERROR_TOO_LONG, -1, 0, "Error 570: Input too long for Version M3-M, requires 11 codewords (maximum 9)", -1 },
        /* 42*/ { 2, 3, "ABCDEABCDEA", 0, 0, 15, "", -1 }, /* 11 alphanumerics, ECC 2, version 3 */
        /* 43*/ { 1, -1, "ABCDEFGHIJABCDEFGHIJA", 0, 0, 17, "", -1 }, /* 21 alphanumerics, ECC 1, version auto-set to 4 */
        /* 44*/ { 1, 4, "ABCDEFGHIJABCDEFGHIJA", 0, 0, 17, "", 0 },
        /* 45*/ { 2, 4, "ABCDEFGHIJABCDEFGHIJA", ZINT_ERROR_TOO_LONG, -1, 0, "Error 565: Input too long for Version M4-M, requires 16 codewords (maximum 14)", -1 },
        /* 46*/ { 2, 4, "ABCDEFGHIJABCDEFGH", 0, 0, 17, "", -1 }, /* 18 alphanumerics, ECC 2, version 4 */
        /* 47*/ { 3, 4, "ABCDEFGHIJABCDEFGH", ZINT_ERROR_TOO_LONG, -1, 0, "Error 565: Input too long for Version M4-Q, requires 14 codewords (maximum 10)", -1 },
        /* 48*/ { 3, 4, "ABCDEFGHIJABC", 0, 0, 17, "", -1 }, /* 13 alphanumerics, ECC 3 (Q), version 4 */
        /* 49*/ { -1, -1, "123456789012345678901234567890123456", ZINT_ERROR_TOO_LONG, -1, 0, "Error 562: Input length 36 too long (maximum 35)", -1 }, /* 35 absolute max */
        /* 50*/ { -1, -1, "貫貫貫貫貫", 0, 0, 17, "", -1 }, /* 5 Kanji max */
        /* 51*/ { -1, -1, "貫貫貫貫貫貫", ZINT_ERROR_TOO_LONG, -1, 0, "Error 565: Input too long for Version M4-L, requires 19 codewords (maximum 16)", -1 },
        /* 52*/ { -1, 1, "A", ZINT_ERROR_INVALID_DATA, -1, 0, "Error 758: Invalid character at position 1 in input for Version M1 (digits only)", -1 }, /* Numeric only */
        /* 53*/ { -1, 2, "A", 0, 0, 13, "", -1 },
        /* 54*/ { -1, 2, " ", 0, 0, 13, "", -1 },
        /* 55*/ { -1, 2, "$%*+", 0, 0, 13, "", -1 },
        /* 56*/ { -1, 2, "-./:", 0, 0, 13, "", -1 },
        /* 57*/ { -1, 2, "a", ZINT_ERROR_INVALID_DATA, -1, 0, "Error 759: Invalid character in input for Version M2 (digits, A-Z, space and \"$%*+-./:\" only)", -1 }, /* Uppercase only */
        /* 58*/ { -1, 2, "!", ZINT_ERROR_INVALID_DATA, -1, 0, "Error 759: Invalid character in input for Version M2 (digits, A-Z, space and \"$%*+-./:\" only)", -1 }, /* Certain punctuation only */
        /* 59*/ { -1, 2, "#", ZINT_ERROR_INVALID_DATA, -1, 0, "Error 759: Invalid character in input for Version M2 (digits, A-Z, space and \"$%*+-./:\" only)", -1 }, /* Certain punctuation only */
        /* 60*/ { -1, 2, ",", ZINT_ERROR_INVALID_DATA, -1, 0, "Error 759: Invalid character in input for Version M2 (digits, A-Z, space and \"$%*+-./:\" only)", -1 }, /* Certain punctuation only */
        /* 61*/ { -1, 2, "@", ZINT_ERROR_INVALID_DATA, -1, 0, "Error 759: Invalid character in input for Version M2 (digits, A-Z, space and \"$%*+-./:\" only)", -1 }, /* Certain punctuation only */
        /* 62*/ { -1, 2, "\177", ZINT_ERROR_INVALID_DATA, -1, 0, "Error 759: Invalid character in input for Version M2 (digits, A-Z, space and \"$%*+-./:\" only)", -1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    struct zint_symbol previous_symbol;

    testStartSymbol("test_microqr_options", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MICROQR, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);
        if (p_ctx->index == -1 && data[i].compare_previous != -1) {
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

static void test_microqr_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_3;
        char *data;
        int ret;
        char *expected;
        int bwipp_cmp;
        int zxingcpp_cmp;
        char *comment;
    };
    /* é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9 */
    /* β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2 */
    /* ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS, UTF-8 E0B881 */
    /* Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447 (\204\107), UTF-8 D096 */
    /* ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS, UTF-8 E0BA81 */
    /* ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5 */
    /* ･ U+FF65 half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xA5 (\245), UTF-8 EFBDA5 */
    /* ¿ U+00BF in ISO 8859-1 0xBF (\277), not in Shift JIS; 0xBF same codepoint as single-byte half-width katakana ｿ (U+FF7F) in Shift JIS (below), UTF-8 C2BF */
    /* ｿ U+FF7F half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xBF (\277), UTF-8 EFBDBF */
    /* ‾ U+203E overline, not in ISO/Win, in Shift JIS single-byte 0x7E (\176) (tilde), UTF-8 E280BE */
    /* 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9 */
    /* 茗 U+8317 kanji, in Shift JIS 0xE4AA (\344\252), UTF-8 E88C97 */
    /* Á U+00C1, UTF-8 C381; ȁ U+0201, UTF-8 C881; Ȃ U+0202, UTF-8 C882; ¢ U+00A2, UTF-8 C2A2; á U+00E1, UTF-8 C3A1 */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "é", 0, "87 A4 00 EC 11 EC 11 EC 00 (68)", 1, 1, "B1 (ISO 8859-1)" },
        /*  1*/ { DATA_MODE, -1, "é", 0, "8B 0E A4 00 EC 11 EC 11 00 (68)", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /*  2*/ { UNICODE_MODE, -1, "β", 0, "C8 80 00 00 EC 11 EC 11 00 (68)", 1, 1, "K1 (Shift JIS)" },
        /*  3*/ { UNICODE_MODE, -1, "ก", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input", 1, 1, "ก not in Shift JIS" },
        /*  4*/ { UNICODE_MODE, -1, "Ж", 0, "C8 91 C0 00 EC 11 EC 11 00 (68)", 1, 1, "K1 (Shift JIS)" },
        /*  5*/ { UNICODE_MODE, -1, "ກ", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input", 1, 1, "ກ not in Shift JIS" },
        /*  6*/ { UNICODE_MODE, -1, "\\", 0, "85 70 00 EC 11 EC 11 EC 00 (68)", 1, 1, "B1 (ASCII)" },
        /*  7*/ { UNICODE_MODE, -1, "¥", 0, "86 94 00 EC 11 EC 11 EC 00 (68)", 1, 1, "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous)" },
        /*  8*/ { UNICODE_MODE, -1, "･", 0, "86 94 00 EC 11 EC 11 EC 00 (68)", 1, 1, "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /*  9*/ { UNICODE_MODE, -1, "¿", 0, "86 FC 00 EC 11 EC 11 EC 00 (68)", 1, 1, "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous)" },
        /* 10*/ { UNICODE_MODE, -1, "ｿ", 0, "86 FC 00 EC 11 EC 11 EC 00 (68)", 1, 1, "B1 (Shift JIS) (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 11*/ { UNICODE_MODE, -1, "~", 0, "85 F8 00 EC 11 EC 11 EC 00 (68)", 1, 1, "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 12*/ { UNICODE_MODE, -1, "‾", 0, "85 F8 00 EC 11 EC 11 EC 00 (68)", 1, 0, "B1 (Shift JIS) (same bytes as ~ ASCII above, so ambiguous); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 13*/ { UNICODE_MODE, -1, "点", 0, "CB 67 C0 00 EC 11 EC 11 00 (68)", 1, 1, "K1 (Shift JIS)" },
        /* 14*/ { DATA_MODE, -1, "\223\137", 0, "8A 4D 7C 00 EC 11 EC 11 00 (68)", 0, 0, "B2 (Shift JIS); BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 15*/ { DATA_MODE, ZINT_FULL_MULTIBYTE, "\223\137", 0, "CB 67 C0 00 EC 11 EC 11 00 (68)", 1, 1, "K1 (Shift JIS) (full multibyte)" },
        /* 16*/ { DATA_MODE, -1, "点", 0, "8F 9E 0A E4 00 EC 11 EC 00 (68)", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 17*/ { UNICODE_MODE, -1, "茗", 0, "CE AA 80 00 EC 11 EC 11 00 (68)", 1, 1, "K1 (Shift JIS)" },
        /* 18*/ { DATA_MODE, -1, "\344\252", 0, "8B 92 A8 00 EC 11 EC 11 00 (68)", 0, 0, "B2 (Shift JIS); BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below; ZXing-C++ test can't handle DATA_MODE for certain inputs)" },
        /* 19*/ { DATA_MODE, ZINT_FULL_MULTIBYTE, "\344\252", 0, "CE AA 80 00 EC 11 EC 11 00 (68)", 1, 1, "K1 (Shift JIS) (full multibyte)" },
        /* 20*/ { DATA_MODE, -1, "茗", 0, "8F A2 32 5C 00 EC 11 EC 00 (68)", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 21*/ { UNICODE_MODE, -1, "¥点", 0, "8D 72 4D 7C 00 EC 11 EC 00 (68)", 1, 0, "B3 (Shift JIS) (optimized from B1 K1); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 22*/ { DATA_MODE, -1, "\134\223\137", 0, "8D 72 4D 7C 00 EC 11 EC 00 (68)", 1, 0, "B3 (Shift JIS) (optimized from B1 K1); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 23*/ { DATA_MODE, -1, "¥点", 0, "97 0A 97 9E 0A E4 00 EC 00 (68)", 1, 0, "B5 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 24*/ { UNICODE_MODE, -1, "点茗", 0, "D3 67 F5 54 00 EC 11 EC 00 (68)", 1, 1, "K2 (Shift JIS)" },
        /* 25*/ { DATA_MODE, -1, "\223\137\344\252", 0, "92 4D 7F 92 A8 00 EC 11 00 (68)", 0, 0, "B4 (Shift JIS; BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below)); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 26*/ { DATA_MODE, ZINT_FULL_MULTIBYTE, "\223\137\344\252", 0, "D3 67 F5 54 00 EC 11 EC 00 (68)", 1, 1, "K2 (Shift JIS) (full multibyte)" },
        /* 27*/ { DATA_MODE, -1, "点茗", 0, "9B 9E 0A E7 A2 32 5C 00 00 (68)", 1, 0, "B6 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 28*/ { DATA_MODE, ZINT_FULL_MULTIBYTE, "点茗", 0, "9B 9E 0A E7 A2 32 5C 00 00 (68)", 1, 1, "B6 (UTF-8)" },
        /* 29*/ { UNICODE_MODE, -1, "点茗･", 0, "D3 67 F5 55 0D 28 00 EC 00 (68)", 1, 1, "K2 B1 (Shift JIS)" },
        /* 30*/ { DATA_MODE, -1, "\223\137\344\252\245", 0, "96 4D 7F 92 AA 94 00 EC 00 (68)", 0, 0, "B5 (Shift JIS); BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 31*/ { DATA_MODE, ZINT_FULL_MULTIBYTE, "\223\137\344\252\245", 0, "D3 67 F5 55 0D 28 00 EC 00 (68)", 1, 1, "K2 B1 (Shift JIS) (full multibyte)" },
        /* 32*/ { DATA_MODE, -1, "点茗･", 0, "A7 9E 0A E7 A2 32 5F BE F6 94 00 (84)", 1, 0, "B9 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 33*/ { UNICODE_MODE, -1, "¥点茗･", 0, "99 72 4D 7F 92 AA 94 00 00 (68)", 1, 0, "B6 (Shift JIS) (optimized from B1 K2 B1); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 34*/ { DATA_MODE, -1, "\134\223\137\344\252\245", 0, "99 72 4D 7F 92 AA 94 00 00 (68)", 1, 0, "B6 (Shift JIS) (optimized from B1 K2 B1); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 35*/ { DATA_MODE, -1, "¥点茗･", 0, "4B C2 A5 E7 82 B9 E8 8C 97 EF BD A5 00 00 (112)", 1, 0, "B11 (UTF-8); ZXing-C++ doesn't map Shift JIS ASCII" },
        /* 36*/ { DATA_MODE, -1, "ÁȁȁȁȂ¢", 0, "4C C3 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00 (112)", 1, 0, "B12 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 37*/ { DATA_MODE, -1, "ÁȁȁȁȁȂ¢", 0, "4E C3 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00 (128)", 0, 0, "B14 (UTF-8); BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 38*/ { DATA_MODE, ZINT_FULL_MULTIBYTE, "ÁȁȁȁȁȂ¢", 0, "41 C3 6C 08 80 44 02 20 11 00 88 0A 12 0D 10 00 (128)", 1, 1, "B1 K6 B1 (UTF-8) (full multibyte)" },
        /* 39*/ { UNICODE_MODE, -1, "áA", 0, "8B 85 04 00 EC 11 EC 11 00 (68)", 0, 1, "B2 (ISO 8859-1); BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below)" },
        /* 40*/ { UNICODE_MODE, ZINT_FULL_MULTIBYTE, "áA", 0, "CE 00 40 00 EC 11 EC 11 00 (68)", 1, 1, "K1 (ISO 8859-1) (full multibyte)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_microqr_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_MICROQR, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, \"%s\", %s, \"%s\", %d, %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode),
                    testUtilOption3Name(BARCODE_MICROQR, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].bwipp_cmp, data[i].zxingcpp_cmp, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, -1, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        char modules_dump[17 * 17 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, -1, -1, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    if (!data[i].zxingcpp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[17 * 17 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Check MICROQR padding (4-bit final codeword for M1 and M3 in particular) */
static void test_microqr_padding(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_1;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { -1, "1", 0, "22 00 00 (20)", "M1, bits left 13" },
        /*  1*/ { -1, "12", 0, "43 00 00 (20)", "M1, bits left 10" },
        /*  2*/ { -1, "123", 0, "63 D8 00 (20)", "M1, bits left 7" },
        /*  3*/ { -1, "1234", 0, "83 DA 00 (20)", "M1, bits left 3" },
        /*  4*/ { -1, "12345", 0, "A3 DA D0 (20)", "M1, bits left 0" },
        /*  5*/ { 1, "123456", 0, "30 F6 E4 00 EC (40)", "M2-L, bits left 15" },
        /*  6*/ { 1, "1234567", 0, "38 F6 E4 38 00 (40)", "M2-L, bits left 11" },
        /*  7*/ { 1, "12345678", 0, "40 F6 E4 4E 00 (40)", "M2-L, bits left 8" },
        /*  8*/ { 1, "123456789", 0, "48 F6 E4 62 A0 (40)", "M2-L, bits left 5" },
        /*  9*/ { 1, "1234567890", 0, "50 F6 E4 62 A0 (40)", "M2-L, bits left 1" },
        /* 10*/ { 2, "1234", 0, "20 F6 80 EC (32)", "M2-M, bits left 13" },
        /* 11*/ { 2, "123456", 0, "30 F6 E4 00 (32)", "M2-M, bits left 7" },
        /* 12*/ { 2, "1234567", 0, "38 F6 E4 38 (32)", "M2-M, bits left 3" },
        /* 13*/ { 2, "12345678", 0, "40 F6 E4 4E (32)", "M2-M, bits left 0" },
        /* 14*/ { 1, "ABCDEF", 0, "E3 9A 8A 54 28 (40)", "M2-L, bits left 3" },
        /* 15*/ { 2, "ABCDE", 0, "D3 9A 8A 4E (32)", "M2-M, bits left 0" },
        /* 16*/ { 1, "1234567890123456789", 0, "26 3D B9 18 A8 18 AC D4 D2 00 00 (84)", "M3-L, bits left 13" },
        /* 17*/ { 1, "12345678901234567890", 0, "28 3D B9 18 A8 18 AC D4 D6 80 00 (84)", "M3-L, bits left 10" },
        /* 18*/ { 1, "123456789012345678901", 0, "2A 3D B9 18 A8 18 AC D4 DC 28 00 (84)", "M3-L, bits left 7" },
        /* 19*/ { 1, "1234567890123456789012", 0, "2C 3D B9 18 A8 18 AC D4 DC 29 00 (84)", "M3-L, bits left 3" },
        /* 20*/ { 1, "12345678901234567890123", 0, "2E 3D B9 18 A8 18 AC D4 DC 29 70 (84)", "M3-L, bits left 0" },
        /* 21*/ { 2, "1234567890", 0, "14 3D B9 18 A8 00 EC 11 00 (68)", "M3-M, bits left 27" },
        /* 22*/ { 2, "123456789012345678", 0, "24 3D B9 18 A8 18 AC D4 C0 (68)", "M3-M, bits left 1" },
        /* 23*/ { 1, "ABCDEFGHIJKLMN", 0, "78 E6 A2 95 0A B8 59 EB 99 7E A0 (84)", "M3-L, bits left 1" },
        /* 24*/ { 2, "ABCDEFGHIJK", 0, "6C E6 A2 95 0A B8 59 EA 80 (68)", "M3-M, bits left 1" },
        /* 25*/ { 1, "1234567890123456789012345678", 0, "0E 0F 6E 46 2A 06 2B 35 37 0A 75 46 F0 00 EC 11 (128)", "M4-L, bits left 25" },
        /* 26*/ { 1, "123456789012345678901234567890", 0, "0F 0F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 00 EC (128)", "M4-L, bits left 19" },
        /* 27*/ { 1, "1234567890123456789012345678901234", 0, "11 0F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 F6 80 (128)", "M4-L, bits left 5" },
        /* 28*/ { 1, "12345678901234567890123456789012345", 0, "11 8F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 F6 B4 (128)", "M4-L, bits left 1" },
        /* 29*/ { 1, "123456789012345ABCDEFGHIJK", 0, "07 8F 6E 46 2A 06 2B 25 67 35 14 A8 55 C2 CF 54 (128)", "M4-L, bits left 0" },
        /* 30*/ { 2, "123456789012345678901234567890", 0, "0F 0F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 (112)", "M4-M, bits left 3" },
        /* 31*/ { 3, "123456789012345678901", 0, "0A 8F 6E 46 2A 06 2B 35 37 0A (80)", "M4-Q, bits left 1" },
        /* 32*/ { 1, "ABCDEFGHIJKLMNOPQRSTU", 0, "35 39 A8 A5 42 AE 16 7A E6 5F AC 51 95 B4 25 E0 (128)", "M4-L, bits left 4" },
        /* 33*/ { 2, "ABCDEFGHIJKLMNOPQR", 0, "32 39 A8 A5 42 AE 16 7A E6 5F AC 51 95 A0 (112)", "M4-M, bits left 5" },
        /* 34*/ { 3, "ABCDEFGHIJKLM", 0, "2D 39 A8 A5 42 AE 16 7A E6 56 (80)", "M4-Q, bits left 0" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_microqr_padding", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
        symbol->input_mode = UNICODE_MODE;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %d, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, data[i].option_1, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, -1, -1, debug)) {
                char modules_dump[17 * 17 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilBwipp(i, symbol, data[i].option_1, -1, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                int cmp_len, ret_len;
                char modules_dump[17 * 17 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
            }
        }
        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_microqr_optimize(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int ret;
        char *expected;
        int bwipp_cmp;
        int zxingcpp_cmp;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, -1, "1", 0, "22 00 00 (20)", 1, 1, "N1" },
        /*  1*/ { UNICODE_MODE, 1, 2, -1, "A123", 0, "92 86 3D 80 EC (40)", 1, 1, "A1 N3" },
        /*  2*/ { UNICODE_MODE, 1, -1, -1, "AAAAAA", 0, "E3 98 73 0E 60 (40)", 1, 1, "A6" },
        /*  3*/ { UNICODE_MODE, 1, -1, -1, "AA123456", 0, "A3 98 61 ED C8 (40)", 1, 1, "A2 N6" },
        /*  4*/ { UNICODE_MODE, 1, 3, -1, "01a", 0, "04 06 16 10 00 EC 11 EC 11 EC 00 (84)", 1, 1, "N3 B1" },
        /*  5*/ { UNICODE_MODE, 1, 4, -1, "01a", 0, "43 30 31 61 00 00 EC 11 EC 11 EC 11 EC 11 EC 11 (128)", 1, 1, "B3" },
        /*  6*/ { UNICODE_MODE, 1, -1, -1, "こんwa、αβ", 0, "46 82 B1 82 F1 77 61 66 00 10 FF 88 00 00 EC 11 (128)", 1, 1, "B6 K3" },
        /*  7*/ { UNICODE_MODE, 1, -1, -1, "こんにwa、αβ", 0, "66 13 10 B8 85 25 09 DD 85 98 00 43 FE 20 00 00 (128)", 1, 1, "K3 B2 K3" },
        /*  8*/ { UNICODE_MODE, 1, 3, -1, "こんAB123\177", 0, "D0 4C 42 E2 91 CD 06 3D C2 FE 00 (84)", 0, 1, "K2 A2 N3 B1; BWIPP different encodation (K2 A5 B1)" },
        /*  9*/ { UNICODE_MODE, 1, 4, -1, "こんAB123\177", 0, "64 13 10 B8 92 9C D0 5E 1A 0B F8 00 EC 11 EC 11 (128)", 1, 1, "K2 A5 B1" },
        /* 10*/ { DATA_MODE, 1, -1, -1, "\223\137", 0, "8A 4D 7C 00 EC 11 EC 11 EC 11 00 (84)", 0, 0, "B2; BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 11*/ { DATA_MODE, 1, -1, ZINT_FULL_MULTIBYTE, "\223\137", 0, "CB 67 C0 00 EC 11 EC 11 EC 11 00 (84)", 1, 1, "K1" },
        /* 12*/ { DATA_MODE, 1, -1, ZINT_FULL_MULTIBYTE | (1 << 8), "\223\137", 0, "CB 67 C0 00 EC 11 EC 11 EC 11 00 (84)", 1, 1, "K1" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_microqr_optimize", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_MICROQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, \"%s\", %d, %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    testUtilOption3Name(BARCODE_MICROQR, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].bwipp_cmp, data[i].zxingcpp_cmp, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                if (!data[i].bwipp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    char modules_dump[17 * 17 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                }
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                if (!data[i].zxingcpp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    int cmp_len, ret_len;
                    char modules_dump[17 * 17 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_microqr_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
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
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 1, -1, -1, "01234567", 0, 13, 13, 1, "ISO 18004 Figure 2 (and I.2) (mask 01)",
                    "1111111010101"
                    "1000001011101"
                    "1011101001101"
                    "1011101001111"
                    "1011101011100"
                    "1000001010001"
                    "1111111001111"
                    "0000000001100"
                    "1101000010001"
                    "0110101010101"
                    "1110011111110"
                    "0001010000110"
                    "1110100110111"
                },
        /*  1*/ { UNICODE_MODE, 2, -1, -1, "12345", 0, 13, 13, 1, "ISO 18004 Figure 38 (mask 00)",
                    "1111111010101"
                    "1000001010000"
                    "1011101011101"
                    "1011101000011"
                    "1011101001110"
                    "1000001010001"
                    "1111111000101"
                    "0000000001011"
                    "1110011110000"
                    "0111100101100"
                    "1110000001110"
                    "0100100010101"
                    "1111111010011"
                },
        /*  2*/ { UNICODE_MODE, 2, -1, 1 << 8, "12345", 0, 13, 13, 1, "ISO 18004 Figure 38, explicit mask 00",
                    "1111111010101"
                    "1000001010000"
                    "1011101011101"
                    "1011101000011"
                    "1011101001110"
                    "1000001010001"
                    "1111111000101"
                    "0000000001011"
                    "1110011110000"
                    "0111100101100"
                    "1110000001110"
                    "0100100010101"
                    "1111111010011"
                },
        /*  3*/ { UNICODE_MODE, 2, -1, ZINT_FULL_MULTIBYTE | 2 << 8, "12345", 0, 13, 13, 1, "Explicit mask 01",
                    "1111111010101"
                    "1000001000001"
                    "1011101001100"
                    "1011101011101"
                    "1011101000000"
                    "1000001000000"
                    "1111111010100"
                    "0000000000101"
                    "1110001011110"
                    "0001101011101"
                    "1000001111111"
                    "0101010011011"
                    "1110001011101"
                },
        /*  4*/ { UNICODE_MODE, 2, -1, 3 << 8, "12345", 0, 13, 13, 1, "Explicit mask 10",
                    "1111111010101"
                    "1000001010001"
                    "1011101001111"
                    "1011101010110"
                    "1011101011010"
                    "1000001010110"
                    "1111111010101"
                    "0000000011010"
                    "1110110110010"
                    "0101001111001"
                    "1010100101010"
                    "0100011010010"
                    "1111111010011"
                },
        /*  5*/ { UNICODE_MODE, 2, -1, ZINT_FULL_MULTIBYTE | 4 << 8, "12345", 0, 13, 13, 1, "Explicit mask 11",
                    "1111111010101"
                    "1000001001110"
                    "1011101010101"
                    "1011101001001"
                    "1011101010000"
                    "1000001001001"
                    "1111111001111"
                    "0000000010101"
                    "1110100011000"
                    "0010110000110"
                    "1111110000000"
                    "0011100101101"
                    "1010101111001"
                },
        /*  6*/ { UNICODE_MODE, 2, -1, 5 << 8, "12345", 0, 13, 13, 1, "Mask > 4 ignored",
                    "1111111010101"
                    "1000001010000"
                    "1011101011101"
                    "1011101000011"
                    "1011101001110"
                    "1000001010001"
                    "1111111000101"
                    "0000000001011"
                    "1110011110000"
                    "0111100101100"
                    "1110000001110"
                    "0100100010101"
                    "1111111010011"
                },
        /*  7*/ { UNICODE_MODE, -1, -1, -1, "12345", 0, 11, 11, 1, "Max capacity M1 5 numbers",
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
        /*  8*/ { UNICODE_MODE, -1, -1, -1, "1234567890", 0, 13, 13, 1, "Max capacity M2-L 10 numbers",
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
        /*  9*/ { UNICODE_MODE, 2, -1, -1, "12345678", 0, 13, 13, 1, "Max capacity M2-M 8 numbers",
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
        /* 10*/ { UNICODE_MODE, -1, -1, -1, "12345678901234567890123", 0, 15, 15, 1, "Max capacity M3-L 23 numbers",
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
        /* 11*/ { UNICODE_MODE, 2, -1, -1, "123456789012345678", 0, 15, 15, 1, "Max capacity M3-L 18 numbers",
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
        /* 12*/ { UNICODE_MODE, -1, -1, -1, "12345678901234567890123456789012345", 0, 17, 17, 1, "Max capacity M4-L 35 numbers",
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
        /* 13*/ { UNICODE_MODE, 2, -1, -1, "123456789012345678901234567890", 0, 17, 17, 1, "Max capacity M4-M 30 numbers",
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
        /* 14*/ { UNICODE_MODE, 3, -1, -1, "123456789012345678901", 0, 17, 17, 1, "Max capacity M4-Q 21 numbers",
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
        /* 15*/ { UNICODE_MODE, 1, -1, -1, "点茗テ点茗テ点茗テ", 0, 17, 17, 1, "Max capacity M4-L 9 Kanji",
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
        /* 16*/ { UNICODE_MODE | ESCAPE_MODE, 1, -1, -1, "P-5-B223G846\015", 0, 17, 17, 1, "Ticket #241, 2.6.7 dropping off last character if mode different (MR #21)",
                    "11111110101010101"
                    "10000010100001100"
                    "10111010011110010"
                    "10111010100000101"
                    "10111010011011111"
                    "10000010111110100"
                    "11111110110111101"
                    "00000000011001000"
                    "10010111011010100"
                    "01110111000001110"
                    "10100011001110111"
                    "00011111000101010"
                    "10011111101100110"
                    "01110000111010100"
                    "11001010110000000"
                    "00011110011010001"
                    "11101010111111111"
                },
        /* 17*/ { UNICODE_MODE, 1, 2, -1, "1ABCDE", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001011100"
                    "1011101000101"
                    "1011101001101"
                    "1011101010111"
                    "1000001010100"
                    "1111111000001"
                    "0000000001011"
                    "1101000010011"
                    "0001000110001"
                    "1111101011010"
                    "0010101010111"
                    "1111101111110"
                },
        /* 18*/ { UNICODE_MODE, 1, 2, -1, "ABCDE1", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001010111"
                    "1011101011110"
                    "1011101011010"
                    "1011101001100"
                    "1000001011101"
                    "1111111011000"
                    "0000000011000"
                    "1101101010110"
                    "0000100110001"
                    "1111011100101"
                    "0101110111101"
                    "1110011101110"
                },
        /* 19*/ { UNICODE_MODE, 1, 2, -1, "12ABCD", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001001011"
                    "1011101010111"
                    "1011101010011"
                    "1011101010101"
                    "1000001001001"
                    "1111111011100"
                    "0000000000110"
                    "1101010111101"
                    "0001111001000"
                    "1011011110011"
                    "0010110100001"
                    "1110110110100"
                },
        /* 20*/ { UNICODE_MODE, 1, 2, -1, "ABCD12", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001001001"
                    "1011101011110"
                    "1011101010000"
                    "1011101010110"
                    "1000001001001"
                    "1111111010010"
                    "0000000000110"
                    "1101010111110"
                    "0111000001111"
                    "1111100101111"
                    "0101111000101"
                    "1101101001100"
                },
        /* 21*/ { UNICODE_MODE, 1, 2, -1, "123ABC", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001000011"
                    "1011101010001"
                    "1011101011001"
                    "1011101011101"
                    "1000001001111"
                    "1111111011101"
                    "0000000000011"
                    "1101010111100"
                    "0000111100000"
                    "1010000100110"
                    "0100000001110"
                    "1011111101011"
                },
        /* 22*/ { UNICODE_MODE, 1, 2, -1, "1234ABC", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001010111"
                    "1011101011001"
                    "1011101011000"
                    "1011101001100"
                    "1000001010001"
                    "1111111010111"
                    "0000000010001"
                    "1101101010100"
                    "0011010001010"
                    "1100001001001"
                    "0100110000101"
                    "1111010110101"
                },
        /* 23*/ { UNICODE_MODE, 1, 2, -1, "ABC1234", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001000010"
                    "1011101011111"
                    "1011101011001"
                    "1011101010001"
                    "1000001000101"
                    "1111111010010"
                    "0000000001010"
                    "1101010111110"
                    "0000011110111"
                    "1111000101111"
                    "0100100011111"
                    "1011111100110"
                },
        /* 24*/ { UNICODE_MODE, 1, 2, -1, "12345AB", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001000101"
                    "1011101011101"
                    "1011101011111"
                    "1011101011010"
                    "1000001001001"
                    "1111111011001"
                    "0000000000111"
                    "1101010111100"
                    "0111110110000"
                    "1000101000010"
                    "0001111010101"
                    "1001001101111"
                },
        /* 25*/ { UNICODE_MODE, 1, 2, -1, "123456AB", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001001000"
                    "1011101010101"
                    "1011101010001"
                    "1011101010100"
                    "1000001001101"
                    "1111111011101"
                    "0000000001111"
                    "1101010110100"
                    "0110000100000"
                    "1010001001011"
                    "0100011101011"
                    "1010110111111"
                },
        /* 26*/ { UNICODE_MODE, 1, 2, -1, "AB123456", 0, 13, 13, 1, "M2-L numeric/alpha mix",
                    "1111111010101"
                    "1000001010000"
                    "1011101010011"
                    "1011101010111"
                    "1011101001011"
                    "1000001010101"
                    "1111111010100"
                    "0000000011000"
                    "1101101010110"
                    "0000111111101"
                    "1000100111101"
                    "0000010110101"
                    "1101001101000"
                },
        /* 27*/ { UNICODE_MODE, 1, 3, -1, "ABCDEFGHIJKLM1", 0, 15, 15, 1, "M3-L numeric/alpha mix",
                    "111111101010101"
                    "100000101100010"
                    "101110100100101"
                    "101110100011011"
                    "101110100010111"
                    "100000100010110"
                    "111111101110000"
                    "000000000011101"
                    "111110010000010"
                    "001011101001111"
                    "101111001010100"
                    "000100011011011"
                    "100011100001100"
                    "000111010101111"
                    "110010111100110"
                },
        /* 28*/ { UNICODE_MODE, 1, 3, -1, "1ABCDEFGHIJKLM", 0, 15, 15, 1, "M3-L numeric/alpha mix",
                    "111111101010101"
                    "100000101100011"
                    "101110100101101"
                    "101110100101101"
                    "101110100010000"
                    "100000100011101"
                    "111111101110000"
                    "000000000001111"
                    "111110010001001"
                    "011111100110100"
                    "101101010100111"
                    "001010011011111"
                    "111011111001100"
                    "001000000001111"
                    "101110101111110"
                },
        /* 29*/ { UNICODE_MODE, 1, 3, -1, "1234567890123456789A", 0, 15, 15, 1, "M3-L numeric/alpha mix",
                    "111111101010101"
                    "100000100000110"
                    "101110100011111"
                    "101110100100110"
                    "101110101101010"
                    "100000101000111"
                    "111111101101110"
                    "000000001010010"
                    "111101100110100"
                    "011011010100111"
                    "110000001001111"
                    "000111010000101"
                    "111101000011001"
                    "000000100101101"
                    "101101100001111"
                },
        /* 30*/ { UNICODE_MODE, 1, 3, -1, "A1234567890123456789", 0, 15, 15, 1, "M3-L numeric/alpha mix",
                    "111111101010101"
                    "100000101010001"
                    "101110101110111"
                    "101110101011111"
                    "101110101111111"
                    "100000100000110"
                    "111111100010010"
                    "000000001011101"
                    "111100110110111"
                    "010100110000010"
                    "111001111001101"
                    "010010001101100"
                    "110000111010101"
                    "000011011000111"
                    "111111101100010"
                },
        /* 31*/ { UNICODE_MODE, 1, 3, -1, "ABCDEFGHIJKa", 0, 15, 15, 1, "M3-L alpha/byte mix",
                    "111111101010101"
                    "100000101110010"
                    "101110100000101"
                    "101110100101011"
                    "101110100000111"
                    "100000100010110"
                    "111111101000000"
                    "000000000111101"
                    "111110010010010"
                    "001001101001111"
                    "101100011010100"
                    "011010011111011"
                    "110101000001110"
                    "001011111101101"
                    "110111010100110"
                },
        /* 32*/ { UNICODE_MODE, 1, 3, -1, "aABCDEFGHIJK", 0, 15, 15, 1, "M3-L alpha/byte mix",
                    "111111101010101"
                    "100000100000001"
                    "101110101110011"
                    "101110101000010"
                    "101110100111101"
                    "100000101001011"
                    "111111100101010"
                    "000000000111101"
                    "111111000011100"
                    "001111011001101"
                    "110110010001100"
                    "011111111111110"
                    "100011111111001"
                    "000000111010011"
                    "111010110000011"
                },
        /* 33*/ { UNICODE_MODE, 1, 3, -1, "abcdefghA", 0, 15, 15, 1, "M3-L alpha/byte mix",
                    "111111101010101"
                    "100000100001011"
                    "101110101111111"
                    "101110101010011"
                    "101110100000100"
                    "100000101011100"
                    "111111100110110"
                    "000000000100001"
                    "111111000111100"
                    "000110011001101"
                    "111011111110100"
                    "011000110111010"
                    "101101110000101"
                    "000010001100010"
                    "100011000101111"
                },
        /* 34*/ { UNICODE_MODE, 1, 3, -1, "Aabcdefgh", 0, 15, 15, 1, "M3-L alpha/byte mix",
                    "111111101010101"
                    "100000100000011"
                    "101110101111111"
                    "101110101010011"
                    "101110100101111"
                    "100000101000000"
                    "111111100100110"
                    "000000000100001"
                    "111111000001100"
                    "000001011110101"
                    "100011110110101"
                    "010001110111010"
                    "100101101100101"
                    "010001110101110"
                    "110101101101111"
                },
        /* 35*/ { UNICODE_MODE, 2, 3, -1, "é", 0, 15, 15, 1, "M3-M byte (B1), bits left 54",
                    "111111101010101"
                    "100000101000101"
                    "101110101011111"
                    "101110101111010"
                    "101110100111111"
                    "100000100010011"
                    "111111100001101"
                    "000000000100100"
                    "100010011101010"
                    "000111100100111"
                    "101101101000110"
                    "010010011100000"
                    "100011100101111"
                    "001111011101000"
                    "110100010100101"
                },
        /* 36*/ { UNICODE_MODE, 1, 4, -1, "A1abcdefghijklm", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B15)",
                    "11111110101010101"
                    "10000010110110110"
                    "10111010110001111"
                    "10111010000000100"
                    "10111010111010000"
                    "10000010111111011"
                    "11111110010101011"
                    "00000000101001111"
                    "10011101000110111"
                    "01101100111101111"
                    "10001101011010010"
                    "00010000110101001"
                    "10101100011011101"
                    "01001101110000111"
                    "10111111010111100"
                    "01101110010001101"
                    "11001111000011100"
                },
        /* 37*/ { UNICODE_MODE, 1, 4, -1, "abcdefghijklmA1", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B15)",
                    "11111110101010101"
                    "10000010010100111"
                    "10111010110110111"
                    "10111010000111010"
                    "10111010000111110"
                    "10000010010000101"
                    "11111110010110111"
                    "00000000000111010"
                    "10010010010110110"
                    "01111101110110110"
                    "10001111011110111"
                    "00011011111111010"
                    "11110100110111010"
                    "00000001100010111"
                    "10100000000110100"
                    "01010111011111011"
                    "10111101110110010"
                },
        /* 38*/ { UNICODE_MODE, 1, 4, -1, "1Aabcdefghijklm", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B15)",
                    "11111110101010101"
                    "10000010110101010"
                    "10111010010101011"
                    "10111010101010001"
                    "10111010010011001"
                    "10000010110001010"
                    "11111110110101011"
                    "00000000001010000"
                    "10010111000010001"
                    "01101111110111010"
                    "11111100110011011"
                    "01010100011011011"
                    "10110000011011111"
                    "00001001110011011"
                    "11011000010011000"
                    "00101111011011000"
                    "10110011001010101"
                },
        /* 39*/ { UNICODE_MODE, 1, 4, -1, "abcdefghijklm1A", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B15)",
                    "11111110101010101"
                    "10000010110111011"
                    "10111010010101011"
                    "10111010111011001"
                    "10111010011011101"
                    "10000010110011001"
                    "11111110110101011"
                    "00000000011011001"
                    "10010111001010101"
                    "00001101100101010"
                    "11100011000101011"
                    "00000010100011001"
                    "10110101101011001"
                    "01001000110001011"
                    "11100010011101000"
                    "01000101100011000"
                    "11110110101010001"
                },
        /* 40*/ { UNICODE_MODE, 1, 4, -1, "A1Aabcdefghijkl", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B15)",
                    "11111110101010101"
                    "10000010110001010"
                    "10111010010101011"
                    "10111010101010000"
                    "10111010001010101"
                    "10000010110111010"
                    "11111110110011011"
                    "00000000001010011"
                    "10010111010011011"
                    "01111011010011010"
                    "10001111010011011"
                    "01001000001010000"
                    "11110011000010001"
                    "00111010110101011"
                    "11111011010011000"
                    "01010100111011000"
                    "11100110011011101"
                },
        /* 41*/ { UNICODE_MODE, 1, 4, -1, "abcdefghijklA1A", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B15)",
                    "11111110101010101"
                    "10000010110111011"
                    "10111010010101011"
                    "10111010111011001"
                    "10111010011011101"
                    "10000010110011001"
                    "11111110111101011"
                    "00000000000011001"
                    "10010111001010101"
                    "00011011000101010"
                    "10110000000101011"
                    "01011011100011001"
                    "10010001101011001"
                    "01000010110001011"
                    "11101000111101000"
                    "01100100100011000"
                    "11100100101010001"
                },
        /* 42*/ { UNICODE_MODE, 1, 4, -1, "A1A1abcdefghijk", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (A4 B11)",
                    "11111110101010101"
                    "10000010101100010"
                    "10111010010001000"
                    "10111010100101100"
                    "10111010001100111"
                    "10000010101100111"
                    "11111110110101101"
                    "00000000010101001"
                    "10010111001100101"
                    "00100010001100100"
                    "11111001110011110"
                    "00001000001100111"
                    "10001101001100111"
                    "00110101101100100"
                    "10100100110111101"
                    "01011100011101101"
                    "11010100111100111"
                },
        /* 43*/ { UNICODE_MODE, 1, 4, -1, "abcdefghijkA1A1", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B11 A4)",
                    "11111110101010101"
                    "10000010100111011"
                    "10111010010101011"
                    "10111010110011001"
                    "10111010011011101"
                    "10000010100011001"
                    "11111110100101011"
                    "00000000001011001"
                    "10010111011010101"
                    "00110011010101010"
                    "10110000010101011"
                    "01011000010011001"
                    "11000101000011001"
                    "01011001100001011"
                    "11110100111101010"
                    "01111111011011000"
                    "11100111011010001"
                },
        /* 44*/ { UNICODE_MODE, 1, 4, -1, "A1A1Aabcdefghij", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (A5 B10)",
                    "11111110101010101"
                    "10000010110010100"
                    "10111010000011000"
                    "10111010110011000"
                    "10111010010011111"
                    "10000010101010111"
                    "11111110111011001"
                    "00000000010101001"
                    "10010111010011001"
                    "01011010001010000"
                    "11010110001010110"
                    "01100011010001011"
                    "10101101110011011"
                    "01010000101010010"
                    "10000101110011001"
                    "00111101000111001"
                    "11010001011111011"
                },
        /* 45*/ { UNICODE_MODE, 1, 4, -1, "abcdefghijA1A1A", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B10 A5)",
                    "11111110101010101"
                    "10000010100101011"
                    "10111010000011011"
                    "10111010101011001"
                    "10111010011111101"
                    "10000010110011001"
                    "11111110110101011"
                    "00000000010011001"
                    "10010111000010101"
                    "01011111100101010"
                    "11001000011101011"
                    "00010001011011001"
                    "11010111011011001"
                    "01011111101001001"
                    "11101111110101010"
                    "01100100000011000"
                    "11101010011010001"
                },
        /* 46*/ { UNICODE_MODE, 1, 4, -1, "A1A1A1abcdefghi", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (A6 B9)",
                    "11111110101010101"
                    "10000010100111100"
                    "10111010000111000"
                    "10111010100110000"
                    "10111010000100111"
                    "10000010101010111"
                    "11111110100111001"
                    "00000000010111001"
                    "10010111011111101"
                    "01001110100010100"
                    "11101000000110010"
                    "00010000110110011"
                    "11001001011101111"
                    "00111000101100101"
                    "11011101111110001"
                    "01101100100110001"
                    "10110010111110111"
                },
        /* 47*/ { UNICODE_MODE, 1, 4, -1, "abcdefghiA1A1A1", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B9 A6)",
                    "11111110101010101"
                    "10000010110001011"
                    "10111010010101011"
                    "10111010110111001"
                    "10111010000111101"
                    "10000010100011001"
                    "11111110111011011"
                    "00000000011011001"
                    "10010111011110101"
                    "01111110011101010"
                    "10010001110101011"
                    "01011110000011001"
                    "11010110101011001"
                    "01111110101001010"
                    "10010000111101010"
                    "00001000100011000"
                    "11110110111010001"
                },
        /* 50*/ { UNICODE_MODE, 1, 4, -1, "A1A1A1Aabcdefghi", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (A7 B9)",
                    "11111110101010101"
                    "10000010111011100"
                    "10111010011101000"
                    "10111010111000000"
                    "10111010011000111"
                    "10000010111000111"
                    "11111110111100101"
                    "00000000011001001"
                    "10010111010001101"
                    "01000011111010100"
                    "10110011001010110"
                    "01000011100000011"
                    "10110001111001011"
                    "00001011011001111"
                    "10000000001011101"
                    "01101000000000001"
                    "11010011110001011"
                },
        /* 49*/ { UNICODE_MODE, 1, 4, -1, "abcdefghiA1A1A1A", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (B9 A7)",
                    "11111110101010101"
                    "10000010110001011"
                    "10111010010101011"
                    "10111010110111001"
                    "10111010000111101"
                    "10000010100111001"
                    "11111110111011011"
                    "00000000011011001"
                    "10010111011110101"
                    "01110111011101010"
                    "11000111110101011"
                    "00101011100011001"
                    "10101110101011001"
                    "00100100001001010"
                    "11001101101101010"
                    "00111010110011000"
                    "11000101111010001"
                },
        /* 50*/ { UNICODE_MODE, 1, 4, -1, "A1A1A1A1abcdefgh", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (A8 B8)",
                    "11111110101010101"
                    "10000010110011100"
                    "10111010001011000"
                    "10111010110000000"
                    "10111010010010111"
                    "10000010110010111"
                    "11111110110010101"
                    "00000000010111101"
                    "10010111010111101"
                    "00111011110010000"
                    "11000000100010010"
                    "00010001010011011"
                    "11110110110111111"
                    "01101010001010100"
                    "10010101111011110"
                    "00110010100101001"
                    "11010101111111011"
                },
        /* 51*/ { UNICODE_MODE, 1, 4, -1, "abcdefghA1A1A1A1", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (A8 B8)",
                    "11111110101010101"
                    "10000010100111011"
                    "10111010011011011"
                    "10111010111011001"
                    "10111010011011101"
                    "10000010111001001"
                    "11111110110101011"
                    "00000000000111001"
                    "10010111001110101"
                    "01101000001001010"
                    "11110000001101011"
                    "01100101111011001"
                    "11001000111111001"
                    "00110000100001000"
                    "10001100000101010"
                    "00011001000011000"
                    "11101100111010001"
                },
        /* 52*/ { UNICODE_MODE, 1, 4, -1, "A1A1A1A1A1abcdefg", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (A10 B7)",
                    "11111110101010101"
                    "10000010111011100"
                    "10111010011011000"
                    "10111010101000000"
                    "10111010011000111"
                    "10000010111000111"
                    "11111110111010101"
                    "00000000001001101"
                    "10010111010001101"
                    "01011011011010000"
                    "11000100111110010"
                    "01001100011000011"
                    "10101101111000011"
                    "01010010011010101"
                    "10001101111001110"
                    "00101010111001001"
                    "10000110110101011"
                },
        /* 53*/ { UNICODE_MODE, 1, 4, -1, "abcdefgA1A1A1A1A1", 0, 17, 17, 0, "M4-L alpha/numeric/byte mix (B7 A10); BWIPP fails with Maximum length exceeded",
                    "11111110101010101"
                    "10000010111010111"
                    "10111010110101111"
                    "10111010001011100"
                    "10111010100110100"
                    "10000010100001000"
                    "11111110001011011"
                    "00000000111000101"
                    "10011101011110001"
                    "00000101001011111"
                    "10100001101100010"
                    "00010010101001000"
                    "11110111000111001"
                    "01011010101000111"
                    "10111011111001101"
                    "00001010111001101"
                    "11111111011111000"
                },
        /* 54*/ { UNICODE_MODE, 1, 4, -1, "A1A1A1A1A1Aabcdef", 0, 17, 17, 1, "M4-L alpha/numeric/byte mix (A11 B6)",
                    "11111110101010101"
                    "10000010100111100"
                    "10111010001111000"
                    "10111010101100000"
                    "10111010000110111"
                    "10000010100110111"
                    "11111110110100101"
                    "00000000000001101"
                    "10010111000111101"
                    "01110100100110000"
                    "11101011110110010"
                    "00000111001110011"
                    "10010111000110011"
                    "00011001100010111"
                    "11110011100111110"
                    "01110100100101001"
                    "11000111011011011"
                },
        /* 55*/ { UNICODE_MODE, 1, 4, -1, "abcdefA1A1A1A1A1A", 0, 17, 17, 0, "M4-L alpha/numeric/byte mix (A11 B6); BWIPP fails with Maximum length exceeded",
                    "11111110101010101"
                    "10000010101101011"
                    "10111010001111011"
                    "10111010111011001"
                    "10111010011001101"
                    "10000010100001001"
                    "11111110100001011"
                    "00000000000001001"
                    "10010111000110101"
                    "01111101001111010"
                    "10001010111011011"
                    "01110000110011001"
                    "11110111110011001"
                    "01101011110000001"
                    "11110110001101001"
                    "00001011000110100"
                    "11010110111110001"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_microqr_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MICROQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    testUtilOption3Name(BARCODE_MICROQR, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[17 * 17 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Not a real test, just performance indicator */
static void test_microqr_perf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

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
    static const struct item data[] = {
        /*  0*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 1, "12345", 0, 11, 11, "Max 5 numbers, M1" },
        /*  1*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 2, "1234567890", 0, 13, 13, "Max 10 numbers, M2-L" },
        /*  2*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 3, "123456789012345", 0, 15, 15, "Max 15 numbers, M3-L" },
        /*  3*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 4, "12345678901234567890123456789012345", 0, 17, 17, "Max 35 numbers, M4-L" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    clock_t start, total_encode = 0, total_buffer = 0, diff_encode, diff_buffer;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_encode = diff_buffer = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            struct zint_symbol *symbol = ZBarcode_Create();
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
    if (p_ctx->index != -1) {
        printf("totals: encode %gms, buffer %gms\n", total_encode * 1000.0 / CLOCKS_PER_SEC, total_buffer * 1000.0 / CLOCKS_PER_SEC);
    }
}

static void test_upnqr_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    /* Ą U+0104 in ISO 8859-2 0xA1, in other ISO 8859 and Win 1250, UTF-8 C484 */
    /* Ŕ U+0154 in ISO 8859-2 0xC0, in Win 1250 but not other ISO 8859 or Win page, UTF-8 C594 */
    /* é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9 */
    /* β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2 */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, "ĄŔ", 0, "(415) 70 44 00 02 A1 C0 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC", "ECI-4 B2 (ISO 8859-2)" },
        /*  1*/ { UNICODE_MODE, "é", 0, "(415) 70 44 00 01 E9 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11", "ECI-4 B1 (ISO 8859-2)" },
        /*  2*/ { UNICODE_MODE, "β", ZINT_ERROR_INVALID_DATA, "Error 572: Invalid character in input for ECI '4'", "β not in ISO 8859-2" },
        /*  3*/ { DATA_MODE, "\300\241", 0, "(415) 70 44 00 02 C0 A1 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC", "ŔĄ" },
        /*  4*/ { GS1_MODE, "[20]12", ZINT_ERROR_INVALID_OPTION, "Error 220: Selected symbology does not support GS1 mode", "" },
        /*  5*/ { UNICODE_MODE, "ĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄ", 0, "(415) 70 44 01 9B A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1", "Length 411" },
        /*  6*/ { UNICODE_MODE, "ĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄ", ZINT_ERROR_TOO_LONG, "Error 573: Input too long, requires 416 codewords (maximum 415)", "Length 412" },
        /*  7*/ { DATA_MODE, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", 0, "(415) 70 44 01 9B 31 32 33 34 35 36 37 38 39 30 31 32 33 34 35 36 37 38 39 30 31 32 33 34", "Length 411" },
        /*  8*/ { DATA_MODE, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012", ZINT_ERROR_TOO_LONG, "Error 573: Input too long, requires 416 codewords (maximum 415)", "Length 412" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    #if 0 /* Need to add "force binary mode" to BWIPP for this to work */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    #endif
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_upnqr_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_UPNQR, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        if (ret < ZINT_ERROR) {
            assert_equal(symbol->eci, 4, "i:%d ZBarcode_Encode symbol->eci %d != 4\n", i, symbol->eci);
        }

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[77 * 77 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_upnqr_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
    };

    /* https://www.upn-qr.si/uploads/files/Tehnicni standard UPN QR.pdf */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, -1, "UPNQR\012\012\012\012\012Janez Novak\012Dunajska 1\0121000 Ljubljana\01200000008105\012\012\012COST\012Plačilo obveznosti 10/2016\012\012SI56051008010486080\012SI0598765432100\012Novo podjetje d.o.o.\012Lepa cesta 15\0123698 Loški Potok\012183\012                                                                                                                                                                                                                     ", 0, 77, 77, "Example A",
                    "11111110111111111100010011100010110001111000010010000011011111010000001111111"
                    "10000010100011011111111001110100100100111110110100100100100111100010101000001"
                    "10111010001000100000110111010001001000001000001101001001001001011000101011101"
                    "10111010101000111111100101110000011010110100010001010110010010111000101011101"
                    "10111010001000001001001011111100000110101100001111110101110011110111101011101"
                    "10000010001111111001010010001001001000001000001000101001011001101010001000001"
                    "11111110101010101010101010101010101010101010101010101010101010101010101111111"
                    "00000000110100001001111110001101011101000100101000100100100100100100100000000"
                    "10110111001011100111111111111011000001101011011111101101010000000101001001011"
                    "11001001101111111100011011010011101000111100110110000001000110000010011010010"
                    "10010011010011001101011011110101100100100110100110100100111100100101100100000"
                    "10100000010000010100110110001000001001001001001101011011000001011001001001000"
                    "11000110111001011001100000101001001011110000000011000110001011000111110000010"
                    "01100000001000001010001010000101000110101100000100100111110000100100001100100"
                    "11011010001110100011010100011001011001001001001001001011001001001000001001101"
                    "01100101000011101111001001011100110111010010010110011000011010010010010010011"
                    "01011011110011001110010110000111101100100110110101001010110101100001100110100"
                    "00000101101101100110100111101110101010100001101001010000001101011001100001011"
                    "00000011000011011001011001010011010110010010010000011000110010010011010110110"
                    "11100101110001011000111101110100110110100100100000001101000100100100100000101"
                    "00011010011000100011111101110011011011101011000011000110011000001011011111011"
                    "11101000111110111101100000101011110010011010110111000001110110000000000010010"
                    "10001011110111101111111001111100111100100100101101100110000100100001100000101"
                    "00100000001000110101001110010000001111000001001011011000101001001001101101010"
                    "11111111110101101000100011111000011100111000001111100111000011010100111111010"
                    "11101000100100011011011010001101100110100100001000100101000000110000100011101"
                    "00111010110010001010010010101001001101001001001010101001101011001111101011100"
                    "01101000100011111100011110001010010000011010001000101010010110010010100010101"
                    "10001111100101011010000111111110101110010110111111101001110000100001111110010"
                    "00011000010101110110101111110001101001010001110000010001101110011010011110001"
                    "01101010101101011001000100111010010010010010010110110010010011010011011010101"
                    "01100101110001010010111010111100100000110101011000100100100010100110111011000"
                    "10111010001110111000010101110011000001101010100001101100111010001100001001111"
                    "01001001001010111000000101001010110110011010110011100011010100100001000100010"
                    "00000110111010100100111010001100100100110101110101100100100100100100111001010"
                    "01011001111110011000100111110001001001010100000000001000001111001011011010001"
                    "00111011010010000010110111001110011010111000000110010111101001010110110011100"
                    "01111100011111110001111011111100000110101111000101110101010000010010100110100"
                    "01001010110110000011110111100101001001011011001001101001010001001001010101101"
                    "00100000000111011011011101101110010010010110010010010010010010010000000110011"
                    "10001110000101001100110001011100101101100001101000000000110110001000000000100"
                    "00110001010111011000100111010011101011001000000000011000000100011001001101001"
                    "01010010001111001110110101011100010011010011010010010010010011111010010110110"
                    "11010101111000010110001010111000100100100100001100100100100000111100101100101"
                    "10100010001001000010001100110101000001101110101101001011001100111101100001001"
                    "01101001011100010101001101101110110000011110110011000111000001000010010000010"
                    "11101111111001110000001011111010100100100101101111100010110101000100111110000"
                    "00111000110101111110000010001100001000001101001000101001001001111001100011000"
                    "01111010100010111010110010101011000010110000001010110110000011000110101010010"
                    "11011000101011001011011010001100000110001010001000110111100000100100100010100"
                    "10001111111100100100000111111010011000001111001111100001001001001001111111101"
                    "11100001001110110011111000111010010010010010010010000010010010000011011010011"
                    "11000111010101101011000010000110111101000110100100110000110101110000100100100"
                    "10110001010110000001010000001010101011100001111101010010001101001001001001001"
                    "01010011111110111110111010111100010010010010010011011100010010010010010000110"
                    "01111101011001100100101010011110010101100100100100101100100100100101101100101"
                    "10010010101001000111101010111011000001101011001001001111011000001101101001001"
                    "01011101011000111010101001000010101101011010100110001111010110000011010010010"
                    "11110111111011001000011101001001111001100100100101100000100100100100000110010"
                    "00101101100110101001001100001011011001001001001001001001001001001000000001010"
                    "01001110101000011100110000011010010010110000010100011100100011010010110010000"
                    "01011001110011001100010011101001001010101100010000110000000000110001100100010"
                    "11011110101100100101101111110111011001001001001010000111001001001101101010000"
                    "10110000001001111010001001110000000010011010010000011000010010010011011010001"
                    "00011011010000111010011010100101001010011110100100101000010101100010000100100"
                    "01100101010110100001100110001101101011000001110111011000101101011001001000011"
                    "01001111100111100110111110110101101110010010011011111010110000010011010001100"
                    "00001001111101000000101011011110100000100100101101111101100100100111001101101"
                    "01111010101101000011101111111001000001100011011111111101111111001101111110011"
                    "00000000101101101110101110001110110100001010111000101010110111000000100011010"
                    "11111110110001001100010110101010100100100100111010100100100011100101101010000"
                    "10000010110110110101000110001011001001001001101000101001001111001001100010100"
                    "10111010010110001000101011111010011100101001101111110110100111010111111111110"
                    "10111010111111011111010010100000000110101100000111110101100010010110000000100"
                    "10111010111000101010000010001111001111011001010101101001101011001000001000101"
                    "10000010001101100100010110101010010010001110010010010010110010010010100101001"
                    "11111110100011011001111101001100101100000010111001100000111111111001000100010"
                },
        /*  1*/ { UNICODE_MODE, -1, -1, -1, "UPNQR\012SI56020170014356205\012\012\012SI003528-990\012Združenje bank Slovenije\012Šubičeva 2\0121000 Ljubljana\01200000128067\012\012\012ADVA\012Plačilo avansa-ponudba 2016/12\012\012SI56051008010486080\012SI00123456-67890-12345\012Novo podjetje d.o.o.\012Lepa cesta 15\0123698 Loški Potok\012238\012                                                                                                                                                              ", 0, 77, 77, "Example B",
                    "11111110000111011101000010011000101001000000001000111011101100110000001111111"
                    "10000010110011111100001100011111010101001001100001010101010101110010101000001"
                    "10111010001010110110001000110010101011101110001000101010100110110000101011101"
                    "10111010001111111101111110011110011101010101110100010001000110000100101011101"
                    "10111010100111000001000111111010001000000110101111111001100110010011101011101"
                    "10000010010011100010000010001100010100010101011000111101010101110110001000001"
                    "11111110101010101010101010101010101010101010101010101010101010101010101111111"
                    "00000000011101111011101110001101001110110101011000110101010101100101100000000"
                    "10101010010001000011011011111000100000001100111111110000111011011110000010010"
                    "00010001100001111111011101100100101101111001110001000110011001001101011101010"
                    "00100011011101000010110111011010101010101110101110100110111011100011001010001"
                    "10111100001110101010101001111101010100010001010101000111010101110101110101011"
                    "11101110011001000011011001111000110111101000111001100110111011111110101000101"
                    "00110001101101111101111101011101100011011011110101000110011001011101111101001"
                    "00111011101101100010110111101010111110101100101110101010010010101011101000010"
                    "10001000000000001000001000011010010101001001010111000111111101010101010011011"
                    "11001011110100100000011000001000100011010000111011111110010011101110101000111"
                    "00011101111110111010111111110010110011111111110001110000010001000101111101010"
                    "00110011111100100001100001111010101010111110101111101100101010101011001110001"
                    "10010100011100100001001100000100000100001101010101110111011101010101010111001"
                    "11010011100010010000011100010001000000010000111001000110011011101001111010101"
                    "00111101111101101010100011101101111110011101111101000000010001000111000001011"
                    "00000011100101101001100111110010111100111010111110001000101010101111001100111"
                    "00100100010100111001010010001100010000010100110011101110110111010001110101001"
                    "11101111101101011000000011111000110011001000011111100010011001101110111111111"
                    "10111000101011101010111010001100000110001101101000101000010011000010100011010"
                    "00011010101001100001100010101010101110101011101010101110101010101100101011001"
                    "10101000101010100011000010001101000011011101101000111101010011010100100011001"
                    "11101111101001001101001011111000100010000001101111100110011001101101111110101"
                    "01110000000111101010101011101101111011010100111100000100010001100000000011000"
                    "11001010011011111100001100000010100010101011101100001010101010001001100111000"
                    "01110001000000110001101011001101010001000101010110110101010101110101000101001"
                    "01010110101011001100101110010000110010000000100110101110111011001101101100101"
                    "01100100001011110011101010001101110001011100111101000100010001000000000100010"
                    "00000011010110110001011100000010100010111010101011101010001010001100100110011"
                    "11000000100010010111110011101101010101001100001100010100110101111111011111010"
                    "10000111101010100110000110110110100010001010111110101110011001000110000001111"
                    "01001101101110000000001011101101110111000111000100000100011001000111000001010"
                    "11101010010101111000101100000110101010111001111010001010101010100110100110101"
                    "11100101000001010001100011001001010101010001110101110101010001111101010001111"
                    "10000011001010101110011010010110000011101100010110101110110011000100011110001"
                    "01101100001011011000011111011110110111010001011100000100000011001111010001010"
                    "10101010110110100011100010010000101011101110111011001010110011100010110110001"
                    "00100101001000000010100101001001010101010000011101010101010101010101010100111"
                    "10000011001011000010001111110000100011001111110110101110111001101110001010001"
                    "00101100011010010000000110011101110111011000101100000110011001101101010111010"
                    "00101111110111111001100111111010101010101000101111101010101011001010111110001"
                    "11101000100000100111111010001001110010010101011000101101010101001101100011011"
                    "00001010101011100101101010101101111000001100111010100100111011011110101010101"
                    "10101000111010111100001010001110110110011101111000100000011001001101100011010"
                    "01111111111111100000000111111000111111101110101111101000101010000010111110001"
                    "00100101000010010000111100011000110000010101010101000101010101100100010101011"
                    "01001010000100100011001110011011010110001000101010100110111011100110100010101"
                    "10111100101000111111001111110001110110111101100001000110001001011101111101011"
                    "01101010110000000110000010101111100110101010101011101110001010101011101000010"
                    "00111100010111010101011101110111110001010101010101000101010101010101111101011"
                    "01100011101001000100001111001000010011001000101010100110011011101110101010101"
                    "10111001010000011101101111100101010011000101100001000110010001000101010101010"
                    "01000010101011000110010110000000100010111010101001101010001010101010001000000"
                    "00100000100101111101010110100101000000010101010111010111010101010100011101001"
                    "01001110000101001100001100100000000010001000101000110111011011101010001010101"
                    "00101000000010001101100000100100101111101101100001000110010001000001110101100"
                    "00101011001011101110001110110100101110011010101001100010001000101111101001101"
                    "01001001001010100101011000011111010110010100010001001111010001010100011101011"
                    "10000011000110110100001010110011010110001000001000111110011001101100001100111"
                    "01110001010000100101100010011001111010101100100111001110010011000101110011010"
                    "01001111000111001110001010010101001111001010000001100110101000101000001000101"
                    "00001000011010100001101010101111000101000100110011110111010111010110111001101"
                    "01111011000000110000000111111010100010001001001111100110011111101100111110001"
                    "00000000110100100000011010001001100011010101111000100100010001100001100011000"
                    "11111110010011010010010010101100111010111011111010101010001001000111101010000"
                    "10000010001110100101010010001111010101011101101000110100010101010101100011001"
                    "10111010101010100101111011111010110100001001101111101111111011101100111110101"
                    "10111010001000111010010011011001110111011100100111100100110011001000110010010"
                    "10111010111111010100110001101100110100111010111110001010001010100101101010001"
                    "10000010001110111011101001101101010101011101110100110101010001000101100000101"
                    "11111110101101000011101011000010100010000010001101101110011110100111001110011"
                },
        /*  2*/ { UNICODE_MODE, -1, -1, -1, "UPNQR\012\012\012\012\012Janez Novak\012Dunajska 1\0121000 Ljubljana\01200000008105\012\012\012RENT\012Plačilo najemnine 10/2016\01215.11.2016\012SI56051008010486080\012RF45SBO2010\012Novo podjetje d.o.o.\012Lepa cesta 15\0123698 Loški Potok\012188\012                                                                                                                                                                                                                ", 0, 77, 77, "Example H",
                    "11111110000111000010011001010010001001000000101010111011100111100000001111111"
                    "10000010110011101110001010110101010101001111000101010101010110110010101000001"
                    "10111010001010100101010100101010101011101011101110101010101010001000101011101"
                    "10111010001111111111100001111111011101110011010110010001000100011100101011101"
                    "10111010100111000110000111111010001000100110001111111011100100011011101011101"
                    "10000010010011000001001010001101010100010110011000110101010100010110001000001"
                    "11111110101010101010101010101010101010101010101010101010101010101010101111111"
                    "00000000000101011111001110001101001100110001011000110101011101110101000000000"
                    "10101010001001000110011011111000100010001000111111101110111011100110100010010"
                    "00001100101001011111011101001100101111111001110011000110011001000101011101010"
                    "00011111001101000001010101100011101010101110101110101010101010101011101010001"
                    "10111100010101101000101100011101010101010101010011010111010101000101010101011"
                    "11110110011001000001011000011000010011001100111011100110111011111110101000101"
                    "00000101111001011011111011111100110111011111110001011110010001010101011101010"
                    "00101111101101100000110101110010111010101010101110101010101010101011101010001"
                    "10101100011100001000001110011011110110010101010011010111010101010101010101011"
                    "11100011111101000000011100001000000100001000111011000100111011101110001000101"
                    "00000001100010011010111011110011110110111101110001001110010001000101111101010"
                    "00011011110101100001100001101011101011001010101110101100001010101011101100001"
                    "10111000001000101001001100000101010101010101010101110101010101010101110101011"
                    "11110011110001000000011110010000110010001000100010100101011011101000111100101"
                    "00001101101111111010100111101100110011011101110000000000010001000111100001010"
                    "00101011111011100001110111110010111010101010100001101111001010101111101000101"
                    "00100000011100100001010010001100010011011101010111000110110101010101110111001"
                    "11011111111011011000011011111000101100010000111111111111111011101100111111101"
                    "10111000111011100010101010001100011111011101111000110100110001000001100010000"
                    "11111010110001100001110010101010100110101010101010101010001010101100101010010"
                    "10001000100100111011011010001101010111000100011000101101010101010101100011111"
                    "10111111101001001100001011111000100000001000111111100110011100101111111110011"
                    "00001000001011100010110011101101110101001100100100001100010000000110000100010"
                    "10011110010011100001111100000010101010101010111110001010101001101011100010010"
                    "00011000000000110011010011001101010001000101100001010101010101010101001110110"
                    "00011111101101000011000110010000100010001000001010001111011001101111101100011"
                    "01001000001101101110101010001101110001011101101100100100010001100110000111010"
                    "11001010110100110110011100000010101010111011111011101010101010101000100111011"
                    "00000100100010011010001011101101010101001000011100110100010001000111000111110"
                    "10000011101010100100101111110110100010000001111110001111010111111110011010111"
                    "10001101101110000101111010001101110111011110100100100100100101110111000110010"
                    "11101011010101110101110100000110101010111000111011101010110010101010111110001"
                    "11100101100000011111101010101001010101010001011100110101010101100111010001011"
                    "10000011001011000111001110010010100011101111100110001110111001101110000111001"
                    "01101101101011000010010010001111110111010100001100000100011001111101000001010"
                    "01101010110111110110001100100100101011101011101010101010101011101010110110001"
                    "01100101001000000100111010001111010101010101111101010101010001000101000101011"
                    "01000011101011000011101111010000100010001101001110101000111111100110001010101"
                    "10101101011010110110001011101111110111011101110100000000011111100101010111010"
                    "01101111110110011011100111111000101010101011101111101100100011001010111110001"
                    "00101000101001000110011010001100010100010101011000110101011101000101100011011"
                    "01001010111011000110001010101011111010001000111010101110101011111110101010101"
                    "10101000111010011110101010001011110111111101111000100110000001010101100011010"
                    "01101111101111000101100111111001111011101010101111101010111010101010111110001"
                    "00100100101000010100111000001011010101010101010101010101010101000100011101011"
                    "01001010001010100101001100011101110011001100101010101110100011111110101010101"
                    "10101101101011111101001111100100110111011111100001000000000001010101010101010"
                    "01011010111111000110000011011110101010101010101011101100100010101010101000001"
                    "00101100100010010101011100111000000011010101010101010101010101010100011101011"
                    "01110011011111000100001011010100100100001000101010101100111011101110001010101"
                    "10001000011001111101101110010011101110111101100001000000010001000101010101000"
                    "01001011100001000110010100100001100110001010101011101100001010101010101000001"
                    "00000001110000110101010011011111010101011101010111000110110101010100111101011"
                    "01100110011100100100001110110010110000000000101000100101111011101010101010111"
                    "00111000010110111101100100010000110011010101100011000000110001000000110101111"
                    "00011111001001000110001000010100101010101010101011100010101010101110101001111"
                    "01101001011110111101001110111111000101010101010111011110110101010101111101001"
                    "10100011000100110100010100111011001100011000101010100111111011101100101010101"
                    "01011001011000101101111010000001101111011101101011000101110001000101010100000"
                    "01001111011111001110000110001101001110111010100011000010101000101010101001011"
                    "00001000001100110001010010101111010001000101111010001100110101010110011100011"
                    "01111011001010100000001111111010100010011000011111111111111100101110111111111"
                    "00000000101000101001111010001001110011001101011000101100010000000001100010010"
                    "11111110011001001010011010101100101010101010111010101010101011101101101010001"
                    "10000010001010100001001010001111010101010101011000110101010101010111100011111"
                    "10111010101110100001000011111010100100010001101111101110011001101001111111101"
                    "10111010001110110111100011010001110111011100101110000100010001100111110001010"
                    "10111010111011000101100001101100101100111011100110001010001010100111101111001"
                    "10000010001010100100100001101101010101001000000101010101110011011111100111001"
                    "11111110101101000110111011000010100010001100001111101110110001100111001110111"
                },
        /*  3*/ { UNICODE_MODE, -1, -1, 2 << 8, "UPNQR\012\012\012\012\012Janez Novak\012Dunajska 1\0121000 Ljubljana\01200000008105\012\012\012RENT\012Plačilo najemnine 10/2016\01215.11.2016\012SI56051008010486080\012RF45SBO2010\012Novo podjetje d.o.o.\012Lepa cesta 15\0123698 Loški Potok\012188\012                                                                                                                                                                                                                ", 0, 77, 77, "Example H with explicit mask 001 (auto-mask 000)",
                    "11111110110010010111001100000111011100010101111111101110110010110100001111111"
                    "10000010000110111011011111100000000000011010010000000000000011100110101000001"
                    "10111010111111110000000001111111111110111110111011111111111111011100101011101"
                    "10111010011010101010110100101010001000100110000011000100010001001000101011101"
                    "10111010010010010011010011111111011101110011011111101110110001001111101011101"
                    "10000010100110010100011110001000000001000011001000100000000001000010001000001"
                    "11111110101010101010101010101010101010101010101010101010101010101010101111111"
                    "00000000010000001010011010001000011001100100001000100000001000100000000000000"
                    "10100011011100010011001111111101110111011101101111111011101110110011100100101"
                    "01011001111100001010001000011001111010101100100110010011001100010000001000000"
                    "01001010011000010100000000110110111111111011111011111111111111111110111111011"
                    "11101001000000111101111001001000000000000000000110000010000000010000000000001"
                    "10100011001100010100001101001101000110011001101110110011101110101011111101111"
                    "01010000101100001110101110101001100010001010100100001011000100000000001000000"
                    "01111010111000110101100000100111101111111111111011111111111111111110111111011"
                    "11111001001001011101011011001110100011000000000110000010000000000000000000001"
                    "10110110101000010101001001011101010001011101101110010001101110111011011101111"
                    "01010100110111001111101110100110100011101000100100011011000100010000101000000"
                    "01001110100000110100110100111110111110011111111011111001011111111110111001011"
                    "11101101011101111100011001010000000000000000000000100000000000000000100000001"
                    "10100110100100010101001011000101100111011101110111110000001110111101101001111"
                    "01011000111010101111110010111001100110001000100101010101000100010010110100000"
                    "01111110101110110100100010100111101111111111110100111010011111111010111101111"
                    "01110101001001110100000111011001000110001000000010010011100000000000100010011"
                    "10001111101110001101001111111101111001000101101111101010101110111001111110111"
                    "11101000101110110111111110001001001010001000101000100001100100010100100011010"
                    "10101010100100110100100110101111110011111111111010111111011111111001101011000"
                    "11011000110001101110001110001000000010010001001000111000000000000000100010101"
                    "11101111111100011001011111111101110101011101101111110011001001111010111111001"
                    "01011101011110110111100110111000100000011001110001011001000101010011010001000"
                    "11001011000110110100101001010111111111111111101011011111111100111110110111000"
                    "01001101010101100110000110011000000100010000110100000000000000000000011011100"
                    "01001010111000010110010011000101110111011101011111011010001100111010111001001"
                    "00011101011000111011111111011000100100001000111001110001000100110011010010000"
                    "10011111100001100011001001010111111111101110101110111111111111111101110010001"
                    "01010001110111001111011110111000000000011101001001100001000100010010010010100"
                    "11010110111111110001111010100011110111010100101011011010000010101011001111101"
                    "11011000111011010000101111011000100010001011110001110001110000100010010011000"
                    "10111110000000100000100001010011111111101101101110111111100111111111101011011"
                    "10110000110101001010111111111100000000000100001001100000000000110010000100001"
                    "11010110011110010010011011000111110110111010110011011011101100111011010010011"
                    "00111000111110010111000111011010100010000001011001010001001100101000010100000"
                    "00111111100010100011011001110001111110111110111111111111111110111111100011011"
                    "00110000011101010001101111011010000000000000101000000000000100010000010000001"
                    "00010110111110010110111010000101110111011000011011111101101010110011011111111"
                    "11111000001111100011011110111010100010001000100001010101001010110000000010000"
                    "00111111100011001110110011111101111111111110111111111001110110011111111111011"
                    "01111000111100010011001110001001000001000000001000100000001000010000100010001"
                    "00011010101110010011011110101110101111011101101010111011111110101011101011111"
                    "11111000101111001011111110001110100010101000101000110011010100000000100010000"
                    "00111111111010010000110011111100101110111111111111111111101111111111111111011"
                    "01110001111101000001101101011110000000000000000000000000000000010001001000001"
                    "00011111011111110000011001001000100110011001111111111011110110101011111111111"
                    "11111000111110101000011010110001100010001010110100010101010100000000000000000"
                    "00001111101010010011010110001011111111111111111110111001110111111111111101011"
                    "01111001110111000000001001101101010110000000000000000000000000000001001000001"
                    "00100110001010010001011110000001110001011101111111111001101110111011011111111"
                    "11011101001100101000111011000110111011101000110100010101000100010000000000010"
                    "00011110110100010011000001110100110011011111111110111001011111111111111101011"
                    "01010100100101100000000110001010000000001000000010010011100000000001101000001"
                    "00110011001001110001011011100111100101010101111101110000101110111111111111101"
                    "01101101000011101000110001000101100110000000110110010101100100010101100000101"
                    "01001010011100010011011101000001111111111111111110110111111111111011111100101"
                    "00111100001011101000011011101010010000000000000010001011100000000000101000011"
                    "11110110010001100001000001101110011001001101111111110010101110111001111111111"
                    "00001100001101111000101111010100111010001000111110010000100100010000000001010"
                    "01001110001010011011010011011000011011101111110110010111111101111111111100001"
                    "00001001011001100100000111111010000100010000101111011001100000000011001001001"
                    "01111010011111110101011011111111110111001101001111101010101001111011111110101"
                    "00000000111101111100101110001100100110011000001000111001000101010100100011000"
                    "11111110101100011111001110101001111111111111101010111111111110111000101011011"
                    "10000010011111110100011110001010000000000000001000100000000000000010100010101"
                    "10111010011011110100010111111111110001000100111111111011001100111100111110111"
                    "10111010011011100010110110000100100010001001111011010001000100110010100100000"
                    "10111010101110010000110100111001111001101110110011011111011111110010111010011"
                    "10000010011111110001110100111000000000011101010000000000100110001010110010011"
                    "11111110111000010011101110010111110111011001011010111011100100110010011011101"
                },
        /*  4*/ { UNICODE_MODE, -1, -1, -1, "Ą˘Ł¤ĽŚ§¨ŠŞŤŹŽŻ°ą˛ł´ľśˇ¸šşťź˝žżŔÁÂĂÄĹĆÇČÉĘËĚÍÎĎĐŃŇÓÔŐÖ×ŘŮÚŰÜÝŢßŕáâăäĺćçčéęëěíîďđńňóôőö÷řůúűüýţ˙", 0, 77, 77, "ISO 8859-2",
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
        /*  5*/ { UNICODE_MODE, -1, -1, -1, "UPNQR\012\012\012\012\012Janez Novak\012Dunajska 1\0121000 Ljubljana\01200000008105\012\012\012COST\012Plačilo obveznosti 10/2016\012\012SI56051008010486080\012SI0598765432100\012Novo podjetje d.o.o.\012Lepa cesta 15\0123698 Loški Potok\012183\012                                                                                                                                                                                                                     ", 0, 77, 77, "Example A fast automatic mask 0 (not 3)",
                    "11111110111111111100010011100010110001111000010010000011011111010000001111111"
                    "10000010100011011111111001110100100100111110110100100100100111100010101000001"
                    "10111010001000100000110111010001001000001000001101001001001001011000101011101"
                    "10111010101000111111100101110000011010110100010001010110010010111000101011101"
                    "10111010001000001001001011111100000110101100001111110101110011110111101011101"
                    "10000010001111111001010010001001001000001000001000101001011001101010001000001"
                    "11111110101010101010101010101010101010101010101010101010101010101010101111111"
                    "00000000110100001001111110001101011101000100101000100100100100100100100000000"
                    "10110111001011100111111111111011000001101011011111101101010000000101001001011"
                    "11001001101111111100011011010011101000111100110110000001000110000010011010010"
                    "10010011010011001101011011110101100100100110100110100100111100100101100100000"
                    "10100000010000010100110110001000001001001001001101011011000001011001001001000"
                    "11000110111001011001100000101001001011110000000011000110001011000111110000010"
                    "01100000001000001010001010000101000110101100000100100111110000100100001100100"
                    "11011010001110100011010100011001011001001001001001001011001001001000001001101"
                    "01100101000011101111001001011100110111010010010110011000011010010010010010011"
                    "01011011110011001110010110000111101100100110110101001010110101100001100110100"
                    "00000101101101100110100111101110101010100001101001010000001101011001100001011"
                    "00000011000011011001011001010011010110010010010000011000110010010011010110110"
                    "11100101110001011000111101110100110110100100100000001101000100100100100000101"
                    "00011010011000100011111101110011011011101011000011000110011000001011011111011"
                    "11101000111110111101100000101011110010011010110111000001110110000000000010010"
                    "10001011110111101111111001111100111100100100101101100110000100100001100000101"
                    "00100000001000110101001110010000001111000001001011011000101001001001101101010"
                    "11111111110101101000100011111000011100111000001111100111000011010100111111010"
                    "11101000100100011011011010001101100110100100001000100101000000110000100011101"
                    "00111010110010001010010010101001001101001001001010101001101011001111101011100"
                    "01101000100011111100011110001010010000011010001000101010010110010010100010101"
                    "10001111100101011010000111111110101110010110111111101001110000100001111110010"
                    "00011000010101110110101111110001101001010001110000010001101110011010011110001"
                    "01101010101101011001000100111010010010010010010110110010010011010011011010101"
                    "01100101110001010010111010111100100000110101011000100100100010100110111011000"
                    "10111010001110111000010101110011000001101010100001101100111010001100001001111"
                    "01001001001010111000000101001010110110011010110011100011010100100001000100010"
                    "00000110111010100100111010001100100100110101110101100100100100100100111001010"
                    "01011001111110011000100111110001001001010100000000001000001111001011011010001"
                    "00111011010010000010110111001110011010111000000110010111101001010110110011100"
                    "01111100011111110001111011111100000110101111000101110101010000010010100110100"
                    "01001010110110000011110111100101001001011011001001101001010001001001010101101"
                    "00100000000111011011011101101110010010010110010010010010010010010000000110011"
                    "10001110000101001100110001011100101101100001101000000000110110001000000000100"
                    "00110001010111011000100111010011101011001000000000011000000100011001001101001"
                    "01010010001111001110110101011100010011010011010010010010010011111010010110110"
                    "11010101111000010110001010111000100100100100001100100100100000111100101100101"
                    "10100010001001000010001100110101000001101110101101001011001100111101100001001"
                    "01101001011100010101001101101110110000011110110011000111000001000010010000010"
                    "11101111111001110000001011111010100100100101101111100010110101000100111110000"
                    "00111000110101111110000010001100001000001101001000101001001001111001100011000"
                    "01111010100010111010110010101011000010110000001010110110000011000110101010010"
                    "11011000101011001011011010001100000110001010001000110111100000100100100010100"
                    "10001111111100100100000111111010011000001111001111100001001001001001111111101"
                    "11100001001110110011111000111010010010010010010010000010010010000011011010011"
                    "11000111010101101011000010000110111101000110100100110000110101110000100100100"
                    "10110001010110000001010000001010101011100001111101010010001101001001001001001"
                    "01010011111110111110111010111100010010010010010011011100010010010010010000110"
                    "01111101011001100100101010011110010101100100100100101100100100100101101100101"
                    "10010010101001000111101010111011000001101011001001001111011000001101101001001"
                    "01011101011000111010101001000010101101011010100110001111010110000011010010010"
                    "11110111111011001000011101001001111001100100100101100000100100100100000110010"
                    "00101101100110101001001100001011011001001001001001001001001001001000000001010"
                    "01001110101000011100110000011010010010110000010100011100100011010010110010000"
                    "01011001110011001100010011101001001010101100010000110000000000110001100100010"
                    "11011110101100100101101111110111011001001001001010000111001001001101101010000"
                    "10110000001001111010001001110000000010011010010000011000010010010011011010001"
                    "00011011010000111010011010100101001010011110100100101000010101100010000100100"
                    "01100101010110100001100110001101101011000001110111011000101101011001001000011"
                    "01001111100111100110111110110101101110010010011011111010110000010011010001100"
                    "00001001111101000000101011011110100000100100101101111101100100100111001101101"
                    "01111010101101000011101111111001000001100011011111111101111111001101111110011"
                    "00000000101101101110101110001110110100001010111000101010110111000000100011010"
                    "11111110110001001100010110101010100100100100111010100100100011100101101010000"
                    "10000010110110110101000110001011001001001001101000101001001111001001100010100"
                    "10111010010110001000101011111010011100101001101111110110100111010111111111110"
                    "10111010111111011111010010100000000110101100000111110101100010010110000000100"
                    "10111010111000101010000010001111001111011001010101101001101011001000001000101"
                    "10000010001101100100010110101010010010001110010010010010110010010010100101001"
                    "11111110100011011001111101001100101100000010111001100000111111111001000100010"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    #if 0 /* Need to add "force binary mode" to BWIPP for this to work */
    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    #endif
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_upnqr_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_UPNQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    testUtilOption3Name(BARCODE_UPNQR, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);
            }

            if (ret < ZINT_ERROR) {
                #if 0
                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    char modules_dump[77 * 77 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                #endif
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[77 * 77 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_1;
        int option_2;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
        const char *comment;
    };
    /* ISO/IEC 23941:2022 Table 6 */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 2, 1, "1", 12, 0, 7, 43, "", "" },
        /*  1*/ { 2, 1, "1", 13, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 1 R7x43-M, requires 7 codewords (maximum 6)", "" },
        /*  2*/ { 2, 1, "A", 7, 0, 7, 43, "", "" },
        /*  3*/ { 2, 1, "A", 8, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 1 R7x43-M, requires 7 codewords (maximum 6)", "" },
        /*  4*/ { 2, 1, "\200", 5, 0, 7, 43, "", "" },
        /*  5*/ { 2, 1, "\200", 6, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 1 R7x43-M, requires 7 codewords (maximum 6)", "" },
        /*  6*/ { 2, 1, "\201", 6, 0, 7, 43, "", "3 Shift JIS 0x8181" },
        /*  7*/ { 2, 1, "\201", 8, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 1 R7x43-M, requires 8 codewords (maximum 6)", "4 Shift JIS 0x8181" },
        /*  8*/ { 4, 1, "1", 5, 0, 7, 43, "", "" },
        /*  9*/ { 4, 1, "1", 6, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 1 R7x43-H, requires 4 codewords (maximum 3)", "" },
        /* 10*/ { 4, 1, "A", 3, 0, 7, 43, "", "" },
        /* 11*/ { 4, 1, "A", 4, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 1 R7x43-H, requires 4 codewords (maximum 3)", "" },
        /* 12*/ { 4, 1, "\200", 2, 0, 7, 43, "", "" },
        /* 13*/ { 4, 1, "\200", 3, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 1 R7x43-H, requires 4 codewords (maximum 3)", "" },
        /* 14*/ { 4, 1, "\201", 2, 0, 7, 43, "", "1 Shift JIS 0x8181" },
        /* 15*/ { 4, 1, "\201", 4, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 1 R7x43-H, requires 4 codewords (maximum 3)", "2 Shift JIS 0x8181" },
        /* 16*/ { 2, 2, "1", 26, 0, 7, 59, "", "" },
        /* 17*/ { 2, 2, "1", 27, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 2 R7x59-M, requires 13 codewords (maximum 12)", "" },
        /* 18*/ { 2, 2, "A", 16, 0, 7, 59, "", "" },
        /* 19*/ { 2, 2, "A", 17, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 2 R7x59-M, requires 13 codewords (maximum 12)", "" },
        /* 20*/ { 2, 2, "\200", 11, 0, 7, 59, "", "" },
        /* 21*/ { 2, 2, "\200", 12, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 2 R7x59-M, requires 13 codewords (maximum 12)", "" },
        /* 22*/ { 2, 2, "\201", 12, 0, 7, 59, "", "6 Shift JIS 0x8181" },
        /* 23*/ { 2, 2, "\201", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 2 R7x59-M, requires 13 codewords (maximum 12)", "7 Shift JIS 0x8181" },
        /* 24*/ { 4, 2, "1", 14, 0, 7, 59, "", "" },
        /* 25*/ { 4, 2, "1", 15, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 2 R7x59-H, requires 8 codewords (maximum 7)", "" },
        /* 26*/ { 4, 2, "A", 8, 0, 7, 59, "", "" },
        /* 27*/ { 4, 2, "A", 9, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 2 R7x59-H, requires 8 codewords (maximum 7)", "" },
        /* 28*/ { 4, 2, "\200", 6, 0, 7, 59, "", "" },
        /* 29*/ { 4, 2, "\200", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 2 R7x59-H, requires 8 codewords (maximum 7)", "" },
        /* 30*/ { 4, 2, "\201", 6, 0, 7, 59, "", "3 Shift JIS 0x8181" },
        /* 31*/ { 4, 2, "\201", 8, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 2 R7x59-H, requires 8 codewords (maximum 7)", "4 Shift JIS 0x8181" },
        /* 32*/ { 2, 3, "1", 45, 0, 7, 77, "", "" },
        /* 33*/ { 2, 3, "1", 46, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 3 R7x77-M, requires 21 codewords (maximum 20)", "" },
        /* 34*/ { 2, 3, "A", 27, 0, 7, 77, "", "" },
        /* 35*/ { 2, 3, "A", 28, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 3 R7x77-M, requires 21 codewords (maximum 20)", "" },
        /* 36*/ { 2, 3, "\200", 19, 0, 7, 77, "", "" },
        /* 37*/ { 2, 3, "\200", 20, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 3 R7x77-M, requires 21 codewords (maximum 20)", "" },
        /* 38*/ { 2, 3, "\201", 22, 0, 7, 77, "", "11 Shift JIS 0x8181" },
        /* 39*/ { 2, 3, "\201", 24, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 3 R7x77-M, requires 21 codewords (maximum 20)", "12 Shift JIS 0x8181" },
        /* 40*/ { 4, 3, "1", 21, 0, 7, 77, "", "" },
        /* 41*/ { 4, 3, "1", 22, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 3 R7x77-H, requires 11 codewords (maximum 10)", "" },
        /* 42*/ { 4, 3, "A", 13, 0, 7, 77, "", "" },
        /* 43*/ { 4, 3, "A", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 3 R7x77-H, requires 11 codewords (maximum 10)", "" },
        /* 44*/ { 4, 3, "\200", 9, 0, 7, 77, "", "" },
        /* 45*/ { 4, 3, "\200", 10, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 3 R7x77-H, requires 11 codewords (maximum 10)", "" },
        /* 46*/ { 4, 3, "\201", 10, 0, 7, 77, "", "5 Shift JIS 0x8181" },
        /* 47*/ { 4, 3, "\201", 12, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 3 R7x77-H, requires 11 codewords (maximum 10)", "6 Shift JIS 0x8181" },
        /* 48*/ { 2, 4, "1", 64, 0, 7, 99, "", "" },
        /* 49*/ { 2, 4, "1", 65, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 4 R7x99-M, requires 29 codewords (maximum 28)", "" },
        /* 50*/ { 2, 4, "A", 39, 0, 7, 99, "", "" },
        /* 51*/ { 2, 4, "A", 40, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 4 R7x99-M, requires 29 codewords (maximum 28)", "" },
        /* 52*/ { 2, 4, "\200", 27, 0, 7, 99, "", "" },
        /* 53*/ { 2, 4, "\200", 28, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 4 R7x99-M, requires 29 codewords (maximum 28)", "" },
        /* 54*/ { 2, 4, "\201", 32, 0, 7, 99, "", "16 Shift JIS 0x8181" },
        /* 55*/ { 2, 4, "\201", 34, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 4 R7x99-M, requires 29 codewords (maximum 28)", "17 Shift JIS 0x8181" },
        /* 56*/ { 4, 4, "1", 30, 0, 7, 99, "", "" },
        /* 57*/ { 4, 4, "1", 31, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 4 R7x99-H, requires 15 codewords (maximum 14)", "" },
        /* 58*/ { 4, 4, "A", 18, 0, 7, 99, "", "" },
        /* 59*/ { 4, 4, "A", 19, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 4 R7x99-H, requires 15 codewords (maximum 14)", "" },
        /* 60*/ { 4, 4, "\200", 13, 0, 7, 99, "", "" },
        /* 61*/ { 4, 4, "\200", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 4 R7x99-H, requires 15 codewords (maximum 14)", "" },
        /* 62*/ { 4, 4, "\201", 16, 0, 7, 99, "", "8 Shift JIS 0x8181" },
        /* 63*/ { 4, 4, "\201", 18, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 4 R7x99-H, requires 16 codewords (maximum 14)", "9 Shift JIS 0x8181" },
        /* 64*/ { 2, 5, "1", 102, 0, 7, 139, "", "" },
        /* 65*/ { 2, 5, "1", 103, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 5 R7x139-M, requires 45 codewords (maximum 44)", "" },
        /* 66*/ { 2, 5, "A", 62, 0, 7, 139, "", "" },
        /* 67*/ { 2, 5, "A", 63, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 5 R7x139-M, requires 45 codewords (maximum 44)", "" },
        /* 68*/ { 2, 5, "\200", 42, 0, 7, 139, "", "" },
        /* 69*/ { 2, 5, "\200", 43, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 5 R7x139-M, requires 45 codewords (maximum 44)", "" },
        /* 70*/ { 2, 5, "\201", 52, 0, 7, 139, "", "26 Shift JIS 0x8181" },
        /* 71*/ { 2, 5, "\201", 54, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 5 R7x139-M, requires 45 codewords (maximum 44)", "27 Shift JIS 0x8181" },
        /* 72*/ { 4, 5, "1", 54, 0, 7, 139, "", "" },
        /* 73*/ { 4, 5, "1", 55, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 5 R7x139-H, requires 25 codewords (maximum 24)", "" },
        /* 74*/ { 4, 5, "A", 33, 0, 7, 139, "", "" },
        /* 75*/ { 4, 5, "A", 34, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 5 R7x139-H, requires 25 codewords (maximum 24)", "" },
        /* 76*/ { 4, 5, "\200", 22, 0, 7, 139, "", "" },
        /* 77*/ { 4, 5, "\200", 23, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 5 R7x139-H, requires 25 codewords (maximum 24)", "" },
        /* 78*/ { 4, 5, "\201", 28, 0, 7, 139, "", "14 Shift JIS 0x8181" },
        /* 79*/ { 4, 5, "\201", 30, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 5 R7x139-H, requires 26 codewords (maximum 24)", "15 Shift JIS 0x8181" },
        /* 80*/ { 2, 6, "1", 26, 0, 9, 43, "", "" },
        /* 81*/ { 2, 6, "1", 27, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 6 R9x43-M, requires 13 codewords (maximum 12)", "" },
        /* 82*/ { 2, 6, "A", 16, 0, 9, 43, "", "" },
        /* 83*/ { 2, 6, "A", 17, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 6 R9x43-M, requires 13 codewords (maximum 12)", "" },
        /* 84*/ { 2, 6, "\200", 11, 0, 9, 43, "", "" },
        /* 85*/ { 2, 6, "\200", 12, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 6 R9x43-M, requires 13 codewords (maximum 12)", "" },
        /* 86*/ { 2, 6, "\201", 12, 0, 9, 43, "", "6 Shift JIS 0x8181" },
        /* 87*/ { 2, 6, "\201", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 6 R9x43-M, requires 13 codewords (maximum 12)", "7 Shift JIS 0x8181" },
        /* 88*/ { 4, 6, "1", 14, 0, 9, 43, "", "" },
        /* 89*/ { 4, 6, "1", 15, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 6 R9x43-H, requires 8 codewords (maximum 7)", "" },
        /* 90*/ { 4, 6, "A", 8, 0, 9, 43, "", "" },
        /* 91*/ { 4, 6, "A", 9, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 6 R9x43-H, requires 8 codewords (maximum 7)", "" },
        /* 92*/ { 4, 6, "\200", 6, 0, 9, 43, "", "" },
        /* 93*/ { 4, 6, "\200", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 6 R9x43-H, requires 8 codewords (maximum 7)", "" },
        /* 94*/ { 4, 6, "\201", 6, 0, 9, 43, "", "3 Shift JIS 0x8181" },
        /* 95*/ { 4, 6, "\201", 8, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 6 R9x43-H, requires 8 codewords (maximum 7)", "4 Shift JIS 0x8181" },
        /* 96*/ { 2, 7, "1", 47, 0, 9, 59, "", "" },
        /* 97*/ { 2, 7, "1", 48, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 7 R9x59-M, requires 22 codewords (maximum 21)", "" },
        /* 98*/ { 2, 7, "A", 29, 0, 9, 59, "", "" },
        /* 99*/ { 2, 7, "A", 30, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 7 R9x59-M, requires 22 codewords (maximum 21)", "" },
        /*100*/ { 2, 7, "\200", 20, 0, 9, 59, "", "" },
        /*101*/ { 2, 7, "\200", 21, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 7 R9x59-M, requires 22 codewords (maximum 21)", "" },
        /*102*/ { 2, 7, "\201", 24, 0, 9, 59, "", "12 Shift JIS 0x8181" },
        /*103*/ { 2, 7, "\201", 26, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 7 R9x59-M, requires 22 codewords (maximum 21)", "13 Shift JIS 0x8181" },
        /*104*/ { 4, 7, "1", 23, 0, 9, 59, "", "" },
        /*105*/ { 4, 7, "1", 24, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 7 R9x59-H, requires 12 codewords (maximum 11)", "" },
        /*106*/ { 4, 7, "A", 14, 0, 9, 59, "", "" },
        /*107*/ { 4, 7, "A", 15, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 7 R9x59-H, requires 12 codewords (maximum 11)", "" },
        /*108*/ { 4, 7, "\200", 10, 0, 9, 59, "", "" },
        /*109*/ { 4, 7, "\200", 11, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 7 R9x59-H, requires 12 codewords (maximum 11)", "" },
        /*110*/ { 4, 7, "\201", 12, 0, 9, 59, "", "6 Shift JIS 0x8181" },
        /*111*/ { 4, 7, "\201", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 7 R9x59-H, requires 13 codewords (maximum 11)", "7 Shift JIS 0x8181" },
        /*112*/ { 2, 8, "1", 71, 0, 9, 77, "", "" },
        /*113*/ { 2, 8, "1", 72, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 8 R9x77-M, requires 32 codewords (maximum 31)", "" },
        /*114*/ { 2, 8, "A", 43, 0, 9, 77, "", "" },
        /*115*/ { 2, 8, "A", 44, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 8 R9x77-M, requires 32 codewords (maximum 31)", "" },
        /*116*/ { 2, 8, "\200", 30, 0, 9, 77, "", "" },
        /*117*/ { 2, 8, "\200", 31, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 8 R9x77-M, requires 32 codewords (maximum 31)", "" },
        /*118*/ { 2, 8, "\201", 36, 0, 9, 77, "", "18 Shift JIS 0x8181" },
        /*119*/ { 2, 8, "\201", 38, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 8 R9x77-M, requires 32 codewords (maximum 31)", "19 Shift JIS 0x8181" },
        /*120*/ { 4, 8, "1", 37, 0, 9, 77, "", "" },
        /*121*/ { 4, 8, "1", 38, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 8 R9x77-H, requires 18 codewords (maximum 17)", "" },
        /*122*/ { 4, 8, "A", 23, 0, 9, 77, "", "" },
        /*123*/ { 4, 8, "A", 24, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 8 R9x77-H, requires 18 codewords (maximum 17)", "" },
        /*124*/ { 4, 8, "\200", 16, 0, 9, 77, "", "" },
        /*125*/ { 4, 8, "\200", 17, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 8 R9x77-H, requires 18 codewords (maximum 17)", "" },
        /*126*/ { 4, 8, "\201", 18, 0, 9, 77, "", "9 Shift JIS 0x8181" },
        /*127*/ { 4, 8, "\201", 20, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 8 R9x77-H, requires 18 codewords (maximum 17)", "10 Shift JIS 0x8181" },
        /*128*/ { 2, 9, "1", 97, 0, 9, 99, "", "" },
        /*129*/ { 2, 9, "1", 98, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 9 R9x99-M, requires 43 codewords (maximum 42)", "" },
        /*130*/ { 2, 9, "A", 59, 0, 9, 99, "", "" },
        /*131*/ { 2, 9, "A", 60, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 9 R9x99-M, requires 43 codewords (maximum 42)", "" },
        /*132*/ { 2, 9, "\200", 40, 0, 9, 99, "", "" },
        /*133*/ { 2, 9, "\200", 41, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 9 R9x99-M, requires 43 codewords (maximum 42)", "" },
        /*134*/ { 2, 9, "\201", 50, 0, 9, 99, "", "25 Shift JIS 0x8181" },
        /*135*/ { 2, 9, "\201", 52, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 9 R9x99-M, requires 44 codewords (maximum 42)", "26 Shift JIS 0x8181" },
        /*136*/ { 4, 9, "1", 49, 0, 9, 99, "", "" },
        /*137*/ { 4, 9, "1", 50, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 9 R9x99-H, requires 23 codewords (maximum 22)", "" },
        /*138*/ { 4, 9, "A", 30, 0, 9, 99, "", "" },
        /*139*/ { 4, 9, "A", 31, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 9 R9x99-H, requires 23 codewords (maximum 22)", "" },
        /*140*/ { 4, 9, "\200", 20, 0, 9, 99, "", "" },
        /*141*/ { 4, 9, "\200", 21, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 9 R9x99-H, requires 23 codewords (maximum 22)", "" },
        /*142*/ { 4, 9, "\201", 24, 0, 9, 99, "", "12 Shift JIS 0x8181" },
        /*143*/ { 4, 9, "\201", 26, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 9 R9x99-H, requires 23 codewords (maximum 22)", "13 Shift JIS 0x8181" },
        /*144*/ { 2, 10, "1", 147, 0, 9, 139, "", "" },
        /*145*/ { 2, 10, "1", 148, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 10 R9x139-M, requires 64 codewords (maximum 63)", "" },
        /*146*/ { 2, 10, "A", 89, 0, 9, 139, "", "" },
        /*147*/ { 2, 10, "A", 90, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 10 R9x139-M, requires 64 codewords (maximum 63)", "" },
        /*148*/ { 2, 10, "\200", 61, 0, 9, 139, "", "" },
        /*149*/ { 2, 10, "\200", 62, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 10 R9x139-M, requires 64 codewords (maximum 63)", "" },
        /*150*/ { 2, 10, "\201", 76, 0, 9, 139, "", "36 Shift JIS 0x8181" },
        /*151*/ { 2, 10, "\201", 78, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 10 R9x139-M, requires 65 codewords (maximum 63)", "37 Shift JIS 0x8181" },
        /*152*/ { 4, 10, "1", 75, 0, 9, 139, "", "" },
        /*153*/ { 4, 10, "1", 76, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 10 R9x139-H, requires 34 codewords (maximum 33)", "" },
        /*154*/ { 4, 10, "A", 46, 0, 9, 139, "", "" },
        /*155*/ { 4, 10, "A", 47, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 10 R9x139-H, requires 34 codewords (maximum 33)", "" },
        /*156*/ { 4, 10, "\200", 31, 0, 9, 139, "", "" },
        /*157*/ { 4, 10, "\200", 32, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 10 R9x139-H, requires 34 codewords (maximum 33)", "" },
        /*158*/ { 4, 10, "\201", 38, 0, 9, 139, "", "19 Shift JIS 0x8181" },
        /*159*/ { 4, 10, "\201", 40, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 10 R9x139-H, requires 34 codewords (maximum 33)", "20 Shift JIS 0x8181" },
        /*160*/ { 2, 11, "1", 14, 0, 11, 27, "", "" },
        /*161*/ { 2, 11, "1", 15, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 11 R11x27-M, requires 8 codewords (maximum 7)", "" },
        /*162*/ { 2, 11, "A", 8, 0, 11, 27, "", "" },
        /*163*/ { 2, 11, "A", 9, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 11 R11x27-M, requires 8 codewords (maximum 7)", "" },
        /*164*/ { 2, 11, "\200", 6, 0, 11, 27, "", "" },
        /*165*/ { 2, 11, "\200", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 11 R11x27-M, requires 8 codewords (maximum 7)", "" },
        /*166*/ { 2, 11, "\201", 6, 0, 11, 27, "", "3 Shift JIS 0x8181" },
        /*167*/ { 2, 11, "\201", 8, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 11 R11x27-M, requires 8 codewords (maximum 7)", "4 Shift JIS 0x8181" },
        /*168*/ { 4, 11, "1", 9, 0, 11, 27, "", "" },
        /*169*/ { 4, 11, "1", 10, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 11 R11x27-H, requires 6 codewords (maximum 5)", "" },
        /*170*/ { 4, 11, "A", 6, 0, 11, 27, "", "" },
        /*171*/ { 4, 11, "A", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 11 R11x27-H, requires 6 codewords (maximum 5)", "" },
        /*172*/ { 4, 11, "\200", 4, 0, 11, 27, "", "" },
        /*173*/ { 4, 11, "\200", 5, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 11 R11x27-H, requires 6 codewords (maximum 5)", "" },
        /*174*/ { 4, 11, "\201", 4, 0, 11, 27, "", "2 Shift JIS 0x8181" },
        /*175*/ { 4, 11, "\201", 6, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 11 R11x27-H, requires 6 codewords (maximum 5)", "3 Shift JIS 0x8181" },
        /*176*/ { 2, 12, "1", 42, 0, 11, 43, "", "" },
        /*177*/ { 2, 12, "1", 43, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 12 R11x43-M, requires 20 codewords (maximum 19)", "" },
        /*178*/ { 2, 12, "A", 26, 0, 11, 43, "", "" },
        /*179*/ { 2, 12, "A", 27, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 12 R11x43-M, requires 20 codewords (maximum 19)", "" },
        /*180*/ { 2, 12, "\200", 18, 0, 11, 43, "", "" },
        /*181*/ { 2, 12, "\200", 19, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 12 R11x43-M, requires 20 codewords (maximum 19)", "" },
        /*182*/ { 2, 12, "\201", 22, 0, 11, 43, "", "11 Shift JIS 0x8181" },
        /*183*/ { 2, 12, "\201", 24, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 12 R11x43-M, requires 21 codewords (maximum 19)", "12 Shift JIS 0x8181" },
        /*184*/ { 4, 12, "1", 23, 0, 11, 43, "", "" },
        /*185*/ { 4, 12, "1", 24, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 12 R11x43-H, requires 12 codewords (maximum 11)", "" },
        /*186*/ { 4, 12, "A", 14, 0, 11, 43, "", "" },
        /*187*/ { 4, 12, "A", 15, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 12 R11x43-H, requires 12 codewords (maximum 11)", "" },
        /*188*/ { 4, 12, "\200", 10, 0, 11, 43, "", "" },
        /*189*/ { 4, 12, "\200", 11, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 12 R11x43-H, requires 12 codewords (maximum 11)", "" },
        /*190*/ { 4, 12, "\201", 12, 0, 11, 43, "", "6 Shift JIS 0x8181" },
        /*191*/ { 4, 12, "\201", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 12 R11x43-H, requires 13 codewords (maximum 11)", "7 Shift JIS 0x8181" },
        /*192*/ { 2, 13, "1", 71, 0, 11, 59, "", "" },
        /*193*/ { 2, 13, "1", 72, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 13 R11x59-M, requires 32 codewords (maximum 31)", "" },
        /*194*/ { 2, 13, "A", 43, 0, 11, 59, "", "" },
        /*195*/ { 2, 13, "A", 44, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 13 R11x59-M, requires 32 codewords (maximum 31)", "" },
        /*196*/ { 2, 13, "\200", 30, 0, 11, 59, "", "" },
        /*197*/ { 2, 13, "\200", 31, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 13 R11x59-M, requires 32 codewords (maximum 31)", "" },
        /*198*/ { 2, 13, "\201", 36, 0, 11, 59, "", "18 Shift JIS 0x8181" },
        /*199*/ { 2, 13, "\201", 38, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 13 R11x59-M, requires 32 codewords (maximum 31)", "19 Shift JIS 0x8181" },
        /*200*/ { 4, 13, "1", 33, 0, 11, 59, "", "" },
        /*201*/ { 4, 13, "1", 34, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 13 R11x59-H, requires 16 codewords (maximum 15)", "" },
        /*202*/ { 4, 13, "A", 20, 0, 11, 59, "", "" },
        /*203*/ { 4, 13, "A", 21, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 13 R11x59-H, requires 16 codewords (maximum 15)", "" },
        /*204*/ { 4, 13, "\200", 14, 0, 11, 59, "", "" },
        /*205*/ { 4, 13, "\200", 15, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 13 R11x59-H, requires 16 codewords (maximum 15)", "" },
        /*206*/ { 4, 13, "\201", 16, 0, 11, 59, "", "8 Shift JIS 0x8181" },
        /*207*/ { 4, 13, "\201", 18, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 13 R11x59-H, requires 16 codewords (maximum 15)", "9 Shift JIS 0x8181" },
        /*208*/ { 2, 14, "1", 100, 0, 11, 77, "", "" },
        /*209*/ { 2, 14, "1", 101, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 14 R11x77-M, requires 44 codewords (maximum 43)", "" },
        /*210*/ { 2, 14, "A", 60, 0, 11, 77, "", "" },
        /*211*/ { 2, 14, "A", 61, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 14 R11x77-M, requires 44 codewords (maximum 43)", "" },
        /*212*/ { 2, 14, "\200", 41, 0, 11, 77, "", "" },
        /*213*/ { 2, 14, "\200", 42, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 14 R11x77-M, requires 44 codewords (maximum 43)", "" },
        /*214*/ { 2, 14, "\201", 50, 0, 11, 77, "", "25 Shift JIS 0x8181" },
        /*215*/ { 2, 14, "\201", 52, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 14 R11x77-M, requires 44 codewords (maximum 43)", "26 Shift JIS 0x8181" },
        /*216*/ { 4, 14, "1", 52, 0, 11, 77, "", "" },
        /*217*/ { 4, 14, "1", 53, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 14 R11x77-H, requires 24 codewords (maximum 23)", "" },
        /*218*/ { 4, 14, "A", 31, 0, 11, 77, "", "" },
        /*219*/ { 4, 14, "A", 32, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 14 R11x77-H, requires 24 codewords (maximum 23)", "" },
        /*220*/ { 4, 14, "\200", 21, 0, 11, 77, "", "" },
        /*221*/ { 4, 14, "\200", 22, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 14 R11x77-H, requires 24 codewords (maximum 23)", "" },
        /*222*/ { 4, 14, "\201", 26, 0, 11, 77, "", "13 Shift JIS 0x8181" },
        /*223*/ { 4, 14, "\201", 28, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 14 R11x77-H, requires 24 codewords (maximum 23)", "14 Shift JIS 0x8181" },
        /*224*/ { 2, 15, "1", 133, 0, 11, 99, "", "" },
        /*225*/ { 2, 15, "1", 134, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 15 R11x99-M, requires 58 codewords (maximum 57)", "" },
        /*226*/ { 2, 15, "A", 81, 0, 11, 99, "", "" },
        /*227*/ { 2, 15, "A", 82, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 15 R11x99-M, requires 58 codewords (maximum 57)", "" },
        /*228*/ { 2, 15, "\200", 55, 0, 11, 99, "", "" },
        /*229*/ { 2, 15, "\200", 56, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 15 R11x99-M, requires 58 codewords (maximum 57)", "" },
        /*230*/ { 2, 15, "\201", 68, 0, 11, 99, "", "34 Shift JIS 0x8181" },
        /*231*/ { 2, 15, "\201", 70, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 15 R11x99-M, requires 58 codewords (maximum 57)", "35 Shift JIS 0x8181" },
        /*232*/ { 4, 15, "1", 66, 0, 11, 99, "", "" },
        /*233*/ { 4, 15, "1", 67, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 15 R11x99-H, requires 30 codewords (maximum 29)", "" },
        /*234*/ { 4, 15, "A", 40, 0, 11, 99, "", "" },
        /*235*/ { 4, 15, "A", 41, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 15 R11x99-H, requires 30 codewords (maximum 29)", "" },
        /*236*/ { 4, 15, "\200", 27, 0, 11, 99, "", "" },
        /*237*/ { 4, 15, "\200", 28, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 15 R11x99-H, requires 30 codewords (maximum 29)", "" },
        /*238*/ { 4, 15, "\201", 34, 0, 11, 99, "", "17 Shift JIS 0x8181" },
        /*239*/ { 4, 15, "\201", 36, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 15 R11x99-H, requires 31 codewords (maximum 29)", "18 Shift JIS 0x8181" },
        /*240*/ { 2, 16, "1", 198, 0, 11, 139, "", "" },
        /*241*/ { 2, 16, "1", 199, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 16 R11x139-M, requires 85 codewords (maximum 84)", "" },
        /*242*/ { 2, 16, "A", 120, 0, 11, 139, "", "" },
        /*243*/ { 2, 16, "A", 121, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 16 R11x139-M, requires 85 codewords (maximum 84)", "" },
        /*244*/ { 2, 16, "\200", 82, 0, 11, 139, "", "" },
        /*245*/ { 2, 16, "\200", 83, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 16 R11x139-M, requires 85 codewords (maximum 84)", "" },
        /*246*/ { 2, 16, "\201", 102, 0, 11, 139, "", "51 Shift JIS 0x8181" },
        /*247*/ { 2, 16, "\201", 104, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 16 R11x139-M, requires 86 codewords (maximum 84)", "52 Shift JIS 0x8181" },
        /*248*/ { 4, 16, "1", 97, 0, 11, 139, "", "" },
        /*249*/ { 4, 16, "1", 98, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 16 R11x139-H, requires 43 codewords (maximum 42)", "" },
        /*250*/ { 4, 16, "A", 59, 0, 11, 139, "", "" },
        /*251*/ { 4, 16, "A", 60, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 16 R11x139-H, requires 43 codewords (maximum 42)", "" },
        /*252*/ { 4, 16, "\200", 40, 0, 11, 139, "", "" },
        /*253*/ { 4, 16, "\200", 41, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 16 R11x139-H, requires 43 codewords (maximum 42)", "" },
        /*254*/ { 4, 16, "\201", 50, 0, 11, 139, "", "25 Shift JIS 0x8181" },
        /*255*/ { 4, 16, "\201", 52, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 16 R11x139-H, requires 44 codewords (maximum 42)", "26 Shift JIS 0x8181" },
        /*256*/ { 2, 17, "1", 26, 0, 13, 27, "", "" },
        /*257*/ { 2, 17, "1", 27, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 17 R13x27-M, requires 13 codewords (maximum 12)", "" },
        /*258*/ { 2, 17, "A", 16, 0, 13, 27, "", "" },
        /*259*/ { 2, 17, "A", 17, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 17 R13x27-M, requires 13 codewords (maximum 12)", "" },
        /*260*/ { 2, 17, "\200", 11, 0, 13, 27, "", "" },
        /*261*/ { 2, 17, "\200", 12, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 17 R13x27-M, requires 13 codewords (maximum 12)", "" },
        /*262*/ { 2, 17, "\201", 12, 0, 13, 27, "", "6 Shift JIS 0x8181" },
        /*263*/ { 2, 17, "\201", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 17 R13x27-M, requires 13 codewords (maximum 12)", "7 Shift JIS 0x8181" },
        /*264*/ { 4, 17, "1", 14, 0, 13, 27, "", "" },
        /*265*/ { 4, 17, "1", 15, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 17 R13x27-H, requires 8 codewords (maximum 7)", "" },
        /*266*/ { 4, 17, "A", 8, 0, 13, 27, "", "" },
        /*267*/ { 4, 17, "A", 9, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 17 R13x27-H, requires 8 codewords (maximum 7)", "" },
        /*268*/ { 4, 17, "\200", 6, 0, 13, 27, "", "" },
        /*269*/ { 4, 17, "\200", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 17 R13x27-H, requires 8 codewords (maximum 7)", "" },
        /*270*/ { 4, 17, "\201", 6, 0, 13, 27, "", "3 Shift JIS 0x8181" },
        /*271*/ { 4, 17, "\201", 8, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 17 R13x27-H, requires 8 codewords (maximum 7)", "4 Shift JIS 0x8181" },
        /*272*/ { 2, 18, "1", 62, 0, 13, 43, "", "" },
        /*273*/ { 2, 18, "1", 63, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 18 R13x43-M, requires 28 codewords (maximum 27)", "" },
        /*274*/ { 2, 18, "A", 37, 0, 13, 43, "", "" },
        /*275*/ { 2, 18, "A", 38, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 18 R13x43-M, requires 28 codewords (maximum 27)", "" },
        /*276*/ { 2, 18, "\200", 26, 0, 13, 43, "", "" },
        /*277*/ { 2, 18, "\200", 27, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 18 R13x43-M, requires 28 codewords (maximum 27)", "" },
        /*278*/ { 2, 18, "\201", 32, 0, 13, 43, "", "16 Shift JIS 0x8181" },
        /*279*/ { 2, 18, "\201", 34, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 18 R13x43-M, requires 29 codewords (maximum 27)", "17 Shift JIS 0x8181" },
        /*280*/ { 4, 18, "1", 28, 0, 13, 43, "", "" },
        /*281*/ { 4, 18, "1", 29, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 18 R13x43-H, requires 14 codewords (maximum 13)", "" },
        /*282*/ { 4, 18, "A", 17, 0, 13, 43, "", "" },
        /*283*/ { 4, 18, "A", 18, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 18 R13x43-H, requires 14 codewords (maximum 13)", "" },
        /*284*/ { 4, 18, "\200", 12, 0, 13, 43, "", "" },
        /*285*/ { 4, 18, "\200", 13, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 18 R13x43-H, requires 14 codewords (maximum 13)", "" },
        /*286*/ { 4, 18, "\201", 14, 0, 13, 43, "", "7 Shift JIS 0x8181" },
        /*287*/ { 4, 18, "\201", 16, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 18 R13x43-H, requires 14 codewords (maximum 13)", "8 Shift JIS 0x8181" },
        /*288*/ { 2, 19, "1", 88, 0, 13, 59, "", "" },
        /*289*/ { 2, 19, "1", 89, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 19 R13x59-M, requires 39 codewords (maximum 38)", "" },
        /*290*/ { 2, 19, "A", 53, 0, 13, 59, "", "" },
        /*291*/ { 2, 19, "A", 54, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 19 R13x59-M, requires 39 codewords (maximum 38)", "" },
        /*292*/ { 2, 19, "\200", 36, 0, 13, 59, "", "" },
        /*293*/ { 2, 19, "\200", 37, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 19 R13x59-M, requires 39 codewords (maximum 38)", "" },
        /*294*/ { 2, 19, "\201", 44, 0, 13, 59, "", "22 Shift JIS 0x8181" },
        /*295*/ { 2, 19, "\201", 46, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 19 R13x59-M, requires 39 codewords (maximum 38)", "23 Shift JIS 0x8181" },
        /*296*/ { 4, 19, "1", 45, 0, 13, 59, "", "" },
        /*297*/ { 4, 19, "1", 46, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 19 R13x59-H, requires 21 codewords (maximum 20)", "" },
        /*298*/ { 4, 19, "A", 27, 0, 13, 59, "", "" },
        /*299*/ { 4, 19, "A", 28, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 19 R13x59-H, requires 21 codewords (maximum 20)", "" },
        /*300*/ { 4, 19, "\200", 18, 0, 13, 59, "", "" },
        /*301*/ { 4, 19, "\200", 19, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 19 R13x59-H, requires 21 codewords (maximum 20)", "" },
        /*302*/ { 4, 19, "\201", 22, 0, 13, 59, "", "11 Shift JIS 0x8181" },
        /*303*/ { 4, 19, "\201", 24, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 19 R13x59-H, requires 21 codewords (maximum 20)", "12 Shift JIS 0x8181" },
        /*304*/ { 2, 20, "1", 124, 0, 13, 77, "", "" },
        /*305*/ { 2, 20, "1", 125, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 20 R13x77-M, requires 54 codewords (maximum 53)", "" },
        /*306*/ { 2, 20, "A", 75, 0, 13, 77, "", "" },
        /*307*/ { 2, 20, "A", 76, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 20 R13x77-M, requires 54 codewords (maximum 53)", "" },
        /*308*/ { 2, 20, "\200", 51, 0, 13, 77, "", "" },
        /*309*/ { 2, 20, "\200", 52, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 20 R13x77-M, requires 54 codewords (maximum 53)", "" },
        /*310*/ { 2, 20, "\201", 62, 0, 13, 77, "", "31 Shift JIS 0x8181" },
        /*311*/ { 2, 20, "\201", 64, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 20 R13x77-M, requires 54 codewords (maximum 53)", "32 Shift JIS 0x8181" },
        /*312*/ { 4, 20, "1", 66, 0, 13, 77, "", "" },
        /*313*/ { 4, 20, "1", 67, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 20 R13x77-H, requires 30 codewords (maximum 29)", "" },
        /*314*/ { 4, 20, "A", 40, 0, 13, 77, "", "" },
        /*315*/ { 4, 20, "A", 41, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 20 R13x77-H, requires 30 codewords (maximum 29)", "" },
        /*316*/ { 4, 20, "\200", 27, 0, 13, 77, "", "" },
        /*317*/ { 4, 20, "\200", 28, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 20 R13x77-H, requires 30 codewords (maximum 29)", "" },
        /*318*/ { 4, 20, "\201", 34, 0, 13, 77, "", "17 Shift JIS 0x8181" },
        /*319*/ { 4, 20, "\201", 36, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 20 R13x77-H, requires 31 codewords (maximum 29)", "18 Shift JIS 0x8181" },
        /*320*/ { 2, 21, "1", 171, 0, 13, 99, "", "" },
        /*321*/ { 2, 21, "1", 172, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 21 R13x99-M, requires 74 codewords (maximum 73)", "" },
        /*322*/ { 2, 21, "A", 104, 0, 13, 99, "", "" },
        /*323*/ { 2, 21, "A", 105, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 21 R13x99-M, requires 74 codewords (maximum 73)", "" },
        /*324*/ { 2, 21, "\200", 71, 0, 13, 99, "", "" },
        /*325*/ { 2, 21, "\200", 72, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 21 R13x99-M, requires 74 codewords (maximum 73)", "" },
        /*326*/ { 2, 21, "\201", 88, 0, 13, 99, "", "44 Shift JIS 0x8181" },
        /*327*/ { 2, 21, "\201", 90, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 21 R13x99-M, requires 75 codewords (maximum 73)", "45 Shift JIS 0x8181" },
        /*328*/ { 4, 21, "1", 80, 0, 13, 99, "", "" },
        /*329*/ { 4, 21, "1", 81, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 21 R13x99-H, requires 36 codewords (maximum 35)", "" },
        /*330*/ { 4, 21, "A", 49, 0, 13, 99, "", "" },
        /*331*/ { 4, 21, "A", 50, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 21 R13x99-H, requires 36 codewords (maximum 35)", "" },
        /*332*/ { 4, 21, "\200", 33, 0, 13, 99, "", "" },
        /*333*/ { 4, 21, "\200", 34, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 21 R13x99-H, requires 36 codewords (maximum 35)", "" },
        /*334*/ { 4, 21, "\201", 40, 0, 13, 99, "", "20 Shift JIS 0x8181" },
        /*335*/ { 4, 21, "\201", 42, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 21 R13x99-H, requires 36 codewords (maximum 35)", "21 Shift JIS 0x8181" },
        /*336*/ { 2, 22, "1", 251, 0, 13, 139, "", "" },
        /*337*/ { 2, 22, "1", 252, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 22 R13x139-M, requires 107 codewords (maximum 106)", "" },
        /*338*/ { 2, 22, "A", 152, 0, 13, 139, "", "" },
        /*339*/ { 2, 22, "A", 153, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 22 R13x139-M, requires 107 codewords (maximum 106)", "" },
        /*340*/ { 2, 22, "\200", 104, 0, 13, 139, "", "" },
        /*341*/ { 2, 22, "\200", 105, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 22 R13x139-M, requires 107 codewords (maximum 106)", "" },
        /*342*/ { 2, 22, "\201", 128, 0, 13, 139, "", "64 Shift JIS 0x8181" },
        /*343*/ { 2, 22, "\201", 130, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 22 R13x139-M, requires 107 codewords (maximum 106)", "65 Shift JIS 0x8181" },
        /*344*/ { 4, 22, "1", 126, 0, 13, 139, "", "" },
        /*345*/ { 4, 22, "1", 127, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 22 R13x139-H, requires 55 codewords (maximum 54)", "" },
        /*346*/ { 4, 22, "A", 76, 0, 13, 139, "", "" },
        /*347*/ { 4, 22, "A", 77, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 22 R13x139-H, requires 55 codewords (maximum 54)", "" },
        /*348*/ { 4, 22, "\200", 52, 0, 13, 139, "", "" },
        /*349*/ { 4, 22, "\200", 53, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 22 R13x139-H, requires 55 codewords (maximum 54)", "" },
        /*350*/ { 4, 22, "\201", 64, 0, 13, 139, "", "32 Shift JIS 0x8181" },
        /*351*/ { 4, 22, "\201", 66, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 22 R13x139-H, requires 55 codewords (maximum 54)", "33 Shift JIS 0x8181" },
        /*352*/ { 2, 23, "1", 76, 0, 15, 43, "", "" },
        /*353*/ { 2, 23, "1", 77, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 23 R15x43-M, requires 34 codewords (maximum 33)", "" },
        /*354*/ { 2, 23, "A", 46, 0, 15, 43, "", "" },
        /*355*/ { 2, 23, "A", 47, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 23 R15x43-M, requires 34 codewords (maximum 33)", "" },
        /*356*/ { 2, 23, "\200", 31, 0, 15, 43, "", "" },
        /*357*/ { 2, 23, "\200", 32, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 23 R15x43-M, requires 34 codewords (maximum 33)", "" },
        /*358*/ { 2, 23, "\201", 38, 0, 15, 43, "", "19 Shift JIS 0x8181" },
        /*359*/ { 2, 23, "\201", 40, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 23 R15x43-M, requires 34 codewords (maximum 33)", "20 Shift JIS 0x8181" },
        /*360*/ { 4, 23, "1", 33, 0, 15, 43, "", "" },
        /*361*/ { 4, 23, "1", 34, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 23 R15x43-H, requires 16 codewords (maximum 15)", "" },
        /*362*/ { 4, 23, "A", 20, 0, 15, 43, "", "" },
        /*363*/ { 4, 23, "A", 21, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 23 R15x43-H, requires 16 codewords (maximum 15)", "" },
        /*364*/ { 4, 23, "\200", 13, 0, 15, 43, "", "" },
        /*365*/ { 4, 23, "\200", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 23 R15x43-H, requires 16 codewords (maximum 15)", "" },
        /*366*/ { 4, 23, "\201", 16, 0, 15, 43, "", "8 Shift JIS 0x8181" },
        /*367*/ { 4, 23, "\201", 18, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 23 R15x43-H, requires 16 codewords (maximum 15)", "9 Shift JIS 0x8181" },
        /*368*/ { 2, 24, "1", 112, 0, 15, 59, "", "" },
        /*369*/ { 2, 24, "1", 113, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 24 R15x59-M, requires 49 codewords (maximum 48)", "" },
        /*370*/ { 2, 24, "A", 68, 0, 15, 59, "", "" },
        /*371*/ { 2, 24, "A", 69, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 24 R15x59-M, requires 49 codewords (maximum 48)", "" },
        /*372*/ { 2, 24, "\200", 46, 0, 15, 59, "", "" },
        /*373*/ { 2, 24, "\200", 47, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 24 R15x59-M, requires 49 codewords (maximum 48)", "" },
        /*374*/ { 2, 24, "\201", 56, 0, 15, 59, "", "28 Shift JIS 0x8181" },
        /*375*/ { 2, 24, "\201", 58, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 24 R15x59-M, requires 49 codewords (maximum 48)", "29 Shift JIS 0x8181" },
        /*376*/ { 4, 24, "1", 59, 0, 15, 59, "", "" },
        /*377*/ { 4, 24, "1", 60, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 24 R15x59-H, requires 27 codewords (maximum 26)", "" },
        /*378*/ { 4, 24, "A", 36, 0, 15, 59, "", "" },
        /*379*/ { 4, 24, "A", 37, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 24 R15x59-H, requires 27 codewords (maximum 26)", "" },
        /*380*/ { 4, 24, "\200", 24, 0, 15, 59, "", "" },
        /*381*/ { 4, 24, "\200", 25, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 24 R15x59-H, requires 27 codewords (maximum 26)", "" },
        /*382*/ { 4, 24, "\201", 30, 0, 15, 59, "", "30 Shift JIS 0x8181" },
        /*383*/ { 4, 24, "\201", 32, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 24 R15x59-H, requires 27 codewords (maximum 26)", "31 Shift JIS 0x8181" },
        /*384*/ { 2, 25, "1", 157, 0, 15, 77, "", "" },
        /*385*/ { 2, 25, "1", 158, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 25 R15x77-M, requires 68 codewords (maximum 67)", "" },
        /*386*/ { 2, 25, "A", 95, 0, 15, 77, "", "" },
        /*387*/ { 2, 25, "A", 96, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 25 R15x77-M, requires 68 codewords (maximum 67)", "" },
        /*388*/ { 2, 25, "\200", 65, 0, 15, 77, "", "" },
        /*389*/ { 2, 25, "\200", 66, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 25 R15x77-M, requires 68 codewords (maximum 67)", "" },
        /*390*/ { 2, 25, "\201", 80, 0, 15, 77, "", "40 Shift JIS 0x8181" },
        /*391*/ { 2, 25, "\201", 82, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 25 R15x77-M, requires 68 codewords (maximum 67)", "41 Shift JIS 0x8181" },
        /*392*/ { 4, 25, "1", 71, 0, 15, 77, "", "" },
        /*393*/ { 4, 25, "1", 72, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 25 R15x77-H, requires 32 codewords (maximum 31)", "" },
        /*394*/ { 4, 25, "A", 43, 0, 15, 77, "", "" },
        /*395*/ { 4, 25, "A", 44, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 25 R15x77-H, requires 32 codewords (maximum 31)", "" },
        /*396*/ { 4, 25, "\200", 29, 0, 15, 77, "", "" },
        /*397*/ { 4, 25, "\200", 30, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 25 R15x77-H, requires 32 codewords (maximum 31)", "" },
        /*398*/ { 4, 25, "\201", 36, 0, 15, 77, "", "18 Shift JIS 0x8181" },
        /*399*/ { 4, 25, "\201", 38, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 25 R15x77-H, requires 32 codewords (maximum 31)", "" },
        /*400*/ { 2, 26, "1", 207, 0, 15, 99, "", "" },
        /*401*/ { 2, 26, "1", 208, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 26 R15x99-M, requires 89 codewords (maximum 88)", "" },
        /*402*/ { 2, 26, "A", 126, 0, 15, 99, "", "" },
        /*403*/ { 2, 26, "A", 127, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 26 R15x99-M, requires 89 codewords (maximum 88)", "" },
        /*404*/ { 2, 26, "\200", 86, 0, 15, 99, "", "" },
        /*405*/ { 2, 26, "\200", 87, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 26 R15x99-M, requires 89 codewords (maximum 88)", "" },
        /*406*/ { 2, 26, "\201", 106, 0, 15, 99, "", "53 Shift JIS 0x8181" },
        /*407*/ { 2, 26, "\201", 108, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 26 R15x99-M, requires 89 codewords (maximum 88)", "54 Shift JIS 0x8181" },
        /*408*/ { 4, 26, "1", 111, 0, 15, 99, "", "" },
        /*409*/ { 4, 26, "1", 112, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 26 R15x99-H, requires 49 codewords (maximum 48)", "" },
        /*410*/ { 4, 26, "A", 68, 0, 15, 99, "", "" },
        /*411*/ { 4, 26, "A", 69, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 26 R15x99-H, requires 49 codewords (maximum 48)", "" },
        /*412*/ { 4, 26, "\200", 46, 0, 15, 99, "", "" },
        /*413*/ { 4, 26, "\200", 47, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 26 R15x99-H, requires 49 codewords (maximum 48)", "" },
        /*414*/ { 4, 26, "\201", 56, 0, 15, 99, "", "28 Shift JIS 0x8181" },
        /*415*/ { 4, 26, "\201", 58, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 26 R15x99-H, requires 49 codewords (maximum 48)", "29 Shift JIS 0x8181" },
        /*416*/ { 2, 27, "1", 301, 0, 15, 139, "", "" },
        /*417*/ { 2, 27, "1", 302, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 27 R15x139-M, requires 128 codewords (maximum 127)", "" },
        /*418*/ { 2, 27, "A", 182, 0, 15, 139, "", "" },
        /*419*/ { 2, 27, "A", 183, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 27 R15x139-M, requires 128 codewords (maximum 127)", "" },
        /*420*/ { 2, 27, "\200", 125, 0, 15, 139, "", "" },
        /*421*/ { 2, 27, "\200", 126, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 27 R15x139-M, requires 128 codewords (maximum 127)", "" },
        /*422*/ { 2, 27, "\201", 154, 0, 15, 139, "", "77 Shift JIS 0x8181" },
        /*423*/ { 2, 27, "\201", 156, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 27 R15x139-M, requires 128 codewords (maximum 127)", "78 Shift JIS 0x8181" },
        /*424*/ { 4, 27, "1", 162, 0, 15, 139, "", "" },
        /*425*/ { 4, 27, "1", 163, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 27 R15x139-H, requires 70 codewords (maximum 69)", "" },
        /*426*/ { 4, 27, "A", 98, 0, 15, 139, "", "" },
        /*427*/ { 4, 27, "A", 99, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 27 R15x139-H, requires 70 codewords (maximum 69)", "" },
        /*428*/ { 4, 27, "\200", 67, 0, 15, 139, "", "" },
        /*429*/ { 4, 27, "\200", 68, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 27 R15x139-H, requires 70 codewords (maximum 69)", "" },
        /*430*/ { 4, 27, "\201", 82, 0, 15, 139, "", "41 Shift JIS 0x8181" },
        /*431*/ { 4, 27, "\201", 84, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 27 R15x139-H, requires 70 codewords (maximum 69)", "42 Shift JIS 0x8181" },
        /*432*/ { 2, 28, "1", 90, 0, 17, 43, "", "" },
        /*433*/ { 2, 28, "1", 91, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 28 R17x43-M, requires 40 codewords (maximum 39)", "" },
        /*434*/ { 2, 28, "A", 55, 0, 17, 43, "", "" },
        /*435*/ { 2, 28, "A", 56, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 28 R17x43-M, requires 40 codewords (maximum 39)", "" },
        /*436*/ { 2, 28, "\200", 37, 0, 17, 43, "", "" },
        /*437*/ { 2, 28, "\200", 38, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 28 R17x43-M, requires 40 codewords (maximum 39)", "" },
        /*438*/ { 2, 28, "\201", 46, 0, 17, 43, "", "23 Shift JIS 0x8181" },
        /*439*/ { 2, 28, "\201", 48, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 28 R17x43-M, requires 40 codewords (maximum 39)", "24 Shift JIS 0x8181" },
        /*440*/ { 4, 28, "1", 47, 0, 17, 43, "", "" },
        /*441*/ { 4, 28, "1", 48, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 28 R17x43-H, requires 22 codewords (maximum 21)", "" },
        /*442*/ { 4, 28, "A", 28, 0, 17, 43, "", "" },
        /*443*/ { 4, 28, "A", 29, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 28 R17x43-H, requires 22 codewords (maximum 21)", "" },
        /*444*/ { 4, 28, "\200", 19, 0, 17, 43, "", "" },
        /*445*/ { 4, 28, "\200", 20, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 28 R17x43-H, requires 22 codewords (maximum 21)", "" },
        /*446*/ { 4, 28, "\201", 24, 0, 17, 43, "", "12 Shift JIS 0x8181" },
        /*447*/ { 4, 28, "\201", 26, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 28 R17x43-H, requires 23 codewords (maximum 21)", "13 Shift JIS 0x8181" },
        /*448*/ { 2, 29, "1", 131, 0, 17, 59, "", "" },
        /*449*/ { 2, 29, "1", 132, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 29 R17x59-M, requires 57 codewords (maximum 56)", "" },
        /*450*/ { 2, 29, "A", 79, 0, 17, 59, "", "" },
        /*451*/ { 2, 29, "A", 80, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 29 R17x59-M, requires 57 codewords (maximum 56)", "" },
        /*452*/ { 2, 29, "\200", 54, 0, 17, 59, "", "" },
        /*453*/ { 2, 29, "\200", 55, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 29 R17x59-M, requires 57 codewords (maximum 56)", "" },
        /*454*/ { 2, 29, "\201", 66, 0, 17, 59, "", "33 Shift JIS 0x8181" },
        /*455*/ { 2, 29, "\201", 68, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 29 R17x59-M, requires 57 codewords (maximum 56)", "34 Shift JIS 0x8181" },
        /*456*/ { 4, 29, "1", 63, 0, 17, 59, "", "" },
        /*457*/ { 4, 29, "1", 64, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 29 R17x59-H, requires 29 codewords (maximum 28)", "" },
        /*458*/ { 4, 29, "A", 38, 0, 17, 59, "", "" },
        /*459*/ { 4, 29, "A", 39, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 29 R17x59-H, requires 29 codewords (maximum 28)", "" },
        /*460*/ { 4, 29, "\200", 26, 0, 17, 59, "", "" },
        /*461*/ { 4, 29, "\200", 27, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 29 R17x59-H, requires 29 codewords (maximum 28)", "" },
        /*462*/ { 4, 29, "\201", 32, 0, 17, 59, "", "16 Shift JIS 0x8181" },
        /*463*/ { 4, 29, "\201", 34, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 29 R17x59-H, requires 29 codewords (maximum 28)", "17 Shift JIS 0x8181" },
        /*464*/ { 2, 30, "1", 183, 0, 17, 77, "", "" },
        /*465*/ { 2, 30, "1", 184, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 30 R17x77-M, requires 79 codewords (maximum 78)", "" },
        /*466*/ { 2, 30, "A", 111, 0, 17, 77, "", "" },
        /*467*/ { 2, 30, "A", 112, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 30 R17x77-M, requires 79 codewords (maximum 78)", "" },
        /*468*/ { 2, 30, "\200", 76, 0, 17, 77, "", "" },
        /*469*/ { 2, 30, "\200", 77, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 30 R17x77-M, requires 79 codewords (maximum 78)", "" },
        /*470*/ { 2, 30, "\201", 94, 0, 17, 77, "", "47 Shift JIS 0x8181" },
        /*471*/ { 2, 30, "\201", 96, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 30 R17x77-M, requires 80 codewords (maximum 78)", "48 Shift JIS 0x8181" },
        /*472*/ { 4, 30, "1", 87, 0, 17, 77, "", "" },
        /*473*/ { 4, 30, "1", 88, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 30 R17x77-H, requires 39 codewords (maximum 38)", "" },
        /*474*/ { 4, 30, "A", 53, 0, 17, 77, "", "" },
        /*475*/ { 4, 30, "A", 54, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 30 R17x77-H, requires 39 codewords (maximum 38)", "" },
        /*476*/ { 4, 30, "\200", 36, 0, 17, 77, "", "" },
        /*477*/ { 4, 30, "\200", 37, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 30 R17x77-H, requires 39 codewords (maximum 38)", "" },
        /*478*/ { 4, 30, "\201", 44, 0, 17, 77, "", "22 Shift JIS 0x8181" },
        /*479*/ { 4, 30, "\201", 46, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 30 R17x77-H, requires 39 codewords (maximum 38)", "23 Shift JIS 0x8181" },
        /*480*/ { 2, 31, "1", 236, 0, 17, 99, "", "" },
        /*481*/ { 2, 31, "1", 237, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 31 R17x99-M, requires 101 codewords (maximum 100)", "" },
        /*482*/ { 2, 31, "A", 143, 0, 17, 99, "", "" },
        /*483*/ { 2, 31, "A", 144, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 31 R17x99-M, requires 101 codewords (maximum 100)", "" },
        /*484*/ { 2, 31, "\200", 98, 0, 17, 99, "", "" },
        /*485*/ { 2, 31, "\200", 99, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 31 R17x99-M, requires 101 codewords (maximum 100)", "" },
        /*486*/ { 2, 31, "\201", 120, 0, 17, 99, "", "60 Shift JIS 0x8181" },
        /*487*/ { 2, 31, "\201", 122, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 31 R17x99-M, requires 101 codewords (maximum 100)", "61 Shift JIS 0x8181" },
        /*488*/ { 4, 31, "1", 131, 0, 17, 99, "", "" },
        /*489*/ { 4, 31, "1", 132, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 31 R17x99-H, requires 57 codewords (maximum 56)", "" },
        /*490*/ { 4, 31, "A", 79, 0, 17, 99, "", "" },
        /*491*/ { 4, 31, "A", 80, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 31 R17x99-H, requires 57 codewords (maximum 56)", "" },
        /*492*/ { 4, 31, "\200", 54, 0, 17, 99, "", "" },
        /*493*/ { 4, 31, "\200", 55, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 31 R17x99-H, requires 57 codewords (maximum 56)", "" },
        /*494*/ { 4, 31, "\201", 66, 0, 17, 99, "", "33 Shift JIS 0x8181" },
        /*495*/ { 4, 31, "\201", 68, ZINT_ERROR_TOO_LONG, 0, 0, "Error 560: Input too long for Version 31 R17x99-H, requires 57 codewords (maximum 56)", "34 Shift JIS 0x8181" },
        /*496*/ { 2, 32, "1", 361, 0, 17, 139, "", "" },
        /*497*/ { 2, 32, "1", 362, ZINT_ERROR_TOO_LONG, 0, 0, "Error 578: Input too long for ECC level M, requires 153 codewords (maximum 152)", "" },
        /*498*/ { 2, 32, "A", 219, 0, 17, 139, "", "" },
        /*499*/ { 2, 32, "A", 220, ZINT_ERROR_TOO_LONG, 0, 0, "Error 578: Input too long for ECC level M, requires 153 codewords (maximum 152)", "" },
        /*500*/ { 2, 32, "\200", 150, 0, 17, 139, "", "" },
        /*501*/ { 2, 32, "\200", 151, ZINT_ERROR_TOO_LONG, 0, 0, "Error 578: Input too long for ECC level M, requires 153 codewords (maximum 152)", "" },
        /*502*/ { 2, 32, "\201", 184, 0, 17, 139, "", "92 Shift JIS 0x8181" },
        /*503*/ { 2, 32, "\201", 186, ZINT_ERROR_TOO_LONG, 0, 0, "Error 578: Input too long for ECC level M, requires 153 codewords (maximum 152)", "93 Shift JIS 0x8181" },
        /*504*/ { 4, 32, "1", 178, 0, 17, 139, "", "" },
        /*505*/ { 4, 32, "1", 179, ZINT_ERROR_TOO_LONG, 0, 0, "Error 578: Input too long for ECC level H, requires 77 codewords (maximum 76)", "" },
        /*506*/ { 4, 32, "A", 108, 0, 17, 139, "", "" },
        /*507*/ { 4, 32, "A", 109, ZINT_ERROR_TOO_LONG, 0, 0, "Error 578: Input too long for ECC level H, requires 77 codewords (maximum 76)", "" },
        /*508*/ { 4, 32, "\200", 74, 0, 17, 139, "", "" },
        /*509*/ { 4, 32, "\200", 75, ZINT_ERROR_TOO_LONG, 0, 0, "Error 578: Input too long for ECC level H, requires 77 codewords (maximum 76)", "" },
        /*510*/ { 4, 32, "\201", 92, 0, 17, 139, "", "46 Shift JIS 0x8181" },
        /*511*/ { 4, 32, "\201", 94, ZINT_ERROR_TOO_LONG, 0, 0, "Error 578: Input too long for ECC level H, requires 78 codewords (maximum 76)", "47 Shift JIS 0x8181" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[ZINT_MAX_DATA_LEN];

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_rmqr_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, ZINT_FULL_MULTIBYTE, -1 /*output_options*/, data_buf, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %d, %d, \"%s\", %d, %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, data[i].option_1, data[i].option_2,
                    testUtilEscape(data[i].pattern, (int) strlen(data[i].pattern), escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d errtxt %s != %s\n", i, symbol->errtxt, data[i].expected_errtxt);
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
            }

            symbol->input_mode |= FAST_MODE;
            ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data_buf, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[17 * 139 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data_buf, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data_buf, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        char *data;
        int ret_encode;
        int ret_vector;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
        int zxingcpp_cmp;
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, -1, "12345", 0, 0, 11, 27, "", 1, "" }, /* ECC auto-set to H, version auto-set to 11 (R11x27) */
        /*  1*/ { UNICODE_MODE, -1, 4, 11, "12345", 0, 0, 11, 27, "", 1, "" },
        /*  2*/ { UNICODE_MODE, -1, 1, -1, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 576: Error correction level L not available in rMQR", 1, "" }, /* ECC L not available */
        /*  3*/ { UNICODE_MODE, -1, 3, -1, "12345", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 577: Error correction level Q not available in rMQR", 1, "" }, /* ECC Q not available */
        /*  4*/ { UNICODE_MODE, -1, 4, 11, "123456789", 0, 0, 11, 27, "", 1, "" }, /* Max capacity ECC H, version 11, 9 numbers */
        /*  5*/ { UNICODE_MODE, -1, 4, 11, "1234567890", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 11 R11x27-H, requires 6 codewords (maximum 5)", 1, "" },
        /*  6*/ { UNICODE_MODE, -1, 2, 11, "12345678901234", 0, 0, 11, 27, "", 1, "" }, /* Max capacity ECC M, version 11, 14 numbers */
        /*  7*/ { UNICODE_MODE, -1, 2, 11, "123456789012345", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 11 R11x27-M, requires 8 codewords (maximum 7)", 1, "" },
        /*  8*/ { UNICODE_MODE, -1, 4, 11, "ABCDEF", 0, 0, 11, 27, "", 1, "" }, /* Max capacity ECC H, version 11, 6 letters */
        /*  9*/ { UNICODE_MODE, -1, 4, 11, "ABCDEFG", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 11 R11x27-H, requires 6 codewords (maximum 5)", 1, "" },
        /* 10*/ { UNICODE_MODE, -1, 2, 11, "ABCDEFGH", 0, 0, 11, 27, "", 1, "" }, /* Max capacity ECC M, version 11, 8 letters */
        /* 11*/ { UNICODE_MODE, -1, 2, 11, "ABCDEFGHI", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 11 R11x27-M, requires 8 codewords (maximum 7)", 1, "" },
        /* 12*/ { UNICODE_MODE, -1, 4, 11, "\177\177\177\177", 0, 0, 11, 27, "", 1, "" }, /* Max capacity ECC H, version 11, 4 bytes */
        /* 13*/ { UNICODE_MODE, -1, 4, 11, "\177\177\177\177\177", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 11 R11x27-H, requires 6 codewords (maximum 5)", 1, "" },
        /* 14*/ { UNICODE_MODE, -1, 2, 11, "\177\177\177\177\177\177", 0, 0, 11, 27, "", 1, "" }, /* Max capacity ECC M, version 11, 6 bytes */
        /* 15*/ { UNICODE_MODE, -1, 2, 11, "\177\177\177\177\177\177\177", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 11 R11x27-M, requires 8 codewords (maximum 7)", 1, "" },
        /* 16*/ { UNICODE_MODE, -1, 4, 11, "点茗", ZINT_WARN_NONCOMPLIANT, 0, 11, 27, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC H, version 11, 2 kanji */
        /* 17*/ { UNICODE_MODE, -1, 4, 11, "点茗点", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 11 R11x27-H, requires 6 codewords (maximum 5)", 1, "" },
        /* 18*/ { UNICODE_MODE, -1, 2, 11, "点茗点", ZINT_WARN_NONCOMPLIANT, 0, 11, 27, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC M, version 11, 3 kanji */
        /* 19*/ { UNICODE_MODE, -1, 2, 11, "点茗点茗", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 11 R11x27-M, requires 8 codewords (maximum 7)", 1, "" },
        /* 20*/ { UNICODE_MODE, -1, -1, 1, "12345", 0, 0, 7, 43, "", 1, "" }, /* ECC auto-set to M, version 1 (R7x43) */
        /* 21*/ { UNICODE_MODE, -1, 2, 1, "12345", 0, 0, 7, 43, "", 1, "" },
        /* 22*/ { UNICODE_MODE, -1, 4, 1, "12345", 0, 0, 7, 43, "", 1, "" }, /* Max capacity ECC H, version 1, 5 numbers */
        /* 23*/ { UNICODE_MODE, -1, 4, 1, "123456", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 1 R7x43-H, requires 4 codewords (maximum 3)", 1, "" },
        /* 24*/ { UNICODE_MODE, -1, 2, 1, "123456789012", 0, 0, 7, 43, "", 1, "" }, /* Max capacity ECC M, version 1, 12 numbers */
        /* 25*/ { UNICODE_MODE, -1, 2, 1, "1234567890123", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 1 R7x43-M, requires 7 codewords (maximum 6)", 1, "" },
        /* 26*/ { UNICODE_MODE, -1, 4, 1, "ABC", 0, 0, 7, 43, "", 1, "" }, /* Max capacity ECC H, version 1, 3 letters */
        /* 27*/ { UNICODE_MODE, -1, 4, 1, "ABCD", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 1 R7x43-H, requires 4 codewords (maximum 3)", 1, "" },
        /* 28*/ { UNICODE_MODE, -1, 2, 1, "ABCDEFG", 0, 0, 7, 43, "", 1, "" }, /* Max capacity ECC M, version 1, 7 letters */
        /* 29*/ { UNICODE_MODE, -1, 2, 1, "ABCDEFGH", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 1 R7x43-M, requires 7 codewords (maximum 6)", 1, "" },
        /* 30*/ { UNICODE_MODE, -1, 4, 1, "\177\177", 0, 0, 7, 43, "", 1, "" }, /* Max capacity ECC H, version 1, 2 bytes */
        /* 31*/ { UNICODE_MODE, -1, 4, 1, "\177\177\177", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 1 R7x43-H, requires 4 codewords (maximum 3)", 1, "" },
        /* 32*/ { UNICODE_MODE, -1, 2, 1, "\177\177\177\177\177", 0, 0, 7, 43, "", 1, "" }, /* Max capacity ECC M, version 1, 5 bytes */
        /* 33*/ { UNICODE_MODE, -1, 2, 1, "\177\177\177\177\177\177", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 1 R7x43-M, requires 7 codewords (maximum 6)", 1, "" },
        /* 34*/ { UNICODE_MODE, -1, 4, 1, "点", ZINT_WARN_NONCOMPLIANT, 0, 7, 43, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC H, version 1, 1 kanji */
        /* 35*/ { UNICODE_MODE, -1, 4, 1, "点茗", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 1 R7x43-H, requires 4 codewords (maximum 3)", 1, "" },
        /* 36*/ { UNICODE_MODE, -1, 2, 1, "点茗点", ZINT_WARN_NONCOMPLIANT, 0, 7, 43, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC M, version 1, 3 kanji */
        /* 37*/ { UNICODE_MODE, -1, 2, 1, "点茗点茗", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 1 R7x43-M, requires 8 codewords (maximum 6)", 1, "" },
        /* 38*/ { UNICODE_MODE, -1, 4, 7, "12345678901234567890123", 0, 0, 9, 59, "", 1, "" }, /* Max capacity ECC H, version 7 (R9x59), 23 numbers */
        /* 39*/ { UNICODE_MODE, -1, 4, 7, "123456789012345678901234", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 7 R9x59-H, requires 12 codewords (maximum 11)", 1, "" },
        /* 40*/ { UNICODE_MODE, -1, 4, 7, "点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 9, 59, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC H, version 7, 6 kanji */
        /* 41*/ { UNICODE_MODE, -1, 4, 7, "点茗点茗点茗点", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 7 R9x59-H, requires 13 codewords (maximum 11)", 1, "" },
        /* 42*/ { UNICODE_MODE, -1, 4, 13, "点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 11, 59, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC H, version 13 (R11x59), 8 kanji */
        /* 43*/ { UNICODE_MODE, -1, 4, 13, "点茗点茗点茗点茗点", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 13 R11x59-H, requires 16 codewords (maximum 15)", 1, "" },
        /* 44*/ { UNICODE_MODE, -1, 4, 20, "点茗点茗点茗点茗点茗点茗点茗点茗点", ZINT_WARN_NONCOMPLIANT, 0, 13, 77, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC H, version 20 (R13x77), 17 kanji */
        /* 45*/ { UNICODE_MODE, -1, 4, 20, "点茗点茗点茗点茗点茗点茗点茗点茗点茗", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 20 R13x77-H, requires 31 codewords (maximum 29)", 1, "" },
        /* 46*/ { UNICODE_MODE, -1, 4, 26, "点茗点茗点茗点茗点茗点茗点茗点茗点点茗点茗点茗点点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 15, 99, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC H, version 26 (R15x99), 28 kanji */
        /* 47*/ { UNICODE_MODE, -1, 4, 26, "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 26 R15x99-H, requires 49 codewords (maximum 48)", 1, "" },
        /* 48*/ { UNICODE_MODE, -1, 4, 32, "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 17, 139, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC H, version 32 (R17x139), 46 kanji */
        /* 49*/ { UNICODE_MODE, -1, 4, 32, "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 578: Input too long for ECC level H, requires 78 codewords (maximum 76)", 1, "" },
        /* 50*/ { UNICODE_MODE, -1, -1, 32, "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 17, 139, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* Max capacity ECC M, version 32, 92 kanji */
        /* 51*/ { UNICODE_MODE, -1, 4, 32, "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 578: Input too long for ECC level H, requires 153 codewords (maximum 76)", 1, "" },
        /* 52*/ { UNICODE_MODE, -1, -1, 33, "点茗点", ZINT_WARN_NONCOMPLIANT, 0, 7, 43, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC auto-set to M, version 33 (R7xAuto-width) auto-sets R7x43 */
        /* 53*/ { UNICODE_MODE, -1, 4, 33, "点茗点", ZINT_WARN_NONCOMPLIANT, 0, 7, 59, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, version 33 (R7xAuto-width) auto-sets R7x59 */
        /* 54*/ { UNICODE_MODE, -1, 4, 33, "点茗点茗点茗点点茗点茗点茗点点", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 33 R7xW-H, requires 26 codewords (maximum 24)", 1, "" },
        /* 55*/ { UNICODE_MODE, -1, -1, 34, "点茗点", ZINT_WARN_NONCOMPLIANT, 0, 9, 43, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC auto-set to H, version 34 (R9xAuto-width) auto-sets R9x43 */
        /* 56*/ { UNICODE_MODE, -1, -1, 35, "点茗点", ZINT_WARN_NONCOMPLIANT, 0, 11, 27, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC auto-set to M, version 35 (R11xAuto-width) auto-sets R11x27 */
        /* 57*/ { UNICODE_MODE, -1, 4, 35, "点茗点茗点茗点", ZINT_WARN_NONCOMPLIANT, 0, 11, 59, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, version 35 (R11xAuto-width) auto-sets R11x59 */
        /* 58*/ { UNICODE_MODE, -1, -1, 35, "点茗点茗点茗点", ZINT_WARN_NONCOMPLIANT, 0, 11, 43, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC auto-set to M, version 35 (R11xAuto-width) auto-sets R11x43 */
        /* 59*/ { UNICODE_MODE, -1, -1, 36, "点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 13, 43, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC auto-set to M, version 36 (R13xAuto-width) auto-sets R13x43 */
        /* 60*/ { UNICODE_MODE, -1, 4, 36, "点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 13, 59, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, version 36 (R13xAuto-width) auto-sets R13x59 */
        /* 61*/ { UNICODE_MODE, -1, -1, 37, "点茗点茗点茗点茗点", ZINT_WARN_NONCOMPLIANT, 0, 15, 43, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC auto-set to M, version 37 (R15xAuto-width) auto-sets R15x43 */
        /* 62*/ { UNICODE_MODE, -1, 4, 37, "点茗点茗点茗点茗点", ZINT_WARN_NONCOMPLIANT, 0, 15, 59, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, version 37 (R15xAuto-width) auto-sets R15x59 */
        /* 63*/ { UNICODE_MODE, -1, 4, 37, "点茗点茗点茗点茗点点茗点茗点茗点茗点点茗点茗点茗点茗点点茗点茗点茗点茗点点茗点茗点茗", ZINT_ERROR_TOO_LONG, -1, 0, 0, "Error 560: Input too long for Version 37 R15xW-H, requires 70 codewords (maximum 69)", 1, "" },
        /* 64*/ { UNICODE_MODE, -1, -1, 38, "点茗点茗点茗点茗点茗点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 17, 43, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC auto-set to M, version 38 (R17xAuto-width) auto-sets R17x43 */
        /* 65*/ { UNICODE_MODE, -1, 4, 38, "点茗点茗点茗点茗点茗点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 17, 77, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, version 38 (R17xAuto-width) auto-sets R17x77 */
        /* 66*/ { UNICODE_MODE, -1, -1, 39, "点茗点", ZINT_ERROR_INVALID_OPTION, -1, 0, 0, "Error 579: Version '39' out of range (1 to 38)", 1, "" },
        /* 67*/ { UNICODE_MODE, -1, 4, -1, "点茗点", ZINT_WARN_NONCOMPLIANT, 0, 13, 27, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, auto-sets R13x27 */
        /* 68*/ { UNICODE_MODE, -1, 4, -1, "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 15, 99, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, auto-sets R15x99 (max capacity) */
        /* 69*/ { UNICODE_MODE, -1, 4, -1, "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", ZINT_WARN_NONCOMPLIANT, 0, 17, 99, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, auto-sets R17x99 */
        /* 70*/ { UNICODE_MODE, -1, 4, -1, "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", ZINT_WARN_NONCOMPLIANT, 0, 17, 139, "Warning 760: Converted to Shift JIS but no ECI specified", 1, "" }, /* ECC set to H, auto-sets R17x139 (max capacity) */
        /* 71*/ { GS1_MODE, 3, -1, -1, "[20]12", ZINT_WARN_NONCOMPLIANT, 0, 11, 27, "Warning 757: Using ECI in GS1 mode not supported by GS1 standards", 1, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_rmqr_options", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);

            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                if (!data[i].zxingcpp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    int cmp_len, ret_len;
                    char modules_dump[17 * 139 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int ret;
        int expected_eci;
        char *expected;
        int bwipp_cmp;
        int zxingcpp_cmp;
        char *comment;
    };
    /* See test_qr_input() for details about test characters */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, 4, 11, -1, "é", 0, 0, "67 A4 00 EC 11", 1, 1, "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, 4, 11, -1, "é", 0, 3, "E0 6C F4 80 EC", 1, 1, "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 20, -1, -1, -1, "é", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "é not in Shift JIS" },
        /*  3*/ { UNICODE_MODE, 26, 4, 11, -1, "é", 0, 26, "E3 4D 61 D4 80", 1, 1, "ECI-26 B2 (UTF-8)" },
        /*  4*/ { DATA_MODE, 0, 4, 11, -1, "é", 0, 0, "6B 0E A4 00 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /*  5*/ { DATA_MODE, 0, 4, 11, -1, "\351", 0, 0, "67 A4 00 EC 11", 1, 1, "B1 (ISO 8859-1)" },
        /*  6*/ { UNICODE_MODE, 0, 4, 11, -1, "β", ZINT_WARN_NONCOMPLIANT, 0, "Warning 88 80 00 EC 11", 1, 1, "K1 (Shift JIS)" },
        /*  7*/ { UNICODE_MODE, 9, 4, 11, -1, "β", 0, 9, "E1 2C F1 00 EC", 1, 1, "ECI-9 B1 (ISO 8859-7)" },
        /*  8*/ { UNICODE_MODE, 20, 4, 11, -1, "β", 0, 20, "E2 91 10 00 EC", 1, 1, "ECI-20 K1 (Shift JIS)" },
        /*  9*/ { UNICODE_MODE, 26, 4, 11, -1, "β", 0, 26, "E3 4D 67 59 00", 1, 1, "ECI-26 B2 (UTF-8)" },
        /* 10*/ { DATA_MODE, 0, 4, 11, -1, "β", 0, 0, "6B 3A C8 00 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 11*/ { UNICODE_MODE, 0, 4, 11, -1, "ก", ZINT_WARN_USES_ECI, 13, "Warning E1 AC D0 80 EC", 1, 1, "ECI-13 B1 (ISO 8859-11)" },
        /* 12*/ { UNICODE_MODE, 13, 4, 11, -1, "ก", 0, 13, "E1 AC D0 80 EC", 1, 1, "ECI-13 B1 (ISO 8859-11)" },
        /* 13*/ { UNICODE_MODE, 20, -1, -1, -1, "ก", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "ก not in Shift JIS" },
        /* 14*/ { UNICODE_MODE, 26, 2, 11, -1, "ก", 0, 26, "E3 4D F0 5C 40 80 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 15*/ { DATA_MODE, 0, 4, 11, -1, "ก", 0, 0, "6F 82 E2 04 00", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 16*/ { UNICODE_MODE, 0, 4, 11, -1, "Ж", ZINT_WARN_NONCOMPLIANT, 0, "Warning 88 91 C0 EC 11", 1, 1, "K1 (Shift JIS)" },
        /* 17*/ { UNICODE_MODE, 7, 4, 11, -1, "Ж", 0, 7, "E0 EC DB 00 EC", 1, 1, "ECI-7 B1 (ISO 8859-5)" },
        /* 18*/ { UNICODE_MODE, 20, 4, 11, -1, "Ж", 0, 20, "E2 91 12 38 EC", 1, 1, "ECI-20 K1 (Shift JIS)" },
        /* 19*/ { UNICODE_MODE, 26, 4, 11, -1, "Ж", 0, 26, "E3 4D 68 4B 00", 1, 1, "ECI-26 B2 (UTF-8)" },
        /* 20*/ { DATA_MODE, 0, 4, 11, -1, "Ж", 0, 0, "6B 42 58 00 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 21*/ { UNICODE_MODE, 0, 2, 11, -1, "ກ", ZINT_WARN_USES_ECI, 26, "Warning E3 4D F0 5D 40 80 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 22*/ { UNICODE_MODE, 20, -1, -1, -1, "ກ", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "ກ not in Shift JIS" },
        /* 23*/ { UNICODE_MODE, 26, 2, 11, -1, "ກ", 0, 26, "E3 4D F0 5D 40 80 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 24*/ { DATA_MODE, 0, 4, 11, -1, "ກ", 0, 0, "6F 82 EA 04 00", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 25*/ { UNICODE_MODE, 0, 4, 11, -1, "\\", 0, 0, "65 70 00 EC 11", 1, 1, "B1 (ASCII)" },
        /* 26*/ { UNICODE_MODE, 20, 4, 11, -1, "\\", 0, 20, "E2 91 00 F8 EC", 1, 1, "ECI-20 K1 (Shift JIS)" },
        /* 27*/ { UNICODE_MODE, 20, 4, 11, -1, "[", 0, 20, "E2 8C AD 80 EC", 1, 1, "B1 (ASCII)" },
        /* 28*/ { UNICODE_MODE, 20, 4, 11, -1, "\177", 0, 20, "E2 8C BF 80 EC", 1, 1, "ECI-20 B1 (ASCII)" },
        /* 29*/ { UNICODE_MODE, 0, 4, 11, -1, "¥", 0, 0, "66 94 00 EC 11", 1, 0, "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous); ZXing-C++ test can't handle it" },
        /* 30*/ { UNICODE_MODE, 3, 4, 11, -1, "¥", 0, 3, "E0 6C D2 80 EC", 1, 1, "ECI-3 B1 (ISO 8859-1)" },
        /* 31*/ { UNICODE_MODE, 20, 4, 11, -1, "¥", 0, 20, "E2 8C AE 00 EC", 1, 0, "ECI-20 B1 (Shift JIS) (to single-byte backslash codepoint 5C, so byte mode); ZXing-C++ test can't handle it" },
        /* 32*/ { UNICODE_MODE, 26, 4, 11, -1, "¥", 0, 26, "E3 4D 61 52 80", 1, 1, "ECI-26 B2 (UTF-8)" },
        /* 33*/ { DATA_MODE, 0, 4, 11, -1, "¥", 0, 0, "6B 0A 94 00 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 34*/ { UNICODE_MODE, 0, 4, 11, -1, "･", ZINT_WARN_NONCOMPLIANT, 0, "Warning 66 94 00 EC 11", 1, 1, "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /* 35*/ { UNICODE_MODE, 3, -1, -1, -1, "･", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 36*/ { UNICODE_MODE, 20, 4, 11, -1, "･", 0, 20, "E2 8C D2 80 EC", 1, 1, "ECI-20 B1 (Shift JIS) single-byte codepoint A5" },
        /* 37*/ { UNICODE_MODE, 26, 2, 11, -1, "･", 0, 26, "E3 4D F7 DE D2 80 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 38*/ { DATA_MODE, 0, 4, 11, -1, "･", 0, 0, "6F BE F6 94 00", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 39*/ { UNICODE_MODE, 0, 4, 11, -1, "¿", 0, 0, "66 FC 00 EC 11", 1, 0, "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous); ZXing-C++ test can't handle it" },
        /* 40*/ { UNICODE_MODE, 3, 4, 11, -1, "¿", 0, 3, "E0 6C DF 80 EC", 1, 1, "ECI-3 B1 (ISO 8859-1)" },
        /* 41*/ { UNICODE_MODE, 20, 4, 11, -1, "¿", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "¿ not in Shift JIS" },
        /* 42*/ { UNICODE_MODE, 26, 4, 11, -1, "¿", 0, 26, "E3 4D 61 5F 80", 1, 1, "ECI-26 B2 (UTF-8)" },
        /* 43*/ { DATA_MODE, 0, 4, 11, -1, "¿", 0, 0, "6B 0A FC 00 EC", 1, 0, "B2 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 44*/ { UNICODE_MODE, 0, 4, 11, -1, "ｿ", ZINT_WARN_NONCOMPLIANT, 0, "Warning 66 FC 00 EC 11", 1, 1, "B1 (Shift JIS) single-byte codepoint BF (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 45*/ { UNICODE_MODE, 3, 4, 11, -1, "ｿ", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 46*/ { UNICODE_MODE, 20, 4, 11, -1, "ｿ", 0, 20, "E2 8C DF 80 EC", 1, 1, "ECI-20 B1 (Shift JIS) single-byte codepoint BF" },
        /* 47*/ { UNICODE_MODE, 26, 2, 11, -1, "ｿ", 0, 26, "E3 4D F7 DE DF 80 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 48*/ { DATA_MODE, 0, 4, 11, -1, "ｿ", 0, 0, "6F BE F6 FC 00", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 49*/ { UNICODE_MODE, 0, 4, 11, -1, "~", 0, 0, "65 F8 00 EC 11", 1, 1, "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 50*/ { UNICODE_MODE, 3, 4, 11, -1, "~", 0, 3, "E0 6C BF 00 EC", 1, 1, "ECI-3 B1 (ASCII)" },
        /* 51*/ { UNICODE_MODE, 20, 4, 11, -1, "~", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input", 1, 1, "tilde not in Shift JIS (codepoint used for overline)" },
        /* 52*/ { UNICODE_MODE, 0, 4, 11, -1, "‾", ZINT_WARN_NONCOMPLIANT, 0, "Warning 65 F8 00 EC 11", 1, 0, "B1 (Shift JIS) single-byte codepoint 7E (same bytes as ~ ASCII above, so ambiguous); ZXing-C++ test can't handle it" },
        /* 53*/ { UNICODE_MODE, 3, 4, 11, -1, "‾", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 54*/ { UNICODE_MODE, 20, 4, 11, -1, "‾", 0, 20, "E2 8C BF 00 EC", 1, 0, "ECI-20 B1 (Shift JIS) (to single-byte tilde codepoint 7E, so byte mode); ZXing-C++ test can't handle it" },
        /* 55*/ { UNICODE_MODE, 26, 2, 11, -1, "‾", 0, 26, "E3 4D F1 40 5F 00 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 56*/ { DATA_MODE, 0, 4, 11, -1, "‾", 0, 0, "6F 8A 02 F8 00", 1, 0, "B3 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 57*/ { UNICODE_MODE, 0, 4, 11, -1, "点", ZINT_WARN_NONCOMPLIANT, 0, "Warning 8B 67 C0 EC 11", 1, 1, "K1 (Shift JIS)" },
        /* 58*/ { UNICODE_MODE, 3, 4, 11, -1, "点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 59*/ { UNICODE_MODE, 20, 4, 11, -1, "点", 0, 20, "E2 91 6C F8 EC", 1, 1, "ECI-20 K1 (Shift JIS)" },
        /* 60*/ { UNICODE_MODE, 26, 2, 11, -1, "点", 0, 26, "E3 4D F3 C1 5C 80 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 61*/ { DATA_MODE, 0, 4, 11, -1, "点", 0, 0, "6F 9E 0A E4 00", 1, 0, "B3 (UTF-8; ZXing-C++ test can't handle DATA_MODE for certain inputs)" },
        /* 62*/ { DATA_MODE, 0, 4, 11, -1, "\223\137", 0, 0, "6A 4D 7C 00 EC", 0, 0, "B2 (Shift JIS) (not full multibyte); BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 63*/ { DATA_MODE, 0, 4, 11, ZINT_FULL_MULTIBYTE, "\223\137", 0, 0, "8B 67 C0 EC 11", 1, 1, "K1 (Shift JIS)" },
        /* 64*/ { UNICODE_MODE, 0, 4, 11, -1, "¥･点", ZINT_WARN_NONCOMPLIANT, 0, "Warning 71 72 96 4D 7C", 1, 0, "B4 (Shift JIS) (optimized to byte mode only); ZXing-C++ test can't handle it" },
        /* 65*/ { UNICODE_MODE, 3, -1, -1, -1, "¥･点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input for ECI '3'", 1, 1, "" },
        /* 66*/ { UNICODE_MODE, 20, 2, 11, -1, "¥･点", 0, 20, "E2 8E 2E 52 C9 AF 80", 1, 0, "ECI-20 B4 (Shift JIS); ZXing-C++ test can't handle it" },
        /* 67*/ { UNICODE_MODE, 26, 2, 17, -1, "¥･点", 0, 26, "E3 4E 30 A9 7B EF 69 79 E0 AE 40 EC", 1, 1, "ECI-26 B8 (UTF-8)" },
        /* 68*/ { DATA_MODE, 0, 4, 11, -1, "\134\245\223\137", 0, 0, "71 72 96 4D 7C", 1, 0, "B8 (Shift JIS; ZXing-C++ test can't handle DATA_MODE for certain inputs)" },
        /* 69*/ { DATA_MODE, 0, 2, 17, -1, "¥･点", 0, 0, "71 85 4B DF 7B 4B CF 05 72 00 EC 11", 1, 0, "B8 (UTF-8); ZXing-C++ test can't handle DATA_MODE for certain inputs" },
        /* 70*/ { UNICODE_MODE, 0, 4, 11, -1, "点茗", ZINT_WARN_NONCOMPLIANT, 0, "Warning 93 67 F5 54 00", 1, 1, "K2 (Shift JIS)" },
        /* 71*/ { UNICODE_MODE, 0, 2, 12, -1, "点茗テ点茗テｿ", ZINT_WARN_NONCOMPLIANT, 0, "Warning 8C D9 FD 55 06 95 B3 FA AA 0D 2B 0D F8 EC 11 EC 11 EC 11", 1, 1, "K6 B1 (Shift JIS)" },
        /* 72*/ { DATA_MODE, 0, 2, 12, -1, "\223\137\344\252\203\145\223\137\344\252\203\145\277", 0, 0, "6D 93 5F E4 AA 83 65 93 5F E4 AA 83 65 BF 00 EC 11 EC 11", 0, 0, "B13 (Shift JIS); BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below); ZXing-C++ test can't handle it" },
        /* 73*/ { DATA_MODE, 0, 2, 12, ZINT_FULL_MULTIBYTE, "\223\137\344\252\203\145\223\137\344\252\203\145\277", 0, 0, "8C D9 FD 55 06 95 B3 FA AA 0D 2B 0D F8 EC 11 EC 11 EC 11", 1, 1, "K6 B1 (Shift JIS) (full multibyte)" },
        /* 74*/ { UNICODE_MODE, 0, 4, 11, -1, "áA", 0, 0, "6B 85 04 00 EC", 0, 1, "B2 (ISO 8859-1); BWIPP uses Kanji (ZINT_FULL_MULTIBYTE) mode, see below)" },
        /* 75*/ { UNICODE_MODE, 0, 4, 11, ZINT_FULL_MULTIBYTE, "áA", 0, 0, "8E 00 40 EC 11", 1, 1, "K1 (ISO 8859-1) (full multibyte)" },
        /* 76*/ { UNICODE_MODE, 0, 2, 23, -1, "A0B1C2D3E4F5G6H7I8J9KLMNOPQRSTUVWXYZ $%*+-./:", 0, 0, "(33) 56 9C 23 E0 87 92 62 7A 55 0B 59 82 33 26 C0 E6 5F AC 51 95 B4 26 B2 DC 1C 3B 9E 76", 1, 1, "A45" },
        /* 77*/ { UNICODE_MODE, 0, 4, 11, -1, "˘", ZINT_WARN_USES_ECI, 4, "Warning E0 8C D1 00 EC", 1, 1, "ECI-4 B1 (ISO 8859-2)" },
        /* 78*/ { UNICODE_MODE, 4, 4, 11, -1, "˘", 0, 4, "E0 8C D1 00 EC", 1, 1, "ECI-4 B1 (ISO 8859-2)" },
        /* 79*/ { UNICODE_MODE, 0, 4, 11, -1, "Ħ", ZINT_WARN_USES_ECI, 5, "Warning E0 AC D0 80 EC", 1, 1, "ECI-5 B1 (ISO 8859-3)" },
        /* 80*/ { UNICODE_MODE, 5, 4, 11, -1, "Ħ", 0, 5, "E0 AC D0 80 EC", 1, 1, "ECI-5 B1 (ISO 8859-3)" },
        /* 81*/ { UNICODE_MODE, 0, 4, 11, -1, "ĸ", ZINT_WARN_USES_ECI, 6, "Warning E0 CC D1 00 EC", 1, 1, "ECI-6 B1 (ISO 8859-4)" },
        /* 82*/ { UNICODE_MODE, 6, 4, 11, -1, "ĸ", 0, 6, "E0 CC D1 00 EC", 1, 1, "ECI-6 B1 (ISO 8859-4)" },
        /* 83*/ { UNICODE_MODE, 0, 4, 11, -1, "Ș", ZINT_WARN_USES_ECI, 18, "Warning E2 4C D5 00 EC", 1, 1, "ECI-18 B1 (ISO 8859-16)" },
        /* 84*/ { UNICODE_MODE, 18, 4, 11, -1, "Ș", 0, 18, "E2 4C D5 00 EC", 1, 1, "ECI-18 B1 (ISO 8859-16)" },
        /* 85*/ { UNICODE_MODE, 0, 4, 11, -1, "テ", ZINT_WARN_NONCOMPLIANT, 0, "Warning 88 69 40 EC 11", 1, 1, "K1 (SHIFT JIS)" },
        /* 86*/ { UNICODE_MODE, 20, 4, 11, -1, "テ", 0, 20, "E2 91 0D 28 EC", 1, 1, "ECI-20 K1 (SHIFT JIS)" },
        /* 87*/ { UNICODE_MODE, 20, 2, 11, -1, "テテ", 0, 20, "E2 92 0D 28 69 40 EC", 1, 1, "ECI-20 K2 (SHIFT JIS)" },
        /* 88*/ { UNICODE_MODE, 20, 2, 11, -1, "\\\\", 0, 20, "E2 92 00 F8 07 C0 EC", 1, 1, "ECI-20 K2 (SHIFT JIS)" },
        /* 89*/ { UNICODE_MODE, 0, 4, 11, -1, "…", ZINT_WARN_NONCOMPLIANT, 0, "Warning 88 08 C0 EC 11", 1, 1, "K1 (SHIFT JIS)" },
        /* 90*/ { UNICODE_MODE, 21, 4, 11, -1, "…", 0, 21, "E2 AC C2 80 EC", 1, 1, "ECI-21 B1 (Win 1250)" },
        /* 91*/ { UNICODE_MODE, 0, 4, 11, -1, "Ґ", ZINT_WARN_USES_ECI, 22, "Warning E2 CC D2 80 EC", 1, 1, "ECI-22 B1 (Win 1251)" },
        /* 92*/ { UNICODE_MODE, 22, 4, 11, -1, "Ґ", 0, 22, "E2 CC D2 80 EC", 1, 1, "ECI-22 B1 (Win 1251)" },
        /* 93*/ { UNICODE_MODE, 0, 4, 11, -1, "˜", ZINT_WARN_USES_ECI, 23, "Warning E2 EC CC 00 EC", 1, 1, "ECI-23 B1 (Win 1252)" },
        /* 94*/ { UNICODE_MODE, 23, 4, 11, -1, "˜", 0, 23, "E2 EC CC 00 EC", 1, 1, "ECI-23 B1 (Win 1252)" },
        /* 95*/ { UNICODE_MODE, 24, 4, 11, -1, "پ", 0, 24, "E3 0C C0 80 EC", 1, 1, "ECI-24 B1 (Win 1256)" },
        /* 96*/ { UNICODE_MODE, 0, 2, 11, -1, "က", ZINT_WARN_USES_ECI, 26, "Warning E3 4D F0 C0 40 00 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /* 97*/ { UNICODE_MODE, 25, 4, 11, -1, "က", 0, 25, "E3 2D 08 00 00", 1, 1, "ECI-25 B2 (UCS-2BE)" },
        /* 98*/ { UNICODE_MODE, 25, 2, 11, -1, "ကက", 0, 25, "E3 2E 08 00 08 00 00", 1, 1, "ECI-25 B4 (UCS-2BE)" },
        /* 99*/ { UNICODE_MODE, 25, 2, 11, -1, "12", 0, 25, "E3 2E 00 18 80 19 00", 1, 1, "ECI-25 B4 (UCS-2BE ASCII)" },
        /*100*/ { UNICODE_MODE, 27, 4, 11, -1, "@", 0, 27, "E3 6C A0 00 EC", 1, 1, "ECI-27 B1 (ASCII)" },
        /*101*/ { UNICODE_MODE, 0, 2, 11, -1, "龘", ZINT_WARN_USES_ECI, 26, "Warning E3 4D F4 DF 4C 00 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /*102*/ { UNICODE_MODE, 28, 4, 11, -1, "龘", 0, 28, "E3 8D 7C EA 80", 1, 1, "ECI-28 B2 (Big5)" },
        /*103*/ { UNICODE_MODE, 28, 2, 11, -1, "龘龘", 0, 28, "E3 8E 7C EA FC EA 80", 1, 1, "ECI-28 B4 (Big5)" },
        /*104*/ { UNICODE_MODE, 0, 2, 11, -1, "齄", ZINT_WARN_USES_ECI, 26, "Warning E3 4D F4 DE C2 00 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /*105*/ { UNICODE_MODE, 29, 4, 11, -1, "齄", 0, 29, "E3 AD 7B FF 00", 1, 1, "ECI-29 B2 (GB 2312)" },
        /*106*/ { UNICODE_MODE, 29, 2, 11, -1, "齄齄", 0, 29, "E3 AE 7B FF 7B FF 00", 1, 1, "ECI-29 B4 (GB 2312)" },
        /*107*/ { UNICODE_MODE, 0, 2, 11, -1, "가", ZINT_WARN_USES_ECI, 26, "Warning E3 4D F5 58 40 00 EC", 1, 1, "ECI-26 B3 (UTF-8)" },
        /*108*/ { UNICODE_MODE, 30, 4, 11, -1, "가", 0, 30, "E3 CD 58 50 80", 1, 1, "ECI-30 B2 (EUC-KR)" },
        /*109*/ { UNICODE_MODE, 30, 2, 11, -1, "가가", 0, 30, "E3 CE 58 50 D8 50 80", 1, 1, "ECI-30 B4 (EUC-KR)" },
        /*110*/ { UNICODE_MODE, 170, 4, 11, -1, "?", 0, 170, "F0 15 4C 9F 80", 1, 1, "ECI-170 B1 (ASCII invariant)" },
        /*111*/ { DATA_MODE, 899, 4, 11, -1, "\200", 0, 899, "F0 70 6C C0 00", 1, 1, "ECI-899 B1 (8-bit binary)" },
        /*112*/ { UNICODE_MODE, 900, 2, 11, -1, "é", 0, 900, "F0 70 8D 61 D4 80 EC", 1, 1, "ECI-900 B2 (no conversion)" },
        /*113*/ { UNICODE_MODE, 16384, 2, 11, -1, "é", 0, 16384, "F8 08 00 0D 61 D4 80", 1, 1, "ECI-16384 B2 (no conversion)" },
        /*114*/ { UNICODE_MODE, 3, 2, 14, -1, "Google Pixel 4a 128 GB Black;price:$439.97", 0, 3, "(43) E0 6C F4 76 F6 F6 76 C6 52 05 06 97 86 56 C2 03 46 14 4E 55 0C 59 91 09 96 CA 6C 61", 0, 1, "ECI-3 B15 A9 B10 A8; BWIPP different encodation (B42) & doesn't fit)" },
        /*115*/ { UNICODE_MODE, 29, 2, 20, -1, "Google 谷歌 Pixel 4a 128 GB黑色;零售价:￥3149.79", 0, 29, "(53) E3 AE 94 76 F6 F6 76 C6 52 0B 9C 8B 8E 82 05 06 97 86 56 C2 03 46 12 03 13 23 82 04", 1, 1, "ECI-29 B41 A7" },
        /*116*/ { UNICODE_MODE, 17, 2, 24, -1, "Google Pixel 4a 128 GB Schwarz;Preis:444,90 €", 0, 17, "(48) E2 2C F4 76 F6 F6 76 C6 52 05 06 97 86 56 C2 03 46 14 27 2A 86 2C C8 84 DC 6A B1 B4", 0, 1, "ECI-17 B15 A9 B21; BWIPP different encodation (B46)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_rmqr_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, %s, \"%s\", %s, %d, \"%s\", %d, %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci,
                    data[i].option_1, data[i].option_2, testUtilOption3Name(BARCODE_RMQR, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), ret < ZINT_ERROR ? symbol->eci : -1,
                    symbol->errtxt, data[i].bwipp_cmp, data[i].zxingcpp_cmp, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        char modules_dump[17 * 139 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    if (!data[i].zxingcpp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not ZXing-C++ compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[17 * 139 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_gs1(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { GS1_MODE, "[01]12345678901231", 0, "A6 00 59 D5 1B EF 43 D8 80 EC 11 EC", "N16" },
        /*  1*/ { GS1_MODE | GS1PARENS_MODE, "(01)12345678901231", 0, "A6 00 59 D5 1B EF 43 D8 80 EC 11 EC", "N16" },
        /*  2*/ { GS1_MODE, "[01]04912345123459[15]970331[30]128[10]ABC123", 0, "A5 D0 29 EB 3A A0 05 67 93 F2 94 B4 B4 E2 4E AF 01 47 EC 17 86", "N29 A9" },
        /*  3*/ { GS1_MODE | GS1PARENS_MODE, "(01)04912345123459(15)970331(30)128(10)ABC123", 0, "A5 D0 29 EB 3A A0 05 67 93 F2 94 B4 B4 E2 4E AF 01 47 EC 17 86", "N29 A9" },
        /*  4*/ { GS1_MODE, "[91]12%[20]12", 0, "A4 9C 79 32 25 1D 24 32 48 00 EC 11", "N4 B2 N4" },
        /*  5*/ { GS1_MODE, "[91]123%[20]12", 0, "A4 BC 79 76 44 A3 A4 86 49 00 EC 11", "N5 B2 N4" },
        /*  6*/ { GS1_MODE, "[91]1234%[20]12", 0, "A4 DC 79 D4 C8 94 74 90 C9 20 EC 11", "N6 B2 N4" },
        /*  7*/ { GS1_MODE, "[91]12345%[20]12", 0, "A4 FC 79 D4 AC 89 47 49 0C 92 00 EC", "N7 B2 N4" },
        /*  8*/ { GS1_MODE, "[91]1A%[20]12", 0, "A8 E6 58 1B ED 49 89 0C 92 00 EC 11", "A6(7) N4" },
        /*  9*/ { GS1_MODE, "[91]%%[20]12", 0, "A4 56 D9 92 92 8E 92 19 24 00 EC 11", "N2 B3 N4" },
        /* 10*/ { GS1_MODE, "[91]%%%[20]12", 0, "A4 56 DA 12 92 92 8E 92 19 24 00 EC", "N2 B4 N4" },
        /* 11*/ { GS1_MODE, "[91]A%%%%12345678A%A", 0, "AC E7 26 28 24 A4 A4 A4 A4 81 ED C8 9C 88 7A 35 C0 EC 11", "B7 N8 A3(4)" },
        /* 12*/ { GS1_MODE, "[91]%23%%6789%%%34567%%%%234%%%%%", 0, "(33) A9 43 2D B5 02 EE D4 DA 84 54 B8 ED 4D A9 B5 08 55 66 1B 30 94 94 94 94 C8 CC D0 94", "A14(20) N5 B12" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_rmqr_gs1", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[17 * 139 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_optimize(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int ret;
        char *expected;
        int bwipp_cmp;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 4, 11, "1", 0, "22 20 EC 11 EC", 1, "N1" },
        /*  1*/ { UNICODE_MODE, 4, 11, "AAA", 0, "46 73 0A 00 EC", 1, "A3" },
        /*  2*/ { UNICODE_MODE, 2, 11, "0123456789", 0, "34 06 2B 35 34 80 EC", 1, "N10 (nayuki.io - pure numeric)" },
        /*  3*/ { UNICODE_MODE, 2, 11, "ABCDEF", 0, "4C 73 51 4A 85 00 EC", 1, "A6 (nayuki.io - pure alphanumeric)" },
        /*  4*/ { UNICODE_MODE, 4, 11, "wxyz", 0, "71 DD E1 E5 E8", 1, "B4 (nayuki.io - pure byte)" },
        /*  5*/ { UNICODE_MODE, 2, 20, "「魔法少女まどか☆マギカ」って、　ИАИ　ｄｅｓｕ　κα？", ZINT_WARN_NONCOMPLIANT, "Warning (53) 8E 80 D6 00 4F C0 57 6A B5 C2 B8 14 70 94 81 64 37 A1 8D 0C 50 0D 82 82 14 40 00 80", 1, "K29 (nayuki.io - pure kanji)" },
        /*  6*/ { UNICODE_MODE, 4, 11, "012A", 0, "48 00 43 20 EC", 1, "A4" },
        /*  7*/ { UNICODE_MODE, 4, 11, "0123A", 0, "28 06 1A 12 80", 1, "N4 A1 (nayuki.io - alpha/numeric)" },
        /*  8*/ { UNICODE_MODE, 4, 11, "0a", 0, "68 C1 84 00 EC", 1, "B2 (nayuki.io - numeric/byte)" },
        /*  9*/ { UNICODE_MODE, 4, 11, "01a", 0, "24 05 96 10 EC", 1, "N2 B1 (nayuki.io - numeric/byte)" },
        /* 10*/ { UNICODE_MODE, 4, 11, "ABCa", 0, "71 05 09 0D 84", 1, "A4 (nayuki.io - alphanumeric/byte)" },
        /* 11*/ { UNICODE_MODE, 2, 11, "ABCDa", 0, "48 73 51 4B 2C 20 EC", 1, "A4 B1 (same bits as B5)" },
        /* 12*/ { UNICODE_MODE, 2, 24, "THE SQUARE ROOT OF 2 IS 1.41421356237309504880168872420969807856967187537694807317667973799", 0, "(48) 46 A9 52 9A A0 D5 42 66 E6 F8 A1 4F 62 3E 56 CC D4 40 2B 98 2C F1 AB 19 2E A2 F8 61", 1, " A26 N65 (nayuki.io - alpha/numeric)" },
        /* 13*/ { UNICODE_MODE, 2, 21, "Golden ratio φ = 1.6180339887498948482045868343656381177203091798057628621354486227052604628189024497072072041893911374......", ZINT_WARN_NONCOMPLIANT, "Warning (73) 65 11 DB DB 19 19 5B 88 1C 98 5D 1A 5B C8 20 F4 C8 0F 48 0C 4B 8B 24 D4 10 FB 97 6E", 1, "B20 N100 A6 (nayuki.io - alpha/numeric/byte)" },
        /* 14*/ { UNICODE_MODE, 2, 18, "こんにちwa、世界！ αβγδ", ZINT_WARN_NONCOMPLIANT, "Warning 84 09 88 5C 42 92 13 F6 B7 76 18 14 19 0A 28 A4 58 14 92 08 40 FF 88 00 40 22 02", 1, "K4 B7 K4 (nayuki.io - kanji/european)" },
        /* 15*/ { UNICODE_MODE, 2, 32, "6547861663com.acme35584af52fa3-88d0-093b-6c14-b37ddafb59c528908608sg.com.dash.www05303790265903SG.COM.NETS46967004B2AE13344004SG.SGQR209710339382.6359SG8236HELLO FOO2517Singapore3272B815", 0, "(152) 20 AA 3B 12 29 8D 97 B1 B7 B6 97 30 B1 B6 B2 99 9A 9A 9C 1A 30 B3 1A 99 33 30 99 96", 1, "N10 B47 N9 B15 N14 A38 N12 A25 B8 A8 (nayuki.io - SGQR alpha/numeric/byte)" },
        /* 16*/ { UNICODE_MODE, 2, 27, "2004年大西洋颶風季是有以来造成人失惨重的大季之一，于2004年6月1日正式始，同年11月30日束，上的日期界定了一年中大多数气旋在大西洋形成的段lll ku", ZINT_WARN_NONCOMPLIANT, "Warning (127) 20 43 21 21 87 27 32 95 77 90 AD F0 33 D5 CF 0E BA 58 46 17 22 19 0C 62 5D 62 DB 14", 0, "N4 K24 N4 B6 K7 B6 K26 B6 (nayuki.io - kanji/byte/numeric, non-Shift JIS removed; BWIPP different encodation (N4 K24 N4 B26 K26 B6)" },
        /* 17*/ { UNICODE_MODE, 2, 17, "AB1234567A", 0, "4A 39 A0 BC 45 8E 72 8A 00 EC 11 EC", 1, "A10" },
        /* 18*/ { UNICODE_MODE, 2, 17, "AB12345678A", 0, "42 39 A5 03 DB 91 39 04 A0 EC 11 EC", 1, "A2 N8 A1" },
        /* 19*/ { UNICODE_MODE, 2, 11, "テaA1", ZINT_WARN_NONCOMPLIANT, "Warning 76 0D 95 85 04 C4 00", 1, "B4" },
        /* 20*/ { UNICODE_MODE, 2, 11, "テaAB1", ZINT_WARN_NONCOMPLIANT, "Warning 6E 0D 95 85 19 CD 04", 0, "B3 A3; BWIPP different encodation (B6)" },
        /* 21*/ { UNICODE_MODE, 2, 32, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機", ZINT_WARN_NONCOMPLIANT, "Warning (152) 82 0E A2 16 20 97 28 BD 02 C1 4F 09 12 61 08 04 A0 83 AA 3E 3D 43 4C 13 0D 68 73 1F", 1, "K8 N1 K8 N1 K10 N2 K33 N2 K16 N1 K7" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_rmqr_optimize", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, ZINT_FULL_MULTIBYTE, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, \"%s\", %s, \"%s\", %d, \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].bwipp_cmp, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, debug)) {
                if (!data[i].bwipp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    char modules_dump[17 * 139 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                }
            }
            if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                int cmp_len, ret_len;
                char modules_dump[17 * 139 + 1];
                assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                               i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_1;
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
    /* ISO/IEC 23941:2022 */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, 4, 11, ZINT_FULL_MULTIBYTE, "0123456", 0, 11, 27, 1, "ISO 23941 Annex I Figure I.2, R11x27-H, same",
                    "111111101010101010101010111"
                    "100000100110100001110100101"
                    "101110100001001111010011111"
                    "101110101111011011110001100"
                    "101110100101110111111001011"
                    "100000101110000100111110010"
                    "111111101111111110001011111"
                    "000000001111101011010010001"
                    "111100000010010100111110101"
                    "101010100110010100111010001"
                    "111010101010101010101011111"
                },
        /*  1*/ { UNICODE_MODE, -1, 4, 11, -1, "0123456", 0, 11, 27, 1, "ISO 23941 Annex I Figure I.2, R11x27-H, same",
                    "111111101010101010101010111"
                    "100000100110100001110100101"
                    "101110100001001111010011111"
                    "101110101111011011110001100"
                    "101110100101110111111001011"
                    "100000101110000100111110010"
                    "111111101111111110001011111"
                    "000000001111101011010010001"
                    "111100000010010100111110101"
                    "101010100110010100111010001"
                    "111010101010101010101011111"
                },
        /*  2*/ { UNICODE_MODE, -1, 2, 17, ZINT_FULL_MULTIBYTE, "12345678901234567890123456", 0, 13, 27, 1, "ISO 23941 6.2 Figure 1, R13x27-M, same",
                    "111111101010101010101010111"
                    "100000100001001100010011001"
                    "101110101100000011001110001"
                    "101110100110101100000100000"
                    "101110101110100110110110011"
                    "100000100011100011001011000"
                    "111111100100111111000011101"
                    "000000001010010101010001100"
                    "110101101011010110010011111"
                    "011001101010101111100010001"
                    "100000100111000111101010101"
                    "100011010010010100000010001"
                    "111010101010101010101011111"
                },
        /*  3*/ { UNICODE_MODE, -1, 2, 2, ZINT_FULL_MULTIBYTE, "0123456789012345", 0, 7, 59, 1, "ISO 23941 7.4.2 Numeric mode Example, R7x59-M, same codewords",
                    "11111110101010101011101010101010101010111010101010101010111"
                    "10000010101111011110100001100001100001101100100101100100101"
                    "10111010100100001011110010110000011110111110111100011011111"
                    "10111010110001100010010100111010101101101011111000110110001"
                    "10111010011001000011100111100000110010111011010111011010101"
                    "10000010101010110110100010111110010010101111101111110010001"
                    "11111110101010101011101010101010101010111010101010101011111"
                },
        /*  4*/ { UNICODE_MODE, -1, 2, 2, ZINT_FULL_MULTIBYTE, "AC-42", 0, 7, 59, 1, "ISO 23941 7.4.3 Alphanumeric mode Example, R7x59-M, same codewords",
                    "11111110101010101011101010101010101010111010101010101010111"
                    "10000010101111010010110011010101100000101011001111100100101"
                    "10111010100100100011100100111100011101111100011011111011111"
                    "10111010110001110110100001101110100111101110000010010110001"
                    "10111010011000110111111101110000110001111000100110111010101"
                    "10000010101010111110100011101110011011101101011010110010001"
                    "11111110101010101011101010101010101010111010101010101011111"
                },
        /*  5*/ { UNICODE_MODE, -1, -1, 1, ZINT_FULL_MULTIBYTE, "123456789012", 0, 7, 43, 1, "R7x43-M max numeric 12 digits",
                    "1111111010101010101011101010101010101010111"
                    "1000001001010111111010100000100011011000101"
                    "1011101010111000100111101101011010111111111"
                    "1011101001100110111000000001111111000010001"
                    "1011101000101100011111100110111110110010101"
                    "1000001011110111111110101101001010111010001"
                    "1111111010101010101011101010101010101011111"
                },
        /*  6*/ { UNICODE_MODE, -1, 4, 32, ZINT_FULL_MULTIBYTE, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", 0, 17, 139, 1, "R17x139-H max numeric 178 digits",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001011001111001111010110101111111110001100100101011011010101100000011101000010101101000110111100000111101010111011011000011100010001001"
                    "1011101010001001010100111011111010110101010101000101101110001111010001011001101110111001010011010101010100101011100001100100000011101011111"
                    "1011101010010100010101100110101001110011110111011110111001011000100100110010111101000101000111100010011000100100101000110111100110001010000"
                    "1011101001010110110010110101011110011101110111001011011100110010101111101110000011010000110001011011001001011001011000011110111101111100111"
                    "1000001010001101001110011100001111111000001001101011001011011101110100010001111100001111001000011001100011101000001111001000010010101001110"
                    "1111111011110001000111011111010111111010101011110111110111001111100011001110111101110110110010110100110111100101000111111001011100000010101"
                    "0000000010010000011010101010101101001101111011101100000001101001101101110100101011101100011111100100101111010100110001110111001000010110000"
                    "1110101110111001111110100101000010011011111000110111001110001001001101101111011001111000111111001011111010010111111110010111100000111001101"
                    "0101000110010110111000000000001111011000011111111100101011001010101111111000110100100001101111111011001111101000001110011100010101010000100"
                    "1001000000100010100001101111110100011001110001011010001111101010010110110101101110010011111100011100011010101001000001101000000111101010011"
                    "0011010010110011011010100010101010011000100010100101100010111110001110111010100110100101110010000100010000100110101000010001101110001011010"
                    "1000110001111011101101101101110110011110110110111110111110110001000111110010000110110000000011100010000101010111011010010111101001011011111"
                    "0010000000111010001110110110011000011010011101100010000011101001011101001011001011001011100010111101010111011000010111001001000010111110001"
                    "1100101100110100100101000011111110111000010001101001001111001110110100000111101011111001100101001000000011101011110100101001010100111110101"
                    "1010111110110100011001111010100011101111001101100101111011100101011100011011011000101010010000000100111000011010100000100111101001110110001"
                    "1110101010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
        /*  7*/ { UNICODE_MODE, -1, 4, 33, ZINT_FULL_MULTIBYTE, "123456789012345678901234567890123456789012345678901234", 0, 7, 139, 1, "R7xauto-H max numeric 54 digits",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001011010011001111100110101000111000110001101010101011100001010000000000011001101000110010111100010100100110101000010111100011001000001"
                    "1011101000011001010110011111111001010000100001011100101110111001110001011101010011111000110100100000100100111011100000000010010010101011111"
                    "1011101001110011010110011110000111010000111100110001101000100000110000000101101100000011000100100101101000000010100100101000110010000010001"
                    "1011101011101101111101010011101111111011101110010111101110110111111000111001010000111011011001100000111000001111111111101001100011001010101"
                    "1000001010001100101101000010110110010110000111100001011011001001011001111000100111101011001110101000010111101110101110000101110011110110001"
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
        /*  8*/ { UNICODE_MODE, -1, 4, 35, ZINT_FULL_MULTIBYTE, "123456789012345678901234567890123", 0, 11, 59, 1, "R11xauto-H with max numeric 33 digits for width 59",
                    "11111110101010101011101010101010101010111010101010101010111"
                    "10000010101001100110111000100110101100101100010101111110101"
                    "10111010000110100111101001001111110010111101000100110110011"
                    "10111010101110011100110101010011111101101001110010001101110"
                    "10111010001000101111000010010101001010011011101010011010001"
                    "10000010000110110010111111111010010110001110100010000110000"
                    "11111110011110011111011101001001101111010001001100001011111"
                    "00000000001111111110100100110100101111101111111000111010001"
                    "11011000100010010111110110111001011100111000100100010010101"
                    "10011101110110100010100011000110110010101001001100100110001"
                    "11101010101010101011101010101010101010111010101010101011111"
                },
        /*  9*/ { UNICODE_MODE, -1, -1, 38, ZINT_FULL_MULTIBYTE, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", 0, 17, 139, 1, "R17xauto-M max numeric 361 digits",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001001000101011111010110101000111111101000111100111011110101001110010101100011101100101011101111100000110010101010100000011111110110001"
                    "1011101011100111011000001111111010110000100000100110001111110101001001010011000110111101110100011101010101001111100001010000010111111111011"
                    "1011101001001100001001111100010100100011000110010110010001110110110101101000001110000111011000110010111111011010010110011101110100001001010"
                    "1011101000010101010111011001110010111100100101101001111100011100100111001101000001011010001000000010111001110001001000101111001000010000101"
                    "1000001000010001001011010010000000011001010001001001111000001000101010010111010101001001010111111101000011100010101111000011001010111001110"
                    "1111111001101010000100101101010011000001101011110111001111100011000000100110111000110100000111110001001010001011110100000000110111011100001"
                    "0000000010110010001111111010101001000000110001110110001011100011110111101000111110001101100100000011111100101100011001111000101001000100110"
                    "1110101100000110000111010011111100101011100100000000101100000010111010011000101011011100111000001100010101010001100011110101001000100000111"
                    "0111101111100011010010111110111010100101011100100101110001010101001010010101100000001000011011101100000000010010100110100000011111110110010"
                    "1000000010110100110011010001111100011111010101100000011111011101000101010011000101110101010111011011100001101101011101000000010111110011011"
                    "0001011000101100111101110000000000001111001110001000000010001010111101101000001110000111011011010011111111111000011010011101110100011011110"
                    "1110010100011011010101000111110001000000010100011111000100100000110111001101000001111010001000100110111110100001110100101111001000110111111"
                    "0111111000011110110011110000010100000010011111010011011011111101011010010111010101001001010111100100001011100110101101000011001010000110001"
                    "1010111000111001011010111011111111010100010000000001111111011111100000100110111000111000000111101001000000001111100111000000110111110110101"
                    "1010100110101011001011101010100000110111001110010011101011010011100100101000111010101110101100010001011100100010101010111100100001000110001"
                    "1110101010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
        /* 10*/ { UNICODE_MODE, -1, 2, 1, ZINT_FULL_MULTIBYTE, "\001\002\003\004\005", 0, 7, 43, 1, "R7x43-M with max 5 binary",
                    "1111111010101010101011101010101010101010111"
                    "1000001001010001101010101011001111011000101"
                    "1011101010111101100111100101110001011111111"
                    "1011101001101111011110011001101001000010001"
                    "1011101000100000100111110110101110110010101"
                    "1000001011110000000010101011001111011010001"
                    "1111111010101010101011101010101010101011111"
                },
        /* 11*/ { UNICODE_MODE, -1, 4, 1, ZINT_FULL_MULTIBYTE, "\001\002", 0, 7, 43, 1, "R7x43-H with max 2 binary",
                    "1111111010101010101011101010101010101010111"
                    "1000001011010010110010111001001111001000001"
                    "1011101011010011101011110001000001010011111"
                    "1011101010111101110110001010001001011010001"
                    "1011101001111011011011111111001110011010101"
                    "1000001001111110100010111110110110101010001"
                    "1111111010101010101011101010101010101011111"
                },
        /* 12*/ { UNICODE_MODE, -1, 2, 1, ZINT_FULL_MULTIBYTE, "ABCDEFG", 0, 7, 43, 1, "R7x43-M with max 7 alphanumerics",
                    "1111111010101010101011101010101010101010111"
                    "1000001001011100111110111000000101011000101"
                    "1011101010111010101011110111101001011111111"
                    "1011101001100001111100011111100010100010001"
                    "1011101000110011100011111110010100110010101"
                    "1000001011111011101110111110000111111010001"
                    "1111111010101010101011101010101010101011111"
                },
        /* 13*/ { UNICODE_MODE, -1, 2, 1, ZINT_FULL_MULTIBYTE, "点茗点", ZINT_WARN_NONCOMPLIANT, 7, 43, 1, "R7x43-M with max 3 Kanji",
                    "1111111010101010101011101010101010101010111"
                    "1000001001011000100010101001010001111000101"
                    "1011101010110101110111111001011101111111111"
                    "1011101001100001101010100110100110100010001"
                    "1011101000100010000011101101011011110010101"
                    "1000001011111100010110101111011000111010001"
                    "1111111010101010101011101010101010101011111"
                },
        /* 14*/ { UNICODE_MODE, -1, 2, 4, ZINT_FULL_MULTIBYTE, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM", 0, 7, 99, 1, "R7x99-M with max 39 alphanumerics",
                    "111111101010101010101011101010101010101010101010111010101010101010101010101110101010101010101010111"
                    "100000100111110010010010101010100110101001000011101100101101000010101110001011100000010001111100101"
                    "101110100101000000101011100100010000001000001111111111011111000000110101111111101111100000100011111"
                    "101110100100011111000100010111001001100101111011100001000001001111101010111010110110100011000110001"
                    "101110101000101011110111101110010000110011110101111110100111100010101000101111101101101111000110101"
                    "100000101010111010111010111110101001111001101011101010101010001100101101111011100101101010110010001"
                    "111111101010101010101011101010101010101010101010111010101010101010101010101110101010101010101011111"
                },
        /* 15*/ { UNICODE_MODE, -1, 4, 5, ZINT_FULL_MULTIBYTE, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026", 0, 7, 139, 1, "R7x139-H with max 22 binary",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001011010100100110110110100110111000001010100110101010111101000011111010001000101100000100001011011110111110100110011110001101001000001"
                    "1011101000010011101011000111111011101001111011110101011110111001000010011111001110111001100001010101110110100011110011110001010001001011111"
                    "1011101001111010000010110010111010001000101110001010101000101110001111011000010111001101000001010101101001110100110011110010000001000010001"
                    "1011101011110101001101101111100101000110001011010111101111110111111011101010001110111100001101011100001010011111111110111010001101001010101"
                    "1000001010000100011110111110111000111001100010111001111011001011011111000110010101101111111111001110101110101010101110110010001100110110001"
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
        /* 16*/ { UNICODE_MODE, -1, 2, 7, ZINT_FULL_MULTIBYTE, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024", 0, 9, 59, 1, "R9x59-M with max 20 binary",
                    "11111110101010101011101010101010101010111010101010101010111"
                    "10000010100110111010110111101111000011101110010011100001001"
                    "10111010101100001011111100111100011111111100011001111100101"
                    "10111010001011110100100111110100100000000110011110000000110"
                    "10111010010001101101111101100001011011110011100011111111111"
                    "10000010000100101110010110010111100000100111010011101010001"
                    "11111110010100010111110011111110011101111100011000011110101"
                    "00000000000011111010111011010100000000101110011010000110001"
                    "11101010101010101011101010101010101010111010101010101011111"
                },
        /* 17*/ { UNICODE_MODE, -1, 2, 9, ZINT_FULL_MULTIBYTE, "12345678901234567890123456789012345678901234567", 0, 9, 99, 1, "R9x99-M with max 47 numerics",
                    "111111101010101010101011101010101010101010101010111010101010101010101010101110101010101010101010111"
                    "100000100010000101001110111001101000101110111000101110001100111010100000001011110010011100011001101"
                    "101110101011011101000011110000001000010010010000111011001001111111000001001110101111110010111000101"
                    "101110101010111000000000001110110110111110001100000001001111011011101101011011011100000001110110100"
                    "101110101111101010100111111010001100011000000110010100011010001111100010110100000110001000111111111"
                    "100000100110100000000110100110010010111001000101101000110011010101110000001010000011101010011010001"
                    "111111100100110111110011110010000010111111100111111100100110000100000111101110010011011110101010101"
                    "000000000111000111011010100110011010111011010011101110110100100111001011111010110011000000001010001"
                    "111010101010101010101011101010101010101010101010111010101010101010101010101110101010101010101011111"
                },
        /* 18*/ { UNICODE_MODE, -1, 2, 11, ZINT_FULL_MULTIBYTE, "\001\002\003\004\005\006", 0, 11, 27, 1, "R11x27-M with max 6 binary",
                    "111111101010101010101010111"
                    "100000101110010000111000101"
                    "101110100111010000000011001"
                    "101110100010000111110101110"
                    "101110100001011100111000001"
                    "100000100110111001101110110"
                    "111111100100001110000111111"
                    "000000000101011011001010001"
                    "111100001110011000010110101"
                    "101001111010101000101010001"
                    "111010101010101010101011111"
                },
        /* 19*/ { UNICODE_MODE, -1, 4, 11, ZINT_FULL_MULTIBYTE, "\001\002\003\004", 0, 11, 27, 1, "R11x27-H with max 4 binary",
                    "111111101010101010101010111"
                    "100000100110100100111000101"
                    "101110100001011000000011001"
                    "101110101111011111110101010"
                    "101110100100000010111000001"
                    "100000101110011001111110010"
                    "111111101101110100001011111"
                    "000000001011111100010010001"
                    "111011000110110000011110101"
                    "101000000010100100111010001"
                    "111010101010101010101011111"
                },
        /* 20*/ { UNICODE_MODE, -1, 2, 13, ZINT_FULL_MULTIBYTE, "12345678901234567890123456789012345678901234567890123456789012345678901", 0, 11, 59, 1, "R11x59-M with max 71 numerics",
                    "11111110101010101011101010101010101010111010101010101010111"
                    "10000010001010111110110110010100101111101111001110101100001"
                    "10111010011100010011110111110001111100111010111101000100011"
                    "10111010011001001000001110100101110100101001110000010010000"
                    "10111010011011101111111010100010101001111000011010010100001"
                    "10000010100111001000111011110110111001101100001001110110100"
                    "11111110010111010111001000011000101010110001001100000111111"
                    "00000000011100100100100000100011000100001111000000000010001"
                    "11111100110000101111110101101001000100111000101010011010101"
                    "10011100001011001110100101001100110111101001101110110110001"
                    "11101010101010101011101010101010101010111010101010101011111"
                },
        /* 21*/ { UNICODE_MODE, -1, 2, 18, ZINT_FULL_MULTIBYTE, "1234567890123456789012345678901234567890123456789012345678901", 0, 13, 43, 1, "R13x43-M with max 61 numerics",
                    "1111111010101010101011101010101010101010111"
                    "1000001011111001100110111000110110011001001"
                    "1011101011100001110011110110101000100010111"
                    "1011101011000100101010011001000010110111010"
                    "1011101010100100111011011011100011000111111"
                    "1000001001101001000000110110001000101101110"
                    "1111111010011101101101101010100100101000011"
                    "0000000011110010100110111001100100101101110"
                    "1111100101001011000001101100111011010111111"
                    "0011110001100101000000100000101100110110001"
                    "1011101111000010101011110011001100100010101"
                    "1000110000010111110010110001001011001010001"
                    "1110101010101010101011101010101010101011111"
                },
        /* 22*/ { UNICODE_MODE, -1, 4, 20, ZINT_FULL_MULTIBYTE, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033", 0, 13, 77, 1, "R13x77-H with max 27 binary",
                    "11111110101010101010101011101010101010101010101010111010101010101010101010111"
                    "10000010101100000001000010111011001111101101111101101110100010001011100001101"
                    "10111010010010101011110111100110000111101101101101111110000000011110111011011"
                    "10111010100101111101110100000000010011011010000000001100100110011010000001100"
                    "10111010000111001111011101001110010010110101111000110001101001100011101011111"
                    "10000010111010000011101000001111101111001010101001101100000001000011110011000"
                    "11111110110111111101110011101010100110011011111111011100001100110110011100101"
                    "00000000110110101010001000000000010111001011110110001100011000011100000101010"
                    "11111101010111111111100011100111100011111001111110111001100010010001100011111"
                    "01000111110100101000101100110111101100000100100001100110100001001011111110001"
                    "10001110011100111011101011110000110011101000011011111010001000011101010110101"
                    "10001001000010011011101110100010010001101111111101101000000110010000011010001"
                    "11101010101010101010101011101010101010101010101010111010101010101010101011111"
                },
        /* 23*/ { UNICODE_MODE, -1, 2, 20, ZINT_FULL_MULTIBYTE, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123", 0, 13, 77, 1, "R13x77-M with max 123 numerics (note was 8 bit cci in draft 2019-6-24 when 7 suffices - corrected in ISO/IEC 23941:2022)",
                    "11111110101010101010101011101010101010101010101010111010101010101010101010111"
                    "10000010001100001001010010111101110110110101101001101011011001000011001101101"
                    "10111010001011111111101011111110010001001001010111111010111100100101101011011"
                    "10111010010011001001111000101011110101101001011100100011110111011001010011110"
                    "10111010010010000101101011100011011110101010010010011000001101000010111011011"
                    "10000010011110010100000110001011110110000101101001100010101000000011101101010"
                    "11111110011111101010010101000001100001110001110111111111111000100101101011111"
                    "00000000010000010110000110000010010100101010010100100011101011011001010101110"
                    "11100011101100001111000101000000101100100001010010011110000001010010101111111"
                    "01010100110000011101011010011100101101000101101111001010100000110111000110001"
                    "10111001001100001100110011110011010111110001111111111011110000001001011110101"
                    "10100110000111111100100010100011100000011010110100101111001000011010001010001"
                    "11101010101010101010101011101010101010101010101010111010101010101010101011111"
                },
        /* 24*/ { UNICODE_MODE, -1, 2, 23, ZINT_FULL_MULTIBYTE, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 0, 15, 43, 1, "R15x43-M with max 31 binary",
                    "1111111010101010101011101010101010101010111"
                    "1000001011010010111010101000000011011011001"
                    "1011101011000000000011111001010001110001111"
                    "1011101000101100100010110000110001110001110"
                    "1011101010001110100001100010100011111100001"
                    "1000001011000001101010010100001111001010110"
                    "1111111011100000010101011001110001110001001"
                    "0000000010010111110000110010110001110001010"
                    "1100111001111100110001101010100000111000111"
                    "0100100011111110010000010100000111101001100"
                    "1010101010110000110001101011000001110011111"
                    "0110000110000110110010110001000001101010001"
                    "1100010010011100000011100110000010000110101"
                    "1001011111001000010010110100001011011110001"
                    "1110101010101010101011101010101010101011111"
                },
        /* 25*/ { UNICODE_MODE, -1, 4, 27, ZINT_FULL_MULTIBYTE, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", 0, 15, 139, 1, "R15x139-H with max 98 alphanumerics",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001000101010110111000110101110111011110000110111111011010110110101010100010000101101000101101000000110101010110111000011111101010011001"
                    "1011101001100101001100010011101100000011101101100110011111100111110110101101110101111001000011000100010000110111111101101011100000111011001"
                    "1011101011111100000110000100011011111010011111100001000000110111001010010001011010000100100101100001011110110010011011101000100100010011100"
                    "1011101010000010010111110001010010000101111011010110100110011101001100001111010101110001110100100011101010101111100110010011001001000101101"
                    "1000001000100000010010001000110011100110010001011011101011111000000010111001010010101101011110101111011010110000100111110010111111110100100"
                    "1111111010100000110011001001111101001101011011110010001100000100000101011011001110110110000000111000011101001011000111111011101000100111101"
                    "0000000011000010100010110000111000010011101010111100100010001001010011000110010101000000001100101001010001011010100010111011011111101111010"
                    "1100110111000001101100101011110001111111000010100000110110101001010011100001110100111010001100000100000000011001110111111000111010111101011"
                    "0100110000100000100100010000001011001010001000111001000001101101100100100011011110100111001111001000110000010100100110011000010010010111000"
                    "1100011110010111111010011001011101100001001001001111100101111011111011111110011000110100101110000110011010010101100001111001111011100111111"
                    "0001111000000010100011001000110111101111101111101010001010001000101100101101000001000010100000101111101000011100010010011111100011110010001"
                    "1100111110100000010000111011110010011100101001110010101110011110110110001100001101111001001110011000111010111011110011001010011001000110101"
                    "1011010110101111010001000110110100010001001001100010101011001000110010111101000000101111001000110110011001111010100011110110001000100010001"
                    "1110101010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
        /* 26*/ { UNICODE_MODE, -1, 2, 28, ZINT_FULL_MULTIBYTE, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, 17, 43, 1, "R17x43-M with max 90 numerics",
                    "1111111010101010101011101010101010101010111"
                    "1000001001000101001010111110000001010100101"
                    "1011101000101110110011110100001001100110011"
                    "1011101010001111111010101000010101101001000"
                    "1011101010001001010101011100110000100111111"
                    "1000001011111110111110000101001001010110000"
                    "1111111001111100101111110111011010000010111"
                    "0000000010011101100110011111010000101101010"
                    "1100111101010110111011011000110101001100111"
                    "0111000100100100011010000110001001101100010"
                    "1110110010011101000111101111000000010011011"
                    "0011001001011100110000101100100111111011110"
                    "1110111010101010010001111101011010001111111"
                    "0011110000011011010110001101001000111110001"
                    "1010000001001101001111101000100011000110101"
                    "1010110110101010011110101110010011011010001"
                    "1110101010101010101011101010101010101011111"
                },
        /* 27*/ { UNICODE_MODE, -1, 4, 29, ZINT_FULL_MULTIBYTE, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032", 0, 17, 59, 1, "R17x59-H with max 26 binary",
                    "11111110101010101011101010101010101010111010101010101010111"
                    "10000010111010100010101100011001100000101011110011110011101"
                    "10111010011000111011101011101011010010111000110100011100011"
                    "10111010101100010010101010011000101000000100011010000100000"
                    "10111010111100100101101000101000101000110110010001100011011"
                    "10000010110011101100010010111000101111100010111011100001100"
                    "11111110111111001101001001000101011110111100010111001111011"
                    "00000000100101101110101100000000101101101110010110001101100"
                    "11111011001011101111100001011101011101110010010011100111111"
                    "01111100111111000000111000001001010100000001000011110011000"
                    "11000001111111001011000010001010001011010000111011111010101"
                    "01100000111110100010100111100111001101101100010100001111010"
                    "11100011100001100111001001110100011101110000100001100111111"
                    "00111000001010001100000100010100100101100101111011111010001"
                    "11001011110100101111111000101000111011111100010100001010101"
                    "10001000111011111010100011010111101101101010011010011110001"
                    "11101010101010101011101010101010101010111010101010101011111"
                },
        /* 28*/ { UNICODE_MODE, -1, 2, 32, ZINT_FULL_MULTIBYTE, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 17, 139, 1, "R17x139-M with max 108 alphanumerics",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001001001010010000011110111001001010110101010100011010100010100111011001101000101010101100000100100110001010101000100111011010000101001"
                    "1011101011101001000100010111100111100100100101100010111110100010110000000011101010111111111001010011110000010011101011000101000000010001001"
                    "1011101001001010011011111000100110001100100000001101000001000100101101001100011111001001100011101000011011111100011000101100010110111010010"
                    "1011101000010011111100010001011011011101010000111000011111011101000101100100100101010000110110001101001010111101111100000110011111111111011"
                    "1000001000010100010000011000101101000110011001100110110010101011010011010101111010101010010100100110100000110110101111011011011011100101010"
                    "1111111010110001111001010111010110101110100000000000110100000011111001011111111010110111100111000011110101100001100011111001100001010001101"
                    "0000000011111011001111110010100100001100101010101101000011010110100100001100000010100111010100101010011111001000010110100100101100100100000"
                    "1100110101110001011110010111111000100011110010001010011111101100000110101100100011011100011011011000001010001101111100000110000111011110011"
                    "0101010100000101010000011010101100001001010100000000110001111111010011011010110010000010010111010001010000110110101011010001010110000100100"
                    "1001011101000001111001110101010101001100110111010001010100010111111001110000110001010101110010110000000001100011010011111000010001011010101"
                    "0011011000000111000111000100101000100110010001010111110010101110110100110110100110101101110001001110111111001000010110100101000000011011110"
                    "1110111001100101010110010111110100010000101110101010101110101100101110011111000010011100111000011110001011001101011100010100101100110111111"
                    "0100010111111011011100011000111100111111001001111000101010111111010010011011010011000000010010111101010000011000110001110010010101100110001"
                    "1001111110101110110000100111110101011101100010011001101111010111011000010000110101111000000110110100010011000111111100101110011011110110101"
                    "1011101011001101111111100110101100110110111011010111001011101011000100010101100000101110111100000010011100011010100100011000001100000110001"
                    "1110101010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_rmqr_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %d, %s, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_1, data[i].option_2,
                    testUtilOption3Name(BARCODE_RMQR, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[17 * 139 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_encode_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, 2, 11, -1, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 11, 27, 1, "Standard example",
                    "111111101010101010101010111"
                    "100000101110110001100111101"
                    "101110100111000001110101111"
                    "101110100010010000000110000"
                    "101110100000011010001111001"
                    "100000100110100000101110110"
                    "111111101001011111100111111"
                    "000000001011110111101010001"
                    "111010111010001000110110101"
                    "101111001111110001001010001"
                    "111010101010101010101011111"
                },
        /*  1*/ { UNICODE_MODE, 2, 11, -1, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 11, 27, 1, "Standard example auto-ECI",
                    "111111101010101010101010111"
                    "100000101110110001100111101"
                    "101110100111000001110101111"
                    "101110100010010000000110000"
                    "101110100000011010001111001"
                    "100000100110100000101110110"
                    "111111101001011111100111111"
                    "000000001011110111101010001"
                    "111010111010001000110110101"
                    "101111001111110001001010001"
                    "111010101010101010101011111"
                },
        /*  2*/ { UNICODE_MODE, 2, 11, -1, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 11, 27, 1, "Standard example inverted",
                    "111111101010101010101010111"
                    "100000101110111000010111101"
                    "101110100111011111110010001"
                    "101110100010111111001100000"
                    "101110100001001011110100001"
                    "100000100111111101101110100"
                    "111111100001000001000111111"
                    "000000001111000000101010001"
                    "111001110101001100010110101"
                    "101010011111101111101010001"
                    "111010101010101010101011111"
                },
        /*  3*/ { UNICODE_MODE, 2, 11, -1, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 11, 27, 1, "Standard example inverted auto-ECI",
                    "111111101010101010101010111"
                    "100000101110111000010111101"
                    "101110100111011111110010001"
                    "101110100010111111001100000"
                    "101110100001001011110100001"
                    "100000100111111101101110100"
                    "111111100001000001000111111"
                    "000000001111000000101010001"
                    "111001110101001100010110101"
                    "101010011111101111101010001"
                    "111010101010101010101011111"
                },
        /*  4*/ { UNICODE_MODE, 2, 32, -1, { { TU("Google Pixel 4a 128 GB Black;price:$439.97"), -1, 3 }, { TU("Google 谷歌 Pixel 4a 128 GB黑色;零售价:￥3149.79"), -1, 29 }, { TU("Google Pixel 4a 128 GB Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 17, 139, 0, "AIM ITS/04-023:2022 Annex A example (shortened); BWIPP different encodation",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001001000010100111001110100000111100110111111110001010100010000110011100101000101101010100111001001101110110110010000011110000101100101"
                    "1011101011101011111110110011100101100111101000001011111111110000100000111001000110111011000011100011000000100011101101100100110100110011011"
                    "1011101001001110000110100100001111011110111110111101000000100101001111110100111010000000011000101101110100000100011001000010001010101111110"
                    "1011101000010000111010100101101000100010100100110100011110111110001100101010001111110110111000000000101011100001101000001000110000000110111"
                    "1000001000001001100011001110100010110001011011011100110000111111011101011110010000000000000010001110010111000010001001110111010101011000110"
                    "1111111010100010010100110101101010100010001110001000100100011111111001000000111011111000110011111010000011011111100111011001000000011010001"
                    "0000000010010110010000100000000010101010110111110011010010110001100100001000010011000000110101111100010001000000100101100111011000011011110"
                    "1101110011111001110010000111111111101100100101001110001111101110010110101100001110110110001110100100111100000111111011110100100101011111001"
                    "0111111100100011001110011010010100110111101101010111000011001111100011011100001100100110011110110101111010101100010101100000010010110101000"
                    "1011000010101000110000001001010011101001100001111011001100010011011010010000101101110011000011010011011111110101101101100101110101100001001"
                    "0001110000010011000000110000010111111010010001010100100000101010000111111001110000000101100101110001010001010010011010001110101101111011100"
                    "1000101101011001001010000001111011100111100000100110101100101100110100100101001101110111101001100111001101111001111010001011011000110111111"
                    "0110100001110101010110010010100000111110011101011100100010111011000010010110001010000001001001001111000010011100111110010011000110100110001"
                    "1001000001100110110011100011111101010111100110100101011110010011101010100011101001111010100111010100110100011111110111100101000101010110101"
                    "1010011110111011011111000010111011001001010001011111011011101000100101110000010011101110110100011010110111010110110011110110000001000110001"
                    "1110101010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
        /*  5*/ { DATA_MODE, 4, 3, -1, { { TU("\266"), 1, 0 }, { TU("\266"), 1, 7 }, { TU("\266"), 1, 0 } }, 0, 7, 77, 1, "Standard example + extra seg, data mode",
                    "11111110101010101010101011101010101010101010101010111010101010101010101010111"
                    "10000010000101011111110110101011011101110110100011101011010110001000110000001"
                    "10111010000101001001111111110011100001100010110011111100010111010001101011111"
                    "10111010001111110101010000111011110111000111100010101100011110001110101010001"
                    "10111010100010101100010111101010000111111001010110111000010010000000100110101"
                    "10000010011011101100010110100010100010101101100001101010000100011011101010001"
                    "11111110101010101010101011101010101010101010101010111010101010101010101011111"
                },
        /*  6*/ { UNICODE_MODE, 2, 4, -1, { { TU("¿é"), -1, 0 }, { TU("กขฯ"), -1, 0 }, { TU("φχψω"), -1, 0 } }, ZINT_WARN_USES_ECI, 7, 99, 1, "Auto-ECI",
                    "111111101010101010101011101010101010101010101010111010101010101010101010101110101010101010101010111"
                    "100000100111110000101010111000010110100100000100101011100110111000000110101010111110010111111100101"
                    "101110100101001001110111111010010100110101100100111110111100100111111111101110010010111000100011111"
                    "101110100100110100000110011111110011101110010000000100000110000101011000100000111111001101100110001"
                    "101110101000100010000111100101110011010000111010111010110011110101110111101111111011111010100110101"
                    "100000101010100100010010111001101101000001111001101100011000001111001110111011001101000110110010001"
                    "111111101010101010101011101010101010101010101010111010101010101010101010101110101010101010101011111"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[4096];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_rmqr_encode_segs", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, -1 /*eci*/,
                            data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/,
                            NULL, 0, debug);
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            char escaped1[4096];
            char escaped2[4096];
            int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
            int length1 = data[i].segs[1].length == -1 ? (int) ustrlen(data[i].segs[1].source) : data[i].segs[1].length;
            int length2 = data[i].segs[2].length == -1 ? (int) ustrlen(data[i].segs[2].source) : data[i].segs[2].length;
            printf("        /*%3d*/ { %s, %d, %d, %d, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode),
                    data[i].option_1, data[i].option_2, data[i].option_3,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

                if (do_bwipp && testUtilCanBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwippSegs(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].segs, seg_count, NULL, cmp_buf, sizeof(cmp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwippSegs ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp) {
                    if ((symbol->input_mode & 0x07) == DATA_MODE) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d multiple segments in DATA_MODE not currently supported for ZXing-C++ testing (%s)\n", i, testUtilBarcodeName(symbol->symbology));
                    } else {
                        int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
                        if (testUtilCanZXingCPP(i, symbol, (const char *) data[i].segs[0].source, length, debug)) {
                            int cmp_len, ret_len;
                            char modules_dump[32768];
                            assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                            ret = testUtilZXingCPP(i, symbol, (const char *) data[i].segs[0].source, length,
                                    modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                            assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                            ret = testUtilZXingCPPCmpSegs(symbol, cmp_msg, cmp_buf, cmp_len, data[i].segs, seg_count,
                                    NULL /*primary*/, escaped, &ret_len);
                            assert_zero(ret, "i:%d %s testUtilZXingCPPCmpSegs %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                           i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                        }
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* #300 Andre Maute */
static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int eci;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int length;
        int ret;
        int bwipp_cmp;
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_QRCODE, GS1_MODE | GS1NOCHECK_MODE, -1, -1, -1, -1, "[]CCCCCLLLLLLLLLLLLLLLLLLLLLCCCCCCCC@CCCCCCCCCCCCCCCCCCCCCCC%%C%C%%%%%%%%%%%%%%LLLCCCCCCCC%%C%C%%%%%%%%%%%%%%LLLLLLLLLLLLLLLLLLL000000032861710*383556LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL155816162LLLLLCC%%C%C%%%%%%%%%%%%%%LLLCCCCCCCC%%C%C%%%%%%%%%%%%%%LLLLLLLLLL)!1661055777[723]T5", -1, 0, 1, "" }, /* #300 (#14), Andre Maute */
        /*  1*/ { BARCODE_QRCODE, DATA_MODE, -1, -1, -1, ZINT_FULL_MULTIBYTE, "\215\215\350\2156750\215\215\215\215\215\215\000\000\000\025\215\215\215\215\215\232\215\232\232\001\361\215\215\215\215\215\221\215\215\215\215JJJJJJNJJJJJJ\215\215\215\2159999\215\215\215\215\215\215\215\215\215\235\215\215\215\215\215\035\004\000\000@\000\000\000\000\375\000\000\000\000\000\000\000\000\000\000\000\000\000\241\000\000\000\000\000\000\000\241\247^^^\377\377\377\000 \000\000\000\000\000\000\377\377u\000\000\000\000\000\000\000^\377\377^^\000:\000\177\377\377\377?\377\377\377\377\377\377\377\377\377\377\377\377\377\377\241\241\232\232\232\232\232\232\232\232\000\377\377\377\242\003\000\000\377\377/\000AA\000\000\000\000\000\000\000\000\000\000\000\000T\000\000\000\000\000\000\000\000WWW\237\250WWWWWW\377\377R30      \377\377\000\000\000", 231, 0, 1, "" }, /* #300 (#15), Andre Maute */
        /*  2*/ { BARCODE_QRCODE, DATA_MODE, 35, -1, -1, ZINT_FULL_MULTIBYTE, "\215\215\215\215\215\350\215\215999\215\21500000\215\215\215\215\215\215\377O\000\000\036\000\000\000\000\357\376\026\377\377\377\377\241\241\232\232\232\232\232\232\235\032@\374:JGB \000\000@d\000\000\000\241\241\000\000\027\002\241\241\000\000\014\000\000\000\000\357\327\004\000\000\000\000\000\000\000\375\000\000\000\000\000\000\000\000\000\000\000\000\0000253]9R4R44,44,4404[255\350999\215\21599999\215\215\215\2150000\215\215\215\215\215\215\215\215\215]9444442<4444,4044%44vA\000\000\002\000'\000\000\215\377@\215\215\350\215\215\215\215\215\215\215\307\306\306n\215\215\000\000\001\000\000\203\000\000\000\000\000\000@\215\215\215[\2154315@]R0", 229, 0, 1, "" }, /* #300 (#16), Andre Maute */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_fuzz", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_qr_large", test_qr_large },
        { "test_qr_options", test_qr_options },
        { "test_qr_input", test_qr_input },
        { "test_qr_gs1", test_qr_gs1 },
        { "test_qr_optimize", test_qr_optimize },
        { "test_qr_encode", test_qr_encode },
        { "test_qr_encode_segs", test_qr_encode_segs },
        { "test_qr_perf", test_qr_perf },

        { "test_microqr_options", test_microqr_options },
        { "test_microqr_input", test_microqr_input },
        { "test_microqr_padding", test_microqr_padding },
        { "test_microqr_optimize", test_microqr_optimize },
        { "test_microqr_encode", test_microqr_encode },
        { "test_microqr_perf", test_microqr_perf },

        { "test_upnqr_input", test_upnqr_input },
        { "test_upnqr_encode", test_upnqr_encode },

        { "test_rmqr_large", test_rmqr_large },
        { "test_rmqr_options", test_rmqr_options },
        { "test_rmqr_input", test_rmqr_input },
        { "test_rmqr_gs1", test_rmqr_gs1 },
        { "test_rmqr_optimize", test_rmqr_optimize },
        { "test_rmqr_encode", test_rmqr_encode },
        { "test_rmqr_encode_segs", test_rmqr_encode_segs },

        { "test_fuzz", test_fuzz },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et norl : */
