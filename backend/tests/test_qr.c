/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2021 Robin Stuart <rstuart114@gmail.com>

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

static void test_qr_options(int index, int debug) {

    struct item {
        int option_1;
        int option_2;
        char *data;
        int ret_encode;
        int ret_vector;
        int expected_size;
        int compare_previous;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { -1, -1, "12345", 0, 0, 21, -1 }, // ECC auto-set to 1 (L), version auto-set to 1
        /*  1*/ { 5, -1, "12345", 0, 0, 21, 0 }, // ECC > 4 ignored
        /*  2*/ { -1, 41, "12345", 0, 0, 21, 0 }, // Version > 40 ignored
        /*  3*/ { -1, 2, "12345", 0, 0, 25, -1 }, // ECC auto-set to 4 (Q), version 2
        /*  4*/ { 4, 2, "12345", 0, 0, 25, 0 }, // ECC 4 (Q), version 2
        /*  5*/ { 1, 2, "12345", 0, 0, 25, 1 }, // ECC 1 (L), version 2
        /*  6*/ { -1, -1, "貫やぐ識禁", 0, 0, 21, -1 }, // ECC auto-set to 1 (L), version auto-set to 1
        /*  7*/ { 1, -1, "貫やぐ識禁", 0, 0, 21, 0 }, // Version auto-set to 1
        /*  8*/ { -1, 1, "貫やぐ識禁", 0, 0, 21, 0 }, // ECC auto-set to 1 (L)
        /*  9*/ { 1, 1, "貫やぐ識禁", 0, 0, 21, 0 },
        /* 10*/ { 2, 1, "貫やぐ識禁", ZINT_ERROR_TOO_LONG, -1, 0, -1 }, // ECC 2 (M), version 1
        /* 11*/ { 2, -1, "貫やぐ識禁", 0, 0, 25, -1 }, // Version auto-set to 2
        /* 12*/ { 2, 2, "貫やぐ識禁", 0, 0, 25, 0 },
        /* 13*/ { 1, 2, "貫やぐ識禁", 0, 0, 25, 1 },
        /* 14*/ { -1, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 29, -1 }, // ECC auto-set to 1 (L), version auto-set to 3
        /* 15*/ { 1, 3, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 29, 0 },
        /* 16*/ { 2, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 33, -1 }, // ECC 2 (M), version auto-set to 4
        /* 17*/ { 2, 4, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 33, 0 },
        /* 18*/ { 3, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 37, -1 }, // ECC 3 (Q), version auto-set to 5
        /* 19*/ { 3, 5, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 37, 0 },
        /* 20*/ { 4, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 41, -1 }, // ECC 4 (H), version auto-set to 6
        /* 21*/ { 4, 6, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, 0, 41, 0 },
        /* 22*/ { -1, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 0, 0, 69, -1 }, // ECC auto-set to 1, version auto-set to 13
        /* 23*/ { 1, 13, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 0, 0, 69, 0 },
        /* 24*/ { 4, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 0, 0, 101, -1 }, // ECC 4, version auto-set to 21
        /* 25*/ { 4, 21, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。", 0, 0, 101, 0 },
        /* 26*/ { -1, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, 0, 105, -1 }, // ECC auto-set to 1, version auto-set to 22
        /* 27*/ { 1, 22, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, 0, 105, 0 },
        /* 28*/ { 4, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, 0, 153, 1 }, // ECC 4, version auto-set 34
        /* 29*/ { 4, 34, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, 0, 153, 0 },
        /* 30*/ { 4, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, 0, 177, -1 }, // 1852 alphanumerics max for ECC 4 (H)
        /* 31*/ { 1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 0, 0, 177, -1 }, // 4296 alphanumerics max for ECC 1 (L)
        /* 32*/ { 4, -1, "貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫", 0, -1, 0, -1 }, // 424 Kanji, ECC 4 (Q), version 1
        /* 33*/ { 4, -1, "貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫貫", ZINT_ERROR_TOO_LONG, -1, 0, -1 }, // 425 Kanji, ECC 4 (Q), version 1
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_symbol previous_symbol;

    testStart("test_qr_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);
        if (index == -1 && data[i].compare_previous != -1) {
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

static void test_qr_input(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int eci;
        int option_3;
        char *data;
        int ret;
        int expected_eci;
        char *expected;
        char *comment;
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
    // Á U+00C1, UTF-8 C381; ȁ U+0201, UTF-8 C881; Ȃ U+0202, UTF-8 C882; ¢ U+00A2, UTF-8 C2A2; á U+00E1, UTF-8 C3A1
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, "é", 0, 0, "40 1E 90 EC 11 EC 11 EC 11", "B1 (ISO 8859-1)" },
        /*  1*/ { UNICODE_MODE, 3, -1, "é", 0, 3, "70 34 01 E9 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /*  2*/ { UNICODE_MODE, 20, -1, "é", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "é not in Shift JIS" },
        /*  3*/ { UNICODE_MODE, 26, -1, "é", 0, 26, "71 A4 02 C3 A9 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /*  4*/ { DATA_MODE, 0, -1, "é", 0, 0, "40 2C 3A 90 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /*  5*/ { DATA_MODE, 0, -1, "\351", 0, 0, "40 1E 90 EC 11 EC 11 EC 11", "B1 (ISO 8859-1)" },
        /*  6*/ { UNICODE_MODE, 0, -1, "β", 0, 0, "80 11 00 00 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /*  7*/ { UNICODE_MODE, 9, -1, "β", 0, 9, "70 94 01 E2 00 EC 11 EC 11", "ECI-9 B1 (ISO 8859-7)" },
        /*  8*/ { UNICODE_MODE, 20, -1, "β", 0, 20, "71 48 01 10 00 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /*  9*/ { UNICODE_MODE, 26, -1, "β", 0, 26, "71 A4 02 CE B2 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 10*/ { DATA_MODE, 0, -1, "β", 0, 0, "40 2C EB 20 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 11*/ { UNICODE_MODE, 0, -1, "ก", ZINT_WARN_USES_ECI, 13, "Warning 70 D4 01 A1 00 EC 11 EC 11", "ECI-13 B1 (ISO 8859-11)" },
        /* 12*/ { UNICODE_MODE, 13, -1, "ก", 0, 13, "70 D4 01 A1 00 EC 11 EC 11", "ECI-13 B1 (ISO 8859-11)" },
        /* 13*/ { UNICODE_MODE, 20, -1, "ก", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "ก not in Shift JIS" },
        /* 14*/ { UNICODE_MODE, 26, -1, "ก", 0, 26, "71 A4 03 E0 B8 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 15*/ { DATA_MODE, 0, -1, "ก", 0, 0, "40 3E 0B 88 10 EC 11 EC 11", "B3 (UTF-8)" },
        /* 16*/ { UNICODE_MODE, 0, -1, "Ж", 0, 0, "80 11 23 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 17*/ { UNICODE_MODE, 7, -1, "Ж", 0, 7, "70 74 01 B6 00 EC 11 EC 11", "ECI-7 B1 (ISO 8859-5)" },
        /* 18*/ { UNICODE_MODE, 20, -1, "Ж", 0, 20, "71 48 01 12 38 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 19*/ { UNICODE_MODE, 26, -1, "Ж", 0, 26, "71 A4 02 D0 96 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 20*/ { DATA_MODE, 0, -1, "Ж", 0, 0, "40 2D 09 60 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 21*/ { UNICODE_MODE, 0, -1, "ກ", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E0 BA 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 22*/ { UNICODE_MODE, 20, -1, "ກ", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "ກ not in Shift JIS" },
        /* 23*/ { UNICODE_MODE, 26, -1, "ກ", 0, 26, "71 A4 03 E0 BA 81 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 24*/ { DATA_MODE, 0, -1, "ກ", 0, 0, "40 3E 0B A8 10 EC 11 EC 11", "B3 (UTF-8)" },
        /* 25*/ { UNICODE_MODE, 0, -1, "\\", 0, 0, "40 15 C0 EC 11 EC 11 EC 11", "B1 (ASCII)" },
        /* 26*/ { UNICODE_MODE, 20, -1, "\\", 0, 20, "71 48 01 00 F8 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 27*/ { UNICODE_MODE, 20, -1, "[", 0, 20, "71 44 01 5B 00 EC 11 EC 11", "B1 (ASCII)" },
        /* 28*/ { UNICODE_MODE, 20, -1, "\177", 0, 20, "71 44 01 7F 00 EC 11 EC 11", "ECI-20 B1 (ASCII)" },
        /* 29*/ { UNICODE_MODE, 0, -1, "¥", 0, 0, "40 1A 50 EC 11 EC 11 EC 11", "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous)" },
        /* 30*/ { UNICODE_MODE, 3, -1, "¥", 0, 3, "70 34 01 A5 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /* 31*/ { UNICODE_MODE, 20, -1, "¥", 0, 20, "71 44 01 5C 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) (to single-byte backslash codepoint 5C, so byte mode)" },
        /* 32*/ { UNICODE_MODE, 26, -1, "¥", 0, 26, "71 A4 02 C2 A5 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 33*/ { DATA_MODE, 0, -1, "¥", 0, 0, "40 2C 2A 50 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 34*/ { UNICODE_MODE, 0, -1, "･", 0, 0, "40 1A 50 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /* 35*/ { UNICODE_MODE, 3, -1, "･", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input data for ECI 3", "" },
        /* 36*/ { UNICODE_MODE, 20, -1, "･", 0, 20, "71 44 01 A5 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) single-byte codepoint A5" },
        /* 37*/ { UNICODE_MODE, 26, -1, "･", 0, 26, "71 A4 03 EF BD A5 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 38*/ { DATA_MODE, 0, -1, "･", 0, 0, "40 3E FB DA 50 EC 11 EC 11", "B3 (UTF-8)" },
        /* 39*/ { UNICODE_MODE, 0, -1, "¿", 0, 0, "40 1B F0 EC 11 EC 11 EC 11", "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous)" },
        /* 40*/ { UNICODE_MODE, 3, -1, "¿", 0, 3, "70 34 01 BF 00 EC 11 EC 11", "ECI-3 B1 (ISO 8859-1)" },
        /* 41*/ { UNICODE_MODE, 20, -1, "¿", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "¿ not in Shift JIS" },
        /* 42*/ { UNICODE_MODE, 26, -1, "¿", 0, 26, "71 A4 02 C2 BF 00 EC 11 EC", "ECI-26 B2 (UTF-8)" },
        /* 43*/ { DATA_MODE, 0, -1, "¿", 0, 0, "40 2C 2B F0 EC 11 EC 11 EC", "B2 (UTF-8)" },
        /* 44*/ { UNICODE_MODE, 0, -1, "ｿ", 0, 0, "40 1B F0 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint BF (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 45*/ { UNICODE_MODE, 3, -1, "ｿ", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input data for ECI 3", "" },
        /* 46*/ { UNICODE_MODE, 20, -1, "ｿ", 0, 20, "71 44 01 BF 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) single-byte codepoint BF" },
        /* 47*/ { UNICODE_MODE, 26, -1, "ｿ", 0, 26, "71 A4 03 EF BD BF 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 48*/ { DATA_MODE, 0, -1, "ｿ", 0, 0, "40 3E FB DB F0 EC 11 EC 11", "B3 (UTF-8)" },
        /* 49*/ { UNICODE_MODE, 0, -1, "~", 0, 0, "40 17 E0 EC 11 EC 11 EC 11", "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 50*/ { UNICODE_MODE, 3, -1, "~", 0, 3, "70 34 01 7E 00 EC 11 EC 11", "ECI-3 B1 (ASCII)" },
        /* 51*/ { UNICODE_MODE, 20, -1, "~", ZINT_ERROR_INVALID_DATA, -1, "Error 800: Invalid character in input data", "tilde not in Shift JIS (codepoint used for overline)" },
        /* 52*/ { UNICODE_MODE, 0, -1, "‾", 0, 0, "40 17 E0 EC 11 EC 11 EC 11", "B1 (Shift JIS) single-byte codepoint 7E (same bytes as ~ ASCII above, so ambiguous)" },
        /* 53*/ { UNICODE_MODE, 3, -1, "‾", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input data for ECI 3", "" },
        /* 54*/ { UNICODE_MODE, 20, -1, "‾", 0, 20, "71 44 01 7E 00 EC 11 EC 11", "ECI-20 B1 (Shift JIS) (to single-byte tilde codepoint 7E, so byte mode)" },
        /* 55*/ { UNICODE_MODE, 26, -1, "‾", 0, 26, "71 A4 03 E2 80 BE 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 56*/ { DATA_MODE, 0, -1, "‾", 0, 0, "40 3E 28 0B E0 EC 11 EC 11", "B3 (UTF-8)" },
        /* 57*/ { UNICODE_MODE, 0, -1, "点", 0, 0, "80 16 CF 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 58*/ { UNICODE_MODE, 3, -1, "点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input data for ECI 3", "" },
        /* 59*/ { UNICODE_MODE, 20, -1, "点", 0, 20, "71 48 01 6C F8 00 EC 11 EC", "ECI-20 K1 (Shift JIS)" },
        /* 60*/ { UNICODE_MODE, 26, -1, "点", 0, 26, "71 A4 03 E7 82 B9 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /* 61*/ { DATA_MODE, 0, -1, "点", 0, 0, "40 3E 78 2B 90 EC 11 EC 11", "B3 (UTF-8)" },
        /* 62*/ { DATA_MODE, 0, -1, "\223\137", 0, 0, "40 29 35 F0 EC 11 EC 11 EC", "B2 (Shift JIS) (not full multibyte)" },
        /* 63*/ { DATA_MODE, 0, ZINT_FULL_MULTIBYTE, "\223\137", 0, 0, "80 16 CF 80 EC 11 EC 11 EC", "K1 (Shift JIS)" },
        /* 64*/ { UNICODE_MODE, 0, -1, "¥･点", 0, 0, "40 45 CA 59 35 F0 EC 11 EC", "B4 (Shift JIS) (optimized to byte mode only)" },
        /* 65*/ { UNICODE_MODE, 3, -1, "¥･点", ZINT_ERROR_INVALID_DATA, -1, "Error 575: Invalid character in input data for ECI 3", "" },
        /* 66*/ { UNICODE_MODE, 20, -1, "¥･点", 0, 20, "71 44 04 5C A5 93 5F 00 EC", "ECI-20 B4 (Shift JIS)" },
        /* 67*/ { UNICODE_MODE, 26, -1, "¥･点", 0, 26, "71 A4 08 C2 A5 EF BD A5 E7 82 B9 00 EC", "ECI-26 B8 (UTF-8)" },
        /* 68*/ { DATA_MODE, 0, -1, "\134\245\223\137", 0, 0, "40 45 CA 59 35 F0 EC 11 EC", "B8 (Shift JIS)" },
        /* 69*/ { DATA_MODE, 0, -1, "¥･点", 0, 0, "40 8C 2A 5E FB DA 5E 78 2B 90 EC 11 EC", "B8 (UTF-8)" },
        /* 70*/ { UNICODE_MODE, 0, -1, "点茗", 0, 0, "80 26 CF EA A8 00 EC 11 EC", "K2 (Shift JIS)" },
        /* 71*/ { UNICODE_MODE, 0, -1, "点茗テ", 0, 0, "80 36 CF EA A8 34 A0 EC 11", "K3 (Shift JIS)" },
        /* 72*/ { UNICODE_MODE, 0, -1, "点茗テ点", 0, 0, "80 46 CF EA A8 34 AD 9F 00", "K4 (Shift JIS)" },
        /* 73*/ { UNICODE_MODE, 0, -1, "点茗テ点茗", 0, 0, "80 56 CF EA A8 34 AD 9F D5 50 00 EC 11", "K5 (Shift JIS)" },
        /* 74*/ { UNICODE_MODE, 0, -1, "点茗テ点茗テ", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 40 EC", "K6 (Shift JIS)" },
        /* 75*/ { UNICODE_MODE, 0, -1, "点茗テ点茗テｿ", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 50 06 FC 00 EC", "K6 B1 (Shift JIS)" },
        /* 76*/ { DATA_MODE, 0, -1, "\223\137\344\252\203\145\223\137\344\252\203\145\277", 0, 0, "40 D9 35 FE 4A A8 36 59 35 FE 4A A8 36 5B F0 EC", "B13 (Shift JIS)" },
        /* 77*/ { DATA_MODE, 0, ZINT_FULL_MULTIBYTE, "\223\137\344\252\203\145\223\137\344\252\203\145\277", 0, 0, "80 66 CF EA A8 34 AD 9F D5 50 69 50 06 FC 00 EC", "K6 B1 (Shift JIS) (full multibyte)" },
        /* 78*/ { DATA_MODE, 0, -1, "点茗テ点茗テｿ", 0, 0, "41 5E 78 2B 9E 88 C9 7E 38 38 6E 78 2B 9E 88 C9 7E 38 38 6E FB DB F0 EC 11 EC 11 EC", "B21 (UTF-8)" },
        /* 79*/ { DATA_MODE, 0, -1, "ÁȁȁȁȁȁȁȂ¢", 0, 0, "41 2C 38 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 2C 2A 20 EC 11", "B18 (UTF-8)" },
        /* 80*/ { DATA_MODE, 0, ZINT_FULL_MULTIBYTE, "ÁȁȁȁȁȁȁȂ¢", 0, 0, "41 2C 38 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 2C 2A 20 EC 11", "B18 (UTF-8) (full multibyte)" },
        /* 81*/ { DATA_MODE, 0, -1, "ÁȁȁȁȁȁȁȁȂ¢", 0, 0, "41 4C 38 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 1C 88 2C 2A 20", "B20 (UTF-8)" },
        /* 82*/ { DATA_MODE, 0, ZINT_FULL_MULTIBYTE, "ÁȁȁȁȁȁȁȁȂ¢", 0, 0, "40 1C 38 09 04 40 22 01 10 08 80 44 02 20 11 00 88 0A 12 00 D1 00", "B1 K9 B1 (UTF-8) (full multibyte)" },
        /* 83*/ { UNICODE_MODE, 0, -1, "ÁȁȁȁȁȁȁȂ¢", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 12 C3 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00", "B18 (UTF-8)" },
        /* 84*/ { UNICODE_MODE, 0, ZINT_FULL_MULTIBYTE, "ÁȁȁȁȁȁȁȂ¢", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 12 C3 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00", "B18 (UTF-8)" },
        /* 85*/ { UNICODE_MODE, 0, -1, "ÁȁȁȁȁȁȁȁȂ¢", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 14 C3 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00 EC 11 EC 11", "B20 (UTF-8)" },
        /* 86*/ { UNICODE_MODE, 0, ZINT_FULL_MULTIBYTE, "ÁȁȁȁȁȁȁȁȂ¢", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 01 C3 80 90 44 02 20 11 00 88 04 40 22 01 10 08 80 A1 20 0D 10 00 EC 11 EC 11", "B1 K9 B1 (UTF-8) (full multibyte)" },
        /* 87*/ { UNICODE_MODE, 0, -1, "áA", 0, 0, "40 2E 14 10 EC 11 EC 11 EC", "B2 (ISO 8859-1)" },
        /* 88*/ { UNICODE_MODE, 0, ZINT_FULL_MULTIBYTE, "áA", 0, 0, "80 1C 00 80 EC 11 EC 11 EC", "K1 (ISO 8859-1) (full multibyte)" },
        /* 89*/ { UNICODE_MODE, 0, -1, "A0B1C2D3E4F5G6H7I8J9KLMNOPQRSTUVWXYZ $%*+-./:", 0, 0, "(34) 21 69 C2 3E 08 79 26 27 A5 50 B5 98 23 32 6C 0E 65 FA C5 19 5B 42 6B 2D C1 C3 B9 E7", "A45" },
        /* 90*/ { UNICODE_MODE, 0, -1, "˘", ZINT_WARN_USES_ECI, 4, "Warning 70 44 01 A2 00 EC 11 EC 11", "ECI-4 B1 (ISO 8859-2)" },
        /* 91*/ { UNICODE_MODE, 4, -1, "˘", 0, 4, "70 44 01 A2 00 EC 11 EC 11", "ECI-4 B1 (ISO 8859-2)" },
        /* 92*/ { UNICODE_MODE, 0, -1, "Ħ", ZINT_WARN_USES_ECI, 5, "Warning 70 54 01 A1 00 EC 11 EC 11", "ECI-5 B1 (ISO 8859-3)" },
        /* 93*/ { UNICODE_MODE, 5, -1, "Ħ", 0, 5, "70 54 01 A1 00 EC 11 EC 11", "ECI-5 B1 (ISO 8859-3)" },
        /* 94*/ { UNICODE_MODE, 0, -1, "ĸ", ZINT_WARN_USES_ECI, 6, "Warning 70 64 01 A2 00 EC 11 EC 11", "ECI-6 B1 (ISO 8859-4)" },
        /* 95*/ { UNICODE_MODE, 6, -1, "ĸ", 0, 6, "70 64 01 A2 00 EC 11 EC 11", "ECI-6 B1 (ISO 8859-4)" },
        /* 96*/ { UNICODE_MODE, 0, -1, "Ș", ZINT_WARN_USES_ECI, 18, "Warning 71 24 01 AA 00 EC 11 EC 11", "ECI-18 B1 (ISO 8859-16)" },
        /* 97*/ { UNICODE_MODE, 18, -1, "Ș", 0, 18, "71 24 01 AA 00 EC 11 EC 11", "ECI-18 B1 (ISO 8859-16)" },
        /* 98*/ { UNICODE_MODE, 0, -1, "テ", 0, 0, "80 10 D2 80 EC 11 EC 11 EC", "K1 (SHIFT JIS)" },
        /* 99*/ { UNICODE_MODE, 20, -1, "テ", 0, 20, "71 48 01 0D 28 00 EC 11 EC", "ECI-20 K1 (SHIFT JIS)" },
        /*100*/ { UNICODE_MODE, 20, -1, "テテ", 0, 20, "71 48 02 0D 28 69 40 EC 11", "ECI-20 K2 (SHIFT JIS)" },
        /*101*/ { UNICODE_MODE, 20, -1, "\\\\", 0, 20, "71 48 02 00 F8 07 C0 EC 11", "ECI-20 K2 (SHIFT JIS)" },
        /*102*/ { UNICODE_MODE, 0, -1, "…", 0, 0, "80 10 11 80 EC 11 EC 11 EC", "K1 (SHIFT JIS)" },
        /*103*/ { UNICODE_MODE, 21, -1, "…", 0, 21, "71 54 01 85 00 EC 11 EC 11", "ECI-21 B1 (Win 1250)" },
        /*104*/ { UNICODE_MODE, 0, -1, "Ґ", ZINT_WARN_USES_ECI, 22, "Warning 71 64 01 A5 00 EC 11 EC 11", "ECI-22 B1 (Win 1251)" },
        /*105*/ { UNICODE_MODE, 22, -1, "Ґ", 0, 22, "71 64 01 A5 00 EC 11 EC 11", "ECI-22 B1 (Win 1251)" },
        /*106*/ { UNICODE_MODE, 0, -1, "˜", ZINT_WARN_USES_ECI, 23, "Warning 71 74 01 98 00 EC 11 EC 11", "ECI-23 B1 (Win 1252)" },
        /*107*/ { UNICODE_MODE, 23, -1, "˜", 0, 23, "71 74 01 98 00 EC 11 EC 11", "ECI-23 B1 (Win 1252)" },
        /*108*/ { UNICODE_MODE, 24, -1, "پ", 0, 24, "71 84 01 81 00 EC 11 EC 11", "ECI-24 B1 (Win 1256)" },
        /*109*/ { UNICODE_MODE, 0, -1, "က", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E1 80 80 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /*110*/ { UNICODE_MODE, 25, -1, "က", 0, 25, "71 94 02 10 00 00 EC 11 EC", "ECI-25 B2 (UCS-2BE)" },
        /*111*/ { UNICODE_MODE, 25, -1, "ကက", 0, 25, "71 94 04 10 00 10 00 00 EC", "ECI-25 B4 (UCS-2BE)" },
        /*112*/ { UNICODE_MODE, 25, -1, "12", 0, 25, "71 94 04 00 31 00 32 00 EC", "ECI-25 B4 (UCS-2BE ASCII)" },
        /*113*/ { UNICODE_MODE, 27, -1, "@", 0, 27, "71 B4 01 40 00 EC 11 EC 11", "ECI-27 B1 (ASCII)" },
        /*114*/ { UNICODE_MODE, 0, -1, "龘", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E9 BE 98 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /*115*/ { UNICODE_MODE, 28, -1, "龘", 0, 28, "71 C4 02 F9 D5 00 EC 11 EC", "ECI-28 B2 (Big5)" },
        /*116*/ { UNICODE_MODE, 28, -1, "龘龘", 0, 28, "71 C4 04 F9 D5 F9 D5 00 EC", "ECI-28 B4 (Big5)" },
        /*117*/ { UNICODE_MODE, 0, -1, "齄", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 E9 BD 84 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /*118*/ { UNICODE_MODE, 29, -1, "齄", 0, 29, "71 D4 02 F7 FE 00 EC 11 EC", "ECI-29 B2 (GB 2312)" },
        /*119*/ { UNICODE_MODE, 29, -1, "齄齄", 0, 29, "71 D4 04 F7 FE F7 FE 00 EC", "ECI-29 B4 (GB 2312)" },
        /*120*/ { UNICODE_MODE, 0, -1, "가", ZINT_WARN_USES_ECI, 26, "Warning 71 A4 03 EA B0 80 00 EC 11", "ECI-26 B3 (UTF-8)" },
        /*121*/ { UNICODE_MODE, 30, -1, "가", 0, 30, "71 E4 02 B0 A1 00 EC 11 EC", "ECI-30 B2 (EUC-KR)" },
        /*122*/ { UNICODE_MODE, 30, -1, "가가", 0, 30, "71 E4 04 B0 A1 B0 A1 00 EC", "ECI-30 B4 (EUC-KR)" },
        /*123*/ { UNICODE_MODE, 170, -1, "?", 0, 170, "78 0A A4 01 3F 00 EC 11 EC", "ECI-170 B1 (ASCII invariant)" },
        /*124*/ { DATA_MODE, 899, -1, "\200", 0, 899, "78 38 34 01 80 00 EC 11 EC", "ECI-899 B1 (8-bit binary)" },
        /*125*/ { UNICODE_MODE, 900, -1, "é", 0, 900, "78 38 44 02 C3 A9 00 EC 11", "ECI-900 B2 (no conversion)" },
        /*126*/ { UNICODE_MODE, 16384, -1, "é", 0, 16384, "7C 04 00 04 02 C3 A9 00 EC", "ECI-16384 B2 (no conversion)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_qr_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, data[i].input_mode, data[i].eci, -1 /*option_1*/, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %s, \"%s\", %s, %d, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), ret < ZINT_ERROR ? symbol->eci : -1, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_gs1(int index, int generate, int debug) {

    struct item {
        int input_mode;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { GS1_MODE, "[01]12345678901231", 0, "51 04 00 B3 AA 37 DE 87 B1", "N16" },
        /*  1*/ { GS1_MODE | GS1PARENS_MODE, "(01)12345678901231", 0, "51 04 00 B3 AA 37 DE 87 B1", "N16" },
        /*  2*/ { GS1_MODE, "[01]04912345123459[15]970331[30]128[10]ABC123", 0, "51 07 40 A7 AC EA 80 15 9E 4F CA 52 D2 D3 84 09 D5 E0 28 FD 82 F0 C0 EC 11 EC 11 EC", "N29 A9" },
        /*  3*/ { GS1_MODE | GS1PARENS_MODE, "(01)04912345123459(15)970331(30)128(10)ABC123", 0, "51 07 40 A7 AC EA 80 15 9E 4F CA 52 D2 D3 84 09 D5 E0 28 FD 82 F0 C0 EC 11 EC 11 EC", "N29 A9" },
        /*  4*/ { GS1_MODE, "[91]12%[20]12", 0, "52 05 99 60 5F B5 35 80 01 08 00 EC 11", "A10(11)" },
        /*  5*/ { GS1_MODE, "[91]123%[20]12", 0, "52 06 19 60 5E 2B 76 A0 5A 05 E0 EC 11", "A11(12)" },
        /*  6*/ { GS1_MODE, "[91]1234%[20]12", 0, "52 06 99 60 5E 22 F6 A6 B0 00 21 00 EC", "A12(13)" },
        /*  7*/ { GS1_MODE, "[91]12345%[20]12", 0, "51 01 F8 F3 A9 48 0F B5 35 80 01 08 00", "N7 A6(7) (same bit count as A13(14))" },
        /*  8*/ { GS1_MODE, "[91]%%[20]12", 0, "52 05 99 6D A9 B5 35 80 01 08 00 EC 11", "A9(11)" },
        /*  9*/ { GS1_MODE, "[91]%%%[20]12", 0, "52 06 99 6D A9 B5 36 A6 B0 00 21 00 EC", "A10(13)" },
        /* 10*/ { GS1_MODE, "[91]A%%%%1234567890123AA%", 0, "52 05 99 63 D1 B5 36 A6 D4 98 40 D1 ED C8 C5 40 C3 20 21 CC DA 80", "A7(11) N13 A3(4)" },
        /* 11*/ { GS1_MODE, "[91]%23%%6789%%%34567%%%%234%%%%%", 0, "(34) 52 17 19 6D A8 17 76 A6 D4 22 A5 C7 6A 6D 4D A8 22 C7 39 61 DA 9B 53 6A 6B 01 17 B5", "A31(46)" },
        /* 12*/ { GS1_MODE, "[91]ABCDEFGHI[92]ABCDEF", 0, "52 0A 19 63 9A 8A 54 2A E1 6A 06 5C E6 A2 95 0A", "A20(23)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_qr_gs1");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_optimize(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int option_1;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "1", 0, "10 04 40 EC 11 EC 11 EC 11", "N1" },
        /*  1*/ { UNICODE_MODE, -1, "AAA", 0, "20 19 CC 28 00 EC 11 EC 11", "A3" },
        /*  2*/ { UNICODE_MODE, -1, "0123456789", 0, "10 28 0C 56 6A 69 00 EC 11", " N10 (nayuki.io - pure numeric)" },
        /*  3*/ { UNICODE_MODE, -1, "ABCDEF", 0, "20 31 CD 45 2A 14 00 EC 11", "A6 (nayuki.io - pure alphanumeric)" },
        /*  4*/ { UNICODE_MODE, -1, "wxyz", 0, "40 47 77 87 97 A0 EC 11 EC", "B4 (nayuki.io - pure byte)" },
        /*  5*/ { UNICODE_MODE, 1, "「魔法少女まどか☆マギカ」って、　ИАИ　ｄｅｓｕ　κα？", 0, "(55) 81 D0 1A C0 09 F8 0A ED 56 B8 57 02 8E 12 90 2C 86 F4 31 A1 8A 01 B0 50 42 88 00 10", "K29 (nayuki.io - pure kanji)" },
        /*  6*/ { UNICODE_MODE, -1, "012345A", 0, "20 38 01 0B A2 E4 A0 EC 11", "A7 (nayuki.io - alpha/numeric)" },
        /*  7*/ { UNICODE_MODE, -1, "0123456A", 0, "10 1C 0C 56 58 80 25 00 EC", "N7 A1 (nayuki.io - alpha/numeric) (note same bits as A8)" },
        /*  8*/ { UNICODE_MODE, -1, "012a", 0, "40 43 03 13 26 10 EC 11 EC", "B4 (nayuki.io - numeric/byte)" },
        /*  9*/ { UNICODE_MODE, -1, "0123a", 0, "10 10 0C 34 01 61 00 EC 11", "N4 B1 (nayuki.io - numeric/byte)" },
        /* 10*/ { UNICODE_MODE, -1, "ABCDEa", 0, "40 64 14 24 34 44 56 10 EC", "B6 (nayuki.io - alphanumeric/byte)" },
        /* 11*/ { UNICODE_MODE, -1, "ABCDEFa", 0, "20 31 CD 45 2A 15 00 58 40", "A6 B1 (nayuki.io - alphanumeric/byte)" },
        /* 12*/ { UNICODE_MODE, 1, "THE SQUARE ROOT OF 2 IS 1.41421356237309504880168872420969807856967187537694807317667973799", 0, "(55) 20 D5 2A 53 54 1A A8 4C DC DF 14 29 EC 47 CA D9 9A 88 05 71 10 59 E3 56 32 5D 45 F0", " A26 N65 (nayuki.io - alpha/numeric)" },
        /* 13*/ { UNICODE_MODE, 1, "Golden ratio φ = 1.6180339887498948482045868343656381177203091798057628621354486227052604628189024497072072041893911374......", 0, "(80) 41 44 76 F6 C6 46 56 E2 07 26 17 46 96 F2 08 3D 32 03 D2 03 12 E1 19 26 A0 87 DC BB", "B20 N100 A6 (nayuki.io - alpha/numeric/byte)" },
        /* 14*/ { UNICODE_MODE, 1, "こんにちwa、世界！ αβγδ", 0, "(34) 41 B8 2B 18 2F 18 2C 98 2B F7 76 18 14 19 0A 28 A4 58 14 92 08 3B F8 3C 08 3C 18 3C", "B27 (nayuki.io - kanji/european **NOT SAME** K4 B2 K4 A1 K4, less bits as nayuki (1.5.0) miscounting byte-mode input as UTF-8)" },
        /* 15*/ { UNICODE_MODE, 1, "こんにちテwa、世界！ αβγδ", 0, "(34) 80 50 98 85 C4 29 21 3F 0D 2A 09 BB B0 C0 A0 C8 51 45 22 C0 A4 90 41 DF C1 E0 41 E0", "K5 B19 (nayuki.io - kanji/european + extra leading kanji **NOT SAME** K5 B2 K4 A1 K4, same reason as above)" },
        /* 16*/ { UNICODE_MODE, 1, "67128177921547861663com.acme35584af52fa3-88d0-093b-6c14-b37ddafb59c528908608sg.com.dash.www0530329356521790265903SG.COM.NETS46968696003522G33250183309051017567088693441243693268766948304B2AE13344004SG.SGQR209710339366720B439682.63667470805057501195235502733744600368027857918629797829126902859SG8236HELLO FOO2517Singapore3272B815", 0, "(232) 10 52 9F 46 70 B3 5D DE 9A 1F A1 7B 1B 7B 69 73 0B 1B 6B 29 99 A9 A9 C1 A3 0B 31 A9", "N20 B47 N9 B15 N22 A11 N14 A1 N47 A19 N15 A8 N65 A20 B8 A8 (nayuki.io - SGQR alpha/numeric/byte)" },
        /* 17*/ { UNICODE_MODE, -1, "纪", ZINT_WARN_USES_ECI, "Warning 71 A4 03 E7 BA AA 00 EC 11", "ECI-26 B3 (UTF-8 E7BAAA, U+7EAA, not in Shift JIS)" },
        /* 18*/ { DATA_MODE, -1, "纪", 0, "40 3E 7B AA A0 EC 11 EC 11", "B3 (UTF-8 or Shift JIS, note ambiguous as 0xE7BA 0xAA happens to be valid Shift JIS 郤ｪ as well)" },
        /* 19*/ { UNICODE_MODE, -1, "郤ｪ", 0, "40 3E 7B AA A0 EC 11 EC 11", "B3 (Shift JIS or UTF-8 E7BAAA 纪, see above)" },
        /* 20*/ { UNICODE_MODE, 1, "2004年大西洋颶風季是有纪录以来造成人员伤亡和财产损失最为惨重的大西洋飓风季之一，于2004年6月1日正式开始，同年11月30日结束，传统上这样的日期界定了一年中绝大多数热带气旋在大西洋形成的时间段lll ku", ZINT_WARN_USES_ECI, "Warning (324) 71 A1 00 43 21 10 04 4B 96 E6 D3 96 92 9F A2 96 FF 9A D2 2F A6 8A DB A6 8A A3 96 B6", "ECI-26 N4 B274 (nayuki.io - kanji/byte/numeric **NOT SAME* N4 K9 B6 K5 etc mixing Shift JIS and UTF-8)" },
        /* 21*/ { UNICODE_MODE, -1, "AB123456A", 0, "20 49 CD 05 E2 2C 73 94 00", "A9" },
        /* 22*/ { UNICODE_MODE, -1, "AB1234567890A", 0, "20 69 CD 05 E2 2C 73 94 33 2A 50 00 EC", "A13" },
        /* 23*/ { UNICODE_MODE, -1, "AB123456789012A", 0, "20 79 CD 05 E2 2C 73 94 33 2A 0B CA 00", "A15" },
        /* 24*/ { UNICODE_MODE, -1, "AB1234567890123A", 0, "20 11 CD 10 34 7B 72 31 50 30 C8 02 50", "A2 N13 A1" },
        /* 25*/ { UNICODE_MODE, -1, "テaABCD1", 0, "40 88 36 56 14 14 24 34 43 10 EC 11 EC", "B8" },
        /* 26*/ { UNICODE_MODE, -1, "テaABCDE1", 0, "40 38 36 56 12 03 1C D4 52 9D C0 EC 11", "B3 A6" },
        /* 27*/ { UNICODE_MODE, -1, "テéaABCDE1", ZINT_WARN_USES_ECI, "Warning 71 A4 06 E3 83 86 C3 A9 61 20 31 CD 45 29 DC 00", "B6 A6" },
        /* 28*/ { UNICODE_MODE, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁", 0, "(44) 80 83 A8 85 88 25 CA 2F 40 B0 53 C2 44 98 41 00 4A 02 0E A8 F8 F5 0D 30 4C 35 A1 CC", "K8 N1 K8 B3" },
        /* 29*/ { UNICODE_MODE, -1, "貫やぐ識禁ぱい再2間変字全ノレ没無8裁花ほゃ過法ひなご札17能つーびれ投覧マ勝動エヨ額界よみ作皇ナヲニ打題ヌルヲ掲布益フが。入35能ト権話しこを断兆モヘ細情おじ名4減エヘイハ側機はょが意見想ハ業独案ユヲウ患職ヲ平美さ毎放どぽたけ家没べお化富べ町大シ情魚ッでれ一冬すぼめり。社ト可化モマ試音ばじご育青康演ぴぎ権型固スで能麩ぜらもほ河都しちほラ収90作の年要とだむ部動ま者断チ第41一1米索焦茂げむしれ。測フ物使だて目月国スリカハ夏検にいへ児72告物ゆは載核ロアメヱ登輸どべゃ催行アフエハ議歌ワ河倫剖だ。記タケウ因載ヒイホヤ禁3輩彦関トえび肝区勝ワリロ成禁ぼよ界白ウヒキレ中島べせぜい各安うしぽリ覧生テ基一でむしゃ中新トヒキソ声碁スしび起田ア信大未ゅもばち。", 0, "(589) 80 20 EA 21 62 09 72 8B D0 2C 14 F0 91 26 10 40 04 A0 08 3A A3 E3 D4 34 C1 30 D6 87", "K8 N1 K8 N1 K10 N2 K33 N2 K16 N1 K89 N2 K14 B5 K28 N2 K40 N1 K65" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_qr_optimize");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_QRCODE, data[i].input_mode, -1 /*eci*/, data[i].option_1, -1, ZINT_FULL_MULTIBYTE, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_qr_encode(int index, int generate, int debug) {

    struct item {
        int symbology;
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
    struct item data[] = {
        /*  0*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, "QR Code Symbol", 0, 21, 21, "ISO 18004 Figure 1 **NOT SAME** uses mask 110 instead of 101",
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
        /*  1*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, 6 << 8, "QR Code Symbol", 0, 21, 21, "ISO 18004 Figure 1, explicit mask 101, same",
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
        /*  2*/ { BARCODE_QRCODE, UNICODE_MODE, 2, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 0, 33, 33, "ISO 18004 Figure 29, same (mask 100)",
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
        /*  3*/ { BARCODE_QRCODE, UNICODE_MODE, 2, 1, -1, "01234567", 0, 21, 21, "ISO 18004 Annex I I.2, same (mask 010)",
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
        /*  4*/ { BARCODE_QRCODE, GS1_MODE, 1, -1, -1, "[01]09501101530003[8200]http://example.com", 0, 25, 25, "GS1 General Specifications 21.0.1 Figure 5.1-7 **NOT SAME** figure uses Byte encodation only",
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
        /*  5*/ { BARCODE_QRCODE, GS1_MODE, 2, -1, -1, "[01]00857674002010[8200]http://www.gs1.org/", 0, 29, 29, "GS1 General Specifications 21.0.1 Figure 5.1-7, same (mask 011)",
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
        /*  6*/ { BARCODE_HIBC_QR, -1, 2, -1, -1, "H123ABC01234567890", 0, 21, 21, "ANSI/HIBC 2.6 - 2016 Figure C5 same (mask 001)",
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
        /*  7*/ { BARCODE_HIBC_QR, -1, 2, -1, -1, "/EU720060FF0/O523201", 0, 25, 25, "HIBC/PAS Section 2.2 2nd Purchase Order **NOT SAME** uses mask 100 instead of 011",
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
        /*  8*/ { BARCODE_HIBC_QR, -1, 2, -1, 4 << 8, "/EU720060FF0/O523201", 0, 25, 25, "HIBC/PAS Section 2.2 2nd Purchase Order same, explicit mask 011",
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
        /*  9*/ { BARCODE_HIBC_QR, -1, 2, -1, -1, "/KN12345", 0, 21, 21, "HIBC/PAS Section 2.2 Asset Tag **NOT SAME** uses mask 000 instead of 100",
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
        /* 10*/ { BARCODE_HIBC_QR, -1, 2, -1, 5 << 8, "/KN12345", 0, 21, 21, "HIBC/PAS Section 2.2 Asset Tag, same, explicit mask 100",
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
        /* 11*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, -1, "12345678901234567890123456789012345678901", 0, 21, 21, "Max capacity ECC 1 Version 1 41 numbers",
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
        /* 12*/ { BARCODE_QRCODE, UNICODE_MODE, 2, -1, -1, "12345678901234567890123456789012345678901", 0, 25, 25, "ECC 2 auto-sets version 2",
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
        /* 13*/ { BARCODE_QRCODE, UNICODE_MODE, 4, 10, -1, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", 0, 57, 57, "Max capacity ECC 4 Version 10 74 kanji",
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
        /* 14*/ { BARCODE_QRCODE, UNICODE_MODE, 4, 27, -1, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", 0, 125, 125, "Max capacity ECC 4 Version 27 385 kanji",
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
        /* 15*/ { BARCODE_QRCODE, UNICODE_MODE, 4, 40, -1, "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点" "点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点点", 0, 177, 177, "Max capacity ECC 4 Version 40 784 kanji",
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
        /* 16*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, ZINT_FULL_MULTIBYTE, "áA", 0, 21, 21, "Mask automatic (001)",
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
        /* 17*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, ZINT_FULL_MULTIBYTE | (8 << 8), "áA", 0, 21, 21, "Mask 111",
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
        /* 18*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1, ZINT_FULL_MULTIBYTE | (9 << 8), "áA", 0, 21, 21, "Mask > 111 ignored",
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
        /* 19*/ { BARCODE_QRCODE, UNICODE_MODE, 2, 1, -1, "1234567890", 0, 21, 21, "test_print example, automatic mask 001 (same score as mask 010)",
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
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_qr_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %d, %s, \"%s\", %s, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2, testUtilOption3Name(data[i].option_3),
                    data[i].data, testUtilErrorName(data[i].ret),
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
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#include <time.h>

#define TEST_PERF_ITERATIONS    1000

// Not a real test, just performance indicator
static void test_qr_perf(int index, int debug) {

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
        /*  0*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    0, 37, 37, "107 chars, Mixed modes" },
        /*  1*/ { BARCODE_QRCODE, UNICODE_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 12345678901234567890123456 点点点点点点点点点点点点点点点点点点点点点点点点点点",
                    0, 105, 105, "963 chars, Mixed modes" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    clock_t start, total_encode = 0, total_buffer = 0, diff_encode, diff_buffer;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) {
        int j;

        if (index != -1 && i != index) continue;

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
    if (index != -1) {
        printf("totals: encode %gms, buffer %gms\n", total_encode * 1000.0 / CLOCKS_PER_SEC, total_buffer * 1000.0 / CLOCKS_PER_SEC);
    }
}

static void test_microqr_options(int index, int debug) {

    struct item {
        char *data;
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
        /* 49*/ { "123456789012345678901234567890123456", -1, -1, ZINT_ERROR_TOO_LONG, -1, 0, -1 }, // 35 absolute max
        /* 50*/ { "貫貫貫貫貫", -1, -1, 0, 0, 17, -1 }, // 5 Kanji max
        /* 51*/ { "貫貫貫貫貫貫", -1, -1, ZINT_ERROR_TOO_LONG, -1, 0, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    struct zint_symbol previous_symbol;

    testStart("test_microqr_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MICROQR, -1 /*input_mode*/, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);
        if (index == -1 && data[i].compare_previous != -1) {
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

static void test_microqr_input(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int option_3;
        char *data;
        int ret;
        char *expected;
        char *comment;
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
    // Á U+00C1, UTF-8 C381; ȁ U+0201, UTF-8 C881; Ȃ U+0202, UTF-8 C882; ¢ U+00A2, UTF-8 C2A2; á U+00E1, UTF-8 C3A1
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "é", 0, "87 A4 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1)" },
        /*  1*/ { DATA_MODE, -1, "é", 0, "8B 0E A4 00 EC 11 EC 11 00", "B2 (UTF-8)" },
        /*  2*/ { UNICODE_MODE, -1, "β", 0, "C8 80 00 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /*  3*/ { UNICODE_MODE, -1, "ก", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input data", "ก not in Shift JIS" },
        /*  4*/ { UNICODE_MODE, -1, "Ж", 0, "C8 91 C0 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /*  5*/ { UNICODE_MODE, -1, "ກ", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input data", "ກ not in Shift JIS" },
        /*  6*/ { UNICODE_MODE, -1, "\\", 0, "85 70 00 EC 11 EC 11 EC 00", "B1 (ASCII)" },
        /*  7*/ { UNICODE_MODE, -1, "¥", 0, "86 94 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1) (same bytes as ･ Shift JIS below, so ambiguous)" },
        /*  8*/ { UNICODE_MODE, -1, "･", 0, "86 94 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) single-byte codepoint A5 (same bytes as ¥ ISO 8859-1 above, so ambiguous)" },
        /*  9*/ { UNICODE_MODE, -1, "¿", 0, "86 FC 00 EC 11 EC 11 EC 00", "B1 (ISO 8859-1) (same bytes as ｿ Shift JIS below, so ambiguous)" },
        /* 10*/ { UNICODE_MODE, -1, "ｿ", 0, "86 FC 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) (same bytes as ¿ ISO 8859-1 above, so ambiguous)" },
        /* 11*/ { UNICODE_MODE, -1, "~", 0, "85 F8 00 EC 11 EC 11 EC 00", "B1 (ASCII) (same bytes as ‾ Shift JIS below, so ambiguous)" },
        /* 12*/ { UNICODE_MODE, -1, "‾", 0, "85 F8 00 EC 11 EC 11 EC 00", "B1 (Shift JIS) (same bytes as ~ ASCII above, so ambiguous)" },
        /* 13*/ { UNICODE_MODE, -1, "点", 0, "CB 67 C0 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 14*/ { DATA_MODE, -1, "\223\137", 0, "8A 4D 7C 00 EC 11 EC 11 00", "B2 (Shift JIS)" },
        /* 15*/ { DATA_MODE, 200, "\223\137", 0, "CB 67 C0 00 EC 11 EC 11 00", "K1 (Shift JIS) (full multibyte)" },
        /* 16*/ { DATA_MODE, -1, "点", 0, "8F 9E 0A E4 00 EC 11 EC 00", "B3 (UTF-8)" },
        /* 17*/ { UNICODE_MODE, -1, "茗", 0, "CE AA 80 00 EC 11 EC 11 00", "K1 (Shift JIS)" },
        /* 18*/ { DATA_MODE, -1, "\344\252", 0, "8B 92 A8 00 EC 11 EC 11 00", "B2 (Shift JIS)" },
        /* 19*/ { DATA_MODE, 200, "\344\252", 0, "CE AA 80 00 EC 11 EC 11 00", "K1 (Shift JIS) (full multibyte)" },
        /* 20*/ { DATA_MODE, -1, "茗", 0, "8F A2 32 5C 00 EC 11 EC 00", "B3 (UTF-8)" },
        /* 21*/ { UNICODE_MODE, -1, "¥点", 0, "8D 72 4D 7C 00 EC 11 EC 00", "B3 (Shift JIS) (optimized from B1 K1)" },
        /* 22*/ { DATA_MODE, -1, "\134\223\137", 0, "8D 72 4D 7C 00 EC 11 EC 00", "B3 (Shift JIS) (optimized from B1 K1)" },
        /* 23*/ { DATA_MODE, -1, "¥点", 0, "97 0A 97 9E 0A E4 00 EC 00", "B5 (UTF-8)" },
        /* 24*/ { UNICODE_MODE, -1, "点茗", 0, "D3 67 F5 54 00 EC 11 EC 00", "K2 (Shift JIS)" },
        /* 25*/ { DATA_MODE, -1, "\223\137\344\252", 0, "92 4D 7F 92 A8 00 EC 11 00", "B4 (Shift JIS)" },
        /* 26*/ { DATA_MODE, 200, "\223\137\344\252", 0, "D3 67 F5 54 00 EC 11 EC 00", "K2 (Shift JIS) (full multibyte)" },
        /* 27*/ { DATA_MODE, -1, "点茗", 0, "9B 9E 0A E7 A2 32 5C 00 00", "B6 (UTF-8)" },
        /* 28*/ { DATA_MODE, 200, "点茗", 0, "9B 9E 0A E7 A2 32 5C 00 00", "B6 (UTF-8)" },
        /* 29*/ { UNICODE_MODE, -1, "点茗･", 0, "D3 67 F5 55 0D 28 00 EC 00", "K2 B1 (Shift JIS)" },
        /* 30*/ { DATA_MODE, -1, "\223\137\344\252\245", 0, "96 4D 7F 92 AA 94 00 EC 00", "B5 (Shift JIS)" },
        /* 31*/ { DATA_MODE, 200, "\223\137\344\252\245", 0, "D3 67 F5 55 0D 28 00 EC 00", "K2 B1 (Shift JIS) (full multibyte)" },
        /* 32*/ { DATA_MODE, -1, "点茗･", 0, "A7 9E 0A E7 A2 32 5F BE F6 94 00", "B9 (UTF-8)" },
        /* 33*/ { UNICODE_MODE, -1, "¥点茗･", 0, "99 72 4D 7F 92 AA 94 00 00", "B6 (Shift JIS) (optimized from B1 K2 B1)" },
        /* 34*/ { DATA_MODE, -1, "\134\223\137\344\252\245", 0, "99 72 4D 7F 92 AA 94 00 00", "B6 (Shift JIS) (optimized from B1 K2 B1)" },
        /* 35*/ { DATA_MODE, -1, "¥点茗･", 0, "4B C2 A5 E7 82 B9 E8 8C 97 EF BD A5 00 00", "B11 (UTF-8)" },
        /* 36*/ { DATA_MODE, -1, "ÁȁȁȁȂ¢", 0, "4C C3 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00", "B12 (UTF-8)" },
        /* 37*/ { DATA_MODE, -1, "ÁȁȁȁȁȂ¢", 0, "4E C3 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00", "B14 (UTF-8)" },
        /* 38*/ { DATA_MODE, 200, "ÁȁȁȁȁȂ¢", 0, "41 C3 6C 08 80 44 02 20 11 00 88 0A 12 0D 10 00", "B1 K6 B1 (UTF-8) (full multibyte)" },
        /* 39*/ { UNICODE_MODE, -1, "áA", 0, "8B 85 04 00 EC 11 EC 11 00", "B2 (ISO 8859-1)" },
        /* 40*/ { UNICODE_MODE, 200, "áA", 0, "CE 00 40 00 EC 11 EC 11 00", "K1 (ISO 8859-1) (full multibyte)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_microqr_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_MICROQR, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_3, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Check MICROQR padding (4-bit final codeword for M1 and M3 in particular)
static void test_microqr_padding(int index, int generate, int debug) {

    struct item {
        char *data;
        int option_1;
        int ret;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { "1", -1, 0, "22 00 00", "M1, bits left 13" },
        /*  1*/ { "12", -1, 0, "43 00 00", "M1, bits left 10" },
        /*  2*/ { "123", -1, 0, "63 D8 00", "M1, bits left 7" },
        /*  3*/ { "1234", -1, 0, "83 DA 00", "M1, bits left 3" },
        /*  4*/ { "12345", -1, 0, "A3 DA D0", "M1, bits left 0" },
        /*  5*/ { "123456", 1, 0, "30 F6 E4 00 EC", "M2-L, bits left 15" },
        /*  6*/ { "1234567", 1, 0, "38 F6 E4 38 00", "M2-L, bits left 11" },
        /*  7*/ { "12345678", 1, 0, "40 F6 E4 4E 00", "M2-L, bits left 8" },
        /*  8*/ { "123456789", 1, 0, "48 F6 E4 62 A0", "M2-L, bits left 5" },
        /*  9*/ { "1234567890", 1, 0, "50 F6 E4 62 A0", "M2-L, bits left 1" },
        /* 10*/ { "1234", 2, 0, "20 F6 80 EC", "M2-M, bits left 13" },
        /* 11*/ { "123456", 2, 0, "30 F6 E4 00", "M2-M, bits left 7" },
        /* 12*/ { "1234567", 2, 0, "38 F6 E4 38", "M2-M, bits left 3" },
        /* 13*/ { "12345678", 2, 0, "40 F6 E4 4E", "M2-M, bits left 0" },
        /* 14*/ { "ABCDEF", 1, 0, "E3 9A 8A 54 28", "M2-L, bits left 3" },
        /* 15*/ { "ABCDE", 2, 0, "D3 9A 8A 4E", "M2-M, bits left 0" },
        /* 16*/ { "1234567890123456789", 1, 0, "26 3D B9 18 A8 18 AC D4 D2 00 00", "M3-L, bits left 13" },
        /* 17*/ { "12345678901234567890", 1, 0, "28 3D B9 18 A8 18 AC D4 D6 80 00", "M3-L, bits left 10" },
        /* 18*/ { "123456789012345678901", 1, 0, "2A 3D B9 18 A8 18 AC D4 DC 28 00", "M3-L, bits left 7" },
        /* 19*/ { "1234567890123456789012", 1, 0, "2C 3D B9 18 A8 18 AC D4 DC 29 00", "M3-L, bits left 3" },
        /* 20*/ { "12345678901234567890123", 1, 0, "2E 3D B9 18 A8 18 AC D4 DC 29 70", "M3-L, bits left 0" },
        /* 21*/ { "1234567890", 2, 0, "14 3D B9 18 A8 00 EC 11 00", "M3-M, bits left 27" },
        /* 22*/ { "123456789012345678", 2, 0, "24 3D B9 18 A8 18 AC D4 C0", "M3-M, bits left 1" },
        /* 23*/ { "ABCDEFGHIJKLMN", 1, 0, "78 E6 A2 95 0A B8 59 EB 99 7E A0", "M3-L, bits left 1" },
        /* 24*/ { "ABCDEFGHIJK", 2, 0, "6C E6 A2 95 0A B8 59 EA 80", "M3-M, bits left 1" },
        /* 25*/ { "1234567890123456789012345678", 1, 0, "0E 0F 6E 46 2A 06 2B 35 37 0A 75 46 F0 00 EC 11", "M4-L, bits left 25" },
        /* 26*/ { "123456789012345678901234567890", 1, 0, "0F 0F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 00 EC", "M4-L, bits left 19" },
        /* 27*/ { "1234567890123456789012345678901234", 1, 0, "11 0F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 F6 80", "M4-L, bits left 5" },
        /* 28*/ { "12345678901234567890123456789012345", 1, 0, "11 8F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0 F6 B4", "M4-L, bits left 1" },
        /* 29*/ { "123456789012345ABCDEFGHIJK", 1, 0, "07 8F 6E 46 2A 06 2B 25 67 35 14 A8 55 C2 CF 54", "M4-L, bits left 0" },
        /* 30*/ { "123456789012345678901234567890", 2, 0, "0F 0F 6E 46 2A 06 2B 35 37 0A 75 46 FB D0", "M4-M, bits left 3" },
        /* 31*/ { "123456789012345678901", 3, 0, "0A 8F 6E 46 2A 06 2B 35 37 0A", "M4-Q, bits left 1" },
        /* 32*/ { "ABCDEFGHIJKLMNOPQRSTU", 1, 0, "35 39 A8 A5 42 AE 16 7A E6 5F AC 51 95 B4 25 E0", "M4-L, bits left 4" },
        /* 33*/ { "ABCDEFGHIJKLMNOPQR", 2, 0, "32 39 A8 A5 42 AE 16 7A E6 5F AC 51 95 A0", "M4-M, bits left 5" },
        /* 34*/ { "ABCDEFGHIJKLM", 3, 0, "2D 39 A8 A5 42 AE 16 7A E6 56", "M4-Q, bits left 0" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_microqr_padding");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;
        if ((debug & ZINT_DEBUG_TEST_PRINT) && !(debug & ZINT_DEBUG_TEST_LESS_NOISY)) printf("i:%d\n", i);

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->symbology = BARCODE_MICROQR;
        symbol->input_mode = UNICODE_MODE;
        if (data[i].option_1 != -1) {
            symbol->option_1 = data[i].option_1;
        }
        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt
        symbol->debug |= debug;

        length = (int) strlen(data[i].data);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { \"%s\", %d, %s, \"%s\", \"%s\" },\n",
                    i, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].option_1,
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_microqr_optimize(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, -1, "1", 0, "22 00 00", "N1" },
        /*  1*/ { UNICODE_MODE, 1, 2, -1, "A123", 0, "92 86 3D 80 EC", "A1 N3" },
        /*  2*/ { UNICODE_MODE, 1, -1, -1, "AAAAAA", 0, "E3 98 73 0E 60", "A6" },
        /*  3*/ { UNICODE_MODE, 1, -1, -1, "AA123456", 0, "A3 98 61 ED C8", "A2 N6" },
        /*  4*/ { UNICODE_MODE, 1, 3, -1, "01a", 0, "04 06 16 10 00 EC 11 EC 11 EC 00", "N3 B1" },
        /*  5*/ { UNICODE_MODE, 1, 4, -1, "01a", 0, "43 30 31 61 00 00 EC 11 EC 11 EC 11 EC 11 EC 11", "B3" },
        /*  6*/ { UNICODE_MODE, 1, -1, -1, "こんwa、αβ", 0, "46 82 B1 82 F1 77 61 66 00 10 FF 88 00 00 EC 11", "B6 K3" },
        /*  7*/ { UNICODE_MODE, 1, -1, -1, "こんにwa、αβ", 0, "66 13 10 B8 85 25 09 DD 85 98 00 43 FE 20 00 00", "K3 B2 K3" },
        /*  8*/ { UNICODE_MODE, 1, 3, -1, "こんAB123\177", 0, "D0 4C 42 E2 91 CD 06 3D C2 FE 00", "K2 A2 N3 B1" },
        /*  9*/ { UNICODE_MODE, 1, 4, -1, "こんAB123\177", 0, "64 13 10 B8 92 9C D0 5E 1A 0B F8 00 EC 11 EC 11", "K2 A5 B1" },
        /* 10*/ { DATA_MODE, 1, -1, -1, "\223\137", 0, "8A 4D 7C 00 EC 11 EC 11 EC 11 00", "B2" },
        /* 11*/ { DATA_MODE, 1, -1, ZINT_FULL_MULTIBYTE, "\223\137", 0, "CB 67 C0 00 EC 11 EC 11 EC 11 00", "K1" },
        /* 12*/ { DATA_MODE, 1, -1, ZINT_FULL_MULTIBYTE | (1 << 8), "\223\137", 0, "CB 67 C0 00 EC 11 EC 11 EC 11 00", "K1" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_microqr_optimize");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_MICROQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_microqr_encode(int index, int generate, int debug) {

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
    struct item data[] = {
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
        /* 15*/ { UNICODE_MODE, -1, -1, -1, "点茗テ点茗テ点茗テ", 0, 17, 17, 0, "Max capacity M4-L 9 Kanji; BWIPP doesn't seem to deal with Kanji correctly",
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
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    testStart("test_microqr_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_MICROQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2, testUtilOption3Name(data[i].option_3),
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
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
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

// Not a real test, just performance indicator
static void test_microqr_perf(int index, int debug) {

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
        /*  0*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 1, "12345", 0, 11, 11, "Max 5 numbers, M1" },
        /*  1*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 2, "1234567890", 0, 13, 13, "Max 10 numbers, M2-L" },
        /*  2*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 3, "123456789012345", 0, 15, 15, "Max 15 numbers, M3-L" },
        /*  3*/ { BARCODE_MICROQR, UNICODE_MODE, 1, 4, "12345678901234567890123456789012345", 0, 17, 17, "Max 35 numbers, M4-L" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    clock_t start, total_encode = 0, total_buffer = 0, diff_encode, diff_buffer;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) {
        int j;

        if (index != -1 && i != index) continue;

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
    if (index != -1) {
        printf("totals: encode %gms, buffer %gms\n", total_encode * 1000.0 / CLOCKS_PER_SEC, total_buffer * 1000.0 / CLOCKS_PER_SEC);
    }
}

static void test_upnqr_input(int index, int generate, int debug) {

    struct item {
        int input_mode;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    // Ą U+0104 in ISO 8859-2 0xA1, in other ISO 8859 and Win 1250, UTF-8 C484
    // Ŕ U+0154 in ISO 8859-2 0xC0, in Win 1250 but not other ISO 8859 or Win page, UTF-8 C594
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "ĄŔ", 0, "(415) 70 44 00 02 A1 C0 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC", "ECI-4 B2 (ISO 8859-2)" },
        /*  1*/ { UNICODE_MODE, "é", 0, "(415) 70 44 00 01 E9 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11", "ECI-4 B1 (ISO 8859-2)" },
        /*  2*/ { UNICODE_MODE, "β", ZINT_ERROR_INVALID_DATA, "Error 572: Invalid character in input data for ECI 4", "β not in ISO 8859-2" },
        /*  3*/ { DATA_MODE, "\300\241", 0, "(415) 70 44 00 02 C0 A1 00 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC 11 EC", "ŔĄ" },
        /*  4*/ { GS1_MODE, "[20]12", ZINT_ERROR_INVALID_OPTION, "Error 220: Selected symbology does not support GS1 mode", "" },
        /*  5*/ { UNICODE_MODE, "ĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄ", 0, "(415) 70 44 01 9B A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1 A1", "" },
        /*  6*/ { UNICODE_MODE, "ĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄĄ", ZINT_ERROR_TOO_LONG, "Error 573: Input too long for selected symbol", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_upnqr_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_UPNQR, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);
        if (ret < ZINT_ERROR) {
            assert_equal(symbol->eci, 4, "i:%d ZBarcode_Encode symbol->eci %d != 4\n", i, symbol->eci);
        }

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret),
                    symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_upnqr_encode(int index, int generate, int debug) {

    struct item {
        int input_mode;
        char *data;
        int option_1;
        int option_2;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
        char *expected;
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
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_upnqr_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_UPNQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %d, %d, %s, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].data, data[i].option_1, data[i].option_2, testUtilErrorName(data[i].ret),
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

static void test_rmqr_options(int index, int debug) {

    struct item {
        char *data;
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
        /* 46*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点点茗点茗点茗点点茗点茗", 4, 26, 0, 0, 15, 99 }, // Max capacity ECC H, version 26 (R15x99), 28 kanji
        /* 47*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", 4, 26, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 48*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", 4, 32, 0, 0, 17, 139 }, // Max capacity ECC H, version 32 (R17x139), 46 kanji
        /* 49*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", 4, 32, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 50*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", -1, 32, 0, 0, 17, 139 }, // Max capacity ECC M, version 32, 92 kanji
        /* 51*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", 4, 32, ZINT_ERROR_TOO_LONG, -1, 0, 0 },
        /* 52*/ { "点茗点", -1, 33, 0, 0, 7, 43 }, // ECC auto-set to M, version 33 (R7xAuto-width) auto-sets R7x43
        /* 53*/ { "点茗点", 4, 33, 0, 0, 7, 59 }, // ECC set to H, version 33 (R7xAuto-width) auto-sets R7x59
        /* 54*/ { "点茗点", -1, 34, 0, 0, 9, 43 }, // ECC auto-set to H, version 34 (R9xAuto-width) auto-sets R9x43
        /* 55*/ { "点茗点", -1, 35, 0, 0, 11, 27 }, // ECC auto-set to M, version 35 (R11xAuto-width) auto-sets R11x27
        /* 56*/ { "点茗点茗点茗点", 4, 35, 0, 0, 11, 59 }, // ECC set to H, version 35 (R11xAuto-width) auto-sets R11x59
        /* 57*/ { "点茗点茗点茗点", -1, 35, 0, 0, 11, 43 }, // ECC auto-set to M, version 35 (R11xAuto-width) auto-sets R11x43
        /* 58*/ { "点茗点茗点茗点茗", -1, 36, 0, 0, 13, 43 }, // ECC auto-set to M, version 36 (R13xAuto-width) auto-sets R13x43
        /* 59*/ { "点茗点茗点茗点茗", 4, 36, 0, 0, 13, 59 }, // ECC set to H, version 36 (R13xAuto-width) auto-sets R13x59
        /* 60*/ { "点茗点茗点茗点茗点", -1, 37, 0, 0, 15, 43 }, // ECC auto-set to M, version 37 (R15xAuto-width) auto-sets R15x43
        /* 61*/ { "点茗点茗点茗点茗点", 4, 37, 0, 0, 15, 59 }, // ECC set to H, version 37 (R15xAuto-width) auto-sets R15x59
        /* 62*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗", -1, 38, 0, 0, 17, 43 }, // ECC auto-set to M, version 38 (R17xAuto-width) auto-sets R17x43
        /* 63*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗", 4, 38, 0, 0, 17, 77 }, // ECC set to H, version 38 (R17xAuto-width) auto-sets R17x77
        /* 64*/ { "点茗点", -1, 39, ZINT_ERROR_INVALID_OPTION, -1, 0, 0 },
        /* 65*/ { "点茗点", 4, -1, 0, 0, 13, 27 }, // ECC set to H, auto-sets R13x27
        /* 66*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", 4, -1, 0, 0, 15, 99 }, // ECC set to H, auto-sets R15x99 (max capacity)
        /* 67*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点", 4, -1, 0, 0, 17, 99 }, // ECC set to H, auto-sets R17x99
        /* 68*/ { "点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗点茗", 4, -1, 0, 0, 17, 139 }, // ECC set to H, auto-sets R17x139 (max capacity)
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_rmqr_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, UNICODE_MODE, -1 /*eci*/, data[i].option_1, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret_encode, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret_encode, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

            ret = ZBarcode_Buffer_Vector(symbol, 0);
            assert_equal(ret, data[i].ret_vector, "i:%d ZBarcode_Buffer_Vector ret %d != %d\n", i, ret, data[i].ret_vector);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_input(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int option_3;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    // ก U+0E01 in ISO 8859-11 Thai (but not other ISO 8859 or Win page), not in Shift JIS, UTF-8 E0B881
    // Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447, UTF-8 D096
    // ກ U+0E81 Lao not in any ISO 8859 (or Win page) or Shift JIS, UTF-8 E0BA81
    // ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5
    // 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9
    // Á U+00C1, UTF-8 C381; ȁ U+0201, UTF-8 C881; Ȃ U+0202, UTF-8 C882; ¢ U+00A2, UTF-8 C2A2
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, "é", 0, "67 A4 00 EC 11", "B1 (ISO 8859-1)" },
        /*  1*/ { DATA_MODE, -1, "é", 0, "6B 0E A4 00 EC", "B2 (UTF-8)" },
        /*  2*/ { DATA_MODE, -1, "\351", 0, "67 A4 00 EC 11", "B1 (ISO 8859-1)" },
        /*  3*/ { UNICODE_MODE, -1, "β", 0, "88 80 00 EC 11", "K1 (Shift JIS)" },
        /*  4*/ { UNICODE_MODE, -1, "ก", ZINT_ERROR_INVALID_DATA, "Error 800: Invalid character in input data", "ก not in ISO 8859-1 or Shift JIS" },
        /*  5*/ { UNICODE_MODE, -1, "Ж", 0, "88 91 C0 EC 11", "K1 (Shift JIS)" },
        /*  6*/ { UNICODE_MODE, -1, "¥･点", 0, "71 72 96 4D 7C", "B2 K1 (Shift JIS) (optimized to byte mode only)" },
        /*  7*/ { DATA_MODE, -1, "ÁȁȁȁȂ¢", 0, "6C C3 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00 EC 11 EC 11 EC", "B12 (UTF-8)" },
        /*  8*/ { DATA_MODE, -1, "ÁȁȁȁȁȂ¢", 0, "6E C3 81 C8 81 C8 81 C8 81 C8 81 C8 82 C2 A2 00 EC 11 EC", "B14 (UTF-8)" },
        /*  9*/ { DATA_MODE, 200, "ÁȁȁȁȁȂ¢", 0, "61 C3 8C 08 80 44 02 20 11 00 88 0A 13 0D 10 EC 11 EC 11", "B1 K6 B1 (UTF-8) (full multibyte)" },
        /* 10*/ { UNICODE_MODE, -1, "áA", 0, "6B 85 04 00 EC", "B2 (ISO 8859-1)" },
        /* 11*/ { UNICODE_MODE, 200, "áA", 0, "8E 00 40 EC 11", "K1 (ISO 8859-1) (full multibyte)" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_rmqr_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_3, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_gs1(int index, int generate, int debug) {

    struct item {
        int input_mode;
        char *data;
        int ret;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { GS1_MODE, "[01]12345678901231", 0, "A6 00 59 D5 1B EF 43 D8 80 EC 11 EC", "N16" },
        /*  1*/ { GS1_MODE | GS1PARENS_MODE, "(01)12345678901231", 0, "A6 00 59 D5 1B EF 43 D8 80 EC 11 EC", "N16" },
        /*  2*/ { GS1_MODE, "[01]04912345123459[15]970331[30]128[10]ABC123", 0, "A5 D0 29 EB 3A A0 05 67 93 F2 94 B4 B4 E2 4E AF 01 47 EC 17 86", "N29 A9" },
        /*  3*/ { GS1_MODE | GS1PARENS_MODE, "(01)04912345123459(15)970331(30)128(10)ABC123", 0, "A5 D0 29 EB 3A A0 05 67 93 F2 94 B4 B4 E2 4E AF 01 47 EC 17 86", "N29 A9" },
        /*  4*/ { GS1_MODE, "[91]12%[20]12", 0, "A4 9C 79 32 25 1D 24 32 48 00 EC 11", "N4 B2 N4" },
        /*  5*/ { GS1_MODE, "[91]123%[20]12", 0, "A4 BC 79 74 3D A9 31 21 92 40 EC 11", "N5 A2 N4" },
        /*  6*/ { GS1_MODE, "[91]1234%[20]12", 0, "A4 DC 79 D4 C8 94 74 90 C9 20 EC 11", "N6 B2 N4" },
        /*  7*/ { GS1_MODE, "[91]12345%[20]12", 0, "A4 FC 79 D4 A8 7B 52 62 43 24 80 EC", "N7 A2(3) N4" },
        /*  8*/ { GS1_MODE, "[91]1A%[20]12", 0, "A8 E6 58 1B ED 49 89 0C 92 00 EC 11", "A6(7) N4" },
        /*  9*/ { GS1_MODE, "[91]%%[20]12", 0, "A4 56 D9 92 92 8E 92 19 24 00 EC 11", "N2 B3 N4" },
        /* 10*/ { GS1_MODE, "[91]%%%[20]12", 0, "A4 56 DA 12 92 92 8E 92 19 24 00 EC", "N2 B4 N4" },
        /* 11*/ { GS1_MODE, "[91]A%%%%12345678A%A", 0, "A8 A6 58 F4 4C C6 4A 4A 4A 48 1E DC 89 C8 87 A3 5C 00 EC", "A4(5) B3 N8 A3(4)" },
        /* 12*/ { GS1_MODE, "[91]%23%%6789%%%34567%%%%234%%%%%", 0, "(33) AA 63 2D B5 02 EE D4 DA 84 54 B8 ED 4D A9 B5 04 58 E7 2C 3B 53 6A 6D 4D 60 22 F6 A3", "A27(38) B4" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_rmqr_gs1");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1, -1, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_optimize(int index, int generate, int debug) {

    struct item {
        int input_mode;
        char *data;
        int option_1;
        int ret;
        char *expected;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, "1", -1, 0, "22 20 EC 11 EC", "N1" },
        /*  1*/ { UNICODE_MODE, "AAA", -1, 0, "46 73 0A 00 EC", "A3" },
        /*  2*/ { UNICODE_MODE, "0123456789", -1, 0, "34 06 2B 35 34 80 EC", "N10 (nayuki.io - pure numeric)" },
        /*  3*/ { UNICODE_MODE, "ABCDEF", -1, 0, "4C 73 51 4A 85 00 EC", "A6 (nayuki.io - pure alphanumeric)" },
        /*  4*/ { UNICODE_MODE, "wxyz", -1, 0, "71 DD E1 E5 E8", "B4 (nayuki.io - pure byte)" },
        /*  5*/ { UNICODE_MODE, "「魔法少女まどか☆マギカ」って、　ИАИ　ｄｅｓｕ　κα？", -1, 0, "(53) 8E 80 D6 00 4F C0 57 6A B5 C2 B8 14 70 94 81 64 37 A1 8D 0C 50 0D 82 82 14 40 00 80", "K29 (nayuki.io - pure kanji)" },
        /*  6*/ { UNICODE_MODE, "012A", -1, 0, "48 00 43 20 EC", "A4" },
        /*  7*/ { UNICODE_MODE, "0123A", -1, 0, "28 06 1A 12 80", "N4 A1 (nayuki.io - alpha/numeric)" },
        /*  8*/ { UNICODE_MODE, "0a", -1, 0, "68 C1 84 00 EC", "B2 (nayuki.io - numeric/byte)" },
        /*  9*/ { UNICODE_MODE, "01a", -1, 0, "24 05 96 10 EC", "N2 B1 (nayuki.io - numeric/byte)" },
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
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_rmqr_optimize");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug |= ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, -1, ZINT_FULL_MULTIBYTE, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, \"%s\", %d, %s, \"%s\", \"%s\" },\n",
                    i, testUtilInputModeName(data[i].input_mode), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].option_1,
                    testUtilErrorName(data[i].ret), symbol->errtxt, data[i].comment);
        } else {
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rmqr_encode(int index, int generate, int debug) {

    struct item {
        int input_mode;
        int option_1;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    // ISO/IEC JTC1/SC31N000 (Draft 2019-6-24)
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 4, 11, "0123456", 0, 11, 27, 1, "Draft ISO 2019-6-24 Annex I Figure I.2, R11x27-H, same",
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
        /*  1*/ { UNICODE_MODE, 2, 17, "12345678901234567890123456", 0, 13, 27, 1, "Draft ISO 2019-6-24 6.2 Figure 1, R13x27-M, same (note data as here not as given in draft)",
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
        /*  2*/ { UNICODE_MODE, 2, 2, "0123456789012345", 0, 7, 59, 1, "Draft ISO 2019-6-24 7.4.2 Numeric mode Example, R7x59-M, same codewords",
                    "11111110101010101011101010101010101010111010101010101010111"
                    "10000010101111011110100001100001100001101100100101100100101"
                    "10111010100100001011110010110000011110111110111100011011111"
                    "10111010110001100010010100111010101101101011111000110110001"
                    "10111010011001000011100111100000110010111011010111011010101"
                    "10000010101010110110100010111110010010101111101111110010001"
                    "11111110101010101011101010101010101010111010101010101011111"
                },
        /*  3*/ { UNICODE_MODE, 2, 2, "AC-42", 0, 7, 59, 0, "Draft ISO 2019-6-24 7.4.3 Alphanumeric mode Example, R7x59-M, same codewords; BWIPP different encodation",
                    "11111110101010101011101010101010101010111010101010101010111"
                    "10000010101111010010110011010101100000101011001111100100101"
                    "10111010100100100011100100111100011101111100011011111011111"
                    "10111010110001110110100001101110100111101110000010010110001"
                    "10111010011000110111111101110000110001111000100110111010101"
                    "10000010101010111110100011101110011011101101011010110010001"
                    "11111110101010101011101010101010101010111010101010101011111"
                },
        /*  4*/ { UNICODE_MODE, -1, 1, "123456789012", 0, 7, 43, 1, "R7x34-M max numeric 12 digits",
                    "1111111010101010101011101010101010101010111"
                    "1000001001010111111010100000100011011000101"
                    "1011101010111000100111101101011010111111111"
                    "1011101001100110111000000001111111000010001"
                    "1011101000101100011111100110111110110010101"
                    "1000001011110111111110101101001010111010001"
                    "1111111010101010101011101010101010101011111"
                },
        /*  5*/ { UNICODE_MODE, 4, 32, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678", 0, 17, 139, 1, "R17x139-H max numeric 178 digits",
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
        /*  6*/ { UNICODE_MODE, 4, 33, "123456789012345678901234567890123456789012345678901234", 0, 7, 139, 1, "R7xauto-H max numeric 54 digits",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001011010011001111100110101000111000110001101010101011100001010000000000011001101000110010111100010100100110101000010111100011001000001"
                    "1011101000011001010110011111111001010000100001011100101110111001110001011101010011111000110100100000100100111011100000000010010010101011111"
                    "1011101001110011010110011110000111010000111100110001101000100000110000000101101100000011000100100101101000000010100100101000110010000010001"
                    "1011101011101101111101010011101111111011101110010111101110110111111000111001010000111011011001100000111000001111111111101001100011001010101"
                    "1000001010001100101101000010110110010110000111100001011011001001011001111000100111101011001110101000010111101110101110000101110011110110001"
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
        /*  7*/ { UNICODE_MODE, 4, 35, "123456789012345678901234567890123", 0, 11, 59, 1, "R11xauto-H with max numeric 33 digits for width 59",
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
        /*  8*/ { UNICODE_MODE, -1, 38, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", 0, 17, 139, 1, "R17xauto-M max numeric 361 digits",
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
        /*  9*/ { UNICODE_MODE, 2, 1, "\001\002\003\004\005", 0, 7, 43, 1, "R7x43-M with max 5 binary",
                    "1111111010101010101011101010101010101010111"
                    "1000001001010001101010101011001111011000101"
                    "1011101010111101100111100101110001011111111"
                    "1011101001101111011110011001101001000010001"
                    "1011101000100000100111110110101110110010101"
                    "1000001011110000000010101011001111011010001"
                    "1111111010101010101011101010101010101011111"
                },
        /* 10*/ { UNICODE_MODE, 4, 1, "\001\002", 0, 7, 43, 1, "R7x43-H with max 2 binary",
                    "1111111010101010101011101010101010101010111"
                    "1000001011010010110010111001001111001000001"
                    "1011101011010011101011110001000001010011111"
                    "1011101010111101110110001010001001011010001"
                    "1011101001111011011011111111001110011010101"
                    "1000001001111110100010111110110110101010001"
                    "1111111010101010101011101010101010101011111"
                },
        /* 11*/ { UNICODE_MODE, 2, 1, "ABCDEFG", 0, 7, 43, 1, "R7x43-M with max 7 alphanumerics",
                    "1111111010101010101011101010101010101010111"
                    "1000001001011100111110111000000101011000101"
                    "1011101010111010101011110111101001011111111"
                    "1011101001100001111100011111100010100010001"
                    "1011101000110011100011111110010100110010101"
                    "1000001011111011101110111110000111111010001"
                    "1111111010101010101011101010101010101011111"
                },
        /* 12*/ { UNICODE_MODE, 2, 1, "点茗点", 0, 7, 43, 0, "R7x43-M with max 3 Kanji; BWIPP different encodation",
                    "1111111010101010101011101010101010101010111"
                    "1000001001011000100010101001010001111000101"
                    "1011101010110101110111111001011101111111111"
                    "1011101001100001101010100110100110100010001"
                    "1011101000100010000011101101011011110010101"
                    "1000001011111100010110101111011000111010001"
                    "1111111010101010101011101010101010101011111"
                },
        /* 13*/ { UNICODE_MODE, 2, 4, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM", 0, 7, 99, 1, "R7x99-M with max 39 alphanumerics",
                    "111111101010101010101011101010101010101010101010111010101010101010101010101110101010101010101010111"
                    "100000100111110010010010101010100110101001000011101100101101000010101110001011100000010001111100101"
                    "101110100101000000101011100100010000001000001111111111011111000000110101111111101111100000100011111"
                    "101110100100011111000100010111001001100101111011100001000001001111101010111010110110100011000110001"
                    "101110101000101011110111101110010000110011110101111110100111100010101000101111101101101111000110101"
                    "100000101010111010111010111110101001111001101011101010101010001100101101111011100101101010110010001"
                    "111111101010101010101011101010101010101010101010111010101010101010101010101110101010101010101011111"
                },
        /* 14*/ { UNICODE_MODE, 4, 5, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026", 0, 7, 139, 1, "R7x139-H with max 22 binary",
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101010111"
                    "1000001011010100100110110110100110111000001010100110101010111101000011111010001000101100000100001011011110111110100110011110001101001000001"
                    "1011101000010011101011000111111011101001111011110101011110111001000010011111001110111001100001010101110110100011110011110001010001001011111"
                    "1011101001111010000010110010111010001000101110001010101000101110001111011000010111001101000001010101101001110100110011110010000001000010001"
                    "1011101011110101001101101111100101000110001011010111101111110111111011101010001110111100001101011100001010011111111110111010001101001010101"
                    "1000001010000100011110111110111000111001100010111001111011001011011111000110010101101111111111001110101110101010101110110010001100110110001"
                    "1111111010101010101010101011101010101010101010101010101110101010101010101010101010111010101010101010101010101011101010101010101010101011111"
                },
        /* 15*/ { UNICODE_MODE, 2, 7, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024", 0, 9, 59, 1, "R9x59-M with max 20 binary",
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
        /* 16*/ { UNICODE_MODE, 2, 9, "12345678901234567890123456789012345678901234567", 0, 9, 99, 1, "R9x99-M with max 47 numerics",
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
        /* 17*/ { UNICODE_MODE, 2, 11, "\001\002\003\004\005\006", 0, 11, 27, 1, "R11x27-M with max 6 binary",
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
        /* 18*/ { UNICODE_MODE, 4, 11, "\001\002\003\004", 0, 11, 27, 1, "R11x27-H with max 4 binary",
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
        /* 19*/ { UNICODE_MODE, 2, 13, "12345678901234567890123456789012345678901234567890123456789012345678901", 0, 11, 59, 1, "R11x59-M with max 71 numerics",
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
        /* 20*/ { UNICODE_MODE, 2, 18, "1234567890123456789012345678901234567890123456789012345678901", 0, 13, 43, 1, "R13x43-M with max 61 numerics",
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
        /* 21*/ { UNICODE_MODE, 4, 20, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033", 0, 13, 77, 1, "R13x77-H with max 27 binary",
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
        /* 22*/ { UNICODE_MODE, 2, 20, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123", 0, 13, 77, 1, "R13x77-M with max 123 numerics (note 8 bit cci in draft 2019-6-24 when 7 suffices)",
                    "11111110101010101010101011101010101010101010101010111010101010101010101010111"
                    "10000010001111011110000010110000001101001111110110101010010010110011111011101"
                    "10111010001010000001000011100001010010100110011011111001000001111110100011001"
                    "10111010010011101110100110011001011011110110011101100011001010000010000010000"
                    "10111010010010111110100011010101011101110000000101110101111110010001000110011"
                    "10000010011110000011110100011001011101111111110110000100110010110011101011000"
                    "11111110111010011010000011010010110010010111011011111111010101111110110011111"
                    "00000000111011110111001110010000001000110001011101100110000110000010110101110"
                    "11111111010011101000010001110101101101110010000101011101110010000001101111111"
                    "01101010101001000000100000100000111110111111111011100100110100001011100110001"
                    "10110000001110100010011011100001011011010111011110111011010101110111111110101"
                    "10101101110110111100111010111111000100010001001101101110100100010100001010001"
                    "11101010101010101010101011101010101010101010101010111010101010101010101011111"
                },
        /* 23*/ { UNICODE_MODE, 2, 23, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", 0, 15, 43, 1, "R15x43-M with max 31 binary",
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
        /* 24*/ { UNICODE_MODE, 4, 27, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST", 0, 15, 139, 1, "R15x139-H with max 98 alphanumerics",
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
        /* 25*/ { UNICODE_MODE, 2, 28, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 0, 17, 43, 1, "R17x43-M with max 90 numerics",
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
        /* 26*/ { UNICODE_MODE, 4, 29, "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032", 0, 17, 59, 1, "R17x59-H with max 26 binary",
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
        /* 27*/ { UNICODE_MODE, 2, 32, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD", 0, 17, 139, 1, "R17x139-M with max 108 alphanumerics",
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
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];
    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    testStart("test_rmqr_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_RMQR, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, ZINT_FULL_MULTIBYTE, -1 /*output_options*/, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, \"%s\", %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_1, data[i].option_2,
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
                        ret = testUtilBwipp(i, symbol, data[i].option_1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
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
        { "test_qr_options", test_qr_options, 1, 0, 1 },
        { "test_qr_input", test_qr_input, 1, 1, 1 },
        { "test_qr_gs1", test_qr_gs1, 1, 1, 1 },
        { "test_qr_optimize", test_qr_optimize, 1, 1, 1 },
        { "test_qr_encode", test_qr_encode, 1, 1, 1 },
        { "test_qr_perf", test_qr_perf, 1, 0, 1 },

        { "test_microqr_options", test_microqr_options, 1, 0, 1 },
        { "test_microqr_input", test_microqr_input, 1, 1, 1 },
        { "test_microqr_padding", test_microqr_padding, 1, 1, 1 },
        { "test_microqr_optimize", test_microqr_optimize, 1, 1, 1 },
        { "test_microqr_encode", test_microqr_encode, 1, 1, 1 },
        { "test_microqr_perf", test_microqr_perf, 1, 0, 1 },

        { "test_upnqr_input", test_upnqr_input, 1, 1, 1 },
        { "test_upnqr_encode", test_upnqr_encode, 1, 1, 1 },

        { "test_rmqr_options", test_rmqr_options, 1, 0, 1 },
        { "test_rmqr_input", test_rmqr_input, 1, 1, 1 },
        { "test_rmqr_gs1", test_rmqr_gs1, 1, 1, 1 },
        { "test_rmqr_optimize", test_rmqr_optimize, 1, 1, 1 },
        { "test_rmqr_encode", test_rmqr_encode, 1, 1, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
