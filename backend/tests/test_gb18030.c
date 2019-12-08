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
#include "test_gb18030_tab.h"
#include "../gb18030.h"

// As control convert to GB 18030 using table generated from GB18030.TXT plus simple processing.
// The version of GB18030.TXT is libiconv-1.11/GB18030.TXT taken from https://haible.de/bruno/charsets/conversion-tables/GB18030.html
// The generated file backend/tests/test_gb18030_tab.h does not include U+10000..10FFFF codepoints to save space.
// See also backend/tests/tools/data/GB18030.TXT.README and backend/tests/tools/gen_test_tab.php.
static int gb18030_wctomb_zint2(unsigned int* r1, unsigned int* r2, unsigned int wc)
{
    unsigned int c;
    // GB18030 two-byte extension (libiconv-1.16/lib/gb18030ext.h)
    if (wc == 0x1E3F) { // GB 18030-2005 change, was PUA U+E7C7 below, see Table 3-39, p.111, Lunde 2nd ed.
        *r1 = 0xA8BC;
        return 2;
    }
    // GB18030 four-byte extension (libiconv-1.16/lib/gb18030uni.h)
    if (wc == 0xE7C7) { // PUA
        *r1 = 0x8135;
        *r2 = 0xF437;
        return 4;
    }
    // GB18030 two-byte extension (libiconv-1.16/lib/gb18030ext.h)
    if (wc >= 0x9FB4 && wc <= 0x9FBB) { // GB 18030-2005 change, were PUA, see Table 3-37, p.108, Lunde 2nd ed.
        if (wc == 0x9FB4) {
            *r1 = 0xFE59;
        } else if (wc == 0x9FB5) {
            *r1 = 0xFE61;
        } else if (wc == 0x9FB6 || wc == 0x9FB7) {
            *r1 = 0xFE66 + (wc - 0x9FB6);
        } else if (wc == 0x9FB8) {
            *r1 = 0xFE6D;
        } else if (wc == 0x9FB9) {
            *r1 = 0xFE7E;
        } else if (wc == 0x9FBA) {
            *r1 = 0xFE90;
        } else {
            *r1 = 0xFEA0;
        }
        return 2;
    }
    // GB18030 two-byte extension (libiconv-1.16/lib/gb18030ext.h)
    if (wc >= 0xFE10 && wc <= 0xFE19) { // GB 18030-2005 change, were PUA, see Table 3-37, p.108, Lunde 2nd ed.
        if (wc == 0xFE10) {
            *r1 = 0xA6D9;
        } else if (wc == 0xFE11) {
            *r1 = 0xA6DB;
        } else if (wc == 0xFE12) {
            *r1 = 0xA6DA;
        } else if (wc >= 0xFE13 && wc <= 0xFE16) {
            *r1 = 0xA6DC + (wc - 0xFE13);
        } else if (wc == 0xFE17 || wc == 0xFE18) {
            *r1 = 0xA6EC + (wc - 0xFE17);
        } else {
            *r1 = 0xA6F3;
        }
        return 2;
    }
    // GB18030 four-byte extension (libiconv-1.16/lib/gb18030uni.h)
    if (wc >= 0xFE1A && wc <= 0xFE2F) { // These are Vertical Forms (U+FE1A..FE1F unassigned) and Combining Half Marks (U+FE20..FE2F)
        if (wc >= 0xFE1A && wc <= 0xFE1D) {
            c = 0x84318336 + (wc - 0xFE1A);
        } else if (wc >= 0xFE1E && wc <= 0xFE27) {
            c = 0x84318430 + (wc - 0xFE1E);
        } else {
            c = 0x84318530 + (wc - 0xFE28);
        }
        *r1 = c >> 16;
        *r2 = c & 0xFFFF;
        return 4;
    }
    // GB18030 (libiconv-1.16/lib/gb18030.h)
    if (wc >= 0x10000 && wc < 0x10400) { // Code set 3 (Unicode U+10000..U+10FFFF)
        c = wc - 0x10000;
        *r1 = 0x9030;
        *r2 = 0x8130 + (c % 10) + 0x100 * (c / 10);
        return 4;
    }
    int tab_length = sizeof(test_gb18030_tab) / sizeof(unsigned int);
    for (int i = test_gb18030_tab_ind[wc >> 12]; i < tab_length; i += 2) {
        if (test_gb18030_tab[i + 1] == wc) {
            c = test_gb18030_tab[i];
            if (c <= 0xFFFF) {
                *r1 = c;
                return c <= 0xFF ? 1 : 2;
            }
            *r1 = c >> 16;
            *r2 = c & 0xFFFF;
            return 4;
        }
    }
    return 0;
}

