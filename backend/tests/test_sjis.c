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
/* vim: set ts=4 sw=4 et : */

#include "testcommon.h"
#include "test_sjis_tab.h"
#include "../sjis.h"

// As control convert to Shift JIS using simple table generated from https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/JIS/SHIFTJIS.TXT plus simple processing
static int sjis_wctomb_zint2(unsigned int *r, unsigned int wc) {
    int tab_length, start_i, end_i;
    int i;
    if (wc < 0x20 || wc == 0x7F) {
        *r = wc;
        return 1;
    }
    // Shortcut
    if ((wc > 0x00F7 && wc < 0x0391) || (wc > 0x0451 && wc < 0x2010) || (wc > 0x9FA0 && wc < 0xE000) || (wc > 0xE757 && wc < 0xFF01) || wc > 0xFFE5) {
        return 0;
    }
    if (wc >= 0xE000 && wc <= 0xE757) { // PUA mappings, not in SHIFTJIS.TXT
        if (wc <= 0xE0BB) {
            *r = wc - 0xE000 + 0xF040 + (wc >= 0xE000 + 0x3F);
        } else if (wc <= 0xE177) {
            *r = wc - 0xE0BC + 0xF140 + (wc >= 0xE0BC + 0x3F);
        } else if (wc <= 0xE233) {
            *r = wc - 0xE178 + 0xF240 + (wc >= 0xE178 + 0x3F);
        } else if (wc <= 0xE2EF) {
            *r = wc - 0xE234 + 0xF340 + (wc >= 0xE234 + 0x3F);
        } else if (wc <= 0xE3AB) {
            *r = wc - 0xE2F0 + 0xF440 + (wc >= 0xE2F0 + 0x3F);
        } else if (wc <= 0xE467) {
            *r = wc - 0xE3AC + 0xF540 + (wc >= 0xE3AC + 0x3F);
        } else if (wc <= 0xE523) {
            *r = wc - 0xE468 + 0xF640 + (wc >= 0xE468 + 0x3F);
        } else if (wc <= 0xE5DF) {
            *r = wc - 0xE524 + 0xF740 + (wc >= 0xE524 + 0x3F);
        } else if (wc <= 0xE69B) {
            *r = wc - 0xE5E0 + 0xF840 + (wc >= 0xE5E0 + 0x3F);
        } else {
            *r = wc - 0xE69C + 0xF940 + (wc >= 0xE69C + 0x3F);
        }
        return 2;
    }
    tab_length = sizeof(test_sjis_tab) / sizeof(unsigned int);
    start_i = test_sjis_tab_ind[wc >> 10];
    end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    for (i = start_i; i < end_i; i += 2) {
        if (test_sjis_tab[i + 1] == wc) {
            *r = test_sjis_tab[i];
            return *r > 0xFF ? 2 : 1;
        }
    }
    return 0;
}

static void test_sjis_wctomb_zint(void) {

    int ret, ret2;
    unsigned int val, val2;
    unsigned int i;

    testStart("test_sjis_wctomb_zint");

    for (i = 0; i < 0xFFFE; i++) {
        if (i >= 0xD800 && i <= 0xDFFF) { // UTF-16 surrogates
            continue;
        }
        val = val2 = 0;
        ret = sjis_wctomb_zint(&val, i);
        ret2 = sjis_wctomb_zint2(&val2, i);
        if (i == 0xFF3C) { // Extra mapping full-width reverse solidus U+FF3C to 0x815F, duplicate of U+005C (backslash)
            assert_equal(ret, 2, "i:%d 0x%04X ret %d != 2, val 0x%04X\n", (int) i, i, ret, val);
            assert_equal(val, 0x815F, "i:%d 0x%04X val 0x%04X != 0x815F\n", (int) i, i, val);
            assert_zero(ret2, "i:%d 0x%04X ret2 %d != 0, val2 0x%04X\n", (int) i, i, ret2, val2);
        } else {
            assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
        }
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
        }
    }

    testFinish();
}

