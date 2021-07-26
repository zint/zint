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
#include "test_gb2312_tab.h"
#include "../gb2312.h"

// As control convert to GB 2312 using simple table generated from unicode.org GB2312.TXT plus simple processing
// GB2312.TXT no longer on unicode.org site but available from https://haible.de/bruno/charsets/conversion-tables/GB2312.html
static int gb2312_wctomb_zint2(unsigned int *r, unsigned int wc) {
    int tab_length, start_i, end_i;
    int i;

    // Shortcut
    if ((wc > 0x0451 && wc < 0x2015) || (wc > 0x3229 && wc < 0x4E00) || (wc > 0x9FA0 && wc < 0xFF01) || wc > 0xFFE5) {
        return 0;
    }
    tab_length = ARRAY_SIZE(test_gb2312_tab);
    start_i = test_gb2312_tab_ind[wc >> 10];
    end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    for (i = start_i; i < end_i; i += 2) {
        if (test_gb2312_tab[i + 1] == wc) {
            *r = test_gb2312_tab[i] + 0x8080; // Table in GB 2312 not EUC-CN
            return 2;
        }
    }
    return 0;
}

static void test_gb2312_wctomb_zint(void) {

    int ret, ret2;
    unsigned int val, val2;
    unsigned int i;

    testStart("test_gb2312_wctomb_zint");

    for (i = 0; i < 0xFFFE; i++) {
        if (i < 0x80) { // ASCII is straight through and not dealt with by gb2312_wctomb_zint()
            continue;
        }
        if (i >= 0xD800 && i <= 0xDFFF) { // UTF-16 surrogates
            continue;
        }
        val = val2 = 0;
        ret = gb2312_wctomb_zint(&val, i);
        ret2 = gb2312_wctomb_zint2(&val2, i);
        if (i == 0xB7) { // Extra mapping middle dot U+00B7 to 0xA1A4, duplicate of U+30FB (Katakana middle dot)
            assert_equal(ret, 2, "i:%d 0x%04X ret %d != 2, val 0x%04X\n", (int) i, i, ret, val);
            assert_equal(val, 0xA1A4, "i:%d 0x%04X val 0x%04X != 0xA1A4\n", (int) i, i, val);
            assert_zero(ret2, "i:%d 0x%04X ret2 %d != 0, val2 0x%04X\n", (int) i, i, ret2, val2);
        } else if (i == 0x2014) { // Extra mapping em dash U+2014 to 0xA1AA, duplicate of U+2015 (horizontal bar)
            assert_equal(ret, 2, "i:%d 0x%04X ret %d != 2, val 0x%04X\n", (int) i, i, ret, val);
            assert_equal(val, 0xA1AA, "i:%d 0x%04X val 0x%04X != 0xA1AA\n", (int) i, i, val);
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

static void test_gb2312_utf8(int index) {

    struct item {
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[20];
        char *comment;
    };
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 2312 0xA8A6, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in GB 2312 0xA6C2, UTF-8 CEB2
    // ¤ U+00A4 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 2312 0xA1E8, UTF-8 C2A4
    // ¥ U+00A5 in ISO 8859-1 0xA5, not in GB 2312, UTF-8 C2A5
    // ・ U+30FB katakana middle dot, not in any ISO or Win page, in GB 2312 "GB2312.TXT" 0xA1A4, duplicate of mapping of U+00B7, UTF-8 E383BB
    // · U+00B7 middle dot in ISO 8859-1 0xB7, in GB 2312 "GB 18030 subset" 0xA1A4, duplicate of mapping of U+30FB, UTF-8 C2B7
    // ― U+2015 horizontal bar in ISO 8859-7 Greek and ISO 8859-10 Nordic, not in any Win page, in GB 2312 "GB2312.TXT" 0xA1AA, duplicate of mapping of U+2014, UTF-8 E28095
    // — U+2014 em dash, not in any ISO, in Win 1250 and other Win, in GB 2312 "GB 18030 subset" 0xA1AA, duplicate of mapping of U+2015, UTF-8 E28094
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "é", -1, 0, 1, { 0xA8A6 }, "" },
        /*  1*/ { "β", -1, 0, 1, { 0xA6C2 }, "" },
        /*  2*/ { "¤", -1, 0, 1, { 0xA1E8 }, "" },
        /*  3*/ { "¥", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "" },
        /*  4*/ { "・", -1, 0, 1, { 0xA1A4 }, "GB2312.TXT mapping" },
        /*  5*/ { "·", -1, 0, 1, { 0xA1A4 }, "GB 18030 subset mapping" },
        /*  6*/ { "―", -1, 0, 1, { 0xA1AA }, "GB2312.TXT mapping" },
        /*  7*/ { "—", -1, 0, 1, { 0xA1AA }, "GB 18030 subset mapping" },
        /*  8*/ { "aβc・·—é—Z", -1, 0, 9, { 'a', 0xA6C2, 'c', 0xA1A4, 0xA1A4, 0xA1AA, 0xA8A6, 0xA1AA, 'Z' }, "" },
        /*  9*/ { "\200", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Invalid UTF-8" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};
    unsigned int gbdata[20];

    testStart("test_gb2312_utf8");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = gb2312_utf8(&symbol, (unsigned char *) data[i].data, &ret_length, gbdata);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < (int) ret_length; j++) {
                assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] %04X != %04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
            }
        }
    }

    testFinish();
}

