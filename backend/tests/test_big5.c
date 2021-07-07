/*
    libzint - the open source barcode library
    Copyright (C) 2021 Robin Stuart <rstuart114@gmail.com>

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
#include "test_big5_tab.h"
#include "../big5.h"

// As control convert to Big5 using simple table generated from https://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/OTHER/BIG5.TXT plus simple processing
static int big5_wctomb_zint2(unsigned int *r, unsigned int wc) {
    int tab_length = ARRAY_SIZE(test_big5_tab);
    int start_i = test_big5_tab_ind[wc >> 10];
    int end_i = start_i + 0x800 > tab_length ? tab_length : start_i + 0x800;
    int i;
    if (wc < 0x80) {
        return 0;
    }
    for (i = start_i; i < end_i; i += 2) {
        if (test_big5_tab[i + 1] == wc) {
            *r = test_big5_tab[i];
            return *r > 0xFF ? 2 : 1;
        }
    }
    return 0;
}

static void test_big5_wctomb_zint(void) {

    unsigned int i;
    int ret, ret2;
    unsigned int val, val2;

    testStart("test_big5_wctomb_zint");

    for (i = 0; i < 0xFFFE; i++) {
        if (i >= 0xD800 && i < 0xE000) { // UTF-16 surrogates
            continue;
        }
        val = val2 = 0;
        ret = big5_wctomb_zint(&val, i);
        ret2 = big5_wctomb_zint2(&val2, i);
        assert_equal(ret, ret2, "i:%d 0x%04X ret %d != ret2 %d, val 0x%04X, val2 0x%04X\n", (int) i, i, ret, ret2, val, val2);
        if (ret2) {
            assert_equal(val, val2, "i:%d 0x%04X val 0x%04X != val2 0x%04X\n", (int) i, i, val, val2);
        }
    }

    testFinish();
}

/* Convert UTF-8 string to Big5 and place in array of ints */
static int big5_utf8(struct zint_symbol *symbol, const unsigned char source[], int *p_length,
                unsigned int *b5data) {
    int error_number;
    unsigned int i, length;
#ifndef _MSC_VER
    unsigned int utfdata[*p_length + 1];
#else
    unsigned int *utfdata = (unsigned int *) _alloca((*p_length + 1) * sizeof(unsigned int));
#endif

    error_number = utf8_to_unicode(symbol, source, utfdata, p_length, 0 /*disallow_4byte*/);
    if (error_number != 0) {
        return error_number;
    }

    for (i = 0, length = *p_length; i < length; i++) {
        if (!big5_wctomb_zint(b5data + i, utfdata[i])) {
            strcpy(symbol->errtxt, "800: Invalid character in input data");
            return ZINT_ERROR_INVALID_DATA;
        }
    }

    return 0;
}
static void test_big5_utf8(int index) {

    struct item {
        char *data;
        int length;
        int ret;
        int ret_length;
        unsigned int expected_b5data[20];
        char *comment;
    };
    // ＿ U+FF3F fullwidth low line, not in ISO/Win, in Big5 0xA1C4, UTF-8 EFBCBF
    // ╴ U+2574 drawings box light left, not in ISO/Win, not in original Big5 but in "Big5-2003" as 0xA15A, UTF-8 E295B4
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "＿", -1, 0, 1, { 0xA1C4 }, "" },
        /*  1*/ { "╴", -1, ZINT_ERROR_INVALID_DATA, -1, {0}, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;

    struct zint_symbol symbol = {0};
    unsigned int b5data[20];

    testStart("test_big5_utf8");

    for (i = 0; i < data_size; i++) {
        int ret_length;

        if (index != -1 && i != index) continue;

        length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        ret_length = length;

        ret = big5_utf8(&symbol, (unsigned char *) data[i].data, &ret_length, b5data);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d (%s)\n", i, ret, data[i].ret, symbol.errtxt);
        if (ret == 0) {
            int j;
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (j = 0; j < ret_length; j++) {
                assert_equal(b5data[j], data[i].expected_b5data[j], "i:%d b5data[%d] %04X != %04X\n", i, j, b5data[j], data[i].expected_b5data[j]);
            }
        }
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_big5_wctomb_zint", test_big5_wctomb_zint, 0, 0, 0 },
        { "test_big5_utf8", test_big5_utf8, 1, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