static void test_sjis_utf8(int index) {

    struct item {
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_jisdata[20];
        char *comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, not in Shift JIS, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in Shift JIS 0x83C0, UTF-8 CEB2
    // Ж U+0416 in ISO 8859-5 Cyrillic (but not other ISO 8859), Win 1251, in Shift JIS 0x8447, UTF-8 D096
    // ¥ U+00A5 in ISO 8859-1 0xA5 (\245), in Shift JIS single-byte 0x5C (\134) (backslash); 0xA5 same codepoint as single-byte half-width katakana ･ (U+FF65) in Shift JIS (below), UTF-8 C2A5
    // ･ U+FF65 half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xA5 (\245), UTF-8 EFBDA5
    // ｿ U+FF7F half-width katakana, not in ISO/Win, in Shift JIS single-byte 0xBF (\277), UTF-8 EFBDBF
    // ‾ U+203E overline, not in ISO/Win, in Shift JIS single-byte 0x7E (\176) (tilde), UTF-8 E280BE
    // ＼ U+FF3C full-width reverse solidus, in Shift JIS 0x815F, duplicate of mapping of U+005C, UTF-8 EFBCBC
    // 点 U+70B9 kanji, in Shift JIS 0x935F (\223\137), UTF-8 E782B9
    // 茗 U+8317 kanji, in Shift JIS 0xE4AA (\344\252), UTF-8 E88C97
    // テ U+30C6 katakana, in Shift JIS 0x8365 (\203\145), UTF-8 E38386
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "é", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "" },
        /*  1*/ { "~", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "" },
        /*  2*/ { "β", -1, 0, 1, { 0x83C0 }, "" },
        /*  3*/ { "¥", -1, 0, 1, { 0x5C }, "" },
        /*  4*/ { "aβcЖ¥･ｿ‾\\＼点茗テ", -1, 0, 13, { 'a', 0x83C0, 'c', 0x8447, 0x5C, 0xA5, 0xBF, 0x7E, 0x815F, 0x815F, 0x935F, 0xE4AA, 0x8365 }, "" },
        /*  5*/ { "\200", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Invalid UTF-8" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};
    unsigned int jisdata[20];

    testStart("test_sjis_utf8");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = sjis_utf8(&symbol, (unsigned char *) data[i].data, &ret_length, jisdata);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < ret_length; j++) {
                assert_equal(jisdata[j], data[i].expected_jisdata[j], "i:%d jisdata[%d] %04X != %04X\n", i, j, jisdata[j], data[i].expected_jisdata[j]);
            }
        }
    }

    testFinish();
}