static void test_gb2312_utf8_to_eci(int index) {

    struct item {
        int eci;
        int full_multibyte;
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[20];
        char *comment;
    };
    // é U+00E9 in ISO 8859-1 0xE9, Win 1250 plus other Win, in GRIDMATRIX Chinese mode first byte range 0xA1..A9, 0xB0..F7
    // β U+03B2 in ISO 8859-7 Greek 0xE2 (but not other ISO 8859 or Win page)
    // ¥ U+00A5 in ISO 8859-1 0xA5, in first byte range 0xA1..A9, 0xB0..F7
    // NBSP U+00A0 in ISO 8859-1 0xA0, outside first byte and second byte range 0xA1..FE, UTF-8 C2A0 (\302\240)
    // ¡ U+00A1 in ISO 8859-1 0xA1, in first byte range
    // © U+00A9 in ISO 8859-1 0xA9, in first byte range
    // ª U+00AA in ISO 8859-1 0xAA, outside first byte range
    // ¯ U+00AF in ISO 8859-1 0xAF, outside first byte range
    // ° U+00B0 in ISO 8859-1 0xB0, in first byte range
    // ÷ U+00F7 in ISO 8859-1 0xF7, in first byte range
    // ø U+00F8 in ISO 8859-1 0xF8, outside first byte range
    // ÿ U+00FF in ISO 8859-1 0xFF, outside first byte and second byte range
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 3, 0, "é", -1, 0, 1, { 0xE9 }, "Not full multibyte" },
        /*  1*/ { 3, 1, "é", -1, 0, 1, { 0xE9 }, "First byte in range but only one byte" },
        /*  2*/ { 3, 0, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Not in ECI 3 (ISO 8859-1)" },
        /*  3*/ { 3, 1, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Not in ECI 3 (ISO 8859-1)" },
        /*  4*/ { 9, 0, "β", -1, 0, 1, { 0xE2 }, "In ECI 9 (ISO 8859-7)" },
        /*  5*/ { 9, 1, "β", -1, 0, 1, { 0xE2 }, "In ECI 9 (ISO 8859-7)" },
        /*  6*/ { 3, 0, "¥", -1, 0, 1, { 0xA5 }, "Not full multibyte" },
        /*  7*/ { 3, 1, "¥", -1, 0, 1, { 0xA5 }, "First byte in range but only one byte" },
        /*  8*/ { 3, 0, "¡é", -1, 0, 2, { 0xA1, 0xE9 }, "Not full multibyte" },
        /*  9*/ { 3, 1, "¡é", -1, 0, 1, { 0xA1E9 }, "In GRIDMATRIX Chinese mode range" },
        /* 10*/ { 3, 0, "¡\302\240", -1, 0, 2, { 0xA1, 0xA0 }, "Not full multibyte" },
        /* 11*/ { 3, 1, "¡\302\240", -1, 0, 2, { 0xA1, 0xA0 }, "First byte in range but not second" },
        /* 12*/ { 3, 0, "©é", -1, 0, 2, { 0xA9, 0xE9 }, "Not full multibyte" },
        /* 13*/ { 3, 1, "©é", -1, 0, 1, { 0xA9E9 }, "In GRIDMATRIX Chinese mode range" },
        /* 14*/ { 3, 0, "©ÿ", -1, 0, 2, { 0xA9, 0xFF }, "Not full multibyte" },
        /* 15*/ { 3, 1, "©ÿ", -1, 0, 2, { 0xA9, 0xFF }, "First byte in range but not second" },
        /* 16*/ { 3, 0, "éaéé©ªª©¯é°°é÷éø", -1, 0, 16, { 0xE9, 0x61, 0xE9, 0xE9, 0xA9, 0xAA, 0xAA, 0xA9, 0xAF, 0xE9, 0xB0, 0xB0, 0xE9, 0xF7, 0xE9, 0xF8 }, "" },
        /* 17*/ { 3, 1, "éaéé©ªª©¯é°°é÷éø", -1, 0, 10, { 0xE9, 0x61, 0xE9E9, 0xA9AA, 0xAA, 0xA9AF, 0xE9B0, 0xB0E9, 0xF7E9, 0xF8 }, "" },
        /* 18*/ { 20, 0, "\\\\", -1, 0, 4, { 0x81, 0x5F, 0x81, 0x5F }, "Shift JIS reverse solidus (backslash) mapping from ASCII to double byte" },
        /* 19*/ { 20, 1, "\\\\", -1, 0, 4, { 0x81, 0x5F, 0x81, 0x5F }, "Shift JIS outside GB 2312 Hanzi mode range" },
        /* 20*/ { 20, 0, "爍", -1, 0, 2, { 0xE0, 0xA1 }, "Shift JIS U+720D" },
        /* 21*/ { 20, 1, "爍", -1, 0, 1, { 0xE0A1 }, "Shift JIS in GB 2312 Hanzi mode range" },
        /* 22*/ { 25, 0, "12", -1, 0, 4, { 0x00, 0x31, 0x00, 0x32 }, "UCS-2BE ASCII" },
        /* 23*/ { 25, 0, "", -1, 0, 4, { 0x00, 0x81, 0x00, 0x81 }, "UCS-2BE U+0081" },
        /* 24*/ { 25, 1, "", -1, 0, 4, { 0x00, 0x81, 0x00, 0x81 }, "UCS-2BE outside GB 2312 Hanzi mode range" },
        /* 25*/ { 25, 0, "ꆩꆩ", -1, 0, 4, { 0xA1, 0xA9, 0xA1, 0xA9 }, "UCS-2BE U+A1A9" },
        /* 26*/ { 25, 1, "ꆩꆩ", -1, 0, 2, { 0xA1A9, 0xA1A9 }, "UCS-2BE in GB 2312 Hanzi mode range" },
        /* 27*/ { 25, 0, "膀膀", -1, 0, 4, { 0x81, 0x80, 0x81, 0x80 }, "UCS-2BE U+8180" },
        /* 28*/ { 25, 1, "膀膀", -1, 0, 4, { 0x81, 0x80, 0x81, 0x80 }, "UCS-2BE outside GB 2312 Hanzi mode range (but in GB 18030 range)" },
        /* 29*/ { 28, 0, "¢¢", -1, 0, 4, { 0xA2, 0x46, 0xA2, 0x46 }, "Big5 U+00A2" },
        /* 30*/ { 28, 1, "¢¢", -1, 0, 4, { 0xA2, 0x46, 0xA2, 0x46 }, "Big5 outside GB 2312 Hanzi mode range (but in GB 18030 range)" },
        /* 31*/ { 28, 0, "陛", -1, 0, 2, { 0xB0, 0xA1 }, "Big5 U+965B" },
        /* 32*/ { 28, 1, "陛", -1, 0, 1, { 0xB0A1 }, "Big5 in GB 2312 Hanzi mode range" },
        /* 33*/ { 29, 0, "¨¨", -1, 0, 4, { 0xA1, 0xA7, 0xA1, 0xA7 }, "GB 2312 U+00A8" },
        /* 34*/ { 29, 1, "¨¨", -1, 0, 2, { 0xA1A7, 0xA1A7 }, "GB 2312" },
        /* 35*/ { 29, 0, "崂", -1, 0, 2, { 0xE1, 0xC0 }, "GB 2312 U+5D02" },
        /* 36*/ { 29, 1, "崂", -1, 0, 1, { 0xE1C0 }, "GB 2312" },
        /* 37*/ { 29, 0, "・", -1, 0, 2, { 0xA1, 0xA4 }, "GB 2312 U+30FB" },
        /* 38*/ { 29, 1, "・", -1, 0, 1, { 0xA1A4 }, "GB 2312" },
        /* 39*/ { 29, 0, "釦", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "GB 18030 U+91E6 not in GB 2312" },
        /* 40*/ { 30, 0, "¡¡", -1, 0, 4, { 0x22 + 0x80, 0x2E + 0x80, 0x22 + 0x80, 0x2E + 0x80 }, "EUC-KR U+00A1 (0xA2AE)" },
        /* 41*/ { 30, 1, "¡¡", -1, 0, 2, { 0x222E + 0x8080, 0x222E + 0x8080 }, "EUC-KR 0xA2AE in GB 2312 Hanzi mode range" },
        /* 42*/ { 30, 0, "詰", -1, 0, 2, { 0x7D + 0x80, 0x7E + 0x80 }, "EUC-KR U+8A70 (0xFDFE)" },
        /* 43*/ { 30, 1, "詰", -1, 0, 2, { 0x7D + 0x80, 0x7E + 0x80 }, "EUC-KR 0xFDFE > 0xF7FE so not in GB 2312 Hanzi mode range" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    unsigned int gbdata[20];

    testStart("test_gb2312_utf8_to_eci");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = gb2312_utf8_to_eci(data[i].eci, (unsigned char *) data[i].data, &ret_length, gbdata, data[i].full_multibyte);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < (int) ret_length; j++) {
                assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] 0x%04X != 0x%04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
            }
        }
    }

    testFinish();
}

