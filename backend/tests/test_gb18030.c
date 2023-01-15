/*
    libzint - the open source barcode library
    Copyright (C) 2019-2023 Robin Stuart <rstuart114@gmail.com>

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
#include "test_gb18030_tab.h"
#include "test_gbk_tab.h"
#include "../eci.h"
/* For local "private" testing using previous libiconv adaptation, not included for licensing reasons */
#if 0
#define TEST_JUST_SAY_GNO */
#endif
#ifdef TEST_JUST_SAY_GNO
#include "../just_say_gno/gb18030_gnu.c"
#include "../just_say_gno/gb2312_gnu.c"
#endif

INTERNAL int u_gb18030_int_test(const unsigned int u, unsigned int *dest1, unsigned int *dest2);

/* As control convert to GB 18030 using table generated from GB18030.TXT plus simple processing.
   The version of GB18030.TXT is jdk-1.4.2/GB18030.TXT taken from
   https://haible.de/bruno/charsets/conversion-tables/GB18030.html
   The generated file backend/tests/test_gb18030_tab.h does not include U+10000..10FFFF codepoints to save space.
   See also backend/tests/tools/data/GB18030.TXT.README and backend/tests/tools/gen_test_tab.php.
*/
static int u_gb18030_int2(unsigned int u, unsigned int *dest1, unsigned int *dest2) {
    unsigned int c;
    int tab_length, start_i, end_i;
    int i;

    /* GB18030 two-byte extension */
    if (u == 0x1E3F) { /* GB 18030-2005 change, was PUA U+E7C7 below, see Table 3-39, p.111, Lunde 2nd ed. */
        *dest1 = 0xA8BC;
        return 2;
    }
    /* GB18030 four-byte extension */
    if (u == 0xE7C7) { /* PUA */
        *dest1 = 0x8135;
        *dest2 = 0xF437;
        return 4;
    }
    /* GB18030 two-byte extension */
    if (u >= 0x9FB4 && u <= 0x9FBB) { /* GB 18030-2005 change, were PUA, see Table 3-37, p.108, Lunde 2nd ed. */
        if (u == 0x9FB4) {
            *dest1 = 0xFE59;
        } else if (u == 0x9FB5) {
            *dest1 = 0xFE61;
        } else if (u == 0x9FB6 || u == 0x9FB7) {
            *dest1 = 0xFE66 + (u - 0x9FB6);
        } else if (u == 0x9FB8) {
            *dest1 = 0xFE6D;
        } else if (u == 0x9FB9) {
            *dest1 = 0xFE7E;
        } else if (u == 0x9FBA) {
            *dest1 = 0xFE90;
        } else {
            *dest1 = 0xFEA0;
        }
        return 2;
    }
    /* GB18030 two-byte extension */
    if (u >= 0xFE10 && u <= 0xFE19) { /* GB 18030-2005 change, were PUA, see Table 3-37, p.108, Lunde 2nd ed. */
        if (u == 0xFE10) {
            *dest1 = 0xA6D9;
        } else if (u == 0xFE11) {
            *dest1 = 0xA6DB;
        } else if (u == 0xFE12) {
            *dest1 = 0xA6DA;
        } else if (u >= 0xFE13 && u <= 0xFE16) {
            *dest1 = 0xA6DC + (u - 0xFE13);
        } else if (u == 0xFE17 || u == 0xFE18) {
            *dest1 = 0xA6EC + (u - 0xFE17);
        } else {
            *dest1 = 0xA6F3;
        }
        return 2;
    }
    /* GB18030 four-byte extension */
    if (u >= 0xFE1A && u <= 0xFE2F) { /* These are Vertical Forms (U+FE1A..FE1F unassigned) and Combining Half Marks (U+FE20..FE2F) */
        if (u >= 0xFE1A && u <= 0xFE1D) {
            c = 0x84318336 + (u - 0xFE1A);
        } else if (u >= 0xFE1E && u <= 0xFE27) {
            c = 0x84318430 + (u - 0xFE1E);
        } else {
            c = 0x84318530 + (u - 0xFE28);
        }
        *dest1 = c >> 16;
        *dest2 = c & 0xFFFF;
        return 4;
    }
    /* GB18030 */
    /* Code set 3 (Unicode U+10000..U+10FFFF) */
    if (u >= 0x10000 /*&& u < 0x10400*/) { /* Not being called for U+10400..U+10FFFF */
        c = u - 0x10000;
        *dest1 = 0x9030;
        *dest2 = 0x8130 + (c % 10) + 0x100 * (c / 10);
        return 4;
    }

    tab_length = ARRAY_SIZE(test_gb18030_tab);
    start_i = test_gb18030_tab_ind[u >> 10];
    end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    for (i = start_i; i < end_i; i += 2) {
        if (test_gb18030_tab[i + 1] == u) {
            c = test_gb18030_tab[i];
            if (c <= 0xFFFF) {
                *dest1 = c;
                return c <= 0xFF ? 1 : 2;
            }
            *dest1 = c >> 16;
            *dest2 = c & 0xFFFF;
            return 4;
        }
    }
    return 0;
}