static void test_sjis_utf8_to_eci(int index) {

    struct item {
        int eci;
        int full_multibyte;
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_jisdata[20];
        char *comment;
    };
    // é U+00E9 in ISO 8859-1 0xE9, Win 1250 plus other Win, in QR Kanji mode first byte range 0x81..9F, 0xE0..EB
    // β U+03B2 in ISO 8859-7 Greek 0xE2 (but not other ISO 8859 or Win page)
    // ¥ U+00A5 in ISO 8859-1 0xA5, outside first byte range 0x81..9F, 0xE0..EB
    // ú U+00FA in ISO 8859-1 0xFA, outside first byte range
    // à U+00EO in ISO 8859-1 0xE0, in first byte range
    // ë U+00EB in ISO 8859-1 0xEB, in first byte range
    // ì U+00EC in ISO 8859-1 0xEC, outside first byte range
    // µ U+00B5 in ISO 8859-1 0xB5, outside first byte range
    // À U+00C0 in ISO 8859-1 0xC0, outside first byte range and 0xEBxx second byte range
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 3, 0, "é", -1, 0, 1, { 0xE9 }, "" },
        /*  1*/ { 3, 1, "é", -1, 0, 1, { 0xE9 }, "" },
        /*  2*/ { 3, 0, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "" },
        /*  3*/ { 3, 1, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "" },
        /*  4*/ { 9, 0, "β", -1, 0, 1, { 0xE2 }, "" },
        /*  5*/ { 9, 1, "β", -1, 0, 1, { 0xE2 }, "" },
        /*  6*/ { 3, 0, "¥", -1, 0, 1, { 0xA5 }, "" },
        /*  7*/ { 3, 1, "¥", -1, 0, 1, { 0xA5 }, "" },
        /*  8*/ { 3, 0, "éa", -1, 0, 2, { 0xE9, 0x61 }, "Not full multibyte" },
        /*  9*/ { 3, 1, "éa", -1, 0, 1, { 0xE961 }, "In QR Kanji mode range" },
        /* 10*/ { 3, 0, "éaúbàcëdìeµ", -1, 0, 11, { 0xE9, 0x61, 0xFA, 0x62, 0xE0, 0x63, 0xEB, 0x64, 0xEC, 0x65, 0xB5 }, "" },
        /* 11*/ { 3, 1, "éaúbàcëdìeµ", -1, 0, 8, { 0xE961, 0xFA, 0x62, 0xE063, 0xEB64, 0xEC, 0x65, 0xB5 }, "" },
        /* 12*/ { 3, 0, "ëÀ", -1, 0, 2, { 0xEB, 0xC0 }, "Not full multibyte" },
        /* 13*/ { 3, 1, "ëÀ", -1, 0, 2, { 0xEB, 0xC0 }, "Outside QR Kanji mode range" },
        /* 14*/ { 20, 0, "\\\\", -1, 0, 4, { 0x81, 0x5F, 0x81, 0x5F }, "Shift JIS reverse solidus (backslash) mapping from ASCII to double byte" },
        /* 15*/ { 20, 1, "\\\\", -1, 0, 2, { 0x815F, 0x815F }, "Shift JIS reverse solidus (backslash) mapping from ASCII to double byte" },
        /* 16*/ { 20, 0, "爍", -1, 0, 2, { 0xE0, 0xA1 }, "Shift JIS U+720D" },
        /* 17*/ { 20, 1, "爍", -1, 0, 1, { 0xE0A1 }, "Shift JIS" },
        /* 18*/ { 20, 0, "~", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "ASCII tilde not in Shift JIS" },
        /* 19*/ { 25, 0, "12", -1, 0, 4, { 0x00, 0x31, 0x00, 0x32 }, "UCS-2BE ASCII" },
        /* 20*/ { 25, 0, "", -1, 0, 4, { 0x00, 0x81, 0x00, 0x81 }, "UCS-2BE U+0081" },
        /* 21*/ { 25, 1, "", -1, 0, 4, { 0x00, 0x81, 0x00, 0x81 }, "UCS-2BE outside QR Kanji mode range" },
        /* 22*/ { 25, 0, "腀", -1, 0, 2, { 0x81, 0x40 }, "UCS-2BE U+8140" },
        /* 23*/ { 25, 1, "腀", -1, 0, 1, { 0x8140 }, "UCS-2BE in QR Kanji mode range" },
        /* 24*/ { 28, 0, "¢¢", -1, 0, 4, { 0xA2, 0x46, 0xA2, 0x46 }, "Big5 U+00A2" },
        /* 25*/ { 28, 1, "¢¢", -1, 0, 4, { 0xA2, 0x46, 0xA2, 0x46 }, "Big5 outside QR Kanji mode range" },
        /* 26*/ { 28, 0, "觡", -1, 0, 2, { 0xE0, 0x40 }, "Big5 U+89E1" },
        /* 27*/ { 28, 1, "觡", -1, 0, 1, { 0xE040 }, "Big5 in QR Kanji mode range" },
        /* 28*/ { 29, 0, "¨¨", -1, 0, 4, { 0xA1, 0xA7, 0xA1, 0xA7 }, "GB 2312 U+00A8" },
        /* 29*/ { 29, 1, "¨¨", -1, 0, 4, { 0xA1, 0xA7, 0xA1, 0xA7 }, "GB 2312 outside QR Kanji mode range" },
        /* 30*/ { 29, 0, "崂", -1, 0, 2, { 0xE1, 0xC0 }, "GB 2312 U+5D02" },
        /* 31*/ { 29, 0, "釦", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "GB 18030 U+91E6 not in GB 2312" },
        /* 32*/ { 29, 1, "崂", -1, 0, 1, { 0xE1C0 }, "GB 2312 in QR Kanji mode range" },
        /* 33*/ { 30, 0, "¡¡", -1, 0, 4, { 0x22 + 0x80, 0x2E + 0x80, 0x22 + 0x80, 0x2E + 0x80 }, "EUC-KR U+00A1 (0xA2AE)" },
        /* 34*/ { 30, 1, "¡¡", -1, 0, 4, { 0x22 + 0x80, 0x2E + 0x80, 0x22 + 0x80, 0x2E + 0x80 }, "EUC-KR 0xA2AE outside QR Kanji mode range" },
        /* 35*/ { 30, 0, "詰", -1, 0, 2, { 0x7D + 0x80, 0x7E + 0x80 }, "EUC-KR U+8A70 (0xFDFE)" },
        /* 36*/ { 30, 1, "詰", -1, 0, 2, { 0x7D + 0x80, 0x7E + 0x80 }, "EUC-KR > 0xEBBF so not in QR Kanji mode range" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    unsigned int jisdata[20];

    testStart("test_sjis_utf8_to_eci");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = sjis_utf8_to_eci(data[i].eci, (unsigned char *) data[i].data, &ret_length, jisdata, data[i].full_multibyte);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < ret_length; j++) {
                assert_equal(jisdata[j], data[i].expected_jisdata[j], "i:%d jisdata[%d] 0x%04X != 0x%04X\n", i, j, jisdata[j], data[i].expected_jisdata[j]);
            }
        }
    }

    testFinish();
}