static void test_gb2312_cpy(int index) {

    struct item {
        int full_multibyte;
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[20];
        char *comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 0, "\351", -1, 0, 1, { 0xE9 }, "Not full multibyte" },
        /*  1*/ { 1, "\351", -1, 0, 1, { 0xE9 }, "In GRIDMATRIX Chinese mode first-byte range but only one byte" },
        /*  2*/ { 0, "\351\241", -1, 0, 2, { 0xE9, 0xA1 }, "Not full multibyte" },
        /*  3*/ { 1, "\351\241", -1, 0, 1, { 0xE9A1 }, "In GRIDMATRIX Chinese range" },
        /*  4*/ { 0, "\241", -1, 0, 1, { 0xA1 }, "Not full multibyte" },
        /*  5*/ { 1, "\241", -1, 0, 1, { 0xA1 }, "In first-byte range but only one byte" },
        /*  6*/ { 0, "\241\241", -1, 0, 2, { 0xA1, 0xA1 }, "Not full multibyte" },
        /*  7*/ { 1, "\241\241", -1, 0, 1, { 0xA1A1 }, "In range" },
        /*  8*/ { 0, "\241\240\241\376\367\376\367\377", -1, 0, 8, { 0xA1, 0xA0, 0xA1, 0xFE, 0xF7, 0xFE, 0xF7, 0xFF }, "" },
        /*  9*/ { 1, "\241\240\241\376\367\376\367\377", -1, 0, 6, { 0xA1, 0xA0, 0xA1FE, 0xF7FE, 0xF7, 0xFF }, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length;

    unsigned int gbdata[20];

    testStart("test_gb2312_cpy");

    for (i = 0; i < data_size; i++) {
        int ret_length;
        int j;

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        gb2312_cpy((unsigned char *) data[i].data, &ret_length, gbdata, data[i].full_multibyte);
        assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
        for (j = 0; j < (int) ret_length; j++) {
            assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] %04X != %04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
        }
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_gb2312_wctomb_zint", test_gb2312_wctomb_zint, 0, 0, 0 },
        { "test_gb2312_utf8", test_gb2312_utf8, 1, 0, 0 },
        { "test_gb2312_utf8_to_eci", test_gb2312_utf8_to_eci, 1, 0, 0 },
        { "test_gb2312_cpy", test_gb2312_cpy, 1, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