static void test_gb18030_wctomb_zint(void)
{
    testStart("");

    int ret, ret2;
    unsigned int val1_1, val1_2, val2_1, val2_2;

    for (unsigned int i = 0; i < 0x10400; i++) { // Don't bother with U+10400..U+10FFFF, programmatically filled
        if (i >= 0xD800 && i <= 0xDFFF) { // UTF-16 surrogates
            continue;
        }
        val1_1 = val1_2 = val2_1 = val2_2 = 0;
        ret = gb18030_wctomb_zint(&val1_1, &val1_2, i);
        ret2 = gb18030_wctomb_zint2(&val2_1, &val2_2, i);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val1_1 0x%04X, val2_1 0x%04X, val1_2 0x%04X, val2_2 0x%04X\n", i, i, ret, ret2, val1_1, val2_1, val1_2, val2_2);
        if (ret2) {
            assert_equal(val1_1, val2_1, "i:%d 0x%04X val1_1 0x%04X != val2_1 0x%04X\n", i, i, val1_1, val2_1);
            assert_equal(val1_2, val2_2, "i:%d 0x%04X val1_2 0x%04X != val2_2 0x%04X\n", i, i, val1_2, val2_2);
        }
    }

    testFinish();
}

static void test_gb18030_utf8tomb(void)
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
    // é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 18030 0xA8A6, UTF-8 C3A9
    // β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in GB 18030 0xA6C2, UTF-8 CEB2
    // ¤ U+00A4 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 18030 0xA1E8, UTF-8 C2A4
    // ¥ U+00A5 in ISO 8859-1 0xA5, in GB 18030 4-byte 0x81308436, UTF-8 C2A5
    // ・ U+30FB katakana middle dot, not in any ISO or Win page, in GB 18030 0xA1A4, duplicate of mapping of U+00B7, UTF-8 E383BB
    // · U+00B7 middle dot in ISO 8859-1 0xB7, in GB 18030 "GB 18030 subset" 0xA1A4, duplicate of mapping of U+30FB, UTF-8 C2B7
    // ― U+2015 horizontal bar in ISO 8859-7 Greek and ISO 8859-10 Nordic, not in any Win page, in GB 18030 "GB18030.TXT" 0xA1AA, duplicate of mapping of U+2014, UTF-8 E28095
    // — U+2014 em dash, not in any ISO, in Win 1250 and other Win, in GB 18030 "GB 18030 subset" 0xA1AA, duplicate of mapping of U+2015, UTF-8 E28094
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "é", -1, 0, 1, { 0xA8A6 }, "" },
        /*  1*/ { "β", -1, 0, 1, { 0xA6C2 }, "" },
        /*  2*/ { "¤", -1, 0, 1, { 0xA1E8 }, "" },
        /*  3*/ { "¥", -1, 0, 2, { 0x8130, 0x8436 }, "0x81308436" },
        /*  4*/ { "・", -1, 0, 2, { 0x8139, 0xA739 }, "" },
        /*  5*/ { "·", -1, 0, 1, { 0xA1A4 }, "GB 18030 subset mapping" },
        /*  6*/ { "―", -1, 0, 1, { 0xA844 }, "GB18030.TXT mapping" },
        /*  7*/ { "—", -1, 0, 1, { 0xA1AA }, "GB 18030 subset mapping" },
        /*  8*/ { "aβc・·—é—Z", -1, 0, 10, { 'a', 0xA6C2, 'c', 0x8139, 0xA739, 0xA1A4, 0xA1AA, 0xA8A6, 0xA1AA, 'Z' }, "" },
    };

    int data_size = sizeof(data) / sizeof(struct item);

    struct zint_symbol symbol;
    unsigned int gbdata[20];

    for (int i = 0; i < data_size; i++) {

        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;
        size_t ret_length = length;

        ret = gb18030_utf8tomb(&symbol, data[i].data, &ret_length, gbdata);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
        if (ret == 0) {
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %zu != %zu\n", i, ret_length, data[i].ret_length);
            for (int j = 0; j < ret_length; j++) {
                assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] 0x%04X != 0x%04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
            }
        }
    }

    testFinish();
}

