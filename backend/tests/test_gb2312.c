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
#include "test_gb2312_tab.h"
#include "../gb2312.h"

// As control convert to GB 2312 using simple table generated from unicode.org GB2312.TXT plus simple processing
// GB2312.TXT no longer on unicode.org site but available from https://haible.de/bruno/charsets/conversion-tables/GB2312.html
static int gb2312_wctomb_zint2(unsigned int* r, unsigned int wc)
{
    // Shortcut
    if ((wc > 0x0451 && wc < 0x2015) || (wc > 0x3229 && wc < 0x4E00) || (wc > 0x9FA0 && wc < 0xFF01) || wc > 0xFFE5) {
        return 0;
    }
    int tab_length = sizeof(test_gb2312_tab) / sizeof(unsigned int);
    for (int i = test_gb2312_tab_ind[wc >> 12]; i < tab_length; i += 2) {
        if (test_gb2312_tab[i + 1] == wc) {
            *r = test_gb2312_tab[i] + 0x8080; // Table in GB 2312 not EUC-CN
            return 2;
        }
    }
    return 0;
}

static void test_gb2312_wctomb_zint(void)
{
    testStart("");

    int ret, ret2;
    unsigned char buf[2], buf2[2];
    unsigned int val, val2;

    for (unsigned int i = 0; i < 0xFFFE; i++) {
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
            assert_equal(ret, 2, "i:%d 0x%04X ret %d != 2, val 0x%04X\n", i, i, ret, val);
            assert_equal(val, 0xA1A4, "i:%d 0x%04X val 0x%04X != 0xA1A4\n", i, i, val);
            assert_zero(ret2, "i:%d 0x%04X ret2 %d != 0, val2 0x%04X\n", i, i, ret2, val2);
        } else if (i == 0x2014) { // Extra mapping em dash U+2014 to 0xA1AA, duplicate of U+2015 (horizontal bar)
            assert_equal(ret, 2, "i:%d 0x%04X ret %d != 2, val 0x%04X\n", i, i, ret, val);
            assert_equal(val, 0xA1AA, "i:%d 0x%04X val 0x%04X != 0xA1AA\n", i, i, val);
            assert_zero(ret2, "i:%d 0x%04X ret2 %d != 0, val2 0x%04X\n", i, i, ret2, val2);
        } else {
            assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", i, i, ret, ret2, val, val2);
        }
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", i, i, val, val2);
        }
    }

    testFinish();
}

static void test_gb2312_utf8tomb(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int length;
        int ret;
        size_t ret_length;
        unsigned int expected_gbdata[20];
        char* comment;
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
        /*  3*/ { "¥", -1, ZINT_ERROR_INVALID_DATA, -1, {}, "" },
        /*  4*/ { "・", -1, 0, 1, { 0xA1A4 }, "GB2312.TXT mapping" },
        /*  5*/ { "·", -1, 0, 1, { 0xA1A4 }, "GB 18030 subset mapping" },
        /*  6*/ { "―", -1, 0, 1, { 0xA1AA }, "GB2312.TXT mapping" },
        /*  7*/ { "—", -1, 0, 1, { 0xA1AA }, "GB 18030 subset mapping" },
        /*  8*/ { "aβc・·—é—Z", -1, 0, 9, { 'a', 0xA6C2, 'c', 0xA1A4, 0xA1A4, 0xA1AA, 0xA8A6, 0xA1AA, 'Z' }, "" },
    };

    int data_size = sizeof(data) / sizeof(struct item);

    struct zint_symbol symbol;
    unsigned int gbdata[20];

    for (int i = 0; i < data_size; i++) {

        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;
        size_t ret_length = length;

        ret = gb2312_utf8tomb(&symbol, data[i].data, &ret_length, gbdata);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
        if (ret == 0) {
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %zu != %zu\n", i, ret_length, data[i].ret_length);
            for (int j = 0; j < ret_length; j++) {
                assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] %04X != %04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
            }
        }
    }

    testFinish();
}

