/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2020 Robin Stuart <rstuart114@gmail.com>

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

static void test_utf8_to_unicode(int index, int debug) {

    testStart("");

    int ret;
    struct item {
        char *data;
        int length;
        int disallow_4byte;
        int ret;
        int ret_length;
        unsigned int expected_vals[20];
        char *comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "", -1, 1, 0, 0, {0}, "" },
        /*  1*/ { "\000a\302\200\340\240\200", 7, 1, 0, 4, { 0, 'a', 0x80, 0x800 }, "NUL a C280 E0A080" },
        /*  2*/ { "\357\277\277", -1, 1, 0, 1, { 0xFFFF }, "EFBFBF" },
        /*  3*/ { "\360\220\200\200", -1, 1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Four-byte F0908080" },
        /*  4*/ { "a\200b", -1, 1, ZINT_ERROR_INVALID_DATA, -1, {0}, "Orphan continuation 0x80" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    unsigned int vals[20];
    struct zint_symbol symbol;
    symbol.debug |= debug;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        int length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;
        int ret_length = length;

        ret = utf8_to_unicode(&symbol, (unsigned char *) data[i].data, vals, &ret_length, data[i].disallow_4byte);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
        if (ret == 0) {
            assert_equal(ret_length, data[i].ret_length, "i:%d ret_length %d != %d\n", i, ret_length, data[i].ret_length);
            for (int j = 0; j < ret_length; j++) {
                assert_equal(vals[j], data[i].expected_vals[j], "i:%d vals[%d] %04X != %04X\n", i, j, vals[j], data[i].expected_vals[j]);
            }
        }
    }

    testFinish();
}

static void test_is_valid_utf8(int index) {

    testStart("");

    int ret;
    struct item {
        char* data;
        int length;
        int ret;
        char* comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { "", -1, 1, "" },
        /*  1*/ { "abcdefghijklmnopqrstuvwxyz", -1, 1, "" },
        /*  2*/ { "éa", -1, 1, "" },
        /*  3*/ { "a\000b", 3, 1, "Embedded nul" },
        /*  4*/ { "\357\273\277a", -1, 1, "Bom" },

        /*  5*/ { "a\xC2", -1, 0, "Missing 2nd byte" },
        /*  6*/ { "a\200b", -1, 0, "Orphan continuation 0x80" },
        /*  7*/ { "\300\201", -1, 0, "Overlong 0xC081" },
        /*  8*/ { "\355\240\200", -1, 0, "Surrogate 0xEDA080" },
    };
    int data_size = ARRAY_SIZE(data);

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        int length = data[i].length == -1 ? (int) strlen(data[i].data) : data[i].length;

        ret = is_valid_utf8((const unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ret %d != %d\n", i, ret, data[i].ret);
    }

    testFinish();
}

static void test_debug_test_codeword_dump_int(int index, int debug) {

    testStart("");

    struct item {
        int codewords[50];
        int length;
        char *expected;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { { 2147483647, -2147483646, 2147483647, 0, 2147483647, 2147483647, 2147483647, 2147483647, 123456 }, 10, "(10) 2147483647 -2147483646 2147483647 0 2147483647 2147483647 2147483647 2147483647 123456" },
        /*  1*/ { { 2147483647, -2147483646, 2147483647, 0, 2147483647, 2147483647, 2147483647, 2147483647, 1234567 }, 10, "(10) 2147483647 -2147483646 2147483647 0 2147483647 2147483647 2147483647 2147483647" },
    };
    int data_size = sizeof(data) / sizeof(struct item);

    struct zint_symbol symbol;
    symbol.debug |= debug;

    for (int i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        debug_test_codeword_dump_int(&symbol, data[i].codewords, data[i].length);
        assert_nonzero(strlen(symbol.errtxt) < 92, "i:%d strlen(%s) >= 92 (%d)\n", i, symbol.errtxt, (int) strlen(symbol.errtxt));
        assert_zero(strcmp(symbol.errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0 (%d, %d)\n", i, symbol.errtxt, data[i].expected, (int) strlen(symbol.errtxt), (int) strlen(data[i].expected));
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_utf8_to_unicode", test_utf8_to_unicode, 1, 0, 1 },
        { "test_debug_test_codeword_dump_int", test_debug_test_codeword_dump_int, 1, 0, 1 },
        { "test_is_valid_utf8", test_is_valid_utf8, 1, 0, 0 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