static void test_gb18030_utf8tosb(void)
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
    // é U+00E9 in ISO 8859-1 0xE9, Win 1250 plus other Win, in HANXIN Chinese mode first byte range 0x81..FE
    // β U+03B2 in ISO 8859-7 Greek 0xE2 (but not other ISO 8859 or Win page)
    // ¥ U+00A5 in ISO 8859-1 0xA5, in first byte range 0x81..FE
    // ÿ U+00FF in ISO 8859-1 0xFF, outside first byte and second/third/fourth byte ranges
    // @ U+0040 in ASCII 0x40, outside first byte range, in double-byte second byte range, outside quad-byte second/third/fourth byte ranges
    // 9 U+0039 in ASCII 0x39, outside first byte range, outside double-byte second byte range and quad-byte third byte range, in quad-byte second/fourth byte ranges
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 3, "é", -1, 0, 1, { 0xE9 }, "First byte in range but only one byte" },
        /*  1*/ { 3, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {}, "Not in ECI 3 (ISO 8859-1)" },
        /*  2*/ { 9, "β", -1, 0, 1, { 0xE2 }, "In ECI 9 (ISO 8859-7)" },
        /*  3*/ { 3, "¥", -1, 0, 1, { 0xA5 }, "First byte in range but only one byte" },
        /*  4*/ { 3, "¥é", -1, 0, 1, { 0xA5E9 }, "In double-byte range" },
        /*  5*/ { 3, "¥ÿ", -1, 0, 2, { 0xA5, 0xFF }, "First byte in range but not second" },
        /*  6*/ { 3, "¥9é9", -1, 0, 2, { 0xA539, 0xE939 }, "In quad-byte range" },
        /*  7*/ { 3, "¥9", -1, 0, 2, { 0xA5, 0x39 }, "In quad-byte first/second range but only 2 bytes, not in double-byte range" },
        /*  8*/ { 3, "¥9é", -1, 0, 3, { 0xA5, 0x39, 0xE9 }, "In quad-byte first/second/third range but only 3 bytes, no bytes in double-byte range" },
        /*  9*/ { 3, "¥9é@", -1, 0, 3, { 0xA5, 0x39, 0xE940 }, "In quad-byte first/second/third range but not fourth, second 2 bytes in double-byte range" },
        /* 10*/ { 3, "¥@é9", -1, 0, 3, { 0xA540, 0xE9, 0x39 }, "In quad-byte first/third/fourth range but not second, first 2 bytes in double-byte range" },
        /* 11*/ { 3, "¥9@9", -1, 0, 4, { 0xA5, 0x39, 0x40, 0x39 }, "In quad-byte first/second/fourth range but not third, no bytes in double-byte range" },
        /* 12*/ { 3, "é9éé¥9é@¥9é9¥9é0é@@¥¥é0é1", -1, 0, 15, { 0xE9, 0x39, 0xE9E9, 0xA5, 0x39, 0xE940, 0xA539, 0xE939, 0xA539, 0xE930, 0xE940, 0x40, 0xA5A5, 0xE930, 0xE931 }, "" },
    };

    int data_size = sizeof(data) / sizeof(struct item);

    unsigned int gbdata[20];

    for (int i = 0; i < data_size; i++) {

        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;
        size_t ret_length = length;

        ret = gb18030_utf8tosb(data[i].eci, data[i].data, &ret_length, gbdata);
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

static void test_gb18030_cpy(void)
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
        /*  0*/ { "\351", -1, 0, 1, { 0xE9 }, "In HANXIN Chinese mode first-byte range but only one byte" },
        /*  1*/ { "\351\241", -1, 0, 1, { 0xE9A1 }, "In HANXIN Chinese range" },
        /*  2*/ { "\241", -1, 0, 1, { 0xA1 }, "In first-byte range but only one byte" },
        /*  3*/ { "\241\241", -1, 0, 1, { 0xA1A1 }, "In range" },
        /*  4*/ { "\241\240\241\376\367\376\367\377\2012\2013", -1, 0, 7, { 0xA1A0, 0xA1FE, 0xF7FE, 0xF7, 0xFF, 0x8132, 0x8133 }, "" },
    };

    int data_size = sizeof(data) / sizeof(struct item);

    unsigned int jisdata[40];

    for (int i = 0; i < data_size; i++) {

        int length = data[i].length == -1 ? strlen(data[i].data) : data[i].length;
        size_t ret_length = length;

        gb18030_cpy(data[i].data, &ret_length, jisdata);
        assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %zu != %zu\n", i, ret_length, data[i].ret_length);
        for (int j = 0; j < ret_length; j++) {
            assert_equal(jisdata[j], data[i].expected_jisdata[j], "i:%d jisdata[%d] %04X != %04X\n", i, j, jisdata[j], data[i].expected_jisdata[j]);
        }
    }

    testFinish();
}

int main()
{
    test_gb18030_wctomb_zint();
    test_gb18030_utf8tomb();
    test_gb18030_utf8tosb();
    test_gb18030_cpy();

    testReport();

    return 0;
}