static void test_gb2312_utf8tosb(void)
{
    testStart("");

    int ret;
    struct item {
        int eci;
        unsigned char* data;
        int length;
        int ret;
        size_t ret_length;
        unsigned int expected_gbdata[20];
        char* comment;
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
        /*  0*/ { 3, "é", -1, 0, 1, { 0xE9 }, "First byte in range but only one byte" },
        /*  1*/ { 3, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {}, "Not in ECI 3 (ISO 8859-1)" },
        /*  2*/ { 9, "β", -1, 0, 1, { 0xE2 }, "In ECI 9 (ISO 8859-7)" },
        /*  3*/ { 3, "¥", -1, 0, 1, { 0xA5 }, "First byte in range but only one byte" },
        /*  4*/ { 3, "¡é", -1, 0, 1, { 0xA1E9 }, "In GRIDMATRIX Chinese mode range" },
        /*  5*/ { 3, "¡\302\240", -1, 0, 2, { 0xA1, 0xA0 }, "First byte in range but not second" },
        /*  6*/ { 3, "©é", -1, 0, 1, { 0xA9E9 }, "In GRIDMATRIX Chinese mode range" },
        /*  7*/ { 3, "©ÿ", -1, 0, 2, { 0xA9, 0xFF }, "First byte in range but not second" },
        /*  8*/ { 3, "éaéé©ªª©¯é°°é÷éø", -1, 0, 10, { 0xE9, 0x61, 0xE9E9, 0xA9AA, 0xAA, 0xA9AF, 0xE9B0, 0xB0E9, 0xF7E9, 0xF8 }, "" },
    };

    int data_size = sizeof(data) / sizeof(struct item);

    unsigned int gbdata[20];

    for (int i = 0; i < data_size; i++) {

        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;
        size_t ret_length = length;

        ret = gb2312_utf8tosb(data[i].eci, data[i].data, &ret_length, gbdata);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %zu != %zu\n", i, ret_length, data[i].ret_length);
            for (int j = 0; j < ret_length; j++) {
                assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] %04X != %04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
            }
        }
    }

    testFinish();
}

static void test_gb2312_cpy(void)
{
    testStart("");

    int ret;
    struct item {
        unsigned char* data;
        int length;
        int ret;
        size_t ret_length;
        unsigned int expected_jisdata[20];
        char* comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "\351", -1, 0, 1, { 0xE9 }, "In GRIDMATRIX Chinese mode first-byte range but only one byte" },
        /*  1*/ { "\351\241", -1, 0, 1, { 0xE9A1 }, "In GRIDMATRIX Chinese range" },
        /*  2*/ { "\241", -1, 0, 1, { 0xA1 }, "In first-byte range but only one byte" },
        /*  3*/ { "\241\241", -1, 0, 1, { 0xA1A1 }, "In range" },
        /*  4*/ { "\241\240\241\376\367\376\367\377", -1, 0, 6, { 0xA1, 0xA0, 0xA1FE, 0xF7FE, 0xF7, 0xFF }, "" },
    };

    int data_size = sizeof(data) / sizeof(struct item);

    unsigned int jisdata[20];

    for (int i = 0; i < data_size; i++) {

        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;
        size_t ret_length = length;

        gb2312_cpy(data[i].data, &ret_length, jisdata);
        assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %zu != %zu\n", i, ret_length, data[i].ret_length);
        for (int j = 0; j < ret_length; j++) {
            assert_equal(jisdata[j], data[i].expected_jisdata[j], "i:%d jisdata[%d] %04X != %04X\n", i, j, jisdata[j], data[i].expected_jisdata[j]);
        }
    }

    testFinish();
}

int main()
{
    test_gb2312_wctomb_zint();
    test_gb2312_utf8tomb();
    test_gb2312_utf8tosb();
    test_gb2312_cpy();

    testReport();

    return 0;
}