#include <time.h>

#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)
#define TEST_PERF_RATIO(a1, a2) (a2 ? TEST_PERF_TIME(a1) / TEST_PERF_TIME(a2) : 0)

#ifdef TEST_JUST_SAY_GNO
#define TEST_INT_PERF_ITERATIONS    250
#endif

static void test_u_gb18030_int(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    int ret, ret2;
    unsigned int val1_1, val1_2, val2_1, val2_2;
    unsigned int i;
    /* See: https://file.allitebooks.com/20160708/CJKV%20Information%20Processing.pdf (table 3-37, p.109, 2nd ed.) */
    static const int nonpua_nonbmp[] = {
        0x20087, 0x20089, 0x200CC, 0x215D7, 0x2298F, 0x241FE
    };
    static const unsigned int nonpua_nonbmp_vals[] = {
        0xFE51, 0xFE52, 0xFE53, 0xFE6C, 0xFE76, 0xFE91
    };

#ifdef TEST_JUST_SAY_GNO
    int j;
    clock_t start;
    clock_t total = 0, total_gno = 0;
#else
    (void)debug;
#endif

    testStart("test_u_gb18030_int");

#ifdef TEST_JUST_SAY_GNO
    if ((debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        printf("test_u_gb18030_int perf iterations: %d\n", TEST_INT_PERF_ITERATIONS);
    }
#endif

    for (i = 0; i < 0x10400; i++) { /* Don't bother with U+10400..U+10FFFF, programmatically filled */
        if (i >= 0xD800 && i <= 0xDFFF) { /* UTF-16 surrogates */
            continue;
        }
        if (testContinue(p_ctx, i)) continue;
        val1_1 = val1_2 = val2_1 = val2_2 = 0;
        ret = u_gb18030_int_test(i, &val1_1, &val1_2);
        ret2 = u_gb18030_int2(i, &val2_1, &val2_2);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val1_1 0x%04X, val2_1 0x%04X, val1_2 0x%04X, val2_2 0x%04X\n", (int) i, i, ret, ret2, val1_1, val2_1, val1_2, val2_2);
        if (ret2) {
            assert_equal(val1_1, val2_1, "i:%d 0x%04X val1_1 0x%04X != val2_1 0x%04X\n", (int) i, i, val1_1, val2_1);
            assert_equal(val1_2, val2_2, "i:%d 0x%04X val1_2 0x%04X != val2_2 0x%04X\n", (int) i, i, val1_2, val2_2);
        }
#ifdef TEST_JUST_SAY_GNO
        if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
            val2_1 = val2_2 = 0;
            ret2 = gb18030_wctomb_zint(&val2_1, &val2_2, i);
        } else {
            for (j = 0; j < TEST_INT_PERF_ITERATIONS; j++) {
                val1_1 = val1_2 = val2_1 = val2_2 = 0;

                start = clock();
                ret = u_gb18030_int_test(i, &val1_1, &val1_2);
                total += clock() - start;

                start = clock();
                ret2 = gb18030_wctomb_zint(&val2_1, &val2_2, i);
                total_gno += clock() - start;
            }
        }

        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val1_1 0x%04X, val2_1 0x%04X, val1_2 0x%04X, val2_2 0x%04X\n", (int) i, i, ret, ret2, val1_1, val2_1, val1_2, val2_2);
        if (ret2) {
            assert_equal(val1_1, val2_1, "i:%d 0x%04X val1_1 0x%04X != val2_1 0x%04X\n", (int) i, i, val1_1, val2_1);
            assert_equal(val1_2, val2_2, "i:%d 0x%04X val1_2 0x%04X != val2_2 0x%04X\n", (int) i, i, val1_2, val2_2);
        }
#endif
    }

    /* u_gb18030() assumes valid Unicode so now returns a nonsense value here */
    val1_1 = val1_2 = 0;
    ret = u_gb18030_int_test(0x110000, &val1_1, &val1_2); /* Invalid Unicode codepoint */
    assert_equal(ret, 4, "0x110000 ret %d != 4, val1_1 0x%04X, val1_2 0x%04X\n", ret, val1_1, val1_2);

    for (i = 0; i < ARRAY_SIZE(nonpua_nonbmp); i++) {
        val1_1 = val1_2 = 0;
        ret = u_gb18030_int_test(nonpua_nonbmp[i], &val1_1, &val1_2);
        assert_equal(ret, 2, "i:%d 0x%04X ret %d != 2, val1_1 0x%04X, val1_2 0x%04X\n", (int) i, nonpua_nonbmp[i], ret, val1_1, val1_2);
        assert_equal(val1_1, nonpua_nonbmp_vals[i], "i:%d 0x%04X val1_1 0x%04X != 0x%04X\n", (int) i, nonpua_nonbmp[i], val1_1, nonpua_nonbmp_vals[i]);
        assert_zero(val1_2, "i:%d 0x%04X val1_2 0x%04X != 0\n", (int) i, nonpua_nonbmp[i], val1_2);
    }

#ifdef TEST_JUST_SAY_GNO
    if ((debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        printf("test_u_gb18030_int perf totals: new % 8gms, gno % 8gms ratio %g\n",
                TEST_PERF_TIME(total), TEST_PERF_TIME(total_gno), TEST_PERF_RATIO(total, total_gno));
    }
#endif

    testFinish();
}

static void test_gb18030_utf8(const testCtx *const p_ctx) {

    struct item {
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[30];
        char *comment;
    };
    /*
       é U+00E9 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 18030 0xA8A6, UTF-8 C3A9
       β U+03B2 in ISO 8859-7 Greek (but not other ISO 8859 or Win page), in GB 18030 0xA6C2, UTF-8 CEB2
       ¤ U+00A4 in ISO 8859-1 plus other ISO 8859 (but not in ISO 8859-7 or ISO 8859-11), Win 1250 plus other Win, in GB 18030 0xA1E8, UTF-8 C2A4
       ¥ U+00A5 in ISO 8859-1 0xA5, in GB 18030 4-byte 0x81308436, UTF-8 C2A5
       ・ U+30FB katakana middle dot, not in any ISO or Win page, in GB 18030 0xA1A4, duplicate of mapping of U+00B7, UTF-8 E383BB
       · U+00B7 middle dot in ISO 8859-1 0xB7, in GB 18030 "GB 18030 subset" 0xA1A4, duplicate of mapping of U+30FB, UTF-8 C2B7
       ― U+2015 horizontal bar in ISO 8859-7 Greek and ISO 8859-10 Nordic, not in any Win page, in GB 18030 "GB18030.TXT" 0xA1AA, duplicate of mapping of U+2014, UTF-8 E28095
       — U+2014 em dash, not in any ISO, in Win 1250 and other Win, in GB 18030 "GB 18030 subset" 0xA1AA, duplicate of mapping of U+2015, UTF-8 E28094
    */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
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
        /*  9*/ { "\200", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Invalid UTF-8" },
        /* 10*/ { "\357\277\276", -1, 0, 2, { 0x8431, 0xA438 }, "U+FFFE (reversed BOM)" },
        /* 11*/ { "\357\277\277", -1, 0, 2, { 0x8431, 0xA439 }, "U+FFFF" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};
    unsigned int gbdata[30];

    testStart("test_gb18030_utf8");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = gb18030_utf8(&symbol, (unsigned char *) data[i].data, &ret_length, gbdata);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
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

static void test_gb18030_utf8_to_eci(const testCtx *const p_ctx) {

    struct item {
        int eci;
        int full_multibyte;
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[30];
        char *comment;
    };
    /*
       é U+00E9 in ISO 8859-1 0xE9, Win 1250 plus other Win, in HANXIN Chinese mode first byte range 0x81..FE
       β U+03B2 in ISO 8859-7 Greek 0xE2 (but not other ISO 8859 or Win page)
       ¥ U+00A5 in ISO 8859-1 0xA5, in first byte range 0x81..FE
       ÿ U+00FF in ISO 8859-1 0xFF, outside first byte and second/third/fourth byte ranges
       @ U+0040 in ASCII 0x40, outside first byte range, in double-byte second byte range, outside quad-byte second/third/fourth byte ranges
       9 U+0039 in ASCII 0x39, outside first byte range, outside double-byte second byte range and quad-byte third byte range, in quad-byte second/fourth byte ranges
    */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 3, 0, "é", -1, 0, 1, { 0xE9 }, "Not full multibyte" },
        /*  1*/ { 3, 1, "é", -1, 0, 1, { 0xE9 }, "First byte in range but only one byte" },
        /*  2*/ { 3, 0, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Not full multibyte" },
        /*  3*/ { 3, 1, "β", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Not in ECI 3 (ISO 8859-1)" },
        /*  4*/ { 9, 0, "β", -1, 0, 1, { 0xE2 }, "Not full multibyte" },
        /*  5*/ { 9, 1, "β", -1, 0, 1, { 0xE2 }, "In ECI 9 (ISO 8859-7)" },
        /*  6*/ { 3, 0, "¥", -1, 0, 1, { 0xA5 }, "Not full multibyte" },
        /*  7*/ { 3, 1, "¥", -1, 0, 1, { 0xA5 }, "First byte in range but only one byte" },
        /*  8*/ { 3, 0, "¥é", -1, 0, 2, { 0xA5, 0xE9 }, "Not full multibyte" },
        /*  9*/ { 3, 1, "¥é", -1, 0, 1, { 0xA5E9 }, "In double-byte range" },
        /* 10*/ { 3, 0, "¥ÿ", -1, 0, 2, { 0xA5, 0xFF }, "Not full multibyte" },
        /* 11*/ { 3, 1, "¥ÿ", -1, 0, 2, { 0xA5, 0xFF }, "First byte in range but not second" },
        /* 12*/ { 3, 0, "¥9é9", -1, 0, 4, { 0xA5, 0x39, 0xE9, 0x39 }, "Not full multibyte" },
        /* 13*/ { 3, 1, "¥9é9", -1, 0, 2, { 0xA539, 0xE939 }, "In quad-byte range" },
        /* 14*/ { 3, 0, "¥9", -1, 0, 2, { 0xA5, 0x39 }, "Not full multibyte" },
        /* 15*/ { 3, 1, "¥9", -1, 0, 2, { 0xA5, 0x39 }, "In quad-byte first/second range but only 2 bytes, not in double-byte range" },
        /* 16*/ { 3, 0, "¥9é", -1, 0, 3, { 0xA5, 0x39, 0xE9 }, "Not full multibyte" },
        /* 17*/ { 3, 1, "¥9é", -1, 0, 3, { 0xA5, 0x39, 0xE9 }, "In quad-byte first/second/third range but only 3 bytes, no bytes in double-byte range" },
        /* 18*/ { 3, 0, "¥9é@", -1, 0, 4, { 0xA5, 0x39, 0xE9, 0x40 }, "Not full multibyte" },
        /* 19*/ { 3, 1, "¥9é@", -1, 0, 3, { 0xA5, 0x39, 0xE940 }, "In quad-byte first/second/third range but not fourth, second 2 bytes in double-byte range" },
        /* 20*/ { 3, 0, "¥@é9", -1, 0, 4, { 0xA5, 0x40, 0xE9, 0x39 }, "Not full multibyte" },
        /* 21*/ { 3, 1, "¥@é9", -1, 0, 3, { 0xA540, 0xE9, 0x39 }, "In quad-byte first/third/fourth range but not second, first 2 bytes in double-byte range" },
        /* 22*/ { 3, 0, "¥9@9", -1, 0, 4, { 0xA5, 0x39, 0x40, 0x39 }, "Not full multibyte" },
        /* 23*/ { 3, 1, "¥9@9", -1, 0, 4, { 0xA5, 0x39, 0x40, 0x39 }, "In quad-byte first/second/fourth range but not third, no bytes in double-byte range" },
        /* 24*/ { 3, 0, "é9éé¥9é@¥9é9¥9é0é@@¥¥é0é1", -1, 0, 25, { 0xE9, 0x39, 0xE9, 0xE9, 0xA5, 0x39, 0xE9, 0x40, 0xA5, 0x39, 0xE9, 0x39, 0xA5, 0x39, 0xE9, 0x30, 0xE9, 0x40, 0x40, 0xA5, 0xA5, 0xE9, 0x30, 0xE9, 0x31 }, "" },
        /* 25*/ { 3, 1, "é9éé¥9é@¥9é9¥9é0é@@¥¥é0é1", -1, 0, 15, { 0xE9, 0x39, 0xE9E9, 0xA5, 0x39, 0xE940, 0xA539, 0xE939, 0xA539, 0xE930, 0xE940, 0x40, 0xA5A5, 0xE930, 0xE931 }, "" },
        /* 26*/ { 20, 0, "\\\\", -1, 0, 4, { 0x81, 0x5F, 0x81, 0x5F }, "Shift JIS reverse solidus (backslash) mapping from ASCII to double byte" },
        /* 27*/ { 20, 1, "\\\\", -1, 0, 2, { 0x815F, 0x815F }, "Shift JIS in GB 18030 Hanzi mode range" },
        /* 28*/ { 20, 0, "爍", -1, 0, 2, { 0xE0, 0xA1 }, "Shift JIS U+720D" },
        /* 29*/ { 20, 1, "爍", -1, 0, 1, { 0xE0A1 }, "Shift JIS in GB 18030 Hanzi mode range" },
        /* 30*/ { 25, 0, "12", -1, 0, 4, { 0x00, 0x31, 0x00, 0x32 }, "UCS-2BE ASCII" },
        /* 31*/ { 25, 0, "", -1, 0, 4, { 0x00, 0x81, 0x00, 0x81 }, "UCS-2BE U+0081" },
        /* 32*/ { 25, 1, "", -1, 0, 4, { 0x00, 0x81, 0x00, 0x81 }, "UCS-2BE outside GB 18030 Hanzi mode range" },
        /* 33*/ { 25, 0, "ꆩꆩ", -1, 0, 4, { 0xA1, 0xA9, 0xA1, 0xA9 }, "UCS-2BE U+A1A9" },
        /* 34*/ { 25, 1, "ꆩꆩ", -1, 0, 2, { 0xA1A9, 0xA1A9 }, "UCS-2BE in GB 18030 Hanzi mode range" },
        /* 35*/ { 25, 0, "膀膀", -1, 0, 4, { 0x81, 0x80, 0x81, 0x80 }, "UCS-2BE U+8180" },
        /* 36*/ { 25, 1, "膀膀", -1, 0, 2, { 0x8180, 0x8180 }, "UCS-2BE in GB 18030 Hanzi mode range (but outside GB 2312 range)" },
        /* 37*/ { 28, 0, "¢¢", -1, 0, 4, { 0xA2, 0x46, 0xA2, 0x46 }, "Big5 U+00A2" },
        /* 38*/ { 28, 1, "¢¢", -1, 0, 2, { 0xA246, 0xA246 }, "Big5 in GB 18030 Hanzi mode range (but outside GB 2312 range)" },
        /* 39*/ { 28, 0, "陛", -1, 0, 2, { 0xB0, 0xA1 }, "Big5 U+965B" },
        /* 40*/ { 28, 1, "陛", -1, 0, 1, { 0xB0A1 }, "Big5 in GB 18030 Hanzi mode range" },
        /* 41*/ { 29, 0, "¨¨", -1, 0, 2, { 0xA1A7, 0xA1A7 }, "GB 2312 U+00A8" },
        /* 42*/ { 29, 1, "¨¨", -1, 0, 2, { 0xA1A7, 0xA1A7 }, "GB 2312" },
        /* 43*/ { 29, 0, "崂", -1, 0, 1, { 0xE1C0 }, "GB 2312 U+5D02" },
        /* 44*/ { 29, 1, "崂", -1, 0, 1, { 0xE1C0 }, "GB 2312" },
        /* 45*/ { 29, 0, "・", -1, 0, 1, { 0xA1A4 }, "GB 2312 U+30FB" },
        /* 46*/ { 29, 1, "・", -1, 0, 1, { 0xA1A4 }, "GB 2312" },
        /* 47*/ { 29, 0, "釦", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "GB 18030 U+91E6 not in GB 2312" },
        /* 48*/ { 30, 0, "¡¡", -1, 0, 4, { 0x22 + 0x80, 0x2E + 0x80, 0x22 + 0x80, 0x2E + 0x80 }, "EUC-KR U+00A1 (0xA2AE)" },
        /* 49*/ { 30, 1, "¡¡", -1, 0, 2, { 0x222E + 0x8080, 0x222E + 0x8080 }, "All EUC-KR in GB 18030 Hanzi mode range" },
        /* 50*/ { 30, 0, "詰", -1, 0, 2, { 0x7D + 0x80, 0x7E + 0x80 }, "EUC-KR U+8A70 (0xFDFE)" },
        /* 51*/ { 30, 1, "詰", -1, 0, 1, { 0x7D7E + 0x8080 }, "All EUC-KR in GB 18030 Hanzi mode range" },
        /* 52*/ { 31, 0, "條", -1, 0, 1, { 0x976C }, "GBK U+689D" },
        /* 53*/ { 31, 1, "條", -1, 0, 1, { 0x976C }, "GBK U+689D" },
        /* 54*/ { 31, 0, "條碼", -1, 0, 2, { 0x976C, 0xB461 }, "GBK U+689D" },
        /* 55*/ { 31, 1, "條碼", -1, 0, 2, { 0x976C, 0xB461 }, "GBK U+689D" },
        /* 56*/ { 31, 0, "釦", -1, 0, 1, { 0xE240 }, "GB 18030 U+91E6 in GBK" },
        /* 57*/ { 31, 0, "€", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "GB 18030 U+20AC not in GBK" },
        /* 58*/ { 32, 0, "¨¨", -1, 0, 2, { 0xA1A7, 0xA1A7 }, "GB 18030 U+00A8" },
        /* 59*/ { 32, 1, "¨¨", -1, 0, 2, { 0xA1A7, 0xA1A7 }, "GB 18030" },
        /* 60*/ { 32, 0, "崂", -1, 0, 1, { 0xE1C0 }, "GB 18030 U+5D02" },
        /* 61*/ { 32, 1, "崂", -1, 0, 1, { 0xE1C0 }, "GB 18030" },
        /* 62*/ { 32, 0, "・", -1, 0, 2, { 0x8139, 0xA739 }, "GB 18030 U+30FB" },
        /* 63*/ { 32, 1, "・", -1, 0, 2, { 0x8139, 0xA739 }, "GB 18030" },
        /* 64*/ { 32, 0, "€", -1, 0, 1, { 0xA2E3 }, "GB 18030 U+20AC " },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    unsigned int gbdata[30];

    testStart("test_gb18030_utf8_to_eci");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = gb18030_utf8_to_eci(data[i].eci, (unsigned char *) data[i].data, &ret_length, gbdata, data[i].full_multibyte);
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

INTERNAL void gb18030_cpy_test(const unsigned char source[], int *p_length, unsigned int *ddata,
                const int full_multibyte);

static void test_gb18030_cpy(const testCtx *const p_ctx) {

    struct item {
        int full_multibyte;
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_gbdata[30];
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    struct item data[] = {
        /*  0*/ { 0, "\351", -1, 0, 1, { 0xE9 }, "Not full multibyte" },
        /*  1*/ { 1, "\351", -1, 0, 1, { 0xE9 }, "In HANXIN Chinese mode first-byte range but only one byte" },
        /*  2*/ { 0, "\351\241", -1, 0, 2, { 0xE9, 0xA1 }, "Not full multibyte" },
        /*  3*/ { 1, "\351\241", -1, 0, 1, { 0xE9A1 }, "In HANXIN Chinese range" },
        /*  4*/ { 0, "\241", -1, 0, 1, { 0xA1 }, "Not full multibyte" },
        /*  5*/ { 1, "\241", -1, 0, 1, { 0xA1 }, "In first-byte range but only one byte" },
        /*  6*/ { 0, "\241\241", -1, 0, 2, { 0xA1, 0xA1 }, "Not full multibyte" },
        /*  7*/ { 1, "\241\241", -1, 0, 1, { 0xA1A1 }, "In range" },
        /*  8*/ { 0, "\241\240\241\376\367\376\367\377\2012\2013", -1, 0, 12, { 0xA1, 0xA0, 0xA1, 0xFE, 0xF7, 0xFE, 0xF7, 0xFF, 0x81, 0x32, 0x81, 0x33 }, "" },
        /*  9*/ { 1, "\241\240\241\376\367\376\367\377\2012\2013", -1, 0, 7, { 0xA1A0, 0xA1FE, 0xF7FE, 0xF7, 0xFF, 0x8132, 0x8133 }, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length;

    unsigned int gbdata[30];

    testStart("test_gb18030_cpy");

    for (i = 0; i < data_size; i++) {
        int ret_length;
        int j;

        if (testContinue(p_ctx, i)) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        gb18030_cpy_test((unsigned char *) data[i].data, &ret_length, gbdata, data[i].full_multibyte);
        assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
        for (j = 0; j < (int) ret_length; j++) {
            assert_equal(gbdata[j], data[i].expected_gbdata[j], "i:%d gbdata[%d] %04X != %04X\n", i, j, gbdata[j], data[i].expected_gbdata[j]);
        }
    }

    testFinish();
}

INTERNAL int u_gbk_int_test(const unsigned int u, unsigned int *dest);

/* Control for GBK */
static int u_gbk_int2(unsigned int u, unsigned int *dest) {
    unsigned int c;
    int tab_length, start_i, end_i;
    int i;

    tab_length = ARRAY_SIZE(test_gbk_tab);
    start_i = test_gbk_tab_ind[u >> 10];
    end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    for (i = start_i; i < end_i; i += 2) {
        if (test_gbk_tab[i + 1] == u) {
            c = test_gbk_tab[i];
            if (c <= 0xFFFF) {
                *dest = c;
                return c <= 0xFF ? 1 : 2;
            }
            return 0;
        }
    }
    return 0;
}

static void test_u_gbk_int(const testCtx *const p_ctx) {

    int ret, ret2;
    unsigned int val, val2;
    unsigned int i;

    testStart("test_u_gbk_int");

    for (i = 0; i < 0xFFFE; i++) {
        if (i >= 0xD800 && i <= 0xDFFF) { /* UTF-16 surrogates */
            continue;
        }
        if (testContinue(p_ctx, i)) continue;

        val = val2 = 0;
        ret = u_gbk_int_test(i, &val);
        ret2 = u_gbk_int2(i, &val2);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
        }
    }

    testFinish();
}

#define TEST_PERF_ITER_MILLES   100
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)

/* Not a real test, just performance indicator */
static void test_perf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        char *data;
        int ret;

        char *comment;
    };
    struct item data[] = {
        /*  0*/ { "1234567890", 0, "10 numerics" },
        /*  1*/ { "条码北京條碼པེ་ཅིང།バーコード바코드", 0, "Small various code pages" },
        /*  2*/ { "Summer Palace Ticket for 6 June 2015 13:00;2015年6月6日夜01時00分PM頤和園のチケット;2015년6월6일13시오후여름궁전티켓.2015年6月6号下午13:00的颐和园门票;", 0, "Small mixed ASCII/Hanzi" },
        /*  3*/ { "汉信码标准\015\012中国物品编码中心\015\012北京网路畅想科技发展有限公司\015\012张成海、赵楠、黄燕滨、罗秋科、王毅、张铎、王越\015\012施煜、边峥、修兴强\015\012汉信码标准\015\012中国物品编码中心\015\012北京网路畅想科技发展有限公司", 0, "Bigger mixed" },
        /*  4*/ { "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\015\012", 0, "Bigger mixed" },
        /*  5*/ { "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\015\012本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40\015\012本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法RS、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术   122", 0, "Medium mixed" },
        /*  6*/ { "本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：84\015\012本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：84\015\012本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方案、信息编码方法、纠错编译码算法、信息排布方法、参考译码算法等内容进行了详细的描述，汉信码可高效表示《GB 18030—2000 信息技术 信息交换用汉字编码字符集基本集的扩充》中的汉字信息，并具有数据容量大、抗畸变和抗污损能力强、外观美观等特点，适合于在我国各行业的广泛应用。 测试文本，测试人：施煜，边峥，修兴强，袁娲，测试目的：汉字表示，测试版本：40本标准规定了一种矩阵式二维条码——汉信码的码制以及编译码方法。本标准中对汉信码的码图方", 0, "Bigger mixed" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};
    int ret_length;
#ifdef TEST_JUST_SAY_GNO
    int ret_length2;
#endif
    unsigned int ddata[8192];
    unsigned char dest[8192];
    int ret2 = 0;
#ifdef TEST_JUST_SAY_GNO
    unsigned int ddata2[8192];
#endif

    clock_t start;
    clock_t total = 0, total_gno = 0, total_eci = 0;
    clock_t diff, diff_gno, diff_eci;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        length = (int) strlen(data[i].data);

        diff = diff_gno = diff_eci = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            ret_length = length;

            start = clock();
            ret = gb18030_utf8(&symbol, (unsigned char *) data[i].data, &ret_length, ddata);
            diff += clock() - start;

#ifdef TEST_JUST_SAY_GNO
            ret_length2 = length;
            start = clock();
            ret2 = gb18030_utf8_wctomb(&symbol, (unsigned char *) data[i].data, &ret_length2, ddata2);
            diff_gno += clock() - start;
#endif
            ret_length = length;

            start = clock();
            (void)utf8_to_eci(32, (unsigned char *) data[i].data, dest, &ret_length);
            diff_eci += clock() - start;
        }
        assert_equal(ret, ret2, "i:%d ret %d != ret2 %d\n", (int) i, ret, ret2);

        printf("%*s: new % 8gms, gno % 8gms ratio % 9g, eci %gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff), TEST_PERF_TIME(diff_gno), TEST_PERF_RATIO(diff, diff_gno), TEST_PERF_TIME(diff_eci));

        total += diff;
        total_gno += diff_gno;
    }
    if (p_ctx->index == -1) {
        printf("%*s: new % 8gms, gno % 8gms ratio % 9g, eci %gms\n", comment_max, "totals",
                TEST_PERF_TIME(total), TEST_PERF_TIME(total_gno), TEST_PERF_RATIO(total, total_gno), TEST_PERF_TIME(total_eci));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_u_gb18030_int", test_u_gb18030_int },
        { "test_gb18030_utf8", test_gb18030_utf8 },
        { "test_gb18030_utf8_to_eci", test_gb18030_utf8_to_eci },
        { "test_gb18030_cpy", test_gb18030_cpy },
        { "test_u_gbk_int", test_u_gbk_int },
        { "test_perf", test_perf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