static void test_sjis_cpy(int index) {

    struct item {
        int full_multibyte;
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_jisdata[20];
        char *comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 0, "\351", -1, 0, 1, { 0xE9 }, "Not full multibyte" },
        /*  1*/ { 1, "\351", -1, 0, 1, { 0xE9 }, "In QR Kanji mode first-byte range but only one byte" },
        /*  2*/ { 0, "\351\141", -1, 0, 2, { 0xE9, 0x61 }, "Not full multibyte" },
        /*  3*/ { 1, "\351\141", -1, 0, 1, { 0xE961 }, "In QR Kanji mode range" },
        /*  4*/ { 1, "\201", -1, 0, 1, { 0x81 }, "In QR Kanji mode first-byte range but only one byte" },
        /*  5*/ { 0, "\201\141", -1, 0, 2, { 0x81, 0x61 }, "Not full multibyte" },
        /*  6*/ { 1, "\201\141", -1, 0, 1, { 0x8161 }, "In QR Kanji mode range" },
        /*  7*/ { 0, "\201\077\201\100\237\374\237\375\340\077\340\100\353\277\353\300", -1, 0, 16, { 0x81, 0x3F, 0x81, 0x40, 0x9F, 0xFC, 0x9F, 0xFD, 0xE0, 0x3F, 0xE0, 0x40, 0xEB, 0xBF, 0xEB, 0xC0 }, "" },
        /*  8*/ { 1, "\201\077\201\100\237\374\237\375\340\077\340\100\353\277\353\300", -1, 0, 12, { 0x81, 0x3F, 0x8140, 0x9FFC, 0x9F, 0xFD, 0xE0, 0x3F, 0xE040, 0xEBBF, 0xEB, 0xC0 }, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length;

    unsigned int jisdata[20];

    testStart("test_sjis_cpy");

    for (i = 0; i < data_size; i++) {
        int ret_length;
        int j;

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        sjis_cpy((unsigned char *) data[i].data, &ret_length, jisdata, data[i].full_multibyte);
        assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
        for (j = 0; j < ret_length; j++) {
            assert_equal(jisdata[j], data[i].expected_jisdata[j], "i:%d jisdata[%d] %04X != %04X\n", i, j, jisdata[j], data[i].expected_jisdata[j]);
        }
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_sjis_wctomb_zint", test_sjis_wctomb_zint, 0, 0, 0 },
        { "test_sjis_utf8", test_sjis_utf8, 1, 0, 0 },
        { "test_sjis_utf8_to_eci", test_sjis_utf8_to_eci, 1, 0, 0 },
        { "test_sjis_cpy", test_sjis_cpy, 1, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
